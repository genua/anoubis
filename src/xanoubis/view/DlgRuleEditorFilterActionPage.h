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

#ifndef _DLGRULEEDITORFILTERACTIONPAGE_H_
#define _DLGRULEEDITORFILTERACTIONPAGE_H_

#include "DlgRuleEditorBase.h"
#include "DlgRuleEditorFilterPage.h"

/**
 * This is the filter action page.
 *
 * This is a derrived class from DlgRuleEditorFilterPage to inherrit the
 * mechanims of selection and deselection and observing the policy. In
 * addition we derrived from DlgRuleEditorFilterActionPageBase to gain
 * access to the widgets and implement the event methods of them.
 */
class DlgRuleEditorFilterActionPage : public DlgRuleEditorFilterPage,
    public DlgRuleEditorFilterActionPageBase
{
	public:
		/**
		 * Constructor of the filter action page.
		 * It has to have the same signature as a ordinary wxPanel,
		 * so wxformbuilder can just exchange wxPanel with this class.
		 */
		DlgRuleEditorFilterActionPage(wxWindow *parent,
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
		 * @param[in] 1st The selected filter policy.
		 * @return Nothing.
		 */
		virtual void select(FilterPolicy *);

		/**
		 * Deselect this page.
		 * This will runn the base class method and hide this page.
		 * @param None.
		 * @return Noting.
		 */
		virtual void deselect(void);

	private:
		/**
		 * Clean the widgets.
		 * Use this to wipe the values from the widgets.
		 * @param None.
		 * @return Nothing.
		 * @see showAction()
		 * @see showLog()
		 */
		virtual void clear(void);

		/**
		 * Update action widgets.
		 * This will enable all widgets related to 'action'.
		 * The given action value is shown by the widgets.
		 * For invalid action values the widgets are disabled.
		 * @param[in] 1st The apn action value.
		 * @return Nothing.
		 */
		void showAction(int);

		/**
		 * Update log widgets.
		 * This will enable all widgets related to 'log'.
		 * The given log value is shown by the widgets.
		 * For invalid log values the widgets are disabled.
		 * @param[in] 1st The apn log value.
		 * @return Nothing.
		 */
		void showLog(int);

		/**
		 * Handle events from allowRadioButton.
		 * This will set the APN_ACTION_ALLOW to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAllowRadioButton(wxCommandEvent &);

		/**
		 * Handle events from denyRadioButton.
		 * This will set the APN_ACTION_DENY to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onDenyRadioButton(wxCommandEvent &);

		/**
		 * Handle events from askRadioButton.
		 * This will set the APN_ACTION_ASK to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAskRadioButton(wxCommandEvent &);

		/**
		 * Handle events from noneRadioButton.
		 * This will set the APN_LOG_NONE to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onNoneRadioButton(wxCommandEvent &);

		/**
		 * Handle events from normalRadioButton.
		 * This will set the APN_LOG_NORMAL to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onNormalRadioButton(wxCommandEvent &);

		/**
		 * Handle events from alertRadioButton.
		 * This will set the APN_LOG_ALERT to the policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAlertRadioButton(wxCommandEvent &);
};

#endif	/* _DLGRULEEDITORFILTERACTIONPAGE_H_ */
