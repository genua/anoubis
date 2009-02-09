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
#include "ProfileCtrl.h"
#include "RuleEditorAddPolicyVisitor.h"
#include "DlgRuleEditorPage.h"
#include "JobCtrl.h"

DlgRuleEditor::DlgRuleEditor(wxWindow* parent)
    : Observer(NULL), DlgRuleEditorBase(parent)
{
	AnEvents *anEvents;

	anEvents = AnEvents::getInstance();

	anEvents->Connect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(DlgRuleEditor::onShow), NULL, this);
	anEvents->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::onLoadNewRuleSet), NULL, this);

	/* We want to know connect-status for save/reload button */
	JobCtrl::getInstance()->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(DlgRuleEditor::onConnectionStateChange),
	    NULL, this);

	appColumns_[APP_ID]	= new ListCtrlColumn(_("ID"), wxT("ID"));
	appColumns_[APP_TYPE]	= new ListCtrlColumn(_("Type"), wxT("TYPE"));
	appColumns_[APP_USER]	= new ListCtrlColumn(_("User"), wxT("USER"));
	appColumns_[APP_BINARY]	= new ListCtrlColumn(_("Binary"),
	    wxT("BINARY"));

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
	alfColumns_[ALF_AF]	= new ListCtrlColumn(_("AF"), wxT("AF"));
	alfColumns_[ALF_FHOST]	= new ListCtrlColumn(_("from host"),
	    wxT("FHOST"));
	alfColumns_[ALF_FPORT]	= new ListCtrlColumn(_("from port"),
	    wxT("FPORT"));
	alfColumns_[ALF_THOST]	= new ListCtrlColumn(_("to host"),
	    wxT("THOST"));
	alfColumns_[ALF_TPORT]	= new ListCtrlColumn(_("to port"),
	    wxT("TPORT"));
	alfColumns_[ALF_TIME]	= new ListCtrlColumn(_("state timeout"),
	    wxT("TIME"));

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
		wxGetApp().getUserOptions()->Read(name, &isVisible, true);
		appColumns_[i]->setVisability(isVisible);
	}

	for (size_t i=1; i<ALF_EOL; i++) {
		name = wxT("/Options/Columns/Alf/") +
		    alfColumns_[i]->getConfKey();
		wxGetApp().getUserOptions()->Read(name, &isVisible, true);
		alfColumns_[i]->setVisability(isVisible);
	}

	for (size_t i=1; i<SFS_EOL; i++) {
		name = wxT("/Options/Columns/Sfs/") +
		    sfsColumns_[i]->getConfKey();
		wxGetApp().getUserOptions()->Read(name, &isVisible, true);
		sfsColumns_[i]->setVisability(isVisible);
	}

	for (size_t i=1; i<CTX_EOL; i++) {
		name = wxT("/Options/Columns/Ctx/") +
		    ctxColumns_[i]->getConfKey();
		wxGetApp().getUserOptions()->Read(name, &isVisible, true);
		ctxColumns_[i]->setVisability(isVisible);
	}

	for (size_t i=1; i<SB_EOL; i++) {
		name = wxT("/Options/Columns/Sb/")
		    + sbColumns_[i]->getConfKey();
		wxGetApp().getUserOptions()->Read(name, &isVisible, true);
		sbColumns_[i]->setVisability(isVisible);
	}

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
		wxGetApp().getUserOptions()->Write(name,
		    appColumns_[i]->isVisible());
	}

	for (size_t i=1; i<ALF_EOL; i++) {
		name = wxT("/Options/Columns/Alf/") +
		    alfColumns_[i]->getConfKey();
		wxGetApp().getUserOptions()->Write(name,
		    alfColumns_[i]->isVisible());
	}

	for (size_t i=1; i<SFS_EOL; i++) {
		name = wxT("/Options/Columns/Sfs/") +
		    sfsColumns_[i]->getConfKey();
		wxGetApp().getUserOptions()->Write(name,
		    sfsColumns_[i]->isVisible());
	}

	for (size_t i=1; i<CTX_EOL; i++) {
		name = wxT("/Options/Columns/Ctx/") +
		    ctxColumns_[i]->getConfKey();
		wxGetApp().getUserOptions()->Write(name,
		    ctxColumns_[i]->isVisible());
	}

	for (size_t i=1; i<SB_EOL; i++) {
		name = wxT("/Options/Columns/Sb/") +
		    sbColumns_[i]->getConfKey();
		wxGetApp().getUserOptions()->Write(name,
		    sbColumns_[i]->isVisible());
	}
}

