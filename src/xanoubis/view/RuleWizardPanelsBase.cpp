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
#include "AnPickFromFs.h"

#include "RuleWizardPanelsBase.h"

///////////////////////////////////////////////////////////////////////////

RuleWizardOverwritePolicyPageBase::RuleWizardOverwritePolicyPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 3, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("... settings:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	helpPage = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	helpPage->SetBackgroundColour( wxColour( 179, 177, 174 ) );
	
	wxBoxSizer* helpSizer;
	helpSizer = new wxBoxSizer( wxVERTICAL );
	
	helpLabel = new wxStaticText( helpPage, wxID_ANY, _("This is the mandatory help text ..."), wxDefaultPosition, wxDefaultSize, 0 );
	helpLabel->Wrap( 400 );
	helpSizer->Add( helpLabel, 1, wxALL|wxEXPAND, 5 );
	
	helpPage->SetSizer( helpSizer );
	helpPage->Layout();
	helpSizer->Fit( helpPage );
	mainSizer->Add( helpPage, 5, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* splitSizer;
	splitSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* questionSizer;
	questionSizer = new wxBoxSizer( wxVERTICAL );
	
	questionLabel = new wxStaticText( this, wxID_ANY, _("Overwrite existing policies?"), wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( -1 );
	questionSizer->Add( questionLabel, 0, wxALL, 5 );
	
	yesRadioButton = new wxRadioButton( this, wxID_ANY, _("Yes (create new policies)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	questionSizer->Add( yesRadioButton, 0, wxALL, 5 );
	
	noRadioButton = new wxRadioButton( this, wxID_ANY, _("No (keep existing policies)"), wxDefaultPosition, wxDefaultSize, 0 );
	questionSizer->Add( noRadioButton, 0, wxALL, 5 );
	
	splitSizer->Add( questionSizer, 45, wxEXPAND, 5 );
	
	alertIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	splitSizer->Add( alertIcon, 10, wxALL|wxEXPAND, 5 );
	
	alertLabel = new wxStaticText( this, wxID_ANY, _("For this application\n... policies\nalready exists."), wxDefaultPosition, wxDefaultSize, 0 );
	alertLabel->Wrap( -1 );
	splitSizer->Add( alertLabel, 44, wxALL, 5 );
	
	mainSizer->Add( splitSizer, 0, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 30, 1, wxEXPAND, 5 );
	
	policyLabel = new wxStaticText( this, wxID_ANY, _("existing policy for \"...\":"), wxDefaultPosition, wxDefaultSize, 0 );
	policyLabel->Wrap( -1 );
	mainSizer->Add( policyLabel, 0, wxALL, 5 );
	
	policyTextCtrl = new wxTextCtrl( this, wxID_ANY, _("/* apn policy */"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	mainSizer->Add( policyTextCtrl, 6, wxALL|wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 7, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	yesRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardOverwritePolicyPageBase::onYesRadioButton ), NULL, this );
	noRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardOverwritePolicyPageBase::onNoRadioButton ), NULL, this );
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
	
	searchTextCtrl = new wxTextCtrl( this, wxID_ANY, _("search"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	headLineSizer->Add( searchTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	searchIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	searchIcon->Hide();
	
	headLineSizer->Add( searchIcon, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	mainSizer->Add( headLineSizer, 0, wxEXPAND, 5 );
	
	serviceListCtrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VRULES );
	mainSizer->Add( serviceListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* addSizer;
	addSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	addSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	addButton = new wxButton( this, wxID_ANY, _("add selected service"), wxDefaultPosition, wxDefaultSize, 0 );
	addButton->Enable( false );
	addButton->SetToolTip( _("close this dialog and return\nthe selected service") );
	
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
	customSizer = new wxFlexGridSizer( 2, 3, 0, 0 );
	customSizer->AddGrowableCol( 1 );
	customSizer->SetFlexibleDirection( wxBOTH );
	customSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	protocolLabel = new wxStaticText( this, wxID_ANY, _("Protocoltype:"), wxDefaultPosition, wxDefaultSize, 0 );
	protocolLabel->Wrap( -1 );
	customSizer->Add( protocolLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* protocolSizer;
	protocolSizer = new wxBoxSizer( wxHORIZONTAL );
	
	tcpRadioButton = new wxRadioButton( this, wxID_ANY, _("tcp"), wxDefaultPosition, wxDefaultSize, 0 );
	protocolSizer->Add( tcpRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	udpRadioButton = new wxRadioButton( this, wxID_ANY, _("udp"), wxDefaultPosition, wxDefaultSize, 0 );
	protocolSizer->Add( udpRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	customSizer->Add( protocolSizer, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	customSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	portLabel = new wxStaticText( this, wxID_ANY, _("Portnumber(s) or -range:"), wxDefaultPosition, wxDefaultSize, 0 );
	portLabel->Wrap( -1 );
	customSizer->Add( portLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	portTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	customSizer->Add( portTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	customAddButton = new wxButton( this, wxID_ANY, _("add custom service"), wxDefaultPosition, wxDefaultSize, 0 );
	customAddButton->SetToolTip( _("close this dialog and return\nthe given custom service") );
	
	customSizer->Add( customAddButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	customSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	portHelpLabel = new wxStaticText( this, wxID_ANY, _("(e.g. 123 or 123, 234 or 123 - 234)"), wxDefaultPosition, wxDefaultSize, 0 );
	portHelpLabel->Wrap( -1 );
	portHelpLabel->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
	
	customSizer->Add( portHelpLabel, 0, wxALL|wxALIGN_RIGHT, 1 );
	
	mainSizer->Add( customSizer, 0, wxEXPAND, 5 );
	
	tailLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	mainSizer->Add( tailLine, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* cancelSizer;
	cancelSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	cancelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	cancelButton = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	cancelButton->SetToolTip( _("leave this dialog without change") );
	
	cancelSizer->Add( cancelButton, 0, wxALL, 5 );
	
	mainSizer->Add( cancelSizer, 0, wxEXPAND, 5 );
	
	this->SetSizer( mainSizer );
	this->Layout();
	
	// Connect Events
	searchTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( RuleWizardAlfDlgAddServiceBase::onSearchTextEnter ), NULL, this );
	serviceListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( RuleWizardAlfDlgAddServiceBase::onServiceListDeselect ), NULL, this );
	serviceListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( RuleWizardAlfDlgAddServiceBase::onServiceListSelect ), NULL, this );
	addButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardAlfDlgAddServiceBase::onAddButton ), NULL, this );
	customAddButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardAlfDlgAddServiceBase::onCustomAddButton ), NULL, this );
	cancelButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardAlfDlgAddServiceBase::onCancelButton ), NULL, this );
}

RuleWizardProgramPageBase::RuleWizardProgramPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 3, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("Choose Program:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	programLabel = new wxStaticText( this, wxID_ANY, _("Program:"), wxDefaultPosition, wxDefaultSize, 0 );
	programLabel->Wrap( -1 );
	programLabel->SetMinSize( wxSize( 90,-1 ) );
	
	mainSizer->Add( programLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	programPicker = new AnPickFromFs( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	mainSizer->Add( programPicker, 0, wxEXPAND | wxALL, 5 );
	
	csumLabel = new wxStaticText( this, wxID_ANY, _("Checksum:"), wxDefaultPosition, wxDefaultSize, 0 );
	csumLabel->Wrap( -1 );
	csumLabel->SetMinSize( wxSize( 90,-1 ) );
	
	mainSizer->Add( csumLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	csumValue = new wxStaticText( this, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, 0 );
	csumValue->Wrap( -1 );
	csumValue->SetFont( wxFont( 8, 70, 90, 90, false, wxEmptyString ) );
	
	mainSizer->Add( csumValue, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	programInfo = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	programInfo->Wrap( -1 );
	mainSizer->Add( programInfo, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 7, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
}

RuleWizardContextPageBase::RuleWizardContextPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 3, wxEXPAND, 5 );
	
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
	
	helpPage = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	helpPage->SetBackgroundColour( wxColour( 179, 177, 174 ) );
	
	wxBoxSizer* helpSizer;
	helpSizer = new wxBoxSizer( wxVERTICAL );
	
	helpLabel = new wxStaticText( helpPage, wxID_ANY, _("The context of an application is the conjunction of it's calling environment. This is the program which runs another one.\n\nIf you are creating policies for a laucher-application or a command shell we suggest the answer: \"no\".\n\nBut in case of the most applications \"yes' seems to be the best fit. "), wxDefaultPosition, wxDefaultSize, 0 );
	helpLabel->Wrap( 500 );
	helpSizer->Add( helpLabel, 0, wxALL, 5 );
	
	helpPage->SetSizer( helpSizer );
	helpPage->Layout();
	helpSizer->Fit( helpPage );
	mainSizer->Add( helpPage, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* inputSizer;
	inputSizer = new wxBoxSizer( wxVERTICAL );
	
	questionLabel = new wxStaticText( this, wxID_ANY, _("Programs started by this application have the same permissions / restrictions as the main program."), wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( 400 );
	inputSizer->Add( questionLabel, 0, wxALL, 5 );
	
	
	inputSizer->Add( 0, 30, 1, wxEXPAND, 5 );
	
	yesRadioButton = new wxRadioButton( this, wxID_ANY, _("Yes (same permissions / restrictions)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	inputSizer->Add( yesRadioButton, 0, wxALL, 5 );
	
	wxBoxSizer* exceptionSizer;
	exceptionSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	exceptionSizer->Add( 30, 0, 0, wxEXPAND, 5 );
	
	exceptionsCheckBox = new wxCheckBox( this, wxID_ANY, _("allow exceptions (-> next page)"), wxDefaultPosition, wxDefaultSize, 0 );
	
	exceptionsCheckBox->Enable( false );
	
	exceptionSizer->Add( exceptionsCheckBox, 0, wxALL, 5 );
	
	inputSizer->Add( exceptionSizer, 0, wxEXPAND, 5 );
	
	noRadioButton = new wxRadioButton( this, wxID_ANY, _("No (each executed program has its own policies)"), wxDefaultPosition, wxDefaultSize, 0 );
	inputSizer->Add( noRadioButton, 0, wxALL, 5 );
	
	mainSizer->Add( inputSizer, 0, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 2, wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 7, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	yesRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardContextPageBase::onYesRadioButton ), NULL, this );
	exceptionsCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RuleWizardContextPageBase::onExceptionsCheckBox ), NULL, this );
	noRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardContextPageBase::onNoRadioButton ), NULL, this );
}

RuleWizardContextExceptionPageBase::RuleWizardContextExceptionPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 3, wxEXPAND, 5 );
	
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
	
	questionLabel = new wxStaticText( this, wxID_ANY, _("Programs been started by this application have the\nsame permissions / restrictions as the main application."), wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( -1 );
	mainSizer->Add( questionLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	exceptionListLabel = new wxStaticText( this, wxID_ANY, _("Exceptions (programs started with own polcies):"), wxDefaultPosition, wxDefaultSize, 0 );
	exceptionListLabel->Wrap( -1 );
	mainSizer->Add( exceptionListLabel, 0, wxALL, 5 );
	
	exceptionListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_ALWAYS_SB|wxLB_SINGLE ); 
	mainSizer->Add( exceptionListBox, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* buttonSizer;
	buttonSizer = new wxBoxSizer( wxHORIZONTAL );
	
	addButton = new wxButton( this, wxID_ANY, _("add..."), wxDefaultPosition, wxDefaultSize, 0 );
	addButton->SetToolTip( _("opens a file-choose-dialog and\nadds the result to the list") );
	
	buttonSizer->Add( addButton, 0, wxALL, 5 );
	
	
	buttonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	deleteButton = new wxButton( this, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, 0 );
	deleteButton->SetToolTip( _("removes the selected entry from the list") );
	
	buttonSizer->Add( deleteButton, 0, wxALL, 5 );
	
	mainSizer->Add( buttonSizer, 0, wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 7, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	addButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardContextExceptionPageBase::onAddButton ), NULL, this );
	deleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardContextExceptionPageBase::onDeleteButton ), NULL, this );
}

RuleWizardAlfPermissionPageBase::RuleWizardAlfPermissionPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 3, wxEXPAND, 5 );
	
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
	
	questionLabel = new wxStaticText( this, wxID_ANY, _("Allow the application \"...\" the usage\nof network services (... functionality)?"), wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( 400 );
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
	
	pageSizer->Add( mainSizer, 7, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	yesRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardAlfPermissionPageBase::onYesRadioButton ), NULL, this );
	defaultRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardAlfPermissionPageBase::onDefaultRadioButton ), NULL, this );
	restrictedRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardAlfPermissionPageBase::onRestrictedRadioButton ), NULL, this );
	noRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardAlfPermissionPageBase::onNoRadioButton ), NULL, this );
}

