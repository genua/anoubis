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

#include "config.h"

#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <err.h>
#include <errno.h>
#ifdef LINUX
#include <grp.h>
#include <bsdcompat.h>
#include <openssl/sha.h>
#endif
#ifdef OPENBSD
#include <sha2.h>
#endif
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* on glibc 2.6+, event.h uses non C89 types :/ */
#include <event.h>
#include "anoubisd.h"
#include "sfs.h"
#include "cert.h"
#include "aqueue.h"
#include "amsg.h"
#include "pe.h"
#include "pe_filetree.h"
#include "cfg.h"
#include "amsg_list.h"
#include <anoubis_alloc.h>

#include <anoubis_protocol.h>

/* Prototypes */
static void	dispatch_timer(int, short, void *);
static void	dispatch_m2p(int, short, void *);
static void	dispatch_p2m(int, short, void *);
static void	dispatch_s2p(int, short, void *);
static void	dispatch_p2s(int, short, void *);
static int	policy_upgrade_fill_chunk(char *buf, int maxlen);

/**
 * This structure is used to keep track of escalations which have been
 * forwarded to the session process. They will be timed out after a
 * configured timeout and a default reply will be generated.
 */
struct reply_wait {
	/**
	 * This field is used to link these structures in the global
	 * list of open escalations.
	 */
	TAILQ_ENTRY(reply_wait)	next;

	/**
	 * The token of the kernel event. This is used in replies and
	 * to locate the escalation.
	 */
	eventdev_token		token;

	/**
	 * The anoubis eventdev reply flags to add to the reply.
	 * These flags cannot be influenced by the user.
	 */
	int			flags;

	/**
	 * The start time of the event.
	 */
	time_t			starttime;

	/**
	 * The timeout for the event relative to the start time.
	 */
	time_t			timeout;

	/**
	 * The log level of the event.
	 */
	int			log;
};

/**
 * The event queue for events that are sent from the policy engine
 * to the master.
 */
static Queue	eventq_p2m;

/**
 * The event queue for events that must be sent from the policy
 * engine to the master but are currently held back due to an old
 * upgrade that is still not processed completly.
 */
static Queue	eventq_p2m_hold;

/**
 * The event queue for events that are sent from the policy engine
 * to the session process.
 */
static Queue	eventq_p2s;

/**
 * The global list of all pending escalations.
 */
static TAILQ_HEAD(, reply_wait)		replyq;

/**
 * The event for incoming messages from the session engine.
 */
static struct event		ev_s2p;

/**
 * The event for incoming messages from the master.
 */
static struct event		ev_m2p;

/**
 * The timer event that is used to timeout pending escalations.
 */
struct event			ev_timer;

/**
 * The value of the timeout used for the timeout event (5 seconds).
 */
struct timeval			tv = {
					.tv_sec = 5,
					.tv_usec = 0,
				};

/**
 * Events for signals. They are stored globally, because the
 * signal handler must be able to remove them from the event
 * queue during shutdown.
 */
struct event			*ev_sigs[10];

/**
 * The current status of the daemon termination. Possible values are:
 * 0: Policy engine is running normally.
 * 1: Termination requested. All signals are blocked at this point and
 *    neither signal nor timer events are active anymore.
 * 2: End of file on the pipe from the master was received. We still wait
 *    for the session engine to flush its queue.
 * 3: End of file from the session engine was received. At this point all
 *    remaining events that have not been answered are denied. This
 *    completes the termination process.
 */
static int terminate = 0;

/**
 * Set the termination status to the given value. The termination level
 * can only increase and transitions trigger certain events. Whenever
 * the status increases, all actions tied to skipped states are performed, too.
 *
 * @param n The new termination status.
 */
static void
set_terminate(int n)
{
	while (terminate < n) {
		++terminate;
		switch (terminate) {
		case 1: {
			sigset_t			 mask;
			int				 i;

			sigfillset(&mask);
			sigprocmask(SIG_SETMASK, &mask, NULL);
			for (i=0; ev_sigs[i]; ++i)
				signal_del(ev_sigs[i]);
			event_del(&ev_timer);
			break;
		}
		case 2:
			/* No special action required. */
			break;
		case 3:
			dispatch_timer(0, 0, NULL);
			break;
		}
	}
}


/**
 * The signal handler for signal events. This is never run from
 * signal context. It is called from the event loop in response
 * to a signal. In theory, it is legal to call this handler manually,
 * but currently this never happens.
 *
 * @param sig The signal. Possible values are:
 *     SIGUSR1: Dump policy related information to the log.
 *     SIGINT/SIGTERM: Initiate graceful shutdown.
 *     SIGQUIT: Forcefully terminate the policy engine.
 * @param event The details of the event (unused).
 * @param arg The callback argument of the event (unused).
 */
static void
policy_sighandler(int sig, short event __used, void *arg __used)
{
	switch (sig) {
	case SIGUSR1:
		pe_dump();
		break;
	case SIGINT:
	case SIGTERM:
		set_terminate(1);
		break;
	case SIGQUIT:
		(void)event_loopexit(NULL);
		/* FALLTRHOUGH */
	}
}

/**
 * This is the main entry pont of the policy engine. It starts a child
 * process that runs the policy engine. For the master process there
 * are no further side effects.
 *
 * The policy engine drops all privileges and runs chrooted. It has
 * read only access to the SFS tree and read/write access to the policy
 * backing store.
 *
 * @param pipes The file descriptors of the pipes that are used by the
 *     anoubisd process to communicate with each other. Two consecutive
 *     descriptors are used to describe on pipe. The array is indexed by
 *     the PIPE_* macros defined in anoubisd.h. This function consumes
 *     the two file descriptors: The policy end of the pipe between master
 *     and policy and the policy end of the pipe between session and policy.
 *     All other file descriptors are closed in the child process.
 * @param loggers The pipes from the anoubis daemon processes to the
 *     logger. This function uses one of the pipes in the child and closes
 *     all others.
 * @return In the master the pid of the child process is retruned. The
 *     function has no other side effects on the master. In the child this
 *     function never returns.
 */