void
DlgRuleEditor::update(Subject *subject)
{
	long		 idx;
	AppPolicy	*parent;

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
		parent = ((FilterPolicy*)subject)->getParentPolicy();
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
	this->Show(event.GetInt());
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
	JobCtrl::ConnectionState newState;

	newState = (JobCtrl::ConnectionState)event.GetInt();
	isConnected_ = (newState == JobCtrl::CONNECTION_CONNECTED);

	updateFooter();
	event.Skip();
}

void
DlgRuleEditor::onLoadNewRuleSet(wxCommandEvent &event)
{
	ProfileCtrl	*profileCtrl;
	PolicyRuleSet	*ruleSet;

	profileCtrl = ProfileCtrl::getInstance();

	/* Release old one's */
	ruleSet = profileCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->unlock();
		removeSubject(ruleSet);
	}
	ruleSet = profileCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->unlock();
		removeSubject(ruleSet);
	}

	/* Get new rulesets */
	userRuleSetId_ = profileCtrl->getUserId();
	adminRuleSetId_ = profileCtrl->getAdminId(geteuid());

	/* Claim new one's */
	ruleSet = profileCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->lock();
		addSubject(ruleSet);
		updateFooter();
	}
	ruleSet = profileCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->lock();
		addSubject(ruleSet);
	}

	loadRuleSet();
	event.Skip();
}

void
DlgRuleEditor::onAppPolicySelect(wxListEvent & event)
{
	RuleEditorAddPolicyVisitor	 addVisitor(this);
	wxString			 newLabel;
	AppPolicy			*policy;
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
			appListDeleteButton->Enable();
		} else {
			appListUpButton->Disable();
			appListDownButton->Disable();
			appListDeleteButton->Disable();
		}

		/* enable customisation of header columns */
		filterListColumnsButton->Enable(true);
		filterListCreateButton->Enable(true);

		/* Ensure the changes will apear on the screen */
		Layout();
		Refresh();
	}
}

void
DlgRuleEditor::onAppPolicyDeSelect(wxListEvent & WXUNUSED(event))
{
	appListPolicyText->SetLabel(wxEmptyString);
	appListUpButton->Disable();
	appListDownButton->Disable();
	appListDeleteButton->Disable();
	filterListCreateButton->Disable();

	wipeFilterList();

	Layout();
	Refresh();
}

void
DlgRuleEditor::onFilterPolicySelect(wxListEvent & event)
{
	FilterPolicy		*policy;
	PolicyRuleSet		*ruleset;
	DlgRuleEditorPage	*filterPage;

	policy = wxDynamicCast((void*)event.GetData(), FilterPolicy);
	if (policy == NULL) {
		return;
	}

	ruleset = policy->getParentRuleSet();
	if (ruleset && geteuid() == 0 || !ruleset->isAdmin()) {
		filterListUpButton->Enable(policy->canMoveUp());
		filterListDownButton->Enable(policy->canMoveDown());
		filterListDeleteButton->Enable();
	} else {
		filterListUpButton->Disable();
		filterListDownButton->Disable();
		filterListDeleteButton->Disable();
	}

	/* Tell the notebook tabs to show themself. */
	for (int i=filterPolicyPanels->GetPageCount() - 1; i>=0; i--) {
		filterPage = dynamic_cast<DlgRuleEditorPage *>(
		    filterPolicyPanels->GetPage(i));
		if (filterPage != NULL) {
			filterPage->select(policy);
		}
	}

	Layout();
	Refresh();
}

