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

#include <sys/types.h>
#include <sys/mman.h>

#include <wx/ffile.h>
#include <wx/init.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "ModAnoubis.h"
#include "VersionCtrl.h"
#include "main.h"

VersionCtrl::VersionCtrl(void)
{
	wxString	repository;
	wxString	userName;

	repository = wxGetApp().getDataDir() + wxT("/.xanoubis/apnvmroot");
	userName = wxGetUserId();

	vm_ = apnvm_init(repository.fn_str(), userName.fn_str());

	if (vm_ != 0) {
		apnvm_result vmrc = apnvm_prepare(vm_);
		prepared_ = (vmrc == APNVM_OK);
	}
	else
		prepared_ = false;
}

VersionCtrl::~VersionCtrl(void)
{
	if (vm_ != 0)
		apnvm_destroy(vm_);
}

VersionCtrl *
VersionCtrl::getInstance(void)
{
	return (instance());
}

bool
VersionCtrl::isInitialized(void) const
{
	return (vm_ != 0);
}

bool
VersionCtrl::isPrepared(void) const
{
	return (prepared_);
}

bool
VersionCtrl::fetchVersionList(void)
{
	struct apnvm_user_head	user_head;
	apnvm_result		vmrc;
	bool			result = true;

	if (!isPrepared())
		return (false);

	versionList_.clear();

	LIST_INIT(&user_head);

	vmrc = apnvm_getuser(vm_, &user_head);
	if (vmrc != APNVM_OK)
		return (false);

	while (!LIST_EMPTY(&user_head)) {
		struct apnvm_user *user = LIST_FIRST(&user_head);

		if (!fetchVersionList(user->username))
			result = false;

		LIST_REMOVE(user, entry);
		free(user->username);
		free(user);
	}

	return (result);
}

bool
VersionCtrl::fetchVersionList(const char *user)
{
	struct apnvm_version_head	version_head;
	struct apnvm_version		*version;
	apnvm_result			vmrc;

	TAILQ_INIT(&version_head);

	vmrc = apnvm_list(vm_, user, &version_head);
	if (vmrc != APNVM_OK)
		return (false);

	TAILQ_FOREACH(version, &version_head, entries) {
		ApnVersion v(version, user);
		versionList_.push_back(v);
	}

	apnvm_version_head_free(&version_head);

	return (true);
}

unsigned int
VersionCtrl::getNumVersions(void) const
{
	return (versionList_.size());
}

const ApnVersion &
VersionCtrl::getVersion(unsigned int idx) const
{
	return (versionList_[idx]);
}

bool
VersionCtrl::deleteVersion(unsigned int no)
{
	apnvm_result vmrc;

	if (!isPrepared())
		return (false);

	if (no >= versionList_.size())
		return (false);

	const ApnVersion &version = getVersion(no);

	vmrc = apnvm_remove(vm_, version.getUsername().fn_str(),
	    version.getVersionNo());

	return (vmrc == APNVM_OK);
}

struct apn_ruleset *
VersionCtrl::fetchRuleSet(unsigned int no, const wxString &profile) const
{
	apnvm_result		vmrc;
	struct apn_ruleset	*rs;

	if (!isPrepared())
		return (0);

	if (no >= versionList_.size())
		return (0);

	const ApnVersion &version = getVersion(no);

	vmrc = apnvm_fetch(vm_, version.getUsername().fn_str(),
	    version.getVersionNo(),
	    (!profile.IsEmpty() ? profile.fn_str() : (char*)0),
	    &rs);

	return (vmrc == APNVM_OK) ? rs : 0;
}

