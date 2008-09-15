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
#include <sys/queue.h>
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#endif
#ifdef LINUX
#include <grp.h>
#include <bsdcompat.h>
#include <queue.h>
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#endif
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <anoubischat.h>
#include <anoubis_msg.h>
#include <anoubis_server.h>
#include <anoubis_notify.h>
#include <anoubis_policy.h>
#include <anoubis_dump.h>

#include "anoubisd.h"
#include "aqueue.h"
#include "amsg.h"


/*
 * The following code utilizes list functions from BSD queue.h, which
 * cannot be reliably annotated. We therefore exclude the following
 * functions from memory checking.
 */
/*@-memchecks@*/
struct session {
	int			 id;		/* session number */
	int			 connfd;	/* saved connect fd */
	uid_t			 uid;	/* user id; not authenticated on -1 */
	struct achat_channel	*channel;	/* communication channel */
	struct anoubis_server	*proto;		/* Server protocol handler */
	struct event		 ev_rdata;	/* event for receiving data */
	struct event		 ev_wdata;	/* event for sending data */
	LIST_ENTRY(session)	 nextSession;	/* linked list element */
};

struct sessionGroup {
	LIST_HEAD(slHead, session)	 sessionList;
	struct achat_channel		*keeper_uds;
	struct event			 ev_connect;
};

struct event_info_session {
	struct event	*ev_s2m, *ev_s2p;
	struct event	*ev_m2s, *ev_p2s;
	struct sessionGroup *seg;
	struct anoubis_policy_comm *policy;
};

struct cbdata {
	eventdev_token	ev_token;
	struct event_info_session	*ev_info;
	struct anoubis_notify_head	*ev_head;
};

struct anoubisd_msg_comm_store {
	u_int64_t	token;
	struct anoubis_policy_comm *comm;
};


static void	session_sighandler(int, short, void *);
static void	notify_callback(struct anoubis_notify_head *, int, void *);
static int	dispatch_policy(struct anoubis_policy_comm *, u_int64_t,
		    u_int32_t, void *, size_t, void *, int);
static void	dispatch_checksum(struct anoubis_server *server,
		    struct anoubis_msg *msg, uid_t uid, void *arg);
static int	dispatch_checksum_reply(void *cbdata, int error, void *data,
		    int len, int end);
static void	dispatch_m2s(int, short, void *);
static void	dispatch_s2m(int, short, void *);
static void	dispatch_s2p(int, short, void *);
static void	dispatch_p2s(int, short, void *);
static void	dispatch_p2s_evt_request(anoubisd_msg_t *,
		    struct event_info_session *);
static void	dispatch_p2s_log_request(anoubisd_msg_t *,
		    struct event_info_session *);
static void	dispatch_p2s_evt_cancel(anoubisd_msg_t *,
		    struct event_info_session *);
static void	dispatch_p2s_pol_reply(anoubisd_msg_t *,
		    struct event_info_session *);
static void	dispatch_m2s_pol_reply(anoubisd_msg_t *msg,
		    struct event_info_session *ev_info);
static void	session_connect(int, short, void *);
static void	session_rxclient(int, short, void *);
static void	session_txclient(int, short, void *);
static void	session_setupuds(struct sessionGroup *,
		struct anoubisd_config *);
static void	session_destroy(struct session *);
static int	dispatch_generic_reply(void *cbdata, int error,
		    void *data, int len, int flags, int orig_opcode);
static void	dispatch_sfsdisable(struct anoubis_server *,
		    struct anoubis_msg *, uid_t, void *);
static int	dispatch_sfsdisable_reply(void *, int, void *, int, int);

static Queue eventq_s2p;
static Queue eventq_s2m;
static Queue requestq;

static Queue headq;

static void
session_sighandler(int sig, short event __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">session_sighandler");
	switch (sig) {
	case SIGTERM:
	case SIGINT:
	case SIGQUIT:
		(void)event_loopexit(NULL);
		/*FALLTRHOUGH*/
	}
}

static void
session_connect(int fd __used, short event __used, void *arg)
{
	struct event_info_session * info = arg;
	struct session		*session = NULL;
	struct sessionGroup	*seg = info->seg;
	static int		 sessionid = 0;

	DEBUG(DBG_TRACE, ">session_connect");

	session = (struct session *)calloc(1, sizeof(struct session));
	if (session == NULL) {
		log_warn("session_connect: calloc");
		DEBUG(DBG_TRACE, "<session_connect (calloc)");
		return;
	}

	session->id  = sessionid++;
	session->uid = -1; /* this session is not authenticated */

	session->channel = acc_opendup(seg->keeper_uds);
	if (session->channel == NULL) {
		log_warn("session_connect: acc_opendup");
		free((void *)session);
		DEBUG(DBG_TRACE, "<session_connect (opendup)");
		return;
	}
	session->proto = anoubis_server_create(session->channel, info->policy);
	if (session->proto == NULL) {
		log_warn("cannot create server protocol handler");
		acc_close(session->channel);
		acc_destroy(session->channel);
		free(session);
		DEBUG(DBG_TRACE, "<session_connect (create)");
		return;
	}
	anoubis_dispatch_create(session->proto, ANOUBIS_P_CSUMREQUEST,
	       dispatch_checksum, info);
	anoubis_dispatch_create(session->proto, ANOUBIS_P_SFSDISABLE,
		dispatch_sfsdisable, info);
	LIST_INSERT_HEAD(&(seg->sessionList), session, nextSession);
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
	session->connfd = session->channel->fd;
	session->channel->event = &session->ev_wdata;
	msg_init(session->connfd, "session");

	DEBUG(DBG_TRACE, "<session_connect");
}

