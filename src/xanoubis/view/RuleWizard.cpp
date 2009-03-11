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
#include "RuleWizardSandboxReadPage.h"
#include "RuleWizardSandboxReadFilesPage.h"
#include "RuleWizardSandboxWritePage.h"
#include "RuleWizardSandboxWriteFilesPage.h"
#include "RuleWizardSandboxExecutePage.h"
#include "RuleWizardSandboxExecuteFilesPage.h"
#include "AppPolicy.h"
#include "AlfAppPolicy.h"
#include "AlfFilterPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "SbAppPolicy.h"
#include "SbAccessFilterPolicy.h"
#include "DefaultFilterPolicy.h"
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
 	CREATE_PAGE(pages_, PAGE_SB_READ, RuleWizardSandboxReadPage, history_);
 	CREATE_PAGE(pages_, PAGE_SB_READ_FILES, RuleWizardSandboxReadFilesPage,
	    history_);
	CREATE_PAGE(pages_, PAGE_SB_WRITE, RuleWizardSandboxWritePage,
	    history_);
	CREATE_PAGE(pages_, PAGE_SB_WRITE_FILES, RuleWizardSandboxWriteFilesPage,
	    history_);
	CREATE_PAGE(pages_, PAGE_SB_EXECUTE, RuleWizardSandboxExecutePage,
	    history_);
	CREATE_PAGE(pages_, PAGE_SB_EXECUTE_FILES,
	    RuleWizardSandboxExecuteFilesPage, history_);

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
		} else if (history_.haveAlfPolicy()) {
			nextPage = PAGE_ALF_OVERWRITE;
		} else {
			nextPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_CTX:
		if (history_.haveContextException()) {
			nextPage = PAGE_CTX_EXCEPT;
		} else if (history_.haveAlfPolicy()) {
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
		} else if (history_.haveSandboxPolicy()) {
			nextPage = PAGE_SB_OVERWRITE;
		} else {
			nextPage = PAGE_SB;
		}
		break;
	case PAGE_ALF_CLIENT:
		/* XXX ch: ALF server pages still pending */
		if (history_.getAlfClientPermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			nextPage = PAGE_ALF_CLIENT_PORTS;
		} else if (history_.haveSandboxPolicy()) {
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
			nextPage = PAGE_SB_READ;
		} else {
			nextPage = PAGE_FINAL;
		}
		break;
	case PAGE_SB_OVERWRITE:
		if (history_.shallOverwriteSandboxPolicy() !=
		    RuleWizardHistory::OVERWRITE_NO) {
			nextPage = PAGE_SB_READ;
		} else {
			nextPage = PAGE_FINAL;
		}
		break;
	case PAGE_SB_READ:
		if (history_.getSandboxReadPermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			nextPage = PAGE_SB_READ_FILES;
		} else {
			nextPage = PAGE_SB_WRITE;
		}
		break;
	case PAGE_SB_READ_FILES:
		nextPage = PAGE_SB_WRITE;
		break;
	case PAGE_SB_WRITE:
		if (history_.getSandboxWritePermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			nextPage = PAGE_SB_WRITE_FILES;
		} else {
			nextPage = PAGE_SB_EXECUTE;
		}
		break;		
	case PAGE_SB_WRITE_FILES:
		nextPage = PAGE_SB_EXECUTE;
		break;
	case PAGE_SB_EXECUTE:
		if (history_.getSandboxExecutePermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			nextPage = PAGE_SB_EXECUTE_FILES;
		} else {
			nextPage = PAGE_FINAL;
		}
		break;
	case PAGE_SB_EXECUTE_FILES:
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
	enum wizardPages				previousPage;
	enum RuleWizardHistory::overwriteAnswer		overwrite;

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
	case PAGE_ALF_CLIENT:
		if (history_.haveAlfPolicy()) {
			previousPage = PAGE_ALF_OVERWRITE;
			break;
		}
		/* FALLTHROUGH */
	case PAGE_ALF_OVERWRITE:
		if (!history_.haveContextPolicy()) {
			previousPage = PAGE_PROGRAM;
		} else if (history_.haveContextException()) {
			previousPage = PAGE_CTX_EXCEPT;
		} else {
			previousPage = PAGE_CTX;
		}
		break;
	case PAGE_ALF_CLIENT_PORTS:
		previousPage = PAGE_ALF_CLIENT;
		break;
	case PAGE_SB:
	case PAGE_SB_OVERWRITE:
		/* XXX ch: ALF server pages still pending */
		overwrite = RuleWizardHistory::OVERWRITE_YES;
		if (history_.haveAlfPolicy()) {
			overwrite = history_.shallOverwriteAlfPolicy();
		}
		if (overwrite == RuleWizardHistory::OVERWRITE_NO) {
			previousPage = PAGE_ALF_OVERWRITE;
		} else if (history_.getAlfClientPermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			previousPage = PAGE_ALF_CLIENT_PORTS;
		} else {
			previousPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_SB_READ:
		if (history_.haveSandboxPolicy()) {
			previousPage = PAGE_SB_OVERWRITE;
		} else {
			previousPage = PAGE_SB;
		}
		break;
	case PAGE_SB_READ_FILES:
		previousPage = PAGE_SB_READ;
		break;
	case PAGE_SB_WRITE:
		if (history_.getSandboxReadPermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			previousPage = PAGE_SB_READ_FILES;
		} else {
			previousPage = PAGE_SB_READ;
		}
		break;
	case PAGE_SB_WRITE_FILES:
		previousPage = PAGE_SB_WRITE;
		break;
	case PAGE_SB_EXECUTE:
		if (history_.getSandboxWritePermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			previousPage = PAGE_SB_WRITE_FILES;
		} else {
			previousPage = PAGE_SB_WRITE;
		}
		break;
	case PAGE_SB_EXECUTE_FILES:
		previousPage = PAGE_SB_EXECUTE;
		break;
	case PAGE_FINAL:
		overwrite = RuleWizardHistory::OVERWRITE_YES;
		if (history_.haveSandboxPolicy()) {
			overwrite = history_.shallOverwriteSandboxPolicy();
		}
		if (overwrite == RuleWizardHistory::OVERWRITE_NO) {
			previousPage = PAGE_SB_OVERWRITE;
		} else if (history_.haveSandbox() !=
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			previousPage = PAGE_SB;
		} else if (history_.getSandboxExecutePermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			previousPage = PAGE_SB_EXECUTE_FILES;
		} else {
			previousPage = PAGE_SB_EXECUTE;
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
	wxCommandEvent		 newRuleSetEvent(anEVT_LOAD_RULESET);
	struct iovec		 iv;
	struct apn_ruleset	*rs;

	ProfileCtrl	*profileCtrl;
	PolicyRuleSet	*ruleSet;

	profileCtrl = ProfileCtrl::getInstance();
	/* XXX CEH: Might want to create Admin Policies as well. */
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
	createSandboxPolicy(ruleSet);
	newRuleSetEvent.SetInt(ruleSet->getRuleSetId());
	newRuleSetEvent.SetExtraLong(ruleSet->getRuleSetId());
	ruleSet->unlock();

	wxPostEvent(AnEvents::getInstance(), newRuleSetEvent);

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
		ctxApp->prependFilterPolicy(ctxFilter);
		ctxFilter->setContextTypeNo(APN_CTX_NEW);
	} else {
		if (history_.haveContextException()) {
			ctxFilter = new ContextFilterPolicy(ctxApp);
			ctxApp->prependFilterPolicy(ctxFilter);
			ctxFilter->setContextTypeNo(APN_CTX_NEW);
			ctxFilter->setBinaryList(
			    history_.getContextExceptionList());
		}
	}

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
		/*
		 * XXX CEH: In case of multiple applications we should
		 * XXX CEH: only through away the application we are
		 * XXX CEH: creating policies for.
		 */
		alfApp->remove();
		alfApp = NULL;
	}

	alfApp = new AlfAppPolicy(ruleSet);
	ruleSet->prependAppPolicy(alfApp);
	alfApp->addBinary(history_.getProgram());
	alfApp->setHashValueString(history_.getChecksum(), 0);

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
		alfFilter->setProtocol(IPPROTO_TCP);

		alfFilter = new AlfFilterPolicy(alfApp);
		alfApp->prependFilterPolicy(alfFilter);
		alfFilter->setActionNo(APN_ACTION_ASK);
		alfFilter->setProtocol(IPPROTO_UDP);
	}

	if (history_.getAlfClientRaw()) {
		alfCap = new AlfCapabilityFilterPolicy(alfApp);
		alfApp->prependFilterPolicy(alfCap);
		alfCap->setCapabilityTypeNo(APN_ALF_CAPRAW);
	}

	list = history_.getAlfClientPortList();
	createAlfPortList(alfApp, list, APN_CONNECT);
}

