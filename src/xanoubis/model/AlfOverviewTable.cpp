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

#include "AlfOverviewTable.h"
#include "MainUtils.h"
#include "PolicyRuleSet.h"
#include "SimpleOverviewRow.h"

AlfOverviewTable::AlfOverviewTable(void)
    : SimpleOverviewTable(typeid(AlfAppPolicy))
{
}

AlfOverviewTable::~AlfOverviewTable(void)
{
}

int
AlfOverviewTable::GetNumberRows()
{
	return (getNumRows());
}

int
AlfOverviewTable::GetNumberCols()
{
	return (COL_EOF);
}

wxString
AlfOverviewTable::GetColLabelValue(int col)
{
	switch (col) {
	case COL_PROGRAM:
		return _("Program");
	case COL_SERVICE:
		return _("Service");
	case COL_ROLE:
		return _("Role");
	case COL_ACTION:
		return _("Action");
	case COL_RESTRICTIONS:
		return _("Restrictions");
	case COL_USER: return _("User");
	}

	return _("???");
}

bool
AlfOverviewTable::IsEmptyCell(int row, int col)
{
	if (row >= getNumRows())
		return (true);

	SimpleOverviewRow *tableRow = getRowAt(row);

	if (tableRow == 0)
		return (true);
	else if ((col == COL_PROGRAM) &&
	    (tableRow->getApplicationPolicy() == 0))
		return (true);
	else if ((col > COL_PROGRAM) && (tableRow->getFilterPolicy() == 0))
		return (true);
	else
		return (false);
}

wxString
AlfOverviewTable::GetValue(int row, int col)
{
	if (row >= getNumRows())
		return _("???");

	SimpleOverviewRow *tableRow = getRowAt(row);

	if (tableRow == 0)
		return _("???");

	switch (col) {
	case COL_PROGRAM:
		return (getProgramText(tableRow));
	case COL_USER:
		return (getUserText(tableRow));
	default:
		return (getFilterText(col, tableRow));
	}
}

void
AlfOverviewTable::SetValue(int WXUNUSED(row), int WXUNUSED(col),
    const wxString & WXUNUSED(value))
{
	/* Unused. Readonly table */
}

wxString
AlfOverviewTable::getProgramText(SimpleOverviewRow *row) const
{
	AppPolicy *appPolicy;
	unsigned int filterIndex;

	if ((appPolicy = row->getApplicationPolicy()) == 0)
		return (wxEmptyString);
	filterIndex = row->getFilterPolicyIndex();

	if (filterIndex == 0 && appPolicy->getBinaryCount() == 0)
		return (wxT("any"));
	else if (filterIndex == 1 && appPolicy->getBinaryCount() == 0 &&
	    appPolicy->getFlag(APN_RULE_PGONLY))
		return _("(applies to playground processes only)");
	else if (filterIndex < appPolicy->getBinaryCount())
		return (appPolicy->getBinaryName(filterIndex));
	else if (appPolicy->getFlag(APN_RULE_PGONLY) &&
	    filterIndex == appPolicy->getBinaryCount())
		return _("(applies to playground processes only)");
	else
		return (wxEmptyString);
}

wxString
AlfOverviewTable::getFilterText(int col, SimpleOverviewRow *row) const
{
	FilterPolicy *policy = row->getFilterPolicy();

	if (policy == 0)
		return (wxEmptyString);

	if (dynamic_cast<AlfFilterPolicy*>(policy)) {
		return (getAlfText(col,
		    dynamic_cast<AlfFilterPolicy*>(policy)));
	} else if (dynamic_cast<AlfCapabilityFilterPolicy*>(policy)) {
		return (getAlfCapabilityText(col,
		    dynamic_cast<AlfCapabilityFilterPolicy*>(policy)));
	} else if (dynamic_cast<DefaultFilterPolicy*>(policy)) {
		return (getDefaultText(col,
		    dynamic_cast<DefaultFilterPolicy*>(policy)));
	} else {
		return _("???");
	}
}

wxString
AlfOverviewTable::getUserText(SimpleOverviewRow *row) const
{
	PolicyRuleSet *ruleSet = row->getRuleSet();
	AppPolicy *appPolicy;
	FilterPolicy *filterPolicy;
	unsigned int filterIndex;

	if (ruleSet == 0)
		return _("???");

	if ((appPolicy = row->getApplicationPolicy()) == 0)
		return (wxEmptyString);
	filterPolicy = row->getFilterPolicy();
	filterIndex = row->getFilterPolicyIndex();

	/* The current row does not contain a policy */
	if (appPolicy->isAnyBlock() && filterIndex > 0)
		return wxEmptyString;
	if (!appPolicy->isAnyBlock() &&
	    filterIndex >= appPolicy->getBinaryCount() && filterPolicy == 0)
		return wxEmptyString;

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
AlfOverviewTable::getAlfText(int col, AlfFilterPolicy *policy) const
{
	switch (col) {
	case COL_SERVICE:
		return policy->getServiceName();
	case COL_ROLE:
		return policy->getRoleName();
	case COL_ACTION:
		return policy->getActionName();
	case COL_RESTRICTIONS:
		return policy->getRestrictionName();
	}

	return wxEmptyString;
}

wxString
AlfOverviewTable::getAlfCapabilityText(int col,
    AlfCapabilityFilterPolicy *policy) const
{
	switch (col) {
	case COL_SERVICE:
		return (wxT("capability ") + policy->getCapabilityTypeName());
	case COL_ACTION:
		return (policy->getActionName());
	case COL_RESTRICTIONS:
		return policy->getRestrictionName();
	}

	return (wxEmptyString);
}

wxString
AlfOverviewTable::getDefaultText(int col, DefaultFilterPolicy *policy) const
{
	switch (col) {
	case COL_SERVICE:
		return (wxT("default"));
	case COL_ACTION:
		return (policy->getActionName());
	case COL_RESTRICTIONS:
		return policy->getRestrictionName();
	}

	return (wxEmptyString);
}
