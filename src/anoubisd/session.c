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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/param.h>
#include <sys/time.h>
#include <sys/un.h>
#include <err.h>
#include <errno.h>
#include <event.h>
#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#endif
#ifdef LINUX
#include <grp.h>
#include <bsdcompat.h>
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#endif
#include <sys/queue.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>


#include <anoubis_chat.h>
#include <anoubis_msg.h>
#include <anoubis_server.h>
#include <anoubis_notify.h>
#include <anoubis_policy.h>
#include <anoubis_dump.h>
#include <anoubis_errno.h>

#include "anoubisd.h"
#include "aqueue.h"
#include "amsg.h"
#include "cfg.h"

/**
 * This structure describes a single user session, i.e. a connection via
 * the anoubis daemon socket. All active user connections are linked in
 * a linked list.
 */
struct session {
	/**
	 * This field is used to link allsessions in the global
	 * session list.
	 */
	LIST_ENTRY(session)	 nextSession;

	/**
	 * The authenticated user ID of the session. If the session is not
	 * authenticated, this value is (uid_t)-1.
	 */
	uid_t			 uid;

	/**
	 * The saved file descriptor of the connection. This usually
	 * corresponds to session->channel->fd but session->channel->fd might
	 * be -1 before we have a change to clean up the message buffers.
	 */
	int			 connfd;

	/**
	 * The communication channel for the session.
	 */
	struct achat_channel	*channel;

	/**
	 * The server protocol object that handles this sesison.
	 */
	struct anoubis_server	*proto;

	/**
	 * This event is used to wait for incoming data on the session.
	 */
	struct event		 ev_rdata;

	/**
	 * This event to wait until the file descriptor become ready to
	 * write outgoing data.
	 */
	struct event		 ev_wdata;
};

/**
 * Events for incoming messages from the master and the policy engine.
 */
static struct event			 ev_m2s, ev_p2s;

/**
 * Message queues for outgoing events to the session engine and the master
 * process.
 */
static Queue				 eventq_s2p, eventq_s2m;

/**
 * The gloal policy communication object. This is used to multiplex
 * and demultiplex events from multiple session to the policy engine
 * or the master process.
 */
static struct anoubis_policy_comm	*policy_comm;

/**
 * Storage from events that are used by signal handlers. The
 * array might not be used completely. These events are removed from
 * the event loop during shutdown.
 */
static struct event			 *ev_sigs[10];

/**
 * The head of the global session list.
 */
static LIST_HEAD(slHead, session)	 sessionList;

/**
 * The channel that is used to listen from incoming connections.
 */
static struct achat_channel		*listening_socket;

/**
 * The event that is used to wait for incoming connections in the
 * global event loop.
 */
static struct event			 ev_connect;

/**
 * This structure encapsulates callback data related to a pending
 * escalation message. Only ask events that need a reply are tracked
 * like this. There are two cases where this data is used:
 * - The protocol callback handler uses it to process a reply from
 *     the user. In particular this allows the sessoin engine to reliably
 *     store the kernel eventdev toekn.
 * - If the policy engine decides to cancel an event (timeout) it send
 *     a message to the session engine which uses this data structure to
 *     free resources associated with the event in the session engine.
 * Fields:
 * next: All aktive events are linked in a global list via this field.
 * ev_token: The eventdev token as received from the kernel.
 * ev_head: The notify head for the escalation event as described in
 *     the anoubis_notify man page.
 */
struct cbdata {
	TAILQ_ENTRY(cbdata)		 next;
	eventdev_token			 ev_token;
	struct anoubis_notify_head	*ev_head;
};

/**
 * All active escalations are stored in this list (one struct cbdata
 * for each, linked via the next field.
 */
static TAILQ_HEAD(, cbdata)		headq;



/* Prototypes. */
static int	dispatch_policy(struct anoubis_policy_comm *, u_int64_t,
		    u_int32_t, const void *, size_t, void *, int);
static void	dispatch_authdata(struct anoubis_server *,
		    struct anoubis_msg *, uid_t auth_uid, void *);
static void	dispatch_checksum(struct anoubis_server *server,
		    struct anoubis_msg *msg, uid_t uid, void *arg);
static int	dispatch_checksum_reply(void *cbdata, int error,
		    const void *data, int len, int end);
static int	dispatch_checksum_list_reply(void *cbdata, int error,
		    const void *data, int len, int end);
static void	dispatch_csmulti(struct anoubis_server *,
		    struct anoubis_msg *, uid_t, void *);
static void	dispatch_list_request(struct anoubis_server *,
		    struct anoubis_msg *, uid_t, void *);
static void	dispatch_pgcommit(struct anoubis_server *,
		    struct anoubis_msg *, uid_t, void *);
static int	dispatch_csmulti_reply(void *, int, const void *, int, int);
static int	dispatch_list_reply(void *, int, const void *, int, int);
static void	dispatch_m2s(int, short, void *);
static void	dispatch_s2m(int, short, void *);
static void	dispatch_s2p(int, short, void *);
static void	dispatch_p2s(int, short, void *);
static void	dispatch_p2s_evt_request(const struct anoubisd_msg *);
static void	dispatch_p2s_log_request(const struct anoubisd_msg *);
void		dispatch_p2s_policychange(const struct anoubisd_msg *);
static void	dispatch_p2s_evt_cancel(const struct anoubisd_msg *);
static void	dispatch_p2s_pol_reply(const struct anoubisd_msg *);
static void	dispatch_p2s_list_reply(const struct anoubisd_msg *);
static void	dispatch_p2s_commit_reply(const struct anoubisd_msg *msg);
static void	dispatch_m2s_checksum_reply(const struct anoubisd_msg *msg);
static void	dispatch_m2s_upgrade_notify(const struct anoubisd_msg *msg);
static void	session_rxclient(int, short, void *);
static void	session_txclient(int, short, void *);
static struct achat_channel*	setup_listening_socket(void);
static void	session_destroy(struct session *);
static int	dispatch_generic_reply(void *cbdata, int error,
		    const void *data, int len, int orig_opcode);
static void	dispatch_passphrase(struct anoubis_server *,
		    struct anoubis_msg *, uid_t, void *);

/**
 * Signal handler for signal related events in the session engine.
 * SIGINT and SIGTERM trigger a graceful shutdown, SIGQUIT exits the
 * event loop immediately.
 *
 * Graceful shutdown blocks all further signals, forces termination of
 * all client sessions and removes all signal related events from the
 * event loop. The latter is neccessary to make sure that the event
 * loop terminates eventually (with no more pending events left).
 *
 * Note that this signal handler is sometimes called manually (without
 * an actual signal). This simulates a signal and initiates graceful
 * shutdown, too. Also note that this handler is called in response to
 * a signal by the event loop. It is not called from signal context
 * directly.
 *
 * @param sig The signal that was caught.
 * @param event The event (unused).
 * @param arg Callback argument (unused)
 * @return None.
 */
static void
session_sighandler(int sig, short event __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">session_sighandler");
	switch (sig) {
	case SIGINT:
	case SIGTERM: {
		struct session			*sess;
		sigset_t			 mask;
		int				 i;

		event_del(&ev_connect);
		while ((sess = LIST_FIRST(&sessionList)))
			session_destroy(sess);
		/* We are terminating: Deregister signal handlers. */
		sigfillset(&mask);
		sigprocmask(SIG_SETMASK, &mask, NULL);
		for (i=0; ev_sigs[i]; ++i) {
			signal_del(ev_sigs[i]);
		}
		break;
	}
	case SIGQUIT:
		(void)event_loopexit(NULL);
	}
}

/**
 * This function is called in response to activity on the listening
 * socket (i.e. a new client connection comes in). The corresponding
 * event is ev_connect. This function accepts the new session creates
 * and initializes a new session object and inserts it into the global
 * session list.
 *
 * @param fd The file of the event (unused, always listening->socket->fd).
 * @param event The type of the event (unused).
 * @param arg The callback argument (unused).
 * @return None.
 */
static void
session_connect(int fd __used, short event __used, void *arg __used)
{
	struct session			*session = NULL;

	DEBUG(DBG_TRACE, ">session_connect");

	session = calloc(1, sizeof(struct session));
	if (session == NULL) {
		log_warn("session_connect: calloc");
		DEBUG(DBG_TRACE, "<session_connect (calloc)");
		return;
	}

	session->uid = -1; /* this session is not authenticated */

	/*
	 * The listening channel is non-blocking which will be inherited
	 * by the new channel on opendup. Thus there is no need to set the
	 * new channel to non-blocking. Beside this wouldn't work anyway
	 * because after acc_opendup it is too late.
	 */
	session->channel = acc_opendup(listening_socket);
	if (session->channel == NULL) {
		log_warn("session_connect: acc_opendup");
		free(session);
		DEBUG(DBG_TRACE, "<session_connect (opendup)");
		return;
	}
	session->connfd = session->channel->fd;
	/* Limit number of connections per user. */
	if (session->channel->euid != 0 &&
	    session->channel->euid != (uid_t) -1) {
		int conn_count = 0;
		struct session *sp;
		LIST_FOREACH(sp, &sessionList, nextSession) {
			if (sp->channel->euid != session->channel->euid)
				continue;
			if (++conn_count > ANOUBISD_MAX_CONNS_PER_USER) {
				log_warnx("Connection limit reached for uid %d",
				    session->channel->euid);
				acc_close(session->channel);
				acc_destroy(session->channel);
				free(session);
				DEBUG(DBG_TRACE, "<session_connect (create)");
				return;
			}
		}
	}

	session->proto = anoubis_server_create(session->channel, policy_comm);
	if (session->proto == NULL) {
		log_warnx("cannot create server protocol handler");
		acc_close(session->channel);
		acc_destroy(session->channel);
		free(session);
		DEBUG(DBG_TRACE, "<session_connect (create)");
		return;
	}
	anoubis_dispatch_create(session->proto, ANOUBIS_P_CSUMREQUEST,
	    &dispatch_checksum, NULL);
	anoubis_dispatch_create(session->proto, ANOUBIS_P_CSMULTIREQUEST,
	    &dispatch_csmulti, NULL);
	anoubis_dispatch_create(session->proto, ANOUBIS_P_PASSPHRASE,
	    &dispatch_passphrase, NULL);
	anoubis_dispatch_create(session->proto, ANOUBIS_P_LISTREQ,
	    &dispatch_list_request, NULL);
	anoubis_dispatch_create(session->proto, ANOUBIS_P_PGCOMMIT,
	    &dispatch_pgcommit, NULL);
	anoubis_dispatch_create(session->proto, ANOUBIS_C_AUTHDATA,
	    &dispatch_authdata, NULL);
	LIST_INSERT_HEAD(&sessionList, session, nextSession);
	if (anoubis_server_start(session->proto) < 0) {
		log_warn("Failed to send initial hello");
		/* Removes session from session List. */
		session_destroy(session);
		DEBUG(DBG_TRACE, "<session_connect (start)");
		return;
	}

	event_set(&(session->ev_rdata), session->channel->fd,
	    EV_READ | EV_PERSIST, session_rxclient, session);
	event_set(&(session->ev_wdata), session->channel->fd,
	    EV_WRITE, session_txclient, session);
	event_add(&(session->ev_rdata), NULL);
	session->channel->event = &session->ev_wdata;
	msg_init(session->channel->fd);

	DEBUG(DBG_TRACE, "<session_connect");
}

