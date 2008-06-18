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
	ruleSet_ = NULL;

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

	parent->Connect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(DlgRuleEditor::OnShow), NULL, this);
	parent->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::OnLoadRuleSet), NULL, this);
	parent->Connect(anEVT_SHOW_RULE,
	    wxCommandEventHandler(DlgRuleEditor::OnShowRule), NULL, this);
	parent->Connect(anEVT_SEND_AUTO_CHECK,
	    wxCommandEventHandler(DlgRuleEditor::OnAutoCheck), NULL, this);
}

DlgRuleEditor::~DlgRuleEditor(void)
{
	delete shortcuts_;
}

void
DlgRuleEditor::updateAppName(wxString appName)
{
	AppPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedId_);
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
	AppPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedId_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AppPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	policy->setBinaryName(binName);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateAction(int action)
{
	AlfPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedId_);
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
	RuleEditorFillTableVisitor	 updateTable(this, selectedId_);
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
	RuleEditorFillTableVisitor	 updateTable(this, selectedId_);
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
	RuleEditorFillTableVisitor	 updateTable(this, selectedId_);
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
	RuleEditorFillTableVisitor	 updateTable(this, selectedId_);
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
	RuleEditorFillTableVisitor	 updateTable(this, selectedId_);
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
	RuleEditorFillTableVisitor	 updateTable(this, selectedId_);
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
	RuleEditorFillTableVisitor       updateTable(this, selectedId_);
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
	RuleEditorFillTableVisitor       updateTable(this, selectedId_);
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
	RuleEditorFillTableVisitor       updateTable(this, selectedId_);
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
	RuleEditorFillTableVisitor       updateTable(this, selectedId_);
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
DlgRuleEditor::OnTableOptionButtonClick(wxCommandEvent& event)
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
DlgRuleEditor::OnAppNameComboBox(wxCommandEvent& event)
{
	updateAppName(appNameComboBox->GetValue());
}

void
DlgRuleEditor::OnAppBinaryTextCtrl(wxCommandEvent& event)
{
	updateBinName(appBinaryTextCtrl->GetValue());
}

void
DlgRuleEditor::OnAppBinaryModifyButton(wxCommandEvent& event)
{
	wxString	 caption = _("Choose a binary");
	wxString	 wildcard = wxT("*");
	wxString	 defaultDir = wxT("/usr/bin/");
	wxString	 defaultFilename = wxEmptyString;
	wxFileDialog	*fileDlg;

	wxBeginBusyCursor();
	fileDlg = new wxFileDialog(NULL, caption, defaultDir, defaultFilename,
	    wildcard, wxOPEN);
	wxEndBusyCursor();

	if (fileDlg->ShowModal() == wxID_OK) {
		updateBinName(fileDlg->GetPath());
	}
}