pid_t
policy_main(int pipes[], int loggers[])
{
	int		 masterfd, sessionfd, logfd, err;
	struct event	 ev_sigterm, ev_sigint, ev_sigquit, ev_sigusr1;
	struct event	 ev_p2s, ev_p2m;
	struct passwd	*pw;
	sigset_t	 mask;
	pid_t		 pid;

	pid = fork();
	if (pid == -1)
		fatal("fork");
		/*NOTREACHED*/
	if (pid)
		return (pid);

	(void)event_init();

	anoubisd_process = PROC_POLICY;

	masterfd = sessionfd = logfd = -1;
	SWAP(masterfd, pipes[PIPE_MAIN_POLICY + 1]);
	SWAP(sessionfd, pipes[PIPE_SESSION_POLICY + 1]);
	SWAP(logfd, loggers[anoubisd_process]);
	cleanup_fds(pipes, loggers);

	log_init(logfd);

	/* open /etc/services before chroot */
	setservent(1);
#ifdef LINUX
	dazukofs_ignore();
#endif

	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		fatal("getpwnam");
	err = mkdir(ANOUBISD_PG, 0750);
	if (err == 0) {
		if (chown(ANOUBISD_PG, pw->pw_uid, pw->pw_gid) < 0)
			fatal("Cannot chown " ANOUBISD_PG);
	} else if (errno != EEXIST) {
		fatal("Cannot create " ANOUBISD_PG);
	}

	if (chroot(PACKAGE_POLICYDIR) == -1)
		fatal("chroot");
	if (chdir(ANOUBISD_POLICYCHROOT) == -1)
		fatal("chdir");

	/* XXX: glibc forgets about stayopen after chroot,
		solved by adding another setservent */
	setservent(1);

	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		fatal("can't drop privileges");

	/* From now on, this is an unprivileged child process. */
	log_info("policy started (pid %d root %s)",
	    getpid(), PACKAGE_POLICYDIR);

	/* We catch or block signals rather than ignoring them. */
	signal_set(&ev_sigterm, SIGTERM, policy_sighandler, NULL);
	signal_set(&ev_sigint, SIGINT, policy_sighandler, NULL);
	signal_set(&ev_sigquit, SIGQUIT, policy_sighandler, NULL);
	signal_set(&ev_sigusr1, SIGUSR1, policy_sighandler, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);
	signal_add(&ev_sigusr1, NULL);
	ev_sigs[0] = &ev_sigterm;
	ev_sigs[1] = &ev_sigint;
	ev_sigs[2] = &ev_sigquit;
	ev_sigs[3] = &ev_sigusr1;
	ev_sigs[4] = NULL;

	anoubisd_defaultsigset(&mask);
	sigdelset(&mask, SIGUSR1);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	queue_init(&eventq_p2m_hold, NULL);
	TAILQ_INIT(&replyq);

	/* init msg_bufs and setup events */
	msg_init(masterfd);
	msg_init(sessionfd);

	/* master process */
	event_set(&ev_m2p, masterfd, EV_READ | EV_PERSIST, dispatch_m2p, NULL);
	event_add(&ev_m2p, NULL);

	event_set(&ev_p2m, masterfd, EV_WRITE, dispatch_p2m, NULL);

	/* session process */
	event_set(&ev_s2p, sessionfd, EV_READ | EV_PERSIST, dispatch_s2p, NULL);
	event_add(&ev_s2p, NULL);

	event_set(&ev_p2s, sessionfd, EV_WRITE, dispatch_p2s, NULL);

	queue_init(&eventq_p2m, &ev_p2m);
	queue_init(&eventq_p2s, &ev_p2s);

	/* Timer for the event timeout. */
	evtimer_set(&ev_timer, &dispatch_timer, NULL);
	event_add(&ev_timer, &tv);

	/* Start policy engine */
	pe_init();
	pe_playground_init();

	setproctitle("policy engine");

	DEBUG(DBG_TRACE, "policy event loop");
	if (event_dispatch() == -1)
		fatal("policy_main: event_dispatch");

	/* Shutdown policy engine */
	pe_shutdown();
	_exit(0);
}

/**
 * This is the event handler for the timer event. This function generates
 * default deny answers for all events that have timed out. Finally it
 * re-schedules the timer event. I.e. this function is called once every
 * five seconds. If the termination level is three, all events are cancelled,
 * not just those that have expired.
 *
 * Cancelling an event involves two action: The kernel must be informed
 * via the master process. The error code used is EPERM. Additionally, the
 * UIs must be notified via the session engine, too.
 *
 * @param sig The signal that triggered the event (unused).
 * @param event The details of the event (unused).
 * @param arg The callback argument of the event (unused).
 */
