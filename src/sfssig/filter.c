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

/**
 * Add a checksum filter request for a file. This will
 * add an request with an GET operation. So its used
 * for HASSUM/NOSUM
 *
 * @param file which should be filtered.
 * @param uid which registered (not) a checksum
 * @param callback which handels the result of the request
 * @return 1 keep file till processing filter,
 *	   0 file can be dismissed
 *	  -1 error occurd
 */
static int filter_sum(char *, uid_t, sumop_callback_t);

/**
 * Add a signature filter request for a file. This will
 * add an request with an GETSIG operation. So its used
 * for HASSIG/NOSIG/ISUPGRADED
 *
 * @param file which should be filtered.
 * @param anoubis_sig struct which contains a keyid. The keyid specifies the
 *	  (not) registered signature
 * @param callback which handels the result of the request
 * @return 1 keep file till processing filter,
 *	   0 file can be dismissed
 *	  -1 error occurd
 */
static int filter_sig(char *, struct anoubis_sig *, sumop_callback_t);

/**
 * Add an request to the filter reqeust tree.
 * @param file which should be filtered.
 * @param operation which should be added ANOUBIS_CHECKSUM_OP_GET2 or
 *	  ANOUBIS_CHECKSUM_OP_GETSIG2 only.
 * @param lenght of the keyid
 * @param keyid of the request
 * @param uid of the request
 * @param callback which handels the result of the request
 * @return 1 keep file till processing filter,
 *	   0 file can be dismissed
 *	  -1 error occurd
 */
static int filter_request_add(char *, int, unsigned int,
    unsigned char *, uid_t, filter_callback_t);

int
filter_hassum_callback(struct anoubis_csmulti_request *req,
    struct anoubis_csmulti_record *rec)
{
	if (req == NULL || rec == NULL || rec->error)
		return 0;
	if (rec->u.get.csum) {
		return (get_value(rec->u.get.csum->cslen) > 0);
	} else
		return 0;
}

int
filter_hasnosum_callback(struct anoubis_csmulti_request *req,
    struct anoubis_csmulti_record *rec)
{
	if (req == NULL || rec == NULL)
		return 0;
	if (rec->error == ENOENT)
		return 1;
	if (rec->error)
		return 0;
	if (rec->u.get.csum)
		return 0;
	else
		return 1;
}

int
filter_hassig_callback(struct anoubis_csmulti_request *req,
    struct anoubis_csmulti_record *rec)
{
	if (req == NULL || rec == NULL || rec->error)
		return 0;
	if (rec->u.get.sig)
		return (get_value(rec->u.get.sig->cslen) > 0);
	else
		return 0;
}

int
filter_hasnosig_callback(struct anoubis_csmulti_request *req,
    struct anoubis_csmulti_record *rec)
{
	if (req == NULL || rec == NULL)
		return 0;
	if (rec->error == ENOENT)
		return 1;
	if (rec->error)
		return 0;
	if (rec->u.get.sig)
		return 0;
	else
		return 1;
}

int
filter_upgraded_callback(struct anoubis_csmulti_request *req,
    struct anoubis_csmulti_record *rec)
{
	if (req == NULL || rec == NULL || rec->error)
		return 0;
	if (rec->u.get.upgrade)
		return (get_value(rec->u.get.upgrade->cslen) > 0);
	else
		return 0;
}

static int
filter_sum(char *altpath, uid_t uid, sumop_callback_t callback)
{
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, " filter_sum\n");
	return filter_request_add(altpath, ANOUBIS_CHECKSUM_OP_GET2, 0, NULL,
	    uid, callback);
}

static int
filter_sig(char *altpath, struct anoubis_sig *as, sumop_callback_t callback)
{
	if ((as == NULL) || (as->keyid == NULL)) {
		fprintf(stderr, "You need to specify a certifcate\n");
		return 0;
	}
	return filter_request_add(altpath, ANOUBIS_CHECKSUM_OP_GETSIG2,
	    as->idlen, as->keyid, 0, callback);
}

static int
filter_request_add(char *arg, int op, unsigned int idlen, unsigned char *keyid,
    uid_t uid, filter_callback_t callback)
{
	struct sfs_request_node *node = NULL;
	char	tmp[PATH_MAX];
	int error;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">filter_request_add\n");

	if (!arg)
		return -EINVAL;

	if (sfs_realpath(arg, tmp) == NULL)
		return -EINVAL;

	if (op != ANOUBIS_CHECKSUM_OP_GETSIG2 &&
	    op != ANOUBIS_CHECKSUM_OP_GET2)
		return -EINVAL;

	node = sfs_find_request(filter_tree, uid, keyid, idlen, op);
	if (node == NULL) {
		node = sfs_insert_request_node(filter_tree, op, uid, keyid,
		    idlen, NULL, callback);
		if (node == NULL)
			return -ENOMEM;
	}

	error = anoubis_csmulti_add(node->req, tmp, NULL, 0);
	if (error != 0)
		return -error;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<filter_request_add\n");

	return 1;
}

int
filter_a_file(char *arg, char *prefix, uid_t uid, struct anoubis_sig *as)
{
	int		 ret = 1;
	char		*altpath;
	struct stat	 sb;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">filter_a_file: %s\n", arg);

	if (!arg)
		return 0;

	if (prefix) {
		altpath = build_path(prefix, arg);
	} else {
		altpath = arg;
	}
	assert(altpath);
	if (lstat(altpath, &sb) < 0) {
		/*
		 * Cannot stat file. We are only interested if we filter
		 * for orphaned files explicitly
		 */
		ret = !!(opts & SFSSIG_OPT_ORPH);
	} else {
		/* Object exists. */
		if (opts & SFSSIG_OPT_ORPH) {
			/* Not interested if we only look for orphaned. */
			ret = 0;
		} else if (opts & SFSSIG_OPT_NOTFILE) {
			/* With NOTFILE, only look for non regular files */
			ret = !S_ISREG(sb.st_mode);
		} else {
			/* Neither ORPHANED nor NOTFILE.  Must be regular. */
			ret = !!S_ISREG(sb.st_mode);
		}
	}

	if ((opts & SFSSIG_OPT_HASSUM) && ret)
		ret = filter_sum(altpath, uid, filter_hassum_callback);
	if ((opts & SFSSIG_OPT_NOSUM) && ret)
		ret = filter_sum(altpath, uid, filter_hasnosum_callback);
	if ((opts & SFSSIG_OPT_HASSIG) && ret)
		ret = filter_sig(altpath, as, filter_hassig_callback);
	if ((opts & SFSSIG_OPT_NOSIG) && ret)
		ret = filter_sig(altpath, as, filter_hasnosig_callback);
	if ((opts & SFSSIG_OPT_UPGRADED) && ret)
		ret = filter_sig(altpath, as, filter_upgraded_callback);
	if (prefix)
		free(altpath);
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<filter_a_file: %d\n", ret);

	return ret;
}
