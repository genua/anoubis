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
	
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("Rule Editor") ), wxVERTICAL );
	
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxVERTICAL );
	
	controlAutoCheck = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("Auto Checksum Check"), wxDefaultPosition, wxDefaultSize, 0 );
	
	bSizer19->Add( controlAutoCheck, 0, wxALL, 5 );
	
	sbSizer12->Add( bSizer19, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer12, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer6;
	sbSizer6 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("Connection") ), wxVERTICAL );
	
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );
	
	autoConnectBox = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("Auto Connect to Daemon"), wxDefaultPosition, wxDefaultSize, 0 );
	
	bSizer191->Add( autoConnectBox, 0, wxALL, 5 );
	
	sbSizer6->Add( bSizer191, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer6, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer7;
	sbSizer7 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("Private Key") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 4, 2, 0, 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText27 = new wxStaticText( tb_MainAnoubisOptions, wxID_ANY, _("Configure Privatekey:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText27->Wrap( -1 );
	m_staticText27->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	fgSizer6->Add( m_staticText27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	PrivKeyChooseButton = new wxButton( tb_MainAnoubisOptions, wxID_ANY, _("Choose Key"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( PrivKeyChooseButton, 0, wxALL, 5 );
	
	m_staticText28 = new wxStaticText( tb_MainAnoubisOptions, wxID_ANY, _("Fingerprint:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText28->Wrap( -1 );
	fgSizer6->Add( m_staticText28, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	PrivKeyFingerprintText = new wxStaticText( tb_MainAnoubisOptions, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, 0 );
	PrivKeyFingerprintText->Wrap( -1 );
	fgSizer6->Add( PrivKeyFingerprintText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticText30 = new wxStaticText( tb_MainAnoubisOptions, wxID_ANY, _("Distinguished Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText30->Wrap( -1 );
	fgSizer6->Add( m_staticText30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	PrivKeyDnText = new wxStaticText( tb_MainAnoubisOptions, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, 0 );
	PrivKeyDnText->Wrap( -1 );
	fgSizer6->Add( PrivKeyDnText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticText32 = new wxStaticText( tb_MainAnoubisOptions, wxID_ANY, _("Passphrase validity:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText32->Wrap( -1 );
	fgSizer6->Add( m_staticText32, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxHORIZONTAL );
	
	wxString PrivKeyValidityChoiceChoices[] = { _("Until end of session"), _("Until specified time") };
	int PrivKeyValidityChoiceNChoices = sizeof( PrivKeyValidityChoiceChoices ) / sizeof( wxString );
	PrivKeyValidityChoice = new wxChoice( tb_MainAnoubisOptions, wxID_ANY, wxDefaultPosition, wxDefaultSize, PrivKeyValidityChoiceNChoices, PrivKeyValidityChoiceChoices, 0 );
	bSizer24->Add( PrivKeyValidityChoice, 0, wxALL, 5 );
	
	PrivKeyValiditySpinCtrl = new wxSpinCtrl( tb_MainAnoubisOptions, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 86400, 0 );
	PrivKeyValiditySpinCtrl->Enable( false );
	
	bSizer24->Add( PrivKeyValiditySpinCtrl, 0, wxALL, 5 );
	
	PrivKeyValidityText = new wxStaticText( tb_MainAnoubisOptions, wxID_ANY, _("Validity end in Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	PrivKeyValidityText->Wrap( -1 );
	PrivKeyValidityText->Enable( false );
	
	bSizer24->Add( PrivKeyValidityText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	fgSizer6->Add( bSizer24, 1, wxEXPAND, 5 );
	
	sbSizer7->Add( fgSizer6, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer7, 0, wxEXPAND, 5 );
	
	sz_MainAnoubisOptions->Add( bSizer13, 1, wxEXPAND, 5 );
	
	tb_MainAnoubisOptions->SetSizer( sz_MainAnoubisOptions );
	tb_MainAnoubisOptions->Layout();
	sz_MainAnoubisOptions->Fit( tb_MainAnoubisOptions );
	tb_MainAnoubisNotify->AddPage( tb_MainAnoubisOptions, _("Options"), false );
	tb_MainAnoubisVersions = new wxPanel( tb_MainAnoubisNotify, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* MainAnoubisVersionsSizer;
	MainAnoubisVersionsSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* VersionListSizer;
	VersionListSizer = new wxBoxSizer( wxVERTICAL );
	
	VersionListCtrl = new wxListCtrl( tb_MainAnoubisVersions, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT );
	VersionListSizer->Add( VersionListCtrl, 80, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* VersionCommentSizer;
	VersionCommentSizer = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisVersions, -1, _("Comment of selected version:") ), wxVERTICAL );
	
	VersionShowCommentTextCtrl = new wxTextCtrl( tb_MainAnoubisVersions, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	VersionShowCommentTextCtrl->Enable( false );
	
	VersionCommentSizer->Add( VersionShowCommentTextCtrl, 1, wxALL|wxEXPAND, 5 );
	
	VersionListSizer->Add( VersionCommentSizer, 20, wxEXPAND, 5 );
	
	MainAnoubisVersionsSizer->Add( VersionListSizer, 70, wxEXPAND, 5 );
	
	wxBoxSizer* VersionButtonSizer;
	VersionButtonSizer = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* VersionActionSizer;
	VersionActionSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	VersionActionSizer->SetFlexibleDirection( wxBOTH );
	VersionActionSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	VersionSelectLabel = new wxStaticText( tb_MainAnoubisVersions, wxID_ANY, _("Selected Version:"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	VersionSelectLabel->Wrap( -1 );
	VersionActionSizer->Add( VersionSelectLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	VersionRestoreButton = new wxButton( tb_MainAnoubisVersions, wxID_ANY, _("restore"), wxDefaultPosition, wxDefaultSize, 0 );
	VersionActionSizer->Add( VersionRestoreButton, 0, wxALL, 0 );
	
	VersionSaveLabel = new wxStaticText( tb_MainAnoubisVersions, wxID_ANY, _("Save current rules\nas new version:"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	VersionSaveLabel->Wrap( -1 );
	VersionActionSizer->Add( VersionSaveLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	VersionSaveButton = new wxButton( tb_MainAnoubisVersions, wxID_ANY, _("save"), wxDefaultPosition, wxDefaultSize, 0 );
	VersionActionSizer->Add( VersionSaveButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0 );
	
	VersionButtonSizer->Add( VersionActionSizer, 0, wxEXPAND, 5 );
	
	VersionCommentLabel = new wxStaticText( tb_MainAnoubisVersions, wxID_ANY, _("Comment for new version:"), wxDefaultPosition, wxDefaultSize, 0 );
	VersionCommentLabel->Wrap( -1 );
	VersionCommentLabel->SetFont( wxFont( 9, 70, 90, 90, false, wxEmptyString ) );
	
	VersionButtonSizer->Add( VersionCommentLabel, 0, wxALL, 5 );
	
	VersionEnterCommentTextCtrl = new wxTextCtrl( tb_MainAnoubisVersions, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	VersionButtonSizer->Add( VersionEnterCommentTextCtrl, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* VersionDetailsSizer;
	VersionDetailsSizer = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisVersions, -1, _("Details...") ), wxVERTICAL );
	
	VersionVersionLabel = new wxStaticText( tb_MainAnoubisVersions, wxID_ANY, _("Version:"), wxDefaultPosition, wxDefaultSize, 0 );
	VersionVersionLabel->Wrap( -1 );
	VersionDetailsSizer->Add( VersionVersionLabel, 0, wxALL, 5 );
	
	VersionImportButton = new wxButton( tb_MainAnoubisVersions, wxID_ANY, _("import ..."), wxDefaultPosition, wxDefaultSize, 0 );
	VersionDetailsSizer->Add( VersionImportButton, 0, wxALL, 5 );
	
	VersionExportButton = new wxButton( tb_MainAnoubisVersions, wxID_ANY, _("export ..."), wxDefaultPosition, wxDefaultSize, 0 );
	VersionDetailsSizer->Add( VersionExportButton, 0, wxALL, 5 );
	
	VersionDeleteButton = new wxButton( tb_MainAnoubisVersions, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, 0 );
	VersionDetailsSizer->Add( VersionDeleteButton, 0, wxALL, 5 );
	
	VersionShowButton = new wxButton( tb_MainAnoubisVersions, wxID_ANY, _("show"), wxDefaultPosition, wxDefaultSize, 0 );
	VersionShowButton->Hide();
	
	VersionDetailsSizer->Add( VersionShowButton, 0, wxALL, 5 );
	
	VersionProfileButton = new wxButton( tb_MainAnoubisVersions, wxID_ANY, _("profile ..."), wxDefaultPosition, wxDefaultSize, 0 );
	VersionProfileButton->SetToolTip( _("a single profile") );
	
	VersionDetailsSizer->Add( VersionProfileButton, 0, wxALL, 5 );
	
	VersionButtonSizer->Add( VersionDetailsSizer, 1, wxEXPAND, 5 );
	
	MainAnoubisVersionsSizer->Add( VersionButtonSizer, 30, wxEXPAND, 5 );
	
	tb_MainAnoubisVersions->SetSizer( MainAnoubisVersionsSizer );
	tb_MainAnoubisVersions->Layout();
	MainAnoubisVersionsSizer->Fit( tb_MainAnoubisVersions );
	tb_MainAnoubisNotify->AddPage( tb_MainAnoubisVersions, _("Version control"), false );
	
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
	cb_SendEscalations->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationDisable ), NULL, this );
	cb_NoEscalationTimeout->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationNoTimeout ), NULL, this );
	m_spinEscalationNotifyTimeout->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( ModAnoubisMainPanelBase::OnEscalationTimeout ), NULL, this );
	cb_SendAlerts->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnAlertDisable ), NULL, this );
	cb_NoAlertTimeout->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnAlertNoTimeout ), NULL, this );
	m_spinAlertNotifyTimeout->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( ModAnoubisMainPanelBase::OnAlertTimeout ), NULL, this );
	controlAutoCheck->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnAutoCheck ), NULL, this );
}

ModAnoubisOverviewPanelBase::ModAnoubisOverviewPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* sz_OverviewAnoubisMain;
	sz_OverviewAnoubisMain = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer4->Add( 20, 0, 1, wxEXPAND, 5 );
	
	anoubisStatusIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( anoubisStatusIcon, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* profileSizer;
	profileSizer = new wxBoxSizer( wxVERTICAL );
	
	highProfileRadioButton = new wxRadioButton( this, wxID_ANY, _("Profile: high"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	profileSizer->Add( highProfileRadioButton, 0, wxALL, 5 );
	
	mediumProfileRadioButton = new wxRadioButton( this, wxID_ANY, _("Profile: medium"), wxDefaultPosition, wxDefaultSize, 0 );
	profileSizer->Add( mediumProfileRadioButton, 0, wxALL, 5 );
	
	adminProfileRadioButton = new wxRadioButton( this, wxID_ANY, _("Profile: admin"), wxDefaultPosition, wxDefaultSize, 0 );
	profileSizer->Add( adminProfileRadioButton, 0, wxALL, 5 );
	
	fgSizer4->Add( profileSizer, 1, wxEXPAND, 5 );
	
	sz_OverviewAnoubisMain->Add( fgSizer4, 1, wxEXPAND, 5 );
	
	this->SetSizer( sz_OverviewAnoubisMain );
	this->Layout();
	sz_OverviewAnoubisMain->Fit( this );
	
	// Connect Events
	highProfileRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( ModAnoubisOverviewPanelBase::OnHighProfileRadioButton ), NULL, this );
	mediumProfileRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( ModAnoubisOverviewPanelBase::OnMediumProfileRadioButton ), NULL, this );
	adminProfileRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( ModAnoubisOverviewPanelBase::OnAdminProfileRadioButton ), NULL, this );
}

ModAnoubisProfileDialogBase::ModAnoubisProfileDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );
	
	DialogLabel = new wxStaticText( this, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, 0 );
	DialogLabel->Wrap( -1 );
	bSizer20->Add( DialogLabel, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );
	
	HighCheckBox = new wxCheckBox( this, wxID_ANY, _("high"), wxDefaultPosition, wxDefaultSize, 0 );
	
	bSizer21->Add( HighCheckBox, 0, wxALL, 5 );
	
	MediumCheckBox = new wxCheckBox( this, wxID_ANY, _("medium"), wxDefaultPosition, wxDefaultSize, 0 );
	
	bSizer21->Add( MediumCheckBox, 0, wxALL, 5 );
	
	AdminCheckBox = new wxCheckBox( this, wxID_ANY, _("admin"), wxDefaultPosition, wxDefaultSize, 0 );
	
	bSizer21->Add( AdminCheckBox, 0, wxALL, 5 );
	
	bSizer20->Add( bSizer21, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer22->Add( 0, 0, 1, wxEXPAND, 5 );
	
	ActionButton = new wxButton( this, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, 0 );
	ActionButton->SetDefault(); 
	ActionButton->Enable( false );
	
	bSizer22->Add( ActionButton, 0, wxALL, 5 );
	
	CancelButton = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer22->Add( CancelButton, 0, wxALL, 5 );
	
	bSizer20->Add( bSizer22, 1, wxEXPAND, 5 );
	
	this->SetSizer( bSizer20 );
	this->Layout();
}
