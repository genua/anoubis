/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include "AnGrid.h"
#include "AnPickFromFs.h"
#include "AnPolicyNotebook.h"
#include "DlgRuleEditorAppPage.h"
#include "DlgRuleEditorFilterSubjectPage.h"

#include "DlgRuleEditorBase.h"

///////////////////////////////////////////////////////////////////////////

DlgRuleEditorBase::DlgRuleEditorBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->Centre( wxBOTH );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* sz_user;
	sz_user = new wxBoxSizer( wxHORIZONTAL );
	
	rb_userMe = new wxRadioButton( this, wxID_ANY, _("Show my rules"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	sz_user->Add( rb_userMe, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	rb_userSelect = new wxRadioButton( this, wxID_ANY, _("Show admin rules of"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_user->Add( rb_userSelect, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	tx_userSelect = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	sz_user->Add( tx_userSelect, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	rb_userDefault = new wxRadioButton( this, wxID_ANY, _("Show default rules"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_user->Add( rb_userDefault, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainSizer->Add( sz_user, 0, wxEXPAND, 5 );
	
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
	appListCreateButton->SetToolTip( _("Creates a new rule") );
	
	appListHeadSizer->Add( appListCreateButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	appListHeadSizer->Add( 0, 0, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	appListColumnsButton = new wxButton( appPanel, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListColumnsButton->SetToolTip( _("customise columns of app-rules list") );
	
	appListHeadSizer->Add( appListColumnsButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	appMainSizer->Add( appListHeadSizer, 0, wxEXPAND, 5 );
	
	appGrid = new AnGrid( appPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	appGrid->CreateGrid( 5, 5 );
	appGrid->EnableEditing( false );
	appGrid->EnableGridLines( true );
	appGrid->EnableDragGridSize( false );
	appGrid->SetMargins( 0, 0 );
	
	// Columns
	appGrid->EnableDragColMove( false );
	appGrid->EnableDragColSize( true );
	appGrid->SetColLabelSize( 30 );
	appGrid->SetColLabelAlignment( wxALIGN_LEFT, wxALIGN_CENTRE );
	
	// Rows
	appGrid->EnableDragRowSize( true );
	appGrid->SetRowLabelSize( 0 );
	appGrid->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	appGrid->SetLabelFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	// Cell Defaults
	appGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	appGrid->SetToolTip( _("Select columns with right click on label.") );
	
	appMainSizer->Add( appGrid, 1, wxALL|wxEXPAND, 5 );
	
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
	appListUpButton->SetToolTip( _("Move up") );
	
	appListFoodSizer->Add( appListUpButton, 0, wxALL, 5 );
	
	appListDownButton = new wxButton( appPanel, wxID_ANY, _("down"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListDownButton->Enable( false );
	appListDownButton->SetToolTip( _("Move down") );
	
	appListFoodSizer->Add( appListDownButton, 0, wxALL, 5 );
	
	
	appListFoodSizer->Add( 50, 0, 0, wxEXPAND, 5 );
	
	appListDeleteButton = new wxButton( appPanel, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appListDeleteButton->Enable( false );
	appListDeleteButton->SetToolTip( _("Removes selected policy") );
	
	appListFoodSizer->Add( appListDeleteButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appMainSizer->Add( appListFoodSizer, 0, wxEXPAND|wxALIGN_RIGHT, 5 );
	
	appPolicyPanels = new AnPolicyNotebook( appPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
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
	
	wxArrayString filterListTypeChoiceChoices;
	filterListTypeChoice = new wxChoice( filterPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, filterListTypeChoiceChoices, 0 );
	filterListTypeChoice->Enable( false );
	
	filterListHeadSizer->Add( filterListTypeChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterListCreateButton = new wxButton( filterPanel, wxID_ANY, _("create"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListCreateButton->Enable( false );
	filterListCreateButton->SetToolTip( _("Creates a new rule") );
	
	filterListHeadSizer->Add( filterListCreateButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	filterListHeadSizer->Add( 0, 0, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	filterListColumnsButton = new wxButton( filterPanel, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListColumnsButton->SetToolTip( _("customise columns of filter-rules list") );
	
	filterListHeadSizer->Add( filterListColumnsButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	filterMainSizer->Add( filterListHeadSizer, 0, wxEXPAND, 5 );
	
	filterGrid = new AnGrid( filterPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	filterGrid->CreateGrid( 5, 5 );
	filterGrid->EnableEditing( false );
	filterGrid->EnableGridLines( true );
	filterGrid->EnableDragGridSize( false );
	filterGrid->SetMargins( 0, 0 );
	
	// Columns
	filterGrid->EnableDragColMove( false );
	filterGrid->EnableDragColSize( true );
	filterGrid->SetColLabelSize( 30 );
	filterGrid->SetColLabelAlignment( wxALIGN_LEFT, wxALIGN_CENTRE );
	
	// Rows
	filterGrid->EnableDragRowSize( true );
	filterGrid->SetRowLabelSize( 0 );
	filterGrid->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	filterGrid->SetLabelFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	// Cell Defaults
	filterGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	filterGrid->SetToolTip( _("Select columns with right click on label.") );
	
	filterMainSizer->Add( filterGrid, 1, wxALL|wxEXPAND, 5 );
	
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
	filterListUpButton->SetToolTip( _("Move up") );
	
	filterListFoodSizer->Add( filterListUpButton, 0, wxALL, 5 );
	
	filterListDownButton = new wxButton( filterPanel, wxID_ANY, _("down"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListDownButton->Enable( false );
	filterListDownButton->SetToolTip( _("Move down") );
	
	filterListFoodSizer->Add( filterListDownButton, 0, wxALL, 5 );
	
	
	filterListFoodSizer->Add( 50, 0, 0, wxEXPAND, 5 );
	
	filterListDeleteButton = new wxButton( filterPanel, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	filterListDeleteButton->Enable( false );
	filterListDeleteButton->SetToolTip( _("Removes selected policy") );
	
	filterListFoodSizer->Add( filterListDeleteButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	filterMainSizer->Add( filterListFoodSizer, 0, wxEXPAND|wxALIGN_RIGHT, 5 );
	
	filterPolicyPanels = new AnPolicyNotebook( filterPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	filterMainSizer->Add( filterPolicyPanels, 1, wxEXPAND | wxALL, 5 );
	
	filterPanel->SetSizer( filterMainSizer );
	filterPanel->Layout();
	filterMainSizer->Fit( filterPanel );
	splitterWindow->SplitVertically( appPanel, filterPanel, 482 );
	mainSizer->Add( splitterWindow, 1, wxEXPAND, 5 );
	
	wxBoxSizer* textFootSizer;
	textFootSizer = new wxBoxSizer( wxHORIZONTAL );
	
	footerRuleSetLabel = new wxStaticText( this, wxID_ANY, _("RuleSet origin:"), wxDefaultPosition, wxDefaultSize, 0 );
	footerRuleSetLabel->Wrap( -1 );
	textFootSizer->Add( footerRuleSetLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	footerRuleSetText = new wxStaticText( this, wxID_ANY, _("none"), wxDefaultPosition, wxDefaultSize, 0 );
	footerRuleSetText->Wrap( -1 );
	textFootSizer->Add( footerRuleSetText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	textFootSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	footerStatusLabel = new wxStaticText( this, wxID_ANY, _("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	footerStatusLabel->Wrap( -1 );
	textFootSizer->Add( footerStatusLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	footerStatusText = new wxStaticText( this, wxID_ANY, _("not modified"), wxDefaultPosition, wxDefaultSize, 0 );
	footerStatusText->Wrap( -1 );
	textFootSizer->Add( footerStatusText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainSizer->Add( textFootSizer, 0, wxEXPAND, 5 );
	
	wxBoxSizer* buttonFootSizer;
	buttonFootSizer = new wxBoxSizer( wxHORIZONTAL );
	
	footerImportButton = new wxButton( this, wxID_ANY, _("import..."), wxDefaultPosition, wxDefaultSize, 0 );
	footerImportButton->SetToolTip( _("import ruleset from file") );
	
	buttonFootSizer->Add( footerImportButton, 0, wxALL, 5 );
	
	footerReloadButton = new wxButton( this, wxID_ANY, _("reload from daemon"), wxDefaultPosition, wxDefaultSize, 0 );
	footerReloadButton->Enable( false );
	footerReloadButton->SetToolTip( _("reload ruleset from daemon") );
	
	buttonFootSizer->Add( footerReloadButton, 0, wxALL, 5 );
	
	
	buttonFootSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	footerExportButton = new wxButton( this, wxID_ANY, _("export..."), wxDefaultPosition, wxDefaultSize, 0 );
	footerExportButton->SetToolTip( _("export ruleset to file (which has to be choosen)") );
	
	buttonFootSizer->Add( footerExportButton, 0, wxALL, 5 );
	
	footerActivateButton = new wxButton( this, wxID_ANY, _("activate"), wxDefaultPosition, wxDefaultSize, 0 );
	footerActivateButton->Enable( false );
	footerActivateButton->SetToolTip( _("send ruleset to daemon") );
	
	buttonFootSizer->Add( footerActivateButton, 0, wxALL, 5 );
	
	mainSizer->Add( buttonFootSizer, 0, wxEXPAND, 5 );
	
	this->SetSizer( mainSizer );
	this->Layout();
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DlgRuleEditorBase::onClose ) );
	rb_userMe->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::onRbUserMe ), NULL, this );
	rb_userSelect->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::onRbUserSelect ), NULL, this );
	tx_userSelect->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorBase::onUserSelectKillFocus ), NULL, this );
	tx_userSelect->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorBase::onUserSelectTextEnter ), NULL, this );
	rb_userDefault->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::onRbUserDefault ), NULL, this );
	appListCreateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onAppListCreateButton ), NULL, this );
	appListColumnsButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onAppListColumnsButtonClick ), NULL, this );
	appGrid->Connect( wxEVT_GRID_SELECT_CELL, wxGridEventHandler( DlgRuleEditorBase::onAppGridCellSelect ), NULL, this );
	appListUpButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onAppListUpClick ), NULL, this );
	appListDownButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onAppListDownClick ), NULL, this );
	appListDeleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onAppListDeleteClick ), NULL, this );
	filterListCreateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFilterListCreateButton ), NULL, this );
	filterListColumnsButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFilterListColumnsButtonClick ), NULL, this );
	filterGrid->Connect( wxEVT_GRID_SELECT_CELL, wxGridEventHandler( DlgRuleEditorBase::onFilterGridCellSelect ), NULL, this );
	filterListUpButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFilterListUpClick ), NULL, this );
	filterListDownButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFilterListDownClick ), NULL, this );
	filterListDeleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFilterListDeleteClick ), NULL, this );
	footerImportButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFooterImportButton ), NULL, this );
	footerReloadButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFooterReloadButton ), NULL, this );
	footerExportButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFooterExportButton ), NULL, this );
	footerActivateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::onFooterActivateButton ), NULL, this );
}

