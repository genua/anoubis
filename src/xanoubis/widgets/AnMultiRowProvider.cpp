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

#include "Debug.h"
#include "AnMultiRowProvider.h"

AnMultiRowProvider::AnMultiRowProvider(void)
{
	clear();
}

AnMultiRowProvider::~AnMultiRowProvider(void)
{
	clear();
}

void
AnMultiRowProvider::addRowProvider(AnRowProvider *newRowProvider)
{
	rowProviderList_.push_back(newRowProvider);
}

int
AnMultiRowProvider::getIndexOf(const AnRowProvider *provider) const
{
	int		ret = 0;

	for(unsigned int i=0; i<rowProviderList_.size(); ++i) {
		if (rowProviderList_[i] == provider)
			return ret;
		ret += rowProviderList_[i]->getSize();
	}
	return -1;
}

void
AnMultiRowProvider::clear(void)
{
	rowProviderList_.clear();
}

AnListClass *
AnMultiRowProvider::getRow(unsigned int idx) const
{
	for (unsigned int i=0; i<rowProviderList_.size(); i++) {
		if (idx < (unsigned int)rowProviderList_[i]->getSize()) {
			return rowProviderList_[i]->getRow(idx);
		} else {
			idx -= rowProviderList_[i]->getSize();
		}
	}
	return NULL;
}

int
AnMultiRowProvider::getSize(void) const
{
	int						size = 0;

	for (unsigned int i=0; i<rowProviderList_.size(); ++i)
		size += rowProviderList_[i]->getSize();

	return size;
}
