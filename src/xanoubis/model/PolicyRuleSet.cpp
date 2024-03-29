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

#include <anoubis_errno.h>

#include "MainUtils.h"
#include "PolicyCtrl.h"
#include "PolicyRowProvider.h"
#include "PolicyVisitor.h"
#include "RuleSetClearModifiedVisitor.h"
#include "RuleSetSearchPolicyVisitor.h"
#include "main.h"

PolicyRuleSet::PolicyRuleSet(int priority, uid_t uid,
    struct apn_ruleset *ruleSet)
{
	refCnt_     = 0;
	ruleSetId_  = wxNewId();
	uid_	    = uid;
	priority_   = priority;
	rowProvider_ = new PolicyRowProvider(this);
	ruleSet_    = NULL; /* Set by crate() */
	origin_     = wxT("native apn ruleset");
	hasErrors_  = false;
	isModified_ = false;
	isInProgress_ = false;
	isDaemonRuleSet_ = false;

	create(ruleSet);
}

PolicyRuleSet::PolicyRuleSet(int priority, uid_t uid, const wxString &fileName)
{
	refCnt_     = 0;
	ruleSetId_  = wxNewId();
	uid_	    = uid;
	priority_   = priority;
	rowProvider_ = new PolicyRowProvider(this);
	ruleSet_    = NULL; /* Set by crate() */
	origin_	    = fileName;
	hasErrors_  = false;
	isModified_ = false;
	isInProgress_ = false;
	isDaemonRuleSet_ = false;

	create(fileName);
}

