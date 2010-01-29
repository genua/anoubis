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

#ifndef _SBMODEL_H_
#define _SBMODEL_H_

#include <vector>

#include <wx/event.h>

#include "SbEntry.h"

/**
 * The sandbox-model.
 *
 * Maintains instances of SbEntry.
 */
class SbModel : private wxEvtHandler
{
	public:
		/**
		 * D'tor.
		 */
		~SbModel(void);

		/**
		 * Returns the number of sandbox-entries.
		 *
		 * @return Number of sandbox-entries.
		 */
		unsigned int getEntryCount(void) const;

		/**
		 * Returns the sandbox-entry at the given index.
		 *
		 * @param idx The requested index
		 * @return The sandbox-entry at the given index or NULL if the
		 *         index is out of range.
		 */
		SbEntry *getEntry(unsigned int) const;

		/**
		 * Returns the sandbox-entry with the given path.
		 *
		 * @param path The requested path
		 * @return The sandbox-entry with the given path. If no such
		 *         entry exists, NULL is returned.
		 */
		SbEntry *getEntry(const wxString &) const;

		/**
		 * Returns (and maybe creates) the sandbox-entry with the given
		 * path.
		 *
		 * @param path The requested path
		 * @param create If set to true, the entry is created, if it
		 *               does not exist.
		 * @return The requested entry. If no such entry exists and the
		 *         <code>create</code>-flag is set to false, NULL
		 *         is returned.
		 */
		SbEntry *getEntry(const wxString &, bool);

		/**
		 * Removes the sandbox-entry at the given index.
		 *
		 * @param idx The index to be removed
		 * @return true if the removal was succuessful, false if idx
		 *         is out of range.
		 */
		bool removeEntry(unsigned int);

		/**
		 * Removes the sandbox-entry with the given path.
		 * @param path The path of the entry to be removed
		 * @return true if the removal was successful, false if no
		 *         such entry exists.
		 */
		bool removeEntry(const wxString &);

		/**
		 * Tests whether you are able to load sandbox-entries from the
		 * default-configuration.
		 *
		 * If the related configuration-file exists, the method returns
		 * true.
		 *
		 * @return true if you can load the default-sandbox-entries.
		 * @see assignDefaults()
		 */
		bool canAssignDefaults(void) const;

		/**
		 * Assigns sandbox-entries from the default-configuration.
		 *
		 * The administrator has the possibility to provide a list
		 * with default-sandbox-entries. The file is usually located
		 * under
		 * <code>
		 * /usr/share/xanoubis/profiles/wizard/sandbox
		 * </code>.
		 *
		 * If the file exists, the content of the file is parsed and
		 * the sandbox-entries are assigned to the list.
		 *
		 * @param permission Only entries with this permission are
		 *                   assigned to the model
		 */
		void assignDefaults(SbEntry::Permission);

	private:
		/**
		 * List of SbEntry-instances.
		 */
		std::vector<SbEntry *> entries_;

		/**
		 * Compares the given permission-string with the permission.
		 *
		 * The first permission-string can contain the characters "r"
		 * (for read-access), "w" (for write-access) and "x" (for
		 * execute-access). The method returns true, if the string
		 * contains the corresponding letter for the permission.
		 *
		 * @param sb The permission-string
		 * @param s The requested permission
		 * @return true is returned, if the permission-string contains
		 *         the corresponding character.
		 */
		bool comparePermission(
		    const wxString &, SbEntry::Permission) const;

		/**
		 * Fires a wxCommandEvent of type anEVT_ROW_SIZECHANGE.
		 *
		 * Needs to be called if the number of entries has changed.
		 */
		void fireSbModelChanged(void);

		/**
		 * Fires a wxCommandEvent of type anEVT_ROW_UPDATE.
		 *
		 * Needs to be called if a SbEntry has changed.
		 */
		void fireSbEntryChanged(SbEntry *);

	friend class SbEntry;
	friend class SbModelRowProvider;
};

#endif	/* _SBMODEL_H_ */
