/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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

#include "DlgRuleEditor.h"

DlgRuleEditor::DlgRuleEditor(wxWindow* parent) : DlgRuleEditorBase(parent)
{
	AnEvents::getInstance()->Connect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(DlgRuleEditor::OnShow), NULL, this);

}

DlgRuleEditor::~DlgRuleEditor(void)
{

}

/*
 * XXX ch: this will be fixed with the next functionality change
 */
#if 0
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/param.h>
#include <sys/socket.h>

#ifndef LINUX
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <wx/defs.h> /* mandatory but missing in choicdlg.h */
#include <wx/choicdlg.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/progdlg.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <apn.h>

#include "AnEvents.h"
#include "AnShortcuts.h"
#include "AnEvents.h"
#include "DlgRuleEditor.h"
#include "JobCtrl.h"
#include "main.h"
#include "Policy.h"
#include "PolicyRuleSet.h"
#include "RuleEditorAddPolicyVisitor.h"
#include "RuleEditorFillWidgetsVisitor.h"
#include "RuleEditorFillTableVisitor.h"
#include "RuleSetSearchPolicyVisitor.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(AddrLineList);

#include <stdio.h>
AddrLine::AddrLine(wxWindow *parent, wxString addr, wxString net)
{
	parent_ = parent;

	lead_ = new wxStaticText(parent, wxID_ANY, wxEmptyString,
	    wxDefaultPosition, wxDefaultSize, 0);
	addr_ = new wxComboBox(parent, wxID_ANY, wxT("Combo!"),
	    wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	delimiter_ = new wxStaticText(parent, wxID_ANY, wxT(" / "),
	    wxDefaultPosition, wxDefaultSize, 0);
	net_ = new wxSpinCtrl(parent, wxID_ANY, wxEmptyString,
	    wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 128, 0);
	remove_ = new wxButton(parent, wxID_ANY, wxT("-"),
	    wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	add_ = new wxButton(parent, wxID_ANY, wxT("+"),
	    wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

	addr_->SetValue(addr);
	net_->SetValue(net);
}


AddrLine::~AddrLine(void)
{
	sizer_->Detach(lead_);
	delete lead_;
	sizer_->Detach(addr_);
	delete addr_;
	sizer_->Detach(delimiter_);
	delete delimiter_;
	sizer_->Detach(net_);
	delete net_;
	sizer_->Detach(remove_);
	delete remove_;
	sizer_->Detach(add_);
	delete add_;

	sizer_->Fit(parent_);
	sizer_->Layout();
}

void
AddrLine::add(wxSizer *sizer, size_t index)
{
	sizer_ = sizer;

	sizer->Insert(index, add_, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	sizer->Insert(index, remove_, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	sizer->Insert(index, net_, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	sizer->Insert(index, delimiter_, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	sizer->Insert(index, addr_, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	sizer->Insert(index, lead_, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

	sizer->Fit(parent_);
	sizer->Layout();
}

void
AddrLine::remove(void)
{
	sizer_->Detach(lead_);
	sizer_->Detach(addr_);
	sizer_->Detach(delimiter_);
	sizer_->Detach(net_);
	sizer_->Detach(remove_);
	sizer_->Detach(add_);
	sizer_->Fit(parent_);
	sizer_->Layout();
}

DlgRuleEditor::DlgRuleEditor(wxWindow* parent) : DlgRuleEditorBase(parent)
{
	wxArrayString	 userList;
	AnEvents	*anEvents;

	selectedId_ = 0;
	selectedIndex_ = 0;
	autoCheck_ = false;
	userRuleSetId_ = -1;
	adminRuleSetId_ = -1;
	anEvents = AnEvents::getInstance();

	columnNames_[RULEDITOR_LIST_COLUMN_PRIO] = _("Index");
	columnWidths_[RULEDITOR_LIST_COLUMN_PRIO] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_RULE] = _("Rule");
	columnWidths_[RULEDITOR_LIST_COLUMN_RULE] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_USER] = _("User");
	columnWidths_[RULEDITOR_LIST_COLUMN_USER] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_BIN] = _("Binary");
	columnWidths_[RULEDITOR_LIST_COLUMN_BIN] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_HASHT] = _("Hash-Type");
	columnWidths_[RULEDITOR_LIST_COLUMN_HASHT] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_HASH] = _("Hash");
	columnWidths_[RULEDITOR_LIST_COLUMN_HASH] = 80;

	columnNames_[RULEDITOR_LIST_COLUMN_TYPE] = _("Type");
	columnWidths_[RULEDITOR_LIST_COLUMN_TYPE] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_SCOPE] = _("Scope");
	columnWidths_[RULEDITOR_LIST_COLUMN_SCOPE] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_ACTION] = _("Action");
	columnWidths_[RULEDITOR_LIST_COLUMN_ACTION] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_LOG] = _("Log");
	columnWidths_[RULEDITOR_LIST_COLUMN_LOG] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_AF] = _("AF");
	columnWidths_[RULEDITOR_LIST_COLUMN_AF] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_CAP] = _("Capability");
	columnWidths_[RULEDITOR_LIST_COLUMN_CAP] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_PROTO] = _("Protocol");
	columnWidths_[RULEDITOR_LIST_COLUMN_PROTO] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_DIR] = _("Direction");
	columnWidths_[RULEDITOR_LIST_COLUMN_DIR] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_FHOST] = _("From Host");
	columnWidths_[RULEDITOR_LIST_COLUMN_FHOST] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_FPORT] = _("From Port");
	columnWidths_[RULEDITOR_LIST_COLUMN_FPORT] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_THOST] = _("To Host");
	columnWidths_[RULEDITOR_LIST_COLUMN_THOST] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_TPORT] = _("To Port");
	columnWidths_[RULEDITOR_LIST_COLUMN_TPORT] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_STATETIMEOUT] = _("State Timeout");
	columnWidths_[RULEDITOR_LIST_COLUMN_STATETIMEOUT] =
	    wxLIST_AUTOSIZE_USEHEADER;

	for (int i=0; i<RULEDITOR_LIST_COLUMN_EOL; i++) {
		ruleListCtrl->InsertColumn(i, columnNames_[i],
		    wxLIST_FORMAT_LEFT, columnWidths_[i]);
	}

	shortcuts_ = new AnShortcuts(this);

	if (ProfileCtrl::getInstance()->getUserId() != -1) {
		this->controlRuleSetSaveButton->Disable();
		this->controlRuleCreateButton->Disable();
		this->controlRuleDeleteButton->Disable();
	}

	controlRuleSetStatusText->SetLabel(wxT("no ruleset loaded"));

	userList = wxGetApp().getListOfUsersName();
	controlUserChoice->Clear();
	for (size_t i=0; i<userList.GetCount(); i++) {
		controlUserChoice->Append(userList.Item(i));
	}

	anEvents->Connect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(DlgRuleEditor::OnShow), NULL, this);
	anEvents->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::OnLoadRuleSet), NULL, this);
	anEvents->Connect(anEVT_SHOW_RULE,
	    wxCommandEventHandler(DlgRuleEditor::OnShowRule), NULL, this);
	anEvents->Connect(anEVT_SEND_AUTO_CHECK,
	    wxCommandEventHandler(DlgRuleEditor::OnAutoCheck), NULL, this);

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_ADD,
	    wxTaskEventHandler(DlgRuleEditor::OnChecksumAdd), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_GET,
	    wxTaskEventHandler(DlgRuleEditor::OnChecksumAdd), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUMCALC,
	    wxTaskEventHandler(DlgRuleEditor::OnChecksumCalc), NULL, this);

	ANEVENTS_IDENT_BCAST_REGISTRATION(DlgRuleEditor);
}

