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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <signal.h>
#include <event.h>
#ifdef LINUX
#include <grp.h>
#include <bsdcompat.h>
#endif
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* on glibc 2.6+, event.h uses non C89 types :/ */
#include <event.h>

#include "anoubisd.h"
#include "amsg.h"
#include "aqueue.h"

static void	dispatch_m2u(int, short, void *);
static void	dispatch_u2m(int, short, void *);

static int	terminate = 0;
static Queue    eventq_u2m;

struct event_info_upgrade {
	struct event	*ev_u2m, *ev_m2u;
	struct event	*ev_sigs[10];
};


static void
upgrade_sighandler(int sig, short event __used, void *arg)
{
	int		  i;
	sigset_t	  mask;
	struct event	**sigs;

	switch (sig) {
	case SIGINT:
	case SIGTERM:
		terminate = 1;
		sigfillset(&mask);
		sigprocmask(SIG_SETMASK, &mask, NULL);
		sigs = arg;
		for (i=0; sigs[i]; ++i) {
			signal_del(sigs[i]);
		}
		break;
	case SIGQUIT:
		(void)event_loopexit(NULL);
		/* FALLTRHOUGH */
	}
}

pid_t
upgrade_main(struct anoubisd_config *conf __used, int pipe_m2u[2],
    int pipe_m2p[2], int pipe_s2p[2], int pipe_m2s[2], int loggers[4])
{
	pid_t		pid;
	sigset_t	mask;
	struct event	ev_m2u, ev_u2m;
	struct event	ev_sigterm, ev_sigint, ev_sigquit;
	static struct event	*sigs[10];
	struct event_info_upgrade ev_info;

	pid = fork();
	if (pid < 0) {
		fatal("fork");
		/*NOTREACHED*/
	}
	if (pid) {
		return (pid);
	}
	anoubisd_process = PROC_UPGRADE;

	(void)event_init();

	log_init(loggers[3]);
	close(loggers[0]);
	close(loggers[1]);
	close(loggers[2]);

	close(pipe_m2u[0]);

	setproctitle("upgrade");
	log_info("upgrade started (pid %d)", getpid());

	signal_set(&ev_sigterm, SIGTERM, upgrade_sighandler, sigs);
	signal_set(&ev_sigint,  SIGINT,  upgrade_sighandler, sigs);
	signal_set(&ev_sigquit, SIGQUIT, upgrade_sighandler, sigs);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint,  NULL);
	signal_add(&ev_sigquit, NULL);
	sigs[0] = &ev_sigterm;
	sigs[1] = &ev_sigint;
	sigs[2] = &ev_sigquit;
	sigs[3] = NULL;

	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGQUIT);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	close(pipe_m2p[0]);
	close(pipe_m2p[1]);
	close(pipe_s2p[0]);
	close(pipe_s2p[1]);
	close(pipe_m2s[0]);
	close(pipe_m2s[1]);
	close(pipe_m2u[0]);

	queue_init(eventq_u2m);
	msg_init(pipe_m2u[1], "m2u");

	/* master process */
	event_set(&ev_m2u, pipe_m2u[1], EV_READ | EV_PERSIST, dispatch_m2u,
	    &ev_info);
	event_add(&ev_m2u, NULL);

	event_set(&ev_u2m, pipe_m2u[1], EV_WRITE, dispatch_u2m,
	    &ev_info);

	ev_info.ev_u2m = &ev_u2m;
	ev_info.ev_m2u = &ev_m2u;

	if (event_dispatch() == -1) {
		fatal("upgrade_main: event_dispatch");
	}

	_exit(0);
}

static void
send_upgrade_message(int type, struct event_info_upgrade *ev_info)
{
	anoubisd_msg_t		*msg;
	anoubisd_msg_upgrade_t	*umsg;

	msg = msg_factory(ANOUBISD_MSG_UPGRADE, sizeof(*umsg));
	if (msg == NULL) {
		log_warnx("send_upgrade_message: Out of memory");
		master_terminate(ENOMEM);
		return;
	}
	umsg = (anoubisd_msg_upgrade_t *)msg->msg;
	umsg->chunksize = 0;
	umsg->upgradetype = type;
	DEBUG(DBG_QUEUE, " Upgrade message: type %d", type);
	enqueue(&eventq_u2m, msg);
	event_add(ev_info->ev_u2m, NULL);
}

