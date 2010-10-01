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

#include <wx/log.h>

#include "AnListCtrl.h"
#include "DlgPlaygroundCommitFileListImpl.h"
#include "PlaygroundCtrl.h"
#include "JobCtrl.h"
#include "PlaygroundListProperty.h"
#include "PlaygroundFileEntry.h"
#include "PlaygroundInfoEntry.h"

DlgPlaygroundCommitFileListImpl::DlgPlaygroundCommitFileListImpl(
    PlaygroundInfoEntry *entry) : DlgPlaygroundCommitFileListBase(NULL)
{
	PlaygroundCtrl		*playgroundCtrl = PlaygroundCtrl::instance();
	JobCtrl			*jobctrl = JobCtrl::instance();
	AnGenericRowProvider	*provider;

	fileList->setStateKey(wxT("/State/PlaygroundFileList"));

	fileList->addColumn(new PlaygroundListProperty(
	    PlaygroundListProperty::PROPERTY_DEV), 80, false);
	fileList->addColumn(new PlaygroundListProperty(
	    PlaygroundListProperty::PROPERTY_INODE), 80, false);
	fileList->addColumn(new PlaygroundListProperty(
	    PlaygroundListProperty::PROPERTY_FILENAME), 4440);

	provider = playgroundCtrl->getFileProvider();
	provider->setFilterProperty(fileList->getRawProperty(2));
	fileList->setRowProvider(provider);

	commitButton->Enable(
	    jobctrl->isConnected() && entry && !entry->isActive());
	delButton->Enable(
	    jobctrl->isConnected() && entry && !entry->isActive());

	playgroundCtrl->Connect(anEVT_PLAYGROUND_ERROR, wxCommandEventHandler(
	    DlgPlaygroundCommitFileListImpl::onPlaygroundError), NULL, this);
	playgroundCtrl->Connect(anEVT_PLAYGROUND_COMPLETED,
	    wxCommandEventHandler(
	    DlgPlaygroundCommitFileListImpl::onPlaygroundCompleted), NULL,
	    this);
	jobctrl->Connect(anTASKEVT_PROGRESS, wxTaskEventHandler(
	    DlgPlaygroundCommitFileListImpl::onTaskProgress), NULL, this);
}

DlgPlaygroundCommitFileListImpl::~DlgPlaygroundCommitFileListImpl(void)
{
	PlaygroundCtrl	*playgroundCtrl = PlaygroundCtrl::instance();
	JobCtrl		*jobctrl = JobCtrl::instance();

	playgroundCtrl->Disconnect(anEVT_PLAYGROUND_ERROR,
	    wxCommandEventHandler(
	    DlgPlaygroundCommitFileListImpl::onPlaygroundError), NULL, this);
	playgroundCtrl->Disconnect(anEVT_PLAYGROUND_COMPLETED,
	    wxCommandEventHandler(
	    DlgPlaygroundCommitFileListImpl::onPlaygroundCompleted), NULL,
	    this);
	jobctrl->Disconnect(anTASKEVT_PROGRESS, wxTaskEventHandler(
	    DlgPlaygroundCommitFileListImpl::onTaskProgress), NULL, this);
}

void
DlgPlaygroundCommitFileListImpl::onColumnButtonClick(wxCommandEvent &event)
{
	event.Skip();
	fileList->showColumnVisibilityDialog();
}

void
DlgPlaygroundCommitFileListImpl::onDeleteClicked(wxCommandEvent &event)
{
	std::vector<int> selection;

	event.Skip();
	for (int i=fileList->getFirstSelection(); i>=0;
	    i=fileList->getNextSelection(i)) {
		selection.push_back(i);
	}
	if (PlaygroundCtrl::instance()->removeFiles(selection)) {
		beginActivity();
	}
}

void
DlgPlaygroundCommitFileListImpl::onCommitClicked(wxCommandEvent &event)
{
	std::vector<int> sel;

	event.Skip();
	for (int i=fileList->getFirstSelection(); i>=0;
	    i=fileList->getNextSelection(i)) {
		sel.push_back(i);
	}
	if (PlaygroundCtrl::instance()->commitFiles(sel)) {
		beginActivity();
	}
}

void
DlgPlaygroundCommitFileListImpl::onCloseClicked(wxCommandEvent &event)
{
	event.Skip();
	Close();
}

void
DlgPlaygroundCommitFileListImpl::onPlaygroundError(wxCommandEvent &)
{
	const std::vector<wxString>		errors
	    = PlaygroundCtrl::instance()->getErrors();

	for (unsigned int i=0; i<errors.size(); i++)
		wxLogError(errors[i]);

	if (errors.size() > 1) {
		wxLogError(_("Multiple errors occured while processing the "
		    "Playground request"));
	}
	PlaygroundCtrl::instance()->clearErrors();
}

void
DlgPlaygroundCommitFileListImpl::onPlaygroundCompleted(wxCommandEvent &)
{
	/* remove the selection */
	fileList->clearSelection();
	/* reenable buttons */
	commitButton->Enable();
	delButton->Enable();
	closeButton->Enable();
	progressText->SetLabel(wxT(""));
	wxEndBusyCursor();
}

void
DlgPlaygroundCommitFileListImpl::beginActivity(void)
{
	commitButton->Disable();
	delButton->Disable();
	closeButton->Disable();
	wxBeginBusyCursor();
}

void
DlgPlaygroundCommitFileListImpl::onTaskProgress(TaskEvent &event)
{
	Task		*task = event.getTask();

	event.Skip();
	progressText->SetLabel(task->getProgressText());
}

void
DlgPlaygroundCommitFileListImpl::onCommitListColClick(wxListEvent &event)
{
	AnGenericRowProvider		*p;

	event.Skip();
	p = PlaygroundCtrl::instance()->getFileProvider();
	fileList->clearSelection();
	p->setSortProperty(fileList->getProperty(event.GetColumn()));
}

void
DlgPlaygroundCommitFileListImpl::onCommitSearchKillFocus(wxFocusEvent &event)
{
	AnGenericRowProvider		*p;

	event.Skip();
	p = PlaygroundCtrl::instance()->getFileProvider();
	if (listSearchEntry->IsModified()) {
		fileList->clearSelection();
		p->setFilterString(listSearchEntry->GetValue());
		listSearchEntry->DiscardEdits();
	}
}

/*
 * NOTE: The event is _not_ skipped intentionally. If we do skip it the
 * dialog will consume the return event for its default action (close), too.
 */
void
DlgPlaygroundCommitFileListImpl::onCommitSearchEnter(wxCommandEvent &event)
{
	AnGenericRowProvider		*p;

	event.Skip(false);
	p = PlaygroundCtrl::instance()->getFileProvider();
	if (listSearchEntry->IsModified()) {
		fileList->clearSelection();
		p->setFilterString(listSearchEntry->GetValue());
		listSearchEntry->DiscardEdits();
	}
}