static void
session_rxclient(int fd __used, short event __used, void *arg)
{
	struct session		*session;
	struct anoubis_msg	*m;
	int			 ret;

	DEBUG(DBG_TRACE, ">session_rxclient");

	session = (struct session *)arg;
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
		if (!m)
			break;
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

static void
session_txclient(int fd __used, short event __used, void *arg)
{
	struct session	*sess = arg;
	/* acc_flush will re-add the event if needed. */
	acc_flush(sess->channel);
}

pid_t
session_main(struct anoubisd_config *conf, int pipe_m2s[2], int pipe_m2p[2],
    int pipe_s2p[2], int loggers[3])
{
	struct event	 ev_sigterm, ev_sigint, ev_sigquit;
	struct event	 ev_m2s, ev_p2s;
	struct event	 ev_s2m, ev_s2p;
	struct event_info_session	ev_info;
	struct sessionGroup		seg;
	struct passwd	*pw;
	sigset_t	 mask;
	pid_t		 pid;


	switch (pid = fork()) {
	case -1:
		fatal("fork");
		/* NOTREACHED */
	case 0:
		break;
	default:
		return (pid);
	}

	(void)event_init();

	log_init(loggers[2]);
	close(loggers[0]);
	close(loggers[1]);
	log_info("session started (pid %d)", getpid());

	anoubisd_process = PROC_SESSION;

	/* while still privileged we install a listening socket */
	session_setupuds(&seg, conf);

	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		fatal("getpwnam");

	if (chroot(pw->pw_dir) == -1)
		fatal("chroot");
	if (chdir("/") == -1)
		fatal("chdir");

#ifdef OPENBSD
	setproctitle("session engine");
#endif

	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		fatal("can't drop privileges");


	/* From now on, this is an unprivileged child process. */
	LIST_INIT(&(seg.sessionList));
	queue_init(eventq_s2p);
	queue_init(eventq_s2m);
	queue_init(requestq);

	queue_init(headq);

	/* We catch or block signals rather than ignoring them. */
	signal_set(&ev_sigterm, SIGTERM, session_sighandler, NULL);
	signal_set(&ev_sigint, SIGINT, session_sighandler, NULL);
	signal_set(&ev_sigquit, SIGQUIT, session_sighandler, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);

	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGQUIT);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	close(pipe_m2s[0]);
	close(pipe_s2p[1]);
	close(pipe_m2p[0]);
	close(pipe_m2p[1]);

	/* init msg_bufs - keep track of outgoing ev_info */
	msg_init(pipe_m2s[1], "s2m");
	msg_init(pipe_s2p[0], "s2p");

	/* master process */
	event_set(&ev_m2s, pipe_m2s[1], EV_READ | EV_PERSIST, dispatch_m2s,
	    &ev_info);
	event_add(&ev_m2s, NULL);

	event_set(&ev_s2m, pipe_m2s[1], EV_WRITE, dispatch_s2m,
	    &ev_info);

	/* policy process */
	event_set(&ev_p2s, pipe_s2p[0], EV_READ | EV_PERSIST, dispatch_p2s,
	    &ev_info);
	event_add(&ev_p2s, NULL);

	event_set(&ev_s2p, pipe_s2p[0], EV_WRITE, dispatch_s2p,
	    &ev_info);

	ev_info.ev_m2s = &ev_m2s;
	ev_info.ev_s2m = &ev_s2m;
	ev_info.ev_p2s = &ev_p2s;
	ev_info.ev_s2p = &ev_s2p;
	ev_info.seg = &seg;
	ev_info.policy = anoubis_policy_comm_create(&dispatch_policy, &ev_info);
	if (!ev_info.policy)
		fatal("Cannot create policy object (out of memory)");

	/* setup keeper of incoming unix domain socket connections */
	if (seg.keeper_uds != NULL) {
		event_set(&(seg.ev_connect), (seg.keeper_uds)->fd,
		    EV_READ | EV_PERSIST, session_connect, &ev_info);
		event_add(&(seg.ev_connect), NULL);
	}

	DEBUG(DBG_TRACE, "session event loop");
	if (event_dispatch() == -1)
		fatal("session_main: event_dispatch");
	DEBUG(DBG_TRACE, "event loop done");

	/* stop events of incoming connects and cleanup sessions */
	event_del(&(seg.ev_connect));
	while (!LIST_EMPTY(&(seg.sessionList))) {
		struct session *session = LIST_FIRST(&(seg.sessionList));
		log_warn("session_main: close remaining session by force");
		/* session_destroy will call LIST_REMOVE. */
		session_destroy(session);
	}
	acc_destroy(seg.keeper_uds);

	_exit(0);
}

