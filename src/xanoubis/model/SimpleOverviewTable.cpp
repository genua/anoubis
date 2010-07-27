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

#include "Debug.h"
#include "PolicyCtrl.h"
#include "PolicyRuleSet.h"
#include "SimpleOverviewRow.h"
#include "SimpleOverviewTable.h"
#include "SimpleOverviewVisitor.h"

SimpleOverviewTable::SimpleOverviewTable(const std::type_info &typeInfo)
    : Observer(0)
{
	userRuleSetId_ = -1;
	adminRuleSetId_ = -1;
	visitor_ = new SimpleOverviewVisitor(
	    (const std::type_info *) &typeInfo);

	AnEvents::instance()->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(SimpleOverviewTable::onLoadRuleSet),
	    NULL, this);
}

SimpleOverviewTable::~SimpleOverviewTable(void)
{
	delete visitor_;
	AnEvents::instance()->Disconnect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(SimpleOverviewTable::onLoadRuleSet),
	    NULL, this);
}

void
SimpleOverviewTable::reload(void)
{
	PolicyCtrl *policyCtrl = PolicyCtrl::instance();
	PolicyRuleSet *ruleSet;

	/* clear the whole list */
	clearTable();

	/* release old ones */
	ruleSet = policyCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		removeSubject(ruleSet);
		ruleSet->unlock();
	}

	ruleSet = policyCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
		removeSubject(ruleSet);
		ruleSet->unlock();
	}

	/* New ruleset-ids */
	userRuleSetId_ = policyCtrl->getUserId();
	adminRuleSetId_ = policyCtrl->getAdminId(geteuid());

	/* get the new ones */
	ruleSet = policyCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		addSubject(ruleSet);
		ruleSet->lock();

		Debug::trace(wxT("Start visiting user-ruleset"));
		visitor_->reset(ruleSet);
		ruleSet->accept(*visitor_);
		Debug::trace(wxT("Finished visiting user-ruleset"));
	}

	ruleSet = policyCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
		addSubject(ruleSet);
		ruleSet->lock();

		Debug::trace(wxT("Start visiting admin-ruleset"));
		visitor_->reset(ruleSet);
		ruleSet->accept(*visitor_);
		Debug::trace(wxT("Finished visiting admin-ruleset"));
	}

	/* Inform the view about the new table */
	fireRowsInserted(0, getNumRows());
}

int
SimpleOverviewTable::getNumRows(void) const
{
	const std::vector<SimpleOverviewRow *> &list = visitor_->getResult();
	return (list.size());
}

SimpleOverviewRow *
SimpleOverviewTable::getRowAt(unsigned int idx) const
{
	const std::vector<SimpleOverviewRow *> &list = visitor_->getResult();
	return (list[idx]);
}

void
SimpleOverviewTable::onLoadRuleSet(wxCommandEvent &event)
{
	reload();
	event.Skip();
}

void
SimpleOverviewTable::update(Subject *)
{
	reload();
}

void
SimpleOverviewTable::updateDelete(Subject *)
{
	reload();
}

void
SimpleOverviewTable::clearTable(void)
{
	int num = getNumRows();
	visitor_->clear();

	/* Inform the view about the clear-operation */
	fireRowsRemoved(0, num);
}

void
SimpleOverviewTable::fireRowsInserted(int start, int count)
{
	if (GetView()) {
		wxGridTableMessage msg(
		    this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED, start, count);
		GetView()->ProcessTableMessage(msg);
	}
}

void
SimpleOverviewTable::fireRowsRemoved(int start, int count)
{
	if (GetView()) {
		wxGridTableMessage msg(
		    this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, start, count);
		GetView()->ProcessTableMessage(msg);
	}
}
