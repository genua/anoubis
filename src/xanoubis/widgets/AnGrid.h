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

#ifndef _ANGRID_H_
#define _ANGRID_H_

#include <wx/grid.h>

/**
 * This realizes the Anoubis Grid-Widget.
 *
 * The AnGrid is an ordinary wxGrid, extended ty an event handler and
 * the ability to show and hide the columns on users choice.
 */
class AnGrid : public wxGrid
{
	public:
		/**
		 * Constructor of AnGrid.
		 * It has to have the same signature as a ordinary wxGrid,
		 * so wxformbuilder can just exchange wxGrid with AnGrid.
		 */
		AnGrid(wxWindow *parent, wxWindowID id,
		    const wxPoint& pos = wxDefaultPosition,
		    const wxSize& size = wxDefaultSize,
		    long style = wxWANTS_CHARS,
		    const wxString& name = wxGridNameStr);

		/**
		 * Destructor.
		 */
		~AnGrid(void);

	private:
		/**
		 * Event handler for right mouse button clicks.
		 * It will show a choice dialog and updates the columns.
		 * @param[in] 1st The grid event.
		 * @return Nothing.
		 */
		void onLabelRightClick(wxGridEvent &);
};

#endif	/* _ANGRID_H_ */
