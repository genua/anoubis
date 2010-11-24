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
#ifndef ANOUBIS_CLIENT_H
#define ANOUBIS_CLIENT_H

#include <anoubis_chat.h>
#include <anoubis_protocol.h>

#include <sys/queue.h>

struct anoubis_client;
struct anoubis_msg;
struct anoubis_transaction;

struct anoubis_csmulti_record {
	TAILQ_ENTRY(anoubis_csmulti_record)		 next;
	char						*path;
	unsigned int					 error;
	unsigned int					 idx;
	unsigned int					 length;
	union {
		struct __csum_get {
			struct anoubis_csentry		*csum;
			struct anoubis_csentry		*sig;
			struct anoubis_csentry		*upgrade;
		} get;
		struct __csum_add {
			unsigned int			 cslen;
			void				*csdata;
		} add;
	} u;
};

struct anoubis_csmulti_request {
	unsigned int					 op;
	unsigned int					 nreqs;
	unsigned int					 openreqs;
	uid_t						 uid;
	void						*keyid;
	int						 idlen;
	struct anoubis_msg				*reply_msg;
	struct anoubis_client				*client;
	struct anoubis_csmulti_record			*last;
	TAILQ_HEAD(, anoubis_csmulti_record)		 reqs;
};

typedef int (*anoubis_client_auth_callback_t)(struct anoubis_client *,
    const struct anoubis_msg *, struct anoubis_msg **);

__BEGIN_DECLS

struct anoubis_msg * anoubis_client_getnotify(struct anoubis_client * client);
struct anoubis_client * anoubis_client_create(
    struct achat_channel * chan, int authtype, anoubis_client_auth_callback_t);
void anoubis_client_destroy(struct anoubis_client *);
int anoubis_client_connect(struct anoubis_client *, unsigned int);
int anoubis_client_connect_old(struct anoubis_client *, unsigned int, int);
struct anoubis_transaction * anoubis_client_connect_start(
    struct anoubis_client *, unsigned int);
int anoubis_client_serverversion(struct anoubis_client *);
int anoubis_client_serverminversion(struct anoubis_client *);
int anoubis_client_selectedversion(struct anoubis_client *);
void anoubis_client_close(struct anoubis_client *);
struct anoubis_transaction * anoubis_client_close_start(
    struct anoubis_client *);
struct anoubis_transaction * anoubis_client_policyrequest_start(
    struct anoubis_client *, void * data, size_t datalen);
int anoubis_client_process(struct anoubis_client *,
    struct anoubis_msg *);
struct anoubis_transaction * anoubis_client_register_start(
    struct anoubis_client * client, anoubis_token_t token, uid_t uid,
    u_int32_t rule_id, u_int32_t subsystem);
struct anoubis_transaction * anoubis_client_unregister_start(
    struct anoubis_client * client, anoubis_token_t token, uid_t uid,
    u_int32_t rule_id, u_int32_t subsystem);
struct anoubis_msg * anoubis_client_getnotify(struct anoubis_client * client);
int anoubis_client_notifyreply(struct anoubis_client * client,
    anoubis_token_t, int error, int delegate);
int anoubis_client_hasnotifies(struct anoubis_client * client);
int anoubis_client_wait(struct anoubis_client * client);
struct anoubis_transaction *anoubis_client_csumrequest_start(
    struct anoubis_client *client, int op, const char *path, u_int8_t *,
    short, short, uid_t, unsigned int flags);
struct anoubis_transaction *anoubis_client_passphrase_start(
    struct anoubis_client *client, const char *passphrase);
struct anoubis_transaction *anoubis_client_version_start(
    struct anoubis_client *client);

/**
 * Create and initialize an anoubis_csmulti_request structure.
 * This structure provides an easy to use interface to the multi
 * request protocol messages.
 *
 * @param op The operation to perfom. Possible values are
 *     ANOUBIS_CHECKSUM_OP_{ADDSUM,ADDSIG,GET2,GETSIG2,DEL,DELSIG}
 * @param uid The user ID that all the requests apply to. This value
 *     must be zero if keyid or idlen is non-zero.
 * @param keyid The key ID that all the requests apply to.
 * @param idlen The length of the key ID.
 * @return A pointer to a new anoubis_csmulti_request structure or NULL
 *     in case of errors.
 */
struct anoubis_csmulti_request *
anoubis_csmulti_create(unsigned int op, uid_t uid, void *keyid, int idlen);

