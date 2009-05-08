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

#include "AnListColumn.h"
#include "AnListCtrl.h"
#include "AnListProperty.h"

unsigned int
AnListColumn::getIndex(void) const
{
	return (this->columnIdx_);
}

AnListProperty *
AnListColumn::getProperty(void) const
{
	return (property_);
}

int
AnListColumn::getWidth(void) const
{
	return (columnInfo_.GetWidth());
}

void
AnListColumn::setWidth(int width)
{
	columnInfo_.SetWidth(width);
	updateColumn();
}

wxListColumnFormat
AnListColumn::getAlign(void) const
{
	return (columnInfo_.GetAlign());
}

void
AnListColumn::setAlign(wxListColumnFormat align)
{
	columnInfo_.SetAlign(align);
	updateColumn();
}

AnListColumn::AnListColumn(AnListProperty *property, AnListCtrl *parent)
{
	this->parent_ = parent;
	this->property_ = property;

	columnInfo_.SetWidth(wxLIST_AUTOSIZE);
	columnInfo_.SetAlign(wxLIST_FORMAT_LEFT);
	columnInfo_.SetText(property->getHeader());

	if (parent_ != 0) {
		columnIdx_ = parent->GetColumnCount();
		parent->InsertColumn(columnIdx_, columnInfo_);
	} else
		columnIdx_ = 0;
}

AnListColumn::~AnListColumn(void)
{
	if (property_ != 0)
		delete property_;
}

void
AnListColumn::setIndex(unsigned int idx)
{
	this->columnIdx_ = idx;
}

void
AnListColumn::updateColumn(void)
{
	if (parent_ != 0)
		parent_->SetColumn(columnIdx_, columnInfo_);
}
