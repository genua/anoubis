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

#include "ModAnoubisPanelsBase.h"

///////////////////////////////////////////////////////////////////////////

ModAnoubisMainPanelBase::ModAnoubisMainPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	sz_MainAnoubisMain = new wxBoxSizer( wxVERTICAL );
	
	tx_MainHeadline = new wxStaticText( this, wxID_ANY, _("Main Panel of Module Anoubis"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_MainHeadline->Wrap( -1 );
	tx_MainHeadline->SetFont( wxFont( 16, 70, 90, 90, false, wxEmptyString ) );
	
	sz_MainAnoubisMain->Add( tx_MainHeadline, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* sz_type;
	sz_type = new wxBoxSizer( wxHORIZONTAL );
	
	tx_type = new wxStaticText( this, wxID_ANY, _("Typ:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_type->Wrap( -1 );
	sz_type->Add( tx_type, 0, wxALIGN_CENTER|wxALL|wxFIXED_MINSIZE, 5 );
	
	wxString ch_typeChoices[] = { _("aktuelle Nachfragen"), _("Meldungen"), _("beantwortete Nachfragen"), _("alle") };
	int ch_typeNChoices = sizeof( ch_typeChoices ) / sizeof( wxString );
	ch_type = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, ch_typeNChoices, ch_typeChoices, 0 );
	sz_type->Add( ch_type, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sz_MainAnoubisMain->Add( sz_type, 0, wxALIGN_CENTER|wxALL|wxFIXED_MINSIZE, 5 );
	
	wxBoxSizer* sz_navigate;
	sz_navigate = new wxBoxSizer( wxHORIZONTAL );
	
	
	sz_navigate->Add( 40, 0, 1, wxALL|wxEXPAND, 5 );
	
	bt_first = new wxButton( this, wxID_ANY, _("<<"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_first->Enable( false );
	
	sz_navigate->Add( bt_first, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bt_previous = new wxButton( this, wxID_ANY, _("<"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_previous->Enable( false );
	
	sz_navigate->Add( bt_previous, 0, wxALIGN_CENTER|wxALL, 5 );
	
	tx_currNumber = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_currNumber->Wrap( -1 );
	sz_navigate->Add( tx_currNumber, 0, wxALIGN_CENTER|wxALL, 5 );
	
	tx_delimiter = new wxStaticText( this, wxID_ANY, _(" / "), wxDefaultPosition, wxDefaultSize, 0 );
	tx_delimiter->Wrap( -1 );
	sz_navigate->Add( tx_delimiter, 0, wxALIGN_CENTER|wxALL, 5 );
	
	tx_maxNumber = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_maxNumber->Wrap( -1 );
	sz_navigate->Add( tx_maxNumber, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bt_next = new wxButton( this, wxID_ANY, _(">"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_next->Enable( false );
	
	sz_navigate->Add( bt_next, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bt_last = new wxButton( this, wxID_ANY, _(">>"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_last->Enable( false );
	
	sz_navigate->Add( bt_last, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sz_navigate->Add( 40, 0, 1, wxALL|wxEXPAND, 5 );
	
	sz_MainAnoubisMain->Add( sz_navigate, 0, wxALIGN_CENTER, 5 );
	
	wxGridSizer* sz_info;
	sz_info = new wxGridSizer( 2, 2, 0, 0 );
	
	tx_fieldSlot1 = new wxStaticText( this, wxID_ANY, _(":"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_fieldSlot1->Wrap( -1 );
	sz_info->Add( tx_fieldSlot1, 0, wxALL, 5 );
	
	tx_valueSlot1 = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_valueSlot1->Wrap( -1 );
	sz_info->Add( tx_valueSlot1, 0, wxALL, 5 );
	
	tx_fieldSlot2 = new wxStaticText( this, wxID_ANY, _(":"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_fieldSlot2->Wrap( -1 );
	sz_info->Add( tx_fieldSlot2, 0, wxALL, 5 );
	
	tx_valueSlot2 = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_valueSlot2->Wrap( -1 );
	sz_info->Add( tx_valueSlot2, 0, wxALL, 5 );
	
	tx_fieldSlot3 = new wxStaticText( this, wxID_ANY, _(":"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_fieldSlot3->Wrap( -1 );
	sz_info->Add( tx_fieldSlot3, 0, wxALL, 5 );
	
	tx_valueSlot3 = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_valueSlot3->Wrap( -1 );
	sz_info->Add( tx_valueSlot3, 0, wxALL, 5 );
	
	sz_MainAnoubisMain->Add( sz_info, 0, wxEXPAND, 5 );
	
	tx_answerValue = new wxStaticText( this, wxID_ANY, _("Diese Nachricht wurde beantwortet."), wxDefaultPosition, wxDefaultSize, 0 );
	tx_answerValue->Wrap( -1 );
	sz_MainAnoubisMain->Add( tx_answerValue, 0, wxALL, 5 );
	
	pn_question = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxGridSizer* sz_question;
	sz_question = new wxGridSizer( 4, 3, 0, 0 );
	
	tx_question = new wxStaticText( pn_question, wxID_ANY, _("Diese Nachricht"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_question->Wrap( -1 );
	sz_question->Add( tx_question, 0, wxALL, 5 );
	
	wxBoxSizer* sz_number;
	sz_number = new wxBoxSizer( wxHORIZONTAL );
	
	rb_number = new wxRadioButton( pn_question, wxID_ANY, _("Anzahl"), wxDefaultPosition, wxDefaultSize, 0 );
	rb_number->SetMinSize( wxSize( 100,-1 ) );
	
	sz_number->Add( rb_number, 0, wxALIGN_CENTER|wxALL, 1 );
	
	sc_number = new wxSpinCtrl( pn_question, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxSP_WRAP, 1, 999, 1 );
	sc_number->SetMinSize( wxSize( 50,-1 ) );
	
	sz_number->Add( sc_number, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	
	sz_question->Add( sz_number, 0, 0, 5 );
	
	
	sz_question->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sz_question->Add( 0, 0, 1, wxEXPAND, 5 );
	
	rb_procend = new wxRadioButton( pn_question, wxID_ANY, _("bis Prozess Ende"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question->Add( rb_procend, 0, wxALL, 1 );
	
	
	sz_question->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sz_question->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_time;
	sz_time = new wxBoxSizer( wxHORIZONTAL );
	
	rb_time = new wxRadioButton( pn_question, wxID_ANY, _("fuer "), wxDefaultPosition, wxDefaultSize, 0 );
	rb_time->SetMinSize( wxSize( 100,-1 ) );
	
	sz_time->Add( rb_time, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1 );
	
	sc_time = new wxSpinCtrl( pn_question, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxSP_WRAP, 1, 999, 1 );
	sc_time->SetMinSize( wxSize( 50,-1 ) );
	
	sz_time->Add( sc_time, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	
	wxString ch_timeChoices[] = { _("Sekunde"), _("Minute"), _("Stunde"), _("Tag") };
	int ch_timeNChoices = sizeof( ch_timeChoices ) / sizeof( wxString );
	ch_time = new wxChoice( pn_question, wxID_ANY, wxDefaultPosition, wxDefaultSize, ch_timeNChoices, ch_timeChoices, 0 );
	sz_time->Add( ch_time, 0, wxALIGN_CENTER|wxALL, 1 );
	
	sz_question->Add( sz_time, 0, 0, 1 );
	
	bt_allow = new wxButton( pn_question, wxID_ANY, _("erlauben"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question->Add( bt_allow, 0, wxALL|wxALIGN_RIGHT, 1 );
	
	
	sz_question->Add( 0, 0, 1, wxEXPAND, 5 );
	
	rb_always = new wxRadioButton( pn_question, wxID_ANY, _("immer"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question->Add( rb_always, 0, wxALL, 1 );
	
	bt_deny = new wxButton( pn_question, wxID_ANY, _("verbieten"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question->Add( bt_deny, 0, wxALL|wxALIGN_RIGHT, 1 );
	
	pn_question->SetSizer( sz_question );
	pn_question->Layout();
	sz_question->Fit( pn_question );
	sz_MainAnoubisMain->Add( pn_question, 0, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( sz_MainAnoubisMain );
	this->Layout();
	sz_MainAnoubisMain->Fit( this );
	
	// Connect Events
	ch_type->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnTypeChoosen ), NULL, this );
	bt_first->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnFirstBtnClick ), NULL, this );
	bt_previous->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnPreviousBtnClick ), NULL, this );
	bt_next->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnNextBtnClick ), NULL, this );
	bt_last->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnLastBtnClick ), NULL, this );
	bt_allow->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnAllowBtnClick ), NULL, this );
	bt_deny->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnDenyBtnClick ), NULL, this );
}

ModAnoubisOverviewPanelBase::ModAnoubisOverviewPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* sz_OverviewAnoubisMain;
	sz_OverviewAnoubisMain = new wxBoxSizer( wxVERTICAL );
	
	tx_OVMainHeadline = new wxStaticText( this, wxID_ANY, _("Show overview information of module Anoubis"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_OVMainHeadline->Wrap( -1 );
	sz_OverviewAnoubisMain->Add( tx_OVMainHeadline, 0, wxALIGN_TOP|wxALL, 5 );
	
	this->SetSizer( sz_OverviewAnoubisMain );
	this->Layout();
	sz_OverviewAnoubisMain->Fit( this );
}