void
DlgRuleEditor::onFilterPolicyDeSelect(wxListEvent & WXUNUSED(event))
{
	DlgRuleEditorPage *filterPage;

	filterListUpButton->Disable();
	filterListDownButton->Disable();
	filterListDeleteButton->Disable();

	/* Tell the notebook tabs to hide themself. */
	for (int i=filterPolicyPanels->GetPageCount() -1; i>=0; i--) {
		filterPage = dynamic_cast<DlgRuleEditorPage *>(
		    filterPolicyPanels->GetPage(i));
		if (filterPage != NULL) {
			filterPage->deselect();
		}
	}

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

	index  = getSelectedIndex(appPolicyListCtrl);
	top = appPolicyListCtrl->GetTopItem();
	policy = NULL;

	if (index >= 0) {
		int	last = 0;
		policy = getSelectedPolicy(appPolicyListCtrl);
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

	appIndex  = getSelectedIndex(appPolicyListCtrl);
	apptop = appPolicyListCtrl->GetTopItem();
	filterIndex = getSelectedIndex(filterPolicyListCtrl);
	filttop = filterPolicyListCtrl->GetTopItem();
	policy = NULL;

	if (filterIndex >= 0) {
		int	last = 0;
		policy = getSelectedPolicy(filterPolicyListCtrl);
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

	if (multiChoiceDlg->ShowModal() == wxID_OK) {
		for (size_t i=1; i<APP_EOL; i++) {
			appColumns_[i]->setVisability(false);
		}
		selections.Clear();
		selections = multiChoiceDlg->GetSelections();
		for (size_t i=0; i<selections.GetCount(); i++) {
			appColumns_[selections.Item(i)+1]->setVisability(true);
		}

		/* call to enforce redrawing of the current view */
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

	if (multiChoiceDlg->ShowModal() == wxID_OK) {
		for (size_t i=1; i < last; i++) {
			columns[i]->setVisability(false);
		}
		selections.Clear();
		selections = multiChoiceDlg->GetSelections();

		for (size_t i=0; i<selections.GetCount(); i++) {
			columns[selections.Item(i)+1]->setVisability(true);
		}

		/* call to enforce redrawing of the current view */
		wipeFilterList();
		addFilterPolicy(policy);
	}

	delete multiChoiceDlg;
}

void
DlgRuleEditor::onAppListCreateButton(wxCommandEvent &)
{
	bool		 rc;
	unsigned int	 id;
	unsigned int	 type;
	long		 index;
	long		 top;
	wxString	 typeSelection;
	Policy		*policy;
	ProfileCtrl	*profileCtrl;
	PolicyRuleSet	*ruleSet;

	rc = false;
	index  = getSelectedIndex(appPolicyListCtrl);
	top = appPolicyListCtrl->GetTopItem();
	profileCtrl = ProfileCtrl::getInstance();
	ruleSet = profileCtrl->getRuleSet(userRuleSetId_);

	/* Where should the new policy been inserted? */
	policy = getSelectedPolicy(appPolicyListCtrl);
	if (policy != NULL) {
		id = policy->getApnRuleId();
	} else {
		id = 0;
	}

	/* What kind of policy shall been created? */
	typeSelection = appListTypeChoice->GetStringSelection();
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

	/* If there's not ruleset, create one ... */
	if (ruleSet == NULL) {
		ruleSet = createEmptyPolicyRuleSet();
		wxMessageBox(
		    _("Because of missing ruleset, a new one was created."),
		    _("Rule Editor"), wxOK | wxICON_INFORMATION, this);
	}

	if (ruleSet != NULL) {
		wipeAppList();
		rc = ruleSet->createAppPolicy(type, id);
		loadRuleSet();
	}

	if (rc) {
		if (index < 0) {
			/* No rule was selected */
			index = 0;
		}
		selectFrame(appPolicyListCtrl, top, index);
	} else {
		/* XXX ch: this breaks coding style guide lines */
		wxMessageBox(
		    _("Couldn't create new rule.\nMaybe the types of selected \
 and been created policy mismatch!"),
		    _("Rule Editor"), wxOK | wxICON_ERROR, this);
	}
}

void
DlgRuleEditor::onFooterImportButton(wxCommandEvent &)
{
	wxFileDialog	 fileDlg(this);
	ProfileCtrl	*profileCtrl;

	wxBeginBusyCursor();

	profileCtrl = ProfileCtrl::getInstance();

	fileDlg.SetMessage(_("Import policy file"));
	fileDlg.SetWildcard(wxT("*"));

	wxEndBusyCursor();

	if (fileDlg.ShowModal() == wxID_OK) {
		if (!profileCtrl->importFromFile(fileDlg.GetPath())) {
			wxMessageBox(
			    _("Couldn't import policy file: it has errors."),
			    _("Error"), wxICON_ERROR);
		}
	}
}

void
DlgRuleEditor::onFooterReloadButton(wxCommandEvent &)
{
	ProfileCtrl::getInstance()->receiveFromDaemon();
	footerStatusText->SetLabel(wxT("reload started..."));
}

void
DlgRuleEditor::onFooterExportButton(wxCommandEvent &)
{
	wxFileDialog	 fileDlg(this, wxEmptyString, wxEmptyString,
			     wxEmptyString, wxEmptyString,
			     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	ProfileCtrl	*profileCtrl;
	PolicyRuleSet	*ruleSet;

	profileCtrl = ProfileCtrl::getInstance();
	ruleSet	    = profileCtrl->getRuleSet(userRuleSetId_);
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
		profileCtrl->exportToFile(ruleSet->getOrigin());
		ruleSet->clearModified();
	}

	wxEndBusyCursor();

	updateFooter();
}

void
DlgRuleEditor::onFooterSaveButton(wxCommandEvent &)
{
	ProfileCtrl	*profileCtrl;
	PolicyRuleSet	*ruleSet;

	profileCtrl = ProfileCtrl::getInstance();

	ruleSet = profileCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet == NULL) {
		/* XXX ch: error dialog to user? */
		return;
	}

	footerStatusText->SetLabel(wxT("sending to daemon"));
	profileCtrl->sendToDaemon(userRuleSetId_);

	/*
	 * XXX ch: This will update the status imediately. If we want
	 * XXX ch: to wait until transmission finished successfully,
	 * XXX ch: we need to register to anTASKEVT_POLICY_SEND ...
	 */
	ruleSet->clearModified();
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
	policy->acceptOnFilter(addVisitor);

	/* Done with loading, remove progress bar. */
	if (filterPolicyLoadProgDlg_ != NULL) {
		delete filterPolicyLoadProgDlg_;
		filterPolicyLoadProgDlg_ = NULL;
		filterPolicyLoadProgIdx_ = 0;
	}
}

void
DlgRuleEditor::loadRuleSet(void)
{
	ProfileCtrl			*profileCtrl;

	profileCtrl = ProfileCtrl::getInstance();

	/* Clear list's. */
	wipeFilterList();
	wipeAppList();

	/* Load ruleset with user-policies. */
	addPolicyRuleSet(profileCtrl->getRuleSet(userRuleSetId_));

	/* Load ruleset with admin-policies. */
	addPolicyRuleSet(profileCtrl->getRuleSet(adminRuleSetId_));

	/* As no app is selected, we remove the accidental filled filters. */
	wipeFilterList();

	/* enable button for customising header columns */
	appListColumnsButton->Enable(true);
}

PolicyRuleSet *
DlgRuleEditor::createEmptyPolicyRuleSet(void)
{
	struct iovec		 iv;
	struct apn_ruleset	*rs;
	PolicyRuleSet		*ruleSet;

	iv.iov_base = (void *)" ";
	iv.iov_len = strlen((char *)iv.iov_base) - 1;

	if (apn_parse_iovec("<iov>", &iv, 1, &rs, 0) != 0) {
		/* This should never happen! */
		ruleSet = NULL;
	} else {
		ruleSet = new PolicyRuleSet(1, geteuid(), rs);
	}

	if (ruleSet != NULL) {
		ruleSet->lock();
		ProfileCtrl::getInstance()->importPolicy(ruleSet);
		userRuleSetId_ = ruleSet->getRuleSetId();
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

		/* Fill address family column */
		updateColumnText(list, rowIdx, alfColumns_[ALF_AF],
		    alfPolicy->getAddrFamilyName());

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

		/* Fill state timeout column */
		updateColumnText(list, rowIdx, alfColumns_[ALF_TIME],
		    alfPolicy->getStateTimeoutName());
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
	PolicyRuleSet	*ruleSet;

	ruleSet = ProfileCtrl::getInstance()->getRuleSet(userRuleSetId_);
	if (ruleSet == NULL) {
		return;
	}

	/* If it's modfied, enable export and save button */
	footerExportButton->Enable(ruleSet->isModified());
	footerReloadButton->Enable(isConnected_);
	footerSaveButton->Enable(isConnected_ && ruleSet->isModified());

	/* Update text */
	if (ruleSet->isDaemonRuleSet()) {
		footerRuleSetText->SetLabel(wxT("daemon"));
	} else {
		footerRuleSetText->SetLabel(ruleSet->getOrigin());
	}
	if (ruleSet->isModified()) {
		footerStatusText->SetLabel(_("modified"));
	} else {
		footerStatusText->SetLabel(wxT("not modified"));
	}

	Layout();
	Refresh();
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
	AppPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AppPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(AppPolicy))) {
		policy->setBinaryName(binName, 0);
		policy->accept(updateTable);
		policy->accept(updateWidgets);
	}
}

void
DlgRuleEditor::updateAction(int action)
{
	FilterPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (FilterPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(FilterPolicy))) {
		policy->setActionNo(action);
		policy->accept(updateTable);
		policy->accept(updateWidgets);
	}
}

