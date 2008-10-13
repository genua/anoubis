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
	
	wxBoxSizer* sz_main;
	sz_main = new wxBoxSizer( wxVERTICAL );
	
	sz_main->SetMinSize( wxSize( 780,-1 ) ); 
	controlRuleSizer = new wxBoxSizer( wxHORIZONTAL );
	
	controlRuleText = new wxStaticText( this, wxID_ANY, _("Rule:"), wxDefaultPosition, wxDefaultSize, 0 );
	controlRuleText->Wrap( -1 );
	controlRuleText->SetMinSize( wxSize( 100,-1 ) );
	
	controlRuleSizer->Add( controlRuleText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString controlCreationChoiceChoices[] = { _("Application"), _("AppFilter"), _("SFS"), _("Variable") };
	int controlCreationChoiceNChoices = sizeof( controlCreationChoiceChoices ) / sizeof( wxString );
	controlCreationChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, controlCreationChoiceNChoices, controlCreationChoiceChoices, wxTAB_TRAVERSAL );
	controlRuleSizer->Add( controlCreationChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlRuleCreateButton = new wxButton( this, wxID_ANY, _("create"), wxDefaultPosition, wxDefaultSize, 0 );
	controlRuleSizer->Add( controlRuleCreateButton, 0, wxALL, 5 );
	
	controlUserLabel = new wxStaticText( this, wxID_ANY, _("for user"), wxDefaultPosition, wxDefaultSize, 0 );
	controlUserLabel->Wrap( -1 );
	controlRuleSizer->Add( controlUserLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxArrayString controlUserChoiceChoices;
	controlUserChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, controlUserChoiceChoices, 0 );
	controlRuleSizer->Add( controlUserChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlRuleText1 = new wxStaticText( this, wxID_ANY, _("/ Rule:"), wxDefaultPosition, wxDefaultSize, 0 );
	controlRuleText1->Wrap( -1 );
	controlRuleSizer->Add( controlRuleText1, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlRuleDeleteButton = new wxButton( this, wxID_ANY, _("delete"), wxDefaultPosition, wxDefaultSize, 0 );
	controlRuleSizer->Add( controlRuleDeleteButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	controlRuleSizer->Add( 1, 0, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlFilterText = new wxStaticText( this, wxID_ANY, _("Search:"), wxDefaultPosition, wxDefaultSize, 0 );
	controlFilterText->Wrap( -1 );
	controlFilterText->Enable( false );
	controlFilterText->Hide();
	
	controlRuleSizer->Add( controlFilterText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlFilterTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_NO_VSCROLL );
	controlFilterTextCtrl->Enable( false );
	controlFilterTextCtrl->Hide();
	
	controlRuleSizer->Add( controlFilterTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlFilterInText = new wxStaticText( this, wxID_ANY, _("in"), wxDefaultPosition, wxDefaultSize, 0 );
	controlFilterInText->Wrap( -1 );
	controlFilterInText->Enable( false );
	controlFilterInText->Hide();
	
	controlRuleSizer->Add( controlFilterInText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString controlFilterChoiceChoices[] = { _("all"), _("Application"), _("Ip") };
	int controlFilterChoiceNChoices = sizeof( controlFilterChoiceChoices ) / sizeof( wxString );
	controlFilterChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, controlFilterChoiceNChoices, controlFilterChoiceChoices, 0 );
	controlFilterChoice->Enable( false );
	controlFilterChoice->Hide();
	
	controlRuleSizer->Add( controlFilterChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sz_main->Add( controlRuleSizer, 0, wxEXPAND, 5 );
	
	wxBoxSizer* controlRuleSetSizer;
	controlRuleSetSizer = new wxBoxSizer( wxHORIZONTAL );
	
	controlRuleSetText = new wxStaticText( this, wxID_ANY, _("Ruleset:"), wxDefaultPosition, wxDefaultSize, 0|wxTAB_TRAVERSAL );
	controlRuleSetText->Wrap( -1 );
	controlRuleSetText->SetMinSize( wxSize( 100,-1 ) );
	
	controlRuleSetSizer->Add( controlRuleSetText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlRuleSetSaveButton = new wxButton( this, wxID_ANY, _("store"), wxDefaultPosition, wxDefaultSize, 0 );
	controlRuleSetSizer->Add( controlRuleSetSaveButton, 0, wxALL, 5 );
	
	
	controlRuleSetSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	controlOptionText = new wxStaticText( this, wxID_ANY, _("Table:"), wxDefaultPosition, wxDefaultSize, 0 );
	controlOptionText->Wrap( -1 );
	controlOptionText->Enable( false );
	controlOptionText->Hide();
	
	controlRuleSetSizer->Add( controlOptionText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlOptionButton = new wxButton( this, wxID_ANY, _("Options..."), wxDefaultPosition, wxDefaultSize, 0 );
	controlOptionButton->Enable( false );
	controlOptionButton->Hide();
	
	controlRuleSetSizer->Add( controlOptionButton, 0, wxALL, 5 );
	
	sz_main->Add( controlRuleSetSizer, 0, wxEXPAND, 5 );
	
	ruleListCtrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL );
	sz_main->Add( ruleListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	ruleEditNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0|wxTAB_TRAVERSAL|wxVSCROLL );
	commonNbPanel = new wxScrolledWindow( ruleEditNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxTRANSPARENT_WINDOW|wxVSCROLL );
	commonNbPanel->SetScrollRate( 5, 5 );
	wxBoxSizer* commonMainSizer;
	commonMainSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* commonRuleBox;
	commonRuleBox = new wxStaticBoxSizer( new wxStaticBox( commonNbPanel, -1, _("Rule") ), wxVERTICAL );
	
	wxFlexGridSizer* commonRuleSizer;
	commonRuleSizer = new wxFlexGridSizer( 5, 2, 0, 0 );
	commonRuleSizer->SetFlexibleDirection( wxBOTH );
	commonRuleSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	commonModuleText = new wxStaticText( commonNbPanel, wxID_ANY, _("Module:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonModuleText->Wrap( -1 );
	commonModuleText->Enable( false );
	commonModuleText->Hide();
	
	commonRuleSizer->Add( commonModuleText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString commonModuleChoiceChoices[] = { _("ALF"), _("SFS"), _("Macro") };
	int commonModuleChoiceNChoices = sizeof( commonModuleChoiceChoices ) / sizeof( wxString );
	commonModuleChoice = new wxChoice( commonNbPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, commonModuleChoiceNChoices, commonModuleChoiceChoices, 0 );
	commonModuleChoice->Enable( false );
	commonModuleChoice->Hide();
	
	commonRuleSizer->Add( commonModuleChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonStateText = new wxStaticText( commonNbPanel, wxID_ANY, _("State:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonStateText->Wrap( -1 );
	commonStateText->Enable( false );
	commonStateText->Hide();
	
	commonRuleSizer->Add( commonStateText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* commonStateSizer;
	commonStateSizer = new wxBoxSizer( wxHORIZONTAL );
	
	commonActiveRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, _("activated"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	commonActiveRadioButton->Enable( false );
	commonActiveRadioButton->Hide();
	
	commonStateSizer->Add( commonActiveRadioButton, 0, wxALL, 5 );
	
	commonDeactiveRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, _("deactivated"), wxDefaultPosition, wxDefaultSize, 0 );
	commonDeactiveRadioButton->Enable( false );
	commonDeactiveRadioButton->Hide();
	
	commonStateSizer->Add( commonDeactiveRadioButton, 0, wxALL, 5 );
	
	commonRuleSizer->Add( commonStateSizer, 1, wxEXPAND, 5 );
	
	commonNameText = new wxStaticText( commonNbPanel, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonNameText->Wrap( -1 );
	commonNameText->Enable( false );
	commonNameText->Hide();
	
	commonRuleSizer->Add( commonNameText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonNameTextCtrl = new wxTextCtrl( commonNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	commonNameTextCtrl->Enable( false );
	commonNameTextCtrl->Hide();
	
	commonRuleSizer->Add( commonNameTextCtrl, 0, wxALL|wxEXPAND, 5 );
	
	commonPriorityText = new wxStaticText( commonNbPanel, wxID_ANY, _("Priority:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonPriorityText->Wrap( -1 );
	commonPriorityText->Enable( false );
	commonPriorityText->Hide();
	
	commonRuleSizer->Add( commonPriorityText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonPrioritySpinCtrl = new wxSpinCtrl( commonNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	commonPrioritySpinCtrl->Enable( false );
	commonPrioritySpinCtrl->Hide();
	
	commonRuleSizer->Add( commonPrioritySpinCtrl, 0, wxALL, 5 );
	
	commonFaderText = new wxStaticText( commonNbPanel, wxID_ANY, _("Profile:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonFaderText->Wrap( -1 );
	commonRuleSizer->Add( commonFaderText, 0, wxALL, 5 );
	
	commonHighProfileRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, _("Profile: high"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	commonRuleSizer->Add( commonHighProfileRadioButton, 0, wxALL, 5 );
	
	
	commonRuleSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	commonMediumProfileRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, _("Profile: medium"), wxDefaultPosition, wxDefaultSize, 0 );
	commonRuleSizer->Add( commonMediumProfileRadioButton, 0, wxALL, 5 );
	
	
	commonRuleSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	commonAdminProfileRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, _("Profile: admin"), wxDefaultPosition, wxDefaultSize, 0 );
	commonRuleSizer->Add( commonAdminProfileRadioButton, 0, wxALL, 5 );
	
	commonCommentText = new wxStaticText( commonNbPanel, wxID_ANY, _("Comment:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonCommentText->Wrap( -1 );
	commonRuleSizer->Add( commonCommentText, 0, wxALL, 5 );
	
	commonCommentTextCtrl = new wxTextCtrl( commonNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	commonRuleSizer->Add( commonCommentTextCtrl, 1, wxALL|wxEXPAND, 5 );
	
	commonRuleBox->Add( commonRuleSizer, 1, wxEXPAND, 5 );
	
	commonMainSizer->Add( commonRuleBox, 1, wxEXPAND, 5 );
	
	wxBoxSizer* commonRightSideSizer;
	commonRightSideSizer = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* logBox;
	logBox = new wxStaticBoxSizer( new wxStaticBox( commonNbPanel, -1, _("Logging") ), wxVERTICAL );
	
	commonNoneLogRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, _("none "), wxDefaultPosition, wxDefaultSize, 0 );
	logBox->Add( commonNoneLogRadioButton, 0, wxALL, 5 );
	
	commonDoLogRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, _("log"), wxDefaultPosition, wxDefaultSize, 0 );
	logBox->Add( commonDoLogRadioButton, 0, wxALL, 5 );
	
	commonAlertLogRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, _("alert"), wxDefaultPosition, wxDefaultSize, 0 );
	logBox->Add( commonAlertLogRadioButton, 0, wxALL, 5 );
	
	commonRightSideSizer->Add( logBox, 1, wxEXPAND, 5 );
	
	commonMainSizer->Add( commonRightSideSizer, 1, wxEXPAND, 5 );
	
	commonNbPanel->SetSizer( commonMainSizer );
	commonNbPanel->Layout();
	commonMainSizer->Fit( commonNbPanel );
	ruleEditNotebook->AddPage( commonNbPanel, _("Common"), false );
	applicationNbPanel = new wxScrolledWindow( ruleEditNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	applicationNbPanel->SetScrollRate( 5, 5 );
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* appMainPanelSizer;
	appMainPanelSizer = new wxFlexGridSizer( 2, 5, 0, 0 );
	appMainPanelSizer->SetFlexibleDirection( wxBOTH );
	appMainPanelSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	appNameText = new wxStaticText( applicationNbPanel, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	appNameText->Wrap( -1 );
	appNameText->SetMinSize( wxSize( 165,-1 ) );
	
	appMainPanelSizer->Add( appNameText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appNameComboBox = new wxComboBox( applicationNbPanel, wxID_ANY, _("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	appMainPanelSizer->Add( appNameComboBox, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	appBinaryText = new wxStaticText( applicationNbPanel, wxID_ANY, _("Binaries:"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryText->Wrap( -1 );
	appMainPanelSizer->Add( appBinaryText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* appGuessSizer;
	appGuessSizer = new wxBoxSizer( wxHORIZONTAL );
	
	appGuessButton = new wxButton( applicationNbPanel, wxID_ANY, _("guess ..."), wxDefaultPosition, wxDefaultSize, 0 );
	appGuessSizer->Add( appGuessButton, 0, wxALL, 5 );
	
	appGuessText = new wxStaticText( applicationNbPanel, wxID_ANY, _("(by application)"), wxDefaultPosition, wxDefaultSize, 0 );
	appGuessText->Wrap( -1 );
	appGuessSizer->Add( appGuessText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appMainPanelSizer->Add( appGuessSizer, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	appBinaryTextCtrl = new wxTextCtrl( applicationNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	appMainPanelSizer->Add( appBinaryTextCtrl, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	appBinaryModifyButton = new wxButton( applicationNbPanel, wxID_ANY, _("modify"), wxDefaultPosition, wxDefaultSize, 0 );
	appMainPanelSizer->Add( appBinaryModifyButton, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	appMainPanelSizer->Add( 35, 0, 1, wxEXPAND, 5 );
	
	appBinaryAddButton = new wxButton( applicationNbPanel, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appBinaryAddButton->Enable( false );
	appBinaryAddButton->Hide();
	
	appMainPanelSizer->Add( appBinaryAddButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appContextText = new wxStaticText( applicationNbPanel, wxID_ANY, _("Context:"), wxDefaultPosition, wxDefaultSize, 0 );
	appContextText->Wrap( -1 );
	appMainPanelSizer->Add( appContextText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appContextTextCtrl = new wxTextCtrl( applicationNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	appMainPanelSizer->Add( appContextTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	appContextModifyButton = new wxButton( applicationNbPanel, wxID_ANY, _("modify"), wxDefaultPosition, wxDefaultSize, 0 );
	appMainPanelSizer->Add( appContextModifyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	appInheritanceAddButton = new wxButton( applicationNbPanel, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appInheritanceAddButton->Enable( false );
	appInheritanceAddButton->Hide();
	
	appMainPanelSizer->Add( appInheritanceAddButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer10->Add( appMainPanelSizer, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer10;
	fgSizer10 = new wxFlexGridSizer( 1, 5, 0, 0 );
	fgSizer10->SetFlexibleDirection( wxBOTH );
	fgSizer10->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	appRegisteredSumLabelText = new wxStaticText( applicationNbPanel, wxID_ANY, _("Checksum (registered):"), wxDefaultPosition, wxDefaultSize, 0 );
	appRegisteredSumLabelText->Wrap( -1 );
	appRegisteredSumLabelText->SetMinSize( wxSize( 165,-1 ) );
	
	fgSizer10->Add( appRegisteredSumLabelText, 0, wxALIGN_BOTTOM|wxALL, 5 );
	
	appRegisteredSumValueText = new wxStaticText( applicationNbPanel, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	appRegisteredSumValueText->Wrap( -1 );
	fgSizer10->Add( appRegisteredSumValueText, 0, wxALIGN_BOTTOM|wxALL, 5 );
	
	
	fgSizer10->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer10->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer10->Add( 0, 0, 1, wxEXPAND, 5 );
	
	appCurrentSumLabelText = new wxStaticText( applicationNbPanel, wxID_ANY, _("Checksum (current):"), wxDefaultPosition, wxDefaultSize, 0 );
	appCurrentSumLabelText->Wrap( -1 );
	fgSizer10->Add( appCurrentSumLabelText, 0, wxALIGN_TOP|wxLEFT, 5 );
	
	appCurrentSumValueText = new wxStaticText( applicationNbPanel, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	appCurrentSumValueText->Wrap( -1 );
	fgSizer10->Add( appCurrentSumValueText, 0, wxLEFT, 5 );
	
	bSizer10->Add( fgSizer10, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer101;
	fgSizer101 = new wxFlexGridSizer( 2, 5, 0, 0 );
	fgSizer101->SetFlexibleDirection( wxBOTH );
	fgSizer101->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	appStatusLabelText = new wxStaticText( applicationNbPanel, wxID_ANY, _("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	appStatusLabelText->Wrap( -1 );
	appStatusLabelText->SetMinSize( wxSize( 165,-1 ) );
	
	fgSizer101->Add( appStatusLabelText, 0, wxALIGN_TOP|wxALL, 5 );
	
	appStatusValueText = new wxStaticText( applicationNbPanel, wxID_ANY, _("mismatch"), wxDefaultPosition, wxDefaultSize, 0 );
	appStatusValueText->Wrap( -1 );
	fgSizer101->Add( appStatusValueText, 0, wxALIGN_TOP|wxALL, 5 );
	
	appValidateChkSumButton = new wxButton( applicationNbPanel, wxID_ANY, _("validate"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer101->Add( appValidateChkSumButton, 0, wxALIGN_TOP|wxALL, 5 );
	
	appUpdateChkSumButton = new wxButton( applicationNbPanel, wxID_ANY, _("update"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer101->Add( appUpdateChkSumButton, 0, wxALIGN_TOP|wxALL, 5 );
	
	bSizer10->Add( fgSizer101, 1, wxEXPAND, 5 );
	
	applicationNbPanel->SetSizer( bSizer10 );
	applicationNbPanel->Layout();
	bSizer10->Fit( applicationNbPanel );
	ruleEditNotebook->AddPage( applicationNbPanel, _("Application"), false );
	alfNbPanel = new wxScrolledWindow( ruleEditNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxTRANSPARENT_WINDOW|wxVSCROLL );
	alfNbPanel->SetScrollRate( 5, 5 );
	wxBoxSizer* alfPanelMainSizer;
	alfPanelMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* alfOptionSizer;
	alfOptionSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
	alfOptionSizer->SetFlexibleDirection( wxBOTH );
	alfOptionSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	alfActionText = new wxStaticText( alfNbPanel, wxID_ANY, _("Action:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfActionText->Wrap( -1 );
	alfOptionSizer->Add( alfActionText, 0, wxALL, 5 );
	
	alfAllowRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("allow"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	alfOptionSizer->Add( alfAllowRadioButton, 0, wxALL, 5 );
	
	alfDenyRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("deny"), wxDefaultPosition, wxDefaultSize, 0 );
	alfOptionSizer->Add( alfDenyRadioButton, 0, wxALL, 5 );
	
	alfAskRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("ask"), wxDefaultPosition, wxDefaultSize, 0 );
	alfOptionSizer->Add( alfAskRadioButton, 0, wxALL, 5 );
	
	alfTypeText = new wxStaticText( alfNbPanel, wxID_ANY, _("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfTypeText->Wrap( -1 );
	alfOptionSizer->Add( alfTypeText, 0, wxALL, 5 );
	
	alfFilterRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("filter"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	alfOptionSizer->Add( alfFilterRadioButton, 0, wxALL, 5 );
	
	alfCapRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("capability"), wxDefaultPosition, wxDefaultSize, 0 );
	alfOptionSizer->Add( alfCapRadioButton, 0, wxALL, 5 );
	
	alfDefaultRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("default"), wxDefaultPosition, wxDefaultSize, 0 );
	alfOptionSizer->Add( alfDefaultRadioButton, 0, wxALL, 5 );
	
	alfPanelMainSizer->Add( alfOptionSizer, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* alfConnectionBox;
	alfConnectionBox = new wxStaticBoxSizer( new wxStaticBox( alfNbPanel, -1, _("Connection") ), wxVERTICAL );
	
	wxFlexGridSizer* alfConnectOptionSizer;
	alfConnectOptionSizer = new wxFlexGridSizer( 3, 4, 0, 0 );
	alfConnectOptionSizer->SetFlexibleDirection( wxBOTH );
	alfConnectOptionSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	alfProtocolText = new wxStaticText( alfNbPanel, wxID_ANY, _("Protocol:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfProtocolText->Wrap( -1 );
	alfConnectOptionSizer->Add( alfProtocolText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfTcpRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("tcp"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	alfConnectOptionSizer->Add( alfTcpRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfUdpRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("udp"), wxDefaultPosition, wxDefaultSize, 0 );
	alfConnectOptionSizer->Add( alfUdpRadioButton, 0, wxALL, 5 );
	
	
	alfConnectOptionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	alfAddrFamilyText = new wxStaticText( alfNbPanel, wxID_ANY, _("Address family:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfAddrFamilyText->Wrap( -1 );
	alfConnectOptionSizer->Add( alfAddrFamilyText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfInetRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("inet"), wxDefaultPosition, wxSize( -1,-1 ), wxRB_GROUP );
	alfConnectOptionSizer->Add( alfInetRadioButton, 0, wxALL, 5 );
	
	alfInet6RadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("inet6"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	alfConnectOptionSizer->Add( alfInet6RadioButton, 0, wxALL, 5 );
	
	alfAnyRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("any"), wxDefaultPosition, wxDefaultSize, 0 );
	alfConnectOptionSizer->Add( alfAnyRadioButton, 0, wxALL, 5 );
	
	alfCapText = new wxStaticText( alfNbPanel, wxID_ANY, _("Capability:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfCapText->Wrap( -1 );
	alfConnectOptionSizer->Add( alfCapText, 0, wxALL, 5 );
	
	alfRawCapRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("raw"), wxDefaultPosition, wxSize( -1,-1 ), wxRB_GROUP );
	alfConnectOptionSizer->Add( alfRawCapRadioButton, 0, wxALL, 5 );
	
	alfOtherCapRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("other"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	alfConnectOptionSizer->Add( alfOtherCapRadioButton, 0, wxALL, 5 );
	
	alfAllCapRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("all"), wxDefaultPosition, wxDefaultSize, 0 );
	alfConnectOptionSizer->Add( alfAllCapRadioButton, 0, wxALL, 5 );
	
	alfDirectionText = new wxStaticText( alfNbPanel, wxID_ANY, _("Direction:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfDirectionText->Wrap( -1 );
	alfConnectOptionSizer->Add( alfDirectionText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfAcceptRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("accept"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	alfConnectOptionSizer->Add( alfAcceptRadioButton, 0, wxALL, 5 );
	
	alfConnectRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("connect"), wxDefaultPosition, wxDefaultSize, 0 );
	alfConnectOptionSizer->Add( alfConnectRadioButton, 0, wxALL, 5 );
	
	alfBothRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, _("both"), wxDefaultPosition, wxDefaultSize, 0 );
	alfBothRadioButton->Hide();
	
	alfConnectOptionSizer->Add( alfBothRadioButton, 0, wxALL, 5 );
	
	alfConnectionBox->Add( alfConnectOptionSizer, 0, wxEXPAND, 5 );
	
	alfConnectAddrSizer = new wxFlexGridSizer( 2, 6, 0, 0 );
	alfConnectAddrSizer->SetFlexibleDirection( wxBOTH );
	alfConnectAddrSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	alfSrcAddrText = new wxStaticText( alfNbPanel, wxID_ANY, _("Source address:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfSrcAddrText->Wrap( -1 );
	alfConnectAddrSizer->Add( alfSrcAddrText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcAddrComboBox = new wxComboBox( alfNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	alfConnectAddrSizer->Add( alfSrcAddrComboBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcAddrDelimiterText = new wxStaticText( alfNbPanel, wxID_ANY, _(" / "), wxDefaultPosition, wxDefaultSize, 0 );
	alfSrcAddrDelimiterText->Wrap( -1 );
	alfConnectAddrSizer->Add( alfSrcAddrDelimiterText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcAddrNetSpinCtrl = new wxSpinCtrl( alfNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 128, 0 );
	alfConnectAddrSizer->Add( alfSrcAddrNetSpinCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcAddrDelButton = new wxButton( alfNbPanel, wxID_ANY, _("-"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	alfConnectAddrSizer->Add( alfSrcAddrDelButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcAddrAddButton = new wxButton( alfNbPanel, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	alfConnectAddrSizer->Add( alfSrcAddrAddButton, 0, wxALL, 5 );
	
	alfDstAddrText = new wxStaticText( alfNbPanel, wxID_ANY, _("Destination address:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfDstAddrText->Wrap( -1 );
	alfConnectAddrSizer->Add( alfDstAddrText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfDstAddrComboBox = new wxComboBox( alfNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	alfConnectAddrSizer->Add( alfDstAddrComboBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfDstAddrDelimiterText = new wxStaticText( alfNbPanel, wxID_ANY, _(" / "), wxDefaultPosition, wxDefaultSize, 0 );
	alfDstAddrDelimiterText->Wrap( -1 );
	alfConnectAddrSizer->Add( alfDstAddrDelimiterText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfDstAddrNetSpinCtrl = new wxSpinCtrl( alfNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 128, 0 );
	alfConnectAddrSizer->Add( alfDstAddrNetSpinCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfDstAddrDelButton = new wxButton( alfNbPanel, wxID_ANY, _("-"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	alfConnectAddrSizer->Add( alfDstAddrDelButton, 0, wxALL, 5 );
	
	alfDstAddrAddButton = new wxButton( alfNbPanel, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	alfConnectAddrSizer->Add( alfDstAddrAddButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcPortText = new wxStaticText( alfNbPanel, wxID_ANY, _("Source Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfSrcPortText->Wrap( -1 );
	alfConnectAddrSizer->Add( alfSrcPortText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcPortComboBox = new wxComboBox( alfNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	alfSrcPortComboBox->Append( _("80") );
	alfSrcPortComboBox->Append( _("443") );
	alfSrcPortComboBox->Append( _("$www") );
	alfSrcPortComboBox->Append( _("21,22") );
	alfConnectAddrSizer->Add( alfSrcPortComboBox, 0, wxALL, 5 );
	
	
	alfConnectAddrSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectAddrSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectAddrSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectAddrSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	alfDstPortText = new wxStaticText( alfNbPanel, wxID_ANY, _("Destination Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfDstPortText->Wrap( -1 );
	alfConnectAddrSizer->Add( alfDstPortText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfDstPortComboBox = new wxComboBox( alfNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	alfDstPortComboBox->Append( _("80") );
	alfDstPortComboBox->Append( _("443") );
	alfDstPortComboBox->Append( _("$www") );
	alfDstPortComboBox->Append( _("21,22") );
	alfConnectAddrSizer->Add( alfDstPortComboBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	alfConnectAddrSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectAddrSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectAddrSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectAddrSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	alfStateTimeoutText = new wxStaticText( alfNbPanel, wxID_ANY, _("State timeout:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfStateTimeoutText->Wrap( -1 );
	alfConnectAddrSizer->Add( alfStateTimeoutText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfStateTimeoutSpinCtrl = new wxSpinCtrl( alfNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 604800, 0 );
	alfStateTimeoutSpinCtrl->SetToolTip( _("To enable stateful filtering use the timeout 600.") );
	
	alfConnectAddrSizer->Add( alfStateTimeoutSpinCtrl, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfConnectionBox->Add( alfConnectAddrSizer, 1, wxEXPAND, 5 );
	
	alfPanelMainSizer->Add( alfConnectionBox, 1, wxEXPAND, 5 );
	
	alfNbPanel->SetSizer( alfPanelMainSizer );
	alfNbPanel->Layout();
	alfPanelMainSizer->Fit( alfNbPanel );
	ruleEditNotebook->AddPage( alfNbPanel, _("ALF"), false );
	sfsNbPanel = new wxScrolledWindow( ruleEditNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	sfsNbPanel->SetScrollRate( 5, 5 );
	wxFlexGridSizer* sfsSizer;
	sfsSizer = new wxFlexGridSizer( 3, 4, 0, 0 );
	sfsSizer->SetFlexibleDirection( wxBOTH );
	sfsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	sfsBinaryLabelText = new wxStaticText( sfsNbPanel, wxID_ANY, _("Binary:"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsBinaryLabelText->Wrap( -1 );
	sfsSizer->Add( sfsBinaryLabelText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sfsBinaryTextCtrl = new wxTextCtrl( sfsNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), wxTE_PROCESS_ENTER );
	sfsSizer->Add( sfsBinaryTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sfsBinaryModifyButton = new wxButton( sfsNbPanel, wxID_ANY, _("modify"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsSizer->Add( sfsBinaryModifyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sfsSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	sfsRegisteredSumLabelText = new wxStaticText( sfsNbPanel, wxID_ANY, _("Checksum (registered):"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsRegisteredSumLabelText->Wrap( -1 );
	sfsSizer->Add( sfsRegisteredSumLabelText, 0, wxALL, 5 );
	
	sfsRegisteredSumValueText = new wxStaticText( sfsNbPanel, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsRegisteredSumValueText->Wrap( -1 );
	sfsSizer->Add( sfsRegisteredSumValueText, 0, wxALL, 5 );
	
	
	sfsSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sfsSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	sfsCurrentSumLabelText = new wxStaticText( sfsNbPanel, wxID_ANY, _("Checksum (current):"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsCurrentSumLabelText->Wrap( -1 );
	sfsSizer->Add( sfsCurrentSumLabelText, 0, wxALL, 5 );
	
	sfsCurrentSumValueText = new wxStaticText( sfsNbPanel, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsCurrentSumValueText->Wrap( -1 );
	sfsSizer->Add( sfsCurrentSumValueText, 0, wxALL, 5 );
	
	
	sfsSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sfsSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	sfsStatusLabelText = new wxStaticText( sfsNbPanel, wxID_ANY, _("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsStatusLabelText->Wrap( -1 );
	sfsSizer->Add( sfsStatusLabelText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sfsStatusValueText = new wxStaticText( sfsNbPanel, wxID_ANY, _("mismatch"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsStatusValueText->Wrap( -1 );
	sfsSizer->Add( sfsStatusValueText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sfsValidateChkSumButton = new wxButton( sfsNbPanel, wxID_ANY, _("validate"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsSizer->Add( sfsValidateChkSumButton, 0, wxALL, 5 );
	
	sfsUpdateChkSumButton = new wxButton( sfsNbPanel, wxID_ANY, _("update"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsSizer->Add( sfsUpdateChkSumButton, 0, wxALL, 5 );
	
	sfsNbPanel->SetSizer( sfsSizer );
	sfsNbPanel->Layout();
	sfsSizer->Fit( sfsNbPanel );
	ruleEditNotebook->AddPage( sfsNbPanel, _("SFS"), true );
	macroNbPanel = new wxScrolledWindow( ruleEditNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	macroNbPanel->SetScrollRate( 5, 5 );
	wxFlexGridSizer* macroSizer;
	macroSizer = new wxFlexGridSizer( 3, 2, 0, 0 );
	macroSizer->SetFlexibleDirection( wxBOTH );
	macroSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	macroTypeLabelText = new wxStaticText( macroNbPanel, wxID_ANY, _("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	macroTypeLabelText->Wrap( -1 );
	macroSizer->Add( macroTypeLabelText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString macroTypeChoiceChoices[] = { _("VAR_APPLICATION"), _("VAR_RULE"), _("VAR_DEFAULT"), _("VAR_HOST"), _("VAR_PORT"), _("VAR_FILENAME") };
	int macroTypeChoiceNChoices = sizeof( macroTypeChoiceChoices ) / sizeof( wxString );
	macroTypeChoice = new wxChoice( macroNbPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, macroTypeChoiceNChoices, macroTypeChoiceChoices, 0 );
	macroSizer->Add( macroTypeChoice, 0, wxALL, 5 );
	
	macroValueLabelText = new wxStaticText( macroNbPanel, wxID_ANY, _("Value:"), wxDefaultPosition, wxDefaultSize, 0 );
	macroValueLabelText->Wrap( -1 );
	macroSizer->Add( macroValueLabelText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	macroValueextCtrl = new wxTextCtrl( macroNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	macroSizer->Add( macroValueextCtrl, 0, wxALL|wxEXPAND, 5 );
	
	macroNbPanel->SetSizer( macroSizer );
	macroNbPanel->Layout();
	macroSizer->Fit( macroNbPanel );
	ruleEditNotebook->AddPage( macroNbPanel, _("Macro"), false );
	
	sz_main->Add( ruleEditNotebook, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( sz_main );
	this->Layout();
	sz_main->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DlgRuleEditorBase::OnClose ) );
	controlRuleCreateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnRuleCreateButton ), NULL, this );
	controlRuleDeleteButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnRuleDeleteButton ), NULL, this );
	controlRuleSetSaveButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnRuleSetSave ), NULL, this );
	controlOptionButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnTableOptionButtonClick ), NULL, this );
	ruleListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( DlgRuleEditorBase::OnLineSelected ), NULL, this );
	commonHighProfileRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnCommonHighProfileButton ), NULL, this );
	commonMediumProfileRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnCommonMediumProfileButton ), NULL, this );
	commonAdminProfileRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnCommonAdminProfileButton ), NULL, this );
	commonNoneLogRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnCommonLogNone ), NULL, this );
	commonDoLogRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnCommonLogLog ), NULL, this );
	commonAlertLogRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnCommonLogAlert ), NULL, this );
	appNameComboBox->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAppNameComboBox ), NULL, this );
	appNameComboBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DlgRuleEditorBase::OnAppNameComboBox ), NULL, this );
	appBinaryTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorBase::OnAppBinaryTextCtrl ), NULL, this );
	appBinaryModifyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppBinaryModifyButton ), NULL, this );
	appValidateChkSumButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppValidateChkSumButton ), NULL, this );
	appUpdateChkSumButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnAppUpdateChkSumButton ), NULL, this );
	alfAllowRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfAllowRadioButton ), NULL, this );
	alfDenyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfDenyRadioButton ), NULL, this );
	alfAskRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfAskRadioButton ), NULL, this );
	alfFilterRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfFilterRadioButton ), NULL, this );
	alfCapRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfCapRadioButton ), NULL, this );
	alfDefaultRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfDefaultRadioButton ), NULL, this );
	alfTcpRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfTcpRadioButton ), NULL, this );
	alfUdpRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfUdpRadioButton ), NULL, this );
	alfInetRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfInetRadioButton ), NULL, this );
	alfInet6RadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfInet6RadioButton ), NULL, this );
	alfAnyRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfAnyRadioButton ), NULL, this );
	alfRawCapRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfRawCapRadioButton ), NULL, this );
	alfOtherCapRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfOtherCapRadioButton ), NULL, this );
	alfAllCapRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfAllCapRadioButton ), NULL, this );
	alfAcceptRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfAcceptRadioButton ), NULL, this );
	alfConnectRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfConnectRadioButton ), NULL, this );
	alfBothRadioButton->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfBothRadioButton ), NULL, this );
	alfSrcAddrComboBox->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfSrcAddrComboBox ), NULL, this );
	alfSrcAddrComboBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfSrcAddrComboBox ), NULL, this );
	alfSrcAddrNetSpinCtrl->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( DlgRuleEditorBase::OnAlfSrcNetmaskSpinCtrl ), NULL, this );
	alfSrcAddrNetSpinCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfSrcNetmaskSpinCtrlText ), NULL, this );
	alfSrcAddrAddButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnSrcAddrAddButton ), NULL, this );
	alfDstAddrComboBox->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfDstAddrComboBox ), NULL, this );
	alfDstAddrComboBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfDstAddrComboBox ), NULL, this );
	alfDstAddrNetSpinCtrl->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( DlgRuleEditorBase::OnAlfDstNetmaskSpinCtrl ), NULL, this );
	alfDstAddrNetSpinCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfDstNetmaskSpinCtrlText ), NULL, this );
	alfDstAddrAddButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnDstAddrAddButton ), NULL, this );
	alfSrcPortComboBox->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfSrcPortComboBox ), NULL, this );
	alfSrcPortComboBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfSrcPortComboBox ), NULL, this );
	alfDstPortComboBox->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfDstPortComboBox ), NULL, this );
	alfDstPortComboBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DlgRuleEditorBase::OnAlfDstPortComboBox ), NULL, this );
	alfStateTimeoutSpinCtrl->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( DlgRuleEditorBase::OnAlfStateTimeoutChange ), NULL, this );
	sfsBinaryTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( DlgRuleEditorBase::OnSfsBinaryTextCtrl ), NULL, this );
	sfsBinaryModifyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnSfsBinaryModifyButton ), NULL, this );
	sfsValidateChkSumButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnSfsValidateChkSumButton ), NULL, this );
	sfsUpdateChkSumButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnSfsUpdateChkSumButton ), NULL, this );
}
