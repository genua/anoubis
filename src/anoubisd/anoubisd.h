/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#ifndef _ANOUBISD_H
#define _ANOUBISD_H

#include "config.h"

#include <sys/file.h>
#include <sys/types.h>
#include <sys/param.h>
#include <grp.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#include <linux/anoubis_sfs.h>
#include <linux/anoubis_playground.h>
#endif
#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#include <dev/anoubis_sfs.h>
#endif

#include <assert.h>

#include <sys/queue.h>
#include <aqueue.h>

#ifdef LINUX

/**
 * Minimal required anoubis version in the kernel. Anoubis Daemon will
 * abort if the version is smaller than this.
 */
#define ANOUBISCORE_MIN_VERSION	0x00010002UL

#elif defined OPENBSD

#define ANOUBISCORE_MIN_VERSION	0x00010001UL

#else

#define ANOUBISCORE_MIN_VERSION ANOUBISCORE_VERSION

#endif

/**
 * First anoubis core version that support LOCK messages, i.e. upgrade
 * tracking.
 */
#define ANOUBISCORE_LOCK_VERSION 0x00010004UL

/**
 * First anoubis core version that supports the playground.
 */
#define ANOUBISCORE_PG_VERSION 0x00010005UL

#include <anoubis_protocol.h>

/**
 * Default value for the maximum size of a policy file. Huge policies
 * of a user can slow down the anoubis daemon.
 */
#define ANOUBISD_MAX_POLICYSIZE		0x1400000

/**
 * Maximum number of client connections per user.
 */
#define ANOUBISD_MAX_CONNS_PER_USER	20

/**
 * Maximum total number of pending unanswered events.
 */
#define ANOUBISD_MAX_PENDNG_EVENTS	1000

/**
 * The name of the anoubisd user for the policy and the session engine.
 */
#define ANOUBISD_USER			"_anoubisd"

/**
 * The name of the anoubisd user for scanner sub-processes.
 */
#define SCAN_USER			"_anoubisscan"

/**
 * The subdirectory where the policies are stored within the chroot
 * environment of the policy engine.
 */
#define ANOUBISD_POLICYCHROOT		"/policy"

/**
 * The subdirectory of the policy directory where the user policies are
 * stored.
 */
#define ANOUBISD_USERDIR		"user"

/**
 * The subdirectory of the policy directory where the admin policies are
 * stored.
 */
#define ANOUBISD_ADMINDIR		"admin"

/**
 * The subdirectory of the policy directory where the public keys are
 * stored.
 */
#define ANOUBISD_PUBKEYDIR		"pubkeys"

/**
 * The name of the default policy file on disk.
 */
#define ANOUBISD_DEFAULTNAME		"default"

/**
 * The directory where the sfs tree is stored (system global value).
 */
#define SFS_CHECKSUMROOT		PACKAGE_POLICYDIR "/sfs"

/**
 * The directory where the sfs tree is stored relative to the chroot
 * environment in the policy engine.
 */
#define SFS_CHECKSUMCHROOT		"/sfs"

/**
 * The directory where the public keys/certificates are stored (system
 * global value).
 */
#define CERT_DIR			PACKAGE_POLICYDIR CERT_DIR_CHROOT

/**
 * The directory where the public keys/ceritificates are stored relative
 * to the chroot environment in the policy engine.
 */
#define CERT_DIR_CHROOT			ANOUBISD_POLICYCHROOT "/pubkeys"

/**
 * The directory where the persistent playgroud  information is stored
 * (system global value).
 */
#define ANOUBISD_PG			PACKAGE_POLICYDIR ANOUBISD_PGCHROOT

/**
 * The directory where the persistent playgroud  information is stored
 * relative to the chroot environment in the policy engine.
 */
#define ANOUBISD_PGCHROOT		"/playground"


/**
 * The version file of the sfs tree (system global value). This file is
 * outside the SFS_CHECKSUMROOT dir to avoid filename collisions with
 * checksum and/or signature files.
 */
#define ANOUBISD_SFS_TREE_VERSIONFILE		PACKAGE_POLICYDIR "/sfs.version"

/**
 * The version file of the sfs tree relative to the chroot environment in
 * the policy engine.
 */
#define ANOUBISD_SFS_TREE_VERSIONFILE_CHROOT	"/sfs.version"

/**
 * The current version of the sfs tree file format.
 */
#define ANOUBISD_SFS_TREE_FORMAT_VERSION	1

/**
 * Macro to suppress warnings about unused function arguments.
 */
