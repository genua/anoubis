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

#ifndef _CSUM_H_
#define _CSUM_H_

#include <sys/types.h>
#include <sys/cdefs.h>
#include <stdio.h>

#include <anoubis_msg.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

struct sfs_entry {
	char			*name;
	unsigned char		*checksum;
	unsigned char		*signature;
	int			 siglen;
	uid_t			 uid;
	unsigned char		*keyid;
	int			 keylen;
	struct sfs_entry	*next;
};

__BEGIN_DECLS

int	  anoubis_csum_calc(const char *file, u_int8_t *cs, int *cslen);
int	  anoubis_csum_link_calc(const char *link, u_int8_t *csbuf,
    int *cslen);
char	**anoubis_csum_list(struct anoubis_msg *m, int *listcnt);
int	  anoubis_print_checksum(FILE *fd, unsigned char *checksum, int len);
int	  anoubis_print_keyid(FILE *fd, unsigned char *key, int len);
int	  anoubis_print_signature(FILE *fd, unsigned char *signature, int len);
int	  anoubis_print_file(FILE *fd, char *name);
int	  anoubis_print_entries(FILE *fd, struct sfs_entry **list,
    int cnt);
void	  anoubis_entry_free(struct sfs_entry *se);
struct sfs_entry *anoubis_build_entry(const char *name,
    const unsigned char *checksum, int csumlen, const unsigned char *signature,
    int siglen, uid_t uid, const unsigned char *keyid, int keylen);
unsigned char ** anoubis_keyid_list(struct anoubis_msg *m, int **idlen_list,
    int *list_cnt);
unsigned char *string2hex(const char *hex, int *cnt);
struct sfs_entry *import_csum(FILE *file);

__END_DECLS

#endif	/* _CSUM_H_ */
