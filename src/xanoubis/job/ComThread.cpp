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

#include <config.h>

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include <sys/un.h>
#include <signal.h>

#include <poll.h>

#include <anoubis_chat.h>
#include <anoubis_protocol.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>
#include <anoubis_dump.h>
#include <anoubis_auth.h>

#include "AnEvents.h"
#include "ComTask.h"
#include "ComThread.h"
#include "main.h"
#include "NotifyAnswer.h"
#include "NotificationCtrl.h"
#include "JobCtrl.h"
#include "ProcCtrl.h"

#define X_AUTH_SUCCESS		0x0000
#define X_AUTH_NOKEY		0x8001

extern "C" int
x_auth_callback(struct anoubis_client *WXUNUSED(client),
    const struct anoubis_msg *in, struct anoubis_msg **outp)
{
	KeyCtrl *keyCtrl = KeyCtrl::instance();

	if (!keyCtrl->canUseLocalKeys()) {
		/* Key-pair not configured */
		return (-X_AUTH_NOKEY);
	}

	if (keyCtrl->loadPrivateKey() != KeyCtrl::RESULT_KEY_OK) {
		/* Failed to load the key */
		return (-X_AUTH_NOKEY);
	}

	PrivKey &privKey = keyCtrl->getPrivateKey();
	LocalCertificate &cert = keyCtrl->getLocalCertificate();

	if (!cert.isLoaded() && !cert.load()) {
		/* Failed to load certificate */
		return (-X_AUTH_NOKEY);
	}

	return (anoubis_auth_callback(privKey.getKey(), cert.getCertificate(),
	    in, outp, 0));

}

ComThread::ComThread(JobCtrl *jobCtrl, const wxString &socketPath)
    : JobThread(jobCtrl)
{
	int	flags;
	this->socketPath_ = socketPath;
	this->channel_ = 0;
	this->client_ = 0;
	answerQueue_ = new SynchronizedQueue<Notification>(false);
	if (pipe(comPipe_) < 0) {
		stop();
		return;
	}
	for(int i=0; i<2; ++i) {
		flags = fcntl(comPipe_[0], F_GETFL, 0);
		flags |= O_NONBLOCK;
		fcntl(comPipe_[0], F_SETFL, flags);
	}
}

ComThread::~ComThread()
{
	close(comPipe_[0]);
	close(comPipe_[1]);
	delete answerQueue_;
}

ComThread::ConnectResult
ComThread::connect(void)
{
	struct sockaddr_storage	 ss;
	struct sockaddr_un	*ss_sun = (struct sockaddr_un *)&ss;
	achat_rc		 rc = ACHAT_RC_ERROR;
	int			 result;

	struct sigaction        act;

	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &act, NULL);

	/*
	 * *** channel ***
	 */

	channel_ = acc_create();
	if (channel_ == 0)
		return (Failure);

	rc = acc_settail(channel_, ACC_TAIL_CLIENT);
	if (rc != ACHAT_RC_OK) {
		disconnect();
		return (Failure);
	}

	rc = acc_setsslmode(channel_, ACC_SSLMODE_CLEAR);
	if (rc != ACHAT_RC_OK) {
		disconnect();
		return (Failure);
	}

	bzero(&ss, sizeof(ss));
	ss_sun->sun_family = AF_UNIX;
	strlcpy(ss_sun->sun_path, socketPath_.fn_str(),
	    sizeof(ss_sun->sun_path));
	rc = acc_setaddr(channel_, &ss, sizeof(struct sockaddr_un));
	if (rc != ACHAT_RC_OK) {
		disconnect();
		return (Failure);
	}

	rc = acc_prepare(channel_);
	if (rc != ACHAT_RC_OK) {
		disconnect();
		return (Failure);
	}

	rc = acc_open(channel_);
	if (rc != ACHAT_RC_OK) {
		disconnect();
		return (Failure);
	}

	/*
	 * *** client ***
	 */

	client_ = anoubis_client_create(channel_, ANOUBIS_AUTH_TRANSPORTANDKEY,
	    &x_auth_callback);
	if (client_ == 0) {
		disconnect();
		return (Failure);
	}

	result = anoubis_client_connect(client_, ANOUBIS_PROTO_BOTH);

	/* Pass protocol version back to the JobCtrl */
	JobCtrl::instance()->protocolVersion_ =
	    anoubis_client_serverversion(client_);

	/* Convert result to a enum */
	switch (-result) {
	/* In case everything is allright */
	case X_AUTH_SUCCESS:
		return Success;
		/* NOTREACHED */
	/* In case of missmatching Version */
	case EPROTONOSUPPORT:
		disconnect();
		return VersionMismatch;
		/* NOTREACHED */
	/* In case of error while authentication*/
	case X_AUTH_NOKEY:
		disconnect();
		return AuthNoKey;
		/* NOTREACHED */
	case ANOUBIS_AUTHERR_KEY_MISMATCH:
		disconnect();
		return AuthWrongKeyId;
		/* NOTREACHED */
	case ANOUBIS_AUTHERR_KEY:
		disconnect();
		return AuthInvalidKey;
		/* NOTREACHED */
	case ANOUBIS_AUTHERR_CERT:
		disconnect();
		return AuthInvalidCert;
		/* NOTREACHED */
	/* Internal System Error */
	case ANOUBIS_AUTHERR_PKG:
	case ANOUBIS_AUTHERR_INVAL:
	case ANOUBIS_AUTHERR_RAND:
	case ANOUBIS_AUTHERR_NOMEM:
	case ANOUBIS_AUTHERR_SIGN:
	default:
		disconnect();
		return AuthSysFail;
		/* NOTREACHED */
	}

	/* Should not happen since we handel everythin before */
	return (Failure);
}

