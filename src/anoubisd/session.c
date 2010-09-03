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
	struct event	*ev_m2s, *ev_p2s;
	struct sessionGroup *seg;
	struct anoubis_policy_comm *policy;
	struct event	*ev_sigs[10];
};

struct cbdata {
	TAILQ_ENTRY(cbdata)		 next;
	eventdev_token			 ev_token;
	struct event_info_session	*ev_info;
	struct anoubis_notify_head	*ev_head;
};

static void	session_sighandler(int, short, void *);
static void	notify_callback(struct anoubis_notify_head *, int, void *);
static int	dispatch_policy(struct anoubis_policy_comm *, u_int64_t,
		    u_int32_t, void *, size_t, void *, int);
static void	dispatch_authdata(struct anoubis_server *,
		    struct anoubis_msg *, uid_t auth_uid, void *);
static void	dispatch_checksum(struct anoubis_server *server,
		    struct anoubis_msg *msg, uid_t uid, void *arg);
static int	dispatch_checksum_reply(void *cbdata, int error, void *data,
		    int len, int end);
static int	dispatch_checksum_list_reply(void *cbdata, int error,
		    void *data, int len, int end);
static void	dispatch_csmulti(struct anoubis_server *,
		    struct anoubis_msg *, uid_t, void *);
static void	dispatch_pglist(struct anoubis_server *,
		    struct anoubis_msg *, uid_t, void *);
static void	dispatch_pgcommit(struct anoubis_server *,
		    struct anoubis_msg *, uid_t, void *);
static int	dispatch_csmulti_reply(void *, int, void *, int, int);
static int	dispatch_pglist_reply(void *, int, void *, int, int);
static void	dispatch_m2s(int, short, void *);
static void	dispatch_s2m(int, short, void *);
static void	dispatch_s2p(int, short, void *);
static void	dispatch_p2s(int, short, void *);
static void	dispatch_p2s_evt_request(struct anoubisd_msg *,
		    struct event_info_session *);
static void	dispatch_p2s_log_request(struct anoubisd_msg *,
		    struct event_info_session *);
void		dispatch_p2s_policychange(struct anoubisd_msg *,
		    struct event_info_session *);
static void	dispatch_p2s_evt_cancel(struct anoubisd_msg *,
		    struct event_info_session *);
static void	dispatch_p2s_pol_reply(struct anoubisd_msg *,
		    struct event_info_session *);
static void	dispatch_p2s_pg_reply(struct anoubisd_msg *,
		    struct event_info_session *);
static void	dispatch_p2s_commit_reply(struct anoubisd_msg *msg,
		    struct event_info_session *ev_info);
static void	dispatch_m2s_checksum_reply(struct anoubisd_msg *msg,
		    struct event_info_session *ev_info);
static void	dispatch_m2s_upgrade_notify(struct anoubisd_msg *msg,
		    struct event_info_session *);
static void	session_connect(int, short, void *);
static void	session_rxclient(int, short, void *);
static void	session_txclient(int, short, void *);
static void	session_setupuds(struct sessionGroup *);
static void	session_destroy(struct session *);
static int	dispatch_generic_reply(void *cbdata, int error,
		    void *data, int len, int orig_opcode);
static void	dispatch_passphrase(struct anoubis_server *,
		    struct anoubis_msg *, uid_t, void *);

static Queue eventq_s2p;
static Queue eventq_s2m;

static TAILQ_HEAD(, cbdata)		headq;