void
DlgRuleEditor::updateType(int WXUNUSED(type))
{
	/*
	 * XXX ch: fix this in RuleEditor Change
	 *
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
	*/
}

void
DlgRuleEditor::updateProtocol(int protocol)
{
	AlfFilterPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AlfFilterPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if ((policy == NULL) || !policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		return;
	}

	policy->setProtocol(protocol);
	policy->accept(updateTable);
	policy->accept(updateWidgets);
}

void
DlgRuleEditor::updateAddrFamily(int addrFamily)
{
	AlfFilterPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AlfFilterPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		policy->setAddrFamilyNo(addrFamily);
		policy->accept(updateTable);
		policy->accept(updateWidgets);
	}
}

void
DlgRuleEditor::updateCapType(int type)
{
	AlfCapabilityFilterPolicy	*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AlfCapabilityFilterPolicy *)ruleListCtrl->GetItemData(
	    selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(AlfCapabilityFilterPolicy))) {
		policy->setCapabilityTypeNo(type);
		policy->accept(updateTable);
		policy->accept(updateWidgets);
	}
}

void
DlgRuleEditor::updateDirection(int direction)
{
	AlfFilterPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);

	policy = (AlfFilterPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		policy->setDirectionNo(direction);
		policy->accept(updateTable);
		policy->accept(updateWidgets);
	}
}

