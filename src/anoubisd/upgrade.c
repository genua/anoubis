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
#include <sys/stat.h>
#include <pwd.h>
#include <signal.h>
#include <event.h>
#include <openssl/sha.h>
#ifdef LINUX
#include <grp.h>
#include <bsdcompat.h>
#endif
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <paths.h>
#include <time.h>
#include <unistd.h>

/* on glibc 2.6+, event.h uses non C89 types :/ */
#include <event.h>

#include "anoubisd.h"
#include "amsg.h"
#include "aqueue.h"

static void	dispatch_m2u(int, short, void *);
static void	dispatch_u2m(int, short, void *);

/**
 * The event queue for outgoing events to the master process.
 */
static Queue		 eventq_u2m;

/**
 * The event for incoming events from the master.
 */
struct event		 ev_m2u;

/**
 * Events for signals. This are used in the singal handler to
 * remove signal events from the event queue during shutdown.
 * This is neccessary to make sure that the event loop terminates.
 */
struct event	*sigs[10];


/**
 * The signal handler for QUIT, TERM and INT signals. INT and TERM
 * cause a gracefule shutdown, QUIT exists the event loop immediately.
 * This functions is sometimes called manually to simulate a signal.
 * This happens if end of file is detected on the incoming file descriptor.
 *
 * @note: This function is never called from signal context. It is and
 *     event handler for signal events that is called from the main
 *     event loop.
 *
 * @param sig The signal.
 * @param event The event type (unused).
 * @param arg The callback argument (unused).
 */
static void
upgrade_sighandler(int sig, short event __used, void *arg __used)
{
	int				 i;
	sigset_t			 mask;

	switch (sig) {
	case SIGINT:
	case SIGTERM:
		sigfillset(&mask);
		sigprocmask(SIG_SETMASK, &mask, NULL);
		for (i=0; sigs[i]; ++i)
			signal_del(sigs[i]);
		break;
	case SIGQUIT:
		(void)event_loopexit(NULL);
		/* FALLTRHOUGH */
	}
}

/**
 * This is the main entry point for the upgrade process. This function
 * is called once by the master. It spawns the upgrade child process and
 * has no other effect on the master process. The upgrade process only
 * communicates with the master and runs with root privileges.
 *
 * @param pipes The file descriptors for the pipes that are used
 *     by the daemon processes to communicate with each other. The
 *     child process only uses one of those file descriptors to setup
 *     the communication with the master. All other descriptors are
 *     closed in the child process.
 * @param loggers The pipes that are used by the anoubis daemon processes
 *     to communicate with the logger process. Only one file descriptor is
 *     used all other descriptors are closed in the child process.
 * @return Returns the process ID of the upgrade child in the parent process.
 *     In the child process this function never returns.
 */
pid_t
upgrade_main(int pipes[], int loggers[])
{
	pid_t				pid;
	int				masterfd, logfd;
	sigset_t			mask;
	struct event			ev_u2m;
	struct event			ev_sigterm, ev_sigint, ev_sigquit;

	pid = fork();
	if (pid == -1)
		fatal("fork");
		/*NOTREACHED*/
	if (pid)
		return (pid);

	(void)event_init();

	anoubisd_process = PROC_UPGRADE;

	masterfd = logfd = -1;
	SWAP(masterfd, pipes[PIPE_MAIN_UPGRADE + 1]);
	SWAP(logfd, loggers[anoubisd_process]);
	cleanup_fds(pipes, loggers);

	log_init(logfd);

	log_info("upgrade started (pid %d)", getpid());

#ifdef LINUX
	dazukofs_ignore();
#endif

	signal_set(&ev_sigterm, SIGTERM, upgrade_sighandler, NULL);
	signal_set(&ev_sigint,  SIGINT,  upgrade_sighandler, NULL);
	signal_set(&ev_sigquit, SIGQUIT, upgrade_sighandler, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint,  NULL);
	signal_add(&ev_sigquit, NULL);
	sigs[0] = &ev_sigterm;
	sigs[1] = &ev_sigint;
	sigs[2] = &ev_sigquit;
	sigs[3] = NULL;

	anoubisd_defaultsigset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	msg_init(masterfd);

	/* master process */
	event_set(&ev_m2u, masterfd, EV_READ | EV_PERSIST, dispatch_m2u, NULL);
	event_add(&ev_m2u, NULL);

	event_set(&ev_u2m, masterfd, EV_WRITE, dispatch_u2m, NULL);
	queue_init(&eventq_u2m, &ev_u2m);

	setproctitle("upgrade");

	if (event_dispatch() == -1) {
		fatal("upgrade_main: event_dispatch");
	}

	_exit(0);
}

