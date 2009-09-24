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
#include "DlgRuleEditorFilterActionPage.h"
#include "DlgRuleEditorFilterNetworkPage.h"
#include "DlgRuleEditorFilterAddressPage.h"
#include "DlgRuleEditorFilterCapabilityPage.h"
#include "DlgRuleEditorFilterSfsPage.h"
#include "DlgRuleEditorFilterSubjectPage.h"
#include "DlgRuleEditorFilterPermissionPage.h"
#include "DlgRuleEditorFilterContextPage.h"

AnPolicyNotebook::AnPolicyNotebook(wxWindow* parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxNotebook(parent, id, pos, size, style, name)
{
	appPolicy_ = NULL;
	ctxPolicy_ = NULL;

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
	} else if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		selectAlfFilterPolicy(policy);
	} else if (policy->IsKindOf(CLASSINFO(AlfCapabilityFilterPolicy))) {
		selectAlfCapabilityFilterPolicy(policy);
	} else if (policy->IsKindOf(CLASSINFO(DefaultFilterPolicy))) {
		selectDefaultFilterPolicy(policy);
	} else if (policy->IsKindOf(CLASSINFO(SfsFilterPolicy))) {
		selectSfsFilterPolicy(policy);
	} else if (policy->IsKindOf(CLASSINFO(SfsDefaultFilterPolicy))) {
		selectSfsDefaultFilterPolicy(policy);
	} else if (policy->IsKindOf(CLASSINFO(ContextFilterPolicy))) {
		ctxPolicy_ = wxDynamicCast(policy, ContextFilterPolicy);
		selectContextFilterPolicy(policy);
	} else if (policy->IsKindOf(CLASSINFO(SbAccessFilterPolicy))) {
		selectSbAccessFilterPolicy(policy);
	} else {
		/* Unknown policy type - nothing to do */
	}
}

void
AnPolicyNotebook::deselect(void)
{
	appPolicy_ = NULL;
	ctxPolicy_ = NULL;
	DeleteAllPages();
}

void
AnPolicyNotebook::onAddButton(wxCommandEvent &)
{
	bool wasAny;
	int  newIndex;

	DlgRuleEditorAppPage		*appPage;
	DlgRuleEditorFilterContextPage	*ctxPage;

	if (appPolicy_ != NULL) {
		wasAny = appPolicy_->isAnyBlock();
		appPolicy_->addBinary(_("(new)"));
		newIndex = appPolicy_->getBinaryCount() - 1;

		/* Create page. */
		if (!wasAny) {
			appPage = new DlgRuleEditorAppPage(this);
			appPage->setBinaryIndex(newIndex);
			appPage->select(appPolicy_);
			AddPage(appPage, _("(new)"), true);
		} else {
			SetPageText(GetSelection(), _("(new)"));
		}
	}
	if (ctxPolicy_ != NULL) {
		wasAny = ctxPolicy_->isAny();
		ctxPolicy_->addBinary(_("(new)"));
		newIndex = ctxPolicy_->getBinaryCount() - 1;

		/* Create page. */
		if (!wasAny) {
			ctxPage = new DlgRuleEditorFilterContextPage(this);
			ctxPage->setBinaryIndex(newIndex);
			ctxPage->select(ctxPolicy_);
			AddPage(ctxPage, _("(new)"), true);
		} else {
			SetPageText(GetSelection(), _("(new)"));
		}
	}
}

void
AnPolicyNotebook::onDeleteButton(wxCommandEvent &)
{
	int	idx = GetSelection();
	if (idx < 0)
		return;
	if (appPolicy_ != NULL) {
		appPolicy_->removeBinary(idx);
		if (idx && idx >= (int)appPolicy_->getBinaryCount())
			idx--;
		/*
		 * Deleteing just the single page would lead to inconsistency
		 * of page number and binary index. Thus we have to re-create
		 * all pages.
		 */
		DeleteAllPages();
		select(appPolicy_);
		ChangeSelection(idx);
	}
	if (ctxPolicy_ != NULL) {
		ctxPolicy_->removeBinary(idx);
		if (idx && idx >= (int)ctxPolicy_->getBinaryCount())
			idx--;
		/*
		 * Deleteing just the single page would lead to inconsistency
		 * of page number and binary index. Thus we have to re-create
		 * all pages.
		 */
		DeleteAllPages();
		select(ctxPolicy_);
		ChangeSelection(idx);
	}
}

