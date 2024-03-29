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
#include <syslog.h>

#include <sys/un.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/tree.h>

#include <sys/queue.h>

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
#include <anoubis_chat.h>

#include "anoubis_csum.h"

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

#define REQUESTS_MAX			1000

typedef int (*sumop_callback_t) (struct anoubis_csmulti_request *,
    struct anoubis_csmulti_record *);
typedef int (*filter_callback_t) (struct anoubis_csmulti_request *,
    struct anoubis_csmulti_record *);

struct sfs_request_node
{
  RB_ENTRY(sfs_request_node) entry;
  struct anoubis_csmulti_request *req;
  struct anoubis_transaction *t;
  sumop_callback_t sumop_callback;
  filter_callback_t filter_callback;
  unsigned long idx;
  char *key;
  int op;
};

struct sfs_request_tree
{
  RB_HEAD(rb_request_tree, sfs_request_node) head;
};

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

/**
 * This adds a checksum request to a request-node of the request-tree.
 * If no request-node is found for this uid/keyid a new one will be
 * created and add to the request tree.
 *
 * @param file The target file name.
 * @param op The operation which is to be performed on the target file
 * @param keyid keyid of the certificate used for this transaction
 * @param idlen Is the length of the keyid stored in keyid.
 * @param uid of the user in which name the operation on the file should be
 *        done.
 * @param cs checksum buffer which can hold keyid and/or checksum and signature.
 * @param cslen Is the length of the complete buffer.
 * @param callback function of the related operation, which is to be
 *        called after receiving the answer of the request.
 * @return 0 on success and a negative errno in case of error.
 */
int sfs_csumop(const char *file, int op, uid_t uid, u_int8_t *keyid, int idlen,
    u_int8_t *cs, int cslen, sumop_callback_t callback);

/**
 * Initialize the Red-Black Request-Tree which holds anoubis_csmulti_requests.
 *
 * @return An empty sfs_request_tree or NULL in case of an error.
 */
struct sfs_request_tree *sfs_init_request_tree(void);

/**
 * Creates a new anoubis_csmulti_requests and puts it into the request tree.
 * The key to search the request can be either the keyid or the uid.
 *
 * @param tree which holds the requests.
 * @param op is the operation which the request should perform
 * @param uid of the user in which name the operation on the file should be
 *        done.
 * @param keyid of the user in which name the operation on the file should be
 *        done.
 * @param idlen length of the keyid
 * @param sumop-callback function of the related operation, which is to be
 *        called after receiving the answer of the sumop request.
 * @param filter-callback function of the related operation, which is to be
 *        called after receiving the answer of the filter request.
 * @return the resulting node or NULL in case of an error.
 */
struct sfs_request_node *sfs_insert_request_node(struct sfs_request_tree
    *tree, int op, uid_t uid, u_int8_t * keyid, int idlen,
    sumop_callback_t scb, filter_callback_t fcb);

/**
 * Find a request inside of the tree.
 *
 * @param tree which should be searched in.
 * @param uid which identifies the desired request, if the request is based
 *        on a uid otherwise 0.
 * @param key which identifies the desired request, if the request is based
 *        on a keyid otherwise NULL.
 * @param idlen is the length of the keyid. The parameter must be 0 if the
 *        request is based on uid.
 * @param operation of the desired node
 * @return The desired node or NULL if no matched node could be found.
 */
struct sfs_request_node *sfs_find_request(struct sfs_request_tree *t,
    uid_t uid, u_int8_t * keyid, int, int);

/**
 * Destroys a request tree
 *
 * @param the tree that should be destroyed.
 * @return Nothing
 */
void sfs_destroy_request_tree(struct sfs_request_tree *t);

/**
 * Deletes a node from the tree.
 *
 * @param tree which holds the node.
 * @param node which should be deleted.
 * @return Nothing.
 */
void sfs_delete_request(struct sfs_request_tree *t,
    struct sfs_request_node *n);

/**
 * Return the first entry of the tree which can be used to iterated over
 * the tree.
 *
 * @param tree which should be iterated over.
 * @return node which is the first node of the tree.
 */
struct sfs_request_node *sfs_first_request(struct sfs_request_tree *);

/**
 * Return the next node of the tree relative to the node of the argument.
 *
 * @param tree which should be iterated over.
 * @param node before the desired node.
 * @return node after the node as parameter.
 */
struct sfs_request_node *sfs_next_request(struct sfs_request_tree *,
    struct sfs_request_node *);

/**
 * Callback function which handles a filter request for the hassum
 * filter. It will get an request and record to find out if a the
 * file got a checksum registered.
 *
 * @param anoubis_csumlti_request structure which which is related to the
 *	  record
 * @param anoubis_csumlti_record structure which contains the result
 *	  for a file.
 * @return 1 to keep the file
 *	   0 to delete the file form the results
 */
int filter_hassum_callback(struct anoubis_csmulti_request *,
    struct anoubis_csmulti_record *);

/**
 * Callback function which handles a filter request for the hasnosum
 * filter. It will get an request and record to find out if a the
 * file got no checksum registered.
 *
 * @param anoubis_csumlti_request structure which which is related to the
 *	  record
 * @param anoubis_csumlti_record structure which contains the result
 *	  for a file.
 * @return 1 to keep the file
 *	   0 to delete the file form the results
 */
int filter_hasnosum_callback(struct anoubis_csmulti_request *,
    struct anoubis_csmulti_record *);

/**
 * Callback function which handles a filter request for the hassig
 * filter. It will get an request and record to find out if a the
 * file got a signature registered.
 *
 * @param anoubis_csumlti_request structure which which is related to the
 *	  record
 * @param anoubis_csumlti_record structure which contains the result
 *	  for a file.
 * @return 1 to keep the file
 *	   0 to delete the file form the results
 */
int filter_hassig_callback(struct anoubis_csmulti_request *,
    struct anoubis_csmulti_record *);

/**
 * Callback function which handles a filter request for the hasnosig
 * filter. It will get an request and record to find out if a the
 * file got no signature registered.
 *
 * @param anoubis_csumlti_request structure which which is related to the
 *	  record
 * @param anoubis_csumlti_record structure which contains the result
 *	  for a file.
 * @return 1 to keep the file
 *	   0 to delete the file form the results
 */
int filter_hasnosig_callback(struct anoubis_csmulti_request *,
    struct anoubis_csmulti_record *);

/**
 * Callback function which handles a filter request for the upgraded
 * filter. It will get an request and record to find out if a the
 * file got a signature registered AND was upgraded.
 *
 * @param anoubis_csumlti_request structure which which is related to the
 *	  record
 * @param anoubis_csumlti_record structure which contains the result
 *	  for a file.
 * @return 1 to keep the file
 *	   0 to delete the file form the results
 */
int filter_upgraded_callback(struct anoubis_csmulti_request *,
    struct anoubis_csmulti_record *);

/**
 * Add a file to the filter request. If the filter can be done now
 * (e.g orphan/not file) it will return the expected result.
 *
 * @param file/path which should be filtered
 * @param path to the file if 1. parameter is just a file otherwise NULL
 * @param uid which registered a checksum
 * @param anoubis_sig structure holding keyid for the registered signature
 * @return 1 to keep a file
 *	   0 if file can already be deleted from file list
 */
int filter_a_file(char *, char *, uid_t, struct anoubis_sig *);

extern struct sfs_request_tree *filter_tree;
extern unsigned int opts;

#endif /* _SFSSIG_H_ */
