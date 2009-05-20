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

class SfsEntry;

/**
 * A handler reports the progress of a filesystem-scan.
 */
class SfsDirectoryScanHandler
{
	public:
		virtual ~SfsDirectoryScanHandler() {}

		/**
		 * A scan-operation started.
		 */
		virtual void scanStarts() = 0;

		/**
		 * The filesystem-scan has finished.
		 *
		 * @param aborted Set to true, if the filesystem-scan was
		 *                aborted.
		 */
		virtual void scanFinished(bool) = 0;

		/**
		 * Reports a progress of the scan.
		 *
		 * The visited arguments reports the number of already scanned
		 * directories and is increased with each invocation of the
		 * method. The total argument tells you how many directories
		 * are already known. This value can be greater than visited,
		 * when there are directories, which are currently not scanned.
		 *
		 * @param visited Number of already visited directories
		 * @param total Number of total known directories.
		 */
		virtual void scanProgress(unsigned long, unsigned long) = 0;
};

/**
 * A SfsDirectory is the root-folter for a list of SfsEntries.
 * The selection is SfsEntry-instances is configured by setting various
 * attributes in the class. Changing the attributes leads into a reload
 * of the SfsEntry-list.
 *
 * You can monitor the filesystem-scan by assinging a
 * SfsDirectoryScanHandler-instance. Between
 * SfsDirectoryScanHandler::scanStarts() and
 * SfsDirectoryScanHandler::scanFinished() the filesystem-scan is running. You
 * can abort the scan by calling abortScan(). When a scan is aborted, already
 * scanned files are inserted into the model.
 */
class SfsDirectory : private wxDirTraverser
{
	public:
		SfsDirectory();
		~SfsDirectory();

		/**
		 * Returns the assigned SfsDirectoryScanHandler.
		 *
		 * This handler can be used to monitor the current progress
		 * of a filesystem-scan while the model is reloaded.
		 *
		 * @return Assigned SfsDirectoryScanHandler
		 */
		SfsDirectoryScanHandler *getScanHandler(void) const;

		/**
		 * Assigns a SfsDirectoryScanHandler.
		 *
		 * This handler is immediately used for monitoring a
		 * filesystem-scan.
		 *
		 * @param scanHandler The handler to be assigned
		 */
		void setScanHandler(SfsDirectoryScanHandler *);

		/**
		 * Aborts a filesystem-scan.
		 *
		 * Between SfsDirectoryScanHandler::scanStarts() and
		 * SfsDirectoryScanHandler::scanFinished() the filesystem-scan
		 * is running. If currently a scan is running, the scan is
		 * aborted as soon as possible. if currenty no scan is running,
		 * an abort has no effect.
		 */
		void abortScan(void);

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
		SfsEntry *getEntry(unsigned int);

		/**
		 * Inserts a new SfsEntry into the SfsDirectory.
		 *
		 * The method does not check the path! It simply adds the new
		 * entry in correct alphabetical order.
		 *
		 * @param path The path of the file to be inserted
		 * @return The SfsEntry, which was created. If you try to
		 *         insert a path, which was already inserted into the
		 *         model, the corresponding SfsEntry is returned.
		 */
		SfsEntry *insertEntry(const wxString &);

		/**
		 * Removes the SfsEntry at the specified index.
		 *
		 * The model is not touched, if the index is out of range.
		 *
		 * @param idx Index of SfsEntry to be removed from model.
		 */
		void removeEntry(unsigned int);

		/**
		 * Cleans the model.
		 *
		 * Entries, which match one of the following conditions, are
		 * removed from the model:
		 *
		 * - The corresponding file does not exist in the local
		 *   filesystem
		 * - Recursive traversal is disabled, but the entry belongs to
		 *   a sub-directory.
		 *
		 * Cleaning up the model might be necessary, if you inserted
		 * by yourself using insertEntry().
		 *
		 * @return true is returned, if the model has changed (at least
		 *         one SfsEntry was removed from the model).
		 */
		bool cleanup(void);

	private:
		wxString path_;
		bool recursive_;
		wxString filter_;
		bool inverseFilter_;
		std::vector<SfsEntry *> entryList_;
		SfsDirectoryScanHandler *scanHandler_;
		bool abortScan_;
		unsigned int totalDirectories_;
		unsigned int visitedDirectories_;

		/**
		 * Removes all elements from entryList_.
		 */
		void clearEntryList();

		/**
		 * Re-fills entryList_
		 */
		void updateEntryList();

		/**
		 * Inserts a new SfsEntry into the SfsDirectory.
		 *
		 * A binary search is used to find the position, where the
		 * SfsEntry should be inserted. If the correct position is
		 * found, the SfsEntry is created from the path-information
		 * and inserted into entryList_.
		 *
		 * @param path The path of the file to be inserted
		 * @param start First index in entryList_. Search operation
		 *              will start here.
		 * @param end Last index in entryList_. Search operation will
		 *            end here.
		 * @return Index in entryList_, where the SfsEntry was
		 *         inserted.
		 * @pre entryList_ is not empty!
		 */
		int insertEntry(const wxString &, unsigned int, unsigned int);

		/**
		 * @see wxDirTraverser::InDir()
		 */
		wxDirTraverseResult OnDir(const wxString &);

		/**
		 * @see wxDirTraverser::InFile()
		 */
		wxDirTraverseResult OnFile(const wxString &);

		/**
		 * Returns the number of sub-directories.
		 *
		 * The method scans the specified directory for
		 * sub-directories.
		 *
		 * @param path The directory to be scanned
		 * @return Number of sub-directories
		 */
		static unsigned int getDirectoryCount(const wxString &);
};

#endif	/* _SFSDIRECTORY_H_ */
