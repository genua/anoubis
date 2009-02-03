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

#include "DlgRuleEditorFilterActionPage.h"
#include "DlgRuleEditorFilterAddressPage.h"
#include "DlgRuleEditorFilterCapabilityPage.h"
#include "DlgRuleEditorFilterContextPage.h"
#include "DlgRuleEditorFilterNetworkPage.h"
#include "DlgRuleEditorFilterPermissionPage.h"
#include "DlgRuleEditorFilterSfsPage.h"
#include "DlgRuleEditorFilterSubjectPage.h"

#include "DlgRuleEditorBase.h"

///////////////////////////////////////////////////////////////////////////

DlgRuleEditorBase::DlgRuleEditorBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->Centre( wxBOTH );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	splitterWindow = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_LIVE_UPDATE );
	splitterWindow->SetMinimumPaneSize( 200 );
	splitterWindow->Connect( wxEVT_IDLE, wxIdleEventHandler( DlgRuleEditorBase::splitterWindowOnIdle ), NULL, this );
	appPanel = new wxPanel( splitterWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* appMainSizer;
	appMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* appListHeadSizer;
	appListHeadSizer = new wxBoxSizer( wxHORIZONTAL );
	
	appListTypeLabel = new wxStaticText( appPanel, wxID_ANY, _("App-Rules"), wxDefaultPosition, wxDefaultSize, 0 );
	appListTypeLabel->Wrap( -1 );
	appListHeadSizer->Add( appListTypeLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString appListTypeChoiceChoices[] = { _("ALF"), _("SB"), _("CTX") };
	int appListTypeChoiceNChoices = sizeof( appListTypeChoiceChoices ) / sizeof( wxString );
	appListTypeChoice = new wxChoice( appPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, appListTypeChoiceNChoices, appListTypeChoiceChoices, 0 );
	appListHeadSizer->Add( appListTypeChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appListCreateButton = new wxButton( appPanel, wxID_ANY, _("create"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListHeadSizer->Add( appListCreateButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	appListHeadSizer->Add( 0, 0, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	appListColumnsButton = new wxButton( appPanel, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListColumnsButton->Enable( false );
	appListColumnsButton->SetToolTip( _("customise columns of app-rules list") );
	
	appListHeadSizer->Add( appListColumnsButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	appMainSizer->Add( appListHeadSizer, 0, wxEXPAND, 5 );
	
	appPolicyListCtrl = new wxListCtrl( appPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL );
	appMainSizer->Add( appPolicyListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* appListFoodSizer;
	appListFoodSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	appListFoodSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	appListPolicyLabel = new wxStaticText( appPanel, wxID_ANY, _("Rule:"), wxDefaultPosition, wxDefaultSize, 0 );
	appListPolicyLabel->Wrap( -1 );
	appListFoodSizer->Add( appListPolicyLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appListPolicyText = new wxStaticText( appPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	appListPolicyText->Wrap( -1 );
	appListFoodSizer->Add( appListPolicyText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appListUpButton = new wxButton( appPanel, wxID_ANY, _("up"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListUpButton->Enable( false );
	
	appListFoodSizer->Add( appListUpButton, 0, wxALL, 5 );
	
	appListDownButton = new wxButton( appPanel, wxID_ANY, _("down"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListDownButton->Enable( false );
	
	appListFoodSizer->Add( appListDownButton, 0, wxALL, 5 );
	
	appListDeleteButton = new wxButton( appPanel, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListDeleteButton->Enable( false );
	
	appListFoodSizer->Add( appListDeleteButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appMainSizer->Add( appListFoodSizer, 0, wxEXPAND|wxALIGN_RIGHT, 5 );
	
	appPolicyPanels = new wxNotebook( appPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
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
	
	appMainSizer->Add( appPolicyPanels, 1, wxEXPAND | wxALL, 5 );
	
	appPanel->SetSizer( appMainSizer );
	appPanel->Layout();
	appMainSizer->Fit( appPanel );
	filterPanel = new wxPanel( splitterWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* filterMainSizer;
	filterMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* filterListHeadSizer;
	filterListHeadSizer = new wxBoxSizer( wxHORIZONTAL );
	
	filterListTypeLabel = new wxStaticText( filterPanel, wxID_ANY, _("Filter-Rules"), wxDefaultPosition, wxDefaultSize, 0 );
	filterListTypeLabel->Wrap( -1 );
	filterListHeadSizer->Add( filterListTypeLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString filterListTypeChoiceChoices[] = { _("ALF Filter"), _("ALF Capability"), _("SFS Filter"), _("SB AccessFilter"), _("CTX Filter"), _("Default") };
	int filterListTypeChoiceNChoices = sizeof( filterListTypeChoiceChoices ) / sizeof( wxString );
	filterListTypeChoice = new wxChoice( filterPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, filterListTypeChoiceNChoices, filterListTypeChoiceChoices, 0 );
	filterListHeadSizer->Add( filterListTypeChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterListCreateButton = new wxButton( filterPanel, wxID_ANY, _("create"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListCreateButton->Enable( false );
	
	filterListHeadSizer->Add( filterListCreateButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	filterListHeadSizer->Add( 0, 0, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	filterListColumnsButton = new wxButton( filterPanel, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListColumnsButton->Enable( false );
	filterListColumnsButton->SetToolTip( _("customise columns of filter-rules list") );
	
	filterListHeadSizer->Add( filterListColumnsButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	filterMainSizer->Add( filterListHeadSizer, 0, wxEXPAND, 5 );
	
	filterPolicyListCtrl = new wxListCtrl( filterPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL );
	filterMainSizer->Add( filterPolicyListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* filterListFoodSizer;
	filterListFoodSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	filterListFoodSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	filterListPolicyLabel = new wxStaticText( filterPanel, wxID_ANY, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
	filterListPolicyLabel->Wrap( -1 );
	filterListFoodSizer->Add( filterListPolicyLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterListPolicyText = new wxStaticText( filterPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	filterListPolicyText->Wrap( -1 );
	filterListFoodSizer->Add( filterListPolicyText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterListUpButton = new wxButton( filterPanel, wxID_ANY, _("up"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListUpButton->Enable( false );
	
	filterListFoodSizer->Add( filterListUpButton, 0, wxALL, 5 );
	
	filterListDownButton = new wxButton( filterPanel, wxID_ANY, _("down"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListDownButton->Enable( false );
	
	filterListFoodSizer->Add( filterListDownButton, 0, wxALL, 5 );
	
	filterListDeleteButton = new wxButton( filterPanel, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListDeleteButton->Enable( false );
	
	filterListFoodSizer->Add( filterListDeleteButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterMainSizer->Add( filterListFoodSizer, 0, wxEXPAND|wxALIGN_RIGHT, 5 );
	
	filterPolicyPanels = new wxNotebook( filterPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	filterActionPage = new DlgRuleEditorFilterActionPage( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	filterActionPage->Hide();
	
	filterPolicyPanels->AddPage( filterActionPage, _("Action / Log"), false );
	filterNetworkPage = new DlgRuleEditorFilterNetworkPage( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	filterNetworkPage->Hide();
	
	filterPolicyPanels->AddPage( filterNetworkPage, _("Network"), false );
	fitlerAddressPage = new DlgRuleEditorFilterAddressPage( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	fitlerAddressPage->Hide();
	
	filterPolicyPanels->AddPage( fitlerAddressPage, _("Address"), false );
	filterCapabilityPage = new DlgRuleEditorFilterCapabilityPage( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	filterCapabilityPage->Hide();
	
	filterPolicyPanels->AddPage( filterCapabilityPage, _("Capability"), false );
	filterSubjectPage = new DlgRuleEditorFilterSubjectPage( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	filterSubjectPage->Hide();
	
	filterPolicyPanels->AddPage( filterSubjectPage, _("Subject"), false );
	filterSfsPage = new DlgRuleEditorFilterSfsPage( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	filterSfsPage->Hide();
	
	filterPolicyPanels->AddPage( filterSfsPage, _("Valid / Invalid / Unknown"), false );
	filterContextPage = new DlgRuleEditorFilterContextPage( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	filterContextPage->Hide();
	
	filterPolicyPanels->AddPage( filterContextPage, _("Context"), false );
	filterPermissionPage = new DlgRuleEditorFilterPermissionPage( filterPolicyPanels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	filterPermissionPage->Hide();
	
	filterPolicyPanels->AddPage( filterPermissionPage, _("Permission"), false );
	
	filterMainSizer->Add( filterPolicyPanels, 1, wxEXPAND | wxALL, 5 );
	
	filterPanel->SetSizer( filterMainSizer );
	filterPanel->Layout();
	filterMainSizer->Fit( filterPanel );
	splitterWindow->SplitVertically( appPanel, filterPanel, 482 );
	mainSizer->Add( splitterWindow, 1, wxEXPAND, 5 );
	
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
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DlgRuleEditorBase::onClose ) );
	appListCreateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onAppListCreateButton ), NULL, this );
	appListColumnsButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onAppListColumnsButtonClick ), NULL, this );
	appPolicyListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( DlgRuleEditorBase::onAppPolicyDeSelect ), NULL, this );
	appPolicyListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( DlgRuleEditorBase::onAppPolicySelect ), NULL, this );
	appListUpButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onAppListUpClick ), NULL, this );
	appListDownButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onAppListDownClick ), NULL, this );
	appListDeleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onAppListDeleteClick ), NULL, this );
	appBinaryTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorBase::OnAppBinaryTextCtrl ), NULL, this );
	appBinaryModifyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppBinaryModifyButton ), NULL, this );
	appBinaryValidateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppValidateChkSumButton ), NULL, this );
	appBinaryUpdateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppUpdateChkSumButton ), NULL, this );
	filterListCreateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFilterListCreateButton ), NULL, this );
	filterListColumnsButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFilterListColumnsButtonClick ), NULL, this );
	filterPolicyListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( DlgRuleEditorBase::onFilterPolicyDeSelect ), NULL, this );
	filterPolicyListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( DlgRuleEditorBase::onFilterPolicySelect ), NULL, this );
	filterListUpButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFilterListUpClick ), NULL, this );
	filterListDownButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFilterListDownClick ), NULL, this );
	filterListDeleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFilterListDeleteClick ), NULL, this );
}

DlgRuleEditorFilterActionPageBase::DlgRuleEditorFilterActionPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxVERTICAL );
	
	mainPage = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	mainPage->SetScrollRate( 5, 5 );
	wxFlexGridSizer* mainSizer;
	mainSizer = new wxFlexGridSizer( 3, 4, 0, 0 );
	mainSizer->SetFlexibleDirection( wxBOTH );
	mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	actionLabel = new wxStaticText( mainPage, wxID_ANY, _("Action:"), wxDefaultPosition, wxDefaultSize, 0 );
	actionLabel->Wrap( -1 );
	mainSizer->Add( actionLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	allowRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("allow"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	mainSizer->Add( allowRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	denyRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("deny"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( denyRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	askRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("ask"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( askRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	logLabel = new wxStaticText( mainPage, wxID_ANY, _("Log:"), wxDefaultPosition, wxDefaultSize, 0 );
	logLabel->Wrap( -1 );
	mainSizer->Add( logLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	noneRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("none"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	mainSizer->Add( noneRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	normalRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("normal"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( normalRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alertRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("alert"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( alertRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainPage->SetSizer( mainSizer );
	mainPage->Layout();
	mainSizer->Fit( mainPage );
	pageSizer->Add( mainPage, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	allowRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterActionPageBase::onAllowRadioButton ), NULL, this );
	denyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterActionPageBase::onDenyRadioButton ), NULL, this );
	askRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterActionPageBase::onAskRadioButton ), NULL, this );
	noneRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterActionPageBase::onNoneRadioButton ), NULL, this );
	normalRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterActionPageBase::onNormalRadioButton ), NULL, this );
	alertRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterActionPageBase::onAlertRadioButton ), NULL, this );
}

DlgRuleEditorFilterNetworkPageBase::DlgRuleEditorFilterNetworkPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxVERTICAL );
	
	mainPage = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	mainPage->SetScrollRate( 5, 5 );
	wxFlexGridSizer* mainSizer;
	mainSizer = new wxFlexGridSizer( 3, 4, 0, 0 );
	mainSizer->SetFlexibleDirection( wxBOTH );
	mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	directionLabel = new wxStaticText( mainPage, wxID_ANY, _("Direction:"), wxDefaultPosition, wxDefaultSize, 0 );
	directionLabel->Wrap( -1 );
	mainSizer->Add( directionLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	inRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("accept"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	inRadioButton->SetMinSize( wxSize( 80,-1 ) );
	
	mainSizer->Add( inRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	outRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("connect"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( outRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bothRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("both"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( bothRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	addressFamilyLabel = new wxStaticText( mainPage, wxID_ANY, _("Address family:"), wxDefaultPosition, wxDefaultSize, 0 );
	addressFamilyLabel->Wrap( -1 );
	mainSizer->Add( addressFamilyLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	inetRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("inet"), wxDefaultPosition, wxSize( -1,-1 ), wxRB_GROUP );
	mainSizer->Add( inetRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	inet6RadioButton = new wxRadioButton( mainPage, wxID_ANY, _("inet6"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	mainSizer->Add( inet6RadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	anyRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("any"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( anyRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	protocolLabel = new wxStaticText( mainPage, wxID_ANY, _("Protocol:"), wxDefaultPosition, wxDefaultSize, 0 );
	protocolLabel->Wrap( -1 );
	mainSizer->Add( protocolLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	tcpRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("tcp"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	mainSizer->Add( tcpRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	udpRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("udp"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( udpRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	stateTimeoutLabel = new wxStaticText( mainPage, wxID_ANY, _("State timeout:"), wxDefaultPosition, wxDefaultSize, 0 );
	stateTimeoutLabel->Wrap( -1 );
	mainSizer->Add( stateTimeoutLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	stateTimeoutSpinCtrl = new wxSpinCtrl( mainPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 604800, 0 );
	stateTimeoutSpinCtrl->SetToolTip( _("To enable stateful filtering use the timeout 600.") );
	
	mainSizer->Add( stateTimeoutSpinCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainPage->SetSizer( mainSizer );
	mainPage->Layout();
	mainSizer->Fit( mainPage );
	pageSizer->Add( mainPage, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	inRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterNetworkPageBase::onInRadioButton ), NULL, this );
	outRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterNetworkPageBase::onOutRadioButton ), NULL, this );
	bothRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterNetworkPageBase::onBothRadioButton ), NULL, this );
	inetRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterNetworkPageBase::onInetRadioButton ), NULL, this );
	inet6RadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterNetworkPageBase::onInet6RadioButton ), NULL, this );
	anyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterNetworkPageBase::onAnyRadioButton ), NULL, this );
	tcpRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterNetworkPageBase::onTcpRadioButton ), NULL, this );
	udpRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterNetworkPageBase::onUdpRadioButton ), NULL, this );
	stateTimeoutSpinCtrl->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( DlgRuleEditorFilterNetworkPageBase::onStateTimeoutSpinCtrl ), NULL, this );
}

DlgRuleEditorFilterAddressPageBase::DlgRuleEditorFilterAddressPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxVERTICAL );
	
	mainPage = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	mainPage->SetScrollRate( 5, 5 );
	mainSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	mainSizer->AddGrowableCol( 1 );
	mainSizer->SetFlexibleDirection( wxBOTH );
	mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	sourceAddressLabel = new wxStaticText( mainPage, wxID_ANY, _("Source address:"), wxDefaultPosition, wxDefaultSize, 0 );
	sourceAddressLabel->Wrap( -1 );
	mainSizer->Add( sourceAddressLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sourceAddressTextCtrl = new wxTextCtrl( mainPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	sourceAddressTextCtrl->SetMinSize( wxSize( 200,-1 ) );
	
	mainSizer->Add( sourceAddressTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	sourcePortLabel = new wxStaticText( mainPage, wxID_ANY, _("Source Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	sourcePortLabel->Wrap( -1 );
	mainSizer->Add( sourcePortLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sourcePortTextCtrl = new wxTextCtrl( mainPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	mainSizer->Add( sourcePortTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	destinationAddressLabel = new wxStaticText( mainPage, wxID_ANY, _("Destination address:"), wxDefaultPosition, wxDefaultSize, 0 );
	destinationAddressLabel->Wrap( -1 );
	mainSizer->Add( destinationAddressLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	destinationAddressTextCtrl = new wxTextCtrl( mainPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	mainSizer->Add( destinationAddressTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	destinationPortLabel = new wxStaticText( mainPage, wxID_ANY, _("Destination Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	destinationPortLabel->Wrap( -1 );
	mainSizer->Add( destinationPortLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	destinationPortTextCtrl = new wxTextCtrl( mainPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	mainSizer->Add( destinationPortTextCtrl, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainPage->SetSizer( mainSizer );
	mainPage->Layout();
	mainSizer->Fit( mainPage );
	pageSizer->Add( mainPage, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	sourceAddressTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorFilterAddressPageBase::onSourceAddressTextKillFocus ), NULL, this );
	sourceAddressTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorFilterAddressPageBase::onSourceAddressTextEnter ), NULL, this );
	sourcePortTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorFilterAddressPageBase::onSourcePortTextKillFocus ), NULL, this );
	sourcePortTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorFilterAddressPageBase::onSourcePortTextEnter ), NULL, this );
	destinationAddressTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorFilterAddressPageBase::onDestinationAddressTextKillFocus ), NULL, this );
	destinationAddressTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorFilterAddressPageBase::onDestinationAddressTextEnter ), NULL, this );
	destinationPortTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorFilterAddressPageBase::onDestinationPortTextKillFocus ), NULL, this );
	destinationPortTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorFilterAddressPageBase::onDestinationPortTextEnter ), NULL, this );
}

DlgRuleEditorFilterCapabilityPageBase::DlgRuleEditorFilterCapabilityPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxVERTICAL );
	
	mainPage = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	mainPage->SetScrollRate( 5, 5 );
	wxFlexGridSizer* mainSizer;
	mainSizer = new wxFlexGridSizer( 1, 4, 0, 0 );
	mainSizer->SetFlexibleDirection( wxBOTH );
	mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	capabilityLabel = new wxStaticText( mainPage, wxID_ANY, _("Capability:"), wxDefaultPosition, wxDefaultSize, 0 );
	capabilityLabel->Wrap( -1 );
	mainSizer->Add( capabilityLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	rawRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("raw"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	mainSizer->Add( rawRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	otherRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("other"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( otherRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	allRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("all"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( allRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainPage->SetSizer( mainSizer );
	mainPage->Layout();
	mainSizer->Fit( mainPage );
	pageSizer->Add( mainPage, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	rawRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterCapabilityPageBase::onRawRadioButton ), NULL, this );
	otherRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterCapabilityPageBase::onOtherRadioButton ), NULL, this );
	allRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterCapabilityPageBase::onAllRadioButton ), NULL, this );
}

DlgRuleEditorFilterSubjectPageBase::DlgRuleEditorFilterSubjectPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxVERTICAL );
	
	mainPage = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	mainPage->SetScrollRate( 5, 5 );
	wxFlexGridSizer* mainSizer;
	mainSizer = new wxFlexGridSizer( 7, 3, 0, 0 );
	mainSizer->AddGrowableCol( 1 );
	mainSizer->SetFlexibleDirection( wxBOTH );
	mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	pathLabel = new wxStaticText( mainPage, wxID_ANY, _("Path:"), wxDefaultPosition, wxDefaultSize, 0 );
	pathLabel->Wrap( -1 );
	mainSizer->Add( pathLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	pathTextCtrl = new wxTextCtrl( mainPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	mainSizer->Add( pathTextCtrl, 0, wxALL|wxEXPAND, 5 );
	
	modifyButton = new wxButton( mainPage, wxID_ANY, _("modify"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( modifyButton, 0, wxALL, 5 );
	
	subjectLabel = new wxStaticText( mainPage, wxID_ANY, _("Subject:"), wxDefaultPosition, wxDefaultSize, 0 );
	subjectLabel->Wrap( -1 );
	mainSizer->Add( subjectLabel, 0, wxALL, 5 );
	
	anyRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("any"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	anyRadioButton->Hide();
	
	mainSizer->Add( anyRadioButton, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	selfRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("self"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( selfRadioButton, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	selfSignedRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("self-signed"), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( selfSignedRadioButton, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* uidSizer;
	uidSizer = new wxBoxSizer( wxHORIZONTAL );
	
	uidRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("uid"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	uidSizer->Add( uidRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	uidTextCtrl = new wxTextCtrl( mainPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	uidSizer->Add( uidTextCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainSizer->Add( uidSizer, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* keySizer;
	keySizer = new wxBoxSizer( wxHORIZONTAL );
	
	keyRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("key"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	keySizer->Add( keyRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	keyTextCtrl = new wxTextCtrl( mainPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	keySizer->Add( keyTextCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainSizer->Add( keySizer, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* csumSizer;
	csumSizer = new wxBoxSizer( wxHORIZONTAL );
	
	csumRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("csum"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	csumSizer->Add( csumRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	csumTextCtrl = new wxTextCtrl( mainPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	csumSizer->Add( csumTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	mainSizer->Add( csumSizer, 1, wxEXPAND, 5 );
	
	mainPage->SetSizer( mainSizer );
	mainPage->Layout();
	mainSizer->Fit( mainPage );
	pageSizer->Add( mainPage, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	pathTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorFilterSubjectPageBase::onPathTextKillFocus ), NULL, this );
	pathTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onPathTextEnter ), NULL, this );
	modifyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onModifyButton ), NULL, this );
	anyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onAnyRadioButton ), NULL, this );
	selfRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onSelfRadioButton ), NULL, this );
	selfSignedRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onSelfSignedRadioButton ), NULL, this );
	uidRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onUidRadioButton ), NULL, this );
	uidTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorFilterSubjectPageBase::onUidTextKillFocus ), NULL, this );
	uidTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onUidTextEnter ), NULL, this );
	keyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onKeyRadioButton ), NULL, this );
	keyTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorFilterSubjectPageBase::onKeyTextKillFocus ), NULL, this );
	keyTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onKeyTextEnter ), NULL, this );
	csumRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onCsumRadioButton ), NULL, this );
	csumTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorFilterSubjectPageBase::onCsumTextKillFocus ), NULL, this );
	csumTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onCsumTextEnter ), NULL, this );
}

DlgRuleEditorFilterSfsPageBase::DlgRuleEditorFilterSfsPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxVERTICAL );
	
	mainPage = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	mainPage->SetScrollRate( 5, 5 );
	wxFlexGridSizer* mainSizer;
	mainSizer = new wxFlexGridSizer( 2, 3, 0, 0 );
	mainSizer->SetFlexibleDirection( wxBOTH );
	mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	validLabel = new wxStaticText( mainPage, wxID_ANY, _("Valid:"), wxDefaultPosition, wxDefaultSize, 0 );
	validLabel->Wrap( -1 );
	mainSizer->Add( validLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString validActionRadioBoxChoices[] = { _("allow"), _("deny"), _("ask") };
	int validActionRadioBoxNChoices = sizeof( validActionRadioBoxChoices ) / sizeof( wxString );
	validActionRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Action"), wxDefaultPosition, wxDefaultSize, validActionRadioBoxNChoices, validActionRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	validActionRadioBox->SetSelection( 0 );
	mainSizer->Add( validActionRadioBox, 0, wxALL, 5 );
	
	wxString validLogRadioBoxChoices[] = { _("none"), _("normal"), _("alert") };
	int validLogRadioBoxNChoices = sizeof( validLogRadioBoxChoices ) / sizeof( wxString );
	validLogRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, validLogRadioBoxNChoices, validLogRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	validLogRadioBox->SetSelection( 2 );
	mainSizer->Add( validLogRadioBox, 0, wxALL, 5 );
	
	invalidLabel = new wxStaticText( mainPage, wxID_ANY, _("Invalid:"), wxDefaultPosition, wxDefaultSize, 0 );
	invalidLabel->Wrap( -1 );
	mainSizer->Add( invalidLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString invalidActionRadioBoxChoices[] = { _("allow"), _("deny"), _("ask") };
	int invalidActionRadioBoxNChoices = sizeof( invalidActionRadioBoxChoices ) / sizeof( wxString );
	invalidActionRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Action"), wxDefaultPosition, wxDefaultSize, invalidActionRadioBoxNChoices, invalidActionRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	invalidActionRadioBox->SetSelection( 1 );
	mainSizer->Add( invalidActionRadioBox, 0, wxALL, 5 );
	
	wxString invalidLogRadioBoxChoices[] = { _("none"), _("normal"), _("alert") };
	int invalidLogRadioBoxNChoices = sizeof( invalidLogRadioBoxChoices ) / sizeof( wxString );
	invalidLogRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, invalidLogRadioBoxNChoices, invalidLogRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	invalidLogRadioBox->SetSelection( 2 );
	mainSizer->Add( invalidLogRadioBox, 0, wxALL, 5 );
	
	unknownLabel = new wxStaticText( mainPage, wxID_ANY, _("Unknown:"), wxDefaultPosition, wxDefaultSize, 0 );
	unknownLabel->Wrap( -1 );
	mainSizer->Add( unknownLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString unknownActionRadioBoxChoices[] = { _("allow"), _("deny"), _("ask") };
	int unknownActionRadioBoxNChoices = sizeof( unknownActionRadioBoxChoices ) / sizeof( wxString );
	unknownActionRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Action"), wxDefaultPosition, wxDefaultSize, unknownActionRadioBoxNChoices, unknownActionRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	unknownActionRadioBox->SetSelection( 1 );
	mainSizer->Add( unknownActionRadioBox, 0, wxALL, 5 );
	
	wxString unknownLogRadioBoxChoices[] = { _("none"), _("normal"), _("alert") };
	int unknownLogRadioBoxNChoices = sizeof( unknownLogRadioBoxChoices ) / sizeof( wxString );
	unknownLogRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, unknownLogRadioBoxNChoices, unknownLogRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	unknownLogRadioBox->SetSelection( 2 );
	mainSizer->Add( unknownLogRadioBox, 0, wxALL, 5 );
	
	mainPage->SetSizer( mainSizer );
	mainPage->Layout();
	mainSizer->Fit( mainPage );
	pageSizer->Add( mainPage, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	validActionRadioBox->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSfsPageBase::onValidActionRadioBox ), NULL, this );
	validLogRadioBox->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSfsPageBase::onValidLogRadioBox ), NULL, this );
	invalidActionRadioBox->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSfsPageBase::onInvalidActionRadioBox ), NULL, this );
	invalidLogRadioBox->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSfsPageBase::onInvalidLogRadioBox ), NULL, this );
	unknownActionRadioBox->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSfsPageBase::onUnknownActionRadioBox ), NULL, this );
	unknownLogRadioBox->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSfsPageBase::onUnknownLogRadioBox ), NULL, this );
}

