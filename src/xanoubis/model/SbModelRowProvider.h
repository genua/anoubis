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

#ifndef _SBMODELROWPROVIDER_H_
#define _SBMODELROWPROVIDER_H_

#include <AnRowProvider.h>

#include "SbEntry.h"

/**
 * The row-provider of a SbModel.
 *
 * Searches for SbEntries with a specific SbEntry::Permission. So you can
 * maintain all sandbox-entries in one model but display only sandbox-entries
 * with a single permission.
 */
class SbModelRowProvider : public AnRowProvider
{
	public:
		/**
		 * Creates the row-provider.
		 *
		 * @param model The model behind the row-provider
		 * @param permission The permission to be filtered by the
		 *                   row-provider.
		 */
		SbModelRowProvider(SbModel *, SbEntry::Permission);

		/**
		 * D'tor.
		 */
		~SbModelRowProvider(void);

		/**
		 * Returns the sandbox-entry at the given index of the
		 * row-provider.
		 *
		 * @param idx The requested index of row-provider. Can be
		 *            different from index of SbModel!
		 * @return SbEntry at the given index, NULL if the index is
		 *         out of range.
		 */
		SbEntry *getEntryAt(unsigned int) const;

		/**
		 * Implementation of AnRowProvider::getRow().
		 *
		 * Only returns SbEntry-instance with the same permission.
		 */
		AnListClass *getRow(unsigned int) const;

		/**
		 * Implementation of AnRowProvider::getSize().
		 */
		int getSize(void) const;

	private:
		/**
		 * The assigned model.
		 */
		SbModel *model_;

		/**
		 * The permission to be filtered by the row-provider.
		 */
		SbEntry::Permission permission_;

		/**
		 * Invoked if the size of the model has changed.
		 */
		void onSizeChange(wxCommandEvent &);

		/**
		 * Invoked if an entry of the model has changed.
		 */
		void onRowChange(wxCommandEvent &);
};

#endif	/* _SBMODELROWPROVIDER_H_ */
