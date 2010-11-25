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

#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <time.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <err.h>
#include <errno.h>
#include <stddef.h>
#ifdef LINUX
#include <grp.h>
#include <bsdcompat.h>
#endif

#ifdef OPENBSD
#include <bsdcompat/dev/anoubis_playground.h>
#endif

#include "anoubisd.h"
#include "amsg.h"
#include "anoubis_alloc.h"
#include "pe.h"
#include "anoubis_alloc.h"
#include "compat_openat.h"
#include "anoubis_errno.h"
#include "amsg_list.h"

/**
 * This structure describes a single playground.
 */
struct playground {
	/**
	 * Link to the next playground structure on the playground list.
	 * The list is rooted at the global variable playgrounds.
	 */
	LIST_ENTRY(playground)			 next;

	/**
	 *  The playground ID of the playground.
	 */
	anoubis_cookie_t			 pgid;

	/**
	 * The playgrond ID of the initial playground process. The
	 * playground name is derived from the command of this process and
	 * the first exec of this process changes the playground name.
	 */
	anoubis_cookie_t			 founder;

	/**
	 * The time that the playground was started.
	 */
	uint64_t				 starttime;
	uint64_t				 scandev;
	uint64_t				 scanino;
	uint64_t				 scantok;

	/**
	 * The total number of running processes in the playground.
	 */
	int					 nrprocs;

	/**
	 * The total number of playground files that are known to this
	 * playground.
	 */
	int					 nrfiles;

	/**
	 * The command (program) that was first started in the playground.
	 * This is modified once after the first exec in the playground to
	 * make sure that anoubisctl and/or xanoubis do not show up as the
	 * playground processes.
	 */
	char					*cmd;

	/**
	 * The user-ID of the user that starts the playground process.
	 */
	uid_t					 uid;

	/**
	 * did_exec: Initialized to zero and set to one after the first process
	 *     in the playground calls exec.
	 */
	int					 did_exec;

	/**
	 * An open file descriptor that points to the playground directory.
	 * in /var/lib/anoubis/playground/.
	 */
	struct atfd				 dirfd;
};

/**
 * This is a linked list of all playgrounds that exist in the system.
 * Each entry of the list is a struct playground.
 */
static LIST_HEAD(, playground)		playgrounds =
					    LIST_HEAD_INITIALIZER(playgrounds);

/**
 * Search the playground list for a playground with the given ID and
 * return a pointer to the playground structure.
 *
 * @param pgid The playground ID to search for.
 * @return The playground structure of the playground or NULL if the
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
				log_warn("Cannot create Playground "
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
	pg->founder = 0;
	pg->nrprocs = 0;
	pg->nrfiles = 0;
	pg->cmd = NULL;
	pg->uid = (uid_t)-1;
	pg->did_exec = 0;
	pg->starttime = 0;
	pg->scandev = 0;
	pg->scanino = 0;
	pg->scantok = 0;
	LIST_INSERT_HEAD(&playgrounds, pg, next);
	return pg;
}

/**
 * Try to remove a playground from the playground list. This will remove
 * the playground directory in /var/lib/anoubis/playground, too.
 *
 * @param pg The playground structure.
 */
static void
pe_playground_trykill(struct playground *pg)
{
	DIR		*dir;
	struct dirent	*dent;
	char		 buf[100];

	if (pg->nrprocs || pg->nrfiles)
		return;
	dir = atfd_fdopendir(&pg->dirfd);
	if (!dir) {
		log_warn("pe_playground_trykill: Cannot open directory "
		    "for playground %" PRIx64, pg->pgid);
		return;
	}
	while ((dent = readdir(dir)) != NULL) {
		if (strcmp(dent->d_name, ".") == 0)
			continue;
		if (strcmp(dent->d_name, "..") == 0)
			continue;
		if (atfd_unlinkat(&pg->dirfd, dent->d_name) != 0)
			log_warn("pe_playground_trykill: Cannot remove "
			    "entry %s in playground directory %" PRIx64,
			    dent->d_name, pg->pgid);
	}
	closedir(dir);
	atfd_close(&pg->dirfd);
	snprintf(buf, sizeof(buf), "%s/%" PRIx64, ANOUBISD_PGCHROOT, pg->pgid);
	if (rmdir(buf) != 0)
		log_warn("pe_playground_trykill: Cannot remove playground "
		    "directory %" PRIx64, pg->pgid);
	LIST_REMOVE(pg, next);
	if (pg->cmd)
		free(pg->cmd);
	abuf_free_type(pg, struct playground);
}

/**
 * Read the contents of a playground file in the given playground and
 * store the data in the buffer. At most bufsize-1 bytes are stored and the
 * buffer is terminated by a NUL byte. If an error occurs an appropriate
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
		log_warn("pe_playground_readfile: Cannot open file %s "
		    "in playground directory %" PRIx64, path, pg->pgid);
		return -1;
	}
	ret = read(fd, buf, bufsize-1);
	close(fd);
	if (ret < 0) {
		log_warn("pe_playground_readfile: Failed to read from "
		    "file %s in playground directory %" PRIx64, path, pg->pgid);
		return -1;
	}
	buf[ret] = 0;
	return ret;
}

/**
 * Write the given string to a file in the playground directory of the
 * playground. The file is created or truncated. This function writes at
 * most strlen(str) bytes to the file descriptor in pg->dirfd. Any error
 * results in an appropriate log message.
 *
 * @param pg The playground structure of the playground.
 * @param path The path relative to the playground directory.
 * @param str The string to write.
 * @return None. Appropriate error messages are logged if something goes wrong.
 */