void
RuleWizard::createSandboxPolicy(PolicyRuleSet *ruleSet) const
{
	wxArrayString		 list;
	SbAppPolicy		*sbApp;
	SbAccessFilterPolicy	*sbFilter;
	DefaultFilterPolicy	*dflFilter;

	sbApp	 = NULL;
	sbFilter = NULL;
	dflFilter  = NULL;

	if (history_.haveSandboxPolicy()) {
		sbApp = ruleSet->searchSandboxAppPolicy(history_.getProgram());
		if (history_.shallOverwriteSandboxPolicy() ==
		    RuleWizardHistory::OVERWRITE_NO) {
			return;
		}
		/*
		 * We just need to remove the filter, but it's easier to
		 * remove all and re-create the app policy.
		 */
		/*
		 * XXX CEH: In case of multiple applications we should
		 * XXX CEH: only through away the application we are
		 * XXX CEH: creating policies for.
		 */
		sbApp->remove();
		sbApp = NULL;
	}

	/* Create new app block. */
	sbApp = new SbAppPolicy(ruleSet);
	ruleSet->prependAppPolicy(sbApp);
	sbApp->addBinary(history_.getProgram());
	sbApp->setHashValueString(history_.getChecksum(), 0);

	createSbPermission(sbApp, APN_SBA_READ);
	createSbPermission(sbApp, APN_SBA_WRITE);
	createSbPermission(sbApp, APN_SBA_EXEC);
}