/**
 * Send an upgrade message without payload to the master process.
 * This function composes a new message and sends it to the master.
 * The type of the message is ANOUBISD_MSG_UPGRADE.
 *
 * @param type The upgrade subtype of the upgrade message. Can be
 *     ANOUBISD_UPGRADE_CHUNK_REQ or ANOUBISD_UPGRADE_END.
 */
static void
send_upgrade_message(int type)
{
	struct anoubisd_msg		*msg;
	struct anoubisd_msg_upgrade	*umsg;

	msg = msg_factory(ANOUBISD_MSG_UPGRADE, sizeof(*umsg));
	if (msg == NULL) {
		log_warnx("send_upgrade_message: Out of memory");
		master_terminate();
	}
	umsg = (struct anoubisd_msg_upgrade *)msg->msg;
	umsg->chunksize = 0;
	umsg->upgradetype = type;
	DEBUG(DBG_QUEUE, " Upgrade message: type %d", type);
	enqueue(&eventq_u2m, msg);
}

/**
 * Calculate the new checksum of an upgraded file and send it to the
 * master. The master will update checksums in the SFS tree with the
 * new value.
 *
 * @param path The path of the file. Only regular files and symlinks
 *     are handled. All other files are skipped.
 */
static void
send_checksum(const char *path)
{
	static int			 devanoubis = -2;
	struct anoubisd_msg		*msg;
	struct anoubisd_sfs_update_all	*umsg;
	struct anoubis_ioctl_csum	 cs;
	int				 fd, len, plen;
	struct stat			 statbuf;

	if (devanoubis == -2) {
		devanoubis = open(_PATH_DEV "anoubis", O_RDONLY);
		if (devanoubis < 0) {
			log_warn("upgrade: Failed to open "_PATH_DEV"anoubis");
			return;
		}
	}
	if (devanoubis < 0)
		return;
	if (lstat(path, &statbuf) < 0) {
		log_warnx("upgrade: Failed to stat file %s", path);
		return;
	}
	if (S_ISLNK(statbuf.st_mode)) {
		char		*buf = malloc(PATH_MAX);
		int		 len;
		SHA256_CTX	 shaCtx;

		if (!buf) {
			log_warnx("upgrade: Out of memory");
			return;
		}
		len = readlink(path, buf, PATH_MAX);
		if (len < 0) {
			free(buf);
			log_warn("upgrade: Readlink failed");
			return;
		}
		SHA256_Init(&shaCtx);
		SHA256_Update(&shaCtx, buf, len);
		SHA256_Final(cs.csum, &shaCtx);
		free(buf);
	} else if (S_ISREG(statbuf.st_mode)) {
		fd = open(path, O_RDONLY);
		if (fd < 0) {
			if (errno != ENOENT)
				log_warn("upgrade: Failed to open file %s",
				    path);
			return;
		}
		cs.fd = fd;
		if (ioctl(devanoubis, ANOUBIS_GETCSUM, &cs) < 0) {
			log_warn("upgrade: Kernel failed to supply checksum "
			    "for %s", path);
		}
		close(fd);
	} else {
		log_warnx("upgrade: Upgraded file %s is not a regular file",
		    path);
		return;
	}
	plen = strlen(path) + 1;
	len = sizeof(struct anoubisd_sfs_update_all) + ANOUBIS_CS_LEN + plen;
	msg = msg_factory(ANOUBISD_MSG_SFS_UPDATE_ALL, len);
	if (msg == NULL) {
		log_warnx("upgrade: Out of memory in send_checksum");
		return;
	}
	umsg = (struct anoubisd_sfs_update_all *)msg->msg;
	umsg->cslen = ANOUBIS_CS_LEN;
	// cppcheck-suppress bufferAccessOutOfBounds
	memcpy(umsg->payload, &cs.csum, ANOUBIS_CS_LEN);
	memcpy(umsg->payload + ANOUBIS_CS_LEN, path, plen);
	DEBUG(DBG_QUEUE, " Checksum upgrade message for %s", path);
	enqueue(&eventq_u2m, msg);
}