static void
pe_playground_writefile(struct playground *pg, const char *path, char *str)
{
	int fd = atfd_openat(&pg->dirfd, path, O_WRONLY|O_CREAT|O_TRUNC, 0640);
	int len, ret;

	if (fd < 0) {
		log_warn("pe_playground_writefile: Cannot open file %s "
		    "in playground directory %" PRIx64, path, pg->pgid);
		return;
	}
	if (!str) {
		if (close(fd) < 0)
			log_warn("pe_playground_writefile: Cannot truncate "
			    "file %s in playground directory %" PRIx64, path,
			    pg->pgid);

		return;
	}
	len = strlen(str);
	ret = write(fd, str, len);
	if (ret < 0) {
		log_warn("pe_playground_writefile: Failed to write to "
		    "file %s in playground directory %" PRIx64, path, pg->pgid);
		close(fd);
		return;
	}
	else if (ret != len) {
		log_warn("pe_playground_writefile: Partial write to file "
		    "%s in playground directory %" PRIx64, path, pg->pgid);
		close(fd);
		return;
	}
	ret = close(fd);
	if (ret < 0)
		log_warn("pe_playground_writefile: Failed to write to "
		    "file %s in playground directory %" PRIx64, path, pg->pgid);

	return;
}

/**
 * Initialize the fields of the playground structure that depend on
 * the first process that creates the playground.
 *
 * @param pg The playground structure.
 * @param proc The process that creates the playground.
 * @return Returns true if the playground attributes have been changed,
 *     false otherwise.
 */
static int
pe_playground_initproc(struct playground *pg, struct pe_proc *proc)
{
	struct pe_proc_ident	*ident = pe_proc_ident(proc);
	char			 buf[40];

	if (pg->did_exec)
		return 0;
	if (pg->founder == 0) {
		pg->founder = pe_proc_task_cookie(proc);
	} else if (pg->founder != pe_proc_task_cookie(proc))
		return 0;
	pg->uid = pe_proc_get_uid(proc);
	snprintf(buf, sizeof(buf), "%d", pg->uid);
	pe_playground_writefile(pg, "UID", buf);
	pg->starttime = time(NULL);
	snprintf(buf, sizeof(buf), "%" PRId64, pg->starttime);
	pe_playground_writefile(pg, "STARTTIME", buf);
	if (pg->cmd) {
		free(pg->cmd);
		pg->cmd = NULL;
	}
	if (ident && ident->pathhint)
		pg->cmd = strdup(ident->pathhint);
	pe_playground_writefile(pg, "CMD", ident ? ident->pathhint : NULL);
	return 1;
}

/**
 * Initialize the fields of the playground structure that depend on the
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
	if (pe_playground_initproc(pg, proc)) {
		pg->did_exec = 1;
		send_pgchange(pg->uid, pg->pgid, ANOUBIS_PGCHANGE_CREATE);
		pe_playground_save_last_pgid(pg->pgid);
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
		if (pg->nrprocs == 0) {
			send_pgchange(pg->uid, pgid,
			    ANOUBIS_PGCHANGE_TERMINATE);
		}
		pe_playground_trykill(pg);
	} else {
		log_warnx("pe_playground_delete: Invalid playground ID %lld",
		    (long long)pgid);
	}
}

static void
pe_playground_devtoname(char *buf, int buflen, uint64_t dev, uint64_t ino)
{
	snprintf(buf, buflen, "%" PRIx64 ":%" PRIx64, dev, ino);
}

/**
 * Add a name to a file in the PG data dir
 *
 * @param pg The playground data structure
 * @param name The name of the PG data file ("device:inode")
 * @param path The new path name to add
 * @return -1 on error, 0 if name was not a new file,
 *         1 if name was a new file
 */
