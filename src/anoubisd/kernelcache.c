/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include "anoubisd.h"
#include "amsg.h"
#include "kernelcache.h"

static struct anoubis_kernel_policy_header*
    kernelcache_expire(struct anoubis_kernel_policy_header* kcache);

/*
 * Add a new policy to the kernel cache-structure in the process struct
 * of anoubisd. This does not upload the new cache to the kernel, which
 * is done by the master process calling kernelcache_upload()
 */
struct anoubis_kernel_policy_header*
kernelcache_add(struct anoubis_kernel_policy_header *kcache,
    struct anoubis_kernel_policy *new_policy)
{
	struct anoubis_kernel_policy *ppolicy;
	int len, newlen;

	if (kcache != NULL)
		kcache = kernelcache_expire(kcache);

	len = sizeof(struct anoubis_kernel_policy) + new_policy->rule_len;

	if (kcache == NULL) {
		newlen = sizeof(struct anoubis_kernel_policy_header) + len;

		kcache = malloc(newlen);

		if (kcache == NULL)
			return NULL;

		kcache->size = len;
	} else {
		struct anoubis_kernel_policy_header *newcache;

		newlen = sizeof(struct anoubis_kernel_policy_header) +
		    kcache->size + len;

		newcache = realloc(kcache, newlen);

		if (newcache == NULL) {
			free(kcache);
			return NULL;
		}

		kcache = newcache;
		kcache->size = kcache->size + len;
	}

	ppolicy = (struct anoubis_kernel_policy*)
	    (((unsigned char*)kcache) + newlen - len);

	memcpy(ppolicy, new_policy, len);

	return kcache;
}

/*
 * Clear the kernel cache-structure in the process struct of anoubisd.
 * This does not clear the cache in the kernel, which is done by
 * the master process calling kernelcache_upload() afterwards
 */
struct anoubis_kernel_policy_header*
kernelcache_clear(struct anoubis_kernel_policy_header *kcache)
{
	if (kcache != NULL)
		free(kcache);

	return NULL;
}

/*
 * Send a message containing the updated cache for the process to master. This
 * calls the function policy_msg2master, which enqueues the message.
 */
int
kernelcache_send2master(struct anoubis_kernel_policy_header *kcache, pid_t pid)
{
	struct anoubis_kernel_policy_header empty_policy;
	anoubisd_msg_t *msg;

	if (pid < 1)
		return 0;

	if (kcache == NULL) {
		empty_policy.size = 0;
		kcache = &empty_policy;
	}

	if ((sizeof(struct anoubis_kernel_policy_header) + kcache->size) >
	    MSG_BUF_SIZE)
		return 0;

	kcache->pid = pid;

	msg = msg_factory(ANOUBISD_MSG_KCACHE,
	    sizeof(struct anoubis_kernel_policy_header) + kcache->size);

	if (msg != NULL) {
		memcpy(msg->msg, kcache,
		    sizeof(struct anoubis_kernel_policy_header) + kcache->size);

		policy_msg2master(msg);
	} else {
		return 0;
	}

	return 1;
}

/*
 * Uploads the content of the provided process dependent kernelcache into the
 * running kernel.
 */
int
kernelcache_upload(int fd, anoubisd_msg_t *msg)
{
	struct anoubis_kernel_policy_header *kcache;

	if (msg->mtype != ANOUBISD_MSG_KCACHE) {
		return 0;
	}

	kcache = (struct anoubis_kernel_policy_header*)(msg->msg);

	if (ioctl(fd, ANOUBIS_REPLACE_POLICY, kcache) < 0)
		return 0;

	return 1;
}

/*
 * Expire rules from our copy of the kernel cache
 * This is only called from within kernelcache.c
 */
static struct anoubis_kernel_policy_header*
kernelcache_expire(struct anoubis_kernel_policy_header* kcache)
{
	struct anoubis_kernel_policy_header* newcache;
	struct anoubis_kernel_policy *ppolicy;
	unsigned char *pend;
	time_t now = time(NULL);

	ppolicy = (struct anoubis_kernel_policy*)
	    (((unsigned char*)kcache) +
	    sizeof(struct anoubis_kernel_policy_header));

	pend = ((unsigned char*)kcache) +
	    sizeof(struct anoubis_kernel_policy_header) +
	    kcache->size;

	while((unsigned char*)ppolicy < pend) {
		struct anoubis_kernel_policy *next;

		next = (struct anoubis_kernel_policy*)
			(((unsigned char*)ppolicy) +
			 sizeof(struct anoubis_kernel_policy) +
			 ppolicy->rule_len);

		if ((ppolicy->expire != 0) && (ppolicy->expire < now)) {
			int remaining;

			remaining = pend - ((unsigned char*)next);

			pend -= sizeof(struct anoubis_kernel_policy) +
			    ppolicy->rule_len;

			memmove(ppolicy, next, remaining);
		} else {
			ppolicy = next;
		}
	}

	newcache = realloc(kcache, pend - ((unsigned char*)kcache));

	if (newcache == NULL) {
		free(kcache);
	}

	return newcache;
}
