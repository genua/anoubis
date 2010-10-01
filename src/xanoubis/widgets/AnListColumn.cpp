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

#include <wx/config.h>

#include "AnListColumn.h"
#include "AnListCtrl.h"
#include "AnListProperty.h"

static const wxString		visiblekey = wxT("visible");
static const wxString		widthkey = wxT("width");

AnListProperty *
AnListColumn::getProperty(void) const
{
	return property_;
}

void
AnListColumn::setVisible(bool visible)
{
	visible_ = visible;
	if (!configPath_.IsEmpty())
		wxConfig::Get()->Write(configPath_ + visiblekey, visible_);
}

void
AnListColumn::setWidth(int width)
{
	if (!configPath_.IsEmpty()) {
		width_ = width;
		wxConfig::Get()->Write(configPath_ + widthkey, width);
	}
}

AnListColumn::AnListColumn(AnListProperty *property, wxString key,
    int width, bool visible, wxListColumnFormat align)
{
	property_ = property;
	configPath_ = key;
	if (configPath_ != wxEmptyString && !configPath_.EndsWith(wxT("/")))
		configPath_ += wxT("/");

	visible_ = visible;
	if (configPath_ != wxEmptyString) {
		wxConfig::Get()->Read(configPath_ + widthkey, &width, width);
		wxConfig::Get()->Read(configPath_ + visiblekey,
		    &visible_, visible);
	}
	width_ = width;
	align_ = align;
}

AnListColumn::~AnListColumn(void)
{
	/* Save state of column */
	if (configPath_ != wxEmptyString) {
		wxConfig::Get()->Write(configPath_ + widthkey, width_);
		wxConfig::Get()->Write(configPath_ + visiblekey, visible_);
	}
	if (property_ != 0)
		delete property_;
}

wxListItem &
AnListColumn::getColumnInfo(void)
{
	colInfo_.SetWidth(width_);
	colInfo_.SetAlign(align_);
	colInfo_.SetText(property_->getHeader());

	return colInfo_;
}
