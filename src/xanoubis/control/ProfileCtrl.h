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

#ifndef _PROFILECTRL_H_
#define _PROFILECTRL_H_

#include <list>
#include <wx/string.h>

#include "Singleton.h"
#include "Task.h"

class PolicyRuleSet;
class TaskEvent;

/**
 * Profile controller.
 *
 * This controller is used to manage policies and profiles.
 *
 * Any policies used by the application are under control of this class.
 * Use the method getUserId() resp. getAdminId() to obtain an identifier for
 * the policy you are interested in. Next call getRuleSet() to fetch the policy
 * itself.
 *
 * You can receive a fresh list of policies from anoubisd by calling
 * receiveFromDaemon(). A policy can be sent back by calling sendToDaemon().
 *
 * There are several ways to import and export policies. On the one hand you
 * can im/export policies into a profile (importFromProfile(),
 * exportToProfile()). On the other hand you can im/export a policy into an
 * arbitrary file (importFromFile(), exportToFile()).
 */
class ProfileCtrl : public wxEvtHandler, public Singleton<ProfileCtrl>
{
	public:
		/**
		 * Destructor of ProfileCtrl.
		 * This will clean up the whole mess. It needs to be public,
		 * but you are not allowed to use it (delete).
		 * @param None.
		 * @return Nothing.
		 */
		~ProfileCtrl(void);

		/**
		 * Get object.
		 * This returns the (only) object of this class
		 * (see singleton pattern).
		 * @param None.
		 * @return It self.
		 */
		static ProfileCtrl *getInstance(void);

		/**
		 * Get user rule set id.
		 * Get the id of the current user rule. Use the id to fetch the
		 * policy with getRuleSet().
		 * @param None.
		 * @return The id of the rule set or -1 if nothing found.
		 */
		long getUserId(void) const;

		/**
		 * Get admin rule set id.
		 * Get the id of the current admin rule set mapped to the
		 * specified user. Use the id to fetch the policy with
		 * getRuleSet().
		 * @param[in] 1st The uid of the user in question.
		 * @return The id of the rule set or -1 if nothing found.
		 */
		long getAdminId(uid_t) const;

		/**
		 * Get rule set.
		 * To obtain the id, use getUserId() and getAdminId().
		 *
		 * @param[in] 1st The id of the rule set in question.
		 * @return The requested rule set or NULL if none was found.
		 *
		 * @see getUserId()
		 * @see getAdminId()
		 */
		PolicyRuleSet *getRuleSet(long) const;

		/**
		 * Returns a list of available profiles.
		 *
		 * There are two types of profiles:
		 * - default profiles: located in the data-directory (normally
		 *   <i>/usr/share/xanoubis/profiles</i>. These profiles are
		 *   readonly.
		 * - user profiles: located in the user-data-directory (
		 *   normally <i>$HOME/.xanoubis/profiles</i>). These profiles
		 *   are writable for the user.
		 *
		 * @return List of available profiles
		 */
		wxArrayString getProfileList(void) const;

		/**
		 * Tests weather a profile with the specified name exists.
		 *
		 * @param name Name of profile to test
		 * @return true if the profile exists, false otherwise.
		 */
		bool haveProfile(const wxString &) const;

		/**
		 * Tests weather the profile is writable for the user.
		 *
		 * This is mandatory for exporting a policy into a profile.
		 *
		 * @param name Name of profile to test
		 * @return true if the profile is wriable for the user, false
		 *         otherwise.
		 */
		bool isProfileWritable(const wxString &) const;

		/**
		 * Removes the profile with the specified name.
		 *
		 * The profile is removed, by removing the related file, thus
		 * you cannot remove default-profiles because the files are
		 * write-protected for the user.
		 *
		 * @param name Name of profile to be removed
		 * @return true on success, false otherwise. The operation can
		 *         fail, if the profile does not exist or the user does
		 *         not have the permission to remove the file.
		 */
		bool removeProfile(const wxString &);

		/**
		 * Exports the currenly loaded user-policy to the specified
		 * profile.
		 *
		 * Note, that the export is only permitted for user-profiles.
		 * Trying to export into a default-profile will lead into an
		 * error! If the profile already exist, the previous content
		 * is overwritten.
		 *
		 * @param name Name of profile
		 * @return true on success, false otherwise.
		 */
		bool exportToProfile(const wxString &);

		/**
		 * Exports the currently loaded user-policy into an arbitrary
		 * file.
		 *
		 * If the file already exist, the content is overwritten.
		 *
		 * @param file Name of file. Note, that the user needs
		 *             write-permission to the file.
		 * @return true on success, false otherwise.
		 */
		bool exportToFile(const wxString &);