static int
pe_playground_file_add(struct playground *pg, const char *name,
	const char *path)
{
	int fd, pathoff;
	int newfile = 1;
	int ret = -1;
	char buf[512];

	fd = atfd_openat(&pg->dirfd, name, O_RDWR | O_CREAT, 0640);
	if (fd < 0) {
		log_warn("pe_playground_file_instantiate: Cannot open %s "
		    "in playground %" PRIx64, name, pg->pgid);
		return -1;
	}
	pathoff = 0;
	while (1) {
		int count = read(fd, buf, sizeof(buf));
		int i;

		if (count == 0)
			break;
		if (count < 0) {
			log_warn("pe_playground_file_instantiate: Error while "
			    "reading file %s in playgournd %" PRIx64,
			    name, pg->pgid);
			goto done;
		}
		newfile = 0;
		for (i=0; i<count; ++i) {
			if (pathoff < 0) {
				/*
				 * This path does not match. Skip up to
				 * next NUL byte.
				 */
				if (buf[i] == 0)
					pathoff = 0;
				continue;
			}
			/*
			 * We had a path match to pathoff. Compare the next
			 * character.
			 */
			if (buf[i] == path[pathoff]) {
				/*
				 * The next character mathes. Advance pathoff
				 * or return if the match is complete.
				 */
				if (path[pathoff] == 0)
					goto done;
				pathoff++;
			} else {
				/*
				 * This character does not match. Skip
				 * everything up to the next NUL byte in buf.
				 * If this is a NUL byte, there's nothting to
				 * skip.
				 */
				if (buf[i] == 0)
					pathoff = 0;
				else
					pathoff = -1;
			}
		}
	}
	/*
	 * Append the new path name to the file. Make sure that the previous
	 * pathname is NUL terminated.
	 */
	if (pathoff) {
		log_warnx("pe_playground_file_instantiate: File not NUL "
		    "termianted.");
		if (write(fd, "", 1) != 1) {
			log_warn("pe_playground_file_instantiate: Failed to "
			    "write path name");
			goto done;
		}
	}
	pathoff = strlen(path)+1;
	if (write(fd, path, pathoff) != pathoff) {
		log_warn("pe_playground_file_instantiate: Failed to "
		    "write path name");
		goto done;
	}
	ret = newfile;
done:
	if (close(fd) < 0) {
		log_warn("pe_playground_file_instantiate: Failed to "
		    "write path name");
		ret = -1;
	}
	/* XXX: unlink file again if it was new and there was an error */
	return ret;
}

/**
 * Search through one PG data file and look for files in a renamed directory.
 * If any are found, add the corresponding names with the new name of the
 * directory.
 *
 * @param pg The playground data structure
 * @param name The name of the PG data file ("device:inode")
 * @param new_path The new name of the dir (without trailing slash)
 * @param old_path The old name of the dir (with a trailing slash)
 */
static void
pe_playground_rename_dir_one_file(struct playground *pg, const char *name,
    const char *new_path, const char *old_path)
{
	int fd;
	char buf[PATH_MAX];
	char *bufend = buf + sizeof(buf);	/* end of buffer */
	char *endp = buf;			/* end of valid data */
	char *p = buf;				/* start of current name */
	int pathlen = strlen(old_path);
	char **files = NULL;
	int size = 0, alloc = 0;

	fd = atfd_openat(&pg->dirfd, name, O_RDONLY, 0);
	if (fd < 0) {
		log_warn("pe_playground_rename_dir_one_file: Cannot open %s "
		    "in playground %" PRIx64, name, pg->pgid);
		return;
	}
	while (1) {
		char *nameend;		/* pointer to \0 end of current name */
		int count = read(fd, endp, bufend - endp);

		if (count == 0)
			break;
		if (count < 0) {
			log_warn("pe_playground_rename_dir_one_file: Error "
			    "while reading file %s in playgournd %" PRIx64,
			    name, pg->pgid);
			goto done;
		}
		endp += count;

		while (endp > p) {
			nameend = memchr(p, '\0', endp - p);
			if (!nameend) {
				if (buf == p) {
					log_warn("pe_playground_rename_dir_one_file: "
					    "Name too long while reading file %s in "
					    "playgournd %" PRIx64, name, pg->pgid);
					return;
				}
				/* move data to start of buffer and read more */
				memmove(buf, p, endp - p);
				endp = buf + (endp - p);
				break;
			}
			if (nameend - p > pathlen &&
			    strncmp(old_path, p, pathlen) == 0) {
				/* match */
				if (size == alloc) {
					char **new;
					alloc += 5;
					new = realloc(files,
					    alloc * sizeof(char *));
					if (!new)
						goto done;
					files = new;
				}
				if (asprintf(&files[size], "%s/%s", new_path,
				    p + pathlen) < 0)
					goto done;
				size++;
			}
			p = nameend + 1;
		}
	}
done:
	if (close(fd) < 0) {
		log_warn("pe_playground_rename_dir_one_file: Failed to "
		    "read %s", name);
	}
	if (files) {
		int i;
		for (i = 0; i < size; i++) {
			pe_playground_file_add(pg, name, files[i]);
			free(files[i]);
		}
		free(files);
	}
	return;
}

/**
 * Search through one playground and add new names for files in a renamed
 * directory.
 *
 * @param pg The playground data structure
 * @param dir_dev The device node of the renamed directory
 * @param dir_ino The inode of the renamed directory
 * @param path The new name of the dir (without trailing slash)
 * @param old_path_no_slash The old name of the dir (without trailing slash)
 */
static void
pe_playground_dir_rename(struct playground *pg, uint64_t dir_dev,
    uint64_t dir_ino, const char *path, const char *old_path_no_slash)
{
	DIR		*dir;
	struct dirent	*dent;
	char *old_path;
	if (asprintf(&old_path, "%s/", old_path_no_slash) < 0)
		return;

	dir = atfd_fdopendir(&pg->dirfd);
	if (dir == NULL) {
		log_warn("pe_playground_read: Cannot read playground "
		    "directory for %" PRIx64, pg->pgid);
		free(old_path);
		return;
	}
	while((dent = readdir(dir)) != NULL) {
		uint64_t	dev, ino;
		char		ch;
		if (sscanf(dent->d_name, "%" PRIx64 ":%" PRIx64 "%c",
		    &dev, &ino, &ch) == 2) {
			if (dev == dir_dev && ino != dir_ino)
				pe_playground_rename_dir_one_file(pg,
				    dent->d_name, path, old_path);
		}
	}
	closedir(dir);
	free(old_path);
}