/* Handle Notify and Alert Messages - coming from master */
static void
notify_callback(struct anoubis_notify_head *head, int verdict, void *cbdata)
{
	anoubisd_msg_t *msg;
	struct eventdev_reply *rep;

	DEBUG(DBG_TRACE, ">notify_callback");

	if (cbdata == NULL) {
		log_warn("notify_callback: null pointer");
		master_terminate(EINVAL);
		return;
	}

	msg = msg_factory(ANOUBISD_MSG_EVENTREPLY,
	    sizeof(struct eventdev_reply));
	rep = (struct eventdev_reply *)msg->msg;
	rep->msg_token = ((struct cbdata *)cbdata)->ev_token;
	rep->reply = verdict;

	enqueue(&eventq_s2p, msg);
	DEBUG(DBG_QUEUE, " >eventq_s2p: %x", rep->msg_token);
	event_add(((struct cbdata *)cbdata)->ev_info->ev_s2p, NULL);

	if (head) {
		anoubis_notify_destroy_head(head);
		DEBUG(DBG_TRACE, " >anoubis_notify_destroy_head");
		DEBUG(DBG_QUEUE, " <headq: %x", rep->msg_token);
		queue_delete(&headq, cbdata);
	}
	free(cbdata);

	DEBUG(DBG_TRACE, "<notify_callback");
}

static void
dispatch_checksum(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t uid, void *arg)
{
	anoubisd_msg_t			*s2m_msg;
	anoubisd_msg_checksum_op_t	*msg_csum;
	struct achat_channel		*chan;
	int err;

	struct event_info_session *ev_info = (struct event_info_session*)arg;

	DEBUG(DBG_TRACE, ">dispatch_checksum");
	DEBUG(DBG_TRACE, "%s\n", m->u.checksumrequest->path);
	chan = anoubis_server_getchannel(server);
	s2m_msg = msg_factory(ANOUBISD_MSG_CHECKSUM_OP,
	    sizeof(anoubisd_msg_checksum_op_t) + m->length);
	msg_csum = (anoubisd_msg_checksum_op_t *)s2m_msg->msg;
	msg_csum->uid = uid;
	msg_csum->len = m->length;
	memcpy(msg_csum->msg, m->u.checksumrequest, m->length);

	err = anoubis_policy_comm_addrequest(ev_info->policy, chan,
	    POLICY_FLAG_START | POLICY_FLAG_END, &dispatch_checksum_reply,
	    server, &msg_csum->token);
	if (err < 0) {
		log_warn("Dropping checksum request (error %d)", err);
		free(s2m_msg);
		return;
	}
	enqueue(&eventq_s2m, s2m_msg);
	DEBUG(DBG_QUEUE, " >eventq_s2m");
	event_add(ev_info->ev_s2m, NULL);

	DEBUG(DBG_TRACE, "<dispatch_checksum");
}

static void
dispatch_sfsdisable(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t uid, void *arg)
{
	anoubisd_msg_t			*s2p_msg;
	anoubisd_msg_sfsdisable_t	*msg_disable;
	struct achat_channel		*chan;
	struct event_info_session	*ev_info;
	int err;

	ev_info = (struct event_info_session*)arg;
	DEBUG(DBG_TRACE, ">dispatch_sfsdisable %d");
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_SfsDisableMessage))) {
		dispatch_generic_reply(server, EINVAL, NULL, 0,
		    POLICY_FLAG_START|POLICY_FLAG_END, ANOUBIS_P_SFSDISABLE);
		return;
	}
	chan = anoubis_server_getchannel(server);
	s2p_msg = msg_factory(ANOUBISD_MSG_SFSDISABLE,
	    sizeof(anoubisd_msg_sfsdisable_t));
	msg_disable = (anoubisd_msg_sfsdisable_t *)s2p_msg->msg;
	msg_disable->uid = uid;
	msg_disable->pid = get_value(m->u.sfsdisable->pid);
	err = anoubis_policy_comm_addrequest(ev_info->policy, chan,
	    POLICY_FLAG_START|POLICY_FLAG_END, &dispatch_sfsdisable_reply,
	    server, &msg_disable->token);
	if (err < 0) {
		dispatch_generic_reply(server, EAGAIN, NULL, 0,
		    POLICY_FLAG_START|POLICY_FLAG_END, ANOUBIS_P_SFSDISABLE);
		free(msg_disable);
		return;
	}
	enqueue(&eventq_s2p, s2p_msg);
	DEBUG(DBG_QUEUE, " >eventq_s2p");
	event_add(ev_info->ev_s2p, NULL);

	DEBUG(DBG_TRACE, "<dispatch_sfsdisable");
}

