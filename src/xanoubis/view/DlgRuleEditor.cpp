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

#include <wx/filedlg.h>
#include <wx/choicdlg.h>
#include <wx/msgdlg.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <apn.h>

#include "AnEvents.h"
#include "AnShortcuts.h"
#include "AnEvents.h"
#include "DlgRuleEditor.h"
#include "main.h"
#include "Policy.h"
#include "PolicyRuleSet.h"
#include "RuleEditorAddPolicyVisitor.h"
#include "RuleEditorFillWidgetsVisitor.h"
#include "RuleEditorFillTableVisitor.h"

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
	sizer_->Remove(lead_);
	sizer_->Remove(addr_);
	sizer_->Remove(delimiter_);
	sizer_->Remove(net_);
	sizer_->Remove(remove_);
	sizer_->Remove(add_);
	sizer_->Fit(parent_);
	sizer_->Layout();
}

DlgRuleEditor::DlgRuleEditor(wxWindow* parent) : DlgRuleEditorBase(parent)
{
	selectedId_ = 0;
	selectedIndex_ = 0;
	autoCheck_ = false;
	userRuleSet_ = NULL;
	adminRuleSet_ = NULL;

	columnNames_[RULEDITOR_LIST_COLUMN_PRIO] = _("ID");
	columnWidths_[RULEDITOR_LIST_COLUMN_PRIO] = wxLIST_AUTOSIZE;

	columnNames_[RULEDITOR_LIST_COLUMN_RULE] = _("Rule");
	columnWidths_[RULEDITOR_LIST_COLUMN_RULE] = wxLIST_AUTOSIZE;

	columnNames_[RULEDITOR_LIST_COLUMN_APP] = _("Application");
	columnWidths_[RULEDITOR_LIST_COLUMN_APP] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_BIN] = _("Binary");
	columnWidths_[RULEDITOR_LIST_COLUMN_BIN] = wxLIST_AUTOSIZE;

	columnNames_[RULEDITOR_LIST_COLUMN_HASHT] = _("Hash-Type");
	columnWidths_[RULEDITOR_LIST_COLUMN_HASHT] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_HASH] = _("Hash");
	columnWidths_[RULEDITOR_LIST_COLUMN_HASH] = 80;

	columnNames_[RULEDITOR_LIST_COLUMN_CTX] = _("Context");
	columnWidths_[RULEDITOR_LIST_COLUMN_CTX] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_TYPE] = _("Type");
	columnWidths_[RULEDITOR_LIST_COLUMN_TYPE] = wxLIST_AUTOSIZE;

	columnNames_[RULEDITOR_LIST_COLUMN_ACTION] = _("Action");
	columnWidths_[RULEDITOR_LIST_COLUMN_ACTION] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_LOG] = _("Log");
	columnWidths_[RULEDITOR_LIST_COLUMN_LOG] = wxLIST_AUTOSIZE;

	columnNames_[RULEDITOR_LIST_COLUMN_AF] = _("AF");
	columnWidths_[RULEDITOR_LIST_COLUMN_AF] = wxLIST_AUTOSIZE;

	columnNames_[RULEDITOR_LIST_COLUMN_CAP] = _("Capability");
	columnWidths_[RULEDITOR_LIST_COLUMN_CAP] = wxLIST_AUTOSIZE;

	columnNames_[RULEDITOR_LIST_COLUMN_PROTO] = _("Protocol");
	columnWidths_[RULEDITOR_LIST_COLUMN_PROTO] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_DIR] = _("Direction");
	columnWidths_[RULEDITOR_LIST_COLUMN_DIR] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_FHOST] = _("From Host");
	columnWidths_[RULEDITOR_LIST_COLUMN_FHOST] = wxLIST_AUTOSIZE;

	columnNames_[RULEDITOR_LIST_COLUMN_FPORT] = _("From Port");
	columnWidths_[RULEDITOR_LIST_COLUMN_FPORT] = wxLIST_AUTOSIZE_USEHEADER;

	columnNames_[RULEDITOR_LIST_COLUMN_THOST] = _("To Host");
	columnWidths_[RULEDITOR_LIST_COLUMN_THOST] = wxLIST_AUTOSIZE;

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

	if (wxGetApp().hasRuleSet() == false) {
		this->controlRuleSetSaveButton->Disable();
		this->controlRuleCreateButton->Disable();
		this->controlRuleDeleteButton->Disable();
	}

	parent->Connect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(DlgRuleEditor::OnShow), NULL, this);
	parent->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::OnLoadRuleSet), NULL, this);
	parent->Connect(anEVT_SHOW_RULE,
	    wxCommandEventHandler(DlgRuleEditor::OnShowRule), NULL, this);
	parent->Connect(anEVT_SEND_AUTO_CHECK,
	    wxCommandEventHandler(DlgRuleEditor::OnAutoCheck), NULL, this);
	parent->Connect(anEVT_ANOUBISD_CSUM_CUR,
	    wxCommandEventHandler(DlgRuleEditor::OnAnoubisCurCsum), NULL, this);
	parent->Connect(anEVT_ANOUBISD_CSUM_SHA,
	    wxCommandEventHandler(DlgRuleEditor::OnAnoubisShaCsum), NULL, this);
	parent->Connect(anEVT_CHECKSUM_ERROR,
	    wxCommandEventHandler(DlgRuleEditor::OnChecksumError), NULL, this);
}

