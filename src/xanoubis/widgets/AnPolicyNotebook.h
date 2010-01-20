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

#ifndef _ANPOLICYNOTEBOOK_H_
#define _ANPOLICYNOTEBOOK_H_

#include <wx/notebook.h>

#include "Policy.h"
#include "AppPolicy.h"
#include "AlfFilterPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "DefaultFilterPolicy.h"
#include "SfsFilterPolicy.h"
#include "SfsDefaultFilterPolicy.h"
#include "ContextFilterPolicy.h"
#include "SbAccessFilterPolicy.h"

/**
 * For the RuleEditor we needed our own notebook to ensure correct creation
 * and removal of the notebook pages (aka tabs) showing the details of
 * an policy.
 */
class AnPolicyNotebook : public wxNotebook
{
	public:
		/**
		 * Constructor of AnPolicyNotebook.
		 * It has to have the same signature as a ordinary wxNotebook,
		 * so wxformbuilder can just exchange wxPanel with this class.
		 */
		AnPolicyNotebook(wxWindow* parent, wxWindowID id,
		    const wxPoint& pos = wxDefaultPosition,
		    const wxSize& size = wxDefaultSize, long style = 0,
		    const wxString& name = wxNotebookNameStr);

		/**
		 * Destructor of AnPolicyNotebook.
		 */
		~AnPolicyNotebook(void);

		/**
		 * Select policy.
		 * This should been called on selection of a policy. The
		 * given policy will be shown by different tabs. These
		 * tabs are created on demand.
		 * @param[in] 1st The selected policy.
		 * @return Nothing.
		 */
		void select(Policy *);

		/**
		 * Deselect policy.
		 * The previous selected policy is not longer selected.
		 * Thus all pages are deleted.
		 * @param None.
		 * @return Nothing.
		 */
		void deselect(void);

		/**
		 * Reset tab selection.
		 * Selects the first tab.
		 * @param None.
		 * @return Nothing.
		 */
		void resetTabSelection(void);

	private:
		/**
		 * This holds the policy been shown by our pages.
		 */
		AppPolicy *appPolicy_;

		/**
		 * This holds the policy been shown by our pages.
		 */
		ContextFilterPolicy *ctxPolicy_;

		/**
		 * This holds the last tab selected.
		 */
		unsigned int selectedTab_;

		/**
		 * Handle events from add button.
		 * This will handle the events of the add button at the
		 * DlgRuleEditorAppPage. This will create a new page
		 * for a new binary '(new)'.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onAddButton(wxCommandEvent &);

		/**
		 * Handle events from delete button.
		 * This will handle the events of the delete button at the
		 * DlgRuleEditorAppPage.\n
		 * The index of the binary to be removed is transported by
		 * this event within the integer field. The given binary
		 * is removed and all pages are deleted and those for the
		 * remaining binaries are re-created.
		 * @param[in] 1st The event. Use GetInt() for the binary index.
		 * @return Nothing.
		 */
		void onDeleteButton(wxCommandEvent &);

		/**
		 * Select application policy.
		 * This will create all nessesary pages to show an application
		 * policy.
		 * @param[in] 1st The policy to show.
		 * @return Nothing.
		 */
		void selectAppPolicy(AppPolicy *);

		/**
		 * Select alf filter policy.
		 * This will create all nessesary pages and show them.
		 * @param[in] 1st The policy to show.
		 * @return Nothing.
		 */
		void selectAlfFilterPolicy(Policy *);

		/**
		 * Select alf capabiliyt filter policy.
		 * This will create all nessesary pages and show them.
		 * @param[in] 1st The policy to show.
		 * @return Nothing.
		 */
		void selectAlfCapabilityFilterPolicy(Policy *);

		/**
		 * Select default filter policy.
		 * This will create all nessesary pages and show them.
		 * @param[in] 1st The policy to show.
		 * @return Nothing.
		 */
		void selectDefaultFilterPolicy(Policy *);

		/**
		 * Select sfs filter policy.
		 * This will create all nessesary pages and show them.
		 * @param[in] 1st The policy to show.
		 * @return Nothing.
		 */
		void selectSfsFilterPolicy(Policy *);

		/**
		 * Select sfs default filter policy.
		 * This will create all nessesary pages and show them.
		 * @param[in] 1st The policy to show.
		 * @return Nothing.
		 */
		void selectSfsDefaultFilterPolicy(Policy *);

		/**
		 * Select context filter policy.
		 * This will create all nessesary pages and show them.
		 * @param[in] 1st The policy to show.
		 * @return Nothing.
		 */
		void selectContextFilterPolicy(Policy *);

		/**
		 * Select sb access filter policy.
		 * This will create all nessesary pages and show them.
		 * @param[in] 1st The policy to show.
		 * @return Nothing.
		 */
		void selectSbAccessFilterPolicy(Policy *);
};

#endif	/* _ANPOLICYNOTEBOOK_H_ */
