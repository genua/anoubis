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

PolicyRuleSet::PolicyRuleSet(struct apn_ruleset *ruleSet)
{
	ruleSet_ = NULL;

	create(ruleSet);
	Connect(anEVT_ANSWER_ESCALATION,
	    wxCommandEventHandler(PolicyRuleSet::OnAnswerEscalation));
}

PolicyRuleSet::PolicyRuleSet(wxString fileName)
{
	ruleSet_ = NULL;

	create(fileName);
	Connect(anEVT_ANSWER_ESCALATION,
	    wxCommandEventHandler(PolicyRuleSet::OnAnswerEscalation));
}

PolicyRuleSet::~PolicyRuleSet(void)
{
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
		alfList_.Append(new AppPolicy(appRule));
	}

	TAILQ_FOREACH(appRule, &(ruleSet->sfs_queue), entry) {
		sfsList_.Append(new AppPolicy(appRule));
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
	varList_.DeleteContents(true);
	varList_.Clear();
}

void
PolicyRuleSet::create(wxString fileName)
{
	wxString		 logEntry;
	int			 rc;
	struct apn_ruleset	*ruleSet;
	struct apn_errmsg	*errMsg;

	ruleSet = NULL;
	rc = apn_parse(fileName.fn_str(), &ruleSet, 0);

	switch (rc) {
	case -1:
		logEntry = _("System error during import of policy file ");
		logEntry += fileName + wxT(" : ");
		logEntry += wxString::From8BitData(strerror(errno));
		wxGetApp().log(logEntry);
		wxGetApp().status(logEntry);
		break;
	case 0:
		logEntry = _("Successfully imported policy file ");
		logEntry += fileName;
		wxGetApp().log(logEntry);
		wxGetApp().status(logEntry);
		create(ruleSet);
		break;
	case 1:
		logEntry = _("Failed import of policy file ");
		if (TAILQ_EMPTY(&(ruleSet->err_queue))) {
			logEntry += fileName;
			wxGetApp().log(logEntry);
			break;
		}
		wxGetApp().status(logEntry);
		TAILQ_FOREACH(errMsg, &(ruleSet->err_queue), entry) {
			logEntry = _("Failed import of policy file ");
			logEntry += wxString::From8BitData(errMsg->msg);
			wxGetApp().log(logEntry);
		}
		break;
	default:
		logEntry = _("Unknown error during import of policy file ");
		logEntry += fileName;
		wxGetApp().log(logEntry);
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

struct apn_alfrule *
PolicyRuleSet::assembleAlfPolicy(AlfPolicy *old, EscalationNotify *escalation)
{
	struct apn_alfrule	*newAlfRule;
	struct apn_afiltrule	*afilt;
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
		}
	}

	answer = escalation->getAnswer();
	newAlfRule->type = old->getTypeNo();

	switch (old->getTypeNo()) {
	case APN_ALF_FILTER:
		afilt = &(newAlfRule->rule.afilt);
		if (answer->wasAllowed()) {
			afilt->action = APN_ACTION_ALLOW;
		} else {
			afilt->action = APN_ACTION_DENY;
		}
		if (old->getDirectionNo() == APN_ACCEPT) {
			apn_free_port(afilt->filtspec.fromport);
			if (hasLocalHost(old->getFromHostList())) {
				apn_free_host(afilt->filtspec.fromhost);
			}
		} else {
			apn_free_port(afilt->filtspec.toport);
			if (hasLocalHost(old->getToHostList())) {
				apn_free_host(afilt->filtspec.tohost);
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
	case APN_ALF_DEFAULT:
		newAlfRule->type = APN_ALF_FILTER;
		afilt = &(newAlfRule->rule.afilt);
		afilt->filtspec.af = 0; /* any */
		afilt->filtspec.proto = escalation->getProtocolNo();
		apn_free_port(afilt->filtspec.fromport);
		apn_free_port(afilt->filtspec.toport);
		if (old->getDirectionNo() == APN_ACCEPT) {
			apn_free_host(afilt->filtspec.tohost);
		} else {
			apn_free_host(afilt->filtspec.fromhost);
		}
		break;
	}

	return (newAlfRule);
}

void
PolicyRuleSet::createAnswerPolicy(EscalationNotify *escalation)
{
	unsigned char			 csum[APN_HASH_SHA256_LEN];
	wxString			 filename;
	Policy				*triggerPolicy;
	Policy				*parentPolicy;
	NotifyAnswer			*answer;
	RuleSetSearchPolicyVisitor	*seeker;
	struct apn_alfrule		*newAlfRule;

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

	filename = escalation->getBinaryName();
	escalation->getChecksum(csum);

	newAlfRule = assembleAlfPolicy((AlfPolicy *)triggerPolicy, escalation);

	if (!parentPolicy->isDefault()) {
		/*
		 * This escalation was caused by a rule not been element of
		 * an any-rule. Thus we can just insert the new policy before
		 * the troggering rule.
		 */
		apn_insert_alfrule(ruleSet_, newAlfRule,
		    escalation->getRuleId());
	} else {
		/*
		 * This escalation was caused by a rule been placed within an
		 * any-rule. Thus we have to copy the any block and insert
		 * the new rule to the new block.
		 */
		apn_copyinsert(ruleSet_, newAlfRule, escalation->getRuleId(),
		    filename.To8BitData(), csum, APN_HASH_SHA256);
	}

	answer = escalation->getAnswer();
	if (answer->causeTmpRule()) {
		if (answer->getType() == NOTIFY_ANSWER_TIME) {
			newAlfRule->scope->timeout = answer->getTime();
		} else {
			newAlfRule->scope->task = escalation->getToken();
		}
	}


	PolicyRuleSet	*nrs = new PolicyRuleSet(ruleSet_);
	wxCommandEvent           event(anEVT_LOAD_RULESET);
	event.SetClientData((void*)nrs);
	wxGetApp().sendEvent(event);

	wxString	tmpFileName;
	tmpFileName = wxFileName::CreateTempFileName(wxT("xanoubis"));
	nrs->exportToFile(tmpFileName);
	wxGetApp().usePolicy(tmpFileName);
}

void
PolicyRuleSet::OnAnswerEscalation(wxCommandEvent& event)
{
	EscalationNotify	*escalation;

	escalation = (EscalationNotify *)event.GetClientObject();
	createAnswerPolicy(escalation);
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
	wxGetApp().log(logEntry);
	wxGetApp().status(logEntry);
}

int
PolicyRuleSet::createAppPolicy(int insertBeforeId)
{
	int		 newId;
	wxCommandEvent	 event(anEVT_LOAD_RULESET);
	struct apn_rule	*newAppRule;

	newId	   = -1;
	newAppRule = CALLOC_STRUCT(apn_rule);

	if (newAppRule == NULL) {
		return (-1);
	}

	newAppRule->type = APN_ALF;
	newAppRule->app = CALLOC_STRUCT(apn_app);
	if (newAppRule->app == NULL) {
		free(newAppRule);
		return (-1);
	}

	newAppRule->app->hashtype = APN_HASH_SHA256;

	if (apn_insert(ruleSet_, newAppRule, insertBeforeId) == 0) {
		newId = newAppRule->id;
		clean();
		create(ruleSet_);
		event.SetClientData((void*)this);
		wxGetApp().sendEvent(event);
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
	struct apn_alfrule		*newAlfRule;

	this->accept(seeker);
	if (! seeker.hasMatchingPolicy()) {
		return (-1);
	}

	newId = -1;
	rc = -1;
	newAlfRule = CALLOC_STRUCT(apn_alfrule);
	parentPolicy = seeker.getMatchingPolicy();

	if (newAlfRule == NULL) {
		return (-1);
	}

	newAlfRule->type = APN_ALF_FILTER;

	if (parentPolicy->IsKindOf(CLASSINFO(AppPolicy))) {
		rc = apn_add2app_alfrule(ruleSet_, newAlfRule, insertBeforeId);
	} else if (parentPolicy->IsKindOf(CLASSINFO(AlfPolicy))) {
		rc = apn_insert_alfrule(ruleSet_, newAlfRule, insertBeforeId);
	}

	if (rc == 0) {
		newId = newAlfRule->id;
		clean();
		create(ruleSet_);
		event.SetClientData((void*)this);
		wxGetApp().sendEvent(event);
	} else {
		free(newAlfRule);
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
	struct apn_sfsrule		*newSfsRule;

	this->accept(seeker);
	if (! seeker.hasMatchingPolicy()) {
		return (-1);
	}

	newId = -1;
	rc = -1;
	newSfsRule = CALLOC_STRUCT(apn_sfsrule);
	parentPolicy = seeker.getMatchingPolicy();

	if (newSfsRule == NULL) {
		return (-1);
	}

	newSfsRule->type = APN_SFS_CHECK;
	newSfsRule->rule.sfscheck.app = CALLOC_STRUCT(apn_app);
	if (newSfsRule->rule.sfscheck.app == NULL) {
		free(newSfsRule);
		return (-1);
	}

	newSfsRule->rule.sfscheck.app->hashtype = APN_HASH_SHA256;

	if (TAILQ_EMPTY(&(ruleSet_->sfs_queue))) {
		sfsRootRule = CALLOC_STRUCT(apn_rule);
		sfsRootRule->type = APN_SFS;
		apn_add_sfsrule(sfsRootRule, ruleSet_);
	}
	/* we assume sfs_queue will contain only one element */
	sfsRootRule = TAILQ_FIRST(&(ruleSet_->sfs_queue));

	if (parentPolicy->IsKindOf(CLASSINFO(SfsPolicy))) {
		rc = apn_insert_sfsrule(ruleSet_, newSfsRule, insertBeforeId);
	} else {
		if (sfsRootRule->rule.sfs != NULL) {
			rc = apn_insert_sfsrule(ruleSet_, newSfsRule,
			    sfsRootRule->rule.sfs->id);
		} else {
			sfsRootRule->rule.sfs = newSfsRule;
			newSfsRule->id = ruleSet_->maxid;
			ruleSet_->maxid += 1;
			rc = 0;
		}
	}

	if (rc == 0) {
		newId = newSfsRule->id;
		clean();
		create(ruleSet_);
		event.SetClientData((void*)this);
		wxGetApp().sendEvent(event);
	} else {
		free(newSfsRule);
	}

	return (newId);
}

int
PolicyRuleSet::createVarPolicy(int insertBeforeId)
{
	/* XXX ch: currently no variables are supported */
	return (-1);
}

void
PolicyRuleSet::clearModified(void)
{
	RuleEditorChecksumVisitor	seeker(0);

	this->accept(seeker);
}

bool
PolicyRuleSet::findMismatchHash(void)
{
	RuleEditorChecksumVisitor	seeker;

	this->accept(seeker);

	return (seeker.hasMismatch());

}
