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

#include "ApnVersion.h"
#include "MainUtils.h"
#include "ModAnoubis.h"
#include "PolicyRuleSet.h"
#include "ProcCtrl.h"
#include "Singleton.cpp"
#include "VersionCtrl.h"

VersionCtrl::VersionCtrl(void)
{
	wxString	repository;
	wxString	userName;

	repository = MainUtils::instance()->getDataDir() +
	    wxT("/.xanoubis/apnvmroot");
	userName = wxGetUserId();

	versionProfile_ = wxEmptyString;
	prepared_ = false;
	vm_ = apnvm_init(repository.fn_str(), userName.fn_str(),
	    procctrl_pidcallback);

	if (vm_ != 0) {
		prepare();
	}
}

void
VersionCtrl::prepare(void)
{
	if (vm_ && !prepared_) {
		apnvm_result vmrc = apnvm_prepare(vm_);
		prepared_ = (vmrc == APNVM_OK);
	}
}

VersionCtrl::~VersionCtrl(void)
{
	clearVersionList();

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
VersionCtrl::isPrepared(void)
{
	if (!prepared_)
		prepare();
	return (prepared_);
}

bool
VersionCtrl::fetchVersionList(const wxString &profile)
{
	struct apnvm_user_head	user_head;
	apnvm_result		vmrc;
	bool			result = true;

	if (!isPrepared())
		return (false);

	clearVersionList();
	versionProfile_ = profile;

	LIST_INIT(&user_head);

	vmrc = apnvm_getuser(vm_, &user_head);
	if (vmrc != APNVM_OK)
		return (false);

	while (!LIST_EMPTY(&user_head)) {
		struct apnvm_user *user = LIST_FIRST(&user_head);

		if (!fetchVersionList(user->username, profile.fn_str()))
			result = false;

		LIST_REMOVE(user, entry);
		free(user->username);
		free(user);
	}
	if (!result)
		versionProfile_ = wxEmptyString;
	return (result);
}

bool
VersionCtrl::fetchVersionList(const char *user, const char *profile)
{
	struct apnvm_version_head	version_head;
	struct apnvm_version		*version;
	apnvm_result			vmrc;

	TAILQ_INIT(&version_head);

	vmrc = apnvm_list(vm_, user, profile, &version_head);
	if (vmrc != APNVM_OK)
		return (false);

	TAILQ_FOREACH(version, &version_head, entries) {
		ApnVersion *v = new ApnVersion(version, user);
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

ApnVersion *
VersionCtrl::getVersion(unsigned int idx) const
{
	if (idx < versionList_.size())
		return (versionList_[idx]);
	else
		return (0);
}

bool
VersionCtrl::deleteVersion(unsigned int no)
{
	apnvm_result vmrc;

	if (!isPrepared())
		return (false);

	if (no >= versionList_.size())
		return (false);

	ApnVersion *version = getVersion(no);

	if (versionProfile_.IsEmpty())
		return (false);
	vmrc = apnvm_remove(vm_, version->getUsername().fn_str(),
	    versionProfile_.fn_str(), version->getVersionNo());

	return (vmrc == APNVM_OK);
}

struct apn_ruleset *
VersionCtrl::fetchRuleSet(unsigned int no, const wxString &profile)
{
	apnvm_result		vmrc;
	struct apn_ruleset	*rs;

	if (!isPrepared())
		return (0);

	if (no >= versionList_.size())
		return (0);

	if (profile.IsEmpty())
		return (0);

	ApnVersion *version = getVersion(no);

	vmrc = apnvm_fetch(vm_, version->getUsername().fn_str(),
	    version->getVersionNo(), profile.fn_str(), &rs);

	return (vmrc == APNVM_OK) ? rs : 0;
}

bool
VersionCtrl::createVersion(PolicyRuleSet *policy, const wxString &profile,
    const wxString &comment, bool autoStore)
{
	struct apn_ruleset	*rs;
	struct apnvm_md		md;
	apnvm_result		vmrc;

	if (!isPrepared())
		return (false);

	if (policy == 0)
		return (false);

	rs = policy->getApnRuleSet();
	if (rs == 0)
		return (false);

	md.comment = strdup(comment.fn_str());
	md.auto_store = autoStore;

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

	int result = apn_print_ruleset(rs, 0, fh);

	if (result == 0)
		fflush(fh);

	fclose(fh);

	return (result == 0);
}

void
VersionCtrl::clearVersionList(void)
{
	while (!versionList_.empty()) {
		ApnVersion *version = versionList_.back();
		versionList_.pop_back();

		delete version;
	}
}

int
VersionCtrl::getSize(void) const
{
	return getNumVersions();
}

AnListClass *
VersionCtrl::getRow(unsigned int idx) const
{
	return getVersion(idx);
}
