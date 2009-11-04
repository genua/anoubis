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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/dir.h>
#include <wx/stdpaths.h>

#include "main.h"
#include "ComPolicyRequestTask.h"
#include "ComPolicySendTask.h"
#include "JobCtrl.h"
#include "PolicyRuleSet.h"
#include "PolicyCtrl.h"
#include "Singleton.cpp"
#include "VersionCtrl.h"

PolicyCtrl::~PolicyCtrl(void)
{
	/* Destroy policies */
	cleanRuleSetList(ruleSetList_, true);
	cleanRuleSetList(gcRuleSetList_, true);

	/* Destroy tasks */
	while (!requestTaskList_.empty()) {
		Task *t = requestTaskList_.front();
		requestTaskList_.pop_front();

		delete t;
	}
	while (!sendTaskList_.empty()) {
		Task *t = sendTaskList_.front();
		sendTaskList_.pop_front();

		delete t;
	}

	JobCtrl::getInstance()->Disconnect(anTASKEVT_POLICY_REQUEST,
	    wxTaskEventHandler(PolicyCtrl::OnPolicyRequest), NULL, this);
	JobCtrl::getInstance()->Disconnect(anTASKEVT_POLICY_SEND,
	    wxTaskEventHandler(PolicyCtrl::OnPolicySend), NULL, this);
	AnEvents::getInstance()->Disconnect(anEVT_ANSWER_ESCALATION,
	    wxCommandEventHandler(PolicyCtrl::OnAnswerEscalation), NULL, this);
	AnEvents::getInstance()->Disconnect(anEVT_POLICY_CHANGE,
	    wxCommandEventHandler(PolicyCtrl::OnPolicyChange), NULL, this);
}

PolicyCtrl *
PolicyCtrl::getInstance(void)
{
	return (Singleton<PolicyCtrl>::instance());
}

long
PolicyCtrl::getUserId(uid_t uid) const
{
	return (seekId(false, uid));
}

long
PolicyCtrl::getUserId(void) const
{
	return (seekId(false, geteuid()));
}

long
PolicyCtrl::getAdminId(uid_t uid) const
{
	return (seekId(true, uid));
}

PolicyRuleSet *
PolicyCtrl::getRuleSet(long id) const
{
	RuleSetList::const_iterator it;

	for (it = ruleSetList_.begin(); it != ruleSetList_.end(); ++it) {
		PolicyRuleSet *rs = (*it);

		if (rs->getRuleSetId() == id)
			return (rs);
	}

	for (it = gcRuleSetList_.begin(); it != gcRuleSetList_.end(); ++it) {
		PolicyRuleSet *rs = (*it);

		if (rs->getRuleSetId() == id)
			return (rs);
	}

	return (0);
}

PolicyRuleSet *
PolicyCtrl::getRuleSet(const wxString &name) const
{
	wxString file;

	/* Try user-profile */
	file = getProfileFile(name, USER_PROFILE);
	if (wxFileExists(file))
		return (new PolicyRuleSet(1, geteuid(), file));

	/* Try default-profile */
	file = getProfileFile(name, DEFAULT_PROFILE);
	if (wxFileExists(file))
		return (new PolicyRuleSet(1, geteuid(), file));

	/* No such profile */
	return (0);
}

wxArrayString
PolicyCtrl::getProfileList(void) const
{
	wxArrayString	result;

	/* Scan for default-profiles */
	scanDirectory(getProfilePath(DEFAULT_PROFILE), result);

	/* Scan for user-profiles */
	scanDirectory(getProfilePath(USER_PROFILE), result);

	return (result);
}

bool
PolicyCtrl::haveProfile(const wxString &name) const
{
	return wxFileExists(getProfileFile(name, USER_PROFILE)) ||
	    wxFileExists(getProfileFile(name, DEFAULT_PROFILE));
}

bool
PolicyCtrl::isProfileWritable(const wxString &name) const
{
	/* Do not allow writes to the "active" profile". */
	if (name == wxString::FromAscii("active"))
		return (false);
	return !wxFileExists(getProfileFile(name, DEFAULT_PROFILE));
}

bool
PolicyCtrl::removeProfile(const wxString &name)
{
	/*
	 * Only try to remove the user-profile,
	 * a default-profile is read-only
	 */
	wxString file = getProfileFile(name, USER_PROFILE);
	if (wxFileExists(file))
		return (wxRemoveFile(file));
	else
		return (false);
}

