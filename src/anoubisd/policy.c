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
#include <anoubis_alloc.h>

#include <anoubis_protocol.h>

static void	policy_sighandler(int, short, void *);
static void	dispatch_timer(int, short, void *);
static void	dispatch_m2p(int, short, void *);
static void	dispatch_p2m(int, short, void *);
static void	dispatch_s2p(int, short, void *);
static void	dispatch_p2s(int, short, void *);

static int	policy_upgrade_fill_chunk(char *buf, int maxlen);

static Queue	eventq_p2m;
static Queue	eventq_p2m_hold;
static Queue	eventq_p2s;

/*
 * Keep track of messages which have been forwarded to the
 * session process. They will be timed out after a configured
 * timeout and a configured default reply will be generated.
 */
struct reply_wait {
	eventdev_token	token;
	int	flags;
	time_t	starttime;
	time_t	timeout;
	int	log;
};
static Queue	replyq;

struct event_info_policy {
	/*@dependent@*/
	struct event	*ev_s2p, *ev_m2p;
	/*@dependent@*/
	struct event	*ev_timer;
	/*@null@*/ /*@temp@*/
	struct timeval	*tv;
	struct event	*ev_sigs[10];
};

static int terminate = 0;

static void
set_terminate(int n, struct event_info_policy *info)
{
	while (terminate < n) {
		++terminate;
		switch (terminate) {
		case 1: {
			sigset_t			 mask;
			int				 i;

			sigfillset(&mask);
			sigprocmask(SIG_SETMASK, &mask, NULL);
			for (i=0; info->ev_sigs[i]; ++i)
				signal_del(info->ev_sigs[i]);
			if (terminate < 1)
				terminate = 1;
			event_del(info->ev_timer);
			break;
		}
		case 2:
			/* No special action required. */
			break;
		case 3:
			dispatch_timer(0, 0, info);
			break;
		}
	}
}


/* ARGSUSED */
static void
policy_sighandler(int sig, short event __used, void *arg)
{
	switch (sig) {
	case SIGUSR1:
		pe_dump();
		break;
	case SIGINT:
	case SIGTERM:
		set_terminate(1, arg);
		break;
	case SIGQUIT:
		(void)event_loopexit(NULL);
		/* FALLTRHOUGH */
	}
}

pid_t
policy_main(int pipes[], int loggers[])
/*@globals undef eventq_p2m, undef eventq_p2s, undef replyq@*/
{
	int		 masterfd, sessionfd, logfd, err;
	struct event	 ev_sigterm, ev_sigint, ev_sigquit, ev_sigusr1;
	struct event	 ev_m2p, ev_s2p;
	struct event	 ev_p2m, ev_p2s;
	struct event	 ev_timer;
	struct timeval	 tv;
	struct event_info_policy ev_info;
	struct passwd	*pw;
	sigset_t	 mask;
	pid_t		 pid;
#ifdef LINUX
	int		 dazukofd;
#endif

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
	dazukofd = dazukofs_ignore();
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

	setproctitle("policy engine");

	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		fatal("can't drop privileges");

	/* From now on, this is an unprivileged child process. */
	log_info("policy started (pid %d root %s)",
	    getpid(), PACKAGE_POLICYDIR);

	/* We catch or block signals rather than ignoring them. */
	signal_set(&ev_sigterm, SIGTERM, policy_sighandler, &ev_info);
	signal_set(&ev_sigint, SIGINT, policy_sighandler, &ev_info);
	signal_set(&ev_sigquit, SIGQUIT, policy_sighandler, NULL);
	signal_set(&ev_sigusr1, SIGUSR1, policy_sighandler, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);
	signal_add(&ev_sigusr1, NULL);
	ev_info.ev_sigs[0] = &ev_sigterm;
	ev_info.ev_sigs[1] = &ev_sigint;
	ev_info.ev_sigs[2] = &ev_sigquit;
	ev_info.ev_sigs[3] = &ev_sigusr1;
	ev_info.ev_sigs[4] = NULL;

	anoubisd_defaultsigset(&mask);
	sigdelset(&mask, SIGUSR1);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	queue_init(&eventq_p2m_hold, NULL);
	queue_init(&replyq, NULL);

	/* init msg_bufs - keep track of outgoing ev_info */
	msg_init(masterfd, "m2p");
	msg_init(sessionfd, "s2p");

	/* master process */
	event_set(&ev_m2p, masterfd, EV_READ | EV_PERSIST, dispatch_m2p,
	    &ev_info);
	event_add(&ev_m2p, NULL);

	event_set(&ev_p2m, masterfd, EV_WRITE, dispatch_p2m, NULL);

	/* session process */
	event_set(&ev_s2p, sessionfd, EV_READ | EV_PERSIST, dispatch_s2p,
	    &ev_info);
	event_add(&ev_s2p, NULL);

	event_set(&ev_p2s, sessionfd, EV_WRITE, dispatch_p2s, NULL);

	queue_init(&eventq_p2m, &ev_p2m);
	queue_init(&eventq_p2s, &ev_p2s);

	ev_info.ev_s2p = &ev_s2p;
	ev_info.ev_m2p = &ev_m2p;

	/* Five second timer for statistics ioctl */
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	ev_info.ev_timer = &ev_timer;
	ev_info.tv = &tv;
	evtimer_set(&ev_timer, &dispatch_timer, &ev_info);
	event_add(&ev_timer, &tv);

	/* Start policy engine */
	cert_init(1);
	pe_init();
	pe_playground_init();

	DEBUG(DBG_TRACE, "policy event loop");
	if (event_dispatch() == -1)
		fatal("policy_main: event_dispatch");

	/* Shutdown policy engine */
	pe_shutdown();
	_exit(0);
}