DlgRuleEditorFilterContextPageBase::DlgRuleEditorFilterContextPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxVERTICAL );
	
	mainPage = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	mainPage->SetScrollRate( 5, 5 );
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* headSizer;
	headSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
	headSizer->SetFlexibleDirection( wxBOTH );
	headSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	typeLabel = new wxStaticText( mainPage, wxID_ANY, _("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	typeLabel->Wrap( -1 );
	headSizer->Add( typeLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* typeSizer;
	typeSizer = new wxBoxSizer( wxHORIZONTAL );
	
	newRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("new"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	typeSizer->Add( newRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	openRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("open"), wxDefaultPosition, wxDefaultSize, 0 );
	typeSizer->Add( openRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	headSizer->Add( typeSizer, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	headSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	headSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	binaryLabel = new wxStaticText( mainPage, wxID_ANY, _("Binary:"), wxDefaultPosition, wxDefaultSize, 0 );
	binaryLabel->Wrap( -1 );
	headSizer->Add( binaryLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	binaryTextCtrl = new wxTextCtrl( mainPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	binaryTextCtrl->SetMinSize( wxSize( 300,-1 ) );
	
	headSizer->Add( binaryTextCtrl, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	modifyButton = new wxButton( mainPage, wxID_ANY, _("modify"), wxDefaultPosition, wxDefaultSize, 0 );
	headSizer->Add( modifyButton, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainSizer->Add( headSizer, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* csumSizer;
	csumSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	csumSizer->SetFlexibleDirection( wxBOTH );
	csumSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	registeredCsumLabel = new wxStaticText( mainPage, wxID_ANY, _("Checksum (registered):"), wxDefaultPosition, wxDefaultSize, 0 );
	registeredCsumLabel->Wrap( -1 );
	csumSizer->Add( registeredCsumLabel, 0, wxALL, 5 );
	
	registeredCsumText = new wxStaticText( mainPage, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	registeredCsumText->Wrap( -1 );
	csumSizer->Add( registeredCsumText, 0, wxALL, 5 );
	
	currentCsumLabel = new wxStaticText( mainPage, wxID_ANY, _("Checksum (current):"), wxDefaultPosition, wxDefaultSize, 0 );
	currentCsumLabel->Wrap( -1 );
	csumSizer->Add( currentCsumLabel, 0, wxALL, 5 );
	
	currentCsumText = new wxStaticText( mainPage, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	currentCsumText->Wrap( -1 );
	csumSizer->Add( currentCsumText, 0, wxALL, 5 );
	
	mainSizer->Add( csumSizer, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* statusSizer;
	statusSizer = new wxFlexGridSizer( 2, 5, 0, 0 );
	statusSizer->SetFlexibleDirection( wxBOTH );
	statusSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	statusLabel = new wxStaticText( mainPage, wxID_ANY, _("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	statusLabel->Wrap( -1 );
	statusLabel->SetMinSize( wxSize( 165,-1 ) );
	
	statusSizer->Add( statusLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	statusText = new wxStaticText( mainPage, wxID_ANY, _("mismatch"), wxDefaultPosition, wxDefaultSize, 0 );
	statusText->Wrap( -1 );
	statusSizer->Add( statusText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	validateButton = new wxButton( mainPage, wxID_ANY, _("validate"), wxDefaultPosition, wxDefaultSize, 0 );
	statusSizer->Add( validateButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	updateButton = new wxButton( mainPage, wxID_ANY, _("update"), wxDefaultPosition, wxDefaultSize, 0 );
	statusSizer->Add( updateButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainSizer->Add( statusSizer, 1, wxEXPAND, 5 );
	
	mainPage->SetSizer( mainSizer );
	mainPage->Layout();
	mainSizer->Fit( mainPage );
	pageSizer->Add( mainPage, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	newRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterContextPageBase::onNewRadioButton ), NULL, this );
	openRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterContextPageBase::onOpenRadioButton ), NULL, this );
	binaryTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorFilterContextPageBase::onBinaryTestKillFocus ), NULL, this );
	binaryTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorFilterContextPageBase::onBinaryTextEnter ), NULL, this );
	modifyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorFilterContextPageBase::onModifyButton ), NULL, this );
	validateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorFilterContextPageBase::onValidateButton ), NULL, this );
	updateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorFilterContextPageBase::onUpdateButton ), NULL, this );
}

DlgRuleEditorFilterPermissionPageBase::DlgRuleEditorFilterPermissionPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxVERTICAL );
	
	mainPage = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	mainPage->SetScrollRate( 5, 5 );
	wxFlexGridSizer* mainSizer;
	mainSizer = new wxFlexGridSizer( 1, 4, 0, 0 );
	mainSizer->SetFlexibleDirection( wxBOTH );
	mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	permissionLabel = new wxStaticText( mainPage, wxID_ANY, _("Permission:"), wxDefaultPosition, wxDefaultSize, 0 );
	permissionLabel->Wrap( -1 );
	mainSizer->Add( permissionLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	readCheckBox = new wxCheckBox( mainPage, wxID_ANY, _("read"), wxDefaultPosition, wxDefaultSize, 0 );
	
	mainSizer->Add( readCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	writeCheckBox = new wxCheckBox( mainPage, wxID_ANY, _("write"), wxDefaultPosition, wxDefaultSize, 0 );
	
	mainSizer->Add( writeCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	executeCheckBox = new wxCheckBox( mainPage, wxID_ANY, _("execute"), wxDefaultPosition, wxDefaultSize, 0 );
	
	mainSizer->Add( executeCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainPage->SetSizer( mainSizer );
	mainPage->Layout();
	mainSizer->Fit( mainPage );
	pageSizer->Add( mainPage, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	readCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DlgRuleEditorFilterPermissionPageBase::onReadCheckBox ), NULL, this );
	writeCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DlgRuleEditorFilterPermissionPageBase::onWriteCheckBox ), NULL, this );
	executeCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DlgRuleEditorFilterPermissionPageBase::onExecuteCheckBox ), NULL, this );
}
