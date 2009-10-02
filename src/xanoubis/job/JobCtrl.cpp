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

#include <wx/thread.h>

#include "AnEvents.h"
#include "ComThread.h"
#include "CsumCalcThread.h"
#include "JobCtrl.h"
#include "main.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(JobThreadList);

#ifndef CALCTASKQUEUE_POP_TIMEOUT
	#define CALCTASKQUEUE_POP_TIMEOUT 60000	/* One Minute */
#endif

JobCtrl::JobCtrl(void)
{
	/* Default configuration */
	this->socketPath_ = wxT(PACKAGE_SOCKET);
	this->sfsdisable_ = false;

	csumCalcTaskQueue_ = new SynchronizedQueue<Task>(true);
	comTaskQueue_ = new SynchronizedQueue<Task>(false);

	Connect(anTASKEVT_REGISTER,
	    wxTaskEventHandler(JobCtrl::onDaemonRegistration), NULL, this);
	Connect(anTASKEVT_VERSION,
	    wxTaskEventHandler(JobCtrl::onDaemonVersion), NULL, this);
	Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(JobCtrl::onConnectionError), NULL, this);
}

JobCtrl::~JobCtrl(void)
{
	stop();

	Disconnect(anTASKEVT_REGISTER,
	    wxTaskEventHandler(JobCtrl::onDaemonRegistration), NULL, this);
	Disconnect(anTASKEVT_VERSION,
	    wxTaskEventHandler(JobCtrl::onDaemonVersion), NULL, this);
	Disconnect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(JobCtrl::onConnectionError), NULL, this);

	delete csumCalcTaskQueue_;
	delete comTaskQueue_;
}

JobCtrl *
JobCtrl::getInstance(void)
{
	return (instance());
}

wxString
JobCtrl::getSocketPath(void) const
{
	return (this->socketPath_);
}

void
JobCtrl::setSocketPath(const wxString &path)
{
	this->socketPath_ = path;
}

bool
JobCtrl::start(void)
{
	/* Checksum calculation thread */
	CsumCalcThread *csumCalcThread = new CsumCalcThread(this);
	threadList_.push_back(csumCalcThread);
	if (!csumCalcThread->start()) {
		stop();
		return (false);
	}

	return (true);
}

void
JobCtrl::stop(void)
{
	/* Stop and destroy all background threads */
	while (!threadList_.empty()) {
		JobThread *thread = threadList_.front();
		threadList_.pop_front();
		thread->stop();
		delete thread;
	}
}

bool
JobCtrl::connect(void)
{
	if (!isConnected()) {
		ComThread	*t;

		t = new ComThread(this, socketPath_);
		if (t->start()) {
			if (t->exitThread()) {
				t->stop();
				delete t;

				return (false);
			} else {
				threadList_.push_back(t);

				regTask_.setAction(
				    ComRegistrationTask::ACTION_REGISTER);
				addTask(&regTask_);

				return (true);
			}
		} else {
			delete t;
			sendComEvent(JobCtrl::CONNECTION_ERR_CONNECT);

			return (false);
		}
	} else
		return (false);
}

void
JobCtrl::disconnect(void)
{
	regTask_.setAction(ComRegistrationTask::ACTION_UNREGISTER);
	JobCtrl::getInstance()->addTask(&regTask_);
}

bool
JobCtrl::isConnected(void) const
{
	ComThread *t = findComThread();
	if (t == 0)
		return (false);
	return (t->isRunning());
}

bool
JobCtrl::isSfsDisable(void) const
{
	return (sfsdisable_);
}

int
JobCtrl::getDaemonApnVersion(void) const
{
	return (versionTask_.getApnVersion());
}

void
JobCtrl::answerNotification(Notification *notify)
{
	ComThread *t = findComThread();

	if (t != 0) {
		t->pushNotification(notify);
		if ((notify != NULL) && IS_ESCALATIONOBJ(notify)) {
			t->wakeup(false);
		}
	}
}

