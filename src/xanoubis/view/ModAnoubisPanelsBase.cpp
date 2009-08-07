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

#include "VersionListCtrl.h"

#include "ModAnoubisPanelsBase.h"

///////////////////////////////////////////////////////////////////////////

ModAnoubisMainPanelBase::ModAnoubisMainPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	sz_MainAnoubisMain = new wxBoxSizer( wxVERTICAL );
	
	tx_MainHeadline = new wxStaticText( this, wxID_ANY, _("Main Panel of Module Anoubis"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	tx_MainHeadline->Wrap( -1 );
	tx_MainHeadline->SetFont( wxFont( 16, 70, 90, 90, false, wxEmptyString ) );
	
	sz_MainAnoubisMain->Add( tx_MainHeadline, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 5 );
	
	tb_MainAnoubisNotify = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	tb_MainAnoubisNotification = new wxPanel( tb_MainAnoubisNotify, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_MainAnoubisNotify;
	sz_MainAnoubisNotify = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* sz_type;
	sz_type = new wxBoxSizer( wxHORIZONTAL );
	
	tx_type = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
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
	bt_first->SetToolTip( _("Jump first") );
	
	sz_navigate->Add( bt_first, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bt_previous = new wxButton( tb_MainAnoubisNotification, wxID_ANY, _("<"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_previous->Enable( false );
	bt_previous->SetToolTip( _("Jump previous") );
	
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
	bt_next->SetToolTip( _("Jump next") );
	
	sz_navigate->Add( bt_next, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bt_last = new wxButton( tb_MainAnoubisNotification, wxID_ANY, _(">>"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_last->Enable( false );
	bt_last->SetToolTip( _("Jump last") );
	
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
	
	pn_Escalation = new wxPanel( tb_MainAnoubisNotification, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_EscalationOptions;
	sz_EscalationOptions = new wxBoxSizer( wxVERTICAL );
	
	pn_EscalationOptions = new wxPanel( pn_Escalation, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER|wxTAB_TRAVERSAL );
	wxBoxSizer* sz_Escalation1;
	sz_Escalation1 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* sz_EscalationLeft;
	sz_EscalationLeft = new wxBoxSizer( wxVERTICAL );
	
	
	sz_EscalationLeft->Add( 0, 0, 1, wxEXPAND, 5 );
	
	rb_EscalationOnce = new wxRadioButton( pn_EscalationOptions, wxID_ANY, _("Once"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	sz_EscalationLeft->Add( rb_EscalationOnce, 0, wxALL, 5 );
	
	rb_EscalationProcess = new wxRadioButton( pn_EscalationOptions, wxID_ANY, _("Until Process Ends"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_EscalationLeft->Add( rb_EscalationProcess, 0, wxALL, 5 );
	
	wxBoxSizer* sz_EscalationTime;
	sz_EscalationTime = new wxBoxSizer( wxHORIZONTAL );
	
	rb_EscalationTime = new wxRadioButton( pn_EscalationOptions, wxID_ANY, _("for"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_EscalationTime->Add( rb_EscalationTime, 0, wxALIGN_CENTER|wxALL, 5 );
	
	spin_EscalationTime = new wxSpinCtrl( pn_EscalationOptions, wxID_ANY, wxT("10"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10000, 0 );
	sz_EscalationTime->Add( spin_EscalationTime, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString ch_EscalationTimeUnitChoices[] = { _("Seconds"), _("Minutes"), _("Hours"), _("Days") };
	int ch_EscalationTimeUnitNChoices = sizeof( ch_EscalationTimeUnitChoices ) / sizeof( wxString );
	ch_EscalationTimeUnit = new wxChoice( pn_EscalationOptions, wxID_ANY, wxDefaultPosition, wxDefaultSize, ch_EscalationTimeUnitNChoices, ch_EscalationTimeUnitChoices, 0 );
	sz_EscalationTime->Add( ch_EscalationTimeUnit, 0, wxALL, 5 );
	
	sz_EscalationLeft->Add( sz_EscalationTime, 0, 0, 5 );
	
	rb_EscalationAlways = new wxRadioButton( pn_EscalationOptions, wxID_ANY, _("Always"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_EscalationLeft->Add( rb_EscalationAlways, 0, wxALL, 5 );
	
	
	sz_EscalationLeft->Add( 0, 0, 1, wxEXPAND, 5 );
	
	sz_Escalation1->Add( sz_EscalationLeft, 0, wxEXPAND, 5 );
	
	m_staticline2 = new wxStaticLine( pn_EscalationOptions, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	sz_Escalation1->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* sz_EscalationRight;
	sz_EscalationRight = new wxBoxSizer( wxVERTICAL );
	
	pn_EscalationAlf = new wxPanel( pn_EscalationOptions, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_EscalationAlf;
	sz_EscalationAlf = new wxBoxSizer( wxVERTICAL );
	
	
	sz_EscalationAlf->Add( 0, 0, 1, wxEXPAND, 5 );
	
	rb_EscalationAlf1 = new wxRadioButton( pn_EscalationAlf, wxID_ANY, _("this port on this host"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	sz_EscalationAlf->Add( rb_EscalationAlf1, 0, wxALL, 5 );
	
	rb_EscalationAlf2 = new wxRadioButton( pn_EscalationAlf, wxID_ANY, _("this port on every host"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_EscalationAlf->Add( rb_EscalationAlf2, 0, wxALL, 5 );
	
	rb_EscalationAlf3 = new wxRadioButton( pn_EscalationAlf, wxID_ANY, _("all ports on this host"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_EscalationAlf->Add( rb_EscalationAlf3, 0, wxALL, 5 );
	
	rb_EscalationAlf4 = new wxRadioButton( pn_EscalationAlf, wxID_ANY, _("all ports on all hosts"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_EscalationAlf->Add( rb_EscalationAlf4, 0, wxALL, 5 );
	
	
	sz_EscalationAlf->Add( 0, 0, 1, wxEXPAND, 5 );
	
	pn_EscalationAlf->SetSizer( sz_EscalationAlf );
	pn_EscalationAlf->Layout();
	sz_EscalationAlf->Fit( pn_EscalationAlf );
	sz_EscalationRight->Add( pn_EscalationAlf, 1, wxALL|wxEXPAND, 5 );
	
	pn_EscalationSb = new wxPanel( pn_EscalationOptions, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pn_EscalationSb->Hide();
	
	wxBoxSizer* sz_EscalationSb;
	sz_EscalationSb = new wxBoxSizer( wxVERTICAL );
	
	
	sz_EscalationSb->Add( 0, 0, 1, wxEXPAND, 5 );
	
	tx_EscalationSbDesc = new wxStaticText( pn_EscalationSb, wxID_ANY, _("Path prefix and permissions for new rule"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_EscalationSbDesc->Wrap( -1 );
	sz_EscalationSb->Add( tx_EscalationSbDesc, 0, wxALL, 5 );
	
	wxBoxSizer* sz_EscalationSbPath;
	sz_EscalationSbPath = new wxBoxSizer( wxHORIZONTAL );
	
	bt_EscalationSbPathLeft = new wxButton( pn_EscalationSb, wxID_ANY, _("<"), wxDefaultPosition, wxSize( -1,-1 ), wxBU_EXACTFIT );
	sz_EscalationSbPath->Add( bt_EscalationSbPathLeft, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sz_EscalationSbPath->Add( 0, 0, 1, wxEXPAND, 5 );
	
	tx_EscalationSbPath = new wxStaticText( pn_EscalationSb, wxID_ANY, _("/"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_EscalationSbPath->Wrap( -1 );
	sz_EscalationSbPath->Add( tx_EscalationSbPath, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sz_EscalationSbPath->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bt_EscalationSbPathRight = new wxButton( pn_EscalationSb, wxID_ANY, _(">"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	sz_EscalationSbPath->Add( bt_EscalationSbPathRight, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sz_EscalationSb->Add( sz_EscalationSbPath, 0, wxEXPAND, 5 );
	
	wxBoxSizer* sz_EscalationSbPerms;
	sz_EscalationSbPerms = new wxBoxSizer( wxHORIZONTAL );
	
	
	sz_EscalationSbPerms->Add( 0, 0, 10, wxEXPAND, 5 );
	
	ck_EscalationSbRead = new wxCheckBox( pn_EscalationSb, wxID_ANY, _("read"), wxDefaultPosition, wxDefaultSize, 0 );
	
	sz_EscalationSbPerms->Add( ck_EscalationSbRead, 0, wxALL, 5 );
	
	
	sz_EscalationSbPerms->Add( 0, 0, 1, wxEXPAND, 5 );
	
	ck_EscalationSbWrite = new wxCheckBox( pn_EscalationSb, wxID_ANY, _("write"), wxDefaultPosition, wxDefaultSize, 0 );
	
	sz_EscalationSbPerms->Add( ck_EscalationSbWrite, 0, wxALL, 5 );
	
	
	sz_EscalationSbPerms->Add( 0, 0, 1, wxEXPAND, 5 );
	
	ck_EscalationSbExec = new wxCheckBox( pn_EscalationSb, wxID_ANY, _("execute"), wxDefaultPosition, wxDefaultSize, 0 );
	
	sz_EscalationSbPerms->Add( ck_EscalationSbExec, 0, wxALL, 5 );
	
	
	sz_EscalationSbPerms->Add( 0, 0, 10, wxEXPAND, 5 );
	
	sz_EscalationSb->Add( sz_EscalationSbPerms, 0, wxEXPAND, 5 );
	
	
	sz_EscalationSb->Add( 0, 0, 1, wxEXPAND, 5 );
	
	pn_EscalationSb->SetSizer( sz_EscalationSb );
	pn_EscalationSb->Layout();
	sz_EscalationSb->Fit( pn_EscalationSb );
	sz_EscalationRight->Add( pn_EscalationSb, 1, wxEXPAND | wxALL, 5 );
	
	pn_EscalationSfs = new wxPanel( pn_EscalationOptions, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pn_EscalationSfs->Hide();
	
	wxBoxSizer* sz_EscalationSfs;
	sz_EscalationSfs = new wxBoxSizer( wxVERTICAL );
	
	
	sz_EscalationSfs->Add( 0, 0, 1, wxEXPAND, 5 );
	
	tx_EscalationSfsDesc = new wxStaticText( pn_EscalationSfs, wxID_ANY, _("Path prefix and Checksum Options for"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_EscalationSfsDesc->Wrap( -1 );
	sz_EscalationSfs->Add( tx_EscalationSfsDesc, 0, wxALL, 5 );
	
	wxBoxSizer* sz_EscalationSfsPath;
	sz_EscalationSfsPath = new wxBoxSizer( wxHORIZONTAL );
	
	bt_EscalationSfsPathLeft = new wxButton( pn_EscalationSfs, wxID_ANY, _("<"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	sz_EscalationSfsPath->Add( bt_EscalationSfsPathLeft, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sz_EscalationSfsPath->Add( 0, 0, 1, wxEXPAND, 5 );
	
	tx_EscalationSfsPath = new wxStaticText( pn_EscalationSfs, wxID_ANY, _("/"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_EscalationSfsPath->Wrap( -1 );
	sz_EscalationSfsPath->Add( tx_EscalationSfsPath, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sz_EscalationSfsPath->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bt_EscalationSfsPathRight = new wxButton( pn_EscalationSfs, wxID_ANY, _(">"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	sz_EscalationSfsPath->Add( bt_EscalationSfsPathRight, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sz_EscalationSfs->Add( sz_EscalationSfsPath, 0, wxEXPAND, 5 );
	
	
	sz_EscalationSfs->Add( 0, 0, 1, wxEXPAND, 5 );
	
	pn_EscalationSfs->SetSizer( sz_EscalationSfs );
	pn_EscalationSfs->Layout();
	sz_EscalationSfs->Fit( pn_EscalationSfs );
	sz_EscalationRight->Add( pn_EscalationSfs, 1, wxEXPAND | wxALL, 5 );
	
	sz_Escalation1->Add( sz_EscalationRight, 1, wxEXPAND, 5 );
	
	pn_EscalationOptions->SetSizer( sz_Escalation1 );
	pn_EscalationOptions->Layout();
	sz_Escalation1->Fit( pn_EscalationOptions );
	sz_EscalationOptions->Add( pn_EscalationOptions, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* sz_Escalation2;
	sz_Escalation2 = new wxBoxSizer( wxHORIZONTAL );
	
	
	sz_Escalation2->Add( 0, 0, 2, wxEXPAND, 5 );
	
	ck_EscalationEditor = new wxCheckBox( pn_Escalation, wxID_ANY, _("open rule editor"), wxDefaultPosition, wxDefaultSize, 0 );
	
	sz_Escalation2->Add( ck_EscalationEditor, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sz_Escalation2->Add( 0, 0, 20, wxEXPAND, 5 );
	
	bt_EscalationAllow = new wxButton( pn_Escalation, wxID_ANY, _("Allow"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_Escalation2->Add( bt_EscalationAllow, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sz_Escalation2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bt_EscalationDeny = new wxButton( pn_Escalation, wxID_ANY, _("Deny"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_Escalation2->Add( bt_EscalationDeny, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sz_Escalation2->Add( 0, 0, 2, wxEXPAND, 5 );
	
	sz_EscalationOptions->Add( sz_Escalation2, 1, wxEXPAND, 5 );
	
	pn_Escalation->SetSizer( sz_EscalationOptions );
	pn_Escalation->Layout();
	sz_EscalationOptions->Fit( pn_Escalation );
	sz_MainAnoubisNotify->Add( pn_Escalation, 0, wxEXPAND | wxALL, 5 );
	
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
	profileDeleteButton->SetToolTip( _("Removes the selected profile") );
	
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
	
	m_staticText40 = new wxStaticText( tb_Profiles, wxID_ANY, _("Saving a profile only creates a new Version. The ruleset is not sent to the daemon. Use \"Activate\" to achieve this."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText40->Wrap( 250 );
	bSizer27->Add( m_staticText40, 0, wxALL, 5 );
	
	profileActivateButton = new wxButton( tb_Profiles, wxID_ANY, _("Activate"), wxDefaultPosition, wxDefaultSize, 0 );
	profileActivateButton->Enable( false );
	profileActivateButton->SetToolTip( _("Send the selected Profile to the Daemon") );
	
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
	bSizer31->Add( m_staticText41, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	VersionActivePolicyRadioButton = new wxRadioButton( tb_MainAnoubisVersions, wxID_ANY, _("Active policy"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer31->Add( VersionActivePolicyRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	VersionProfilePolicyRadioButton = new wxRadioButton( tb_MainAnoubisVersions, wxID_ANY, _("Policy from profile"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer31->Add( VersionProfilePolicyRadioButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString VersionProfileChoiceChoices;
	VersionProfileChoice = new wxChoice( tb_MainAnoubisVersions, wxID_ANY, wxDefaultPosition, wxDefaultSize, VersionProfileChoiceChoices, 0 );
	VersionProfileChoice->Enable( false );
	
	bSizer31->Add( VersionProfileChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	VersionListSizer->Add( bSizer31, 0, wxALL|wxEXPAND, 5 );
	
	versionListCtrl = new VersionListCtrl( tb_MainAnoubisVersions, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_VIRTUAL );
	VersionListSizer->Add( versionListCtrl, 80, wxALL|wxEXPAND, 5 );
	
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
	VersionExportButton1->SetToolTip( _("Export the selected Version of the profile to a file.") );
	
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
	
	cb_SendEscalations = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("Send Escalations"), wxDefaultPosition, wxDefaultSize, 0 );
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
	
	cb_SendAlerts = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("Send Alerts"), wxDefaultPosition, wxDefaultSize, 0 );
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
	
	wxStaticBoxSizer* sbSizer10;
	sbSizer10 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("Upgrade") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	cb_ShowUpgradeMsg = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("Enable Upgrade Message"), wxDefaultPosition, wxDefaultSize, 0 );
	cb_ShowUpgradeMsg->SetValue(true);
	
	cb_ShowUpgradeMsg->SetToolTip( _("Check to be notified when upgrade event had taken place") );
	
	fgSizer6->Add( cb_ShowUpgradeMsg, 0, wxALL, 5 );
	
	sbSizer10->Add( fgSizer6, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer10, 0, wxEXPAND, 5 );
	
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
	
	autoConnectBox->SetToolTip( _("Check to auto-connect at startup") );
	
	bSizer191->Add( autoConnectBox, 0, wxALL, 5 );
	
	sbSizer6->Add( bSizer191, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer6, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer61;
	sbSizer61 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("Tool Tips") ), wxVERTICAL );
	
	wxBoxSizer* bSizer1911;
	bSizer1911 = new wxBoxSizer( wxHORIZONTAL );
	
	toolTipCheckBox = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("Enable Tool Tips after"), wxDefaultPosition, wxDefaultSize, 0 );
	toolTipCheckBox->SetValue(true);
	
	toolTipCheckBox->SetToolTip( _("Check to enable tooltips") );
	
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
	rb_EscalationOnce->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationOnceButton ), NULL, this );
	rb_EscalationProcess->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationProcessButton ), NULL, this );
	rb_EscalationTime->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationTimeoutButton ), NULL, this );
	rb_EscalationAlways->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationAlwaysButton ), NULL, this );
	bt_EscalationSbPathLeft->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationSbPathLeft ), NULL, this );
	bt_EscalationSbPathRight->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationSbPathRight ), NULL, this );
	bt_EscalationSfsPathLeft->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationSfsPathLeft ), NULL, this );
	bt_EscalationSfsPathRight->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnEscalationSfsPathRight ), NULL, this );
	bt_EscalationAllow->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnAllowBtnClick ), NULL, this );
	bt_EscalationDeny->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnDenyBtnClick ), NULL, this );
	profileList->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( ModAnoubisMainPanelBase::OnProfileSelectionChanged ), NULL, this );
	profileDeleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnProfileDeleteClicked ), NULL, this );
	profileLoadButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnProfileLoadClicked ), NULL, this );
	profileSaveButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnProfileSaveClicked ), NULL, this );
	profileActivateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnProfileActivateClicked ), NULL, this );
	VersionActivePolicyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnVersionActivePolicyClicked ), NULL, this );
	VersionProfilePolicyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnVersionProfilePolicyClicked ), NULL, this );
	VersionProfileChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnVersionProfileChoice ), NULL, this );
	versionListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( ModAnoubisMainPanelBase::OnVersionListCtrlSelected ), NULL, this );
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
	connectButton->SetToolTip( _("Establishes a connection to the Anoubis daemon") );
	
	bSizer26->Add( connectButton, 0, wxALL, 5 );
	
	disconnectButton = new wxButton( this, wxID_ANY, _("Disconnect"), wxDefaultPosition, wxDefaultSize, 0 );
	disconnectButton->SetToolTip( _("Disconnects from the Anoubis daemon") );
	
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
