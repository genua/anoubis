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

#ifndef _SFSSIG_H_
#define _SFSSIG_H_

#include <limits.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <getopt.h>
#include <pwd.h>

#include <sys/un.h>
#include <sys/mman.h>
#include <sys/socket.h>

#ifndef NEEDBSDCOMPAT
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <anoubis_protocol.h>
#include <anoubis_errno.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>
#include <anoubis_dump.h>
#include <anoubis_sig.h>
#include <anoubischat.h>

#include "csum/csum.h"

#ifdef LINUX
#include <bsdcompat.h>
#include <linux/anoubis.h>
#include <openssl/sha.h>
#include <attr/xattr.h>
#endif
#ifdef OPENBSD
#include <sha2.h>
#include <dev/anoubis.h>
#endif

#ifndef __used
#define __used __attribute__((unused))
#endif

#ifndef __dead
#define __dead __attribute__((noreturn))
#endif

#define SFSSIG_OPT_NOACTION		0x00001
#define SFSSIG_OPT_VERBOSE		0x00002
#define SFSSIG_OPT_VERBOSE2		0x00004
#define SFSSIG_OPT_TREE			0x00008
#define SFSSIG_OPT_DEBUG		0x00020
#define SFSSIG_OPT_DEBUG2		0x00040
#define SFSSIG_OPT_SIG			0x00080
#define SFSSIG_OPT_SUM			0x00100

#define SFSSIG_OPT_HASSUM		0x00200
#define SFSSIG_OPT_NOSUM		0x00400
#define SFSSIG_OPT_HASSIG		0x00800
#define SFSSIG_OPT_NOSIG		0x01000
#define SFSSIG_OPT_ORPH			0x02000
#define SFSSIG_OPT_NOTFILE		0x04000
#define SFSSIG_OPT_UPGRADED		0x08000
#define SFSSIG_OPT_FILTER		0x0fe00

#define SFSSIG_OPT_LN			0x10000
#define SFSSIG_OPT_FORCE		0x20000
#define SFSSIG_OPT_ALLUID		0x40000
#define SFSSIG_OPT_ALLCERT		0x80000

#define SYSSIGNAME "security.anoubis_syssig"
#define SKIPSUMNAME "security.anoubis_skipsum"

int filter_one_file(char *path, char *prefix, uid_t uid,
    struct anoubis_sig *as);
int	filter_hassum(char *arg, uid_t uid);
int	filter_hassig(char *arg, struct anoubis_sig *as);
struct anoubis_sig *load_keys(int priv_key, int show_err, char *cert,
    char *keyfile);
char **file_input(int *file_cnt, char *file);
char *sfs_realpath(const char *path, char resolved[PATH_MAX]);
char *build_path(const char *path, const char *file);
int openregular(char *filename, int *error);
int opensha256(char *filename, unsigned char md[SHA256_DIGEST_LENGTH],
    int *error);
int syssig_add(char *file, unsigned char cksum[SHA256_DIGEST_LENGTH]);
int syssig_list(int argc, char *argv[]);
int syssig_update(int argc, char *argv[]);
int syssig_remove(int argc, char *argv[]);
int skipsum_update(char *file, int op);
int str2hash(const char *s, unsigned char dest[SHA256_DIGEST_LENGTH]);
struct anoubis_transaction	*sfs_sumop_flags(char *file, int operation,
    u_int8_t *cs, int cslen, int idlen, uid_t uid, unsigned int);

extern unsigned int opts;

#endif	/* _SFSSIG_H_ */