/**
 * Add the file name of a newly instantiated playground file to the
 * playground structure on disk. If old_path was given, triger a rename
 * of the PG-files in the dir.
 *
 * @param pgid The playground ID.
 * @param dev The device of the file system that the file lives on.
 * @param ino The inode of the file.
 * @param path The path name of the file relative to the root of the
 *     file system.
 * @param old_path If file is a dir that is renamed, the old path name of the
 *                 dir relative to the root of the file system. NULL otherwise.
 * @return None.
 */
void
pe_playground_file_instantiate(anoubis_cookie_t pgid, uint64_t dev,
    uint64_t ino, const char *path, const char *old_path)
{
	struct playground *pg;
	char buf[50];
	int newfile;

	if (!path || path[0] == 0)
		return;
	pg = pe_playground_find(pgid);
	if (!pg) {
		log_warnx("pe_playground_file_instantiate: File instantiated "
		    "in unknown playground");
		pg = pe_playground_create(pgid, 1);
		if (!pg)
			master_terminate();
	}
	pe_playground_devtoname(buf, sizeof(buf), dev, ino);
	newfile = pe_playground_file_add(pg, buf, path);
	if (newfile == 1)
		pg->nrfiles++;
	else if (old_path) {
		pe_playground_dir_rename(pg, dev, ino, path, old_path);
	}
}

/**
 * Remove a file from a playground after its playground label got removed.
 *
 * @param pgid The playground ID.
 * @param dev The device of the file.
 * @param ino The inode number of the file.
 * @return None.
 */
void
pe_playground_file_delete(anoubis_cookie_t pgid, uint64_t dev, uint64_t ino)
{
	struct playground *pg = pe_playground_find(pgid);
	char buf[50];

	if (!pg)
		return;
	pe_playground_devtoname(buf, sizeof(buf), dev, ino);
	if (atfd_unlinkat(&pg->dirfd, buf) == 0) {
		pg->nrfiles--;
		pe_playground_trykill(pg);
	}
}

/**
 * Check if the given user is allowed to request scanning of the
 * given file. Additionally, the file is instantiated and the device
 * and inode numbers are recorded in the playground. This is the only
 * file that the user can request for scanning.
 * Only the root user and the user that owns the playground are allowed
 * to request scanning of the file.
 *
 * @param pgid The playground ID of the file.
 * @param dev The device number of the file.
 * @param ino The inode number of the file.
 * @param path The path name of the file relative to root of the given
 *             device.
 * @param uid The user-ID of the user that requested the event.
 */
int
pe_playground_file_scanrequest(anoubis_cookie_t pgid, uint64_t dev,
    uint64_t ino, const char *path, uid_t uid)
{
	struct playground		*pg;

	if (path == NULL || path[0] == 0)
		return -EINVAL;
	pe_playground_file_instantiate(pgid, dev, ino, path, NULL);
	pg = pe_playground_find(pgid);
	if (pg == NULL)
		return -ESRCH;
	if (uid && (pg->uid != uid || pg->uid == (uid_t)-1))
		return -EPERM;
	pg->scandev = dev;
	pg->scanino = ino;
	return 0;
}

/**
 * Dump a list of all playgrounds to the current log file.
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
	DIR				*dir;
	struct dirent			*dent;

	if (pg) {
		log_warnx("pe_playground_read: Playground %" PRIx64 " exists",
		    pgid);
	} else {
		/*
		 * Do not try to create the management directory, it
		 * already exists.
		 */
		pg = pe_playground_create(pgid, 0);
		if (!pg)
			fatal("pe_playground_read: Out of memory");
		pg->did_exec = 1;
	}
	pg->uid = (uid_t)-1;
	if (pe_playground_readfile(pg, "UID", buf, sizeof(buf)) >= 0) {
		int	uid = -1;
		sscanf(buf, "%d", &uid);
		pg->uid = uid;
	}
	if (pe_playground_readfile(pg, "STARTTIME", buf, sizeof(buf)) >= 0) {
		if (sscanf(buf, "%" PRId64, &pg->starttime) != 1)
			pg->starttime = 0;
	}
	if (pg->cmd) {
		free(pg->cmd);
		pg->cmd = NULL;
	}
	if (pe_playground_readfile(pg, "CMD", buf, sizeof(buf)) >= 0)
		pg->cmd = strdup(buf);
	dir = atfd_fdopendir(&pg->dirfd);
	if (dir == NULL) {
		log_warn("pe_playground_read: Cannot read playground "
		    "directory for %" PRIx64, pgid);
		return;
	}
	while((dent = readdir(dir)) != NULL) {
		uint64_t	dev, ino;
		char		ch;
		if (sscanf(dent->d_name, "%" PRIx64 ":%" PRIx64 "%c",
		    &dev, &ino, &ch) == 2)
			pg->nrfiles++;
	}
	closedir(dir);
	pe_playground_trykill(pg);
}

/**
 * Initialize the playground subsystem in the policy engine.
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
		if (strcmp(dent->d_name, ANOUBISD_PG_LAST_PGID_FILENAME) == 0)
			continue;
		if (sscanf(dent->d_name, "%" PRIx64 "%c", &pgid, &dummy) != 1) {
			log_warnx("pe_playground_init: Skipping malformed "
			    "entry %s in " ANOUBISD_PG, dent->d_name);
			continue;
		}
		pe_playground_read(pgid);
	}
	closedir(pgdir);
}

/**
 * Free all memory related to playground during shutdown.
 */
