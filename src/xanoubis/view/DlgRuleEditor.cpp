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

#include <wx/arrimpl.cpp>
#include <wx/filedlg.h>
#include <wx/config.h>

#include "main.h"
#include "DlgRuleEditor.h"

#include "AnGrid.h"
#include "AnRowProvider.h"
#include "AlfAppPolicy.h"
#include "ContextAppPolicy.h"
#include "SfsAppPolicy.h"
#include "FilterPolicy.h"
#include "PolicyRuleSet.h"
#include "PolicyCtrl.h"
#include "DlgRuleEditorPage.h"
#include "DlgRuleEditorAppPage.h"
#include "JobCtrl.h"
#include "AnPolicyNotebook.h"
#include "DlgRuleEditorListProperty.h"
#include "DlgRuleEditorAttrProvider.h"


#define ADD_PROPERTY(table, type, width) \
	table->addProperty(new DlgRuleEditorListProperty( \
	    DlgRuleEditorListProperty::type), width)

#define SET_TABLE(grid, table) \
	do { \
		grid->SetTable(table, false, wxGrid::wxGridSelectRows); \
		table->assignColumnWidth();\
	} while(0)


DlgRuleEditor::DlgRuleEditor(wxWindow* parent)
    : Observer(NULL), DlgRuleEditorBase(parent)
{
	AnEvents *anEvents;

	anEvents = AnEvents::getInstance();

	anEvents->Connect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(DlgRuleEditor::onShow), NULL, this);
	anEvents->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::onLoadNewRuleSet), NULL, this);
	anEvents->Connect(anEVT_SEND_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::onSendRuleSet), NULL, this);
	anEvents->Connect(anEVT_SHOW_RULE,
	    wxCommandEventHandler(DlgRuleEditor::onShowRule), NULL, this);

	adminRuleSetId_ = -1;
	userRuleSetId_ = -1;

	/* We want to know connect-status for save/reload button */
	JobCtrl::getInstance()->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(DlgRuleEditor::onConnectionStateChange),
	    NULL, this);

	isConnected_ = false;

	editorUid_ = geteuid();
	if (editorUid_ != 0) {
		rb_userSelect->Disable();
		rb_userDefault->Disable();
	}
	tx_userSelect->Disable();

	/* Fill table for app polilcies with columns. */
	appTable_ = new AnTable(wxT("/State/DlgRuleEditor/Columns/App/"));
	appTable_->SetAttrProvider(new DlgRuleEditorAttrProvider(appTable_));
	ADD_PROPERTY(appTable_, PROPERTY_ID, 45);
	ADD_PROPERTY(appTable_, PROPERTY_TYPE, 40);
	ADD_PROPERTY(appTable_, PROPERTY_USER, 55);
	ADD_PROPERTY(appTable_, PROPERTY_NOSFS, 55);
	ADD_PROPERTY(appTable_, PROPERTY_BINARY, 255);
	SET_TABLE(appGrid, appTable_);
	appGrid->setCursorVisibility(false);

	/* Fill table for alf filter polilcies with columns. */
	alfTable_ = new AnTable(wxT("/State/DlgRuleEditor/Columns/Alf/"));
	alfTable_->SetAttrProvider(new DlgRuleEditorAttrProvider(alfTable_));
	ADD_PROPERTY(alfTable_, PROPERTY_ID, 45);
	ADD_PROPERTY(alfTable_, PROPERTY_TYPE, 55);
	ADD_PROPERTY(alfTable_, PROPERTY_ACTION, 50);
	ADD_PROPERTY(alfTable_, PROPERTY_LOG, 55);
	ADD_PROPERTY(alfTable_, PROPERTY_SCOPE, 55);
	ADD_PROPERTY(alfTable_, PROPERTY_CAP, 72);
	ADD_PROPERTY(alfTable_, PROPERTY_DIR, 70);
	ADD_PROPERTY(alfTable_, PROPERTY_PROT, 65);
	ADD_PROPERTY(alfTable_, PROPERTY_FHOST, 80);
	ADD_PROPERTY(alfTable_, PROPERTY_FPORT, 80);
	ADD_PROPERTY(alfTable_, PROPERTY_THOST, 80);
	ADD_PROPERTY(alfTable_, PROPERTY_TPORT, 80);
	SET_TABLE(filterGrid, alfTable_);
	filterGrid->setCursorVisibility(false);

	/* Fill table for sfs filter polilcies with columns. */
	sfsTable_ = new AnTable(wxT("/State/DlgRuleEditor/Columns/Sfs/"));
	sfsTable_->SetAttrProvider(new DlgRuleEditorAttrProvider(sfsTable_));
	ADD_PROPERTY(sfsTable_, PROPERTY_ID, 45);
	ADD_PROPERTY(sfsTable_, PROPERTY_TYPE, 55);
	ADD_PROPERTY(sfsTable_, PROPERTY_PATH, 80);
	ADD_PROPERTY(sfsTable_, PROPERTY_SUB, 80);
	ADD_PROPERTY(sfsTable_, PROPERTY_SCOPE, 50);
	ADD_PROPERTY(sfsTable_, PROPERTY_VALACT, 85);
	ADD_PROPERTY(sfsTable_, PROPERTY_VALLOG, 65);
	ADD_PROPERTY(sfsTable_, PROPERTY_INVALACT, 95);
	ADD_PROPERTY(sfsTable_, PROPERTY_INVALLOG, 75);
	ADD_PROPERTY(sfsTable_, PROPERTY_UNKACT, 120);
	ADD_PROPERTY(sfsTable_, PROPERTY_UNKLOG, 100);

	/* Fill table for context filter polilcies with columns. */
	ctxTable_ = new AnTable(wxT("/State/DlgRuleEditor/Columns/Ctx/"));
	ctxTable_->SetAttrProvider(new DlgRuleEditorAttrProvider(ctxTable_));
	ADD_PROPERTY(ctxTable_, PROPERTY_ID, 45);
	ADD_PROPERTY(ctxTable_, PROPERTY_TYPE, 55);
	ADD_PROPERTY(ctxTable_, PROPERTY_BINARY, 300);

	/* Fill table for sandbox filter polilcies with columns. */
	sbTable_ = new AnTable(wxT("/State/DlgRuleEditor/Columns/Sb/"));
	sbTable_->SetAttrProvider(new DlgRuleEditorAttrProvider(sbTable_));
	ADD_PROPERTY(sbTable_, PROPERTY_ID, 45);
	ADD_PROPERTY(sbTable_, PROPERTY_TYPE, 55);
	ADD_PROPERTY(sbTable_, PROPERTY_ACTION, 51);
	ADD_PROPERTY(sbTable_, PROPERTY_LOG, 52);
	ADD_PROPERTY(sbTable_, PROPERTY_SCOPE, 53);
	ADD_PROPERTY(sbTable_, PROPERTY_PATH, 80);
	ADD_PROPERTY(sbTable_, PROPERTY_SUB, 80);
	ADD_PROPERTY(sbTable_, PROPERTY_MASK, 98);
}