/**
 * This function is called by the event loop when data can be read
 * from a session file descriptor. It tries to read messages from
 * the descriptor until EOF is reached and processes them.
 *
 * @param fd The file descriptor (unused, equal to session->channel->fd)
 * @param event The type of event (unused)
 * @param arg The callback argument. This is the session structure
 *     for the file descriptor.
 * @return None.
 */
static void
session_rxclient(int fd __used, short event __used, void *arg)
{
	struct session		*session = arg;
	struct anoubis_msg	*m;
	int			 ret;

	DEBUG(DBG_TRACE, ">session_rxclient");
	while(1) {
		ret = get_client_msg(session->channel->fd, &m);
		if (ret == 0) {
			session_destroy(session);
			DEBUG(DBG_TRACE, "<session_rxclient (receivemsg)");
			return;
		}
		if (ret < 0)
			goto err;
		/* Only incomplete message. Nothing to do */
		if (!m) {
			DEBUG(DBG_TRACE,
			    " session_rxclient: Incomplete message");
			break;
		} else {
			DEBUG(DBG_TRACE,
			    " session_rxclient: got message (size=%d)",
			    m->length);
		}
		/* At this point we actually got a message. */
		if (!session->proto)
			goto err;
		/*
		 * Return codes: less than zero is an error. Zero means no
		 * error but message did not fit into protocol stream.
		 */
		ret = anoubis_server_process(session->proto,
		    m->u.buf, m->length);
		if (ret < 0)
			goto err;
		anoubis_msg_free(m);
		if (anoubis_server_eof(session->proto)) {
			session_destroy(session);
			break;
		}
	}
	DEBUG(DBG_TRACE, "<session_rxclient");
	return;
err:
	if (m)
		anoubis_msg_free(m);
	session_destroy(session);
	log_warnx("session_rxclient: error processing client message %d", ret);

	DEBUG(DBG_TRACE, "<session_rxclient (error)");
}

/**
 * This event handler is called when outgoing data is pending on a
 * client session file descriptor. It flushes as much data as possible
 * without blocking and re-adds the event if more data is pending.
 *
 * @param fd The file descriptort (unused, equal to sess->channel->fd)
 * @param event The type of the event (unused)
 * @param arg The callback argument. This is the session structure for
 *     the current session.
 * @return None.
 */
static void
session_txclient(int fd __used, short event __used, void *arg)
{
	struct session	*sess = arg;
	/* acc_flush will re-add the event if needed. */
	acc_flush(sess->channel);
}

/**
 * This is the main entry point for the session engine. The master
 * calls this function to start the session process. This function
 * spaws a child process and sets up listening socket for client
 * connections, communication pipes and event and signal handling.
 * Finally, it enters the main event loop. For the parent process, this
 * function is a NOOP. In particular, no file descriptors are closed
 * in the parent process.
 *
 * @param pipes This array contains all pipe related file descriptors,
 *     two descriptors per pipe. Defines like PIPE_MAIN_SESSION should
 *     be used to access this array. This function uses the file
 *     descriptors at index PIPE_MAIN_SESSION+1 and PIPE_SESSION_POLICY.
 *     All other descriptors are closed in the child.
 * @param loggers This array contains the log file descriptors for the
 *     pipes from the other anoubis processes to the logger. It is indexed
 *     by process identification, i.e. we use the file descriptor at
 *     index PROC_SESSION. All other file descriptors are closed in
 *     the child.
 * @return In the parent process the process ID of the child process is
 *     returned. The child process never returns from this function.
 */
pid_t
session_main(int pipes[], int loggers[])
{
	int				 masterfd, policyfd, logfd;
	struct event			 ev_sigterm, ev_sigint, ev_sigquit;
	struct event			 ev_s2m, ev_s2p;
	struct passwd			*pw;
	sigset_t			 mask;
	pid_t				 pid;


	pid = fork();
	if (pid == -1)
		fatal("fork");
	if (pid)
		return pid;

	(void)event_init();

	anoubisd_process = PROC_SESSION;

	masterfd = policyfd = logfd = -1;
	SWAP(masterfd, pipes[PIPE_MAIN_SESSION + 1]);
	SWAP(policyfd, pipes[PIPE_SESSION_POLICY]);
	SWAP(logfd, loggers[anoubisd_process]);
	cleanup_fds(pipes, loggers);

	log_init(logfd);

	/* while still privileged we install a listening socket */
	listening_socket = setup_listening_socket();
#ifdef LINUX
	dazukofs_ignore();
#endif

	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		fatal("getpwnam");

	if (chroot(pw->pw_dir) == -1)
		fatal("chroot");
	if (chdir("/") == -1)
		fatal("chdir");
	log_info("session started (pid %d root %s)", getpid(), pw->pw_dir);

	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		fatal("can't drop privileges");


	/* From now on, this is an unprivileged child process. */
	LIST_INIT(&sessionList);
	TAILQ_INIT(&headq);

	/* We catch or block signals rather than ignoring them. */
	signal_set(&ev_sigterm, SIGTERM, session_sighandler, NULL);
	signal_set(&ev_sigint, SIGINT, session_sighandler, NULL);
	signal_set(&ev_sigquit, SIGQUIT, session_sighandler, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);
	ev_sigs[0] = &ev_sigterm;
	ev_sigs[1] = &ev_sigint;
	ev_sigs[2] = &ev_sigquit;
	ev_sigs[3] = NULL;

	anoubisd_defaultsigset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	/* init message buffers. */
	msg_init(masterfd);
	msg_init(policyfd);

	/* master process */
	event_set(&ev_m2s, masterfd, EV_READ | EV_PERSIST, dispatch_m2s, NULL);
	event_add(&ev_m2s, NULL);

	event_set(&ev_s2m, masterfd, EV_WRITE, dispatch_s2m, NULL);

	/* policy process */
	event_set(&ev_p2s, policyfd, EV_READ | EV_PERSIST, dispatch_p2s, NULL);
	event_add(&ev_p2s, NULL);

	event_set(&ev_s2p, policyfd, EV_WRITE, dispatch_s2p, NULL);

	queue_init(&eventq_s2p, &ev_s2p);
	queue_init(&eventq_s2m, &ev_s2m);

	policy_comm = anoubis_policy_comm_create(&dispatch_policy, NULL);
	if (policy_comm == NULL)
		fatal("Cannot create policy object (out of memory)");

	/* setup socket for incoming unix domain socket connections */
	if (listening_socket != NULL) {
		event_set(&ev_connect, listening_socket->fd,
		    EV_READ | EV_PERSIST, session_connect, NULL);
		event_add(&ev_connect, NULL);
	}

	setproctitle("session engine");

	DEBUG(DBG_TRACE, "session event loop");
	if (event_dispatch() == -1)
		fatal("session_main: event_dispatch");
	DEBUG(DBG_TRACE, "event loop done");

	/* stop events of incoming connects and cleanup sessions */
	event_del(&ev_connect);
	while (!LIST_EMPTY(&sessionList)) {
		struct session *session = LIST_FIRST(&sessionList);

		log_warnx("session_main: closing remaining session by force");
		/* session_destroy will call LIST_REMOVE. */
		session_destroy(session);
	}
	if (listening_socket)
		acc_destroy(listening_socket);

	_exit(0);
}

/**
 * This function handles replies to notification. It is usually called
 * from the protocol object as a callback whenever a session sends a
 * reply to a privous event. However, it may be called manually, too.
 * This happens if the policy engine decides that the event has timed
 * out.
 *
 * @param head The notification head may be NULL if the head is already
 *     destroyed!
 * @param verdict The error code for the event, zero if the event is
 *     allowed. This result must be sent to the policy engine.
 * @param _cbdata The callback data structure for the event. It contains
 *     a pointer back to the head and the kernel token for the event.
 * @return None.
 */

static void
notify_callback(struct anoubis_notify_head *head, int verdict, void *_cbdata)
{
	struct anoubisd_msg		*msg;
	struct eventdev_reply		*rep;
	struct cbdata			*cbdata = _cbdata;

	DEBUG(DBG_TRACE, ">notify_callback");

	if (cbdata == NULL) {
		log_warnx("notify_callback: null pointer");
		master_terminate();
	}

	msg = msg_factory(ANOUBISD_MSG_EVENTREPLY,
	    sizeof(struct eventdev_reply));
	if (!msg)
		master_terminate();
	rep = (struct eventdev_reply *)msg->msg;
	rep->msg_token = cbdata->ev_token;
	rep->reply = verdict;

	enqueue(&eventq_s2p, msg);
	DEBUG(DBG_QUEUE, " >eventq_s2p: %x", rep->msg_token);

	if (head) {
		anoubis_notify_destroy_head(head);
		DEBUG(DBG_TRACE, " >anoubis_notify_destroy_head");
		DEBUG(DBG_QUEUE, " <headq: %x reply=%d", rep->msg_token,
		    rep->reply);
		TAILQ_REMOVE(&headq, cbdata, next);
	}
	free(cbdata);

	DEBUG(DBG_TRACE, "<notify_callback");
}

/**
 * Send a notification message (i.e. a message that does not expect a
 * reply) to all interested (and authorized) user session. This function
 * creates the notification head and iterates over all sessions sending
 * the message if appropriate.
 *
 * @param m The notification message.
 * @return None.
 */
