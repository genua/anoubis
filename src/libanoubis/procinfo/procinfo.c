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

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/param.h>
#include <string.h>
#include <ctype.h>

#include <anoubis_procinfo.h>

#ifdef OPENBSD

#include <sys/sysctl.h>
#include <kvm.h>
#include <sys/proc.h>

/**
 * Openbsd implementation of an anoubis_proc_handle. This is simply
 * a warpper around a kvm_t handle.
 * Fields:
 * kd: The kvm_t handle used to access kernel memory.
 * errbuf: Error messages from kvm operations.
 */
struct anoubis_proc_handle {
	kvm_t		*kd;
	char		 errbuf[_POSIX2_LINE_MAX];
};

struct anoubis_proc_handle *
anoubis_proc_open(void)
{
	struct anoubis_proc_handle	*handle;

	handle = malloc(sizeof(struct anoubis_proc_handle));
	if (handle == NULL)
		return NULL;
	handle->kd = kvm_openfiles(NULL, NULL, NULL, KVM_NO_FILES,
	    handle->errbuf);
	if (handle->kd == NULL) {
		free(handle);
		return NULL;
	}
	return handle;
}

void
anoubis_proc_close(struct anoubis_proc_handle *handle)
{
	if (handle == NULL)
		return;
	if (handle->kd) {
		kvm_close(handle->kd);
		handle->kd = NULL;
	}
	free(handle);
}

struct anoubis_proc *
anoubis_proc_get(struct anoubis_proc_handle *handle, pid_t pid)
{
	struct anoubis_proc		*p;
	struct kinfo_proc2		*kp;
	int				 cnt = 0;
	char				**argv;

	if (!handle || handle->kd == NULL)
		return NULL;
	kp = kvm_getproc2(handle->kd, KERN_PROC_PID, pid,
	    sizeof(struct kinfo_proc2), &cnt);
	if (kp == NULL || cnt < 1)
		return NULL;
	p = malloc(sizeof(struct anoubis_proc));
	if (p == NULL)
		return NULL;
	p->groups = NULL;
	p->comm = NULL;
	p->command = NULL;
	p->pid = kp->p_pid;
	p->ppid = kp->p_ppid;
	p->ruid = kp->p_ruid;
	p->euid = kp->p_uid;
	p->suid = kp->p_svuid;
	p->rgid = kp->p_rgid;
	p->egid = kp->p_gid;
	p->sgid = kp->p_svgid;
	p->ngroups = kp->p_ngroups;
	if (p->ngroups) {
		unsigned int		i;

		p->groups = malloc(p->ngroups * sizeof(*p->groups));
		if (p->groups == NULL)
			goto nomem;
		for (i=0; i<p->ngroups; ++i)
			p->groups[i] = kp->p_groups[i];
	} else {
		p->ngroups = NULL;
	}
	p->comm = strdup(kp->p_comm);
	if (p->comm == NULL)
		goto nomem;
	argv = kvm_getargv2(handle->kd, kp, 0);
	p->command = strdup(argv[0]);
	return p;
nomem:
	anoubis_proc_destroy(p);
	return NULL;
}

#endif

#ifdef LINUX

/**
 * Linux Implementation of anoubis_proc_handle. This does not require
 * any data at all. We add the error message field for compatibility
 * but currently it is used neither on OpenBSD nor on Linux.
 * Fields:
 * errbuf: Future storage for errors.
 */
struct anoubis_proc_handle {
	char		 errbuf[_POSIX2_LINE_MAX];
};

struct anoubis_proc_handle *
anoubis_proc_open(void)
{
	struct anoubis_proc_handle	*handle;

	handle = malloc(sizeof(struct anoubis_proc_handle));
	return handle;
}

void
anoubis_proc_close(struct anoubis_proc_handle *handle)
{
	free(handle);
}

/*
 * This is the linux implementation of anoubis_proc_get simply parses
 * the file /proc/<pid>/status.
 */
struct anoubis_proc *
anoubis_proc_get(struct anoubis_proc_handle *handle __attribute__((unused)),
    pid_t searchpid)
{
	char			 buf[_POSIX2_LINE_MAX];
	FILE			*fp;
	struct anoubis_proc	*p;
	char			 pid = 0, ppid = 0, uid = 0, gid = 0;
	char			 groups = 0, name = 0;
	char			*ret;

	snprintf(buf, 1024, "/proc/%d/status", searchpid);
	fp = fopen(buf, "r");
	if (fp == NULL)
		return NULL;
	p = malloc(sizeof(struct anoubis_proc));
	if (p == NULL)
		return NULL;
	p->groups = NULL;
	p->comm = NULL;
	p->command = NULL;
	while (1) {
		char		*arg;

		if (fgets(buf, sizeof(buf), fp) == NULL)
			break;
		if (feof(fp))
			break;
		/* Remove trailing newline. */
		arg = strchr(buf, '\n');
		if (arg)
			*arg = 0;
		arg = strchr(buf, ':');
		if (arg == NULL)
			continue;
		*arg = 0;
		arg++;
		if (strcasecmp(buf, "pid") == 0) {
			if (sscanf(arg, "%d", &p->pid) == 1)
				pid = 1;
		} else if (strcasecmp(buf, "ppid") == 0) {
			if (sscanf(arg, "%d", &p->ppid) == 1)
				ppid = 1;
		} else if (strcasecmp(buf, "uid") == 0) {
			if (sscanf(arg, "%d %d %d",
			    &p->ruid, &p->euid, &p->suid) == 3)
				uid = 1;
		} else if (strcasecmp(buf, "gid") == 0) {
			if (sscanf(arg, "%d %d %d",
			    &p->rgid, &p->egid, &p->sgid) == 3)
				gid = 1;
		} else if (strcasecmp(buf, "groups") == 0) {
			char		*gp = arg;
			int		 ngrp = 0, len;
			unsigned int	 i, g;

			/* First count the number of entries. */
			while (1) {
				while (isspace(*gp))
					gp++;
				if (*gp == 0)
					break;
				if (sscanf(gp, "%d%n", &g, &len) < 1)
					break;
				ngrp++;
				gp += len;
			}
			if (*gp)
				continue;
			p->ngroups = ngrp;
			if (p->ngroups == 0) {
				groups = 1;
				continue;
			}
			/* Re-scan the line and store group IDs */
			p->groups = malloc(p->ngroups * sizeof(*p->groups));
			gp = arg;
			for (i=0; i<p->ngroups; ++i) {
				if (sscanf(gp, "%d%n", &g, &len) < 1)
					break;
				p->groups[i] = g;
			}
			if (i == p->ngroups)
				groups = 1;
		} else if (strcasecmp(buf, "name") == 0) {
			while(isspace(*arg))
				arg++;
			if (*arg && (p->comm = strdup(arg)))
				name = 1;
		}
	}
	fclose(fp);
	if (!pid || !ppid || !uid || !gid || !groups || !name) {
		anoubis_proc_destroy(p);
		return NULL;
	}
	/* Return available info even if we fail to parse /proc/.../cmdline */
	snprintf(buf, 1024, "/proc/%d/cmdline", searchpid);
	fp = fopen(buf, "r");
	if (fp == NULL)
		return p;
	ret = fgets(buf, sizeof(buf), fp);
	fclose(fp);
	if (ret && strlen(buf)) {
		p->command = strdup(buf);
	} else {
		/* Fall back to short command name, e.g. for kernel threads */
		p->command = strdup(p->comm);
	}
	return p;
}

#endif