DlgRuleEditor::~DlgRuleEditor(void)
{
	AnEvents *anEvents;

	anEvents = AnEvents::getInstance();

	anEvents->Disconnect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(DlgRuleEditor::OnShow), NULL, this);
	anEvents->Disconnect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::OnLoadRuleSet), NULL, this);
	anEvents->Disconnect(anEVT_SHOW_RULE,
	    wxCommandEventHandler(DlgRuleEditor::OnShowRule), NULL, this);
	anEvents->Disconnect(anEVT_SEND_AUTO_CHECK,
	    wxCommandEventHandler(DlgRuleEditor::OnAutoCheck), NULL, this);

	ANEVENTS_IDENT_BCAST_DEREGISTRATION(DlgRuleEditor);

	delete shortcuts_;
}

void
DlgRuleEditor::updateBinName(wxString binName)
{
	Policy				*policy;
	AppPolicy			*appPolicy;
	SfsPolicy			*sfsPolicy;
	CtxPolicy			*ctxPolicy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(SfsPolicy))) {
		sfsPolicy = (SfsPolicy *)policy;
		sfsPolicy->setBinaryName(binName);
	} else if (policy->IsKindOf(CLASSINFO(CtxPolicy))) {
		ctxPolicy = (CtxPolicy *)policy;
		ctxPolicy->setBinaryName(binName);
	} else {
		appPolicy = (AppPolicy *)policy;
		appPolicy->setBinaryName(binName);
	}

	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateAction(int action)
{
	AlfPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}
	policy->setAction(action);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateType(int type)
{
	AlfPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setType(type);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateProtocol(int protocol)
{
	AlfPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setProtocol(protocol);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateAddrFamily(int addrFamily)
{
	AlfPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setAddrFamily(addrFamily);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateCapType(int type)
{
	AlfPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setCapType(type);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateDirection(int direction)
{
	AlfPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setDirection(direction);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateTimeout(int timeout)
{
	AlfPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setStateTimeout(timeout);
	policy->accept(updateTable);
}

void
DlgRuleEditor::updateAlfSrcAddr(wxString address, int netmask, int af)
{
	AlfPolicy                       *policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setAlfSrcAddress(address, netmask, af);
	policy->accept(updateTable);
}

void
DlgRuleEditor::updateAlfDstAddr(wxString address, int netmask, int af)
{
	AlfPolicy                       *policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setAlfDstAddress(address, netmask, af);
	policy->accept(updateTable);
}

void
DlgRuleEditor::updateAlfSrcPort(int port)
{
	AlfPolicy                       *policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor     updateWidgets(this);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setAlfSrcPort(port);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateAlfDstPort(int port)
{
	AlfPolicy                       *policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor     updateWidgets(this);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setAlfDstPort(port);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateLog(int logNo)
{
	Policy				*policy;
	AlfPolicy			*alfPolicy;
	SfsPolicy			*sfsPolicy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);

	policy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	if (policy->IsKindOf(CLASSINFO(SfsPolicy))) {
		sfsPolicy = (SfsPolicy *)policy;
		sfsPolicy->setLogNo(logNo);
	} else {
		alfPolicy = (AlfPolicy *)policy;
		alfPolicy->setLogNo(logNo);
	}

	policy->accept(updateTable);
}

void
DlgRuleEditor::OnCommonLogNone(wxCommandEvent& )
{
	updateLog(APN_LOG_NONE);
	modified();
}

void
DlgRuleEditor::OnCommonLogLog(wxCommandEvent& )
{
	updateLog(APN_LOG_NORMAL);
	modified();
}

void
DlgRuleEditor::OnCommonLogAlert(wxCommandEvent& )
{
	updateLog(APN_LOG_ALERT);
	modified();
}

void
DlgRuleEditor::OnTableOptionButtonClick(wxCommandEvent& )
{
	wxArrayString		 choices;
	wxMultiChoiceDialog	*multiChoiceDlg;

	for (int i=0; i<RULEDITOR_LIST_COLUMN_EOL; i++) {
		choices.Add(columnNames_[i]);
	}

	multiChoiceDlg = new wxMultiChoiceDialog(this, _("Table columns"),
	    _("Please select the columns you're interested in"), choices);

	if (multiChoiceDlg->ShowModal() == wxID_OK) {
		/* XXX: alter table columns here -- ch */
	}

	delete multiChoiceDlg;
}

void
DlgRuleEditor::OnAppBinaryTextCtrl(wxCommandEvent& event)
{
	updateBinName(event.GetString());
	modified();
}

void
DlgRuleEditor::onAppContextTextCtrl(wxCommandEvent& )
{
}

void
DlgRuleEditor::OnSfsBinaryTextCtrl(wxCommandEvent& event)
{
	updateBinName(event.GetString());
	modified();
}

void
DlgRuleEditor::OnAppBinaryModifyButton(wxCommandEvent& )
{
	wxString	 caption = _("Choose a binary");
	wxString	 wildcard = wxT("*");
	wxString	 defaultDir = wxT("/usr/bin/");
	wxString	 defaultFilename = wxEmptyString;
	wxFileDialog	*fileDlg;

	if (wxIsBusy())
		return;

	wxBeginBusyCursor();
	fileDlg = new wxFileDialog(NULL, caption, defaultDir, defaultFilename,
	    wildcard, wxOPEN);
	wxEndBusyCursor();

	if (fileDlg->ShowModal() == wxID_OK) {
		updateBinName(fileDlg->GetPath());
		modified();
	}
}

void
DlgRuleEditor::OnAppUpdateChkSumButton(wxCommandEvent& )
{
	Policy				*policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);
	unsigned char			 *csum;
	wxString			 curHash;

	if (wxIsBusy())
		return;

	policy = (Policy*)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	if (policy->IsKindOf(CLASSINFO(AppPolicy))) {
		AppPolicy	*appPolicy = (AppPolicy*)policy;
		csum = appPolicy->getCurrentSum();
		appPolicy->setHashValue(csum);
		appPolicy->accept(updateTable);
		appPolicy->accept(updateWidgets);
		modified();
	} else if (policy->IsKindOf(CLASSINFO(CtxPolicy))) {
		CtxPolicy	*ctxPolicy = (CtxPolicy*)policy;
		csum = ctxPolicy->getCurrentSum();
		ctxPolicy->setHashValue(csum);
		ctxPolicy->accept(updateTable);
		ctxPolicy->accept(updateWidgets);
		modified();
	}
}

void
DlgRuleEditor::OnAppValidateChkSumButton(wxCommandEvent& )
{
	Policy				*genpolicy;
	RuleEditorFillWidgetsVisitor	 updateVisitor(this);
	unsigned char			 csum[MAX_APN_HASH_LEN];
	wxString			 currHash;
	wxString			 message;
	wxString			 title;
	int				 ret;

	if (wxIsBusy())
		return;

	genpolicy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!genpolicy)
		return;

	if (genpolicy->IsKindOf(CLASSINFO(AppPolicy))) {
		AppPolicy	*policy = (AppPolicy*)genpolicy;

		ret = policy->calcCurrentHash(csum);
		if (ret == 1) {
			updateVisitor.setPropagation(false);
			policy->accept(updateVisitor);
			modified();
		} else if (ret == 0) {
			wxGetApp().calChecksum(policy->getBinaryName());
			wxBeginBusyCursor();
			modified();
		} else if (ret == -2) {
			message = _("Wrong file access permission");
			title = _("File is not set readable for the user.");
			wxMessageBox(message, title, wxICON_INFORMATION);
		} else {
			message = _("File does not exist");
			title = _("File does not exist");
			wxMessageBox(message, title, wxICON_INFORMATION);
		}
	} else if (genpolicy->IsKindOf(CLASSINFO(CtxPolicy))) {
		CtxPolicy	*policy = (CtxPolicy*)genpolicy;

		ret = policy->calcCurrentHash(csum);
		if (ret == 1) {
			updateVisitor.setPropagation(false);
			policy->accept(updateVisitor);
			modified();
		} else if (ret == 0) {
			wxGetApp().calChecksum(policy->getBinaryName());
			wxBeginBusyCursor();
			modified();
		} else if (ret == -2) {
			message = _("Wrong file access permission");
			title = _("File is not set readable for the user.");
			wxMessageBox(message, title, wxICON_INFORMATION);
		} else {
			message = _("File does not exist");
			title = _("File does not exist");
			wxMessageBox(message, title, wxICON_INFORMATION);
		}
	}
}

void
DlgRuleEditor::OnRuleCreateButton(wxCommandEvent& )
{
	int		 id;
	uid_t		 uid;
	long		 rsid;
	PolicyRuleSet	*ruleSet;
	ProfileCtrl	*profileCtrl;

	id = -1;
	profileCtrl = ProfileCtrl::getInstance();

	if (geteuid() != 0) {
		rsid = userRuleSetId_;
	} else {
		wxArrayString userList = wxGetApp().getListOfUsersName();
		uid = wxGetApp().getUserIdByName(
		    userList.Item(controlUserChoice->GetSelection()));
		rsid = profileCtrl->getAdminId(uid);
	}

	ruleSet = profileCtrl->getRuleSet(rsid);
	if (ruleSet == NULL) {
		wxMessageBox(_("Error: Couldn't access rule set to create."),
		    _("Rule Editor"), wxOK, this);
		return;
	}

	/*
	 * Indices in the selection:
	 *  0 == ALF Application
	 *  1 == ALF Filter
	 *  2 == Sandbox Application
	 *  3 == Sandobx Filter
	 *  4 == Context Application
	 *  5 == Context Filter
	 *  6 == SFS
	 */
	switch (controlCreationChoice->GetSelection()) {
	case 0: /* ALF Application */
		id = ruleSet->createAlfAppPolicy(selectedId_);
		break;
	case 1: /* ALF Filter*/
		id = ruleSet->createAlfPolicy(selectedId_);
		break;
	case 2: /* Sandbox Application */
	case 3: /* Sandbox Filter */
		/* XXX CEH: This needs implementation. */
		break;
	case 4: /* Context Application */
		id = ruleSet->createCtxAppPolicy(selectedId_);
		break;
	case 5: /* Context NEW */
		id = ruleSet->createCtxNewPolicy(selectedId_);
		break;
	case 6: /* SFS */
		id = ruleSet->createSfsPolicy(selectedId_);
		break;
	default:
		break;
	}

	if (id == -1) {
		wxMessageBox(_("Could not create new rule."), _("Error"),
		    wxOK, this);
	} else {
		modified();
		loadRuleSet();
	}
}

void
DlgRuleEditor::OnRuleDeleteButton(wxCommandEvent& )
{
	Policy		*policy;
	PolicyRuleSet	*rs;

	policy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	rs = policy->getRsParent();
	if (rs == NULL) {
		return;
	}

	if ((geteuid() != 0) && rs->isAdmin()) {
		wxMessageBox(_("No permission to edit admin ruleset."),
		    _("Rule Editor"), wxOK, this);
		return;
	}

	rs->deletePolicy(selectedId_);
	modified();
}

void
DlgRuleEditor::OnClose(wxCloseEvent& )
{
	wxCommandEvent  showEvent(anEVT_RULEEDITOR_SHOW);

	showEvent.SetInt(false);

	wxPostEvent(AnEvents::getInstance(), showEvent);
}

void
DlgRuleEditor::OnAlfAllowRadioButton(wxCommandEvent& )
{
	updateAction(APN_ACTION_ALLOW);
	modified();
}

void
DlgRuleEditor::OnAlfDenyRadioButton(wxCommandEvent& )
{
	updateAction(APN_ACTION_DENY);
	modified();
}

void
DlgRuleEditor::OnAlfAskRadioButton(wxCommandEvent& )
{
	updateAction(APN_ACTION_ASK);
	modified();
}

void
DlgRuleEditor::OnAlfFilterRadioButton(wxCommandEvent& )
{
	updateType(APN_ALF_FILTER);
	modified();
}

void
DlgRuleEditor::OnAlfCapRadioButton(wxCommandEvent& )
{
	updateType(APN_ALF_CAPABILITY);
	modified();
}

void
DlgRuleEditor::OnAlfDefaultRadioButton(wxCommandEvent& )
{
	updateType(APN_DEFAULT);
	modified();
}

void
DlgRuleEditor::OnAlfTcpRadioButton(wxCommandEvent& )
{
	updateProtocol(IPPROTO_TCP);
	modified();
}

void
DlgRuleEditor::OnAlfUdpRadioButton(wxCommandEvent& )
{
	updateProtocol(IPPROTO_UDP);
	modified();
}

void
DlgRuleEditor::OnAlfInetRadioButton(wxCommandEvent& )
{
	updateAddrFamily(AF_INET);
	modified();
}

void
DlgRuleEditor::OnAlfInet6RadioButton(wxCommandEvent& )
{
	updateAddrFamily(AF_INET6);
	modified();
}

void
DlgRuleEditor::OnAlfAnyRadioButton(wxCommandEvent& )
{
	updateAddrFamily(0);
	modified();
}

void
DlgRuleEditor::OnAlfRawCapRadioButton(wxCommandEvent& )
{
	updateCapType(APN_ALF_CAPRAW);
	modified();
}

void
DlgRuleEditor::OnAlfOtherCapRadioButton(wxCommandEvent& )
{
	updateCapType(APN_ALF_CAPOTHER);
	modified();
}

void
DlgRuleEditor::OnAlfAllCapRadioButton(wxCommandEvent& )
{
	updateCapType(APN_ALF_CAPALL);
	modified();
}

void
DlgRuleEditor::OnAlfAcceptRadioButton(wxCommandEvent& )
{
	updateDirection(APN_ACCEPT);
	modified();
}

void
DlgRuleEditor::OnAlfConnectRadioButton(wxCommandEvent& )
{
	updateDirection(APN_CONNECT);
	modified();
}

void
DlgRuleEditor::OnAlfBothRadioButton(wxCommandEvent& )
{
	updateDirection(APN_BOTH);
	modified();
}

void
DlgRuleEditor::OnSfsBinaryModifyButton(wxCommandEvent& )
{
	wxString	 caption = _("Choose a binary");
	wxString	 wildcard = wxT("*");
	wxString	 defaultDir = wxT("/usr/bin/");
	wxString	 defaultFilename = wxEmptyString;
	wxFileDialog	*fileDlg;

	if (wxIsBusy())
		return;

	wxBeginBusyCursor();
	fileDlg = new wxFileDialog(NULL, caption, defaultDir, defaultFilename,
	    wildcard, wxOPEN);
	wxEndBusyCursor();

	if (fileDlg->ShowModal() == wxID_OK) {
		updateBinName(fileDlg->GetPath());
		modified();
	}
}

void
DlgRuleEditor::OnSfsUpdateChkSumButton(wxCommandEvent& )
{
	RuleEditorFillWidgetsVisitor	 updateVisitor(this);
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	SfsPolicy			*policy;
	unsigned char			*csum;

	if (wxIsBusy())
		return;

	policy = (SfsPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	csum = policy->getCurrentSum();
	policy->setHashValue(csum);
	updateVisitor.setPropagation(false);
	policy->accept(updateVisitor);
	policy->accept(updateTable);
	modified();
}

void
DlgRuleEditor::OnSfsValidateChkSumButton(wxCommandEvent& )
{
	SfsPolicy			*policy;
	wxString			 currHash;
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);
	unsigned char			 csum[MAX_APN_HASH_LEN];
	wxString			 message;
	wxString			 title;
	int				 ret;

	if (wxIsBusy())
		return;

	policy = (SfsPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	ret = policy->calcCurrentHash(csum);

	switch (ret) {
	case  1:
		updateWidgets.setPropagation(false);
		policy->accept(updateWidgets);
		modified();
		break;
	case  0:
		wxGetApp().calChecksum(policy->getBinaryName());
		wxBeginBusyCursor();
		modified();
		break;
	case (-2):
		message = _("Wrong file access permission");
		title = _("File is not set readable for the user.");
		wxMessageBox(message, title, wxICON_INFORMATION);
		break;
	default:
		message = _("File does not exist");
		title = _("File does not exist");
		wxMessageBox(message, title, wxICON_INFORMATION);
		break;
	}

}
#endif

void
DlgRuleEditor::OnShow(wxCommandEvent& event)
{
	this->Show(event.GetInt());
	event.Skip();
}

/*
 * XXX ch: this will be fixed with the next functionality change
 */
#if 0
void
DlgRuleEditor::OnRuleSetSave(wxCommandEvent& )
{
	wxString	 tmpPreFix;
	wxString	 tmpName;
	wxString	 message;
	PolicyRuleSet	*rs;
	ProfileCtrl	*profileCtrl;

	profileCtrl = ProfileCtrl::getInstance();
	rs = profileCtrl->getRuleSet(userRuleSetId_);
	if (rs == NULL) {
		wxMessageBox(_("Error: Couldn't access rule set to store."),
		    _("Rule Editor"), wxOK, this);
		return;
	}
	if (rs->findMismatchHash())
	{
		message = _("Mismatch of Checksums in one or more Rule.\n");
		message += _("Do you want to store anyway?");
		int answer = wxMessageBox(message,
		    _("Mismatch of Checksums"), wxYES_NO, this);
		if (answer == wxNO) {
			return;
		}
	}

	rs->clearModified();
	profileCtrl->sendToDaemon(rs->getId());

	controlRuleSetStatusText->SetLabel(wxT("sent to daemon"));
	controlRuleSetSaveButton->Disable();

	if (geteuid() == 0) {
		wxProgressDialog progDlg(_("Rule Editor"),
		    _("Sending admin rule sets ..."),
		    foreignAdminRsIds_.GetCount(), this);
		for (size_t i=0; i<foreignAdminRsIds_.GetCount(); i++) {
			progDlg.Update(i);
			rs = profileCtrl->getRuleSet(
			    foreignAdminRsIds_.Item(i));
			profileCtrl->sendToDaemon(rs->getId());
		}
	}
}

void
DlgRuleEditor::loadRuleSet(void)
{
	RuleEditorAddPolicyVisitor	 addVisitor(this);
	PolicyRuleSet			*ruleSet;
	ProfileCtrl			*profileCtrl;
	unsigned long			 uid;

	profileCtrl = ProfileCtrl::getInstance();

	ruleListCtrl->DeleteAllItems();

	ruleSet = profileCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		wxString text;

		if (ruleSet->isModified()) {
			modified();
		} else {
			text = _("loaded from ") + ruleSet->getOrigin();
			if (ruleSet->hasErrors()) {
				text += _(" but has error");
			}
			controlRuleSetStatusText->SetLabel(text);
		}

		addVisitor.setAdmin(false);
		ruleSet->accept(addVisitor);
	}

	ruleSet = profileCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
		addVisitor.setAdmin(true);
		ruleSet->accept(addVisitor);
	}

	if (geteuid() == 0) {
		long		rsid;
		wxArrayString	userList = wxGetApp().getListOfUsersId();

		for (size_t i=0; i<userList.GetCount(); i++) {
			userList.Item(i).ToULong(&uid);

			rsid = profileCtrl->getAdminId((uid_t)uid);
			if (rsid == -1) {
				continue;
			}

			foreignAdminRsIds_.Add(rsid);
			ruleSet = profileCtrl->getRuleSet(rsid);
			if (ruleSet != NULL) {
				addVisitor.setAdmin(true);
				ruleSet->accept(addVisitor);
			}
		}
	}

	/* trigger new calculation of column width */
	for (int i=0; i<RULEDITOR_LIST_COLUMN_EOL; i++) {
		ruleListCtrl->SetColumnWidth(i, columnWidths_[i]);
	}
}

void
DlgRuleEditor::OnLoadRuleSet(wxCommandEvent& event)
{
	ProfileCtrl	*profileCtrl;

	profileCtrl = ProfileCtrl::getInstance();

	this->controlRuleSetSaveButton->Enable();
	this->controlRuleCreateButton->Enable();
	this->controlRuleDeleteButton->Enable();

	userRuleSetId_ = profileCtrl->getUserId();

	adminRuleSetId_ = profileCtrl->getAdminId(geteuid());

	for (size_t i=0; i<foreignAdminRsIds_.GetCount(); i++) {
		foreignAdminRsIds_.RemoveAt(i);
	}

	loadRuleSet();
	selectLine(selectedIndex_);
	event.Skip();
}

void
DlgRuleEditor::OnLineSelected(wxListEvent& event)
{
	RuleEditorFillWidgetsVisitor	 updateVisitor(this);
	Policy				*policy;
	unsigned long			 newSelection;

	if (wxIsBusy())
		return;

	newSelection = event.GetIndex();

	if (autoCheck_ && (newSelection != selectedIndex_)) {
		if (!CheckLastSelection()) {
			selectLine(selectedIndex_);
			return;
		}
	}
	selectedIndex_ = newSelection;
	updateVisitor.setPropagation(false);
	policy = (Policy *)event.GetData();
	selectedId_ = policy->getId();
	if (!policy)
		return;
	policy->accept(updateVisitor);
}

void
DlgRuleEditor::OnSrcAddrAddButton(wxCommandEvent& )
{
}

void
DlgRuleEditor::OnAlfStateTimeoutChange(wxSpinEvent& event)
{
	updateTimeout(event.GetPosition());
	modified();
}

void
DlgRuleEditor::onAlfSrcAddrTextCtrlEnter(wxCommandEvent& event)
{
	int af = alfInet6RadioButton->GetValue() ? AF_INET6 : AF_INET;

	if (alfAnyRadioButton->GetValue()) {
		updateAddrFamily(AF_INET);
	}

	updateAlfSrcAddr(event.GetString(), alfSrcAddrNetSpinCtrl->GetValue(),
	    af);
	modified();
}

void
DlgRuleEditor::onAlfDstAddrTextCtrlEnter(wxCommandEvent& event)
{
	if (alfAnyRadioButton->GetValue()) {
		updateAddrFamily(AF_INET);
	}

	int af = alfInet6RadioButton->GetValue() ? AF_INET6 : AF_INET;
	updateAlfDstAddr(event.GetString(), alfDstAddrNetSpinCtrl->GetValue(),
	    af);
	modified();
}

void
DlgRuleEditor::OnAlfSrcNetmaskSpinCtrl(wxSpinEvent& event)
{
	if (alfAnyRadioButton->GetValue()) {
		updateAddrFamily(AF_INET);
	}

	int af = alfInet6RadioButton->GetValue() ? AF_INET6 : AF_INET;
	updateAlfSrcAddr(alfSrcAddrTextCtrl->GetValue(),
	    event.GetPosition(), af);
	modified();
}

void
DlgRuleEditor::OnAlfDstNetmaskSpinCtrl(wxSpinEvent& event)
{
	if (alfAnyRadioButton->GetValue()) {
		updateAddrFamily(AF_INET);
	}


	int af = alfInet6RadioButton->GetValue() ? AF_INET6 : AF_INET;
	updateAlfDstAddr(alfDstAddrTextCtrl->GetValue(),
	    event.GetPosition(), af);
	modified();
}

void
DlgRuleEditor::onAlfSrcPortTextCtrlEnter(wxCommandEvent& event)
{
	unsigned long int       port;

	if (!event.GetString().Cmp(wxT("any"))) {
		port = 0;
	} else {
		event.GetString().ToULong(&port);
	}
	updateAlfSrcPort(port);
	modified();
}

void
DlgRuleEditor::onAlfDstPortTextCtrlEnter(wxCommandEvent& event)
{
	unsigned long int       port;

	if (!event.GetString().Cmp(wxT("any"))) {
		port = 0;
	} else {
		event.GetString().ToULong(&port);
	}
	updateAlfDstPort(port);
	modified();
}

void
DlgRuleEditor::OnShowRule(wxCommandEvent& event)
{
	wxListView	*selecter;
	unsigned long	 id;
	Policy		*policy;
	PolicyRuleSet	*rs;
	ProfileCtrl	*profileCtrl;

	profileCtrl = ProfileCtrl::getInstance();
	event.Skip();

	rs = profileCtrl->getRuleSet(userRuleSetId_);
	if (rs == NULL) {
		wxMessageBox(_("Error: Couldn't access rule set to store."),
		    _("Rule Editor"), wxOK, this);
		return;
	}

	/* show RuleEditor Dialog and corresponding Ruleid entry */
	this->Show();
	selecter = (wxListView*)ruleListCtrl;
	id = event.GetExtraLong();
	RuleSetSearchPolicyVisitor seeker(id);
	rs->accept(seeker);
	if (! seeker.hasMatchingPolicy()) {
		rs = profileCtrl->getRuleSet(adminRuleSetId_);
		if (rs == NULL) {
			return;
		}
		rs->accept(seeker);
		if (! seeker.hasMatchingPolicy())
			return;
	}
	policy = seeker.getMatchingPolicy();

	selecter->Select(policy->getIndex());
}

void
DlgRuleEditor::OnAutoCheck(wxCommandEvent& event)
{
	autoCheck_ = event.GetInt();
	event.Skip();
}

bool
DlgRuleEditor::CheckLastSelection(void)
{
	Policy				*policy;
	RuleEditorFillWidgetsVisitor	 updateVisitor(this);
	AppPolicy			*appPolicy;
	SfsPolicy			*sfsPolicy;
	AlfPolicy			*alfPolicy;
	unsigned char			 csum[MAX_APN_HASH_LEN];
	int				 mismatch;
	wxString			 currHash;
	wxString			 regHash;
	wxString			 message;

	sfsPolicy = NULL;
	alfPolicy = NULL;
	appPolicy = NULL;
	policy	  = NULL;

	policy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy) {
		selectedIndex_ = 0;
		return (false);
	}

	if (policy->IsKindOf(CLASSINFO(SfsPolicy))) {
		sfsPolicy = (SfsPolicy *)policy;
		if (!sfsPolicy->isModified())
			return (true);

		if (sfsPolicy->calcCurrentHash(csum)) {
			currHash = wxT("0x");
			for (unsigned int i=0; i<MAX_APN_HASH_LEN; i++)
			{
				currHash += wxString::Format(
				wxT("%2.2x"), (unsigned char)csum[i]);
			}
		} else {
			/* XXX: KM Better Error Handling is needed */
			currHash = _("unable to calculate checksum");
			return (true);
		}
		regHash = sfsPolicy->getHashValue();
		mismatch = regHash.Cmp(currHash);
	} else {
		if (policy->IsKindOf(CLASSINFO(AlfPolicy))) {
			alfPolicy = (AlfPolicy *)policy;
			appPolicy = (AppPolicy *)alfPolicy->getParent();
		} else {
			appPolicy = (AppPolicy *)policy;
		}
		if (!appPolicy->isModified() ||
		    !appPolicy->getBinaryName().Cmp(_("any")))
			return (true);
		if (appPolicy->calcCurrentHash(csum)) {
			currHash = wxT("0x");
			for (unsigned int i=0; i<MAX_APN_HASH_LEN; i++)
			{
				currHash += wxString::Format(
				wxT("%2.2x"), (unsigned char)csum[i]);
			}
		} else {
			/* XXX: KM Better Error Handling is needed */
			currHash = _("unable to calculate checksum");
			return (true);
		}
		regHash = appPolicy->getHashValue();
		mismatch = regHash.Cmp(currHash);
	}

	if (mismatch) {
		message = _("Checksums don't match for this Rule.\n");
		message += _("Back to Rule?");
		int answer = wxMessageBox(message,
		    _("Back to Rule?"),
		    wxYES_NO, this);
		if (answer == wxYES) {
			updateVisitor.setPropagation(false);
			policy = (Policy *)ruleListCtrl->GetItemData(
			    selectedIndex_);
			if (!policy)
				return (false);
			if (policy->IsKindOf(CLASSINFO(AlfPolicy))) {
				alfPolicy = (AlfPolicy *)policy;
				appPolicy = (AppPolicy *)alfPolicy->getParent();
				selectedIndex_ = appPolicy->getIndex();
				policy = appPolicy;
			}
			policy->accept(updateVisitor);
			return (false);
		}

	}
	return (true);
}

void
DlgRuleEditor::OnChecksumCalc(TaskEvent &event)
{
	wxString			 sum;
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);
	unsigned char			 csum[MAX_APN_HASH_LEN];
	Policy				*policy;

	CsumCalcTask *task = dynamic_cast<CsumCalcTask*>(event.getTask());

	if (task == 0) {
		/* Calculation failed */
		wxString message = _("Undefined result received!");
		wxString title = _("Undefined result");

		wxEndBusyCursor();
		wxMessageBox(message, title, wxICON_INFORMATION);

		event.Skip();
		return;
	}

	if (task->getResult() != 0) {
		/* Calculation failed */
		wxString message = _("Permission denied for this file.");
		wxString title = _("Permission denied");

		wxEndBusyCursor();
		wxMessageBox(message, title, wxICON_INFORMATION);

		event.Skip();
		return;
	}

	sum = wxT("0x") + task->getCsumStr();
	bcopy(task->getCsum(), csum, MAX_APN_HASH_LEN);

	policy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy) {
		event.Skip();
		return;
	}

	if(policy->IsKindOf(CLASSINFO(SfsPolicy))) {
		SfsPolicy *sfsPolicy = (SfsPolicy *)policy;

		sfsPolicy->setCurrentSum(csum);
		sfsPolicy->setCurrentHash(sum);
		wxGetApp().getChecksum(sfsPolicy->getBinaryName());
	} else {
		/* AppPolicy doesn't need to be checked with Shaodwtree*/
		AppPolicy *appPolicy = (AppPolicy *)policy;

		appPolicy->setCurrentSum(csum);
		appPolicy->setCurrentHash(sum);

		policy->accept(updateWidgets);
		wxEndBusyCursor();
	}
	modified();

	event.Skip();
}

