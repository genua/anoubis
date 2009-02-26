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

#include "AnDetails.h"

#include "RuleWizardPanelsBase.h"

///////////////////////////////////////////////////////////////////////////

RuleWizardProgramPageBase::RuleWizardProgramPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 2, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("Choose Program:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	wxBoxSizer* programSizer;
	programSizer = new wxBoxSizer( wxHORIZONTAL );
	
	programLabel = new wxStaticText( this, wxID_ANY, _("Program:"), wxDefaultPosition, wxDefaultSize, 0 );
	programLabel->Wrap( -1 );
	programSizer->Add( programLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	programTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	programSizer->Add( programTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	pickButton = new wxButton( this, wxID_ANY, _("Pick..."), wxDefaultPosition, wxDefaultSize, 0 );
	programSizer->Add( pickButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainSizer->Add( programSizer, 1, wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 8, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	programTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( RuleWizardProgramPageBase::onProgramTextKillFocus ), NULL, this );
	programTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( RuleWizardProgramPageBase::onProgramTextEnter ), NULL, this );
	pickButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardProgramPageBase::onPickButton ), NULL, this );
}

RuleWizardContextPageBase::RuleWizardContextPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 2, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("Context settings:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	wxBoxSizer* splitSizer;
	splitSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* inputSizer;
	inputSizer = new wxBoxSizer( wxVERTICAL );
	
	questionLabel = new wxStaticText( this, wxID_ANY, _("Programs been started by this application have the same permissions / restrictions as the main program."), wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( 300 );
	inputSizer->Add( questionLabel, 0, wxALL, 5 );
	
	
	inputSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	yesRadioButton = new wxRadioButton( this, wxID_ANY, _("Yes (same permissions / restrictions)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	inputSizer->Add( yesRadioButton, 0, wxALL, 5 );
	
	wxBoxSizer* yesExceptionSizer;
	yesExceptionSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	yesExceptionSizer->Add( 30, 0, 0, wxEXPAND, 5 );
	
	yesExceptionsCheckBox = new wxCheckBox( this, wxID_ANY, _("allow exceptions (-> next page)"), wxDefaultPosition, wxDefaultSize, 0 );
	
	yesExceptionSizer->Add( yesExceptionsCheckBox, 0, wxALL, 5 );
	
	inputSizer->Add( yesExceptionSizer, 0, wxEXPAND, 5 );
	
	noRadioButton = new wxRadioButton( this, wxID_ANY, _("No (each executed program has its own policies)"), wxDefaultPosition, wxDefaultSize, 0 );
	inputSizer->Add( noRadioButton, 0, wxALL, 5 );
	
	wxBoxSizer* noExceptionSizer;
	noExceptionSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	noExceptionSizer->Add( 30, 0, 0, wxEXPAND, 5 );
	
	noExceptionsCheckBox = new wxCheckBox( this, wxID_ANY, _("allow exceptions (-> next page)"), wxDefaultPosition, wxDefaultSize, 0 );
	
	noExceptionsCheckBox->Enable( false );
	
	noExceptionSizer->Add( noExceptionsCheckBox, 0, wxALL, 5 );
	
	inputSizer->Add( noExceptionSizer, 0, wxEXPAND, 5 );
	
	splitSizer->Add( inputSizer, 0, wxEXPAND, 5 );
	
	contextHelpPage = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	contextHelpPage->SetBackgroundColour( wxColour( 179, 177, 174 ) );
	
	wxBoxSizer* contextHelpSizer;
	contextHelpSizer = new wxBoxSizer( wxVERTICAL );
	
	contextHelpLabel = new wxStaticText( contextHelpPage, wxID_ANY, _("This is the mandatory help text for context..."), wxDefaultPosition, wxDefaultSize, 0 );
	contextHelpLabel->Wrap( 300 );
	contextHelpSizer->Add( contextHelpLabel, 0, wxALL, 5 );
	
	contextHelpPage->SetSizer( contextHelpSizer );
	contextHelpPage->Layout();
	contextHelpSizer->Fit( contextHelpPage );
	splitSizer->Add( contextHelpPage, 0, wxALL, 5 );
	
	mainSizer->Add( splitSizer, 1, wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 8, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	yesRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardContextPageBase::onYesRadioButton ), NULL, this );
	yesExceptionsCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RuleWizardContextPageBase::onYesExceptionsCheckBox ), NULL, this );
	noRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardContextPageBase::onNoRadioButton ), NULL, this );
	noExceptionsCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RuleWizardContextPageBase::onNoExceptionsCheckBox ), NULL, this );
}

