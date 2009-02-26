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

#include "PolicyRuleSet.h"
#include "PolicyUtils.h"

#include <wx/utils.h>
#include <wx/ffile.h>
#include <wx/string.h>

#include "main.h"
#include "ProfileCtrl.h"
#include "PolicyVisitor.h"
#include "RuleSetSearchPolicyVisitor.h"
#include "RuleSetClearModifiedVisitor.h"
#include "RuleEditorChecksumVisitor.h"

IMPLEMENT_CLASS(PolicyRuleSet, Subject);

PolicyRuleSet::PolicyRuleSet(int priority, uid_t uid,
    struct apn_ruleset *ruleSet)
{
	refCnt_     = 0;
	ruleSetId_  = wxNewId();
	uid_	    = uid;
	priority_   = priority;
	ruleSet_    = NULL; /* Set by crate() */
	origin_     = wxT("native apn ruleset");
	hasErrors_  = false;
	isModified_ = false;
	isDaemonRuleSet_ = false;

	create(ruleSet);
}

PolicyRuleSet::PolicyRuleSet(int priority, uid_t uid, const wxString &fileName,
    bool checkPerm)
{
	refCnt_     = 0;
	ruleSetId_  = wxNewId();
	uid_	    = uid;
	priority_   = priority;
	ruleSet_    = NULL; /* Set by crate() */
	origin_	    = fileName;
	hasErrors_  = false;
	isModified_ = false;
	isDaemonRuleSet_ = false;

	create(fileName, checkPerm);
}

PolicyRuleSet::~PolicyRuleSet(void)
{
	clean();
	apn_free_ruleset(ruleSet_);
}

void
PolicyRuleSet::lock(void)
{
	refCnt_++;
}

void
PolicyRuleSet::unlock(void)
{
	if (refCnt_ > 0)
		refCnt_--;
}

bool
PolicyRuleSet::isLocked(void) const
{
	return (refCnt_ > 0);
}

bool
PolicyRuleSet::isEmpty(void) const
{
	bool isEmpty;

	if (alfList_.IsEmpty() &&
	    sfsList_.IsEmpty() &&
	    ctxList_.IsEmpty() &&
	    sbList_.IsEmpty()     ) {
		isEmpty = true;
	} else {
		isEmpty = false;
	}

	return (isEmpty);
}

bool
PolicyRuleSet::isAdmin(void) const
{
	bool isAdmin;

	if (priority_ > 0) {
		isAdmin = false;
	} else {
		isAdmin = true;
	}

	return (isAdmin);
}

void
PolicyRuleSet::addRuleInformation(EscalationNotify *escalation)
{
	RuleSetSearchPolicyVisitor	*seeker;
	FilterPolicy			*filter = NULL;
	AppPolicy			*parent = NULL, *other = NULL;
	unsigned char			 csum[APN_HASH_SHA256_LEN];
	wxString			 hashValue;
	bool				 canEdit = true;
	const char *			 path;

	if (isAdmin() && geteuid() != 0)
		canEdit = false;
	seeker = new RuleSetSearchPolicyVisitor(escalation->getRuleId());
	this->accept(*seeker);
	if (seeker->hasMatchingPolicy()) {
		filter = wxDynamicCast(seeker->getMatchingPolicy(),
		    FilterPolicy);
		if (filter) {
			parent = wxDynamicCast(filter->getParentPolicy(),
			    AppPolicy);
		}
	}
	delete seeker;
	if (!parent) {
		canEdit = false;
	} else if (parent->isAnyBlock() && parent->getTypeID() != APN_SFS) {
		if (escalation->getCtxChecksum(csum)) {
			PolicyUtils::csumToString(csum,
			    MAX_APN_HASH_LEN, hashValue);
			seeker = new RuleSetSearchPolicyVisitor(hashValue);
			this->accept(*seeker);
			if (seeker->hasMatchingPolicy()) {
				other = wxDynamicCast(
				    seeker->getMatchingPolicy(), AppPolicy);
			}
			delete seeker;
		} else {
			canEdit = false;
		}
		if (other != NULL && other->getTypeID() == parent->getTypeID())
			canEdit = false;
	}
	escalation->setAllowEdit(canEdit);
	path = NULL;
	if (filter) {
		escalation->setRulePath(filter->getRulePrefix());
	}
}

