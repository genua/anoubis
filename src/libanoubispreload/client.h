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

#ifndef _CLIENT_H_
#define _CLIENT_H_

/**
 * A handle to the preload-client.
 */
struct _preloadclient;
typedef struct _preloadclient* preloadclient_t;

/**
 * Results from client-operations.
 */
enum preloadclient_rc
{
	PRELOADCLIENT_OK = 0,	/* Success */
	PRELOADCLIENT_WARN,	/* An error occured but you can still
				   continue */
	PRELOADCLIENT_FATAL	/* An fatal error occured and you need to abort
				   operations. */
};
typedef enum preloadclient_rc preloadclient_rc;

/**
 * Initializes the client.
 * You need to call this method only once.
 *
 * On success a valid handle is returned. If an error occured, NULL is
 * returned.
 */
preloadclient_t preloadclient_initialize(void);

/**
 * Destroys the given handle again.
 */
void preloadclient_destroy(preloadclient_t);

/**
 * The given file was modified.
 *
 * The function calculates the new checksum and sends it to anoubisd.
 * The send-operation is performed for each user, which has a registered
 * checksum for the given file.
 *
 * The first parameter is a handle created by preloadclient_initialize(). The
 * second parameter is the path of the file, which was modified.
 */
preloadclient_rc preloadclient_modified(preloadclient_t, const char *);

/**
 * The given file was deleted.
 *
 * The function asks anoubisd to remove the checksum for each user, which has
 * still a registered checksum for the given file.
 *
 * The first parameter is a handle created by preloadclient_initialize(). The
 * second paramater is the path of the file, which was removed.
 */
preloadclient_rc preloadclient_removed(preloadclient_t, const char *);

#endif	/* _CLIENT_H_ */
