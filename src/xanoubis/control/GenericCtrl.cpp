/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include "GenericCtrl.h"

GenericCtrl::GenericCtrl(void)
{
	errorList_.clear();
	taskList_.clear();
}

GenericCtrl::~GenericCtrl(void)
{
	std::set<Task *>::iterator	 it;
	Task				*task;

	errorList_.clear();
	while (!taskList_.empty()) {
		it = taskList_.begin();
		task = *it;
		taskList_.erase(it);
		delete task;
	}
}

bool
GenericCtrl::hasErrors(void) const
{
	return (!errorList_.empty());
}

const std::vector<wxString>
GenericCtrl::getErrors(void) const
{
	return (errorList_);
}

void
GenericCtrl::clearErrors(void)
{
	errorList_.clear();
}

bool
GenericCtrl::addError(const wxString & error)
{
	if (error.IsEmpty()) {
		return (false);
	}

	errorList_.push_back(error);
	return (true);
}

bool
GenericCtrl::addTask(Task *task)
{
	if (task == NULL) {
		return (false);
	}

	taskList_.insert(task);
	return (true);
}

bool
GenericCtrl::removeTask(Task *task)
{
	std::set<Task *>::iterator it;

	it = taskList_.find(task);
	if (it == taskList_.end()) {
		/* Task not in our list! */
		return (false);
	}

	taskList_.erase(it);
	return (true);
}

bool
GenericCtrl::isValidTask(Task *task) const
{
	if (task == NULL) {
		return (false);
	}

	if (taskList_.find(task) == taskList_.end()) {
		/* Belongs to someone other, ignore it */
		return (false);
	}

	return (true);
}

void
GenericCtrl::sendEvent(WXTYPE type)
{
	wxCommandEvent event(type);

	event.SetEventObject(this);
	ProcessEvent(event);
}
