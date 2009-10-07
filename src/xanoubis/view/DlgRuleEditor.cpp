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

#include <wx/defs.h>	/* mandatory but missing in choicdlg.h */
#include <wx/choicdlg.h>
#include <wx/dynarray.h>
#include <wx/msgdlg.h>

#include <wx/arrimpl.cpp>
#include <wx/filedlg.h>
#include <wx/config.h>

#include "main.h"
#include "DlgRuleEditor.h"

#include "AlfAppPolicy.h"
#include "ContextAppPolicy.h"
#include "SfsAppPolicy.h"
#include "FilterPolicy.h"
#include "PolicyRuleSet.h"
#include "PolicyCtrl.h"
#include "RuleEditorAddPolicyVisitor.h"
#include "DlgRuleEditorPage.h"
#include "DlgRuleEditorAppPage.h"
#include "JobCtrl.h"
#include "AnPolicyNotebook.h"

#define	BINARY_COLUMN_WIDTH	200

DlgRuleEditor::DlgRuleEditor(wxWindow* parent)
    : Observer(NULL), DlgRuleEditorBase(parent)
{
	AnEvents *anEvents;

	anEvents = AnEvents::getInstance();

	anEvents->Connect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(DlgRuleEditor::onShow), NULL, this);
	anEvents->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::onLoadNewRuleSet), NULL, this);
	anEvents->Connect(anEVT_SHOW_RULE,
	    wxCommandEventHandler(DlgRuleEditor::onShowRule), NULL, this);
	anEvents->Connect(anEVT_SEND_AUTO_CHECK,
	    wxCommandEventHandler(DlgRuleEditor::onAutoCheck), NULL, this);

	adminRuleSetId_ = -1;
	userRuleSetId_ = -1;

	/* We want to know connect-status for save/reload button */
	JobCtrl::getInstance()->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(DlgRuleEditor::onConnectionStateChange),
	    NULL, this);

	appColumns_[APP_ID]	= new ListCtrlColumn(_("ID"), wxT("ID"));
	appColumns_[APP_TYPE]	= new ListCtrlColumn(_("Type"), wxT("TYPE"));
	appColumns_[APP_USER]	= new ListCtrlColumn(_("User"), wxT("USER"));
	appColumns_[APP_BINARY]	= new ListCtrlColumn(_("Binary"),
	    wxT("BINARY"), BINARY_COLUMN_WIDTH);

	for (size_t i=0; i<APP_EOL; i++) {
		appColumns_[i]->setIndex(i);
	}

	alfColumns_[ALF_ID]	= new ListCtrlColumn(_("ID"), wxT("ID"));
	alfColumns_[ALF_TYPE]	= new ListCtrlColumn(_("Type"), wxT("TYPE"));
	alfColumns_[ALF_ACTION]	= new ListCtrlColumn(_("Action"),
	    wxT("ACTION"));
	alfColumns_[ALF_LOG]	= new ListCtrlColumn(_("Log"), wxT("LOG"));
	alfColumns_[ALF_SCOPE]	= new ListCtrlColumn(_("Scope"), wxT("SCOPE"));
	alfColumns_[ALF_CAP]	= new ListCtrlColumn(_("Capability"),
	    wxT("CAP"));
	alfColumns_[ALF_DIR]	= new ListCtrlColumn(_("Direction"),
	    wxT("DIR"));
	alfColumns_[ALF_PROT]	= new ListCtrlColumn(_("Protocol"),
	    wxT("PROT"));
	alfColumns_[ALF_FHOST]	= new ListCtrlColumn(_("from host"),
	    wxT("FHOST"));
	alfColumns_[ALF_FPORT]	= new ListCtrlColumn(_("from port"),
	    wxT("FPORT"));
	alfColumns_[ALF_THOST]	= new ListCtrlColumn(_("to host"),
	    wxT("THOST"));
	alfColumns_[ALF_TPORT]	= new ListCtrlColumn(_("to port"),
	    wxT("TPORT"));

	for (size_t i=0; i<ALF_EOL; i++) {
		alfColumns_[i]->setIndex(i);
	}

	sfsColumns_[SFS_ID]	= new ListCtrlColumn(_("ID"), wxT("ID"));
	sfsColumns_[SFS_TYPE]	= new ListCtrlColumn(_("Type"), wxT("TYPE"));
	sfsColumns_[SFS_PATH]	= new ListCtrlColumn(_("Path"), wxT("PATH"));
	sfsColumns_[SFS_SUB]	= new ListCtrlColumn(_("Subject"), wxT("SUB"));
	sfsColumns_[SFS_SCOPE]	= new ListCtrlColumn(_("Scope"), wxT("SCOPE"));
	sfsColumns_[SFS_VA]	= new ListCtrlColumn(_("Valid action"),
	    wxT("VA"));
	sfsColumns_[SFS_VL]	= new ListCtrlColumn(_("Valid log"), wxT("VL"));
	sfsColumns_[SFS_IA]	= new ListCtrlColumn(_("Invalid action"),
	    wxT("IA"));
	sfsColumns_[SFS_IL]	= new ListCtrlColumn(_("Invalid log"),
	    wxT("IL"));
	sfsColumns_[SFS_UA]	= new ListCtrlColumn(_("Unknown action"),
	    wxT("UA"));
	sfsColumns_[SFS_UL]	= new ListCtrlColumn(_("Unknown log"),
	    wxT("UL"));

	for (size_t i=0; i<SFS_EOL; i++) {
		sfsColumns_[i]->setIndex(i);
	}

	ctxColumns_[CTX_ID]	= new ListCtrlColumn(_("ID"), wxT("ID"));
	ctxColumns_[CTX_TYPE]	= new ListCtrlColumn(_("Type"), wxT("TYPE"));
	ctxColumns_[CTX_BINARY]	= new ListCtrlColumn(_("Binary"),
	    wxT("BINARY"));

	for (size_t i=0; i<CTX_EOL; i++) {
		ctxColumns_[i]->setIndex(i);
	}

	sbColumns_[SB_ID]	= new ListCtrlColumn(_("ID"), wxT("ID"));
	sbColumns_[SB_TYPE]	= new ListCtrlColumn(_("Type"), wxT("TYPE"));
	sbColumns_[SB_ACTION]	= new ListCtrlColumn(_("Action"),
	    wxT("ACTION"));
	sbColumns_[SB_LOG]	= new ListCtrlColumn(_("Log"), wxT("LOG"));
	sbColumns_[SB_SCOPE]	= new ListCtrlColumn(_("Scope"), wxT("SCOPE"));
	sbColumns_[SB_PATH]	= new ListCtrlColumn(_("Path"), wxT("PATH"));
	sbColumns_[SB_SUB]	= new ListCtrlColumn(_("Subject"), wxT("SUB"));
	sbColumns_[SB_MASK]	= new ListCtrlColumn(_("Mask"), wxT("MASK"));

	for (size_t i=0; i<SB_EOL; i++) {
		sbColumns_[i]->setIndex(i);
	}

	isConnected_ = false;

	appPolicyLoadProgIdx_ = 0;
	appPolicyLoadProgDlg_ = NULL;
	filterPolicyLoadProgIdx_ = 0;
	filterPolicyLoadProgDlg_ = NULL;

	/* read and restore header selections for rule editor view */
	readOptions();

	editorUid_ = geteuid();
	if (editorUid_ != 0) {
		rb_userSelect->Disable();
	}
	tx_userSelect->Disable();
}

DlgRuleEditor::~DlgRuleEditor(void)
{
	AnEvents *anEvents;

	anEvents = AnEvents::getInstance();

	wipeFilterList();
	wipeAppList();

	anEvents->Disconnect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(DlgRuleEditor::onShow), NULL, this);
	anEvents->Disconnect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::onLoadNewRuleSet), NULL, this);
	anEvents->Disconnect(anEVT_SHOW_RULE,
	    wxCommandEventHandler(DlgRuleEditor::onShowRule), NULL, this);

	JobCtrl::getInstance()->Disconnect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(DlgRuleEditor::onConnectionStateChange),
	    NULL, this);

	/* write header selections for rule editor view to config file */
	writeOptions();

	for (size_t i=0; i<APP_EOL; i++) {
		delete appColumns_[i];
	}
	for (size_t i=0; i<ALF_EOL; i++) {
		delete alfColumns_[i];
	}
	for (size_t i=0; i<SFS_EOL; i++) {
		delete sfsColumns_[i];
	}
	for (size_t i=0; i<CTX_EOL; i++) {
		delete ctxColumns_[i];
	}
	for (size_t i=0; i<SB_EOL; i++) {
		delete sbColumns_[i];
	}
}