RuleWizardAlfServicePageBase::RuleWizardAlfServicePageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 3, wxEXPAND, 5 );
	
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
	
	questionLabel = new wxStaticText( this, wxID_ANY, _("Options for restricted network access\nfor application \"...\":"), wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( -1 );
	mainSizer->Add( questionLabel, 0, wxALL, 5 );
	
	portListLabel = new wxStaticText( this, wxID_ANY, _("List of allowed services:"), wxDefaultPosition, wxDefaultSize, 0 );
	portListLabel->Wrap( -1 );
	mainSizer->Add( portListLabel, 0, wxALL, 5 );
	
	portListCtrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT );
	mainSizer->Add( portListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* buttonSizer;
	buttonSizer = new wxBoxSizer( wxHORIZONTAL );
	
	addButton = new wxButton( this, wxID_ANY, _(" add..."), wxDefaultPosition, wxDefaultSize, 0 );
	addButton->SetToolTip( _("opens a service-choose-dialog\nand adds the result to the list") );
	
	buttonSizer->Add( addButton, 0, wxALL, 5 );
	
	
	buttonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	defaultsButton = new wxButton( this, wxID_ANY, _("add default services"), wxDefaultPosition, wxDefaultSize, 0 );
	defaultsButton->Enable( false );
	defaultsButton->SetToolTip( _("adds the default services to the list") );
	
	buttonSizer->Add( defaultsButton, 0, wxALL, 5 );
	
	
	buttonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	deleteButton = new wxButton( this, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, 0 );
	deleteButton->Enable( false );
	deleteButton->SetToolTip( _("removes the selected items from the list") );
	
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
	
	pageSizer->Add( mainSizer, 7, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	portListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( RuleWizardAlfServicePageBase::onPortListDeselect ), NULL, this );
	portListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( RuleWizardAlfServicePageBase::onPortListSelect ), NULL, this );
	addButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardAlfServicePageBase::onAddButton ), NULL, this );
	defaultsButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardAlfServicePageBase::onDefaultsButton ), NULL, this );
	deleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardAlfServicePageBase::onDeleteButton ), NULL, this );
	askCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RuleWizardAlfServicePageBase::onAskCheckBox ), NULL, this );
	rawCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RuleWizardAlfServicePageBase::onRawCheckBox ), NULL, this );
}