static int
dispatch_generic_reply(void *cbdata, int error, void *data, int len, int flags,
    int orig_opcode)
{
	struct anoubis_server	*server = cbdata;
	struct anoubis_msg	*m;
	int			 ret;

	m = anoubis_msg_new(sizeof(Anoubis_AckPayloadMessage)+len);
	if (!m)
		return -ENOMEM;
	if (flags != (POLICY_FLAG_START|POLICY_FLAG_END)) {
		log_warn("dispatch_generic_reply: flags is %x");
		return -EINVAL;
	}
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

static int
dispatch_checksum_reply(void *cbdata, int error, void *data, int len, int flags)
{
	return dispatch_generic_reply(cbdata, error, data, len, flags,
	    ANOUBIS_P_CSUMREQUEST);
}

static int
dispatch_sfsdisable_reply(void *cbdata, int error, void *data, int len,
    int flags)
{
	return dispatch_generic_reply(cbdata, error, data, len, flags,
	    ANOUBIS_P_SFSDISABLE);
}

static int
dispatch_policy(struct anoubis_policy_comm *comm, u_int64_t token,
    u_int32_t uid, void *buf, size_t len, void *arg, int flags)
{
	struct event_info_session *ev_info = (struct event_info_session*)arg;
	anoubisd_msg_t *msg;
	anoubisd_msg_comm_t *msg_comm;
	struct anoubisd_msg_comm_store *msg_store;

	DEBUG(DBG_TRACE, ">dispatch_policy token = %lld", (long long)token);

	msg = msg_factory(ANOUBISD_MSG_POLREQUEST,
	    sizeof(anoubisd_msg_comm_t) + len);
	msg_comm = (anoubisd_msg_comm_t *)msg->msg;
	msg_comm->token = token;
	msg_comm->uid = uid;
	msg_comm->flags = flags;
	msg_comm->len = len;
	bcopy(buf, msg_comm->msg, len);

	if ((msg_store = malloc(sizeof(struct anoubisd_msg_comm_store))) ==
	    NULL) {
		log_warn("dispatch_policy: cannot allocate memory");
		free(msg);
		master_terminate(ENOMEM);
		return -ENOMEM;
	}
	msg_store->token = msg_comm->token;
	msg_store->comm = comm;

	enqueue(&requestq, msg_store);
	DEBUG(DBG_QUEUE, " >requestq: %x", token);

	enqueue(&eventq_s2p, msg);
	DEBUG(DBG_QUEUE, " >eventq_s2p: %x", token);
	event_add(ev_info->ev_s2p, NULL);

	DEBUG(DBG_TRACE, "<dispatch_policy");
	return 0;
}

/* Handle Notify and Alert Messages - coming from master */
static void
dispatch_m2s(int fd, short sig __used, void *arg)
{
	struct event_info_session *ev_info = (struct event_info_session*)arg;
	struct anoubis_notify_head * head;
	struct anoubis_msg * m;
	anoubisd_msg_t *msg;
	struct eventdev_hdr *hdr;
	struct session * sess;
	unsigned int extra;

	DEBUG(DBG_TRACE, ">dispatch_m2s");

	for (;;) {
		u_int64_t task;
		if ((msg = get_msg(fd)) == NULL) {
			DEBUG(DBG_TRACE, "<dispatch_m2s");
			return;
		}
		if (msg->mtype == ANOUBISD_MSG_POLREPLY) {
			dispatch_m2s_pol_reply(msg, ev_info);
			continue;
		}
		if (msg->mtype != ANOUBISD_MSG_EVENTDEV) {
			log_warn("dispatch_m2s: bad mtype %d", msg->mtype);
			continue;
		}

		hdr = (struct eventdev_hdr *)msg->msg;

		DEBUG(DBG_QUEUE, " >m2s: %x", hdr->msg_token);

		if (hdr->msg_flags & EVENTDEV_NEED_REPLY) {
			log_warn("dispatch_m2s: bad flags %x", hdr->msg_flags);
		}

		DEBUG(DBG_QUEUE, " >m2s: %x",
		    ((struct eventdev_hdr *)msg->msg)->msg_token);

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
		set_value(m->u.notify->uid, hdr->msg_uid);
		set_value(m->u.notify->subsystem, hdr->msg_source);
		set_value(m->u.notify->operation, 0 /* XXX ?? */);
		set_value(m->u.notify->csumoff, 0);
		set_value(m->u.notify->csumlen, 0);
		set_value(m->u.notify->pathoff, 0);
		set_value(m->u.notify->pathlen, 0);
		set_value(m->u.notify->evoff, 0);
		set_value(m->u.notify->evlen, extra);
		memcpy(m->u.notify->payload, &hdr[1], extra);
		head = anoubis_notify_create_head(m, NULL, NULL);
		if (!head) {
			/* malloc failure, then we don't send the message */
			anoubis_msg_free(m);
			free(msg);
			DEBUG(DBG_TRACE, "<dispatch_m2s (free)");
			continue;
		}
		DEBUG(DBG_TRACE, " >anoubis_notify_create_head");

		free(msg);

		LIST_FOREACH(sess, &ev_info->seg->sessionList, nextSession) {
			struct anoubis_notify_group * ng;

			if (sess->proto == NULL)
				continue;
			ng = anoubis_server_getnotify(sess->proto);
			if (!ng)
				continue;
			anoubis_notify(ng, head);
			DEBUG(DBG_TRACE, " >anoubis_notify: %x",
			    hdr->msg_token);
		}
		anoubis_notify_destroy_head(head);
		DEBUG(DBG_TRACE, " >anoubis_notify_destroy_head");

		DEBUG(DBG_TRACE, "<dispatch_m2s (loop)");
	}
}

/* Handle Request Messages - coming from policy */
static void
dispatch_p2s(int fd, short sig __used, void *arg)
{
	struct event_info_session *ev_info = (struct event_info_session*)arg;
	anoubisd_msg_t *msg;

	DEBUG(DBG_TRACE, ">dispatch_p2s");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL) {
			DEBUG(DBG_TRACE, "<dispatch_p2s");
			return;
		}

		switch (msg->mtype) {

		case ANOUBISD_MSG_POLREPLY:
			DEBUG(DBG_QUEUE, " >p2s");
			dispatch_p2s_pol_reply(msg, ev_info);
			break;

		case ANOUBISD_MSG_EVENTDEV:
		case ANOUBISD_MSG_EVENTASK:
		case ANOUBISD_MSG_SFSOPEN:
			DEBUG(DBG_QUEUE, ">p2s");
			dispatch_p2s_evt_request(msg, ev_info);
			break;

		case ANOUBISD_MSG_LOGREQUEST:
			DEBUG(DBG_QUEUE, " >p2s: log %x",
			    ((struct anoubisd_msg_logrequest*)msg->msg)
				->hdr.msg_token);
			dispatch_p2s_log_request(msg, ev_info);
			break;

		case ANOUBISD_MSG_EVENTCANCEL:
			DEBUG(DBG_QUEUE, " >p2s: %x",
			    ((struct eventdev_hdr *)msg->msg)->msg_token);
			dispatch_p2s_evt_cancel(msg, ev_info);
			break;

		default:
			log_warn("dispatch_p2s: bad mtype %d", msg->mtype);
			break;
		}

		free(msg);

		DEBUG(DBG_TRACE, "<dispatch_p2s (loop)");
	}
}

