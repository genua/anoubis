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
#include <wx/msgdlg.h> /* XXX Used by bad AnoubisGuiApp::OnPolicySend */
#include <wx/stdpaths.h>

#include "main.h"
#include "AnUtils.h"
#include "ComPolicyRequestTask.h"
#include "ComPolicySendTask.h"
#include "JobCtrl.h"
#include "PolicyRuleSet.h"
#include "ProfileCtrl.h"
#include "VersionCtrl.h"

ProfileCtrl::~ProfileCtrl(void)
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

	AnEvents::getInstance()->Disconnect(anEVT_ANSWER_ESCALATION,
	    wxCommandEventHandler(ProfileCtrl::OnAnswerEscalation),
	    NULL, this);
}

ProfileCtrl *
ProfileCtrl::getInstance(void)
{
	return (Singleton<ProfileCtrl>::instance());
}

long
ProfileCtrl::getUserId(void) const
{
	return (seekId(false, geteuid()));
}

long
ProfileCtrl::getAdminId(uid_t uid) const
{
	return (seekId(true, uid));
}

PolicyRuleSet *
ProfileCtrl::getRuleSet(long id) const
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
ProfileCtrl::getRuleSet(const wxString &name) const
{
	wxString file;

	/* Try user-profile */
	file = getProfileFile(name, USER_PROFILE);
	if (wxFileExists(file))
		return (new PolicyRuleSet(1, geteuid(), file, true));

	/* Try default-profile */
	file = getProfileFile(name, DEFAULT_PROFILE);
	if (wxFileExists(file))
		return (new PolicyRuleSet(1, geteuid(), file, false));

	/* No such profile */
	return (0);
}

wxArrayString
ProfileCtrl::getProfileList(void) const
{
	wxArrayString	result;

	/* Scan for default-profiles */
	scanDirectory(getProfilePath(DEFAULT_PROFILE), result);

	/* Scan for user-profiles */
	scanDirectory(getProfilePath(USER_PROFILE), result);

	return (result);
}

bool
ProfileCtrl::haveProfile(const wxString &name) const
{
	return wxFileExists(getProfileFile(name, USER_PROFILE)) ||
	    wxFileExists(getProfileFile(name, DEFAULT_PROFILE));
}

bool
ProfileCtrl::isProfileWritable(const wxString &name) const
{
	/* Do not allow writes to the "active" profile". */
	if (name == wxString::FromAscii("active"))
		return (false);
	return !wxFileExists(getProfileFile(name, DEFAULT_PROFILE));
}

bool
ProfileCtrl::removeProfile(const wxString &name)
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
ProfileCtrl::exportToProfile(const wxString &name)
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

	/*
	 * XXX It's hard to determine weather export was successful
	 * because PolicyRuleSet::exportToFile() returns void.
	 */
	rs->exportToFile(file);

	/* Make a backup by putting the policy into version-control */
	if (!makeBackup(name))
		return (false);

	return (true);
}

bool
ProfileCtrl::exportToFile(const wxString &file)
{
	/* The user policy */
	PolicyRuleSet *rs = getRuleSet(seekId(false, geteuid()));

	if (rs == 0) {
		/* No such policy */
		return (false);
	}

	/*
	 * XXX It's hard to determine whether export was successful
	 * because PolicyRuleSet::exportToFile() returns void.
	 */
	rs->exportToFile(file);

	return (true);
}

bool
ProfileCtrl::importFromProfile(const wxString &name)
{
	PolicyRuleSet	*rs = 0;
	wxString	file;

	/* Try user-profile */
	file = getProfileFile(name, USER_PROFILE);
	if (wxFileExists(file))
		rs = new PolicyRuleSet(1, geteuid(), file, true);

	if (rs == 0) {
		/* Try default-profile */
		file = getProfileFile(name, DEFAULT_PROFILE);
		if (wxFileExists(file))
			rs = new PolicyRuleSet(1, geteuid(), file, false);
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
ProfileCtrl::importFromFile(const wxString &file)
{
	PolicyRuleSet	*rs;
	bool		success;

	rs = new PolicyRuleSet(1, geteuid(), file, false);
	success = importPolicy(rs);

	if (!success)
		delete rs;

	return (success);
}

bool
ProfileCtrl::importPolicy(PolicyRuleSet *rs)
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
		event.SetClientData((void*)rs);
		wxPostEvent(AnEvents::getInstance(), event);
	}

	return (true);
}

bool
ProfileCtrl::importPolicy(PolicyRuleSet *rs, const wxString &name)
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

	/*
	 * XXX It's hard to determine weather export was successful
	 * because PolicyRuleSet::exportToFile() returns void.
	 */
	rs->exportToFile(file);

	return (true);
}

bool
ProfileCtrl::receiveFromDaemon(void)
{
	int idx;
	unsigned long uid;
	TaskList::const_iterator it;
	wxArrayString userList = wxGetApp().getListOfUsersId();

	if (requestTaskList_.empty()) {
		requestTaskList_.push_back(new ComPolicyRequestTask(1,
		    geteuid()));
		for (idx=userList.GetCount() - 1; idx >= 0; idx--) {
			userList.Item(idx).ToULong(&uid);
			requestTaskList_.push_back(
			    new ComPolicyRequestTask(0, (uid_t)uid));
		}
	}

	for (it=requestTaskList_.begin(); it != requestTaskList_.end(); it++) {
		JobCtrl::getInstance()->addTask(*it);
	}

	return (true);
}

bool
ProfileCtrl::sendToDaemon(long id)
{
	PolicyRuleSet *rs = getRuleSet(id);
	ComPolicySendTask *task = new ComPolicySendTask(rs);

	sendTaskList_.push_back(task);
	JobCtrl::getInstance()->addTask(task);

	return (true);
}