bool
PolicyCtrl::exportToProfile(const wxString &name)
{
	/* The user policy */
	PolicyRuleSet *rs = getRuleSet(seekId(false, geteuid()));

	if (rs == 0) {
		/* No such policy */
		return (false);
	}

	if (getProfileSpec(name) == DEFAULT_PROFILE) {
		/* Only export to user-profile allowed */
		return (false);
	}

	/*
	 * Store-operation allowed only for user-profiles,
	 * cannot overwrite default-profiles!
	 */
	wxString file = getProfileFile(name, USER_PROFILE);

	/*
	 * Make sure, the directory exists. This is the users home-directory,
	 * so you can try to create the directory.
	 */
	wxFileName fn(file);
	wxFileName::Mkdir(fn.GetPath(),  0700, wxPATH_MKDIR_FULL);

	if (wxFileExists(file)) {
		/*
		 * Remove a previous version of the profile,
		 * it is now re-created.
		 */
		wxRemoveFile(file);
	}

	if (!rs->exportToFile(file))
		return (false);

	/* Make a backup by putting the policy into version-control */
	if (!makeBackup(name))
		return (false);

	return (true);
}

bool
PolicyCtrl::exportToFile(const wxString &file)
{
	/* The user policy */
	PolicyRuleSet *rs = getRuleSet(seekId(false, geteuid()));

	if (rs == 0) {
		/* No such policy */
		return (false);
	}

	if (!rs->exportToFile(file))
		return (false);

	return (true);
}

bool
PolicyCtrl::importFromProfile(const wxString &name)
{
	PolicyRuleSet	*rs = 0;
	wxString	file;

	/* Try user-profile */
	file = getProfileFile(name, USER_PROFILE);
	if (wxFileExists(file))
		rs = new PolicyRuleSet(1, geteuid(), file);

	if (rs == 0) {
		/* Try default-profile */
		file = getProfileFile(name, DEFAULT_PROFILE);
		if (wxFileExists(file))
			rs = new PolicyRuleSet(1, geteuid(), file);
	}

	if (rs != 0) {
		bool success = importPolicy(rs);

		if (!success)
			delete rs;

		return (success);
	} else {
		/* No such profile */
		return (false);
	}
}

bool
PolicyCtrl::importFromFile(const wxString &file)
{
	PolicyRuleSet	*rs;
	bool		success;

	rs = new PolicyRuleSet(1, geteuid(), file);
	success = importPolicy(rs);

	if (!success)
		delete rs;

	return (success);
}

bool
PolicyCtrl::importPolicy(PolicyRuleSet *rs)
{
	/* Try to clean outdated policies */
	cleanRuleSetList(gcRuleSetList_, false);

	if (rs == 0 || rs->hasErrors())
		return (false);

	/* Search and replace user-policy */
	int id = seekId(rs->isAdmin(), rs->getUid());
	if (id != -1) {
		/*
		 * Move previously inserted policy into "garbage" list if
		 * locked for deletion
		 */
		PolicyRuleSet *oldrs = getRuleSet(id);
		ruleSetList_.remove(oldrs);

		if (oldrs->isLocked())
			gcRuleSetList_.push_back(oldrs);
		else
			delete oldrs;
	}

	ruleSetList_.push_back(rs);

	if (eventBroadcastEnabled_) {
		wxCommandEvent event(anEVT_LOAD_RULESET);
		event.SetInt(id);
		event.SetExtraLong(rs->getRuleSetId());
		wxPostEvent(AnEvents::getInstance(), event);
	}

	return (true);
}

bool
PolicyCtrl::importPolicy(PolicyRuleSet *rs, const wxString &name)
{
	if (rs == 0 || rs->hasErrors())
		return (false);

	if (getProfileSpec(name) == DEFAULT_PROFILE) {
		/* Only export to user-profile allowed */
		return (false);
	}

	/*
	 * Store-operation allowed only for user-profiles,
	 * cannot overwrite default-profiles!
	 */
	wxString file = getProfileFile(name, USER_PROFILE);

	/*
	 * Make sure, the directory exists. This is the users home-directory,
	 * so you can try to create the directory.
	 */
	wxFileName fn(file);
	wxFileName::Mkdir(fn.GetPath(),  0700, wxPATH_MKDIR_FULL);

	if (wxFileExists(file)) {
		/*
		 * Remove a previous version of the profile,
		 * it is now re-created.
		 */
		wxRemoveFile(file);
	}

	if (!rs->exportToFile(file))
		return (false);

	return (true);
}

