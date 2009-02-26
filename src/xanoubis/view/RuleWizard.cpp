/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#include "RuleWizard.h"

#include "RuleWizardPage.h"
#include "RuleWizardProgramPage.h"
#include "RuleWizardContextPage.h"
#include "RuleWizardContextExceptionPage.h"
#include "RuleWizardAlfKeepPolicyPage.h"
#include "RuleWizardAlfClientPage.h"
#include "RuleWizardAlfClientPortsPage.h"
#include "RuleWizardFinalPage.h"

#include "AppPolicy.h"
#include "ProfileCtrl.h"
#include "PolicyRuleSet.h"

#define CREATE_PAGE(list, idx, name, history)				\
	do {								\
		list[idx] = new RuleWizardPage(this, idx);		\
		new name(list[idx], &history);				\
	} while (0)

RuleWizard::RuleWizard(void)
{
	Create(NULL, wxID_ANY, wxT("Rule Wizard"));

	CREATE_PAGE(pages_, PAGE_PROGRAM, RuleWizardProgramPage, history_);
	CREATE_PAGE(pages_, PAGE_CTX, RuleWizardContextPage, history_);
	CREATE_PAGE(pages_, PAGE_CTX_EXCEPT, RuleWizardContextExceptionPage,
	    history_);
	CREATE_PAGE(pages_, PAGE_ALF_KEEP_POLICY, RuleWizardAlfKeepPolicyPage,
	    history_);
	CREATE_PAGE(pages_, PAGE_ALF_CLIENT, RuleWizardAlfClientPage, history_);
	CREATE_PAGE(pages_, PAGE_ALF_CLIENT_PORTS, RuleWizardAlfClientPortsPage,
	    history_);
	CREATE_PAGE(pages_, PAGE_FINAL, RuleWizardFinalPage, history_);

	GetPageAreaSizer()->Add(pages_[PAGE_PROGRAM]);

	Connect(wxEVT_WIZARD_FINISHED,
	    wxWizardEventHandler(RuleWizard::onWizardFinished),
	    NULL, this);
}

wxWizardPage *
RuleWizard::getPage(enum wizardPages index)
{
	wxWizardPage *page;

	page = NULL;

	if ((PAGE_PROGRAM <= index) && (index < PAGE_EOL)) {
		page = pages_[index];
	}

	return (page);
}