RuleWizardSandboxPageBase::RuleWizardSandboxPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 3, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("Sandbox settings:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	helpPage = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	helpPage->SetBackgroundColour( wxColour( 179, 177, 174 ) );
	
	wxBoxSizer* helpSizer;
	helpSizer = new wxBoxSizer( wxVERTICAL );
	
	helpLabel = new wxStaticText( helpPage, wxID_ANY, _("With the sandbox you can confine an application.\nThis is done by specifying which files it may read, write or execute."), wxDefaultPosition, wxDefaultSize, 0 );
	helpLabel->Wrap( 500 );
	helpSizer->Add( helpLabel, 0, wxALL, 5 );
	
	helpPage->SetSizer( helpSizer );
	helpPage->Layout();
	helpSizer->Fit( helpPage );
	mainSizer->Add( helpPage, 1, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* inputSizer;
	inputSizer = new wxBoxSizer( wxVERTICAL );
	
	questionLabel = new wxStaticText( this, wxID_ANY, _("Create sandbox policies?"), wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( 400 );
	inputSizer->Add( questionLabel, 0, wxALL, 5 );
	
	
	inputSizer->Add( 0, 30, 1, wxEXPAND, 5 );
	
	yesWizardRadioButton = new wxRadioButton( this, wxID_ANY, _("Yes, create policies (wizard guided)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	inputSizer->Add( yesWizardRadioButton, 0, wxALL, 5 );
	
	yesDefaultsRadioButton = new wxRadioButton( this, wxID_ANY, _("Yes, load default policies (skip wizard)"), wxDefaultPosition, wxDefaultSize, 0 );
	inputSizer->Add( yesDefaultsRadioButton, 0, wxALL, 5 );
	
	noRadioButton = new wxRadioButton( this, wxID_ANY, _("No, do not  create sandbox policies"), wxDefaultPosition, wxDefaultSize, 0 );
	inputSizer->Add( noRadioButton, 0, wxALL, 5 );
	
	mainSizer->Add( inputSizer, 0, wxEXPAND, 5 );
	
	
	mainSizer->Add( 0, 0, 2, wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 7, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	yesWizardRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardSandboxPageBase::onYesWizardRadioButton ), NULL, this );
	yesDefaultsRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardSandboxPageBase::onYesDefaultsRadioButton ), NULL, this );
	noRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardSandboxPageBase::onNoRadioButton ), NULL, this );
}

