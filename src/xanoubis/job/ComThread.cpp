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

#include <poll.h>

#include <anoubischat.h>
#include <anoubis_protocol.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>

#include "AlertNotify.h"
#include "AnEvents.h"
#include "ComTask.h"
#include "ComThread.h"
#include "EscalationNotify.h"
#include "LogNotify.h"
#include "main.h"
#include "NotifyAnswer.h"
#include "StatusNotify.h"

ComThread::ComThread(JobCtrl *jobCtrl, const wxString &socketPath)
    : JobThread(jobCtrl)
{
	this->socketPath_ = socketPath;
	this->channel_ = 0;
	this->client_ = 0;
	answerQueue_ = new SynchronizedQueue<Notification>(false);
}

ComThread::~ComThread()
{
	delete answerQueue_;
}

bool
ComThread::connect(void)
{
	struct sockaddr_storage	 ss;
	struct sockaddr_un	*ss_sun = (struct sockaddr_un *)&ss;
	achat_rc		 rc = ACHAT_RC_ERROR;

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
	bool fetchOnIdle = false; /* Workaround! See #854 */

	while (!exitThread()) {
		Task *task = getNextTask(Task::TYPE_COM);
		ComTask *comTask = dynamic_cast<ComTask*>(task);

		if (comTask != 0) {
			comTask->setComHandler(this);
			comTask->exec();

			ComRegistrationTask *regTask =
			    dynamic_cast<ComRegistrationTask*>(comTask);
			if (regTask != 0) {
				if (regTask->getAction() ==
				    ComRegistrationTask::ACTION_REGISTER) {
					fetchOnIdle = true;
				}
			}

			TaskEvent event(comTask, wxID_ANY);
			sendEvent(event);
		} else if (fetchOnIdle)
			waitForMessage();
	}

	/* Thread is short before exit, disconnect again */
	disconnect();

	return (0);
}

struct anoubis_client *
ComThread::getClient(void) const
{
	return (this->client_);
}

bool
ComThread::waitForMessage(void)
{
	struct pollfd		fds[1];
	int			result;

	if ((channel_ == 0) || (client_ == 0)) {
		/* Not connected */
		return (false);
	}

	fds[0].fd = channel_->fd;
	fds[0].events = POLLIN;

	/*
	 * XXX use event.h instead of poll
	 */
	result = poll(fds, 1, 200);
	if (result < 0) {
		if (errno == EINTR)
			return (true);

		return (false);
	}

	if (result > 0 && fds[0].revents) {
		size_t			size = 4096;
		struct anoubis_msg	*msg;

		if ((msg = anoubis_msg_new(size)) == 0)
			return (false);

		fds[0].revents = 0;
		achat_rc rc = acc_receivemsg(
		    channel_, (char*)(msg->u.buf), &size);
		if (rc != ACHAT_RC_OK) {
			anoubis_msg_free(msg);
			return (false);
		}

		anoubis_msg_resize(msg, size);

		/* This will free the message. */
		if (anoubis_client_process(client_, msg) != 1)
			return (false);

		if (anoubis_client_hasnotifies(client_) != 0) {
			struct anoubis_msg *notifyMsg = NULL;

			notifyMsg = anoubis_client_getnotify(client_);
			switch(get_value(notifyMsg->u.general->type)) {
			case ANOUBIS_N_POLICYCHANGE:
				/*
				 * XXX CEH: Policy change must be handled
				 * XXX CEH: properly.
				 */
				anoubis_msg_free(notifyMsg);
				break;
			default:
				if (checkNotify(notifyMsg))
					break;
				sendNotify(notifyMsg);
				break;
			}
		}
	}

	Notification *notification;
	while ((notification = answerQueue_->pop()) != 0) {
		anoubis_token_t		 token;
		int			 allowed;
		EscalationNotify	*escalation;

		escalation = (EscalationNotify *)notification;

		token = escalation->getToken();
		allowed = escalation->getAnswer()->wasAllowed();
		anoubis_client_notifyreply(client_, token,
		    allowed ? 0 : EPERM, 0);
	}

	return (true);
}

bool
ComThread::startHook(void)
{
	return (connect());
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
		if (pid == getpid()) {
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
	wxCommandEvent		 addEvent(anEVT_ADD_NOTIFICATION);
	int			 type;
	int			 subsystem;
	Notification		*notify = 0;

	type = get_value((notifyMsg->u.general)->type);
	subsystem = get_value((notifyMsg->u.notify)->subsystem);
	if (ANOUBIS_IS_NOTIFY(type)) {
		switch (type) {
		case ANOUBIS_N_ASK:
			notify = new EscalationNotify(notifyMsg);
			break;
		case ANOUBIS_N_NOTIFY:
			if (subsystem == ANOUBIS_SOURCE_STAT) {
				notify = new StatusNotify(notifyMsg);
			} else {
				notify = new LogNotify(notifyMsg);
			}
			break;
		case ANOUBIS_N_RESYOU:
			/*
			 * XXX ST: #461
			 * The handling of ANOUBIS_N_RESYOU has to be
			 * implemented asap.
			 * Currently we just fake the handling to prevent
			 * xanoubis from aborting.
			 */
			return;
			break;
		case ANOUBIS_N_RESOTHER:
			/*
			 * XXX ST: #461
			 * The handling of ANOUBIS_N_RESOTHER has to be
			 * implemented asap.
			 * Currently we just fake the handling to prevent
			 * xanoubis from aborting.
			 */
			return;
			break;
		case ANOUBIS_N_LOGNOTIFY:
			switch (get_value((notifyMsg->u.notify)->loglevel)) {
			case APN_LOG_NONE:
				/* don't show notifies of loglevel 0/none */
				break;
			case APN_LOG_NORMAL:
				notify = new LogNotify(notifyMsg);
				break;
			case APN_LOG_ALERT:
				notify = new AlertNotify(notifyMsg);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}

		addEvent.SetClientObject((wxClientData*)notify);
		wxGetApp().sendEvent(addEvent);
	}
}

void
ComThread::pushNotification(Notification *notify)
{
	if ((notify != NULL) && IS_ESCALATIONOBJ(notify))
		answerQueue_->push(notify);
}