void
DlgRuleEditor::updateTimeout(int timeout)
{
	AlfFilterPolicy			*policy;
	RuleEditorFillTableVisitor	 updateTable(this, selectedIndex_);

	policy = (AlfFilterPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		policy->setStateTimeout(timeout);
		policy->accept(updateTable);
	}
}

void
DlgRuleEditor::updateAlfSrcAddr(wxString address, int WXUNUSED(netmask),
    int WXUNUSED(af))
{
	AlfFilterPolicy			*policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);

	policy = (AlfFilterPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		policy->setFromHostName(address);
		policy->accept(updateTable);
	}
}

void
DlgRuleEditor::updateAlfDstAddr(wxString address, int WXUNUSED(netmask),
    int WXUNUSED(af))
{
	AlfFilterPolicy			*policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);

	policy = (AlfFilterPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		policy->setToHostName(address);
		policy->accept(updateTable);
	}
}

void
DlgRuleEditor::updateAlfSrcPort(wxString port)
{
	AlfFilterPolicy			*policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor     updateWidgets(this);

	policy = (AlfFilterPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		policy->setFromPortName(port);
		policy->accept(updateTable);
		policy->accept(updateWidgets);
	}
}

void
DlgRuleEditor::updateAlfDstPort(wxString port)
{
	AlfFilterPolicy			*policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor     updateWidgets(this);

	policy = (AlfFilterPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		policy->setToPortName(port);
		policy->accept(updateTable);
		policy->accept(updateWidgets);
	}
}

