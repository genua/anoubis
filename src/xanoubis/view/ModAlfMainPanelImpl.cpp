/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#include "AlfOverviewTable.h"
#include "ModAlfMainPanelImpl.h"
#include "OverviewAttrProvider.h"
#include "PolicyRuleSet.h"
#include "SimpleOverviewRow.h"

ModAlfMainPanelImpl::ModAlfMainPanelImpl(wxWindow* parent,
    wxWindowID id) : ModAlfMainPanelBase(parent, id)
{
	AlfOverviewTable *table = new AlfOverviewTable;
	table->SetAttrProvider(new OverviewAttrProvider(table));
	lst_Rules->SetTable(table, true, wxGrid::wxGridSelectRows);

	lst_Rules->SetColSize(0, 323);
	lst_Rules->SetColSize(1, 170);
	lst_Rules->SetColSize(2, 55);
	lst_Rules->SetColSize(3, 70);
	lst_Rules->SetColSize(4, 90);
	lst_Rules->SetColSize(5, 50);
}

ModAlfMainPanelImpl::~ModAlfMainPanelImpl(void)
{
}

void
ModAlfMainPanelImpl::OnGridCellLeftDClick(wxGridEvent& event)
{
	wxCommandEvent		showEvent(anEVT_SHOW_RULE);
	AlfOverviewTable	*table;
	SimpleOverviewRow	*tableRow;
	FilterPolicy		*policy;
	PolicyRuleSet		*ruleset;

	table		= dynamic_cast<AlfOverviewTable *> (lst_Rules->GetTable());

	if (table == NULL) {
		return;
	}

	tableRow	= table->getRowAt(event.GetRow());
	policy		= tableRow->getFilterPolicy();

	if (policy == NULL) {
		return;
	}

	ruleset = policy->getParentRuleSet();

	if (ruleset == NULL) {
		return;
	}

	showEvent.SetInt(ruleset->isAdmin());
	showEvent.SetExtraLong(policy->getApnRuleId());
	wxPostEvent(AnEvents::getInstance(), showEvent);
}

