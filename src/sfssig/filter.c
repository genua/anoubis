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

#include "config.h"

#include "sfssig.h"

static int	filter_orphaned(char *arg);
static int	filter_notfile(char *arg);
static int	filter_hasnosum(char *arg, uid_t uid);
static int	filter_hasnosig(char *arg, struct anoubis_sig *as);
static int	filter_sumsig(char *arg, int op, unsigned int idlen, unsigned
		    char *keyid, uid_t uid);


static int
filter_orphaned(char *arg)
{
	struct stat sb;

	if (lstat(arg, &sb) < 0) {
		if (errno == ENOENT)
			return 1;
		else
			return 0;
	}
	if (S_ISREG(sb.st_mode))
		return 0;
	else
		return 1;
}

static int
filter_notfile(char *arg)
{
	struct stat sb;

	if (lstat(arg, &sb) < 0) {
		return 0;
	}
	if (S_ISREG(sb.st_mode))
		return 0;
	else
		return 1;
}

int
filter_hassum(char *arg, uid_t uid)
{
	return filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GET2, 0, NULL, uid);
}

int
filter_hassig(char *arg, struct anoubis_sig *as)
{
	if ((as == NULL) || (as->keyid == NULL)) {
			fprintf(stderr, "You need to specify a "
			    "certifcate\n");
			return 0;
	}
	return filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GETSIG2, as->idlen,
	    as->keyid, 0);
}

static int
filter_hasnosum(char *arg, uid_t uid)
{
	int rc = 0;
	rc = filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GET2, 0, NULL, uid);
	if (rc)
		return 0;
	else
		return 1;
}

static int
filter_hasnosig(char *arg, struct anoubis_sig *as)
{
	int rc = 0;
	if ((as == NULL) || (as->keyid == NULL)) {
			fprintf(stderr, "You need to specify a "
			    "certifcate\n");
			return 0;
	}
	rc =  filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GETSIG2, as->idlen,
	    as->keyid, 0);
	if (rc)
		return 0;
	else
		return 1;
}

static int
filter_sumsig(char *arg, int op, unsigned int idlen, unsigned char *keyid,
    uid_t uid)
{
	struct anoubis_transaction	*t = NULL;
	char				 tmp[PATH_MAX];
	int				 len;
	const void			*data;

	if (sfs_realpath(arg, tmp) == NULL)
		return 0;

	t = sfs_sumop(tmp, op, keyid, 0, idlen, uid);
	if (!t) {
		if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "filter_sumsig: failed transaction\n");
		return 0;
	}
	if (t->result) {
		if (opts & SFSSIG_OPT_VERBOSE2)
			fprintf(stderr, "%s: Has no entry\n", arg);
		anoubis_transaction_destroy(t);
		return 0;
	}
	if (op == ANOUBIS_CHECKSUM_OP_GET2) {
		len = anoubis_extract_sig_type(t->msg, ANOUBIS_SIG_TYPE_CS,
		    &data);
	} else {
		len = anoubis_extract_sig_type(t->msg, ANOUBIS_SIG_TYPE_SIG,
		    &data);
	}
	anoubis_transaction_destroy(t);
	return (len > 0);
}

int
filter_one_file(char *arg, char *prefix, uid_t uid, struct anoubis_sig *as)
{
	int		 ret = 1;
	char		*altpath;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">filter_one_file: %s\n", arg);

	if (!arg)
		return 0;

	if (prefix) {
		altpath = build_path(prefix, arg);
	} else {
		altpath = arg;
	}
	assert(altpath);
	if (opts & SFSSIG_OPT_ORPH && ret)
		ret = filter_orphaned(altpath);
	if (opts & SFSSIG_OPT_NOTFILE && ret)
		ret = filter_notfile(altpath);
	if (opts & SFSSIG_OPT_HASSUM && ret)
		ret = filter_hassum(altpath, uid);
	if (opts & SFSSIG_OPT_NOSUM && ret)
		ret = filter_hasnosum(altpath, uid);
	if (opts & SFSSIG_OPT_HASSIG && ret)
		ret = filter_hassig(altpath, as);
	if (opts & SFSSIG_OPT_NOSIG && ret)
		ret = filter_hasnosig(altpath, as);
	if (prefix)
		free(altpath);
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<filter_args: %d\n", ret);

	return ret;
}
