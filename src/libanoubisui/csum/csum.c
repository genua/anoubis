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

#include <config.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <anoubis_msg.h>

#ifdef LINUX
#include <linux/anoubis.h>
#include <bsdcompat.h>
#endif

#ifdef OPENBSD
#include <sha2.h>
#include <dev/anoubis.h>
#endif

#include <openssl/sha.h>

#include "csum.h"

static int
chartohex(char ch)
{
#ifndef S_SPLINT_S
	switch(ch) {
	case '0' ... '9':
		return ch - '0';
	case 'A' ... 'F':
		return 10 + ch - 'A';
	case 'a' ... 'f':
		return 10 + ch - 'a';
	default:
		break;
	}
#endif
	return -1;
}

unsigned char *
string2hex(const char *hex, int *cnt)
{
	unsigned char *res;
	int i, len, k, tmp1, tmp2;

	if (!hex || !cnt)
		return NULL;

	len = strlen(hex);
	if (len%2)
		return NULL;

	res = calloc(len/2, sizeof(unsigned char));
	if (!res)
		return NULL;

	for (i = 0, k = 0; i < len-1; i+=2, k++) {
		tmp1 = chartohex(hex[i]) * 16;
		if (tmp1 < 0)
			goto err;

		tmp2 = chartohex(hex[i+1]);
		if (tmp2 < 0)
			goto err;
		res[k] = tmp1 + tmp2;
	}
	*cnt = len / 2;
	return res;
err:
	free(res);
	return NULL;
}

int
anoubis_csum_calc(const char *file, u_int8_t * csbuf, int *cslen)
{
	int ret, fd, afd;
	struct anoubis_ioctl_csum cs;

	if (*cslen < ANOUBIS_CS_LEN)
		return -EINVAL;
	afd = open("/dev/anoubis", O_RDONLY);
	if (afd < 0)
		return -errno;
	fd = open(file, O_RDONLY);
	if (fd < 0) {
		ret = -errno;
		close(afd);
		return ret;
	}
	cs.fd = fd;
	if (ioctl(afd, ANOUBIS_GETCSUM, &cs) < 0) {
		ret = -errno;
		close(fd);
		close(afd);
		return ret;
	}
	close(fd);
	close(afd);
	memcpy(csbuf, cs.csum, ANOUBIS_CS_LEN);
	*cslen = ANOUBIS_CS_LEN;
	return 0;
}

int anoubis_csum_calc_userspace(const char *file, u_int8_t *cs, int *cslen)
{
	SHA256_CTX	shaCtx;
	int		fd, nread;
	unsigned char	buf[1024];

	if ((file == NULL) || (cs == NULL) || (cslen == NULL) ||
	    (*cslen < ANOUBIS_CS_LEN)) {
		return -EINVAL;
	}

	fd = open(file, O_RDONLY);
	if (fd == -1)
		return (-errno);

	SHA256_Init(&shaCtx);

	/* Read file chunk by chunk and put it into SHA256_CTX */
	while ((nread = read(fd, buf, sizeof(buf))) > 0) {
		SHA256_Update(&shaCtx, buf, nread);
	}

	SHA256_Final(cs, &shaCtx);

	close(fd);

	if (nread == -1) {
		/* read operation failed */
		return (-errno);
	}

	return (0);
}

int
anoubis_csum_link_calc(const char *link, u_int8_t * csbuf, int *cslen)
{
	SHA256_CTX	shaCtx;
	char		buf[PATH_MAX];
	struct stat	sb;
	ssize_t		ret;

	if (*cslen < ANOUBIS_CS_LEN)
		return -EINVAL;

	if(lstat(link, &sb) < 0)
		return -errno;
	if (!S_ISLNK(sb.st_mode))
		return anoubis_csum_calc(link, csbuf, cslen);

	if ((ret = readlink(link, buf, PATH_MAX)) != -1)
		buf[ret] = '\0';
	SHA256_Init(&shaCtx);
	SHA256_Update(&shaCtx, buf, ret);
	SHA256_Final(csbuf, &shaCtx);

	*cslen = ANOUBIS_CS_LEN;
	return 0;
}

unsigned char **
anoubis_keyid_list(struct anoubis_msg *m, int **idlen_list, int *list_cnt)
{
	unsigned char	**result = NULL;
	char		**tmp_res = NULL;
	int		 *id_res = NULL;
	int		  cnt = 0, i;

	if (m == NULL || list_cnt == NULL)
		return NULL;

	if ((tmp_res = anoubis_csum_list(m, &cnt)) == NULL) {
		*list_cnt = 0;
		return NULL;
	}
	if (cnt <= 0) {
		*list_cnt = 0;
		return NULL;
	}
	*list_cnt = cnt;
	result = calloc(cnt, sizeof(unsigned char *));
	if (!result)
		goto err;
	id_res = calloc(cnt, sizeof(int));
	if (!id_res)
		goto err;
	for (i = 0; i < cnt; i++) {
		if (tmp_res[i][0] == 'k')
			result[i] = string2hex(&tmp_res[i][1], &id_res[i]);
		else
			result[i] = string2hex(tmp_res[i], &id_res[i]);
		if (!result[i])
			goto err;
	}
	*idlen_list = id_res;
	goto out;
err:
	if (id_res)
		free(id_res);

	if (result) {
		for (i = 0; i < cnt; i++)
			free(result[i]);
		free(result);
		result = NULL;
	}
out:
	if (tmp_res) {
		for (i = 0; i < cnt; i++)
			free(tmp_res[i]);
		free(tmp_res);
	}
	return result;
}