void
pe_playground_shutdown(void)
{
	struct playground		*pg;

	while ((pg = LIST_FIRST(&playgrounds)) != NULL) {
		LIST_REMOVE(pg, next);
		if (pg->cmd)
			free(pg->cmd);
		abuf_free_type(pg, struct playground);
	}
}

/**
 * Parse the playground directory and extract the maximum playground ID.
 * This value is sent to the kernel via an appropriate ioctl. This function
 * is called by the master process during startup.
 *
 * @param anoubisfd The file descriptor for the anoubis device.
 * @param eventfd The event file descriptor. This is used for authentication
 *     with the kernel.
 * @return None. An error message is logged if the request fails.
 */
void
pe_playground_initpgid(int anoubisfd __used, int eventfd __used)
{
	DIR			*pgdir;
	struct dirent		*dent;
	anoubis_cookie_t	 maxpgid = 0;
	anoubis_cookie_t	 loaded_last_pgid = 0;

	pgdir = opendir(ANOUBISD_PG);
	if (pgdir == NULL) {
		log_warnx("No playground directory. Not setting initial "
		    "playground ID.");
		return;
	}
	while ((dent = readdir(pgdir)) != NULL) {
		char			dummy;
		anoubis_cookie_t	pgid;

		if (dent->d_name[0] == '.')
			continue;
		if (strcmp(dent->d_name, ANOUBISD_PG_LAST_PGID_FILENAME) == 0)
			continue;
		if (sscanf(dent->d_name, "%" PRIx64 "%c", &pgid, &dummy) == 1) {
			if (pgid > maxpgid)
				maxpgid = pgid;
		}
	}
	closedir(pgdir);

	/* restore saved pgid from disk if it's > maxpgid */
	loaded_last_pgid = pe_playground_load_last_pgid();
	if (loaded_last_pgid > maxpgid)
		maxpgid = loaded_last_pgid;

	if  (maxpgid == 0)
		return;
#if ANOUBISCORE_VERSION >= ANOUBISCORE_PG_VERSION
	{
		struct anoubis_ioctl_lastpgid		lastpgid;

		lastpgid.lastpgid = maxpgid;
		lastpgid.fd = eventfd;
		if (ioctl(anoubisfd, ANOUBIS_SET_LASTPGID, &lastpgid) < 0) {
			if (errno != ERANGE)
				log_warn("Cannot set initial playground ID");
		}
	}
#endif
}

/**
 * Send information about a single or all known playgrounds to the queue
 * given as parameter. This function iterates over all playgrounds and
 * sends one or more messages containing ANOUBIS_REC_PGLIST records to
 * the given queue. Each record represents one playground. If the playground
 * ID is zero information all playgrounds are sent.
 *
 * @param token The token for the playground reply message.
 * @param pgid Send only information about the playground with this ID.
 *     A value of zero implies that information about all playgrounds is
 *     sent.
 * @param q Messages are added to this queue.
 */
int
pe_playground_send_pglist(uint64_t token, anoubis_cookie_t pgid,
    Queue *q)
{
	struct playground		*pg;
	struct amsg_list_context	 ctx;
	int				 error;

	error = amsg_list_init(&ctx, token, ANOUBIS_REC_PGLIST);
	if (error < 0)
		return error;
	LIST_FOREACH(pg, &playgrounds, next) {
		unsigned int			 size;
		Anoubis_PgInfoRecord		*rec;

		if (pgid && pg->pgid != pgid)
			continue;
		size = sizeof(Anoubis_PgInfoRecord) + 1;
		if (pg->cmd)
			size += strlen(pg->cmd);
		size = (size+7UL) & ~7UL;
		if (size > abuf_length(ctx.buf)) {
			amsg_list_send(&ctx, q);
			error = amsg_list_init(&ctx, token,
			    ANOUBIS_REC_PGLIST);
			if (error < 0)
				return error;
			ctx.flags = 0;
		}
		/* Record permanently too long */
		if (size > abuf_length(ctx.buf)) {
			if (ctx.msg)
				free(ctx.msg);
			return -EFAULT;
		}
		rec = abuf_cast(ctx.buf, Anoubis_PgInfoRecord);
		set_value(rec->reclen, size);
		set_value(rec->uid, pg->uid);
		set_value(rec->pgid, pg->pgid);
		set_value(rec->starttime, pg->starttime);
		set_value(rec->nrprocs, pg->nrprocs);
		set_value(rec->nrfiles, pg->nrfiles);
		if (pg->cmd) {
			strcpy(rec->path, pg->cmd);
		} else {
			rec->path[0] = 0;
		}
		amsg_list_addrecord(&ctx, size);
	}
	ctx.flags |= POLICY_FLAG_END;
	amsg_list_send(&ctx, q);
	return 0;
}