void
dispatch_p2s_log_request(anoubisd_msg_t *msg,
    struct event_info_session *ev_info)
{
	unsigned int extra;
	struct anoubisd_msg_logrequest * req;
	struct anoubis_notify_head * head;
	struct anoubis_msg * m;
	struct session * sess;
	anoubis_cookie_t task = 0;

	DEBUG(DBG_TRACE, ">dispatch_p2s_log_request");

	req = (struct anoubisd_msg_logrequest *)msg->msg;
	extra = req->hdr.msg_size - sizeof(struct eventdev_hdr);
	m = anoubis_msg_new(sizeof(Anoubis_NotifyMessage) + extra);
	if (extra >= sizeof(struct anoubis_event_common))
		task = ((struct anoubis_event_common *)
		    ((&req->hdr)+1))->task_cookie;
	if (!m) {
		/* malloc failure, then we don't send the message */
		DEBUG(DBG_TRACE, "<dispatch_p2s_log_request (bad new)");
		return;
	}
	set_value(m->u.notify->type, ANOUBIS_N_LOGNOTIFY);
	m->u.notify->token = req->hdr.msg_token;
	set_value(m->u.notify->pid, req->hdr.msg_pid);
	set_value(m->u.notify->task_cookie, task);
	set_value(m->u.notify->rule_id, req->rule_id);
	set_value(m->u.notify->uid, req->hdr.msg_uid);
	set_value(m->u.notify->subsystem, req->hdr.msg_source);
	set_value(m->u.notify->operation, 0 /* XXX ?? */);
	set_value(m->u.notify->loglevel, req->loglevel);
	set_value(m->u.notify->error, req->error);
	set_value(m->u.notify->csumoff, 0);
	set_value(m->u.notify->csumlen, 0);
	set_value(m->u.notify->pathoff, 0);
	set_value(m->u.notify->pathlen, 0);
	set_value(m->u.notify->evoff, 0);
	set_value(m->u.notify->evlen, extra);
	memcpy(m->u.notify->payload, (&req->hdr)+1, extra);
	head = anoubis_notify_create_head(m, NULL, NULL);
	if (!head) {
		/* malloc failure, then we don't send the message */
		anoubis_msg_free(m);
		DEBUG(DBG_TRACE, "<dispatch_p2s (free)");
		return;
	}
	DEBUG(DBG_TRACE, " >anoubis_notify_create_head");

