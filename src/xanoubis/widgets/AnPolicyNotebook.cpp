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

#include <wx/filename.h>

#include "AnPolicyNotebook.h"

#include "AppPolicy.h"
#include "DlgRuleEditorAppPage.h"
#include "DlgRuleEditorPage.h"

AnPolicyNotebook::AnPolicyNotebook(wxWindow* parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxNotebook(parent, id, pos, size, style, name)
{
	appPolicy_ = NULL;

	Connect(ID_APP_PAGE_ADD, wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(AnPolicyNotebook::onAddButton), NULL, this);
	Connect(ID_APP_PAGE_DELETE, wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(AnPolicyNotebook::onDeleteButton),
	    NULL, this);
}

AnPolicyNotebook::~AnPolicyNotebook(void)
{
	Disconnect(ID_APP_PAGE_ADD, wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(AnPolicyNotebook::onAddButton), NULL, this);
	Disconnect(ID_APP_PAGE_DELETE, wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(AnPolicyNotebook::onDeleteButton),
	    NULL, this);
}

void
AnPolicyNotebook::select(Policy *policy)
{
	if (policy->IsKindOf(CLASSINFO(AppPolicy))) {
		appPolicy_ = wxDynamicCast(policy, AppPolicy);
		selectAppPolicy(appPolicy_);
	}
}

void
AnPolicyNotebook::deselect(void)
{
	appPolicy_ = NULL;
	DeleteAllPages();
}

void
AnPolicyNotebook::onAddButton(wxCommandEvent &)
{
	DlgRuleEditorAppPage *page;

	if (appPolicy_ != NULL) {
		appPolicy_->addBinary(_("(new)"));

		/* Create page. */
		page = new DlgRuleEditorAppPage(this);
		page->setBinaryIndex(appPolicy_->getBinaryCount() - 1);
		page->select(appPolicy_);
		AddPage(page, _("(new)"), true);
	}
}

void
AnPolicyNotebook::onDeleteButton(wxCommandEvent &event)
{
	if (appPolicy_ != NULL) {
		appPolicy_->removeBinary(event.GetInt());
		/*
		 * Deleteing just the single page would lead to inconsistency
		 * of page number and binary index. Thus we have to re-create
		 * all pages.
		 */
		DeleteAllPages();
		select(appPolicy_);
	}
}

void
AnPolicyNotebook::selectAppPolicy(AppPolicy *policy)
{
	int			 index;
	wxString		 pageName;
	wxFileName		 baseName;
	DlgRuleEditorAppPage	*page;

	if (policy == NULL) {
		return;
	}

	index = policy->getBinaryCount() - 1;
	do {
		/* Adjust index for case 'any'. */
		if (index < 0) {
			index = 0;
		}

		/* Assemble page name (aka text on tab). */
		baseName.Assign(policy->getBinaryName(index));
		pageName = baseName.GetFullName();
		if (pageName.IsEmpty()) {
			pageName = _("(new)");
		}

		/* Create page. */
		page = new DlgRuleEditorAppPage(this);
		page->setBinaryIndex(index);
		page->select(policy);
		InsertPage(0, page, pageName);

		index--;
	} while (index >= 0);
}