static void
dispatch_timer(int sig __used, short event __used, void *arg __used)
{
	struct reply_wait		*msg_wait, *msg_next;
	struct anoubisd_msg		*msg;
	eventdev_token			*tk;
	struct eventdev_reply		*rep;
	time_t				 now = time(NULL);

	DEBUG(DBG_TRACE, ">dispatch_timer");

	msg_next = TAILQ_FIRST(&replyq);
	for (msg_wait = msg_next;  msg_wait; msg_wait = msg_next) {
		msg_next = TAILQ_NEXT(msg_wait, next);
		if (terminate < 3
		    && now < (msg_wait->starttime + msg_wait->timeout))
			continue;
		msg = msg_factory(ANOUBISD_MSG_EVENTREPLY,
			    sizeof(struct eventdev_reply));
		if (!msg)
			master_terminate();
		rep = (struct eventdev_reply *)msg->msg;
		rep->msg_token = msg_wait->token;
		rep->reply = EPERM;
		enqueue(&eventq_p2m, msg);
		DEBUG(DBG_QUEUE, " >eventq_p2m: %x", rep->msg_token);

		msg = msg_factory(ANOUBISD_MSG_EVENTCANCEL,
		    sizeof(eventdev_token));
		if (!msg)
			master_terminate();
		tk = (eventdev_token *)msg->msg;
		*tk = msg_wait->token;
		enqueue(&eventq_p2s, msg);
		DEBUG(DBG_QUEUE, " >eventq_p2s: %x", msg_wait->token);

		DEBUG(DBG_QUEUE, " <replyq: %x error=%d", msg_wait->token,
		    rep->reply);
		TAILQ_REMOVE(&replyq, msg_wait, next);
		switch(msg_wait->log) {
		case APN_LOG_NORMAL:
			log_info("token %u: no  user reply (denied)",
			    msg_wait->token);
			break;
		case APN_LOG_ALERT:
			log_warnx("token %u: no user reply (denied)",
			    msg_wait->token);
			break;
		}
		abuf_free_type(msg_wait, struct reply_wait);
	}
	if (terminate < 3)
		event_add(&ev_timer, &tv);

	DEBUG(DBG_TRACE, "<dispatch_timer");
}

/**
 * Process a request from the master to invalidate a certain entry
 * in the cache for the SFS tree. The entry is specified by its path
 * name and the uid or key. This function removes the corresponding
 * entry from the SFS cache.
 *
 * @param The message (must be of type ANOUBISD_MSG_SFSCACHE_INVALIDATE)
 *     The message is not freed by thios function.
 */
static void
dispatch_sfscache_invalidate(struct anoubisd_msg *msg)
{
	struct anoubisd_sfscache_invalidate	*invmsg;
	int					 total;

	DEBUG(DBG_SFSCACHE, ">dispatch_sfscache_invalidate");
	total = sizeof(*msg) + sizeof(*invmsg);
	if (msg->size < total) {
		log_warnx("Short sfscache_invalidate message");
		return;
	}
	invmsg = (struct anoubisd_sfscache_invalidate*)msg->msg;
	total += invmsg->keylen + invmsg->plen;
	if (msg->size < total) {
		log_warnx("Short sfscache_invalidate message");
		return;
	}
	invmsg->payload[invmsg->plen-1] = 0;
	invmsg->payload[invmsg->plen + invmsg->keylen - 1] = 0;
	if (invmsg->keylen) {
		sfshash_invalidate_key(invmsg->payload,
		    invmsg->payload + invmsg->plen);
		DEBUG(DBG_SFSCACHE, " dispatch_sfscache_invalidate: path %s "
		    "key %s", invmsg->payload, invmsg->payload+invmsg->plen);
	} else {
		sfshash_invalidate_uid(invmsg->payload, (uid_t)invmsg->uid);
		DEBUG(DBG_SFSCACHE, " dispatch_sfscache_invalidate: path %s "
		    "uid %d", invmsg->payload, invmsg->uid);
	}
}

/**
 * Send an upgrade start message to the upgrade process (relayed via
 * the master process) and initialize the upgrade iterator. The upgrade
 * process will start to request chunks of file names. The message
 * is composed in this function and sent directly to the master.
 */
void
send_upgrade_start(void)
{
	struct anoubisd_msg			*msg;
	struct anoubisd_msg_upgrade		*upg;

	DEBUG(DBG_UPGRADE, ">send_upgrade_start");

	msg = msg_factory(ANOUBISD_MSG_UPGRADE,
	    sizeof(struct anoubisd_msg_upgrade));
	if (msg == NULL) {
		log_warnx("send_upgrade_start: Out of memory");
		return;
	}

	upg = (struct anoubisd_msg_upgrade *)msg->msg;
	upg->upgradetype = ANOUBISD_UPGRADE_START;
	upg->chunksize = 0;

	DEBUG(DBG_UPGRADE, " send_upgrade_start: "
	    "enqueue ANOUBISD_UPGRADE_START");
	enqueue(&eventq_p2m, msg);

	/* Prepare upgrade-iterator, used later for creation of chunks */
	pe_upgrade_filelist_start();

	DEBUG(DBG_UPGRADE, "<send_upgrade_start");
}

/**
 * Copy as many filenames as possible from the upgrade file tree to the
 * buffer. The upgrade iterator is used to determine which file names
 * to copy. The Caller must call pe_upgrade_filelist_start before calling
 * this for the first time.
 * A path that does not fit into the buffer is not copied at all and will
 * be reconsidered with the next chunk.
 *
 * @param buf The buffer for the path names.
 * @param maxlen The length of the buffer.
 * @return The number of bytes actually copied.
 */
static int
policy_upgrade_fill_chunk(char *buf, int maxlen)
{
	int	reallen = 0;

	while (1) {
		struct pe_filetree_node	*node = pe_upgrade_filelist_get();
		int len;

		if (!node)
			break;
		len = strlen(node->path) + 1;
		if (reallen + len > maxlen)
			break;
		memcpy(buf+reallen, node->path, len);
		reallen += len;
		pe_upgrade_filelist_next();
	}
	return reallen;
}

/**
 * Send a chunk of filenames of upgraded files to the upgrade process
 * (relayed via the master). This function fills a message with path names
 * and sends it to the master process.
 */
