/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#ifndef _NOTIFICATIONPERSPECTIVE_H_
#define _NOTIFICATIONPERSPECTIVE_H_

#include <wx/dynarray.h>

#include "Subject.h"

/**
 * This is a perspective to a subset of Notifications.
 * It is a dynamic array storing the ids of the related notification.
 * The notifications itself are stored by NotificationCtrl and the
 * control is the only one who fills a perspective.
 * With begin() and end() you can get iterators to walk up and down
 * this list and if you are a observer you can also be notified
 * about new entries.
 */
class NotificationPerspective : public Subject
{
	public:
		/**
		 * Constructor of NotificationPerspective.
		 * @param None.
		 */
		NotificationPerspective(void);

		/**
		 * Get the size of this list.
		 * @paran None.
		 * @return The number of elements within this list.
		 */
		long getSize(void) const;

		/**
		 * Get the id of a notification depending on the given index.
		 * @param[in] 1st Index at current list.
		 * @return Id of notify at NotificationCtrl.
		 */
		long getId(long) const;

		/**
		 * Get iterator.
		 *
		 * This delegates the begin() to ids_ array to get a
		 * iterator pointing to the first element.
		 * @param None.
		 * @return Iterator to first element.
		 */
		wxArrayLong::const_iterator begin(void) const;

		/**
		 * Get iterator.
		 *
		 * This delegates the end() to ids_ array to get a
		 * iterator pointing to the last element.
		 * @param None.
		 * @return Iterator to last element.
		 */
		wxArrayLong::const_iterator end(void) const;

	private:
		/**
		 * The array of ids forming this list.
		 */
		wxArrayLong ids_;

		/**
		 * Destructor of NotificationPerspective.
		 * @param None.
		 */
		~NotificationPerspective(void);

		/**
		 * Append a new element at the end of the list.
		 * @param[in] 1st The new element: id of a notification.
		 * @return Nothing.
		 */
		void addId(long);

		/**
		 * Remove a given id from the list.
		 * @param[in] 1st The id to remove.
		 * @return Nothing.
		 */
		void removeId(long);

	friend class NotificationCtrl;
};

#endif	/* _NOTIFICATIONPERSPECTIVE_H_ */