DlgRuleEditor::~DlgRuleEditor(void)
{
	delete shortcuts_;
}

void
DlgRuleEditor::updateAppName(wxString appName)
{
	AppPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AppPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setApplicationName(appName);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateBinName(wxString binName)
{
	Policy				*policy;
	AppPolicy			*appPolicy;
	SfsPolicy			*sfsPolicy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(SfsPolicy))) {
		sfsPolicy = (SfsPolicy *)policy;
		sfsPolicy->setBinaryName(binName);
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
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setStateTimeout(timeout);
	policy->accept(updateTable);
	/*
	 * ST: disabled as wxSpinCtrl::SetValue ends up in an endless event loop
	 * policy->accept(updateWidgets);
	 */
}

void
DlgRuleEditor::updateAlfSrcAddr(wxString address, int netmask, int af)
{
	AlfPolicy                       *policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor     updateWidgets(this);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setAlfSrcAddress(address, netmask, af);
	policy->accept(updateTable);
	/*
	 * ST: disabled as wxSpinCtrl::SetValue ends up in an endless event loop
	 * policy->accept(updateWidgets);
	 */
}

void
DlgRuleEditor::updateAlfDstAddr(wxString address, int netmask, int af)
{
	AlfPolicy                       *policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor     updateWidgets(this);

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setAlfDstAddress(address, netmask, af);
	policy->accept(updateTable);
	/*
	 * ST: disabled as wxSpinCtrl::SetValue ends up in an endless event loop
	 * policy->accept(updateWidgets);
	 */
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
	/* policy->accept(updateWidgets); */
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
	/* policy->accept(updateWidgets); */
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
}

void
DlgRuleEditor::OnCommonLogLog(wxCommandEvent& )
{
	updateLog(APN_LOG_NORMAL);
}

void
DlgRuleEditor::OnCommonLogAlert(wxCommandEvent& )
{
	updateLog(APN_LOG_ALERT);
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
DlgRuleEditor::OnAppNameComboBox(wxCommandEvent& )
{
	updateAppName(appNameComboBox->GetValue());
}

void
DlgRuleEditor::OnAppBinaryTextCtrl(wxCommandEvent& )
{
	updateBinName(appBinaryTextCtrl->GetValue());
}

void
DlgRuleEditor::OnSfsBinaryTextCtrl(wxCommandEvent& )
{
	updateBinName(sfsBinaryTextCtrl->GetValue());
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
	}
}