bool
ProfileCtrl::isEventBroadcastEnabled(void) const
{
	return (eventBroadcastEnabled_);
}

void
ProfileCtrl::setEventBroadcastEnabled(bool enabled)
{
	eventBroadcastEnabled_ = enabled;
}

void
ProfileCtrl::OnPolicyRequest(TaskEvent &event)
{
	PolicyRuleSet		*rs;
	ComPolicyRequestTask	*task =
	    dynamic_cast<ComPolicyRequestTask*>(event.getTask());

	if (task == 0)
		return;

	event.Skip();

	if (requestTaskList_.IndexOf(task) == wxNOT_FOUND) {
		/* This is not "my" task. Ignore it. */
		return;
	}
	/* XXX Error-path? */
	rs = task->getPolicy();
	importPolicy(rs);
	/*
	 * If this is a new active policy, create a version for it.
	 * XXX CEH: Also do this for admin rule sets?
	 */
	if (!rs->isAdmin() && rs->getUid() == geteuid()) {
		makeBackup(wxT("active"));
	}
}

void
ProfileCtrl::OnPolicySend(TaskEvent &event)
{
	ComPolicySendTask	*task;
	ComTask::ComTaskResult	taskResult;
	wxString		message;
	bool			isAdmin;
	wxString		user;

	task = dynamic_cast<ComPolicySendTask*>(event.getTask());
	if (task == 0)
		return;

	event.Skip();

	if (sendTaskList_.IndexOf(task) == wxNOT_FOUND) {
		/* This is not "my" task. Ignore it. */
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
			    _("Communication error while sending admin-policy\n\
of %s to daemon."), user.c_str());
		else if (isAdmin && (task->getPriority() > 0))
			message.Printf(_("Communication error while sending\
user-policy\nof %s to daemon."), user.c_str());
		else
			message = _("Error while sending policy to daemon.");
	} else if (taskResult == ComTask::RESULT_REMOTE_ERROR) {
		if (isAdmin && (task->getPriority() == 0))
			message.Printf(_("Got error (%s) from daemon\n\
after sent admin-policy of %s."),
			    wxStrError(task->getResultDetails()).c_str(),
			    user.c_str());
		else if (isAdmin && (task->getPriority() > 0))
			message.Printf(_("Got error (%s) from daemon\n\
after sent user-policy of %s."),
			    wxStrError(task->getResultDetails()).c_str(),
			    user.c_str());
		else
			message.Printf(_("Got error (%s) from daemon\n\
after sent policy."), wxStrError(task->getResultDetails()).c_str());
	} else if (taskResult != ComTask::RESULT_SUCCESS) {
		message.Printf(_("An unexpected error occured.\n\
Error code: %i"), taskResult);
	}

	/* XXX: This is the wrong place to show up a message-box! */
	if (taskResult != ComTask::RESULT_SUCCESS) {
		wxMessageBox(message, _("Error while sending policy"),
		    wxICON_ERROR);
	} else {
		/* The policy was successfully activated, now make a backup */
		if (!makeBackup(wxT("active"))) {
			wxMessageBox(
			    _("Failed to make a backup of the policy."),
			    _("Error while sending policy"), wxICON_ERROR);
		}
	}

	sendTaskList_.remove(task);
}

wxString
ProfileCtrl::getProfilePath(ProfileSpec spec)
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
ProfileCtrl::getProfileFile(const wxString &name, ProfileSpec spec)
{
	if (spec != NO_PROFILE)
		return (getProfilePath(spec) + wxT("/") + name);
	else
		return wxEmptyString;
}

ProfileCtrl::ProfileSpec
ProfileCtrl::getProfileSpec(const wxString &name)
{
	if (wxFileExists(getProfileFile(name, USER_PROFILE)))
		return (USER_PROFILE);
	else if (wxFileExists(getProfileFile(name, DEFAULT_PROFILE)))
		return (DEFAULT_PROFILE);
	else
		return (NO_PROFILE);
}

long
ProfileCtrl::seekId(bool isAdmin, uid_t uid) const
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
ProfileCtrl::makeBackup(const wxString &profile)
{
	/* The user policy */
	PolicyRuleSet *rs = getRuleSet(seekId(false, geteuid()));
	if (rs == 0)
		return (false);

	wxString comment = wxString::Format(
	    _("Automatically created version while saving the %s profile"),
	    profile.c_str());

	VersionCtrl *versionCtrl = VersionCtrl::getInstance();
	return (versionCtrl->createVersion(rs, profile, comment, true));
}

void
ProfileCtrl::scanDirectory(const wxString &path, wxArrayString &dest)
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
ProfileCtrl::cleanRuleSetList(RuleSetList &list, bool force)
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
ProfileCtrl::OnAnswerEscalation(wxCommandEvent &event)
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

ProfileCtrl::ProfileCtrl(void) : Singleton<ProfileCtrl>()
{
	eventBroadcastEnabled_ = true;

	JobCtrl *jobCtrl = JobCtrl::getInstance();

	jobCtrl->Connect(anTASKEVT_POLICY_REQUEST,
	    wxTaskEventHandler(ProfileCtrl::OnPolicyRequest), NULL, this);
	jobCtrl->Connect(anTASKEVT_POLICY_SEND,
	    wxTaskEventHandler(ProfileCtrl::OnPolicySend), NULL, this);

	AnEvents::getInstance()->Connect(anEVT_ANSWER_ESCALATION,
	    wxCommandEventHandler(ProfileCtrl::OnAnswerEscalation),
	    NULL, this);
}
