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

#ifndef _STRINGLISTMODEL_H_
#define _STRINGLISTMODEL_H_

#include <vector>

#include <AnRowProvider.h>

/**
 * A thin wrapper around a wxString.
 *
 * This class is used to put a wxString into a AnRowProvider-derived model.
 *
 * The str()-method is used to retrieve the original string.
 */
class StringWrapper : public AnListClass
{
	public:
		/**
		 * Constructs the wrapper around the given string.
		 *
		 * @param str The string to be wrapped
		 */
		StringWrapper(const wxString &str)
		{
			str_ = str;
		}

		/**
		 * Returns the wrapped string:
		 * @return The string
		 */
		wxString str(void) const
		{
			return (str_);
		}

	private:
		/**
		 * The wrapped string
		 */
		wxString str_;
};

/**
 * A generic model maintains a list of strings.
 *
 * YOu have the possibility to add, remove and query strings from the model.
 */
class StringListModel : public AnRowProvider
{
	public:
		/**
		 * D'tor.
		 */
		~StringListModel(void);

		/**
		 * Appends a string at the end of the model.
		 *
		 * @param str String to be appended
		 */
		void add(const wxString &);

		/**
		 * Appends a string at the given index.
		 *
		 * @param str String to be added
		 * @param idx The index, where the string is inserted. If the
		 *            index is out of range, the string is simply
		 *            appended.
		 */
		void add(const wxString &, unsigned int);

		/**
		 * Removes the string from model.
		 *
		 * Nothing happends, if the string is not part of the model.
		 *
		 * @param str The string to be removed from the model
		 */
		void remove(const wxString &);

		/**
		 * Removes the string at the given index.
		 *
		 * Nothing happens, if the index is out of range.
		 *
		 * @param idx The model-index to be removed
		 */
		void remove(unsigned int);

		/**
		 * Removes all elements from the model.
		 *
		 * Afterwards the model is empty.
		 */
		void clear(void);

		/**
		 * Searches the given string.
		 *
		 * @param str The string you are looking for
		 * @return Index of the requested string. If the string is not
		 *         part of the model, -1 is returned.
		 */
		int find(const wxString &) const;

		/**
		 * Returns the number of strings in the model.
		 *
		 * @return Number of strings
		 */
		unsigned int count(void) const;

		/**
		 * Returns the string at the given index.
		 *
		 * @param idx The requested index
		 * @return The string at the given index. If the index is out
		 *         of range, an empty string is returned.
		 */
		wxString get(unsigned int) const;

		/**
		 * Returns the string at the given index.
		 *
		 * @param idx The requested index
		 * @return The string at the given index. If the index is out
		 *         of range, an empty string is returned.
		 */
		wxString operator[](unsigned int) const;

		/**
		 * Implementation of AnRowProvider::getRow().
		 *
		 * Returns a StringWrapper-instance of the string at the given
		 * index.
		 *
		 * @param idx The requested index
		 * @return The StringProxy-instance at the given index. If the
		 *         index is out of range, NULL is returned.
		 */
		AnListClass *getRow(unsigned int) const;

		/**
		 * Implementation of AnRowProvider::getSize().
		 *
		 * Returns the number of elements of the model.
		 * @return Number of strings.
		 */
		int getSize(void) const;

	private:
		/**
		 * List of model-elements.
		 */
		std::vector<StringWrapper *> stringList_;
};

#endif	/* _STRINGLISTMODEL_H_ */