/**
 * Parse and process an upgrade message received from the master.
 * Accepted messages are of type ANOUBISD_UPGRADE_START or
 * ANOUBISD_UPGRADE_CHUNK. In the latter case all files in the chunk
 * are checksumed and the checksums are sent to the master. Additionally,
 * this function requests a new chunk until an empty chunk is received,
 * i.e. all upgraded files have been processed.
 *
 * @param msg The upgrade message.
 */
static void
dispatch_upgrade(struct anoubisd_msg *msg)
{
	struct anoubisd_msg_upgrade	*umsg;
	size_t				 pos;

	if (msg->size < (int)(sizeof(*msg) + sizeof(*umsg))) {
		log_warnx("dispatch_upgrade: short message");
		return;
	}
	umsg = (struct anoubisd_msg_upgrade *)msg->msg;
	if (msg->size < (int)(sizeof(*msg) + sizeof(*umsg) + umsg->chunksize)) {
		log_warnx("dispatch_upgrade: short message");
		return;
	}
	switch(umsg->upgradetype) {
	case ANOUBISD_UPGRADE_START:
		send_upgrade_message(ANOUBISD_UPGRADE_CHUNK_REQ);
		break;
	case ANOUBISD_UPGRADE_CHUNK:
		if (umsg->chunksize == 0) {
			send_upgrade_message(ANOUBISD_UPGRADE_END);
			break;
		}
		/* Force NUL-termination of strings in umsg->chunk */
		umsg->chunk[umsg->chunksize-1] = 0;
		pos = 0;
		while (pos < umsg->chunksize) {
			send_checksum(umsg->chunk + pos);
			pos += strlen(umsg->chunk + pos) + 1;
		}
		send_upgrade_message(ANOUBISD_UPGRADE_CHUNK_REQ);
		break;
	default:
		log_warnx("dispatch_upgrade: Bad upgrade message type %d",
		    umsg->upgradetype);
	}
}

/**
 * This is the event handler for read events on the pipe from the master.
 * Only messages of type ANOUBISD_MSG_UPGRADE are acceptable. If EOF is
 * received on this file descriptor the upgrade process initiates a
 * gracefule shutdown.
 *
 * @param fd The file descriptor of the event.
 * @param sig The type of the event (unused).
 * @param arg The callback argument (unused).
 */
static void
dispatch_m2u(int fd, short sig __used, void *arg __used)
{
	struct anoubisd_msg		*msg;

	DEBUG(DBG_TRACE, ">dispatch_m2u");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL)
			break;
		if (msg->size < (int)sizeof(struct anoubisd_msg)) {
			log_warnx(" dispatch_m2u: short message");
			free(msg);
			continue;
		}
		DEBUG(DBG_QUEUE, " m2u: type %d", msg->mtype);
		switch(msg->mtype) {
		case ANOUBISD_MSG_UPGRADE:
			dispatch_upgrade(msg);
			break;
		default:
			log_warnx(" dispatch_m2u: Unknown message type");
		}
		free(msg);
	}
	if (msg_eof(fd)) {
		upgrade_sighandler(SIGTERM, 0, NULL);
		event_del(&ev_m2u);
	}

	DEBUG(DBG_TRACE, "<dispatch_m2u");
}

/**
 * This is the event handler for write events on the pipe to the master.
 *
 * @param fd The file descriptor of the event.
 * @param sig The type of the event (unused).
 * @param arg The callback argument of the event (unused).
 */
static void
dispatch_u2m(int fd, short sig __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_u2m");
	dispatch_write_queue(&eventq_u2m, fd);
	DEBUG(DBG_TRACE, "<dispatch_u2m");
}
