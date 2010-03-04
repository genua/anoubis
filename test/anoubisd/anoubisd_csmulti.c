/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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
#include "version.h"

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <err.h>
#include <unistd.h>
#include <syslog.h>

#ifdef LINUX
#include <bsdcompat.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sha2.h>
#include <dev/anoubis.h>
#endif

#include <anoubis_msg.h>
#include <anoubis_protocol.h>
#include <anoubis_errno.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_sig.h>
#include <anoubis_transaction.h>
#include <anoubis_dump.h>
#include <protocol_utils.h>

#include <anoubis_chat.h>

static struct achat_channel	*channel = NULL;
struct anoubis_client		*client = NULL;

struct csfile {
	char		*path;
	unsigned char	 csdata[ANOUBIS_CS_LEN];
};

#define NFILES	1024

static struct csfile	csfiles[NFILES];

static void
init_files(void)
{
	int		i;

	for (i=0; i<NFILES; ++i) {
		if (asprintf(&csfiles[i].path, "/virtual/file%d", i) < 0) {
			perror("asprintf");
			exit(1);
		}
		memcpy(csfiles[i].csdata, &i, sizeof(int));
	}
}

static int
process_request(struct anoubis_csmulti_request *req)
{
	int		ret;

	while (1) {
		struct anoubis_csmulti_record	*r;
		struct anoubis_transaction	*t;

		TAILQ_FOREACH(r, &req->reqs, next)
			if (r->error == EAGAIN)
				break;
		if (r == NULL)
			break;
		t = anoubis_client_csmulti_start(client, req);
		if (!t)
			return -ENOMEM;
		while ((t->flags & ANOUBIS_T_DONE) == 0) {
			int ret = anoubis_client_wait(client);
			if (ret <= 0) {
				if (ret == 0)
					fprintf(stderr,
					    "Client wait returned EOF\n");
				return ret;
			}
		}
		ret = -t->result;
		anoubis_transaction_destroy(t);
		if (ret) {
			fprintf(stderr, "Tranaction error: %d\n", -ret);
			return ret;
		}
	}
	return 0;
}

struct anoubis_csmulti_request *
create_request(int op)
{
	struct anoubis_csmulti_request	*req;
	int				 ret, i;

	req = anoubis_csmulti_create(op, geteuid(), NULL, 0);
	if (!req)
		return NULL;
	for (i=0; i<NFILES; ++i) {
		if (op == ANOUBIS_CHECKSUM_OP_ADDSUM) {
			ret = anoubis_csmulti_add(req, csfiles[i].path,
			    csfiles[i].csdata, ANOUBIS_CS_LEN);
		} else {
			ret = anoubis_csmulti_add(req, csfiles[i].path,
			    NULL, 0);
		}
		if (ret < 0) {
			errno = -ret;
			perror("anoubis_csmulti_add");
			return NULL;
		}
	}
	return req;
}

static int
verify_errors(struct anoubis_csmulti_request *req, int errs[])
{
	struct anoubis_csmulti_record	*r;
	int				 ret = 0;

	TAILQ_FOREACH(r, &req->reqs, next) {
		int		i;

		for (i=0; errs[i] >= 0; ++i)
			if (r->error == (unsigned int)errs[i])
				break;
		if (errs[i] < 0) {
			fprintf(stderr, "Wrong error %d for index %d\n",
			    r->error, r->idx);
			ret = -EINVAL;
		}
	}
	return 0;
}

static int
verify_csums(struct anoubis_csmulti_request *req)
{
	struct anoubis_csmulti_record	*r;
	int				 ret = 0;

	TAILQ_FOREACH(r, &req->reqs, next) {
		if (r->error) {
			fprintf(stderr, "Checksum request error: %d idx=%d\n",
			    r->error, r->idx);
			ret = -EINVAL;
			continue;
		}
		if (r->u.get.csum == NULL
		    || get_value(r->u.get.csum->cstype) != ANOUBIS_SIG_TYPE_CS
		    || get_value(r->u.get.csum->cslen) != ANOUBIS_CS_LEN) {
			fprintf(stderr, "No/Bad checksum for index %d\n",
			    r->idx);
			ret = -EINVAL;
			continue;
		}
		if (memcmp(r->u.get.csum->csdata, csfiles[r->idx].csdata,
		    ANOUBIS_CS_LEN) != 0) {
			fprintf(stderr, "Wrong checksum data for index %d\n",
			    r->idx);
			ret = -EINVAL;
		}
	}
	return ret;
}


static int	 err_noent[] = { 0, ENOENT, -1 };
static int	 err_ok[] = { 0, -1 };

int main()
{
	struct anoubis_csmulti_request		*req;
	int					 ret;

	init_files();
	create_channel(&channel, &client);

	/* First delete all entries. */
	req = create_request(ANOUBIS_CHECKSUM_OP_DEL);
	if (!req) {
		perror("create_request");
		return 1;
	}
	ret = process_request(req);
	if (ret < 0) {
		errno = -ret;
		perror("process_request (DEL)");
		return 1;
	}
	if (verify_errors(req, err_noent) < 0) {
		fprintf(stderr, "Wrong error codes at line %s:%d\n",
		    __FILE__, __LINE__);
		return 1;
	}
	anoubis_csmulti_destroy(req);

	/* Add checksum for all files. */
	req = create_request(ANOUBIS_CHECKSUM_OP_ADDSUM);
	if (!req) {
		perror("create_request");
		return 1;
	}
	ret = process_request(req);
	if (ret < 0) {
		errno = -ret;
		perror("process_request (ADDSUM)");
		return 1;
	}
	if (verify_errors(req, err_ok) < 0) {
		fprintf(stderr, "Wrong error codes at line %s:%d\n",
		    __FILE__, __LINE__);
		return 1;
	}
	anoubis_csmulti_destroy(req);

	/* Get and validate all checksums. */
	req = create_request(ANOUBIS_CHECKSUM_OP_GET2);
	if (!req) {
		perror("create_request");
		return 1;
	}
	ret = process_request(req);
	if (ret < 0) {
		errno = -ret;
		perror("process_request (GET2)");
		return 1;
	}
	if (verify_csums(req) < 0) {
		fprintf(stderr, "Wrong checksums at line %s:%d\n",
		    __FILE__, __LINE__);
		return 1;
	}
	anoubis_csmulti_destroy(req);

	destroy_channel(channel, client);
	return 0;
}
