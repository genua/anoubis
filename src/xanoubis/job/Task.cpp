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

#include "Task.h"
#include "TaskEvent.h"
#include "JobCtrl.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(TaskList);

Task::Task(Type type)
{
	this->type_ = type;
	this->abortRequest_ = false;
}

Task::~Task(void)
{
}

Task::Type
Task::getType(void) const
{
	return (this->type_);
}

void
Task::abort(void)
{
	abortRequest_ = true;
}

bool
Task::shallAbort(void) const
{
	return (abortRequest_);
}

void
Task::progress(wxString msg)
{
	TaskEvent	 evt(this, wxID_ANY, anTASKEVT_PROGRESS);
	JobCtrl		*jobctrl = JobCtrl::existingInstance();

	progressText_ = msg;
	if (jobctrl)
		wxPostEvent(jobctrl, evt);
}