void
DlgRuleEditor::readOptions(void)
{
	/* read and set the stored column header settings from config file */
	bool		isVisible = true;
	wxString	name;

	for (size_t i=1; i<APP_EOL; i++) {
		name = wxT("/Options/Columns/App/") +
		    appColumns_[i]->getConfKey();
		wxConfig::Get()->Read(name, &isVisible, true);
		appColumns_[i]->setVisability(isVisible);
	}

	for (size_t i=1; i<ALF_EOL; i++) {
		name = wxT("/Options/Columns/Alf/") +
		    alfColumns_[i]->getConfKey();
		wxConfig::Get()->Read(name, &isVisible, true);
		alfColumns_[i]->setVisability(isVisible);
	}

	for (size_t i=1; i<SFS_EOL; i++) {
		name = wxT("/Options/Columns/Sfs/") +
		    sfsColumns_[i]->getConfKey();
		wxConfig::Get()->Read(name, &isVisible, true);
		sfsColumns_[i]->setVisability(isVisible);
	}

	for (size_t i=1; i<CTX_EOL; i++) {
		name = wxT("/Options/Columns/Ctx/") +
		    ctxColumns_[i]->getConfKey();
		wxConfig::Get()->Read(name, &isVisible, true);
		ctxColumns_[i]->setVisability(isVisible);
	}

	for (size_t i=1; i<SB_EOL; i++) {
		name = wxT("/Options/Columns/Sb/")
		    + sbColumns_[i]->getConfKey();
		wxConfig::Get()->Read(name, &isVisible, true);
		sbColumns_[i]->setVisability(isVisible);
	}

	name = wxT("/Options/AutoChecksumCheck");
	wxConfig::Get()->Read(name, &isAutoChecksumCheck_, true);

	updateListColumns(appPolicyListCtrl,    appColumns_, APP_EOL);
	updateListColumns(filterPolicyListCtrl, alfColumns_, ALF_EOL);
	updateListColumns(filterPolicyListCtrl, sfsColumns_, SFS_EOL);
	updateListColumns(filterPolicyListCtrl, ctxColumns_, CTX_EOL);
	updateListColumns(filterPolicyListCtrl, sbColumns_,  SB_EOL);
}

void
DlgRuleEditor::writeOptions(void)
{
	wxString	name;

	/* save the column header settings to config */
	for (size_t i=1; i<APP_EOL; i++) {
		name = wxT("/Options/Columns/App/") +
		    appColumns_[i]->getConfKey();
		wxConfig::Get()->Write(name, appColumns_[i]->isVisible());
	}

	for (size_t i=1; i<ALF_EOL; i++) {
		name = wxT("/Options/Columns/Alf/") +
		    alfColumns_[i]->getConfKey();
		wxConfig::Get()->Write(name, alfColumns_[i]->isVisible());
	}

	for (size_t i=1; i<SFS_EOL; i++) {
		name = wxT("/Options/Columns/Sfs/") +
		    sfsColumns_[i]->getConfKey();
		wxConfig::Get()->Write(name, sfsColumns_[i]->isVisible());
	}

	for (size_t i=1; i<CTX_EOL; i++) {
		name = wxT("/Options/Columns/Ctx/") +
		    ctxColumns_[i]->getConfKey();
		wxConfig::Get()->Write(name, ctxColumns_[i]->isVisible());
	}

	for (size_t i=1; i<SB_EOL; i++) {
		name = wxT("/Options/Columns/Sb/") +
		    sbColumns_[i]->getConfKey();
		wxConfig::Get()->Write(name, sbColumns_[i]->isVisible());
	}
}

