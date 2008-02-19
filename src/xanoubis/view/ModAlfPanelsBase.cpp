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
	pan_TabOptions = new wxPanel( note_MainAlf, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	note_MainAlf->AddPage( pan_TabOptions, _("Options"), false );
	pan_Rules = new wxPanel( note_MainAlf, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_AlfRules;
	sz_AlfRules = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* sz_RulesNS;
	sz_RulesNS = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* sz_RulesWE;
	sz_RulesWE = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( pan_Rules, -1, _("PLACEHOLDER_RULE_WIDGET") ), wxVERTICAL );
	
	sz_RulesWE->Add( sbSizer2, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_RulesOperation;
	sz_RulesOperation = new wxBoxSizer( wxVERTICAL );
	
	tx_RulesOperation1stHeader = new wxStaticText( pan_Rules, wxID_ANY, _("Rule:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_RulesOperation1stHeader->Wrap( -1 );
	sz_RulesOperation->Add( tx_RulesOperation1stHeader, 0, wxALL, 5 );
	
	ln_RulesOperationSep = new wxStaticLine( pan_Rules, wxID_RulesOperationSep, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	sz_RulesOperation->Add( ln_RulesOperationSep, 0, wxEXPAND | wxALL, 5 );
	
	tx_RulesOperation2ndHeader = new wxStaticText( pan_Rules, wxID_ANY, _("Information:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_RulesOperation2ndHeader->Wrap( -1 );
	sz_RulesOperation->Add( tx_RulesOperation2ndHeader, 0, wxALL, 5 );
	
	sz_RulesWE->Add( sz_RulesOperation, 1, wxEXPAND, 5 );
	
	sz_RulesNS->Add( sz_RulesWE, 1, wxEXPAND, 5 );
	
	sz_AlfRules->Add( sz_RulesNS, 1, wxEXPAND, 5 );
	
	pan_Rules->SetSizer( sz_AlfRules );
	pan_Rules->Layout();
	sz_AlfRules->Fit( pan_Rules );
	note_MainAlf->AddPage( pan_Rules, _("Rules"), false );
	pan_TabAppView = new wxPanel( note_MainAlf, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_AlfAppView1;
	sz_AlfAppView1 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* sz_AppSel1;
	sz_AppSel1 = new wxBoxSizer( wxVERTICAL );
	
	tx_AppSelHeader1 = new wxStaticText( pan_TabAppView, wxID_ANY, _("Application:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_AppSelHeader1->Wrap( -1 );
	sz_AppSel1->Add( tx_AppSelHeader1, 0, wxALL, 5 );
	
	lst_AppSelApplications1 = new wxListBox( pan_TabAppView, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	lst_AppSelApplications1->Append( _("gvim") );
	lst_AppSelApplications1->Append( _("firefox") );
	lst_AppSelApplications1->Append( _("thunderbird") );
	lst_AppSelApplications1->Append( _("irc") );
	lst_AppSelApplications1->Append( _("ssh") );
	sz_AppSel1->Add( lst_AppSelApplications1, 0, wxALL, 5 );
	
	sz_AlfAppView1->Add( sz_AppSel1, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_AppContext1;
	sz_AppContext1 = new wxBoxSizer( wxVERTICAL );
	
	tx_AppContextHeader1 = new wxStaticText( pan_TabAppView, wxID_ANY, _("Context/Program:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_AppContextHeader1->Wrap( -1 );
	sz_AppContext1->Add( tx_AppContextHeader1, 0, wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer11;
	sbSizer11 = new wxStaticBoxSizer( new wxStaticBox( pan_TabAppView, -1, _("PLACEHOLDER") ), wxVERTICAL );
	
	sbSizer11->SetMinSize( wxSize( 180,260 ) ); 
	sz_AppContext1->Add( sbSizer11, 1, wxEXPAND, 5 );
	
	sz_AlfAppView1->Add( sz_AppContext1, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_AppGroup1;
	sz_AppGroup1 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* sz_AppGroupVert1;
	sz_AppGroupVert1 = new wxBoxSizer( wxVERTICAL );
	
	tx_AppGroupChoice1 = new wxStaticText( pan_TabAppView, wxID_ANY, _("Grouping:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_AppGroupChoice1->Wrap( -1 );
	sz_AppGroupVert1->Add( tx_AppGroupChoice1, 0, wxALL, 5 );
	
	rad_GroupChoiceOne1 = new wxRadioButton( pan_TabAppView, wxID_GroupingContex, _("Context"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_AppGroupVert1->Add( rad_GroupChoiceOne1, 0, wxALL, 5 );
	
	rad_GroupChoiceTwo1 = new wxRadioButton( pan_TabAppView, wxID_GroupingProg, _("Program"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_AppGroupVert1->Add( rad_GroupChoiceTwo1, 0, wxALL, 5 );
	
	ln_GroupChoiceSepLine1 = new wxStaticLine( pan_TabAppView, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	sz_AppGroupVert1->Add( ln_GroupChoiceSepLine1, 0, wxEXPAND | wxALL, 5 );
	
	tx_AppGroupInfo1 = new wxStaticText( pan_TabAppView, wxID_ANY, _("Information:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_AppGroupInfo1->Wrap( -1 );
	sz_AppGroupVert1->Add( tx_AppGroupInfo1, 0, wxALL, 5 );
	
	sz_AppGroup1->Add( sz_AppGroupVert1, 1, wxEXPAND, 5 );
	
	sz_AlfAppView1->Add( sz_AppGroup1, 1, wxEXPAND, 5 );
	
	pan_TabAppView->SetSizer( sz_AlfAppView1 );
	pan_TabAppView->Layout();
	sz_AlfAppView1->Fit( pan_TabAppView );
	note_MainAlf->AddPage( pan_TabAppView, _("Application View"), true );
	
	sz_MainALFMain->Add( note_MainAlf, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( sz_MainALFMain );
	this->Layout();
	sz_MainALFMain->Fit( this );
}

ModAlfOverviewPanelBase::ModAlfOverviewPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* sz_OverviewALFMain;
	sz_OverviewALFMain = new wxBoxSizer( wxVERTICAL );
	
	tx_OVMainHeadline = new wxStaticText( this, wxID_ANY, _("Overview Panel of Module ALF"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_OVMainHeadline->Wrap( -1 );
	sz_OverviewALFMain->Add( tx_OVMainHeadline, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	this->SetSizer( sz_OverviewALFMain );
	this->Layout();
	sz_OverviewALFMain->Fit( this );
}