#define __used __attribute__((unused))

#ifndef __dead

/**
 * Macro to tell the compiler that a function never returns.
 */
#define __dead __attribute__((__noreturn__))

#endif

#ifdef LINUX
#define UID_MAX	UINT_MAX
#endif

/**
 * Constants for the upgrade mode.
 */
typedef enum
{
	ANOUBISD_UPGRADE_MODE_OFF,
	ANOUBISD_UPGRADE_MODE_STRICT_LOCK,
	ANOUBISD_UPGRADE_MODE_LOOSE_LOCK,
	ANOUBISD_UPGRADE_MODE_PROCESS
} anoubisd_upgrade_mode;

/**
 * Constants for the authentication mode.
 */
typedef enum
{
	ANOUBISD_AUTH_MODE_ENABLED,
	ANOUBISD_AUTH_MODE_OPTIONAL,
	ANOUBISD_AUTH_MODE_OFF
} anoubisd_auth_mode;

/**
 * Declaration of the upgrade trigger list.
 */
LIST_HEAD(anoubisd_upgrade_trigger_list, anoubisd_upgrade_trigger);

/**
 * An entry in the upgrade trigger list.
 */
struct anoubisd_upgrade_trigger {
	/**
	 * A file name that acts as an upgrade trigger.
	 */
	char					*arg;

	/**
	 * Used to link upgrade trigger structures in the upgrade trigger
	 * list.
	 */
	LIST_ENTRY(anoubisd_upgrade_trigger)	 entries;
};

/**
 * Declaration of the playground scanner list.
 */
CIRCLEQ_HEAD(anoubisd_pg_scanner_list, anoubisd_pg_scanner);

/**
 * An entry in the playground scanner list.
 */
struct anoubisd_pg_scanner {
	/**
	 * True if this scanner is required, false if it is reommended.
	 */
	int					 required;

	/**
	 * The path  of the scanner binary.
	 */
	char					*path;

	/**
	 * Human readable description of the scanner.
	 */
	char					*description;

	/**
	 * Used to link individual scanners in the global playground
	 * scanner list.
	 */
	CIRCLEQ_ENTRY(anoubisd_pg_scanner)	 link;
};

/**
 * The global anoubis daemon configuration. Some aspects of the command
 * line configuration are not stored in this structure.
 */
struct anoubisd_config {
	/**
	 * The unix domain socket for client connections. Defaults to
	 * ANOUBISD_SOCKETNAME.
	 */
	char					*unixsocket;

	/**
	 * The path to root's private key for automatic update of signature
	 * during upgrade. See the manual page for the security risks
	 * implied by this option.
	 */
	char					*rootkey;

	/**
	 * True if a configured root key must be available during system
	 * upgrade. If true system upgrades will be denied if root's private
	 * key is not configured or not usable.
	 */
	int					 rootkey_required;

	/**
	 * The upgrade mode on the system. This determines how we detect
	 * start end end of a system upgrade.
	 */
	anoubisd_upgrade_mode			 upgrade_mode;

	/**
	 * This is a list of lock files that trigger upgrade tracking
	 * when locked.
	 */
	struct anoubisd_upgrade_trigger_list	 upgrade_trigger;

	/**
	 * The current authentication mode.
	 */
	anoubisd_auth_mode			 auth_mode;

	/**
	 * Allow anoubis daemon core dumps for debugging purposes.
	 */
	int					 allow_coredumps;

	/**
	 * Maximum size of a policy file.
	 */
	int					 policysize;

	/**
	 * The global list of configured playground scanners.
	 */
	struct anoubisd_pg_scanner_list		 pg_scanner;

	/**
	 * The timeout for playground scanners. Each individual scanner
	 * is granted this amount of time before a scan failure is assumed.
	 */
	int					 scanner_timeout;
};

/**
 * A generic message passed between different anoubisd processes.
 */
struct anoubisd_msg {
	/**
	 * The size of the message. This includes the size and the
	 * the type field.
	 */
	int		size;

	/**
	 * The message type. This is one of the types listed in the
	 * @see anoubisd_msg_type
	 */
	int		mtype;

	/**
	 * The message payload. This is one of the anoubisd_msg_* structures
	 * defined below.
	 */
	char		msg[0];
};

/**
 * Possible message types for anoubis daemon messages passed between
 * different anoubis daemon processes. Each field is annotated with the
 * type in the message payload.
 */
