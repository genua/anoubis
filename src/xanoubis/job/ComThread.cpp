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

#include <anoubischat.h>
#include <anoubis_protocol.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>
#include <anoubis_dump.h>

#include "AnEvents.h"
#include "ComTask.h"
#include "ComThread.h"
#include "main.h"
#include "NotifyAnswer.h"
#include "NotificationCtrl.h"
#include "JobCtrl.h"
#include "ProcCtrl.h"

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

bool
ComThread::connect(void)
{
	struct sockaddr_storage	 ss;
	struct sockaddr_un	*ss_sun = (struct sockaddr_un *)&ss;
	achat_rc		 rc = ACHAT_RC_ERROR;

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
		return (false);

	rc = acc_settail(channel_, ACC_TAIL_CLIENT);
	if (rc != ACHAT_RC_OK) {
		disconnect();
		return (false);
	}

	rc = acc_setsslmode(channel_, ACC_SSLMODE_CLEAR);
	if (rc != ACHAT_RC_OK) {
		disconnect();
		return (false);
	}

	bzero(&ss, sizeof(ss));
	ss_sun->sun_family = AF_UNIX;
	strlcpy(ss_sun->sun_path, socketPath_.fn_str(),
	    sizeof(ss_sun->sun_path));
	rc = acc_setaddr(channel_, &ss, sizeof(struct sockaddr_un));
	if (rc != ACHAT_RC_OK) {
		disconnect();
		return (false);
	}

	rc = acc_prepare(channel_);
	if (rc != ACHAT_RC_OK) {
		disconnect();
		return (false);
	}

	rc = acc_open(channel_);
	if (rc != ACHAT_RC_OK) {
		disconnect();
		return (false);
	}

	/*
	 * *** client ***
	 */

	client_ = anoubis_client_create(channel_);
	if (client_ == 0) {
		disconnect();
		return (false);
	}

	if (anoubis_client_connect(client_, ANOUBIS_PROTO_BOTH) != 0) {
		disconnect();
		return (false);
	}

	return (true);
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

	if (!connect()) {
		sendComEvent(JobCtrl::CONNECTION_ERR_CONNECT);
		return (0);
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
		}
		/* Step 3: If no task is running, try to start a new one. */
		if (current == NULL) {
			Task	*task = getNextTask(Task::TYPE_COM);

			current = dynamic_cast<ComTask *>(task);
			if (current) {
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
		if (current && current->done()) {
			TaskEvent	event(current, wxID_ANY);
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
		sendComEvent(JobCtrl::CONNECTION_ERROR);
		return (false);
	}

	if ((msg = anoubis_msg_new(size)) == 0) {
		sendComEvent(JobCtrl::CONNECTION_ERROR);
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
		sendComEvent(JobCtrl::CONNECTION_ERROR);

		return (false);
	}

	anoubis_msg_resize(msg, size);

	if (Debug::instance()->checkLevel(DEBUG_CHAT)) {
		anoubis_dump_str(msg, NULL, &str);
		Debug::instance()->log(wxString::FromAscii(str), DEBUG_CHAT);
		free(str);
	}

	/* This will free the message. */
	int result = anoubis_client_process(client_, msg);
	if (result < 0) {
		/* Error */
		/* XXX CEH */
		sendComEvent(JobCtrl::CONNECTION_ERROR);
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
	EscalationNotify *notify = new EscalationNotify(notifyMsg);

	if (notifyMsg->u.general == NULL)
		return false;

	type = get_value((notifyMsg->u.general)->type);

	if (type == ANOUBIS_N_ASK) {
		pid = get_value((notifyMsg->u.notify)->pid);
		if (ProcCtrl::getInstance()->findPid(pid)) {
			answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE, true);
			notify->setAnswer(answer);
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
	wxCommandEvent		 pce(anEVT_POLICY_CHANGE);
	Notification		*notify;
	NotificationCtrl	*notifyCtrl;

	type = get_value((notifyMsg->u.general)->type);
	notify = NULL;
	notifyCtrl = NotificationCtrl::instance();

	if (type == ANOUBIS_N_POLICYCHANGE) {
		pce.SetInt(get_value(notifyMsg->u.policychange->prio));
		pce.SetExtraLong(get_value(notifyMsg->u.policychange->uid));
		if (dynamic_cast<AnoubisGuiApp*>(wxTheApp)) {
			wxPostEvent(AnEvents::getInstance(), pce);
		}
	} else {
		notify = Notification::factory(notifyMsg);
		if (notify != NULL) {
			notifyCtrl->addNotification(notify);
		}
	}
}

void
ComThread::pushNotification(Notification *notify)
{
	if ((notify != NULL) && IS_ESCALATIONOBJ(notify)) {
		answerQueue_->push(notify);
	}
}

void
ComThread::sendComEvent(JobCtrl::ConnectionState state)
{
	wxCommandEvent event(anEVT_COM_CONNECTION);

	event.SetInt(state);
	event.SetString(wxT("localhost"));

	wxPostEvent(JobCtrl::getInstance(), event);
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
