/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>

#include "AnFader.h"

AnFader::AnFader(wxWindow *parent) : wxPanel(parent)
{
	wxBoxSizer	*sz_h;
	wxBoxSizer	*sz_v;

	sz_h = new wxBoxSizer(wxHORIZONTAL);
	sz_v = new wxBoxSizer(wxVERTICAL);
	slider = new wxSlider(this, wxID_ANY, 0, 0, 5, wxDefaultPosition,
	    wxDefaultSize, wxSL_VERTICAL);

	tx_p0 = new wxStaticText(this, wxID_ANY, _("high"));
	tx_p1 = new wxStaticText(this, wxID_ANY, _("medium"));
	tx_p2 = new wxStaticText(this, wxID_ANY, _("admin (normal)"));
	tx_p3 = new wxStaticText(this, wxID_ANY, _("off"));

	hlight = wxFont(10, 74, 90, 92, false, wxT("Sans"));
	normal = wxFont(10, 74, 90, 90, false, wxT("Sans"));

	slider->SetRange(0, 3);
	slider->SetPageSize(4);
	slider->SetValue(0);
	tx_p0->SetFont(hlight);

	sz_h->Add(slider, 0, wxALL | wxEXPAND, 1);
	sz_h->Add(sz_v, 0, wxALL | wxEXPAND, 1);

	sz_v->Add(tx_p0, 5, wxALIGN_LEFT | wxEXPAND, 1);
	sz_v->Add(tx_p1, 5, wxALIGN_LEFT | wxEXPAND, 1);
	sz_v->Add(tx_p2, 5, wxALIGN_LEFT | wxEXPAND, 1);
	sz_v->Add(tx_p3, 5, wxALIGN_LEFT | wxEXPAND, 1);

	SetSizer(sz_h);
	Layout();

	Connect(wxEVT_SCROLL_CHANGED, wxScrollEventHandler(AnFader::OnFade),
	    NULL, this );
	Connect(wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(AnFader::OnFade),
	    NULL, this );
}

AnFader::~AnFader()
{
	/* Destructor */
}

void
AnFader::update(int value)
{
	tx_p0->SetFont(normal);
	tx_p1->SetFont(normal);
	tx_p2->SetFont(normal);
	tx_p3->SetFont(normal);

	switch (value) {
	case 0:
		tx_p0->SetFont(hlight);
		break;
	case 1:
		tx_p1->SetFont(hlight);
		break;
	case 2:
		tx_p2->SetFont(hlight);
		break;
	case 3:
		tx_p3->SetFont(hlight);
		break;
	}
}

void
AnFader::OnFade(wxScrollEvent& event)
{
	update(event.GetPosition());
}

void
AnFader::setValue(int value)
{
	slider->SetValue(value);
	update(value);
}

int
AnFader::getValue(void)
{
	return (slider->GetValue());
}
