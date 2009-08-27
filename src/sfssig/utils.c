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

int
str2hash(const char * s, unsigned char dest[SHA256_DIGEST_LENGTH])
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

char**
file_input(int *file_cnt, char *file)
{
	FILE	 *fp = NULL;
	char	**args = NULL;
	char	**tmp = NULL;
	char	  fileinput[PATH_MAX];
	int	  cnt, i, end;

	if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "Input from file handler\n");
	if (!file_cnt || !file)
		return NULL;

	cnt = 0;
	i = 0;

	if (strcmp(file, "-") == 0) {
		fp = stdin;
	} else {
		fp = fopen(file, "r");
		if (!fp) {
			perror(file);
			return NULL;
		}
	}

	while (fgets(fileinput, PATH_MAX, fp) != NULL) {
		cnt++;
		tmp = realloc(args, cnt * sizeof(char *));
		if (!tmp) {
			cnt--;
			for (i = 0; i < cnt; i++)
				free(args[i]);
			free(args);
			perror(file);
			fclose(fp);
			return NULL;
		}
		args = tmp;
		args[cnt-1] = strdup(fileinput);
		if (!args[cnt-1]) {
			for (i = 0; i < cnt; i++)
				free(args[i]);
			free(args);
			fclose(fp);
			return NULL;
		}
		end = strlen(args[cnt-1]) - 1;
		if (args[cnt-1][end] == '\n')
			args[cnt-1][end] = '\0';
	}
	fclose(fp);

	*file_cnt = cnt;
	return args;
}

/* We need our own Version of realpath since we need to
 * distinguish between filesystem and shadowtree.
 * The usual realpath would not resolv a file
 * which is not in the filesystem anymore.
 */
char *
sfs_realpath(const char *path, char resolved[PATH_MAX])
{
	struct stat sb;
	char *p, *q, *s;
	size_t left_len, resolved_len;
	unsigned symlinks;
	int serrno, slen, lstat_fail = 0;
	char left[PATH_MAX], next_token[PATH_MAX], symlink[PATH_MAX];

	serrno = errno;
	symlinks = 0;
	if (path[0] == '/') {
		resolved[0] = '/';
		resolved[1] = '\0';
		if (path[1] == '\0')
			return (resolved);
		resolved_len = 1;
		left_len = strlcpy(left, path + 1, sizeof(left));
	} else {
		if (getcwd(resolved, PATH_MAX) == NULL) {
			strlcpy(resolved, ".", PATH_MAX);
			return (NULL);
		}
		resolved_len = strlen(resolved);
		left_len = strlcpy(left, path, sizeof(left));
	}
	if (left_len >= sizeof(left) || resolved_len >= PATH_MAX) {
		errno = ENAMETOOLONG;
		return (NULL);
	}

	/*
	 * Iterate over path components in `left'.
	 */
	while (left_len != 0) {
		/*
		 * Extract the next path component and adjust `left'
		 * and its length.
		 */
		p = strchr(left, '/');
		s = p ? p : left + left_len;
		if ((unsigned)(s - left) >= sizeof(next_token)) {
			errno = ENAMETOOLONG;
			return (NULL);
		}
		memcpy(next_token, left, s - left);
		next_token[s - left] = '\0';
		left_len -= s - left;
		if (p != NULL)
			memmove(left, s + 1, left_len + 1);
		if (resolved[resolved_len - 1] != '/') {
			if (resolved_len + 1 >= PATH_MAX) {
				errno = ENAMETOOLONG;
				return (NULL);
			}
			resolved[resolved_len++] = '/';
			resolved[resolved_len] = '\0';
		}
		if (next_token[0] == '\0')
			continue;
		else if (strcmp(next_token, ".") == 0)
			continue;
		else if (strcmp(next_token, "..") == 0) {
			/*
			 * Strip the last path component except when we have
			 * single "/"
			 */
			if (resolved_len > 1) {
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}
			continue;
		}

		/*
		 * Append the next path component and lstat() it. If
		 * lstat() fails we still can return successfully if
		 * there are no more path components left.
		 */
		resolved_len = strlcat(resolved, next_token, PATH_MAX);
		if (resolved_len >= PATH_MAX) {
			errno = ENAMETOOLONG;
			return (NULL);
		}

		/* If lstat failed once then the resolving path doesn't
		 * exisit in the filesystem, but probably in the
		 * shadowtree. So we don't lstat anymore.
		 */
		if (lstat_fail)
			continue;

		if (lstat(resolved, &sb) != 0) {
			lstat_fail++;
			continue;
		}
		/* If lstat is succsessful and its a link we should
		 * resolving it.
		 */
		if (S_ISLNK(sb.st_mode)) {
			if (symlinks++ > MAXSYMLINKS) {
				errno = ELOOP;
				return (NULL);
			}
			slen = readlink(resolved, symlink, sizeof(symlink) - 1);
			if (slen < 0)
				return (NULL);
			symlink[slen] = '\0';
			if (symlink[0] == '/') {
				resolved[1] = 0;
				resolved_len = 1;
			} else if (resolved_len > 1) {
				/* Strip the last path component. */
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}

			/*
			 * If there are any path components left, then
			 * append them to symlink. The result is placed
			 * in `left'.
			 */
			if (p != NULL) {
				if (symlink[slen - 1] != '/') {
					if ((unsigned)(slen + 1) >=
					    sizeof(symlink)) {
						errno = ENAMETOOLONG;
						return (NULL);
					}
					symlink[slen] = '/';
					symlink[slen + 1] = 0;
				}
				left_len = strlcat(symlink, left, sizeof(left));
				if (left_len >= sizeof(left)) {
					errno = ENAMETOOLONG;
					return (NULL);
				}
			}
			left_len = strlcpy(left, symlink, sizeof(left));
		}
	}

	/*
	 * Remove trailing slash except when the resolved pathname
	 * is a single "/".
	 */
	if (resolved_len > 1 && resolved[resolved_len - 1] == '/')
		resolved[resolved_len - 1] = '\0';
	return (resolved);
}