		/**
		 * Reads a policy from a profile and loads it as the
		 * user-policy.
		 *
		 * The previously loaded user-policy is unloaded before. The
		 * new policy is not sent to the daemon!
		 *
		 * When the policy was successfully imported, an wxCommandEvent
		 * of type anEVT_LOAD_RULESET is fired.
		 *
		 * @param name Name of profile to be loaded
		 * @return true on success, false otherwise.
		 */
		bool importFromProfile(const wxString &);

		/**
		 * Reads a policy from an arbitrary file and loads it as the
		 * user-policy.
		 *
		 * The previously loaded user-policy is unloaded before. The
		 * new policy is not sent to the daemon!
		 *
		 * When the policy was successfully imported, an wxCommandEvent
		 * of type anEVT_LOAD_RULESET is fired.
		 *
		 * @param file Name of file.
		 * @return true on success, false otherwise.
		 */
		bool importFromFile(const wxString &);

		/**
		 * Imports and loads the specified policy.
		 *
		 * The previously loaded policy of the same priority
		 * (user- or admin-policy) and the same user (related uid) is
		 * unloaded before.
		 *
		 * The profile-controller takes over the ownership of the
		 * policy. Thus, the policy is destroyed if necessary.
		 *
		 * When the policy was successfully imported, an wxCommandEvent
		 * of type anEVT_LOAD_RULESET is fired.
		 *
		 * @param ruleset The ruleset to import
		 * @return true is returned, if the policy was imported
		 *         successfully. The import can fail, if the policy
		 *         contains errors.
		 * @see PolicyRuleSet::isAdmin()
		 * @see PolicyRuleSet::getUid()
		 * @see PolicyRuleSet::hasErrors()
		 */
		bool importPolicy(PolicyRuleSet *);

		/**
		 * Fetches a list of policies from anoubisd.
		 *
		 * Which policies are fetched depends the the user. A
		 * non-root-user receives its own user-policy and its readonly
		 * admin-policy. The user root also receive its user-policy and
		 * additionally the admin-policies of all user.
		 *
		 * Previously loaded policies are unloaded before.
		 *
		 * The method runs asynchronous and does not block until all
		 * requested policies has arrived. When the method leaves, the
		 * procedure is started. For each successfully received and
		 * loaded policy, an wxCommandEvent of type anEVT_LOAD_RULESET
		 * is fired.
		 *
		 * @return true is returned, if the procdure was started
		 *         successfully. On error, false is returned. This
		 *         might happen, if no connection to anoubisd is
		 *         established.
		 */
		bool receiveFromDaemon(void);

		/**
		 * Sends the policy with the specified id to the daemon.
		 *
		 * You can obtain the id by calling getUserId() or
		 * getAdminId().
		 *
		 * The method runs asynchronous and does not block until the
		 * policy was sent.When the method leaves, the procedure is
		 * started.
		 *
		 * @param id The id of the policy to be sent
		 * @return true is returned, if the procdure was started
		 *         successfully. On error, false is returned. This
		 *         might happen, if no connection to anoubisd is
		 *         established.
		 */
		bool sendToDaemon(long);

		/**
		 * Tests weather the class broadcasts any events.
		 *
		 * Methods can fire events after completition of an operation.
		 * Any kind of event-broadcasting can be enabled and disabled.
		 * By default, this feature is enabled.
		 *
		 * @param true if event-broadcasting is enabled.
		 */
		bool isEventBroadcastEnabled(void) const;

		/**
		 * Updates the event-broadcasting-capability.
		 *
		 * @param enabled Set to true, if event-broadcasting is
		 *                enabled.
		 */
		void setEventBroadcastEnabled(bool);

	protected:
		/**
		 * Constructor of ProfileCtrl.
		 */
		ProfileCtrl(void);

	private:
		enum ProfileSpec
		{
			NO_PROFILE = 0,
			DEFAULT_PROFILE,
			USER_PROFILE
		};

		bool				eventBroadcastEnabled_;
		std::list<PolicyRuleSet *>	ruleSetList_;
		TaskList			requestTaskList_;
		TaskList			sendTaskList_;

		void OnPolicyRequest(TaskEvent &);
		void OnPolicySend(TaskEvent &);

		long seekId(bool, uid_t) const;
		static wxString getProfilePath(ProfileSpec);
		static wxString getProfileFile(const wxString &, ProfileSpec);
		static ProfileSpec getProfileSpec(const wxString &);
		static void scanDirectory(
		    const wxString &path, wxArrayString &dest);

	friend class Singleton<ProfileCtrl>;
};

#endif	/* _PROFILECTRL_H_ */
