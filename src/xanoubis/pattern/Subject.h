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

#ifndef _SUBJECT_H_
#define _SUBJECT_H_

#include <wx/list.h>

#include "Observer.h"

/**
 * Design pattern: Observer.
 *
 * This is a behavioral patterns. It's used to publish changes of an object
 * to one or may other depending objects. The object of interest is
 * called the subject. The objects(s) subscribed to it are called observer(s).
 *
 * This is the subject.
 * You have to add observers using the method addObserver() to get them
 * informed about changes of this subject. All added observers are also informed
 * about the end of live of the subject.
 *
 * You can't start the notification by yourself directly. Within each method
 * altering the subject you have to declare the begin and the end of a change.
 * This is done by calling the protected methods startChange() and
 * finishChange(). These methods ensure in case of multiple changes the
 * observers are notified only once.
 *
 * @code
 * ConcreteSubject::setValue(int value)
 * {
 *	startChange();
 *	value_ = value;
 *	finishChange();
 * }
 * @endcode
 */
class Subject
{
	public:
		/**
		 * Constructor of subject object.
		 * It ensures a clean observer list.
		 */
		Subject(void);

		/**
		 * Destructor of subject.
		 * This will remove this subject from each of it's observers.
		 */
		virtual ~Subject(void);

		/**
		 * Add an observer.
		 * @param 1st The observer to add.
		 * @return True on success.
		 * @see removeObserver()
		 */
		bool addObserver(Observer *);

		/**
		 * Remove an observer.
		 * @param 1st The observer to remove.
		 * @return True on success.
		 * @see addObserver()
		 */
		bool removeObserver(Observer *);

	protected:
		/**
		 * Start a new/next change.
		 * This method must be called before each setter method
		 * starts it's work.
		 * @param None.
		 * @return Nothing.
		 * @see finishChange()
		 */
		void startChange(void);

		/**
		 * End a change.
		 * This method must be called as last step of each
		 * setter method. This will notify all observers.
		 * @param None.
		 * @return Nothing.
		 * @see startChange()
		 */
		void finishChange(void);

	private:
		int		changeCount_;	/**< changes in progress */
		ObserverList	observers_;	/**< List of observers */

		/**
		 * Notify the observers.
		 * @param None
		 * @return Nothing
		 */
		void notifyObservers(void);
};

WX_DECLARE_LIST(Subject, SubjectList);

#endif	/* _SUBJECT_H_ */