	LIST_FOREACH(sess, &ev_info->seg->sessionList, nextSession) {
		struct anoubis_notify_group * ng;

		if (sess->proto == NULL)
			continue;
		ng = anoubis_server_getnotify(sess->proto);
		if (!ng)
			continue;
		anoubis_notify(ng, head);
		DEBUG(DBG_TRACE, " >anoubis_notify: %x", req->hdr.msg_token);
	}
	anoubis_notify_destroy_head(head);
	DEBUG(DBG_TRACE, " >anoubis_notify_destroy_head");
}

void
dispatch_p2s_evt_request(anoubisd_msg_t	*msg,
    struct event_info_session *ev_info)
{
	struct anoubis_notify_head * head;
	struct anoubis_msg * m;
	anoubisd_msg_eventask_t *eventask = NULL;
	struct eventdev_hdr *hdr;
	struct session	*sess;
	struct cbdata	*cbdata;
	unsigned int extra;
	int sent;
	u_int64_t task = 0;
	u_int32_t rule_id = 0;
	int plen = 0, cslen = 0;

	DEBUG(DBG_TRACE, ">dispatch_p2s_evt_request");

	switch(msg->mtype) {
	case ANOUBISD_MSG_EVENTDEV:
		hdr = (struct eventdev_hdr *)msg->msg;
		break;
	case ANOUBISD_MSG_EVENTASK:
		eventask = (anoubisd_msg_eventask_t *)(msg->msg);
		rule_id = eventask->rule_id;
		hdr = (struct eventdev_hdr *)
		    (eventask->payload + eventask->evoff);
		plen = eventask->pathlen;
		cslen = eventask->csumlen;
		break;
	case ANOUBISD_MSG_SFSOPEN:
		hdr = &((anoubisd_msg_sfsopen_t*)msg->msg)->hdr;
		rule_id = ((anoubisd_msg_sfsopen_t*)msg->msg)->rule_id;
		break;
	default:
		log_warn("dispatch_p2s_evt_request: bad mtype %d", msg->mtype);
		return;
	}

	if ((hdr->msg_flags & EVENTDEV_NEED_REPLY) == 0) {
		log_warn("dispatch_p2s: bad flags %x", hdr->msg_flags);
	}

	extra = hdr->msg_size - sizeof(struct eventdev_hdr);
	if (extra >= sizeof(struct anoubis_event_common))
		task = ((struct anoubis_event_common*)(hdr+1))->task_cookie;
	m = anoubis_msg_new(sizeof(Anoubis_NotifyMessage)
	    + extra + plen + cslen);
	if (!m) {
		/* malloc failure, then we don't send the message */
		DEBUG(DBG_TRACE, "<dispatch_p2s_evt_request (bad new)");
		return;
	}

	if ((cbdata = malloc(sizeof(struct cbdata))) == NULL) {
		/* malloc failure, then we don't send the message */
		anoubis_msg_free(m);
		DEBUG(DBG_TRACE, "<dispatch_p2s_evt_request (bad cbdata)");
		return;
	}
	cbdata->ev_token = hdr->msg_token;
	cbdata->ev_info = ev_info;

	set_value(m->u.notify->type, ANOUBIS_N_ASK);
	m->u.notify->token = hdr->msg_token;
	set_value(m->u.notify->pid, hdr->msg_pid);
	set_value(m->u.notify->task_cookie, task);
	set_value(m->u.notify->rule_id, rule_id);
	set_value(m->u.notify->uid, hdr->msg_uid);
	set_value(m->u.notify->subsystem, hdr->msg_source);
	set_value(m->u.notify->operation, 0 /* XXX ?? */);
	set_value(m->u.notify->evoff, 0);
	set_value(m->u.notify->evlen, extra);
	memcpy(m->u.notify->payload, &hdr[1], extra);
	if (cslen) {
		set_value(m->u.notify->csumoff, extra);
		set_value(m->u.notify->csumlen, cslen);
		memcpy(m->u.notify->payload + extra,
		    eventask->payload + eventask->csumoff, cslen);
	} else {
		set_value(m->u.notify->csumoff, 0);
		set_value(m->u.notify->csumlen, 0);
	}
	if (plen) {
		set_value(m->u.notify->pathoff, extra + cslen);
		set_value(m->u.notify->pathlen, plen);
		memcpy(m->u.notify->payload + extra + cslen,
		    eventask->payload + eventask->pathoff, plen);
	} else {
		set_value(m->u.notify->pathoff, 0);
		set_value(m->u.notify->pathlen, 0);
	}
	head = anoubis_notify_create_head(m, notify_callback, cbdata);
	cbdata->ev_head = head;

