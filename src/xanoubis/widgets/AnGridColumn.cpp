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

#include <wx/config.h>

#include "AnGridColumn.h"

bool
AnGridColumn::isVisible(void) const
{
	return (visibility_);
}

void
AnGridColumn::setVisibility(bool visibility)
{
	visibility_ = visibility;
}

AnListProperty *
AnGridColumn::getProperty(void) const
{
	return (property_);
}

int
AnGridColumn::getWidth(void) const
{
	return (width_);
}

void
AnGridColumn::setWidth(int width)
{
	width_ = width;
}

AnGridColumn::AnGridColumn(AnListProperty *property,
    const wxString & configPath, int width)
{
	wxString	key;

	property_ = property;
	configPath_ = configPath;

	if (!configPath_.IsEmpty()) {
		key = configPath_ + wxT("visibility");
		wxConfig::Get()->Read(key, &visibility_, true);

		key = configPath_ + wxT("width");
		wxConfig::Get()->Read(key, &width_, width);
	} else {
		visibility_ = true;
		width_ = width;
	}
}

AnGridColumn::~AnGridColumn(void)
{
	wxString	key;

	if (!configPath_.IsEmpty()) {
		key = configPath_ + wxT("visibility");
		wxConfig::Get()->Write(key, visibility_);

		key = configPath_ + wxT("width");
		wxConfig::Get()->Write(key, width_);

		wxConfig::Get()->Flush();
	}
}