static void
send_upgrade_chunk(void)
{
	struct anoubisd_msg		*msg;
	struct anoubisd_msg_upgrade	*upg;

	DEBUG(DBG_UPGRADE, ">send_upgrade_chunk");

	/*
	 * Create the next chunk.
	 *
	 * ->chunksize can be 0! It means that no more files are available.
	 *
	 * The buffer has a size of PATH_MAX to make sure, that at least
	 * one big pathname can be transfered.
	 */
	msg = msg_factory(ANOUBISD_MSG_UPGRADE,
	    sizeof(struct anoubisd_msg_upgrade) + PATH_MAX);
	if (msg == NULL) {
		log_warnx("send_upgrade_chunk: Out of memory");
		return;
	}

	upg = (struct anoubisd_msg_upgrade *)msg->msg;
	upg->upgradetype = ANOUBISD_UPGRADE_CHUNK;
	upg->chunksize = policy_upgrade_fill_chunk(upg->chunk, PATH_MAX);
	msg_shrink(msg, sizeof(struct anoubisd_msg_upgrade) + upg->chunksize);

	DEBUG(DBG_UPGRADE, " send_upgrade_chunk: "
	    "enqueue ANOUBISD_UPGRADE_CHUNK, chunksize = %i",
	    upg->chunksize);
	enqueue(&eventq_p2m, msg);

	DEBUG(DBG_UPGRADE, "<send_upgrade_chunk");
}

/**
 * Process a message of type ANOUBISD_MSG_UPGRADE received from the
 * master process. The messages usually originate from the upgrade process
 * but are relayed via the master process. The message can be of three
 * different types:
 * - The upgrade process requests a chunk of upgrade filenames.
 * - The upgrade process informs us that processing of the current
 *     upgrade completed.
 * - The master informs the policy engine if it should track new upgrades
 *
 * @param The message of type ANOUBISD_MSG_UPGRADE. The message is not
 *     freed by this function.
 */
static void
dispatch_upgrade(struct anoubisd_msg *msg)
{
	struct anoubisd_msg_upgrade	*upg;
	struct eventdev_reply		*rep;
	int				total;

	DEBUG(DBG_UPGRADE, ">dispatch_upgrade");

	total = sizeof(*msg) + sizeof(*upg);
	if (msg->size < total) {
		log_warnx("Short anoubisd_msg_upgrade message");
		return;
	}

	upg = (struct anoubisd_msg_upgrade *)msg->msg;

	if (upg->upgradetype == ANOUBISD_UPGRADE_CHUNK_REQ) {
		DEBUG(DBG_UPGRADE, " dispatch_upgrade: "
		    "upgradetype = ANOUBISD_UPGRADE_CHUNK_REQ");

		/*
		 * Another chunk was requested,
		 * collect files and send them back
		 */
		send_upgrade_chunk();
	} else if (upg->upgradetype == ANOUBISD_UPGRADE_END) {
		DEBUG(DBG_UPGRADE, " dispatch_upgrade: "
		    "upgradetype = ANOUBISD_UPGRADE_END");

		/*
		 * All files are processed, we can finish the upgrade, now.
		 */
		pe_upgrade_finish();
		pe_proc_release();
		while (1) {
			struct anoubisd_msg		*msg;

			msg = dequeue(&eventq_p2m_hold);
			if (!msg)
				break;
			enqueue(&eventq_p2m, msg);
			rep = (struct eventdev_reply *)msg->msg;
			DEBUG(DBG_QUEUE, " p2m_hold->p2m: %x",
			    rep->msg_token);
		}
	} else if (upg->upgradetype == ANOUBISD_UPGRADE_OK) {
		DEBUG(DBG_UPGRADE, " dispatch_upgrade: "
		    "type = ANOUBISD_UPGRADE_OK arg = %d", upg->chunk[0]);
		pe_set_upgrade_ok(upg->chunk[0]);
	} else {
		/* Unexpected message */
		log_warnx("dispatch_upgrade: unexpected upgradetype (%i)",
		    upg->upgradetype);
	}

	DEBUG(DBG_UPGRADE, "<dispatch_upgrade");
}

/**
 * Helper function that calculates the size of a process identifier
 * for the purpose of sending it to the session engine, e.g. as part of
 * an escalation message. It is the size of the checksum plus the
 * length of the path name including the terminating NUL byte.
 *
 * @param pident The process identifier.
 * @return The size.
 */
static int
pident_size(struct pe_proc_ident *pident)
{
	int ret;
	if (!pident)
		return 0;
	ret = abuf_length(pident->csum);
	if (pident->pathhint)
		ret += 1+strlen(pident->pathhint);
	return ret;
}

/**
 * Internal helper function. This copies <code>datalen</code> bytes from
 * <code>data</code> to the offset pointed to by <code>offp</code> in
 * <code>buf</code>. The value pointed to by <code>offp</code> is updated
 * accordingly. Additionally, the initial offset from <code>offp</code> and
 * the length of the data copied are stored in <code>roffp</code> and
 * <code>rlenp</code>.
 *
 * @param buf The target buffer.
 * @param offp The pointer to the target offset. The target offset is
 *     increased by the number of copied bytes.
 * @param data The data to copy.
 * @param datalen The length of th data to copy.
 * @param roffp The initial offset of the copy is stored here.
 * @param rlenp The length of the copy is stored here.
 */
static void
do_copy(char *buf, int *offp, const void *data, int datalen,
    u_int16_t *roffp, u_int16_t *rlenp)
{
	if (data)
		memcpy(buf + *offp, data, datalen);
	else
		datalen = 0;
	*roffp = *offp;
	*rlenp = datalen;
	*offp += datalen;
}

/**
 * The same as do_copy but the data and data length are taken from an
 * abuf_buffer and not from a pointer.
 *
 * @param buf The target buffer.
 * @param offp The pointer to the target offset. The target offset is
 *     increase by the number of copied bytes.
 * @param data The buffer containing the data to copy. The entire buffer
 *     is copied.
 * @param roffp The initial offset of the copy is stored here.
 * @param rlenp The length of the copy is stored here.
 */