	if (!head) {
		/* malloc failure, then we don't send the message */
		anoubis_msg_free(m);
		free(cbdata);
		DEBUG(DBG_TRACE, "<dispatch_p2s_evt_request (bad head)");
		return;
	}

	sent = 0;
	LIST_FOREACH(sess, &ev_info->seg->sessionList, nextSession) {
		struct anoubis_notify_group * ng;
		int ret;

		if (sess->proto == NULL)
			continue;
		ng = anoubis_server_getnotify(sess->proto);
		if (!ng)
			continue;
		ret = anoubis_notify(ng, head);
		if (ret < 0)
			log_warn("anoubis_notify: Error code %d", -ret);
		if (ret > 0) {
			DEBUG(DBG_TRACE, " >anoubis_notify: %x",
			    hdr->msg_token);
			sent = 1;
		}
	}

	if (sent) {
		enqueue(&headq, cbdata);
		DEBUG(DBG_TRACE, " >headq: %x", hdr->msg_token);
	} else {
		anoubis_notify_destroy_head(head);
		notify_callback(NULL, EPERM, cbdata);
		DEBUG(DBG_TRACE, ">anoubis_notify_destroy_head");
	}

	DEBUG(DBG_TRACE, "<dispatch_p2s_evt_request");
}

static int
cbdata_cmp(void *msg1, void *msg2)
{
	if (msg1 == NULL || msg2 == NULL) {
		DEBUG(DBG_TRACE, "cbdata_cmp: null msg pointer");
		return 0;
	}
	if (((struct cbdata *)msg1)->ev_token ==
	    ((struct cbdata *)msg2)->ev_token)
		return 1;
	return 0;
}

static void
dispatch_p2s_evt_cancel(anoubisd_msg_t *msg,
    struct event_info_session *ev_info __used)
{
	struct anoubis_notify_head *head;
	struct cbdata	*cbdata;
	struct cbdata	cbdatatmp;

	DEBUG(DBG_TRACE, ">dispatch_p2s_evt_cancel");

	cbdatatmp.ev_token = *(eventdev_token*)msg->msg;
	if ((cbdata = queue_find(&headq, &cbdatatmp, cbdata_cmp))) {
		head = cbdata->ev_head;
		anoubis_notify_sendreply(head, EPERM /* XXX RD */,
		    NULL, 0);
		DEBUG(DBG_TRACE, " >anoubis_notify_sendreply: %x",
			    cbdata->ev_token);
	}

	DEBUG(DBG_TRACE, "<dispatch_p2s_evt_cancel");
}

static int
pol_reply_cmp(void *msg1, void *msg2)
{
	if (msg1 == NULL || msg2 == NULL) {
		DEBUG(DBG_TRACE, "pol_reply_cmp: null msg pointer");
		return 0;
	}
	if (((struct anoubisd_msg_comm_store *)msg1)->token ==
	    ((struct anoubisd_msg_comm_store *)msg2)->token)
		return 1;
	return 0;
}

static void
dispatch_p2s_pol_reply(anoubisd_msg_t *msg,
    struct event_info_session *ev_info __used)
{
	anoubisd_reply_t *reply;
	struct anoubisd_msg_comm_store msg_comm_store_tmp, *msg_comm;
	void *buf = NULL;
	int end, ret;

	DEBUG(DBG_TRACE, ">dispatch_p2s_pol_reply");

	reply = (anoubisd_reply_t *)msg->msg;
	msg_comm_store_tmp.token = reply->token;
	msg_comm = queue_find(&requestq, &msg_comm_store_tmp,  pol_reply_cmp);
	if (msg_comm == NULL) {
		log_warn("dispatch_p2s_pol_reply: comm not found for 0x%x",
		    reply->token);
		DEBUG(DBG_TRACE, "<dispatch_p2s_pol_reply (not found)");
		return;
	}

	if (reply->len)
		buf = reply->msg;

	end = reply->flags & POLICY_FLAG_END;
	ret = anoubis_policy_comm_answer(msg_comm->comm, reply->token,
	    reply->reply, buf, reply->len, end != 0);
	DEBUG(DBG_TRACE, " >anoubis_policy_comm_answer: %lld %d %d",
	    (long long)reply->token, ret, reply->reply);

	DEBUG(DBG_QUEUE, " <requestq: %x", reply->token);
	if (end) {
		queue_delete(&requestq, msg_comm);
		free(msg_comm);
	}

	DEBUG(DBG_TRACE, "<dispatch_p2s_pol_reply");
}

static void
dispatch_m2s_pol_reply(anoubisd_msg_t *msg, struct event_info_session *ev_info)
{
	anoubisd_reply_t	*reply;
	int			 ret, end;

	reply = (anoubisd_reply_t *)msg->msg;
	end = reply->flags & POLICY_FLAG_END;
	ret = anoubis_policy_comm_answer(ev_info->policy, reply->token,
	    reply->reply, reply->msg, reply->len, end);
	if (ret < 0) {
		errno = -ret;
		log_warn("dispatch_m2s_pol_reply: Failed to process answer");
	}
}