/**
 * Append records for all path names that are known for a single playground
 * file to the message in the reply context. If the message becomes full
 * it is sent to the queue and a new message is created.
 *
 * @param ctx The message context containing the current message. This
 *     message may or may not contain records.
 * @param pgid The playground ID of the current file. This value is stored in
 *     each ANOUBIS_REC_PGFILELIST record that is added to the message.
 * @param dev The device ID of the current file. This value is stored in
 *     each ANOUBIS_REC_PGFILELIST record that is added to the message.
 * @param ino The inode number of the  current file. This value is stored in
 *     each ANOUBIS_REC_PGFILELIST record that is added to the message.
 * @param fd The file descriptor of the file that contains the playground
 *     path names for the file. Each pathname results in one record.
 * @param q Completed messages are added to this queue.
 * @return Zero in case of success, a negative error code if an error occured.
 */
static int
pe_playground_send_onefile(struct amsg_list_context *ctx, uint64_t pgid,
    uint64_t dev, uint64_t ino, int fd, Queue *q)
{
	char	buf[512];	/* The read buffer. */
	int	i=0;		/* Current offset in read buffer */
	int	len=0;		/* Valid bytes in in read buffer */
	int	recoff = 0;	/* bytes already copied to current record */
	int	err;

	while (1) {
		int			 slen, headlen, complete;
		struct abuf_buffer	 tmp;
		Anoubis_PgFileRecord	*rec;
		/*
		 * If the read buffer is empty (completly processed) try
		 * to read more data.
		 */
		if (i >= len) {
			len = read(fd, buf, sizeof(buf));
			if (len < 0)
				return -errno;
			if (len == 0)
				break;
			i = 0;
		}
		/*
		 * Search for the NUL byte or the end of the buffer and
		 * copy this data over to the target message.
		 */
		complete = 0;
		for (slen = 0; i+slen < len; slen++) {
			if (buf[i+slen] == 0) {
				slen++;
				complete = 1;
				break;
			}
		}
		/*
		 * If this is the start of a new record, we must copy
		 * the header, otherwise the header is already included.
		 */
		if (recoff)
			headlen = 0;
		else
			headlen = sizeof(Anoubis_PgFileRecord);
		/*
		 * If the current record (or the known part thereof)
		 * does not fit into the message, send the message and
		 * allocate a new one. Partial record data that has
		 * been copied into the message is first copied over to the
		 * new message.
		 */
		if (recoff + headlen + slen > (int)abuf_length(ctx->buf)) {
			struct amsg_list_context	nctx;

			err = amsg_list_init(&nctx, ctx->dmsg->token,
			    ANOUBIS_REC_PGFILELIST);
			if (err < 0)
				return err;
			abuf_copy_part(nctx.buf, 0, ctx->buf, 0, recoff);
			amsg_list_send(ctx, q);
			nctx.flags = ctx->flags;
			(*ctx) = nctx;
			continue;
		}
		/* No header yet, add one. */
		if (recoff == 0) {
			rec = abuf_cast(ctx->buf, Anoubis_PgFileRecord);
			if (!rec)
				return -ENOMEM;
			set_value(rec->reclen, 0);
			set_value(rec->_pad, 0);
			set_value(rec->pgid, pgid);
			set_value(rec->dev, dev);
			set_value(rec->ino, ino);
			recoff = offsetof(Anoubis_PgFileRecord, path);
		}
		/* Append the path data to the current record. */
		tmp = abuf_open(ctx->buf, recoff);
		abuf_copy_tobuf(tmp, buf+i, slen);
		/* Adjust record offset and position in the read buffer. */
		recoff += slen;
		i += slen;
		/*
		 * If the string is complete (we found a NUL byte) we must
		 * account for the record in the context.
		 */
		if (complete) {
			int	nrecoff = (recoff + 7UL) & ~7UL;

			if (nrecoff <= (int)abuf_length(ctx->buf))
				recoff = nrecoff;
			rec = abuf_cast(ctx->buf, Anoubis_PgFileRecord);
			if (!rec)
				return -ENOMEM;
			set_value(rec->reclen, recoff);
			amsg_list_addrecord(ctx, recoff);
			recoff = 0;
		}
	}
	return 0;
}

/**
 * Send a list of all files in a particular playground to the user.
 * Only root is allowed to list files in playgrounds created by the other
 * users.
 *
 * @param token The token that should be used for replies.
 * @param pgid All files in the playground with the ID should be listed.
 * @param auth_uid The authorized user-ID of the requesting user.
 * @param q Messages are appended to this queue.
 * @return Zero in case of success, a negative error code in case of errors.
 */
int
pe_playground_send_filelist(uint64_t token, uint64_t pgid, uint32_t auth_uid,
    Queue *q)
{
	struct playground		*pg;
	int				 error = 0;
	DIR				*dir;
	struct dirent			*dent;
	struct amsg_list_context	 ctx;

	ctx.msg = NULL;
	pg = pe_playground_find(pgid);
	if (!pg)
		return -ESRCH;
	if (pg->uid != auth_uid && auth_uid != 0)
		return -EPERM;
	dir = atfd_fdopendir(&pg->dirfd);
	if (!dir)
		return -errno;
	error = amsg_list_init(&ctx, token, ANOUBIS_REC_PGFILELIST);
	if (error < 0)
		goto err;

	while((dent = readdir(dir)) != NULL) {
		uint64_t		dev, ino;
		char			ch;
		int			fd;

		/* Skip entries that do not represent a file. */
		if (sscanf(dent->d_name, "%" PRIx64 ":%" PRIx64 "%c",
		    &dev, &ino, &ch) != 2)
			continue;
		fd = atfd_openat(&pg->dirfd, dent->d_name, O_RDONLY, 0);
		if (fd < 0) {
			error = -errno;
			goto err;
		}

		error = pe_playground_send_onefile(&ctx, pgid, dev, ino, fd, q);
		close(fd);
		if (error < 0)
			goto err;
	}
	closedir(dir);
	ctx.flags |= POLICY_FLAG_END;
	amsg_list_send(&ctx, q);
	return 0;
err:
	closedir(dir);
	if (ctx.msg)
		free(ctx.msg);
	return error;
}