RuleWizardContextExceptionPageBase::RuleWizardContextExceptionPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 2, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("Context settings:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	questionLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( -1 );
	mainSizer->Add( questionLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	exceptionListLabel = new wxStaticText( this, wxID_ANY, _("Exceptions:"), wxDefaultPosition, wxDefaultSize, 0 );
	exceptionListLabel->Wrap( -1 );
	mainSizer->Add( exceptionListLabel, 0, wxALL, 5 );
	
	exceptionListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_ALWAYS_SB|wxLB_SINGLE ); 
	mainSizer->Add( exceptionListBox, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* buttonSizer;
	buttonSizer = new wxBoxSizer( wxHORIZONTAL );
	
	addButton = new wxButton( this, wxID_ANY, _("add..."), wxDefaultPosition, wxDefaultSize, 0 );
	buttonSizer->Add( addButton, 0, wxALL, 5 );
	
	
	buttonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	deleteButton = new wxButton( this, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, 0 );
	buttonSizer->Add( deleteButton, 0, wxALL, 5 );
	
	mainSizer->Add( buttonSizer, 0, wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 8, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	addButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardContextExceptionPageBase::onAddButton ), NULL, this );
	deleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardContextExceptionPageBase::onDeleteButton ), NULL, this );
}

RuleWizardAlfKeepPolicyPageBase::RuleWizardAlfKeepPolicyPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 2, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("Application Level Firewall settings:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	wxBoxSizer* splitSizer;
	splitSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* questionSizer;
	questionSizer = new wxBoxSizer( wxVERTICAL );
	
	questionLabel = new wxStaticText( this, wxID_ANY, _("Keep existing policies?"), wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( -1 );
	questionSizer->Add( questionLabel, 0, wxALL, 5 );
	
	yesRadioButton = new wxRadioButton( this, wxID_ANY, _("Yes"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	questionSizer->Add( yesRadioButton, 0, wxALL, 5 );
	
	noRadioButton = new wxRadioButton( this, wxID_ANY, _("No"), wxDefaultPosition, wxDefaultSize, 0 );
	questionSizer->Add( noRadioButton, 0, wxALL, 5 );
	
	splitSizer->Add( questionSizer, 1, wxEXPAND, 5 );
	
	alertIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	splitSizer->Add( alertIcon, 0, wxALL|wxEXPAND, 5 );
	
	alertLabel = new wxStaticText( this, wxID_ANY, _("For this application\nalf policies already exists."), wxDefaultPosition, wxDefaultSize, 0 );
	alertLabel->Wrap( -1 );
	splitSizer->Add( alertLabel, 0, wxALL, 5 );
	
	mainSizer->Add( splitSizer, 0, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	policyLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	policyLabel->Wrap( -1 );
	mainSizer->Add( policyLabel, 0, wxALL, 5 );
	
	policyTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	mainSizer->Add( policyTextCtrl, 1, wxALL|wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 8, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	yesRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardAlfKeepPolicyPageBase::onYesRadioButton ), NULL, this );
	noRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardAlfKeepPolicyPageBase::onNoRadioButton ), NULL, this );
}

RuleWizardAlfClientPageBase::RuleWizardAlfClientPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 2, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("Application Level Firewall settings:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	wxBoxSizer* inputSizer;
	inputSizer = new wxBoxSizer( wxVERTICAL );
	
	questionLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( 300 );
	inputSizer->Add( questionLabel, 0, wxALL, 5 );
	
	
	inputSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	yesRadioButton = new wxRadioButton( this, wxID_ANY, _("Yes (without restrictions)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	inputSizer->Add( yesRadioButton, 0, wxALL, 5 );
	
	defaultRadioButton = new wxRadioButton( this, wxID_ANY, _("restricted (default)"), wxDefaultPosition, wxDefaultSize, 0 );
	inputSizer->Add( defaultRadioButton, 0, wxALL, 5 );
	
	wxBoxSizer* defaultSizer;
	defaultSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	defaultSizer->Add( 30, 0, 0, wxEXPAND, 5 );
	
	defaultLabel = new wxStaticText( this, wxID_ANY, _("Default services are allowed, any other network\naccess is asked on demand."), wxPoint( -1,-1 ), wxDefaultSize, 0 );
	defaultLabel->Wrap( -1 );
	defaultSizer->Add( defaultLabel, 0, wxALL, 5 );
	
	inputSizer->Add( defaultSizer, 0, wxEXPAND, 5 );
	
	restrictedRadioButton = new wxRadioButton( this, wxID_ANY, _("restricted"), wxDefaultPosition, wxDefaultSize, 0 );
	inputSizer->Add( restrictedRadioButton, 0, wxALL, 5 );
	
	wxBoxSizer* restrictedSizer;
	restrictedSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	restrictedSizer->Add( 30, 0, 0, wxEXPAND, 5 );
	
	restrictedLabel = new wxStaticText( this, wxID_ANY, _("Settings may be configured on the next page."), wxPoint( -1,-1 ), wxDefaultSize, 0 );
	restrictedLabel->Wrap( -1 );
	restrictedSizer->Add( restrictedLabel, 0, wxALL, 5 );
	
	inputSizer->Add( restrictedSizer, 0, wxEXPAND, 5 );
	
	noRadioButton = new wxRadioButton( this, wxID_ANY, _("No (deny access of network resources)"), wxDefaultPosition, wxDefaultSize, 0 );
	inputSizer->Add( noRadioButton, 0, wxALL, 5 );
	
	mainSizer->Add( inputSizer, 0, wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 8, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	yesRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardAlfClientPageBase::onYesRadioButton ), NULL, this );
	defaultRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardAlfClientPageBase::onDefaultRadioButton ), NULL, this );
	restrictedRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardAlfClientPageBase::onRestrictedRadioButton ), NULL, this );
	noRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardAlfClientPageBase::onNoRadioButton ), NULL, this );
}

