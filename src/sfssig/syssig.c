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

#ifndef OPENBSD
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

int
syssig_add(char * file, unsigned char cksum[SHA256_DIGEST_LENGTH])
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

int
syssig_list(int argc, char * argv[])
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

int
syssig_update(int argc, char * argv[])
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

int
syssig_remove(int argc, char * argv[])
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
skipsum_update(char * file, int op)
{
	int ret = 0;

	switch (op) {
		case 0:
			if ((removexattr(file, SKIPSUMNAME) < 0) &&
			    (errno != ENODATA && errno != ENOATTR)) {
				perror(file);
				ret = 1;
			} else
				printf("skipsum attribute cleared\n");
			break;
		case 1:
			if (setxattr(file, SKIPSUMNAME, "1", 1, 0) < 0) {
				perror(file);
				ret = 1;
			} else
				printf("skipsum attribute added\n");
			break;
	}

	return ret;
}

#else

int
syssig_remove(int argc __used, char * argv[] __used)
{
	fprintf(stderr, "Not implemented in OpenBSD yet\n");
	return 1;
}

int
syssig_update(int argc __used, char * argv[] __used)
{
	fprintf(stderr, "Not implemented in OpenBSD yet\n");
	return 1;
}

int
syssig_list(int argc __used, char * argv[] __used)
{
	fprintf(stderr, "Not implemented in OpenBSD yet\n");
	return 1;
}

int
syssig_add(char * file __used, unsigned char cksum[SHA256_DIGEST_LENGTH] __used)
{
	fprintf(stderr, "Not implemented in OpenBSD yet\n");
	return 1;
}

int
skipsum_update(char * file __used, int op __used)
{
	fprintf(stderr, "Not implemented in OpenBSD yet\n");
	return 1;
}
#endif /* ifndef OPENBSD */
