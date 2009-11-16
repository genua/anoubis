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

#include "SfsOverviewAttrProvider.h"

SfsOverviewAttrProvider::SfsOverviewAttrProvider(void)
{
	attrFilterEven_ = new wxGridCellAttr;
	attrFilterEven_->SetBackgroundColour(wxColour(0xee, 0xee, 0xee));
}

SfsOverviewAttrProvider::~SfsOverviewAttrProvider(void)
{
	attrFilterEven_->DecRef();
}

wxGridCellAttr *
SfsOverviewAttrProvider::GetAttr(int row, int col,
    wxGridCellAttr::wxAttrKind kind) const
{
	wxGridCellAttr *attr = wxGridCellAttrProvider::GetAttr(row, col, kind);

	if (row % 2 == 0) { /* Even row */
		attr = setupAttr(attr, attrFilterEven_);
	}

	return (attr);
}

inline wxGridCellAttr *
SfsOverviewAttrProvider::setupAttr(
    wxGridCellAttr *in, wxGridCellAttr *tpl) const
{
	if (in == 0) {
		/* Template is used */
		wxGridCellAttr *out = tpl;
		out->IncRef();

		return (out);
	} else {
		/* Copy properties from template */
		wxGridCellAttr *out = in->Clone();
		out->SetBackgroundColour(in->GetBackgroundColour());

		in->DecRef();

		return (out);
	}
}
