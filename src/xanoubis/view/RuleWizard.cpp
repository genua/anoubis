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

#include "AlfAppPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "AlfFilterPolicy.h"
#include "AnMessageDialog.h"
#include "AppPolicy.h"
#include "DefaultFilterPolicy.h"
#include "JobCtrl.h"
#include "MainUtils.h"
#include "PolicyCtrl.h"
#include "PolicyRuleSet.h"
#include "RuleWizard.h"
#include "RuleWizardAlfClientPage.h"
#include "RuleWizardAlfClientPortsPage.h"
#include "RuleWizardAlfOverwritePage.h"
#include "RuleWizardContextExceptionPage.h"
#include "RuleWizardContextPage.h"
#include "RuleWizardFinalPage.h"
#include "RuleWizardPage.h"
#include "RuleWizardProgramPage.h"
#include "RuleWizardSandboxExecuteFilesPage.h"
#include "RuleWizardSandboxExecutePage.h"
#include "RuleWizardSandboxOverwritePage.h"
#include "RuleWizardSandboxPage.h"
#include "RuleWizardSandboxReadFilesPage.h"
#include "RuleWizardSandboxReadPage.h"
#include "RuleWizardSandboxWriteFilesPage.h"
#include "RuleWizardSandboxWritePage.h"
#include "SbAccessFilterPolicy.h"
#include "SbAppPolicy.h"
#include "SbEntry.h"
#include "SbModel.h"
#include "Service.h"
#include "ServiceList.h"
#include "StringListModel.h"

#define CREATE_PAGE(list, idx, name, history)				\
	do {								\
		list[idx] = new RuleWizardPage(this, idx);		\
		new name(list[idx], history);				\
	} while (0)