void
DlgRuleEditor::OnAppUpdateChkSumButton(wxCommandEvent& )
{
	AppPolicy			*policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);
	unsigned char			 *csum;
	wxString			 curHash;

	if (wxIsBusy())
		return;

	policy = (AppPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	csum = policy->getCurrentSum();
	policy->setHashValue(csum);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::OnAppValidateChkSumButton(wxCommandEvent& )
{
	AppPolicy			*policy;
	RuleEditorFillWidgetsVisitor	 updateVisitor(this);
	unsigned char			 csum[MAX_APN_HASH_LEN];
	wxString			 currHash;
	wxString			 message;
	wxString			 title;
	int				 ret;

	if (wxIsBusy())
		return;

	policy = (AppPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	ret = policy->calcCurrentHash(csum);

	if (ret == 1) {
		updateVisitor.setPropagation(false);
		policy->accept(updateVisitor);
	} else if (ret == 0) {
		wxGetApp().calChecksum(policy->getBinaryName());
		wxBeginBusyCursor();
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

void
DlgRuleEditor::OnRuleCreateButton(wxCommandEvent& )
{
	int id = -1;

	switch (controlCreationChoice->GetSelection()) {
	case 0: /* Application */
		id = userRuleSet_->createAppPolicy(selectedId_);
		break;
	case 1: /* ALF */
		id = userRuleSet_->createAlfPolicy(selectedId_);
		break;
	case 2: /* SFS */
		id = userRuleSet_->createSfsPolicy(selectedId_);
		break;
	case 3: /* Variable */
		id = userRuleSet_->createVarPolicy(selectedId_);
		break;
	default:
		break;
	}

	if (id == -1) {
		wxMessageBox(_("Could not create new rule."), _("Error"),
		    wxOK, this);
	} else {
		loadRuleSet(userRuleSet_);
	}
}

void
DlgRuleEditor::OnRuleDeleteButton(wxCommandEvent& )
{
	userRuleSet_->deletePolicy(selectedId_);
}

void
DlgRuleEditor::OnClose(wxCloseEvent& )
{
	wxCommandEvent  showEvent(anEVT_RULEEDITOR_SHOW);
	showEvent.SetInt(false);
	wxGetApp().sendEvent(showEvent);
}

void
DlgRuleEditor::OnAlfAllowRadioButton(wxCommandEvent& )
{
	updateAction(APN_ACTION_ALLOW);
}

void
DlgRuleEditor::OnAlfDenyRadioButton(wxCommandEvent& )
{
	updateAction(APN_ACTION_DENY);
}

void
DlgRuleEditor::OnAlfAskRadioButton(wxCommandEvent& )
{
	updateAction(APN_ACTION_ASK);
}

void
DlgRuleEditor::OnAlfFilterRadioButton(wxCommandEvent& )
{
	updateType(APN_ALF_FILTER);
}

void
DlgRuleEditor::OnAlfCapRadioButton(wxCommandEvent& )
{
	updateType(APN_ALF_CAPABILITY);
}

void
DlgRuleEditor::OnAlfDefaultRadioButton(wxCommandEvent& )
{
	updateType(APN_ALF_DEFAULT);
}

void
DlgRuleEditor::OnAlfTcpRadioButton(wxCommandEvent& )
{
	updateProtocol(IPPROTO_TCP);
}

void
DlgRuleEditor::OnAlfUdpRadioButton(wxCommandEvent& )
{
	updateProtocol(IPPROTO_UDP);
}

void
DlgRuleEditor::OnAlfInetRadioButton(wxCommandEvent& )
{
	updateAddrFamily(AF_INET);
}

void
DlgRuleEditor::OnAlfInet6RadioButton(wxCommandEvent& )
{
	updateAddrFamily(AF_INET6);
}

void
DlgRuleEditor::OnAlfAnyRadioButton(wxCommandEvent& )
{
	updateAddrFamily(0);
}

void
DlgRuleEditor::OnAlfRawCapRadioButton(wxCommandEvent& )
{
	updateCapType(APN_ALF_CAPRAW);
}

void
DlgRuleEditor::OnAlfOtherCapRadioButton(wxCommandEvent& )
{
	updateCapType(APN_ALF_CAPOTHER);
}

void
DlgRuleEditor::OnAlfAllCapRadioButton(wxCommandEvent& )
{
	updateCapType(APN_ALF_CAPALL);
}

void
DlgRuleEditor::OnAlfAcceptRadioButton(wxCommandEvent& )
{
	updateDirection(APN_ACCEPT);
}

void
DlgRuleEditor::OnAlfConnectRadioButton(wxCommandEvent& )
{
	updateDirection(APN_CONNECT);
}

void
DlgRuleEditor::OnAlfBothRadioButton(wxCommandEvent& )
{
	updateDirection(APN_BOTH);
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
		break;
	case  0:
		wxGetApp().calChecksum(policy->getBinaryName());
		wxBeginBusyCursor();
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

void
DlgRuleEditor::OnShow(wxCommandEvent& event)
{
	this->Show(event.GetInt());
}

void
DlgRuleEditor::OnRuleSetSave(wxCommandEvent& )
{
	wxString	tmpPreFix;
	wxString	tmpName;
	wxString	message;

	if (userRuleSet_->findMismatchHash())
	{
		message = _("Mismatch of Checksums in one or more Rule.\n");
		message += _("Do you want to store anyway?");
		int answer = wxMessageBox(message,
		    _("Mismatch of Checksums"), wxYES_NO, this);
		if (answer == wxNO) {
			return;
		}
	}

	userRuleSet_->clearModified();

	/* XXX: KM there should be a better way, like apn_parse_xxx */
	tmpPreFix = wxT("xanoubis");
	tmpName = wxFileName::CreateTempFileName(tmpPreFix);
	wxGetApp().exportPolicyFile(tmpName);
	wxGetApp().usePolicy(tmpName);
}

void
DlgRuleEditor::loadRuleSet(PolicyRuleSet *ruleSet)
{
	RuleEditorAddPolicyVisitor	 addVisitor(this);

	ruleListCtrl->DeleteAllItems();
	/* we just remember the ruleSet for further usage */
	if (ruleSet->isReadOnly()) {
		adminRuleSet_ = ruleSet;
	} else {
		userRuleSet_ = ruleSet;
	}
	if (userRuleSet_ != NULL) {
		addVisitor.setAdmin(false);
		userRuleSet_->accept(addVisitor);
	}
	if (adminRuleSet_!= NULL) {
		addVisitor.setAdmin(true);
		adminRuleSet_->accept(addVisitor);
	}

	/* trigger new calculation of column width */
	for (int i=0; i<RULEDITOR_LIST_COLUMN_EOL; i++) {
		ruleListCtrl->SetColumnWidth(i, columnWidths_[i]);
	}
}

void
DlgRuleEditor::OnLoadRuleSet(wxCommandEvent& event)
{
	PolicyRuleSet *ruleSet;

	this->controlRuleSetSaveButton->Enable();
	this->controlRuleCreateButton->Enable();
	this->controlRuleDeleteButton->Enable();

	ruleSet = (PolicyRuleSet *)event.GetClientData();
	loadRuleSet(ruleSet);
	selectLine(selectedIndex_);
}

void
DlgRuleEditor::OnLineSelected(wxListEvent& event)
{
	RuleEditorFillWidgetsVisitor	 updateVisitor(this);
	Policy				*policy;
	wxListView			*selecter;

	if (wxIsBusy())
		return;

	if (autoCheck_) {
		if (!CheckLastSelection()) {
			selecter = (wxListView *)ruleListCtrl;
			selecter->Focus(selectedIndex_);
			return;
		}
	}
	selectedIndex_ = event.GetIndex();
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
}

void
DlgRuleEditor::OnAlfSrcAddrComboBox(wxCommandEvent& )
{
	int af = alfInet6RadioButton->GetValue() ? AF_INET6 : AF_INET;

	if (alfAnyRadioButton->GetValue()) {
		updateAddrFamily(AF_INET);
	}

	updateAlfSrcAddr(alfSrcAddrComboBox->GetValue(),
	    alfSrcAddrNetSpinCtrl->GetValue(), af);
}

void
DlgRuleEditor::OnAlfDstAddrComboBox(wxCommandEvent& )
{
	if (alfAnyRadioButton->GetValue()) {
		updateAddrFamily(AF_INET);
	}

	int af = alfInet6RadioButton->GetValue() ? AF_INET6 : AF_INET;
	updateAlfDstAddr(alfDstAddrComboBox->GetValue(),
	    alfDstAddrNetSpinCtrl->GetValue(), af);
}

void
DlgRuleEditor::OnAlfSrcNetmaskSpinCtrl(wxSpinEvent& event)
{
	if (alfAnyRadioButton->GetValue()) {
		updateAddrFamily(AF_INET);
	}

	int af = alfInet6RadioButton->GetValue() ? AF_INET6 : AF_INET;
	updateAlfSrcAddr(alfSrcAddrComboBox->GetValue(),
	    event.GetPosition(), af);
}

void
DlgRuleEditor::OnAlfSrcNetmaskSpinCtrlText(wxCommandEvent&)
{
	if (alfAnyRadioButton->GetValue()) {
		updateAddrFamily(AF_INET);
	}

	int af = alfInet6RadioButton->GetValue() ? AF_INET6 : AF_INET;
	updateAlfSrcAddr(alfSrcAddrComboBox->GetValue(),
	    alfSrcAddrNetSpinCtrl->GetValue(), af);
}

void
DlgRuleEditor::OnAlfDstNetmaskSpinCtrl(wxSpinEvent& event)
{
	if (alfAnyRadioButton->GetValue()) {
		updateAddrFamily(AF_INET);
	}

	int af = alfInet6RadioButton->GetValue() ? AF_INET6 : AF_INET;
	updateAlfDstAddr(alfDstAddrComboBox->GetValue(),
	    event.GetPosition(), af);
}

void
DlgRuleEditor::OnAlfDstNetmaskSpinCtrlText(wxCommandEvent&)
{
	if (alfAnyRadioButton->GetValue()) {
		updateAddrFamily(AF_INET);
	}

	int af = alfInet6RadioButton->GetValue() ? AF_INET6 : AF_INET;
	updateAlfDstAddr(alfDstAddrComboBox->GetValue(),
	    alfDstAddrNetSpinCtrl->GetValue(), af);
}

void
DlgRuleEditor::OnAlfSrcPortComboBox(wxCommandEvent& )
{
	unsigned long int       port;

	if (!alfSrcPortComboBox->GetValue().Cmp(wxT("any"))) {
		port = 0;
	} else {
		alfSrcPortComboBox->GetValue().ToULong(&port);
	}
	updateAlfSrcPort(port);
}

void
DlgRuleEditor::OnAlfDstPortComboBox(wxCommandEvent&)
{
	unsigned long int       port;

	if (!alfDstPortComboBox->GetValue().Cmp(wxT("any"))) {
		port = 0;
	} else {
		alfDstPortComboBox->GetValue().ToULong(&port);
	}
	updateAlfDstPort(port);
}

void
DlgRuleEditor::OnShowRule(wxCommandEvent& event)
{
	wxListView *selecter;

	/* show RuleEditor Dialog and corresponding Ruleid entry */
	this->Show();
	selecter = (wxListView*)ruleListCtrl;
	selecter->Select(event.GetExtraLong());
}

void
DlgRuleEditor::OnAutoCheck(wxCommandEvent& event)
{
	autoCheck_ = event.GetInt();
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
			policy->accept(updateVisitor);
			return (false);
		}

	}
	return (true);
}

void
DlgRuleEditor::OnAnoubisCurCsum(wxCommandEvent& event)
{
	wxString			 sum;
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);
	unsigned char			 csum[MAX_APN_HASH_LEN];
	Policy				*policy;

	sum = wxT("0x");
	sum += event.GetString();
	bcopy((unsigned char *) event.GetClientData(), csum, MAX_APN_HASH_LEN);

	policy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

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

}

void
DlgRuleEditor::OnAnoubisShaCsum(wxCommandEvent& event)
{
	wxString			 sum;
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);
	unsigned char			 csum[MAX_APN_HASH_LEN];
	Policy				*policy;

	sum = wxT("0x");
	sum += event.GetString();
	bcopy((unsigned char *) event.GetClientData(), csum, MAX_APN_HASH_LEN);

	policy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	SfsPolicy *sfsPolicy = (SfsPolicy *)policy;

	if (sum.Cmp(sfsPolicy->getCurrentHash())) {
		/* if not matching update shadowtree */
		wxGetApp().sendChecksum(sfsPolicy->getBinaryName());
	}

	policy->accept(updateWidgets);
	wxEndBusyCursor();
}

void
DlgRuleEditor::OnChecksumError(wxCommandEvent& event)
{
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);
	SfsPolicy			*policy;
	wxString			 errMsg;
	wxString			 message;
	wxString			 title;

	policy = (SfsPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	errMsg = event.GetString();

	if ((errMsg.Cmp(_("add")) == 0) || (errMsg.Cmp(_("cal")) == 0)) {
		wxEndBusyCursor();
		message = _("Permission denied for this file.");
		title = _("Permission denied");
		wxMessageBox(message, title, wxICON_INFORMATION);
	} else if (errMsg.Cmp(_("notcon")) == 0) {
		message = _("Not connected to daemon.");
		title = _("Not connected");
		wxMessageBox(message, title, wxICON_INFORMATION);
	} else {
		policy->accept(updateWidgets);
	}

	wxEndBusyCursor();
}

void
DlgRuleEditor::selectLine(unsigned long index)
{
	wxListView			*selecter;

	selecter = (wxListView*)ruleListCtrl;
	selecter->Select(index);
}