bool
PolicyCtrl::receiveOneFromDaemon(long prio, long uid)
{
	ComPolicyRequestTask	*task = new ComPolicyRequestTask(prio, uid);
	struct iovec		 iov;
	struct apn_ruleset	*apnrs = NULL;

	requestTaskList_.push_back(task);
	JobCtrl::getInstance()->addTask(task);

	if (seekId(prio, uid) >= 0)
		return (true);
	/*
	 * The ruleset is not yet there. Make sure that at least a dummy
	 * ruleset exists.
	 */
	iov.iov_base = (void *)" ";
	iov.iov_len = 1;
	if (apn_parse_iovec("<iov>", &iov, 1, &apnrs, 0) == 0) {
		return importPolicy(new PolicyRuleSet(prio, uid, apnrs));
	} else {
		apn_free_ruleset(apnrs);
		return false;
	}
}

PolicyCtrl::PolicyResult
PolicyCtrl::sendToDaemon(long id)
{
	PolicyRuleSet		*rs = getRuleSet(id);
	ComPolicySendTask	*task;
	KeyCtrl::KeyResult	keyRes;

	if (!rs)
		return (RESULT_POL_ERR);

	task = new ComPolicySendTask;
	if (!task->setPolicy(rs)) {
		delete task;
		return (RESULT_POL_ERR);
	}

	KeyCtrl *keyCtrl = KeyCtrl::getInstance();
	if (keyCtrl->canUseLocalKeys()) {
		/* You need to sign the policy, the private key is required */
		keyRes = keyCtrl->loadPrivateKey();
		if (keyRes == KeyCtrl::RESULT_KEY_ERROR) {
			delete task;
			return (RESULT_POL_ERR);
		} else if (keyRes == KeyCtrl::RESULT_KEY_ABORT) {
			delete task;
			return (RESULT_POL_ABORT);
		} else if (keyRes == KeyCtrl::RESULT_KEY_WRONG_PASS) {
			delete task;
			return (RESULT_POL_WRONG_PASS);
		}

		PrivKey &privKey = keyCtrl->getPrivateKey();
		task->setPrivateKey(privKey.getKey());
	}

	sendTaskList_.push_back(task);
	JobCtrl::getInstance()->addTask(task);

	return (RESULT_POL_OK);
}

bool
PolicyCtrl::isEventBroadcastEnabled(void) const
{
	return (eventBroadcastEnabled_);
}

void
PolicyCtrl::setEventBroadcastEnabled(bool enabled)
{
	eventBroadcastEnabled_ = enabled;
}

void
PolicyCtrl::OnPolicyRequest(TaskEvent &event)
{
	PolicyRuleSet		*rs;
	ComPolicyRequestTask	*task;
	ComTask::ComTaskResult	taskResult;
	wxString		message;
	bool			isAdmin;
	wxString		user;

	task = dynamic_cast<ComPolicyRequestTask*>(event.getTask());

	if (task == 0)
		return;

	if (requestTaskList_.IndexOf(task) == wxNOT_FOUND) {
		/* This is not "my" task. Ignore it. */
		event.Skip();
		return;
	}

	isAdmin = (geteuid() == 0);
	taskResult = task->getComTaskResult();
	user = wxGetApp().getUserNameById(task->getUid());

	if (taskResult == ComTask::RESULT_COM_ERROR) {
		if (isAdmin && (task->getPriority() == 0))
			message.Printf(_("Communication error while"
			" receiving\nadmin-policy of %ls."), user.c_str());
		else if (isAdmin && (task->getPriority() > 0))
			message.Printf(_("Communication error while "
			    "receiving\nuser-policy of %ls."), user.c_str());
		else
			message = _("Error while receiving policy.");
	} else if (taskResult == ComTask::RESULT_REMOTE_ERROR) {
		if (isAdmin && (task->getPriority() == 0))
			message.Printf(_("Got error (%hs) from daemon\n"
			    "after receiving admin-policy of %ls."),
			    strerror(task->getResultDetails()),
			    user.c_str());
		else if (isAdmin && (task->getPriority() > 0))
			message.Printf(_("Got error (%hs) from daemon\n"
			    "after receiving user-policy of %ls."),
			    strerror(task->getResultDetails()),
			    user.c_str());
		else
			message.Printf(_("Got error (%hs) from daemon\n"
			    "after receiving policy."),
			    strerror(task->getResultDetails()));
	} else if (taskResult != ComTask::RESULT_SUCCESS) {
		message.Printf(_("An unexpected error occured.\n"
		    "Error code: %i"), taskResult);
	}

	/* XXX: This is the wrong place to show up a message-box! */
	if (taskResult != ComTask::RESULT_SUCCESS) {
		anMessageBox(message, _("Error while receiving policy"),
		    wxICON_ERROR);

	} else {
		/* XXX Error-path? */
		rs = task->getPolicy();
		importPolicy(rs);

		/*
		 * If this is a new active policy, create a version for it.
		 * XXX CEH: Also do this for admin rule sets?
		 */
		if ((rs != 0) && !rs->isAdmin() && rs->getUid() == geteuid()) {
			makeBackup(wxT("active"));
		}
	}

	requestTaskList_.remove(task);
	delete task;
}