static void
send_checksum(const char *path, struct event_info_upgrade *ev_info)
{
	static int			 devanoubis = -2;
	anoubisd_msg_t			*msg;
	anoubisd_sfs_update_all_t	*umsg;
	struct anoubis_ioctl_csum	 cs;
	int				 fd, len, plen;

	if (devanoubis == -2) {
		devanoubis = open("/dev/anoubis", O_RDONLY);
		if (devanoubis < 0) {
			log_warn("upgrade: Failed to open /dev/anoubis");
			return;
		}
	}
	if (devanoubis < 0)
		return;
	fd = open(path, O_RDONLY);
	if (fd < 0) {
		if (errno != ENOENT)
			log_warn("upgrade: Failed to open file %s", path);
		return;
	}
	cs.fd = fd;
	if (ioctl(devanoubis, ANOUBIS_GETCSUM, &cs) < 0) {
		log_warn("upgrade: Kernel failed to supply checksum for %s",
		    path);
	}
	close(fd);
	plen = strlen(path) + 1;
	len = sizeof(anoubisd_sfs_update_all_t) + ANOUBIS_CS_LEN + plen;
	msg = msg_factory(ANOUBISD_MSG_SFS_UPDATE_ALL, len);
	if (msg == NULL) {
		log_warnx("upgrade: Out of memory in send_checksum");
		return;
	}
	umsg = (anoubisd_sfs_update_all_t *)msg->msg;
	umsg->cslen = ANOUBIS_CS_LEN;
	memcpy(umsg->payload, &cs.csum, ANOUBIS_CS_LEN);
	memcpy(umsg->payload + ANOUBIS_CS_LEN, path, plen);
	DEBUG(DBG_QUEUE, " Checksum upgrade message for %s", path);
	enqueue(&eventq_u2m, msg);
	event_add(ev_info->ev_u2m, NULL);
}

static void
dispatch_upgrade(anoubisd_msg_t *msg, struct event_info_upgrade *ev_info)
{
	anoubisd_msg_upgrade_t		*umsg;
	size_t				 pos;

	if (msg->size < (int)(sizeof(*msg) + sizeof(*umsg))) {
		log_warnx("dispatch_upgrade: short message");
		return;
	}
	umsg = (anoubisd_msg_upgrade_t *)msg->msg;
	if (msg->size < (int)(sizeof(*msg) + sizeof(*umsg) + umsg->chunksize)) {
		log_warnx("dispatch_upgrade: short message");
		return;
	}
	/* Force NUL-termination of strings in umsg->chunk */
	umsg->chunk[umsg->chunksize-1] = 0;
	switch(umsg->upgradetype) {
	case ANOUBISD_UPGRADE_START:
		send_upgrade_message(ANOUBISD_UPGRADE_CHUNK_REQ, ev_info);
		break;
	case ANOUBISD_UPGRADE_CHUNK:
		if (umsg->chunksize == 0) {
			send_upgrade_message(ANOUBISD_UPGRADE_END, ev_info);
			break;
		}
		pos = 0;
		while (pos < umsg->chunksize) {
			send_checksum(umsg->chunk + pos, ev_info);
			pos += strlen(umsg->chunk + pos) + 1;
		}
		send_upgrade_message(ANOUBISD_UPGRADE_CHUNK_REQ, ev_info);
		break;
	default:
		log_warnx("dispatch_upgrade: Bad upgrade message type %d",
		    umsg->upgradetype);
	}
}

static void
dispatch_m2u(int fd, short sig __used, void *arg)
{
	struct event_info_upgrade	*ev_info = arg;
	anoubisd_msg_t			*msg;

	DEBUG(DBG_TRACE, ">dispatch_m2u");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL)
			break;
		if (msg->size < (int)sizeof(anoubisd_msg_t)) {
			log_warnx(" dispatch_m2u: short message");
			free(msg);
			continue;
		}
		DEBUG(DBG_QUEUE, " m2u: type %d", msg->mtype);
		switch(msg->mtype) {
		case ANOUBISD_MSG_UPGRADE:
			dispatch_upgrade(msg, ev_info);
			break;
		default:
			log_warnx(" dispatch_m2u: Unknown message type");
		}
		free(msg);
	}
	if (msg_eof(fd)) {
		if (terminate < 2)
			terminate = 2;
		event_del(ev_info->ev_m2u);
		event_add(ev_info->ev_u2m, NULL);
	}

	DEBUG(DBG_TRACE, "<dispatch_m2u");
}

static void
dispatch_u2m(int fd, short sig __used, void *arg)
{
	struct event_info_upgrade	*ev_info = arg;
	anoubisd_msg_t			*msg;
	int				 ret;

	DEBUG(DBG_TRACE, ">dispatch_u2m");

	msg = queue_peek(&eventq_u2m);
	ret = send_msg(fd, msg);

	if (msg && ret != 0) {
		msg = dequeue(&eventq_u2m);
		if (ret < 0)
			DEBUG(DBG_QUEUE, " dispatch_u2m: Dropping message "
			    "(error %d)", ret);
		free(msg);
	}

	/* If the queue is not empty, we want to be called again */
	if (queue_peek(&eventq_u2m) || msg_pending(fd))
		event_add(ev_info->ev_u2m, NULL);

	DEBUG(DBG_TRACE, "<dispatch_u2m");
}
