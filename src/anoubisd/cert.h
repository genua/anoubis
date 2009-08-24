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

#ifndef _CERT_H_
#define _CERT_H_

#include <sys/types.h>
#include <anoubisd.h>

#ifdef LINUX
#include <queue.h>
#include <bsdcompat.h>
#else
#include <sys/queue.h>
#endif

#include <openssl/evp.h>

struct cert {
	TAILQ_ENTRY(cert)	 entry;
	uid_t			 uid;
	EVP_PKEY		*pubkey;
	EVP_PKEY		*privkey;
	unsigned char		*keyid;
	int			 kidlen;
	X509			*req;
};

void	 cert_init(int);
void	 cert_reconfigure(int);
char *	 cert_keyid_for_uid(uid_t uid);

struct cert	*cert_get_by_uid(uid_t u);
struct cert	*cert_get_by_keyid(const unsigned char *keyid, int klen);
void		 cert_load_priv_key(struct cert *, const char *, char *);

#endif	/* _CERT_H_ */