/**
 * Destroy an anoubis_csmulti_request structure previously allocated
 * by anoubis_csmulti_create. This function frees all the memory
 * associated with the request.
 *
 * @param request The request.
 */
void
anoubis_csmulti_destroy(struct anoubis_csmulti_request *request);

/**
 * Add an individual checksum requst to an anoubis_csmulti_request.
 *
 * @param request The anoubis_csmulti_request.
 * @param path The pathname of the checksum request.
 * @param csdata The checksum data. This value must be NULL if the
 *     operation of the request is not an ADD request.
 * @param cslen The length of the checksum data. This value must be zero
 *     if the operation of the request is not an ADD request.
 * @return Zero in case of success, a negative error code in case of
 *     errors.
 */
int
anoubis_csmulti_add(struct anoubis_csmulti_request *request,
    const char *path, const void *csdata, unsigned int cslen);

/**
 * Add an individual entry to the request with a pre-set error code.
 *
 * @param request The request.
 * @param path The path for the entry.
 * @param error The error number to use.
 * @return  Zero in case of success, a negative error code (probably ENOMEM)
 *     in case of errors.
 */
int
anoubis_csmulti_add_error(struct anoubis_csmulti_request *request,
    const char *path, int error);

/**
 * Find the request record with a given index in a csmulti request.
 *
 * @param req The csmulti request.
 * @param idx The index of the record.
 * @return A pointer to the record or NULL if the index was not found.
 */
struct anoubis_csmulti_record *
anoubis_csmulti_find(struct anoubis_csmulti_request *req,
    unsigned int idx);

/**
 * Create the csmulti message that actually starts an anoubis_csmulti_request.
 * This function is mainly exported for testing. You normally want to use
 * anoubis_client_csmulti_start instead.
 *
 * @param request The request.
 * @return An anoubis_msg for the request.
 */
struct anoubis_msg *
anoubis_csmulti_msg(struct anoubis_csmulti_request *request);

/**
 * Actually start a multi checksum request. The request data is
 * taken from the anoubis_csmulti_request structure. This structure
 * is owned by the transaction and must not be freed before the transaction
 * is complete.
 *
 * @param client The protocol client object for the request.
 * @param request The anoubis_csmulti_request structure.
 * @return A transaction or NULL if an error occured.
 */
struct anoubis_transaction *
anoubis_client_csmulti_start(struct anoubis_client *client,
    struct anoubis_csmulti_request *request);

/**
 * Request a list from the daemon. Possible lists are a list of all
 * playgrounds, a list of all files in a particular playground or a
 * list of all processes of a user.
 *
 * @param client The procotol client object for the request.
 * @param listtype The type of list to request. (ANOUBIS_REC_*)
 * @param arg This argument restricts the elements to list. For playground
 *     related list this is a playground ID for process lists it is a
 *     user ID.
 * @return A transaction for the request or NULL if an error occured.
 */
struct anoubis_transaction *anoubis_client_list_start(
    struct anoubis_client *client, uint32_t listtype, uint64_t arg);

/**
 * Start a transaction that request that a file is committed.
 *
 * @param client The protocol client object for the request.
 * @param pgid The playground ID of the file.
 * @param path The full path name of the file.
 * @param ignore_recommended_scanners A flag that can be set to ignore
 *        content scanners marked as recommended for this commit request
 * @return A transaction or NULL if an error occured.
 */
struct anoubis_transaction *anoubis_client_pgcommit_start(
    struct anoubis_client *client, uint64_t pgid, const char *path,
    uint8_t ignore_recommended_scanners);

/**
 * Start a transaction that request an unlink-operation.
 *
 * @param client The protocol client object for the request.
 * @param pgid The playground ID of the file.
 * @param dev The device ID of the file.
 * @param ino The inode of the file.
 */
struct anoubis_transaction *anoubis_client_pgunlink_start(
    struct anoubis_client *client, uint64_t pgid, uint64_t dev, uint64_t ino);

/**
 * Decompose the strings contained in the reply message to a commit request
 * into a list of individual strings.
 *
 * The reply message contains pairs of NULL terminated strings. The first
 * string of each pair is the scanner's description, the second string is
 * the scanner output.
 *
 * @param m The (single) reply message.
 * @return A malloced array of pointers that point the the strings in the
 *     message. The caller should free the pointer array but must not free
 *     the strings as these point into the message. The list of pointers
 *     is termianted by a NULL pointer. There is no guarantee that the
 *     number of pointers in the list is even!
 */
const char **anoubis_client_parse_pgcommit_reply(struct anoubis_msg *m);

__END_DECLS

#endif