void
DlgRuleEditor::update(Subject *subject)
{
	long		idx;
	AppPolicy	*parent;
	FilterPolicy	*filter;

	if (subject->IsKindOf(CLASSINFO(AppPolicy))) {
		idx = findListRow(appPolicyListCtrl, (Policy *)subject);
		if (idx != -1) {
			updateListAppPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		idx = findListRow(filterPolicyListCtrl, (Policy *)subject);
		if (idx != -1) {
			updateListAlfFilterPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(AlfCapabilityFilterPolicy))) {
		idx = findListRow(filterPolicyListCtrl, (Policy *)subject);
		if (idx != -1) {
			updateListAlfCapabilityFilterPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(SfsFilterPolicy))) {
		idx = findListRow(filterPolicyListCtrl, (Policy *)subject);
		if (idx != -1) {
			updateListSfsFilterPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(ContextFilterPolicy))) {
		idx = findListRow(filterPolicyListCtrl, (Policy *)subject);
		if (idx != -1) {
			updateListContextFilterPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(SbAccessFilterPolicy))) {
		idx = findListRow(filterPolicyListCtrl, (Policy *)subject);
		if (idx != -1) {
			updateListSbAccessFilterPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(DefaultFilterPolicy))) {
		idx = findListRow(filterPolicyListCtrl, (Policy *)subject);
		filter = wxDynamicCast(subject, FilterPolicy);
		parent = filter->getParentPolicy();
		if ((idx != -1) && (parent != NULL)) {
			if (parent->IsKindOf(CLASSINFO(AlfAppPolicy))) {
				updateListAlfFilterPolicy(idx);
			} else if (parent->IsKindOf(CLASSINFO(SbAppPolicy))) {
				updateListSbAccessFilterPolicy(idx);
			}
		}
	} else if (subject->IsKindOf(CLASSINFO(SfsDefaultFilterPolicy))) {
		idx = findListRow(filterPolicyListCtrl, (Policy *)subject);
		if (idx != -1) {
			updateListSfsFilterPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(PolicyRuleSet))) {
		updateFooter();
	} else {
		/* Unknown subject type - do nothing */
	}
}

void
DlgRuleEditor::updateDelete(Subject *subject)
{
	long		 idx = -1;

	idx = findListRow(appPolicyListCtrl, (Policy *)subject);
	if (idx != -1) {
		appPolicyListCtrl->SetItemPtrData(idx, (wxUIntPtr)0);
	} else {
		idx = findListRow(filterPolicyListCtrl, (Policy *)subject);
		if (idx != -1) {
			filterPolicyListCtrl->SetItemPtrData(idx, (wxUIntPtr)0);
		}
	}
}

void
DlgRuleEditor::addAppPolicy(AppPolicy *policy)
{
	long index;

	updateListColumns(appPolicyListCtrl, appColumns_, APP_EOL);
	index = addListRow(appPolicyListCtrl, policy);
	updateListAppPolicy(index);
	updateProgDlg(appPolicyLoadProgDlg_, &appPolicyLoadProgIdx_, policy);
}

void
DlgRuleEditor::addAlfFilterPolicy(AlfFilterPolicy *policy)
{
	long index;

	updateListColumns(filterPolicyListCtrl, alfColumns_, ALF_EOL);
	index = addListRow(filterPolicyListCtrl, policy);
	updateListAlfFilterPolicy(index);
	updateProgDlg(filterPolicyLoadProgDlg_, &filterPolicyLoadProgIdx_,
	    policy);
}

void
DlgRuleEditor::addAlfCapabilityFilterPolicy(AlfCapabilityFilterPolicy *policy)
{
	long index;

	updateListColumns(filterPolicyListCtrl, alfColumns_, ALF_EOL);
	index = addListRow(filterPolicyListCtrl, policy);
	updateListAlfCapabilityFilterPolicy(index);
	updateProgDlg(filterPolicyLoadProgDlg_, &filterPolicyLoadProgIdx_,
	    policy);
}

void
DlgRuleEditor::addDefaultFilterPolicy(DefaultFilterPolicy *policy)
{
	long		 index;
	AppPolicy	*parent;

	parent = policy->getParentPolicy();
	if (parent->IsKindOf(CLASSINFO(AlfAppPolicy))) {
		updateListColumns(filterPolicyListCtrl, alfColumns_, ALF_EOL);
		index = addListRow(filterPolicyListCtrl, policy);
		updateListAlfFilterPolicy(index);
		updateProgDlg(filterPolicyLoadProgDlg_,
		    &filterPolicyLoadProgIdx_, policy);
	} else if (parent->IsKindOf(CLASSINFO(SbAppPolicy))) {
		updateListColumns(filterPolicyListCtrl, sbColumns_, SB_EOL);
		index = addListRow(filterPolicyListCtrl, policy);
		updateListSbAccessFilterPolicy(index);
		updateProgDlg(filterPolicyLoadProgDlg_,
		    &filterPolicyLoadProgIdx_, policy);
	} else {
		/* This should never been reached. */
	}
}

void
DlgRuleEditor::addSfsFilterPolicy(SfsFilterPolicy *policy)
{
	long index;

	updateListColumns(filterPolicyListCtrl, sfsColumns_, SFS_EOL);
	index = addListRow(filterPolicyListCtrl, policy);
	updateListSfsFilterPolicy(index);
	updateProgDlg(filterPolicyLoadProgDlg_, &filterPolicyLoadProgIdx_,
	    policy);
}

void
DlgRuleEditor::addSfsDefaultFilterPolicy(SfsDefaultFilterPolicy *policy)
{
	long index;

	updateListColumns(filterPolicyListCtrl, sfsColumns_, SFS_EOL);
	index = addListRow(filterPolicyListCtrl, policy);
	updateListSfsFilterPolicy(index);
	updateProgDlg(filterPolicyLoadProgDlg_, &filterPolicyLoadProgIdx_,
	    policy);
}

void
DlgRuleEditor::addContextFilterPolicy(ContextFilterPolicy *policy)
{
	long index;

	updateListColumns(filterPolicyListCtrl, ctxColumns_, CTX_EOL);
	index = addListRow(filterPolicyListCtrl, policy);
	updateListContextFilterPolicy(index);
	updateProgDlg(filterPolicyLoadProgDlg_, &filterPolicyLoadProgIdx_,
	    policy);
}

void
DlgRuleEditor::addSbAccessFilterPolicy(SbAccessFilterPolicy *policy)
{
	long index;

	updateListColumns(filterPolicyListCtrl, sbColumns_, SB_EOL);
	index = addListRow(filterPolicyListCtrl, policy);
	updateListSbAccessFilterPolicy(index);
	updateProgDlg(filterPolicyLoadProgDlg_, &filterPolicyLoadProgIdx_,
	    policy);
}

/*
 * Private subroutines
 */

void
DlgRuleEditor::onShow(wxCommandEvent &event)
{
	if (IsShown() != event.GetInt()) {
		this->Show(event.GetInt());
	}
	event.Skip();
}

void
DlgRuleEditor::onClose(wxCloseEvent & WXUNUSED(event))
{
	wxCommandEvent  showEvent(anEVT_RULEEDITOR_SHOW);

	showEvent.SetInt(false);

	wxPostEvent(AnEvents::getInstance(), showEvent);
}

void
DlgRuleEditor::onConnectionStateChange(wxCommandEvent& event)
{
	JobCtrl::ConnectionState	 newState;
	PolicyCtrl			*policyCtrl;

	newState = (JobCtrl::ConnectionState)event.GetInt();
	isConnected_ = (newState == JobCtrl::CONNECTED);

	if (isConnected_) {
		policyCtrl = PolicyCtrl::getInstance();
		policyCtrl->receiveOneFromDaemon(0, geteuid());
		policyCtrl->receiveOneFromDaemon(1, geteuid());
		switchRuleSet(policyCtrl->getAdminId(geteuid()),
		    policyCtrl->getUserId());
	}
	rb_userMe->SetValue(true);
	updateFooter();
	event.Skip();
}

void
DlgRuleEditor::onLoadNewRuleSet(wxCommandEvent &event)
{
	int		 admin, user;
	PolicyCtrl	*policyCtrl = PolicyCtrl::getInstance();

	/*
	 * Switch to the the user rule set if no ruleset was loaded
	 * at all.
	 */
	admin = policyCtrl->getAdminId(editorUid_);
	if (editorUid_ == (int)geteuid()) {
		user = policyCtrl->getUserId();
	} else {
		user = -1;
	}
	if (admin != adminRuleSetId_ || user != userRuleSetId_) {
		/* At least one Id changed. Update them. */
		switchRuleSet(admin, user);
	} else if ((admin >= 0 && event.GetInt() == admin)
	    || (user >= 0 && event.GetInt() == user)) {
		/*
		 * Ids did not change but one of our ruleset was
		 * modified. Reload it.
		 */
		loadRuleSet();
	}
	event.Skip();
}

void
DlgRuleEditor::onShowRule(wxCommandEvent& event)
{
	PolicyCtrl	*policyCtrl;
	struct apn_rule	*apnrule;
	FilterPolicy	*filter;
	AppPolicy	*app;
	wxListEvent	 ev;
	PolicyRuleSet	*rs;
	int		 idx;

	policyCtrl = PolicyCtrl::getInstance();
	event.Skip();

	switchRuleSet(policyCtrl->getAdminId(geteuid()),
	    policyCtrl->getUserId());
	if (event.GetInt()) {
		rs = policyCtrl->getRuleSet(adminRuleSetId_);
	} else {
		rs = policyCtrl->getRuleSet(userRuleSetId_);
	}
	if (rs == NULL)
		return;
	apnrule = apn_find_rule(rs->getApnRuleSet(), event.GetExtraLong());
	if (apnrule == NULL)
		return;
	filter = dynamic_cast<FilterPolicy*>((Policy*)apnrule->userdata);
	if (!filter)
		return;
	app = filter->getParentPolicy();
	idx = findListRow(appPolicyListCtrl, app);
	if (idx < 0)
		return;
	if (!IsShown()) {
		wxCommandEvent	ev(anEVT_RULEEDITOR_SHOW);
		ev.SetInt(true);
		wxPostEvent(AnEvents::getInstance(), ev);
	}
	this->Raise();
	appPolicyListCtrl->SetItemState(idx, wxLIST_STATE_SELECTED,
	    wxLIST_STATE_SELECTED);
	idx = findListRow(filterPolicyListCtrl, filter);
	if (idx >= 0) {
		filterPolicyListCtrl->SetItemState(idx, wxLIST_STATE_SELECTED,
		    wxLIST_STATE_SELECTED);
	}
}

void
DlgRuleEditor::onAutoCheck(wxCommandEvent & event)
{
	isAutoChecksumCheck_ = event.GetInt();
}

void
DlgRuleEditor::onAppPolicySelect(wxListEvent & event)
{
	AppPolicy	*policy;
	wxString			 newLabel;
	PolicyRuleSet			*ruleset;

	policy = wxDynamicCast((void*)event.GetData(), AppPolicy);
	if (policy != NULL) {
		/* Show filters of selected application */
		addFilterPolicy(policy);

		/* Show selected policy below application list */
		newLabel.Printf(_("%ls %ls"),
		    policy->getTypeIdentifier().c_str(),
		    policy->getBinaryName().c_str());
		appListPolicyText->SetLabel(newLabel);
		ruleset = policy->getParentRuleSet();
		if (ruleset && (geteuid() == 0 || !ruleset->isAdmin())) {
			appListUpButton->Enable(policy->canMoveUp());
			appListDownButton->Enable(policy->canMoveDown());
			appListDeleteButton->Enable(policy->canDelete());
		} else {
			appListUpButton->Disable();
			appListDownButton->Disable();
			appListDeleteButton->Disable();
		}

		/* enable customisation of header columns */
		filterListColumnsButton->Enable(true);
		filterListTypeChoice->Enable(true);
		filterListCreateButton->Enable(true);

		/* Adjust type choice of filter policy creation. */
		appListTypeChoice->SetStringSelection(
		    policy->getTypeIdentifier());
		filterListTypeChoice->Clear();
		if (policy->IsKindOf(CLASSINFO(AlfAppPolicy))) {
			filterListTypeChoice->Append(wxT("ALF"));
			filterListTypeChoice->Append(wxT("Capability"));
			filterListTypeChoice->Append(wxT("Default"));
		} else if (policy->IsKindOf(CLASSINFO(SfsAppPolicy))) {
			filterListTypeChoice->Append(wxT("SFS"));
			filterListTypeChoice->Append(wxT("Default"));
		} else if (policy->IsKindOf(CLASSINFO(ContextAppPolicy))) {
			filterListTypeChoice->Append(wxT("CTX"));
		} else if (policy->IsKindOf(CLASSINFO(SbAppPolicy))) {
			filterListTypeChoice->Append(wxT("SB"));
			filterListTypeChoice->Append(wxT("Default"));
		}
		filterListTypeChoice->Select(0);

		/* create tabs for each binary */
		appPolicyPanels->select(policy);

		/* Ensure the changes will apear on the screen */
		Layout();
		Refresh();
	}
}

void
DlgRuleEditor::onAppPolicyDeSelect(wxListEvent &event)
{
	saveFilterColumnWidth(event.GetIndex());
	appListPolicyText->SetLabel(wxEmptyString);
	appListUpButton->Disable();
	appListDownButton->Disable();
	appListDeleteButton->Disable();
	filterListTypeChoice->Clear();
	filterListTypeChoice->Disable();
	filterListCreateButton->Disable();

	wipeFilterList();

	/* remove binary tabs */
	appPolicyPanels->deselect();

	Layout();
	Refresh();
}

void
DlgRuleEditor::onFilterPolicySelect(wxListEvent & event)
{
	FilterPolicy		*policy;
	PolicyRuleSet		*ruleset;

	policy = wxDynamicCast((void*)event.GetData(), FilterPolicy);
	if (policy == NULL) {
		return;
	}

	ruleset = policy->getParentRuleSet();
	if (ruleset && (geteuid() == 0 || !ruleset->isAdmin())) {
		filterListUpButton->Enable(policy->canMoveUp());
		filterListDownButton->Enable(policy->canMoveDown());
		filterListDeleteButton->Enable();
	} else {
		filterListUpButton->Disable();
		filterListDownButton->Disable();
		filterListDeleteButton->Disable();
	}

	/* Tell the notebook tabs to show themself. */
	filterPolicyPanels->select(policy);

	Layout();
	Refresh();
}

void
DlgRuleEditor::onFilterPolicyDeSelect(wxListEvent & WXUNUSED(event))
{
	filterListUpButton->Disable();
	filterListDownButton->Disable();
	filterListDeleteButton->Disable();

	/* Tell the notebook tabs to hide themself. */
	filterPolicyPanels->deselect();

	Layout();
	Refresh();
}

void
DlgRuleEditor::onAppListUpClick(wxCommandEvent &)
{
	long	 index,  top;
	Policy	*policy;

	index  = getSelectedIndex(appPolicyListCtrl);
	top = appPolicyListCtrl->GetTopItem();
	policy = NULL;

	if (index > 0) {
		policy = getSelectedPolicy(appPolicyListCtrl);
		if (policy != NULL) {
			wipeAppList();
			if (policy->moveUp())
				index--;
			loadRuleSet();
			selectFrame(appPolicyListCtrl, top, index);
			appListUpButton->Hide();
			appListUpButton->Show();
		}
	}
}

void
DlgRuleEditor::onAppListDownClick(wxCommandEvent &)
{
	long	 index, top;
	Policy	*policy;

	index  = getSelectedIndex(appPolicyListCtrl);
	top = appPolicyListCtrl->GetTopItem();
	policy = NULL;

	if (index >= 0) {
		policy = getSelectedPolicy(appPolicyListCtrl);
		if (policy != NULL) {
			wipeAppList();
			if (policy->moveDown())
				index++;
			loadRuleSet();
			selectFrame(appPolicyListCtrl, top, index);
			appListDownButton->Hide();
			appListDownButton->Show();
		}
	}
}

void
DlgRuleEditor::onFilterListUpClick(wxCommandEvent &)
{
	long	 appIndex, apptop;
	long	 filterIndex, filttop;
	Policy	*policy;

	appIndex = getSelectedIndex(appPolicyListCtrl);
	apptop = appPolicyListCtrl->GetTopItem();
	filterIndex = getSelectedIndex(filterPolicyListCtrl);
	filttop = filterPolicyListCtrl->GetTopItem();
	policy = NULL;

	if (filterIndex > 0) {
		policy = getSelectedPolicy(filterPolicyListCtrl);
		if (policy != NULL) {
			wipeAppList();
			if (policy->moveUp())
				filterIndex--;
			loadRuleSet();
			selectFrame(appPolicyListCtrl, apptop, appIndex);
			selectFrame(filterPolicyListCtrl, filttop, filterIndex);
			filterListUpButton->Hide();
			filterListUpButton->Show();
		}
	}
}

void
DlgRuleEditor::onFilterListDownClick(wxCommandEvent &)
{
	long	 appIndex, apptop;
	long	 filterIndex, filttop;
	Policy	*policy;

	appIndex = getSelectedIndex(appPolicyListCtrl);
	apptop = appPolicyListCtrl->GetTopItem();
	filterIndex = getSelectedIndex(filterPolicyListCtrl);
	filttop = filterPolicyListCtrl->GetTopItem();
	policy = NULL;

	if (filterIndex >= 0) {
		policy = getSelectedPolicy(filterPolicyListCtrl);
		if (policy != NULL) {
			wipeAppList();
			if (policy->moveDown())
				filterIndex++;
			loadRuleSet();
			selectFrame(appPolicyListCtrl, apptop, appIndex);
			selectFrame(filterPolicyListCtrl, filttop, filterIndex);
			filterListDownButton->Hide();
			filterListDownButton->Show();
		}
	}
}

void
DlgRuleEditor::onAppListDeleteClick(wxCommandEvent &)
{
	long	 index, top;
	Policy	*policy;
	int	answer;
	wxString question;

	index  = getSelectedIndex(appPolicyListCtrl);
	top = appPolicyListCtrl->GetTopItem();
	policy = NULL;

	if (index >= 0) {
		int	last = 0;
		policy = getSelectedPolicy(appPolicyListCtrl);
		question.Printf(_("Delete rule %ld?"), policy->getApnRuleId());
		answer = wxMessageBox(question, _("Confirm"), wxYES_NO, this);
		if (answer != wxYES) {
			return;
		}
		if (appPolicyListCtrl->GetItemCount() == index + 1)
			last = 1;
		if (policy != NULL) {
			wipeAppList();
			if (policy->remove() && index && last)
				index--;
			loadRuleSet();
			selectFrame(appPolicyListCtrl, top, index);
			appListDeleteButton->Hide();
			appListDeleteButton->Show();
		}
	}
}

void
DlgRuleEditor::onFilterListDeleteClick(wxCommandEvent &)
{
	long	 appIndex, apptop;
	long	 filterIndex, filttop;
	Policy	*policy;
	int	answer;
	wxString question;

	appIndex  = getSelectedIndex(appPolicyListCtrl);
	apptop = appPolicyListCtrl->GetTopItem();
	filterIndex = getSelectedIndex(filterPolicyListCtrl);
	filttop = filterPolicyListCtrl->GetTopItem();
	policy = NULL;

	if (filterIndex >= 0) {
		int	last = 0;
		policy = getSelectedPolicy(filterPolicyListCtrl);
		question.Printf(_("Delete rule %ld?"), policy->getApnRuleId());
		answer = wxMessageBox(question, _("Confirm"), wxYES_NO, this);
		if (answer != wxYES) {
			return;
		}
		if (filterPolicyListCtrl->GetItemCount() == filterIndex + 1)
			last = 1;
		if (policy != NULL) {
			wipeAppList();
			if (policy->remove() && filterIndex && last)
				filterIndex--;
			loadRuleSet();
			selectFrame(appPolicyListCtrl, apptop, appIndex);
			selectFrame(filterPolicyListCtrl, filttop, filterIndex);
			filterListDeleteButton->Hide();
			filterListDeleteButton->Show();
		}
	}
}

void
DlgRuleEditor::onAppListColumnsButtonClick(wxCommandEvent &)
{
	wxArrayString           choices;
	wxMultiChoiceDialog     *multiChoiceDlg;
	wxArrayInt              selections;

	/* set visible selections for policy */
	for (size_t i=1; i<APP_EOL; i++) {
		choices.Add(appColumns_[i]->getTitle());
		/* get visible selections */
		if (appColumns_[i]->isVisible()) {
			selections.Add(i-1);
		}
	}

	/* get chosen column headers from user dialogue */
	multiChoiceDlg = new wxMultiChoiceDialog(this, _("Table columns"),
	    _("Please select the columns to be shown"), choices);
	multiChoiceDlg->SetSelections(selections);
	multiChoiceDlg->SetSize(400,350);
	multiChoiceDlg->Layout();

	if (multiChoiceDlg->ShowModal() == wxID_OK) {
		/*
		 * First wipe the app List. This makes sure that column
		 * widths are saved before we change visibility.
		 */
		wipeAppList();
		for (size_t i=1; i<APP_EOL; i++) {
			appColumns_[i]->setVisability(false);
		}
		selections.Clear();
		selections = multiChoiceDlg->GetSelections();
		for (size_t i=0; i<selections.GetCount(); i++) {
			appColumns_[selections.Item(i)+1]->setVisability(true);
		}

		/*
		 * Redraw the current view. No need to call wipeAppList here
		 * because the rule sets did not change.
		 */
		loadRuleSet();
	}

	delete multiChoiceDlg;
}

void
DlgRuleEditor::onFilterListColumnsButtonClick(wxCommandEvent &)
{
	wxArrayString	choices;
	wxArrayInt	selections;
	long		idx;
	size_t		last;

	void			 *data;
	AppPolicy		 *policy;
	wxMultiChoiceDialog	 *multiChoiceDlg;
	ListCtrlColumn		**columns;

	last	= 0;
	columns = NULL;
	idx	= getSelectedIndex(appPolicyListCtrl);
	data	= (void*)appPolicyListCtrl->GetItemData(idx);
	policy	= wxDynamicCast(data, AppPolicy);

	if (policy == NULL) {
		filterListColumnsButton->Disable();
		return;
	}

	if (policy->IsKindOf(CLASSINFO(AlfAppPolicy))) {
		columns = alfColumns_;
		last = ALF_EOL;
	} else if (policy->IsKindOf(CLASSINFO(ContextAppPolicy))) {
		columns = ctxColumns_;
		last = CTX_EOL;
	} else if (policy->IsKindOf(CLASSINFO(SfsAppPolicy))) {
		columns = sfsColumns_;
		last = SFS_EOL;
	} else if (policy->IsKindOf(CLASSINFO(SbAppPolicy))) {
		columns = sbColumns_;
		last = SB_EOL;
	} else {
		/*
		 * This should never happen! But if, there's a unknown
		 * app policy, which has to be added to this list.
		*/
		filterListColumnsButton->Disable();
		return;
	}

	/* set visible selections depending on policy */
	for (size_t i=1; i<last; i++) {
		choices.Add(columns[i]->getTitle());
		if (columns[i]->isVisible()) {
			selections.Add(i-1);
		}
	}

	/* get chosen column headers from user dialogue */
	multiChoiceDlg = new wxMultiChoiceDialog(this, _("Table columns"),
	   _("Please select the columns to be shown"), choices);
	multiChoiceDlg->SetSelections(selections);
	multiChoiceDlg->SetSize(400,440);
	multiChoiceDlg->Layout();

	if (multiChoiceDlg->ShowModal() == wxID_OK) {
		/*
		 * Wipe the filter list first. This ensures that column widths
		 * are saved before we change visibility.
		 */
		wipeFilterList();
		for (size_t i=1; i < last; i++) {
			columns[i]->setVisability(false);
		}
		selections.Clear();
		selections = multiChoiceDlg->GetSelections();

		for (size_t i=0; i<selections.GetCount(); i++) {
			columns[selections.Item(i)+1]->setVisability(true);
		}

		addFilterPolicy(policy);
	}

	delete multiChoiceDlg;
}

void
DlgRuleEditor::onAppListCreateButton(wxCommandEvent &)
{
	unsigned int	 id;
	unsigned int	 type;
	long		 top;
	long		 index;
	long		 indexSuggestion;
	long		 ruleSet_id;
	wxString	 typeSelection;
	wxString	 message;
	Policy		*policy;
	PolicyCtrl	*policyCtrl;
	PolicyRuleSet	*ruleSet;

	index = getSelectedIndex(appPolicyListCtrl);
	top   = appPolicyListCtrl->GetTopItem();

	typeSelection = appListTypeChoice->GetStringSelection();
	policyCtrl = PolicyCtrl::getInstance();
	policy = getSelectedPolicy(appPolicyListCtrl);

	ruleSet_id = -1;

	if (policy) {
		ruleSet = policy->getParentRuleSet();
		if (ruleSet && ruleSet->isAdmin() && geteuid() != 0) {
			/*
			 * A non-admin user has selected an admin policy, but
			 * he is not allowed to create a new admin policy!
			 * Thus, fetch the user ruleset.
			 */
			ruleSet = policyCtrl->getRuleSet(userRuleSetId_);
			policy = NULL; /* "Clear" selection */
		}
	} else {
		if (rb_userSelect->GetValue()) {
			if (!tx_userSelect->IsEmpty()) {
				ruleSet_id = adminRuleSetId_;
			}
		} else {
			ruleSet_id = userRuleSetId_;
		}
		ruleSet = policyCtrl->getRuleSet(ruleSet_id);
	}

	if (ruleSet == NULL) {
		ruleSet = createEmptyPolicyRuleSet();
		if (ruleSet == NULL) {
			message = _("Couldn't create new ruleset!");
			wxMessageBox(message, _("Rule Editor"),
			    wxOK | wxICON_ERROR, this);
			return;
		} else {
			message = _("Created new ruleset.");
			wxMessageBox(message, _("Rule Editor"),
			    wxOK | wxICON_INFORMATION, this);
			switchRuleSet(adminRuleSetId_,
			    policyCtrl->getUserId());
		}
	}

	/* Where should the new policy be inserted? */
	if (policy != NULL) {
		id = policy->getApnRuleId();
		if (typeSelection.Cmp(policy->getTypeIdentifier()) != 0) {
			id = 0;
			index = -1;
		}
	} else {
		id = 0;
		index = -1;
	}

	/*
	 * What kind of policy shall been created?
	 * In addition if nothing was selected, calc index of new row.
	 */
	if (typeSelection.Cmp(wxT("ALF")) == 0) {
		type = APN_ALF;
	} else if (typeSelection.Cmp(wxT("CTX")) == 0) {
		type = APN_CTX;
	} else if (typeSelection.Cmp(wxT("SB")) == 0) {
		type = APN_SB;
	} else {
		/* No valid policy type. */
		return;
	}

	wipeAppList();
	indexSuggestion = ruleSet->createPolicy(type, id, NULL);

	if (indexSuggestion < 0) {
		message = _("Error: Couldn't create new application rule.");
		wxMessageBox(message, _("Rule Editor"), wxOK | wxICON_ERROR,
		    this);
	}
	if (index < 0) {
		index = indexSuggestion;
	}

	loadRuleSet();
	selectFrame(appPolicyListCtrl, top, index);
	appListCreateButton->Hide();
	appListCreateButton->Show();
}


void
DlgRuleEditor::onFilterListCreateButton(wxCommandEvent &)
{
	unsigned int	 id;
	unsigned int	 type;
	long		 appIndex;
	long		 appTop;
	long		 filterIndex;
	long		 filterTop;
	wxString	 typeSelection;
	Policy		*policy;
	AppPolicy	*parent;
	PolicyCtrl	*policyCtrl;
	PolicyRuleSet	*ruleSet;

	appIndex    = getSelectedIndex(appPolicyListCtrl);
	appTop      = appPolicyListCtrl->GetTopItem();
	filterIndex = getSelectedIndex(filterPolicyListCtrl);
	filterTop   = filterPolicyListCtrl->GetTopItem();
	policyCtrl  = PolicyCtrl::getInstance();

	policy = getSelectedPolicy(appPolicyListCtrl);
	parent = wxDynamicCast(policy, AppPolicy);
	if (parent == NULL) {
		/* Can't create filter without app parent */
		return;
	}

	ruleSet = parent->getParentRuleSet();
	if (ruleSet == NULL) {
		/* This is extremely odd and wrong - abort. */
		return;
	}
	if (ruleSet->isAdmin() && geteuid() != 0) {
		wxString message = _("Cannot edit admin ruleset!");
		wxMessageBox(message, _("RuleEditor"),
		    wxOK | wxICON_ERROR, this);
		return;
	}

	/* Where should the new policy been inserted? */
	policy = getSelectedPolicy(filterPolicyListCtrl);
	if (policy != NULL) {
		id = policy->getApnRuleId();
	} else {
		id = 0;
		filterIndex = 0;
	}

	/* What kind of policy shall been created? */
	typeSelection = filterListTypeChoice->GetStringSelection();
	if (typeSelection.Cmp(wxT("ALF")) == 0) {
		type = APN_ALF_FILTER;
	} else if (typeSelection.Cmp(wxT("Capability")) == 0) {
		type = APN_ALF_CAPABILITY;
	} else if (typeSelection.Cmp(wxT("SFS")) == 0) {
		type = APN_SFS_ACCESS;
	} else if (typeSelection.Cmp(wxT("CTX")) == 0) {
		type = APN_CTX_RULE;
	} else if (typeSelection.Cmp(wxT("SB")) == 0) {
		type = APN_SB_ACCESS;
	} else if (typeSelection.Cmp(wxT("Default")) == 0) {
		if (parent->IsKindOf(CLASSINFO(SfsAppPolicy))) {
			type = APN_SFS_DEFAULT;
		} else {
			type = APN_DEFAULT;
		}
	} else {
		/* No valid policy type. */
		return;
	}

	wipeAppList();
	if (ruleSet->createPolicy(type, id, parent) < 0) {
		wxMessageBox(
		    _("Error: Couldn't create new filter rule."),
		    _("Rule Editor"), wxOK | wxICON_ERROR, this);
	}

	loadRuleSet();
	selectFrame(appPolicyListCtrl, appTop, appIndex);
	selectFrame(filterPolicyListCtrl, filterTop, filterIndex);
	filterListCreateButton->Hide();
	filterListCreateButton->Show();
}

void
DlgRuleEditor::onFooterImportButton(wxCommandEvent &)
{
	wxFileDialog	 fileDlg(this);
	PolicyCtrl	*policyCtrl;

	wxBeginBusyCursor();

	policyCtrl = PolicyCtrl::getInstance();

	fileDlg.SetMessage(_("Import policy file"));
	fileDlg.SetWildcard(wxT("*"));

	wxEndBusyCursor();

	if (fileDlg.ShowModal() == wxID_OK) {
		if (!policyCtrl->importFromFile(fileDlg.GetPath())) {
			wxMessageBox(
			    _("Couldn't import policy file: it has errors."),
			    _("Error"), wxICON_ERROR);
		}
		switchRuleSet(-1, policyCtrl->getUserId());
	}
}

bool
DlgRuleEditor::reloadRuleSet(long id)
{
	PolicyCtrl	*policyCtrl;
	PolicyRuleSet	*rs;
	int		 prio;

	if (id >= 0) {
		policyCtrl = PolicyCtrl::getInstance();
		rs = policyCtrl->getRuleSet(id);
		if (rs) {
			if (rs->isAdmin()) {
				prio = 0;
			} else {
				prio = 1;
			}
			return policyCtrl->receiveOneFromDaemon(prio,
			    rs->getUid());
		}
	}
	return false;
}

void
DlgRuleEditor::onFooterReloadButton(wxCommandEvent &)
{

	bool	userok;
	bool	adminok;
	adminok = reloadRuleSet(adminRuleSetId_);
	userok = reloadRuleSet(userRuleSetId_);
	if (userok || adminok) {
		footerStatusText->SetLabel(wxT("reload started..."));
	}
}

void
DlgRuleEditor::onFooterExportButton(wxCommandEvent &)
{
	wxFileDialog	 fileDlg(this, wxEmptyString, wxEmptyString,
			     wxEmptyString, wxEmptyString,
			     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	PolicyCtrl	*policyCtrl;
	PolicyRuleSet	*ruleSet = NULL;

	policyCtrl = PolicyCtrl::getInstance();
	if (userRuleSetId_ != -1) {
		ruleSet = policyCtrl->getRuleSet(userRuleSetId_);
	} else if (adminRuleSetId_ != -1) {
		ruleSet = policyCtrl->getRuleSet(adminRuleSetId_);
	}
	if (ruleSet == NULL) {
		/* XXX ch: error dialog to user? */
		return;
	}

	footerStatusText->SetLabel(wxT("exporting to file"));
	Layout();
	Refresh();

	wxBeginBusyCursor();

	fileDlg.SetMessage(_("Export ruleset..."));
	fileDlg.SetWildcard(wxT("*"));

	if (fileDlg.ShowModal() == wxID_OK) {
		ruleSet->setOrigin(fileDlg.GetPath());
		if (policyCtrl->exportToFile(ruleSet->getOrigin()))
			ruleSet->clearModified();
		else
			wxMessageBox(
			    _("Failed to export the ruleset into a file."),
			    _("Export ruleset"), wxOK|wxICON_ERROR, this);
	}

	wxEndBusyCursor();

	updateFooter();
}

void
DlgRuleEditor::onFooterActivateButton(wxCommandEvent &)
{
	wxString	 message;
	PolicyCtrl	*policyCtrl;
	PolicyCtrl::PolicyResult	 polRes;
	PolicyRuleSet	*admin = NULL, *user = NULL;

	policyCtrl = PolicyCtrl::getInstance();

	if (adminRuleSetId_ != -1) {
		admin = policyCtrl->getRuleSet(adminRuleSetId_);
	}
	if (userRuleSetId_ != -1) {
		user = policyCtrl->getRuleSet(userRuleSetId_);
	}
	if (admin == NULL && user == NULL) {
		return;
	}

	if (user) {
		footerStatusText->SetLabel(wxT("sending to daemon"));
		polRes = policyCtrl->sendToDaemon(userRuleSetId_);
		switch (polRes) {
		case PolicyCtrl::RESULT_POL_WRONG_PASS:
			message = _("The entered password is incorrect.");
			wxMessageBox(message, _("Key Load Error"),
			    wxOK|wxICON_ERROR, this);
			footerStatusText->SetLabel(wxT("Wrong password!"));
			break;
		case PolicyCtrl::RESULT_POL_ERR:
			message = _("An error occured while sending policy to"
			    " daemon.");
			wxMessageBox(message, _("Policy Load Error"),
			    wxOK|wxICON_ERROR, this);
			footerStatusText->SetLabel(wxT("Error while sending "
			    "policy to daemon!"));
			break;
		case PolicyCtrl::RESULT_POL_ABORT:
		case PolicyCtrl::RESULT_POL_OK:
			break;
		}
	}
	if (admin && admin->isModified() && geteuid() == 0) {
		footerStatusText->SetLabel(wxT("sending to daemon"));
		polRes = policyCtrl->sendToDaemon(adminRuleSetId_);
		switch (polRes) {
		case PolicyCtrl::RESULT_POL_WRONG_PASS:
			message = _("The entered password is incorrect.");
			wxMessageBox(message, _("Key Load Error"),
			    wxOK|wxICON_ERROR, this);
			footerStatusText->SetLabel(wxT("Wrong password!"));
			break;
		case PolicyCtrl::RESULT_POL_ERR:
			message = _("An error occured while sending admin"
			    " policy to the daemon.");
			wxMessageBox(message, _("Policy Load Error"),
			    wxOK|wxICON_ERROR, this);
			footerStatusText->SetLabel(wxT("Error while sending"
			    " admin policy to daemon."));
			break;
		case PolicyCtrl::RESULT_POL_ABORT:
		case PolicyCtrl::RESULT_POL_OK:
			break;
		}
	}

	updateFooter();
}

long
DlgRuleEditor::addListRow(wxListCtrl *list, Policy *policy)
{
	long index;

	index = list->GetItemCount();

	/* Create new line @ given list. */
	list->InsertItem(index, wxEmptyString);
	list->SetItemPtrData(index, (wxUIntPtr)policy);

	addSubject(policy); /* to those been observed. */

	return (index);
}

void
DlgRuleEditor::removeListRow(wxListCtrl *list, long rowIdx)
{
	Policy	*policy;

	policy = wxDynamicCast((void*)list->GetItemData(rowIdx), Policy);
	if (policy != NULL) {
		/* Stop observing subject if it's a policy. */
		removeSubject(policy);
	}

	list->DeleteItem(rowIdx);
}

long
DlgRuleEditor::findListRow(wxListCtrl *list, Policy *policy)
{
	for (int i = list->GetItemCount(); i >= 0; i--) {
		if (policy == (Policy *)list->GetItemData(i)) {
			return (i);
		}
	}

	return (-1); /* in case of 'not found' */
}

long
DlgRuleEditor::getSelectedIndex(wxListCtrl *list)
{
	return list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

Policy *
DlgRuleEditor::getSelectedPolicy(wxListCtrl *list)
{
	long	 idx;
	Policy	*policy;

	idx = getSelectedIndex(list);
	policy = NULL;

	if (idx != -1) {
		policy = wxDynamicCast((void*)list->GetItemData(idx), Policy);
	}

	return (policy);
}

void
DlgRuleEditor::selectFrame(wxListCtrl *list, long top, long index)
{
	long length;

	length = list->GetCountPerPage();

	/* Select the row. */
	list->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	list->EnsureVisible(top);
	list->EnsureVisible(top + length - 1);
	list->EnsureVisible(index);

	Layout();
	Refresh();
}

void
DlgRuleEditor::deselect(wxListCtrl *list)
{
	long index;

	index = getSelectedIndex(list);
	if (index != -1) {
		list->SetItemState(index, 0, wxLIST_STATE_SELECTED);
	}
}

void
DlgRuleEditor::wipeAppList(void)
{
	saveColumnWidth(appPolicyListCtrl, appColumns_, APP_EOL);
	wipeFilterList();
	deselect(appPolicyListCtrl);

	/*
	 * Remove all lines / items. We have to do it by hand, because
	 * DeleteAllItems() and ClearAll() does not send events.
	 */
	for (int i = appPolicyListCtrl->GetItemCount() - 1; i >= 0; i--) {
		removeListRow(appPolicyListCtrl, i);
	}

	/* Remove all columns, too and update view. */
	appPolicyListCtrl->ClearAll();

	/* disable button for customising header columns */
	appListColumnsButton->Enable(false);

	Refresh();
}

void
DlgRuleEditor::wipeFilterList(void)
{
	saveFilterColumnWidth();
	deselect(filterPolicyListCtrl);

	/*
	 * Remove all lines / items. We have to do it by hand, because
	 * DeleteAllItems() and ClearAll() does not send events.
	 */
	for (int i = filterPolicyListCtrl->GetItemCount() - 1; i >= 0; i--) {
		removeListRow(filterPolicyListCtrl, i);
	}

	/* Remove all columns, too and update view. */
	filterPolicyListCtrl->ClearAll();

	Refresh();
}

void
DlgRuleEditor::addPolicyRuleSet(PolicyRuleSet *ruleSet)
{
	int				policyCount;
	wxString			title;
	RuleEditorAddPolicyVisitor	addVisitor(this);

	if (ruleSet == NULL) {
		return;
	}

	policyCount = ruleSet->getAppPolicyCount();
	if (ruleSet->isAdmin()) {
		title =
		    _("Rule Editor filling application table (admin ruleset)");
	} else {
		title =
		    _("Rule Editor filling application table (user ruleset)");
	}

	/* Show progress bar for big rulesets. */
	if ((policyCount > 100) && (appPolicyLoadProgDlg_ == NULL)) {
		appPolicyLoadProgDlg_ = new wxProgressDialog(title,
		    _("Loading application policy..."), policyCount, this);
		appPolicyLoadProgIdx_ = 0;
	}

	/* Bring new row into sight. */
	ruleSet->accept(addVisitor);

	/* Done with loading, remove progress bar. */
	if (appPolicyLoadProgDlg_ != NULL) {
		delete appPolicyLoadProgDlg_;
		appPolicyLoadProgDlg_ = NULL;
		appPolicyLoadProgIdx_ = 0;
	}
}

void
DlgRuleEditor::addFilterPolicy(AppPolicy *policy)
{
	int				filterCount;
	wxString			title;
	RuleEditorAddPolicyVisitor	addVisitor(this);

	if (policy == NULL) {
		return;
	}

	filterCount = policy->getFilterPolicyCount();
	title = _("Rule Editor filling filter table...");

	/* Show progress bar for big rulesets. */
	if ((filterCount > 100) && (filterPolicyLoadProgDlg_ == NULL)) {
		filterPolicyLoadProgDlg_ = new wxProgressDialog(title,
		    _("Loading filter policy..."), filterCount, this);
		filterPolicyLoadProgIdx_ = 0;
	}

	/* Bring new row into sight. */
	addVisitor.setPropagation(true);
	policy->acceptOnFilter(addVisitor);

	/* Done with loading, remove progress bar. */
	if (filterPolicyLoadProgDlg_ != NULL) {
		delete filterPolicyLoadProgDlg_;
		filterPolicyLoadProgDlg_ = NULL;
		filterPolicyLoadProgIdx_ = 0;
	}
}

void
DlgRuleEditor::switchRuleSet(long admin,  long user)
{
	PolicyCtrl	*policyCtrl = PolicyCtrl::getInstance();
	PolicyRuleSet	*oldrs, *newrs;
	int		 reload = 0;

	if (admin != adminRuleSetId_) {
		oldrs = NULL;
		newrs = NULL;
		if (adminRuleSetId_ != -1) {
			oldrs = policyCtrl->getRuleSet(adminRuleSetId_);
		}
		if (admin != -1) {
			newrs = policyCtrl->getRuleSet(admin);
		}
		if (oldrs) {
			oldrs->unlock();
			removeSubject(oldrs);
		}
		if (newrs == NULL) {
			adminRuleSetId_ = -1;
		} else {
			adminRuleSetId_ = admin;
			newrs->lock();
			addSubject(newrs);
		}
		reload = 1;
	}
	if (user != userRuleSetId_) {
		oldrs = NULL;
		newrs = NULL;
		if (userRuleSetId_ != -1) {
			oldrs = policyCtrl->getRuleSet(userRuleSetId_);
		}
		if (user != -1) {
			newrs = policyCtrl->getRuleSet(user);
		}
		if (oldrs) {
			oldrs->unlock();
			removeSubject(oldrs);
		}
		if (newrs == NULL) {
			userRuleSetId_ = -1;
		} else {
			userRuleSetId_ = user;
			newrs->lock();
			addSubject(newrs);
		}
		reload = 1;
	}
	if (reload)
		loadRuleSet();
}

void
DlgRuleEditor::loadRuleSet(void)
{
	PolicyCtrl			*policyCtrl;

	policyCtrl = PolicyCtrl::getInstance();

	/* Clear list's. */
	wipeFilterList();
	wipeAppList();

	/* Load ruleset with user-policies. */
	if (userRuleSetId_ != -1) {
		addPolicyRuleSet(policyCtrl->getRuleSet(userRuleSetId_));
	}

	/* Load ruleset with admin-policies. */
	if (adminRuleSetId_ != -1) {
		addPolicyRuleSet(policyCtrl->getRuleSet(adminRuleSetId_));
	}

	/* As no app is selected, we remove the accidental filled filters. */
	wipeFilterList();

	/* enable button for customising header columns */
	appListColumnsButton->Enable(true);

	updateFooter();
}

PolicyRuleSet *
DlgRuleEditor::createEmptyPolicyRuleSet(void)
{
	struct iovec		 iv;
	struct apn_ruleset	*rs = NULL;
	PolicyRuleSet		*ruleSet;

	iv.iov_base = (void *)" ";
	iv.iov_len = strlen((char *)iv.iov_base) - 1;

	if (apn_parse_iovec("<iov>", &iv, 1, &rs, 0) != 0) {
		/* This should never happen! */
		apn_free_ruleset(rs);
		ruleSet = NULL;
	} else {
		ruleSet = new PolicyRuleSet(1, geteuid(), rs);
	}

	if (ruleSet != NULL) {
		ruleSet->lock();
		/*
		 * XXX When editing admin rules we should import this as an
		 * XXX admin rule set.
		 */
		PolicyCtrl::getInstance()->importPolicy(ruleSet);
		switchRuleSet(adminRuleSetId_, ruleSet->getRuleSetId());
	}

	return (ruleSet);
}

void
DlgRuleEditor::updateProgDlg(wxProgressDialog *dlg, int *idx, Policy *policy)
{
	wxString message;

	message.Printf(_("loading policy with id: %d"), policy->getApnRuleId());

	if (dlg != NULL) {
		dlg->Update(*idx, message);
		(*idx)++;
	}
}

void
DlgRuleEditor::updateListColumns(wxListCtrl *list,
    ListCtrlColumn *columnList[], size_t length)
{
	long index;

	if (list->GetColumnCount() != 0) {
		return; /* List already has columns. */
	}
	index = 0;

	for (size_t i=0; i<length; i++) {
		if (!columnList[i]->isVisible()) {
			continue; /* Skip this column. */
		}
		columnList[i]->setIndex(index);
		list->InsertColumn(index, columnList[i]->getTitle());
		list->SetColumnWidth(index, columnList[i]->getWidth());
		index++;
	}
}

void
DlgRuleEditor::updateColumnID(wxListCtrl *list, long rowIdx,
    ListCtrlColumn *column, Policy *policy)
{
	long		columnIdx;
	wxString	columnText;

	if (column->isVisible()) {
		columnIdx = column->getIndex();
		columnText.Printf(wxT("%d:"), policy->getApnRuleId());
		list->SetItem(rowIdx, columnIdx, columnText);
	}
}

void
DlgRuleEditor::updateColumnText(wxListCtrl *list, long rowIdx,
    ListCtrlColumn *column, wxString text)
{
	long		columnIdx;

	if (column->isVisible()) {
		columnIdx = column->getIndex();
		list->SetItem(rowIdx, columnIdx, text);
	}
}

void
DlgRuleEditor::updateListAppPolicy(long rowIdx)
{
	wxString	 columnText;
	void		*data;
	AppPolicy	*policy;
	PolicyRuleSet	*ruleset;

	data   = (void*)appPolicyListCtrl->GetItemData(rowIdx);
	policy = wxDynamicCast(data, AppPolicy);
	if (policy == NULL) {
		/* This row has no AppPolicy assigned! */
		return;
	}

	ruleset = policy->getParentRuleSet();

	/* Fill id column */
	updateColumnID(appPolicyListCtrl, rowIdx, appColumns_[APP_ID], policy);

	/* Fill type column */
	columnText = policy->getTypeIdentifier();
	if ((ruleset != NULL) && ruleset->isAdmin()) {
		columnText.Append(wxT("(A)"));
	}
	updateColumnText(appPolicyListCtrl, rowIdx, appColumns_[APP_TYPE],
	    columnText);

	/* Fill user column */
	columnText = _("(unknown)");
	if (ruleset != NULL) {
		columnText = wxGetApp().getUserNameById(ruleset->getUid());
	}
	updateColumnText(appPolicyListCtrl, rowIdx, appColumns_[APP_USER],
	    columnText);

	/* Fill binary column */
	updateColumnText(appPolicyListCtrl, rowIdx, appColumns_[APP_BINARY],
	    policy->getBinaryName());

	/* Set background colour in case of admin policy */
	if ((ruleset != NULL) && ruleset->isAdmin()) {
		appPolicyListCtrl->SetItemBackgroundColour(rowIdx,
		    wxTheColourDatabase->Find(wxT("LIGHT GREY")));
	}
}

void
DlgRuleEditor::updateListAlfFilterPolicy(long rowIdx)
{
	void		*data;
	wxListCtrl	*list;
	Policy		*policy;

	AlfFilterPolicy		*alfPolicy;
	FilterPolicy	*filterPolicy;

	list = filterPolicyListCtrl;
	data   = (void*)list->GetItemData(rowIdx);
	policy = wxDynamicCast(data, Policy);
	alfPolicy = wxDynamicCast(data, AlfFilterPolicy);
	filterPolicy = wxDynamicCast(data, FilterPolicy);
	if (policy == NULL) {
		/* This row has no Policy at alll! */
		return;
	}

	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy)) ||
	    policy->IsKindOf(CLASSINFO(DefaultFilterPolicy))) {
		/* Fill id column */
		updateColumnID(list, rowIdx, alfColumns_[ALF_ID], filterPolicy);

		/* Fill type column */
		updateColumnText(list, rowIdx, alfColumns_[ALF_TYPE],
		    filterPolicy->getTypeIdentifier());

		/* Fill action column*/
		updateColumnText(list, rowIdx, alfColumns_[ALF_ACTION],
		    filterPolicy->getActionName());

		/* Fill log column */
		updateColumnText(list, rowIdx, alfColumns_[ALF_LOG],
		    filterPolicy->getLogName());
	}

	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		/* Fill scope column */
		updateColumnText(list, rowIdx, alfColumns_[ALF_SCOPE],
		    alfPolicy->getScopeName());

		/* Fill direction column */
		updateColumnText(list, rowIdx, alfColumns_[ALF_DIR],
		    alfPolicy->getDirectionName());

		/* Fill protocol column */
		updateColumnText(list, rowIdx, alfColumns_[ALF_PROT],
		    alfPolicy->getProtocolName());

		/* Fill from host column */
		updateColumnText(list, rowIdx, alfColumns_[ALF_FHOST],
		    alfPolicy->getFromHostName());

		/* Fill from port column */
		updateColumnText(list, rowIdx, alfColumns_[ALF_FPORT],
		    alfPolicy->getFromPortName());

		/* Fill to host column */
		updateColumnText(list, rowIdx, alfColumns_[ALF_THOST],
		    alfPolicy->getToHostName());

		/* Fill to port column */
		updateColumnText(list, rowIdx, alfColumns_[ALF_TPORT],
		    alfPolicy->getToPortName());
	}
}