static void
dispatch_timer(int sig __used, short event __used, void *arg)
{
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;
	struct reply_wait *msg_wait;
	anoubisd_msg_t *msg;
	eventdev_token *tk;
	/*@dependent@*/
	Qentry *qep_cur;
	/*@dependent@*/
	Qentry *qep_next;
	struct eventdev_reply *rep;
	time_t now = time(NULL);

	DEBUG(DBG_TRACE, ">dispatch_timer");

	for(qep_cur = queue_head(&replyq); qep_cur; qep_cur = qep_next) {
		qep_next = queue_walk(&replyq, qep_cur);

		msg_wait = qep_cur->entry;
		if (now > (msg_wait->starttime + msg_wait->timeout)
		    || terminate >= 3) {
			msg = msg_factory(ANOUBISD_MSG_EVENTREPLY,
			    sizeof(struct eventdev_reply));
			if (!msg) {
				master_terminate(ENOMEM);
				return;
			}
			rep = (struct eventdev_reply *)msg->msg;
			rep->msg_token = msg_wait->token;
			rep->reply = EPERM;
			enqueue(&eventq_p2m, msg);
			DEBUG(DBG_QUEUE, " >eventq_p2m: %x", rep->msg_token);

			msg = msg_factory(ANOUBISD_MSG_EVENTCANCEL,
			    sizeof(eventdev_token));
			if (!msg) {
				master_terminate(ENOMEM);
				return;
			}
			tk = (eventdev_token *)msg->msg;
			*tk = msg_wait->token;
			enqueue(&eventq_p2s, msg);
			DEBUG(DBG_QUEUE, " >eventq_p2s: %x", msg_wait->token);

			DEBUG(DBG_QUEUE, " <replyq: %x", msg_wait->token);
			queue_delete(&replyq, msg_wait);
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
	}
	if (terminate < 3)
		event_add(ev_info->ev_timer, ev_info->tv);

	DEBUG(DBG_TRACE, "<dispatch_timer");
}

/* Does not free the message */
static void
dispatch_sfscache_invalidate(anoubisd_msg_t *msg)
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

void
send_upgrade_start(void)
{
	anoubisd_msg_t			*msg;
	struct anoubisd_msg_upgrade	*upg;

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

static void
send_upgrade_chunk(void)
{
	anoubisd_msg_t			*msg;
	struct anoubisd_msg_upgrade	*upg;
	char				buf[PATH_MAX];
	int				buf_size;

	DEBUG(DBG_UPGRADE, ">send_upgrade_chunk");

	/*
	 * Create the next chunk.
	 *
	 * buf_size can be 0! It means that no more files are available.
	 *
	 * The buffer has a size of PATH_MAX to make sure, that at least
	 * one big pathname can be transfered.
	 */
	buf_size = policy_upgrade_fill_chunk(buf, sizeof(buf));

	msg = msg_factory(ANOUBISD_MSG_UPGRADE,
	    sizeof(struct anoubisd_msg_upgrade) + buf_size);
	if (msg == NULL) {
		log_warnx("send_upgrade_chunk: Out of memory");
		return;
	}

	upg = (struct anoubisd_msg_upgrade *)msg->msg;
	upg->upgradetype = ANOUBISD_UPGRADE_CHUNK;
	upg->chunksize = buf_size;
	memcpy(upg->chunk, buf, buf_size);

	DEBUG(DBG_UPGRADE, " send_upgrade_chunk: "
	    "enqueue ANOUBISD_UPGRADE_CHUNK, chunksize = %i",
	    upg->chunksize);
	enqueue(&eventq_p2m, msg);

	DEBUG(DBG_UPGRADE, "<send_upgrade_chunk");
}

/* Does not free the message */
static void
dispatch_upgrade(anoubisd_msg_t *msg)
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
			anoubisd_msg_t	*msg = dequeue(&eventq_p2m_hold);
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

/*
 * Helper function. This copies @datalen bytes from @data to the
 * offset pointed to by @offp in @buf. The value pointed to by @offp
 * is updated accordingly.
 * Additionally the offset from @offp and the datalenth @datalen are stored
 * in @roffp and @rlenp.
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
 * The same as do_copy but the data and data length are taken from a
 * abuf_buffer and not from a pointer.
 */
static void
do_copy_buf(char *buf, int *offp, const struct abuf_buffer data,
    uint16_t *roffp, uint16_t *rlenp)
{
	do_copy(buf, offp, abuf_toptr(data, 0, abuf_length(data)),
	    abuf_length(data), roffp, rlenp);
}

/*
 * Copy a proc ident by calling @do_copy twice. Once for the csum and
 * once for the pathhint. The offset pointed to by @offp is updated.
 * Additionally the target offsets and lengths are of the copies are
 * stored in @rcsoffp/@rcslenp and @rpathoffp/@rpathlenp respectively.
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

static void
dispatch_m2p(int fd, short sig __used, void *arg)
{
	struct reply_wait *msg_wait;
	anoubisd_msg_t *msg, *msg_reply;
	anoubisd_reply_t *reply;
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;
	/*@dependent@*/
	struct eventdev_hdr *hdr;
	struct eventdev_reply *rep;

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
				log_warn("policy: reconfigure failed");
			}
			free(msg);
			continue;
		default:
			DEBUG(DBG_TRACE, "<dispatch_m2p (bad type %d)",
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
		    hdr->msg_source != ANOUBIS_SOURCE_PLAYGROUNDPROC)) {
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
			anoubisd_msg_t *nmsg;
			anoubisd_msg_eventask_t *eventask;
			int extra, off;

			extra = hdr->msg_size;
			extra += pident_size(reply->pident);
			extra += pident_size(reply->ctxident);
			nmsg = msg_factory(ANOUBISD_MSG_EVENTASK,
			    sizeof(anoubisd_msg_eventask_t) + extra);
			if (!nmsg) {
				free(msg);
				free(reply);
				master_terminate(ENOMEM);
				return;
			}
			eventask = (anoubisd_msg_eventask_t *)nmsg->msg;
			eventask->rule_id = reply->rule_id;
			eventask->prio = reply->prio;
			eventask->sfsmatch = reply->sfsmatch;
			off = 0;
			do_copy(eventask->payload, &off, hdr, hdr->msg_size,
			    &eventask->evoff, &eventask->evlen);
			do_copy_ident(eventask->payload, &off, reply->pident,
			    &eventask->csumoff, &eventask->csumlen,
			    &eventask->pathoff, &eventask->pathlen);
			do_copy_ident(eventask->payload, &off, reply->ctxident,
			    &eventask->ctxcsumoff, &eventask->ctxcsumlen,
			    &eventask->ctxpathoff, &eventask->ctxpathlen);
			hdr = (struct eventdev_hdr *)eventask->payload;

			free(msg);

			msg = nmsg;
			msg_wait = abuf_alloc_type(struct reply_wait);
			if (msg_wait == NULL) {
				log_warn("dispatch_m2p: can't allocate memory");
				free(reply);
				master_terminate(ENOMEM);
				continue;
			}

			msg_wait->token = hdr->msg_token;
			if (time(&msg_wait->starttime) == -1) {
				free(msg);
				free(reply);
				log_warn("dispatch_m2p: failed to get time");
				master_terminate(EIO);
				continue;
			}
			msg_wait->flags = ANOUBIS_RET_FLAGS(reply->reply);
			msg_wait->timeout = reply->timeout;
			msg_wait->log = reply->log;

			enqueue(&replyq, msg_wait);
			DEBUG(DBG_QUEUE, " >replyq: %x", msg_wait->token);

			/* send msg to the session */
			enqueue(&eventq_p2s, msg);
			DEBUG(DBG_QUEUE, " >eventq_p2s: %x", hdr->msg_token);
		} else {
			int	hold = reply->hold;
			msg_reply = msg_factory(ANOUBISD_MSG_EVENTREPLY,
			    sizeof(struct eventdev_reply));
			if (!msg_reply) {
				free(msg);
				free(reply);
				master_terminate(ENOMEM);
				return;
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
		set_terminate(2, ev_info);
		event_del(ev_info->ev_m2p);
		event_add(eventq_p2s.event, NULL);
	}
	DEBUG(DBG_TRACE, "<dispatch_m2p (no msg)");
}

