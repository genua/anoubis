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
///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep 28 2007)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "ModSbPanelsBase.h"

///////////////////////////////////////////////////////////////////////////

ModSbMainPanelBase::ModSbMainPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	sz_MainSbMain = new wxBoxSizer( wxVERTICAL );
	
	tx_MainHeadline = new wxStaticText( this, wxID_ANY, _("Main Panel of Module SB"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_MainHeadline->Wrap( -1 );
	tx_MainHeadline->SetFont( wxFont( 16, 70, 90, 90, false, wxEmptyString ) );
	
	sz_MainSbMain->Add( tx_MainHeadline, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	note_MainSb = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	pan_Rules = new wxPanel( note_MainSb, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_SbRules;
	sz_SbRules = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* sz_Rules;
	sz_Rules = new wxBoxSizer( wxVERTICAL );
	
	lst_Rules = new wxListCtrl( pan_Rules, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL );
	sz_Rules->Add( lst_Rules, 1, wxALL|wxEXPAND, 5 );
	
	sz_SbRules->Add( sz_Rules, 66, wxEXPAND, 5 );
	
	pan_Rules->SetSizer( sz_SbRules );
	pan_Rules->Layout();
	sz_SbRules->Fit( pan_Rules );
	note_MainSb->AddPage( pan_Rules, _("Rules"), false );
	
	sz_MainSbMain->Add( note_MainSb, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( sz_MainSbMain );
	this->Layout();
}

ModSbOverviewPanelBase::ModSbOverviewPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetToolTip( _("Displays the status of the Sandbox (SB) while connected.") );
	
	wxBoxSizer* sz_OverviewSBMain;
	sz_OverviewSBMain = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* sz_OVSB;
	sz_OVSB = new wxFlexGridSizer( 2, 4, 0, 0 );
	sz_OVSB->SetFlexibleDirection( wxBOTH );
	sz_OVSB->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	
	sz_OVSB->Add( 20, 0, 1, wxEXPAND, 5 );
	
	sbStatusIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	sz_OVSB->Add( sbStatusIcon, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* sz_lables;
	sz_lables = new wxBoxSizer( wxVERTICAL );
	
	txt_status = new wxStaticText( this, wxID_ANY, _("status (SB):"), wxDefaultPosition, wxDefaultSize, 0 );
	txt_status->Wrap( -1 );
	sz_lables->Add( txt_status, 0, wxALL, 5 );
	
	txt_nachfragen = new wxStaticText( this, wxID_ANY, _("open requests (SB): "), wxDefaultPosition, wxDefaultSize, 0 );
	txt_nachfragen->Wrap( -1 );
	sz_lables->Add( txt_nachfragen, 0, wxALL, 5 );
	
	sz_OVSB->Add( sz_lables, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_values;
	sz_values = new wxBoxSizer( wxVERTICAL );
	
	txt_statusValue = new wxStaticText( this, wxID_ANY, _("everything OK"), wxDefaultPosition, wxDefaultSize, 0 );
	txt_statusValue->Wrap( -1 );
	sz_values->Add( txt_statusValue, 0, wxALL, 5 );
	
	txt_requestValue = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	txt_requestValue->Wrap( -1 );
	sz_values->Add( txt_requestValue, 0, wxALL, 5 );
	
	sz_OVSB->Add( sz_values, 1, wxEXPAND, 5 );
	
	sz_OverviewSBMain->Add( sz_OVSB, 0, wxEXPAND, 5 );
	
	this->SetSizer( sz_OverviewSBMain );
	this->Layout();
}