char**
anoubis_csum_list(struct anoubis_msg *m, int *listcnt)
{
	char	**result = NULL;
	char	**tmp = NULL;
	int	  cnt,
		  end,
		  msg_end,
		  name_size = 0;

	if (m == NULL || listcnt == NULL)
		goto err;

	/* Return Message from Daemon cannot return partial names */
	end = cnt = 0;
	while (m) {
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_ChecksumPayloadMessage)))
			goto err;

		msg_end = m->length - sizeof(Anoubis_ChecksumPayloadMessage)
		    - CSUM_LEN;

		/* Check if messeag is empty. Yes? We are done */
		if (msg_end == 0)
			break;

		/* This should let reach the end of the message */
		msg_end -= 1;
		if (msg_end < 1 ||
		    m->u.checksumpayload->payload[msg_end] != '\0')
			goto err;

		end = 0;
		while (end <= msg_end) {
			name_size = strlen(
			    (char *)&m->u.checksumpayload->payload[end]) + 1;
			cnt++;
			tmp = (char **)realloc(result, cnt * sizeof(char *));
			if (!tmp) {
				cnt--;
				goto err;
			}
			result = tmp;
			result[cnt-1] = calloc(name_size, sizeof(char));
			if (!result)
				goto err;
			strlcpy(result[cnt-1],
			    (char *)&m->u.checksumpayload->payload[end],
			    name_size);
			end += name_size;
		}
		m = m->next;
	}

	*listcnt = cnt;
	return result;

err:
	if (result) {
		for(end = 0; end < cnt; end++) {
			if (result[end])
				free(result[end]);
		}
		free(result);
	}

	*listcnt = -1;
	return NULL;
}

int
anoubis_print_checksum(FILE *fd, unsigned char *checksum, int len)
{
	if (len != ANOUBIS_CS_LEN)
		return EINVAL;
	return anoubis_print_signature(fd, checksum, ANOUBIS_CS_LEN);
}

int
anoubis_print_keyid(FILE *fd, unsigned char *key, int len)
{
	return anoubis_print_signature(fd, key, len);
}

int
anoubis_print_signature(FILE *fd, unsigned char *signature, int len)
{
	int i;

	if (!fd || !signature || len < 0)
		return EINVAL;

	for (i = 0; i < len; i++)
		fprintf(fd, "%2.2x", signature[i]);

	return 0;

}

int
anoubis_print_file(FILE *fd, char *name)
{
	int	len, i;

	if (!fd || !name)
		return EINVAL;

	len = strlen(name);
	for (i = 0; i < len; i++) {
		if (name[i] < 32)
			fprintf(fd, "\\%03o", name[i]);
		else if ((name[i] == ' ') || (name[i] == '\\'))
			fprintf(fd, "\\%c", name[i]);
		else
			fprintf(fd, "%c", name[i]);
	}
	return 0;
}

int
anoubis_print_entries(FILE *fd, struct sfs_entry **list, int cnt)
{
	int	i;

	if (!fd || !list || (cnt < 0))
		return EINVAL;

	for (i = 0; i < cnt; i++) {
		if (!list[i]) {
			continue;
		}
		if (!list[i]->name) {
			continue;
		}
		anoubis_print_file(fd, list[i]->name);
		fprintf(fd, " ");
		if (list[i]->checksum) {
			fprintf(fd, "%u ", list[i]->uid);
			anoubis_print_checksum(fd, list[i]->checksum,
			    ANOUBIS_CS_LEN);
			fprintf(fd, " ");
		} else {
			fprintf(fd, "- ");
		}
		if (list[i]->signature) {
			anoubis_print_keyid(fd, list[i]->keyid,
			    list[i]->keylen);
			fprintf(fd, " ");
			anoubis_print_keyid(fd, list[i]->signature,
			    list[i]->siglen);
		} else {
			fprintf(fd, "-");
		}
		fprintf(fd, "\n");
	}

	return 0;
}

struct sfs_entry *
anoubis_build_entry(const char *name, unsigned char *checksum, int csumlen,
    unsigned char *signature, int siglen, uid_t uid, unsigned char *keyid,
    int keylen)
{
	struct sfs_entry	*se = NULL;

	if (!name)
		return NULL;
	if (!checksum && !signature)
		return NULL;
	if ((se = calloc(1, sizeof(struct sfs_entry))) == NULL)
		return NULL;

	if((se->name = strdup(name)) == NULL) {
		free(se);
		return NULL;
	}
	if (checksum) {
		if (csumlen != ANOUBIS_CS_LEN) {
			free(se);
			return NULL;
		}
		if ((se->checksum = calloc(ANOUBIS_CS_LEN,
		    sizeof(unsigned char))) == NULL) {
			anoubis_entry_free(se);
			return NULL;
		}
		memcpy(se->checksum, checksum, ANOUBIS_CS_LEN);
	}
	if (signature && siglen > 0) {
		if (!keyid || keylen < 0) {
			free(se);
			return NULL;
		}
		if ((se->signature = calloc(siglen, sizeof(unsigned char)))
		    == NULL) {
			anoubis_entry_free(se);
			return NULL;
		}
		memcpy(se->signature, signature, siglen);
		if ((se->keyid = calloc(keylen, sizeof(unsigned char)))
		    == NULL) {
			anoubis_entry_free(se);
			return NULL;
		}
		memcpy(se->keyid, keyid, keylen);

	}
	se->uid = uid;
	se->siglen = siglen;
	se->keylen = keylen;
	return (se);
}

void
anoubis_entry_free(struct sfs_entry *se)
{
	if (!se)
		return;

	if (se->signature)
		free(se->signature);
	if (se->checksum)
		free(se->checksum);
	if (se->keyid)
		free(se->keyid);
	free(se);
}