static void
dispatch_p2m(int fd, short sig __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_p2m");
	dispatch_write_queue(&eventq_p2m, fd);
	DEBUG(DBG_TRACE, "<dispatch_p2m");
}

int
token_cmp(void *msg1, void *msg2)
{
	if (msg1 == NULL || msg2 == NULL) {
		DEBUG(DBG_TRACE, "token_cmp: null msg pointer");
		return 0;
	}
	if (((struct reply_wait *)msg1)->token ==
	    ((struct reply_wait *)msg2)->token)
		return 1;
	return 0;
}

void
send_lognotify(struct eventdev_hdr *hdr, u_int32_t error, u_int32_t loglevel,
    u_int32_t rule_id, u_int32_t prio, u_int32_t sfsmatch)
{
	int size;
	anoubisd_msg_t * msg;
	struct anoubisd_msg_logrequest * req;

	size = sizeof(anoubisd_msg_t)
	    + sizeof(struct anoubisd_msg_logrequest)
	    + hdr->msg_size
	    - sizeof(struct eventdev_hdr);
	msg = malloc(size);
	if (!msg) {
		log_warn("send_lognotify: can't allocate memory");
		return;
	}
	msg->size = size;
	msg->mtype = ANOUBISD_MSG_LOGREQUEST;
	req = (struct anoubisd_msg_logrequest *)msg->msg;
	req->error = error;
	req->loglevel = loglevel;
	req->rule_id = rule_id;
	req->prio = prio;
	req->sfsmatch = sfsmatch;
	bcopy(hdr, &req->hdr, hdr->msg_size);
	enqueue(&eventq_p2s, msg);
}