void
JobCtrl::addTask(Task *task)
{
	JobThread	*t = NULL;
	switch (task->getType()) {
	case Task::TYPE_CSUMCALC:
		csumCalcTaskQueue_->push(task);
		t = findCsumCalcThread();
		break;
	case Task::TYPE_COM:
		comTaskQueue_->push(task);
		t = findComThread();
		break;
	}
	if (t)
		t->wakeup(false);
}

Task *
JobCtrl::popTask(Task::Type type)
{
	switch (type) {
	case Task::TYPE_CSUMCALC:
		return (csumCalcTaskQueue_->pop(CALCTASKQUEUE_POP_TIMEOUT));
	case Task::TYPE_COM:
		return (comTaskQueue_->pop());
	}

	return (0); /* Never reached */
}

void
JobCtrl::onDaemonRegistration(TaskEvent &event)
{
	ComRegistrationTask *task =
	    dynamic_cast<ComRegistrationTask*>(event.getTask());

	if (task == 0)
		return;

	if (task->getAction() == ComRegistrationTask::ACTION_REGISTER) {
		if (task->getComTaskResult() == ComTask::RESULT_SUCCESS) {
			sfsdisable_ = true;
			/* Next, fetch versions from daemon */
			addTask(&versionTask_);
		} else if (task->getComTaskResult() ==
		    ComTask::RESULT_COM_SFS_DISABLE_ERROR) {
			sfsdisable_ = false;
			/* Next, fetch versions from daemon */
			addTask(&versionTask_);
		} else {
			/* Registration failed, disconnect again */
			sendComEvent(JobCtrl::CONNECTION_ERR_REG);
			disconnect();
		}
	} else { /* ACTION_UNREGISTER */
		ComThread *t;

		if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
			sendComEvent(JobCtrl::CONNECTION_ERR_REG);
		}

		/* Disconnect independent from unregistration-result */
		while ((t = findComThread()) != 0) {
			threadList_.DeleteObject(t);
			t->stop();
			delete t;
		}

		sendComEvent(JobCtrl::CONNECTION_DISCONNECTED);
	}

	event.Skip();
}

void
JobCtrl::onDaemonVersion(TaskEvent &event)
{
	event.Skip();

	if (event.getTask() != &versionTask_) {
		/* Not "my" task, do nothing */
		return;
	}

	if (versionTask_.getComTaskResult() != ComTask::RESULT_SUCCESS) {
		sendComEvent(JobCtrl::CONNECTION_ERR_VERSION);
		disconnect();

		return;
	}

	if (versionTask_.getApnVersion() != apn_parser_version()) {
		sendComEvent(JobCtrl::CONNECTION_ERR_VERSION);
		disconnect();

		return;
	}

	sendComEvent(JobCtrl::CONNECTION_CONNECTED);
}

void
JobCtrl::onConnectionError(wxCommandEvent &event)
{
	ComThread *t;

	if (event.GetInt() == JobCtrl::CONNECTION_ERROR) {
		while ((t = findComThread()) != 0) {
			threadList_.DeleteObject(t);
			t->stop();
			delete t;
		}
	}
	event.Skip();
}

ComThread *
JobCtrl::findComThread(void) const
{
	const JobThreadList::const_iterator end = threadList_.end();

	for (JobThreadList::const_iterator it = threadList_.begin();
	    it != end; ++it) {

		ComThread *t = dynamic_cast<ComThread*>(*it);

		if (t != 0)
			return (t);
	}

	return (0); /* No such thread */
}

CsumCalcThread *
JobCtrl::findCsumCalcThread(void) const
{
	const JobThreadList::const_iterator end = threadList_.end();

	for (JobThreadList::const_iterator it = threadList_.begin();
	    it != end; ++it) {

		CsumCalcThread *t = dynamic_cast<CsumCalcThread*>(*it);

		if (t != 0)
			return (t);
	}

	return (0); /* No such thread */
}

void
JobCtrl::sendComEvent(JobCtrl::ConnectionState state)
{
	wxCommandEvent event(anEVT_COM_CONNECTION);

	event.SetInt(state);
	event.SetString(wxT("localhost"));

	wxPostEvent(JobCtrl::getInstance(), event);
}
