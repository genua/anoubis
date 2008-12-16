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

#include <sys/param.h>
#include <sys/socket.h>

#ifndef LINUX
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>
#include <wx/string.h>
#include <wx/ffile.h>
#include <wx/window.h>
#include <wx/progdlg.h>
#include <wx/utils.h>

#include <apn.h>
#include <errno.h>

#include "AnEvents.h"
#include "main.h"
#include "Policy.h"
#include "AppPolicy.h"
#include "VarPolicy.h"
#include "PolicyRuleSet.h"
#include "PolicyVisitor.h"
#include "RuleSetSearchPolicyVisitor.h"
#include "RuleEditorChecksumVisitor.h"

#define CALLOC_STRUCT(type) (struct type *)calloc(1, sizeof(struct type))

PolicyRuleSet::PolicyRuleSet(int priority, uid_t uid,
    struct apn_ruleset *ruleSet)
{
	id_ = wxNewId();
	uid_ = uid;
	hasErrors_ = false;
	isModified_ = false;
	ruleSet_ = NULL;
	priority_ = priority;
	origin_ = wxT("Daemon");

	create(ruleSet);
	AnEvents::getInstance()->Connect(anEVT_ANSWER_ESCALATION,
	    wxCommandEventHandler(PolicyRuleSet::OnAnswerEscalation),
	    NULL, this);
}

PolicyRuleSet::PolicyRuleSet(int priority, uid_t uid, wxString fileName,
    bool checkPerm)
{
	id_ = wxNewId();
	uid_ = uid;
	hasErrors_ = false;
	isModified_ = false;
	ruleSet_ = NULL;
	priority_ = priority;
	origin_ = fileName;

	create(fileName, checkPerm);
	AnEvents::getInstance()->Connect(anEVT_ANSWER_ESCALATION,
	    wxCommandEventHandler(PolicyRuleSet::OnAnswerEscalation),
	    NULL, this);
}

PolicyRuleSet::~PolicyRuleSet(void)
{
	AnEvents::getInstance()->Disconnect(anEVT_ANSWER_ESCALATION,
	    wxCommandEventHandler(PolicyRuleSet::OnAnswerEscalation),
	    NULL, this);

	clean();
	apn_free_ruleset(ruleSet_);
}

