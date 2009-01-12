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

#include "Subject.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(SubjectList);

IMPLEMENT_CLASS(Subject, wxObject);

Subject::Subject(void)
{
	changeCount_ = 0;
	observers_.Clear();
}

Subject::~Subject(void)
{
	ObserverList::iterator	it;

	it = observers_.begin();
	while (it != observers_.end()) {
		(*it)->removeSubject(this);
		it++;
	}
	observers_.Clear();
}

bool
Subject::addObserver(Observer *observer)
{
	/* Is this a valid observer been added? */
	if (observer == NULL) {
		return (false);
	}

	/* Is the given observer already in the list? */
	if (observers_.IndexOf(observer) != wxNOT_FOUND) {
		return (false);
	}

	observers_.Append(observer);

	return (true);
}

bool
Subject::removeObserver(Observer *observer)
{
	return (observers_.DeleteObject(observer));
}

void
Subject::startChange(void)
{
	changeCount_++;
}

void
Subject::finishChange(void)
{
	if (changeCount_ > 0) {
		changeCount_--;
	}
	if (changeCount_ == 0) {
		notifyObservers();
	}
}

void
Subject::notifyObservers(void)
{
	ObserverList::iterator	it;

	it = observers_.begin();
	while (it != observers_.end()) {
		(*it)->update(this);
		it++;
	}
}
