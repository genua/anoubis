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

#ifndef _ANOUBIS_PROCINFO_H_
#define _ANOUBIS_PROCINFO_H_

#include <config.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <inttypes.h>
#include <stdint.h>

/**
 * Abstract data type that keeps the state of the process reading.
 * Must be created with anoubis_open_proc and destroyed with
 * anoubis_close_proc.
 */
struct		anoubis_proc_handle;

/**
 * Architecture independent information about a process.
 * The structure itself is usually malloced and all pointer fields
 * point to malloced data, too. Use anoubis_proc_destroy to free
 * memory associated with this data structure.
 * Fields:
 *
 * pid: The process ID
 * ppid: The parent process ID
 * ruid: The real user ID
 * euid: The effective user ID
 * suid: The saved user ID
 * gid: The real group ID
 * egid: The effective group ID
 * sgid: The saved group ID
 * ngroups: The number of entries in the groups array
 * groups: The list of supplementary group IDs
 * comm: The short form of the command.
 * command: The full version of the command (including command line arguments)
 */
struct anoubis_proc {
	uint32_t		 pid;
	uint32_t		 ppid;
	uint32_t		 ruid;
	uint32_t		 euid;
	uint32_t		 suid;
	uint32_t		 rgid;
	uint32_t		 egid;
	uint32_t		 sgid;
	uint32_t		 ngroups;
	int			*groups;
	char			*comm;
	char			*command;
};

__BEGIN_DECLS

/**
 * Release all memory associated with the given anoubis process
 * data structure.
 *
 * @param p The anoubis process data structure.
 * @return None.
 */
static inline void
anoubis_proc_destroy(struct anoubis_proc *p)
{
	if (p->groups)
		free(p->groups);
	if (p->comm)
		free(p->comm);
	if (p->command)
		free(p->command);
	free(p);
}

/**
 * Set up a handle that can later be used to retrieve information about
 * individual processes.
 *
 * @return The handle.
 */
extern struct anoubis_proc_handle *
anoubis_proc_open(void);

/**
 * Release all data associated with a process reading handle.
 *
 * @param handle The handle.
 * @return None.
 */
extern void
anoubis_proc_close(struct anoubis_proc_handle *handle);

/**
 * Return information about a particular process in the system.
 *
 * @param handle The process reading handle returned from
 *     anoubis_proc_open.
 * @param pid The process ID of the process to look for.
 * @return A pointer to a malloced anoubis_proc record that describes
 *     the process or NULL if the process was not found.
 */
extern struct anoubis_proc *
anoubis_proc_get(struct anoubis_proc_handle *handle, pid_t pid);

__END_DECLS

#endif	/* _ANOUBIS_PROCINFO_H_ */