PolicyRuleSet::~PolicyRuleSet(void)
{
	clean();
	apn_free_ruleset(ruleSet_);
	delete rowProvider_;
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
	AppPolicy			*parent = NULL;
	wxString			 hashValue;
	bool				 canEdit = true;

	if (isAdmin() && geteuid() != 0)
		canEdit = false;
	seeker = new RuleSetSearchPolicyVisitor(escalation->getRuleId());
	this->accept(*seeker);
	if (seeker->hasMatchingPolicy()) {
		filter = dynamic_cast<FilterPolicy*>(
		    seeker->getMatchingPolicy());
		if (filter) {
			parent = dynamic_cast<AppPolicy*>(
			    filter->getParentPolicy());
		}
	}
	delete seeker;
	if (!parent) {
		canEdit = false;
	}
	escalation->setAllowEdit(canEdit);
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
	std::vector<Policy *>::iterator i;

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

bool
PolicyRuleSet::toString(wxString &policy) const
{
	int	 result;
	bool	 success;
	wxString content;
	wxString tmpFileName;
	wxFFile	 tmpFile;

	result = 1;
	content = wxEmptyString;
	success = true;
	tmpFileName = wxFileName::CreateTempFileName(wxEmptyString);

	if (!tmpFile.Open(tmpFileName, wxT("w"))) {
		/* Couldn't open / fill tmp file. */
		return (false);
	}

	/* Write the ruleset to tmp file. */
	result = apn_print_ruleset(ruleSet_, 0, tmpFile.fp());
	tmpFile.Flush();
	tmpFile.Close();

	if (result != 0) {
		/* Something went wrong during file filling! */

		if (wxFileExists(tmpFileName))
			wxRemoveFile(tmpFileName);

		return (false);
	}

	if (tmpFile.Open(tmpFileName, wxT("r"))) {
		success = tmpFile.ReadAll(&content);
		tmpFile.Close();
	} else
		success = false;

	wxRemoveFile(tmpFileName);

	if (success)
		policy = content;

	return (success);
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

	if (result == 0) {
		/* Successful export, flush content */
		exportFile.Flush();
	}

	exportFile.Close();

	return (result == 0);
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

void
PolicyRuleSet::setInProgress(void)
{
	isInProgress_ = true;
}

void
PolicyRuleSet::clearInProgress(void)
{
	isInProgress_ = false;
}

bool
PolicyRuleSet::isInProgress(void) const
{
	return isInProgress_;
}

size_t
PolicyRuleSet::getAppPolicyCount(void) const
{
	size_t count;

	count  = alfList_.size();
	count += sfsList_.size();
	count += ctxList_.size();
	count += sbList_.size();

	return (count);
}

AppPolicy *
PolicyRuleSet::getPolicyAt(unsigned int idx) const
{
	Policy *policy = NULL;

	if (idx < alfList_.size()) {
		policy = alfList_[idx];
		return (dynamic_cast<AppPolicy*>(policy));
	}
	idx -= alfList_.size();

	if (idx < sfsList_.size()) {
		policy = sfsList_[idx];
		return (dynamic_cast<AppPolicy*>(policy));
	}
	idx -= sfsList_.size();

	if (idx < ctxList_.size()) {
		policy = ctxList_[idx];
		return (dynamic_cast<AppPolicy*>(policy));
	}
	idx -= ctxList_.size();

	if (idx < sbList_.size()) {
		policy = sbList_[idx];
		return (dynamic_cast<AppPolicy*>(policy));
	}

	if (policy == NULL) {
		return (NULL);
	}

	return (dynamic_cast<AppPolicy*>(policy));
}

int
PolicyRuleSet::getIndexOfPolicy(AppPolicy *policy) const
{
	int idx = 0;

	if (policy == 0)
		return (-1);

	if (dynamic_cast<AlfAppPolicy*>(policy)) {
		for (unsigned int i = 0; i < alfList_.size(); i++) {
			if (alfList_[i] == policy)
				return (i);
		}

		return (-1);
	}

	idx += alfList_.size();

	if (dynamic_cast<SfsAppPolicy*>(policy)) {
		for (unsigned int i = 0; i < sfsList_.size(); i++) {
			if (sfsList_[i] == policy)
				return (idx + i);
		}

		return (-1);
	}

	idx += sfsList_.size();

	if (dynamic_cast<ContextAppPolicy*>(policy)) {
		for (unsigned int i = 0; i < ctxList_.size(); i++) {
			if (ctxList_[i] == policy)
				return (idx + i);
		}

		return (-1);
	}

	idx += ctxList_.size();

	if (dynamic_cast<SbAppPolicy*>(policy)) {
		for (unsigned int i = 0; i < sbList_.size(); i++) {
			if (sbList_[i] == policy)
				return (idx + i);
		}

		return (-1);
	}

	return (-1);
}

AlfAppPolicy *
PolicyRuleSet::searchAlfAppPolicy(wxString binary) const
{
	wxArrayString	 list;
	struct apn_rule	*rule;
	AlfAppPolicy	*result;

	result = NULL;

	rule = apn_match_appname(&(ruleSet_->alf_queue),
	    binary.To8BitData(), 0);
	if (rule != NULL) {
		result = dynamic_cast<AlfAppPolicy*>((Policy*) rule->userdata);
	}
	if (result != NULL) {
		list = result->getBinaryList();
		if (list.Index(binary) == wxNOT_FOUND) {
			result = NULL;
		}
	}

	return (result);
}

ContextAppPolicy *
PolicyRuleSet::searchContextAppPolicy(wxString binary) const
{
	wxArrayString		 list;
	struct apn_rule		*rule;
	ContextAppPolicy	*result;

	result = NULL;

	rule = apn_match_appname(&(ruleSet_->ctx_queue),
	    binary.To8BitData(), 0);
	if (rule != NULL) {
		result = dynamic_cast<ContextAppPolicy*>(
		    (Policy*) rule->userdata);
	}
	if (result != NULL) {
		list = result->getBinaryList();
		if (list.Index(binary) == wxNOT_FOUND) {
			result = NULL;
		}
	}

	return (result);
}

SbAppPolicy *
PolicyRuleSet::searchSandboxAppPolicy(wxString binary) const
{
	wxArrayString	 list;
	struct apn_rule	*rule;
	SbAppPolicy	*result;

	result = NULL;

	rule = apn_match_appname(&(ruleSet_->sb_queue),
	    binary.To8BitData(), 0);
	if (rule != NULL) {
		result = dynamic_cast<SbAppPolicy*>((Policy*) rule->userdata);
	}
	if (result != NULL) {
		list = result->getBinaryList();
		if (list.Index(binary) == wxNOT_FOUND) {
			result = NULL;
		}
	}

	return (result);
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
		index  = alfList_.size();
		index += sfsList_.size();
		break;
	case APN_SB:
		success = SbAppPolicy::createApnInserted(this, id);
		index  = alfList_.size();
		index += sfsList_.size();
		index += ctxList_.size();
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
		refresh();
		setModified();
		rowProvider_->sizeChangeEvent(getAppPolicyCount());
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

	if (dynamic_cast<ContextAppPolicy*>(app)) {
		ctxList_.insert(ctxList_.begin(), app);
	} else if (dynamic_cast<AlfAppPolicy*>(app)) {
		alfList_.insert(alfList_.begin(), app);
	} else if (dynamic_cast<SbAppPolicy*>(app)) {
		sbList_.insert(sbList_.begin(), app);
	} else {
		return (false);
	}

	rowProvider_->sizeChangeEvent(getAppPolicyCount());

	return (true);
}

void
PolicyRuleSet::clean(void)
{
#define CLEAR_VECTOR(v) \
	while (!(v).empty()) { \
		Policy *policy = (v).back(); \
		(v).pop_back(); \
		delete policy; \
	}

	CLEAR_VECTOR(alfList_);
	CLEAR_VECTOR(sfsList_);
	CLEAR_VECTOR(ctxList_);
	CLEAR_VECTOR(sbList_);

#undef CLEAR_VECTOR
}

void
PolicyRuleSet::create(wxString fileName)
{
	wxString		 logEntry;
	int			 rc;
	int			 flags;
	struct apn_ruleset	*ruleSet;
	struct apn_errmsg	*errMsg;

	ruleSet = NULL;
	flags = 0;

	rc = apn_parse(fileName.fn_str(), &ruleSet, flags);

	switch (rc) {
	case -1:
		logEntry = wxString::Format(_("System error during import of "
		    "policy file %ls: %hs"),
		    fileName.c_str(), anoubis_strerror(errno));
		Debug::err(logEntry);
		status(logEntry);
		hasErrors_ = true;
		break;
	case 0:
		logEntry = wxString::Format(
		    _("Successfully imported policy file %ls"),
		    fileName.c_str());
		Debug::info(logEntry);
		status(logEntry);
		create(ruleSet);
		break;
	case 1:
		hasErrors_ = true;
		if (TAILQ_EMPTY(&(ruleSet->err_queue))) {
			logEntry = wxString::Format(
			    _("Failed import of policy file %ls"),
			    fileName.c_str());
			Debug::err(logEntry);
			break;
		}
		status(logEntry);
		TAILQ_FOREACH(errMsg, &(ruleSet->err_queue), entry) {
			logEntry = wxString::Format(
			    _("Failed import of policy file %hs"),
			    errMsg->msg);
			Debug::err(logEntry);
		}
		apn_free_ruleset(ruleSet);
		break;
	default:
		logEntry = wxString::Format(
		    _("Unknown error during import of policy file %ls"),
		    fileName.c_str());
		Debug::err(logEntry);
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
		alfList_.push_back(new AlfAppPolicy(this, rule));
	}

	TAILQ_FOREACH(rule, &(ruleSet->sfs_queue), entry) {
		sfsList_.push_back(new SfsAppPolicy(this, rule));
	}

	TAILQ_FOREACH(rule, &(ruleSet->ctx_queue), entry) {
		ctxList_.push_back(new ContextAppPolicy(this, rule));
	}

	TAILQ_FOREACH(rule, &(ruleSet->sb_queue), entry) {
		sbList_.push_back(new SbAppPolicy(this, rule));
	}
	ruleSet_->destructor = policy_destructor;
}

void
PolicyRuleSet::refresh(void)
{
	struct apn_ruleset	*ruleset = ruleSet_;
	clean();
	create(ruleset);

	rowProvider_->sizeChangeEvent(getAppPolicyCount());
}

AnRowProvider *
PolicyRuleSet::getRowProvider(void) const
{
	return (this->rowProvider_);
}

bool
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
	int				 ispg;
	bool				 copyblock;

	TAILQ_INIT(&tmpchain);
	rs = getApnRuleSet();
	if (!rs)
		return false;

	Debug::trace(wxT("PolicyRuleSet::createAnswerPolicy"));

	answer = escalation->getAnswer();
	ispg = (escalation->getPlaygroundID() != 0);
	triggerid = escalation->getRuleId();
	flags = answer->getFlags();
	triggerrule = apn_find_rule(rs, triggerid);
	if (!triggerrule)
		return false;
	triggerPolicy = dynamic_cast<FilterPolicy*>(
	    (Policy*) triggerrule->userdata);
	if (!triggerPolicy)
		return false;
	parentPolicy = triggerPolicy->getParentPolicy();
	if (!parentPolicy)
		return -1;
	copyblock = false;
	if (module != wxT("CTX")) {
		if (parentPolicy->isAnyBlock() && module != wxT("SFS"))
			copyblock = true;
		if (ispg && !parentPolicy->getFlag(APN_RULE_PGONLY))
			copyblock = true;
	}
	if (copyblock) {
		wxString		 filename;
		struct apn_rule		*tmp;

		filename = escalation->getCtxBinaryName();
		if (filename.IsEmpty())
			goto err;
		newblock = apn_find_rule(rs, parentPolicy->getApnRuleId());
		if (!newblock)
			goto err;
		/* XXX CEH: This check might need more thought. */
		tmp = apn_match_appname(newblock->pchain,
		    filename.To8BitData(), ispg);
		if (tmp != newblock) {
			newblock = NULL;
			goto err;
		}
		newblock = apn_copy_one_rule(newblock);
		if (!newblock)
			goto err;
		if (ispg)
			newblock->flags |= APN_RULE_PGONLY;
		TAILQ_FOREACH(triggerrule, &newblock->rule.chain, entry)
			if (triggerrule->apn_id == triggerid)
				break;
		if (!triggerrule)
			goto err;
		if (newblock->app == NULL) {
			struct apn_subject	 tmpsubject;

			tmpsubject.type = APN_CS_NONE;
			if (apn_add_app(newblock, filename.To8BitData(),
			    &tmpsubject) != 0)
				goto err;
		}
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

	event.SetInt(getRuleSetId());
	event.SetExtraLong(getRuleSetId());
	wxPostEvent(AnEvents::instance(), event);
	if (tmp && answer->getEditor()) {
		editevent.SetInt(false);
		editevent.SetExtraLong(tmp->apn_id);
		wxPostEvent(AnEvents::instance(), editevent);
	}
	return true;
err:
	while(!TAILQ_EMPTY(&tmpchain)) {
		struct apn_rule		*tmp = TAILQ_FIRST(&tmpchain);
		TAILQ_REMOVE(&tmpchain, tmp, entry);
		apn_free_one_rule(tmp, NULL);
	}
	if (newblock)
		apn_free_one_rule(newblock, NULL);
	return false;
}

void
PolicyRuleSet::status(const wxString &msg)
{
	/*
	 * Enable logging if AnoubisGuiApp is available.
	 * You don't have a AnoubisGuiApp in can of a unit-test.
	 */
	if (dynamic_cast<AnoubisGuiApp*>(wxTheApp))
		MainUtils::instance()->status(msg);
}