enum anoubisd_msg_type {
	ANOUBISD_MSG_POLREQUEST,	/** anoubisd_msg_polrequest */
	ANOUBISD_MSG_POLREQUEST_ABORT,	/** anoubisd_msg_polrequest_abort */
	ANOUBISD_MSG_POLREPLY,		/** anoubisd_msg_polreply */
	ANOUBISD_MSG_CHECKSUMREPLY,	/** anoubisd_msg_csumreply */
	ANOUBISD_MSG_EVENTDEV,		/** struct eventdev_hdr */
	ANOUBISD_MSG_LOGREQUEST,	/** anoubisd_msg_eventask */
	ANOUBISD_MSG_EVENTREPLY,	/** struct eventdev_reply */
	ANOUBISD_MSG_EVENTCANCEL,	/** eventdev_token */
	ANOUBISD_MSG_CHECKSUM_OP,	/** anoubisd_msg_csumop */
	ANOUBISD_MSG_EVENTASK,		/** anoubisd_msg_eventask */
	ANOUBISD_MSG_POLICYCHANGE,	/** anoubisd_msg_pchange */
	ANOUBISD_MSG_PGCHANGE,		/** anoubisd_msg_pgchange */
	ANOUBISD_MSG_SFSCACHE_INVALIDATE, /** anoubisd_sfscache_invalidate */
	ANOUBISD_MSG_UPGRADE,		/** anoubisd_msg_upgrade */
	ANOUBISD_MSG_SFS_UPDATE_ALL,	/** anoubisd_sfs_update_all */
	ANOUBISD_MSG_CONFIG,		/** anoubisd_msg_config */
	ANOUBISD_MSG_LOGIT,		/** anoubisd_msg_logit */
	ANOUBISD_MSG_PASSPHRASE,	/** anoubisd_msg_passphrase */
	ANOUBISD_MSG_AUTH_REQUEST,	/** anoubisd_msg_authrequest */
	ANOUBISD_MSG_AUTH_CHALLENGE,	/** anoubisd_msg_authchallenge */
	ANOUBISD_MSG_AUTH_VERIFY,	/** anoubisd_msg_authverify */
	ANOUBISD_MSG_AUTH_RESULT,	/** anoubisd_msg_authresult */
	ANOUBISD_MSG_CSMULTIREQUEST,	/** anoubisd_msg_csumop */
	ANOUBISD_MSG_CSMULTIREPLY,	/** anoubisd_msg_csumreply */
	ANOUBISD_MSG_LISTREQUEST,	/** anoubisd_msg_listrequest */
	ANOUBISD_MSG_LISTREPLY,		/** anoubisd_msg_listreply */
	ANOUBISD_MSG_PGCOMMIT,		/** anoubisd_msg_pgcommit */
	ANOUBISD_MSG_PGCOMMIT_REPLY,	/** anoubisd_msg_pgcommit_reply */
};

/**
 * Message format of ANOUBISD_MSG_EVENTASK and ANOUBISD_MSG_LOGREQUEST
 * messages.
 */
struct anoubisd_msg_eventask
{
	/**
	 * The rule ID that triggered the event.
	 */
	uint32_t	rule_id;

	/**
	 * The priority of the rule that triggered the events.
	 */
	uint32_t	prio;

	/**
	 * The part of an SFS rule that triggered the event.
	 */
	uint32_t	sfsmatch;

	/**
	 * The error code of the event. This is only set in log requests.
	 * Escalation messages do not have a result, yet.
	 */
	uint32_t	error;

	/**
	 * The log level of the event. Only applicable to log requests.
	 */
	uint32_t	loglevel;

	/**
	 * Offset and length of the program's checksum. The offset applies
	 * the the payload field below.
	 */
	uint16_t	csumoff, csumlen;

	/**
	 * Offset and length (including the NUL byte) of the program's path
	 */
	uint16_t	pathoff, pathlen;

	/**
	 * Offset and length of the checksum of the context the program
	 * is running in.
	 */
	uint16_t	ctxcsumoff, ctxcsumlen;

	/**
	 * Offset and length of the path of the context the program is
	 * running in. The length includes the NUL byte.
	 */
	uint16_t	ctxpathoff, ctxpathlen;

	/**
	 * Offset and length of the raw kernel event in the payload below.
	 */
	uint16_t	evoff, evlen;

	/**
	 * The payload data. Offsets and lengths given above refer to
	 * this data.
	 */
	char		payload[0];
};

/**
 * Message format of ANOUBISD_MSG_POLREQUEST messages.
 */
