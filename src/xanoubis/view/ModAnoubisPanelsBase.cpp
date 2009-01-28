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

#include "ModAnoubisPanelsBase.h"

///////////////////////////////////////////////////////////////////////////

ModAnoubisMainPanelBase::ModAnoubisMainPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	sz_MainAnoubisMain = new wxBoxSizer( wxVERTICAL );
	
	tx_MainHeadline = new wxStaticText( this, wxID_ANY, _("Main Panel of Module Anoubis"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_MainHeadline->Wrap( -1 );
	tx_MainHeadline->SetFont( wxFont( 16, 70, 90, 90, false, wxEmptyString ) );
	
	sz_MainAnoubisMain->Add( tx_MainHeadline, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 5 );
	
	tb_MainAnoubisNotify = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	tb_MainAnoubisNotification = new wxPanel( tb_MainAnoubisNotify, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_MainAnoubisNotify;
	sz_MainAnoubisNotify = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* sz_type;
	sz_type = new wxBoxSizer( wxHORIZONTAL );
	
	tx_type = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("Typ:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_type->Wrap( -1 );
	sz_type->Add( tx_type, 0, wxALIGN_CENTER|wxALL|wxFIXED_MINSIZE, 5 );
	
	wxString ch_typeChoices[] = { _("current requests"), _("messages"), _("closed requests"), _("all") };
	int ch_typeNChoices = sizeof( ch_typeChoices ) / sizeof( wxString );
	ch_type = new wxChoice( tb_MainAnoubisNotification, wxID_ANY, wxDefaultPosition, wxDefaultSize, ch_typeNChoices, ch_typeChoices, 0 );
	sz_type->Add( ch_type, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sz_MainAnoubisNotify->Add( sz_type, 0, wxALIGN_CENTER|wxALL|wxFIXED_MINSIZE, 5 );
	
	wxBoxSizer* sz_navigate;
	sz_navigate = new wxBoxSizer( wxHORIZONTAL );
	
	
	sz_navigate->Add( 40, 0, 1, wxALL|wxEXPAND, 5 );
	
	bt_first = new wxButton( tb_MainAnoubisNotification, wxID_ANY, _("<<"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_first->Enable( false );
	
	sz_navigate->Add( bt_first, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bt_previous = new wxButton( tb_MainAnoubisNotification, wxID_ANY, _("<"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_previous->Enable( false );
	
	sz_navigate->Add( bt_previous, 0, wxALIGN_CENTER|wxALL, 5 );
	
	tx_currNumber = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("00000000"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_currNumber->Wrap( -1 );
	sz_navigate->Add( tx_currNumber, 0, wxALIGN_CENTER|wxALL, 5 );
	
	tx_delimiter = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(" / "), wxDefaultPosition, wxDefaultSize, 0 );
	tx_delimiter->Wrap( -1 );
	sz_navigate->Add( tx_delimiter, 0, wxALIGN_CENTER|wxALL, 5 );
	
	tx_maxNumber = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("00000000"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_maxNumber->Wrap( -1 );
	sz_navigate->Add( tx_maxNumber, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bt_next = new wxButton( tb_MainAnoubisNotification, wxID_ANY, _(">"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_next->Enable( false );
	
	sz_navigate->Add( bt_next, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bt_last = new wxButton( tb_MainAnoubisNotification, wxID_ANY, _(">>"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_last->Enable( false );
	
	sz_navigate->Add( bt_last, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sz_navigate->Add( 40, 0, 1, wxALL|wxEXPAND, 5 );
	
	sz_MainAnoubisNotify->Add( sz_navigate, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* slotSizer;
	slotSizer = new wxFlexGridSizer( 6, 2, 0, 0 );
	slotSizer->AddGrowableCol( 0 );
	slotSizer->AddGrowableCol( 1 );
	slotSizer->SetFlexibleDirection( wxHORIZONTAL );
	slotSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );
	
	slotLabelText1 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText1->Wrap( -1 );
	slotSizer->Add( slotLabelText1, 0, wxALL, 5 );
	
	slotValueText1 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText1->Wrap( -1 );
	slotSizer->Add( slotValueText1, 0, wxALL, 5 );
	
	slotLabelText2 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText2->Wrap( -1 );
	slotSizer->Add( slotLabelText2, 0, wxALL, 5 );
	
	slotValueText2 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText2->Wrap( -1 );
	slotSizer->Add( slotValueText2, 0, wxALL, 5 );
	
	slotLabelText3 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText3->Wrap( -1 );
	slotSizer->Add( slotLabelText3, 0, wxALL, 5 );
	
	slotValueText3 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText3->Wrap( -1 );
	slotSizer->Add( slotValueText3, 0, wxALL, 5 );
	
	slotLabelText4 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText4->Wrap( -1 );
	slotSizer->Add( slotLabelText4, 0, wxALL, 5 );
	
	slotValueText4 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText4->Wrap( -1 );
	slotSizer->Add( slotValueText4, 0, wxALL, 5 );
	
	slotLabelText5 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText5->Wrap( -1 );
	slotSizer->Add( slotLabelText5, 0, wxALL, 5 );
	
	slotValueText5 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText5->Wrap( -1 );
	slotSizer->Add( slotValueText5, 0, wxALL, 5 );
	
	slotLabelText6 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText6->Wrap( -1 );
	slotSizer->Add( slotLabelText6, 0, wxALL, 5 );
	
	slotValueText6 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText6->Wrap( -1 );
	slotSizer->Add( slotValueText6, 0, wxALL, 5 );
	
	bSizer10->Add( slotSizer, 0, 0, 5 );
	
	sz_MainAnoubisNotify->Add( bSizer10, 0, wxALL, 5 );
	
	tx_answerValue = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("This message was answered."), wxDefaultPosition, wxDefaultSize, 0 );
	tx_answerValue->Wrap( -1 );
	sz_MainAnoubisNotify->Add( tx_answerValue, 0, wxALL, 5 );
	
	wxBoxSizer* sz_question_top;
	sz_question_top = new wxBoxSizer( wxVERTICAL );
	
	pn_question = new wxPanel( tb_MainAnoubisNotification, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_message;
	sz_message = new wxBoxSizer( wxVERTICAL );
	
	wxGridBagSizer* sz_question;
	sz_question = new wxGridBagSizer( 0, 0 );
	sz_question->SetFlexibleDirection( wxBOTH );
	sz_question->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	tx_question = new wxStaticText( pn_question, wxID_ANY, _("This message"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_question->Wrap( -1 );
	tx_question->SetMinSize( wxSize( 140,-1 ) );
	
	sz_question->Add( tx_question, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL, 5 );
	
	wxGridSizer* sz_question_sub;
	sz_question_sub = new wxGridSizer( 4, 3, 0, 0 );
	
	rb_number = new wxRadioButton( pn_question, wxID_ANY, _("once"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	sz_question_sub->Add( rb_number, 0, wxALL, 1 );
	
	
	sz_question_sub->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sz_question_sub->Add( 0, 0, 1, wxEXPAND, 5 );
	
	rb_procend = new wxRadioButton( pn_question, wxID_ANY, _("till process ends"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question_sub->Add( rb_procend, 0, wxALL, 1 );
	
	
	sz_question_sub->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sz_question_sub->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_time;
	sz_time = new wxBoxSizer( wxHORIZONTAL );
	
	rb_time = new wxRadioButton( pn_question, wxID_ANY, _("for "), wxDefaultPosition, wxDefaultSize, 0 );
	rb_time->SetMinSize( wxSize( 100,-1 ) );
	
	sz_time->Add( rb_time, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1 );
	
	sc_time = new wxSpinCtrl( pn_question, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxSP_WRAP, 1, 999, 1 );
	sc_time->SetMinSize( wxSize( 50,-1 ) );
	
	sz_time->Add( sc_time, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	
	wxString ch_timeChoices[] = { _("seconds"), _("minutes"), _("hours"), _("days") };
	int ch_timeNChoices = sizeof( ch_timeChoices ) / sizeof( wxString );
	ch_time = new wxChoice( pn_question, wxID_ANY, wxDefaultPosition, wxDefaultSize, ch_timeNChoices, ch_timeChoices, 0 );
	sz_time->Add( ch_time, 0, wxALIGN_CENTER|wxALL, 1 );
	
	sz_question_sub->Add( sz_time, 0, 0, 1 );
	
	bt_allow = new wxButton( pn_question, wxID_ANY, _("allow"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question_sub->Add( bt_allow, 0, wxALL|wxALIGN_RIGHT, 1 );
	
	
	sz_question_sub->Add( 0, 0, 1, wxEXPAND, 5 );
	
	rb_always = new wxRadioButton( pn_question, wxID_ANY, _("always"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question_sub->Add( rb_always, 0, wxALL, 1 );
	
	bt_deny = new wxButton( pn_question, wxID_ANY, _("deny"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question_sub->Add( bt_deny, 0, wxALL|wxALIGN_RIGHT, 1 );
	
	sz_question->Add( sz_question_sub, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	sz_message->Add( sz_question, 1, wxALL, 5 );
	
	pn_question->SetSizer( sz_message );
	pn_question->Layout();
	sz_message->Fit( pn_question );
	sz_question_top->Add( pn_question, 0, wxALIGN_TOP|wxALL, 5 );
	
	sz_MainAnoubisNotify->Add( sz_question_top, 0, wxEXPAND, 5 );
	
	tb_MainAnoubisNotification->SetSizer( sz_MainAnoubisNotify );
	tb_MainAnoubisNotification->Layout();
	sz_MainAnoubisNotify->Fit( tb_MainAnoubisNotification );
	tb_MainAnoubisNotify->AddPage( tb_MainAnoubisNotification, _("Notifications"), true );
	tb_Profiles = new wxPanel( tb_MainAnoubisNotify, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer( wxVERTICAL );
	
	profileList = new wxListCtrl( tb_Profiles, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL );
	bSizer26->Add( profileList, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText35 = new wxStaticText( tb_Profiles, wxID_ANY, _("Profile:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText35->Wrap( -1 );
	bSizer29->Add( m_staticText35, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	selectedProfileText = new wxStaticText( tb_Profiles, wxID_ANY, _("none"), wxDefaultPosition, wxDefaultSize, 0 );
	selectedProfileText->Wrap( -1 );
	bSizer29->Add( selectedProfileText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer29->Add( 0, 0, 1, wxEXPAND, 5 );
	
	profileDeleteButton = new wxButton( tb_Profiles, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	profileDeleteButton->Enable( false );
	
	bSizer29->Add( profileDeleteButton, 0, wxALL, 5 );
	
	bSizer26->Add( bSizer29, 0, wxEXPAND, 5 );
	
	bSizer25->Add( bSizer26, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText38 = new wxStaticText( tb_Profiles, wxID_ANY, _("Last loaded profile:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText38->Wrap( -1 );
	bSizer30->Add( m_staticText38, 0, wxALL, 5 );
	
	loadedProfileText = new wxStaticText( tb_Profiles, wxID_ANY, _("none"), wxDefaultPosition, wxDefaultSize, 0 );
	loadedProfileText->Wrap( -1 );
	bSizer30->Add( loadedProfileText, 0, wxALL, 5 );
	
	bSizer27->Add( bSizer30, 0, 0, 5 );
	
	profileLoadButton = new wxButton( tb_Profiles, wxID_ANY, _("Load"), wxDefaultPosition, wxDefaultSize, 0 );
	profileLoadButton->Enable( false );
	profileLoadButton->SetToolTip( _("Load the selected profile into the rule editor.") );
	
	bSizer27->Add( profileLoadButton, 0, wxALL, 5 );
	
	profileSaveButton = new wxButton( tb_Profiles, wxID_ANY, _("Save..."), wxDefaultPosition, wxDefaultSize, 0 );
	profileSaveButton->SetToolTip( _("Save the rule set that is currently loaded into the rule editor as a profile.") );
	
	bSizer27->Add( profileSaveButton, 0, wxALL, 5 );
	
	
	bSizer27->Add( 0, 20, 0, wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( tb_Profiles, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer27->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );
	
	m_staticText40 = new wxStaticText( tb_Profiles, wxID_ANY, _("Saving a profile does not activate it! You need to store (activate) the profile, if you want to use it."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText40->Wrap( 250 );
	bSizer27->Add( m_staticText40, 0, wxALL, 5 );
	
	profileActivateButton = new wxButton( tb_Profiles, wxID_ANY, _("Store (Activate)"), wxDefaultPosition, wxDefaultSize, 0 );
	profileActivateButton->Enable( false );
	profileActivateButton->SetToolTip( _("Activate the selected profile.") );
	
	bSizer27->Add( profileActivateButton, 0, wxALL, 5 );
	
	bSizer25->Add( bSizer27, 0, wxEXPAND, 5 );
	
	tb_Profiles->SetSizer( bSizer25 );
	tb_Profiles->Layout();
	bSizer25->Fit( tb_Profiles );
	tb_MainAnoubisNotify->AddPage( tb_Profiles, _("Profiles"), false );
	tb_MainAnoubisVersions = new wxPanel( tb_MainAnoubisNotify, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* MainAnoubisVersionsSizer;
	MainAnoubisVersionsSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* VersionListSizer;
	VersionListSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText41 = new wxStaticText( tb_MainAnoubisVersions, wxID_ANY, _("Choose policy:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	bSizer31->Add( m_staticText41, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	VersionActivePolicyRadioButton = new wxRadioButton( tb_MainAnoubisVersions, wxID_ANY, _("Active policy"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer31->Add( VersionActivePolicyRadioButton, 0, wxALL, 5 );
	
	VersionProfilePolicyRadioButton = new wxRadioButton( tb_MainAnoubisVersions, wxID_ANY, _("Policy from profile"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer31->Add( VersionProfilePolicyRadioButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString VersionProfileChoiceChoices;
	VersionProfileChoice = new wxChoice( tb_MainAnoubisVersions, wxID_ANY, wxDefaultPosition, wxDefaultSize, VersionProfileChoiceChoices, 0 );
	VersionProfileChoice->Enable( false );
	
	bSizer31->Add( VersionProfileChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	VersionListSizer->Add( bSizer31, 0, wxALL|wxEXPAND, 5 );
	
	VersionListCtrl = new wxListCtrl( tb_MainAnoubisVersions, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT );
	VersionListSizer->Add( VersionListCtrl, 80, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* VersionCommentSizer;
	VersionCommentSizer = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisVersions, -1, _("Comment of selected version:") ), wxVERTICAL );
	
	VersionShowCommentTextCtrl = new wxTextCtrl( tb_MainAnoubisVersions, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	VersionShowCommentTextCtrl->Enable( false );
	
	VersionCommentSizer->Add( VersionShowCommentTextCtrl, 1, wxALL|wxEXPAND, 5 );
	
	VersionListSizer->Add( VersionCommentSizer, 20, wxEXPAND, 5 );
	
	MainAnoubisVersionsSizer->Add( VersionListSizer, 1, 0, 5 );
	
	wxBoxSizer* VersionButtonSizer;
	VersionButtonSizer = new wxBoxSizer( wxVERTICAL );
	
	
	VersionButtonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	VersionRestoreButton = new wxButton( tb_MainAnoubisVersions, wxID_ANY, _("restore"), wxDefaultPosition, wxDefaultSize, 0 );
	VersionRestoreButton->SetToolTip( _("Restore the selected version of the profile.") );
	
	VersionButtonSizer->Add( VersionRestoreButton, 0, wxALL, 5 );
	
	VersionExportButton1 = new wxButton( tb_MainAnoubisVersions, wxID_ANY, _("export ..."), wxDefaultPosition, wxDefaultSize, 0 );
	VersionExportButton1->SetToolTip( _("Export the selected Version of the profile to a file.#") );
	
	VersionButtonSizer->Add( VersionExportButton1, 0, wxALL, 5 );
	
	VersionDeleteButton = new wxButton( tb_MainAnoubisVersions, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, 0 );
	VersionDeleteButton->SetToolTip( _("Delete the selected version of the profile.") );
	
	VersionButtonSizer->Add( VersionDeleteButton, 0, wxALL, 5 );
	
	VersionShowButton = new wxButton( tb_MainAnoubisVersions, wxID_ANY, _("show"), wxDefaultPosition, wxDefaultSize, 0 );
	VersionShowButton->Hide();
	
	VersionButtonSizer->Add( VersionShowButton, 0, wxALL, 5 );
	
	
	VersionButtonSizer->Add( 0, 0, 3, wxEXPAND, 5 );
	
	MainAnoubisVersionsSizer->Add( VersionButtonSizer, 0, wxEXPAND, 5 );
	
	tb_MainAnoubisVersions->SetSizer( MainAnoubisVersionsSizer );
	tb_MainAnoubisVersions->Layout();
	MainAnoubisVersionsSizer->Fit( tb_MainAnoubisVersions );
	tb_MainAnoubisNotify->AddPage( tb_MainAnoubisVersions, _("Version control"), false );
	tb_MainAnoubisOptions = new wxPanel( tb_MainAnoubisNotify, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_MainAnoubisOptions;
	sz_MainAnoubisOptions = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("Escalation Notifier Settings") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 3, 2, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	cb_SendEscalations = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("send Escalations"), wxDefaultPosition, wxDefaultSize, 0 );
	cb_SendEscalations->SetValue(true);
	
	cb_SendEscalations->SetToolTip( _("Check to disable the messaging via Notifier") );
	
	fgSizer2->Add( cb_SendEscalations, 0, wxALL, 5 );
	
	
	fgSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	cb_NoEscalationTimeout = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("No Timeout"), wxDefaultPosition, wxDefaultSize, 0 );
	cb_NoEscalationTimeout->SetValue(true);
	
	cb_NoEscalationTimeout->SetToolTip( _("Check to let the Notifier never timeout") );
	
	fgSizer2->Add( cb_NoEscalationTimeout, 0, wxALL, 5 );
	
	
	fgSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );
	
	m_spinEscalationNotifyTimeout = new wxSpinCtrl( tb_MainAnoubisOptions, wxID_ANY, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 120, 1 );
	m_spinEscalationNotifyTimeout->Enable( false );
	
	bSizer14->Add( m_spinEscalationNotifyTimeout, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	tx_EscalationNotifyTimeoutLabel = new wxStaticText( tb_MainAnoubisOptions, wxID_ANY, _("Timeout in Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_EscalationNotifyTimeoutLabel->Wrap( -1 );
	tx_EscalationNotifyTimeoutLabel->Enable( false );
	
	bSizer14->Add( tx_EscalationNotifyTimeoutLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	fgSizer2->Add( bSizer14, 1, wxEXPAND, 5 );
	
	sbSizer3->Add( fgSizer2, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer3, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("Alert Notifier Settings") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	cb_SendAlerts = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("send Alerts"), wxDefaultPosition, wxDefaultSize, 0 );
	cb_SendAlerts->SetValue(true);
	
	cb_SendAlerts->SetToolTip( _("Check to disable the messaging via Notifier") );
	
	fgSizer3->Add( cb_SendAlerts, 0, wxALL, 5 );
	
	
	fgSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
	
	cb_NoAlertTimeout = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("No Timeout"), wxDefaultPosition, wxDefaultSize, 0 );
	
	cb_NoAlertTimeout->SetToolTip( _("Check to let the Notifier never timeout") );
	
	fgSizer3->Add( cb_NoAlertTimeout, 0, wxALL, 5 );
	
	
	fgSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxHORIZONTAL );
	
	m_spinAlertNotifyTimeout = new wxSpinCtrl( tb_MainAnoubisOptions, wxID_ANY, wxT("10"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 120, 10 );
	bSizer15->Add( m_spinAlertNotifyTimeout, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	tx_AlertNotifyTimeoutLabel = new wxStaticText( tb_MainAnoubisOptions, wxID_ANY, _("Timeout in Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_AlertNotifyTimeoutLabel->Wrap( -1 );
	bSizer15->Add( tx_AlertNotifyTimeoutLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	fgSizer3->Add( bSizer15, 1, wxEXPAND, 5 );
	
	sbSizer4->Add( fgSizer3, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer4, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("Autostart") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	cb_DoAutostart = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("Autostart"), wxDefaultPosition, wxDefaultSize, 0 );
	cb_DoAutostart->SetValue(true);
	
	cb_DoAutostart->SetToolTip( _("Check to enable Autostart of xanoubis") );
	
	fgSizer7->Add( cb_DoAutostart, 0, wxALL, 5 );
	
	
	fgSizer7->Add( 0, 0, 1, wxEXPAND, 5 );
	
	sbSizer8->Add( fgSizer7, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer8, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("Rule Editor") ), wxVERTICAL );
	
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxVERTICAL );
	
	controlAutoCheck = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("Auto Checksum Check"), wxDefaultPosition, wxDefaultSize, 0 );
	
	controlAutoCheck->SetToolTip( _("Check to automatically validate updated application rules") );
	
	bSizer19->Add( controlAutoCheck, 0, wxALL, 5 );
	
	sbSizer12->Add( bSizer19, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer12, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer6;
	sbSizer6 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("Connection") ), wxVERTICAL );
	
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );
	
	autoConnectBox = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("Auto Connect to Daemon"), wxDefaultPosition, wxDefaultSize, 0 );
	autoConnectBox->SetValue(true);
	
	bSizer191->Add( autoConnectBox, 0, wxALL, 5 );
	
	sbSizer6->Add( bSizer191, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer6, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer61;
	sbSizer61 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("Tool Tips") ), wxVERTICAL );
	
	wxBoxSizer* bSizer1911;
	bSizer1911 = new wxBoxSizer( wxHORIZONTAL );
	
	toolTipCheckBox = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("Enable Tool Tips after"), wxDefaultPosition, wxDefaultSize, 0 );
	toolTipCheckBox->SetValue(true);
	
	bSizer1911->Add( toolTipCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	toolTipSpinCtrl = new wxSpinCtrl( tb_MainAnoubisOptions, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	bSizer1911->Add( toolTipSpinCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText411 = new wxStaticText( tb_MainAnoubisOptions, wxID_ANY, _("Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText411->Wrap( -1 );
	bSizer1911->Add( m_staticText411, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sbSizer61->Add( bSizer1911, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer61, 0, wxEXPAND, 5 );
	
	sz_MainAnoubisOptions->Add( bSizer13, 1, wxEXPAND, 5 );
	
	tb_MainAnoubisOptions->SetSizer( sz_MainAnoubisOptions );
	tb_MainAnoubisOptions->Layout();
	sz_MainAnoubisOptions->Fit( tb_MainAnoubisOptions );
	tb_MainAnoubisNotify->AddPage( tb_MainAnoubisOptions, _("Options"), false );
	
	sz_MainAnoubisMain->Add( tb_MainAnoubisNotify, 1, wxEXPAND | wxALL, 5 );
	
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
	profileList->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( ModAnoubisMainPanelBase::OnProfileSelectionChanged ), NULL, this );
	profileDeleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnProfileDeleteClicked ), NULL, this );
	profileLoadButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnProfileLoadClicked ), NULL, this );
	profileSaveButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnProfileSaveClicked ), NULL, this );
	profileActivateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnProfileActivateClicked ), NULL, this );
	VersionActivePolicyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnVersionActivePolicyClicked ), NULL, this );
	VersionProfilePolicyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnVersionProfilePolicyClicked ), NULL, this );
	VersionProfileChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnVersionProfileChoice ), NULL, this );
	VersionListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( ModAnoubisMainPanelBase::OnVersionListCtrlSelected ), NULL, this );
	VersionRestoreButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnVersionRestoreButtonClick ), NULL, this );
	VersionExportButton1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnVersionExportButtonClick ), NULL, this );
	VersionDeleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnVersionDeleteButtonClick ), NULL, this );
	VersionShowButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnVersionShowButtonClick ), NULL, this );
	cb_SendEscalations->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationDisable ), NULL, this );
	cb_NoEscalationTimeout->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationNoTimeout ), NULL, this );
	m_spinEscalationNotifyTimeout->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( ModAnoubisMainPanelBase::OnEscalationTimeout ), NULL, this );
	cb_SendAlerts->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnAlertDisable ), NULL, this );
	cb_NoAlertTimeout->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnAlertNoTimeout ), NULL, this );
	m_spinAlertNotifyTimeout->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( ModAnoubisMainPanelBase::OnAlertTimeout ), NULL, this );
	cb_DoAutostart->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnDoAutostart ), NULL, this );
	controlAutoCheck->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnAutoCheck ), NULL, this );
	toolTipCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnToolTipCheckBox ), NULL, this );
	toolTipSpinCtrl->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( ModAnoubisMainPanelBase::OnToolTipSpinCtrl ), NULL, this );
	toolTipSpinCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnToolTipSpinCtrlText ), NULL, this );
}

ModAnoubisOverviewPanelBase::ModAnoubisOverviewPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer4->Add( 20, 0, 1, wxEXPAND, 5 );
	
	anoubisStatusIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( anoubisStatusIcon, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer25->Add( 20, 0, 0, wxEXPAND, 5 );
	
	bSizer24->Add( bSizer25, 0, wxEXPAND, 5 );
	
	m_staticText35 = new wxStaticText( this, wxID_ANY, _("Connection to localhost:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText35->Wrap( -1 );
	bSizer24->Add( m_staticText35, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer26->Add( 20, 0, 0, wxEXPAND, 5 );
	
	connectButton = new wxButton( this, wxID_ANY, _("Connect"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer26->Add( connectButton, 0, wxALL, 5 );
	
	disconnectButton = new wxButton( this, wxID_ANY, _("Disconnect"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer26->Add( disconnectButton, 0, wxALL, 5 );
	
	bSizer24->Add( bSizer26, 0, wxEXPAND, 5 );
	
	fgSizer4->Add( bSizer24, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer4 );
	this->Layout();
	fgSizer4->Fit( this );
	
	// Connect Events
	connectButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisOverviewPanelBase::OnConnectClicked ), NULL, this );
	disconnectButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisOverviewPanelBase::OnDisconnectClicked ), NULL, this );
}

ModAnoubisProfileSelectionDialogBase::ModAnoubisProfileSelectionDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText40 = new wxStaticText( this, wxID_ANY, _("Select a profile from the list below or enter a new profile."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText40->Wrap( 250 );
	bSizer30->Add( m_staticText40, 0, wxALL, 5 );
	
	profilesCombo = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	bSizer30->Add( profilesCombo, 0, wxALL|wxEXPAND, 5 );
	
	buttonSizer = new wxStdDialogButtonSizer();
	buttonSizerOK = new wxButton( this, wxID_OK );
	buttonSizer->AddButton( buttonSizerOK );
	buttonSizerCancel = new wxButton( this, wxID_CANCEL );
	buttonSizer->AddButton( buttonSizerCancel );
	buttonSizer->Realize();
	bSizer30->Add( buttonSizer, 1, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( bSizer30 );
	this->Layout();
	bSizer30->Fit( this );
	
	// Connect Events
	profilesCombo->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( ModAnoubisProfileSelectionDialogBase::OnTextChanged ), NULL, this );
}
