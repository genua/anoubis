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
#ifdef OPENBSD
#include <sys/queue.h>
#else
#include "queue.h"
#endif
#include <sys/un.h>

#include <err.h>
#include <errno.h>
#include <event.h>
#ifdef LINUX
#include <grp.h>
#include <bsdcompat.h>
#endif
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <anoubischat.h>
#include <anoubisd.h>
#include <anoubis_server.h>
#include <anoubis_msg.h>
#include <anoubis_notify.h>

struct session {
	int			 id;	/* session number */
	uid_t			 uid;	/* user id; not authenticated on -1 */
	struct achat_channel	*channel;	/* communication channel */
	struct anoubis_server	*proto;		/* Server protocol handler */
	struct event		 ev_data;	/* event for receiving data */
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
	struct sessionGroup * seg;
};

static TAILQ_HEAD(head_m2s, anoubisd_event_in) eventq_s2f =
    TAILQ_HEAD_INITIALIZER(eventq_s2f);

static void	session_sighandler(int, short, void *);
static void	m2s_dispatch(int, short, void *);
static void	s2m_dispatch(int, short, void *);
static void	s2p_dispatch(int, short, void *);
static void	p2s_dispatch(int, short, void *);
static void	s2f_dispatch(int, short, void *);
static void	session_connect(int, short, void *);
static void	session_rxclient(int, short, void *);
static void	session_setupuds(struct sessionGroup *,
		struct anoubisd_config *);
static void	session_destroy(struct session *);


static void
session_sighandler(int sig, short event, void *arg)
{
	switch (sig) {
	case SIGTERM:
	case SIGINT:
	case SIGQUIT:
		(void)event_loopexit(NULL);
		/* FALLTRHOUGH */
	}
}

static void
session_connect(int fd, short event, void *arg)
{
	struct session		*session = NULL;
	struct sessionGroup	*seg = (struct sessionGroup *)arg;
	static int		 sessionid = 0;

	session = (struct session *)calloc(1, sizeof(struct session));
	if (session == NULL) {
		log_warn("session_connect: calloc");
		return;
	}

	session->id  = sessionid++;
	session->uid = -1; /* this session is not authenticated */

	session->channel = acc_opendup(seg->keeper_uds);
	if (session->channel == NULL) {
		log_warn("session_connect: acc_opendup");
		free((void *)session);
		return;
	}
	session->proto = anoubis_server_create(session->channel);
	if (session->proto == NULL) {
		log_warn("cannot create server protocol handler");
		acc_close(session->channel);
		acc_destroy(session->channel);
		free(session);
		return;
	}
	if (anoubis_server_start(session->proto) < 0) {
		log_warn("Failed to send initial hello");
		session_destroy(session);
		return;
	}

	event_set(&(session->ev_data), session->channel->connfd,
	    EV_READ | EV_PERSIST, session_rxclient, session);
	event_add(&(session->ev_data), NULL);

	LIST_INSERT_HEAD(&(seg->sessionList), session, nextSession);
}

static void
session_rxclient(int fd, short event, void *arg)
{
	struct session	*session;
	struct anoubis_msg * m = NULL;
	achat_rc	 rc;
	size_t		 size;

	session = (struct session *)arg;
	m = anoubis_msg_new(1000 /* XXX */);
	if(!m)
		goto err;
	size = m->length;
	rc = acc_receivemsg(session->channel, m->u.buf, &size);
	if (rc == ACHAT_RC_EOF) {
		anoubis_msg_free(m);
		session_destroy(session);
		return;
	}
	if (rc != ACHAT_RC_OK)
		goto err;
	if (!session->proto)
		goto err;
	anoubis_msg_resize(m, size);
	/*
	 * Return codes: less than zero is an error. Zero means the no
	 * error but message did not fit into protocol stream.
	 */
	if (anoubis_server_process(session->proto, m->u.buf, size) < 0)
		goto err;
	anoubis_msg_free(m);
	if (anoubis_server_eof(session->proto))
		session_destroy(session);
	return;
err:
	if (m)
		anoubis_msg_free(m);
	session_destroy(session);
	log_warn("session_rxclient: error reading client message");
}

pid_t
session_main(struct anoubisd_config *conf, int pipe_m2s[2], int pipe_m2p[2],
    int pipe_s2p[2])
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

	(void)event_init();
	LIST_INIT(&(seg.sessionList));
	TAILQ_INIT(&eventq_s2f);

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

	event_set(&ev_m2s, pipe_m2s[1], EV_READ | EV_PERSIST, m2s_dispatch,
	    &ev_info);
	event_add(&ev_m2s, NULL);

	event_set(&ev_s2m, pipe_m2s[1], EV_WRITE, s2m_dispatch,
	    &ev_info);

	event_set(&ev_p2s, pipe_s2p[1], EV_READ | EV_PERSIST, p2s_dispatch,
	    &ev_info);
	event_add(&ev_p2s, NULL);

	event_set(&ev_s2p, pipe_s2p[1], EV_WRITE, s2p_dispatch,
	    &ev_info);

	ev_info.ev_m2s = &ev_m2s;
	ev_info.ev_s2m = &ev_s2m;
	ev_info.ev_p2s = &ev_p2s;
	ev_info.ev_s2p = &ev_s2p;
	ev_info.seg = &seg;

	/* setup keeper of incomming unix domain socket connections */
	if (seg.keeper_uds != NULL) {
		event_set(&(seg.ev_connect), (seg.keeper_uds)->sockfd,
		    EV_READ | EV_PERSIST, session_connect, &seg);
		event_add(&(seg.ev_connect), NULL);
	}

	if (event_dispatch() == -1)
		fatal("session_main: event_dispatch");

	/* stop events of incomming connects and cleanup sessions */
	event_del(&(seg.ev_connect));
	while (!LIST_EMPTY(&(seg.sessionList))) {
		struct session *session = LIST_FIRST(&(seg.sessionList));
		log_warn("session_main: close remaining session by force");
		LIST_REMOVE(session, nextSession);
		session_destroy(session);
	}
	acc_destroy(seg.keeper_uds);

	_exit(0);
}