static void
__send_notify(struct anoubis_msg *m)
{
	struct session			*sess;
	struct anoubis_notify_head	*head;

	head = anoubis_notify_create_head(m, NULL, NULL);
	if (!head) {
		/* malloc failure, then we don't send the message */
		anoubis_msg_free(m);
		DEBUG(DBG_TRACE, "<__send_notify (free)");
		return;
	}
	DEBUG(DBG_TRACE, " >anoubis_notify_create_head");

	LIST_FOREACH(sess, &sessionList, nextSession) {
		struct anoubis_notify_group * ng;

		if (sess->proto == NULL)
			continue;
		ng = anoubis_server_getnotify(sess->proto);
		if (!ng)
			continue;
		anoubis_notify(ng, head, ANOUBISD_MAX_PENDNG_EVENTS);
	}
	anoubis_notify_destroy_head(head);
	DEBUG(DBG_TRACE, " >anoubis_notify_destroy_head");
}

/**
 * This is the dispatcher function for ANOUBIS_P_CSMULTIREQUEST protocol
 * requests received from a user session. It translates the request into
 * a ANOUBISD_MSG_CSMULTIREQUEST daemon message and forwards it to
 * the anoubis daemon master process. policy_comm is used to track
 * pending requests.
 *
 * @param server The server object associated with the client session.
 * @param m The request message as received from the client.
 * @param uid The authenticated user ID of the client seesion.
 * @param arg The callback argument (unused).
 */
static void
dispatch_csmulti(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t uid, void *arg __used)
{
	struct anoubisd_msg		*s2m_msg;
	struct anoubisd_msg_csumop	*csum_msg;
	struct achat_channel		*chan;
	int				 err, reallen;

	DEBUG(DBG_TRACE, ">dispatch_csmulti");
	reallen = amsg_sfs_checksumop_size(m->u.buf, m->length);
	if (reallen < 0 || reallen > (int)m->length)
		goto invalid;
	if (reallen + 8 < (int)m->length)
		goto invalid;
	s2m_msg = msg_factory(ANOUBISD_MSG_CSMULTIREQUEST,
	    sizeof(struct anoubisd_msg_csumop) + m->length);
	if (!s2m_msg)
		master_terminate();
	chan = anoubis_server_getchannel(server);
	csum_msg = (struct anoubisd_msg_csumop *)(s2m_msg->msg),
	csum_msg->uid = uid;
	csum_msg->len = m->length;
	memcpy(csum_msg->msg, m->u.buf, m->length);
	err = anoubis_policy_comm_addrequest(policy_comm, chan,
	    POLICY_FLAG_START | POLICY_FLAG_END,
	    &dispatch_csmulti_reply, NULL, server,
	    &csum_msg->token);
	if (err < 0) {
		log_warnx("Dropping checksum request (error %d)", err);
		free(s2m_msg);
		DEBUG(DBG_TRACE, "<dispatch_csmulti (error %d)", err);
		return;
	}
	enqueue(&eventq_s2m, s2m_msg);
	DEBUG(DBG_QUEUE, " >eventq_s2m");
	DEBUG(DBG_TRACE, "<dispatch_csmulti");
	return;

invalid:
	dispatch_csmulti_reply(server, EINVAL, NULL, 0,
	    POLICY_FLAG_START | POLICY_FLAG_END);
	DEBUG(DBG_TRACE, "<dispatch_csmulti (EINVAL)");
}

/**
 * This is the dispatcher function for ANOUBIS_P_LISTREQ protocol
 * requests received from a user session. It translates the request into
 * a ANOUBISD_MSG_LISTREQUEST daemon message and forwards it to
 * the policy process. policy_comm is used to track pending requests and
 * demultiplex replies.
 *
 * @param server The server object associated with the client session.
 * @param m The request message as received from the client.
 * @param auth_uid The authenticated user ID of the client seesion.
 * @param arg The callback argument (unused).
 */
static void
dispatch_list_request(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t auth_uid, void *arg __used)
{
	struct anoubisd_msg			*msg;
	struct anoubisd_msg_listrequest		*pgmsg;
	struct achat_channel			*chan;
	int					 err;

	DEBUG(DBG_TRACE, ">dispatch_list_request");

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_ListRequestMessage))) {
		dispatch_list_reply(server, EFAULT, NULL, 0,
		    POLICY_FLAG_START | POLICY_FLAG_END);
		DEBUG(DBG_TRACE, "<dispatch_list_request: verify length");
		return;
	}

	msg = msg_factory(ANOUBISD_MSG_LISTREQUEST,
	    sizeof(struct anoubisd_msg_listrequest));
	if (!msg)
		master_terminate();

	pgmsg = (struct anoubisd_msg_listrequest *)msg->msg;
	pgmsg->auth_uid = auth_uid;
	pgmsg->listtype = get_value(m->u.listreq->listtype);
	pgmsg->arg = get_value(m->u.listreq->arg);
	chan = anoubis_server_getchannel(server);
	err = anoubis_policy_comm_addrequest(policy_comm, chan,
	    POLICY_FLAG_START | POLICY_FLAG_END, &dispatch_list_reply,
	    NULL, server, &pgmsg->token);
	if (err < 0) {
		log_warnx("Dropping list request (error %d)", -err);
		free(msg);
		return;
	}

	enqueue(&eventq_s2p, msg);
	DEBUG(DBG_QUEUE, " >eventq_s2p");
	DEBUG(DBG_TRACE, "<dispatch_list_request");
}

/**
 * This is the dispatcher function for ANOUBIS_P_PGCOMMIT protocol
 * requests received from a user session. It translates the request into
 * an ANOUBIS_P_PGCOMMIT daemon message and forwards it to
 * the policy process. policy_comm is used to track pending requests and
 * demultiplex replies.
 *
 * @param server The server object associated with the client session.
 * @param m The request message as received from the client.
 * @param auth_uid The authenticated user ID of the client seesion.
 * @param arg The callback argument (unused).
 */
static void
dispatch_pgcommit(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t auth_uid, void *arg __used)
{
	struct anoubisd_msg			*msg;
	struct anoubisd_msg_pgcommit		*pgmsg;
	struct achat_channel			*chan;
	int					 err, len;

	DEBUG(DBG_TRACE, ">dispatch_pgcommit");

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_PgCommitMessage))
	    || PAYLOAD_LEN(m, pgcommit, payload) < 2) {
		dispatch_generic_reply(server, EINVAL, NULL, 0,
		    ANOUBIS_P_PGCOMMIT);
		DEBUG(DBG_TRACE, "<dispatch_pgcommit: verify length");
		return;
	}
	/* Force NULL termination on the string. */
	len = PAYLOAD_LEN(m, pgcommit, payload);
	m->u.pgcommit->payload[len-1] = 0;
	len = strlen(m->u.pgcommit->payload) + 1;

	msg = msg_factory(ANOUBISD_MSG_PGCOMMIT,
	    sizeof(struct anoubisd_msg_pgcommit) + len);
	if (!msg)
		master_terminate();

	pgmsg = (struct anoubisd_msg_pgcommit *)msg->msg;
	pgmsg->auth_uid = auth_uid;
	pgmsg->ignore_recommended_scanners =
	    m->u.pgcommit->ignore_recommended_scanners;
	pgmsg->pgid = get_value(m->u.pgcommit->pgid);
	pgmsg->dev = 0;
	pgmsg->ino = 0;
	memcpy(pgmsg->path, m->u.pgcommit->payload, len);
	chan = anoubis_server_getchannel(server);
	err = anoubis_policy_comm_addrequest(policy_comm, chan,
	    POLICY_FLAG_START | POLICY_FLAG_END, &dispatch_generic_reply,
	    NULL, server, &pgmsg->token);
	if (err < 0) {
		log_warnx("Dropping pgcommit request (error %d)", -err);
		free(msg);
		return;
	}

	enqueue(&eventq_s2p, msg);
	DEBUG(DBG_QUEUE, " >eventq_s2p: token=%" PRId64, pgmsg->token);
	DEBUG(DBG_TRACE, "<dispatch_pgcommit");
}

/**
 * This is the dispatcher function for ANOUBIS_P_CSUMREQUEST protocol
 * requests received from a user session. It translates the request into
 * an ANOUBISD_MSG_CHECKSUM_OP daemon message and forwards it to
 * the master process. policy_comm is used to track pending requests and
 * demultiplex replies. The callback function for replies depends on the
 * type of the checksum operation.
 *
 * @param server The server object associated with the client session.
 * @param m The request message as received from the client.
 * @param uid The authenticated user ID of the client seesion.
 * @param arg The callback argument (unused).
 */
static void
dispatch_checksum(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t uid, void *arg __used)
{
	struct anoubisd_msg		*s2m_msg;
	struct anoubisd_msg_csumop	*msg_csum;
	struct achat_channel		*chan;
	int				 err, opp = 0, reallen;


	DEBUG(DBG_TRACE, ">dispatch_checksum");
	if (!VERIFY_FIELD(m, checksumrequest, operation))
		goto invalid;
	opp = get_value(m->u.checksumrequest->operation);
	reallen = amsg_sfs_checksumop_size(m->u.buf, m->length);
	if (reallen < 0 || reallen > (int)m->length)
		goto invalid;
	if (reallen + 8 < (int)m->length)
		goto invalid;
	chan = anoubis_server_getchannel(server);
	s2m_msg = msg_factory(ANOUBISD_MSG_CHECKSUM_OP,
	    sizeof(struct anoubisd_msg_csumop) + m->length);
	if (!s2m_msg)
		master_terminate();
	msg_csum = (struct anoubisd_msg_csumop *)s2m_msg->msg;
	msg_csum->uid = uid;
	msg_csum->len = m->length;
	memcpy(msg_csum->msg, m->u.checksumrequest, m->length);
	if (opp == ANOUBIS_CHECKSUM_OP_GENERIC_LIST) {
		err = anoubis_policy_comm_addrequest(policy_comm, chan,
		    POLICY_FLAG_START | POLICY_FLAG_END,
		    &dispatch_checksum_list_reply, NULL, server,
		    &msg_csum->token);
	} else {
		err = anoubis_policy_comm_addrequest(policy_comm, chan,
		    POLICY_FLAG_START | POLICY_FLAG_END,
		    &dispatch_checksum_reply, NULL, server, &msg_csum->token);
	}
	if (err < 0) {
		log_warnx("Dropping checksum request (error %d)", err);
		free(s2m_msg);
		return;
	}
	enqueue(&eventq_s2m, s2m_msg);
	DEBUG(DBG_QUEUE, " >eventq_s2m");
	DEBUG(DBG_TRACE, "<dispatch_checksum");
	return;
invalid:
	dispatch_generic_reply(server, EINVAL, NULL, 0, opp);
}

