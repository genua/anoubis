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

#include "main.h"
#include "Policy.h"
#include "AppPolicy.h"
#include "VarPolicy.h"
#include "PolicyRuleSet.h"
#include "PolicyVisitor.h"

PolicyRuleSet::PolicyRuleSet(wxWindow *parent, struct apn_ruleset *ruleSet)
{
	ruleSet_ = NULL;

	create(parent, ruleSet);
}

PolicyRuleSet::PolicyRuleSet(wxWindow *parent, wxString fileName)
{
	ruleSet_ = NULL;

	create(parent, fileName);
}

PolicyRuleSet::~PolicyRuleSet(void)
{
	alfList_.DeleteContents(true);
	sfsList_.DeleteContents(true);
	varList_.DeleteContents(true);

	apn_free_ruleset(ruleSet_);
}

void
PolicyRuleSet::create(wxWindow *parent, struct apn_ruleset *ruleSet)
{
	wxString		 title;
	wxProgressDialog	*dialog;
	struct apn_rule		*appRule;
	struct var		*variable;

	ruleSet_ = ruleSet;

	title = _("Creating policy structure ...");

	dialog = new wxProgressDialog(title, title, ruleSet->maxid, parent);

	wxGetApp().status(title);
	TAILQ_FOREACH(appRule, &(ruleSet->alf_queue), entry) {
		alfList_.Append(new AppPolicy(appRule));
		dialog->Update(appRule->id, _("creating alf policies ..."));
	}

	TAILQ_FOREACH(appRule, &(ruleSet->sfs_queue), entry) {
		sfsList_.Append(new AppPolicy(appRule));
		dialog->Update(appRule->id, _("creating sfs policies ..."));
	}

	TAILQ_FOREACH(variable, &(ruleSet->var_queue), entry) {
		varList_.Append(new VarPolicy(variable));
		dialog->Update(appRule->id, _("creating var policies ..."));
	}
	dialog->Update(ruleSet->maxid);
	wxGetApp().status(_("done"));
}

void
PolicyRuleSet::create(wxWindow *parent, wxString fileName)
{
	wxString		 logEntry;
	int			 rc;
	struct apn_ruleset	*ruleSet;
	struct apn_errmsg	*errMsg;

	ruleSet = NULL;
	rc = apn_parse(fileName.fn_str(), &ruleSet, 0);

	switch (rc) {
	case -1:
		logEntry = wxT("System error during import of policy file ");
		logEntry += fileName + wxT(" : ");
		logEntry += wxString::From8BitData(strerror(errno));
		wxGetApp().log(logEntry);
		wxGetApp().status(logEntry);
		break;
	case 0:
		logEntry = wxT("Succsessfully imported policy file ");
		logEntry += fileName;
		wxGetApp().log(logEntry);
		wxGetApp().status(logEntry);
		create(parent, ruleSet);
		break;
	case 1:
		logEntry = wxT("Failed import of policy file ");
		if (TAILQ_EMPTY(&(ruleSet->err_queue))) {
			logEntry += fileName;
			wxGetApp().log(logEntry);
		}
		wxGetApp().status(logEntry);
		TAILQ_FOREACH(errMsg, &(ruleSet->err_queue), entry) {
			logEntry = wxT("Failed import of policy file ");
			logEntry += wxString::From8BitData(errMsg->msg);
			wxGetApp().log(logEntry);
		}
		break;
	default:
		logEntry = wxT("Unknown error during import of policy file ");
		logEntry += fileName;
		wxGetApp().log(logEntry);
		break;
	}
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
			logEntry = wxT("Policies exported successfully to ");
		}
		fchmod(fileno(exportFile->fp()), S_IRUSR);
		exportFile->Close();
	} else {
		logEntry = wxT("Could not open file for export: ");
	}
	logEntry += fileName;
	wxGetApp().log(logEntry);
	wxGetApp().status(logEntry);
}
