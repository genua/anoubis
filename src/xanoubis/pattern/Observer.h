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

#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include <wx/list.h>

class Subject;
class SubjectList;

/**
 * Design pattern: Observer.
 *
 * This is a behavioral patterns. It's used to publish changes of an object
 * to one or may other depending objects. The object of interest is
 * called the subject. The objects(s) subscribed to it are called observer(s).
 *
 * This is the observer.
 * The subject given to the constructor may be NULL. During each time you can
 * add more subjects with the method addSubject().
 *
 * You have to provide an implementation of method update(). The precise
 * change of the subject must be queried from it.
 */
class Observer
{
	public:
		/**
		 * Constructor of observer object.
		 * The given subject may be NULL. Additional subjects may be
		 * added with addSubject(). This automatically registers for
		 * update notifications of the given subject.
		 * @param 1st The subject to observer.
		 * @see addSubject()
		 */
		Observer(Subject *);

		/**
		 * Destructor of observer.
		 * This will automatically unregister from all subjects.
		 * @param None.
		 */
		virtual ~Observer(void);

		/**
		 * Notification from the given subject about it's change.
		 * This must be implemented by a derived class.
		 * @param 1st The changed subject.
		 * @return Nothing.
		 */
		virtual void update(Subject *) = 0;

		/**
		 * Notification from the given subject about it's
		 * impending destruction.
		 * @param 1st The changed subject.
		 * @return Nothing.
		 * NOTE: Even if you overwrite this method there is no
		 * NOTE: need to call removeSubject manually.
		 */
		virtual void updateDelete(Subject *);

		/**
		 * Add a subject.
		 * This will also register for update notifications at the
		 * given subject.
		 * @param 1st The subject to add.
		 * @return True on success.
		 */
		bool addSubject(Subject *);

		/**
		 * Remove a subject.
		 * This will also unregister from updates of the given subject.
		 * @param 1st The subject to remove.
		 * @return True on success.
		 */
		bool removeSubject(Subject *);

	private:
		SubjectList *subjects_; /**< The observed subjects */
		void deleteHandler(Subject *);

	friend class Subject;
};

WX_DECLARE_LIST(Observer, ObserverList);

#endif	/* _OBSERVER_H_ */
