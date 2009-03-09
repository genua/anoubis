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
	csum_	 = wxEmptyString;

	haveContextPolicy_ = false;
	isSameContext_	   = true;
	haveContextException_ = false;
	contextExceptionList_.Clear();

	haveAlfPolicy_	     = false;
	overwriteAlfPolicy_  = OVERWRITE_YES;
	alfClientPermission_ = PERM_DENY_ALL;
	alfClientAsk_	     = true;
	alfClientRaw_	     = false;
	alfServerPermission_ = PERM_DENY_ALL;

	haveSandbox_		  = PERM_RESTRICT_USER;
	haveSandboxPolicy_	  = false;
	overwriteSandboxPolicy_   = OVERWRITE_NO;
	sandboxReadPermission_    = PERM_DENY_ALL;
	sandboxWritePermission_   = PERM_DENY_ALL;
	sandboxExecutePermission_ = PERM_DENY_ALL;
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
		haveContextPolicy_ = false;
		haveAlfPolicy_	   = false;
		haveSandboxPolicy_ = false;
		return;
	}

	ruleSet->lock();

	app = ruleSet->searchContextAppPolicy(program);
	if (app != NULL) {
		haveContextPolicy_ = true;
	} else {
		haveContextPolicy_ = false;
	}

	app = ruleSet->searchAlfAppPolicy(program);
	if (app != NULL) {
		haveAlfPolicy_ = true;
	} else {
		haveAlfPolicy_ = false;
	}

	app = ruleSet->searchSandboxAppPolicy(program);
	if (app != NULL) {
		haveSandboxPolicy_ = true;
	} else {
		haveSandboxPolicy_ = false;
	}

	ruleSet->unlock();
}

wxString
RuleWizardHistory::getProgram(void) const
{
	return (program_);
}

void
RuleWizardHistory::setChecksum(const wxString & csum)
{
	csum_ = csum;
}

wxString
RuleWizardHistory::getChecksum(void) const
{
	return (csum_);
}

/*
 * Context methods
 */
bool
RuleWizardHistory::haveContextPolicy(void) const
{
	return (haveContextPolicy_);
}

void
RuleWizardHistory::setSameContext(bool same)
{
	isSameContext_ = same;
}

bool
RuleWizardHistory::isSameContext(void) const
{
	return (isSameContext_);
}

void
RuleWizardHistory::setContextException(bool exception)
{
	haveContextException_ = exception;
	if (!haveContextException_) {
		contextExceptionList_.Clear();
	}
}

bool
RuleWizardHistory::haveContextException(void) const
{
	return (haveContextException_);
}

void
RuleWizardHistory::setContextExceptionList(const wxArrayString & list)
{
	contextExceptionList_ = list;
}

wxArrayString
RuleWizardHistory::getContextExceptionList(void) const
{
	return (contextExceptionList_);
}

/*
 * Alf methods
 */
bool
RuleWizardHistory::haveAlfPolicy(void) const
{
	return (haveAlfPolicy_);
}

void
RuleWizardHistory::setOverwriteAlfPolicy(enum overwriteAnswer answer)
{
	overwriteAlfPolicy_ = answer;
}

RuleWizardHistory::overwriteAnswer
RuleWizardHistory::shallOverwriteAlfPolicy(void) const
{
	return (overwriteAlfPolicy_);
}

void
RuleWizardHistory::setAlfClientPermission(enum permissionAnswer answer)
{
	alfClientPermission_ = answer;
}

RuleWizardHistory::permissionAnswer
RuleWizardHistory::getAlfClientPermission(void) const
{
	return (alfClientPermission_);
}

void
RuleWizardHistory::setAlfClientPortList(const wxArrayString & list)
{
	alfClientPortList_ = list;
}

wxArrayString
RuleWizardHistory::getAlfClientPortList(void) const
{
	return (alfClientPortList_);
}

void
RuleWizardHistory::setAlfClientAsk(bool ask)
{
	alfClientAsk_ = ask;
}

bool
RuleWizardHistory::getAlfClientAsk(void) const
{
	return (alfClientAsk_);
}

void
RuleWizardHistory::setAlfClientRaw(bool raw)
{
	alfClientRaw_ = raw;
}

bool
RuleWizardHistory::getAlfClientRaw(void) const
{
	return (alfClientRaw_);
}

void
RuleWizardHistory::setAlfServerPermission(enum permissionAnswer answer)
{
	alfServerPermission_ = answer;
}

RuleWizardHistory::permissionAnswer
RuleWizardHistory::getAlfServerPermission(void) const
{
	return (alfServerPermission_);
}

/*
 * Sandbox methods
 */
void
RuleWizardHistory::setSandbox(enum permissionAnswer answer)
{
	haveSandbox_ = answer;
}

RuleWizardHistory::permissionAnswer
RuleWizardHistory::haveSandbox(void) const
{
	return (haveSandbox_);
}

bool
RuleWizardHistory::haveSandboxPolicy(void) const
{
	return (haveSandboxPolicy_);
}

