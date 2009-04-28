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
static int	filter_hasnosum(char *arg);
static int	filter_hasnosig(char *arg, struct anoubis_sig *as);
static int	filter_sumsig(char *arg, int op, unsigned int idlen, unsigned
		    char *keyid);


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
filter_hassum(char *arg)
{
	return filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GET, 0, NULL);
}

int
filter_hassig(char *arg, struct anoubis_sig *as)
{
	if ((as == NULL) || (as->keyid == NULL)) {
			fprintf(stderr, "You need to specify a "
			    "certifcate\n");
			return 0;
	}
	return filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GETSIG, as->idlen,
	    as->keyid);
}

static int
filter_hasnosum(char *arg)
{
	int rc = 0;
	rc = filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GET, 0, NULL);
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
	rc =  filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GETSIG, as->idlen,
	    as->keyid);
	if (rc)
		return 0;
	else
		return 1;
}

static int
filter_sumsig(char *arg, int op, unsigned int idlen, unsigned char *keyid)
{
	struct anoubis_transaction	*t = NULL;
	char				 tmp[PATH_MAX];

	if (sfs_realpath(arg, tmp) == NULL)
		return 0;

	t = sfs_sumop(tmp, op, keyid, 0, idlen);
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
	} else {
		return 1;
	}
}

char **
filter_args(int *argc, char **argv, char *path, struct anoubis_sig *as)
{
	int ret, i;
	int cnt = 0;
	char				**result = NULL;
	char				*altpath;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">filter_args\n");

	if (argc == NULL || *argc <= 0)
		return NULL;
	if (argv == NULL) {
		*argc = -1;
		return NULL;
	}

	for (i = 0; i < *argc; i++) {
		ret = 1;

		if (path) {
			if ((altpath = build_path(path, argv[i])) == NULL) {
				*argc = -1;
				return NULL;
			}
		} else {
			 altpath = strdup(argv[i]);
			 if (!altpath) {
				 *argc = -1;
				 return NULL;
			 }
		}
		if (opts & SFSSIG_OPT_ORPH && ret)
			ret = filter_orphaned(altpath);
		if (opts & SFSSIG_OPT_NOTFILE && ret)
			ret = filter_notfile(altpath);
		if (opts & SFSSIG_OPT_HASSUM && ret)
			ret = filter_hassum(altpath);
		if (opts & SFSSIG_OPT_NOSUM && ret)
			ret = filter_hasnosum(altpath);
		if (opts & SFSSIG_OPT_HASSIG && ret)
			ret = filter_hassig(altpath, as);
		if (opts & SFSSIG_OPT_NOSIG && ret)
			ret = filter_hasnosig(altpath, as);

		if (ret) {
			cnt++;
			if ((result = realloc(result, cnt * sizeof(char *)))
			    == NULL) {
				*argc = -1;
				return NULL;
			}
			result[cnt-1] = strdup(argv[i]);
			if (!result[cnt-1]) {
				*argc = -1;
				return NULL;
			}
		}

		if (altpath) {
			free(altpath);
			altpath = NULL;
		}
	}

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<filter_args\n");

	*argc = cnt;
	return result;
}