struct anoubisd_msg_polrequest {
	/**
	 * The token of the policy request (assigned by the session engine.
	 */
	uint64_t	token;

	/**
	 * The authenticated user ID of the user responsible for the request.
	 */
	uint32_t	auth_uid;

	/**
	 * Policy flags. These flags tell the other side if more data
	 * belongs to the same request. Possible flags are POLICY_FLAG_START
	 * and POLICY_FLAG_END.
	 */
	uint32_t	flags;

	/**
	 * The length of the policy request stored in the data field.
	 */
	unsigned short	len;

	/**
	 * The policy request as received from the user. The format of the
	 * data is a Policy_Generic structure (incarnated as Policy_GetByUid
	 * or Policy_SetByUid.
	 */
	char		data[0];
};

/**
 * Message format of ANOUBISD_MSG_POLREQUEST_ABORT message. These are
 * sent by the session engine to the policy engine if the client connection
 * terminates in the middle of a policy request.
 */
struct anoubisd_msg_polrequest_abort {
	/**
	 * The token of the aborted request.
	 */
	uint64_t	token;
};


/**
 * Message format of ANOUBISD_MSG_POLREPLY messages. These messages
 * are sent by the policy engine in reply to a policy request from
 * the user.
 */
struct anoubisd_msg_polreply {
	/**
	 * The token of the user's policy request.
	 */
	uint64_t	token;

	/**
	 * The error code of the policy request.
	 */
	int		reply;

	/**
	 * Flags of the reply. Possible flags are POLICY_FLAG_START and
	 * POLICY_FLAG_END.
	 */
	uint32_t	flags;

	/**
	 * Length of the data that follows.
	 */
	short		len;		/* of following msg */

	/**
	 * The policy reply payload data. This is empty for SET requests
	 * and (part of) the ascii representation of the policy for GET
	 * requests.
	 */
	char		data[0];
};

/**
 * Format of ANOUBISD_MSG_CHECKSUM_OP and ANOUBISD_MSG_CSMULTIREQUEST
 * requests. This simply encapsulates a checksum requests received from
 * the user and forwards it to the master process.
 */
struct anoubisd_msg_csumop {
	/**
	 * The token assigned to this request by the session engine.
	 */
	uint64_t	token;

	/**
	 * The authenticated user ID of the user responsible for the request.
	 */
	uint32_t	uid;

	/**
	 * The length of the request data.
	 */
	short		len;

	/**
	 * The request data itself. It is an anoubis protocol message of
	 * type ANOUBIS_P_CSUMREQUEST or ANOUBIS_P_CSMULTIREQUEST.
	 */
	char		msg[0];
};

/**
 * Format of ANOUBISD_MSG_CHECKSUMREPLY and ANOUBISD_MSG_CSMULTIREPLY
 * messages. These are sent in reply to ANOUBISD_MSG_CHECKSUM_OP and
 * ANOUBISD_MSG_CSMULTIREQUEST messages respectively.
 */
struct anoubisd_msg_csumreply {
	/**
	 * The token assigend to the request by the session engine.
	 */
	uint64_t	token;

	/**
	 * The error code of the request.
	 */
	int		reply;

	/**
	 * The request flags. Possible flags are POLICY_FLAG_START and
	 * POLICY_FLAG_END.
	 */
	uint32_t	flags;

	/**
	 * The length of the reply.
	 */
	short		len;

	/**
	 * The reply data. It contains a file name lists for list requests or
	 * an Anoubis_CSMultiReplyMessage structure.
	 */
	char		data[0];
};

/**
 * Message format of ANOUBISD_MSG_SFSCACHE_INVALIDATE messages. These
 * are sent by the master if an individual checksum changes on disk the
 * cache in policy engine must be updated.
 */
struct anoubisd_sfscache_invalidate {
	/**
	 * The user ID of the modified unsigned checksum. Only used if
	 * we are invalidating an unsigned checksum. If keylen is non-zero
	 * this value is not used.
	 */
	uint32_t	uid;

	/**
	 * The length of the path name.
	 */
	uint32_t	plen;

	/**
	 * The length of the key ID. Only used if we are invalidating
	 * a singed checksum. Zero otherwise.
	 */
	uint32_t	keylen;

	/**
	 * Contains the NUL-terminted path name (plen bytes) followed by
	 * keylen bytes of the key ID.
	 */
	char		payload[0];
};

