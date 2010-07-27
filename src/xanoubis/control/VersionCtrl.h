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

#ifndef _VERSIONCTRL_H_
#define _VERSIONCTRL_H_

#include <list>
#include <vector>

#include <anoubis_apnvm.h>

#include "Singleton.h"
#include "AnRowProvider.h"
#include "ApnVersion.h"

class PolicyRuleSet;

/**
 * Singleton class which encapsulates the version management library.
 *
 * Use this class rather than the library directly! Initialization and
 * preparation of the underlaying library is performed during construction
 * of the class-instance. The library-instance is destroyed during destruction.
 * Use isPrepared() is make sure, you are ready to any of the operations.
 *
 * The list of currently available versions is fetched with the
 * fetchVersionList()-method. The result is stored internally and many other
 * operation are relying on this result. Many operations takes an
 * index-parameter, which addresses a previously fetched version.
 */
class VersionCtrl : public Singleton<VersionCtrl>, public AnRowProvider
{
	public:
		~VersionCtrl(void);

		/**
		 * Tests weather the version management library is initialized.
		 *
		 * Normally the initialization is performed during the
		 * construction of the instance. If the initialization failed,
		 * you are not able to use any of the operation of the class.
		 *
		 * @return true if the initialiation succeeded, false
		 *         otherwise.
		 */
		bool isInitialized(void) const;

		/**
		 * Tests weather the version management library is prepared.
		 *
		 * Normally the preparation is performed during the
		 * construction of the instance. If the preparation failed,
		 * this function will try to do the preparation now.
		 *
		 * @return true if the preparation succeeded, false otherwise.
		 */
		bool isPrepared(void);

		/**
		 * Rereads the list of versions from the underlaying version
		 * management library.
		 *
		 * The result of the fetch-operation is stored internally in
		 * the instance and any other operation is working on the
		 * result. Thus, if you think, that the version-list has
		 * changed, you have to invoke the method to reread the list.
		 *
		 * @return true on success, false otherwise.
		 */
		bool fetchVersionList(const wxString &profile);

		/**
		 * Returns the number of versions.
		 *
		 * This method relies on the last fetchVersionList()-
		 * invocation!
		 *
		 * @return Number of versions
		 */
		unsigned int getNumVersions(void) const;

		/**
		 * Returns information about a requested version.
		 *
		 * This method relies on the last fetchVersionList()-
		 * invocation!
		 *
		 * @param idx Index of the requested version (index-counter
		 *            starts at 0)
		 * @return Detail information about the requested version. If
		 *         idx is out of range, NULL is returned.
		 */
		ApnVersion* getVersion(unsigned int) const;

		/**
		 * Deletes the specified version.
		 *
		 * This method relies on the last fetchVersionList()-
		 * invocation!
		 *
		 * @param idx Index of requested version (index-counter starts
		 *            at 0)
		 * @return true on success, false otherwise.
		 */
		bool deleteVersion(unsigned int);

		/**
		 * Fetches a ruleset from the versioning system.
		 *
		 * This method relies on the last fetchVersionList()-
		 * invocation!
		 *
		 * @param idx Index of the requested version (index-counter
		 *            starts at 0)
		 * @param profile Name of profile to be fetched. Pass an empty
		 *                string, if you want to fetch th profile-less
		 *                ruleset.
		 * @return Ruleset of the requested version/profile. On error
		 *         0 is returned.
		 */
		struct apn_ruleset *fetchRuleSet(unsigned int,
		    const wxString &);

		/**
		 * Creates a new version.
		 *
		 * The method takes the ruleset of the specified profile and
		 * appends it to the versioning-system.
		 *
		 * @param policy The policy to to versioned
		 * @param profile The profile to be versioned
		 * @param comment The comment of the version to be created
		 * @param autoStore Value of autoStore-flag
		 * @return true on success, false otherwise.
		 */
		bool createVersion(PolicyRuleSet *, const wxString &,
		    const wxString &, bool);

		/**
		 * Exports a version/profile into a local file.
		 *
		 * The method fetches the ruleset from the specified
		 * version/profile and writes the result into the local
		 * filesystem.
		 *
		 * This method relies on the last fetchVersionList()-
		 * invocation!
		 *
		 * @param idx Index of the requested version (index-counter
		 *            starts at 0)
		 * @param profile The profile to be exported
		 * @param path Path of destination file
		 * @return true on success, false otherwise.
		 */
		bool exportVersion(unsigned int, const wxString &,
		    const wxString &);

		wxString getVersionProfile() {
			return versionProfile_;
		}
	protected:
		VersionCtrl(void);

	private:
		apnvm	*vm_;
		bool	prepared_;
		std::vector<ApnVersion *> versionList_;
		wxString	versionProfile_;

		void prepare(void);
		bool fetchVersionList(const char *user, const char *profile);
		void clearVersionList(void);

	friend class Singleton<VersionCtrl>;

	public:
		/**
		 * Implementation of AnRowProvider::getRow().
		 * @param idx The index of the row.
		 * @return The object associated with the index.
		 */
		AnListClass *getRow(unsigned int idx) const;

		/**
		 * Implementation of AnRowProvider::getSize().
		 * @return The total number of objects in the model.
		 */
		int getSize(void) const;
};

#endif	/* _VERSIONCTRL_H_ */