void
DlgRuleEditor::updateListAlfCapabilityFilterPolicy(long rowIdx)
{
	void				*data;
	AlfCapabilityFilterPolicy	*policy;

	data   = (void*)filterPolicyListCtrl->GetItemData(rowIdx);
	policy = wxDynamicCast(data, AlfCapabilityFilterPolicy);
	if (policy == NULL) {
		/* This row has no AlfCapabilityFilterPolicy assigned! */
		return;
	}

	/* Fill id column */
	updateColumnID(filterPolicyListCtrl, rowIdx, alfColumns_[ALF_ID],
	    policy);

	/* Fill type column */
	updateColumnText(filterPolicyListCtrl, rowIdx, alfColumns_[ALF_TYPE],
	    policy->getTypeIdentifier());

	/* Fill capability column*/
	updateColumnText(filterPolicyListCtrl, rowIdx, alfColumns_[ALF_CAP],
	    policy->getCapabilityTypeName());

	/* Fill scope column */
	updateColumnText(filterPolicyListCtrl, rowIdx, alfColumns_[ALF_SCOPE],
	    policy->getScopeName());
}

void
DlgRuleEditor::updateListSfsFilterPolicy(long rowIdx)
{
	void		*data;
	FilterPolicy	*policy;
	SfsFilterPolicy	*sfsPolicy;
	SfsDefaultFilterPolicy *dftPolicy;

	data   = (void*)filterPolicyListCtrl->GetItemData(rowIdx);
	policy = wxDynamicCast(data, FilterPolicy);
	sfsPolicy = wxDynamicCast(data, SfsFilterPolicy);
	dftPolicy = wxDynamicCast(data, SfsDefaultFilterPolicy);
	if (policy == NULL) {
		/* This row has no SfsFilterPolicy assigned! */
		return;
	}

	if (policy->IsKindOf(CLASSINFO(SfsFilterPolicy)) ||
	    policy->IsKindOf(CLASSINFO(SfsDefaultFilterPolicy))) {
		/* Fill id column */
		updateColumnID(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_ID], policy);

		/* Fill type column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_TYPE], policy->getTypeIdentifier());
	}

	if (policy->IsKindOf(CLASSINFO(SfsDefaultFilterPolicy))) {
		/* Fill path column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_PATH], dftPolicy->getPath());

		/* Show default action as valid action */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_VA], dftPolicy->getActionName());

		/* Show default log as valid log */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_VL], dftPolicy->getLogName());
	}

	if (policy->IsKindOf(CLASSINFO(SfsFilterPolicy))) {
		/* Fill scope column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_SCOPE], sfsPolicy->getScopeName());

		/* Fill path column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_PATH], sfsPolicy->getPath());

		/* Fill subject column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_SUB], sfsPolicy->getSubjectName());

		/* Fill valid action column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_VA], sfsPolicy->getValidActionName());

		/* Fill valid log column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_VL], sfsPolicy->getValidLogName());

		/* Fill invalid action column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_IA], sfsPolicy->getInvalidActionName());

		/* Fill invalid log column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_IL], sfsPolicy->getInvalidLogName());

		/* Fill unknown action column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_UA], sfsPolicy->getUnknownActionName());

		/* Fill unknown log column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sfsColumns_[SFS_UL], sfsPolicy->getUnknownLogName());
	}
}

void
DlgRuleEditor::updateListContextFilterPolicy(long rowIdx)
{
	void			*data;
	ContextFilterPolicy	*policy;

	data   = (void*)filterPolicyListCtrl->GetItemData(rowIdx);
	policy = wxDynamicCast(data, ContextFilterPolicy);
	if (policy == NULL) {
		/* This row has no ContextFilterPolicy assigned! */
		return;
	}

	/* Fill id column */
	updateColumnID(filterPolicyListCtrl, rowIdx, ctxColumns_[CTX_ID],
	    policy);

	/* Fill context type */
	updateColumnText(filterPolicyListCtrl, rowIdx, ctxColumns_[CTX_TYPE],
	    policy->getContextTypeName());

	/* Fill binary */
	updateColumnText(filterPolicyListCtrl, rowIdx, ctxColumns_[CTX_BINARY],
	    policy->getBinaryName());
}