/**
 * Message format of ANOUBISD_MSG_SFS_UPDATE_ALL messages. These are
 * send by the upgrade process after calculating the new checksum of
 * an upgraded file. The master process will update all checksums of
 * the file in the SFS tree.
 */
struct anoubisd_sfs_update_all {
	/**
	 * The length of the checksum inside the payload. Should be
	 * ANOUBIS_CS_LEN.
	 */
	uint32_t	cslen;

	/**
	 * The payload data. This consists of cslen bytes of checksum
	 * data followed by the NUL-terminated path name of the upgraded
	 * file.
	 */
	char		payload[0];
};

/**
 * Message format of ANOUBISD_MSG_LOGIT messages. These are sent from
 * all daemon processes to the logger.
 */
struct anoubisd_msg_logit {
	/**
	 * The log priority. This is used as the priority argument for
	 * sysylog(3c).
	 */
	uint32_t		prio;

	/**
	 * The message data (a NUL-terminated string).
	 */
	char			msg[0];
};

/**
 * Message format of ANOUBISD_MSG_POLICYCHANGE messages. These are sent
 * by the policy engine to the session whenever a user policy is modified.
 * Connected user interfaces can use this to update a displayed ruleset.
 */
struct anoubisd_msg_pchange
{
	/**
	 * The user ID of the modified policy.
	 */
	uint32_t	uid;

	/**
	 * The priority of the modified policy.
	 */
	uint32_t	prio;
};

/**
 * Message format of ANOUBISD_MSG_PGCHANGE message. These are sent
 * by teh policy engine if the playground status of a playground changes.
 * Connected user interfaces can use this to show notifications to the user.
 */
struct anoubisd_msg_pgchange
{
	/**
	 * The user ID of the playground owner.
	 */
	uint32_t			uid;

	/**
	 * The playground ID.
	 */
	uint64_t			pgid;

	/**
	 * The playground operation.
	 */
	uint32_t			pgop;
};

/**
 * The sub-type for upgrade messages. Used in the upgradetype field of
 * anoubisd_msg_upgrade messages.
 */
enum anoubisd_upgrade {
	ANOUBISD_UPGRADE_START,
	ANOUBISD_UPGRADE_END,
	ANOUBISD_UPGRADE_CHUNK_REQ,
	ANOUBISD_UPGRADE_CHUNK,
	ANOUBISD_UPGRADE_OK,	 /* Chunk is a character: true or false */
	ANOUBISD_UPGRADE_NOTIFY, /* chunksize == number of upgraded files */
};

/**
 * Message format of ANOUBISD_MSG_UPGRADE messages. These messages
 * are exchanged between anoubis daemon processes during an upgrade.
 * The exact format of the payload data in the chunk field depends on
 * the upgrade type.
 */
struct anoubisd_msg_upgrade
{
	/**
	 * The upgrade type. This is one of the values defined in the
	 * anoubisd_upgrade enumeration.
	 */
	uint32_t	upgradetype;

	/**
	 * This value is zero for ANOUBISD_UPGRADE_START, ANOUBISD_UPGRADE_END
	 * and ANOUBISD_UPGRADE_CHUNK_REQ messages. For a message of type
	 * ANOUBISD_UPGRADE_CHUNK or ANOUBISD_UPGRADE_OK it is the size of
	 * the data in the chunk field. For a message of type
	 * ANOUBISD_UPGRADE_NOTIFY if is the total number of upgraded files
	 * in the last upgrade and chunk is unused.
	 */
	uint32_t	chunksize;

	/**
	 * This field contains payload data. It is only used for two of
	 * the upgrade messages. If the upgrade type is ANOUBISD_UPGRADE_CHUNK
	 * this field contains chunksize bytes of path names. Each path name
	 * is terminated by a NUL byte. For messages of type
	 * ANOUBISD_UPGRADE_OK it contains a single byte that can be either
	 * zero or one.
	 */
	char		chunk[0];
};

/**
 * Message format of ANOUBISD_MSG_PASSPHRASE messages. These are sent
 * from the session engine to the master process if the administrator
 * decides to unlock its private key before a system upgrade. Please
 * refer to the user documentation for the security implications of this.
 */
struct anoubisd_msg_passphrase
{
	/**
	 * The path phrase (a NUL-terminated string) of root's private key.
	 */
	char		payload[0];
};

/**
 * Message format for messages of type ANOUBISD_MSG_AUTH_REQUEST.
 * These messages are sent by the session engine when a client starts
 * its authentication.
 */