RuleWizard::wizardPages
RuleWizard::forwardTransition(enum wizardPages pageNo) const
{
	enum wizardPages nextPage;

	switch (pageNo) {
	case PAGE_PROGRAM:
		nextPage = PAGE_CTX;
		break;
	case PAGE_CTX:
		if (history_.getCtxException()) {
			nextPage = PAGE_CTX_EXCEPT;
		} else {
			if (history_.getAlfHavePolicy()) {
				nextPage = PAGE_ALF_KEEP_POLICY;
			} else {
				nextPage = PAGE_ALF_CLIENT;
			}
		}
		break;
	case PAGE_CTX_EXCEPT:
		if (history_.getAlfHavePolicy()) {
			nextPage = PAGE_ALF_KEEP_POLICY;
		} else {
			nextPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_ALF_KEEP_POLICY:
		nextPage = PAGE_ALF_CLIENT;
		break;
	case PAGE_ALF_CLIENT:
		nextPage = PAGE_ALF_CLIENT_PORTS;
		break;
	case PAGE_ALF_CLIENT_PORTS:
		nextPage = PAGE_FINAL;
		break;
	case PAGE_FINAL:
		/* FALLTHROUGH */
	default:
		nextPage = PAGE_EOL;
		break;
	}

	return (nextPage);
}

RuleWizard::wizardPages
RuleWizard::backwardTransition(enum wizardPages pageNo) const
{
	enum wizardPages previousPage;

	switch (pageNo) {
	case PAGE_PROGRAM:
		previousPage = PAGE_EOL;
		break;
	case PAGE_CTX:
		previousPage = PAGE_PROGRAM;
		break;
	case PAGE_CTX_EXCEPT:
		previousPage = PAGE_CTX;
		break;
	case PAGE_ALF_KEEP_POLICY:
		if (history_.getCtxException()) {
			previousPage = PAGE_CTX_EXCEPT;
		} else {
			previousPage = PAGE_CTX;
		}
		break;
	case PAGE_ALF_CLIENT:
		if (history_.getAlfHavePolicy()) {
			previousPage = PAGE_ALF_KEEP_POLICY;
		} else {
			if (history_.getCtxException()) {
				previousPage = PAGE_CTX_EXCEPT;
			} else {
				previousPage = PAGE_CTX;
			}
		}
		break;
	case PAGE_ALF_CLIENT_PORTS:
		previousPage = PAGE_ALF_CLIENT;
		break;
	case PAGE_FINAL:
		previousPage = PAGE_ALF_CLIENT_PORTS;
		break;
	default:
		/* XXX ch: does this make any sense? */
		previousPage = PAGE_PROGRAM;
		break;
	}

	return (previousPage);
}

void
RuleWizard::onWizardFinished(wxWizardEvent &)
{
	//wxCommandEvent	 newRuleSetEvent(anEVT_LOAD_RULESET);
	struct iovec		 iv;
	struct apn_ruleset	*rs;

	ProfileCtrl	*profileCtrl;
	PolicyRuleSet	*ruleSet;

	profileCtrl = ProfileCtrl::getInstance();
	ruleSet = profileCtrl->getRuleSet(profileCtrl->getUserId());
	if (ruleSet == NULL) {
		/* As we have no ruleset, create empty ruleset. */
		iv.iov_base = (void *)" ";
		iv.iov_len  = strlen((char *)iv.iov_base) - 1;
		if (apn_parse_iovec("<iov>", &iv, 1, &rs, 0) == 0) {
			ruleSet = new PolicyRuleSet(1, geteuid(), rs);
		}
		if (ruleSet != NULL) {
			ruleSet->lock();
			profileCtrl->importPolicy(ruleSet);
		} else {
			/* Ok, we have a serious problem here. */
			// XXX ch: error dlg missing
			return;
		}
	} else {
		ruleSet->lock();
	}

	if (!history_.getCtxHavePolicy()) {
		createContextPolicy(ruleSet);
	}

	createAlfPolicy(ruleSet);

	ruleSet->unlock();

	return;
}

void
RuleWizard::createContextPolicy(PolicyRuleSet *ruleSet) const
{
	ContextAppPolicy	*ctxApp;
	ContextFilterPolicy	*ctxFilter;

	if (ruleSet == NULL) {
		return;
	}

	ctxApp = new ContextAppPolicy(ruleSet);
	if (ctxApp == NULL) {
		return;
	}

	ruleSet->prependAppPolicy(ctxApp);
	ctxApp->addBinary(history_.getProgram());
	//XXX ch: set csum

	//XXX ch: contextExceptions?
	if (!history_.getCtxSame()) {
		/* Create 'context new any' */
		ctxFilter = new ContextFilterPolicy(ctxApp);
		if (ctxFilter != NULL) {
			ctxFilter->setContextTypeNo(APN_CTX_NEW);
			ctxApp->prependFilterPolicy(ctxFilter);
		}
	}
	/* else: empty context block */

	return;
}

void
RuleWizard::createAlfPolicy(PolicyRuleSet *ruleSet) const
{
	AlfAppPolicy	*alfApp;
	AlfFilterPolicy	*alfFilter;

	if (ruleSet == NULL) {
		return;
	}

	alfApp = NULL;
	if (history_.getAlfHavePolicy()) {
		alfApp = ruleSet->searchAlfAppPolicy(history_.getProgram());
	}
	if (!history_.getAlfKeepPolicy() && (alfApp != NULL)) {
		alfApp->remove();
		alfApp = NULL;
	}

	if (alfApp == NULL) {
		alfApp = new AlfAppPolicy(ruleSet);
		if (alfApp == NULL) {
			return;
		}
		ruleSet->prependAppPolicy(alfApp);
	}

	if (alfApp->isAnyBlock()) {
		alfApp->addBinary(history_.getProgram());
	}

	alfFilter = new AlfFilterPolicy(alfApp);
	if (alfFilter == NULL){
		/* Serious problem */
		return;
	}
	alfApp->prependFilterPolicy(alfFilter);

	switch (history_.getAlfClientPermission()) {
	case RuleWizardHistory::ALF_CLIENT_ALLOW_ALL:
		alfFilter->setActionNo(APN_ACTION_ALLOW);
		break;
	case RuleWizardHistory::ALF_CLIENT_DENY_ALL:
		alfFilter->setActionNo(APN_ACTION_DENY);
		break;
	default:
		break;
	}
}
