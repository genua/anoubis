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

#include <wx/stattext.h>
#include <wx/filename.h>

#include "RuleWizardHistory.h"
#include "AppPolicy.h"
#include "ProfileCtrl.h"
#include "PolicyRuleSet.h"

RuleWizardHistory::RuleWizardHistory(void)
{
	normalSectionFont_ = wxFont(12, wxFONTFAMILY_DEFAULT,
	    wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString);
	activeSectionFont_ = wxFont(12, wxFONTFAMILY_DEFAULT,
	    wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString);

	program_ = wxEmptyString;

	ctxHavePolicy_ = false;
	ctxSame_ = true;
	ctxException_ = false;
	ctxExceptionList_.Clear();

	alfHavePolicy_ = false;
	alfKeepPolicy_ = true;
	alfClientPermission_ = ALF_CLIENT_DENY_ALL;
}

void
RuleWizardHistory::setProgram(const wxString &program)
{
	AppPolicy	*app;
	ProfileCtrl	*profileCtrl;
	PolicyRuleSet	*ruleSet;

	program_ = program;

	/* Search for this program. Does it have alf policies? */
	profileCtrl = ProfileCtrl::getInstance();
	ruleSet = profileCtrl->getRuleSet(profileCtrl->getUserId());
	if (ruleSet == NULL) {
		ctxHavePolicy_ = false;
		alfHavePolicy_ = false;
		return;
	}

	ruleSet->lock();

	app = ruleSet->searchContextAppPolicy(program);
	if (app != NULL) {
		ctxHavePolicy_ = true;
	} else {
		ctxHavePolicy_ = false;
	}

	app = ruleSet->searchAlfAppPolicy(program);
	if (app != NULL) {
		alfHavePolicy_ = true;
	} else {
		alfHavePolicy_ = false;
	}

	ruleSet->unlock();
}

wxString
RuleWizardHistory::getProgram(void) const
{
	return (program_);
}

bool
RuleWizardHistory::getCtxHavePolicy(void) const
{
	return (ctxHavePolicy_);
}

void
RuleWizardHistory::setCtxSame(bool flag)
{
	ctxSame_ = flag;
}

bool
RuleWizardHistory::getCtxSame(void) const
{
	return (ctxSame_);
}

void
RuleWizardHistory::setCtxException(bool flag)
{
	ctxException_ = flag;
	if (!ctxException_) {
		ctxExceptionList_.Clear();
	}
}

bool
RuleWizardHistory::getCtxException(void) const
{
	return (ctxException_);
}

void
RuleWizardHistory::setCtxExceptionList(const wxArrayString & list)
{
	ctxExceptionList_ = list;
}

wxArrayString
RuleWizardHistory::getCtxExceptionList(void) const
{
	return (ctxExceptionList_);
}

bool
RuleWizardHistory::getAlfHavePolicy(void) const
{
	return (alfHavePolicy_);
}

void
RuleWizardHistory::setAlfKeepPolicy(bool flag)
{
	alfKeepPolicy_ = flag;
}

bool
RuleWizardHistory::getAlfKeepPolicy(void) const
{
	return (alfKeepPolicy_);
}

void
RuleWizardHistory::setAlfClientPermission(enum alfPermissions permission)
{
	alfClientPermission_ = permission;
}

RuleWizardHistory::alfPermissions
RuleWizardHistory::getAlfClientPermission(void) const
{
	return (alfClientPermission_);
}

void
RuleWizardHistory::fillProgramNavi(wxWindow *parent, wxSizer *naviSizer,
    bool active) const
{
	wxFileName baseName;

	addTitle(parent, naviSizer, active, _("Program:"));

	baseName.Assign(program_);
	addValue(parent, naviSizer, baseName.GetFullName());
}

void
RuleWizardHistory::fillContextNavi(wxWindow *parent, wxSizer *naviSizer,
    bool active) const
{
	size_t	 count;
	wxString text;

	addTitle(parent, naviSizer, active, _("Context:"));

	if (ctxSame_) {
		text = _("restrictions: yes");
	} else {
		text = _("restrictions: no");
	}
	addValue(parent, naviSizer, text);

	if (ctxException_) {
		text = _("exceptions: yes");
	} else {
		text = _("exceptions: no");
	}
	addValue(parent, naviSizer, text);

	count = ctxExceptionList_.GetCount();
	if (count == 1) {
		text = _("1 exception");
		addValue(parent, naviSizer, text);
	} else if (count > 0) {
		text.Printf(_("%d exceptions"), count);
		addValue(parent, naviSizer, text);
	}
}

void
RuleWizardHistory::fillAlfNavi(wxWindow *parent, wxSizer *naviSizer,
    bool active) const
{
	addTitle(parent, naviSizer, active, _("ALF:"));

	if (alfKeepPolicy_ && alfHavePolicy_) {
		addValue(parent, naviSizer, _("keep policies"));
	} else if (!alfKeepPolicy_ && alfHavePolicy_){
		addValue(parent, naviSizer, _("discard policies"));
	}

	switch (alfClientPermission_) {
	case ALF_CLIENT_ALLOW_ALL:
		addValue(parent, naviSizer, _("allow all"));
		break;
	case ALF_CLEINT_RESTRICT_DEFAULT:
		addValue(parent, naviSizer, _("allow defaults"));
		break;
	case ALF_CLEINT_RESTRICT_USER:
		addValue(parent, naviSizer, _("allow user defined"));
		break;
	case ALF_CLIENT_DENY_ALL:
		addValue(parent, naviSizer, _("deny all"));
		break;
	case ALF_NONE:
		/* FALLTHROUGH */
	default:
		/* do nothing */
		break;
	}
}

void
RuleWizardHistory::addTitle(wxWindow *parent, wxSizer *naviSizer, bool active,
    wxString text) const
{
	wxStaticText *label;

	label = new wxStaticText(parent, wxID_ANY, text);
	label->Wrap(-1);
	if (active) {
		label->SetFont(activeSectionFont_);
	} else {
		label->SetFont(normalSectionFont_);
	}
	naviSizer->Add(label, 0, wxALL, 5);
}

void
RuleWizardHistory::addValue(wxWindow *parent, wxSizer *naviSizer,
    wxString text) const
{
	wxBoxSizer	*lineSizer;
	wxStaticText	*label;

	lineSizer = new wxBoxSizer(wxHORIZONTAL);
	lineSizer->Add(15, 0, 0, wxEXPAND, 5);

	label = new wxStaticText(parent, wxID_ANY, text);
	label->Wrap(-1);

	lineSizer->Add(label, 0, wxALL, 5);
	naviSizer->Add(lineSizer, 0, wxALL, 5);
}
