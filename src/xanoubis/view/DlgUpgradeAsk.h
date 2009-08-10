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

#ifndef _DLGUPGRADEASK_H_
#define _DLGUPGRADEASK_H_

#include <wx/progdlg.h>
#include <wx/config.h>

#include "DlgUpgradeAskBase.h"
#include "main.h"


/**
 * This is the upgrade-messagebox.
 * This is shown if there has been an upgrade of the system
 * and there are signed files.
 */
class DlgUpgradeAsk : public DlgUpgradeAskBase
{
	public:
		/*
		 * Constructor of DlgUpgradeAsk
		 * @param[in] 1st The parent window
		 */
		DlgUpgradeAsk(wxWindow *);

		/*
		 * Destructor of DlgUpgradeAsk
		 * @param None.
		 */
		~DlgUpgradeAsk(void);

		/**
		 * This is called when the button 'Close' is clicked.
		 * The messagebox is going to be closed.
		 * @param[in] 1st The event of the button
		 * @return Nothing.
		 */
		virtual void close(wxCommandEvent &);

		/**
		 * This is called when the button
		 * 'Open SFS Browser now' is clicked.
		 * The SFS Browser will be started
		 * @param[in] 1st The event of the button
		 * @return Nothing.
		 * */
		virtual void openSFS(wxCommandEvent &);

		/**
		 * This is called when the checkbox 'cb_showAgain' is modified
		 * @param[in] 1st The event of the checkbox
		 * @return Nothing.
		 * */
		virtual void onUpgradeNotifyCheck(wxCommandEvent &);

	private:
		bool		ShowUpgradeMessage_;
		wxConfig	*userOptions_;
		bool		upgradeMessage_;
};

#endif	/* _DLGUPGRADEASK_H_ */
