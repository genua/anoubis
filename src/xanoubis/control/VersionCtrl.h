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

#include "ApnVersion.h"
#include "Singleton.h"

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
class VersionCtrl : public Singleton<VersionCtrl>
{
	public:
		~VersionCtrl(void);

		/**
		 * Returns the singleton instance of this class.
		 *
		 * @return The one and only instance of the class.
		 */
		static VersionCtrl *getInstance(void);

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
		 * construction of the instance. If the preparation failed, you
		 * are not able to use any of the operation of the class.
		 *
		 * @return true if the preparation succeeded, false otherwise.
		 */
		bool isPrepared(void) const;

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
		bool fetchVersionList(void);

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
		 *         idx is out of range, the result is unspecified.
		 */
		const ApnVersion & getVersion(unsigned int) const;

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
		    const wxString &) const;

		/**
		 * Restores a version.
		 *
		 * The method fetches the rulesets of the specified profiles
		 * and writes them into the home-directory of the user. If one
		 * of the specified profiles has no ruleset in the version, it
		 * is skipped. Next the ruleset of the currently active profile
		 * is send to the Anoubis-daemon.
		 *
		 * This method relies on the last fetchVersionList()-
		 * invocation!
		 *
		 * @param idx Index of the requested version (index-counter
		 *            starts at 0)
		 * @param profileList List of profiles to be restored.
		 * @return true on success, false otherwise.
		 */
		bool restoreVersion(unsigned int, const std::list<wxString>&);

		/**
		 * Creates a new version.
		 *
		 * The method takes the ruleset of the specified profile and
		 * appends it to the versioning-system.
		 *
		 * @param profile The profile to be versioned
		 * @param comment The comment of the version to be created
		 * @param autoStore Value of autoStore-flag
		 * @return true on success, false otherwise.
		 */
		bool createVersion(const wxString &, const wxString &, bool);

		/**
		 * Imports a version from a local file.
		 *
		 * The method reads the ruleset from the specified file and
		 * creates a new version from it. The ruleset is put into the
		 * specified profile.
		 *
		 * @param path Path to file which contains the ruleset
		 * @param profile The profile to be versioned
		 * @param comment The comment of the version to be created
		 * @param autoStore Value of autoStore-flag
		 * @return true on success, false otherwise.
		 */
		bool importVersion(const wxString &, const wxString &,
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

	protected:
		VersionCtrl(void);

	private:
		apnvm	*vm_;
		bool	prepared_;
		std::vector<class ApnVersion> versionList_;

		bool fetchVersionList(const char *user);

	friend class Singleton<VersionCtrl>;

};

#endif	/* _VERSIONCTRL_H_ */