bool
VersionCtrl::restoreVersion(unsigned int no,
    const std::list<wxString>& profileList)
{
	apnvm_result vmrc;

	if (!isPrepared())
		return (false);

	if (no >= versionList_.size())
		return (false);

	const ApnVersion &version = getVersion(no);

	for (std::list<wxString>::const_iterator it = profileList.begin();
	   it != profileList.end(); ++it) {

		struct apn_ruleset	*rs;
		const wxString		profile = (*it);
		wxString		path;

		/* Build path of file to be written */
		if (!profile.IsEmpty())
			path = wxGetApp().getRulesetPath(profile, false);
		else
			path = wxGetApp().getRulesetPath(wxT("none"), false);

		/* Remove file, is rewritten later */
		if (wxFileExists(path))
			wxRemoveFile(path);

		/* Fetch profile's ruleset */
		if (!profile.IsEmpty())
			vmrc = apnvm_fetch(vm_, version.getUsername().fn_str(),
			    version.getVersionNo(), profile.fn_str(), &rs);
		else
			vmrc = apnvm_fetch(vm_, version.getUsername().fn_str(),
			    version.getVersionNo(), 0, &rs);

		if ((vmrc == APNVM_OK) && (rs != 0)) {
			/* Dump ruleset to file */
			wxFFile f(path, wxT("w"));

			if (f.IsOpened()) {
				apn_print_ruleset(rs, 0, f.fp());
				fchmod(fileno(f.fp()), S_IRUSR);
				f.Close();
			}
		}

		if (ProfileCtrl::getInstance()->getProfileName() == profile) {
			/* Current profile, reactivate */
			if (rs == 0) {
				/*
				 * Version was restored, but the current
				 * profile has no ruleset. Take default-ruleset
				 */
				wxString f =
				    wxGetApp().getRulesetPath(profile, true);

				if (apn_parse(f.fn_str(), &rs, 0) != 0)
					rs = 0;
			}

			if (rs != 0)
				wxGetApp().importPolicyRuleSet(1, geteuid(),
				    rs);
		}
	}

	/* Send result back to daemon */
	wxGetApp().profileFromDiskToDaemon(
	    ProfileCtrl::getInstance()->getProfileName());

	return (true);
}

bool
VersionCtrl::createVersion(const wxString &profile, const wxString &comment,
    bool autoStore)
{
	struct apn_ruleset	*rs;
	struct apnvm_md		md;
	wxString		path;
	apnvm_result		vmrc;

	if (!isPrepared())
		return (false);

	/* Dump profile to disk */
	if (!wxGetApp().profileFromDaemonToDisk(profile))
		return (false);

	/* File, where ruleset is stored */
	path = wxGetApp().getRulesetPath(profile, true);

	/* Path content */
	if ((apn_parse(path.fn_str(), &rs, 0) != 0) || (rs == 0))
		return (false);

	md.comment = strdup(comment.fn_str());
	md.auto_store = autoStore;

	vmrc = apnvm_insert(vm_, wxGetUserId().fn_str(), profile.fn_str(),
	    rs, &md);

	free((void*)md.comment);
	return (vmrc == APNVM_OK);
}

bool
VersionCtrl::importVersion(const wxString &path, const wxString &profile,
    const wxString &comment, bool autoStore)
{
	struct apn_ruleset	*rs;
	struct apnvm_md		md;
	apnvm_result		vmrc;
	int			fd;
	struct iovec		iov;

	if ((fd = open(path.fn_str(), O_RDONLY)) == -1)
		return (false);

	/* Detect filesize by jumping to the end */
	off_t offs = lseek(fd, 0, SEEK_END);
	if (offs == (off_t)-1) {
		close(fd);
		return (false);
	}
	iov.iov_len = offs;

	/* Jump back to the start */
	if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
		close(fd);
		return (false);
	}

	iov.iov_base = mmap(NULL, iov.iov_len, PROT_READ, MAP_PRIVATE, fd, 0);
	if (iov.iov_base == MAP_FAILED) {
		close(fd);
		return (false);
	}

	/* Parse the ruleset */
	if ((apn_parse_iovec("<iov>", &iov, 1, &rs, 0) != 0) || (rs == 0)) {
		munmap(iov.iov_base, iov.iov_len);
		close(fd);
		return (false);
	}

	munmap(iov.iov_base, iov.iov_len);
	close(fd);

	md.comment = strdup(comment.fn_str());
	md.auto_store = autoStore;

	/* Insert into versioning-system */
	vmrc = apnvm_insert(vm_, wxGetUserId().fn_str(), profile.fn_str(),
	    rs, &md);

	free((void*)md.comment);
	return (vmrc == APNVM_OK);
}

bool
VersionCtrl::exportVersion(unsigned int no, const wxString &profile,
    const wxString &path)
{
	struct apn_ruleset	*rs;
	FILE			*fh;

	rs = fetchRuleSet(no, profile);
	if (rs == 0)
		return (false);

	if ((fh = fopen(path.fn_str(), "w")) == 0)
		return (false);

	apn_print_ruleset(rs, 0, fh);

	fflush(fh);
	fclose(fh);

	return (true);
}
