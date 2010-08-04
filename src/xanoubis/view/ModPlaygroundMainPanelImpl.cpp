/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include <sys/wait.h>

#define __STDC_FORMAT_MACROS
#include <anoubis_errno.h>
#include <anoubis_playground.h>
#include <inttypes.h>
#include <signal.h>
#include <wx/log.h>
#include <wx/tokenzr.h>

#include "AnEvents.h"
#include "AnListColumn.h"
#include "AnListCtrl.h"
#include "AnMessageDialog.h"
#include "AnPickFromFs.h"
#include "Debug.h"
#include "DlgPlaygroundCommitFileListImpl.h"
#include "JobCtrl.h"
#include "ModPlaygroundMainPanelImpl.h"
#include "ModPlaygroundRowProperty.h"
#include "PlaygroundCtrl.h"
#include "PlaygroundInfoEntry.h"
#include "PlaygroundListProperty.h"

#define ADD_PROPERTY(list, type, width) \
	do { \
		AnListColumn *_col; \
		_col = list->addColumn(new PlaygroundListProperty( \
		    PlaygroundListProperty::type)); \
		_col->setWidth(width); \
	} while (0)

#ifdef LINUX
static void
chldhandler(int)
{
	int	status;
	pid_t	pid;

	pid = wait(&status);
	if (WIFEXITED(status)) {
		Debug::trace(wxString::Format(wxT("Playground - child "
		    "with pid %d exited with status %d"), pid,
		    WEXITSTATUS(status)));
	}
}
#endif /* LINUX */

ModPlaygroundMainPanelImpl::ModPlaygroundMainPanelImpl(wxWindow* parent,
    wxWindowID id) : ModPlaygroundMainPanelBase(parent, id)
{
	PlaygroundCtrl *playgroundCtrl = NULL;

	playgroundCtrl = PlaygroundCtrl::instance();

	ADD_PROPERTY(pgList, PROPERTY_ATTENTION, 20);
	ADD_PROPERTY(pgList, PROPERTY_ID, 55);
	ADD_PROPERTY(pgList, PROPERTY_COMMAND, 260);
	ADD_PROPERTY(pgList, PROPERTY_STAT, 80);
	ADD_PROPERTY(pgList, PROPERTY_FILES, 65);
	ADD_PROPERTY(pgList, PROPERTY_USER, 70);
	ADD_PROPERTY(pgList, PROPERTY_TIME, 180);

	pgList->setStateKey(wxT("/State/ModPlaygroundMainPanelImpl"));
	pgList->setRowProvider(playgroundCtrl->getInfoProvider());
	pgList->setRowProperty(new ModPlaygroundRowProperty);

	playgroundCtrl->Connect(anEVT_PLAYGROUND_ERROR, wxCommandEventHandler(
	    ModPlaygroundMainPanelImpl::onPlaygroundError), NULL, this);
	JobCtrl::instance()->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(
	      ModPlaygroundMainPanelImpl::onConnectionStateChange),
	    NULL, this);
}

ModPlaygroundMainPanelImpl::~ModPlaygroundMainPanelImpl(void)
{
	PlaygroundCtrl::instance()->Disconnect(anEVT_PLAYGROUND_ERROR,
	    wxCommandEventHandler(
	    ModPlaygroundMainPanelImpl::onPlaygroundError), NULL, this);
	JobCtrl::instance()->Disconnect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(
	      ModPlaygroundMainPanelImpl::onConnectionStateChange),
	    NULL, this);

}

void
ModPlaygroundMainPanelImpl::update(void)
{
	refreshPlaygroundList();
}

void
ModPlaygroundMainPanelImpl::onConnectionStateChange(wxCommandEvent &event)
{
	JobCtrl::ConnectionState state =
	    (JobCtrl::ConnectionState)event.GetInt();
	pgRefreshButton->Enable(state == JobCtrl::CONNECTED);

	event.Skip();
}

void
ModPlaygroundMainPanelImpl::onAppPathEntered(wxCommandEvent &)
{
	applicationStartButton->Enable(
	    applicationComboBox->GetValue().Length() > 0);
}

void
ModPlaygroundMainPanelImpl::onAppStartEnter(wxCommandEvent &)
{
	startApplication();
}

void
ModPlaygroundMainPanelImpl::onAppStart(wxCommandEvent &)
{
	startApplication();
}

