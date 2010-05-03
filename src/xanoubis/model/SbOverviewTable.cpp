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

#include "main.h"
#include "PolicyRuleSet.h"
#include "SbAppPolicy.h"
#include "SbOverviewTable.h"
#include "SimpleOverviewRow.h"
#include <typeinfo>

SbOverviewTable::SbOverviewTable(void)
    : SimpleOverviewTable(typeid(SbAppPolicy))
{
}

SbOverviewTable::~SbOverviewTable(void)
{
}

int
SbOverviewTable::GetNumberRows()
{
	return (getNumRows());
}

int
SbOverviewTable::GetNumberCols()
{
	return (COL_EOF);
}

wxString
SbOverviewTable::GetColLabelValue(int col)
{
	switch (col) {
	case COL_PROGRAM:
		return _("Program");
	case COL_PATH:
		return _("Path");
	case COL_SUB:
		return _("Subject");
	case COL_ACTION:
		return _("Action");
	case COL_MASK:
		return _("Mask");
	case COL_SCOPE:
		return _("Temporary");
	case COL_USER:
		return _("User");
	}

	return _("???");
}

bool
SbOverviewTable::IsEmptyCell(int row, int col)
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
SbOverviewTable::GetValue(int row, int col)
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
SbOverviewTable::SetValue(int WXUNUSED(row), int WXUNUSED(col),
    const wxString & WXUNUSED(value))
{
	/* Unused. Readonly table */
}

wxString
SbOverviewTable::getProgramText(SimpleOverviewRow *row) const
{
	AppPolicy *appPolicy;
	unsigned int filterIndex;

	if ((appPolicy = row->getApplicationPolicy()) == 0)
		return (wxEmptyString);
	filterIndex = row->getFilterPolicyIndex();

	if (filterIndex == 0 && appPolicy->getBinaryCount() == 0)
		return (wxT("any"));
	else if (filterIndex < appPolicy->getBinaryCount())
		return (appPolicy->getBinaryName(filterIndex));
	else
		return (wxEmptyString);
}

wxString
SbOverviewTable::getFilterText(int col, SimpleOverviewRow *row) const
{
	FilterPolicy *policy = row->getFilterPolicy();

	if (policy == 0)
		return (wxEmptyString);

	if (dynamic_cast<SbAccessFilterPolicy*>(policy)) {
		return (getSbAccessText(col,
		    dynamic_cast<SbAccessFilterPolicy*>(policy)));
	} else if (dynamic_cast<DefaultFilterPolicy*>(policy)) {
		return (getDefaultText(col,
		    dynamic_cast<DefaultFilterPolicy*>(policy)));
	} else {
		return _("???");
	}
}

wxString
SbOverviewTable::getUserText(SimpleOverviewRow *row) const
{
	PolicyRuleSet *ruleSet = row->getRuleSet();

	if (ruleSet == 0)
		return _("???");

	if (ruleSet->isAdmin()) {
		wxString userName =
		    wxGetApp().getUserNameById(ruleSet->getUid());
		return wxString::Format(_("admin ruleset of %ls"),
		    userName.c_str());
	} else {
		return (wxGetUserId());
	}
}

wxString
SbOverviewTable::getSbAccessText(int col, SbAccessFilterPolicy *policy) const
{
	switch (col) {
	case COL_PATH:
		return (policy->getPath());
	case COL_SUB:
		return (policy->getSubjectName());
	case COL_ACTION:
		return (policy->getActionName());
	case COL_MASK:
		return (policy->getAccessMaskName());
	case COL_SCOPE:
		return (policy->hasScope() ? wxT("T") : wxEmptyString);
	}

	return (wxEmptyString);
}

wxString
SbOverviewTable::getDefaultText(int col, DefaultFilterPolicy *policy) const
{
	switch (col) {
	case COL_PATH:
		return (wxT("default"));
	case COL_ACTION:
		return (policy->getActionName());
	case COL_SCOPE:
		return (policy->hasScope() ? wxT("T") : wxEmptyString);
	}

	return (wxEmptyString);
}
