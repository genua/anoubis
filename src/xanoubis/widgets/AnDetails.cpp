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

#include "AnDetails.h"
#include "MainUtils.h"

AnDetails::AnDetails(wxWindow *parent, wxWindowID id, const wxPoint & pos,
    const wxSize & size, long style, const wxString & name)
    : wxPanel(parent, id, pos, size, style, name)
{
	wxBoxSizer* mainSizer;
	wxBoxSizer* headSizer;
	MainUtils*  utils;

	isVisible_    = false;
	contentSizer_ = NULL;
	utils	      = MainUtils::instance();
	downArrow_    = utils->loadIcon(wxT("General_downarrow_16.png"));
	rightArrow_   = utils->loadIcon(wxT("General_rightarrow_16.png"));

	mainSizer = new wxBoxSizer(wxVERTICAL);
	headSizer = new wxBoxSizer(wxHORIZONTAL);

	detailsIcon_ = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap,
	    wxDefaultPosition, wxDefaultSize, 0);
	headSizer->Add(detailsIcon_, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

	detailsLabel_ = new wxStaticText(this, wxID_ANY, name,
	    wxDefaultPosition, wxDefaultSize, 0);
	detailsLabel_->Wrap( -1 );
	headSizer->Add(detailsLabel_, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

	detailsLine_ = new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
	    wxDefaultSize, wxLI_HORIZONTAL);
	headSizer->Add(detailsLine_, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5);

	mainSizer->Add(headSizer, 0, wxEXPAND, 5);

	update();

	this->SetSizer(mainSizer);
	this->Layout();

	detailsIcon_->Connect(wxEVT_LEFT_UP,
	    wxMouseEventHandler(AnDetails::onDetailClick), NULL, this);
	detailsLabel_->Connect(wxEVT_LEFT_UP,
	    wxMouseEventHandler(AnDetails::onDetailClick), NULL, this);
}

void
AnDetails::SetSizer(wxSizer *sizer, bool deleteOld)
{
	wxSizer *mainSizer;

	mainSizer = GetSizer();
	if (mainSizer == NULL) {
		wxWindowBase::SetSizer(sizer, deleteOld);
	} else {
		if (contentSizer_ != NULL) {
			mainSizer->Detach(contentSizer_);
			if (deleteOld) {
				delete contentSizer_;
			}
		}
		mainSizer->Add(sizer, 1, wxEXPAND, 0);
		contentSizer_ = sizer;
	}

	update();
}

bool
AnDetails::detailsVisible(void) const
{
	return (isVisible_);
}

void
AnDetails::showDetails(bool visible)
{
	isVisible_ = visible;
	update();
}

void
AnDetails::onDetailClick(wxMouseEvent &event)
{
	event.Skip();
	showDetails(!isVisible_);
}

void
AnDetails::update(void)
{
	wxWindow *parent;

	parent = GetParent();

	/* modify the widgets */
	if (contentSizer_ != NULL) {
		contentSizer_->Show(isVisible_);
		contentSizer_->Layout();
	}

	if (isVisible_) {
		detailsIcon_->SetIcon(*downArrow_);
		detailsIcon_->SetToolTip(_("hide"));
		detailsLabel_->SetToolTip(_("hide"));
	} else {
		detailsIcon_->SetIcon(*rightArrow_);
		detailsIcon_->SetToolTip(_("show"));
		detailsLabel_->SetToolTip(_("show"));
	}

	/* update the view */
	if (parent == NULL) {
		Layout();
		Refresh();
	} else {
		parent->Layout();
		parent->Refresh();
	}
}