void
ModPlaygroundMainPanelImpl::onPlaygroundError(wxCommandEvent &)
{
	const wxArrayString &errors = PlaygroundCtrl::instance()->getErrors();

	for (unsigned int i=0; i<errors.Count(); i++) {
		wxLogError(errors[i]);
	}

	if (errors.Count() > 1) {
		wxLogError(_("Multiple errors occured while processing the "
		    "Playground request"));
	}
	PlaygroundCtrl::instance()->clearErrors();
}

void
ModPlaygroundMainPanelImpl::onPgListRefreshClicked(wxCommandEvent &)
{
	refreshPlaygroundList();
}

void
ModPlaygroundMainPanelImpl::onPgNotebookChanging(wxCommandEvent& event)
{
	/* Check key config if the target is the playground overview tab */
	if (pgNotebook->GetPage(event.GetSelection()) == pgPage) {
		refreshPlaygroundList();
	}
}

void
ModPlaygroundMainPanelImpl::onCommitFiles(wxCommandEvent &)
{
	openCommitDialog();
}

void
ModPlaygroundMainPanelImpl::onDeleteFiles(wxCommandEvent &)
{
	PlaygroundCtrl      *playgroundCtrl;
	AnRowProvider       *rowProvider;
	AnListClass         *item;
	PlaygroundInfoEntry *entry;
	uint64_t            pgId;

	/* basic validation */
	if (!pgList->hasSelection()) {
		/* How did we get here if nothing is selected? */
		pgCommitButton->Disable();
		pgDeleteButton->Disable();
		return;
	}

	/* determine pgid */
	playgroundCtrl = PlaygroundCtrl::instance();
	rowProvider = playgroundCtrl->getInfoProvider();
	item = rowProvider->getRow(pgList->getFirstSelection());
	entry = dynamic_cast<PlaygroundInfoEntry *>(item);
	if (entry != NULL) {
		pgId = entry->getPgid();
	} else {
		pgId = 0;
	}

	/* start delete task */
	if (!PlaygroundCtrl::instance()->removePlayground(pgId)) {
		anMessageBox(_("Could not delete playground.\n"),
		    _("Playground error"), wxOK | wxICON_ERROR, this);
	}
}

void
ModPlaygroundMainPanelImpl::onPgListItemSelect(wxListEvent &event)
{
	AnListClass		*item = NULL;
	AnRowProvider		*rowProvider = NULL;
	PlaygroundCtrl		*playgroundCtrl = NULL;
	PlaygroundInfoEntry	*entry = NULL;

	playgroundCtrl = PlaygroundCtrl::instance();
	rowProvider = playgroundCtrl->getInfoProvider();
	item = rowProvider->getRow(event.GetIndex());
	entry = dynamic_cast<PlaygroundInfoEntry *>(item);

	if ((entry != NULL) && (entry->getNumFiles() > 0) &&
	    (entry->getUid() == geteuid())) {
		pgCommitButton->Enable();
		pgDeleteButton->Enable();
	}
}

void
ModPlaygroundMainPanelImpl::onPgListItemDeselect(wxListEvent &)
{
	pgCommitButton->Disable();
	pgDeleteButton->Disable();
}

void
ModPlaygroundMainPanelImpl::onPgListItemActivate(wxListEvent &event)
{
	AnListClass		*item = NULL;
	AnRowProvider		*rowProvider = NULL;
	PlaygroundCtrl		*playgroundCtrl = NULL;
	PlaygroundInfoEntry	*entry = NULL;
	AnMessageDialog		*dlg = NULL;

	playgroundCtrl = PlaygroundCtrl::instance();
	rowProvider = playgroundCtrl->getInfoProvider();
	item = rowProvider->getRow(event.GetIndex());
	entry = dynamic_cast<PlaygroundInfoEntry *>(item);

	if ((entry != NULL) && (entry->getNumFiles() > 0)) {
		if (entry->getUid() == geteuid()) {
			openCommitDialog();
		}
	} else {
		dlg = new AnMessageDialog(this, _("There are no files to "
		    "commit!"), _("Playground error"), wxOK | wxICON_ERROR);
		dlg->ShowModal();
		dlg->Destroy();
	}
}