void
PolicyRuleSet::create(struct apn_ruleset *ruleSet)
{
	struct apn_rule *appRule;
	struct var	*variable;

	ruleSet_ = ruleSet;

	TAILQ_FOREACH(appRule, &(ruleSet->alf_queue), entry) {
		alfList_.Append(new AppPolicy(appRule, this));
	}

	TAILQ_FOREACH(appRule, &(ruleSet->sfs_queue), entry) {
		sfsList_.Append(new AppPolicy(appRule, this));
	}

	TAILQ_FOREACH(appRule, &(ruleSet->ctx_queue), entry) {
		ctxList_.Append(new AppPolicy(appRule, this));
	}

	TAILQ_FOREACH(variable, &(ruleSet->var_queue), entry) {
		varList_.Append(new VarPolicy(variable));
	}
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
	varList_.DeleteContents(true);
	varList_.Clear();
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

bool
PolicyRuleSet::hasLocalHost(wxArrayString list)
{
	bool result = false;

	if (list.IsEmpty()) {
		return (false);
	}

	for (size_t i=0; i<list.GetCount(); i++) {
		if ((list.Item(i).Cmp(wxT("127.0.0.1")) == 0) ||
		   (list.Item(i).Cmp(wxT("::1")) == 0)) {
			result = true;
		}
	}

	return (result);
}

struct apn_rule *
PolicyRuleSet::assembleAlfPolicy(AlfPolicy *old, EscalationNotify *escalation)
{
	struct apn_rule		*newAlfRule;
	struct apn_afiltrule	*afilt;
	struct apn_acaprule	*acap;
	NotifyAnswer		*answer;

	newAlfRule = old->cloneRule();

	if (newAlfRule == NULL) {
		return (NULL);
	}

	if (escalation->getAnswer()->causeTmpRule()) {
		if (newAlfRule->scope == NULL) {
			newAlfRule->scope = CALLOC_STRUCT(apn_scope);
		}
	} else {
		if (newAlfRule->scope != NULL) {
			free(newAlfRule->scope);
			newAlfRule->scope = NULL;
		}
	}

	answer = escalation->getAnswer();
	newAlfRule->apn_type = old->getTypeNo();

	switch (old->getTypeNo()) {
	case APN_ALF_FILTER:
		afilt = &(newAlfRule->rule.afilt);
		if (answer->wasAllowed()) {
			afilt->action = APN_ACTION_ALLOW;
		} else {
			afilt->action = APN_ACTION_DENY;
		}
		afilt->filtspec.netaccess = escalation->getDirectionNo();

		if (old->getDirectionNo() == APN_ACCEPT) {
			apn_free_port(afilt->filtspec.fromport);
			afilt->filtspec.fromport = NULL;
			if (hasLocalHost(old->getFromHostList())) {
				apn_free_host(afilt->filtspec.fromhost);
				afilt->filtspec.fromhost = NULL;
			}
		} else {
			apn_free_port(afilt->filtspec.toport);
			afilt->filtspec.toport = NULL;
			if (hasLocalHost(old->getToHostList())) {
				apn_free_host(afilt->filtspec.tohost);
				afilt->filtspec.tohost = NULL;
			}
		}
		break;
	case APN_ALF_CAPABILITY:
		newAlfRule->rule.acap.capability = old->getCapTypeNo();
		if (answer->wasAllowed()) {
			newAlfRule->rule.acap.action = APN_ACTION_ALLOW;
		} else {
			newAlfRule->rule.acap.action = APN_ACTION_DENY;
		}
		break;
	case APN_DEFAULT:
		short proto = escalation->getProtocolNo();
		int log = old->getLogNo();
		if (proto == IPPROTO_TCP || proto == IPPROTO_UDP) {
			newAlfRule->apn_type = APN_ALF_FILTER;
			afilt = &(newAlfRule->rule.afilt);
			afilt->filtspec.log = log;
			afilt->filtspec.netaccess =
			    escalation->getDirectionNo();
			if (answer->wasAllowed()) {
				afilt->action = APN_ACTION_ALLOW;
			} else {
				afilt->action = APN_ACTION_DENY;
			}
			afilt->filtspec.af = 0; /* any */
			afilt->filtspec.proto = escalation->getProtocolNo();
			apn_free_port(afilt->filtspec.fromport);
			afilt->filtspec.fromport = NULL;
			apn_free_port(afilt->filtspec.toport);
			afilt->filtspec.toport = NULL;
			if (old->getDirectionNo() == APN_ACCEPT) {
				apn_free_host(afilt->filtspec.tohost);
				afilt->filtspec.tohost = NULL;
			} else {
				apn_free_host(afilt->filtspec.fromhost);
				afilt->filtspec.fromhost = NULL;
				}
		} else if (proto == IPPROTO_ICMP) {
			newAlfRule->apn_type = APN_ALF_CAPABILITY;
			acap = &(newAlfRule->rule.acap);
			acap->log = log;
			if (answer->wasAllowed()) {
				acap->action = APN_ACTION_ALLOW;
			} else {
				acap->action = APN_ACTION_DENY;
			}
			acap->capability = APN_ALF_CAPRAW;;
		} else {
			apn_free_one_rule(newAlfRule, NULL);
			newAlfRule = NULL;
		}
		break;
	}

	return (newAlfRule);
}

void
PolicyRuleSet::createAnswerPolicy(EscalationNotify *escalation)
{
	unsigned char			 csum[APN_HASH_SHA256_LEN];
	wxString			 hashValue;
	wxString			 filename;
	Policy				*triggerPolicy;
	Policy				*parentPolicy;
	NotifyAnswer			*answer;
	RuleSetSearchPolicyVisitor	*seeker;
	struct apn_rule			*newAlfRule;
	struct apn_rule			*newTmpAlfRule;
	bool				 hasChecksum;
	wxCommandEvent			 event(anEVT_LOAD_RULESET);

	/* get the policy caused this escalation */
	seeker = new RuleSetSearchPolicyVisitor(escalation->getRuleId());
	this->accept(*seeker);
	if (! seeker->hasMatchingPolicy()) {
		return;
	}
	triggerPolicy = seeker->getMatchingPolicy();
	delete seeker;

	/* get the enclosing app policy */
	parentPolicy = triggerPolicy->getParent();
	if (!parentPolicy) {
		status(_("Could not modify Policy (no such rule)"));
		return;
	}

	filename = escalation->getBinaryName();
	hasChecksum = escalation->getChecksum(csum);

	newAlfRule = assembleAlfPolicy((AlfPolicy *)triggerPolicy, escalation);
	if (newAlfRule == NULL) {
		status(_("Couldn't clone and insert policy."));
		return;
	}

	if (!parentPolicy->isDefault()) {
		/*
		 * This escalation was caused by a rule not been element of
		 * an any-rule. Thus we can just insert the new policy before
		 * the troggering rule.
		 */
		newAlfRule->apn_id = 0;
		apn_insert_alfrule(ruleSet_, newAlfRule,
		    escalation->getRuleId());
	} else {
		int				 hashType;
		RuleSetSearchPolicyVisitor	*seekDouble;
		/*
		 * This escalation was caused by a rule been placed within an
		 * any-rule. Thus we have to copy the any block and insert
		 * the new rule to the new block.
		 */
		if (hasChecksum) {
			hashType = APN_HASH_SHA256;
			/* XXX ch: move this into a method */
			hashValue = wxT("0x");
			for (unsigned int i=0; i<APN_HASH_SHA256_LEN; i++) {
				hashValue += wxString::Format(wxT("%2.2x"),
				    (unsigned char)csum[i]);
			}
		} else {
			hashType = APN_HASH_NONE;
			hashValue = wxEmptyString;
		}
		seekDouble = new RuleSetSearchPolicyVisitor(hashValue);
		this->accept(*seekDouble);
		if (!seekDouble->hasMatchingPolicy()) {
			apn_copyinsert_alf(ruleSet_, newAlfRule,
			    escalation->getRuleId(), filename.To8BitData(),
			    csum, hashType);
		}
		delete seekDouble;
		/*
		 * XXX ch: we also add an allow rule to the any-block,
		 * because the current running application may not leave it.
		 * Refere to bug #630 for this topic.
		 */
		newTmpAlfRule = assembleAlfPolicy((AlfPolicy *)triggerPolicy,
		    escalation);
		if (newAlfRule == NULL) {
			status(_("Couldn't clone and insert policy."));
			return;
		}
		if (newTmpAlfRule->scope == NULL) {
			newTmpAlfRule->scope = CALLOC_STRUCT(apn_scope);
		}
		newTmpAlfRule->scope->task = escalation->getTaskCookie();
		answer = escalation->getAnswer();
		if (answer->getType() == NOTIFY_ANSWER_TIME) {
			newAlfRule->scope->timeout = answer->getTime();
		}
		apn_insert_alfrule(ruleSet_, newTmpAlfRule,
		    escalation->getRuleId());
	}

	answer = escalation->getAnswer();
	if (answer->causeTmpRule()) {
		if (answer->getType() == NOTIFY_ANSWER_TIME) {
			newAlfRule->scope->timeout = answer->getTime();
		} else {
			newAlfRule->scope->task = escalation->getTaskCookie();
		}
	}

	/* this PolicyRuleSet has changed, update myself */
	clean();
	create(this->ruleSet_);

	ProfileCtrl::getInstance()->sendToDaemon(getId());

	event.SetClientData(this);
	wxPostEvent(AnEvents::getInstance(), event);
}

void
PolicyRuleSet::OnAnswerEscalation(wxCommandEvent& event)
{
	EscalationNotify	*escalation;

	escalation = (EscalationNotify *)event.GetClientObject();
	if (escalation->getAnswer()->causeTmpRule() ||
	    escalation->getAnswer()->causePermRule()) {
		createAnswerPolicy(escalation);
	}

	event.Skip();
}

void
PolicyRuleSet::accept(PolicyVisitor& visitor)
{
	PolicyList::iterator	i;

	for (i=alfList_.begin(); i != alfList_.end(); ++i) {
		(*i)->accept(visitor);
	}
	for (i=sfsList_.begin(); i != sfsList_.end(); ++i) {
		(*i)->accept(visitor);
	}
	for (i=ctxList_.begin(); i != ctxList_.end(); ++i) {
		(*i)->accept(visitor);
	}
	for (i=varList_.begin(); i != varList_.end(); ++i) {
		(*i)->accept(visitor);
	}
}

void
PolicyRuleSet::exportToFile(wxString fileName)
{
	wxString	 logEntry;
	wxFFile		*exportFile;

	exportFile = new wxFFile(fileName, wxT("w"));

	if (exportFile->IsOpened()) {
		if (apn_print_ruleset(ruleSet_, 0, exportFile->fp()) == 0) {
			logEntry = _("Policies exported successfully to ");
		}
		fchmod(fileno(exportFile->fp()), S_IRUSR);
		exportFile->Close();
	} else {
		logEntry = _("Could not open file for export: ");
	}         logEntry += fileName;
	log(logEntry);
	status(logEntry);
}

int
PolicyRuleSet::createAppPolicy(int type, int insertBeforeId)
{
	int		 newId;
	int		 rc;
	wxCommandEvent	 event(anEVT_LOAD_RULESET);
	struct apn_rule	*newAppRule;

	newId	   = -1;
	newAppRule = CALLOC_STRUCT(apn_rule);

	if (newAppRule == NULL) {
		return (-1);
	}

	newAppRule->apn_type = type;
	newAppRule->app = CALLOC_STRUCT(apn_app);
	if (newAppRule->app == NULL) {
		free(newAppRule);
		return (-1);
	}

	newAppRule->app->hashtype = APN_HASH_NONE;
	switch(type) {
	case APN_ALF:
		if (TAILQ_EMPTY(&ruleSet_->alf_queue))
			insertBeforeId = 0;
		break;
	case APN_SB:
		if (TAILQ_EMPTY(&ruleSet_->sb_queue))
			insertBeforeId = 0;
		break;
	case APN_CTX:
		if (TAILQ_EMPTY(&ruleSet_->ctx_queue))
			insertBeforeId = 0;
		break;
	}
	rc = apn_insert(ruleSet_, newAppRule, insertBeforeId);
	if (rc == 0) {
		newId = newAppRule->apn_id;
		clean();
		create(ruleSet_);
		event.SetClientData((void*)this);
		wxPostEvent(AnEvents::getInstance(), event);
	} else {
		free(newAppRule->app);
		free(newAppRule);
	}

	return (newId);
}

int
PolicyRuleSet::createAlfPolicy(int insertBeforeId)
{
	int				 newId;
	int				 rc;
	wxCommandEvent			 event(anEVT_LOAD_RULESET);
	RuleSetSearchPolicyVisitor	 seeker(insertBeforeId);
	Policy				*parentPolicy;
	struct apn_rule			*newAlfRule;

	this->accept(seeker);
	if (! seeker.hasMatchingPolicy()) {
		return (-1);
	}

	newId = -1;
	rc = -1;
	newAlfRule = CALLOC_STRUCT(apn_rule);
	parentPolicy = seeker.getMatchingPolicy();

	if (newAlfRule == NULL) {
		return (-1);
	}

	newAlfRule->apn_type = APN_ALF_FILTER;
	newAlfRule->rule.afilt.filtspec.proto = IPPROTO_TCP;
	newAlfRule->rule.afilt.filtspec.af = AF_INET;
	newAlfRule->rule.afilt.filtspec.netaccess = APN_CONNECT;

	if (parentPolicy->IsKindOf(CLASSINFO(AppPolicy))) {
		rc = apn_add2app_alfrule(ruleSet_, newAlfRule, insertBeforeId);
	} else if (parentPolicy->IsKindOf(CLASSINFO(AlfPolicy))) {
		rc = apn_insert_alfrule(ruleSet_, newAlfRule, insertBeforeId);
	}

	if (rc == 0) {
		newId = newAlfRule->apn_id;
		clean();
		create(ruleSet_);
		event.SetClientData((void*)this);
		wxPostEvent(AnEvents::getInstance(), event);
	} else {
		free(newAlfRule);
	}

	return (newId);
}

int
PolicyRuleSet::createCtxNewPolicy(int insertBeforeId)
{
	int				 newId;
	int				 rc;
	wxCommandEvent			 event(anEVT_LOAD_RULESET);
	RuleSetSearchPolicyVisitor	 seeker(insertBeforeId);
	Policy				*parentPolicy;
	struct apn_rule			*newrule;

	this->accept(seeker);
	if (! seeker.hasMatchingPolicy()) {
		return (-1);
	}

	newId = -1;
	rc = -1;
	newrule = CALLOC_STRUCT(apn_rule);
	parentPolicy = seeker.getMatchingPolicy();

	if (newrule == NULL) {
		return (-1);
	}
	newrule->apn_type = APN_CTX_RULE;
	newrule->rule.apncontext.application = CALLOC_STRUCT(apn_app);
	if (newrule->rule.apncontext.application == NULL) {
		free(newrule);
		return (-1);
	}

	newrule->rule.apncontext.application->hashtype = APN_HASH_NONE;
	if (parentPolicy->IsKindOf(CLASSINFO(AppPolicy))) {
		rc = apn_add2app_ctxrule(ruleSet_, newrule, insertBeforeId);
	} else if (parentPolicy->IsKindOf(CLASSINFO(CtxPolicy))) {
		rc = apn_insert_ctxrule(ruleSet_, newrule, insertBeforeId);
	}

	if (rc == 0) {
		newId = newrule->apn_id;
		clean();
		create(ruleSet_);
		event.SetClientData((void*)this);
		wxPostEvent(AnEvents::getInstance(), event);
	} else {
		free(newrule->rule.apncontext.application);
		free(newrule);
	}

	return (newId);
}

int
PolicyRuleSet::createSfsPolicy(int insertBeforeId)
{
	int				 newId;
	int				 rc;
	wxCommandEvent			 event(anEVT_LOAD_RULESET);
	RuleSetSearchPolicyVisitor	 seeker(insertBeforeId);
	Policy				*parentPolicy;
	struct apn_rule			*sfsRootRule;
	struct apn_rule			*newSfsRule;

	this->accept(seeker);

	newId = -1;
	rc = -1;
	newSfsRule = CALLOC_STRUCT(apn_rule);
	if (! seeker.hasMatchingPolicy()) {
		parentPolicy = NULL;
	} else {
		parentPolicy = seeker.getMatchingPolicy();
	}

	if (newSfsRule == NULL) {
		return (-1);
	}

	newSfsRule->apn_type = APN_SFS_CHECK;
	newSfsRule->rule.sfscheck.app = CALLOC_STRUCT(apn_app);
	if (newSfsRule->rule.sfscheck.app == NULL) {
		free(newSfsRule);
		return (-1);
	}

	newSfsRule->apn_id = 0;
	newSfsRule->rule.sfscheck.app->hashtype = APN_HASH_NONE;

	if (TAILQ_EMPTY(&(ruleSet_->sfs_queue))) {
		sfsRootRule = CALLOC_STRUCT(apn_rule);
		sfsRootRule->apn_type = APN_SFS;
		apn_add(ruleSet_, sfsRootRule);
	}
	/* we assume sfs_queue will contain only one element */
	sfsRootRule = TAILQ_FIRST(&(ruleSet_->sfs_queue));

	if (parentPolicy && parentPolicy->IsKindOf(CLASSINFO(SfsPolicy))) {
		rc = apn_insert_sfsrule(ruleSet_, newSfsRule, insertBeforeId);
	} else {
		rc = apn_add2app_sfsrule(ruleSet_, newSfsRule,
		    sfsRootRule->apn_id);
	}

	if (rc == 0) {
		newId = newSfsRule->apn_id;
		clean();
		create(ruleSet_);
		event.SetClientData((void*)this);
		wxPostEvent(AnEvents::getInstance(), event);
	} else {
		free(newSfsRule);
	}

	return (newId);
}

void
PolicyRuleSet::clearModified(void)
{
	RuleEditorChecksumVisitor	seeker(0);

	this->accept(seeker);
	isModified_ = false;
}

bool
PolicyRuleSet::findMismatchHash(void)
{
	RuleEditorChecksumVisitor	seeker;

	this->accept(seeker);

	return (seeker.hasMismatch());

}

bool
PolicyRuleSet::deletePolicy(int id)
{
	wxCommandEvent			 event(anEVT_LOAD_RULESET);

	apn_remove(ruleSet_, id);
	clean();
	create(ruleSet_);
	event.SetClientData((void*)this);
	wxPostEvent(AnEvents::getInstance(), event);

	return (true);
}

bool
PolicyRuleSet::isEmpty(void)
{
	return (alfList_.IsEmpty() && sfsList_.IsEmpty() &&
	    varList_.IsEmpty() && ctxList_.IsEmpty());
}

bool
PolicyRuleSet::isAdmin(void)
{
	bool result;

	if (priority_ > 0) {
		result = false;
	} else {
		result = true;
	}

	return (result);
}

wxString
PolicyRuleSet::getOrigin(void)
{
	return (origin_);
}

wxString
PolicyRuleSet::toString(void) const
{
	wxString tmpFile = wxFileName::CreateTempFileName(wxT(""));
	wxString content = wxT("");
	int result = 1;

	wxFFile f(tmpFile, wxT("w"));

	if (f.IsOpened()) {
		result = apn_print_ruleset(ruleSet_, 0, f.fp());

		f.Flush();
		f.Close();
	}

	if (result != 0) {
		wxRemoveFile(tmpFile);
		return (content);
	}

	if (f.Open(tmpFile, wxT("r"))) {
		if (!f.ReadAll(&content))
			content = wxT("");

		f.Close();
	}

	wxRemoveFile(tmpFile);

	return (content);
}

long
PolicyRuleSet::getId(void) const
{
	return (id_);
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

bool
PolicyRuleSet::hasErrors(void) const
{
	return (hasErrors_);
}

bool
PolicyRuleSet::isModified(void) const
{
	return (isModified_);
}

void
PolicyRuleSet::setModified(bool modified)
{
	isModified_ = modified;
}

void
PolicyRuleSet::log(const wxString &msg)
{
	/*
	 * Enable logging if AnoubisGuiApp is available.
	 * You don't have a AnoubisGuiApp in can of a unit-test.
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
