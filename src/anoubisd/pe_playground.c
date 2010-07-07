/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <err.h>
#include <errno.h>
#ifdef LINUX
#include <grp.h>
#include <bsdcompat.h>
#endif

#include "anoubisd.h"
#include "pe.h"
#include "anoubis_alloc.h"
#include "compat_openat.h"
#include "anoubis_errno.h"

/**
 * This structure describes a single playground. Fields:
 * next: Link to the next playground structure on the playground list.
 *     The list is rooted at the global variable playgrounds.
 * pgid: The playground ID of the playground.
 * nrprocs: The total number of running processes in the playground.
 * cmd: The command (program) that was first started in the playground.
 *     This is modified once after the first exec in the playground to
 *     make sure that anoubisctl and/or xanoubis do not show up as the
 *     playground processes.
 * uid: The user-ID of the user that starts the playground process.
 * did_exec: Initialized to zero and set to one after the first process
 *     in the playground calls exec.
 * dirfd: An open file descriptor that points to the playground directory.
 *     in /var/lib/anoubis/playground/.
 */
struct playground {
	LIST_ENTRY(playground)			 next;
	anoubis_cookie_t			 pgid;
	int					 nrprocs;
	char					*cmd;
	uid_t					 uid;
	int					 did_exec;
	struct atfd				 dirfd;
};

/**
 * This is a linked list of all playground that exist in the system.
 * Each entry of the list is a struct playground.
 */
static LIST_HEAD(, playground)		playgrounds =
					    LIST_HEAD_INITIALIZER(playgrounds);

/**
 * Search the playground list for a playground with the given ID and
 * return a pointer to the playground structure.
 *
 * @param pgid The playground ID to search for.
 * @return The playground structure of the playground of NULL if the
 *     playground ID is not in use.
 */
static struct playground *
pe_playground_find(anoubis_cookie_t pgid)
{
	struct playground	*pg;

	LIST_FOREACH(pg, &playgrounds, next) {
		if (pg->pgid == pgid)
			return pg;
	}
	return NULL;
}

/**
 * Create a new playground. This function allocates and initializes a
 * new playground structure. Optionally, it creates the management directory
 * in ANOUBISD_PG, too.
 *
 * @param pgid The playground ID of the new playground.
 * @param do_mkdir True if the management directory in ANOUBISD_PG should
 *     be created. A warning is printed if this is parameter is true and
 *     the directory exists.
 * @return A pointer to the new playground structure.
 */
static struct playground *
pe_playground_create(anoubis_cookie_t pgid, int do_mkdir)
{
	struct playground	*pg = pe_playground_find(pgid);
	char			 pgname[30];
	int			 err;

	if (pg)
		return pg;
	snprintf(pgname, sizeof(pgname), "%s/%" PRIx64,
	    ANOUBISD_PGCHROOT, pgid);
	if (do_mkdir) {
		if (mkdir(pgname, 0750) < 0) {
			if (errno == EEXIST)
				log_warnx("Playground %" PRIx64
				    " already exists", pgid);
			else
				log_warnx("Cannot create Playground "
				    "dir %" PRIx64, pgid);
		}
	}
	pg = abuf_alloc_type(struct playground);
	if (!pg)
		return NULL;
	err = atfd_open(&pg->dirfd, pgname);
	if (err < 0)
		log_warnx("pe_playground_create: Cannot open playground "
		    "directory: %s", anoubis_strerror(-err));
	pg->pgid = pgid;
	pg->nrprocs = 0;
	pg->cmd = NULL;
	pg->uid = (uid_t)-1;
	pg->did_exec = 0;
	LIST_INSERT_HEAD(&playgrounds, pg, next);
	return pg;
}

