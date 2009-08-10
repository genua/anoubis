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

#ifndef _SFS_H_
#define _SFS_H_

#include <sys/types.h>
#include <anoubisd.h>

#define ANOUBISD_CSUM_NONE	0
#define ANOUBISD_CSUM_USER	1
#define ANOUBISD_CSUM_ROOT	2

struct sfs_checksumop {
	int		 op;		/* Checksum Operation */
	int		 allflag;	/* ANOUBIS_CSUM_ALL was given */
	uid_t		 uid;		/* Requested UID */
	uid_t		 auth_uid;	/* UID of requesting user */
	int		 idlen;		/* Length of key id (if any) */
	u_int8_t	*keyid;		/* The Key ID in binary form */
	int		 siglen;	/* Length of csum/signature data */
	u_int8_t	*sigdata;	/* Signature data */
	char		*path;		/* NUL-terminated path name. */

};

int	 sfs_checksumop(const char *path, unsigned int operation, uid_t uid,
	     unsigned char *md, u_int8_t **sign, int *siglen, int len,
	     int idlen);
int	 sfs_checksumop_chroot(const char *path, unsigned int operation,
	     uid_t uid, unsigned char *md, u_int8_t **sign, int *siglen,
	     int len, int idlen);
int	 sfs_haschecksum_chroot(const char *path);
int	 sfs_update_all(const char *path, u_int8_t *md, int len);
int	 convert_user_path(const char *path, char **dir, int is_dir);
int	 check_if_exist(const char *path);
char	*remove_escape_seq(const char *name);

/* Public SFS-Cache functions */
void	 sfshash_init(void);
void	 sfshash_flush(void);
void	 sfshash_invalidate_uid(const char *, uid_t);
void	 sfshash_invalidate_key(const char *, const char*);
int	 sfshash_get_uid(const char *, uid_t, u_int8_t cs[ANOUBIS_CS_LEN]);
int	 sfshash_get_key(const char *, const char *,
	     u_int8_t cs[ANOUBIS_CS_LEN]);

#endif	/* _SFS_H_ */