static void
do_copy_buf(char *buf, int *offp, const struct abuf_buffer data,
    uint16_t *roffp, uint16_t *rlenp)
{
	do_copy(buf, offp, abuf_toptr(data, 0, abuf_length(data)),
	    abuf_length(data), roffp, rlenp);
}

/**
 * Copy a proc ident by calling do_copy twice. Once for the csum and once
 * for the pathhint. The offset pointed to by <code>offp</code> is updated.
 * Additionally the target offsets and lengths of the copies are
 * stored in <code>rcsoffp/rcslenp</code> and
 * <code>rpathoffp/rpathlenp</code> respectively.
 *
 * @param The target buffer.
 * @param offp The pointer to the target offset. The target offset is
 *     increased by the number of copied bytes.
 * @param pident The process identifier to copy.
 * @param rcsoffp The offset of the checksum is stored here.
 * @param rcslenp The length of the checksum is stored here.
 * @param rpathoffp The offset of the path is stored here.
 * @param rpathlenp The length of the path is stored here.
 */
static void
do_copy_ident(char *buf, int *offp, const struct pe_proc_ident *pident,
    u_int16_t *rcsoffp, u_int16_t *rcslenp,
    u_int16_t *rpathoffp, u_int16_t *rpathlenp)
{
	int plen = 0;
	if (!pident) {
		*rcsoffp = *rpathoffp = *offp;
		*rcslenp = *rpathlenp = 0;
		return;
	}
	do_copy_buf(buf, offp, pident->csum, rcsoffp, rcslenp);
	if (pident->pathhint)
		plen = strlen(pident->pathhint)+1;
	do_copy(buf, offp, pident->pathhint, plen, rpathoffp, rpathlenp);
}

/**
 * Compose an escalation message that will be sent to the session engine.
 * The data of the event is taken from the kernel event and the policy
 * engine reply to that event.
 *
 * @param type The type of the event (either ANOUBISD_MSG_EVENTASK or
 *     ANOUBIS_MSG_LOGREQUEST).
 * @param hdr The kernel event.
 * @param reply The policy engine reply for the event.
 * @return A newly allocated message ready to be sent to the session engine.
 */
static struct anoubisd_msg *
fill_eventask_message(int type, struct eventdev_hdr *hdr,
    struct anoubisd_reply *reply)
{
	struct anoubisd_msg		*msg;
	struct anoubisd_msg_eventask	*eventask;
	int				 extra, off;

	extra = hdr->msg_size;
	extra += pident_size(reply->pident);
	extra += pident_size(reply->ctxident);
	msg = msg_factory(type, sizeof(struct anoubisd_msg_eventask) + extra);
	if (!msg)
		return NULL;
	eventask = (struct anoubisd_msg_eventask *)msg->msg;
	if (reply->ask)
		eventask->error = 0;
	else
		eventask->error = reply->reply;
	eventask->rule_id = reply->rule_id;
	eventask->prio = reply->prio;
	eventask->sfsmatch = reply->sfsmatch;
	eventask->loglevel = reply->log;
	off = 0;
	do_copy(eventask->payload, &off, hdr, hdr->msg_size,
	    &eventask->evoff, &eventask->evlen);
	do_copy_ident(eventask->payload, &off, reply->pident,
	    &eventask->csumoff, &eventask->csumlen, &eventask->pathoff,
	    &eventask->pathlen);
	do_copy_ident(eventask->payload, &off, reply->ctxident,
	    &eventask->ctxcsumoff, &eventask->ctxcsumlen,
	    &eventask->ctxpathoff, &eventask->ctxpathlen);
	return msg;
}

/**
 * Process a message received from the master. If end of file is detected
 * on the incoming pipe a graceful termination is initiated or continued.
 * Possible message type are:
 * ANOUBISD_MSG_SFSCACHE_INVALIDATE: Invalidate an entry in the sfs cache.
 * ANOUBISD_MSG_EVENTDEV: Process a kernel event according to the policy.
 *     These messages are either answered directly according to the
 *     relevant policies or the event is forwarded to the sesssion engine.
 *     In the latter case a the event is tracked and a timeout is attached
 *     to it. The event will be denied if the session engine does not answer
 *     the event within this timeout.
 * ANOUBISD_MSG_UPGRADE: Upgrade messages.
 * ANOUBISD_MSG_CONFIG: Configuration changes.
 * ANOUBISD_MSG_PGCOMMIT_REPLY: Replies to commit request for the playground.
 *
 * @param fd The file descriptor of the incoming message.
 * @param sig The event details (unused).
 * @param arg The callback argument of the event (unused).
 */