static void
dispatch_s2m(int fd, short sig __used, void *arg)
{
	struct event_info_session *ev_info = (struct event_info_session *)arg;
	anoubisd_msg_t *msg;
	int		ret;

	DEBUG(DBG_TRACE, ">dispatch_s2m");

	if ((msg = queue_peek(&eventq_s2m)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_s2m (no msg)");
		return;
	}

	if ((ret = send_msg(fd, msg)) == 1) {
		msg = dequeue(&eventq_s2m);
		DEBUG(DBG_QUEUE, " <eventq_s2m: %x",
		    ((struct eventdev_reply *)msg->msg)->msg_token);
		free(msg);
	} else if (ret == -1) {
		msg = dequeue(&eventq_s2m);
		DEBUG(DBG_QUEUE, " <eventq_s2m: dropping %x",
		    ((struct eventdev_reply *)msg->msg)->msg_token);
		free(msg);
	}

	/* If the queue is not empty, we want to be called again */
	if (queue_peek(&eventq_s2m) || msg_pending(fd))
		event_add(ev_info->ev_s2m, NULL);

	DEBUG(DBG_TRACE, "<dispatch_s2m");
}

static void
dispatch_s2p(int fd, short sig __used, void *arg)
{
	struct event_info_session *ev_info = (struct event_info_session *)arg;
	anoubisd_msg_t *msg;
	int		ret;

	DEBUG(DBG_TRACE, ">dispatch_s2p");

	if ((msg = queue_peek(&eventq_s2p)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_s2p (no msg)");
		return;
	}

	if ((ret = send_msg(fd, msg)) == 1) {
		msg = dequeue(&eventq_s2p);
		DEBUG(DBG_QUEUE, " <eventq_s2p: %x",
		    ((struct eventdev_reply *)msg->msg)->msg_token);
		free(msg);
	} else if (ret == -1) {
		msg = dequeue(&eventq_s2p);
		DEBUG(DBG_QUEUE, " <eventq_s2p: dropping %x",
		    ((struct eventdev_reply *)msg->msg)->msg_token);
		free(msg);
	}

	/* If the queue is not empty, we want to be called again */
	if (queue_peek(&eventq_s2p) || msg_pending(fd))
		event_add(ev_info->ev_s2p, NULL);

	DEBUG(DBG_TRACE, "<dispatch_s2p");
}

static void
session_destroy(struct session *session)
{
	DEBUG(DBG_TRACE, ">session_destroy");

	if (session->proto) {
		anoubis_server_destroy(session->proto);
		session->proto = NULL;
	}
	event_del(&(session->ev_rdata));
	event_del(&(session->ev_wdata));
	msg_release(session->connfd);
	session->connfd = -1;
	acc_destroy(session->channel);

	LIST_REMOVE(session, nextSession);
	bzero(session, sizeof(struct session));

	free((void *)session);

	DEBUG(DBG_TRACE, "<session_destroy");
}

void
session_setupuds(struct sessionGroup *seg, struct anoubisd_config * conf)
{
	struct sockaddr_storage	 ss;
	achat_rc		 rc = ACHAT_RC_ERROR;

	DEBUG(DBG_TRACE, ">session_setupuds");

	seg->keeper_uds = acc_create();
	if (seg->keeper_uds == NULL) {
		log_warn("session_setupuds: acc_create");
		return;
	}

	rc = acc_settail(seg->keeper_uds, ACC_TAIL_SERVER);
	if (rc != ACHAT_RC_OK) {
		log_warn("session_setupuds: acc_settail");
		acc_destroy(seg->keeper_uds);
		return;
	}

	rc = acc_setsslmode(seg->keeper_uds, ACC_SSLMODE_CLEAR);
	if (rc != ACHAT_RC_OK) {
		log_warn("session_setupuds: acc_setsslmode");
		acc_destroy(seg->keeper_uds);
		return;
	}

	bzero(&ss, sizeof(ss));
	((struct sockaddr_un *)&ss)->sun_family = AF_UNIX;
	strlcpy(((struct sockaddr_un *)&ss)->sun_path, conf->unixsocket,
	    sizeof(((struct sockaddr_un *)&ss)->sun_path));
	rc = acc_setaddr(seg->keeper_uds, &ss, sizeof(struct sockaddr_un));
	if (rc != ACHAT_RC_OK) {
		log_warn("session_setupuds: acc_setaddr");
		acc_destroy(seg->keeper_uds);
		return;
	}

	rc = acc_prepare(seg->keeper_uds);
	if (rc != ACHAT_RC_OK) {
		log_warn("session_setupuds: acc_prepare");
		acc_destroy(seg->keeper_uds);
		return;
	}

	DEBUG(DBG_TRACE, "<session_setupuds");
}
/*@=memchecks@*/
