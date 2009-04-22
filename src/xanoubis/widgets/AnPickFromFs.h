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

#ifndef _ANPICKFROMFS_H_
#define _ANPICKFROMFS_H_

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

/**
 * This realizes the Pick-From-Filesystem-Widgets.
 *
 * The PickFromFs widget is an ordinary panel containing the widgets for
 * choosing a file. It's possible to enter the filename manualy or to hit
 * the pick-button, which will open a gtk file picker dialog.\n
 * \n
 * \b Usage (with the wxformbuilder):\n
 * - Add a normal wxPanel
 * - Set the property 'window_name' to the value you want to be shown by the
 *   label on the left side of the text input widget. (Think of "Binary" or
 *   "Program".) This is also used for the title of the picker dialog.
 * - Set the property 'subclass.name' to "AnPickFromFs"
 * - Set the property 'subclass.include' to "AnPickFromFs.h"
 * - Disable the stretching of the panel. (It's Alt-s or the icon with the
 *   square and the arrow on it's top and on the right side.)
 * - The panel can't be extedned by additional widgets.
 */
class AnPickFromFs : public wxPanel
{
	public:
		/**
		 * Constructor of the details panel.
		 * It has to have the same signature as a ordinary wxPanel,
		 * so wxformbuilder can just exchange wxPanel with AnPickFromFs.
		 */
		AnPickFromFs(wxWindow *parent, wxWindowID id = wxID_ANY,
		    const wxPoint& pos = wxDefaultPosition,
		    const wxSize& size = wxDefaultSize,
		    long style = wxTAB_TRAVERSAL,
		    const wxString& name = wxPanelNameStr);

	protected:
		/**
		 * This is the text just left to the textCtrl. It is set to the
		 * value of window_name (set within the properties in the
		 * wxformbuilder).\n
		 */
		wxStaticText *label_;

		/**
		 * This is the text control a user can enter the file manually.
		 * This will also show the file picked by file chooser dialog.
		 */
		wxTextCtrl *inputTextCtrl_;

		/**
		 * This button will open the file chooser dialog.
		 */
		wxButton *pickButton_;

	private:
		/**
		 * Handle focus events from inputTextCtrl (e.g on hit <tab>).
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onTextKillFocus(wxFocusEvent &);

		/**
		 * Handle events from inputTextCtrl.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onTextEnter(wxCommandEvent &);

		/**
		 * Handle events from pickButton.
		 * This will open a file chooser and on 'ok' the choosen
		 * file is written to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onPickButton(wxCommandEvent &);
};

#endif	/* _ANPICKFROMFS_H_ */
