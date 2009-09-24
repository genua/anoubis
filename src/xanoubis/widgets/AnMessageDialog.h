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

#ifndef _ANMESSAGEDIALOG_H_
#define _ANMESSAGEDIALOG_H_

#include <wx/dialog.h>

class wxCheckBox;
class wxStaticBitmap;
class wxStaticText;
class AnMessageDialog;
#include "main.h"

/**
 * This is the AnMessageDialog widget class
 * This widget shall be used when a standard dialog should be presented
 * to the user.
 */
class AnMessageDialog : public wxDialog
{
	public:
		/**
		 * Constructor of AnMessageDialog
		 * @param[in] 1st The parent window
		 * @param[in] 2nd The dialog message
		 * @param[in] 3rd The dialog caption
		 * @param[in] 4th The dialog style
		 * @param[in] 5th The dialog position
		 */
		AnMessageDialog(wxWindow *parent,
		    const wxString &message,
		    const wxString &caption,
		    long style = wxOK | wxCANCEL,
		    const wxPoint &pos = wxDefaultPosition);

		/**
		 * Destructor of AnMessageDialog
		 * @param None.
		 */
		~AnMessageDialog(void);

		/**
		 * Gets called by the callee and decides if the message dialog
		 * should be shown or not.
		 * @param None.
		 * @return int
		 */
		int ShowModal(void);

		/**
		 * Reads config file by the given config string and
		 * shows dialog if the returned value is TRUE.
		 * @return boolean
		 */
		bool onNotifyCheck(const wxString &);

	private:
		/**
		 * The checkbox of the dialog widget
		 */
		wxCheckBox*	dontShowMessageAgain;

		/**
		 * The userOptions object provided by the config file
		 */
		wxConfig*	userOptions_;

		/**
		 * The userOptions config string
		 */
		wxString	configString_;

		/**
		 * Chooses the Icon for the dialog based on the
		 * style attribute.
		 * @param[in] 1st the dialog style FLAG.
		 * @return wxStaticBitmap
		 */
		wxStaticBitmap *createIcon(long);

		/**
		 * Creates the message text for the dialog.
		 * @param[in] 1st the message string to be shown.
		 * @return wxStaticText
		 */
		wxStaticText *createText(const wxString &);

		/**
		 * Handle events from checkBox
		 * This will write the value of the Checkbox to the
		 * corresponding userOptions.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnDontShowMessageAgain(wxCommandEvent &);
};

#endif	/* _ANMESSAGEDIALOG_H_ */