bool
PolicyRuleSet::hasErrors(void) const
{
	return (hasErrors_);
}

long
PolicyRuleSet::getRuleSetId(void) const
{
	return (ruleSetId_);
}

struct apn_ruleset *
PolicyRuleSet::getApnRuleSet(void) const
{
	return (ruleSet_);
}

uid_t
PolicyRuleSet::getUid(void) const
{
	return (uid_);
}

int
PolicyRuleSet::getPriority(void) const
{
	return (priority_);
}

void
PolicyRuleSet::setOrigin(wxString origin)
{
	startChange();
	origin_ = origin;
	finishChange();
}

wxString
PolicyRuleSet::getOrigin(void) const
{
	return (origin_);
}

void
PolicyRuleSet::setDaemonRuleSet(void)
{
	isDaemonRuleSet_ = true;
}

bool
PolicyRuleSet::isDaemonRuleSet(void) const
{
	return (isDaemonRuleSet_);
}

void
PolicyRuleSet::accept(PolicyVisitor& visitor)
{
	PolicyList::iterator	i;

	for (i=alfList_.begin(); i != alfList_.end(); i++) {
		(*i)->accept(visitor);
	}
	for (i=sfsList_.begin(); i != sfsList_.end(); i++) {
		(*i)->accept(visitor);
	}
	for (i=ctxList_.begin(); i != ctxList_.end(); i++) {
		(*i)->accept(visitor);
	}
	for (i=sbList_.begin(); i != sbList_.end(); i++) {
		(*i)->accept(visitor);
	}
}

wxString
PolicyRuleSet::toString(void) const
{
	int	 result;
	wxString content;
	wxString tmpFileName;
	wxFFile	 tmpFile;

	result = 1;
	content = wxEmptyString;
	tmpFileName = wxFileName::CreateTempFileName(wxEmptyString);

	if (!tmpFile.Open(tmpFileName, wxT("w"))) {
		/* Couldn't open / fill tmp file. */
		return (wxEmptyString);
	}

	/* Write the ruleset to tmp file. */
	result = apn_print_ruleset(ruleSet_, 0, tmpFile.fp());
	tmpFile.Flush();
	tmpFile.Close();

	if (result != 0) {
		/* Something went wrong during file filling! */
		wxRemoveFile(tmpFileName);
		return (wxEmptyString);
	}

	if (tmpFile.Open(tmpFileName, wxT("r"))) {
		if (!tmpFile.ReadAll(&content)) {
			/* Error during file read - clear read stuff. */
			content = wxEmptyString;
		}
		tmpFile.Close();
	}

	wxRemoveFile(tmpFileName);

	return (content);
}

bool
PolicyRuleSet::exportToFile(const wxString &fileName) const
{
	int	result;
	wxFFile exportFile;

	if (!exportFile.Open(fileName, wxT("w"))) {
		/* Couldn't open file for writing. */
		return (false);
	}

	result = apn_print_ruleset(ruleSet_, 0, exportFile.fp());

	fchmod(fileno(exportFile.fp()), S_IRUSR);
	exportFile.Flush();
	exportFile.Close();

	if (result != 0) {
		/* Something went wrong - remove broken file. */
		wxRemoveFile(fileName);
		return (false);
	}

	return (true);
}

void
PolicyRuleSet::setModified(void)
{
	startChange();
	isModified_ = true;
	finishChange();
}

void
PolicyRuleSet::clearModified(void)
{
	RuleSetClearModifiedVisitor resetVisitor;

	this->accept(resetVisitor);
	isModified_ = false;
}

bool
PolicyRuleSet::isModified(void) const
{
	return (isModified_);
}