static void
session_sighandler(int sig, short event __used, void *arg)
{
	DEBUG(DBG_TRACE, ">session_sighandler");
	switch (sig) {
	case SIGINT:
	case SIGTERM: {
		struct event_info_session	*info;
		struct session			*sess;
		sigset_t			 mask;
		int				 i;

		info = arg;
		event_del(&info->seg->ev_connect);
		while ((sess = LIST_FIRST(&info->seg->sessionList)))
			session_destroy(sess);
		/* We are terminating: Deregister signal handlers. */
		sigfillset(&mask);
		sigprocmask(SIG_SETMASK, &mask, NULL);
		for (i=0; info->ev_sigs[i]; ++i) {
			signal_del(info->ev_sigs[i]);
		}
		break;
	}
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

	/*
	 * The listening channel (seg->keeper_uds) is non-blocking which
	 * will be inherited by the new channel on opendup. Thus there is
	 * no need to set the new channel to non-blocking. Beside this
	 * wouldn't work anyway because after acc_opendup it is too late.
	 */
	session->channel = acc_opendup(seg->keeper_uds);
	if (session->channel == NULL) {
		log_warn("session_connect: acc_opendup");
		free((void *)session);
		DEBUG(DBG_TRACE, "<session_connect (opendup)");
		return;
	}
	if (session->channel->euid != 0 &&
	    session->channel->euid != (uid_t) -1) {
		int conn_count = 0;
		struct session *sp;
		LIST_FOREACH(sp, &seg->sessionList, nextSession) {
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

	session->proto = anoubis_server_create(session->channel, info->policy);
	if (session->proto == NULL) {
		log_warnx("cannot create server protocol handler");
		acc_close(session->channel);
		acc_destroy(session->channel);
		free(session);
		DEBUG(DBG_TRACE, "<session_connect (create)");
		return;
	}
	anoubis_dispatch_create(session->proto, ANOUBIS_P_CSUMREQUEST,
	    &dispatch_checksum, info);
	anoubis_dispatch_create(session->proto, ANOUBIS_P_CSMULTIREQUEST,
	    &dispatch_csmulti, info);
	anoubis_dispatch_create(session->proto, ANOUBIS_P_PASSPHRASE,
	    &dispatch_passphrase, NULL);
	anoubis_dispatch_create(session->proto, ANOUBIS_P_PGLISTREQ,
	    &dispatch_pglist, info);
	anoubis_dispatch_create(session->proto, ANOUBIS_P_PGCOMMIT,
	    &dispatch_pgcommit, info);
	anoubis_dispatch_create(session->proto, ANOUBIS_C_AUTHDATA,
	    &dispatch_authdata, info);
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
	msg_init(session->connfd);

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

static void
session_txclient(int fd __used, short event __used, void *arg)
{
	struct session	*sess = arg;
	/* acc_flush will re-add the event if needed. */
	acc_flush(sess->channel);
}

pid_t
session_main(int pipes[], int loggers[])
{
	int		 masterfd, policyfd, logfd;
	struct event	 ev_sigterm, ev_sigint, ev_sigquit;
	struct event	 ev_m2s, ev_p2s;
	struct event	 ev_s2m, ev_s2p;
	struct event_info_session	ev_info;
	struct sessionGroup		seg;
	struct passwd	*pw;
	sigset_t	 mask;
	pid_t		 pid;
#ifdef LINUX
	int		 dazukofd;
#endif


	pid = fork();
	if (pid == -1)
		fatal("fork");
		/* NOTREACHED */
	if (pid)
		return (pid);

	(void)event_init();

	anoubisd_process = PROC_SESSION;

	masterfd = policyfd = logfd = -1;
	SWAP(masterfd, pipes[PIPE_MAIN_SESSION + 1]);
	SWAP(policyfd, pipes[PIPE_SESSION_POLICY]);
	SWAP(logfd, loggers[anoubisd_process]);
	cleanup_fds(pipes, loggers);

	log_init(logfd);

	/* while still privileged we install a listening socket */
	session_setupuds(&seg);
#ifdef LINUX
	dazukofd = dazukofs_ignore();
#endif

	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		fatal("getpwnam");

	if (chroot(pw->pw_dir) == -1)
		fatal("chroot");
	if (chdir("/") == -1)
		fatal("chdir");
	log_info("session started (pid %d root %s)", getpid(), pw->pw_dir);

	setproctitle("session engine");

	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		fatal("can't drop privileges");


	/* From now on, this is an unprivileged child process. */
	LIST_INIT(&(seg.sessionList));
	TAILQ_INIT(&headq);

	/* We catch or block signals rather than ignoring them. */
	signal_set(&ev_sigterm, SIGTERM, session_sighandler, &ev_info);
	signal_set(&ev_sigint, SIGINT, session_sighandler, &ev_info);
	signal_set(&ev_sigquit, SIGQUIT, session_sighandler, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);
	ev_info.ev_sigs[0] = &ev_sigterm;
	ev_info.ev_sigs[1] = &ev_sigint;
	ev_info.ev_sigs[2] = &ev_sigquit;
	ev_info.ev_sigs[3] = NULL;

	anoubisd_defaultsigset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	/* init msg_bufs - keep track of outgoing ev_info */
	msg_init(masterfd);
	msg_init(policyfd);

	/* master process */
	event_set(&ev_m2s, masterfd, EV_READ | EV_PERSIST, dispatch_m2s,
	    &ev_info);
	event_add(&ev_m2s, NULL);

	event_set(&ev_s2m, masterfd, EV_WRITE, dispatch_s2m, NULL);

	/* policy process */
	event_set(&ev_p2s, policyfd, EV_READ | EV_PERSIST, dispatch_p2s,
	    &ev_info);
	event_add(&ev_p2s, NULL);

	event_set(&ev_s2p, policyfd, EV_WRITE, dispatch_s2p, NULL);

	queue_init(&eventq_s2p, &ev_s2p);
	queue_init(&eventq_s2m, &ev_s2m);

	ev_info.ev_m2s = &ev_m2s;
	ev_info.ev_p2s = &ev_p2s;
	ev_info.seg = &seg;
	ev_info.policy = anoubis_policy_comm_create(&dispatch_policy, NULL);
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
		log_warnx("session_main: close remaining session by force");
		/* session_destroy will call LIST_REMOVE. */
		session_destroy(session);
	}
	acc_destroy(seg.keeper_uds);

	_exit(0);
}

/* Handle Notify and Alert Messages - coming from master */
static void
notify_callback(struct anoubis_notify_head *head, int verdict, void *_cbdata)
{
	struct anoubisd_msg		*msg;
	struct eventdev_reply		*rep;
	struct cbdata			*cbdata = _cbdata;

	DEBUG(DBG_TRACE, ">notify_callback");

	if (cbdata == NULL) {
		log_warnx("notify_callback: null pointer");
		master_terminate(EINVAL);
		return;
	}

	msg = msg_factory(ANOUBISD_MSG_EVENTREPLY,
	    sizeof(struct eventdev_reply));
	if (!msg) {
		master_terminate(ENOMEM);
		return;
	}
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

static void
__send_notify(struct anoubis_msg *m, struct event_info_session *ev_info)
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

	LIST_FOREACH(sess, &ev_info->seg->sessionList, nextSession) {
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

static void
dispatch_csmulti(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t uid, void *arg)
{
	struct event_info_session	*ev_info = arg;
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
	if (!s2m_msg) {
		master_terminate(ENOMEM);
		return;
	}
	chan = anoubis_server_getchannel(server);
	csum_msg = (struct anoubisd_msg_csumop *)(s2m_msg->msg),
	csum_msg->uid = uid;
	csum_msg->len = m->length;
	memcpy(csum_msg->msg, m->u.buf, m->length);
	err = anoubis_policy_comm_addrequest(ev_info->policy, chan,
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

static void
dispatch_pglist(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t auth_uid, void *arg)
{
	struct anoubisd_msg			*msg;
	struct anoubisd_msg_pgrequest		*pgmsg;
	struct event_info_session		*ev_info = arg;
	struct achat_channel			*chan;
	int					 err;

	DEBUG(DBG_TRACE, ">dispatch_pglist");

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_PgRequestMessage))) {
		dispatch_pglist_reply(server, EFAULT, NULL, 0,
		    POLICY_FLAG_START | POLICY_FLAG_END);
		DEBUG(DBG_TRACE, "<dispatch_pglist(error): verify length");
		return;
	}

	msg = msg_factory(ANOUBISD_MSG_PGREQUEST,
	    sizeof(struct anoubisd_msg_pgrequest));
	if (!msg) {
		master_terminate(ENOMEM);
		return;
	}

	pgmsg = (struct anoubisd_msg_pgrequest *)msg->msg;
	pgmsg->auth_uid = auth_uid;
	pgmsg->listtype = get_value(m->u.pgreq->listtype);
	pgmsg->pgid = get_value(m->u.pgreq->pgid);
	chan = anoubis_server_getchannel(server);
	err = anoubis_policy_comm_addrequest(ev_info->policy, chan,
	    POLICY_FLAG_START | POLICY_FLAG_END, &dispatch_pglist_reply,
	    NULL, server, &pgmsg->token);
	if (err < 0) {
		log_warnx("Dropping pglist request (error %d)", -err);
		free(msg);
		return;
	}

	enqueue(&eventq_s2p, msg);
	DEBUG(DBG_QUEUE, " >eventq_s2p");
	DEBUG(DBG_TRACE, "<dispatch_pglist");
}

static void
dispatch_pgcommit(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t auth_uid, void *arg)
{
	struct anoubisd_msg			*msg;
	struct anoubisd_msg_pgcommit		*pgmsg;
	struct event_info_session		*ev_info = arg;
	struct achat_channel			*chan;
	int					 err, len;

	DEBUG(DBG_TRACE, ">dispatch_pgcommit");

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_PgCommitMessage))
	    || PAYLOAD_LEN(m, pgcommit, payload) < 2) {
		dispatch_generic_reply(server, EINVAL, NULL, 0,
		    ANOUBIS_P_PGCOMMIT);
		DEBUG(DBG_TRACE, "<dispatch_pglist(error): verify length");
		return;
	}
	/* Force NULL termination on the string. */
	len = PAYLOAD_LEN(m, pgcommit, payload);
	m->u.pgcommit->payload[len-1] = 0;
	len = strlen(m->u.pgcommit->payload) + 1;

	msg = msg_factory(ANOUBISD_MSG_PGCOMMIT,
	    sizeof(struct anoubisd_msg_pgcommit) + len);
	if (!msg) {
		master_terminate(ENOMEM);
		return;
	}

	pgmsg = (struct anoubisd_msg_pgcommit *)msg->msg;
	pgmsg->auth_uid = auth_uid;
	pgmsg->ignore_recommended_scanners =
	    m->u.pgcommit->ignore_recommended_scanners;
	pgmsg->pgid = get_value(m->u.pgcommit->pgid);
	pgmsg->dev = 0;
	pgmsg->ino = 0;
	memcpy(pgmsg->path, m->u.pgcommit->payload, len);
	chan = anoubis_server_getchannel(server);
	err = anoubis_policy_comm_addrequest(ev_info->policy, chan,
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

static void
dispatch_checksum(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t uid, void *arg)
{
	struct anoubisd_msg		*s2m_msg;
	struct anoubisd_msg_csumop	*msg_csum;
	struct achat_channel		*chan;
	int				 err, opp = 0, reallen;

	struct event_info_session *ev_info = arg;

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
	if (!s2m_msg) {
		master_terminate(ENOMEM);
		return;
	}
	msg_csum = (struct anoubisd_msg_csumop *)s2m_msg->msg;
	msg_csum->uid = uid;
	msg_csum->len = m->length;
	memcpy(msg_csum->msg, m->u.checksumrequest, m->length);
	if (opp == ANOUBIS_CHECKSUM_OP_GENERIC_LIST) {
		err = anoubis_policy_comm_addrequest(ev_info->policy, chan,
		    POLICY_FLAG_START | POLICY_FLAG_END,
		    &dispatch_checksum_list_reply, NULL, server,
		    &msg_csum->token);
	} else {
		err = anoubis_policy_comm_addrequest(ev_info->policy, chan,
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
	if (!msg) {
		master_terminate(ENOMEM);
		return;
	}
	pass = (anoubisd_msg_passphrase_t *)msg->msg;
	memcpy(pass->payload, m->u.passphrase->payload, plen);

	enqueue(&eventq_s2m, msg);
	DEBUG(DBG_QUEUE, " >eventq_s2m");
	DEBUG(DBG_TRACE, "<dispatch_passphrase");
}

/*
 * Handle a challenge received from the master.
 * Invariantes ensured by the way anoubis_policy_answer is called:
 * - data points to an anoubis_msg_authchallenge and the length in this
 *   message matches the lenght values in the challenge message.
 * - error is the error code from the challenge.
 * - end must be TRUE.
 */
static int
dispatch_auth_challenge(void *cbdata, int error __used, void *data,
    int len, int end __used)
{
	struct anoubis_server			*server = cbdata;
	struct anoubis_auth			*auth;
	struct anoubisd_msg_authchallenge	*challenge = data;
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
	if (!auth->auth_private) {
		master_terminate(ENOMEM);
		return -ENOMEM;
	}
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

static int
dispatch_auth_result(void *cbdata, int error, void *data __used,
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

static void
dispatch_authdata(struct anoubis_server *server, struct anoubis_msg *m,
    uid_t auth_uid __used, void *arg)
{
	struct anoubis_auth		*auth = anoubis_server_getauth(server);
	int				 err = ANOUBIS_E_INVAL;
	struct event_info_session	*ev_info = arg;

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
		if (!msg) {
			master_terminate(ENOMEM);
			return;
		}
		authreq = (struct anoubisd_msg_authrequest *)msg->msg;
		authreq->auth_uid = auth->chan->euid;
		err = - anoubis_policy_comm_addrequest(ev_info->policy,
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
		if (!msg) {
			master_terminate(ENOMEM);
			return;
		}
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
		err = - anoubis_policy_comm_addrequest(ev_info->policy,
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
	auth->error = err;
	auth->state = ANOUBIS_AUTH_FAILURE;
	auth->uid = -1;
	auth->finish_callback(auth->cbdata);
	return;
}

static int
dispatch_generic_reply(void *cbdata, int error, void *data, int len,
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

static int
dispatch_checksum_list_reply(void *cbdata, int error, void *data, int len,
    int flags)
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

static int
dispatch_checksum_reply(void *cbdata, int error, void *data, int len, int flags)
{
	if (flags != (POLICY_FLAG_START | POLICY_FLAG_END))
		log_warnx("Wrong flags value in dispatch_checksum_reply");
	return dispatch_generic_reply(cbdata, error, data, len,
	    ANOUBIS_P_CSUMREQUEST);
}

static int
dispatch_csmulti_reply(void *cbdata, int error, void *data, int len, int flags)
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

static int
dispatch_pglist_reply(void *cbdata, int error, void *data, int len, int flags)
{
	struct anoubis_server		*server = cbdata;
	struct anoubis_msg		*m;
	int				 ret;

	DEBUG(DBG_TRACE, ">dispatch_pglist_reply");
	if (flags & ~(POLICY_FLAG_START | POLICY_FLAG_END))
		log_warnx("Wrong flags value in dispatch_pglist_reply");
	if (error)
		goto err;
	if (len < (int)sizeof(Anoubis_PgReplyMessage)) {
		log_warnx(" dispatch_pglist_reply: "
		    "Short pglist reply message: len=%d\n", len);
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
	DEBUG(DBG_TRACE, "<dispatch_pglist_reply: error=%d", -ret);
	return ret;
err:
	m = anoubis_msg_new(sizeof(Anoubis_PgReplyMessage));
	if (!m)
		return -ENOMEM;
	set_value(m->u.pgreply->type, ANOUBIS_P_PGLISTREP);
	set_value(m->u.pgreply->flags, flags | POLICY_FLAG_END);
	set_value(m->u.pgreply->error, error);
	set_value(m->u.pgreply->nrec, 0);
	set_value(m->u.pgreply->rectype, 0);
	ret = anoubis_msg_send(anoubis_server_getchannel(server), m);
	anoubis_msg_free(m);
	DEBUG(DBG_TRACE, "<dispatch_pglist_reply: error=%d ret=%d",
	    error, ret);
	return ret;
}

static int
dispatch_policy(struct anoubis_policy_comm *comm __used, uint64_t token,
    uint32_t uid, void *buf, size_t len, void *arg __used, int flags)
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
		if (!msg) {
			master_terminate(ENOMEM);
			return -ENOMEM;
		}
		abort = (struct anoubisd_msg_polrequest_abort *)msg->msg;
		abort->token = token;
	} else {
		if (verify_polrequest(buf, len, flags) < 0) {
			log_warnx("Dropping malformed policy request");
			return -EINVAL;
		}
		msg = msg_factory(ANOUBISD_MSG_POLREQUEST,
		    sizeof(struct anoubisd_msg_polrequest) + len);
		if (!msg) {
			master_terminate(ENOMEM);
			return -ENOMEM;
		}
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

/* Handle Notify and Alert Messages - coming from master */
static void
dispatch_m2s(int fd, short sig __used, void *arg)
{
	struct event_info_session	*ev_info = arg;
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
			dispatch_m2s_checksum_reply(msg, ev_info);
			free(msg);
			continue;
		}
		if (msg->mtype == ANOUBISD_MSG_UPGRADE) {
			dispatch_m2s_upgrade_notify(msg, ev_info);
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
			ret = anoubis_policy_comm_answer(ev_info->policy,
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
			ret = anoubis_policy_comm_answer(ev_info->policy,
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
		__send_notify(m, ev_info);

		DEBUG(DBG_TRACE, "<dispatch_m2s (loop)");
	}
	if (msg_eof(fd)) {
		session_sighandler(SIGTERM, 0, ev_info);
		event_del(ev_info->ev_m2s);
	}

	DEBUG(DBG_TRACE, "<dispatch_m2s");
}

/* Handle Request Messages - coming from policy */
static void
dispatch_p2s(int fd, short sig __used, void *arg)
{
	struct event_info_session	*ev_info = arg;
	struct eventdev_hdr		*hdr;
	struct anoubisd_msg		*msg;

	DEBUG(DBG_TRACE, ">dispatch_p2s");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL)
			break;

		switch (msg->mtype) {
		case ANOUBISD_MSG_POLREPLY:
			DEBUG(DBG_QUEUE, " >p2s: polreply");
			dispatch_p2s_pol_reply(msg, ev_info);
			break;

		case ANOUBISD_MSG_PGREPLY:
			DEBUG(DBG_QUEUE, " >p2s: pgreply");
			dispatch_p2s_pg_reply(msg, ev_info);
			break;

		case ANOUBISD_MSG_PGCOMMIT_REPLY:
			DEBUG(DBG_QUEUE, " >p2s: commitreply");
			dispatch_p2s_commit_reply(msg, ev_info);
			break;

		case ANOUBISD_MSG_EVENTASK:
			DEBUG(DBG_QUEUE, ">p2s: eventask");
			dispatch_p2s_evt_request(msg, ev_info);
			break;

		case ANOUBISD_MSG_LOGREQUEST:
			DEBUG(DBG_QUEUE, " >p2s: log");
			dispatch_p2s_log_request(msg, ev_info);
			break;

		case ANOUBISD_MSG_POLICYCHANGE:
			DEBUG(DBG_QUEUE, " >p2s: policychange");
			dispatch_p2s_policychange(msg, ev_info);
			break;

		case ANOUBISD_MSG_EVENTCANCEL:
			hdr = (struct eventdev_hdr *)msg->msg;
			DEBUG(DBG_QUEUE, " >p2s: %x",
			    hdr->msg_token);
			dispatch_p2s_evt_cancel(msg, ev_info);
			break;

		default:
			log_warnx("dispatch_p2s: bad mtype %d", msg->mtype);
			break;
		}

		free(msg);

		DEBUG(DBG_TRACE, "<dispatch_p2s (loop)");
	}
	if (msg_eof(fd)) {
		session_sighandler(SIGTERM, 0, ev_info);
		event_del(ev_info->ev_p2s);
	}
	DEBUG(DBG_TRACE, "<dispatch_p2s");
}

void
dispatch_p2s_policychange(struct anoubisd_msg *msg,
    struct event_info_session *ev_info)
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
	__send_notify(m, ev_info);
	DEBUG(DBG_TRACE, "<dispatch_p2s_policychange");
}

/*
 * Copy @srclen bytes from @srcbuf to @dstbuf. The data in @srcbuf starts
 * at offset @srcoff and the target of the copy is the offset pointed to
 * by @offp. The latter is updated after the data was copied.
 * The target offset and the total length of the data copied are stored
 * in @roffp nad @rlenp respectively.
 */
static void
do_copy(char *dstbuf, int *offp, const char *srcbuf, int srcoff, int srclen,
    u16n *roffp, u16n *rlenp)
{
	int		 off = *offp;
	char		*dstp = dstbuf + *offp;
	const char	*srcp = srcbuf + srcoff;

	set_value(*roffp, off);
	set_value(*rlenp, srclen);
	*offp += srclen;
	memcpy(dstp, srcp, srclen);
}

static struct anoubis_msg *
eventask_to_notify(int notifytype, struct anoubisd_msg_eventask *eventask)
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

void
dispatch_p2s_log_request(struct anoubisd_msg *msg,
    struct event_info_session *ev_info)
{
	struct anoubisd_msg_eventask	*eventask;
	struct anoubis_msg		*m;

	DEBUG(DBG_TRACE, ">dispatch_p2s_log_request");

	eventask = (struct anoubisd_msg_eventask *)msg->msg;
	m = eventask_to_notify(ANOUBIS_N_LOGNOTIFY, eventask);
	if (!m) {
		log_warnx("Out of memory in dispatch_p2s_log_request");
		return;
	}
	__send_notify(m, ev_info);
	DEBUG(DBG_TRACE, "<dispatch_p2s_log_request");
}

static void
dispatch_p2s_evt_request(struct anoubisd_msg *msg,
    struct event_info_session *ev_info)
{
	struct anoubis_notify_head	*head;
	struct anoubis_msg		*m;
	struct anoubisd_msg_eventask	*eventask = NULL;
	struct session			*sess;
	struct cbdata			*cbdata;
	int				 sent;

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
	cbdata->ev_info = ev_info;

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

static void
dispatch_p2s_evt_cancel(struct anoubisd_msg *msg,
    struct event_info_session *ev_info __used)
{
	struct anoubis_notify_head	*head;
	eventdev_token			*tokenp;
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

static void
dispatch_p2s_pol_reply(struct anoubisd_msg *msg,
    struct event_info_session *ev_info)
{
	struct anoubisd_msg_polreply *reply;
	void *buf = NULL;
	int end, ret;

	DEBUG(DBG_TRACE, ">dispatch_p2s_pol_reply");

	reply = (struct anoubisd_msg_polreply *)msg->msg;
	if (reply->len)
		buf = reply->data;
	end = reply->flags & POLICY_FLAG_END;
	ret = anoubis_policy_comm_answer(ev_info->policy, reply->token,
	    reply->reply, buf, reply->len, end != 0);
	if (ret < 0)
		log_warnx("Dropping unexpected policy reply");
	DEBUG(DBG_TRACE, " >anoubis_policy_comm_answer: %" PRId64 " %d %d",
	    reply->token, ret, reply->reply);

	DEBUG(DBG_TRACE, "<dispatch_p2s_pol_reply");
}

/* Does not free the message */
static void
dispatch_p2s_pg_reply(struct anoubisd_msg *msg,
    struct event_info_session *ev_info)
{
	struct anoubisd_msg_pgreply	*pgreply;
	void				*buf = NULL;
	int				 end, ret;

	DEBUG(DBG_TRACE, ">dispatch_p2s_pg_reply");

	pgreply = (struct anoubisd_msg_pgreply *)msg->msg;
	if (pgreply->len)
		buf = pgreply->data;
	end = pgreply->flags & POLICY_FLAG_END;
	ret = anoubis_policy_comm_answer(ev_info->policy, pgreply->token, 0,
	    buf, pgreply->len, end != 0);
	if (ret < 0)
		log_warnx("Dropping unexpected playground list reply");
	DEBUG(DBG_TRACE, ">anoubis_policy_comm_answer: %" PRId64,
	    pgreply->token);
	DEBUG(DBG_TRACE, "<dispatch_p2s_pg_reply");
}

static void
dispatch_p2s_commit_reply(struct anoubisd_msg *msg,
    struct event_info_session *ev_info)
{
	struct anoubisd_msg_pgcommit_reply	*commitreply;
	void					*buf = NULL;
	int					 ret;

	DEBUG(DBG_TRACE, ">dispatch_p2s_commit_reply");
	commitreply = (struct anoubisd_msg_pgcommit_reply *)msg->msg;
	DEBUG(DBG_QUEUE, " dispatch_p2s_commit_reply: token=%" PRId64,
	    commitreply->token);
	if (commitreply->len)
		buf = commitreply->payload;
	ret = anoubis_policy_comm_answer(ev_info->policy, commitreply->token,
	    commitreply->error, buf, commitreply->len,
	    POLICY_FLAG_START | POLICY_FLAG_END);
	if (ret < 0)
		log_warnx("Dropping unexpected playground commit reply");
	DEBUG(DBG_TRACE, "<dispatch_p2s_commit_reply");
}

/* Does not free the message */
static void
dispatch_m2s_checksum_reply(struct anoubisd_msg *msg,
    struct event_info_session *ev_info)
{
	struct anoubisd_msg_csumreply	*reply;
	int				 ret, end;

	reply = (struct anoubisd_msg_csumreply *)msg->msg;
	end = reply->flags & POLICY_FLAG_END;
	ret = anoubis_policy_comm_answer(ev_info->policy, reply->token,
	    reply->reply, reply->data, reply->len, end);
	if (ret < 0) {
		errno = -ret;
		log_warnx("dispatch_m2s_checksum_reply: "
		    "Failed to process answer");
	}
}


/* Does not free the message. */
static void
dispatch_m2s_upgrade_notify(struct anoubisd_msg *msg,
    struct event_info_session *ev_info)
{
	struct anoubisd_msg_upgrade	*umsg;
	int				 count;
	struct anoubis_msg		*m;

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
	__send_notify(m, ev_info);
	DEBUG(DBG_TRACE, "<dispatch_m2s_upgrade_notify");
}

static void
dispatch_s2m(int fd, short sig __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_s2m");
	dispatch_write_queue(&eventq_s2m, fd);
	DEBUG(DBG_TRACE, "<dispatch_s2m");
}

static void
dispatch_s2p(int fd, short sig __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_s2p");
	dispatch_write_queue(&eventq_s2p, fd);
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
session_setupuds(struct sessionGroup *seg)
{
	struct sockaddr_un	 ss;
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
		seg->keeper_uds = NULL;
		return;
	}

	rc = acc_setsslmode(seg->keeper_uds, ACC_SSLMODE_CLEAR);
	if (rc != ACHAT_RC_OK) {
		log_warnx("session_setupuds: acc_setsslmode");
		acc_destroy(seg->keeper_uds);
		seg->keeper_uds = NULL;
		return;
	}
	/*
	 * This will affect the blocking mode of the new connections.
	 */
	rc = acc_setblockingmode(seg->keeper_uds, ACC_NON_BLOCKING);
	if (rc != ACHAT_RC_OK) {
		log_warn("session_setupuds: acc_setblockingmode");
		acc_destroy(seg->keeper_uds);
		seg->keeper_uds = NULL;
		return;
	}

	bzero(&ss, sizeof(ss));
	(&ss)->sun_family = AF_UNIX;
	strlcpy((&ss)->sun_path,
	    anoubisd_config.unixsocket,
	    sizeof((&ss)->sun_path));
	rc = acc_setaddr(seg->keeper_uds, (struct sockaddr_storage *)&ss,
	    sizeof(struct sockaddr_un));
	if (rc != ACHAT_RC_OK) {
		log_warn("session_setupuds: acc_setaddr");
		acc_destroy(seg->keeper_uds);
		seg->keeper_uds = NULL;
		return;
	}

	rc = acc_prepare(seg->keeper_uds);
	if (rc != ACHAT_RC_OK) {
		log_warnx("session_setupuds: acc_prepare");
		acc_destroy(seg->keeper_uds);
		seg->keeper_uds = NULL;
		return;
	}

	DEBUG(DBG_TRACE, "<session_setupuds");
}
/*@=memchecks@*/