RuleWizardAlfClientPortsPageBase::RuleWizardAlfClientPortsPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 2, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("Application Level Firewall settings:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	questionLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( -1 );
	mainSizer->Add( questionLabel, 0, wxALL, 5 );
	
	portListLabel = new wxStaticText( this, wxID_ANY, _("List of allowed services:"), wxDefaultPosition, wxDefaultSize, 0 );
	portListLabel->Wrap( -1 );
	mainSizer->Add( portListLabel, 0, wxALL, 5 );
	
	portListCtrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL );
	mainSizer->Add( portListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* buttonSizer;
	buttonSizer = new wxBoxSizer( wxHORIZONTAL );
	
	addButton = new wxButton( this, wxID_ANY, _(" add..."), wxDefaultPosition, wxDefaultSize, 0 );
	buttonSizer->Add( addButton, 0, wxALL, 5 );
	
	
	buttonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	defaultsButton = new wxButton( this, wxID_ANY, _("add default services"), wxDefaultPosition, wxDefaultSize, 0 );
	buttonSizer->Add( defaultsButton, 0, wxALL, 5 );
	
	
	buttonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	deleteButton = new wxButton( this, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, 0 );
	buttonSizer->Add( deleteButton, 0, wxALL, 5 );
	
	mainSizer->Add( buttonSizer, 0, wxEXPAND, 5 );
	
	detailsPanel = new AnDetails( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxT("Details...") );
	wxBoxSizer* detailsSizer;
	detailsSizer = new wxBoxSizer( wxVERTICAL );
	
	askCheckBox = new wxCheckBox( detailsPanel, wxID_ANY, _("ask on any other network access"), wxDefaultPosition, wxDefaultSize, 0 );
	askCheckBox->SetValue(true);
	
	detailsSizer->Add( askCheckBox, 0, wxALL, 5 );
	
	rawCheckBox = new wxCheckBox( detailsPanel, wxID_ANY, _("allow raw network access"), wxDefaultPosition, wxDefaultSize, 0 );
	
	detailsSizer->Add( rawCheckBox, 0, wxALL, 5 );
	
	detailsPanel->SetSizer( detailsSizer );
	detailsPanel->Layout();
	detailsSizer->Fit( detailsPanel );
	mainSizer->Add( detailsPanel, 0, wxEXPAND | wxALL, 5 );
	
	pageSizer->Add( mainSizer, 8, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	addButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardAlfClientPortsPageBase::onAddButton ), NULL, this );
	defaultsButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardAlfClientPortsPageBase::onDefaultsButton ), NULL, this );
	deleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardAlfClientPortsPageBase::onDeleteButton ), NULL, this );
	askCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RuleWizardAlfClientPortsPageBase::onAskCheckBox ), NULL, this );
	rawCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RuleWizardAlfClientPortsPageBase::onRawCheckBox ), NULL, this );
}

