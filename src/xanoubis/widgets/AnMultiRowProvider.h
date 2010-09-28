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

#ifndef _ANMULTIROWPROVIDER_H_
#define _ANMULTIROWPROVIDER_H_

#include <vector>

#include "AnRowProvider.h"

/**
 * This class is a implementation of a row provider. But instead of
 * providing own rows, it acts a proxy and asks previously added
 * row provider for their row.
 */
class AnMultiRowProvider : public AnRowProvider
{
	public:
		/**
		 * Constructor.
		 */
		AnMultiRowProvider(void);

		/**
		 * Destructor.
		 */
		~AnMultiRowProvider(void);

		/**
		 * Add a row provider to the list of providerts been
		 * asked for a row.
		 * @param[in] 1st Row provider to add.
		 * @return Nothing.
		 */
		void addRowProvider(AnRowProvider *);

		/**
		 * Return the index of a row provider in the list of
		 * providers. This is useful if you want to search
		 * for a particular item, e.g. an application rule and
		 * highlight it by its current index.
		 *
		 * @param provider The row provider.
		 * @return The index of the first row that is governed
		 *     by this row provider. Negative if the provider is
		 *     not part of the multi row provider.
		 */
		int getIndexOf(const AnRowProvider *provider) const;

		/**
		 * Remove all row providers from the list of providers.
		 * @param None.
		 * @return Nothing.
		 */
		void clear(void);

		/**
		 * Return the element with index idx in the model.
		 * @param idx The index.
		 * @return The object associated with the idex.
		 *     If the index is out range NULL may be returned.
		 */
		virtual AnListClass *getRow(unsigned int) const;

		/**
		 * Return the total number of items in the model.
		 * @return The total number of items in the model.
		 */
		virtual int getSize(void) const;

	private:
		/**
		 * List of row providers.
		 */
		std::vector<AnRowProvider *> rowProviderList_;
};

#endif	/* _ANMULTIROWPROVIDER_H_ */
