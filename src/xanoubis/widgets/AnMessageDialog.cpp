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
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include "AnIconList.h"
#include "AnMessageDialog.h"

AnMessageDialog::AnMessageDialog(wxWindow *parent, const wxString &message,
    const wxString &caption, long style, const wxPoint &pos)
    : wxDialog(parent, -1, caption, pos, wxDefaultSize, style)
{
	this->SetSizeHints(wxDefaultSize, wxSize(-1,-1));

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* messageSizer = new wxBoxSizer(wxHORIZONTAL);

	userOptions_ = wxGetApp().getUserOptions();
	configString_ = wxEmptyString;

	/* Icon is part of messageSizer */
	wxStaticBitmap *icon = createIcon(style);
	messageSizer->Add(icon, 0,
	    wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);

	/* Text is part of messageSizer */
	wxStaticText *text = createText(message);
	messageSizer->Add(text, 1, wxALL | wxEXPAND | wxFIXED_MINSIZE, 5);

	/* messageSizer is part of mainSizer */
	mainSizer->Add(messageSizer, 1, wxALL | wxEXPAND, 5);

	/* Buttons are part of mainSizer */
	mainSizer->Add(CreateButtonSizer(style), 0, wxALL | wxEXPAND, 5);

	/* Checkbox is part of mainSizer */
	dontShowMessageAgain = new wxCheckBox(this, wxID_ANY,
	    _("Don't show this message again!"),
	    wxDefaultPosition, wxDefaultSize, 0);
	dontShowMessageAgain->Hide();
	mainSizer->Add(dontShowMessageAgain, 0, wxALL | wxEXPAND, 5);

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
	userOptions_->Read(configString_, &showDialog);

	if (showDialog)
		return (wxDialog::ShowModal());
	else
		return (GetAffirmativeId());
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

	userOptions_->Read(userOption, &showMessage);

	if (showMessage) {
		dontShowMessageAgain->SetValue(!showMessage);
		dontShowMessageAgain->Show();
	} else
		dontShowMessageAgain->Hide();

	return (showMessage);
}

void
AnMessageDialog::OnDontShowMessageAgain(wxCommandEvent& WXUNUSED(event))
{
	userOptions_->Write(configString_, !dontShowMessageAgain->IsChecked());
	userOptions_->Flush();

	wxCommandEvent event(anEVT_ANOUBISOPTIONS_UPDATE);
	wxPostEvent(AnEvents::getInstance(), event);
}