/**
 * Send an alert message to the session engine when a process is forced
 * into a playground due to APN rules. The caller must provide the eventdev
 * header of the exec event. This is turned into a fake event with source
 * ANOUBIS_SOURCE_PLAYGROUNDPROC. The playload of such an event is a
 * pg_proc_message structure. The event is logged to syslog, too.
 *
 * @param ident The process identifier of the process after a successful
 *     exec. This identifier is added to the log message for both the process
 *     and the context information.
 * @param orighdr The eventdev header of the exec event. Event data is taken
 *     from this event where appropriate.
 * @param ruleid The rule ID of the block that forced this process into
 *     the playground.
 * @param prio The priority of the rule that forced this process into
 *     the playground.
 */
void
pe_playground_notify_forced(struct pe_proc_ident *ident,
    struct eventdev_hdr *orighdr, uint32_t ruleid, uint32_t prio)
{
	struct eventdev_hdr		*hdr;
	struct pg_proc_message		*pg, *origpg;
	int				 nsize;

	nsize = sizeof(struct eventdev_hdr) + sizeof(struct pg_proc_message);
	if (nsize > orighdr->msg_size) {
		log_warnx("pe_playground_notify_force: Short eventdev hdr");
		return;
	}
	hdr = malloc(nsize);
	if (!hdr) {
		log_warn("pe_playground_notify_force");
		return;
	}
	origpg = (struct pg_proc_message *)(orighdr+1);
	pg = (struct pg_proc_message *)(hdr+1);
	hdr->msg_size = nsize;
	hdr->msg_source = ANOUBIS_SOURCE_PLAYGROUNDPROC;
	hdr->msg_flags = 0;
	hdr->msg_token = orighdr->msg_token;
	hdr->msg_pid = orighdr->msg_pid;
	hdr->msg_uid = orighdr->msg_uid;
	pg->common = origpg->common;
	log_warnx("%s (pid=%d, uid=%d) is forced into a playground ruleid "
	    "%d prio %d", ident ? ident->pathhint : NULL, hdr->msg_pid,
	    hdr->msg_uid, ruleid, prio);
	__send_lognotify(ident, ident, hdr, 0 /* error */, APN_LOG_ALERT,
	    ruleid, prio, 0 /* sfsmatch */);
	free(hdr);
}

/**
 * Handle a commit request received from the session process. The
 * user must be root or own the playground. Additionally, a file to
 * scan must have been recorded previously. The device and inode of the
 * file is filled in and the message is sent to the master process.
 *
 * This function frees (or reuses) the incoming message. It is no
 * longer available to the caller after this function returns.
 *
 * @param msg The playground message received from the session engine.
 *     This message is either freed or reused for communication with
 *     the master.
 * @param session Messages that should go back to the session engine are
 *     sent to this queue. This function only sends error messages to
 *     session.
 * @param master Messages that should go to the master process are sent to
 *     this queue. If everything is ok, the incoming message is forwarded
 *     to this queue.
 */
void
pe_playground_dispatch_commit(struct anoubisd_msg *msg, Queue *session,
    Queue *master)
{
	struct anoubisd_msg			*repmsg;
	struct anoubisd_msg_pgcommit		*pgmsg;
	struct anoubisd_msg_pgcommit_reply	*pgrep;
	struct playground			*pg, *pg2;
	int					 err;
	int					 i, match;
	static const char			 badcomp[] = "/..";

	DEBUG(DBG_TRACE, ">pe_playground_dispatch_commit");
	pgmsg = (struct anoubisd_msg_pgcommit *)msg->msg;
	DEBUG(DBG_PG, " pe_playground_dispatch_commit: pgid=%" PRIx64
	    " token=%" PRId64, pgmsg->pgid, pgmsg->token);
	err = ESRCH;
	pg = pe_playground_find(pgmsg->pgid);
	if (pg == NULL)
		goto error;
	err = EPERM;
	if (pgmsg->auth_uid && pg->uid != pgmsg->auth_uid)
		goto error;
	err = EINVAL;
	if (pgmsg->dev || pgmsg->ino || pg->scandev == 0 || pg->scanino == 0)
		goto error;
	err = EBUSY;
	/* Only one scan per user. */
	LIST_FOREACH(pg2, &playgrounds, next) {
		if (pg2->uid == pg->uid && pg2->scantok)
			goto error;
	}
	/* Make sure that the path is absolute and has no hidden components. */
	if (pgmsg->path[0] != '/')
		goto error;
	match = 0;
	for (i=0; pgmsg->path[i]; ++i) {
		if (pgmsg->path[i] == '/' && match)
			goto error;
		if (pgmsg->path[i] == badcomp[match])
			match++;
		else
			match = 0;
	}
	if (match)
		goto error;
	pgmsg->dev = pg->scandev;
	pgmsg->ino = pg->scanino;
	pg->scantok = pgmsg->token;
	enqueue(master, msg);
	DEBUG(DBG_TRACE, "<pe_playground_dispatch_commit (success)");
	return;
error:
	repmsg = msg_factory(ANOUBISD_MSG_PGCOMMIT_REPLY,
	    sizeof(struct anoubisd_msg_pgcommit_reply));
	pgrep = (struct anoubisd_msg_pgcommit_reply *)repmsg->msg;
	pgrep->error = err;
	pgrep->token = pgmsg->token;
	pgrep->len = 0;
	enqueue(session, repmsg);
	free(msg);
	DEBUG(DBG_TRACE, "<pe_playground_dispatch_commit "
	    "(error=%d token=%" PRId64 ")", pgrep->error, pgrep->token);
}

