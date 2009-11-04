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

#ifndef _POLICYCTRL_H_
#define _POLICYCTRL_H_

#include <list>
#include <wx/string.h>

#include "AnEvents.h"
#include "Singleton.h"
#include "Task.h"

class PolicyRuleSet;
class TaskEvent;

/**
 * Policy controller.
 *
 * This controller is used to manage policies and profiles.
 *
 * Any policies used by the application are under control of this class.
 * Use the method getUserId() resp. getAdminId() to obtain an identifier for
 * the policy you are interested in. Next call getRuleSet() to fetch the policy
 * itself.
 *
 * You can receive a fresh policy from anoubisd by calling
 * receiveOneFromDaemon(prio, uid). A policy can be sent back by calling
 * sendToDaemon().
 *
 * There are several ways to import and export policies. On the one hand you
 * can im/export policies into a profile (importFromProfile(),
 * exportToProfile()). On the other hand you can im/export a policy into an
 * arbitrary file (importFromFile(), exportToFile()).
 */
class PolicyCtrl : public wxEvtHandler, public Singleton<PolicyCtrl>
{
	public:

		/**
		 * Result of sending policy to daemon
		 */
		enum PolicyResult
		{
			RESULT_POL_OK,		/*<! Everything worked fine. */
			RESULT_POL_ERR,		/*<! An Error occured */
			RESULT_POL_ABORT,	/*<! Abort operation */
			RESULT_POL_WRONG_PASS	/*<! RESULT_POL_WRONG_PASS */
		};

		/**
		 * Destructor of PolicyCtrl.
		 * This will clean up the whole mess. It needs to be public,
		 * but you are not allowed to use it (delete).
		 * @param None.
		 * @return Nothing.
		 */
		~PolicyCtrl(void);

		/**
		 * Get object.
		 * This returns the (only) object of this class
		 * (see singleton pattern).
		 * @param None.
		 * @return It self.
		 */
		static PolicyCtrl *getInstance(void);

		/**
		 * Get user rule set id.
		 * Get the id of the current user rule. Use the id to fetch the
		 * policy with getRuleSet().
		 * @param None.
		 * @return The id of the rule set or -1 if nothing found.
		 */
		long getUserId(void) const;

		/**
		 * Get user rule set id.
		 * Get the id of the user rule set mapped to the specified user.
		 * Use the id to fetch the policy with getRuleSet().
		 * @param[in] 1st The uid of the user in question.
		 * @return The id of the rule set or -1 if nothing found.
		 */
		long getUserId(uid_t) const;

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
		 * Get ruleset for the specified profile.
		 *
		 * Use this method if you need to known the policy of a
		 * profile.
		 *
		 * @param name Name of profile.
		 * @return The policy of the profile. If the profile does not
		 *         exist, 0 is returned.
		 * @note The PolicyCtrl is not the owner of the returned
		 *       policy! Thus make sure to destroy the object by
		 *       yourself.
		 */
		PolicyRuleSet *getRuleSet(const wxString &) const;

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
		 * Tests whether a profile with the specified name exists.
		 *
		 * @param name Name of profile to test
		 * @return true if the profile exists, false otherwise.
		 */
		bool haveProfile(const wxString &) const;

		/**
		 * Tests whether the profile is writable for the user.
		 *
		 * This is mandatory for exporting a policy into a profile.
		 *
		 * @param name Name of profile to test
		 * @return true if the profile is writable for the user, false
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
		 * The policy-controller takes over the ownership of the
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
		 * Imports a policy into a profile.
		 *
		 * The policy of the specified profile is replaced with the new
		 * one. Note, that the policy is not loaded!
		 *
		 * The policy-controller does <b>not</b> take over the
		 * ownership of the policy! Thus you need to destroy it by
		 * yourself.
		 *
		 * @param ruleset The ruleset to import
		 * @param name Name of destination profile
		 * @return true is returned, if the policy was imported
		 *         successfully. The import can fail, if the policy
		 *         contains errors.
		 */
		bool importPolicy(PolicyRuleSet *, const wxString &);

		/**
		 * Fetches a single policy from the Daemon.
		 * @param[in] 1st The priority of the policy.
		 * @param[in] 2nd The user ID.
		 * @return True on success.
		 */
		bool receiveOneFromDaemon(long prio, long uid);

		/**
		 * Sends the policy with the specified id to the daemon.
		 *
		 * You can obtain the id by calling getUserId() or
		 * getAdminId().
		 *
		 * The method runs asynchronous and does not block until the
		 * policy was sent. When the method leaves, the procedure is
		 * started.
		 *
		 * When the policy was successfully send, an wxCommandEvent
		 * of type anEVT_SEND_RULESET is fired.
		 *
		 * @param id The id of the policy to be sent
		 * @return a PolicyResult-Code
		 */
		PolicyResult sendToDaemon(long);

