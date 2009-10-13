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

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/config.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include "AnIconList.h"
#include "AnMessageDialog.h"
#include "main.h"

AnMessageDialog::AnMessageDialog(wxWindow *parent, const wxString &message,
    const wxString &caption, long style, const wxPoint &pos)
    : wxDialog(parent, -1, caption, pos, wxDefaultSize, style)
{
	this->SetSizeHints(wxDefaultSize, wxSize(-1,-1));

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* messageSizer = new wxBoxSizer(wxHORIZONTAL);

	configString_ = wxEmptyString;

	/* Icon is part of messageSizer */
	iconCtrl_ = createIcon(style);
	messageSizer->Add(iconCtrl_, 0,
	    wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);

	/* Text is part of messageSizer */
	textCtrl_ = createText(message);
	messageSizer->Add(textCtrl_, 1, wxALL | wxEXPAND, 5);

	/* messageSizer is part of mainSizer */
	mainSizer->Add(messageSizer, 1, wxALL | wxEXPAND, 5);

	/* Buttons are part of mainSizer */
	buttonSizer_ = CreateButtonSizer(style);
	mainSizer->Add(buttonSizer_, 0, wxALL | wxEXPAND, 5);

	if (wxGetEnv(wxT("TNT_COMPATIBILITY"), NULL)) {
		wxButton *b = new wxButton(this, -1, _("Don't understand"));
		buttonSizer_->Add(b, 0, wxALL, 5);
	}

	if (style & wxNO)
		SetEscapeId(wxID_NO);
	if (style & wxCANCEL)
		SetEscapeId(wxID_CANCEL);

	/* Checkbox is part of mainSizer */
	dontShowMessageAgain = new wxCheckBox(this, wxID_ANY,
	    _("Don't show this message again!"),
	    wxDefaultPosition, wxDefaultSize, 0);
	dontShowMessageAgain->Hide();
	mainSizer->Add(dontShowMessageAgain, 0, wxALL | wxEXPAND, 5);

	Connect(wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(AnMessageDialog::onButton), NULL, this);
	dontShowMessageAgain->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
	    wxCommandEventHandler(AnMessageDialog::OnDontShowMessageAgain),
	    NULL, this );

	this->SetSizer(mainSizer);
	this->Layout();
	mainSizer->Fit(this);
}

AnMessageDialog::~AnMessageDialog() {}

int
AnMessageDialog::ShowModal(void)
{
	bool showDialog = false;

	/*
	 * Display the dialog, if configString_ does not exist
	 * (Read()-operation will fail) or the dialog is configured to be
	 * displayed (showDialog set to true by Read()).
	 */
	int buttonId;
	if (!wxConfig::Get()->Read(configString_, &showDialog) || showDialog)
		buttonId = wxDialog::ShowModal();
	else
		buttonId = GetAffirmativeId();

	switch (buttonId) {
	case wxID_OK:
		return (wxOK);
	case wxID_YES:
		return (wxYES);
	case wxID_NO:
		return (wxNO);
	case wxID_CANCEL:
		return (wxCANCEL);
	default:
		fprintf(stderr, "Unexpected return-code: %i\n", buttonId);
		return (wxCANCEL);
	}
}

wxStaticBitmap *
AnMessageDialog::createIcon(long style)
{
	wxStaticBitmap *icon = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap,
	    wxDefaultPosition, wxSize(48, 48), 0);

	/* handle Icon settings */
	AnIconList* iconList = AnIconList::getInstance();

	if (style & wxICON_HAND || style & wxICON_ERROR) {
		icon->SetIcon(iconList->GetIcon(AnIconList::ICON_ERROR_48));
	} else if (style & wxICON_QUESTION) {
		icon->SetIcon(iconList->GetIcon(AnIconList::ICON_QUESTION_48));
	} else if (style & wxICON_INFORMATION) {
		icon->SetIcon(iconList->GetIcon(AnIconList::ICON_PROBLEM_48));
	} else if (style & wxICON_EXCLAMATION) {
		icon->SetIcon(iconList->GetIcon(AnIconList::ICON_ALERT_48));
	}

	return (icon);
}

wxStaticText *
AnMessageDialog::createText(const wxString &message)
{
	wxStaticText *text = new wxStaticText(this, wxID_ANY, message,
	    wxPoint(-1, -1), wxSize(350, -1), wxST_NO_AUTORESIZE);
	text->Wrap(350);

	return (text);
}

bool
AnMessageDialog::onNotifyCheck(const wxString &userOption)
{
	bool showMessage = false;
	configString_ = userOption;

	wxConfig::Get()->Read(userOption, &showMessage);

	if (showMessage) {
		dontShowMessageAgain->SetValue(!showMessage);
		dontShowMessageAgain->Show();
	} else
		dontShowMessageAgain->Hide();

	this->Layout();
	GetSizer()->Fit(this);

	return (showMessage);
}

void
AnMessageDialog::onButton(wxCommandEvent &event)
{
	const int id = event.GetId();

	/*
	 * Hide dialog, if NO or CANCEL was clicked. The remaining buttons
	 * are already handled by wxDialog.
	 */
	if (id == wxID_NO || id == wxID_CANCEL)
		EndModal(id);
	else
		event.Skip();
}

void
AnMessageDialog::OnDontShowMessageAgain(wxCommandEvent& WXUNUSED(event))
{
	wxConfig::Get()->Write(configString_,
	    !dontShowMessageAgain->IsChecked());
}

int anMessageBox(const wxString &message, const wxString &caption,
    int style, wxWindow *parent, int x, int y)
{
	long decorated_style = style;

	if ((style &
	    (wxICON_EXCLAMATION | wxICON_HAND | wxICON_INFORMATION |
	    wxICON_QUESTION)) == 0)
	{
		decorated_style |= ( style & wxYES )
		    ? wxICON_QUESTION : wxICON_INFORMATION ;
	}

	AnMessageDialog dlg(parent, message, caption, decorated_style,
	    wxPoint(x, y));
	return (dlg.ShowModal());
}
