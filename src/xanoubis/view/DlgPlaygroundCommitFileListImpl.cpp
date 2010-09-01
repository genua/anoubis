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
#include "AnListColumn.h"
#include "DlgPlaygroundCommitFileListImpl.h"
#include "PlaygroundCtrl.h"
#include "JobCtrl.h"
#include "PlaygroundListProperty.h"
#include "PlaygroundFileEntry.h"

#define ADD_PROPERTY(list, type, width) \
	do { \
		AnListColumn *_col; \
		_col = fileList->addColumn(new PlaygroundListProperty( \
		    PlaygroundListProperty::type)); \
		_col->setWidth(width, true); \
	} while(0);

DlgPlaygroundCommitFileListImpl::DlgPlaygroundCommitFileListImpl(void)
    : DlgPlaygroundCommitFileListBase(NULL)
{
	PlaygroundCtrl	*playgroundCtrl = PlaygroundCtrl::instance();
	JobCtrl		*jobctrl = JobCtrl::instance();

	ADD_PROPERTY(fileList, PROPERTY_DEV, 80);
	ADD_PROPERTY(fileList, PROPERTY_INODE, 80);
	ADD_PROPERTY(fileList, PROPERTY_FILENAME, 440);

	fileList->getColumn(0)->setVisible(false);
	fileList->getColumn(1)->setVisible(false);

	fileList->setRowProvider(playgroundCtrl->getFileProvider());

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
	const wxArrayString &errors = PlaygroundCtrl::instance()->getErrors();

	for (unsigned int i=0; i<errors.Count(); i++)
		wxLogError(errors[i]);

	if (errors.Count() > 1) {
		wxLogError(_("Multiple errors occured while processing the "
		    "Playground request"));
	}
	PlaygroundCtrl::instance()->clearErrors();
}

void
DlgPlaygroundCommitFileListImpl::onPlaygroundCompleted(wxCommandEvent &)
{
	/* remove the selection */
	for (int i=fileList->getFirstSelection(); i>=0;
	    i=fileList->getNextSelection(i)) {
		fileList->SetItemState(i, 0,
		    wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
	}

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
