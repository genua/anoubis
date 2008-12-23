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

#include "DlgRuleEditorBase.h"

///////////////////////////////////////////////////////////////////////////

DlgRuleEditorBase::DlgRuleEditorBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->Centre( wxBOTH );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxGridSizer* mainPanelSizer;
	mainPanelSizer = new wxGridSizer( 2, 2, 0, 0 );
	
	wxBoxSizer* appListSizer;
	appListSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* appListHeadSizer;
	appListHeadSizer = new wxBoxSizer( wxHORIZONTAL );
	
	appListTypeLabel = new wxStaticText( this, wxID_ANY, _("App-Rules"), wxDefaultPosition, wxDefaultSize, 0 );
	appListTypeLabel->Wrap( -1 );
	appListHeadSizer->Add( appListTypeLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString appListTypeChoiceChoices[] = { _("ALF"), _("SFS"), _("SB"), _("CTX") };
	int appListTypeChoiceNChoices = sizeof( appListTypeChoiceChoices ) / sizeof( wxString );
	appListTypeChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, appListTypeChoiceNChoices, appListTypeChoiceChoices, 0 );
	appListHeadSizer->Add( appListTypeChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appListCreateButton = new wxButton( this, wxID_ANY, _("create"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListCreateButton->Enable( false );
	
	appListHeadSizer->Add( appListCreateButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	appListHeadSizer->Add( 0, 0, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	appListColumnsButton = new wxButton( this, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListColumnsButton->Enable( false );
	
	appListHeadSizer->Add( appListColumnsButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	appListSizer->Add( appListHeadSizer, 0, wxEXPAND, 5 );
	
	appPolicyListCtrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_ICON );
	appListSizer->Add( appPolicyListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* appListFoodSizer;
	appListFoodSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	appListFoodSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	appListPolicyLabel = new wxStaticText( this, wxID_ANY, _("Rule:"), wxDefaultPosition, wxDefaultSize, 0 );
	appListPolicyLabel->Wrap( -1 );
	appListFoodSizer->Add( appListPolicyLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appListPolicyText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	appListPolicyText->Wrap( -1 );
	appListFoodSizer->Add( appListPolicyText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appListUpButton = new wxButton( this, wxID_ANY, _("up"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListUpButton->Enable( false );
	
	appListFoodSizer->Add( appListUpButton, 0, wxALL, 5 );
	
	appListDownButton = new wxButton( this, wxID_ANY, _("down"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListDownButton->Enable( false );
	
	appListFoodSizer->Add( appListDownButton, 0, wxALL, 5 );
	
	appListDeleteButton = new wxButton( this, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListDeleteButton->Enable( false );
	
	appListFoodSizer->Add( appListDeleteButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appListSizer->Add( appListFoodSizer, 0, wxEXPAND|wxALIGN_RIGHT, 5 );
	
	mainPanelSizer->Add( appListSizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* filterListSizer;
	filterListSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* filterListHeadSizer;
	filterListHeadSizer = new wxBoxSizer( wxHORIZONTAL );
	
	filterListTypeLabel = new wxStaticText( this, wxID_ANY, _("Filter-Rules"), wxDefaultPosition, wxDefaultSize, 0 );
	filterListTypeLabel->Wrap( -1 );
	filterListHeadSizer->Add( filterListTypeLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString filterListTypeChoiceChoices[] = { _("ALF Filter"), _("ALF Capability"), _("SFS Filter"), _("SB AccessFilter"), _("CTX Filter"), _("Default") };
	int filterListTypeChoiceNChoices = sizeof( filterListTypeChoiceChoices ) / sizeof( wxString );
	filterListTypeChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, filterListTypeChoiceNChoices, filterListTypeChoiceChoices, 0 );
	filterListHeadSizer->Add( filterListTypeChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterListCreateButton = new wxButton( this, wxID_ANY, _("create"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListCreateButton->Enable( false );
	
	filterListHeadSizer->Add( filterListCreateButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	filterListHeadSizer->Add( 0, 0, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	filterListColumnsButton = new wxButton( this, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListColumnsButton->Enable( false );
	
	filterListHeadSizer->Add( filterListColumnsButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	filterListSizer->Add( filterListHeadSizer, 0, wxEXPAND, 5 );
	
	filterPolicyListCtrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_ICON );
	filterListSizer->Add( filterPolicyListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* filterListFoodSizer;
	filterListFoodSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	filterListFoodSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	filterListPolicyLabel = new wxStaticText( this, wxID_ANY, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterListPolicyLabel->Wrap( -1 );
	filterListFoodSizer->Add( filterListPolicyLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterListPolicyText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	filterListPolicyText->Wrap( -1 );
	filterListFoodSizer->Add( filterListPolicyText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterListUpButton = new wxButton( this, wxID_ANY, _("up"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListUpButton->Enable( false );
	
	filterListFoodSizer->Add( filterListUpButton, 0, wxALL, 5 );
	
	filterListDownButton = new wxButton( this, wxID_ANY, _("down"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListDownButton->Enable( false );
	
	filterListFoodSizer->Add( filterListDownButton, 0, wxALL, 5 );
	
	filterListDeleteButton = new wxButton( this, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListDeleteButton->Enable( false );
	
	filterListFoodSizer->Add( filterListDeleteButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterListSizer->Add( filterListFoodSizer, 0, wxEXPAND|wxALIGN_RIGHT, 5 );
	
	mainPanelSizer->Add( filterListSizer, 1, wxEXPAND, 5 );
	
	appPolicyPanels = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryPage = new wxScrolledWindow( appPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	appBinaryPage->SetScrollRate( 5, 5 );
	appBinaryPage->Enable( false );
	
	wxBoxSizer* appBinaryMainSizer;
	appBinaryMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* appBinaryHaedSizer;
	appBinaryHaedSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
	appBinaryHaedSizer->SetFlexibleDirection( wxBOTH );
	appBinaryHaedSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	appBinaryLabel = new wxStaticText( appBinaryPage, wxID_ANY, _("Binary:"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryLabel->Wrap( -1 );
	appBinaryHaedSizer->Add( appBinaryLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appBinaryTextCtrl = new wxTextCtrl( appBinaryPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	appBinaryTextCtrl->SetMinSize( wxSize( 300,-1 ) );
	
	appBinaryHaedSizer->Add( appBinaryTextCtrl, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	appBinaryModifyButton = new wxButton( appBinaryPage, wxID_ANY, _("modify"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryHaedSizer->Add( appBinaryModifyButton, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	appBinaryMainSizer->Add( appBinaryHaedSizer, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* appBinaryChecksumSizer;
	appBinaryChecksumSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	appBinaryChecksumSizer->SetFlexibleDirection( wxBOTH );
	appBinaryChecksumSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	appBinaryCsumRegLabel = new wxStaticText( appBinaryPage, wxID_ANY, _("Checksum (registered):"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryCsumRegLabel->Wrap( -1 );
	appBinaryCsumRegLabel->SetMinSize( wxSize( 165,-1 ) );
	
	appBinaryChecksumSizer->Add( appBinaryCsumRegLabel, 0, wxALL, 5 );
	
	appBinaryCsumRegText = new wxStaticText( appBinaryPage, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryCsumRegText->Wrap( -1 );
	appBinaryChecksumSizer->Add( appBinaryCsumRegText, 0, wxALL, 5 );
	
	appBinaryCsumCurLabel = new wxStaticText( appBinaryPage, wxID_ANY, _("Checksum (current):"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryCsumCurLabel->Wrap( -1 );
	appBinaryChecksumSizer->Add( appBinaryCsumCurLabel, 0, wxALIGN_TOP|wxALL, 5 );
	
	appBinaryCsumCurText = new wxStaticText( appBinaryPage, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryCsumCurText->Wrap( -1 );
	appBinaryChecksumSizer->Add( appBinaryCsumCurText, 0, wxALL, 5 );
	
	appBinaryMainSizer->Add( appBinaryChecksumSizer, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* appBinaryStatusSizer;
	appBinaryStatusSizer = new wxFlexGridSizer( 2, 5, 0, 0 );
	appBinaryStatusSizer->SetFlexibleDirection( wxBOTH );
	appBinaryStatusSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	appBinaryStatusLabel = new wxStaticText( appBinaryPage, wxID_ANY, _("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryStatusLabel->Wrap( -1 );
	appBinaryStatusLabel->SetMinSize( wxSize( 165,-1 ) );
	
	appBinaryStatusSizer->Add( appBinaryStatusLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appBinaryStatusText = new wxStaticText( appBinaryPage, wxID_ANY, _("mismatch"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryStatusText->Wrap( -1 );
	appBinaryStatusSizer->Add( appBinaryStatusText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appBinaryValidateButton = new wxButton( appBinaryPage, wxID_ANY, _("validate"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryStatusSizer->Add( appBinaryValidateButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appBinaryUpdateButton = new wxButton( appBinaryPage, wxID_ANY, _("update"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryStatusSizer->Add( appBinaryUpdateButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appBinaryMainSizer->Add( appBinaryStatusSizer, 1, wxEXPAND, 5 );
	
	appBinaryPage->SetSizer( appBinaryMainSizer );
	appBinaryPage->Layout();
	appBinaryMainSizer->Fit( appBinaryPage );
	appPolicyPanels->AddPage( appBinaryPage, _("Binaries"), false );
	
	mainPanelSizer->Add( appPolicyPanels, 1, wxEXPAND | wxALL, 5 );
	
	filterPolicyPanels = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	filterCommonPage = new wxScrolledWindow( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	filterCommonPage->SetScrollRate( 5, 5 );
	filterCommonPage->Enable( false );
	
	wxFlexGridSizer* filterCommonSizer;
	filterCommonSizer = new wxFlexGridSizer( 3, 4, 0, 0 );
	filterCommonSizer->SetFlexibleDirection( wxBOTH );
	filterCommonSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	filterCommonActionText = new wxStaticText( filterCommonPage, wxID_ANY, _("Action:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterCommonActionText->Wrap( -1 );
	filterCommonSizer->Add( filterCommonActionText, 0, wxALL, 5 );
	
	filterCommonAllowRadioButton = new wxRadioButton( filterCommonPage, wxID_ANY, _("allow"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	filterCommonSizer->Add( filterCommonAllowRadioButton, 0, wxALL, 5 );
	
	filterCommonDenyRadioButton = new wxRadioButton( filterCommonPage, wxID_ANY, _("deny"), wxDefaultPosition, wxDefaultSize, 0 );
	filterCommonSizer->Add( filterCommonDenyRadioButton, 0, wxALL, 5 );
	
	filterCommonAskRadioButton = new wxRadioButton( filterCommonPage, wxID_ANY, _("ask"), wxDefaultPosition, wxDefaultSize, 0 );
	filterCommonSizer->Add( filterCommonAskRadioButton, 0, wxALL, 5 );
	
	filterCommonLogText = new wxStaticText( filterCommonPage, wxID_ANY, _("Log:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterCommonLogText->Wrap( -1 );
	filterCommonSizer->Add( filterCommonLogText, 0, wxALL, 5 );
	
	filterCommonLogNoneRadioButton = new wxRadioButton( filterCommonPage, wxID_ANY, _("none"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	filterCommonSizer->Add( filterCommonLogNoneRadioButton, 0, wxALL, 5 );
	
	filterCommonLogNormalRadioButton = new wxRadioButton( filterCommonPage, wxID_ANY, _("normal"), wxDefaultPosition, wxDefaultSize, 0 );
	filterCommonSizer->Add( filterCommonLogNormalRadioButton, 0, wxALL, 5 );
	
	filterCommonLogAlertRadioButton = new wxRadioButton( filterCommonPage, wxID_ANY, _("alert"), wxDefaultPosition, wxDefaultSize, 0 );
	filterCommonSizer->Add( filterCommonLogAlertRadioButton, 0, wxALL, 5 );
	
	filterCommonPage->SetSizer( filterCommonSizer );
	filterCommonPage->Layout();
	filterCommonSizer->Fit( filterCommonPage );
	filterPolicyPanels->AddPage( filterCommonPage, _("Action / Log"), false );
	filterNetworkPage = new wxScrolledWindow( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	filterNetworkPage->SetScrollRate( 5, 5 );
	filterNetworkPage->Enable( false );
	filterNetworkPage->Hide();
	
	wxFlexGridSizer* filterNetworkMainSizer;
	filterNetworkMainSizer = new wxFlexGridSizer( 3, 4, 0, 0 );
	filterNetworkMainSizer->SetFlexibleDirection( wxBOTH );
	filterNetworkMainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	filterNetworkDirectionLabel = new wxStaticText( filterNetworkPage, wxID_ANY, _("Direction:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterNetworkDirectionLabel->Wrap( -1 );
	filterNetworkMainSizer->Add( filterNetworkDirectionLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkInRadioButton = new wxRadioButton( filterNetworkPage, wxID_ANY, _("accept"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	filterNetworkInRadioButton->SetMinSize( wxSize( 80,-1 ) );
	
	filterNetworkMainSizer->Add( filterNetworkInRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkOutRadionButton = new wxRadioButton( filterNetworkPage, wxID_ANY, _("connect"), wxDefaultPosition, wxDefaultSize, 0 );
	filterNetworkMainSizer->Add( filterNetworkOutRadionButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkBothRadioButton = new wxRadioButton( filterNetworkPage, wxID_ANY, _("both"), wxDefaultPosition, wxDefaultSize, 0 );
	filterNetworkMainSizer->Add( filterNetworkBothRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkAddrFamilyLabel = new wxStaticText( filterNetworkPage, wxID_ANY, _("Address family:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterNetworkAddrFamilyLabel->Wrap( -1 );
	filterNetworkMainSizer->Add( filterNetworkAddrFamilyLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkInetRadioButton = new wxRadioButton( filterNetworkPage, wxID_ANY, _("inet"), wxDefaultPosition, wxSize( -1,-1 ), wxRB_GROUP );
	filterNetworkMainSizer->Add( filterNetworkInetRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkInet6RadioButton = new wxRadioButton( filterNetworkPage, wxID_ANY, _("inet6"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	filterNetworkMainSizer->Add( filterNetworkInet6RadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkAnyRadioButton = new wxRadioButton( filterNetworkPage, wxID_ANY, _("any"), wxDefaultPosition, wxDefaultSize, 0 );
	filterNetworkMainSizer->Add( filterNetworkAnyRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkProtocolLabel = new wxStaticText( filterNetworkPage, wxID_ANY, _("Protocol:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterNetworkProtocolLabel->Wrap( -1 );
	filterNetworkMainSizer->Add( filterNetworkProtocolLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkTcpRadioButton = new wxRadioButton( filterNetworkPage, wxID_ANY, _("tcp"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	filterNetworkMainSizer->Add( filterNetworkTcpRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkUdpRadioButton = new wxRadioButton( filterNetworkPage, wxID_ANY, _("udp"), wxDefaultPosition, wxDefaultSize, 0 );
	filterNetworkMainSizer->Add( filterNetworkUdpRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	filterNetworkMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	filterNetworkStateTimeoutLabel = new wxStaticText( filterNetworkPage, wxID_ANY, _("State timeout:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterNetworkStateTimeoutLabel->Wrap( -1 );
	filterNetworkMainSizer->Add( filterNetworkStateTimeoutLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkStateTimeoutSpinCtrl = new wxSpinCtrl( filterNetworkPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 604800, 0 );
	filterNetworkStateTimeoutSpinCtrl->SetToolTip( _("To enable stateful filtering use the timeout 600.") );
	
	filterNetworkMainSizer->Add( filterNetworkStateTimeoutSpinCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterNetworkPage->SetSizer( filterNetworkMainSizer );
	filterNetworkPage->Layout();
	filterNetworkMainSizer->Fit( filterNetworkPage );
	filterPolicyPanels->AddPage( filterNetworkPage, _("Network"), false );
	filterAddressPage = new wxScrolledWindow( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	filterAddressPage->SetScrollRate( 5, 5 );
	filterAddressPage->Enable( false );
	filterAddressPage->Hide();
	
	filterAddressMainSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
	filterAddressMainSizer->AddGrowableCol( 1 );
	filterAddressMainSizer->SetFlexibleDirection( wxBOTH );
	filterAddressMainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	filterAddressSourceLabel = new wxStaticText( filterAddressPage, wxID_ANY, _("Source address:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterAddressSourceLabel->Wrap( -1 );
	filterAddressMainSizer->Add( filterAddressSourceLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterAddressSourceTextCtrl = new wxTextCtrl( filterAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	filterAddressSourceTextCtrl->SetMinSize( wxSize( 200,-1 ) );
	
	filterAddressMainSizer->Add( filterAddressSourceTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	filterAddressSourceDelimiterLabel = new wxStaticText( filterAddressPage, wxID_ANY, _(" / "), wxDefaultPosition, wxDefaultSize, 0 );
	filterAddressSourceDelimiterLabel->Wrap( -1 );
	filterAddressMainSizer->Add( filterAddressSourceDelimiterLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterAddressSoruceNetSpinCtrl = new wxSpinCtrl( filterAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 55,-1 ), wxSP_ARROW_KEYS, 0, 128, 0 );
	filterAddressMainSizer->Add( filterAddressSoruceNetSpinCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterAddressSourcePortLabel = new wxStaticText( filterAddressPage, wxID_ANY, _("Source Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterAddressSourcePortLabel->Wrap( -1 );
	filterAddressMainSizer->Add( filterAddressSourcePortLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterAddressSorucePortTextCtrl = new wxTextCtrl( filterAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	filterAddressMainSizer->Add( filterAddressSorucePortTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	filterAddressMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterAddressMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	filterAddressDestinationLabel = new wxStaticText( filterAddressPage, wxID_ANY, _("Destination address:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterAddressDestinationLabel->Wrap( -1 );
	filterAddressMainSizer->Add( filterAddressDestinationLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterAddressDestinationTextCtrl = new wxTextCtrl( filterAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	filterAddressMainSizer->Add( filterAddressDestinationTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	filterAddressDestinationDelimiterLabel = new wxStaticText( filterAddressPage, wxID_ANY, _(" / "), wxDefaultPosition, wxDefaultSize, 0 );
	filterAddressDestinationDelimiterLabel->Wrap( -1 );
	filterAddressMainSizer->Add( filterAddressDestinationDelimiterLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterAddressDestinationNetSpinCtrl = new wxSpinCtrl( filterAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 55,-1 ), wxSP_ARROW_KEYS, 0, 128, 0 );
	filterAddressMainSizer->Add( filterAddressDestinationNetSpinCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterAddressDestinationPortLabel = new wxStaticText( filterAddressPage, wxID_ANY, _("Destination Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterAddressDestinationPortLabel->Wrap( -1 );
	filterAddressMainSizer->Add( filterAddressDestinationPortLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterAddressDestinationPortTextCtrl = new wxTextCtrl( filterAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	filterAddressMainSizer->Add( filterAddressDestinationPortTextCtrl, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterAddressPage->SetSizer( filterAddressMainSizer );
	filterAddressPage->Layout();
	filterAddressMainSizer->Fit( filterAddressPage );
	filterPolicyPanels->AddPage( filterAddressPage, _("Addresses"), false );
	filterCapabilityPage = new wxScrolledWindow( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	filterCapabilityPage->SetScrollRate( 5, 5 );
	filterCapabilityPage->Enable( false );
	filterCapabilityPage->Hide();
	
	wxFlexGridSizer* filterCapabilityMainSizer;
	filterCapabilityMainSizer = new wxFlexGridSizer( 1, 4, 0, 0 );
	filterCapabilityMainSizer->SetFlexibleDirection( wxBOTH );
	filterCapabilityMainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	filterCapabilityLabel = new wxStaticText( filterCapabilityPage, wxID_ANY, _("Capability:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterCapabilityLabel->Wrap( -1 );
	filterCapabilityMainSizer->Add( filterCapabilityLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterCapabilityRawRadioButton = new wxRadioButton( filterCapabilityPage, wxID_ANY, _("raw"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	filterCapabilityMainSizer->Add( filterCapabilityRawRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterCapabilityOtherRadioButton = new wxRadioButton( filterCapabilityPage, wxID_ANY, _("other"), wxDefaultPosition, wxDefaultSize, 0 );
	filterCapabilityMainSizer->Add( filterCapabilityOtherRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterCapabilityAllRadioButton = new wxRadioButton( filterCapabilityPage, wxID_ANY, _("all"), wxDefaultPosition, wxDefaultSize, 0 );
	filterCapabilityMainSizer->Add( filterCapabilityAllRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterCapabilityPage->SetSizer( filterCapabilityMainSizer );
	filterCapabilityPage->Layout();
	filterCapabilityMainSizer->Fit( filterCapabilityPage );
	filterPolicyPanels->AddPage( filterCapabilityPage, _("Capability"), false );
	filterSubjectPage = new wxScrolledWindow( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	filterSubjectPage->SetScrollRate( 5, 5 );
	filterSubjectPage->Enable( false );
	filterSubjectPage->Hide();
	
	wxFlexGridSizer* filterSubjectMainSizer;
	filterSubjectMainSizer = new wxFlexGridSizer( 7, 3, 0, 0 );
	filterSubjectMainSizer->AddGrowableCol( 1 );
	filterSubjectMainSizer->SetFlexibleDirection( wxBOTH );
	filterSubjectMainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	filterSubjectPathLabel = new wxStaticText( filterSubjectPage, wxID_ANY, _("Path:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterSubjectPathLabel->Wrap( -1 );
	filterSubjectMainSizer->Add( filterSubjectPathLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterSubjectPathTextCtrl = new wxTextCtrl( filterSubjectPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	filterSubjectMainSizer->Add( filterSubjectPathTextCtrl, 0, wxALL|wxEXPAND, 5 );
	
	filterSubjectPathModifyButton = new wxButton( filterSubjectPage, wxID_ANY, _("modify"), wxDefaultPosition, wxDefaultSize, 0 );
	filterSubjectMainSizer->Add( filterSubjectPathModifyButton, 0, wxALL, 5 );
	
	filterSubjectLabel = new wxStaticText( filterSubjectPage, wxID_ANY, _("Subject:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterSubjectLabel->Wrap( -1 );
	filterSubjectMainSizer->Add( filterSubjectLabel, 0, wxALL, 5 );
	
	filterSubjectAnyRadioButton = new wxRadioButton( filterSubjectPage, wxID_ANY, _("any"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	filterSubjectMainSizer->Add( filterSubjectAnyRadioButton, 0, wxALL, 5 );
	
	
	filterSubjectMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterSubjectMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	filterSubjectSelfRadioButton = new wxRadioButton( filterSubjectPage, wxID_ANY, _("self"), wxDefaultPosition, wxDefaultSize, 0 );
	filterSubjectMainSizer->Add( filterSubjectSelfRadioButton, 0, wxALL, 5 );
	
	
	filterSubjectMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterSubjectMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	filterSubjectSelfSignedRadioButton = new wxRadioButton( filterSubjectPage, wxID_ANY, _("self-signed"), wxDefaultPosition, wxDefaultSize, 0 );
	filterSubjectMainSizer->Add( filterSubjectSelfSignedRadioButton, 0, wxALL, 5 );
	
	
	filterSubjectMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterSubjectMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* filterSubjectUidSizer;
	filterSubjectUidSizer = new wxBoxSizer( wxHORIZONTAL );
	
	filterSubjectUidRadioButton = new wxRadioButton( filterSubjectPage, wxID_ANY, _("uid"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	filterSubjectUidSizer->Add( filterSubjectUidRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterSubjectUidTextCtrl = new wxTextCtrl( filterSubjectPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	filterSubjectUidSizer->Add( filterSubjectUidTextCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterSubjectMainSizer->Add( filterSubjectUidSizer, 1, wxEXPAND, 5 );
	
	
	filterSubjectMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterSubjectMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* filterSubjectKeySizer;
	filterSubjectKeySizer = new wxBoxSizer( wxHORIZONTAL );
	
	filterSubjectKeyLRadioButton = new wxRadioButton( filterSubjectPage, wxID_ANY, _("key"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	filterSubjectKeySizer->Add( filterSubjectKeyLRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterSubjectKeyTextCtrl = new wxTextCtrl( filterSubjectPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	filterSubjectKeySizer->Add( filterSubjectKeyTextCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterSubjectMainSizer->Add( filterSubjectKeySizer, 1, wxEXPAND, 5 );
	
	
	filterSubjectMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterSubjectMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* filterSubjectCsumSizer;
	filterSubjectCsumSizer = new wxBoxSizer( wxHORIZONTAL );
	
	filterSubjectCsumRadioButton = new wxRadioButton( filterSubjectPage, wxID_ANY, _("csum"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	filterSubjectCsumSizer->Add( filterSubjectCsumRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterSubjectCsumTextCtrl = new wxTextCtrl( filterSubjectPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	filterSubjectCsumSizer->Add( filterSubjectCsumTextCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterSubjectMainSizer->Add( filterSubjectCsumSizer, 1, wxEXPAND, 5 );
	
	filterSubjectPage->SetSizer( filterSubjectMainSizer );
	filterSubjectPage->Layout();
	filterSubjectMainSizer->Fit( filterSubjectPage );
	filterPolicyPanels->AddPage( filterSubjectPage, _("Subject"), false );
	filterSfsPage = new wxScrolledWindow( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	filterSfsPage->SetScrollRate( 5, 5 );
	filterSfsPage->Enable( false );
	filterSfsPage->Hide();
	
	wxFlexGridSizer* filterSfsMainSizer;
	filterSfsMainSizer = new wxFlexGridSizer( 2, 3, 0, 0 );
	filterSfsMainSizer->SetFlexibleDirection( wxBOTH );
	filterSfsMainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	filterSfsValidLabel = new wxStaticText( filterSfsPage, wxID_ANY, _("Valid:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterSfsValidLabel->Wrap( -1 );
	filterSfsMainSizer->Add( filterSfsValidLabel, 0, wxALL, 5 );
	
	
	filterSfsMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterSfsMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterSfsMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString filterSfsValidActionRadioBoxChoices[] = { _("allow"), _("deny"), _("ask") };
	int filterSfsValidActionRadioBoxNChoices = sizeof( filterSfsValidActionRadioBoxChoices ) / sizeof( wxString );
	filterSfsValidActionRadioBox = new wxRadioBox( filterSfsPage, wxID_ANY, _("Action"), wxDefaultPosition, wxDefaultSize, filterSfsValidActionRadioBoxNChoices, filterSfsValidActionRadioBoxChoices, 1, wxRA_SPECIFY_ROWS );
	filterSfsValidActionRadioBox->SetSelection( 0 );
	filterSfsMainSizer->Add( filterSfsValidActionRadioBox, 0, wxALL, 5 );
	
	wxString filterSfsValidLogRadioBoxChoices[] = { _("none"), _("normal"), _("alert") };
	int filterSfsValidLogRadioBoxNChoices = sizeof( filterSfsValidLogRadioBoxChoices ) / sizeof( wxString );
	filterSfsValidLogRadioBox = new wxRadioBox( filterSfsPage, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, filterSfsValidLogRadioBoxNChoices, filterSfsValidLogRadioBoxChoices, 1, wxRA_SPECIFY_ROWS );
	filterSfsValidLogRadioBox->SetSelection( 2 );
	filterSfsMainSizer->Add( filterSfsValidLogRadioBox, 0, wxALL, 5 );
	
	filterSfsInvalidLabel = new wxStaticText( filterSfsPage, wxID_ANY, _("Invalid:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterSfsInvalidLabel->Wrap( -1 );
	filterSfsMainSizer->Add( filterSfsInvalidLabel, 0, wxALL, 5 );
	
	
	filterSfsMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterSfsMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterSfsMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString filterSfsInvalidActionRadioBoxChoices[] = { _("allow"), _("deny"), _("ask") };
	int filterSfsInvalidActionRadioBoxNChoices = sizeof( filterSfsInvalidActionRadioBoxChoices ) / sizeof( wxString );
	filterSfsInvalidActionRadioBox = new wxRadioBox( filterSfsPage, wxID_ANY, _("Action"), wxDefaultPosition, wxDefaultSize, filterSfsInvalidActionRadioBoxNChoices, filterSfsInvalidActionRadioBoxChoices, 1, wxRA_SPECIFY_ROWS );
	filterSfsInvalidActionRadioBox->SetSelection( 1 );
	filterSfsMainSizer->Add( filterSfsInvalidActionRadioBox, 0, wxALL, 5 );
	
	wxString filterSfsInvalidLogRadioBoxChoices[] = { _("none"), _("normal"), _("alert") };
	int filterSfsInvalidLogRadioBoxNChoices = sizeof( filterSfsInvalidLogRadioBoxChoices ) / sizeof( wxString );
	filterSfsInvalidLogRadioBox = new wxRadioBox( filterSfsPage, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, filterSfsInvalidLogRadioBoxNChoices, filterSfsInvalidLogRadioBoxChoices, 1, wxRA_SPECIFY_ROWS );
	filterSfsInvalidLogRadioBox->SetSelection( 2 );
	filterSfsMainSizer->Add( filterSfsInvalidLogRadioBox, 0, wxALL, 5 );
	
	filterSfsUnknownLabel = new wxStaticText( filterSfsPage, wxID_ANY, _("Unknown:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterSfsUnknownLabel->Wrap( -1 );
	filterSfsMainSizer->Add( filterSfsUnknownLabel, 0, wxALL, 5 );
	
	
	filterSfsMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterSfsMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterSfsMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString filterSfsUnknownActionRadioBoxChoices[] = { _("allow"), _("deny"), _("ask") };
	int filterSfsUnknownActionRadioBoxNChoices = sizeof( filterSfsUnknownActionRadioBoxChoices ) / sizeof( wxString );
	filterSfsUnknownActionRadioBox = new wxRadioBox( filterSfsPage, wxID_ANY, _("Action"), wxDefaultPosition, wxDefaultSize, filterSfsUnknownActionRadioBoxNChoices, filterSfsUnknownActionRadioBoxChoices, 1, wxRA_SPECIFY_ROWS );
	filterSfsUnknownActionRadioBox->SetSelection( 1 );
	filterSfsMainSizer->Add( filterSfsUnknownActionRadioBox, 0, wxALL, 5 );
	
	wxString filterSfsUnknownLogRadioBoxChoices[] = { _("none"), _("normal"), _("alert") };
	int filterSfsUnknownLogRadioBoxNChoices = sizeof( filterSfsUnknownLogRadioBoxChoices ) / sizeof( wxString );
	filterSfsUnknownLogRadioBox = new wxRadioBox( filterSfsPage, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, filterSfsUnknownLogRadioBoxNChoices, filterSfsUnknownLogRadioBoxChoices, 1, wxRA_SPECIFY_ROWS );
	filterSfsUnknownLogRadioBox->SetSelection( 2 );
	filterSfsMainSizer->Add( filterSfsUnknownLogRadioBox, 0, wxALL, 5 );
	
	filterSfsPage->SetSizer( filterSfsMainSizer );
	filterSfsPage->Layout();
	filterSfsMainSizer->Fit( filterSfsPage );
	filterPolicyPanels->AddPage( filterSfsPage, _("Action / Log"), false );
	filterContextPage = new wxScrolledWindow( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	filterContextPage->SetScrollRate( 5, 5 );
	filterContextPage->Enable( false );
	filterContextPage->Hide();
	
	wxBoxSizer* filterContextMainSizer;
	filterContextMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* filterContextHeadSizer;
	filterContextHeadSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
	filterContextHeadSizer->SetFlexibleDirection( wxBOTH );
	filterContextHeadSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	filterContextTypeLabel = new wxStaticText( filterContextPage, wxID_ANY, _("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextTypeLabel->Wrap( -1 );
	filterContextHeadSizer->Add( filterContextTypeLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* filterContextTypeSizer;
	filterContextTypeSizer = new wxBoxSizer( wxHORIZONTAL );
	
	filterContextNewRadioButton = new wxRadioButton( filterContextPage, wxID_ANY, _("new"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	filterContextTypeSizer->Add( filterContextNewRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterContextOpenRadioButton = new wxRadioButton( filterContextPage, wxID_ANY, _("open"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextTypeSizer->Add( filterContextOpenRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterContextHeadSizer->Add( filterContextTypeSizer, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	filterContextHeadSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	filterContextHeadSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	filterContextBinaryLabel = new wxStaticText( filterContextPage, wxID_ANY, _("Binary:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextBinaryLabel->Wrap( -1 );
	filterContextHeadSizer->Add( filterContextBinaryLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterContextBinaryTextCtrl = new wxTextCtrl( filterContextPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	filterContextBinaryTextCtrl->SetMinSize( wxSize( 300,-1 ) );
	
	filterContextHeadSizer->Add( filterContextBinaryTextCtrl, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterContextBinaryModifyButton = new wxButton( filterContextPage, wxID_ANY, _("modify"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextHeadSizer->Add( filterContextBinaryModifyButton, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterContextMainSizer->Add( filterContextHeadSizer, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* filterContextChecksumSizer;
	filterContextChecksumSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	filterContextChecksumSizer->SetFlexibleDirection( wxBOTH );
	filterContextChecksumSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	filterContextCsumRegLabel = new wxStaticText( filterContextPage, wxID_ANY, _("Checksum (registered):"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextCsumRegLabel->Wrap( -1 );
	filterContextChecksumSizer->Add( filterContextCsumRegLabel, 0, wxALL, 5 );
	
	filterContextCsumRegText = new wxStaticText( filterContextPage, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextCsumRegText->Wrap( -1 );
	filterContextChecksumSizer->Add( filterContextCsumRegText, 0, wxALL, 5 );
	
	filterContextCsumCurLabel = new wxStaticText( filterContextPage, wxID_ANY, _("Checksum (current):"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextCsumCurLabel->Wrap( -1 );
	filterContextChecksumSizer->Add( filterContextCsumCurLabel, 0, wxALL, 5 );
	
	filterContextCsumCurText = new wxStaticText( filterContextPage, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextCsumCurText->Wrap( -1 );
	filterContextChecksumSizer->Add( filterContextCsumCurText, 0, wxALL, 5 );
	
	filterContextMainSizer->Add( filterContextChecksumSizer, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* filterContextStatusSizer;
	filterContextStatusSizer = new wxFlexGridSizer( 2, 5, 0, 0 );
	filterContextStatusSizer->SetFlexibleDirection( wxBOTH );
	filterContextStatusSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	filterContextStatusLabel = new wxStaticText( filterContextPage, wxID_ANY, _("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextStatusLabel->Wrap( -1 );
	filterContextStatusLabel->SetMinSize( wxSize( 165,-1 ) );
	
	filterContextStatusSizer->Add( filterContextStatusLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterContextStatusText = new wxStaticText( filterContextPage, wxID_ANY, _("mismatch"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextStatusText->Wrap( -1 );
	filterContextStatusSizer->Add( filterContextStatusText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterContextValidateButton = new wxButton( filterContextPage, wxID_ANY, _("validate"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextStatusSizer->Add( filterContextValidateButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterContextUpdateButton = new wxButton( filterContextPage, wxID_ANY, _("update"), wxDefaultPosition, wxDefaultSize, 0 );
	filterContextStatusSizer->Add( filterContextUpdateButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterContextMainSizer->Add( filterContextStatusSizer, 1, wxEXPAND, 5 );
	
	filterContextPage->SetSizer( filterContextMainSizer );
	filterContextPage->Layout();
	filterContextMainSizer->Fit( filterContextPage );
	filterPolicyPanels->AddPage( filterContextPage, _("Context"), false );
	filterPermissionPage = new wxScrolledWindow( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	filterPermissionPage->SetScrollRate( 5, 5 );
	filterPermissionPage->Enable( false );
	filterPermissionPage->Hide();
	
	wxFlexGridSizer* filterPermissionMainSizer;
	filterPermissionMainSizer = new wxFlexGridSizer( 1, 4, 0, 0 );
	filterPermissionMainSizer->SetFlexibleDirection( wxBOTH );
	filterPermissionMainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	filterPermissionLabel = new wxStaticText( filterPermissionPage, wxID_ANY, _("Permission:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterPermissionLabel->Wrap( -1 );
	filterPermissionMainSizer->Add( filterPermissionLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterPermissionReadCheckBox = new wxCheckBox( filterPermissionPage, wxID_ANY, _("read"), wxDefaultPosition, wxDefaultSize, 0 );
	
	filterPermissionMainSizer->Add( filterPermissionReadCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterPermissionWriteCheckBox = new wxCheckBox( filterPermissionPage, wxID_ANY, _("write"), wxDefaultPosition, wxDefaultSize, 0 );
	
	filterPermissionMainSizer->Add( filterPermissionWriteCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterPermissionExecuteCheckBox = new wxCheckBox( filterPermissionPage, wxID_ANY, _("execute"), wxDefaultPosition, wxDefaultSize, 0 );
	
	filterPermissionMainSizer->Add( filterPermissionExecuteCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterPermissionPage->SetSizer( filterPermissionMainSizer );
	filterPermissionPage->Layout();
	filterPermissionMainSizer->Fit( filterPermissionPage );
	filterPolicyPanels->AddPage( filterPermissionPage, _("Permission"), true );
	
	mainPanelSizer->Add( filterPolicyPanels, 1, wxEXPAND | wxALL, 5 );
	
	mainSizer->Add( mainPanelSizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* mainFootSizer;
	mainFootSizer = new wxBoxSizer( wxHORIZONTAL );
	
	mainFooterRuleSetLabel = new wxStaticText( this, wxID_ANY, _("RuleSet:"), wxDefaultPosition, wxDefaultSize, 0 );
	mainFooterRuleSetLabel->Wrap( -1 );
	mainFootSizer->Add( mainFooterRuleSetLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainFooterRuleSetText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	mainFooterRuleSetText->Wrap( -1 );
	mainFootSizer->Add( mainFooterRuleSetText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainFooterReloadButton = new wxButton( this, wxID_ANY, _("relaod"), wxDefaultPosition, wxDefaultSize, 0 );
	mainFooterReloadButton->Enable( false );
	
	mainFootSizer->Add( mainFooterReloadButton, 0, wxALL, 5 );
	
	
	mainFootSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	mainFooterStatusLabel = new wxStaticText( this, wxID_ANY, _("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	mainFooterStatusLabel->Wrap( -1 );
	mainFootSizer->Add( mainFooterStatusLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainFooterStatusText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	mainFooterStatusText->Wrap( -1 );
	mainFootSizer->Add( mainFooterStatusText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainFooterSaveButton = new wxButton( this, wxID_ANY, _("save"), wxDefaultPosition, wxDefaultSize, 0 );
	mainFooterSaveButton->Enable( false );
	
	mainFootSizer->Add( mainFooterSaveButton, 0, wxALL, 5 );
	
	mainSizer->Add( mainFootSizer, 0, wxEXPAND, 5 );
	
	this->SetSizer( mainSizer );
	this->Layout();
	
	// Connect Events
	appBinaryTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorBase::OnAppBinaryTextCtrl ), NULL, this );
	appBinaryModifyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppBinaryModifyButton ), NULL, this );
	appBinaryValidateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppValidateChkSumButton ), NULL, this );
	appBinaryUpdateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppUpdateChkSumButton ), NULL, this );
	filterCommonAllowRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfAllowRadioButton ), NULL, this );
	filterCommonDenyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfDenyRadioButton ), NULL, this );
	filterCommonAskRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfAskRadioButton ), NULL, this );
	filterCommonLogNoneRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfFilterRadioButton ), NULL, this );
	filterCommonLogNormalRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfCapRadioButton ), NULL, this );
	filterCommonLogAlertRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfDefaultRadioButton ), NULL, this );
	filterNetworkInRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfAcceptRadioButton ), NULL, this );
	filterNetworkOutRadionButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfConnectRadioButton ), NULL, this );
	filterNetworkInetRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfInetRadioButton ), NULL, this );
	filterNetworkInet6RadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfInet6RadioButton ), NULL, this );
	filterNetworkAnyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfAnyRadioButton ), NULL, this );
	filterNetworkTcpRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfTcpRadioButton ), NULL, this );
	filterNetworkUdpRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfUdpRadioButton ), NULL, this );
	filterNetworkStateTimeoutSpinCtrl->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( DlgRuleEditorBase::OnAlfStateTimeoutChange ), NULL, this );
	filterAddressSourceTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorBase::onAlfSrcAddrTextCtrlEnter ), NULL, this );
	filterAddressSoruceNetSpinCtrl->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( DlgRuleEditorBase::OnAlfSrcNetmaskSpinCtrl ), NULL, this );
	filterAddressSorucePortTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorBase::onAlfSrcPortTextCtrlEnter ), NULL, this );
	filterAddressDestinationTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorBase::onAlfDstAddrTextCtrlEnter ), NULL, this );
	filterAddressDestinationNetSpinCtrl->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( DlgRuleEditorBase::OnAlfDstNetmaskSpinCtrl ), NULL, this );
	filterAddressDestinationPortTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorBase::onAlfDstPortTextCtrlEnter ), NULL, this );
	filterCapabilityRawRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfAllowRadioButton ), NULL, this );
	filterCapabilityOtherRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfDenyRadioButton ), NULL, this );
	filterCapabilityAllRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfAskRadioButton ), NULL, this );
	filterContextBinaryTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorBase::OnAppBinaryTextCtrl ), NULL, this );
	filterContextBinaryModifyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppBinaryModifyButton ), NULL, this );
	filterContextValidateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppValidateChkSumButton ), NULL, this );
	filterContextUpdateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppUpdateChkSumButton ), NULL, this );
}