/**
 * Read the contents of a playground file in the given playground and
 * store the data in the buffer. At most bufsize-1 bytes are stored and the
 * buffer in terminated by a NUL byte. If an error occurs an appropriate
 * message is logged.
 *
 * @param pg The playground to read from.
 * @param path The path of the file relative to the playground directory
 *     in pg->dirfd.
 * @param buf The buffer for the result.
 * @param bufsize The length of the buffer.
 * @return The number of bytes placed into the buffer or -1 if an error
 *     occured. In the latter case an error message is logged.
 */
static int
pe_playground_readfile(struct playground *pg, const char *path, char *buf,
    int bufsize)
{
	int fd = atfd_openat(&pg->dirfd, path, O_RDONLY, 0);
	int ret;

	if (fd < 0) {
		log_warnx("pe_playground_readfile: Cannot open file %s "
		    "in playground directory %" PRIx64, path, pg->pgid);
		return -1;
	}
	ret = read(fd, buf, bufsize-1);
	close(fd);
	if (ret < 0) {
		log_warnx("pe_playground_readfile: Failed to read from "
		    "file %s in playground directory %" PRIx64, path, pg->pgid);
		return -1;
	}
	buf[ret] = 0;
	return ret;
}

/**
 * Write the given string to a file in the playground directory of the
 * playground. This function writes at most strlen(str) bytes to the file
 * descriptor in pg->dirfd. Any error results in an appropriate log message.
 *
 * @param pg The playground structure of the playground.
 * @param path The path relative to the playground binary.
 * @param str The string to write.
 * @return None. Appropriate error messages are logged if something goes wrong.
 */
static void
pe_playground_writefile(struct playground *pg, const char *path, char *str)
{
	int fd = atfd_openat(&pg->dirfd, path, O_WRONLY|O_CREAT|O_TRUNC, 0640);
	int len, ret;

	if (fd < 0) {
		log_warnx("pe_playground_writefile: Cannot open file %s "
		    "in playground directory %" PRIx64, path, pg->pgid);
		return;
	}
	if (!str) {
		close(fd);
		return;
	}
	len = strlen(str);
	ret = write(fd, str, len);
	close(fd);
	if (ret < 0) {
		log_warnx("pe_playground_writefile: Failed to write to "
		    "file %s in playground directory %" PRIx64, path, pg->pgid);
	} else if (ret != len) {
		log_warnx("pe_playground_writefile: Partial write to file "
		    "%s in playground directory %" PRIx64, path, pg->pgid);
	}
}

/**
 * Initialize the fields of the playground structure that depend on
 * the first process that creates the playground.
 *
 * @param The playground structure.
 * @param The process that creates the playground.
 * @return None.
 */
static void
pe_playground_initproc(struct playground *pg, struct pe_proc *proc)
{
	struct pe_proc_ident	*ident = pe_proc_ident(proc);
	char			 buf[20];

	pg->uid = pe_proc_get_uid(proc);
	snprintf(buf, 20, "%d", pg->uid);
	pe_playground_writefile(pg, "UID", buf);
	if (pg->cmd) {
		free(pg->cmd);
		pg->cmd = NULL;
	}
	if (ident && ident->pathhint)
		pg->cmd = strdup(ident->pathhint);
	pe_playground_writefile(pg, "CMD", ident ? ident->pathhint : NULL);
}

/**
 * Initailize the fields of the playground structure that depend on the
 * first playground process. This is only done for the first exec in this
 * playground.
 *
 * @param pgid The playground that has seen an exec.
 * @param proc The process that just did an exec.
 * @return None.
 */
void
pe_playground_postexec(anoubis_cookie_t pgid, struct pe_proc *proc)
{
	struct playground	*pg = pe_playground_find(pgid);

	if (!pg) {
		log_warnx("pe_playground_postexec: Non existent playground "
		    "%" PRIx64, pgid);
		return;
	}
	if (!pg->did_exec) {
		pe_playground_initproc(pg, proc);
		pg->did_exec = 1;
	}
}

/**
 * Add a process to a playground. We only increase the process counter
 * at this time. If the playground does not exist, it is created.
 *
 * @param pgid The playground ID.
 * @param proc The process.
 * @return None.
 */