/**
 * Clear scanning status in the playground once a reply is received.
 * After this function is called, it will be possible to start a new commit
 * request.
 *
 * @param msg The commit reply message.
 * @return None.
 */
void
pe_playground_dispatch_commitreply(struct anoubisd_msg *msg)
{
	struct anoubisd_msg_pgcommit_reply	*pgmsg;
	struct playground			*pg;

	pgmsg = (struct anoubisd_msg_pgcommit_reply *)msg->msg;
	LIST_FOREACH(pg, &playgrounds, next) {
		if (pgmsg->token == pg->scantok)
			break;
	}
	if (pg == NULL) {
		log_warnx("Playground for reply token not found");
		return;
	}
	pg->scandev = 0;
	pg->scanino = 0;
	pg->scantok = 0;
}

void
pe_playground_dispatch_unlink(struct anoubisd_msg *msg, Queue *session)
{
	struct anoubisd_msg			*repmsg;
	struct anoubisd_msg_pgunlink		*pgmsg;
	struct anoubisd_msg_pgunlink_reply	*pgrep;
	struct playground			*pg;
	int					 err = 0;

	pgmsg = (struct anoubisd_msg_pgunlink *)msg->msg;

	pg = pe_playground_find(pgmsg->pgid);
	if (pg == NULL) {
		err = ESRCH;
		goto error;
	}

	if (pgmsg->auth_uid && pg->uid != pgmsg->auth_uid) {
		err = EPERM;
		goto error;
	}

	pe_playground_file_delete(pgmsg->pgid, pgmsg->dev, pgmsg->ino);

error:
	repmsg = msg_factory(ANOUBISD_MSG_PGUNLINK_REPLY,
	    sizeof(struct anoubisd_msg_pgunlink_reply));
	pgrep = (struct anoubisd_msg_pgunlink_reply *)repmsg->msg;

	pgrep->error = err;
	pgrep->token = pgmsg->token;
	enqueue(session, repmsg);
}

/**
 * Store Playground ID of newest playground for later retrieval.
 * The ID has to be saved to disk to be avoid collisions of playground IDs after
 * system a reboot in case the cleanup of dead playgrounds fails and files with
 * the same ID still exist in the filesystem.
 *
 * @param pgid The ID of the last created playground.
 * @return none.
 */
void
pe_playground_save_last_pgid(uint64_t pgid)
{
	FILE	   *fp;
	const char *errmsg_str	= "while attempting to save last playground id";

	fp = fopen(ANOUBISD_PG_LAST_PGID_FILE_CHROOT, "w");
	if (!fp) {
		log_warn("ERROR: failed to open file '%s' %s",
		    ANOUBISD_PG_LAST_PGID_FILE_CHROOT, errmsg_str);
	} else {
		if (fprintf(fp, "%" PRIx64 "\n", pgid) < 0)
			log_warn("ERROR %s", errmsg_str);
		fclose(fp);
	}
	DEBUG(DBG_TRACE, "<pe_playground_save_last_pgid (errno = %d, pgid="
	    "%" PRIx64 ")", errno, pgid);
}

/**
 * Retrieve and return the playground ID of the latest playground which was
 * stored using pe_playground_save_last_pgid().
 *
 * @param none.
 * @return latest playground id, 0 if none exists or on failure to load.
 */
uint64_t
pe_playground_load_last_pgid()
{
	FILE	    *fp		= NULL;
	uint64_t     pgid	= 0;
	int	     n		= 0;
	const char  *errmsg_str	= "while attempting to load last playground id";

	fp = fopen(ANOUBISD_PG_LAST_PGID_FILE, "r");
	if (!fp) {
		if (errno != ENOENT)
			log_warn("ERROR %s", errmsg_str);
	} else {
		char dummy;
		n = fscanf(fp, "%" PRIx64 "%c", &pgid, &dummy);
		if (n != 2 || dummy != '\n') {
			log_warnx("ERROR %s: '%s' does not contain a valid"
			    " playground id!", errmsg_str,
			    ANOUBISD_PG_LAST_PGID_FILE);
			pgid = 0;
		}
		fclose(fp);
	}
	DEBUG(DBG_TRACE, "<pe_playground_load_last_pgid (errno = %d, pgid="
	    "%" PRIx64 ")", errno, pgid);
	return pgid;
}
