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
///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep 28 2007)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "ModAlfPanelsBase.h"

///////////////////////////////////////////////////////////////////////////

ModAlfMainPanelBase::ModAlfMainPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	sz_MainALFMain = new wxBoxSizer( wxVERTICAL );
	
	tx_MainHeadline = new wxStaticText( this, wxID_ANY, _("Main Panel of Module ALF"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_MainHeadline->Wrap( -1 );
	tx_MainHeadline->SetFont( wxFont( 16, 70, 90, 90, false, wxEmptyString ) );
	
	sz_MainALFMain->Add( tx_MainHeadline, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	note_MainAlf = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	pan_Rules = new wxPanel( note_MainAlf, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_AlfRules;
	sz_AlfRules = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* sz_RulesNS;
	sz_RulesNS = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* sz_RulesWE;
	sz_RulesWE = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* sz_Rules;
	sz_Rules = new wxBoxSizer( wxVERTICAL );
	
	lst_Rules = new wxListCtrl( pan_Rules, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL );
	sz_Rules->Add( lst_Rules, 1, wxALL|wxEXPAND, 5 );
	
	sz_RulesWE->Add( sz_Rules, 66, wxEXPAND, 5 );
	
	wxBoxSizer* sz_RulesOperations;
	sz_RulesOperations = new wxBoxSizer( wxVERTICAL );
	
	tx_RulesOperation1stHeader = new wxStaticText( pan_Rules, wxID_ANY, _("Rule:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_RulesOperation1stHeader->Wrap( -1 );
	sz_RulesOperations->Add( tx_RulesOperation1stHeader, 0, wxALL, 5 );
	
	ln_RulesOperationSep = new wxStaticLine( pan_Rules, wxID_RulesOperationSep, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	sz_RulesOperations->Add( ln_RulesOperationSep, 0, wxEXPAND | wxALL, 5 );
	
	tx_RulesOperation2ndHeader = new wxStaticText( pan_Rules, wxID_ANY, _("Information:\n\nThis outlines the rules of the ALF.\nThey are displayed depending on\ntheir priority."), wxDefaultPosition, wxDefaultSize, 0 );
	tx_RulesOperation2ndHeader->Wrap( -1 );
	sz_RulesOperations->Add( tx_RulesOperation2ndHeader, 0, wxALL, 5 );
	
	sz_RulesWE->Add( sz_RulesOperations, 34, wxEXPAND, 5 );
	
	sz_RulesNS->Add( sz_RulesWE, 1, wxEXPAND, 5 );
	
	sz_AlfRules->Add( sz_RulesNS, 1, wxEXPAND, 5 );
	
	pan_Rules->SetSizer( sz_AlfRules );
	pan_Rules->Layout();
	sz_AlfRules->Fit( pan_Rules );
	note_MainAlf->AddPage( pan_Rules, _("Rules"), false );
	
	sz_MainALFMain->Add( note_MainAlf, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( sz_MainALFMain );
	this->Layout();
}

ModAlfOverviewPanelBase::ModAlfOverviewPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetToolTip( _("The level of restrictions increases with the\nprofile chosen.\nRules and restrictions of a lower profile\nare automatically valid in all profiles above.\nAll settings you choose are contingent upon\nthe chosen profile.\nIt may occur that you can't switch off the\nmodule because of restrictions.") );
	
	wxBoxSizer* sz_OverviewALFMain;
	sz_OverviewALFMain = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* sz_OVALF;
	sz_OVALF = new wxFlexGridSizer( 2, 4, 0, 0 );
	sz_OVALF->SetFlexibleDirection( wxBOTH );
	sz_OVALF->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	
	sz_OVALF->Add( 20, 0, 1, wxEXPAND, 5 );
	
	alfStatusIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	sz_OVALF->Add( alfStatusIcon, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* sz_lables;
	sz_lables = new wxBoxSizer( wxVERTICAL );
	
	txt_status = new wxStaticText( this, wxID_ANY, _("status (ALF):"), wxDefaultPosition, wxDefaultSize, 0 );
	txt_status->Wrap( -1 );
	sz_lables->Add( txt_status, 0, wxALL, 5 );
	
	txt_nachfragen = new wxStaticText( this, wxID_ANY, _("open requests (ALF):"), wxDefaultPosition, wxDefaultSize, 0 );
	txt_nachfragen->Wrap( -1 );
	sz_lables->Add( txt_nachfragen, 0, wxALL, 5 );
	
	sz_OVALF->Add( sz_lables, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_values;
	sz_values = new wxBoxSizer( wxVERTICAL );
	
	txt_statusValue = new wxStaticText( this, wxID_ANY, _("everything OK"), wxDefaultPosition, wxDefaultSize, 0 );
	txt_statusValue->Wrap( -1 );
	sz_values->Add( txt_statusValue, 0, wxALL, 5 );
	
	txt_requestValue = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	txt_requestValue->Wrap( -1 );
	sz_values->Add( txt_requestValue, 0, wxALL, 5 );
	
	sz_OVALF->Add( sz_values, 1, wxEXPAND, 5 );
	
	sz_OverviewALFMain->Add( sz_OVALF, 0, wxEXPAND, 5 );
	
	this->SetSizer( sz_OverviewALFMain );
	this->Layout();
}