DlgRuleEditor::~DlgRuleEditor(void)
{
	AnEvents *anEvents;

	anEvents = AnEvents::getInstance();

	anEvents->Disconnect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(DlgRuleEditor::onShow), NULL, this);
	anEvents->Disconnect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::onLoadNewRuleSet), NULL, this);
	anEvents->Disconnect(anEVT_SEND_RULESET,
	    wxCommandEventHandler(DlgRuleEditor::onSendRuleSet), NULL, this);
	anEvents->Disconnect(anEVT_SHOW_RULE,
	    wxCommandEventHandler(DlgRuleEditor::onShowRule), NULL, this);

	JobCtrl::getInstance()->Disconnect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(DlgRuleEditor::onConnectionStateChange),
	    NULL, this);

	appGrid->SetTable(NULL);
	filterGrid->SetTable(NULL);
	if (appTable_ != NULL) {
		delete appTable_;
	}
	if (alfTable_ != NULL) {
		delete alfTable_;
	}
	if (sfsTable_ != NULL) {
		delete sfsTable_;
	}
	if (ctxTable_ != NULL) {
		delete ctxTable_;
	}
	if (sbTable_ != NULL) {
		delete sbTable_;
	}
}

void
DlgRuleEditor::update(Subject *subject)
{
	PolicyRuleSet	*ruleSet;
	AppPolicy	*appPolicy;

	ruleSet = dynamic_cast<PolicyRuleSet*>(subject);

	if (ruleSet != NULL) {
		appPolicy = dynamic_cast<AppPolicy*>
		    (getSelectedPolicy(appGrid));
		if (appPolicy != NULL)
			setAppPolicyLabel(appPolicy);
		updateFooter();
		return;
	}

	/* Unknown subject type - do nothing */
}

