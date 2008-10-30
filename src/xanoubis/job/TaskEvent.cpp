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

DEFINE_LOCAL_EVENT_TYPE(anTASKEVT_CSUMCALC)
DEFINE_LOCAL_EVENT_TYPE(anTASKEVT_REGISTER)
DEFINE_LOCAL_EVENT_TYPE(anTASKEVT_POLICY_SEND)
DEFINE_LOCAL_EVENT_TYPE(anTASKEVT_POLICY_REQUEST)
DEFINE_LOCAL_EVENT_TYPE(anTASKEVT_CSUM_ADD)
DEFINE_LOCAL_EVENT_TYPE(anTASKEVT_CSUM_GET)
DEFINE_LOCAL_EVENT_TYPE(anTASKEVT_SFS_LIST)

TaskEvent::TaskEvent(Task *task, int id)
    : wxEvent(id, task->getEventType())
{
	this->m_propagationLevel = wxEVENT_PROPAGATE_MAX;
	this->task_ = task;
}

TaskEvent::TaskEvent(const TaskEvent &other)
    : wxEvent(other.GetId(), other.GetEventType())
{
	SetEventObject(other.GetEventObject());
	SetTimestamp(other.GetTimestamp());

	this->m_propagationLevel = wxEVENT_PROPAGATE_MAX;
	this->task_ = other.task_;
}

wxEvent *
TaskEvent::Clone(void) const
{
	return new TaskEvent(*this);
}

Task *
TaskEvent::getTask(void) const
{
	return (this->task_);
}