size_t
PolicyRuleSet::getAppPolicyCount(void) const
{
	size_t count;

	count  = alfList_.GetCount();
	count += sfsList_.GetCount();
	count += ctxList_.GetCount();
	count += sbList_.GetCount();

	return (count);
}

long
PolicyRuleSet::createPolicy(unsigned int type, unsigned int id,
    AppPolicy *parent)
{
	bool success;
	long index;

	success = false;
	index   = -1;

	switch (type) {
	case APN_ALF:
		success = AlfAppPolicy::createApnInserted(this, id);
		index = 0;
		break;
	case APN_CTX:
		success = ContextAppPolicy::createApnInserted(this, id);
		index  = alfList_.GetCount();
		index += sfsList_.GetCount();
		break;
	case APN_SB:
		success = SbAppPolicy::createApnInserted(this, id);
		index  = alfList_.GetCount();
		index += sfsList_.GetCount();
		index += ctxList_.GetCount();
		break;
	case APN_ALF_FILTER:
		success = AlfFilterPolicy::createApnInserted(parent, id);
		index = 0;
		break;
	case APN_ALF_CAPABILITY:
		success = AlfCapabilityFilterPolicy::createApnInserted(
		    parent, id);
		index = 0;
		break;
	case APN_SFS_ACCESS:
		success = SfsFilterPolicy::createApnInserted(parent, id);
		index = 0;
		break;
	case APN_CTX_RULE:
		success = ContextFilterPolicy::createApnInserted(parent, id);
		index = 0;
		break;
	case APN_SB_ACCESS:
		success = SbAccessFilterPolicy::createApnInserted(parent, id);
		index = 0;
		break;
	case APN_SFS_DEFAULT:
		success = SfsDefaultFilterPolicy::createApnInserted(parent, id);
		index = 0;
		break;
	case APN_DEFAULT:
		success = DefaultFilterPolicy::createApnInserted(parent, id);
		index = 0;
		break;
	default:
		success = false;
		index = -1;
		break;
	}

	if (success) {
		setModified();
		refresh();
	}

	return (index);
}

bool
PolicyRuleSet::prependAppPolicy(AppPolicy *app)
{
	if (app == NULL) {
		return (false);
	}

	if (apn_add(ruleSet_, app->getApnRule()) != 0) {
		return (false);
	}
	setModified();

	if (app->IsKindOf(CLASSINFO(ContextAppPolicy))) {
		ctxList_.Insert((size_t)0, app);
	} else if (app->IsKindOf(CLASSINFO(AlfAppPolicy))) {
		alfList_.Insert((size_t)0, app);
	} else if (app->IsKindOf(CLASSINFO(SbAppPolicy))) {
		sbList_.Insert((size_t)0, app);
	} else {
		return (false);
	}

	return (true);
}

void
PolicyRuleSet::clean(void)
{
	alfList_.DeleteContents(true);
	alfList_.Clear();
	sfsList_.DeleteContents(true);
	sfsList_.Clear();
	ctxList_.DeleteContents(true);
	ctxList_.Clear();
	sbList_.DeleteContents(true);
	sbList_.Clear();
}

void
PolicyRuleSet::create(wxString fileName, bool checkPerm)
{
	wxString		 logEntry;
	int			 rc;
	int			 flags;
	struct apn_ruleset	*ruleSet;
	struct apn_errmsg	*errMsg;

	ruleSet = NULL;
	flags = 0;

	if (!checkPerm) {
		flags |= APN_FLAG_NOPERMCHECK;
	}

	rc = apn_parse(fileName.fn_str(), &ruleSet, flags);

	switch (rc) {
	case -1:
		logEntry = _("System error during import of policy file ");
		logEntry += fileName + wxT(" : ");
		logEntry += wxString::From8BitData(strerror(errno));
		log(logEntry);
		status(logEntry);
		hasErrors_ = true;
		break;
	case 0:
		logEntry = _("Successfully imported policy file ");
		logEntry += fileName;
		log(logEntry);
		status(logEntry);
		create(ruleSet);
		break;
	case 1:
		logEntry = _("Failed import of policy file ");
		if (TAILQ_EMPTY(&(ruleSet->err_queue))) {
			logEntry += fileName;
			log(logEntry);
			break;
		}
		status(logEntry);
		TAILQ_FOREACH(errMsg, &(ruleSet->err_queue), entry) {
			logEntry = _("Failed import of policy file ");
			logEntry += wxString::From8BitData(errMsg->msg);
			log(logEntry);
		}
		hasErrors_ = true;
		break;
	default:
		logEntry = _("Unknown error during import of policy file ");
		logEntry += fileName;
		log(logEntry);
		hasErrors_ = true;
		break;
	}
}