void
send_policychange(u_int32_t uid, u_int32_t prio)
{
	anoubisd_msg_t			*msg;
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

int send_policy_data(u_int64_t token, int fd)
{
	anoubisd_msg_t			*msg;
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
	master_terminate(ENOMEM);
	return -ENOMEM;
}

static void
dispatch_s2p(int fd, short sig __used, void *arg)
{
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;
	struct reply_wait rep_tmp, *rep_wait;
	struct eventdev_reply * evrep;
	anoubisd_msg_t *msg, *msg_rep;

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
			free(msg);
			break;

		case ANOUBISD_MSG_EVENTREPLY:

			evrep = (struct eventdev_reply *)msg->msg;
			rep_tmp.token = evrep->msg_token;

			if ((rep_wait = queue_find(&replyq, &rep_tmp,
			    token_cmp)) != 0) {
				/*
				 * Only send message if still in queue. It
				 * might have already been replied to by a
				 * timeout or other GUI
				 */
				queue_delete(&replyq, rep_wait);
				DEBUG(DBG_QUEUE, " <replyq: %x",
				    rep_wait->token);
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
				enqueue(&eventq_p2m, msg);
				DEBUG(DBG_QUEUE, " >eventq_p2m: %x",
					evrep->msg_token);
			} else {
				free(msg);
			}
			break;

		default:

			DEBUG(DBG_TRACE, "dispatch_s2p: msg type %d",
			    msg->mtype);
			free(msg);
			break;
		}
	}
	if (msg_eof(fd)) {
		event_del(ev_info->ev_s2p);
		set_terminate(3, ev_info);
	}
	DEBUG(DBG_TRACE, "<dispatch_s2p");
}

static void
dispatch_p2s(int fd, short sig __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_p2s");
	dispatch_write_queue(&eventq_p2s, fd);
	if (terminate >= 2 && !queue_peek(&eventq_p2s) && !msg_pending(fd))
		shutdown(fd, SHUT_WR);
	DEBUG(DBG_TRACE, "<dispatch_p2s");
}

/*
 * Caller must call pe_upgrade_filelist_start before calling this
 * function for the first time. Fills as many paths as possible
 * from the upgrade filelist into buf. A path that does not fit into
 * the buffer is not copied at all and will be reconsidered with the
 * next chunk.
 *
 * Returns the number of bytes actually copied.
 */
static int
policy_upgrade_fill_chunk(char *buf, int maxlen)
{
	int	reallen = 0;

	while (1) {
		struct pe_file_node	*node = pe_upgrade_filelist_get();
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
