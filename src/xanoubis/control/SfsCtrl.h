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

#ifndef _SFSCTRL_H_
#define _SFSCTRL_H_

#include <wx/event.h>

#include "SfsDirectory.h"

/**
 * The SfsController acts as a bridge between an SfsDirectory and the view.
 * Operations on the directory are started here and changes on the directory
 * are reported back to the view.
 *
 * The the content of the assigned SfsDirectory has changed, an wxCommandEvent
 * with  event-type anEVT_SFSDIR_CHANGED is created.
 */
class SfsCtrl : public wxEvtHandler
{
	public:
		/**
		 * @see SfsDirectory::getPath()
		 */
		wxString getPath() const;
		void setPath(const wxString &);

		/**
		 * @see SfsDirectory::getFilter()
		 */
		wxString getFilter() const;
		void setFilter(const wxString &);

		/**
		 * @see SfsDirectory::isFilterInversed()
		 */
		bool isFilterInversed() const;
		void setFilterInversed(bool);

		/**
		 * Returns the instance of the SfsDirectory.
		 */
		const SfsDirectory &getSfsDirectory() const;

	private:
		SfsDirectory sfsDir_;

		void sendChangedEvent();
};

#endif	/* _SFSCTRL_H_ */
