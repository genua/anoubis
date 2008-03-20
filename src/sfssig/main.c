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

#include "config.h"

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <openssl/sha.h>

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef OPENBSD

#include <attr/xattr.h>

#ifndef __dead
#define __dead __attribute__((noreturn))
#endif

void		 usage(void) __dead;

#define SYSSIGNAME "security.anoubis_syssig"

__dead void
usage(void)
{
	extern char	*__progname;

	/*
	 * NOTE: Capitalized options are supposed to affect system signatures
	 * NOTE: other options letters will be used for cert signed checksums.
	 */
	/* Add System checksum */
	fprintf(stderr, "usage: %s -A checksum file\n", __progname); 
	/* Update or add system checksum according to current file contents */
	fprintf(stderr, "       %s -U file...\n", __progname);
	/* Remove system checksum from files */
	fprintf(stderr, "       %s -R file...\n", __progname);
	/* List system checksum */
	fprintf(stderr, "       %s -L file...\n", __progname);
	exit(1);
}

int openregular(char * filename, int * error)
{
	int fd;
	struct stat statbuf;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		(*error) = errno;
		return -1;
	}
	if (fstat(fd, &statbuf) < 0) {
		(*error) = errno;
		close(fd);
		return -1;
	}
	if (!S_ISREG(statbuf.st_mode)) {
		(*error) = EINVAL;
		close(fd);
		return -1;
	}
	return fd;
}

int str2hash(const char * s, unsigned char dest[SHA256_DIGEST_LENGTH])
{
	unsigned int i;
	char t[3];

	if (strlen(s) != 2*SHA256_DIGEST_LENGTH)
		return -1;
	t[2] = 0;
	for (i=0; i<SHA256_DIGEST_LENGTH; ++i) {
		t[0] = s[2*i];
		t[1] = s[2*i+1];
		if (!isxdigit(t[0]) || !isxdigit(t[1]))
			return -1;
		dest[i] = strtoul(t, NULL, 16);
	}
	return 0;
}

/* Return  file descriptor for use with fstat later on. */
int
opensha256(char * filename, unsigned char md[SHA256_DIGEST_LENGTH],
    int * error)
{
	SHA256_CTX c;
	int fd;
	static char buf[40960];

	fd = openregular(filename, error);
	if (fd < 0)
		return -1;
	SHA256_Init(&c);
	while(1) {
		int ret = read(fd, buf, sizeof(buf));
		if (ret == 0)
			break;
		if (ret < 0) {
			(*error) = errno;
			close(fd);
			return -1;
		}
		SHA256_Update(&c, buf, ret);
	}
	SHA256_Final(md, &c);
	return fd;
}

static int syssig_add(char * file, unsigned char cksum[SHA256_DIGEST_LENGTH])
{
	unsigned char filesum[SHA256_DIGEST_LENGTH];
	int err;

	int fd = opensha256(file, filesum, &err);
	if (fd < 0) {
		errno = err;
		perror(file);
		close(fd);
		return 1;
	}
	if (memcmp(filesum, cksum, SHA256_DIGEST_LENGTH) != 0) {
		fprintf(stderr, "Warning: Signature provided with -A does "
		    "not match file\n");
	}
	if (fsetxattr(fd, SYSSIGNAME, cksum, SHA256_DIGEST_LENGTH, 0) < 0) {
		perror(file);
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}

static int syssig_list(int argc, char * argv[])
{
	int i, ret = 0;
	const char * ok;
	unsigned char csum[SHA256_DIGEST_LENGTH];
	unsigned char fcsum[SHA256_DIGEST_LENGTH];

	for (i=0; i<argc; ++i) {
		int fd, err, j;
		ok = "BAD";
		err = getxattr(argv[i], SYSSIGNAME, csum, SHA256_DIGEST_LENGTH);
		if (err < 0 && errno == ENOATTR)
			continue;
		if (err < 0) {
			perror(argv[i]);
			ret = 1;
			continue;
		}
		if (err != SHA256_DIGEST_LENGTH) {
			fprintf(stderr, "%s: Invalid system checksum\n",
			    argv[i]);
			ret = 1;
			continue;
		}
		fd = opensha256(argv[i], fcsum, &err);
		if (fd < 0) {
			ok = "??";
		} else {
			close(fd);
			if (memcmp(csum, fcsum, SHA256_DIGEST_LENGTH) == 0)
				ok = "ok";
		}
		for (j=0; j<SHA256_DIGEST_LENGTH; ++j)
			printf("%02x", csum[j]);
		printf(" %3s %s\n", ok, argv[i]);
	}
	return ret;
}

static int syssig_update(int argc, char * argv[])
{
	int i, err, ret = 0;
	unsigned char csum[SHA256_DIGEST_LENGTH];

	for (i=0; i<argc; ++i) {
		int fd = opensha256(argv[i], csum, &err);
		if (fd < 0) {
			errno = err;
			perror(argv[i]);
			ret = 1;
			continue;
		}
		if (fsetxattr(fd, SYSSIGNAME, csum,
		    SHA256_DIGEST_LENGTH, 0) < 0) {
		    	perror(argv[i]);
			ret = 1;
		}
		close(fd);
	}
	return ret;
}

static int syssig_remove(int argc, char * argv[])
{
	int i, ret = 0;
	for (i=0; i<argc; ++i) {
		int err = removexattr(argv[i], SYSSIGNAME);
		if (err < 0 && errno != ENOATTR) {
			perror(argv[i]);
			ret = 1;
		}
	}
	return ret;
}

int
main(int argc, char *argv[])
{
	char ch;
	int syssigmode = 0;
	char * argcsumstr = NULL;
	unsigned char argcsum[SHA256_DIGEST_LENGTH];

	while ((ch = getopt(argc, argv, "A:URL")) != -1) {
		switch (ch) {
		case 'A':
			argcsumstr = optarg;
		case 'U':
		case 'R':
		case 'L':
			if (syssigmode)
				usage();
			syssigmode = ch;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	if (syssigmode == 0)
		usage();
	argc -= optind;
	argv += optind;
	if (argc <= 0)
		usage();

	switch(syssigmode) {
	case 'A':
		if (argc > 1)
			usage();
		if (strlen(argcsumstr) != 2 * SHA256_DIGEST_LENGTH)
			usage();
		if (str2hash(argcsumstr, argcsum) < 0)
			usage();
		return syssig_add(argv[0], argcsum);
	case 'R':
		return syssig_remove(argc, argv);
	case 'L':
		return syssig_list(argc, argv);
	case 'U':
		return syssig_update(argc, argv);
	default:
		/* Internal error */
		assert(0);
	}
	return 1;
}
#else

int main()
{
	fprintf(stderr, "System signatures are not yet supported on OpenBSD\n");
	return 1;
}

#endif