RuleWizardAlfDlgAddServiceBase::RuleWizardAlfDlgAddServiceBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* headLineSizer;
	headLineSizer = new wxBoxSizer( wxHORIZONTAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("Available predefined services:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineSizer->Add( headLineLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	headLineSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	searchTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	headLineSizer->Add( searchTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	mainSizer->Add( headLineSizer, 0, wxEXPAND, 5 );
	
	serviceListCtrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VRULES );
	mainSizer->Add( serviceListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* addSizer;
	addSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	addSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	addButton = new wxButton( this, wxID_ANY, _("add selected service"), wxDefaultPosition, wxDefaultSize, 0 );
	addSizer->Add( addButton, 0, wxALL, 5 );
	
	mainSizer->Add( addSizer, 0, wxEXPAND, 5 );
	
	wxBoxSizer* borderSizer;
	borderSizer = new wxBoxSizer( wxHORIZONTAL );
	
	borderLabel = new wxStaticText( this, wxID_ANY, _("Configure custom service"), wxDefaultPosition, wxDefaultSize, 0 );
	borderLabel->Wrap( -1 );
	borderSizer->Add( borderLabel, 0, wxALL, 5 );
	
	borderLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	borderSizer->Add( borderLine, 1, wxEXPAND | wxALL, 5 );
	
	mainSizer->Add( borderSizer, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* customSizer;
	customSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
	customSizer->AddGrowableCol( 2 );
	customSizer->SetFlexibleDirection( wxBOTH );
	customSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	protocolLabel = new wxStaticText( this, wxID_ANY, _("Protocol:"), wxDefaultPosition, wxDefaultSize, 0 );
	protocolLabel->Wrap( -1 );
	customSizer->Add( protocolLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	tcpRadioButton = new wxRadioButton( this, wxID_ANY, _("tcp"), wxDefaultPosition, wxDefaultSize, 0 );
	customSizer->Add( tcpRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	portLabel = new wxStaticText( this, wxID_ANY, _("Portnumber or -range"), wxDefaultPosition, wxDefaultSize, 0 );
	portLabel->Wrap( -1 );
	customSizer->Add( portLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	customSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	customSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	udpRadioButton = new wxRadioButton( this, wxID_ANY, _("udb"), wxDefaultPosition, wxDefaultSize, 0 );
	customSizer->Add( udpRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	portTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	customSizer->Add( portTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	customAddButton = new wxButton( this, wxID_ANY, _("add custom service"), wxDefaultPosition, wxDefaultSize, 0 );
	customSizer->Add( customAddButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainSizer->Add( customSizer, 0, wxEXPAND, 5 );
	
	this->SetSizer( mainSizer );
	this->Layout();
	
	// Connect Events
	addButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardAlfDlgAddServiceBase::onAddButton ), NULL, this );
}

RuleWizardFinalPageBase::RuleWizardFinalPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 2, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("End."), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	finalLabel = new wxStaticText( this, wxID_ANY, _("This is the final page. If you click on 'final' the policies will been created."), wxDefaultPosition, wxDefaultSize, 0 );
	finalLabel->Wrap( 300 );
	mainSizer->Add( finalLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 8, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
}
