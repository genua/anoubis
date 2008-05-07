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
	ruleSet_ = NULL;

	columnNames_[RULEDITOR_LIST_COLUMN_PRIO] = _("ID");
	columnWidths_[RULEDITOR_LIST_COLUMN_PRIO] = wxLIST_AUTOSIZE;

	columnNames_[RULEDITOR_LIST_COLUMN_APP] = _("Application");
	columnWidths_[RULEDITOR_LIST_COLUMN_APP] = wxLIST_AUTOSIZE;

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

	for (int i=0; i<RULEDITOR_LIST_COLUMN_EOL; i++) {
		ruleListCtrl->InsertColumn(i, columnNames_[i],
		    wxLIST_FORMAT_LEFT, columnWidths_[i]);
	}

	shortcuts_ = new AnShortcuts(this);

	Connect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(DlgRuleEditor::OnShow));
	Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::OnLoadRuleSet), NULL, this);
}

DlgRuleEditor::~DlgRuleEditor(void)
{
	delete shortcuts_;
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
DlgRuleEditor::OnBinaryModifyButtonClick(wxCommandEvent& event)
{
	wxString	caption = _("Choose a binary");
	wxString	wildcard = wxT("*");
	wxString	defaultDir = wxT("/usr/bin/");
	wxString	defaultFilename = wxEmptyString;
	wxFileDialog	fileDlg(NULL, caption, defaultDir, defaultFilename,
			    wildcard, wxOPEN);

	if (fileDlg.ShowModal() == wxID_OK) {
		appBinaryTextCtrl->Clear();
		appBinaryTextCtrl->AppendText(fileDlg.GetPath());
	}
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
	AlfPolicy *policy;

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setAction(APN_ACTION_ALLOW);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_ACTION,
	    policy->getActionName());
}

void
DlgRuleEditor::OnAlfDenyRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setAction(APN_ACTION_DENY);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_ACTION,
	    policy->getActionName());
}

void
DlgRuleEditor::OnAlfAskRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;

	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setAction(APN_ACTION_ASK);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_ACTION,
	    policy->getActionName());
}

void
DlgRuleEditor::OnAlfFilterRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setType(APN_ALF_FILTER);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_TYPE,
	    policy->getTypeName());
}

void
DlgRuleEditor::OnAlfCapRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setType(APN_ALF_CAPABILITY);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_TYPE,
	    policy->getTypeName());
}

void
DlgRuleEditor::OnAlfDefaultRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setType(APN_ALF_DEFAULT);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_TYPE,
	    policy->getTypeName());
}

void
DlgRuleEditor::OnAlfTcpRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setProtocol(IPPROTO_TCP);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_PROTO,
	    policy->getProtocolName());
}

void
DlgRuleEditor::OnAlfUdpRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setProtocol(IPPROTO_UDP);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_PROTO,
	    policy->getProtocolName());
}

void
DlgRuleEditor::OnAlfInetRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setAddrFamily(AF_INET);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_AF,
	    policy->getAddrFamilyName());
}

void
DlgRuleEditor::OnAlfInet6RadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setAddrFamily(AF_INET6);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_AF,
	    policy->getAddrFamilyName());
}

void
DlgRuleEditor::OnAlfAnyRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setAddrFamily(0);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_AF,
	    policy->getAddrFamilyName());
}

void
DlgRuleEditor::OnAlfRawCapRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setCapType(APN_ALF_CAPRAW);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_CAP,
	    policy->getCapTypeName());
}

void
DlgRuleEditor::OnAlfOtherCapRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setCapType(APN_ALF_CAPOTHER);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_CAP,
	    policy->getCapTypeName());
}

void
DlgRuleEditor::OnAlfAllCapRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setCapType(APN_ALF_CAPALL);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_CAP,
	    policy->getCapTypeName());
}

void
DlgRuleEditor::OnAlfAcceptRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setDirection(APN_ACCEPT);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_DIR,
	    policy->getDirectionName());
}

void
DlgRuleEditor::OnAlfConnectRadioButton(wxCommandEvent& event)
{
	AlfPolicy *policy;
	policy = (AlfPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;
	policy->setDirection(APN_CONNECT);
	ruleListCtrl->SetItem(selectedId_, RULEDITOR_LIST_COLUMN_DIR,
	    policy->getDirectionName());
}

void
DlgRuleEditor::OnSfsBinaryModifyButton(wxCommandEvent& event)
{
	SfsPolicy	*policy;
	policy = (SfsPolicy *)ruleListCtrl->GetItemData(selectedId_);
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

	policy = (SfsPolicy *)ruleListCtrl->GetItemData(selectedId_);
	if (!policy)
		return;

	policy->calcCurrentHash(csum);
	policy->setHashValue(csum);
	updateVisitor.setPropagation(false);
	policy->accept(updateVisitor);
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

#include <stdio.h>
void
DlgRuleEditor::OnLineSelected(wxListEvent& event)
{
	RuleEditorFillWidgetsVisitor	 updateVisitor(this);
	Policy				*policy;

	selectedId_ = event.GetIndex();

	updateVisitor.setPropagation(false);
	policy = (Policy *)event.GetData();
	if (!policy)
		return;
	policy->accept(updateVisitor);

	fprintf(stderr, "DlgRuleEditor::OnLineSelected id=%ld\n", selectedId_);
}

void
DlgRuleEditor::OnSrcAddrAddButton(wxCommandEvent& event)
{
	fprintf(stderr, "DlgRuleEditor::OnSrcAddrAddButton\n");
}

void
DlgRuleEditor::OnCreationChoice(wxCommandEvent& event)
{
	/* keep this in sync with controlCreationChoice elements */
	switch (event.GetSelection()) {
	case 0:
		ruleSet_->insertAlfPolicy(selectedId_);
		break;
	case 1:
		ruleSet_->insertSfsPolicy(selectedId_);
		break;
	case 2:
		ruleSet_->insertVarPolicy(selectedId_);
		break;
	default:
		break;
	}

	fprintf(stderr, "DlgRuleEditor::OnCreationChoice %d\n", event.GetSelection());
	loadRuleSet(ruleSet_);
}
