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

#include "RuleWizardPage.h"

RuleWizardPage::RuleWizardPage(wxWizard *wizard, RuleWizard::wizardPages page)
    : wxWizardPage(wizard)
{
	pageNo_ = page;
}

bool
RuleWizardPage::isNextEnabled(void) const
{
	return (isButtonEnabled(wxID_FORWARD));
}

void
RuleWizardPage::setNextEnabled(bool enabled)
{
	setButtonEnabled(wxID_FORWARD, enabled);
}

wxWizardPage *
RuleWizardPage::GetNext(void) const
{
	RuleWizard::wizardPages  nextPageNo;
	wxWizardPage		*nextPage;
	RuleWizard		*wizard;

	nextPage   = NULL;
	wizard     = dynamic_cast<RuleWizard*>(GetParent());

	if (wizard != NULL) {
		nextPageNo = wizard->forwardTransition(pageNo_);
		nextPage   = wizard->getPage(nextPageNo);
	}

	return (nextPage);
}

bool
RuleWizardPage::isPreviousEnabled(void) const
{
	return (isButtonEnabled(wxID_BACKWARD));
}

void
RuleWizardPage::setPreviousEnabled(bool enabled)
{
	setButtonEnabled(wxID_BACKWARD, enabled);
}

wxWizardPage *
RuleWizardPage::GetPrev(void) const
{
	RuleWizard::wizardPages  previousPageNo;
	wxWizardPage		*previousPage;
	RuleWizard		*wizard;

	previousPage   = NULL;
	wizard = dynamic_cast<RuleWizard*>(GetParent());

	if (wizard != NULL) {
		previousPageNo = wizard->backwardTransition(pageNo_);
		previousPage   = wizard->getPage(previousPageNo);
	}

	return (previousPage);
}

bool
RuleWizardPage::isButtonEnabled(long id) const
{
	wxWizard *wizard = dynamic_cast<RuleWizard*>(GetParent());
	if (wizard == NULL) {
		/* Wrong parent, rarely possible */
		return (false);
	}

	wxWindow *button = wizard->FindWindow(id);
	if (button == NULL) {
		/* No such button */
		return (false);
	}

	return (button->IsEnabled());
}

void
RuleWizardPage::setButtonEnabled(long id, bool enabled)
{
	wxWizard *wizard = dynamic_cast<RuleWizard*>(GetParent());
	if (wizard == NULL) {
		/* Wrong parent, rarely possible */
		return;
	}

	wxWindow *button = wizard->FindWindow(id);
	if (button == NULL) {
		/* No such button */
		return;
	}

	button->Enable(enabled);
}