void
DlgRuleEditor::OnChecksumAdd(TaskEvent &event)
{
	ComCsumAddTask *task = dynamic_cast<ComCsumAddTask*>(event.getTask());

	if (task == 0) {
		wxString message = _("Undefined result received!");
		wxString title = _("Undefined result");

		wxEndBusyCursor();
		wxMessageBox(message, title, wxICON_INFORMATION);
	}

	if (task != 0 && task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		wxString message = _("Permission denied for this file.");
		wxString title = _("Permission denied");

		wxEndBusyCursor();
		wxMessageBox(message, title, wxICON_INFORMATION);
	}

	event.Skip();
}

void
DlgRuleEditor::OnChecksumGet(TaskEvent& event)
{
	wxString			 sum;
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);
	unsigned char			 csum[MAX_APN_HASH_LEN];
	Policy				*policy;

	ComCsumGetTask *task = dynamic_cast<ComCsumGetTask*>(event.getTask());

	if (task == 0) {
		wxString message = _("Undefined result received!");
		wxString title = _("Undefined result");

		wxEndBusyCursor();
		wxMessageBox(message, title, wxICON_INFORMATION);

		event.Skip();
		return;
	}

	if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		/* Error-path */
		wxString message = _("Failed to receive checksum from daemon.");
		wxString title = _("Checksum failure");

		wxEndBusyCursor();
		wxMessageBox(message, title, wxICON_INFORMATION);

		event.Skip();
		return;
	}

	sum = wxT("0x") + task->getCsumStr();
	task->getCsum(csum, MAX_APN_HASH_LEN);

	policy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy) {
		event.Skip();
		return;
	}

	SfsPolicy *sfsPolicy = (SfsPolicy *)policy;

	if (sum.Cmp(sfsPolicy->getCurrentHash())) {
		/* if not matching update shadowtree */
		wxGetApp().sendChecksum(sfsPolicy->getBinaryName());
	}

	policy->accept(updateWidgets);
	wxEndBusyCursor();

	event.Skip();
}

void
DlgRuleEditor::selectLine(unsigned long index)
{
	wxListView			*selecter;

	selecter = (wxListView*)ruleListCtrl;
	selecter->Select(index);
}

void
DlgRuleEditor::modified(void)
{
	wxString	 text;
	ProfileCtrl	*profileCtrl;
	PolicyRuleSet	*ruleSet;

	text = _("modified");

	profileCtrl = ProfileCtrl::instance();
	ruleSet = profileCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->setModified(true);
		if (ruleSet->hasErrors()) {
			text += _(" but has errors");
		}
	}

	controlRuleSetStatusText->SetLabel(text);
	controlRuleSetSaveButton->Enable();
}

ANEVENTS_IDENT_BCAST_METHOD_DEFINITION(DlgRuleEditor)

#endif