/**
 * Forward the passphrase for root's public key to the master process.
 * The administrator can do this before an update if root signed
 * signatures should be updated automatically in the cause of the update.
 * This function is called in response to an ANOUBIS_P_PASSPHRASE request.
 * The passphrase is stored in an ANOUBISD_MSG_PASSPHRASE message and
 * forwarded to the master. No reply messages are generated.
 *
 * @param server The server object associated with the client session.
 * @param m The request message as received from the client.
 * @param auth_uid The authenticated user ID of the client seesion.
 * @param arg The callback argument (unused).
 */
static void
dispatch_passphrase(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t auth_uid, void *arg __used)
{
	int				 plen;
	struct anoubisd_msg		*msg;
	anoubisd_msg_passphrase_t	*pass;

	DEBUG(DBG_TRACE, ">dispatch_passphrase");
	if (auth_uid != 0) {
		dispatch_generic_reply(server, EPERM, NULL, 0,
		    ANOUBIS_P_PASSPHRASE);
		log_warnx("Non root user %d tried to set passphrase", auth_uid);
		return;
	}
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_PassphraseMessage))) {
		dispatch_generic_reply(server, EFAULT, NULL, 0,
		    ANOUBIS_P_PASSPHRASE);
		DEBUG(DBG_TRACE, "<dispatch_passphrase(error): verify length");
		return;
	}
	plen = m->length - CSUM_LEN - sizeof(Anoubis_PassphraseMessage);
	if (plen < 1) {
		dispatch_generic_reply(server, EFAULT, NULL, 0,
		    ANOUBIS_P_PASSPHRASE);
		DEBUG(DBG_TRACE, "<dispatch_passphrase(error): plen < 1");
		return;
	}
	m->u.passphrase->payload[plen-1] = 0;
	plen = strlen(m->u.passphrase->payload) + 1;
	/*
	 * NOTE: We report success at this point and do not try to send an
	 * NOTE: error message back to the user if anything else fails in
	 * NOTE: the master. This prevents stuff like brute force attacks
	 * NOTE: on the passphrase.
	 */
	dispatch_generic_reply(server, 0, NULL, 0, ANOUBIS_P_PASSPHRASE);
	msg = msg_factory(ANOUBISD_MSG_PASSPHRASE,
	    sizeof(anoubisd_msg_passphrase_t) + plen);
	if (!msg)
		master_terminate();
	pass = (anoubisd_msg_passphrase_t *)msg->msg;
	memcpy(pass->payload, m->u.passphrase->payload, plen);

	enqueue(&eventq_s2m, msg);
	DEBUG(DBG_QUEUE, " >eventq_s2m");
	DEBUG(DBG_TRACE, "<dispatch_passphrase");
}

/**
 * Handle an authentication challenge received from the master. This
 * function is the callback for the first reply of the master in
 * response to an authentication request. This response either is
 * an acknowlegement or contains a challenge that the user must sign
 * with its private key. The challenge (if any) must be forwarded
 * to the session.
 *
 * Invariantes ensured by the way anoubis_policy_answer is called:
 * - cbdata Points to the server object of the user connection.
 * - data points to an anoubis_msg_authchallenge and the length in this
 *   message matches the len parameter of this function.
 * - error is the error code from the challenge.
 * - end must be TRUE.
 *
 * The challenge received is stored in the authentication object
 * of the server. Once a challenge reply is received the signed data
 * must be compared with the saved challenge.
 *
 * @param cbdata The callback data, i.e. the server object.
 * @param error Not used. Equal to challenge->error.
 * @param data The challenge message.
 * @param len The total length of the challenge message.
 * @param end Always true. Indicates the end of the reply for
 *     requests where multi part replies are possible.
 * @return Zero in case of success, a negative error code in case of
 *     an error.
 *
 * NOTE: This function is called from policy_comm, after an
 *     ANOUBISD_MSG_AUTH_CHALLENGE was received from the master and
 *     forwarded to policy_comm for processing.
 */
static int
dispatch_auth_challenge(void *cbdata, int error __used, const void *data,
    int len, int end __used)
{
	struct anoubis_server			*server = cbdata;
	struct anoubis_auth			*auth;
	const struct anoubisd_msg_authchallenge	*challenge = data;
	struct anoubis_msg			*m;
	int					 ret;

	auth = anoubis_server_getauth(server);
	if (!auth || auth->state != ANOUBIS_AUTH_INPROGRESS) {
		log_warnx("dispatch_auth_challenge: Received unexpected "
		    "auth challenge from master");
		return -EINVAL;
	}
	if (challenge->error || auth->chan->euid != challenge->auth_uid) {
		auth->error = challenge->error;
		auth->state = ANOUBIS_AUTH_FAILURE;
		auth->uid = -1;
		auth->finish_callback(auth->cbdata);
		return 0;
	}
	if (challenge->challengelen == 0 && challenge->idlen == 0) {
		auth->error = 0;
		auth->state = ANOUBIS_AUTH_SUCCESS;
		auth->uid  = auth->chan->euid;
		auth->finish_callback(auth->cbdata);
		return 0;
	}
	/*
	 * If the master sent us a challenge but the user did not request
	 * key based authentication, return an authentication failure.
	 */
	if (auth->auth_type != ANOUBIS_AUTH_TRANSPORTANDKEY) {
		auth->error = ANOUBIS_E_PERM;
		auth->state = ANOUBIS_AUTH_FAILURE;
		auth->uid = -1;
		auth->finish_callback(auth->cbdata);
	}
	/*
	 * Save the challenge in auth_private. We must copy the memory
	 * because the buffer passed to this function is part of the
	 * anoubisd message and will be freed by the caller.
	 */
	auth->auth_private = malloc(len);
	if (!auth->auth_private)
		master_terminate();
	memcpy(auth->auth_private, data, len);
	m = anoubis_msg_new(sizeof(Anoubis_AuthChallengeMessage)
	    + challenge->challengelen + challenge->idlen);
	set_value(m->u.authchallenge->type, ANOUBIS_C_AUTHDATA);
	set_value(m->u.authchallenge->auth_type, ANOUBIS_AUTH_CHALLENGE);
	set_value(m->u.authchallenge->challengelen, challenge->challengelen);
	set_value(m->u.authchallenge->idlen, challenge->idlen);
	memcpy(m->u.authchallenge->payload, challenge->payload,
	    challenge->challengelen + challenge->idlen);
	ret = anoubis_msg_send(auth->chan, m);
	anoubis_msg_free(m);
	return ret;
}

/**
 * Handle an authentication result. What previously happend is:
 * - The client requested authentication.
 * - The anoubis daemon master replied with an authentication challenge.
 * - The client signed the challenge and we already forwarded it to the
 *   master for verification.
 * This message contains the master's result of this verification and
 * finishes the authentication process with either success or failure.
 *
 * The function is a callback that is called by policy_comm in response
 * to an ANOUBISD_MSG_AUTH_RESULT message.
 *
 * @param cbdata Always the server object of the client session.
 * @param error The authentication error, zero for succcess.
 * @param data Callback data (unused).
 * @param len The length of the callback  data (unused).
 * @param end Always true (unused).
 * @return Zero in case of success, a negative error code in
 *     case of an error.
 */
static int
dispatch_auth_result(void *cbdata, int error, const void *data __used,
    int len __used, int end __used)
{
	struct anoubis_server	*server = cbdata;
	struct anoubis_auth	*auth = anoubis_server_getauth(server);

	if (!auth || auth->state != ANOUBIS_AUTH_INPROGRESS
	    || auth->auth_private) {
		log_warnx("dispatch_auth_result: Received unexpected "
		    "auth result from master");
		return -EINVAL;
	}
	if (error) {
		auth->uid = -1;
		auth->error = error;
		auth->state = ANOUBIS_AUTH_FAILURE;
	} else {
		auth->uid = auth->chan->euid;
		auth->error = 0;
		auth->state = ANOUBIS_AUTH_SUCCESS;
	}
	auth->finish_callback(auth->cbdata);
	return 0;
}

/**
 * This is the central dispatcher function for client messages of type
 * ANOUBIS_C_AUTHDATA. These are protocol messages that are exchanged
 * between client and anoubis daemon during authentication. These
 * messages are not interpreted further by the procotol library.
 *
 * The authentication data has a type that can either request the start
 * or it can contain a challenge reply for challenge/response based
 * authentications.
 *
 * If this message tries to start authentication this request is forwarded
 * to the master and the master will either reply with a challenge or
 * with authentication succes.
 *
 * If the message contains a challenge reply the reply and the original
 * challenge (stored in the authentication object) is fowarded to the
 * master for verification. The master will reply with an authentication
 * result (ANOUBISD_AUTH_RESULT).
 *
 * @param server The server object for the client session.
 * @param m The client message.
 * @param auth_uid Unused.
 * @param arg The callback argument (unused).
 */