struct anoubisd_msg_authrequest {
	/**
	 * The token of the request as assigned by the session engine.
	 */
	uint64_t	token;

	/**
	 * The user ID of the user that want's to authenticate. This is
	 * taken from the communication channel. The user may still be
	 * required to proof access to the private key.
	 */
	uint32_t	auth_uid;
};

/**
 * Message format for messages of type ANOUBISD_MSG_AUTH_CHALLENGE.
 * These messages are sent by the master process for some authentication
 * types. They contain a random authentication challenge that must be
 * signed with the user's private key.
 */
struct anoubisd_msg_authchallenge {
	/**
	 * The token of the request as assigend by the session engine.
	 */
	uint64_t	token;

	/**
	 * The user ID of the user that wants to authenticate. This is
	 * copied from the authentication request.
	 */
	uint32_t	auth_uid;

	/**
	 * The error code for the authentication. If this value is
	 * non-zero the authentication failed permanently. If it is zero
	 * the message might contain a challenge. If the error code is
	 * zero and no challenge is present, the authentication succeeded.
	 */
	uint32_t	error;

	/**
	 * The length of the authentication challenge. Will be zero in
	 * case of an error. If zero in case of succes the authentication
	 * succeeded.
	 */
	uint32_t	challengelen;

	/**
	 * The length of the key ID. The key identified by the key ID is
	 * the public key of the user. The user must sign the challenge
	 * with the corresponding private key.
	 */
	uint32_t	idlen;

	/**
	 * The payload data. This consists of challengelen bytes of
	 * challenge data followed by idlen bytes for the key ID.
	 */
	char		payload[0];
};

/**
 * Message format for messages of type ANOUBISD_MSG_AUTH_VERIFY. These
 * messages are sent by the session engine after it received a reply to
 * a challenge with a signature. The master will verify the signature and
 * send an appropriate response to the session engine. The master is
 * completely stateless wrt. authentication, i.e. the session engine must
 * provide both challenge and signature data along with the user ID.
 */
struct anoubisd_msg_authverify {
	/**
	 * The token of the request assigned by the session engine.
	 */
	uint64_t	token;

	/**
	 * The uid that wants to authenticate.
	 */
	uint32_t	auth_uid;

	/**
	 * The length of the challenge data.
	 */
	uint32_t	datalen;

	/**
	 * The length of the signature data.
	 */
	uint32_t	siglen;

	/**
	 * datalen bytes of challenge data followed by siglen bytes of
	 * signature data.
	 */
	char		payload[0];
};

/**
 * Message format for messages of type ANOUBISD_MSG_AUTH_RESULT. These
 * are sent by the master process after a successful authentication (i.e.
 * a challenge was properly signed and the signature could be verified.
 * Authentications that can proceed without a challenge never trigger this
 * kind of message. Instead the authentication challenge is empty.
 */
struct anoubisd_msg_authresult {
	/**
	 * The request token assigned by the session engine.
	 */
	uint64_t	token;

	/**
	 * The user ID of the user that tried to authenticate.
	 */
	uint32_t	auth_uid;

	/**
	 * The result of the authentication. Zero means success.
	 */
	uint32_t	error;
};

/**
 * Message format for ANOUBISD_MSG_CONFIG messages. These are sent
 * by the master if the configuration read from anoubisd.conf changes.
 */
struct anoubisd_msg_config
{
	/**
	 * The number of upgrade triggers in the payload.
	 */
	uint32_t		triggercount;

	/**
	 * The value of the policysize limit in the new configuration.
	 */
	uint32_t		policysize;

	/**
	 * The new upgrade mode.
	 */
	uint8_t		upgrade_mode;

	/**
	 * The payload data. This consist of NUL-terminated string.
	 * The first one is the configuration value of the unix socket.
	 * The following triggercount strings are upgrade triggers. Note
	 * that although the the unix socket is passed to other processes,
	 * it cannot effectively be changed as this would require privileges
	 * in the session engine.
	 */
	char			chunk[0];
};

/**
 * Format of ANOUBISD_MSG_LISTREQUEST messages. These are sent by the
 * session engine to the policy engine in response to a list request from
 * user.
 */
struct anoubisd_msg_listrequest {
	/**
	 * The token of the request as assigned by the session engine.
	 */
	uint64_t	token;

	/**
	 * The authenticated user ID of the user responsible for the request.
	 */
	uint32_t	auth_uid;

	/**
	 * The type of this list request.
	 */
	uint32_t	listtype;

