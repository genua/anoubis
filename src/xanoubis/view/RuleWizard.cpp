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
#include "RuleWizardAlfOverwritePage.h"
#include "RuleWizardAlfClientPage.h"
#include "RuleWizardAlfClientPortsPage.h"
#include "RuleWizardSandboxPage.h"
#include "RuleWizardSandboxOverwritePage.h"
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

	/* Program */
	CREATE_PAGE(pages_, PAGE_PROGRAM, RuleWizardProgramPage, history_);

	/* Context */
	CREATE_PAGE(pages_, PAGE_CTX, RuleWizardContextPage, history_);
	CREATE_PAGE(pages_, PAGE_CTX_EXCEPT, RuleWizardContextExceptionPage,
	    history_);

	/* ALF */
	CREATE_PAGE(pages_, PAGE_ALF_OVERWRITE, RuleWizardAlfOverwritePage,
	    history_);
	CREATE_PAGE(pages_, PAGE_ALF_CLIENT, RuleWizardAlfClientPage, history_);
	CREATE_PAGE(pages_, PAGE_ALF_CLIENT_PORTS, RuleWizardAlfClientPortsPage,
	    history_);

	/* Sandbox */
	CREATE_PAGE(pages_, PAGE_SB, RuleWizardSandboxPage, history_);
	CREATE_PAGE(pages_, PAGE_SB_OVERWRITE, RuleWizardSandboxOverwritePage,
	    history_);

	/* Final */
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
		if (!history_.haveContextPolicy()) {
			nextPage = PAGE_CTX;
			break;
		}
		if (history_.haveAlfPolicy()) {
			nextPage = PAGE_ALF_OVERWRITE;
		} else {
			nextPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_CTX:
		if (history_.haveContextException()) {
			nextPage = PAGE_CTX_EXCEPT;
			break;
		}
		if (history_.haveAlfPolicy()) {
			nextPage = PAGE_ALF_OVERWRITE;
		} else {
			nextPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_CTX_EXCEPT:
		if (history_.haveAlfPolicy()) {
			nextPage = PAGE_ALF_OVERWRITE;
		} else {
			nextPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_ALF_OVERWRITE:
		if (history_.shallOverwriteAlfPolicy() !=
		    RuleWizardHistory::OVERWRITE_NO) {
			nextPage = PAGE_ALF_CLIENT;
			break;
		}
		if (history_.haveSandboxPolicy()) {
			nextPage = PAGE_SB_OVERWRITE;
		} else {
			nextPage = PAGE_SB;
		}
		break;
	case PAGE_ALF_CLIENT:
		if (history_.getAlfClientPermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			nextPage = PAGE_ALF_CLIENT_PORTS;
			break;
		}
		/* XXX ch: ALF server pages still pending */
		if (history_.haveSandboxPolicy()) {
			nextPage = PAGE_SB_OVERWRITE;
		} else {
			nextPage = PAGE_SB;
		}
		break;
	case PAGE_ALF_CLIENT_PORTS:
		/* XXX ch: ALF server pages still pending */
		if (history_.haveSandboxPolicy()) {
			nextPage = PAGE_SB_OVERWRITE;
		} else {
			nextPage = PAGE_SB;
		}
		break;
	case PAGE_SB:
		if (history_.haveSandbox() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			//nextPage = PAGE_SB_READ;
			//break;
		}
		nextPage = PAGE_FINAL;
		break;
	case PAGE_SB_OVERWRITE:
		if (history_.shallOverwriteSandboxPolicy() !=
		    RuleWizardHistory::OVERWRITE_NO) {
			//nextPage = PAGE_SB_READ;
			//break;
		}
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
	case PAGE_ALF_OVERWRITE:
		if (!history_.haveContextPolicy()) {
			previousPage = PAGE_PROGRAM;
			break;
		}
		if (history_.haveContextException()) {
			previousPage = PAGE_CTX_EXCEPT;
		} else {
			previousPage = PAGE_CTX;
		}
		break;
	case PAGE_ALF_CLIENT:
		if (history_.haveAlfPolicy()) {
			previousPage = PAGE_ALF_OVERWRITE;
			break;
		}
		if (!history_.haveContextPolicy()) {
			previousPage = PAGE_PROGRAM;
			break;
		}
		if (history_.haveContextException()) {
			previousPage = PAGE_CTX_EXCEPT;
		} else {
			previousPage = PAGE_CTX;
		}
		break;
	case PAGE_ALF_CLIENT_PORTS:
		previousPage = PAGE_ALF_CLIENT;
		break;
	case PAGE_SB:
		/* XXX ch: ALF server pages still pending */
		if (history_.getAlfClientPermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			previousPage = PAGE_ALF_CLIENT_PORTS;
		} else {
			previousPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_SB_OVERWRITE:
		/* XXX ch: ALF server pages still pending */
		if (history_.getAlfClientPermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			previousPage = PAGE_ALF_CLIENT_PORTS;
		} else {
			previousPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_FINAL:
		if (history_.haveSandboxPolicy()) {
			previousPage = PAGE_SB_OVERWRITE;
		} else {
			previousPage = PAGE_SB;
		}
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

	if (!history_.haveContextPolicy()) {
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
	ctxApp->setHashValueString(history_.getChecksum(), 0);

	if (!history_.isSameContext()) {
		/* Create 'context new any' */
		ctxFilter = new ContextFilterPolicy(ctxApp);
		if (ctxFilter != NULL) {
			ctxFilter->setContextTypeNo(APN_CTX_NEW);
			ctxApp->prependFilterPolicy(ctxFilter);
			if (history_.haveContextException()) {
				ctxFilter->setBinaryList(
				    history_.getContextExceptionList());
			}
		}
	}
	/* else: empty context block */

	return;
}

void
RuleWizard::createAlfPolicy(PolicyRuleSet *ruleSet) const
{
	wxArrayString			 list;
	AlfAppPolicy			*alfApp;
	AlfCapabilityFilterPolicy	*alfCap;
	AlfFilterPolicy			*alfFilter;

	alfApp	  = NULL;
	alfCap	  = NULL;
	alfFilter = NULL;

	if (ruleSet == NULL) {
		return;
	}

	if (history_.haveAlfPolicy()) {
		alfApp = ruleSet->searchAlfAppPolicy(history_.getProgram());
		if (history_.shallOverwriteAlfPolicy() ==
		    RuleWizardHistory::OVERWRITE_NO) {
			return;
		}
		/*
		 * We just need to remove the filter, but it's easier to
		 * remove all and re-create the app policy.
		 */
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
		alfApp->setHashValueString(history_.getChecksum(), 0);
	}

	switch (history_.getAlfClientPermission()) {
	case RuleWizardHistory::PERM_ALLOW_ALL:
		alfFilter = new AlfFilterPolicy(alfApp);
		alfApp->prependFilterPolicy(alfFilter);
		alfFilter->setActionNo(APN_ACTION_ALLOW);
		return;
		break;
	case RuleWizardHistory::PERM_DENY_ALL:
		alfFilter = new AlfFilterPolicy(alfApp);
		alfApp->prependFilterPolicy(alfFilter);
		alfFilter->setActionNo(APN_ACTION_DENY);
		return;
		break;
	case RuleWizardHistory::PERM_RESTRICT_DEFAULT:
		//XXX ch: load defaults
		break;
	default:
		break;
	}

	if (history_.getAlfClientAsk()) {
		alfFilter = new AlfFilterPolicy(alfApp);
		alfApp->prependFilterPolicy(alfFilter);
		alfFilter->setActionNo(APN_ACTION_ASK);
	}

	if (history_.getAlfClientRaw()) {
		alfCap = new AlfCapabilityFilterPolicy(alfApp);
		alfApp->prependFilterPolicy(alfCap);
		alfCap->setCapabilityTypeNo(APN_ALF_CAPRAW);
	}

	list = history_.getAlfClientPortList();
	for (size_t i=0; i<list.GetCount(); i=i+3) {
		alfFilter = new AlfFilterPolicy(alfApp);
		alfApp->prependFilterPolicy(alfFilter);
		alfFilter->setActionNo(APN_ACTION_ALLOW);
		alfFilter->setDirectionNo(APN_CONNECT);
		if (list.Item(i+2) == wxT("tcp")) {
			alfFilter->setProtocol(IPPROTO_TCP);
		} else {
			alfFilter->setProtocol(IPPROTO_UDP);
		}
		alfFilter->setToPortName(list.Item(i+1));
	}
}