RuleWizardSandboxPermissionPageBase::RuleWizardSandboxPermissionPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 3, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("Sandbox settings:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	wxBoxSizer* inputSizer;
	inputSizer = new wxBoxSizer( wxVERTICAL );
	
	questionLabel = new wxStaticText( this, wxID_ANY, _("Select permissions of application \"...\"\nfor ... file access:"), wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( 400 );
	inputSizer->Add( questionLabel, 0, wxALL, 5 );
	
	
	inputSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	allowAllRadioButton = new wxRadioButton( this, wxID_ANY, _("unrestricted"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	inputSizer->Add( allowAllRadioButton, 0, wxALL, 5 );
	
	defaultRadioButton = new wxRadioButton( this, wxID_ANY, _("restricted (default)"), wxDefaultPosition, wxDefaultSize, 0 );
	inputSizer->Add( defaultRadioButton, 0, wxALL, 5 );
	
	wxBoxSizer* defaultSizer;
	defaultSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	defaultSizer->Add( 30, 0, 0, wxEXPAND, 5 );
	
	defaultLabel = new wxStaticText( this, wxID_ANY, _("Default permissions are allowed, any other\naccess is asked on demand."), wxPoint( -1,-1 ), wxDefaultSize, 0 );
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
	
	mainSizer->Add( inputSizer, 0, wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 7, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	allowAllRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardSandboxPermissionPageBase::onAllowAllRadioButton ), NULL, this );
	defaultRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardSandboxPermissionPageBase::onDefaultRadioButton ), NULL, this );
	restrictedRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( RuleWizardSandboxPermissionPageBase::onRestrictedRadioButton ), NULL, this );
}