void
RuleWizardHistory::setOverwriteSandboxPolicy(enum overwriteAnswer answer)
{
	overwriteSandboxPolicy_ = answer;
}

RuleWizardHistory::overwriteAnswer
RuleWizardHistory::shallOverwriteSandboxPolicy(void) const
{
	return (overwriteSandboxPolicy_);
}

void
RuleWizardHistory::setSandboxReadPermission(enum permissionAnswer answer)
{
	sandboxReadPermission_ = answer;
}

RuleWizardHistory::permissionAnswer
RuleWizardHistory::getSandboxReadPermission(void) const
{
	return (sandboxReadPermission_);
}

void
RuleWizardHistory::setSandboxWritePermission(enum permissionAnswer answer)
{
	sandboxWritePermission_ = answer;
}

RuleWizardHistory::permissionAnswer
RuleWizardHistory::getSandboxWritePermission(void) const
{
	return (sandboxWritePermission_);
}

void
RuleWizardHistory::setSandboxExecutePermission(enum permissionAnswer answer)
{
	sandboxExecutePermission_ = answer;
}

RuleWizardHistory::permissionAnswer
RuleWizardHistory::getSandboxExecutePermission(void) const
{
	return (sandboxExecutePermission_);
}

/*
 * Navigation methods
 */
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
	wxString text;

	addTitle(parent, naviSizer, active, _("Context:"));

	if (haveContextPolicy_) {
		addValue(parent, naviSizer, _("keep policies"));
		return;
	}

	if (isSameContext_) {
		text = _("same: yes");
	} else {
		text = _("same: no");
	}
	addValue(parent, naviSizer, text);

	if (haveContextException_) {
		text.Printf(_("exceptions (%d): yes"),
		    contextExceptionList_.GetCount());
	} else {
		text = _("exceptions: no");
	}
	addValue(parent, naviSizer, text);
}

void
RuleWizardHistory::fillAlfNavi(wxWindow *parent, wxSizer *naviSizer,
    bool active) const
{
	addTitle(parent, naviSizer, active, _("ALF:"));

	if (haveAlfPolicy_) {
		if (overwriteAlfPolicy_ == OVERWRITE_NO) {
			addValue(parent, naviSizer, _("keep policies"));
			return;
		} else {
			addValue(parent, naviSizer, _("discard policies"));
		}
	}

	switch (alfClientPermission_) {
	case PERM_ALLOW_ALL:
		addValue(parent, naviSizer, _("client allow all"));
		break;
	case PERM_RESTRICT_DEFAULT:
		addValue(parent, naviSizer, _("client defaults"));
		break;
	case PERM_RESTRICT_USER:
		addValue(parent, naviSizer, _("client user defined"));
		break;
	case PERM_DENY_ALL:
		addValue(parent, naviSizer, _("client deny all"));
		break;
	case PERM_NONE:
		/* FALLTHROUGH */
	default:
		/* do nothing */
		break;
	}

	if (alfClientAsk_) {
		addValue(parent, naviSizer, _("client ask"));
	} else {
		addValue(parent, naviSizer, _("client don't ask"));
	}

	if (alfClientRaw_) {
		addValue(parent, naviSizer, _("client allow raw"));
	} else {
		addValue(parent, naviSizer, _("client deny raw"));
	}

	switch (alfServerPermission_) {
	case PERM_ALLOW_ALL:
		addValue(parent, naviSizer, _("server allow all"));
		break;
	case PERM_RESTRICT_DEFAULT:
		addValue(parent, naviSizer, _("server defaults"));
		break;
	case PERM_RESTRICT_USER:
		addValue(parent, naviSizer, _("server user defined"));
		break;
	case PERM_DENY_ALL:
		addValue(parent, naviSizer, _("server deny all"));
		break;
	case PERM_NONE:
		/* FALLTHROUGH */
	default:
		/* do nothing */
		break;
	}
}

void
RuleWizardHistory::fillSandboxNavi(wxWindow *parent, wxSizer *naviSizer,
    bool active) const
{
	addTitle(parent, naviSizer, active, _("Sandbox:"));

	if (haveSandboxPolicy_) {
		if (overwriteSandboxPolicy_ == OVERWRITE_NO) {
			addValue(parent, naviSizer, _("keep policies"));
			return;
		} else {
			addValue(parent, naviSizer, _("discard policies"));
		}
	} else if (haveSandbox_ == PERM_RESTRICT_USER) {
		addValue(parent, naviSizer, _("user defined"));
	} else if (haveSandbox_ == PERM_RESTRICT_DEFAULT) {
		addValue(parent, naviSizer, _("defaults"));
		return;
	} else {
		addValue(parent, naviSizer, _("no sandbox"));
		return;
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
	lineSizer->Add(15, 0, 0, wxEXPAND, 1);

	label = new wxStaticText(parent, wxID_ANY, text);
	label->Wrap(-1);

	lineSizer->Add(label, 0, wxALL, 1);
	naviSizer->Add(lineSizer, 0, wxALL, 1);
}