void
DlgRuleEditor::updateListSbAccessFilterPolicy(long rowIdx)
{
	void			*data;
	SbAccessFilterPolicy	*sbPolicy;
	FilterPolicy		*policy;

	data   = (void*)filterPolicyListCtrl->GetItemData(rowIdx);
	policy = wxDynamicCast(data, FilterPolicy);
	sbPolicy = wxDynamicCast(data, SbAccessFilterPolicy);
	if (policy == NULL) {
		/* This row has no SbAccessFilterPolicy assigned! */
		return;
	}

	if (policy->IsKindOf(CLASSINFO(SbAccessFilterPolicy)) ||
	    policy->IsKindOf(CLASSINFO(DefaultFilterPolicy))) {
		/* Fill id column */
		updateColumnID(filterPolicyListCtrl, rowIdx, sbColumns_[SB_ID],
		    policy);

		/* Fill type column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sbColumns_[SB_TYPE], policy->getTypeIdentifier());

		/* Fill action column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sbColumns_[SB_ACTION], policy->getActionName());

		/* Fill log column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sbColumns_[SB_LOG], policy->getLogName());
	}

	if (policy->IsKindOf(CLASSINFO(SbAccessFilterPolicy))) {
		/* Fill scope column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sbColumns_[SB_SCOPE], sbPolicy->getScopeName());

		/* Fill path column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sbColumns_[SB_PATH], sbPolicy->getPath());

		/* Fill subject column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sbColumns_[SB_SUB], sbPolicy->getSubjectName());

		/* Fill mask column */
		updateColumnText(filterPolicyListCtrl, rowIdx,
		    sbColumns_[SB_MASK], sbPolicy->getAccessMaskName());
	}
}

