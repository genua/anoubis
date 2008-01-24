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
#include "main.h"

/*
 * This macro calculates the (maybe) new position (x,y) within a given
 * field rectangle and panel size. Additionally 2 pixels are added to
 * every panels y value due accurate Rectangle placement.
 */
#define computePanelPosition(newx, newy, field, panel) \
	do { \
		newx = field.x + (field.width  - panel.x) / 2;     \
		newy = field.y + (field.height - panel.y) / 2 + 2; \
	} while (0)

AnStatusBar::AnStatusBar(wxWindow *parent) : wxStatusBar(parent, ID_STATUSBAR)
{
	wxBoxSizer	*raisedLogViewerSizer;
	wxBoxSizer	*sunkenLogViewerSizer;
	wxStaticText	*raisedLogViewerLabel;
	wxStaticText	*sunkenLogViewerLabel;
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
	 * setup LogViewer panels (aka toggle button)
	 */
	raisedLogViewerPanel_ = new wxPanel(this, wxID_ANY, wxDefaultPosition,
	    wxSize(70, 18), wxRAISED_BORDER);
	sunkenLogViewerPanel_ = new wxPanel(this, wxID_ANY, wxDefaultPosition,
	    wxSize(70, 18), wxSUNKEN_BORDER);

	raisedLogViewerLabel = new wxStaticText(raisedLogViewerPanel_,
	    wxID_ANY, _("LogViewer"));
	raisedLogViewerLabel->Wrap(-1);
	raisedLogViewerLabel->SetFont(panelFont);
	sunkenLogViewerLabel = new wxStaticText(sunkenLogViewerPanel_,
	    wxID_ANY, _("LogViewer"));
	sunkenLogViewerLabel->Wrap(-1);
	sunkenLogViewerLabel->SetFont(panelFont);

	raisedLogViewerSizer = new wxBoxSizer(wxVERTICAL);
	raisedLogViewerSizer->AddStretchSpacer();
	raisedLogViewerSizer->Add(raisedLogViewerLabel, 0, wxALIGN_CENTER, 1);
	raisedLogViewerSizer->AddStretchSpacer();

	sunkenLogViewerSizer = new wxBoxSizer(wxVERTICAL);
	sunkenLogViewerSizer->AddStretchSpacer();
	sunkenLogViewerSizer->Add(sunkenLogViewerLabel, 0, wxALIGN_CENTER, 1);
	sunkenLogViewerSizer->AddStretchSpacer();

	raisedLogViewerPanel_->SetSizer(raisedLogViewerSizer);
	raisedLogViewerPanel_->Layout();
	sunkenLogViewerPanel_->SetSizer(sunkenLogViewerSizer);
	sunkenLogViewerPanel_->Layout();

	isLogViewerPressed_ = false;
	enterLogViewerPanel(false);
	redrawLogViewerPanel();

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
	raisedLogViewerPanel_->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnLogViewerClick), NULL, this);
	raisedLogViewerLabel->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnLogViewerClick), NULL, this);
	sunkenLogViewerPanel_->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnLogViewerClick), NULL, this);
	sunkenLogViewerLabel->Connect(wxEVT_LEFT_DOWN,
	    wxMouseEventHandler(AnStatusBar::OnLogViewerClick), NULL, this);

	raisedLogViewerPanel_->Connect(wxEVT_ENTER_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnLogViewerEnter), NULL, this);
	raisedLogViewerPanel_->Connect(wxEVT_LEAVE_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnLogViewerEnter), NULL, this);

	sunkenLogViewerPanel_->Connect(wxEVT_ENTER_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnLogViewerEnter), NULL, this);
	sunkenLogViewerPanel_->Connect(wxEVT_LEAVE_WINDOW,
	    wxMouseEventHandler(AnStatusBar::OnLogViewerEnter), NULL, this);

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
	delete raisedLogViewerPanel_;
	delete sunkenLogViewerPanel_;
	delete raisedRuleEditorPanel_;
	delete sunkenRuleEditorPanel_;
}

void
AnStatusBar::enterLogViewerPanel(bool isInside)
{
	wxColour highlight, normal, active;

	highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT);
	normal = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWFRAME);
	active = wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION);

	if (isInside) {
		raisedLogViewerPanel_->SetBackgroundColour(highlight);
		sunkenLogViewerPanel_->SetBackgroundColour(highlight);
	} else {
		raisedLogViewerPanel_->SetBackgroundColour(normal);
		sunkenLogViewerPanel_->SetBackgroundColour(active);
	}
}

void
AnStatusBar::redrawLogViewerPanel(void)
{
	if (isLogViewerPressed_) {
		raisedLogViewerPanel_->Hide();
		sunkenLogViewerPanel_->Show();
	} else {
		sunkenLogViewerPanel_->Hide();
		raisedLogViewerPanel_->Show();
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

	/* placing 'LogViewer' panels */
	GetFieldRect(FIELD_IDX_LOG, fieldRectangle);

	panelSize = raisedLogViewerPanel_->GetSize();
	computePanelPosition(x, y, fieldRectangle, panelSize);
	raisedLogViewerPanel_->Move(x, y);

	panelSize = sunkenLogViewerPanel_->GetSize();
	computePanelPosition(x, y, fieldRectangle, panelSize);
	sunkenLogViewerPanel_->Move(x, y);

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
AnStatusBar::OnLogViewerClick(wxMouseEvent& event)
{
	isLogViewerPressed_ = !isLogViewerPressed_;
	wxGetApp().setLogViewerVisability(isLogViewerPressed_);
	event.Skip();
}

void
AnStatusBar::OnLogViewerEnter(wxMouseEvent& event)
{
	enterLogViewerPanel(event.Entering());
	event.Skip();
}

void
AnStatusBar::OnRuleEditorClick(wxMouseEvent& event)
{
	isRuleEditorPressed_ = !isRuleEditorPressed_;
	wxGetApp().setRuleEditorVisability(isRuleEditorPressed_);
	event.Skip();
}

void
AnStatusBar::OnRuleEditorEnter(wxMouseEvent& event)
{
	enterRuleEditorPanel(event.Entering());
	event.Skip();
}

void
AnStatusBar::setLogViewerVisability(bool visible)
{
	isLogViewerPressed_ = visible;
	redrawLogViewerPanel();
}

void
AnStatusBar::setRuleEditorVisability(bool visible)
{
	isRuleEditorPressed_ = visible;
	redrawRuleEditorPanel();
}