void
DlgRuleEditor::updateDelete(Subject * WXUNUSED(subject))
{
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
	} else if (editorUid_ == (long)-1) {
		user = policyCtrl->getUserId((long)-1);
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
DlgRuleEditor::onSendRuleSet(wxCommandEvent &event)
{
	updateFooter();

	event.Skip();
}

void
DlgRuleEditor::onShowRule(wxCommandEvent& event)
{
	PolicyCtrl	*policyCtrl;
	struct apn_rule	*apnrule;
	FilterPolicy	*filter;
	AppPolicy	*app;
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
	if (rs == NULL) {
		return;
	}

	apnrule = apn_find_rule(rs->getApnRuleSet(), event.GetExtraLong());
	if (apnrule == NULL) {
		return;
	}

	filter = dynamic_cast<FilterPolicy*>((Policy*)apnrule->userdata);
	if (!filter) {
		app = dynamic_cast<AppPolicy*>((Policy*)apnrule->userdata);
		if (app == NULL)
			return;
	} else {
		app = filter->getParentPolicy();
	}
	idx = rs->getIndexOfPolicy(app);
	if (idx < 0) {
		return;
	}

	if (!IsShown()) {
		wxCommandEvent	ev(anEVT_RULEEDITOR_SHOW);
		ev.SetInt(true);
		wxPostEvent(AnEvents::getInstance(), ev);
	}
	this->Raise();

	selectRow(appGrid, idx);
	if (filter) {
		idx = app->getIndexOfFilterPolicy(filter);
		if (idx >= 0) {
			selectRow(filterGrid, idx);
		}
	}
}

void
DlgRuleEditor::onAppGridCellSelect(wxGridEvent &event)
{
	AnListClass		*item;
	AnRowProvider		*rowProvider;
	AppPolicy		*appPolicy;
	AlfAppPolicy		*alfApp;
	SfsAppPolicy		*sfsApp;
	ContextAppPolicy	*ctxApp;
	SbAppPolicy		*sbApp;
	PolicyRuleSet		*ruleSet;

	rowProvider = appTable_->getRowProvider();
	if (rowProvider == NULL) {
		/*
		 * This is weird. But if no ruleset is loaded, the table
		 * has no row provider. At the same time switching one
		 * or may columns on/off will trap in this.
		 */
		return;
	}
	item = rowProvider->getRow(event.GetRow());
	appPolicy = dynamic_cast<AppPolicy*>(item);
	alfApp = dynamic_cast<AlfAppPolicy*>(item);
	sfsApp = dynamic_cast<SfsAppPolicy*>(item);
	ctxApp = dynamic_cast<ContextAppPolicy*>(item);
	sbApp  = dynamic_cast<SbAppPolicy*>(item);

	/* Update filter table and type choice. */
	filterListTypeChoice->Clear();
	if (alfApp != NULL) {
		rowProvider = alfApp->getRowProvider();
		alfTable_->setRowProvider(rowProvider);
		SET_TABLE(filterGrid, alfTable_);

		filterListTypeChoice->Append(wxT("ALF"));
		filterListTypeChoice->Append(wxT("Capability"));
		filterListTypeChoice->Append(wxT("Default"));
	} else if (sfsApp != NULL) {
		rowProvider = sfsApp->getRowProvider();
		sfsTable_->setRowProvider(rowProvider);
		SET_TABLE(filterGrid, sfsTable_);

		filterListTypeChoice->Append(wxT("SFS"));
		filterListTypeChoice->Append(wxT("Default"));
	} else if (ctxApp != NULL) {
		rowProvider = ctxApp->getRowProvider();
		ctxTable_->setRowProvider(rowProvider);
		SET_TABLE(filterGrid, ctxTable_);

		filterListTypeChoice->Append(wxT("CTX"));
	} else if (sbApp != NULL) {
		rowProvider = sbApp->getRowProvider();
		sbTable_->setRowProvider(rowProvider);
		SET_TABLE(filterGrid, sbTable_);

		filterListTypeChoice->Append(wxT("SB"));
		filterListTypeChoice->Append(wxT("Default"));
	}
	filterListTypeChoice->Select(0);

	setAppPolicyLabel(appPolicy);

	/* Set app move and delete buttons. */
	ruleSet = appPolicy->getParentRuleSet();
	if (ruleSet && (geteuid() == 0 || !ruleSet->isAdmin())) {
		appListUpButton->Enable(appPolicy->canMoveUp());
		appListDownButton->Enable(appPolicy->canMoveDown());
		appListDeleteButton->Enable(appPolicy->canDelete());
	} else {
		appListUpButton->Disable();
		appListDownButton->Disable();
		appListDeleteButton->Disable();
	}

	/* Adjust type choice of filter policy creation. */
	filterListCreateButton->Enable(true);
	filterListTypeChoice->Enable(true);
	appListTypeChoice->SetStringSelection(appPolicy->getTypeIdentifier());

	/* create tabs for each binary */
	appPolicyPanels->deselect();
	appPolicyPanels->select(appPolicy);

	filterPolicyPanels->resetTabSelection();
	filterPolicyPanels->deselect();
	filterListUpButton->Disable();
	filterListDownButton->Disable();
	filterListDeleteButton->Disable();

	/* Ensure the changes will apear on the screen */
	Layout();
	Refresh();

	/* Select the whole row and skip event to be processed by grid. */
	appGrid->SelectRow(event.GetRow());
	event.Skip();
}

void
DlgRuleEditor::onAppGridLabelClick(wxGridEvent &event)
{
	if (event.GetCol() == -1) {
		/* A Row-Lable was clicked. */
		selectRow(appGrid, event.GetRow());
	}
	/* We don't skip the event, to avoid further actions. */
}

void
DlgRuleEditor::onFilterGridCellSelect(wxGridEvent &event)
{
	AnTable			*table;
	AnListClass		*item;
	AnRowProvider		*rowProvider;
	PolicyRuleSet		*ruleSet;
	FilterPolicy		*policy;

	table = dynamic_cast<AnTable*>(filterGrid->GetTable());
	if (table == NULL) {
		return;
	}

	rowProvider = table->getRowProvider();
	if (rowProvider == NULL) {
		return;
	}

	item = rowProvider->getRow(event.GetRow());
	policy = dynamic_cast<FilterPolicy*>(item);
	if (policy == NULL) {
		return;
	}

	ruleSet = policy->getParentRuleSet();
	if (ruleSet && (geteuid() == 0 || !ruleSet->isAdmin())) {
		filterListUpButton->Enable(policy->canMoveUp());
		filterListDownButton->Enable(policy->canMoveDown());
		filterListDeleteButton->Enable();
	} else {
		filterListUpButton->Disable();
		filterListDownButton->Disable();
		filterListDeleteButton->Disable();
	}

	/* Tell the notebook tabs to show themself. */
	filterPolicyPanels->deselect();
	filterPolicyPanels->select(policy);

	Layout();
	Refresh();

	/* Select the whole row and skip event to select cell. */
	filterGrid->SelectRow(event.GetRow());
	event.Skip();
}

void
DlgRuleEditor::onFilterGridLabelClick(wxGridEvent &event)
{
	if (event.GetCol() == -1) {
		/* A Row-Lable was clicked. */
		selectRow(filterGrid, event.GetRow());
	}
	/* We don't skip the event, to avoid further actions. */
}

void
DlgRuleEditor::onAppListUpClick(wxCommandEvent &)
{
	long	 index;
	Policy	*policy;

	index  = appGrid->GetGridCursorRow();
	policy = NULL;

	if (index > 0) {
		policy = getSelectedPolicy(appGrid);
		if (policy != NULL) {
			if (policy->moveUp()) {
				index--;
			}
			loadRuleSet();
			/*
			 * No need to mess with the filter list. It's
			 * contents do not change.
			 */
			selectRow(appGrid, index);
			appListUpButton->Hide();
			appListUpButton->Show();
		}
	}
}

void
DlgRuleEditor::onAppListDownClick(wxCommandEvent &)
{
	long	 index;
	Policy	*policy;

	index  = appGrid->GetGridCursorRow();
	policy = NULL;

	if (index >= 0) {
		policy = getSelectedPolicy(appGrid);
		if (policy != NULL) {
			if (policy->moveDown()) {
				index++;
			}
			loadRuleSet();
			/*
			 * No need to mess with the filter list. It's
			 * contents do not change.
			 */
			selectRow(appGrid, index);
			appListDownButton->Hide();
			appListDownButton->Show();
		}
	}
}

void
DlgRuleEditor::onFilterListUpClick(wxCommandEvent &)
{
	long	 appIndex;
	long	 filterIndex;
	Policy	*policy;

	appIndex = appGrid->GetGridCursorRow();
	filterIndex = filterGrid->GetGridCursorRow();
	policy = NULL;

	if (filterIndex > 0) {
		policy = getSelectedPolicy(filterGrid);
		if (policy != NULL) {
			if (policy->moveUp()) {
				filterIndex--;
			}
			loadRuleSet();
			selectRow(appGrid, appIndex);
			selectRow(filterGrid, filterIndex);
			filterListUpButton->Hide();
			filterListUpButton->Show();
		}
	}
}

void
DlgRuleEditor::onFilterListDownClick(wxCommandEvent &)
{
	long	 appIndex;
	long	 filterIndex;
	Policy	*policy;

	appIndex = appGrid->GetGridCursorRow();
	filterIndex = filterGrid->GetGridCursorRow();
	policy = NULL;

	if (filterIndex >= 0) {
		policy = getSelectedPolicy(filterGrid);
		if (policy != NULL) {
			if (policy->moveDown()) {
				filterIndex++;
			}
			loadRuleSet();
			selectRow(appGrid, appIndex);
			selectRow(filterGrid, filterIndex);
			filterListDownButton->Hide();
			filterListDownButton->Show();
		}
	}
}

void
DlgRuleEditor::onAppListDeleteClick(wxCommandEvent &)
{
	bool		 isLast;
	int		 answer;
	long		 index;
	wxString	 question;
	Policy		*policy;

	isLast = false;
	index  = appGrid->GetGridCursorRow();
	policy = NULL;

	if (index >= 0) {
		policy = getSelectedPolicy(appGrid);
		if (policy == NULL) {
			return;
		}
		question.Printf(_("Delete rule %ld?"), policy->getApnRuleId());
		answer = anMessageBox(question, _("Confirm"), wxYES_NO, this);
		if (answer != wxYES) {
			return;
		}

		if (appGrid->GetNumberRows() == (index + 1)) {
			isLast = true;
		}

		filterPolicyPanels->resetTabSelection();
		filterPolicyPanels->deselect();
		filterListUpButton->Disable();
		filterListDownButton->Disable();
		filterListDeleteButton->Disable();
		if (policy->remove() && index && isLast) {
			index--;
		}
		loadRuleSet();
		selectRow(appGrid, index);
		appListDeleteButton->Hide();
		appListDeleteButton->Show();
	}
}

void
DlgRuleEditor::onFilterListDeleteClick(wxCommandEvent &)
{
	bool		 isLast;
	int		 answer;
	long		 appIndex;
	long		 filterIndex;
	wxString	 question;
	Policy		*policy;

	isLast = false;
	appIndex = appGrid->GetGridCursorRow();
	filterIndex = filterGrid->GetGridCursorRow();
	policy = NULL;

	if (filterIndex >= 0) {
		policy = getSelectedPolicy(filterGrid);
		if (policy == NULL) {
			return;
		}
		question.Printf(_("Delete rule %ld?"), policy->getApnRuleId());
		answer = anMessageBox(question, _("Confirm"), wxYES_NO, this);
		if (answer != wxYES) {
			return;
		}

		if (filterGrid->GetNumberRows() == (filterIndex + 1)) {
			isLast = true;
		}

		if (policy->remove() && filterIndex && isLast) {
			filterIndex--;
		}
		loadRuleSet();
		selectRow(appGrid, appIndex);
		selectRow(filterGrid, filterIndex);
		filterListDeleteButton->Hide();
		filterListDeleteButton->Show();
	}
}

void
DlgRuleEditor::onAppListColumnsButtonClick(wxCommandEvent &)
{
	wxGridEvent	event;

	event.SetEventType(wxEVT_GRID_LABEL_RIGHT_CLICK);
	wxPostEvent(appGrid, event);
}

void
DlgRuleEditor::onFilterListColumnsButtonClick(wxCommandEvent &)
{
	wxGridEvent	event;

	event.SetEventType(wxEVT_GRID_LABEL_RIGHT_CLICK);
	wxPostEvent(filterGrid, event);
}

void
DlgRuleEditor::onAppListCreateButton(wxCommandEvent &)
{
	unsigned int	 id;
	unsigned int	 type;
	long		 index;
	long		 indexSuggestion;
	long		 ruleSet_id;
	wxString	 typeSelection;
	wxString	 message;
	Policy		*policy;
	PolicyCtrl	*policyCtrl;
	PolicyRuleSet	*ruleSet;

	index = appGrid->GetGridCursorRow();
	typeSelection = appListTypeChoice->GetStringSelection();
	policyCtrl = PolicyCtrl::getInstance();
	policy = getSelectedPolicy(appGrid);
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
			anMessageBox(message, _("Rule Editor"),
			    wxOK | wxICON_ERROR, this);
			return;
		} else {
			message = _("Created new ruleset.");
			anMessageBox(message, _("Rule Editor"),
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

	indexSuggestion = ruleSet->createPolicy(type, id, NULL);
	if (indexSuggestion < 0) {
		message = _("Error: Couldn't create new application rule.");
		anMessageBox(message, _("Rule Editor"), wxOK | wxICON_ERROR,
		    this);
	}
	if (index < 0) {
		index = indexSuggestion;
	}

	filterPolicyPanels->resetTabSelection();
	filterPolicyPanels->deselect();
	filterListUpButton->Disable();
	filterListDownButton->Disable();
	filterListDeleteButton->Disable();

	loadRuleSet();
	selectRow(appGrid, index);
	appListCreateButton->Hide();
	appListCreateButton->Show();
}

void
DlgRuleEditor::onFilterListCreateButton(wxCommandEvent &)
{
	unsigned int	 id;
	unsigned int	 type;
	long		 appIndex;
	long		 filterIndex;
	wxString	 typeSelection;
	Policy		*policy;
	AppPolicy	*parent;
	PolicyCtrl	*policyCtrl;
	PolicyRuleSet	*ruleSet;

	appIndex    = appGrid->GetGridCursorRow();
	filterIndex = filterGrid->GetGridCursorRow();
	policyCtrl  = PolicyCtrl::getInstance();

	policy = getSelectedPolicy(appGrid);
	parent = dynamic_cast<AppPolicy*>(policy);
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
		anMessageBox(message, _("RuleEditor"),
		    wxOK | wxICON_ERROR, this);
		return;
	}

	/* Where should the new policy been inserted? */
	policy = getSelectedPolicy(filterGrid);
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
		if (dynamic_cast<SfsAppPolicy*>(parent)) {
			type = APN_SFS_DEFAULT;
		} else {
			type = APN_DEFAULT;
		}
	} else {
		/* No valid policy type. */
		return;
	}

	if (ruleSet->createPolicy(type, id, parent) < 0) {
		anMessageBox(
		    _("Error: Couldn't create new filter rule."),
		    _("Rule Editor"), wxOK | wxICON_ERROR, this);
	}

	loadRuleSet();
	selectRow(appGrid, appIndex);
	selectRow(filterGrid, filterIndex);
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
			anMessageBox(
			    _("Couldn't import policy file: it has errors."),
			    _("Error"), wxICON_ERROR, this);
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
			anMessageBox(
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
			anMessageBox(message, _("Key Load Error"),
			    wxOK|wxICON_ERROR, this);
			footerStatusText->SetLabel(wxT("Wrong password!"));
			break;
		case PolicyCtrl::RESULT_POL_ERR:
			message = _("An error occured while sending policy to"
			    " daemon.");
			anMessageBox(message, _("Policy Load Error"),
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
			anMessageBox(message, _("Key Load Error"),
			    wxOK|wxICON_ERROR, this);
			footerStatusText->SetLabel(wxT("Wrong password!"));
			break;
		case PolicyCtrl::RESULT_POL_ERR:
			message = _("An error occured while sending admin"
			    " policy to the daemon.");
			anMessageBox(message, _("Policy Load Error"),
			    wxOK|wxICON_ERROR, this);
			footerStatusText->SetLabel(wxT("Error while sending"
			    " admin policy to daemon."));
			break;
		case PolicyCtrl::RESULT_POL_ABORT:
		case PolicyCtrl::RESULT_POL_OK:
			break;
		}
	}
}

