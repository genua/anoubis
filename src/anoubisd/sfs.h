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

/**
 * Data structure that is used to manage individual entries in a csmulti
 * request. Each checksumop for a csmulti request hold an array of
 * theses structures. These structure are created during paring or the
 * request message.
 *
 * Fields:
 * @index: The index of this request. This is copied from the index field
 *     in the request message.
 * @path: The path name that this csmulti entriy applies to.
 * @csdata: The buffer that hold additional data for add requests.
 *
 * Both the pathname and the checksum data reference memory that is
 * located in the request message. This means that this data must not
 * be freed indepenently. Instead the original request message must be
 * freed.
 */
struct sfs_csmulti_record {
	unsigned int		 index;
	const char		*path;
	struct abuf_buffer	 csdata;
};

/* This defines a bounds checked array of sfs_csmulti_record structures. */
#include <sfs_csmulti_record_array.h>

/**
 * This data structure encapsulates data of a checksum request.
 * Normally, the data in this structure references the request message
 * directly, i.e. the memory of field like @path or @sigbuf must not be
 * freed independently.
 *
 * Fields:
 * @op: Checksum operation (see anoubis_protocol.h).
 * @listflags: Checksum flags for a list operation (see anoubis_protocol.h
 *     for a list of possible flags. This field is only used for list
 *     operations, it is always zero for other operations (add, get, del).
 * @uid: The user-ID that this operation applies to. This field is only used
 *     is @keyid is not empty.
 * @auth_uid: The user-ID of the authenticated user that initiated the request.
 * @keyid: The (binary representation of) the keyid that this request applies
 *     to.
 * @sigbuf: This buffer contains checksum or signature data for add
 *     operations. It is only used for old style checksum operations,
 *     csmulti operations only use it temporarily. It's use is deprecated.
 * @path: The path name string for old style checksum operations. The field
 *     is only used for old style checksum operations, csmulti operations
 *     only use it temporarily. It's use is deprecated.
 * @nrec: The total number of request records in a csmulti checksum
 *     operation. Old style checksum operations do not use this field.
 * @csmulti: An array of csmulti request records. The memory for the array
 *     is allocated dynamically. The memory referenced from the records in
 *     this array is part of the request message.
 */
struct sfs_checksumop {
	int				 op;
	unsigned int			 listflags;
	uid_t				 uid;
	uid_t				 auth_uid;
	struct abuf_buffer		 keyid;
	/* Only used for non-csmulti requests */
	struct abuf_buffer		 sigbuf;
	const char			*path;
	/* Only used for cs-multi requests */
	unsigned int			 nrec;
	struct csmultiarr_array		 csmulti;
};

/**
 * This structure is used to transfer checksum and signature data to and
 * from an on disk file.
 *
 * Fields:
 * @sigdata: The signature.
 * @csdata: The unsigned checksum.
 * @upgradecsdata: The checksum from an upgrade.
 */
struct sfs_data {
	struct abuf_buffer	sigdata;
	struct abuf_buffer	csdata;
	struct abuf_buffer	upgradecsdata;
};

/*
 * Public SFS tree functions.
 *
 * Implementation and inline documentation can be found in sfs.c.
 */
int	 sfs_checksumop(const struct sfs_checksumop *csop,
	     struct abuf_buffer *buf);
int	 sfs_checksumop_chroot(const struct sfs_checksumop *csop,
	     struct sfs_data *data);
void	 sfs_freesfsdata(struct sfs_data *);
int	 sfs_haschecksum_chroot(const char *path);
int	 sfs_update_all(const char *path, struct abuf_buffer md);
int	 sfs_update_signature(const char *path, struct cert *cert,
	     struct abuf_buffer md);
int	 convert_user_path(const char *path, char **dir, int is_dir);
char	*remove_escape_seq(const char *name);
void	 sfs_remove_index(const char *, struct sfs_checksumop *);
int	 sfs_wantentry(const struct sfs_checksumop *, const char *sfs_path,
	     const char *entryname);
int	 sfs_parse_checksumop(struct sfs_checksumop *dst,
	     struct anoubis_msg *m, uid_t auth_uid);
int	 sfs_parse_csmulti(struct sfs_checksumop *dst,
	     struct abuf_buffer msg, uid_t auth_uid);

/*
 * Public SFS-Cache functions.
 *
 * Implementation and inline documentation can be found in pe_sfscache.c.
 */
void	 sfshash_init(void);
void	 sfshash_flush(void);
void	 sfshash_invalidate_uid(const char *, uid_t);
void	 sfshash_invalidate_key(const char *, const char*);
int	 sfshash_get_uid(const char *, uid_t, struct abuf_buffer *);
int	 sfshash_get_key(const char *, const char *, struct abuf_buffer *);

#endif	/* _SFS_H_ */
