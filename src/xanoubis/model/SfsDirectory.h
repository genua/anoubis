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
#include <IndexTree.h>

#include <AnRowProvider.h>

class SfsEntry;

class SfsEntryList : public IndexTree<SfsEntry> {
	int Cmp(const SfsEntry *, const SfsEntry *) const;
};

/**
 * A handler reports the progress of a filesystem-scan.
 * Conceptually, at the toplevel a single item is scaned. However,
 * each item may divided into several steps using scanPush.
 */
class SfsDirectoryScanHandler
{
	public:
		/**
		 * Initialize the class.
		 * @param limit Only call the update function if the total
		 *     number of elements found exceed this limit.
		 */
		SfsDirectoryScanHandler(int limit);

		/**
		 * Destructor.
		 */
		virtual ~SfsDirectoryScanHandler();

		/**
		 * Initalize the scan handler for a new filesystem scan.
		 */
		void scanStarts(void);

		/**
		 * Scanning the current element of in the scan will
		 * require the given numer of steps.
		 * @param[in] 1st The number of steps to scan the current
		 *     item.
		 */
		void scanPush(unsigned long);

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
		 * @param[in] 1st The number of progress steps.
		 */
		void scanProgress(unsigned long);

	private:
		/**
		 * Trigger an update of the progress bar. This function is
		 * called by the scanProgress if neccessary.
		 *
		 * @param Current state of the progress bar.
		 * @param Total length of the progress bar.
		 *
		 * NOTE: Both values are entirely virtual and have no direct
		 *     connection to the nubmer of times scanProgress has been
		 *     called.
		 */
		virtual void scanUpdate(unsigned int, unsigned int) = 0;

		std::vector<unsigned long>	levels_;
		std::vector<unsigned long>	curstate_;
		unsigned long			limit_;
		unsigned long			total_;
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
class SfsDirectory : public AnRowProvider, private wxDirTraverser
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
		SfsEntry *getEntry(unsigned int) const;

		/**
		 * Inserts a new SfsEntry into the SfsDirectory.
		 *
		 * The method does not check the path! It simply adds the new
		 * entry in correct alphabetical order. If neccessary this
		 * function sends an approriate row change event. See
		 * beginChange() and endChange() for details.
		 *
		 * @param path The path of the file to be inserted
		 * @return The SfsEntry, which was created. If you try to
		 *     insert a path, which was already inserted into the
		 *     model, NULL is returned.
		 */
		SfsEntry *insertEntry(const wxString &);

		/**
		 * Start a series of modifications that do not send
		 * individual row upadate events. Changes can be nested,
		 * i.e. multiple calls to beginChange() must be followed
		 * by multiple calls to endChange().
		 */
		void beginChange(void);

		/**
		 * End a series of modifications. If this ends the last
		 * active change the view is updated by sending a
		 * suitable row change event.
		 */
		void endChange(void);

		/**
		 * Removes the SfsEntry at the specified index.
		 *
		 * The model is not touched, if the index is out of range.
		 *
		 * If neccessary an appropriate row change event is sent.
		 * See beginChange() and endChange() for details.
		 *
		 * @param idx Index of SfsEntry to be removed from model.
		 * @see AnRowProvider::sizeChangeEvent()
		 */
		void removeEntry(unsigned int);

		/**
		 * Removes all entries from the directory.
		 *
		 * If at least one entry was removed, a wxCommandEvent of type
		 * anEVT_ROW_SIZECHANGE is fired.
		 *
		 * @see AnRowProvider::sizeChangeEvent()
		 */
		void removeAllEntries(void);

		/**
		 * Scans the local filesystem.
		 *
		 * The model is refreshed. The filesystem-scan is monitored by
		 * the assinged SfsDirectoryScanHandler-instance.
		 *
		 * At the end, a wxCommandEvent of type anEVT_ROW_SIZECHANGE
		 * is fired.
		 *
		 * @see AnRowProvider::sizeChangeEvent()
		 */
		void scanLocalFilesystem();

		/**
		 * Implementation of AnRowProvider::getRow().
		 *
		 * Returns the SfsEntry at the given index.
		 *
		 * @param idx The index
		 * @return The SfsEntry at the requested index. If the index is
		 *         out of range, NULL is returned.
		 * @see getEntry()
		 */
		AnListClass *getRow(unsigned int idx) const;

		/**
		 * Implementation of AnRowProvider::getSize().
		 *
		 * Returns the number of SfsEntry-instances of the directory.
		 * @return Number of SfsEntries.
		 * @see getNumEntries()
		 */
		int getSize(void) const;

	private:
		wxString path_;
		bool recursive_;
		wxString filter_;
		bool inverseFilter_;
		SfsEntryList entryList_;
		SfsDirectoryScanHandler *scanHandler_;
		bool abortScan_;

		/**
		 * Non-zero while a change is in progress. No events are
		 * sent from insertEntry() and removeEntry() if this value
		 * is set.
		 */
		int changeInProgress_;

		/**
		 * Tests whether a SfsEntry can be inserted into the directory.
		 *
		 * This method is called by insertEntry() to ensure that the
		 * new entry fits to the assigned filter options.
		 */
		bool canInsert(const wxString &) const;

		/**
		 * Removes all entries from the directory.
		 *
		 * @param sendEvent if set to true, a wxCommandEvent of type
		 *                  anEVT_ROW_SIZECHANGE is fired.
		 * @see AnRowProvider::sizeChangeEvent()
		 */
		void removeAllEntries(bool);

		/**
		 * @see wxDirTraverser::InDir()
		 */
		wxDirTraverseResult OnDir(const wxString &);

		/**
		 * @see wxDirTraverser::InFile()
		 */
		wxDirTraverseResult OnFile(const wxString &);

		/**
		 * Returns the number of subdirectories in the given directory.
		 * The method scans the specified directory and counts
		 * the number of subdirectories.
		 *
		 * @param path The directory to be scanned
		 * @return The total number of entries.
		 */
		static unsigned int getDirectoryCount(const wxString &);

	friend class SfsEntry;
};

#endif	/* _SFSDIRECTORY_H_ */