void
DlgRuleEditor::updateLog(int logNo)
{
	FilterPolicy			*policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);

	policy = (FilterPolicy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(FilterPolicy))) {
		policy->setLogNo(logNo);
		policy->accept(updateTable);
	}
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
	AppPolicy			*policy;
	RuleEditorFillTableVisitor       updateTable(this, selectedIndex_);
	RuleEditorFillWidgetsVisitor	 updateWidgets(this);
	unsigned char			 csum[MAX_APN_HASH_LEN];
	wxString			 curHash;

	if (wxIsBusy())
		return;

	policy = (AppPolicy*)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy)
		return;

	if (policy->IsKindOf(CLASSINFO(AppPolicy))) {
		/* XXX ch: this does not work for lists */
		UtilsAppPolicy::calculateHash(policy->getBinaryName(0),
		    csum, MAX_APN_HASH_LEN);
		policy->setHashValueNo(csum, 0);
		policy->accept(updateTable);
		policy->accept(updateWidgets);
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
		/* XXX ch: this does not work for lists */
		ret = UtilsAppPolicy::calculateHash(
		    ((AppPolicy *)genpolicy)->getBinaryName(0), csum,
			MAX_APN_HASH_LEN);
		switch (ret) {
		case 1:
			updateVisitor.setPropagation(false);
			genpolicy->accept(updateVisitor);
			modified();
			break;
		case 0:
			wxGetApp().calChecksum(
			    ((AppPolicy *)genpolicy)->getBinaryName(0));
			wxBeginBusyCursor();
			modified();
			break;
		case -2:
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
}

void
DlgRuleEditor::OnRuleCreateButton(wxCommandEvent& )
{
	/*
	 * XXX ch: re-enable this with RuleEditor change
	 */
		//#if 0
	int		 id;
	uid_t		 uid;
	long		 rsid;
	PolicyRuleSet	*ruleSet;
	ProfileCtrl	*profileCtrl;

	id = -1;
	result = false;
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
		//#endif
}

void
DlgRuleEditor::OnRuleDeleteButton(wxCommandEvent& )
{
	/*
	 * XXX ch: re-enable this with RuleEditor change
	 */
		//#if 0
	Policy		*policy;
	PolicyRuleSet	*rs;

	policy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (policy == NULL) {
		return;
	}

	rs = policy->getParentRuleSet();
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
		//#endif
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
	/*
	 * XXX ch: this button was already disabled/removed
	 * XXX ch: fix this in RuleEditor chagne
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
	*/
}

void
DlgRuleEditor::OnSfsValidateChkSumButton(wxCommandEvent& )
{
	/*
	 * XXX ch: this button was already disabled/removed
	 * XXX ch: fix this in RuleEditor chagne
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
 */
}
//#endif

void
DlgRuleEditor::OnShow(wxCommandEvent& event)
{
	this->Show(event.GetInt());
	event.Skip();
}

/*
 * XXX ch: this will be fixed with the next functionality change
 */
//#if 0
void
DlgRuleEditor::OnRuleSetSave(wxCommandEvent& )
{
	/*
	 * XXX ch: re-enable with RuleEditor change
	 */
		//#if 0
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
	profileCtrl->sendToDaemon(rs->getRuleSetId());

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
			profileCtrl->sendToDaemon(rs->getRuleSetId());
		}
	}
		//#endif
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

		ruleSet->accept(addVisitor);
	}

	ruleSet = profileCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
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
	selectedId_ = policy->getApnRuleId();
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
	updateAlfSrcPort(event.GetString());
	modified();
}

