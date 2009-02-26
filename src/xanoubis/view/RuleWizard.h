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

#ifndef _RULEWIZARD_H_
#define _RULEWIZARD_H_

#include <wx/wizard.h>

#include "RuleWizardHistory.h"
#include "PolicyRuleSet.h"

/**
 * This is the main wizard class.
 */
class RuleWizard : public wxWizard
{
	public:
		/**
		 * The wizard knows about these pages. The pages are organized
		 * as array and these are the used indices.
		 */
		enum wizardPages {
			PAGE_PROGRAM = 0,
			PAGE_CTX,
			PAGE_CTX_EXCEPT,
			PAGE_ALF_KEEP_POLICY,
			PAGE_ALF_CLIENT,
			PAGE_ALF_CLIENT_PORTS,
			PAGE_FINAL,
			PAGE_EOL
		};

		/**
		 * Constructor of Wizard.
		 * @param None.
		 */
		RuleWizard(void);

		/**
		 * Get a special page.
		 * This will deliver the specified page if the given index
		 * meets the need of wizardPages range.
		 * @param[in] 1st Page index.
		 * @return Requested page or NULL in case of error.
		 */
		wxWizardPage *getPage(enum wizardPages);

		/**
		 * Get next page index.
		 * Based on the current page (given index) and the input
		 * already made, this will decide about the next page and
		 * return it's index.
		 * @param[in] 1st Index of the current page.
		 * @return Index of the next page.
		 */
		RuleWizard::wizardPages forwardTransition(enum wizardPages)
		    const;

		/**
		 * Get previous page index.
		 * Based on the current page (given index) and the input
		 * already made, this will decide about the previous page and
		 * return it's index.
		 * @param[in] 1st Index of the current page.
		 * @return Index of the previous page.
		 */
		RuleWizard::wizardPages backwardTransition(enum wizardPages)
		    const;

	private:
		/**
		 * Store made input within this class.
		 */
		RuleWizardHistory history_;

		/**
		 * The array of pages.
		 */
		wxWizardPage *pages_[PAGE_EOL];

		/**
		 * Handle event of finished wizard.
		 * With this we start creating the (new) policies.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onWizardFinished(wxWizardEvent &);

		/**
		 * Helper method to create context policies.
		 * @param[in] 1st The ruleset of policies.
		 * @return Nothind.
		 */
		void createContextPolicy(PolicyRuleSet *) const;

		/**
		 * Helper method to create alf policies.
		 * @param[in] 1st The ruleset of policies.
		 * @return Nothind.
		 */
		void createAlfPolicy(PolicyRuleSet *) const;
};

#endif	/* _RULEWIZARD_H_ */
