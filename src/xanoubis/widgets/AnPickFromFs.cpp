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

#include "AnPickFromFs.h"

AnPickFromFs::AnPickFromFs(wxWindow *parent, wxWindowID id, const wxPoint & pos,
    const wxSize & size, long style, const wxString & name)
    : wxPanel(parent, id, pos, size, style, name)
{
	wxBoxSizer* mainSizer;

	mainSizer = new wxBoxSizer(wxHORIZONTAL);

	label_ = new wxStaticText(this, wxID_ANY, name,
	    wxDefaultPosition, wxDefaultSize, 0);
	mainSizer->Add(label_, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	inputTextCtrl_ = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
	    wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	mainSizer->Add(inputTextCtrl_, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	pickButton_ = new wxButton(this, wxID_ANY, _("Pick..."),
	    wxDefaultPosition, wxDefaultSize, 0);
	pickButton_->SetToolTip(_("Choose a binary"));
	mainSizer->Add(pickButton_, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	inputTextCtrl_->Connect(wxEVT_KILL_FOCUS,
	    wxFocusEventHandler(AnPickFromFs::onTextKillFocus), NULL, this);
	inputTextCtrl_->Connect(wxEVT_COMMAND_TEXT_ENTER,
	    wxCommandEventHandler(AnPickFromFs::onTextEnter), NULL, this);
	pickButton_->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(AnPickFromFs::onPickButton), NULL, this);

	this->SetSizer(mainSizer);
	this->Layout();
}

void
AnPickFromFs::onTextKillFocus(wxFocusEvent &)
{
	/*
	 * Only validate on a set-Focus event if the user has modified
	 * the text in the text ctrl since we last called setBinary.
	 */
	if (inputTextCtrl_->IsModified()) {
		//setBinary(binaryTextCtrl->GetValue());
	}
}

void
AnPickFromFs::onTextEnter(wxCommandEvent &)
{
	//setBinary(event.GetString());
}

void
AnPickFromFs::onPickButton(wxCommandEvent &)
{
	wxFileName	defaultPath;
	wxFileDialog	fileDlg(this);
	/*
	if (appPolicy_ != NULL) {
		defaultPath.Assign(appPolicy_->getBinaryName(binaryIndex_));
	}
	if (ctxPolicy_ != NULL) {
		defaultPath.Assign(ctxPolicy_->getBinaryName(binaryIndex_));
	}
	*/
	wxBeginBusyCursor();
	//fileDlg.SetDirectory(defaultPath.GetPath());
	//fileDlg.SetFilename(defaultPath.GetFullName());
	fileDlg.SetWildcard(wxT("*"));
	wxEndBusyCursor();

	if (fileDlg.ShowModal() == wxID_OK) {
		//setBinary(fileDlg.GetPath());
	}
}