DlgRuleEditorAppPageBase::DlgRuleEditorAppPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxVERTICAL );
	
	mainPage = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	mainPage->SetScrollRate( 5, 5 );
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	subjPage = new DlgRuleEditorFilterSubjectPage( mainPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	mainSizer->Add( subjPage, 1, wxALL|wxEXPAND, 5 );
	
	noSfsCheckbox = new wxCheckBox( mainPage, wxID_ANY, _("Disable SFS"), wxDefaultPosition, wxDefaultSize, 0 );
	
	noSfsCheckbox->SetToolTip( _("Activate it, if SFS should be disabled for this policy (and all of it's binaries).") );
	
	mainSizer->Add( noSfsCheckbox, 0, wxALL, 5 );
	
	wxBoxSizer* footerSizer;
	footerSizer = new wxBoxSizer( wxHORIZONTAL );
	
	addButton = new wxButton( mainPage, ID_APP_PAGE_ADD, _("add"), wxDefaultPosition, wxDefaultSize, 0 );
	addButton->SetToolTip( _("Appends the binary to the policy") );
	
	footerSizer->Add( addButton, 0, wxALL, 5 );
	
	deleteButton = new wxButton( mainPage, ID_APP_PAGE_DELETE, _("delete"), wxDefaultPosition, wxDefaultSize, 0 );
	deleteButton->SetToolTip( _("Removes the binary from the policy") );
	
	footerSizer->Add( deleteButton, 0, wxALL, 5 );
	
	
	footerSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	mainSizer->Add( footerSizer, 0, wxEXPAND, 5 );
	
	mainPage->SetSizer( mainSizer );
	mainPage->Layout();
	mainSizer->Fit( mainPage );
	pageSizer->Add( mainPage, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	noSfsCheckbox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DlgRuleEditorAppPageBase::onNoSfsClicked ), NULL, this );
	addButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorAppPageBase::onAddButton ), NULL, this );
	deleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorAppPageBase::onDeleteButton ), NULL, this );
}