static void
dispatch_m2p(int fd, short sig __used, void *arg __used)
{
	struct reply_wait			*msg_wait;
	struct anoubisd_msg			*msg;
	struct anoubisd_msg			*msg_reply;
	struct anoubisd_reply			*reply;
	struct eventdev_hdr			*hdr;
	struct eventdev_reply			*rep;

	DEBUG(DBG_TRACE, ">dispatch_m2p");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL)
			break;
		hdr = (struct eventdev_hdr *)msg->msg;
		DEBUG(DBG_QUEUE, " >m2p: %x", hdr->msg_token);
		switch(msg->mtype) {
		case ANOUBISD_MSG_SFSCACHE_INVALIDATE:
			dispatch_sfscache_invalidate(msg);
			free(msg);
			continue;
		case ANOUBISD_MSG_EVENTDEV:
			hdr = (struct eventdev_hdr *)msg->msg;
			break;
		case ANOUBISD_MSG_UPGRADE:
			dispatch_upgrade(msg);
			free(msg);
			continue;
		case ANOUBISD_MSG_CONFIG:
			pe_reconfigure();
			if (cfg_msg_parse(msg) == 0) {
				log_info("policy: reconfigure");
				/* XXX ch: do we need to do more? */
			} else {
				log_warnx("policy: reconfigure failed");
			}
			free(msg);
			continue;
		case ANOUBISD_MSG_PGCOMMIT_REPLY:
			pe_playground_dispatch_commitreply(msg);
			enqueue(&eventq_p2s, msg);
			continue;
		default:
			log_warnx("dispatch_m2p: bad message type %d",
			    msg->mtype);
			free(msg);
			continue;
		}

		/*
		 * At this point the message must be of
		 * type ANOUBISD_MSG_EVENTDEV.
		 */

		DEBUG(DBG_PE, "dispatch_m2p: src=%d pid=%d", hdr->msg_source,
		    hdr->msg_pid);
		if (((hdr->msg_flags & EVENTDEV_NEED_REPLY) == 0) &&
		    (hdr->msg_source != ANOUBIS_SOURCE_PROCESS &&
		    hdr->msg_source != ANOUBIS_SOURCE_SFSEXEC &&
		    hdr->msg_source != ANOUBIS_SOURCE_IPC &&
		    hdr->msg_source != ANOUBIS_SOURCE_PLAYGROUNDPROC &&
		    hdr->msg_source != ANOUBIS_SOURCE_PLAYGROUNDFILE)) {
			free(msg);
			DEBUG(DBG_TRACE, "<dispatch_m2p (not NEED_REPLY)");
			continue;
		}
		reply = policy_engine(msg);

		if (reply == NULL) {
			free(msg);
			continue;
		}

		if (reply->ask) {
			struct anoubisd_msg	*nmsg;
			eventdev_token		 token = hdr->msg_token;

			nmsg = fill_eventask_message(ANOUBISD_MSG_EVENTASK,
			    hdr, reply);
			if (!nmsg) {
				free(msg);
				free(reply);
				master_terminate();
			}
			/* The hdr is in the freed message! */
			hdr = NULL;
			free(msg);

			msg = nmsg;
			msg_wait = abuf_alloc_type(struct reply_wait);
			if (msg_wait == NULL) {
				log_warn("dispatch_m2p: can't allocate memory");
				free(reply);
				master_terminate();
			}

			msg_wait->token = token;
			if (time(&msg_wait->starttime) == -1) {
				free(msg);
				free(reply);
				log_warn("dispatch_m2p: failed to get time");
				master_terminate();
			}
			msg_wait->flags = ANOUBIS_RET_FLAGS(reply->reply);
			msg_wait->timeout = reply->timeout;
			msg_wait->log = reply->log;

			TAILQ_INSERT_TAIL(&replyq, msg_wait, next);
			DEBUG(DBG_QUEUE, " >replyq: %x flags=%x",
			    msg_wait->token, msg_wait->flags);

			/* send msg to the session */
			enqueue(&eventq_p2s, msg);
			DEBUG(DBG_QUEUE, " >eventq_p2s: %x", token);
		} else {
			int	hold = reply->hold;
			msg_reply = msg_factory(ANOUBISD_MSG_EVENTREPLY,
			    sizeof(struct eventdev_reply));
			if (!msg_reply) {
				free(msg);
				free(reply);
				master_terminate();
			}
			rep = (struct eventdev_reply *)msg_reply->msg;
			rep->msg_token = hdr->msg_token;
			rep->reply = reply->reply;

			free(msg);

			if (hold) {
				enqueue(&eventq_p2m_hold, msg_reply);
				DEBUG(DBG_QUEUE, " >eventq_p2m_hold: %x",
				    hdr->msg_token);
			} else {
				enqueue(&eventq_p2m, msg_reply);
				DEBUG(DBG_QUEUE, " >eventq_p2m: %x",
				    hdr->msg_token);
			}
		}

		free(reply);

		DEBUG(DBG_TRACE, "<dispatch_m2p (loop)");
	}
	if (msg_eof(fd)) {
		set_terminate(2);
		event_del(&ev_m2p);
		event_add(eventq_p2s.ev, NULL);
	}
	DEBUG(DBG_TRACE, "<dispatch_m2p (no msg)");
}

/**
 * This is the event handler for outgoing message from the policy engine
 * to the master process.
 *
 * @param fd The file descriptor for the event.
 * @param sig The event details (unused).
 * @param arg The callback argument of the event (unused).
 */
static void
dispatch_p2m(int fd, short sig __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_p2m");
	dispatch_write_queue(&eventq_p2m, fd);
	DEBUG(DBG_TRACE, "<dispatch_p2m");
}

/**
 * Send a log notification to the session engine. This function allocates
 * and fills an event message of type ANOUBISD_MSG_LOGREQUEST with the
 * data from the arguments and the given kernel event. The message is
 * then sent to the session engine.
 *
 * @param pident The identifier of the process that triggered the event.
 * @param ctxident The identifier of the context.
 * @param hdr The raw kernel event.
 * @param error The reply to the event.
 * @param loglevel The loglevel according to the policy.
 * @param rule_id The rule ID that triggered the log event.
 * @param prio the priority of the ruleset that triggered the log event.
 * @param sfsmatch For log events triggered by an SFS rule this is the
 *     part of the SFS rule that actually matched.
 */
void
__send_lognotify(struct pe_proc_ident *pident, struct pe_proc_ident *ctxident,
    struct eventdev_hdr *hdr, u_int32_t error, u_int32_t loglevel,
    u_int32_t rule_id, u_int32_t prio, u_int32_t sfsmatch)
{
	struct anoubisd_msg		*msg;
	struct anoubisd_reply		 reply;

	DEBUG(DBG_TRACE, ">__send_lognotify: rule_id=%d, prio=%d source=%d",
	    rule_id, prio, hdr->msg_source);
	reply.ask = 0;
	reply.reply = error;
	reply.rule_id = rule_id;
	reply.prio = prio;
	reply.sfsmatch = sfsmatch;
	reply.pident = pident;
	reply.ctxident = ctxident;
	reply.log = loglevel;

	msg = fill_eventask_message(ANOUBISD_MSG_LOGREQUEST, hdr, &reply);
	if (!msg) {
		log_warn("send_lognotify: can't allocate memory");
		return;
	}
	enqueue(&eventq_p2s, msg);
	DEBUG(DBG_QUEUE, ">&eventq_p2s");
	DEBUG(DBG_QUEUE, "<__send_lognotify");
}

