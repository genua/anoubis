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

#ifndef _ANROWPROVIDER_H_
#define _ANROWPROVIDER_H_

#include <AnListClass.h>
#include <AnRowViewer.h>

class AnListCtrl;

/**
 * This interface must be implemented by a model if the data in the model
 * must be displayed in an AnListCtrl. A model implementing this interface
 * must also call the update functions of the rowViewer if the model changes.
 */
class AnRowProvider {
	protected:
		/*
		 * This friend directive enables us to make the rowViewer_
		 * variable protected.
		 */
		friend class AnListCtrl;

		/**
		 * The rowViewer_ (aka AnListCtrl) associated with the model.
		 * The variable may be NULL.
		 */
		class AnRowViewer		*rowViewer_;

		/**
		 * Assign a viewer to the model.
		 * Currently only a single viewer can be assigned to a model
		 * at any point in time. Setting the viewer to NULL will
		 * is allowed.
		 * @param v The new viewer.
		 * @return None.
		 *
		 * NOTE: This function should normally not be called
		 *     directly. It is usually called by AnListCtrl only.
		 */
		void setViewer(class AnRowViewer *v) { rowViewer_ = v; };

	public:
		/**
		 * Return the element with index idx in the model.
		 * @param idx The index.
		 * @return The object associated with the idex.
		 *     If the index is out range NULL may be returned.
		 */
		virtual AnListClass *getRow(unsigned int idx) const = 0;

		/**
		 * Return the total number of items in the model.
		 * @return The total number of items in the model.
		 */
		virtual int getSize(void) const = 0;
};

#endif	/* _ANROWPROVIDER_H_ */