void
RuleWizard::createAlfPortList(AlfAppPolicy *app, const wxArrayString & list,
    int direction) const
{
	AlfFilterPolicy	*filter;

	for (size_t i=0; i<list.GetCount(); i=i+3) {
		filter = new AlfFilterPolicy(app);
		app->prependFilterPolicy(filter);

		filter->setActionNo(APN_ACTION_ALLOW);
		filter->setDirectionNo(direction);

		if (list.Item(i+2) == wxT("tcp")) {
			filter->setProtocol(IPPROTO_TCP);
		} else {
			filter->setProtocol(IPPROTO_UDP);
		}
		filter->setToPortName(list.Item(i+1));
	}
}

void
RuleWizard::createSbFileList(SbAppPolicy *app, const wxArrayString & list,
    int permission) const
{
	SbAccessFilterPolicy	*filter;

	for (size_t i=0; i<list.GetCount(); i++) {
		filter = new SbAccessFilterPolicy(app);
		app->prependFilterPolicy(filter);

		filter->setActionNo(APN_ACTION_ALLOW);
		filter->setPath(list.Item(i));
		filter->setAccessMask(permission);
	}
}

void
RuleWizard::createSbPermission(SbAppPolicy *app, int section) const
{
	bool					 ask;
	bool					 signature;
	RuleWizardHistory::permissionAnswer	 permission;
	wxArrayString				 list;
	SbAccessFilterPolicy			*filter;

	switch (section) {
	case APN_SBA_READ:
		ask = history_.getSandboxReadAsk();
		signature = history_.getSandboxReadValidSignature();
		permission = history_.getSandboxReadPermission();
		list = history_.getSandboxReadFileList();
		break;
	case APN_SBA_WRITE:
		ask = history_.getSandboxWriteAsk();
		signature = history_.getSandboxWriteValidSignature();
		permission = history_.getSandboxWritePermission();
		list = history_.getSandboxWriteFileList();
		break;
	case APN_SBA_EXEC:
		ask = history_.getSandboxExecuteAsk();
		signature = history_.getSandboxExecuteValidSignature();
		permission = history_.getSandboxExecutePermission();
		list = history_.getSandboxExecuteFileList();
		break;
	default:
		return;
	}

	filter = new SbAccessFilterPolicy(app);
	app->prependFilterPolicy(filter);

	filter->setPath(wxT("/"));
	filter->setAccessMask(section);
	if (ask) {
		filter->setActionNo(APN_ACTION_ASK);
	} else if (permission == RuleWizardHistory::PERM_ALLOW_ALL) {
		filter->setActionNo(APN_ACTION_ALLOW);
	} else {
		filter->setActionNo(APN_ACTION_DENY);
	}

	if (signature) {
		filter = new SbAccessFilterPolicy(app);
		app->prependFilterPolicy(filter);

		filter->setSubjectSelf(true);
		filter->setPath(wxT("/"));
		filter->setAccessMask(section);
		filter->setActionNo(APN_ACTION_ALLOW);

		filter = new SbAccessFilterPolicy(app);
		app->prependFilterPolicy(filter);

		filter->setSubjectSelf(false);
		filter->setPath(wxT("/"));
		filter->setAccessMask(section);
		filter->setActionNo(APN_ACTION_ALLOW);
	}

	switch (permission) {
	case RuleWizardHistory::PERM_RESTRICT_DEFAULT:
		// XXX ch: load defautls
		break;
	case RuleWizardHistory::PERM_RESTRICT_USER:
		createSbFileList(app, list, section);
		break;
	default:
		break;
	}
}