/**
 * Send a log notification to the session engine. This function allocates
 * and fills an event message of type ANOUBISD_MSG_LOGREQUEST with the
 * data from the arguments and the given kernel event. The message is
 * then sent to the session engine.
 *
 * @param proc The identifiers for the process and the context are
 *     derived from this process.
 * @param hdr The raw kernel event.
 * @param error The reply to the event.
 * @param loglevel The loglevel according to the policy.
 * @param rule_id The rule ID that triggered the log event.
 * @param prio the priority of the ruleset that triggered the log event.
 * @param sfsmatch For log events triggered by an SFS rule this is the
 *     part of the SFS rule that actually matched.
 */
void
send_lognotify(struct pe_proc *proc, struct eventdev_hdr *hdr,
    u_int32_t error, u_int32_t loglevel, u_int32_t rule_id, u_int32_t prio,
    u_int32_t sfsmatch)
{
	struct pe_proc_ident	*pident = NULL;
	struct pe_proc_ident	*ctxident = NULL;
	if (proc) {
		pident = pe_proc_ident(proc);
		ctxident = pe_context_get_ident(
		    pe_proc_get_context(proc, prio));
	}
	return __send_lognotify(pident, ctxident, hdr, error, loglevel,
	    rule_id, prio, sfsmatch);
}

/**
 * Send a message that informs the user of a modified policy.
 *
 * @param The user ID of the modified policy.
 * @param prio The priority of the modified policy.
 */
void
send_policychange(u_int32_t uid, u_int32_t prio)
{
	struct anoubisd_msg		*msg;
	struct anoubisd_msg_pchange	*pchange;

	msg = msg_factory(ANOUBISD_MSG_POLICYCHANGE, sizeof(*pchange));
	if (!msg) {
		log_warn("send_lognotify: can't allocate memory");
		return;
	}
	pchange = (struct anoubisd_msg_pchange*)msg->msg;
	pchange->uid = uid;
	pchange->prio = prio;
	enqueue(&eventq_p2s, msg);
}

/**
 * Send the contents of a file to the session engine. The content is
 * broken into one or more messages of type ANOUBISD_MSG_POLREPLY.
 *
 * @param The token to use for the messages.
 * @param fd A file descriptor of the policy file. The contents of this
 *     file are sent to the daemon.
 */
int
send_policy_data(u_int64_t token, int fd)
{
	struct anoubisd_msg		*msg;
	struct anoubisd_msg_polreply	*polreply;
	int				 flags = POLICY_FLAG_START;
	int				 size;

	size = sizeof(struct anoubisd_msg_polreply) + 3000;
	while(1) {
		msg = msg_factory(ANOUBISD_MSG_POLREPLY, size);
		if (!msg)
			goto oom;
		polreply = (struct anoubisd_msg_polreply *)msg->msg;
		polreply->token = token;
		polreply->reply = 0;
		polreply->flags = flags;
		// cppcheck-suppress bufferAccessOutOfBounds
		polreply->len = read(fd, polreply->data, 3000);
		if (polreply->len < 0) {
			int ret = -errno;
			free(msg);
			return ret;
		}
		msg_shrink(msg,
		    sizeof(struct anoubisd_msg_polreply) + polreply->len);
		if (polreply->len == 0) {
			polreply->flags |= POLICY_FLAG_END;
			enqueue(&eventq_p2s, msg);
			break;
		}
		enqueue(&eventq_p2s, msg);
		flags = 0;
	}
	return 0;
oom:
	log_warn("send_policy_data: can't allocate memory");
	master_terminate();
}

/**
 * Send a playground change message to the session engine.
 *
 * @param uid The user Id of the playground owner.
 * @param pgid The playground ID of the playground.
 * @param pgop The playground operation.
 * @param cmd The command executed in the playground
 */
void
send_pgchange(unsigned int uid, anoubis_cookie_t pgid, unsigned int pgop,
    const char *cmd)
{
	struct anoubisd_msg		*msg;
	struct anoubisd_msg_pgchange	*pgmsg;
	int				 cmdlen = 0;

	DEBUG(DBG_PG, "send_pgchange: uid=%d pgid=%llx pgop=%d cmd=%s",
	    uid, (long long)pgid, pgop, cmd);

	if (cmd != NULL)
		cmdlen = strlen(cmd);

	msg = msg_factory(ANOUBISD_MSG_PGCHANGE,
	    sizeof(struct anoubisd_msg_pgchange) + cmdlen + 1);
	if (msg == NULL)
		return;
	pgmsg = (struct anoubisd_msg_pgchange *)msg->msg;
	pgmsg->uid = uid;
	pgmsg->pgid = pgid;
	pgmsg->pgop = pgop;
	strcpy(pgmsg->cmd, cmd);
	enqueue(&eventq_p2s, msg);
}

/**
 * This function handles list requests received from the session engine,
 * i.e. the user.  The request message is passed as a paramter. Any response
 * messages that must be generated are added to the given queue.
 * The user is notified of any error via error messages.
 *
 * @param inmsg The request message of type anoubisd_msg_listrequest.
 * @param queue Reply messages are added to this queue.
 */
