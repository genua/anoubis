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

#include <cstdlib>

#include <wx/filedlg.h>
#include <wx/filename.h>

#include <anoubis_errno.h>

#include "AnMessageDialog.h"
#include "AnPickFromFs.h"
#include "JobCtrl.h"
#include "RuleWizardPage.h"
#include "RuleWizardProgramPage.h"

RuleWizardProgramPage::RuleWizardProgramPage(wxWindow *parent,
    RuleWizardHistory *history) : Observer(NULL),
    RuleWizardProgramPageBase(parent)
{
	history->get();
	history_ = history;

	history_->setChecksumType(APN_CS_NONE);
	addSubject(programPicker);
	programPicker->setTitle(wxT("")); /* Title shown as extra label. */
	programPicker->setMode(AnPickFromFs::MODE_FILE);

	RuleWizardPage *page = wxDynamicCast(parent, RuleWizardPage);

	page->Connect(wxEVT_WIZARD_PAGE_CHANGING,
	    wxWizardEventHandler(RuleWizardProgramPage::onPageChanging),
	    NULL, this);
	page->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardProgramPage::onPageChanged),
	    NULL, this);

	/* Disabled until a valid program was selected */
	page->setNextEnabled(false);
	csumGetTask_ = NULL;
	csumAddTask_ = NULL;
	csumCalcTask_ = NULL;

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_GET,
	    wxTaskEventHandler(RuleWizardProgramPage::onCsumGet), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_ADD,
	    wxTaskEventHandler(RuleWizardProgramPage::onCsumAdd), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUMCALC,
	    wxTaskEventHandler(RuleWizardProgramPage::onCsumCalc), NULL, this);
}

RuleWizardProgramPage::~RuleWizardProgramPage(void)
{
	JobCtrl::getInstance()->Disconnect(anTASKEVT_CSUM_GET,
	    wxTaskEventHandler(RuleWizardProgramPage::onCsumGet), NULL, this);
	JobCtrl::getInstance()->Disconnect(anTASKEVT_CSUM_ADD,
	    wxTaskEventHandler(RuleWizardProgramPage::onCsumAdd), NULL, this);
	JobCtrl::getInstance()->Disconnect(anTASKEVT_CSUMCALC,
	    wxTaskEventHandler(RuleWizardProgramPage::onCsumCalc), NULL, this);
	if (csumGetTask_)
		delete csumGetTask_;
	if (csumAddTask_)
		delete csumAddTask_;
	if (csumCalcTask_)
		delete csumCalcTask_;
	RuleWizardHistory::put(history_);
}

void
RuleWizardProgramPage::update(Subject *subject)
{
	if (subject == programPicker) {
		if (wxFileExists(programPicker->getFileName())) {
			wxString	path = programPicker->getFileName();
			setProgram(path);
			updateNavi();

			/*
			 * A binary is selected, you need to fetch the checksum
			 * because maybe a checksum needs to be registered for
			 * the selected binary.
			 */
			if (JobCtrl::getInstance()->isConnected()) {
				csumGetTask_ = new ComCsumGetTask();
				csumGetTask_->addPath(path);
				JobCtrl::getInstance()->addTask(csumGetTask_);
			} else {
				getWizardPage()->setNextEnabled(true);
			}
		} else {
			getWizardPage()->setNextEnabled(false);
		}
	}
}

void
RuleWizardProgramPage::updateDelete(Subject *)
{
	removeSubject(programPicker);
}

void
RuleWizardProgramPage::onPageChanging(wxWizardEvent &event)
{
	wxString message;

	message = wxEmptyString;

	/* If no program was set, we'll not proceed to the next page. */
	if (history_->getProgram().IsEmpty()) {
		message = _("Please choose a program first.");
	}
	if (JobCtrl::getInstance()->isConnected()
	    && history_->getChecksumType() == APN_CS_NONE) {
		message = wxString::Format(_("The checksum for %ls "
		    "does not match! Please go to the SFS Browser and update "
		    "the checksum."), history_->getProgram().c_str());
	}
	if (!message.IsEmpty()) {
		anMessageBox(message, _("Rule Wizard"), wxOK | wxICON_ERROR,
		    this);
		event.Veto();
	}
}