	/**
	 * The selector argument for the list request. Its meaning depends
	 * on the value of the listtype.
	 */
	uint64_t	arg;
};

/**
 * Message format of ANOUBISD_MSG_LISTREPLY messages. These are sent by
 * the policy engine in response to an ANOUBISD_MSG_LISTREQUEST. Multiple
 * messages of this type may be sent in response to a single request.
 */
struct anoubisd_msg_listreply {
	/**
	 * The request token as assigned by the session engine.
	 */
	uint64_t	token;

	/**
	 * The flags for this reply. Possible values are POLICY_FLAG_START
	 * and POLICY_FLAG_END.
	 */
	uint32_t	flags;

	/**
	 * The length of the reply data.
	 */
	uint32_t	len;

	/**
	 * The reply data itself. This is a sequence of zero or more
	 * reply records. The exact layout of each record depends on the
	 * list type. There is no need for the anoubis daemon to analyze
	 * this data is is passed to the client unmodified.
	 */
	char		data[0];
};

/**
 * Message format for ANOUBISD_MSG_PGCOMMIT messages. These messages are
 * sent during a playground commit.
 */
struct anoubisd_msg_pgcommit {
	/**
	 * The token of the request as assigned by the session engine.
	 */
	uint64_t	token;

	/**
	 * The playground ID of the file to be committed.
	 */
	uint64_t	pgid;

	/**
	 * The device number of the file to be committed. This is the
	 * kernel representation of device numbers.
	 */
	uint64_t	dev;

	/**
	 * The inode number of the file to be committed-.
	 */
	uint64_t	ino;

	/**
	 * The authenticated user ID of the requesting user.
	 */
	uint32_t	auth_uid;

	/**
	 * True if recommended scanners should be ignored for this commit.
	 */
	uint8_t	ignore_recommended_scanners;

	/**
	 * The path name of the file. This is the full path name not just
	 * the device relative part.
	 */
	char		path[0];
};

/**
 * Message format for ANOUBISD_MSG_PGCOMMIT_REPLY messages. These are
 * sent in response to a commit request. If the commit failed they contain
 * scanner results of scanners that rejected the file.
 */
struct anoubisd_msg_pgcommit_reply {
	/**
	 * The token of the request as assigned by the session engine.
	 */
	uint64_t	token;

	/**
	 * The length of the payload data.
	 */
	uint32_t	len;

	/**
	 * The result of the commit. Zero in case of success, EAGAIN if
	 * only recommended scanners failed and EPERM if at least one
	 * required scanner failed. Other error codes are possible if
	 * some system error occurs during commit.
	 */
	uint32_t	error;

	/**
	 * The payload data. This is a possibly empty sequence of
	 * NUL-terminated strings. The number of strings is even. Strings
	 * come in pair. The first string in each pair is the description
	 * of a scanner from the anoubisd.conf file, the second is its
	 * output for the given file. Only scanners that rejected the file
	 * are listed.
	 */
	char		payload[0];
};

/* Pre-declare important abstract types. */
struct pe_proc;
struct pe_proc_ident;


/**
 * These are used to populate the logger socketpairs and should be even
 * numbers to leave room for the odd endpoints.
 */
enum anoubisd_process_type {
	PROC_MAIN		= 0,
	PROC_POLICY		= 2,
	PROC_SESSION		= 4,
	PROC_UPGRADE		= 6,
	PROC_LOGGER		= 8	/* logger should be last */
};

/**
 * These are used to populate the pipes socketpairs and should be even
 * numbers to leave room for the odd endpoints.
 */
enum anoubisd_pipes {
	PIPE_MAIN_SESSION	= 0,
	PIPE_MAIN_POLICY	= 2,
	PIPE_SESSION_POLICY	= 4,
	PIPE_MAIN_UPGRADE	= 6,
	PIPE_MAX		= 8	/* should be last */
};

/**
 * Swap two integers. This is used to simplify handling of file descriptor
 * arrays.
 */
#define SWAP(a, b) do { int t; t = a; a = b; b = t; } while(0)
extern void	cleanup_fds(int[], int[]);

extern pid_t	session_main(int[], int[]);
extern pid_t	policy_main(int[], int[]);
extern pid_t	upgrade_main(int[], int[]);
extern pid_t	logger_main(int[], int[]);

extern void	pe_init(void);
extern void	pe_dump(void);
extern void	pe_shutdown(void);
extern void	pe_reconfigure(void);
extern void	pe_playground_initpgid(int, int);
extern void	log_init(int fd);
extern void	log_warn(const char *, ...)
		    __attribute__ ((format (printf, 1, 2)));