void
DlgRuleEditor::updateFooter(void)
{
	PolicyRuleSet	*admin = NULL, *user = NULL;

	admin = PolicyCtrl::getInstance()->getRuleSet(adminRuleSetId_);
	user = PolicyCtrl::getInstance()->getRuleSet(userRuleSetId_);

	if (admin == NULL && user == NULL) {
		return;
	}

	/* Is communication with daemon possible (aka connected)? */
	footerReloadButton->Enable(isConnected_);
	footerActivateButton->Enable(isConnected_);

	/* Update text */
	if (user && user->isDaemonRuleSet()) {
		footerRuleSetText->SetLabel(wxT("daemon"));
	} else if (user) {
		footerRuleSetText->SetLabel(user->getOrigin());
	} else if (admin->isDaemonRuleSet()) {
		footerRuleSetText->SetLabel(wxT("daemon"));
	} else {
		footerRuleSetText->SetLabel(admin->getOrigin());
	}
	if ((user && user->isModified()) || (admin && admin->isModified())) {
		footerStatusText->SetLabel(_("modified"));
	} else {
		footerStatusText->SetLabel(_("not modified"));
	}

	Layout();
	Refresh();
}

void
DlgRuleEditor::setUser(long uid)
{
	PolicyCtrl	*policyCtrl = PolicyCtrl::getInstance();
	long		 user = -1, admin = -1;

	editorUid_ = uid;
	if (rb_userMe->GetValue()) {
		user = policyCtrl->getUserId();
	}
	admin = policyCtrl->getAdminId(uid);
	if (admin < 0) {
		policyCtrl->receiveOneFromDaemon(0, uid);
		admin = policyCtrl->getAdminId(uid);
	}
	switchRuleSet(admin, user);
}