void
RuleWizardProgramPage::onPageChanged(wxWizardEvent &)
{
	updateNavi();
}

void
RuleWizardProgramPage::onCsumGet(TaskEvent &event)
{
	if (event.getTask() != csumGetTask_) {
		/* Not "my" task, skip it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	ComTask::ComTaskResult result = csumGetTask_->getComTaskResult();

	if (result == ComTask::RESULT_SUCCESS
	    && csumGetTask_->getChecksumError(0) == 0) {
		/*
		 * A checksum is registered. Next, compare with local checksum.
		 * If the checksums does not match, the rule is useable.
		 */
		csumCalcTask_ = new CsumCalcTask();
		csumCalcTask_->setPath(programPicker->getFileName());
		JobCtrl::getInstance()->addTask(csumCalcTask_);
		/*
		 * NOTE: Do NOT delete comCsumGetTask_ here.
		 * NOTE: The handler for ComCsumCalcTask will need it.
		 */
	} else if (result == ComTask::RESULT_SUCCESS &&
	    csumGetTask_->getChecksumError(0) == ENOENT) {
		/*
		 * No checksum registered. Next, register a checksum for the
		 * selected binary. Otherwise the rule is not useable.
		 */
		csumAddTask_ = new ComCsumAddTask();
		csumAddTask_->addPath(programPicker->getFileName());
		JobCtrl::getInstance()->addTask(csumAddTask_);
		delete csumGetTask_;
		csumGetTask_ = NULL;
	} else {
		const char	*err;
		int		 error;

		/* An error occured while fetching the checksum */
		getWizardPage()->setNextEnabled(false);

		wxString message;
		wxString path = csumGetTask_->getPath(0);
		if (result == ComTask::RESULT_SUCCESS)
			error = csumGetTask_->getChecksumError(0);
		else
			error = csumGetTask_->getResultDetails();
		err = anoubis_strerror(error);

		switch (result) {
		case ComTask::RESULT_REMOTE_ERROR:
		case ComTask::RESULT_SUCCESS:
			message.Printf(_("Got error from daemon (%hs) while "
			    "fetching the checksum for %ls."),
			    err, path.c_str());
			break;
		case ComTask::RESULT_COM_ERROR:
			message.Printf(_("Communication error while fetching "
			"the checksum for %ls."), path.c_str());
			break;
		case ComTask::RESULT_LOCAL_ERROR:
			message.Printf(_("Failed to resolve %ls"),
			    path.c_str());
			break;
		default:
			message.Printf(_("An unexpected error occured (%hs) "
			    "while fetching the checksum for %ls."),
			    err, path.c_str());
			break;
		}

		anMessageBox(message, _("Rule Wizard"), wxOK | wxICON_ERROR,
		    this);
		delete csumGetTask_;
		csumGetTask_ = NULL;
	}
}

