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

#include "SbModel.h"
#include "SbModelRowProvider.h"

SbModelRowProvider::SbModelRowProvider(SbModel *model,
    SbEntry::Permission permission)
{
	this->model_ = model;
	this->permission_ = permission;

	model->Connect(anEVT_ROW_SIZECHANGE,
	    wxCommandEventHandler(SbModelRowProvider::onSizeChange),
	    NULL, this);
	model->Connect(anEVT_ROW_UPDATE,
	    wxCommandEventHandler(SbModelRowProvider::onRowChange),
	    NULL, this);
}

SbModelRowProvider::~SbModelRowProvider(void)
{
	model_->Disconnect(anEVT_ROW_SIZECHANGE,
	    wxCommandEventHandler(SbModelRowProvider::onSizeChange),
	    NULL, this);
	model_->Disconnect(anEVT_ROW_UPDATE,
	    wxCommandEventHandler(SbModelRowProvider::onRowChange),
	    NULL, this);
}

SbEntry *
SbModelRowProvider::getEntryAt(unsigned int idx) const
{
	return (dynamic_cast<SbEntry *>(getRow(idx)));
}

AnListClass *
SbModelRowProvider::getRow(unsigned int idx) const
{
	unsigned int this_idx = 0;

	/* You have another indexing than the model. Depends on permission. */
	for (unsigned int i = 0; i < model_->getEntryCount(); i++) {
		SbEntry *entry = model_->getEntry(i);

		/* Entry for this row-provider found */
		if (entry->hasPermission(permission_)) {
			if (this_idx == idx) {
				/* This is the entry you asked for */
				return (entry);
			}

			this_idx++;
		}
	}

	return (0);
}

int
SbModelRowProvider::getSize(void) const
{
	unsigned int size = 0;

	/*
	 * Collect all SbEntries with the permission-flag assigned to this
	 * class.
	 */
	for (unsigned int i = 0; i < model_->getEntryCount(); i++) {
		SbEntry *entry = model_->getEntry(i);
		if (entry->hasPermission(permission_))
			size++;
	}

	return (size);
}

void
SbModelRowProvider::onSizeChange(wxCommandEvent &event)
{
	event.Skip();

	/*
	 * Size of model has changed. Also inform any view. But use the number
	 * of rows of the row-provider. The number might be less than the
	 * number of sandbox-entries in the model.
	 */
	sizeChangeEvent(getSize());
}

void
SbModelRowProvider::onRowChange(wxCommandEvent &event)
{
	event.Skip();

	/*
	 * A sandbox-entry of the model has changed. But the model-index of
	 * the entry might be different from the row-index of this
	 * row-provider. You need to map the model-index to the
	 * row-provider-index.
	 */

	unsigned int idx = event.GetInt(); /* Model-index */
	unsigned int this_idx = 0; /* Row-provider-index */

	SbEntry *entry = model_->getEntry(idx);
	if (entry == 0 || !entry->hasPermission(permission_)) {
		/* Entry does not belong to provider, no event needed */
		return;
	}

	/* Calculate row-provider-index */
	for (unsigned int i = 0; i <= idx; i++) {
		SbEntry *e = model_->getEntry(i);

		if (e == entry) {
			/* Entry found, fire event */
			rowChangeEvent(this_idx, this_idx);
			return;
		}

		/* This entry belongs to the row-provider, increate index */
		if (e->hasPermission(permission_))
			this_idx++;
	}
}