Policy *
DlgRuleEditor::getSelectedPolicy(wxGrid *grid)
{
	int		 idx;
	Policy		*policy;
	AnTable		*table;
	AnRowProvider	*rowProvider;

	table = dynamic_cast<AnTable*>(grid->GetTable());
	if (table == NULL) {
		return (NULL);
	}
	rowProvider = table->getRowProvider();
	if (rowProvider == NULL) {
		return (NULL);
	}

	idx = grid->GetGridCursorRow();
	policy = dynamic_cast<Policy*>(rowProvider->getRow(idx));

	return (policy);
}

void
DlgRuleEditor::selectRow(wxGrid *grid, int index) const
{
	if (grid != NULL) {
		grid->SelectRow(index);
		grid->SetGridCursor(index, 0);
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
	PolicyRuleSet	*ruleSet;
	PolicyCtrl	*policyCtrl;

	policyCtrl = PolicyCtrl::getInstance();
	multiRowProvider_.clear();

	/* Load ruleset with user-policies. */
	if (userRuleSetId_ != -1) {
		ruleSet = policyCtrl->getRuleSet(userRuleSetId_);
		multiRowProvider_.addRowProvider(ruleSet->getRowProvider());
	}

	/* Load ruleset with admin-policies. */
	if (adminRuleSetId_ != -1) {
		ruleSet = policyCtrl->getRuleSet(adminRuleSetId_);
		multiRowProvider_.addRowProvider(ruleSet->getRowProvider());
	}

	appTable_->setRowProvider(&multiRowProvider_);
	selectRow(appGrid, appGrid->GetGridCursorRow());
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
DlgRuleEditor::setAppPolicyLabel(AppPolicy *appPolicy)
{
	wxString    newLabel;

	/* Show selected policy below application list */
	newLabel.Printf(_("%ls %ls"), appPolicy->getTypeIdentifier().c_str(),
	    appPolicy->getBinaryName().c_str());
	appListPolicyText->SetLabel(newLabel);
}


void
DlgRuleEditor::setUser(long uid)
{
	PolicyCtrl	*policyCtrl = PolicyCtrl::getInstance();
	long		 user = -1, admin = -1;

	editorUid_ = uid;
	if (rb_userMe->GetValue()) {
		user = policyCtrl->getUserId();
	} else if (rb_userDefault->GetValue()) {
		user = policyCtrl->getUserId((uid_t)-1);
		if (user < 0)
			policyCtrl->receiveOneFromDaemon(1, (uid_t)-1);
	}
	admin = policyCtrl->getAdminId(uid);
	if (admin < 0) {
		policyCtrl->receiveOneFromDaemon(0, uid);
		admin = policyCtrl->getAdminId(uid);
	}
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
		anMessageBox(msg, _("RuleEditor"), wxOK | wxICON_ERROR, this);
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
DlgRuleEditor::onRbUserDefault(wxCommandEvent &)
{
	tx_userSelect->Disable();
	setUser((long)-1);
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