void
DlgRuleEditor::setUser(wxString user)
{
	uid_t	uid;

	/* Do nothing to avoid KillFocus Event loops. */
	if (user == wxT("") || tx_userSelect->IsModified() == false) {
		return;
	}
	uid = wxGetApp().getUserIdByName(user);
	tx_userSelect->DiscardEdits();
	if (uid == (uid_t)-1) {
		wxString	msg;

		msg = _("Invalid user Name");
		wxMessageBox(msg, _("RuleEditor"), wxOK | wxICON_ERROR, this);
	} else {
		setUser(uid);
	}
	/* Clear the IsModified flag. */
}

void
DlgRuleEditor::onRbUserSelect(wxCommandEvent &)
{
	tx_userSelect->Enable();
	tx_userSelect->SetValue(wxT("root"));
}

void
DlgRuleEditor::onRbUserMe(wxCommandEvent &)
{
	tx_userSelect->Disable();
	setUser(geteuid());
}

void
DlgRuleEditor::onUserSelectTextEnter(wxCommandEvent &)
{
	/* Force input validation on return. */
	tx_userSelect->MarkDirty();
	setUser(tx_userSelect->GetValue());
}

void
DlgRuleEditor::onUserSelectKillFocus(wxFocusEvent &)
{
	setUser(tx_userSelect->GetValue());
}