static void
dispatch_authdata(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t auth_uid __used, void *arg __used)
{
	struct anoubis_auth		*auth = anoubis_server_getauth(server);
	int				 err = ANOUBIS_E_INVAL;

	if (!auth) {
		log_warnx("Received invalid authentication data");
		dispatch_generic_reply(server, err, NULL, 0,
		    ANOUBIS_C_AUTHDATA);
		return;
	}
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_AuthTransportMessage))
	    || get_value(m->u.general->type) != ANOUBIS_C_AUTHDATA
	    || !auth->chan)
		goto error;
	switch(get_value(m->u.authtransport->auth_type)) {
	case ANOUBIS_AUTH_TRANSPORT:
	case ANOUBIS_AUTH_TRANSPORTANDKEY: {
		struct anoubisd_msg		*msg;
		struct anoubisd_msg_authrequest	*authreq;

		if (auth->state != ANOUBIS_AUTH_INIT)
			goto error;
		auth->auth_type = get_value(m->u.authtransport->auth_type);
		err = ANOUBIS_E_PERM;
		if (auth->chan->euid == (uid_t)-1)
			goto error;

		msg = msg_factory(ANOUBISD_MSG_AUTH_REQUEST,
		    sizeof(struct anoubisd_msg_authrequest));
		if (!msg)
			master_terminate();
		authreq = (struct anoubisd_msg_authrequest *)msg->msg;
		authreq->auth_uid = auth->chan->euid;
		err = - anoubis_policy_comm_addrequest(policy_comm,
		    auth->chan, POLICY_FLAG_START | POLICY_FLAG_END,
		    &dispatch_auth_challenge, NULL, server,
		    &authreq->token);
		auth->state = ANOUBIS_AUTH_INPROGRESS;
		if (err) {
			free(msg);
			goto error;
		}
		enqueue(&eventq_s2m, msg);
		DEBUG(DBG_QUEUE, " >eventq_s2m: auth request");
		break;
	}
	case ANOUBIS_AUTH_CHALLENGEREPLY: {
		int					 ivlen, siglen;
		u_int32_t				 uid;
		struct anoubisd_msg_authchallenge	*challenge;
		struct anoubisd_msg_authverify		*verify;
		struct anoubisd_msg			*msg;

		if (auth->auth_type != ANOUBIS_AUTH_TRANSPORTANDKEY
		    || auth->state != ANOUBIS_AUTH_INPROGRESS
		    || auth->auth_private == NULL)
			goto error;
		err = ANOUBIS_E_FAULT;
		if (!VERIFY_FIELD(m, authchallengereply, payload))
			goto error;
		ivlen = get_value(m->u.authchallengereply->ivlen);
		siglen = get_value(m->u.authchallengereply->siglen);
		if (!VERIFY_BUFFER(m, authchallengereply, payload, 0, ivlen))
			goto error;
		if (!VERIFY_BUFFER(m, authchallengereply, payload, ivlen,
		    siglen))
			goto error;
		err = ANOUBIS_E_PERM;
		uid = get_value(m->u.authchallengereply->uid);
		if (auth->chan->euid != uid)
			goto error;
		challenge = auth->auth_private;
		if (challenge->auth_uid != uid)
			goto error;
		msg = msg_factory(ANOUBISD_MSG_AUTH_VERIFY,
		    sizeof(struct anoubisd_msg_authverify)
		    + challenge->challengelen + ivlen + siglen);
		if (!msg)
			master_terminate();
		verify = (struct anoubisd_msg_authverify *)msg->msg;
		verify->auth_uid = uid;
		verify->datalen = challenge->challengelen + ivlen;
		verify->siglen = siglen;
		memcpy(verify->payload, m->u.authchallengereply->payload,
		    ivlen);
		memcpy(verify->payload+ivlen, challenge->payload,
		    challenge->challengelen);
		memcpy(verify->payload + ivlen + challenge->challengelen,
		    m->u.authchallengereply->payload + ivlen, siglen);
		free(auth->auth_private);
		auth->auth_private = NULL;
		err = - anoubis_policy_comm_addrequest(policy_comm,
		    auth->chan, POLICY_FLAG_START | POLICY_FLAG_END,
		    &dispatch_auth_result, NULL, server,
		    &verify->token);
		if (err) {
			free(msg);
			goto error;
		}
		enqueue(&eventq_s2m, msg);
		DEBUG(DBG_QUEUE, " >eventq_s2m: auth request");
		break;
	}
	default:
		auth->state = ANOUBIS_AUTH_FAILURE;
		auth->error = ANOUBIS_E_INVAL;
		auth->uid = -1;
		auth->finish_callback(auth->cbdata);
		break;
	}
	return;
error:
	if (auth->auth_private) {
		free(auth->auth_private);
		auth->auth_private = NULL;
	}
	auth->error = err;
	auth->state = ANOUBIS_AUTH_FAILURE;
	auth->uid = -1;
	auth->finish_callback(auth->cbdata);
	return;
}

/**
 * This is a helper function that can be used to construct a generic
 * reply message of type ANOUBIS_REPLY. The caller can provide the
 * error code and optionally the payload data. This reply will be
 * forwarded to the client connection given by cbdata.
 *
 * @param cbdata The server object of the client connection.
 * @param error The error code to used in the reply.
 * @param data The payload data (may be NULL).
 * @param len The length of the payload data.
 * @param orig_opcode This value is used to fill in the opcode field of
 *     the reply message. It tells the client which type of request we
 *     are replying to.
 * @return Zero if the message could be sent, a negative error code
 *     otherwise.
 *
 * @note This function can only be used to reply to messages in the
 *     policy part of the protocol. The token value in the reply is
 *     always set to zero.
 */
static int
dispatch_generic_reply(void *cbdata, int error, const void *data, int len,
    int orig_opcode)
{
	struct anoubis_server	*server = cbdata;
	struct anoubis_msg	*m;
	int			 ret;

	m = anoubis_msg_new(sizeof(Anoubis_AckPayloadMessage)+len);
	if (!m)
		return -ENOMEM;
	set_value(m->u.ackpayload->type, ANOUBIS_REPLY);
	set_value(m->u.ackpayload->error, error);
	set_value(m->u.ackpayload->opcode, orig_opcode);
	m->u.ack->token = 0;
	if (len)
		memcpy(m->u.ackpayload->payload, data, len);
	ret = anoubis_msg_send(anoubis_server_getchannel(server), m);
	anoubis_msg_free(m);
	if (ret < 0)
		return ret;
	return 0;
}

/**
 * Forward a reply to a checksum list request from the master to the
 * client session. The incoming data is an ANOUBIS_P_CSUM_LIST message
 * that contains the payload data for an Anoubis_ChecksumPayloadMessage
 * protocol message.
 *
 * @param cbdata The server object of the client session for this reply.
 * @param error The error value to use in the protocol message.
 * @param data The payload data for the message.
 * @param len The length of the payload data.
 * @param flags The policy flags. These flags tell the client whether
 *     more data is to be expected as part of the reply.
 * @return Zero if the message could be sent, a negative error code in
 *     case of an error.
 */
static int
dispatch_checksum_list_reply(void *cbdata, int error, const void *data,
    int len, int flags)
{
	struct anoubis_server	*server = cbdata;
	struct anoubis_msg	*m;
	int			 ret;

	m = anoubis_msg_new(sizeof(Anoubis_ChecksumPayloadMessage)+len);
	if (!m)
		return -ENOMEM;
	set_value(m->u.checksumpayload->type, ANOUBIS_P_CSUM_LIST);
	set_value(m->u.checksumpayload->error, error);
	set_value(m->u.checksumpayload->flags, flags);
	if (len)
		memcpy(m->u.checksumpayload->payload, data, len);
	ret = anoubis_msg_send(anoubis_server_getchannel(server), m);
	anoubis_msg_free(m);
	return ret;
}

/**
 * This is the callback function for replies of the anoubis daemon master
 * to checksum requests other than list and CSMULTI. The data in the
 * request message is simply forworded to the client in an ANOUBIS_REPLY
 * message with payload.
 *
 * @param cbdata The server object of the client connection.
 * @param error The error code for the request.
 * @param data The payload data (may be NULL).
 * @param len The length of the payload data.
 * @param flags Policy flags. Must always be
 *     (POLICY_FLAG_START | POLICY_FLAG_END).
 * @return Zero if the message could be sent, a negative error code in
 *     case of an error.
 */
static int
dispatch_checksum_reply(void *cbdata, int error, const void *data,
    int len, int flags)
{
	if (flags != (POLICY_FLAG_START | POLICY_FLAG_END))
		log_warnx("Wrong flags value in dispatch_checksum_reply");
	return dispatch_generic_reply(cbdata, error, data, len,
	    ANOUBIS_P_CSUMREQUEST);
}

/**
 * Forward the reply to a CSMULTI checksum request from the master process
 * to the client.
 *
 * @param cbdata The server object of the client connection.
 * @param error The error code for the request.
 * @param data The payload data (an Anoubis_CSMultiReplyMessage).
 * @param len The length of the payload data.
 * @param flags Policy flags. Must always be
 *     (POLICY_FLAG_START | POLICY_FLAG_END).
 * @return Zero if the message could be sent, a negative error code in
 *     case of an error.
 */
static int
dispatch_csmulti_reply(void *cbdata, int error, const void *data,
    int len, int flags)
{
	struct anoubis_server		*server = cbdata;
	struct anoubis_msg		*m;
	int				 ret;

	DEBUG(DBG_TRACE, ">dispatch_csmulti_reply");
	if (flags != (POLICY_FLAG_START | POLICY_FLAG_END))
		log_warnx("Wrong flags value in dispatch_csmulti_reply");
	if (error)
		goto err;
	if (len < (int)sizeof(Anoubis_CSMultiReplyMessage)) {
		log_warnx(" dispatch_csmulti_reply: "
		    "Short csmulti reply message: len=%d\n", len);
		return -EFAULT;
	}
	m = anoubis_msg_new(len);
	if (!m) {
		error = ENOMEM;
		goto err;
	}
	memcpy(m->u.buf, data, len);
	ret = anoubis_msg_send(anoubis_server_getchannel(server), m);
	anoubis_msg_free(m);
	DEBUG(DBG_TRACE, "<dispatch_csmulti_reply: error=%d", -ret);
	return ret;
err:
	m = anoubis_msg_new(sizeof(Anoubis_CSMultiReplyMessage));
	if (!m)
		return -ENOMEM;
	set_value(m->u.csmultireply->type, ANOUBIS_P_CSMULTIREPLY);
	/*
	 * NOTE: This is a very rare error case and we don't know the
	 * NOTE: original operation. Use one that is not totally bogus.
	 */
	set_value(m->u.csmultireply->operation, ANOUBIS_CHECKSUM_OP_DEL);
	set_value(m->u.csmultireply->error, error);
	ret = anoubis_msg_send(anoubis_server_getchannel(server), m);
	anoubis_msg_free(m);
	DEBUG(DBG_TRACE, "<dispatch_csmulti_reply: error=%d ret=%d",
	    error, ret);
	return ret;
}

/**
 * Forward the reply to a generic list request from the policy engine
 * to the client. Currently list requests for playground, playground files
 * and user processes are supported.
 *
 * @param cbdata The server object of the client connection.
 * @param error The error code for the request.
 * @param data The payload data (an Anoubis_ListMessage).
 * @param len The length of the payload data.
 * @param flags Policy flags. Must always be
 *     (POLICY_FLAG_START | POLICY_FLAG_END).
 * @return Zero if the message could be sent, a negative error code in
 *     case of an error.
 */