void
PolicyCtrl::OnPolicySend(TaskEvent &event)
{
	ComPolicySendTask	*task;
	ComTask::ComTaskResult	taskResult;
	wxString		message;
	bool			isAdmin;
	wxString		user;

	task = dynamic_cast<ComPolicySendTask*>(event.getTask());
	if (task == 0)
		return;

	if (sendTaskList_.IndexOf(task) == wxNOT_FOUND) {
		/* This is not "my" task. Ignore it. */
		event.Skip();
		return;
	}

	isAdmin = (geteuid() == 0);
	taskResult = task->getComTaskResult();
	user = wxGetApp().getUserNameById(task->getUid());

	if (taskResult == ComTask::RESULT_INIT) {
		message = _("The task was not executed.");
	} else if (taskResult == ComTask::RESULT_OOM) {
		message = _("Out of memory.");
	} else if (taskResult == ComTask::RESULT_COM_ERROR) {
		if (isAdmin && (task->getPriority() == 0))
			message.Printf(
			    _("Communication error while sending admin-policy\n"
			    "of %ls to daemon."), user.c_str());
		else if (isAdmin && (task->getPriority() > 0))
			message.Printf(_("Communication error while sending "
			    "user-policy\nof %ls to daemon."), user.c_str());
		else
			message = _("Error while sending policy to daemon.");
	} else if (taskResult == ComTask::RESULT_REMOTE_ERROR) {
		if (isAdmin && (task->getPriority() == 0))
			message.Printf(_("Got error (%hs) from daemon\n"
			    "after sending admin-policy of %ls."),
			    strerror(task->getResultDetails()),
			    user.c_str());
		else if (isAdmin && (task->getPriority() > 0))
			message.Printf(_("Got error (%hs) from daemon\n"
			    "after sending user-policy of %ls."),
			    strerror(task->getResultDetails()),
			    user.c_str());
		else
			message.Printf(_("Got error (%hs) from daemon\n"
			    "after sending policy."),
			    strerror(task->getResultDetails()));
	} else if (taskResult == ComTask::RESULT_LOCAL_ERROR) {
		if (isAdmin && (task->getPriority() == 0))
			message.Printf(
			    _("Failed to sign the admin-policy of %ls: %hs."),
			    user.c_str(), strerror(task->getResultDetails()));
		else if (isAdmin && (task->getPriority() > 0))
			message.Printf(
			    _("Failed to sign the user-policy of %ls: %hs."),
			    user.c_str(), strerror(task->getResultDetails()));
		else
			message.Printf(
			    _("Failed to sign the policy: %hs."),
			    strerror(task->getResultDetails()));
	} else if (taskResult != ComTask::RESULT_SUCCESS) {
		message.Printf(_("An unexpected error occured.\n"
		    "Error code: %i"), taskResult);
	}

	/* XXX: This is the wrong place to show up a message-box! */
	if (taskResult != ComTask::RESULT_SUCCESS) {
		anMessageBox(message, _("Error while sending policy"),
		    wxICON_ERROR);
	} else {
		/* The policy was successfully activated, now make a backup */
		if (!makeBackup(wxT("active"))) {
			anMessageBox(
			    _("Failed to make a backup of the policy."),
			    _("Error while sending policy"), wxICON_ERROR);
		}
	}

	if (eventBroadcastEnabled_) {
		wxCommandEvent event(anEVT_SEND_RULESET);
		event.SetInt(seekId(isAdmin, task->getUid()));
		wxPostEvent(AnEvents::getInstance(), event);
	}

	sendTaskList_.remove(task);
	delete(task);
}

wxString
PolicyCtrl::getProfilePath(ProfileSpec spec)
{
	switch (spec) {
	case DEFAULT_PROFILE:
		return wxStandardPaths::Get().GetDataDir() +
		    wxT("/profiles");
	case USER_PROFILE:
		return wxStandardPaths::Get().GetUserDataDir() +
		    wxT("/profiles");
	case NO_PROFILE:
		return wxEmptyString;
	}

	return (wxEmptyString); /* Should never be reached */
}

wxString
PolicyCtrl::getProfileFile(const wxString &name, ProfileSpec spec)
{
	if (spec != NO_PROFILE)
		return (getProfilePath(spec) + wxT("/") + name);
	else
		return wxEmptyString;
}

