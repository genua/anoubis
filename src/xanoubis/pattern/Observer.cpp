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

#include "Observer.h"
#include "Subject.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(ObserverList);

Observer::Observer(Subject *subject)
{
	subjects_ = new SubjectList();
	addSubject(subject);
}

Observer::~Observer(void)
{
	SubjectList::iterator	it;

	if (subjects_ == NULL) {
		return;
	}

	/*
	 * The wxList Iterator is not stable accross removes of the
	 * current object from the list. Thus we always use the
	 * first element from the list and rely on the fact that
	 * removeSubject will remove the current object from the
	 * list.
	 */
	while ((it = subjects_->begin()) != subjects_->end()) {
		removeSubject(*it);
	}

	delete subjects_;
}

void
Observer::deleteHandler(Subject *subject)
{
	removeSubject(subject);
	updateDelete(subject);
}

bool
Observer::addSubject(Subject *subject)
{
	bool result;

	/* Is this a valid subject been added? */
	if ((subject == NULL) || (subjects_ == NULL)) {
		return (false);
	}

	/* Is the given subject already in the list? */
	if (subjects_->IndexOf(subject) != wxNOT_FOUND) {
		return (false);
	}

	subjects_->Append(subject);
	result = subject->addObserver(this);

	return (result);
}

bool
Observer::removeSubject(Subject *subject)
{
	bool result;

	if ((subject == NULL) || (subjects_ == NULL)) {
		return (false);
	}

	/* remove this observer from subject */
	result = subject->removeObserver(this);
	if (result == false) {
		return (false);
	}

	/* remove subject for list of observed subjects */
	result = subjects_->DeleteObject(subject);
	return (result);
}