static void
dispatch_list_request(struct anoubisd_msg *inmsg, Queue *queue)
{
	int				 err = 0;
	uint64_t			 token = 0;
	struct anoubisd_msg_listrequest	*listreq;
	struct amsg_list_context	 ctx;

	listreq = (struct anoubisd_msg_listrequest *)inmsg->msg;
	token = listreq->token;
	switch(listreq->listtype) {
	case ANOUBIS_REC_PGLIST:
		err = pe_playground_send_pglist(listreq->token, listreq->arg,
		    queue);
		break;
	case ANOUBIS_REC_PGFILELIST:
		err = pe_playground_send_filelist(listreq->token, listreq->arg,
		    listreq->auth_uid, queue);
		break;
	case ANOUBIS_REC_PROCLIST:
		err = pe_proc_send_pslist(listreq->token, listreq->arg,
		    listreq->auth_uid, queue);
		break;
	default:
		log_warnx("pe_playground_dispatch_request: Dropping invalid "
		    "message of type %d", listreq->listtype);
	}
	if (err == 0)
		return;
	/* Send an error message */
	if (amsg_list_init(&ctx, token, ANOUBIS_REC_NOTYPE) < 0)
		master_terminate();
	ctx.flags = POLICY_FLAG_START | POLICY_FLAG_END;
	set_value(ctx.pmsg->error, -err);
	amsg_list_send(&ctx, queue);
}

/**
 * Handle incoming messages received from the session engine.
 * Possible message types are:
 * ANOUBISD_MSG_POLREQUEST: A request to change or get a particular
 *     policy. Handled by pe_dispatch_policy.
 * ANOUBISD_MSG_EVENTREPLY: Replies to event escalations.
 * ANOUBISD_MSG_LISTREQUEST: A user requested a list of items (playgrounds,
 *     processes, etc.). Handled by dispatch_list_request.
 * ANOUBISD_MSG_PGCOMMIT: A playground commit request. This is usually
 *     forwarded to the master but the playground management must keep
 *     track of these requests, too.
 * If EOF is detected on the incoming file descriptor, the termination
 * status advances to stage 3.
 *
 * @param fd The file descriptor for incoming messages.
 * @param sig The details of the event (unused).
 * @param arg The callback argument of the event (unused).
 */
static void
dispatch_s2p(int fd, short sig __used, void *arg __used)
{
	struct reply_wait			*rep_wait;
	struct eventdev_reply			*evrep;
	struct anoubisd_msg			*msg;
	struct anoubisd_msg			*msg_rep;

	DEBUG(DBG_TRACE, ">dispatch_s2p");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL)
			break;

		switch (msg->mtype) {
		case ANOUBISD_MSG_POLREQUEST:
		case ANOUBISD_MSG_POLREQUEST_ABORT:
			msg_rep = pe_dispatch_policy(msg);
			if (msg_rep)
				enqueue(&eventq_p2s, msg_rep);
			break;

		case ANOUBISD_MSG_EVENTREPLY:

			evrep = (struct eventdev_reply *)msg->msg;
			TAILQ_FOREACH(rep_wait, &replyq, next) {
				if (rep_wait->token == evrep->msg_token)
					break;
			}
			if (rep_wait != NULL) {
				/*
				 * Only send message if still in queue. It
				 * might have already been replied to by a
				 * timeout or other GUI
				 */
				TAILQ_REMOVE(&replyq, rep_wait, next);
				DEBUG(DBG_QUEUE, " <replyq: %x error=%d",
				    rep_wait->token, evrep->reply);
				switch(rep_wait->log) {
				case APN_LOG_NORMAL:
					log_info("token %u: Reply %d",
					    evrep->msg_token, evrep->reply);
					break;
				case APN_LOG_ALERT:
					log_warnx("token %u: Reply %d",
					    evrep->msg_token, evrep->reply);
					break;
				}
				/* restore flags set via APN */
				evrep->reply |= rep_wait->flags;
				abuf_free_type(rep_wait, struct reply_wait);
				DEBUG(DBG_QUEUE, " >eventq_p2m: %x error=%d",
					evrep->msg_token, evrep->reply);
				enqueue(&eventq_p2m, msg);
				/* Prevent freeing of message below. */
				msg = NULL;
			} else {
				DEBUG(DBG_QUEUE, " reply not found for 0x%x",
				    evrep->msg_token);
			}
			break;
		case ANOUBISD_MSG_LISTREQUEST:
			dispatch_list_request(msg, &eventq_p2s);
			break;
		case ANOUBISD_MSG_PGCOMMIT:
			/* NOTE: This functions frees or reuses the message. */
			pe_playground_dispatch_commit(msg, &eventq_p2s,
			    &eventq_p2m);
			msg = NULL;
			break;
		case ANOUBISD_MSG_PGUNLINK:
			pe_playground_dispatch_unlink(msg, &eventq_p2s);
			break;
		default:

			DEBUG(DBG_TRACE, "dispatch_s2p: msg type %d",
			    msg->mtype);
		}
		if (msg)
			free(msg);
	}
	if (msg_eof(fd)) {
		event_del(&ev_s2p);
		set_terminate(3);
	}
	DEBUG(DBG_TRACE, "<dispatch_s2p");
}

/**
 * This is the event handler for write events on the outgoing pipe to
 * the session engine. If the outgoing queue is empty and the termination
 * state is at least 2 the write end of the pipe is shut down.
 *
 * @param fd The file descriptor for outgoing messages.
 * @param sig The details of the event (unused).
 * @param arg The callback argument of the event (unused).
 */
static void
dispatch_p2s(int fd, short sig __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_p2s");
	dispatch_write_queue(&eventq_p2s, fd);
	if (terminate >= 2 && !queue_peek(&eventq_p2s) && !msg_pending(fd))
		shutdown(fd, SHUT_WR);
	DEBUG(DBG_TRACE, "<dispatch_p2s");
}
