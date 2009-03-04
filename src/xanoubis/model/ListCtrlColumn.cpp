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

#include "ListCtrlColumn.h"

#include <wx/listctrl.h>

ListCtrlColumn::ListCtrlColumn(void)
{
	index_ = -1;
	title_ = wxEmptyString;
	width_ = -1;
	visability_ = false;
}

ListCtrlColumn::ListCtrlColumn(const wxString &title)
{
	index_ = -1;
	title_ = title;
	width_ = wxLIST_AUTOSIZE_USEHEADER;
	visability_ = true;
}

ListCtrlColumn::ListCtrlColumn(const wxString &title, const wxString &confkey)
{
	index_ = -1;
	title_ = title;
	confkey_ = confkey;
	width_ = wxLIST_AUTOSIZE_USEHEADER;
	visability_ = true;
}

ListCtrlColumn::ListCtrlColumn(const wxString &title, const wxString &confkey,
    int width)
{
	index_ = -1;
	title_ = title;
	confkey_ = confkey;
	width_ = width;
	visability_ = true;
}

void
ListCtrlColumn::setIndex(long index)
{
	index_ = index;
}

long
ListCtrlColumn::getIndex(void) const
{
	if (visability_) {
		return (index_);
	} else {
		return (-1);
	}
}

void
ListCtrlColumn::setTitle(const wxString & title)
{
	title_ = title;
}

wxString
ListCtrlColumn::getTitle(void) const
{
	return (title_);
}

void
ListCtrlColumn::setWidth(int width)
{
	width_ = width;
}

int
ListCtrlColumn::getWidth(void) const
{
	return (width_);
}

void
ListCtrlColumn::setVisability(bool visability)
{
	visability_ = visability;
}

bool
ListCtrlColumn::isVisible(void) const
{
	return (visability_);
}

wxString
ListCtrlColumn::getConfKey(void) const
{
	return (confkey_);
}
