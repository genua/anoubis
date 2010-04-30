/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include <wx/settings.h>

#include "AnRowProvider.h"
#include "Policy.h"
#include "PolicyRuleSet.h"
#include "DlgRuleEditorAttrProvider.h"

DlgRuleEditorAttrProvider::DlgRuleEditorAttrProvider(AnTable *table)
{
	wxColor	grey;

	/* Get system color of greyed (disabled) text. */
	grey = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);

	table_ = table;
	adminTemplateAttr_ = new wxGridCellAttr;
	adminTemplateAttr_->SetTextColour(grey);
}

DlgRuleEditorAttrProvider::~DlgRuleEditorAttrProvider(void)
{
	adminTemplateAttr_->DecRef();
}

wxGridCellAttr *
DlgRuleEditorAttrProvider::GetAttr(int row, int col,
    wxGridCellAttr::wxAttrKind kind) const
{
	Policy		*policy		= NULL;
	PolicyRuleSet	*ruleSet	= NULL;
	wxGridCellAttr	*attr		= NULL;
	AnRowProvider	*rowProvider	= NULL;

	attr = wxGridCellAttrProvider::GetAttr(row, col, kind);

	if (table_ ==  NULL) {
		/* Can't get row information. */
		return (attr);
	}

	rowProvider = table_->getRowProvider();
	if (rowProvider == NULL) {
		/* Can't get row information. */
		return (attr);
	}

	policy = dynamic_cast<Policy*>(rowProvider->getRow(row));
	if (policy == NULL) {
		/* Can't get row information. */
		return (attr);
	}

	ruleSet = policy->getParentRuleSet();
	/*
	 * If we
	 *   - successfully fetched the ruleSet
	 *   - we are not root
	 *   - the ruleSet contains the admin policies
	 * then apply admin template.
	 */
	if ((ruleSet != NULL) && (geteuid() != 0) && ruleSet->isAdmin()) {
		attr = setupAttr(attr);
	}

	return (attr);
}

inline wxGridCellAttr *
DlgRuleEditorAttrProvider::setupAttr(wxGridCellAttr *in) const
{
	wxGridCellAttr	*out = NULL;

	if (in == NULL) {
		/* Use the template. */
		out = adminTemplateAttr_;
		out->IncRef();
	} else {
		/* Copy properties from template. */
		out = in->Clone();
		out->SetBackgroundColour(in->GetBackgroundColour());
		in->DecRef();
	}

	return (out);
}