char *
build_path(const char *path, const char *file)
{
	char	*res = NULL;
	int	 ret = 0,
		 len = 0;

	if (!path || !file)
		return NULL;
	len = strlen(path);

	if (path[len-1] == '/')
		ret = asprintf(&res, "%s%s", path, file);
	else
		ret = asprintf(&res, "%s/%s", path, file);
	if (ret < 0) {
		perror(path);
		return NULL;
	}

	len = strlen(res);
	if ((len > 1) && res[len - 1] == '/')
		res[len - 1] = '\0';
	return res;
}

struct anoubis_sig *
load_keys(int priv_key, int show_err, char *cert, char *keyfile)
{
	char			*homepath = NULL;
	int			 error = 0;
	struct stat		 sb;
	struct anoubis_sig	*ast = NULL;

	if (cert == NULL) {
		homepath = getenv("HOME");
		if ((cert = build_path(homepath, ANOUBIS_DEF_CRT)) == NULL) {
			fprintf(stderr, "Error while allocating"
			    "memory\n");
		}
	}

	if (stat(cert, &sb) != 0) {
		if (show_err)
			fprintf(stderr, "Could not find certficate: %s\n",
			    cert);
		return NULL;
	}
	/* You also need a keyfile if you want to add a signature */
	if (priv_key == 1) {
		if (keyfile == NULL) {
			homepath = getenv("HOME");
			if ((keyfile = build_path(homepath, ANOUBIS_DEF_KEY))
			    == NULL) {
				fprintf(stderr, "Error while allocating"
				    "memory\n");
			}
		}
		if (stat(keyfile, &sb) != 0) {
			if (show_err)
				fprintf(stderr, "You have to specify a keyfile:"
				    " %s\n", keyfile);
			return NULL;
		}
	}

	ast = anoubis_sig_priv_init(keyfile, cert, pass_cb, &error);
	if (ast == NULL) {
		if (show_err)
			fprintf(stderr, "Error while loading keyfile/certfile "
			    "%s\n", strerror(error));
		return NULL;
	}
	return ast;
}