void
DlgRuleEditor::onAlfDstPortTextCtrlEnter(wxCommandEvent& event)
{
	updateAlfDstPort(event.GetString());
	modified();
}

void
DlgRuleEditor::OnShowRule(wxCommandEvent& event)
{
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

	ruleListCtrl->SetItemState(policy->getRuleEditorIndex(),
	    wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
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
	AppPolicy			*appPolicy;
	RuleEditorFillWidgetsVisitor	 updateVisitor(this);
	unsigned char			 csum[MAX_APN_HASH_LEN];
	int				 mismatch;
	wxString			 currHash;
	wxString			 regHash;
	wxString			 message;

	policy = (Policy *)ruleListCtrl->GetItemData(selectedIndex_);
	if (!policy) {
		selectedIndex_ = 0;
		return (false);
	}

	if (policy->IsKindOf(CLASSINFO(FilterPolicy))) {
		policy = ((FilterPolicy *)policy)->getParentPolicy();
		if (!policy) {
			selectedIndex_ = 0;
			return (false);
		}
	}

	if (!policy->isModified()) {
		return (true);
	}

	mismatch = false;
	/* XXX ch: this does not work for lists */
	if (policy->IsKindOf(CLASSINFO(AppPolicy))) {
		appPolicy = (AppPolicy*)policy;
		if (appPolicy->getBinaryName(0).Cmp(wxT("any")) == 0) {
			return (true);
		}
		if (UtilsAppPolicy::calculateHash(appPolicy->getBinaryName(0),
		    csum, MAX_APN_HASH_LEN)) {
			UtilsAppPolicy::csumToString(csum, MAX_APN_HASH_LEN,
			    currHash);
		} else {
			/* XXX: KM Better Error Handling is needed */
			currHash = _("unable to calculate checksum");
			return (true);
		}
		regHash = appPolicy->getHashValueName(0);
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
			if (policy == NULL) {
				return (false);
			}
			if (policy->IsKindOf(CLASSINFO(FilterPolicy))) {
				policy =
				    ((FilterPolicy *)policy)->getParentPolicy();
				selectedIndex_ = policy->getRuleEditorIndex();
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

	if(policy->IsKindOf(CLASSINFO(SfsAppPolicy))) {
		((AppPolicy *)policy)->setHashValueString(sum, 0);
		wxGetApp().getChecksum(((AppPolicy *)policy)->getBinaryName(0));
	} else {
		/* AppPolicy doesn't need to be checked with Shaodwtree */
		((AppPolicy *)policy)->setHashValueString(sum, 0);

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

	policy->accept(updateWidgets);
	wxEndBusyCursor();

	event.Skip();
}

void
DlgRuleEditor::selectLine(unsigned long index)
{
	ruleListCtrl->SetItemState(index, wxLIST_STATE_SELECTED,
	    wxLIST_STATE_SELECTED);
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
		ruleSet->setModified();
		if (ruleSet->hasErrors()) {
			text += _(" but has errors");
		}
	}

	controlRuleSetStatusText->SetLabel(text);
	controlRuleSetSaveButton->Enable();
}

ANEVENTS_IDENT_BCAST_METHOD_DEFINITION(DlgRuleEditor)

#endif