void
DlgRuleEditor::OnAppUpdateChkSumButton(wxCommandEvent& event)
{
	AppPolicy			*policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedId_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);
	unsigned char			 csum[MAX_APN_HASH_LEN];

	policy = (AppPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	policy->calcCurrentHash(csum);
	policy->setHashValue(csum);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::OnAppValidateChkSumButton(wxCommandEvent& event)
{
	AppPolicy			*policy;
	RuleEditorFillWidgetsVisitor	 updateVisitor(this);
	wxString			 currHash;
	unsigned char			 csum[MAX_APN_HASH_LEN];

	policy = (AppPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	if (policy->calcCurrentHash(csum)) {
		currHash = wxT("0x");
		for (unsigned int i=0; i<MAX_APN_HASH_LEN; i++) {
			currHash += wxString::Format(wxT("%2.2x"),
			(unsigned char)csum[i]);
		}
	} else {
		currHash = _("unable to calculate checksum");
	}
	policy->setCurrentHash(currHash);
	updateVisitor.setPropagation(false);
	policy->accept(updateVisitor);
}

void
DlgRuleEditor::OnRuleCreateButton(wxCommandEvent& event)
{
	int id = -1;

	switch (controlCreationChoice->GetSelection()) {
	case 0: /* Application */
		id = ruleSet_->createAppPolicy(selectedId_);
		break;
	case 1: /* ALF */
		id = ruleSet_->createAlfPolicy(selectedId_);
		break;
	case 2: /* SFS */
		id = ruleSet_->createSfsPolicy(selectedId_);
		break;
	case 3: /* Variable */
		id = ruleSet_->createVarPolicy(selectedId_);
		break;
	default:
		break;
	}

	if (id == -1) {
		wxMessageBox(_("Could not create new rule."), _("Error"),
		    wxOK, this);
	} else {
		loadRuleSet(ruleSet_);
	}
}

void
DlgRuleEditor::OnRuleDeleteButton(wxCommandEvent& event)
{
}

void
DlgRuleEditor::OnRuleSetSave(wxCommandEvent& event)
{
	wxString	tmpPreFix;
	wxString	tmpName;

	/* XXX: KM there should be a better way, like apn_parse_xxx */
	tmpPreFix = wxT("xanoubis");
	tmpName = wxFileName::CreateTempFileName(tmpPreFix);
	wxGetApp().exportPolicyFile(tmpName);
	wxGetApp().usePolicy(tmpName);
}

void
DlgRuleEditor::OnClose(wxCloseEvent& event)
{
	wxCommandEvent  showEvent(anEVT_RULEEDITOR_SHOW);
	showEvent.SetInt(false);
	wxGetApp().sendEvent(showEvent);
}

void
DlgRuleEditor::OnAlfAllowRadioButton(wxCommandEvent& event)
{
	updateAction(APN_ACTION_ALLOW);
}

void
DlgRuleEditor::OnAlfDenyRadioButton(wxCommandEvent& event)
{
	updateAction(APN_ACTION_DENY);
}

void
DlgRuleEditor::OnAlfAskRadioButton(wxCommandEvent& event)
{
	updateAction(APN_ACTION_ASK);
}

void
DlgRuleEditor::OnAlfFilterRadioButton(wxCommandEvent& event)
{
	updateType(APN_ALF_FILTER);
}

void
DlgRuleEditor::OnAlfCapRadioButton(wxCommandEvent& event)
{
	updateType(APN_ALF_CAPABILITY);
}

void
DlgRuleEditor::OnAlfDefaultRadioButton(wxCommandEvent& event)
{
	updateType(APN_ALF_DEFAULT);
}

void
DlgRuleEditor::OnAlfTcpRadioButton(wxCommandEvent& event)
{
	updateProtocol(IPPROTO_TCP);
}

void
DlgRuleEditor::OnAlfUdpRadioButton(wxCommandEvent& event)
{
	updateProtocol(IPPROTO_UDP);
}

void
DlgRuleEditor::OnAlfInetRadioButton(wxCommandEvent& event)
{
	updateAddrFamily(AF_INET);
}

void
DlgRuleEditor::OnAlfInet6RadioButton(wxCommandEvent& event)
{
	updateAddrFamily(AF_INET6);
}

void
DlgRuleEditor::OnAlfAnyRadioButton(wxCommandEvent& event)
{
	updateAddrFamily(0);
}

void
DlgRuleEditor::OnAlfRawCapRadioButton(wxCommandEvent& event)
{
	updateCapType(APN_ALF_CAPRAW);
}

void
DlgRuleEditor::OnAlfOtherCapRadioButton(wxCommandEvent& event)
{
	updateCapType(APN_ALF_CAPOTHER);
}

void
DlgRuleEditor::OnAlfAllCapRadioButton(wxCommandEvent& event)
{
	updateCapType(APN_ALF_CAPALL);
}

void
DlgRuleEditor::OnAlfAcceptRadioButton(wxCommandEvent& event)
{
	updateDirection(APN_ACCEPT);
}

void
DlgRuleEditor::OnAlfConnectRadioButton(wxCommandEvent& event)
{
	updateDirection(APN_CONNECT);
}

void
DlgRuleEditor::OnSfsBinaryModifyButton(wxCommandEvent& event)
{
	SfsPolicy	*policy;
	policy = (SfsPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;
	wxString	 caption = _("Choose a binary");
	wxString	 wildcard = wxT("*");
	wxString	 defaultDir = wxT("/usr/bin/");
	wxString	 defaultFilename = policy->getBinaryName();
	wxFileDialog	 fileDlg(NULL, caption, defaultDir, defaultFilename,
			    wildcard, wxOPEN);

	if (fileDlg.ShowModal() == wxID_OK) {
		policy->setBinaryName(fileDlg.GetPath());
		sfsBinaryTextCtrl->Clear();
		sfsBinaryTextCtrl->AppendText(policy->getBinaryName());
		ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_BIN,
		    policy->getBinaryName());
	}
}

void
DlgRuleEditor::OnSfsUpdateChkSumButton(wxCommandEvent& event)
{
	RuleEditorFillWidgetsVisitor	 updateVisitor(this);
	SfsPolicy			*policy;
	unsigned char			 csum[MAX_APN_HASH_LEN];

	policy = (SfsPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	policy->calcCurrentHash(csum);
	policy->setHashValue(csum);
	updateVisitor.setPropagation(false);
	policy->accept(updateVisitor);

	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_HASH,
	   policy->getHashValue());
}

void
DlgRuleEditor::OnSfsValidateChkSumButton(wxCommandEvent& event)
{
	SfsPolicy			*policy;
	wxString			 currHash;
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);
	unsigned char			 csum[MAX_APN_HASH_LEN];

	policy = (SfsPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	if (policy->calcCurrentHash(csum)) {
		currHash = wxT("0x");
		for (unsigned int i=0; i<MAX_APN_HASH_LEN; i++) {
			currHash += wxString::Format(wxT("%2.2x"),
			(unsigned char)csum[i]);
		}
	} else {
		currHash = _("unable to calculate checksum");
	}
	policy->setCurrentHash(currHash);
	updateWidgets.setPropagation(false);
	policy->accept(updateWidgets);

}

void
DlgRuleEditor::OnShow(wxCommandEvent& event)
{
	this->Show(event.GetInt());
}

void
DlgRuleEditor::loadRuleSet(PolicyRuleSet *ruleSet)
{
	RuleEditorAddPolicyVisitor	 addVisitor(this);

	ruleListCtrl->DeleteAllItems();
	/* we just remember the ruleSet for further usage */
	ruleSet_ = ruleSet;
	ruleSet_->accept(addVisitor);

	/* trigger new calculation of column width */
	for (int i=0; i<RULEDITOR_LIST_COLUMN_EOL; i++) {
		ruleListCtrl->SetColumnWidth(i, columnWidths_[i]);
	}

}

void
DlgRuleEditor::OnLoadRuleSet(wxCommandEvent& event)
{
	PolicyRuleSet *ruleSet;

	ruleSet = (PolicyRuleSet *)event.GetClientData();
	loadRuleSet(ruleSet);
}

void
DlgRuleEditor::OnLineSelected(wxListEvent& event)
{
	RuleEditorFillWidgetsVisitor	 updateVisitor(this);
	Policy				*policy;
	wxListView			*selecter;

	if (autoCheck_) {
		if (!CheckLastSelection()) {
			selecter = (wxListView *)ruleListCtrl;
			selecter->Focus(selectedId_);
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
DlgRuleEditor::OnSrcAddrAddButton(wxCommandEvent& event)
{
}

void
DlgRuleEditor::OnAlfStateTimeoutChange(wxSpinEvent& event)
{
	updateTimeout(event.GetPosition());
}

void
DlgRuleEditor::OnAlfSrcAddrComboBox(wxCommandEvent& event)
{
	int af = alfInet6RadioButton->GetValue() ? AF_INET6 : AF_INET;

	if (alfAnyRadioButton->GetValue()) {
		updateAddrFamily(AF_INET);
	}

	updateAlfSrcAddr(alfSrcAddrComboBox->GetValue(),
	    alfSrcAddrNetSpinCtrl->GetValue(), af);
}

void
DlgRuleEditor::OnAlfDstAddrComboBox(wxCommandEvent& event)
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
DlgRuleEditor::OnAlfSrcPortComboBox(wxCommandEvent& event)
{
	unsigned long int	port;

	if ((alfSrcPortComboBox->GetValue()).ToULong(&port)) {
		updateAlfSrcPort(port);
	}
}

void
DlgRuleEditor::OnAlfDstPortComboBox(wxCommandEvent& event)
{
	unsigned long int       port;

	if (alfDstPortComboBox->GetValue().ToULong(&port)) {
		updateAlfDstPort(port);
	}
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

void
DlgRuleEditor::SetRuleSetToNotModified(void)
{
	Policy				*policy;
	AppPolicy			*appPolicy;
	SfsPolicy			*sfsPolicy;
	AlfPolicy			*alfPolicy;
	long				 iterator;

	for (iterator = 0; iterator < ruleListCtrl->GetItemCount(); iterator++)
	{
		policy = (Policy *)ruleListCtrl->GetItemData(iterator);
		if (!policy)
			return;

		if (policy->IsKindOf(CLASSINFO(SfsPolicy))) {
			sfsPolicy = (SfsPolicy *)policy;
			sfsPolicy->setModified(false);
		} else {
			if (policy->IsKindOf(CLASSINFO(AlfPolicy))) {
				alfPolicy = (AlfPolicy *)policy;
				appPolicy = (AppPolicy *)alfPolicy->getParent();
			} else {
				appPolicy = (AppPolicy *)policy;
			}
			appPolicy->setModified(false);
		}

	}
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
	if (!policy)
		return (false);

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
		sfsPolicy->setCurrentHash(currHash);
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
		appPolicy->setCurrentHash(currHash);
		regHash = appPolicy->getHashValue();
		mismatch = regHash.Cmp(currHash);
	}

	if (mismatch) {
		message = _("Checksums don't match for this Rule.\n \
		    Back to Rule");
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
