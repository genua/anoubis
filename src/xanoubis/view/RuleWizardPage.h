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

#ifndef _RULEWIZARDPAGE_H_
#define _RULEWIZARDPAGE_H_

#include <wx/wizard.h>
#include "RuleWizard.h"

/**
 * Base class for all of our pages.
 */
class RuleWizardPage : public wxWizardPage
{
	public:
		/**
		 * Constructor of this page.
		 */
		RuleWizardPage(wxWizard *, RuleWizard::wizardPages);

		/**
		 * Tests whether the [Next] button is enabled in this page.
		 * @param None.
		 * @return True, if the [Next] button is enabled.
		 */
		bool isNextEnabled(void) const;

		/**
		 * Enables/disables the [Next] button in this page.
		 * If disabled, the [Next] button is still displayed but cannot
		 * be selected.
		 * @param[in] 1st Set tu true, if the button should be enabled.
		 * @return Nothing.
		 */
		void setNextEnabled(bool);

		/**
		 * Get next page.
		 * This will deliver the next page (after next button was hit).
		 * @param None.
		 * @return The next wizard page.
		 */
		virtual wxWizardPage* GetNext(void) const;

		/**
		 * Tests whether the [Previous] button is enabled in this page.
		 * @param None.
		 * @return True, if the [Previous] button is enabled.
		 */
		bool isPreviousEnabled(void) const;

		/**
		 * Enables/disables the [Previous] button in this page.
		 * If disabled, the [Previous] button is still displayed but
		 * cannot be selected.
		 * @param[in] 1st Set tu true, if the button should be enabled.
		 * @return Nothing.
		 */
		void setPreviousEnabled(bool);

		/**
		 * Get previous page.
		 * This will deliver the previous page (after back button
		 * was hit).
		 * @param None.
		 * @return The previous wizard page.
		 */
		virtual wxWizardPage* GetPrev(void) const;

	private:
		/**
		 * Keep our page number here. This is used to determine next
		 * and previous page.
		 */
		RuleWizard::wizardPages pageNo_;

		/**
		 * Tests whether the button with the given id is enabled in
		 * this page.
		 * @param[in] 1st The internal id of the button
		 * @return True, if the button is enabled.
		 */
		bool isButtonEnabled(long) const;

		/**
		 * Enables/disables the button with the given in this page.
		 * If disabled, the button is still displayed but cannot be
		 * selected.
		 * @param[in] 1st The internal id of the button
		 * @param[in] 2nd Set tu true, if the button should be enabled.
		 * @return Nothing.
		 */
		void setButtonEnabled(long, bool);
};

#endif	/* _RULEWIZARDPAGE_H_ */