void
RuleWizardProgramPage::onCsumAdd(TaskEvent &event)
{
	int	error = 0;
	if (event.getTask() != csumAddTask_) {
		/* Not "my" task, skip it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	ComTask::ComTaskResult result = csumAddTask_->getComTaskResult();

	if (result == ComTask::RESULT_SUCCESS) {
		error = csumAddTask_->getChecksumError(0);
		if (error)
			result = ComTask::RESULT_REMOTE_ERROR;
	} else {
		error = csumAddTask_->getResultDetails();
	}
	if (result == ComTask::RESULT_SUCCESS) {
		/*
		 * Checksum was successfully added to the shadowtree, allow
		 * to continue with the wizard.
		 */
		getWizardPage()->setNextEnabled(true);
		history_->setChecksumType(APN_CS_UID_SELF);
	} else {
		/*
		 * An error occured while adding the checksum to the shadowtree
		 */
		getWizardPage()->setNextEnabled(false);

		wxString message;
		wxString path = csumAddTask_->getPath(0);
		const char *err = anoubis_strerror(error);

		switch (result) {
		case ComTask::RESULT_LOCAL_ERROR:
			message.Printf(_("Failed to calculate the checksum "
			    "for %ls: %hs"), path.c_str(), err);
			break;
		case ComTask::RESULT_COM_ERROR:
			message.Printf(_("Communication error while register "
			    "the checksum for %ls."), path.c_str());
			break;
		case ComTask::RESULT_REMOTE_ERROR:
			message.Printf(_("Got error from daemon (%hs) while "
			    "register the checksum for %ls."),
			    err, path.c_str());
			break;
		default:
			message.Printf(_("An unexpected error occured (%hs) "
			    "while register the checksum for %ls."),
			    err, path.c_str());
			break;
		}

		anMessageBox(message, _("Rule Wizard"), wxOK | wxICON_ERROR,
		    this);
	}
	delete csumAddTask_;
	csumAddTask_ = NULL;
}

void
RuleWizardProgramPage::onCsumCalc(TaskEvent &event)
{
	if (event.getTask() != csumCalcTask_) {
		/* Not "my" task, skip it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	if (csumCalcTask_->getResult() != 0) {
		/* Calculation failed */
		getWizardPage()->setNextEnabled(false);

		wxString message = wxString::Format(
		    _("Failed to calculate the checksum for %ls: %hs"),
		    csumCalcTask_->getPath().c_str(),
		    anoubis_strerror(csumCalcTask_->getResult()));

		anMessageBox(message, _("Rule Wizard"), wxOK | wxICON_ERROR,
		    this);
		delete csumCalcTask_;
		csumCalcTask_ = NULL;
		if (csumGetTask_) {
			delete csumGetTask_;
			csumGetTask_ = NULL;
		}
		return;
	}

	/* Compare with daemon-checksum */
	const u_int8_t *localCsum = csumCalcTask_->getCsum();
	const u_int8_t *daemonCsum;
	size_t		daemonCsumSize;

	/* Daemon-checksum still stored in comCsumGetTask_ */
	bool csok = csumGetTask_->getChecksumData(0, ANOUBIS_SIG_TYPE_CS,
	    daemonCsum, daemonCsumSize);

	if (csok && daemonCsumSize >= ANOUBIS_CS_LEN
	    && memcmp(localCsum, daemonCsum, ANOUBIS_CS_LEN) == 0) {
		/*
		 * Checksum matches with checksum from shadowtree, allow to
		 * continue with the wizard.
		 */
		history_->setChecksumType(APN_CS_UID_SELF);
		getWizardPage()->setNextEnabled(true);
	} else {
		/*
		 * Checksum does not match with checksum from shadowtree.
		 * Cannot continue with wizard because the policy will not
		 * work.
		 */
		getWizardPage()->setNextEnabled(false);

		wxString message = wxString::Format(_("The checksum for %ls "
		    "does not match! Please go to the SFS Browser and update "
		    "the checksum."), csumCalcTask_->getPath().c_str());

		anMessageBox(message, _("Rule Wizard"), wxOK | wxICON_ERROR,
		    this);
	}
	delete csumCalcTask_;
	csumCalcTask_ = NULL;
	if (csumGetTask_) {
		delete csumGetTask_;
		csumGetTask_ = NULL;
	}
}

void
RuleWizardProgramPage::setProgram(const wxString &binary)
{
	/* Store binary */
	if (binary == history_->getProgram()) {
		return;
	}

	history_->setProgram(binary);
	history_->setChecksumType(APN_CS_NONE);
	Layout();
	Refresh();
}

void
RuleWizardProgramPage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, true);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillAlfNavi(this, naviSizer, false);
	history_->fillSandboxNavi(this, naviSizer, false);
	Layout();
	Refresh();
}

inline RuleWizardPage *
RuleWizardProgramPage::getWizardPage(void) const
{
	return (wxDynamicCast(GetParent(), RuleWizardPage));
}