void
ComThread::disconnect(void)
{
	if (client_ != 0) {
		anoubis_client_close(client_);
		anoubis_client_destroy(client_);
		client_ = 0;
	}

	if (channel_ != 0) {
		acc_destroy(channel_);
		channel_ = 0;
	}
}

bool
ComThread::isConnected(void) const
{
	return (this->client_ != 0);
}

void *
ComThread::Entry(void)
{
	struct pollfd	 fds[2];
	ComTask		*current = NULL;
	Notification	*notify;
	ConnectResult	 connectResult;

	connectResult = connect();

	switch (connectResult) {
	case Success:
		break;
	case Failure:
		sendComEvent(JobCtrl::ERR_CONNECT);
		return (0);
	case VersionMismatch:
		sendComEvent(JobCtrl::ERR_VERSION_PROT);
		return (0);
	case AuthNoKey:
		sendComEvent(JobCtrl::ERR_NO_KEY);
		return (0);
	case AuthWrongKeyId:
		sendComEvent(JobCtrl::ERR_KEYID);
		return (0);
	case AuthInvalidKey:
		sendComEvent(JobCtrl::ERR_INV_KEY);
		return (0);
	case AuthInvalidCert:
		sendComEvent(JobCtrl::ERR_INV_CERT);
		return (0);
	case AuthSysFail:
		sendComEvent(JobCtrl::ERR_AUTH_SYS_FAIL);
		return(0);
	}

	fds[0].fd = comPipe_[0];
	fds[0].events = POLLIN;
	fds[1].fd = channel_->fd;
	fds[1].events = POLLIN;

	while (1) {
		/*
		 * Step 1: Clear the notification pipe. We do this first
		 *         to ensure that other events that come in along
		 *         the way will cause the poll below to return
		 *         immediately.
		 */
		while(1) {
			int ret;
			char buf[100];

			ret = read(comPipe_[0], buf, sizeof(buf));
			if (ret <= 0)
				break;
		}
		/* Step 2: Send any pending escalation answers. */
		while((notify = answerQueue_->pop()) != 0) {
			anoubis_token_t		 token;
			bool			 allowed;
			EscalationNotify	*escalation;

			escalation = dynamic_cast<EscalationNotify *>(notify);
			if (escalation) {
				allowed = escalation->getAnswer()->wasAllowed();
				token = escalation->getToken();
				anoubis_client_notifyreply(client_, token,
				    allowed ? 0 : EPERM, 0);
			}
			if (notify->needFree())
				delete notify;
		}
		/* Step 3: If no task is running, try to start a new one. */
		if (current == NULL) {
			Task	*task = getNextTask(Task::TYPE_COM);

			current = dynamic_cast<ComTask *>(task);
			if (current && !current->shallAbort()) {
				current->setClient(client_);
				current->exec();
			}
		}
		/*
		 * Step 4: If the current task is done, handle this.
		 *         This also handles errors in current->exec()
		 *         and performs the next stpes of the task if the
		 *         task is not yet done.
		 */
		if (current && (current->shallAbort() || current->done())) {
			TaskEvent	event(current, wxID_ANY);

			/* Task is no longer a COM task. Reschedule. */
			if (current->getType() != Task::TYPE_COM) {
				JobCtrl::instance()->addTask(current);
				current = NULL;
				continue;
			}
			if (current->shallAbort())
				current->setTaskResultAbort();
			sendEvent(event);
			/* Restart in case there are more tasks on the list. */
			current = NULL;
			continue;
		}
		/*
		 * Step 5: At this point we have an active task that is
		 *         waiting for data from the network and all
		 *         pending answer requests have been handled.
		 *         We poll until either new Jobs or answers
		 *         become ready or a message from the daemon arrives.
		 */
		if (!isConnected() || exitThread())
			break;
		fds[0].revents = 0;
		fds[1].revents = 0;
		if (poll(fds, 2, 60000) <= 0) {
			continue;
		}
		/*
		 * Step 6: Process a message from the Daemon if there is
		 *         one. Then start over.
		 */
		if (fds[1].revents) {
			if (!readMessage())
				break;
		}
	}

	/* Thread is short before exit, disconnect again */
	disconnect();

	return (0);
}
bool
ComThread::readMessage(void)
{
	size_t			 size = 4096;
	struct anoubis_msg	*msg;
	char			*str = NULL;
	wxString		 message;
	achat_rc		 rc;

	if ((channel_ == 0) || (client_ == 0)) {
		sendComEvent(JobCtrl::ERR_RW);
		return (false);
	}

	if ((msg = anoubis_msg_new(size)) == 0) {
		sendComEvent(JobCtrl::ERR_RW);
		return (false);
	}
	while(1) {
		rc = acc_receivemsg(channel_, (char*)(msg->u.buf), &size);
		if (rc != ACHAT_RC_NOSPACE)
			break;
		size *= 2;
		if (anoubis_msg_resize(msg, size) < 0)
			rc = ACHAT_RC_ERROR;
	}
	if (rc != ACHAT_RC_OK) {
		anoubis_msg_free(msg);
		sendComEvent(JobCtrl::ERR_RW);

		return (false);
	}

	anoubis_msg_resize(msg, size);

	if (Debug::checkLevel(Debug::CHAT)) {
		anoubis_dump_str(msg, NULL, &str);
		Debug::chat(wxString::FromAscii(str));
		free(str);
	}

	/* This will free the message. */
	int result = anoubis_client_process(client_, msg);
	if (result < 0) {
		/* Error */
		/* XXX CEH */
		sendComEvent(JobCtrl::ERR_RW);
		return (false);
	} else if (result == 0) {
		/*
		 * Message does not fit into current protocol flow.
		 * Skip it.
		 */
		fprintf(stderr, "ComThread: Message does not fit ");
		fprintf(stderr, "into protocol flow, skipping...\n");
	} else if (anoubis_client_hasnotifies(client_) != 0) {
		struct anoubis_msg	*notifyMsg = NULL;
		bool			 handled;

		notifyMsg = anoubis_client_getnotify(client_);
		handled = checkNotify(notifyMsg);
		if (!handled) {
			sendNotify(notifyMsg);
		}
	}
	return (true);
}