static int
dispatch_list_reply(void *cbdata, int error, const void *data,
    int len, int flags)
{
	struct anoubis_server		*server = cbdata;
	struct anoubis_msg		*m;
	int				 ret;

	DEBUG(DBG_TRACE, ">dispatch_list_reply");
	if (flags & ~(POLICY_FLAG_START | POLICY_FLAG_END))
		log_warnx("Wrong flags value in dispatch_list_reply");
	if (error)
		goto err;
	if (len < (int)sizeof(Anoubis_ListMessage)) {
		log_warnx(" dispatch_list_reply: "
		    "Short list reply message: len=%d\n", len);
		return -EFAULT;
	}
	m = anoubis_msg_new(len);
	if (!m) {
		error = ENOMEM;
		goto err;
	}
	memcpy(m->u.buf, data, len);
	ret = anoubis_msg_send(anoubis_server_getchannel(server), m);
	anoubis_msg_free(m);
	DEBUG(DBG_TRACE, "<dispatch_list_reply: error=%d", -ret);
	return ret;
err:
	m = anoubis_msg_new(sizeof(Anoubis_ListMessage));
	if (!m)
		return -ENOMEM;
	set_value(m->u.listreply->type, ANOUBIS_P_LISTREP);
	set_value(m->u.listreply->flags, flags | POLICY_FLAG_END);
	set_value(m->u.listreply->error, error);
	set_value(m->u.listreply->nrec, 0);
	set_value(m->u.listreply->rectype, 0);
	ret = anoubis_msg_send(anoubis_server_getchannel(server), m);
	anoubis_msg_free(m);
	DEBUG(DBG_TRACE, "<dispatch_list_reply: error=%d ret=%d",
	    error, ret);
	return ret;
}

/**
 * Forward a policy request (get or set) received from a client
 * sesssion to the policy engine. Handling of the reply data is done
 * by policy_comm internally. This function is a callback of policy_comm
 * that is called from there in response to a policy request.
 *
 * @param comm The policy_comm object. Always equal to the global
 *     policy_comm variable (unused).
 * @param token The anoubis daemon internal token assigned to this
 *     request by policy_comm. This is used to de-multiplex replies
 *     from different sources to the individual client sessions.
 * @param uid The authenticated user ID of the client session.
 * @param buf Details of the policy request. This data is passed on to
 *     the policy engine. This variable can be NULL. In this case we
 *     send a an abort message to the policy daemon. This is useful if
 *     the client connection breaks down in the middle of a policy request.
 * @param len Length of the policy request details data.
 * @param arg The callback argument (unused)
 * @param flags A combination of POLICY_FLAG_START and/or POLICY_FLAG_END.
 *     Indicates if more data belongs to the policy request.
 * @return Zero if the message could be sent, a negative error code in
 *     case of an error.
 */
static int
dispatch_policy(struct anoubis_policy_comm *comm __used, uint64_t token,
    uint32_t uid, const void *buf, size_t len, void *arg __used, int flags)
{
	struct anoubisd_msg		*msg;
	struct anoubisd_msg_polrequest	*polreq;

	DEBUG(DBG_TRACE, ">dispatch_policy token = %" PRId64, token);

	if (buf == NULL) {
		/*
		 * NOTE: buf == NULL means that the request is aborted,
		 * NOTE: usually due to a closed client connection.
		 */
		struct anoubisd_msg_polrequest_abort	*abort;
		msg = msg_factory(ANOUBISD_MSG_POLREQUEST_ABORT,
		    sizeof(struct anoubisd_msg_polrequest_abort));
		if (!msg)
			master_terminate();
		abort = (struct anoubisd_msg_polrequest_abort *)msg->msg;
		abort->token = token;
	} else {
		if (verify_polrequest(buf, len, flags) < 0) {
			log_warnx("Dropping malformed policy request");
			return -EINVAL;
		}
		msg = msg_factory(ANOUBISD_MSG_POLREQUEST,
		    sizeof(struct anoubisd_msg_polrequest) + len);
		if (!msg)
			master_terminate();
		polreq = (struct anoubisd_msg_polrequest *)msg->msg;
		polreq->token = token;
		polreq->auth_uid = uid;
		polreq->flags = flags;
		polreq->len = len;
		if (len)
			bcopy(buf, polreq->data, len);
	}

	enqueue(&eventq_s2p, msg);
	DEBUG(DBG_QUEUE, " >eventq_s2p: %" PRIx64, token);
	DEBUG(DBG_TRACE, "<dispatch_policy");
	return 0;
}

/**
 * Handle messages received from the master process by the session engine.
 * Possible messages include replies to checksum or authentication requests,
 * configuration changes, and kernel notifications that do not need a
 * reply. This function is an event handler for read events on the
 * pipe that is used to communicate with the master process.
 *
 * If EOF is encountered on the file descriptor, the session engine
 * simulates a signal and initiates a graceful shutdown.
 *
 * @param fd The file descriptor.
 * @param sig The type of the event (unused)
 * @param arg The callback argument (unused)
 * @return None.
 */
static void
dispatch_m2s(int fd, short sig __used, void *arg __used)
{
	struct anoubis_msg		*m;
	struct anoubisd_msg		*msg;
	struct eventdev_hdr		*hdr;
	unsigned int			 extra;

	DEBUG(DBG_TRACE, ">dispatch_m2s");

	for (;;) {
		u_int64_t	task;
		if ((msg = get_msg(fd)) == NULL)
			break;
		if (msg->mtype == ANOUBISD_MSG_CHECKSUMREPLY
		    || msg->mtype == ANOUBISD_MSG_CSMULTIREPLY) {
			dispatch_m2s_checksum_reply(msg);
			free(msg);
			continue;
		}
		if (msg->mtype == ANOUBISD_MSG_UPGRADE) {
			dispatch_m2s_upgrade_notify(msg);
			free(msg);
			continue;
		}
		if (msg->mtype == ANOUBISD_MSG_CONFIG) {
			if (cfg_msg_parse(msg) == 0) {
				log_info("session: reconfigure");
				/*
				 * Because we already droped you priviledges
				 * nothing more can be done here.
				 */
			} else {
				log_warnx("session: reconfigure failed");
			}
			free(msg);
			continue;
		}
		if (msg->mtype == ANOUBISD_MSG_AUTH_CHALLENGE) {
			struct anoubisd_msg_authchallenge	*auth;
			int					 ret;

			auth = (struct anoubisd_msg_authchallenge *)msg->msg;
			ret = anoubis_policy_comm_answer(policy_comm,
			    auth->token, auth->error, auth,
			    sizeof(*auth) + auth->idlen + auth->challengelen,
			    POLICY_FLAG_START | POLICY_FLAG_END);
			if  (ret < 0)
				log_warnx("Dropping unexpected auth challenge");
			free(msg);
			continue;
		}
		if (msg->mtype == ANOUBISD_MSG_AUTH_RESULT) {
			struct anoubisd_msg_authresult	*result;
			int				 ret;

			result = (struct anoubisd_msg_authresult *)msg->msg;
			ret = anoubis_policy_comm_answer(policy_comm,
			    result->token, result->error, NULL, 0,
			    POLICY_FLAG_START | POLICY_FLAG_END);
			if (ret < 0)
				log_warnx("Dropping unexpected auth result");
			free(msg);
			continue;
		}
		if (msg->mtype != ANOUBISD_MSG_EVENTDEV) {
			log_warnx("dispatch_m2s: bad mtype %d", msg->mtype);
			free(msg);
			continue;
		}

		hdr = (struct eventdev_hdr *)msg->msg;

		DEBUG(DBG_QUEUE, " >m2s: %x", hdr->msg_token);

		if (hdr->msg_flags & EVENTDEV_NEED_REPLY) {
			log_warnx("dispatch_m2s: bad flags %x", hdr->msg_flags);
		}

		hdr = (struct eventdev_hdr *)msg->msg;
		DEBUG(DBG_QUEUE, " >m2s: %x", hdr->msg_token);

		extra = hdr->msg_size -  sizeof(struct eventdev_hdr);
		task = 0;
		if (extra >= sizeof(struct anoubis_event_common))
			task = ((struct anoubis_event_common *)
			    (hdr+1))->task_cookie;
		m = anoubis_msg_new(sizeof(Anoubis_NotifyMessage) + extra);
		if (!m) {
			/* malloc failure, then we don't send the message */
			free(msg);
			DEBUG(DBG_TRACE, "<dispatch_m2s (new)");
			continue;
		}
		set_value(m->u.notify->type, ANOUBIS_N_NOTIFY);
		m->u.notify->token = hdr->msg_token;
		set_value(m->u.notify->pid, hdr->msg_pid);
		set_value(m->u.notify->task_cookie, task);
		set_value(m->u.notify->rule_id, 0);
		set_value(m->u.notify->prio, 0);
		set_value(m->u.notify->uid, hdr->msg_uid);
		set_value(m->u.notify->subsystem, hdr->msg_source);
		set_value(m->u.notify->sfsmatch, ANOUBIS_SFS_NONE);
		set_value(m->u.notify->csumoff, 0);
		set_value(m->u.notify->csumlen, 0);
		set_value(m->u.notify->pathoff, 0);
		set_value(m->u.notify->pathlen, 0);
		set_value(m->u.notify->ctxcsumoff, 0);
		set_value(m->u.notify->ctxcsumlen, 0);
		set_value(m->u.notify->ctxpathoff, 0);
		set_value(m->u.notify->ctxpathlen, 0);
		set_value(m->u.notify->evoff, 0);
		set_value(m->u.notify->evlen, extra);
		memcpy(m->u.notify->payload, &hdr[1], extra);
		free(msg);
		__send_notify(m);

		DEBUG(DBG_TRACE, "<dispatch_m2s (loop)");
	}
	if (msg_eof(fd)) {
		session_sighandler(SIGTERM, 0, NULL);
		event_del(&ev_m2s);
	}

	DEBUG(DBG_TRACE, "<dispatch_m2s");
}

/**
 * Handle messages received from the policy process by the session engine.
 * Possible messages include replies to policy or list requests, log
 * requests and ask events. This function is an event handler for read
 * events on the pipe that is used to communicate with the policy process.
 *
 * If EOF is encountered on the file descriptor, the session engine
 * simulates a signal and initiates a graceful shutdown.
 *
 * @param fd The file descriptor.
 * @param sig The type of the event (unused)
 * @param arg The callback argument (unused)
 * @return None.
 */
