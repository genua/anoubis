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

#include <typeinfo>

#include "MainUtils.h"
#include "PolicyRuleSet.h"
#include "SfsAppPolicy.h"
#include "SfsOverviewTable.h"
#include "SimpleOverviewRow.h"

SfsOverviewTable::SfsOverviewTable(void)
    : SimpleOverviewTable(typeid(SfsAppPolicy))
{
}

SfsOverviewTable::~SfsOverviewTable(void)
{
}

int
SfsOverviewTable::GetNumberRows()
{
	return (getNumRows());
}

int
SfsOverviewTable::GetNumberCols()
{
	return (COL_EOF);
}

wxString
SfsOverviewTable::GetColLabelValue(int col)
{
	switch (col) {
	case COL_PATH:
		return _("Path");
	case COL_SUB:
		return _("Subject");
	case COL_VA:
		return _("Valid action");
	case COL_IA:
		return _("Invalid action");
	case COL_UA:
		return _("Unknown action");
	case COL_SCOPE:
		return _("Temporary");
	case COL_USER:
		return _("User");
	}

	return _("???");
}

bool
SfsOverviewTable::IsEmptyCell(int, int)
{
	return (false);
}

wxString
SfsOverviewTable::GetValue(int row, int col)
{
	if (row >= getNumRows())
		return _("???");

	SimpleOverviewRow *tableRow = getRowAt(row);

	if (tableRow == 0)
		return _("???");

	switch (col) {
	case COL_USER:
		return (getUserText(tableRow));
	default:
		return (getFilterText(col, tableRow));
	}
}

void
SfsOverviewTable::SetValue(int WXUNUSED(row), int WXUNUSED(col),
    const wxString & WXUNUSED(value))
{
	/* Unused. Readonly table */
}

wxString
SfsOverviewTable::getFilterText(int col, SimpleOverviewRow *row) const
{
	FilterPolicy *policy = row->getFilterPolicy();

	if (policy == 0)
		return (wxEmptyString);

	if (dynamic_cast<SfsFilterPolicy*>(policy)) {
		return (getSfsText(col,
		    dynamic_cast<SfsFilterPolicy*>(policy)));
	} else if (dynamic_cast<SfsDefaultFilterPolicy*>(policy)) {
		return (getSfsDefaultText(col,
		    dynamic_cast<SfsDefaultFilterPolicy*>(policy)));
	} else {
		return _("???");
	}
}

wxString
SfsOverviewTable::getUserText(SimpleOverviewRow *row) const
{
	PolicyRuleSet *ruleSet = row->getRuleSet();

	if (ruleSet == 0)
		return _("???");

	if (ruleSet->isAdmin()) {
		wxString userName = MainUtils::instance()->getUserNameById(
		    ruleSet->getUid());
		return wxString::Format(_("admin ruleset of %ls"),
		    userName.c_str());
	} else {
		return (wxGetUserId());
	}
}

wxString
SfsOverviewTable::getSfsText(int col, SfsFilterPolicy *policy) const
{
	switch (col) {
	case COL_PATH:
		return (policy->getPath());
	case COL_SUB:
		return (policy->getSubjectName());
	case COL_VA:
		return (policy->getValidActionName());
	case COL_IA:
		return (policy->getInvalidActionName());
	case COL_UA:
		return (policy->getUnknownActionName());
	case COL_SCOPE:
		return (policy->hasScope() ? wxT("T") : wxEmptyString);
	}

	return (wxEmptyString);
}

wxString
SfsOverviewTable::getSfsDefaultText(int col,
    SfsDefaultFilterPolicy *policy) const
{
	switch (col) {
	case COL_PATH:
		return (policy->getPath());
	case COL_VA:
		return (wxString::Format(_("default %ls"),
		    policy->getActionName().c_str()));
	case COL_SCOPE:
		return (policy->hasScope() ? wxT("T") : wxEmptyString);
	}

	return (wxEmptyString);
}
