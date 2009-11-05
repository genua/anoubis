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

#include <climits>
#include <cstdlib>

#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/string.h>

#include "AnPickFromFs.h"
#include "ContextAppPolicy.h"
#include "DlgRuleEditorAppPage.h"
#include "DlgRuleEditorFilterSubjectPage.h"

DlgRuleEditorAppPage::DlgRuleEditorAppPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorPage(),
    DlgRuleEditorAppPageBase(parent, id, pos, size, style)
{
	binaryIndex_ = 0;
	appPolicy_ = NULL;
	ctxPolicy_ = NULL;
	pageHeader_ = wxT("");
}

DlgRuleEditorAppPage::~DlgRuleEditorAppPage(void)
{
}

void
DlgRuleEditorAppPage::update(Subject *subject)
{
	if (subject == appPolicy_ || subject == ctxPolicy_)
		setBinary();

	if (subject && subject->IsKindOf(CLASSINFO(ContextAppPolicy))) {
		setDisableSFS();
	}
}

void
DlgRuleEditorAppPage::select(Policy *policy)
{
	if (policy->IsKindOf(CLASSINFO(AppPolicy))) {
		appPolicy_ = wxDynamicCast(policy, AppPolicy);
		DlgRuleEditorPage::select(policy);
		if (appPolicy_->getTypeID() == APN_SFS) {
			enable_ = false;
		}
		subjPage->select(policy);
		Enable(enable_);
	}
	if (policy->IsKindOf(CLASSINFO(ContextFilterPolicy))) {
		ctxPolicy_ = wxDynamicCast(policy, ContextFilterPolicy);
		DlgRuleEditorPage::select(policy);
		subjPage->select(policy);
		Enable(enable_);
	}

	/*
	 * The "Disable SFS" checkbox is only displayed for
	 * Context applications
	 */
	noSfsCheckbox->Show(policy &&
	    policy->IsKindOf(CLASSINFO(ContextAppPolicy)));
}

void
DlgRuleEditorAppPage::deselect(void)
{
	appPolicy_ = NULL;
	ctxPolicy_ = NULL;
	DlgRuleEditorPage::deselect();
}

void
DlgRuleEditorAppPage::setBinaryIndex(unsigned int index)
{
	binaryIndex_ = index;
	subjPage->setBinaryIndex(index);
}

void
DlgRuleEditorAppPage::onNoSfsClicked(wxCommandEvent &event)
{
	if (appPolicy_ != 0 &&
	    appPolicy_->IsKindOf(CLASSINFO(ContextAppPolicy))) {
		appPolicy_->setFlag(APN_RULE_NOSFS, event.IsChecked());
	}

	event.Skip();
}

void
DlgRuleEditorAppPage::setBinary(void)
{
	int		 selection;
	wxString	 current = wxT("any");
	wxFileName	 baseName;
	wxNotebook	*parentNotebook = NULL;
	wxWindow	*win = GetParent();

	if (appPolicy_ != NULL) {
		if (!appPolicy_->isAnyBlock()) {
			current = appPolicy_->getBinaryName(binaryIndex_);
		}
	}
	if (ctxPolicy_ != NULL) {
		if (!ctxPolicy_->isAny()) {
			current = ctxPolicy_->getBinaryName(binaryIndex_);
		}
	}
	if (current.Cmp(pageHeader_) == 0)
		return;

	while(win) {
		parentNotebook = wxDynamicCast(win, wxNotebook);
		if (parentNotebook)
			break;
		win = win->GetParent();
	}
	if (parentNotebook != NULL) {
		selection = parentNotebook->GetSelection();
		if (selection == (int)binaryIndex_) {
			baseName.Assign(current);
			parentNotebook->SetPageText(selection,
			    baseName.GetFullName());
			pageHeader_ = current;
		}
	}
}

void
DlgRuleEditorAppPage::setDisableSFS(void)
{
	noSfsCheckbox->SetValue(appPolicy_->getFlag(APN_RULE_NOSFS));
}
