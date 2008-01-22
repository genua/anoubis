/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#include <wx/panel.h>
#include <wx/statusbr.h>

#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "AnStatusBar.h"

/*
 * This macro calculates the (maybe) new position (x,y) within a given
 * field rectangle and panel size. Additionally 2 pixels are added to
 * every panels y value due corfieldRectangle placement.
 */
#define computePanelPosition(newx, newy, field, panel) \
	do { \
		newx = field.x + (field.width  - panel.x) / 2;     \
		newy = field.y + (field.height - panel.y) / 2 + 2; \
	} while (0)

AnStatusBar::AnStatusBar(wxWindow *parent) : wxStatusBar(parent, ID_STATUSBAR)
{
	wxBoxSizer	*raisedLogSizer;
	wxBoxSizer	*sunkenLogSizer;
	wxStaticText	*raisedLogLabel;
	wxStaticText	*sunkenLogLabel;
	wxBoxSizer	*raisedRuleEditorSizer;
	wxBoxSizer	*sunkenRuleEditorSizer;
	wxStaticText	*raisedRuleEditorLabel;
	wxStaticText	*sunkenRuleEditorLabel;

	/*
	 * 4 fields, the left one takes all remaining space, two
	 * fields for buttons, one for the resizer
	 */
	static const int fieldWidths[FIELD_IDX_LAST] = { -1, 80, 80, 25 };
	wxFont panelFont = wxFont(8, 70, 90, 90, false, wxT("Sans"));

	SetFieldsCount(FIELD_IDX_LAST);
	SetStatusWidths(FIELD_IDX_LAST, fieldWidths);

	/*
	 * setup Log panels (aka toggle button)
	 */
	raisedLogPanel_ = new wxPanel(this, wxID_ANY, wxDefaultPosition,
	    wxSize(70, 18), wxRAISED_BORDER);
	sunkenLogPanel_ = new wxPanel(this, wxID_ANY, wxDefaultPosition,
	    wxSize(70, 18), wxSUNKEN_BORDER);

	raisedLogLabel = new wxStaticText(raisedLogPanel_, wxID_ANY, _("Log"));
	raisedLogLabel->Wrap(-1);
	raisedLogLabel->SetFont(panelFont);
	sunkenLogLabel = new wxStaticText(sunkenLogPanel_, wxID_ANY, _("Log"));
	sunkenLogLabel->Wrap(-1);
	sunkenLogLabel->SetFont(panelFont);

	raisedLogSizer = new wxBoxSizer(wxVERTICAL);
	raisedLogSizer->AddStretchSpacer();
	raisedLogSizer->Add(raisedLogLabel, 0, wxALIGN_CENTER, 1);
	raisedLogSizer->AddStretchSpacer();

	sunkenLogSizer = new wxBoxSizer(wxVERTICAL);
	sunkenLogSizer->AddStretchSpacer();
	sunkenLogSizer->Add(sunkenLogLabel, 0, wxALIGN_CENTER, 1);
	sunkenLogSizer->AddStretchSpacer();

	raisedLogPanel_->SetSizer(raisedLogSizer);
	raisedLogPanel_->Layout();
	sunkenLogPanel_->SetSizer(sunkenLogSizer);
	sunkenLogPanel_->Layout();

	isLogPressed_ = false;
	enterLogPanel(false);
	redrawLogPanel();

	/*
	 * setup RuleEditor panels (aka toggle button)
	 */
	raisedRuleEditorPanel_ = new wxPanel(this, wxID_ANY,
	    wxDefaultPosition, wxSize(70, 18), wxRAISED_BORDER);
	sunkenRuleEditorPanel_ = new wxPanel(this, wxID_ANY,
	    wxDefaultPosition, wxSize(70, 18), wxSUNKEN_BORDER);

	raisedRuleEditorLabel = new wxStaticText(raisedRuleEditorPanel_,
	    wxID_ANY, _("RuleEditor"));
	raisedRuleEditorLabel->Wrap(-1);
	raisedRuleEditorLabel->SetFont(panelFont);
	sunkenRuleEditorLabel = new wxStaticText(sunkenRuleEditorPanel_,
	    wxID_ANY, _("RuleEditor"));
	sunkenRuleEditorLabel->Wrap(-1);
	sunkenRuleEditorLabel->SetFont(panelFont);

	raisedRuleEditorSizer = new wxBoxSizer(wxVERTICAL);
	raisedRuleEditorSizer->AddStretchSpacer();
	raisedRuleEditorSizer->Add(raisedRuleEditorLabel, 0, wxALIGN_CENTER, 1);
	raisedRuleEditorSizer->AddStretchSpacer();

	sunkenRuleEditorSizer = new wxBoxSizer(wxVERTICAL);
	sunkenRuleEditorSizer->AddStretchSpacer();
	sunkenRuleEditorSizer->Add(sunkenRuleEditorLabel, 0, wxALIGN_CENTER, 1);
	sunkenRuleEditorSizer->AddStretchSpacer();

	raisedRuleEditorPanel_->SetSizer(raisedRuleEditorSizer);
	raisedRuleEditorPanel_->Layout();
	sunkenRuleEditorPanel_->SetSizer(sunkenRuleEditorSizer);
	sunkenRuleEditorPanel_->Layout();

	isRuleEditorPressed_ = false;
	enterRuleEditorPanel(false);
	redrawRuleEditorPanel();

	/*
	 * connect evnets
	 */
	this->Connect(wxEVT_SIZE, wxSizeEventHandler(AnStatusBar::OnSize));

	/*
	 * connect the mouse down events
	 * We need to connect both the panel and the static text
	 * to make the whole field clickable
	 */
	raisedLogPanel_->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnLogClick), NULL, this);
	raisedLogLabel->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnLogClick), NULL, this);
	sunkenLogPanel_->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnLogClick), NULL, this);
	sunkenLogLabel->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnLogClick), NULL, this);

	raisedLogPanel_->Connect(wxEVT_ENTER_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnLogEnter), NULL, this);
	raisedLogPanel_->Connect(wxEVT_LEAVE_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnLogEnter), NULL, this);

	sunkenLogPanel_->Connect(wxEVT_ENTER_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnLogEnter), NULL, this);
	sunkenLogPanel_->Connect(wxEVT_LEAVE_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnLogEnter), NULL, this);

	raisedRuleEditorPanel_->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnRuleEditorClick), NULL, this);
	raisedRuleEditorLabel->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnRuleEditorClick), NULL, this);
	sunkenRuleEditorPanel_->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnRuleEditorClick), NULL, this);
	sunkenRuleEditorLabel->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnRuleEditorClick), NULL, this);

	raisedRuleEditorPanel_->Connect(wxEVT_ENTER_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnRuleEditorEnter), NULL, this);
	raisedRuleEditorPanel_->Connect(wxEVT_LEAVE_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnRuleEditorEnter), NULL, this);

	sunkenRuleEditorPanel_->Connect(wxEVT_ENTER_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnRuleEditorEnter), NULL, this);
	sunkenRuleEditorPanel_->Connect(wxEVT_LEAVE_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnRuleEditorEnter), NULL, this);
}

