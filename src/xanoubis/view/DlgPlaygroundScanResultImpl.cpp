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

#include <wx/sizer.h>
#include <wx/textctrl.h>

#include "AnDetails.h"
#include "AnIconList.h"
#include "DlgPlaygroundScanResultImpl.h"

#define TEXTSTYLE (wxHSCROLL|wxTE_DONTWRAP|wxTE_MULTILINE|wxTE_READONLY)
#define BUTTONSTATE(button, state) \
	do { \
		button->Enable(state); \
		button->Show(state); \
	} while (0)

DlgPlaygroundScanResultImpl::DlgPlaygroundScanResultImpl(void) :
    DlgPlaygroundScanResultBase(NULL)
{
	setRequired(true);
	alertIcon->SetIcon(AnIconList::instance()->GetIcon(
	    AnIconList::ICON_PROBLEM_48));
}

void
DlgPlaygroundScanResultImpl::addResult(const wxString & title,
    const wxString & result)
{
	wxBoxSizer	*sizer = NULL;
	AnDetails	*details = NULL;
	wxTextCtrl	*text = NULL;

	sizer = new wxBoxSizer(wxVERTICAL);
	details = new AnDetails(this, wxID_ANY, wxDefaultPosition,
	    wxDefaultSize, wxTAB_TRAVERSAL, title);
	text = new wxTextCtrl(details, wxID_ANY, wxEmptyString,
	    wxDefaultPosition, wxSize(-1, 160), TEXTSTYLE);

	details->Connect(wxEVT_SIZE, wxSizeEventHandler(
	    DlgPlaygroundScanResultImpl::onSize), NULL, this);
	text->WriteText(result);

	sizer->Add(text, 0, wxALL|wxEXPAND, 5);
	details->SetSizer(sizer, true);
	resultSizer->Add(details, 0, wxALL|wxEXPAND, 0);
}

void
DlgPlaygroundScanResultImpl::setRequired(bool isRequired)
{
	BUTTONSTATE(okButton, isRequired);
	BUTTONSTATE(skipButton, !isRequired);
	BUTTONSTATE(commitButton, !isRequired);

	if (isRequired) {
		questionLabel->SetLabel(_("The file(s) will not be"
		    " committed."));
	} else {
		questionLabel->SetLabel(_("Do you really want to commit the"
		    " file(s)?"));
	}

	Layout();
	Refresh();
}

void
DlgPlaygroundScanResultImpl::setFileName(const wxString & fileName)
{
	fileNameLabel->SetLabel(fileName);
	Layout();
	Refresh();
}

void
DlgPlaygroundScanResultImpl::onSize(wxSizeEvent & event)
{
	event.Skip();
	Layout();
	Fit();
	Refresh();
}

void
DlgPlaygroundScanResultImpl::onOkButtonClick(wxCommandEvent &)
{
	EndModal(wxOK);
}

void
DlgPlaygroundScanResultImpl::onSkipButtonClick(wxCommandEvent &)
{
	EndModal(wxNO);
}

void
DlgPlaygroundScanResultImpl::onCommitButtonClick(wxCommandEvent &)
{
	EndModal(wxYES);
}
