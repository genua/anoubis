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
#include <anoubis_msg.h>
#include <cert.h>
#include <anoubis_alloc.h>

#define ANOUBISD_CSUM_NONE	0
#define ANOUBISD_CSUM_USER	1
#define ANOUBISD_CSUM_ROOT	2

#define SFSDATA_FORMAT_VERSION		1
#define SFSDATA_MAX_FIELD_LENGTH	4096

struct sfs_csmulti_record {
	unsigned int		 index;
	const char		*path;
	struct abuf_buffer	 csdata;
};

struct sfs_checksumop {
	int		 op;		/* Checksum Operation */
	unsigned int	 listflags;	/* Flags for list operations */
	uid_t		 uid;		/* Requested UID */
	uid_t		 auth_uid;	/* UID of requesting user */
	int		 idlen;		/* Length of key id (if any) */
	const u_int8_t	*keyid;		/* The Key ID in binary form */
	/* Only used for non-csmulti requests */
	struct abuf_buffer	sigbuf;	/* Signature data */
	const char	*path;		/* NUL-terminated path name. */
	/* Only used for cs-multi requests */
	unsigned int	 nrec;		/* Number of recrods */
	struct sfs_csmulti_record *csmulti;	/* The records. */
};

struct sfs_data {
	struct abuf_buffer	sigdata;
	struct abuf_buffer	csdata;
	struct abuf_buffer	upgradecsdata;
};

int	 sfs_checksumop(const struct sfs_checksumop *csop,
	     struct abuf_buffer *buf);
int	 sfs_checksumop_chroot(const struct sfs_checksumop *csop,
	     struct sfs_data *data);
void	 sfs_freesfsdata(struct sfs_data *);
int	 sfs_haschecksum_chroot(const char *path);
int	 sfs_update_all(const char *path, u_int8_t *md, unsigned int len);
int	 sfs_update_signature(const char *path, struct cert *cert,
	     u_int8_t *md, int len);
int	 convert_user_path(const char *path, char **dir, int is_dir);
char	*remove_escape_seq(const char *name);
void	 sfs_remove_index(const char *, struct sfs_checksumop *);
int	 sfs_wantentry(const struct sfs_checksumop *, const char *sfs_path,
	     const char *entryname);
int	 sfs_parse_checksumop(struct sfs_checksumop *dst,
	     struct anoubis_msg *m, uid_t auth_uid);
int	 sfs_parse_csmulti(struct sfs_checksumop *dst,
	     struct abuf_buffer msg, uid_t auth_uid);

/* Public SFS-Cache functions */
void	 sfshash_init(void);
void	 sfshash_flush(void);
void	 sfshash_invalidate_uid(const char *, uid_t);
void	 sfshash_invalidate_key(const char *, const char*);
int	 sfshash_get_uid(const char *, uid_t, struct abuf_buffer *);
int	 sfshash_get_key(const char *, const char *, struct abuf_buffer *);

#endif	/* _SFS_H_ */
