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
#include <anoubis_errno.h>

#define FT_HASH_SHIFT   (24)
#define FT_HASH_MASK    ((1UL<<FT_HASH_SHIFT)-1)

static int hashcmp(struct sfs_request_node *, struct sfs_request_node *);
static unsigned long hash_fn(const char *data, int cnt);

RB_PROTOTYPE_STATIC(rb_request_tree, sfs_request_node, entry, hashcmp);
RB_GENERATE_STATIC(rb_request_tree, sfs_request_node, entry, hashcmp);

static int
hashcmp(struct sfs_request_node *e1, struct sfs_request_node *e2)
{
	if (e1->idx == e2->idx)
		return strcmp(e1->key, e2->key);
	return (e1->idx - e2->idx);
}

static unsigned long
hash_fn(const char *data, int cnt)
{
	unsigned long   ret = 0;
	int	     i;

	if (data == NULL)
		return 0;

	for (i=0; i<cnt; ++i,++data) {
		ret <<= 1;
		ret ^= *data;
		ret = (ret ^ (ret >> FT_HASH_SHIFT)) & FT_HASH_MASK;
	}
	return ret & FT_HASH_MASK;
}

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
	int slen, lstat_fail = 0;
	char left[PATH_MAX], next_token[PATH_MAX], symlink[PATH_MAX];

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
			return NULL;
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
				return NULL;
			}
		}
		if (stat(keyfile, &sb) != 0) {
			if (show_err)
				fprintf(stderr, "You have to specify a keyfile:"
				    " %s\n", keyfile);
			return NULL;
		}
	}

	error = anoubis_sig_create(&ast, priv_key ? keyfile : NULL, cert,
	    pass_cb);
	if (error < 0) {
		if (show_err)
			fprintf(stderr, "Error while loading keyfile/certfile "
			    "%s\n", anoubis_strerror(-error));
		return NULL;
	}
	return ast;
}

struct sfs_request_tree *
sfs_init_request_tree(void)
{
	struct sfs_request_tree *tree;

	tree = malloc(sizeof(*tree));
	if (tree == NULL)
		return NULL;
	RB_INIT(&tree->head);
	return tree;
}

struct sfs_request_node  *
sfs_insert_request_node(struct sfs_request_tree *t, int op, uid_t uid,
    u_int8_t *keyid, int idlen, sumop_callback_t callback)
{
	struct sfs_request_node *n = NULL, *other = NULL;
	struct anoubis_csmulti_request *req = NULL;

	if (t == NULL)
		return NULL;

	n = malloc(sizeof(struct sfs_request_node));
	if (n == NULL)
		return NULL;

	if (idlen == 0)
		keyid = NULL;

	req = anoubis_csmulti_create(op, uid, keyid, idlen);
	if (req == NULL) {
		free(n);
		return NULL;
	}

	if (idlen) {
		n->key = anoubis_sig_key2char(idlen, keyid);
		if (n->key == NULL) {
			free(n);
			anoubis_csmulti_destroy(req);
			return NULL;
		}
	} else {
		if (asprintf(&n->key, "%u", uid) < 0) {
			free(n);
			anoubis_csmulti_destroy(req);
			return NULL;
		}
	}
	n->idx = hash_fn(n->key, strlen(n->key));
	n->req = req;
	n->t = NULL;
	n->callback = callback;

	other = RB_INSERT(rb_request_tree, &t->head, n);
	if (other) {
		anoubis_csmulti_destroy(n->req);
		free(n->key);
		free(n);
		return NULL;
	}

	return n;
}

struct sfs_request_node *
sfs_find_request(struct sfs_request_tree *t, uid_t uid, u_int8_t *keyid,
    int idlen)
{
	struct sfs_request_node n, *res = NULL;
	char *key = NULL;

	if (t == NULL)
		return NULL;

	if (idlen) {
		if((key = anoubis_sig_key2char(idlen, keyid)) == NULL)
			return NULL;
	} else {
		if (asprintf(&key, "%u", uid) < 0)
			return NULL;
	}

	n.key = key;
	n.idx = hash_fn(key, strlen(key));
	res = RB_FIND(rb_request_tree, &t->head, &n);
	return res;
}

void
sfs_delete_request(struct sfs_request_tree *t, struct sfs_request_node *n)
{
	if (t == NULL || n == NULL)
		return;
	if (RB_REMOVE(rb_request_tree, &t->head, n) == NULL)
		return;
	free(n->key);
	free(n);
}

void
sfs_destroy_request_tree(struct sfs_request_tree *t)
{
	struct sfs_request_node *var, *nxt;

	if (t == NULL)
		return;
	for (var = RB_MIN(rb_request_tree, &t->head); var != NULL; var = nxt) {
		nxt = RB_NEXT(rb_request_tree, &t->head, var);
		RB_REMOVE(rb_request_tree, &t->head, var);
		free(var->key);
		free(var);
	}
	free(t);

}

/* gets a first node in the tree. this can be used to
 * initlize a iterator:
 * for (np = sfs_first_request(f); np != NULL; np = sfs_next_request(f, np)) {
 *      ...
 * }
 */
struct sfs_request_node *
sfs_first_request(struct sfs_request_tree *f)
{
	if (f == NULL)
		return NULL;
	return RB_MIN(rb_request_tree, &f->head);
}

/* gets the next node in the tree from the 'last node'
 * it will return NULL if the 'last node' was the last node
 */
struct sfs_request_node *
sfs_next_request(struct sfs_request_tree *f, struct sfs_request_node *last)
{
	if (f == NULL || last == NULL)
		return NULL;
	return RB_NEXT(rb_request_tree, &f->head, last);
}
