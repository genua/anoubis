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
#include "PolicyCtrl.h"
#include "PolicyRuleSet.h"

#include "main.h"
#include "JobCtrl.h"

#define CREATE_PAGE(list, idx, name, history)				\
	do {								\
		list[idx] = new RuleWizardPage(this, idx);		\
		new name(list[idx], &history);				\
	} while (0)

RuleWizard::RuleWizard(wxWindow *parent)
{
	Create(parent, wxID_ANY, wxT("Rule Wizard"));

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
	CREATE_PAGE(pages_, PAGE_SB_WRITE_FILES,
	    RuleWizardSandboxWriteFilesPage, history_);
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
		if (history_.haveContextPolicy()) {
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
	wxString		message;
	struct iovec		 iv;
	struct apn_ruleset	*rs = NULL;

	PolicyCtrl	*policyCtrl;
	PolicyRuleSet	*ruleSet;

	PolicyCtrl::PolicyResult	polRes;

	policyCtrl = PolicyCtrl::getInstance();
	/* XXX CEH: Might want to create Admin Policies as well. */
	ruleSet = policyCtrl->getRuleSet(policyCtrl->getUserId());
	if (ruleSet == NULL) {
		/* As we have no ruleset, create empty ruleset. */
		iv.iov_base = (void *)" ";
		iv.iov_len  = strlen((char *)iv.iov_base) - 1;
		if (apn_parse_iovec("<iov>", &iv, 1, &rs, 0) == 0) {
			ruleSet = new PolicyRuleSet(1, geteuid(), rs);
		} else {
			apn_free_ruleset(rs);
		}
		if (ruleSet != NULL) {
			ruleSet->lock();
			policyCtrl->importPolicy(ruleSet);
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
	if (history_.haveSandbox() != RuleWizardHistory::PERM_NONE)
		createSandboxPolicy(ruleSet);

	newRuleSetEvent.SetInt(ruleSet->getRuleSetId());
	newRuleSetEvent.SetExtraLong(ruleSet->getRuleSetId());
	ruleSet->unlock();

	if(history_.shallActivatePolicy()) {
		polRes = policyCtrl->sendToDaemon(ruleSet->getRuleSetId());
		switch (polRes) {
		case PolicyCtrl::RESULT_POL_WRONG_PASS:
			message = _("The entered password is incorrect.");
			anMessageBox(message, _("Key Load Error"),
			    wxOK|wxICON_ERROR, this);
			wxGetApp().status(wxT("Wrong password!"));
			break;
		case PolicyCtrl::RESULT_POL_ERR:
			message = _("An error occured while sending admin"
			    " policy to the daemon.");
			anMessageBox(message, _("Policy Load Error"),
				wxOK|wxICON_ERROR, this);
			wxGetApp().status(wxT("Error while sending"
			    " admin policy to daemon."));
			break;
		case PolicyCtrl::RESULT_POL_ABORT:
		case PolicyCtrl::RESULT_POL_OK:
			wxGetApp().status(_("policy sent to daemon"));
			break;
		}

	}

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
	if (history_.getChecksumType() == APN_CS_NONE) {
		ctxApp->setSubjectNone(0);
	} else {
		ctxApp->setSubjectSelf(0, false);
	}

	ctxApp->setFlag(APN_RULE_NOSFS, history_.isSfsDisabled());

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
			for (unsigned int i=0;
			    i<history_.getContextExceptionCount(); ++i) {
				ctxFilter->addBinary(
				    history_.getContextExceptionBinary(i));
				ctxFilter->setSubjectNone(i);
			}
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
	DefaultFilterPolicy		*dflFilter;

	alfApp	  = NULL;
	alfCap	  = NULL;
	dflFilter = NULL;

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
		if (alfApp->getBinaryCount() > 1) {
			alfApp->removeBinary(history_.getProgram());
		} else {
			alfApp->remove();
		}
	}

	/* Create new application bock */
	alfApp = new AlfAppPolicy(ruleSet);
	ruleSet->prependAppPolicy(alfApp);
	alfApp->addBinary(history_.getProgram());
	if (history_.getChecksumType() == APN_CS_NONE) {
		alfApp->setSubjectNone(0);
	} else {
		alfApp->setSubjectSelf(0, false);
	}

	dflFilter = new DefaultFilterPolicy(alfApp);
	alfApp->prependFilterPolicy(dflFilter);

	switch (history_.getAlfClientPermission()) {
	case RuleWizardHistory::PERM_ALLOW_ALL:
		dflFilter->setActionNo(APN_ACTION_ALLOW);
		break;
	case RuleWizardHistory::PERM_DENY_ALL:
		dflFilter->setActionNo(APN_ACTION_DENY);
		dflFilter->setLogNo(APN_LOG_NORMAL);
		break;
	case RuleWizardHistory::PERM_RESTRICT_DEFAULT:
		dflFilter->setActionNo(APN_ACTION_ASK);
		list = history_.getAlfDefaults();
		createAlfPortList(alfApp, list, APN_CONNECT);
		break;
	default:
		if (history_.getAlfClientAsk()) {
			dflFilter->setActionNo(APN_ACTION_ASK);
		} else {
			dflFilter->setActionNo(APN_ACTION_DENY);
			dflFilter->setLogNo(APN_LOG_NORMAL);
		}

		if (history_.getAlfClientRaw()) {
			alfCap = new AlfCapabilityFilterPolicy(alfApp);
			alfApp->prependFilterPolicy(alfCap);
			alfCap->setCapabilityTypeNo(APN_ALF_CAPRAW);
		}

		list = history_.getAlfClientPortList();
		createAlfPortList(alfApp, list, APN_CONNECT);
		break;
	}
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
		if (sbApp->getBinaryCount() > 1) {
			sbApp->removeBinary(history_.getProgram());
		} else {
			sbApp->remove();
		}
	}
	/* Create new app block. */
	sbApp = new SbAppPolicy(ruleSet);
	ruleSet->prependAppPolicy(sbApp);
	sbApp->addBinary(history_.getProgram());
	if (history_.getChecksumType() == APN_CS_NONE) {
		sbApp->setSubjectNone(0);
	} else {
		sbApp->setSubjectSelf(0, false);
	}

	/* Create a default ASK policy just to be sure. */
	dflFilter = new DefaultFilterPolicy(sbApp);
	sbApp->prependFilterPolicy(dflFilter);
	dflFilter->setActionNo(APN_ACTION_ASK);

	createSbPermission(sbApp, APN_SBA_EXEC);
	createSbPermission(sbApp, APN_SBA_WRITE);
	createSbPermission(sbApp, APN_SBA_READ);
}

void
RuleWizard::createAlfPortList(AlfAppPolicy *app, const wxArrayString & list,
    int direction) const
{
	AlfFilterPolicy	*filter;
	static int	 dirs[2] = { APN_CONNECT, APN_ACCEPT };

	switch (direction) {
	case APN_SEND:
		direction = APN_CONNECT;
		break;
	case APN_RECEIVE:
		direction = APN_ACCEPT;
		break;
	}
	for (size_t i=0; i<list.GetCount(); i=i+3) {
		int proto = IPPROTO_UDP;

		if (list.Item(i+2) == wxT("tcp")) {
			proto = IPPROTO_TCP;
		}
		/*
		 * For UDP generate rules for both directions.
		 */
		for (int x=0; x<2; ++x) {
			if (proto == IPPROTO_TCP && dirs[x] != direction)
				continue;
			filter = new AlfFilterPolicy(app);
			app->prependFilterPolicy(filter);

			filter->setActionNo(APN_ACTION_ALLOW);
			filter->setDirectionNo(dirs[x]);
			filter->setProtocol(proto);
			/*
			 * For TCP-accept we want to allow connections from
			 * any remote port to a specific local port, thus
			 * set to ToPortName in this case, too.
			 */
			if (dirs[x] == APN_CONNECT || proto == IPPROTO_TCP) {
				filter->setToPortName(list.Item(i+1));
			} else {
				filter->setFromPortName(list.Item(i+1));
			}
		}
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
		filter->setLogNo(APN_LOG_NONE);
		filter->setPath(list.Item(i));
		filter->setAccessMask(permission);
	}
}

void
RuleWizard::createSbDefaultFileList(SbAppPolicy *app, int section) const
{
	wxArrayString		list;

	switch(section) {
	case APN_SBA_READ:
		list = history_.getSandboxDefaults(wxT("r"));
		break;
	case APN_SBA_WRITE:
		list = history_.getSandboxDefaults(wxT("w"));
		break;
	case APN_SBA_EXEC:
		list = history_.getSandboxDefaults(wxT("x"));
		break;
	}
	createSbFileList(app, list, section);
}

void
RuleWizard::createSbPermissionRoot(SbAppPolicy *app, int section,
    int action) const
{
	SbAccessFilterPolicy		*filter;

	filter = new SbAccessFilterPolicy(app);
	app->prependFilterPolicy(filter);
	filter->setPath(wxT("/"));
	filter->setAccessMask(section);
	filter->setActionNo(action);
	if (action == APN_ACTION_DENY) {
		filter->setLogNo(APN_LOG_NORMAL);
	} else {
		filter->setLogNo(APN_LOG_NONE);
	}
}

void
RuleWizard::createSbPermission(SbAppPolicy *app, int section) const
{
	bool					 ask;
	bool					 signature;
	RuleWizardHistory::permissionAnswer	 permission;
	wxArrayString				 list;

	/* First check the global permissions. */
	permission = history_.haveSandbox();
	switch (permission) {
	case RuleWizardHistory::PERM_RESTRICT_DEFAULT:
		createSbDefaultFileList(app, section);
		return;
	case RuleWizardHistory::PERM_RESTRICT_USER:
		break;
	case RuleWizardHistory::PERM_ALLOW_ALL:
		/* Should not happen. */
		createSbPermissionRoot(app, section, APN_ACTION_ALLOW);
		return;
	case RuleWizardHistory::PERM_NONE:
		/* Handled by the caller. */
		return;
	case RuleWizardHistory::PERM_DENY_ALL:
		/* Not allowed for Sandbox rules. */
		return;
	}

	/*
	 * At this point we know that the global permission choice is
	 * PERM_RESTRICT_USER. Look at the local permissions.
	 */
	switch (section) {
	case APN_SBA_READ:
		permission = history_.getSandboxReadPermission();
		break;
	case APN_SBA_WRITE:
		permission = history_.getSandboxWritePermission();
		break;
	case APN_SBA_EXEC:
		permission = history_.getSandboxExecutePermission();
		break;
	}

	switch (permission) {
	case RuleWizardHistory::PERM_ALLOW_ALL:
		createSbPermissionRoot(app, section, APN_ACTION_ALLOW);
		return;
	case RuleWizardHistory::PERM_RESTRICT_DEFAULT:
		createSbDefaultFileList(app, section);
		return;
	case RuleWizardHistory::PERM_RESTRICT_USER:
		break;
	case RuleWizardHistory::PERM_NONE:
	case RuleWizardHistory::PERM_DENY_ALL:
		/* Should not happen. Nothing to do. */
		return;
	}

	/*
	 * At this point both the global and the local permission are
	 * PERM_RESTRICT_USER. Generate rules accordingly.
	 */
	switch (section) {
	case APN_SBA_READ:
		ask = history_.getSandboxReadAsk();
		signature = history_.getSandboxReadValidSignature();
		list = history_.getSandboxReadFileList();
		break;
	case APN_SBA_WRITE:
		ask = history_.getSandboxWriteAsk();
		signature = history_.getSandboxWriteValidSignature();
		list = history_.getSandboxWriteFileList();
		break;
	case APN_SBA_EXEC:
		ask = history_.getSandboxExecuteAsk();
		signature = history_.getSandboxExecuteValidSignature();
		list = history_.getSandboxExecuteFileList();
		break;
	default:
		return;
	}

	/* Create the rule for the root directory. */
	if (ask) {
		createSbPermissionRoot(app, section, APN_ACTION_ASK);
	} else {
		createSbPermissionRoot(app, section, APN_ACTION_DENY);
	}
	/* Create special rules for the files in the file list. */
	createSbFileList(app, list, section);

	/* Finally put signature rules first if those were requested. */
	if (signature) {
		SbAccessFilterPolicy	*filter;
		int			 action = APN_ACTION_ALLOW;
		int			 log = APN_LOG_NONE;

		if (section == APN_SBA_WRITE) {
			action = APN_ACTION_DENY;
			log = APN_LOG_NORMAL;
		}
		filter = new SbAccessFilterPolicy(app);
		app->prependFilterPolicy(filter);

		filter->setSubjectSelf(true);
		filter->setPath(wxT("/"));
		filter->setAccessMask(section);
		filter->setActionNo(action);
		filter->setLogNo(log);

		filter = new SbAccessFilterPolicy(app);
		app->prependFilterPolicy(filter);

		filter->setSubjectSelf(false);
		filter->setPath(wxT("/"));
		filter->setAccessMask(section);
		filter->setActionNo(action);
		filter->setLogNo(log);
	}
}