RuleWizardSandboxFilesPageBase::RuleWizardSandboxFilesPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 3, wxEXPAND, 5 );
	
	naviMainDelimiter = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	pageSizer->Add( naviMainDelimiter, 0, wxEXPAND | wxALL, 5 );
	
	
	pageSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	headLineLabel = new wxStaticText( this, wxID_ANY, _("Sandbox settings:"), wxDefaultPosition, wxDefaultSize, 0 );
	headLineLabel->Wrap( -1 );
	headLineLabel->SetFont( wxFont( 18, 70, 90, 92, false, wxEmptyString ) );
	
	mainSizer->Add( headLineLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 50, 0, wxEXPAND, 5 );
	
	questionLabel = new wxStaticText( this, wxID_ANY, _("Options for ... access of \"...\":"), wxDefaultPosition, wxDefaultSize, 0 );
	questionLabel->Wrap( -1 );
	mainSizer->Add( questionLabel, 0, wxALL, 5 );
	
	fileListLabel = new wxStaticText( this, wxID_ANY, _("... grand access:"), wxDefaultPosition, wxDefaultSize, 0 );
	fileListLabel->Wrap( -1 );
	mainSizer->Add( fileListLabel, 0, wxALL, 5 );
	
	fileListCtrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT );
	mainSizer->Add( fileListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* buttonSizer;
	buttonSizer = new wxBoxSizer( wxHORIZONTAL );
	
	addFileButton = new wxButton( this, wxID_ANY, _(" add file..."), wxDefaultPosition, wxDefaultSize, 0 );
	addFileButton->SetToolTip( _("opens a file-choose-dialog\nand adds the result to the list") );
	
	buttonSizer->Add( addFileButton, 0, wxALL, 5 );
	
	addDirectoryButton = new wxButton( this, wxID_ANY, _(" add directory..."), wxDefaultPosition, wxDefaultSize, 0 );
	addDirectoryButton->SetToolTip( _("opens a directory-choose-dialog\nand adds the result to the list") );
	
	buttonSizer->Add( addDirectoryButton, 0, wxALL, 5 );
	
	
	buttonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	defaultsButton = new wxButton( this, wxID_ANY, _("add default permissions"), wxDefaultPosition, wxDefaultSize, 0 );
	defaultsButton->Enable( false );
	defaultsButton->SetToolTip( _("adds the default services to the list") );
	
	buttonSizer->Add( defaultsButton, 0, wxALL, 5 );
	
	
	buttonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	deleteButton = new wxButton( this, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, 0 );
	deleteButton->Enable( false );
	deleteButton->SetToolTip( _("removes the selected items from the list") );
	
	buttonSizer->Add( deleteButton, 0, wxALL, 5 );
	
	mainSizer->Add( buttonSizer, 0, wxEXPAND, 5 );
	
	detailsPanel = new AnDetails( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxT("Details...") );
	wxBoxSizer* detailsSizer;
	detailsSizer = new wxBoxSizer( wxVERTICAL );
	
	askCheckBox = new wxCheckBox( detailsPanel, wxID_ANY, _("ask on any other access"), wxDefaultPosition, wxDefaultSize, 0 );
	askCheckBox->SetValue(true);
	
	detailsSizer->Add( askCheckBox, 0, wxALL, 5 );
	
	validCheckBox = new wxCheckBox( detailsPanel, wxID_ANY, _("always allow access on valid checksum / signature"), wxDefaultPosition, wxDefaultSize, 0 );
	
	detailsSizer->Add( validCheckBox, 0, wxALL, 5 );
	
	detailsPanel->SetSizer( detailsSizer );
	detailsPanel->Layout();
	detailsSizer->Fit( detailsPanel );
	mainSizer->Add( detailsPanel, 0, wxEXPAND | wxALL, 5 );
	
	pageSizer->Add( mainSizer, 7, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
	
	// Connect Events
	fileListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( RuleWizardSandboxFilesPageBase::onFileListDeselect ), NULL, this );
	fileListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( RuleWizardSandboxFilesPageBase::onFileListSelect ), NULL, this );
	addFileButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardSandboxFilesPageBase::onAddFileButton ), NULL, this );
	addDirectoryButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardSandboxFilesPageBase::onAddDirectoryButton ), NULL, this );
	defaultsButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardSandboxFilesPageBase::onDefaultsButton ), NULL, this );
	deleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RuleWizardSandboxFilesPageBase::onDeleteButton ), NULL, this );
	askCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RuleWizardSandboxFilesPageBase::onAskCheckBox ), NULL, this );
	validCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RuleWizardSandboxFilesPageBase::onValidCheckBox ), NULL, this );
}

RuleWizardFinalPageBase::RuleWizardFinalPageBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* pageSizer;
	pageSizer = new wxBoxSizer( wxHORIZONTAL );
	
	naviSizer = new wxBoxSizer( wxVERTICAL );
	
	pageSizer->Add( naviSizer, 3, wxEXPAND, 5 );
	
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
	
	finalLabel = new wxStaticText( this, wxID_ANY, _("This is the final page. If you click on 'finish'\nthe policies will be created.\n\nTo activate the newly created policies, please open the RuleEditor. Activate the policies with the button 'activate' in the lower right corner.\n(To make this work you need to be connected.)"), wxDefaultPosition, wxDefaultSize, 0 );
	finalLabel->Wrap( 400 );
	mainSizer->Add( finalLabel, 0, wxALL, 5 );
	
	
	mainSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	pageSizer->Add( mainSizer, 7, wxEXPAND, 5 );
	
	this->SetSizer( pageSizer );
	this->Layout();
}