extern void	log_warnx(const char *, ...)
		    __attribute__ ((format (printf, 1, 2)));
extern void	log_info(const char *, ...)
		    __attribute__ ((format (printf, 1, 2)));
extern void	log_debug(const char *, ...)
		    __attribute__ ((format (printf, 1, 2)));

extern void	anoubisd_defaultsigset(sigset_t *);

extern __dead void	fatal(const char *);
extern __dead void	master_terminate(void);
extern __dead void	early_err(const char *);
extern __dead void	early_errx(const char *);

extern void	dazukofs_ignore(void);

extern int	send_policy_data(uint64_t token, int fd);
void		send_pgchange(unsigned int uid, anoubis_cookie_t pgid,
		    unsigned int pgop);
extern void	__send_lognotify(struct pe_proc_ident *pident,
		    struct pe_proc_ident *ctxident, struct eventdev_hdr *,
		    uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern void	send_lognotify(struct pe_proc *proc, struct eventdev_hdr *,
		    uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern void	send_policychange(uint32_t uid, uint32_t prio);
extern void	flush_log_queue(void);

extern void	send_upgrade_start(void);

/* Scanner functions */
extern int	anoubisd_scan_start(uint64_t token, int fd, uint64_t auth_uid,
		    int flags);
extern int	anoubisd_scanproc_exit(pid_t pid, int status, int anoubisfd,
		    Queue *queue);
extern void	anoubisd_scanners_detach(void);

/* External variable declarations. */
extern struct anoubisd_config		 anoubisd_config;
extern enum anoubisd_process_type	 anoubisd_process;
extern char				*logname;
extern uint32_t				 debug_flags;
extern char				 debug_stderr;
extern char				 anoubisd_noaction;
extern gid_t				 anoubisd_gid;
extern unsigned long			 version;

/**
 * Return the process name of the current anoubis daemon process. The
 * return value is a pointer to a static string.
 *
 * @return A static string that identifies the current anoubis daemon
 *     process for logging purposes.
 */
static inline const char	*anoubisd_proc_name(void)
{
	switch (anoubisd_process) {
	case PROC_MAIN:		return "main";
	case PROC_LOGGER:	return "logger";
	case PROC_POLICY:	return "policy";
	case PROC_SESSION:	return "session";
	case PROC_UPGRADE:	return "upgrade";
	default:		return "<unknown>";
	}
}

#ifndef S_SPLINT_S

/**
 * Print a debug message iff this type of debug message should be printed.
 *
 * @param flag The message type. This is one of DBG_MSG_* defined below.
 * @param fmt The printf-like format string for the debug message. The
 *     rest of the parameters is the list of format arguments.
 */
#define DEBUG(flag, fmt, ...)						\
	do {								\
		if (flag & debug_flags) {				\
			log_debug("%s: " fmt,				\
			    anoubisd_proc_name(), ## __VA_ARGS__);	\
		}							\
	} while(0)

#else

/* Splint does not parse varadic macros */
#define DEBUG

#endif

#define DBG_MSG_FD	0x0001
#define DBG_MSG_SEND	0x0002
#define DBG_MSG_RECV	0x0004
#define DBG_TRACE	0x0008
#define DBG_QUEUE	0x0010
#define DBG_PE		0x0020
#define DBG_PE_PROC	0x0040
#define DBG_PE_SFS	0x0080
#define DBG_PE_ALF	0x0100
#define DBG_PE_POLICY	0x0200
#define DBG_PE_TRACKER	0x0400
#define DBG_PE_CTX	0x0800
#define DBG_PE_DECALF	0x1000
#define DBG_SESSION	0x2000
#define DBG_SANDBOX	0x4000
#define DBG_SFSCACHE	0x8000
#define DBG_PE_BORROW	0x10000
#define DBG_UPGRADE	0x20000
#define DBG_CSUM	0x40000
#define DBG_PG		0x80000


/**
 * Compatibility function to make playground code compile on OpenBSD, too.
 * Return the value of the pgid field in the struct anoubis_event_common or
 * zero if the structure does not have this field.
 */
static inline anoubis_cookie_t
extract_pgid(struct anoubis_event_common *common __attribute__((unused)))
{
#if ANOUBISCORE_VERSION < ANOUBISCORE_PG_VERSION
	return 0;
#else
	return common->pgid;
#endif
}

#endif /* !_ANOUBISD_H */
