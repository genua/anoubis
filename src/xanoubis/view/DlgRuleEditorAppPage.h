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

#ifndef _DLGRULEEDITORAPPPAGE_H_
#define _DLGRULEEDITORAPPPAGE_H_

#include <wx/string.h>

#include "DlgRuleEditorBase.h"
#include "DlgRuleEditorPage.h"
#include "AppPolicy.h"
#include "ContextFilterPolicy.h"
#include "CsumCalcTask.h"
#include "JobCtrl.h"

/**
 * This is the application page.
 *
 * This page is responsible for all AppPolicies.
 *
 * This is a derrived class from DlgRuleEditorFilterPage to inherrit the
 * mechanims of selection and deselection and observing the policy. In
 * addition we derrived from DlgRuleEditorAppPageBase to gain
 * access to the widgets and implement the event methods of them.
 *
 * The event handling method for add button on this page are
 * implemented in the AnPolicyNotebook. This is needed to trigger
 * save creation of new pages.\n
 * To savely remove this page the main work is also done at AnPolicyNotebook.
 * To get this done propperly the binary index is send with the same event.
 */
class DlgRuleEditorAppPage : public DlgRuleEditorPage,
    public DlgRuleEditorAppPageBase
{
	public:
		/**
		 * Constructor of the application page.
		 * It has to have the same signature as a ordinary wxPanel,
		 * so wxformbuilder can just exchange wxPanel with this class.
		 */
		DlgRuleEditorAppPage(wxWindow *parent,
		    wxWindowID id = wxID_ANY,
		    const wxPoint& pos = wxDefaultPosition,
		    const wxSize& size = wxDefaultSize,
		    long style = wxTAB_TRAVERSAL);

		/**
		 * Destructor of the application page.
		 */
		~DlgRuleEditorAppPage(void);

		/**
		 * Update the widgets.
		 * This is called whenever the assigned policy was modified
		 * or during selection to fill all widgets with the appropriate
		 * value.
		 * @param[in] 1st The policy been observed and modified.
		 * @return Nothing.
		 */
		virtual void update(Subject *);

		/**
		 * Select this page.
		 * This will check for the type of policy. If the policy is
		 * one of the types this page is interrested in the
		 * base class method will been called and this page is shown.
		 * @param[in] 1st The selected filter policy.
		 * @return Nothing.
		 */
		virtual void select(Policy *);

		/**
		 * Deselect this page.
		 * This will runn the base class method and hide this page.
		 * @param None.
		 * @return Noting.
		 */
		virtual void deselect(void);

		/**
		 * Set binary index.
		 * This page is responsible for handling the binary
		 * with the given index.
		 * @param[in] 1st Bianry index.
		 * @return Nothing.
		 */
		void setBinaryIndex(unsigned int);

	private:
		/**
		 * This page is responsible for the binary with this index.
		 */
		unsigned int binaryIndex_;

		/**
		 * In case of new binary to automatic validate/update.
		 */
		bool automaticOnNew_;

		/**
		 * Cache the checksum of this binary.
		 */
		wxString csumCache_;

		/**
		 * The task to calculate the checksum.
		 */
		CsumCalcTask calcTask_;

		/**
		 * Store the application policy of this page.
		 */
		AppPolicy *appPolicy_;

		/**
		 * Store the context filter policy of this page.
		 */
		ContextFilterPolicy *ctxPolicy_;

		/**
		 * Fill the widgets showing the binary.
		 * @param None.
		 * @return Nothing.
		 */
		void showBinary(void);

		/**
		 * Fill the widgets showing the checksum.
		 * @param None.
		 * @return Nothing.
		 */
		void showCsum(void);

		/**
		 * Update the widgets of the status.
		 * @param None.
		 * @return Nothing.
		 */
		void showStatus(void);

		/**
		 * Write the binary to the policy.
		 * @param[in] 1st The new binary.
		 * @return Nothind.
		 */
		void setBinary(wxString);

		/**
		 * Start calculation of current csum.
		 * This will start the calculation of the current checksum.
		 * The result is cached in csumCache_.
		 * @param None.
		 * @return Nothing.
		 */
		void startCalculation(void);

		/**
		 * Update the csum of the policy.
		 * This will write the cached current checksum from csumCache_
		 * to the policy.
		 * @param None.
		 * @return Nothing.
		 */
		void doCsumUpdate();

		/**
		 * Handle focus events from binaryTextCtrl (e.g on hit <tab>).
		 * This will write the binary to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onBinaryTextKillFocus(wxFocusEvent &);

		/**
		 * Handle events from binaryTextCtrl.
		 * This will write the binary to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onBinaryTextEnter(wxCommandEvent &);

		/**
		 * Handle events from pickButton.
		 * This will open a file chooser and on 'ok' the choosen
		 * file is written to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onPickButton(wxCommandEvent &);

		/**
		 * Handle events from validateButton.
		 * This will start the calculation of the current checksum.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onValidateButton(wxCommandEvent &);

		/**
		 * Handle events from updateButton.
		 * This will write the cached current checksum from csumCache_
		 * to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onUpdateButton(wxCommandEvent &);

		/**
		 * Handle events from deleteButton.
		 * This will catch the click-event and add the binary index.
		 * Slipping the event is mandatory, thus the notebook also
		 * has to receive it.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onDeleteButton(wxCommandEvent &);

		/**
		 * Handle events from checksum calculation task.
		 * This will extract the result of the calculation from
		 * the task and store it in csumCache_.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onCsumCalcTask(TaskEvent &);

		/**
		 * Set the text of the Info label. An empty text hides
		 * the label.
		 * @param[in] 1st The new text.
		 */
		void showInfo(const wxString &);
};

#endif	/* _DLGRULEEDITORAPPPAGE_H_ */