void
PolicyRuleSet::create(struct apn_ruleset *ruleSet)
{
	struct apn_rule *rule;

	ruleSet_ = ruleSet;

	TAILQ_FOREACH(rule, &(ruleSet->alf_queue), entry) {
		alfList_.Append(new AlfAppPolicy(this, rule));
	}

	TAILQ_FOREACH(rule, &(ruleSet->sfs_queue), entry) {
		sfsList_.Append(new SfsAppPolicy(this, rule));
	}

	TAILQ_FOREACH(rule, &(ruleSet->ctx_queue), entry) {
		ctxList_.Append(new ContextAppPolicy(this, rule));
	}

	TAILQ_FOREACH(rule, &(ruleSet->sb_queue), entry) {
		sbList_.Append(new SbAppPolicy(this, rule));
	}
}

void
PolicyRuleSet::refresh(void)
{
	struct apn_ruleset	*ruleset = ruleSet_;
	clean();
	create(ruleset);
}

void
PolicyRuleSet::createAnswerPolicy(EscalationNotify *escalation)
{
	struct apn_ruleset		*rs;
	unsigned int			 triggerid;
	unsigned long			 flags;
	struct apn_rule			*triggerrule, *newblock = NULL, *tmp;
	struct apn_chain		 tmpchain;
	const struct anoubis_msg	*msg;
	const struct alf_event		*kernevent;
	struct apn_default		 action;
	time_t				 timeout = 0;
	anoubis_cookie_t		 task = 0;
	NotifyAnswer			*answer;
	wxCommandEvent			 event(anEVT_LOAD_RULESET);
	wxCommandEvent			 editevent(anEVT_SHOW_RULE);
	FilterPolicy			*triggerPolicy;
	AppPolicy			*parentPolicy;
	wxString			 module = escalation->getModule();

	TAILQ_INIT(&tmpchain);
	rs = getApnRuleSet();
	if (!rs)
		return;

	answer = escalation->getAnswer();
	triggerid = escalation->getRuleId();
	flags = answer->getFlags();
	triggerrule = apn_find_rule(rs, triggerid);
	if (!triggerrule)
		return;
	triggerPolicy = wxDynamicCast(triggerrule->userdata, FilterPolicy);
	if (!triggerPolicy)
		return;
	parentPolicy = triggerPolicy->getParentPolicy();
	if (!parentPolicy)
		return;
	if (parentPolicy->isAnyBlock() && module != wxT("SFS")) {
		wxString		 filename;
		unsigned char		 csum[APN_HASH_SHA256_LEN];
		struct apn_rule		*tmp;

		filename = escalation->getCtxBinaryName();
		if (filename.IsEmpty())
			goto err;
		if (!escalation->getCtxChecksum(csum))
			goto err;
		newblock = apn_find_rule(rs, parentPolicy->getApnRuleId());
		if (!newblock)
			goto err;
		tmp = apn_match_app(newblock->pchain, filename.To8BitData(),
		    csum);
		if (tmp != newblock) {
			newblock = NULL;
			goto err;
		}
		newblock = apn_copy_one_rule(newblock);
		if (!newblock)
			goto err;
		TAILQ_FOREACH(triggerrule, &newblock->rule.chain, entry)
			if (triggerrule->apn_id == triggerid)
				break;
		if (!triggerrule)
			goto err;
		if (apn_add_app(newblock, filename.To8BitData(), csum) != 0)
			goto err;
	}
	msg = escalation->rawMsg();
	if (answer->wasAllowed()) {
		action.action = APN_ACTION_ALLOW;
		action.log = APN_LOG_NONE;
	} else {
		action.action = APN_ACTION_DENY;
		action.log = APN_LOG_NORMAL;
	}
	if (answer->causeTmpRule()) {
		if (answer->getType() == NOTIFY_ANSWER_TIME) {
			timeout = answer->getTime();
		} else {
			task = escalation->getTaskCookie();
		}
	}
	if (module == wxT("ALF")) {
		kernevent = (struct alf_event *)&msg->u.notify->payload;
		if (apn_escalation_rule_alf(&tmpchain, kernevent,
		    &action, flags) != 0) {
			goto err;
		}
	} else if (module == wxT("SANDBOX")) {
		wxString	prefix = answer->getPrefix();
		unsigned long	flags = answer->getFlags();

		if (flags == 0)
			goto err;
		if (prefix.IsEmpty())
			goto err;
		if (apn_escalation_rule_sb(&tmpchain, triggerrule,
		    &action, prefix.To8BitData(), flags) != 0)
			goto err;
	} else if (module == wxT("SFS")) {
		wxString	prefix = answer->getPrefix();
		int		sfsmatch;

		if (prefix.IsEmpty())
			goto err;
		sfsmatch = get_value(msg->u.notify->sfsmatch);
		if (apn_escalation_rule_sfs(&tmpchain, triggerrule,
		    &action, prefix.To8BitData(), sfsmatch) != 0)
			goto err;
	} else {
		goto err;
	}
	if (apn_escalation_addscope(&tmpchain, triggerrule->scope,
	    timeout, task) != 0)
		goto err;
	tmp = TAILQ_FIRST(&tmpchain);
	if (newblock) {
		if (apn_escalation_splice(NULL, triggerrule, &tmpchain) != 0)
			goto err;
		if (apn_insert(rs, newblock, parentPolicy->getApnRuleId()) != 0)
			goto err;
	} else {
		if (apn_escalation_splice(rs, triggerrule, &tmpchain) != 0)
			goto err;
	}
	/* this PolicyRuleSet has changed, update myself */
	refresh();

	ProfileCtrl::getInstance()->sendToDaemon(getRuleSetId());
	event.SetInt(getRuleSetId());
	event.SetExtraLong(getRuleSetId());
	wxPostEvent(AnEvents::getInstance(), event);
	if (tmp && answer->getEditor()) {
		editevent.SetInt(false);
		editevent.SetExtraLong(tmp->apn_id);
		wxPostEvent(AnEvents::getInstance(), editevent);
	}
	return;
err:
	/* XXX CEH: Serious error! Show a popup here? */
	while(!TAILQ_EMPTY(&tmpchain)) {
		struct apn_rule		*tmp = TAILQ_FIRST(&tmpchain);
		TAILQ_REMOVE(&tmpchain, tmp, entry);
		apn_free_one_rule(tmp, NULL);
	}
	if (newblock)
		apn_free_one_rule(newblock, NULL);
	return;
}

void
PolicyRuleSet::log(const wxString &msg)
{
	/*
	 * Enable logging if AnoubisGuiApp is available.
	 * You don't have a AnoubisGuiApp in case of a unit-test.
	 */
	if (dynamic_cast<AnoubisGuiApp*>(wxTheApp))
		wxGetApp().log(msg);
}

void
PolicyRuleSet::status(const wxString &msg)
{
	/*
	 * Enable logging if AnoubisGuiApp is available.
	 * You don't have a AnoubisGuiApp in can of a unit-test.
	 */
	if (dynamic_cast<AnoubisGuiApp*>(wxTheApp))
		wxGetApp().status(msg);
}
