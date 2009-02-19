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

#ifndef _DLGRULEEDITORFILTERCONTEXTPAGE_H_
#define _DLGRULEEDITORFILTERCONTEXTPAGE_H_

#include "DlgRuleEditorBase.h"
#include "DlgRuleEditorPage.h"
#include "FilterPolicy.h"
#include "ContextFilterPolicy.h"

/**
 * This is the filter context page.
 *
 * This page is responsible only for ContextFilterPolicies.
 *
 * This is a derrived class from DlgRuleEditorPage to inherrit the
 * mechanims of selection and deselection and observing the policy. In
 * addition we derrived from DlgRuleEditorFilterContextPageBase to gain
 * access to the widgets and implement the event methods of them.
 */
class DlgRuleEditorFilterContextPage : public DlgRuleEditorPage,
    public DlgRuleEditorFilterContextPageBase
{
	public:
		/**
		 * Constructor of the filter context page.
		 * It has to have the same signature as a ordinary wxPanel,
		 * so wxformbuilder can just exchange wxPanel with this class.
		 */
		DlgRuleEditorFilterContextPage(wxWindow *parent,
		    wxWindowID id = wxID_ANY,
		    const wxPoint& pos = wxDefaultPosition,
		    const wxSize& size = wxDefaultSize,
		    long style = wxTAB_TRAVERSAL);

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
		 * @param[in] 1st The selected policy.
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
		 * This holds the policy been edited by this page.
		 */
		ContextFilterPolicy *filterPolicy_;

		/**
		 * Update direction widgets.
		 * @param None.
		 * @return Nothing.
		 */
		void showContextType(void);

		/**
		 * Handle events from newRadioButton.
		 * This will set the APN_CTX_NEW to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onNewRadioButton(wxCommandEvent &);

		/**
		 * Handle events from openRadioButton.
		 * This will set the APN_CTX_OPEN to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onOpenRadioButton(wxCommandEvent &);
};

#endif	/* _DLGRULEEDITORFILTERCONTEXTPAGE_H_ */
