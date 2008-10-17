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

#include <wx/filename.h>

#include <CsumCalcTask.h>
#include <JobCtrl.h>

#include "TcCsumCalcTask.h"

static wxString csumFile1 =
    wxT("ae20df65c0a9dd7e9ab9d9ce2c13caf7a08f71278065fd9928590a492ec012eb");
static wxString csumFile2 =
    wxT("1344d2fa0e5825c10f07889f7931692fae6d15c18b313087b2873cf1a9958276");
static wxString csumFile3 =
    wxT("db2440958ad76c3773cd20a90997aa5c623ed713087bccc247186bb5abaeff9d");

TcCsumCalcTask::TcCsumCalcTask()
{
	this->result_ = 0;
	this->haveResult1_ = false;
	this->haveResult2_ = false;
	this->haveResult3_ = false;

	this->task1_ = new CsumCalcTask;
	this->task2_ = new CsumCalcTask;

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUMCALC,
	    wxTaskEventHandler(TcCsumCalcTask::OnTaskEvent), NULL, this);
}

TcCsumCalcTask::~TcCsumCalcTask()
{
	delete this->task1_;
	delete this->task2_;
}

void TcCsumCalcTask::startTest()
{
	wxString home = wxFileName::GetHomeDir();

	this->task1_->setPath(home + wxT("/csumFile1"));
	this->task2_->setPath(home + wxT("/csumFile2"));

	JobCtrl::getInstance()->addTask(this->task1_);
	JobCtrl::getInstance()->addTask(this->task2_);
}

bool TcCsumCalcTask::canExitTest() const
{
	return this->haveResult1_ && this->haveResult2_ && this->haveResult3_;
}

int TcCsumCalcTask::getTestResult() const
{
	return (this->result_);
}

void
TcCsumCalcTask::OnTaskEvent(TaskEvent &event)
{
	if (event.getTask() == this->task1_) {
		if (!this->haveResult1_) {
			this->haveResult1_ = true;
			checkCsumFile1();

			wxString home = wxFileName::GetHomeDir();
			this->task1_->setPath(home + wxT("/csumFile3"));
			JobCtrl::getInstance()->addTask(this->task1_);
		}
		else /* !this->haveResult3_ */ {
			this->haveResult3_ = true;
			checkCsumFile3();
		}
	}
	else if (event.getTask() == this->task2_) {
		this->haveResult2_ = true;
		checkCsumFile2();
	}
}

void
TcCsumCalcTask::checkCsumFile1()
{
	CsumCalcTask *t = this->task1_;

	if (t->getResult() < 0) {
		this->result_ = -t->getResult();
		fprintf(stderr, "No csum received for csumFile1: %i (%s)\n",
		    t->getResult(), strerror(this->result_));

		return;
	}

	if (t->getCsumStr() != csumFile1) {
		this->result_ = 4711;
		fprintf(stderr, "wrong checksum (%s) but should be %s\n",
		    (const char*)t->getCsumStr().fn_str(),
		    (const char*)csumFile1.fn_str());

		return;
	}
}

void
TcCsumCalcTask::checkCsumFile2()
{
	CsumCalcTask *t = this->task2_;

	if (t->getResult() < 0) {
		this->result_ = -t->getResult();
		fprintf(stderr, "No csum received for csumFile2: %i (%s)\n",
		    t->getResult(), strerror(this->result_));

		return;
	}

	if (t->getCsumStr() != csumFile2) {
		this->result_ = 4712;
		fprintf(stderr, "wrong checksum (%s) but should be %s\n",
		    (const char*)t->getCsumStr().fn_str(),
		    (const char*)csumFile1.fn_str());

		return;
	}
}

void
TcCsumCalcTask::checkCsumFile3()
{
	/* 3rd file part of task1! */
	CsumCalcTask *t = this->task1_;
	if (t->getResult() < 0) {
		this->result_ = -t->getResult();
		fprintf(stderr, "No csum received for csumFile3: %i (%s)\n",
		    t->getResult(), strerror(this->result_));
		return;
	}

	if (t->getCsumStr() != csumFile3) {
		this->result_ = 4713;
		fprintf(stderr, "wrong checksum (%s) but should be %s\n",
		    (const char*)t->getCsumStr().fn_str(),
		    (const char*)csumFile1.fn_str());

		return;
	}
}