void
ModPlaygroundMainPanelImpl::startApplication(void)
{
	int	  rc;
	int	  idx;
	char	**argv;
	wxString  message;
	wxString  command;

	command = applicationComboBox->GetValue();
	argv = convertStringToArgV(command);
	if (argv == NULL) {
		message = wxString::Format(_("Sytem error: %hs"),
		    anoubis_strerror(errno));
		anMessageBox(message, _("Playground error"),
		    wxOK | wxICON_ERROR, this);
		return;
	}

#ifdef LINUX
	signal(SIGCHLD, chldhandler);
	rc = playground_start_fork(argv);
#else
	rc = -ENOSYS;
#endif
	if (rc < 1) {
		/* Start failed, got no pid. */
		message = wxString::Format(_("Could not start program \"%ls\" "
		    "within playground: %ls"), command.BeforeFirst(' ').c_str(),
		    wxString::From8BitData(anoubis_strerror(-rc)).c_str());
		anMessageBox(message, _("Playground error"),
		    wxOK | wxICON_ERROR, this);
	} else {
		/* Successfull start. */
		Debug::trace(wxString::Format(wxT("Playground - start "
		    "program \"%ls\" with pid %d"), command.c_str(), rc));

		/* Clear combobox input. */
		applicationComboBox->SetValue(wxEmptyString);
		applicationStartButton->Enable(false);

		/*
		 * Do history.
		 * Search for command in history and remove it if found (to
		 * avoid multiple entries). Insert on top so the last command
		 * is always the first.
		 */
		idx = applicationComboBox->FindString(command);
		if (idx != wxNOT_FOUND) {
			applicationComboBox->Delete(idx);
		}
		applicationComboBox->Insert(command, 0);

		/* Update list of playgrounds. */
		if (JobCtrl::instance()->isConnected()) {
			refreshPlaygroundList();
		}
	}

	/* Cleanup argv. */
	idx = 0;
	while (argv[idx] != NULL) {
		free(argv[idx++]);
	}
	free(argv);
}

void
ModPlaygroundMainPanelImpl::refreshPlaygroundList(void)
{
	if (!PlaygroundCtrl::instance()->updatePlaygroundInfo()) {
		anMessageBox(_("Could not refresh list of playgrounds."),
		    _("Playground error"), wxOK | wxICON_ERROR, this);
	}
}

void
ModPlaygroundMainPanelImpl::openCommitDialog(void)
{
	DlgPlaygroundCommitFileListImpl *dlg;

	uint64_t		 pgId = 0;
	AnListClass		*item = NULL;
	AnRowProvider		*rowProvider = NULL;
	PlaygroundCtrl		*playgroundCtrl = NULL;
	PlaygroundInfoEntry	*entry = NULL;

	if (!pgList->hasSelection()) {
		/* How did we get here if nothing is selected? */
		pgCommitButton->Disable();
		pgDeleteButton->Disable();
		return;
	}

	playgroundCtrl = PlaygroundCtrl::instance();
	rowProvider = playgroundCtrl->getInfoProvider();
	item = rowProvider->getRow(pgList->getFirstSelection());
	entry = dynamic_cast<PlaygroundInfoEntry *>(item);
	if (entry != NULL) {
		pgId = entry->getPgid();
	} else {
		pgId = 0;
	}

	playgroundCtrl->updatePlaygroundFiles(pgId);

	dlg = new DlgPlaygroundCommitFileListImpl();
	dlg->SetTitle(
	    wxString::Format(_("Commit files for playground %"PRIx64), pgId));
	#ifdef USE_WXGUITESTING
		dlg->Show();
		/* Note: the dialog will not be cleaned up. This is a
		 * memleak, but only for the test. */
	#else
		dlg->ShowModal();
		dlg->Destroy();
	#endif
}

char **
ModPlaygroundMainPanelImpl::convertStringToArgV(const wxString &input) const
{
	int			  count;
	wxStringTokenizer	  tokenizer;
	char			**argv;

	tokenizer.SetString(input);
	argv = (char **)calloc(tokenizer.CountTokens() + 1, sizeof(char *));
	if (argv == NULL) {
		return (NULL);
	}

	count = 0;
	while (tokenizer.HasMoreTokens()) {
		argv[count] = strdup(tokenizer.GetNextToken().To8BitData());
		count++;
	}
	argv[count] = NULL;

	return (argv);
}