static void
m2s_dispatch(int fd, short sig, void *arg)
{
	struct event_info_session *ev_info = (struct event_info_session*)arg;
	struct anoubis_notify_head * head;
	struct anoubis_msg * m;
#define BUFSIZE 8192
	char buf[BUFSIZE];
	int len, extra;
	struct anoubisd_event_in *ev_in = (struct anoubisd_event_in*)buf;
	struct anoubisd_event_in *ev;
	struct session * sess;

	/* First get size of message */
	len = read(fd, buf, sizeof(struct anoubisd_event_in));
	if (len < 0) {
		log_warn("read error");
		return;
	}
	if (len == 0) {
		event_del(ev_info->ev_m2s);
		event_loopexit(NULL);
		return;
	}
	if (len < sizeof(struct anoubisd_event_in)) {
		log_warn("short read");
		return;
	}

	/* Size of remaining message */
	if (ev_in->event_size > (sizeof(buf) -
	    sizeof(struct anoubisd_event_in))) {
		log_warn("message too large");
		return;
	}

	/* Get remainder of the message */
	len = read(fd, buf + sizeof(struct anoubisd_event_in),
	    ev_in->event_size - sizeof(struct anoubisd_event_in));
	if (len < 0) {
		log_warn("read error");
		return;
	}
	if (len == 0) {
		event_del(ev_info->ev_m2s);
		event_loopexit(NULL);
		return;
	}
	if (len < ev_in->event_size - sizeof(struct anoubisd_event_in)) {
		log_warn("short read");
		return;
	}

	if ((ev = malloc(ev_in->event_size)) == NULL) {
		return;
	}

	memcpy(ev, ev_in, ev_in->event_size);

	TAILQ_INSERT_TAIL(&eventq_s2f, ev, events);

	extra = ev_in->hdr.msg_size - sizeof(struct eventdev_hdr);
	m = anoubis_msg_new(sizeof(Anoubis_NotifyMessage) + extra);
	if (!m) {
		/* XXX */
		return;
	}
	set_value(m->u.notify->type, ANOUBIS_N_NOTIFY);
	m->u.notify->token = ev_in->hdr.msg_token;
	set_value(m->u.notify->pid, ev_in->hdr.msg_pid);
	set_value(m->u.notify->rule_id, 0);
	set_value(m->u.notify->uid, ev_in->hdr.msg_uid);
	set_value(m->u.notify->subsystem, ev_in->hdr.msg_source);
	set_value(m->u.notify->operation, 0 /* XXX */);
	memcpy(m->u.notify->payload, ev_in->msg, extra);
	head = anoubis_notify_create_head(ev_in->hdr.msg_pid, m, NULL, NULL);
	if (!head) {
		/* XXX */
		anoubis_msg_free(m);
		return;
	}
	LIST_FOREACH(sess, &ev_info->seg->sessionList, nextSession) {
		struct anoubis_notify_group * ng;

		if (sess->proto == NULL)
			continue;
		ng = anoubis_server_getnotify(sess->proto);
		if (!ng)
			continue;
		anoubis_notify(ng, head);
	}
	anoubis_notify_destroy_head(head);
	/* XXX: Wait for frontend to handle the data */
	//event_add(ev_info->ev_s2f, NULL);

	/* Directly call s2f_dispatch, for now... */
	s2f_dispatch(fd, sig, arg);
}

static void
s2m_dispatch(int fd, short sig, void *arg)
{
	/* XXX MG: Todo */
}

static void
s2p_dispatch(int fd, short sig, void *arg)
{
	/* XXX HJH: Todo */
}

static void
p2s_dispatch(int fd, short sig, void *arg)
{
	/* XXX MG: Todo */
}

static void
s2f_dispatch(int fd, short sig, void *arg)
{
	struct anoubisd_event_in *ev;
	//struct event_info_session *ev_info = (struct event_info_session*)arg;

	if (TAILQ_EMPTY(&eventq_s2f))
		return;

	ev = TAILQ_FIRST(&eventq_s2f);

	/* XXX: Do magical things here */

	TAILQ_REMOVE(&eventq_s2f, ev, events);
	free(ev);

	/* If the queue is not empty, we want to be called again */
	if (!TAILQ_EMPTY(&eventq_s2f)) {
		//event_add(ev_info->ev_s2f, NULL);
		s2f_dispatch(fd, sig, arg);
	}
}

static void
session_destroy(struct session *session)
{
	if (session->proto) {
		anoubis_server_destroy(session->proto);
		session->proto = NULL;
	}
	acc_destroy(session->channel);
	event_del(&(session->ev_data));

	LIST_REMOVE(session, nextSession);
	bzero(session, sizeof(struct session));

	free((void *)session);
}

void
session_setupuds(struct sessionGroup *seg, struct anoubisd_config * conf)
{
	struct sockaddr_storage	 ss;
	achat_rc		 rc = ACHAT_RC_ERROR;

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
	rc = acc_setaddr(seg->keeper_uds, &ss);
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

	return;
}