		/**
		 * Tests whether the class broadcasts any events.
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
		 * Constructor of PolicyCtrl.
		 */
		PolicyCtrl(void);

	private:
		/**
		 * Different types of profiles.
		 */
		enum ProfileSpec
		{
			NO_PROFILE = 0,		/*!< No valid profile */
			DEFAULT_PROFILE,	/*!< A default profile. Visible
						     for every user, but cannot
						     be overwritten. */
			USER_PROFILE		/*!< A user-specific profile.
						     Visible only for the user
						     and can be overwritten. */
		};

		/**
		 * A list of ruleset-pointers.
		 */
		typedef std::list<PolicyRuleSet *> RuleSetList;

		/**
		 * Flags specifies whether the controller is able to broadcast
		 * any events.
		 *
		 * When the controller is used in a test, broadcasting should
		 * be switched off to prevent errors because the
		 * main-application is not available.
		 */
		bool				eventBroadcastEnabled_;

		/**
		 * Container of policies maintained by the controller.
		 */
		RuleSetList			ruleSetList_;

		/**
		 * Another container of policies.
		 * Contains rulesets, which can be deleted but still locked.
		 */
		RuleSetList			gcRuleSetList_;

		/**
		 * Container for ComPolicyRequestTasks, where the controller
		 * is the owner.
		 */
		TaskList			requestTaskList_;

		/**
		 * Container for ComPolicySendTasks, where the controller is
		 * the owner.
		 */
		TaskList			sendTaskList_;

		/**
		 * Invoked when a policy-requested was answered by anoubisd.
		 * @see anTASKEVT_POLICY_REQUEST
		 */
		void OnPolicyRequest(TaskEvent &);

		/**
		 * Invoked when a polciy was sent to anoubisd.
		 * @see anTASKEVT_POLICY_SEND
		 */
		void OnPolicySend(TaskEvent &);

		/**
		 * Searches for a policy in ruleSetList_, which matches the
		 * specified arguments.
		 * @param isAdmin Set to true, if you can looking for an
		 *                admin-policy.
		 * @param uid Uid of assigned user
		 * @return The id of the policy. If the policy was not found,
		 *         -1 is returned.
		 */
		long seekId(bool, uid_t) const;

		/**
		 * Creates a backup of the currently loaded user-policy.
		 *
		 * The policy is stored under the specified profile in the
		 * version-management.
		 *
		 * @param name Name of profile in version-management.
		 * @return true on success, false otherwise.
		 */
		bool makeBackup(const wxString &);

		/**
		 * Returns the path, where profiles of the specified type are
		 * stored.
		 * @param spec Type of profile.
		 * @return Path, where the profiles are stored. If
		 *         PolicyCtrl::NO_PROFILE is requested, en empty
		 *         string is returned.
		 */
		static wxString getProfilePath(ProfileSpec);

		/**
		 * Returns the path to the file, where the specified profile
		 * is stored.
		 * @param name Name of profile
		 * @param spec Type of profile
		 * @return Path to the file, where the profile is stored.
		 */
		static wxString getProfileFile(const wxString &, ProfileSpec);

		/**
		 * Returns the type of the specified profile.
		 * @param name Name of profile
		 * @return Type of profile. If no such profile exists,
		 *         PolicyCtrl::NO_PROFILE is returned.
		 */
		static ProfileSpec getProfileSpec(const wxString &);

		/**
		 * Scans a directory for files.
		 * @param path Path of directory to scan
		 * @param dest Destination array. Filenames (without
		 *             directory-part) are put into this array.
		 */
		static void scanDirectory(const wxString &, wxArrayString &);

		/**
		 * Removes and destroys rulsets from a list.
		 *
		 * Rulesets are removed and destroyed, if the ruleset is not
		 * locked or the force-flag is set.
		 *
		 * @param list List of rulesets to be cleaned
		 * @param force If set to true, the rulesets are removed and
		 *              destroy independent from the lock-state.
		 *
		 * @see PolicyRuleSet::isLocked()
		 */
		static void cleanRuleSetList(RuleSetList &, bool);

		/**
		 * Answer escalation
		 * A previously received escalation was answered by the user.
		 * An anEVT_ANSWER_ESCALATION event was sent to inform anyone
		 * within the gui.\n
		 * This method will cause the current PolicyRuleSet to create
		 * a concerning new policy matching the answer.
		 *
		 * @param[in] 1st The command event anEVT_ANSWER_ESCALATION.
		 * @return Nothing.
		 */
		void OnAnswerEscalation(wxCommandEvent &);

		/**
		 * Policy Change
		 * A Policy in the daemon just changed.
		 * @param[in] 1st The command event anEVT_POLICY_CHANGE.
		 * @return Nothring.
		 */
		void OnPolicyChange(wxCommandEvent &);

	friend class Singleton<PolicyCtrl>;
	friend int testPolicyCtrl(PolicyCtrl *, int);
};

#endif	/* _POLICYCTRL_H_ */