static void
dispatch_p2s(int fd, short sig __used, void *arg __used)
{
	struct eventdev_hdr		*hdr;
	struct anoubisd_msg		*msg;

	DEBUG(DBG_TRACE, ">dispatch_p2s");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL)
			break;

		switch (msg->mtype) {
		case ANOUBISD_MSG_POLREPLY:
			DEBUG(DBG_QUEUE, " >p2s: polreply");
			dispatch_p2s_pol_reply(msg);
			break;

		case ANOUBISD_MSG_LISTREPLY:
			DEBUG(DBG_QUEUE, " >p2s: listreply");
			dispatch_p2s_list_reply(msg);
			break;

		case ANOUBISD_MSG_PGCOMMIT_REPLY:
			DEBUG(DBG_QUEUE, " >p2s: commitreply");
			dispatch_p2s_commit_reply(msg);
			break;

		case ANOUBISD_MSG_EVENTASK:
			DEBUG(DBG_QUEUE, ">p2s: eventask");
			dispatch_p2s_evt_request(msg);
			break;

		case ANOUBISD_MSG_LOGREQUEST:
			DEBUG(DBG_QUEUE, " >p2s: log");
			dispatch_p2s_log_request(msg);
			break;

		case ANOUBISD_MSG_POLICYCHANGE:
			DEBUG(DBG_QUEUE, " >p2s: policychange");
			dispatch_p2s_policychange(msg);
			break;

		case ANOUBISD_MSG_EVENTCANCEL:
			hdr = (struct eventdev_hdr *)msg->msg;
			DEBUG(DBG_QUEUE, " >p2s: %x",
			    hdr->msg_token);
			dispatch_p2s_evt_cancel(msg);
			break;

		default:
			log_warnx("dispatch_p2s: bad mtype %d", msg->mtype);
			break;
		}

		free(msg);

		DEBUG(DBG_TRACE, "<dispatch_p2s (loop)");
	}
	if (msg_eof(fd)) {
		session_sighandler(SIGTERM, 0, NULL);
		event_del(&ev_p2s);
	}
	DEBUG(DBG_TRACE, "<dispatch_p2s");
}

/**
 * Notify all interested session about a modifies policy. This is
 * called in response to an ANOUBISD_MSG_POLICYCHANGE message received
 * from the policy process.
 *
 * @param msg The policy change message that includes user ID and priority
 *     of the modified policy.
 * @return None.
 */
void
dispatch_p2s_policychange(const struct anoubisd_msg *msg)
{
	struct anoubisd_msg_pchange	*pchange;
	struct anoubis_msg		*m;

	DEBUG(DBG_TRACE, ">dispatch_p2s_policychange");

	pchange = (struct anoubisd_msg_pchange *)msg->msg;
	m = anoubis_msg_new(sizeof(Anoubis_PolicyChangeMessage));
	if (!m) {
		DEBUG(DBG_TRACE, "<dispatch_p2s_policychange (bad new)");
		return;
	}
	set_value(m->u.policychange->type, ANOUBIS_N_POLICYCHANGE);
	set_value(m->u.policychange->uid, pchange->uid);
	set_value(m->u.policychange->prio, pchange->prio);
	__send_notify(m);
	DEBUG(DBG_TRACE, "<dispatch_p2s_policychange");
}

/**
 * Copy bytes from one buffer to another buffer. This is a helper function
 * that is used to fill in escalation messages.
 *
 * @param dstbuf The destination buffer of the copy.
 * @param dstoffp This is the offset of the copy target in the destination
 *     buffer. This value is incremented by the number of bytes copied.
 * @param srcbuf The source buffer of the copy.
 * @param srcoff The offset in the source buffer where the copy should
 *     start.
 * @param len The number of bytes to copy.
 * @param roffp The original offset in the destination buffer is stored in
 *     this location.
 * @param rlenp The number of bytes copied is stored in this location.
 * @return None.
 */
static void
do_copy(char *dstbuf, int *dstoffp, const char *srcbuf, int srcoff, int len,
    u16n *roffp, u16n *rlenp)
{
	int		 dstoff = *dstoffp;
	char		*dstp = dstbuf + dstoff;
	const char	*srcp = srcbuf + srcoff;

	set_value(*roffp, dstoff);
	set_value(*rlenp, len);
	*dstoffp += len;
	memcpy(dstp, srcp, len);
}

/**
 * Convert an anoubisd_msg_eventask message received from the policy engine
 * to an escalation message that can be sent to a client session.
 * The client message is newly allocated and must be freed by the caller
 * (Usually, this is done indirectly by calling something like __send_notify).
 * This function can be used for both alert and escalation message.
 *
 * @param notifytype The type to use in the client message
 *     (ANOUBIS_N_LOGNOTIFY or ANOUBIS_N_ASK).
 * @param eventask The eventask message received from the policy engine.
 * @return The newly allocated client message or NULL if an error occured.
 */
static struct anoubis_msg *
eventask_to_notify(int notifytype, const struct anoubisd_msg_eventask *eventask)
{
	uint64_t		 task = 0;
	int			 plen = 0, cslen = 0, ctxplen = 0, ctxcslen = 0;
	unsigned int		 extra;
	struct eventdev_hdr	*hdr;
	int			 off;
	struct anoubis_msg	*m;

	hdr = (struct eventdev_hdr *)(eventask->payload + eventask->evoff);
	plen = eventask->pathlen;
	cslen = eventask->csumlen;
	ctxplen = eventask->ctxpathlen;
	ctxcslen = eventask->ctxcsumlen;
	extra = hdr->msg_size - sizeof(struct eventdev_hdr);
	if (extra >= sizeof(struct anoubis_event_common))
		task = ((struct anoubis_event_common*)(hdr+1))->task_cookie;
	m = anoubis_msg_new(sizeof(Anoubis_NotifyMessage)
	    + extra + plen + cslen + ctxplen + ctxcslen);
	if (!m)
		return NULL;
	set_value(m->u.notify->type, notifytype);
	m->u.notify->token = hdr->msg_token;
	set_value(m->u.notify->pid, hdr->msg_pid);
	set_value(m->u.notify->task_cookie, task);
	set_value(m->u.notify->rule_id, eventask->rule_id);
	set_value(m->u.notify->prio, eventask->prio);
	set_value(m->u.notify->uid, hdr->msg_uid);
	set_value(m->u.notify->subsystem, hdr->msg_source);
	set_value(m->u.notify->sfsmatch, eventask->sfsmatch);
	set_value(m->u.notify->loglevel, eventask->loglevel);
	set_value(m->u.notify->error, eventask->error);
	off = 0;
	do_copy(m->u.notify->payload, &off, (void*)&hdr[1], 0, extra,
	    &m->u.notify->evoff, &m->u.notify->evlen);
	do_copy(m->u.notify->payload, &off, eventask->payload,
	    eventask->csumoff, cslen, &m->u.notify->csumoff,
	    &m->u.notify->csumlen);
	do_copy(m->u.notify->payload, &off, eventask->payload,
	    eventask->pathoff, plen, &m->u.notify->pathoff,
	    &m->u.notify->pathlen);
	do_copy(m->u.notify->payload, &off, eventask->payload,
	    eventask->ctxcsumoff, ctxcslen, &m->u.notify->ctxcsumoff,
	    &m->u.notify->ctxcsumlen);
	do_copy(m->u.notify->payload, &off, eventask->payload,
	    eventask->ctxpathoff, ctxplen, &m->u.notify->ctxpathoff,
	    &m->u.notify->ctxpathlen);
	return m;
}

/**
 * Convert a log request received from the anoubis policy engine to
 * a corresponding client protocol message and forward the message to
 * all appropriate client session.
 *
 * The input message is not freed.  The protocol message itself is
 * allocated dynamically but this function makes sure that the message
 * will be freed again (done by __send_notify). I.e. from a caller's
 * perspective this function does not allocate or free memory.
 *
 * @param msg The log request message. The payload is of type
 *     struct anoubisd_msg_eventask.
 * @return None.
 */
void
dispatch_p2s_log_request(const struct anoubisd_msg *msg)
{
	const struct anoubisd_msg_eventask	*eventask;
	struct anoubis_msg			*m;

	DEBUG(DBG_TRACE, ">dispatch_p2s_log_request");

	eventask = (struct anoubisd_msg_eventask *)msg->msg;
	m = eventask_to_notify(ANOUBIS_N_LOGNOTIFY, eventask);
	if (!m) {
		log_warnx("Out of memory in dispatch_p2s_log_request");
		return;
	}
	__send_notify(m);
	DEBUG(DBG_TRACE, "<dispatch_p2s_log_request");
}

/**
 * Convert an escalation message received from the anoubis policy engine
 * to a corresponding client protocol message and forward the message to
 * all appropriate client sessions.
 *
 * The input message is not freed.  The protocol message itself is
 * allocated dynamically but this function makes sure that the message
 * will be freed again (done by __send_notify). I.e. from a caller's
 * perspective this function does not allocate or free memory.
 *
 * @param msg The escalation message. The payload is of type
 *     struct anoubisd_msg_eventask.
 * @return None.
 */
static void
dispatch_p2s_evt_request(const struct anoubisd_msg *msg)
{
	struct anoubis_notify_head		*head;
	struct anoubis_msg			*m;
	const struct anoubisd_msg_eventask	*eventask = NULL;
	struct session				*sess;
	struct cbdata				*cbdata;
	int					 sent;

	DEBUG(DBG_TRACE, ">dispatch_p2s_evt_request");

	eventask = (anoubisd_msg_eventask_t *)(msg->msg);
	m = eventask_to_notify(ANOUBIS_N_ASK, eventask);
	if (!m) {
		log_warn("Out of memory in dispatch_p2s_evt_request");
		return;
	}
	cbdata = malloc(sizeof(struct cbdata));
	if (cbdata == NULL) {
		anoubis_msg_free(m);
		log_warn("Out of memory in dispatch_p2s_evt_request");
		return;
	}
	cbdata->ev_token = m->u.notify->token;

	head = anoubis_notify_create_head(m, notify_callback, cbdata);
	cbdata->ev_head = head;

	if (!head) {
		/* malloc failure: we don't send the message */
		anoubis_msg_free(m);
		free(cbdata);
		DEBUG(DBG_TRACE, "<dispatch_p2s_evt_request (bad head)");
		return;
	}

	sent = 0;
	LIST_FOREACH(sess, &sessionList, nextSession) {
		struct anoubis_notify_group * ng;
		int ret;

		if (sess->proto == NULL)
			continue;
		ng = anoubis_server_getnotify(sess->proto);
		if (!ng)
			continue;
		ret = anoubis_notify(ng, head, ANOUBISD_MAX_PENDNG_EVENTS);
		if (ret < 0)
			log_warnx("anoubis_notify: Error code %d", -ret);
		if (ret > 0) {
			DEBUG(DBG_TRACE, " >anoubis_notify: %x",
			    cbdata->ev_token);
			sent = 1;
		}
	}

	if (sent) {
		TAILQ_INSERT_TAIL(&headq, cbdata, next);
		DEBUG(DBG_TRACE, " >headq: %x", cbdata->ev_token);
	} else {
		anoubis_notify_destroy_head(head);
		notify_callback(NULL, EPERM, cbdata);
		DEBUG(DBG_TRACE, ">anoubis_notify_destroy_head");
	}

	DEBUG(DBG_TRACE, "<dispatch_p2s_evt_request");
}