RuleWizard::RuleWizard(wxWindow *parent)
{
	Create(parent, wxID_ANY, wxT("Rule Wizard"));

	history_ = new RuleWizardHistory();

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

RuleWizard::~RuleWizard(void)
{
	RuleWizardHistory::put(history_);
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
		if (!history_->haveContextPolicy()) {
			nextPage = PAGE_CTX;
		} else if (history_->haveAlfPolicy()) {
			nextPage = PAGE_ALF_OVERWRITE;
		} else {
			nextPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_CTX:
		if (history_->haveContextException()) {
			nextPage = PAGE_CTX_EXCEPT;
		} else if (history_->haveAlfPolicy()) {
			nextPage = PAGE_ALF_OVERWRITE;
		} else {
			nextPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_CTX_EXCEPT:
		if (history_->haveAlfPolicy()) {
			nextPage = PAGE_ALF_OVERWRITE;
		} else {
			nextPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_ALF_OVERWRITE:
		if (history_->shallOverwriteAlfPolicy() !=
		    RuleWizardHistory::OVERWRITE_NO) {
			nextPage = PAGE_ALF_CLIENT;
		} else if (history_->haveSandboxPolicy()) {
			nextPage = PAGE_SB_OVERWRITE;
		} else {
			nextPage = PAGE_SB;
		}
		break;
	case PAGE_ALF_CLIENT:
		/* XXX ch: ALF server pages still pending */
		if (history_->getAlfClientPermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			nextPage = PAGE_ALF_CLIENT_PORTS;
		} else if (history_->haveSandboxPolicy()) {
			nextPage = PAGE_SB_OVERWRITE;
		} else {
			nextPage = PAGE_SB;
		}
		break;
	case PAGE_ALF_CLIENT_PORTS:
		/* XXX ch: ALF server pages still pending */
		if (history_->haveSandboxPolicy()) {
			nextPage = PAGE_SB_OVERWRITE;
		} else {
			nextPage = PAGE_SB;
		}
		break;
	case PAGE_SB:
		if (history_->haveSandbox() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			nextPage = PAGE_SB_READ;
		} else {
			nextPage = PAGE_FINAL;
		}
		break;
	case PAGE_SB_OVERWRITE:
		if (history_->shallOverwriteSandboxPolicy() !=
		    RuleWizardHistory::OVERWRITE_NO) {
			nextPage = PAGE_SB_READ;
		} else {
			nextPage = PAGE_FINAL;
		}
		break;
	case PAGE_SB_READ:
		if (history_->getSandboxPermission(SbEntry::READ) ==
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
		if (history_->getSandboxPermission(SbEntry::WRITE) ==
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
		if (history_->getSandboxPermission(SbEntry::EXECUTE) ==
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
		if (history_->haveAlfPolicy()) {
			previousPage = PAGE_ALF_OVERWRITE;
			break;
		}
		/* FALLTHROUGH */
	case PAGE_ALF_OVERWRITE:
		if (history_->haveContextPolicy()) {
			previousPage = PAGE_PROGRAM;
		} else if (history_->haveContextException()) {
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
		if (history_->haveAlfPolicy()) {
			overwrite = history_->shallOverwriteAlfPolicy();
		}
		if (overwrite == RuleWizardHistory::OVERWRITE_NO) {
			previousPage = PAGE_ALF_OVERWRITE;
		} else if (history_->getAlfClientPermission() ==
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			previousPage = PAGE_ALF_CLIENT_PORTS;
		} else {
			previousPage = PAGE_ALF_CLIENT;
		}
		break;
	case PAGE_SB_READ:
		if (history_->haveSandboxPolicy()) {
			previousPage = PAGE_SB_OVERWRITE;
		} else {
			previousPage = PAGE_SB;
		}
		break;
	case PAGE_SB_READ_FILES:
		previousPage = PAGE_SB_READ;
		break;
	case PAGE_SB_WRITE:
		if (history_->getSandboxPermission(SbEntry::READ) ==
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
		if (history_->getSandboxPermission(SbEntry::WRITE) ==
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
		if (history_->haveSandboxPolicy()) {
			overwrite = history_->shallOverwriteSandboxPolicy();
		}
		if (overwrite == RuleWizardHistory::OVERWRITE_NO) {
			previousPage = PAGE_SB_OVERWRITE;
		} else if (history_->haveSandbox() !=
		    RuleWizardHistory::PERM_RESTRICT_USER) {
			previousPage = PAGE_SB;
		} else if (history_->getSandboxPermission(SbEntry::EXECUTE) ==
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

	if (!history_->haveContextPolicy()) {
		createContextPolicy(ruleSet);
	}
	createAlfPolicy(ruleSet);
	if (history_->haveSandbox() != RuleWizardHistory::PERM_NONE)
		createSandboxPolicy(ruleSet);

	newRuleSetEvent.SetInt(ruleSet->getRuleSetId());
	newRuleSetEvent.SetExtraLong(ruleSet->getRuleSetId());
	ruleSet->unlock();

	if(history_->shallActivatePolicy()) {
		polRes = policyCtrl->sendToDaemon(ruleSet->getRuleSetId());
		switch (polRes) {
		case PolicyCtrl::RESULT_POL_WRONG_PASS:
			message = _("The entered password is incorrect.");
			anMessageBox(message, _("Key Load Error"),
			    wxOK|wxICON_ERROR, this);
			MainUtils::instance()->status(wxT("Wrong password!"));
			break;
		case PolicyCtrl::RESULT_POL_ERR:
			message = _("An error occured while sending admin"
			    " policy to the daemon.");
			anMessageBox(message, _("Policy Load Error"),
				wxOK|wxICON_ERROR, this);
			MainUtils::instance()->status(wxT("Error while sending"
			    " admin policy to daemon."));
			break;
		case PolicyCtrl::RESULT_POL_ABORT:
		case PolicyCtrl::RESULT_POL_OK:
			MainUtils::instance()->status(
			    _("policy sent to daemon"));
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
	ctxApp->addBinary(history_->getProgram());
	if (history_->getChecksumType() == APN_CS_NONE) {
		ctxApp->setSubjectNone(0);
	} else {
		ctxApp->setSubjectSelf(0, false);
	}

	ctxApp->setFlag(APN_RULE_NOSFS, history_->isSfsDisabled());

	if (!history_->isSameContext()) {
		/* Create 'context new any' */
		ctxFilter = new ContextFilterPolicy(ctxApp);
		ctxApp->prependFilterPolicy(ctxFilter);
		ctxFilter->setContextTypeNo(APN_CTX_NEW);
	} else {
		if (history_->haveContextException()) {
			StringListModel *ctxModel =
			    history_->getContextExceptions();

			ctxFilter = new ContextFilterPolicy(ctxApp);
			ctxApp->prependFilterPolicy(ctxFilter);
			ctxFilter->setContextTypeNo(APN_CTX_NEW);

			for (unsigned int i = 0; i < ctxModel->count(); i++) {
				ctxFilter->addBinary(ctxModel->get(i));
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

	if (history_->haveAlfPolicy()) {
		alfApp = ruleSet->searchAlfAppPolicy(history_->getProgram());
		if (history_->shallOverwriteAlfPolicy() ==
		    RuleWizardHistory::OVERWRITE_NO) {
			return;
		}
		/*
		 * We just need to remove the filter, but it's easier to
		 * remove all and re-create the app policy.
		 */
		if (alfApp->getBinaryCount() > 1) {
			alfApp->removeBinary(history_->getProgram());
		} else {
			alfApp->remove();
		}
	}

	/* Create new application bock */
	alfApp = new AlfAppPolicy(ruleSet);
	ruleSet->prependAppPolicy(alfApp);
	alfApp->addBinary(history_->getProgram());
	if (history_->getChecksumType() == APN_CS_NONE) {
		alfApp->setSubjectNone(0);
	} else {
		alfApp->setSubjectSelf(0, false);
	}

	dflFilter = new DefaultFilterPolicy(alfApp);
	alfApp->prependFilterPolicy(dflFilter);

	switch (history_->getAlfClientPermission()) {
	case RuleWizardHistory::PERM_ALLOW_ALL:
		dflFilter->setActionNo(APN_ACTION_ALLOW);
		break;
	case RuleWizardHistory::PERM_DENY_ALL:
		dflFilter->setActionNo(APN_ACTION_DENY);
		dflFilter->setLogNo(APN_LOG_NORMAL);
		break;
	case RuleWizardHistory::PERM_RESTRICT_DEFAULT:
		dflFilter->setActionNo(APN_ACTION_ASK);
		history_->getAlfClientPortList()->assignDefaultServices();
		createAlfPortList(alfApp, APN_CONNECT);
		break;
	default:
		if (history_->getAlfClientAsk()) {
			dflFilter->setActionNo(APN_ACTION_ASK);
		} else {
			dflFilter->setActionNo(APN_ACTION_DENY);
			dflFilter->setLogNo(APN_LOG_NORMAL);
		}

		if (history_->getAlfClientRaw()) {
			alfCap = new AlfCapabilityFilterPolicy(alfApp);
			alfApp->prependFilterPolicy(alfCap);
			alfCap->setCapabilityTypeNo(APN_ALF_CAPRAW);
		}

		createAlfPortList(alfApp, APN_CONNECT);
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

	if (history_->haveSandboxPolicy()) {
		sbApp = ruleSet->searchSandboxAppPolicy(history_->getProgram());
		if (history_->shallOverwriteSandboxPolicy() ==
		    RuleWizardHistory::OVERWRITE_NO) {
			return;
		}
		/*
		 * We just need to remove the filter, but it's easier to
		 * remove all and re-create the app policy.
		 */
		if (sbApp->getBinaryCount() > 1) {
			sbApp->removeBinary(history_->getProgram());
		} else {
			sbApp->remove();
		}
	}
	/* Create new app block. */
	sbApp = new SbAppPolicy(ruleSet);
	ruleSet->prependAppPolicy(sbApp);
	sbApp->addBinary(history_->getProgram());
	if (history_->getChecksumType() == APN_CS_NONE) {
		sbApp->setSubjectNone(0);
	} else {
		sbApp->setSubjectSelf(0, false);
	}

	/* Create a default ASK policy just to be sure. */
	dflFilter = new DefaultFilterPolicy(sbApp);
	sbApp->prependFilterPolicy(dflFilter);
	dflFilter->setActionNo(APN_ACTION_ASK);

	createSbPermissions(sbApp);
}

void
RuleWizard::createAlfPortList(AlfAppPolicy *app, int direction) const
{
	ServiceList	*list = history_->getAlfClientPortList();
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
	for (size_t i = 0; i < list->getServiceCount(); i++) {
		Service	*service = list->getServiceAt(i);
		int	proto = IPPROTO_UDP;

		if (service->getProtocol() == Service::TCP)
			proto = IPPROTO_TCP;

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

			/*
			 * XXX This is poor! You need a way to put a
			 * XXX port-number into the policy (without) converting
			 * XXX it into a string.
			 */
			wxString port_str =
			    wxString::Format(wxT("%d"), service->getPort());

			if (dirs[x] == APN_CONNECT || proto == IPPROTO_TCP)
				filter->setToPortName(port_str);
			else
				filter->setFromPortName(port_str);
		}
	}
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
RuleWizard::createSbPermissions(SbAppPolicy *app) const
{
	/*
	 * Stage 1: The user can choose, if default-sandbox-policies should be
	 *          created.
	 */
	createSbPermissionsStage1();

	/*
	 * Stage 2: For each permission (read, write, execute) the user can
	 *          choose, if default-policies should be created or the access
	 *          can be allowed for everything.
	 */
	createSbPermissionsStage2(app, SbEntry::READ);
	createSbPermissionsStage2(app, SbEntry::WRITE);
	createSbPermissionsStage2(app, SbEntry::EXECUTE);

	/*
	 * Stage 3: For each permission (read, write, execute) the user can
	 *          specify an own list of allowed files.
	 */
	createSbPermissionsStage3(app, SbEntry::READ);
	createSbPermissionsStage3(app, SbEntry::WRITE);
	createSbPermissionsStage3(app, SbEntry::EXECUTE);

	/*
	 * Finally convert entries of the SbModel into policies
	 */
	SbModel *model = history_->getSandboxFileList();
	for (unsigned int i = 0; i < model->getEntryCount(); i++) {
		SbEntry *entry = model->getEntry(i);

		SbAccessFilterPolicy *filter = new SbAccessFilterPolicy(app);
		app->prependFilterPolicy(filter);

		filter->setActionNo(APN_ACTION_ALLOW);
		filter->setLogNo(APN_LOG_NONE);
		filter->setPath(entry->getPath());

		int accessMask = 0;
		if (entry->hasPermission(SbEntry::READ))
			accessMask |= APN_SBA_READ;
		if (entry->hasPermission(SbEntry::WRITE))
			accessMask |= APN_SBA_WRITE;
		if (entry->hasPermission(SbEntry::EXECUTE))
			accessMask |= APN_SBA_EXEC;
		filter->setAccessMask(accessMask);
	}
}

void
RuleWizard::createSbPermissionsStage1(void) const
{
	if (history_->haveSandbox() ==
	    RuleWizardHistory::PERM_RESTRICT_DEFAULT) {
		/*
		 * The user has chosen
		 * "Yes, load default policies (skip wizard)".
		 * Load defaults for all permissions.
		 */
		history_->getSandboxFileList()->assignDefaults(SbEntry::READ);
		history_->getSandboxFileList()->assignDefaults(SbEntry::WRITE);
		history_->getSandboxFileList()
		    ->assignDefaults(SbEntry::EXECUTE);
	}

	/*
	 * For any other possibilities nothing is to do.
	 * - "Yes, create policies (wizard guided)": The decision is made in a
	 *   later step of th wizard
	 * - "No, do not create sandbox policies": No sandbox-policies should
	 *   be created.
	 */
}

void
RuleWizard::createSbPermissionsStage2(SbAppPolicy *app,
    SbEntry::Permission permission) const
{
	if (history_->haveSandbox() != RuleWizardHistory::PERM_RESTRICT_USER) {
		/* Wizard has not reached stage 2, abort */
		return;
	}

	switch (history_->getSandboxPermission(permission)) {
	case RuleWizardHistory::PERM_ALLOW_ALL:
		/*
		 * The user has chosen "unrestricted".
		 * Create an allow-policy for the root-directory.
		 */
		createSbPermissionRoot(app, toAPN_SBA(permission),
		    APN_ACTION_ALLOW);
		break;
	case RuleWizardHistory::PERM_RESTRICT_DEFAULT:
		/*
		 * The user has chosen "restricted (default)".
		 * Load defaults for the given permission.
		 */
		history_->getSandboxFileList()->assignDefaults(permission);
		break;
	default:
		/*
		 * For the third possibility "restricted" nothing is to do
		 * at this point. The decision is made in the next step of
		 * the wizard.
		 */
		break;
	}
}

void
RuleWizard::createSbPermissionsStage3(SbAppPolicy *app,
    SbEntry::Permission permission) const
{
	if (history_->getSandboxPermission(permission) !=
	    RuleWizardHistory::PERM_RESTRICT_USER) {
		/* The wizard has not reached stage 3, abort */
		return;
	}

	int section = toAPN_SBA(permission);

	/*
	 * The user has filled up the sandbox-model by its own.
	 * You only need to convert it into policies.
	 */

	/* Create the rule for the root directory. */
	if (history_->getSandboxAsk(permission))
		createSbPermissionRoot(app, section, APN_ACTION_ASK);
	else
		createSbPermissionRoot(app, section, APN_ACTION_DENY);

	/* Finally put signature rules first if those were requested. */
	if (history_->getSandboxValidSignature(permission)) {
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

inline int
RuleWizard::toAPN_SBA(SbEntry::Permission permission)
{
	switch (permission) {
	case SbEntry::READ:
		return (APN_SBA_READ);
	case SbEntry::WRITE:
		return (APN_SBA_WRITE);
	case SbEntry::EXECUTE:
		return (APN_SBA_EXEC);
	}

	return (-1); /* Never reached */
}