PolicyCtrl::ProfileSpec
PolicyCtrl::getProfileSpec(const wxString &name)
{
	if (wxFileExists(getProfileFile(name, USER_PROFILE)))
		return (USER_PROFILE);
	else if (wxFileExists(getProfileFile(name, DEFAULT_PROFILE)))
		return (DEFAULT_PROFILE);
	else
		return (NO_PROFILE);
}

long
PolicyCtrl::seekId(bool isAdmin, uid_t uid) const
{
	RuleSetList::const_iterator it;

	for (it = ruleSetList_.begin(); it != ruleSetList_.end(); ++it) {
		PolicyRuleSet *rs = (*it);

		if ((rs->isAdmin() == isAdmin) && (rs->getUid() == uid))
			return ((*it)->getRuleSetId());
	}

	return (-1);
}

bool
PolicyCtrl::makeBackup(const wxString &profile)
{
	/* The user policy */
	PolicyRuleSet *rs = getRuleSet(seekId(false, geteuid()));
	if (rs == 0)
		return (false);

	wxString comment = wxString::Format(
	    _("Automatically created version while saving the %ls profile"),
	    profile.c_str());

	VersionCtrl *versionCtrl = VersionCtrl::getInstance();
	return (versionCtrl->createVersion(rs, profile, comment, true));
}

void
PolicyCtrl::scanDirectory(const wxString &path, wxArrayString &dest)
{
	if (!wxDir::Exists(path)) {
		/* No such directory */
		return;
	}

	wxDir		dir(path);
	wxString	filename;
	bool		cont;

	cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
	while (cont) {
		dest.Add(filename);
		cont = dir.GetNext(&filename);
	}
}

void
PolicyCtrl::cleanRuleSetList(RuleSetList &list, bool force)
{
	RuleSetList::iterator it;

	for (it = list.begin(); it != list.end(); ++it) {
		PolicyRuleSet *rs = (*it);

		if (!rs->isLocked() || force) {
			it = list.erase(it);
			delete rs;
		}
	}
}

void
PolicyCtrl::OnAnswerEscalation(wxCommandEvent &event)
{
	EscalationNotify	*escalation;
	PolicyRuleSet		*ruleset = NULL;

	escalation = (EscalationNotify *)event.GetClientObject();
	if (escalation->isAdmin()) {
		ruleset = getRuleSet(getAdminId(geteuid()));
	} else {
		ruleset = getRuleSet(getUserId());
	}

	if ((ruleset == NULL) || (escalation == NULL)) {
		/* This is strange and should never happen. Unable to act. */
		event.Skip();
		return;
	}

	/* Does the answer involve a temporary or permanent policy? */
	if (escalation->getAnswer()->causeTmpRule() ||
	    escalation->getAnswer()->causePermRule()) {
		ruleset->createAnswerPolicy(escalation);
	}
	event.Skip(); /* Ensures others can react to this event, too. */
}

void
PolicyCtrl::OnPolicyChange(wxCommandEvent &event)
{
	long		 prio = event.GetInt();
	long		 uid = event.GetExtraLong();
	PolicyRuleSet	*oldrs = NULL;
	int id;

	id = seekId(prio == 0, uid);
	if (id >= 0) {
		oldrs = getRuleSet(id);
	}
	if (oldrs && oldrs->isModified()) {
		wxCommandEvent	backupEvent(anEVT_BACKUP_POLICY);

		oldrs->lock();
		backupEvent.SetExtraLong(id);
		wxPostEvent(AnEvents::getInstance(), backupEvent);
	}
	receiveOneFromDaemon(prio, uid);
}

PolicyCtrl::PolicyCtrl(void) : Singleton<PolicyCtrl>()
{
	eventBroadcastEnabled_ = true;
	JobCtrl *jobCtrl = JobCtrl::getInstance();

	jobCtrl->Connect(anTASKEVT_POLICY_REQUEST,
	    wxTaskEventHandler(PolicyCtrl::OnPolicyRequest), NULL, this);
	jobCtrl->Connect(anTASKEVT_POLICY_SEND,
	    wxTaskEventHandler(PolicyCtrl::OnPolicySend), NULL, this);
	AnEvents::getInstance()->Connect(anEVT_ANSWER_ESCALATION,
	    wxCommandEventHandler(PolicyCtrl::OnAnswerEscalation), NULL, this);
	AnEvents::getInstance()->Connect(anEVT_POLICY_CHANGE,
	    wxCommandEventHandler(PolicyCtrl::OnPolicyChange), NULL, this);
}