AnStatusBar::~AnStatusBar(void)
{
	delete raisedLogPanel_;
	delete sunkenLogPanel_;
	delete raisedRuleEditorPanel_;
	delete sunkenRuleEditorPanel_;
}

void
AnStatusBar::enterLogPanel(bool isInside)
{
	wxColour highlight, normal, active;

	highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT);
	normal = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWFRAME);
	active = wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION);

	if (isInside) {
		raisedLogPanel_->SetBackgroundColour(highlight);
		sunkenLogPanel_->SetBackgroundColour(highlight);
	} else {
		raisedLogPanel_->SetBackgroundColour(normal);
		sunkenLogPanel_->SetBackgroundColour(active);
	}
}

void
AnStatusBar::redrawLogPanel(void)
{
	if (isLogPressed_) {
		raisedLogPanel_->Hide();
		sunkenLogPanel_->Show();
	} else {
		sunkenLogPanel_->Hide();
		raisedLogPanel_->Show();
	}
}

void
AnStatusBar::enterRuleEditorPanel(bool isInside)
{
	wxColour highlight, normal, active;

	highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT);
	normal = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWFRAME);
	active = wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION);

	if (isInside) {
		raisedRuleEditorPanel_->SetBackgroundColour(highlight);
		sunkenRuleEditorPanel_->SetBackgroundColour(highlight);
	} else {
		raisedRuleEditorPanel_->SetBackgroundColour(normal);
		sunkenRuleEditorPanel_->SetBackgroundColour(active);
	}
}

void
AnStatusBar::redrawRuleEditorPanel(void)
{
	if (isRuleEditorPressed_) {
		raisedRuleEditorPanel_->Hide();
		sunkenRuleEditorPanel_->Show();
	} else {
		sunkenRuleEditorPanel_->Hide();
		raisedRuleEditorPanel_->Show();
	}
}

/*
 * Place the buttons in the statusbar when the window is resized.
 * This method is also called several times when the application starts.
 */
void
AnStatusBar::OnSize(wxSizeEvent& event)
{
	wxRect fieldRectangle;
	wxSize panelSize;
	int x, y;

	/* placing 'Log' panels */
	GetFieldRect(FIELD_IDX_LOG, fieldRectangle);

	panelSize = raisedLogPanel_->GetSize();
	computePanelPosition(x, y, fieldRectangle, panelSize);
	raisedLogPanel_->Move(x, y);

	panelSize = sunkenLogPanel_->GetSize();
	computePanelPosition(x, y, fieldRectangle, panelSize);
	sunkenLogPanel_->Move(x, y);

	/* placing 'Rule Editor' panels */
	GetFieldRect(FIELD_IDX_RULEEDITOR, fieldRectangle);

	panelSize = raisedRuleEditorPanel_->GetSize();
	computePanelPosition(x, y, fieldRectangle, panelSize);
	raisedRuleEditorPanel_->Move(x, y);

	panelSize = sunkenRuleEditorPanel_->GetSize();
	computePanelPosition(x, y, fieldRectangle, panelSize);
	sunkenRuleEditorPanel_->Move(x, y);

	event.Skip();
}

void
AnStatusBar::OnLogClick(wxMouseEvent& event)
{
	isLogPressed_ = !isLogPressed_;
	redrawLogPanel();
	event.Skip();
}

void
AnStatusBar::OnLogEnter(wxMouseEvent& event)
{
	enterLogPanel(event.Entering());
	event.Skip();
}

void
AnStatusBar::OnRuleEditorClick(wxMouseEvent& event)
{
	isRuleEditorPressed_ = !isRuleEditorPressed_;
	redrawRuleEditorPanel();
	event.Skip();
}

void
AnStatusBar::OnRuleEditorEnter(wxMouseEvent& event)
{
	enterRuleEditorPanel(event.Entering());
	event.Skip();
}

void
AnStatusBar::setLogVisability(bool visible)
{
	isLogPressed_ = visible;
	redrawLogPanel();
}

void
AnStatusBar::setRuleEditorVisability(bool visible)
{
	isRuleEditorPressed_ = visible;
	redrawRuleEditorPanel();
}