void
pe_playground_add(anoubis_cookie_t pgid, struct pe_proc *proc)
{
	struct playground	*pg = pe_playground_create(pgid, 1);

	if (pg) {
		if (pg->cmd == NULL)
			pe_playground_initproc(pg, proc);
		pg->nrprocs++;
		DEBUG(DBG_PG, "pe_playground_add: pgid=0x%" PRIx64
		    " task=0x%" PRIx64 " nrprocs=%d", pg->pgid,
		    pe_proc_task_cookie(proc), pg->nrprocs);
	}
}

/**
 * Remove a process from a playground. This usually happens when the
 * process exits.
 *
 * @param pgid The playground ID.
 * @param proc The process that must be removed from the playground.
 * @return None.
 */
void
pe_playground_delete(anoubis_cookie_t pgid, struct pe_proc *proc)
{
	struct playground	*pg = pe_playground_find(pgid);

	if (pg && pg->nrprocs > 0) {
		pg->nrprocs--;
		DEBUG(DBG_PG, "pe_playground_delete: pgid=0x%" PRIx64
		    " task=0x%" PRIx64 " nrprocs=%d", pg->pgid,
		    pe_proc_task_cookie(proc), pg->nrprocs);
	} else {
		log_warnx("pe_playground_delete: Invalid playground ID %lld",
		    (long long)pgid);
	}
}

/**
 * Dump a list of all playground to the current log file.
 *
 * @param None.
 * @return None.
 */
void
pe_playground_dump(void)
{
	struct playground	*pg;

	log_info("List of known playgrounds:");
	LIST_FOREACH(pg, &playgrounds, next) {
		log_info("Playgound ID=%" PRIx64 " procs=%d", pg->pgid,
		    pg->nrprocs);
	}
}

/**
 * Read a single playground directory from disk and create a playground
 * structure for it. This function is called during initializaton of the
 * anoubisd daemon and throws a fatal error if something goes wrong.
 *
 * @param pgid The playground ID.
 * @return None.
 */
static void
pe_playground_read(anoubis_cookie_t pgid)
{
	struct playground		*pg = pe_playground_find(pgid);
	char				 buf[512];

	if (pg) {
		log_warnx("pe_playground_read: Playground %" PRIx64 " exists",
		    pgid);
	} else {
		/*
		 * Do not try to create the management directory, it
		 * already exists.
		 */
		pg = pe_playground_create(pgid, 0);
		pg->did_exec = 1;
	}
	if (!pg)
		fatal("pe_playground_read: Out of memory");
	pg->uid = (uid_t)-1;
	if (pe_playground_readfile(pg, "UID", buf, sizeof(buf)) >= 0) {
		int	uid = -1;
		sscanf(buf, "%d", &uid);
		pg->uid = uid;
	}
	if (pg->cmd) {
		free(pg->cmd);
		pg->cmd = NULL;
	}
	if (pe_playground_readfile(pg, "CMD", buf, sizeof(buf)) >= 0)
		pg->cmd = strdup(buf);
}

/**
 * Initialize the playground subsystem.
 *
 * @param None.
 * @return None.
 */
void
pe_playground_init(void)
{
	DIR			*pgdir;
	struct dirent		*dent;
	anoubis_cookie_t	 pgid;

	pgdir = opendir(ANOUBISD_PGCHROOT);
	if (pgdir == NULL)
		fatal("Cannot read playground directory");
	while ((dent = readdir(pgdir)) != NULL) {
		char		dummy;

		if (dent->d_name[0] == '.')
			continue;
		if (sscanf("%" PRIx64 "%c", dent->d_name, &pgid, &dummy) != 1) {
			log_warnx("pe_playground_init: Skipping malformed "
			    "entry %s in " ANOUBISD_PG, dent->d_name);
			continue;
		}
		pe_playground_read(pgid);
	}
	closedir(pgdir);
}