DlgRuleEditorFilterActionPageBase::DlgRuleEditorFilterActionPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxVERTICAL );
	
	mainPage = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	mainPage->SetScrollRate( 5, 5 );
	wxBoxSizer* scrollSizer;
	scrollSizer = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* mainSizer;
	mainSizer = new wxFlexGridSizer( 3, 4, 0, 0 );
	mainSizer->SetFlexibleDirection( wxBOTH );
	mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	actionLabel = new wxStaticText( mainPage, wxID_ANY, _("Action:"), wxDefaultPosition, wxDefaultSize, 0 );
	actionLabel->Wrap( -1 );
	mainSizer->Add( actionLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	allowRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("allow"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	allowRadioButton->SetToolTip( _("Allow connection") );
	
	mainSizer->Add( allowRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	denyRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("deny"), wxDefaultPosition, wxDefaultSize, 0 );
	denyRadioButton->SetToolTip( _("Deny connection") );
	
	mainSizer->Add( denyRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	askRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("ask"), wxDefaultPosition, wxDefaultSize, 0 );
	askRadioButton->SetToolTip( _("Ask user") );
	
	mainSizer->Add( askRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	logLabel = new wxStaticText( mainPage, wxID_ANY, _("Log:"), wxDefaultPosition, wxDefaultSize, 0 );
	logLabel->Wrap( -1 );
	mainSizer->Add( logLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	noneRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("none"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	noneRadioButton->SetToolTip( _("Disable logging") );
	
	mainSizer->Add( noneRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	normalRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("normal"), wxDefaultPosition, wxDefaultSize, 0 );
	normalRadioButton->SetToolTip( _("Enable logging") );
	
	mainSizer->Add( normalRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alertRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("alert"), wxDefaultPosition, wxDefaultSize, 0 );
	alertRadioButton->SetToolTip( _("Enable higher priority logging") );
	
	mainSizer->Add( alertRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	scrollSizer->Add( mainSizer, 0, wxEXPAND, 5 );
	
	defaultPathPicker = new AnPickFromFs( mainPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxT("Default Path:") );
	scrollSizer->Add( defaultPathPicker, 0, wxALL|wxEXPAND, 5 );
	
	mainPage->SetSizer( scrollSizer );
	mainPage->Layout();
	scrollSizer->Fit( mainPage );
	pageSizer->Add( mainPage, 1, wxALL|wxEXPAND, 5 );
	
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
	inRadioButton->SetToolTip( _("Filter incoming connection-attempts") );
	inRadioButton->SetMinSize( wxSize( 80,-1 ) );
	
	mainSizer->Add( inRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	outRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("connect"), wxDefaultPosition, wxDefaultSize, 0 );
	outRadioButton->SetToolTip( _("Filter outgoing connection-attempts") );
	
	mainSizer->Add( outRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bothRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("both"), wxDefaultPosition, wxDefaultSize, 0 );
	bothRadioButton->SetToolTip( _("Allow any direction") );
	
	mainSizer->Add( bothRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	protocolLabel = new wxStaticText( mainPage, wxID_ANY, _("Protocol:"), wxDefaultPosition, wxDefaultSize, 0 );
	protocolLabel->Wrap( -1 );
	mainSizer->Add( protocolLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	tcpRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("tcp"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	tcpRadioButton->SetToolTip( _("Filter TCP streams") );
	
	mainSizer->Add( tcpRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	udpRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("udp"), wxDefaultPosition, wxDefaultSize, 0 );
	udpRadioButton->SetToolTip( _("Filter UDP datagrams") );
	
	mainSizer->Add( udpRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sctpRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("sctp"), wxDefaultPosition, wxDefaultSize, 0 );
	sctpRadioButton->SetToolTip( _("Filter SCTP datagrams") );
	
	mainSizer->Add( sctpRadioButton, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
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
	tcpRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterNetworkPageBase::onTcpRadioButton ), NULL, this );
	udpRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterNetworkPageBase::onUdpRadioButton ), NULL, this );
	sctpRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterNetworkPageBase::onSctpRadioButton ), NULL, this );
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
	rawRadioButton->SetToolTip( _("Allow access to raw sockets") );
	
	mainSizer->Add( rawRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	otherRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("other"), wxDefaultPosition, wxDefaultSize, 0 );
	otherRadioButton->SetToolTip( _("Allow access to other sockets") );
	
	mainSizer->Add( otherRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	allRadioButton = new wxRadioButton( mainPage, wxID_ANY, _("all"), wxDefaultPosition, wxDefaultSize, 0 );
	allRadioButton->SetToolTip( _("No capability limits") );
	
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
	
	scrollPanel = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	scrollPanel->SetScrollRate( 5, 5 );
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	pathPicker = new AnPickFromFs( scrollPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	mainSizer->Add( pathPicker, 0, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* subjectSizer;
	subjectSizer = new wxFlexGridSizer( 7, 3, 0, 0 );
	subjectSizer->AddGrowableCol( 1 );
	subjectSizer->SetFlexibleDirection( wxBOTH );
	subjectSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	subjectLabel = new wxStaticText( scrollPanel, wxID_ANY, _("Subject:"), wxDefaultPosition, wxDefaultSize, 0 );
	subjectLabel->Wrap( -1 );
	subjectSizer->Add( subjectLabel, 0, wxALL, 5 );
	
	anyRadioButton = new wxRadioButton( scrollPanel, wxID_ANY, _("any"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	anyRadioButton->Hide();
	anyRadioButton->SetToolTip( _("Not specified") );
	
	subjectSizer->Add( anyRadioButton, 0, wxALL, 5 );
	
	
	subjectSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	subjectSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	selfRadioButton = new wxRadioButton( scrollPanel, wxID_ANY, _("self"), wxDefaultPosition, wxDefaultSize, 0 );
	selfRadioButton->SetToolTip( _("Trust own checksum") );
	
	subjectSizer->Add( selfRadioButton, 0, wxALL, 5 );
	
	
	subjectSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	subjectSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	selfSignedRadioButton = new wxRadioButton( scrollPanel, wxID_ANY, _("self-signed"), wxDefaultPosition, wxDefaultSize, 0 );
	selfSignedRadioButton->SetToolTip( _("Trust own signature") );
	
	subjectSizer->Add( selfSignedRadioButton, 0, wxALL, 5 );
	
	
	subjectSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	subjectSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* uidSizer;
	uidSizer = new wxBoxSizer( wxHORIZONTAL );
	
	uidRadioButton = new wxRadioButton( scrollPanel, wxID_ANY, _("uid"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	uidRadioButton->SetToolTip( _("Trust the specified user") );
	
	uidSizer->Add( uidRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	uidTextCtrl = new wxTextCtrl( scrollPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	uidSizer->Add( uidTextCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	subjectSizer->Add( uidSizer, 1, wxEXPAND, 5 );
	
	
	subjectSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	subjectSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* keySizer;
	keySizer = new wxBoxSizer( wxHORIZONTAL );
	
	keyRadioButton = new wxRadioButton( scrollPanel, wxID_ANY, _("key"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	keyRadioButton->SetToolTip( _("Trust certificate with specified keyid") );
	
	keySizer->Add( keyRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	keyTextCtrl = new wxTextCtrl( scrollPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	keySizer->Add( keyTextCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	subjectSizer->Add( keySizer, 1, wxEXPAND, 5 );
	
	
	subjectSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	mainSizer->Add( subjectSizer, 0, wxEXPAND, 5 );
	
	scrollPanel->SetSizer( mainSizer );
	scrollPanel->Layout();
	mainSizer->Fit( scrollPanel );
	pageSizer->Add( scrollPanel, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	pageSizer->Fit( this );
	
	// Connect Events
	anyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onAnyRadioButton ), NULL, this );
	selfRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onSelfRadioButton ), NULL, this );
	selfSignedRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onSelfSignedRadioButton ), NULL, this );
	uidRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onUidRadioButton ), NULL, this );
	uidTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorFilterSubjectPageBase::onUidTextKillFocus ), NULL, this );
	uidTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onUidTextEnter ), NULL, this );
	keyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onKeyRadioButton ), NULL, this );
	keyTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DlgRuleEditorFilterSubjectPageBase::onKeyTextKillFocus ), NULL, this );
	keyTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorFilterSubjectPageBase::onKeyTextEnter ), NULL, this );
}

DlgRuleEditorFilterSfsPageBase::DlgRuleEditorFilterSfsPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxVERTICAL );
	
	mainPage = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	mainPage->SetScrollRate( 5, 5 );
	wxFlexGridSizer* mainSizer;
	mainSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	mainSizer->AddGrowableRow( 2 );
	mainSizer->AddGrowableRow( 5 );
	mainSizer->AddGrowableRow( 8 );
	mainSizer->SetFlexibleDirection( wxBOTH );
	mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	validLabel = new wxStaticText( mainPage, wxID_ANY, _("Valid:"), wxDefaultPosition, wxDefaultSize, 0 );
	validLabel->Wrap( -1 );
	mainSizer->Add( validLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString validActionRadioBoxChoices[] = { _("allow"), _("deny"), _("ask"), _("continue") };
	int validActionRadioBoxNChoices = sizeof( validActionRadioBoxChoices ) / sizeof( wxString );
	validActionRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Action"), wxDefaultPosition, wxDefaultSize, validActionRadioBoxNChoices, validActionRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	validActionRadioBox->SetSelection( 1 );
	validActionRadioBox->SetToolTip( _("Action if checksum matches") );
	
	mainSizer->Add( validActionRadioBox, 0, wxALL, 5 );
	
	wxString validLogRadioBoxChoices[] = { _("none"), _("normal"), _("alert") };
	int validLogRadioBoxNChoices = sizeof( validLogRadioBoxChoices ) / sizeof( wxString );
	validLogRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, validLogRadioBoxNChoices, validLogRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	validLogRadioBox->SetSelection( 2 );
	validLogRadioBox->SetToolTip( _("Logging configuration if checksum matches") );
	
	mainSizer->Add( validLogRadioBox, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	invalidLabel = new wxStaticText( mainPage, wxID_ANY, _("Invalid:"), wxDefaultPosition, wxDefaultSize, 0 );
	invalidLabel->Wrap( -1 );
	mainSizer->Add( invalidLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString invalidActionRadioBoxChoices[] = { _("allow"), _("deny"), _("ask"), _("continue") };
	int invalidActionRadioBoxNChoices = sizeof( invalidActionRadioBoxChoices ) / sizeof( wxString );
	invalidActionRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Action"), wxDefaultPosition, wxDefaultSize, invalidActionRadioBoxNChoices, invalidActionRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	invalidActionRadioBox->SetSelection( 1 );
	invalidActionRadioBox->SetToolTip( _("Action if checksum doesn't match") );
	
	mainSizer->Add( invalidActionRadioBox, 0, wxALL, 5 );
	
	wxString invalidLogRadioBoxChoices[] = { _("none"), _("normal"), _("alert") };
	int invalidLogRadioBoxNChoices = sizeof( invalidLogRadioBoxChoices ) / sizeof( wxString );
	invalidLogRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, invalidLogRadioBoxNChoices, invalidLogRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	invalidLogRadioBox->SetSelection( 2 );
	invalidLogRadioBox->SetToolTip( _("Logging configuration if checksum doesn't match") );
	
	mainSizer->Add( invalidLogRadioBox, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	unknownLabel = new wxStaticText( mainPage, wxID_ANY, _("Unknown:"), wxDefaultPosition, wxDefaultSize, 0 );
	unknownLabel->Wrap( -1 );
	mainSizer->Add( unknownLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxString unknownActionRadioBoxChoices[] = { _("allow"), _("deny"), _("ask"), _("continue") };
	int unknownActionRadioBoxNChoices = sizeof( unknownActionRadioBoxChoices ) / sizeof( wxString );
	unknownActionRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Action"), wxDefaultPosition, wxDefaultSize, unknownActionRadioBoxNChoices, unknownActionRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	unknownActionRadioBox->SetSelection( 0 );
	unknownActionRadioBox->SetToolTip( _("Action if checksum is missing") );
	
	mainSizer->Add( unknownActionRadioBox, 0, wxALL, 5 );
	
	wxString unknownLogRadioBoxChoices[] = { _("none"), _("normal"), _("alert") };
	int unknownLogRadioBoxNChoices = sizeof( unknownLogRadioBoxChoices ) / sizeof( wxString );
	unknownLogRadioBox = new wxRadioBox( mainPage, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, unknownLogRadioBoxNChoices, unknownLogRadioBoxChoices, 1, wxRA_SPECIFY_ROWS|wxTAB_TRAVERSAL );
	unknownLogRadioBox->SetSelection( 2 );
	unknownLogRadioBox->SetToolTip( _("Logging configuration if checksum is missing") );
	
	mainSizer->Add( unknownLogRadioBox, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
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
	
	scrollPanel = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	scrollPanel->SetScrollRate( 5, 5 );
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* typeSizer;
	typeSizer = new wxBoxSizer( wxHORIZONTAL );
	
	typeLabel = new wxStaticText( scrollPanel, wxID_ANY, _("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	typeLabel->Wrap( -1 );
	typeSizer->Add( typeLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	newRadioButton = new wxRadioButton( scrollPanel, wxID_ANY, _("new"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	newRadioButton->SetToolTip( _("Switch to context") );
	
	typeSizer->Add( newRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	openRadioButton = new wxRadioButton( scrollPanel, wxID_ANY, _("open"), wxDefaultPosition, wxDefaultSize, 0 );
	openRadioButton->SetToolTip( _("Switch to context on open") );
	
	typeSizer->Add( openRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainSizer->Add( typeSizer, 0, wxEXPAND, 5 );
	
	appPage = new DlgRuleEditorAppPage( scrollPanel, wxID_ANY, wxDefaultPosition, wxSize( 400,200 ), wxTAB_TRAVERSAL );
	mainSizer->Add( appPage, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5 );
	
	scrollPanel->SetSizer( mainSizer );
	scrollPanel->Layout();
	mainSizer->Fit( scrollPanel );
	pageSizer->Add( scrollPanel, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	pageSizer->Fit( this );
	
	// Connect Events
	newRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterContextPageBase::onNewRadioButton ), NULL, this );
	openRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorFilterContextPageBase::onOpenRadioButton ), NULL, this );
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
	
	readCheckBox->SetToolTip( _("Read permission") );
	
	mainSizer->Add( readCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	writeCheckBox = new wxCheckBox( mainPage, wxID_ANY, _("write"), wxDefaultPosition, wxDefaultSize, 0 );
	
	writeCheckBox->SetToolTip( _("Write permission") );
	
	mainSizer->Add( writeCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	executeCheckBox = new wxCheckBox( mainPage, wxID_ANY, _("execute"), wxDefaultPosition, wxDefaultSize, 0 );
	
	executeCheckBox->SetToolTip( _("Execute permission") );
	
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