/**
 * This function is called in response to a request from the policy
 * engine that an escalation request should be canceled (usually due
 * to a timeout). This function finds the callback data for the escalation
 * in the global list (headq) sends a reply to the users.
 *
 * @param msg The cancelation message. The payload is of type
 *     eventdev_token.
 * @return  None.
 */
static void
dispatch_p2s_evt_cancel(const struct anoubisd_msg *msg)
{
	struct anoubis_notify_head	*head;
	const eventdev_token		*tokenp;
	struct cbdata			*cbdata;

	DEBUG(DBG_TRACE, ">dispatch_p2s_evt_cancel");

	tokenp = (eventdev_token*)msg->msg;
	TAILQ_FOREACH(cbdata, &headq, next) {
		if (cbdata->ev_token == *tokenp)
			break;
	}
	if (cbdata) {
		head = cbdata->ev_head;
		anoubis_notify_sendreply(head, EPERM, NULL, 0);
		DEBUG(DBG_TRACE, " >anoubis_notify_sendreply: %x",
			    cbdata->ev_token);
	}

	DEBUG(DBG_TRACE, "<dispatch_p2s_evt_cancel");
}

/**
 * Handle a message with policy data (or a reply to a policy set request).
 * This function is called in response to a policy reply message received
 * from the master process. This function uses policy_comm to process
 * the message which then calls the appropriate callback.
 *
 * @param msg The reply message.
 * @return None.
 */
static void
dispatch_p2s_pol_reply(const struct anoubisd_msg *msg)
{
	const struct anoubisd_msg_polreply	*reply;
	const void				*buf = NULL;
	int					 end, ret;

	DEBUG(DBG_TRACE, ">dispatch_p2s_pol_reply");

	reply = (struct anoubisd_msg_polreply *)msg->msg;
	if (reply->len)
		buf = reply->data;
	end = reply->flags & POLICY_FLAG_END;
	ret = anoubis_policy_comm_answer(policy_comm, reply->token,
	    reply->reply, buf, reply->len, end != 0);
	if (ret < 0)
		log_warnx("Dropping unexpected policy reply");
	DEBUG(DBG_TRACE, " >anoubis_policy_comm_answer: %" PRId64 " %d %d",
	    reply->token, ret, reply->reply);

	DEBUG(DBG_TRACE, "<dispatch_p2s_pol_reply");
}

/**
 * Handle a list reply message received from the policy engine.
 * This function forwards the message to policy_comm which then calls
 * the appropriate callback function.
 *
 * @param msg The reply message.
 * @return None.
 */
static void
dispatch_p2s_list_reply(const struct anoubisd_msg *msg)
{
	const struct anoubisd_msg_listreply	*listreply;
	const void				*buf = NULL;
	int					 end, ret;

	DEBUG(DBG_TRACE, ">dispatch_p2s_list_reply");

	listreply = (struct anoubisd_msg_listreply *)msg->msg;
	if (listreply->len)
		buf = listreply->data;
	end = listreply->flags & POLICY_FLAG_END;
	ret = anoubis_policy_comm_answer(policy_comm, listreply->token, 0,
	    buf, listreply->len, end != 0);
	if (ret < 0)
		log_warnx("Dropping unexpected playground list reply");
	DEBUG(DBG_TRACE, ">anoubis_policy_comm_answer: %" PRId64,
	    listreply->token);
	DEBUG(DBG_TRACE, "<dispatch_p2s_list_reply");
}

/**
 * Handle a reply to a commit request received from the policy engine.
 * This function uses policy_comm to process the message which then calls
 * the approriate callback.
 *
 * @param msg The commit reply message.
 * @return None.
 */
static void
dispatch_p2s_commit_reply(const struct anoubisd_msg *msg)
{
	const struct anoubisd_msg_pgcommit_reply	*commitreply;
	const void					*buf = NULL;
	int						 ret;

	DEBUG(DBG_TRACE, ">dispatch_p2s_commit_reply");
	commitreply = (struct anoubisd_msg_pgcommit_reply *)msg->msg;
	DEBUG(DBG_QUEUE, " dispatch_p2s_commit_reply: token=%" PRId64,
	    commitreply->token);
	if (commitreply->len)
		buf = commitreply->payload;
	ret = anoubis_policy_comm_answer(policy_comm, commitreply->token,
	    commitreply->error, buf, commitreply->len,
	    POLICY_FLAG_START | POLICY_FLAG_END);
	if (ret < 0)
		log_warnx("Dropping unexpected playground commit reply");
	DEBUG(DBG_TRACE, "<dispatch_p2s_commit_reply");
}

/**
 * This function handles checksum  reply messages received from the
 * master. It uses policy_comm to process the message which in turn
 * calls the appropriate callback function.
 *
 * @param msg The commit reply message.
 * @return None.
 */
static void
dispatch_m2s_checksum_reply(const struct anoubisd_msg *msg)
{
	const struct anoubisd_msg_csumreply	*reply;
	int					 ret, end;

	reply = (struct anoubisd_msg_csumreply *)msg->msg;
	end = reply->flags & POLICY_FLAG_END;
	ret = anoubis_policy_comm_answer(policy_comm, reply->token,
	    reply->reply, reply->data, reply->len, end);
	if (ret < 0) {
		errno = -ret;
		log_warnx("dispatch_m2s_checksum_reply: "
		    "Failed to process answer");
	}
}

/**
 * Notify any interested sessions about a system upgrade that just
 * completed. This function is called in response to an ANOUBISD_MSG_UPGRADE
 * message received from the master process. It sends a status notification
 * of type ANOUBIS_STATUS_UPGRADE to all session registered for this
 * type of event.
 *
 * @param msg The commit reply message.
 * @return None.
 */
static void
dispatch_m2s_upgrade_notify(const struct anoubisd_msg *msg)
{
	const struct anoubisd_msg_upgrade	*umsg;
	int					 count;
	struct anoubis_msg			*m;

	DEBUG(DBG_TRACE, ">dispatch_m2s_upgrade_notify");
	umsg = (struct anoubisd_msg_upgrade *)msg->msg;
	if (umsg->upgradetype != ANOUBISD_UPGRADE_NOTIFY) {
		log_warnx("Bad upgrade message type=%d in session engine",
		    umsg->upgradetype);
		return;
	}
	count = umsg->chunksize;
	m = anoubis_msg_new(sizeof(Anoubis_StatusNotifyMessage));
	if (!m) {
		DEBUG(DBG_TRACE, "<dispatch_m2s_upgrade_notify (oom)");
		return;
	}
	set_value(m->u.statusnotify->type, ANOUBIS_N_STATUSNOTIFY);
	set_value(m->u.statusnotify->statuskey, ANOUBIS_STATUS_UPGRADE);
	set_value(m->u.statusnotify->statusvalue, count);
	__send_notify(m);
	DEBUG(DBG_TRACE, "<dispatch_m2s_upgrade_notify");
}

/**
 * This is the event handler for write events on the outgoing pipe
 * to the master process. The event is only active if there is data
 * pending either for that file descriptor.
 *
 * @param fd The file descriptor
 * @param sig The type of the event (unused)
 * @param arg The callback argument (unused)
 * @return None.
 */
static void
dispatch_s2m(int fd, short sig __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_s2m");
	dispatch_write_queue(&eventq_s2m, fd);
	DEBUG(DBG_TRACE, "<dispatch_s2m");
}

/**
 * This is the event handler for write events on the outgoing pipe
 * to the policy process. The event is only active if there is data
 * pending either for that file descriptor.
 *
 * @param fd The file descriptor
 * @param sig The type of the event (unused)
 * @param arg The callback argument (unused)
 * @return None.
 */
static void
dispatch_s2p(int fd, short sig __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_s2p");
	dispatch_write_queue(&eventq_s2p, fd);
	DEBUG(DBG_TRACE, "<dispatch_s2p");
}

/**
 * Remove a session from the global list of sessions and free all
 * memory associated with the session. Events that were registered
 * for this session are removed.
 *
 * @param session The session to destroy.
 * @return None.
 */
static void
session_destroy(struct session *session)
{
	DEBUG(DBG_TRACE, ">session_destroy");

	if (session->proto) {
		struct anoubis_auth	*auth;

		auth = anoubis_server_getauth(session->proto);
		if (auth && auth->auth_private) {
			free(auth->auth_private);
			auth->auth_private = NULL;
		}
		anoubis_server_destroy(session->proto);
		session->proto = NULL;
	}
	event_del(&(session->ev_rdata));
	event_del(&(session->ev_wdata));
	msg_release(session->connfd);
	acc_destroy(session->channel);

	LIST_REMOVE(session, nextSession);
	bzero(session, sizeof(struct session));

	free((void *)session);

	DEBUG(DBG_TRACE, "<session_destroy");
}

/**
 * Setup the listening socket for client connections and return the
 * the achat channel of the socket.
 *
 * @return The listening channel. NULL if an error occured.
 */
static struct achat_channel *
setup_listening_socket(void)
{
	struct sockaddr_un	 ss;
	struct achat_channel	*listener = NULL;

	DEBUG(DBG_TRACE, ">setup_listening_socket");

	listener = acc_create();
	if (listener == NULL)
		goto err;

	if (acc_settail(listener, ACC_TAIL_SERVER) != ACHAT_RC_OK)
		goto err;
	if (acc_setsslmode(listener, ACC_SSLMODE_CLEAR) != ACHAT_RC_OK)
		goto err;
	/* This will affect the blocking mode of the new connections. */
	if (acc_setblockingmode(listener, ACC_NON_BLOCKING))
		goto err;

	bzero(&ss, sizeof(ss));
	ss.sun_family = AF_UNIX;
	strlcpy(ss.sun_path, anoubisd_config.unixsocket, sizeof(ss.sun_path));
	if (acc_setaddr(listener, (struct sockaddr_storage *)&ss,
	    sizeof(struct sockaddr_un)) != ACHAT_RC_OK)
		goto err;
	if (acc_prepare(listener) != ACHAT_RC_OK)
		goto err;

	DEBUG(DBG_TRACE, "<session_setupuds");
	return listener;

err:
	if (listener)
		acc_destroy(listener);
	log_warnx("Failed to setup listening socket.");
	return NULL;
}