void
AnPolicyNotebook::selectAppPolicy(AppPolicy *policy)
{
	unsigned int		 index;
	wxString		 pageName;
	wxFileName		 baseName;
	DlgRuleEditorAppPage	*page;

	if (policy == NULL) {
		return;
	}

	/* Always iterate at least once for index 0 to handle "any". */
	for (index=0; !index || index < policy->getBinaryCount(); ++index) {
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
		AddPage(page, pageName);
	}
}

void
AnPolicyNotebook::selectAlfFilterPolicy(Policy *policy)
{
	DlgRuleEditorFilterActionPage	*actionPage;
	DlgRuleEditorFilterNetworkPage	*networkPage;
	DlgRuleEditorFilterAddressPage	*addressPage;

	actionPage = new DlgRuleEditorFilterActionPage(this);
	actionPage->select(policy);
	AddPage(actionPage, _("Action / Log"));

	networkPage = new DlgRuleEditorFilterNetworkPage(this);
	networkPage->select(policy);
	AddPage(networkPage, _("Network"));

	addressPage = new DlgRuleEditorFilterAddressPage(this);
	addressPage->select(policy);
	AddPage(addressPage, _("Address"));
}

void
AnPolicyNotebook::selectAlfCapabilityFilterPolicy(Policy *policy)
{
	DlgRuleEditorFilterActionPage		*actionPage;
	DlgRuleEditorFilterCapabilityPage	*capPage;

	actionPage = new DlgRuleEditorFilterActionPage(this);
	actionPage->select(policy);
	AddPage(actionPage, _("Action / Log"));

	capPage = new DlgRuleEditorFilterCapabilityPage(this);
	capPage->select(policy);
	AddPage(capPage, _("Capability"));
}

void
AnPolicyNotebook::selectDefaultFilterPolicy(Policy *policy)
{
	DlgRuleEditorFilterActionPage *page;

	page = new DlgRuleEditorFilterActionPage(this);
	page->select(policy);
	AddPage(page, _("Action / Log"));
}

void
AnPolicyNotebook::selectSfsFilterPolicy(Policy *policy)
{
	DlgRuleEditorFilterSfsPage	*sfsPage;
	DlgRuleEditorFilterSubjectPage	*subjectPage;

	sfsPage = new DlgRuleEditorFilterSfsPage(this);
	sfsPage->select(policy);
	AddPage(sfsPage, _("Valid / Invalid / Unknown"));

	subjectPage = new DlgRuleEditorFilterSubjectPage(this);
	subjectPage->select(policy);
	AddPage(subjectPage, _("Subject"));
}

void
AnPolicyNotebook::selectSfsDefaultFilterPolicy(Policy *policy)
{
	DlgRuleEditorFilterActionPage *page;

	page = new DlgRuleEditorFilterActionPage(this);
	page->select(policy);
	AddPage(page, _("Action / Log"));
}

void
AnPolicyNotebook::selectContextFilterPolicy(Policy *policy)
{
	unsigned int	 idx;
	wxString	 pageName;
	wxFileName	 baseName;

	ContextFilterPolicy		*filterPolicy;
	DlgRuleEditorFilterContextPage	*page;

	if (policy == NULL) {
		return;
	}

	filterPolicy = wxDynamicCast(policy, ContextFilterPolicy);
	if (filterPolicy == NULL) {
		return;
	}

	/* Always iterate at least once to handle the "any" case. */
	for (idx=0; !idx || idx < filterPolicy->getBinaryCount(); ++idx) {
		/* Assemble page name (aka text on tab). */
		baseName.Assign(filterPolicy->getBinaryName(idx));
		pageName = baseName.GetFullName();
		if (pageName.IsEmpty()) {
			pageName = _("(new)");
		}

		/* Create page. */
		page = new DlgRuleEditorFilterContextPage(this);
		page->setBinaryIndex(idx);
		page->select(filterPolicy);
		AddPage(page, pageName);
	}
}

void
AnPolicyNotebook::selectSbAccessFilterPolicy(Policy *policy)
{
	DlgRuleEditorFilterActionPage	  *actionPage;
	DlgRuleEditorFilterSubjectPage	  *subjectPage;
	DlgRuleEditorFilterPermissionPage *permissionPage;

	actionPage = new DlgRuleEditorFilterActionPage(this);
	actionPage->select(policy);
	AddPage(actionPage, _("Action / Log"));

	subjectPage = new DlgRuleEditorFilterSubjectPage(this);
	subjectPage->select(policy);
	AddPage(subjectPage, _("Subject"));

	permissionPage = new DlgRuleEditorFilterPermissionPage(this);
	permissionPage->select(policy);
	AddPage(permissionPage, _("Permissions"));
}