bool
ComThread::checkNotify(struct anoubis_msg *notifyMsg)
{
	int type;
	pid_t pid;
	NotifyAnswer *answer;

	if (notifyMsg->u.general == NULL)
		return false;

	type = get_value((notifyMsg->u.general)->type);

	if (type == ANOUBIS_N_ASK) {
		pid = get_value((notifyMsg->u.notify)->pid);
		if (ProcCtrl::instance()->findPid(pid)) {
			EscalationNotify	*notify;

			notify  = new EscalationNotify(notifyMsg);
			answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE, true);
			notify->setAnswer(answer);
			notify->setNeedFree();
			pushNotification(notify);
			return true;
		}
	}

	return false;
}

void
ComThread::sendNotify(struct anoubis_msg *notifyMsg)
{
	int			 type;
	Notification		*notify;
	NotificationCtrl	*notifyCtrl;

	type = get_value((notifyMsg->u.general)->type);
	notify = NULL;
	/*
	 * NOTE: We must not allocate the NotificationCtrl from the
	 * NOTE: ComThread. It must be created in the main Thread.
	 * NOTE: This is particularly imporant in tests where the
	 * NOTE: event handlers of the NotificationCtrl do not work.
	 */
	notifyCtrl = NotificationCtrl::existingInstance();

	if (type == ANOUBIS_N_POLICYCHANGE) {
		wxCommandEvent		 pce(anEVT_POLICY_CHANGE);
		pce.SetInt(get_value(notifyMsg->u.policychange->prio));
		pce.SetExtraLong(get_value(notifyMsg->u.policychange->uid));
		if (dynamic_cast<AnoubisGuiApp*>(wxTheApp)) {
			wxPostEvent(AnEvents::instance(), pce);
		}
	} else if (type == ANOUBIS_N_PGCHANGE) {
		unsigned int	pgop = get_value(notifyMsg->u.pgchange->pgop);
		uint64_t	pgid = get_value(notifyMsg->u.pgchange->pgid);
		const char	*cmd = notifyMsg->u.pgchange->cmd;

		if (pgop == ANOUBIS_PGCHANGE_TERMINATE) {
			wxCommandEvent	pgchange(anEVT_PG_CHANGE);

			Debug::info(_("Playground %llx (%hs) terminated"),
			    pgid, cmd);

			/* XXX CEH: Should be a long long... */
			pgchange.SetExtraLong(pgid);
			if (dynamic_cast<AnoubisGuiApp *>(wxTheApp))
				wxPostEvent(AnEvents::instance(), pgchange);
		} else if (pgop == ANOUBIS_PGCHANGE_CREATE)
			Debug::info(_("Playground %llx (%hs) created"),
			    pgid, cmd);
	} else if (type == ANOUBIS_N_STATUSNOTIFY) {
		unsigned int		key, value;

		if (!VERIFY_FIELD(notifyMsg, statusnotify, statuskey)
		    || !VERIFY_FIELD(notifyMsg, statusnotify, statusvalue)) {
			anoubis_msg_free(notifyMsg);
			Debug::err(_("Dropping short message from Daemon"));
			return;
		}
		key = get_value(notifyMsg->u.statusnotify->statuskey);
		value = get_value(notifyMsg->u.statusnotify->statusvalue);
		switch(key) {
		case ANOUBIS_STATUS_UPGRADE: {
			if (dynamic_cast<AnoubisGuiApp*>(wxTheApp)) {
				wxCommandEvent	upg(anEVT_UPGRADENOTIFY);
				wxPostEvent(AnEvents::instance(), upg);
			}
			break;
		}
		default:
			Debug::info(_("Unkown status message key=%x value=%d"),
			    key, value);
		}
	} else {
		/*
		 * Ignore any notifies if we don't have a
		 * NotificationCtrl. This can happen in unit tests.
		 */
		if (notifyCtrl) {
			notify = Notification::factory(notifyMsg);
			if (notify != NULL) {
				notifyCtrl->addNotification(notify);
				notifyMsg = NULL;
			}
		}
	}
	if (notifyMsg)
		anoubis_msg_free(notifyMsg);
}

void
ComThread::pushNotification(Notification *notify)
{
	if ((notify != NULL) && IS_ESCALATIONOBJ(notify)) {
		answerQueue_->push(notify);
	} else if (notify->needFree()) {
		delete notify;
	}
}

void
ComThread::sendComEvent(JobCtrl::ConnectionState state)
{
	wxCommandEvent event(anEVT_COM_CONNECTION);

	event.SetInt(state);
	event.SetString(wxT("localhost"));

	wxPostEvent(JobCtrl::instance(), event);
}

void
ComThread::wakeup(bool)
{
	/*
	 * We don't care about the return value (might be EAGAIN).
	 * The cast to void silences a warning about the unused result
	 * on ubuntu.
	 */
	int ret = write(comPipe_[1], "A", 1);
	(void)ret;
}
