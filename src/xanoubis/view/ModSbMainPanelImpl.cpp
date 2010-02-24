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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ModSbMainPanelImpl.h"
#include "OverviewAttrProvider.h"
#include "PolicyRuleSet.h"
#include "SbOverviewTable.h"
#include "SimpleOverviewRow.h"

ModSbMainPanelImpl::ModSbMainPanelImpl(wxWindow* parent, wxWindowID id) :
    ModSbMainPanelBase(parent, id)
{
	SbOverviewTable *table = new SbOverviewTable;
	table->SetAttrProvider(new OverviewAttrProvider(table));
	lst_Rules->SetTable(table, true, wxGrid::wxGridSelectRows);

	lst_Rules->SetColSize(0, 204);
	lst_Rules->SetColSize(1, 204);
	lst_Rules->SetColSize(2, 90);
	lst_Rules->SetColSize(3, 70);
	lst_Rules->SetColSize(4, 50);
	lst_Rules->SetColSize(5, 90);
	lst_Rules->SetColSize(6, 50);
}

ModSbMainPanelImpl::~ModSbMainPanelImpl()
{
}

void
ModSbMainPanelImpl::OnGridCellLeftDClick(wxGridEvent& event)
{
	wxCommandEvent		showEvent(anEVT_SHOW_RULE);
	SbOverviewTable	*table;
	SimpleOverviewRow	*tableRow;
	Policy			*policy;
	PolicyRuleSet		*ruleset;

	table = dynamic_cast<SbOverviewTable *> (lst_Rules->GetTable());
	if (table == NULL)
		return;

	tableRow = table->getRowAt(event.GetRow());

	policy = tableRow->getFilterPolicy();
	if (policy == NULL)
		policy = tableRow->getApplicationPolicy();
	if (policy == NULL)
		return;

	ruleset = policy->getParentRuleSet();
	if (ruleset == NULL)
		return;

	showEvent.SetInt(ruleset->isAdmin());
	showEvent.SetExtraLong(policy->getApnRuleId());
	wxPostEvent(AnEvents::getInstance(), showEvent);
}
