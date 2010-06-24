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

	protected:
		/**
		 * Handles selection events from the "Disable SFS" checkbox.
		 * Updates the model.
		 * @param[in] 1st The event.
		 */
		void onNoSfsClicked(wxCommandEvent &);

		/**
		 * Handles selection events from the "Run in playground"
		 * checkbox.
		 * Updates the model.
		 * @param[in] 1st The event.
		 */
		void onPlaygroundClicked(wxCommandEvent &);

	private:
		/**
		 * This page is responsible for the binary with this index.
		 */
		unsigned int binaryIndex_;

		/**
		 * Store the application policy of this page.
		 */
		AppPolicy *appPolicy_;

		/**
		 * Store the context filter policy of this page.
		 */
		ContextFilterPolicy *ctxPolicy_;

		/**
		 * Binary name that is currently displayed in the page header.
		 */
		wxString pageHeader_;

		/**
		 * Write the binary to page header.
		 * @param None.
		 * @return Nothing.
		 */
		void setBinary(void);

		/**
		 * Puts the value of the nosfs-Flag into the checkbox.
		 */
		void setDisableSFS(void);

		/**
		 * Puts the value of the playerground-flag into the checkbox.
		 */
		void setRunInPlayground(void);
};

#endif	/* _DLGRULEEDITORAPPPAGE_H_ */
