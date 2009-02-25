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
	#define CALCTASKQUEUE_POP_TIMEOUT 500
#endif

JobCtrl::JobCtrl(void)
{
	/* Default configuration */
	this->socketPath_ = wxT("/var/run/anoubisd.sock");

	csumCalcTaskQueue_ = new SynchronizedQueue<Task>(true);
	comTaskQueue_ = new SynchronizedQueue<Task>(false);
}

JobCtrl::~JobCtrl(void)
{
	stop();

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
		ComThread	*t = new ComThread(this, socketPath_);

		if (t->start()) {
			threadList_.push_back(t);
			return (true);
		} else {
			delete t;
			return (false);
		}
	} else
		return (false);
}

void
JobCtrl::disconnect(void)
{
	ComThread *t;

	while ((t = findComThread()) != 0) {
		t->stop();
		threadList_.DeleteObject(t);
		delete t;
	}
}

bool
JobCtrl::isConnected(void) const
{
	ComThread *t = findComThread();
	return (t != 0 ? t->isRunning() : false);
}

void
JobCtrl::answerNotification(Notification *notify)
{
	ComThread *t = findComThread();

	if (t != 0)
		t->pushNotification(notify);
}

void
JobCtrl::addTask(Task *task)
{
	switch (task->getType()) {
	case Task::TYPE_CSUMCALC:
		csumCalcTaskQueue_->push(task);
		break;
	case Task::TYPE_COM:
		comTaskQueue_->push(task);
		break;
	}
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
