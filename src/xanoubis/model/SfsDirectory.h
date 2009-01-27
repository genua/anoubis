/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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

#ifndef _SFSDIRECTORY_H_
#define _SFSDIRECTORY_H_

#include <vector>

#include <wx/dir.h>

#include "SfsEntry.h"

/**
 * A SfsDirectory is the root-folter for a list of SfsEntries.
 * The selection is SfsEntry-instances is configured by setting various
 * attributes in the class. Changing the attributes leads into a reload
 * of the SfsEntry-list.
 */
class SfsDirectory : private wxDirTraverser
{
	public:
		SfsDirectory();

		/**
		 * Returns the root-path.
		 * Files below the directory are insered into the SfsEnty-list.
		 */
		wxString getPath() const;

		/**
		 * Updates the root-path.
		 * @param path The new path
		 * @return true if the SfsEntry-list has changed, false
		 *         otherwise.
		 */
		bool setPath(const wxString &);

		/**
		 * Tests whether recursive traversal through the filesystem is
		 * enabled.
		 *
		 * If enabled, all subsequent directories are visited, too.
		 *
		 * @return true is returned, if recursive traversal is enabled,
		 *         false otherwise.
		 * @note Traversing through the filesystem might take a long
		 *       time!
		 */
		bool isDirTraversalEnabled(void) const;

		/**
		 * Updates the traversal-flag.
		 *
		 * If enabled, all subsequent directories are visited, too.
		 *
		 * @param enabled Set to true, if you want to enabled the
		 *                recursive traversal feature.
		 * @return true, if the SfsEntry-list has changed, false
		 *         otherwise.
		 * @note Updating the SfsEntry-list might take a long time, if
		 *       traversal is enabled!
		 */
		bool setDirTraversal(bool);

		/**
		 * Returns the filename-filter.
		 * Only files, which filename contains the filter are insered
		 * into the SfsEntry-list.
		 */
		wxString getFilter() const;

		/**
		 * Updates the filename-filter.
		 * @param filter The new filter
		 * @return true if the SfsEntry-list has changed, false
		 *         otherwise.
		 */
		bool setFilter(const wxString &);

		/**
		 * Tests, weather the filename-filter is inversed.
		 * If the filename-filter is inversed, only files, which
		 * filename not contains the filter are inserted into the
		 * SfsEntry-list.
		 */
		bool isFilterInversed() const;

		/**
		 * Updates the filter-inversed-flag.
		 * @param inversed The the value
		 * @return true if the SfsEntry-list has changed, false
		 *         otherwise.
		 */
		bool setFilterInversed(bool inverse);

		/**
		 * Returns the number of SfsEntry-instances.
		 * @return Number of SfsEntry-instances
		 */
		unsigned int getNumEntries() const;

		/**
		 * Returns the index of the SfsEntry with the specified
		 * filename.
		 *
		 * @param filename The absolute path the SfsEntry you are
		 *                 searching for.
		 * @return The index of the SfsEntry you are looking for. If
		 *         no such entry exists, -1 is returned.
		 */
		int getIndexOf(const wxString &) const;

		/**
		 * Returns the SfsEntry-instance at the specified index.
		 * @param idx The requested index (count starts at 0)
		 * @return The SfsEntry-instance at the specified index
		 */
		SfsEntry &getEntry(unsigned int);

	private:
		wxString path_;
		bool recursive_;
		wxString filter_;
		bool inverseFilter_;
		std::vector<SfsEntry> entryList_;

		/**
		 * Re-fills entryList_
		 */
		void updateEntryList();

		/**
		 * @see wxDirTraverser::InDir()
		 */
		wxDirTraverseResult OnDir(const wxString &);

		/**
		 * @see wxDirTraverser::InFile()
		 */
		wxDirTraverseResult OnFile(const wxString &);
};

#endif	/* _SFSDIRECTORY_H_ */
