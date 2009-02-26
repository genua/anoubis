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

#ifndef _RULEWIZARDPROGRAMPAGE_H_
#define _RULEWIZARDPROGRAMPAGE_H_

#include <wx/wizard.h>

#include "RuleWizardPanelsBase.h"
#include "RuleWizardHistory.h"

/**
 *
 */
class RuleWizardProgramPage : public RuleWizardProgramPageBase
{
	public:
		/**
		 * Constructor of this page.
		 */
		RuleWizardProgramPage(wxWindow *, RuleWizardHistory *);

	private:
		/**
		 * Store the input here.
		 */
		RuleWizardHistory *history_;

		/**
		 * Handle events from wizard.
		 * The page change is in progress we block this
		 * if no program was set.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onPageChanging(wxWizardEvent &);

		/**
		 * Handle events from wizard.
		 * We became the current page. Update view.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onPageChanged(wxWizardEvent &);

		/**
		 * Handle focus events from programTextCtrl (e.g on hit <tab>).
		 * This will write the program to the history.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onProgramTextKillFocus(wxFocusEvent &);

		/**
		 * Handle events from programTextCtrl.
		 * This will write the program to the history.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onProgramTextEnter(wxCommandEvent &);

		/**
		 * Handle events from pickButton.
		 * This will open a file chooser and on 'ok' the choosen
		 * file is written to the history.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onPickButton(wxCommandEvent &);

		/**
		 * Update navigation.
		 * @param None.
		 * @return Nothing.
		 */
		void updateNavi(void);
};

#endif	/* _RULEWIZARDPROGRAMPAGE_H_ */
