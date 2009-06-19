/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#include <stdlib.h>

#include "client.h"

struct _preloadclient
{
	/* TODO Fill with all information you need! */
};

preloadclient_t
preloadclient_initialize(void)
{
	preloadclient_t handle;

	handle = (preloadclient_t)malloc(sizeof(struct _preloadclient));
	if (handle == NULL)
		return (NULL);

	return (handle);
}

void
preloadclient_destroy(preloadclient_t handle)
{
	free(handle);
}

preloadclient_rc
preloadclient_modified(preloadclient_t handle, const char *path)
{
	if ((handle == NULL) || (path == NULL))
		return (PRELOADCLIENT_FATAL);

	return (PRELOADCLIENT_OK);
}

preloadclient_rc
preloadclient_removed(preloadclient_t handle, const char *path)
{
	if ((handle == NULL) || (path == NULL))
		return (PRELOADCLIENT_FATAL);

	return (PRELOADCLIENT_OK);
}