/*
 * IMPLEMENTATION NOTE:
 * This function assumes that the columns in cols correspond to the columns
 * in list (with invisible columns in cols skipped). Callers must make sure
 * that this is actually the case.
 */
void
DlgRuleEditor::saveColumnWidth(wxListCtrl *list, ListCtrlColumn *cols[],
    int len)
{
	int	k = 0;
	for (int i=0; i < len && k < list->GetColumnCount(); ++i) {
		if (cols[i]->isVisible() == false)
			continue;
		cols[i]->setWidth(list->GetColumnWidth(k++));
	}
}

/*
 * IMPLEMENTATION NOTE:
 * This function tries to autodetect the columns that are currently
 * visible in the filter list based on the line that is selected in
 * the application list if idx is set to -1 (default). However, this
 * fails for the deselect case because the line is no longer selected
 * when this function is called.
 */
void
DlgRuleEditor::saveFilterColumnWidth(int idx)
{
	void			*data;
	AppPolicy		*policy;

	if (idx < 0) {
		idx	= getSelectedIndex(appPolicyListCtrl);
	}
	data	= (void*)appPolicyListCtrl->GetItemData(idx);
	policy	= wxDynamicCast(data, AppPolicy);
	if (policy) {
		int			  len = 0;
		ListCtrlColumn		**columns = NULL;

		if (policy->IsKindOf(CLASSINFO(AlfAppPolicy))) {
			columns = alfColumns_;
			len = ALF_EOL;
		} else if (policy->IsKindOf(CLASSINFO(ContextAppPolicy))) {
			columns = ctxColumns_;
			len = CTX_EOL;
		} else if (policy->IsKindOf(CLASSINFO(SfsAppPolicy))) {
			columns = sfsColumns_;
			len = SFS_EOL;
		} else if (policy->IsKindOf(CLASSINFO(SbAppPolicy))) {
			columns = sbColumns_;
			len = SB_EOL;
		}
		if (len) {
			saveColumnWidth(filterPolicyListCtrl, columns, len);
		}
	}
}
