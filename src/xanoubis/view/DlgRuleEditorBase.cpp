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
	
	wxBoxSizer* controlRuleSizer;
	controlRuleSizer = new wxBoxSizer( wxHORIZONTAL );
	
	controlRuleText = new wxStaticText( this, wxID_ANY, wxT("Rule:"), wxDefaultPosition, wxDefaultSize, 0 );
	controlRuleText->Wrap( -1 );
	controlRuleText->SetMinSize( wxSize( 100,-1 ) );
	
	controlRuleSizer->Add( controlRuleText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlRuleCreateButton = new wxButton( this, wxID_ANY, wxT("create"), wxDefaultPosition, wxDefaultSize, 0 );
	controlRuleSizer->Add( controlRuleCreateButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlRuleDeleteButton = new wxButton( this, wxID_ANY, wxT("delete"), wxDefaultPosition, wxDefaultSize, 0 );
	controlRuleSizer->Add( controlRuleDeleteButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	controlRuleSizer->Add( 1, 0, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlFilterText = new wxStaticText( this, wxID_ANY, wxT("Search:"), wxDefaultPosition, wxDefaultSize, 0 );
	controlFilterText->Wrap( -1 );
	controlRuleSizer->Add( controlFilterText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlFilterTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_NO_VSCROLL );
	controlRuleSizer->Add( controlFilterTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlFilterInText = new wxStaticText( this, wxID_ANY, wxT("in"), wxDefaultPosition, wxDefaultSize, 0 );
	controlFilterInText->Wrap( -1 );
	controlRuleSizer->Add( controlFilterInText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString controlFilterChoiceChoices[] = { wxT("all"), wxT("Application"), wxT("Ip") };
	int controlFilterChoiceNChoices = sizeof( controlFilterChoiceChoices ) / sizeof( wxString );
	controlFilterChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, controlFilterChoiceNChoices, controlFilterChoiceChoices, 0 );
	controlRuleSizer->Add( controlFilterChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sz_main->Add( controlRuleSizer, 0, wxEXPAND, 5 );
	
	wxBoxSizer* controlRuleSetSizer;
	controlRuleSetSizer = new wxBoxSizer( wxHORIZONTAL );
	
	controlRuleSetText = new wxStaticText( this, wxID_ANY, wxT("Ruleset:"), wxDefaultPosition, wxDefaultSize, 0 );
	controlRuleSetText->Wrap( -1 );
	controlRuleSetText->SetMinSize( wxSize( 100,-1 ) );
	
	controlRuleSetSizer->Add( controlRuleSetText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlRuleSetSaveButton = new wxButton( this, wxID_ANY, wxT("store"), wxDefaultPosition, wxDefaultSize, 0 );
	controlRuleSetSizer->Add( controlRuleSetSaveButton, 0, wxALL, 5 );
	
	
	controlRuleSetSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	controlOptionText = new wxStaticText( this, wxID_ANY, wxT("Table:"), wxDefaultPosition, wxDefaultSize, 0 );
	controlOptionText->Wrap( -1 );
	controlRuleSetSizer->Add( controlOptionText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	controlOptionButton = new wxButton( this, wxID_ANY, wxT("Options..."), wxDefaultPosition, wxDefaultSize, 0 );
	controlRuleSetSizer->Add( controlOptionButton, 0, wxALL, 5 );
	
	sz_main->Add( controlRuleSetSizer, 0, wxEXPAND, 5 );
	
	ruleListCtrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT );
	sz_main->Add( ruleListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	ruleEditNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0|wxVSCROLL );
	commonNbPanel = new wxScrolledWindow( ruleEditNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	commonNbPanel->SetScrollRate( 5, 5 );
	wxBoxSizer* commonMainSizer;
	commonMainSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* commonRuleBox;
	commonRuleBox = new wxStaticBoxSizer( new wxStaticBox( commonNbPanel, -1, wxT("Rule") ), wxVERTICAL );
	
	wxFlexGridSizer* commonRuleSizer;
	commonRuleSizer = new wxFlexGridSizer( 5, 2, 0, 0 );
	commonRuleSizer->SetFlexibleDirection( wxBOTH );
	commonRuleSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	commonModuleText = new wxStaticText( commonNbPanel, wxID_ANY, wxT("Module:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonModuleText->Wrap( -1 );
	commonRuleSizer->Add( commonModuleText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString commonModuleChoiceChoices[] = { wxT("ALF"), wxT("SFS"), wxT("Macro") };
	int commonModuleChoiceNChoices = sizeof( commonModuleChoiceChoices ) / sizeof( wxString );
	commonModuleChoice = new wxChoice( commonNbPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, commonModuleChoiceNChoices, commonModuleChoiceChoices, 0 );
	commonRuleSizer->Add( commonModuleChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonStateText = new wxStaticText( commonNbPanel, wxID_ANY, wxT("State:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonStateText->Wrap( -1 );
	commonRuleSizer->Add( commonStateText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* commonStateSizer;
	commonStateSizer = new wxBoxSizer( wxHORIZONTAL );
	
	commonActiveRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, wxT("activated"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	commonStateSizer->Add( commonActiveRadioButton, 0, wxALL, 5 );
	
	commonDeactiveRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, wxT("deactivated"), wxDefaultPosition, wxDefaultSize, 0 );
	commonStateSizer->Add( commonDeactiveRadioButton, 0, wxALL, 5 );
	
	commonRuleSizer->Add( commonStateSizer, 1, wxEXPAND, 5 );
	
	commonNameText = new wxStaticText( commonNbPanel, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonNameText->Wrap( -1 );
	commonRuleSizer->Add( commonNameText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonNameTextCtrl = new wxTextCtrl( commonNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	commonRuleSizer->Add( commonNameTextCtrl, 0, wxALL|wxEXPAND, 5 );
	
	commonPriorityText = new wxStaticText( commonNbPanel, wxID_ANY, wxT("Priority:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonPriorityText->Wrap( -1 );
	commonRuleSizer->Add( commonPriorityText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonPrioritySpinCtrl = new wxSpinCtrl( commonNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	commonRuleSizer->Add( commonPrioritySpinCtrl, 0, wxALL, 5 );
	
	commonFaderText = new wxStaticText( commonNbPanel, wxID_ANY, wxT("Profile:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonFaderText->Wrap( -1 );
	commonRuleSizer->Add( commonFaderText, 0, wxALL, 5 );
	
	commonFader = new AnFader(commonNbPanel);
	commonRuleSizer->Add( commonFader, 0, wxALL, 5 );
	
	commonCommentText = new wxStaticText( commonNbPanel, wxID_ANY, wxT("Comment:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonCommentText->Wrap( -1 );
	commonRuleSizer->Add( commonCommentText, 0, wxALL, 5 );
	
	commonCommentTextCtrl = new wxTextCtrl( commonNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	commonRuleSizer->Add( commonCommentTextCtrl, 1, wxALL|wxEXPAND, 5 );
	
	commonRuleBox->Add( commonRuleSizer, 1, wxEXPAND, 5 );
	
	commonMainSizer->Add( commonRuleBox, 1, wxEXPAND, 5 );
	
	wxBoxSizer* commonRightSideSizer;
	commonRightSideSizer = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* commonTimeBox;
	commonTimeBox = new wxStaticBoxSizer( new wxStaticBox( commonNbPanel, -1, wxT("Duration") ), wxVERTICAL );
	
	wxBoxSizer* commonCountSizer;
	commonCountSizer = new wxBoxSizer( wxHORIZONTAL );
	
	commonCountRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, wxT("hitcount"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	commonCountRadioButton->SetMinSize( wxSize( 100,-1 ) );
	
	commonCountSizer->Add( commonCountRadioButton, 0, wxALIGN_CENTER|wxALL, 1 );
	
	commonCountSpinCtrl = new wxSpinCtrl( commonNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxSP_WRAP, 1, 999, 1 );
	commonCountSpinCtrl->SetMinSize( wxSize( 50,-1 ) );
	
	commonCountSizer->Add( commonCountSpinCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	
	commonTimeBox->Add( commonCountSizer, 0, 0, 5 );
	
	commonProcEndRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, wxT("until end of process"), wxDefaultPosition, wxDefaultSize, 0 );
	commonTimeBox->Add( commonProcEndRadioButton, 0, wxALL, 1 );
	
	wxBoxSizer* commonTimeSizer;
	commonTimeSizer = new wxBoxSizer( wxHORIZONTAL );
	
	commonTimeRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, wxT("duration"), wxDefaultPosition, wxDefaultSize, 0 );
	commonTimeRadioButton->SetMinSize( wxSize( 100,-1 ) );
	
	commonTimeSizer->Add( commonTimeRadioButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1 );
	
	commonTimeSpinCtrl = new wxSpinCtrl( commonNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxSP_WRAP, 1, 999, 1 );
	commonTimeSpinCtrl->SetMinSize( wxSize( 50,-1 ) );
	
	commonTimeSizer->Add( commonTimeSpinCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	
	wxString commonTimeUnitChoiceChoices[] = { wxT("second"), wxT("minute"), wxT("hour"), wxT("day") };
	int commonTimeUnitChoiceNChoices = sizeof( commonTimeUnitChoiceChoices ) / sizeof( wxString );
	commonTimeUnitChoice = new wxChoice( commonNbPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, commonTimeUnitChoiceNChoices, commonTimeUnitChoiceChoices, 0 );
	commonTimeSizer->Add( commonTimeUnitChoice, 0, wxALIGN_CENTER|wxALL, 1 );
	
	commonTimeBox->Add( commonTimeSizer, 0, 0, 1 );
	
	commonAlwaysRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, wxT("always"), wxDefaultPosition, wxDefaultSize, 0 );
	commonTimeBox->Add( commonAlwaysRadioButton, 0, wxALL, 1 );
	
	commonRightSideSizer->Add( commonTimeBox, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* logBox;
	logBox = new wxStaticBoxSizer( new wxStaticBox( commonNbPanel, -1, wxT("Logging") ), wxVERTICAL );
	
	commonNoneLogRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, wxT("none"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	logBox->Add( commonNoneLogRadioButton, 0, wxALL, 5 );
	
	commonDoLogRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, wxT("log"), wxDefaultPosition, wxDefaultSize, 0 );
	logBox->Add( commonDoLogRadioButton, 0, wxALL, 5 );
	
	commonAlertLogRadioButton = new wxRadioButton( commonNbPanel, wxID_ANY, wxT("alert"), wxDefaultPosition, wxDefaultSize, 0 );
	logBox->Add( commonAlertLogRadioButton, 0, wxALL, 5 );
	
	commonRightSideSizer->Add( logBox, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* commonModifyBox;
	commonModifyBox = new wxStaticBoxSizer( new wxStaticBox( commonNbPanel, -1, wxT("Modification") ), wxVERTICAL );
	
	wxFlexGridSizer* commonModifySizer;
	commonModifySizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	commonModifySizer->SetFlexibleDirection( wxBOTH );
	commonModifySizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	commonCreatedText = new wxStaticText( commonNbPanel, wxID_ANY, wxT("created:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonCreatedText->Wrap( -1 );
	commonModifySizer->Add( commonCreatedText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonCreateTextValue = new wxStaticText( commonNbPanel, wxID_ANY, wxT("Fri Mar  7 14:33:02 CET 2008"), wxDefaultPosition, wxDefaultSize, 0 );
	commonCreateTextValue->Wrap( -1 );
	commonModifySizer->Add( commonCreateTextValue, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonModifiedText = new wxStaticText( commonNbPanel, wxID_ANY, wxT("modified:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonModifiedText->Wrap( -1 );
	commonModifySizer->Add( commonModifiedText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonModifiedTextValue = new wxStaticText( commonNbPanel, wxID_ANY, wxT("Fri Mar  8 14:33:02 CET 2008"), wxDefaultPosition, wxDefaultSize, 0 );
	commonModifiedTextValue->Wrap( -1 );
	commonModifySizer->Add( commonModifiedTextValue, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonModificatorText = new wxStaticText( commonNbPanel, wxID_ANY, wxT("modified by:"), wxDefaultPosition, wxDefaultSize, 0 );
	commonModificatorText->Wrap( -1 );
	commonModifySizer->Add( commonModificatorText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonModificatorTextValue = new wxStaticText( commonNbPanel, wxID_ANY, wxT("trahm"), wxDefaultPosition, wxDefaultSize, 0 );
	commonModificatorTextValue->Wrap( -1 );
	commonModifySizer->Add( commonModificatorTextValue, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	commonModifyBox->Add( commonModifySizer, 0, wxEXPAND, 5 );
	
	commonRightSideSizer->Add( commonModifyBox, 1, wxEXPAND, 5 );
	
	commonMainSizer->Add( commonRightSideSizer, 1, wxEXPAND, 5 );
	
	commonNbPanel->SetSizer( commonMainSizer );
	commonNbPanel->Layout();
	commonMainSizer->Fit( commonNbPanel );
	ruleEditNotebook->AddPage( commonNbPanel, wxT("Common"), false );
	applicationNbPanel = new wxScrolledWindow( ruleEditNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	applicationNbPanel->SetScrollRate( 5, 5 );
	wxFlexGridSizer* appMainPanelSizer;
	appMainPanelSizer = new wxFlexGridSizer( 2, 5, 0, 0 );
	appMainPanelSizer->SetFlexibleDirection( wxBOTH );
	appMainPanelSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	appNameText = new wxStaticText( applicationNbPanel, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	appNameText->Wrap( -1 );
	appMainPanelSizer->Add( appNameText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appNameComboBox = new wxComboBox( applicationNbPanel, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	appMainPanelSizer->Add( appNameComboBox, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	appBinaryText = new wxStaticText( applicationNbPanel, wxID_ANY, wxT("Binaries:"), wxDefaultPosition, wxDefaultSize, 0 );
	appBinaryText->Wrap( -1 );
	appMainPanelSizer->Add( appBinaryText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* appGuessSizer;
	appGuessSizer = new wxBoxSizer( wxHORIZONTAL );
	
	appGuessButton = new wxButton( applicationNbPanel, wxID_ANY, wxT("guess ..."), wxDefaultPosition, wxDefaultSize, 0 );
	appGuessSizer->Add( appGuessButton, 0, wxALL, 5 );
	
	appGuessText = new wxStaticText( applicationNbPanel, wxID_ANY, wxT("(by application)"), wxDefaultPosition, wxDefaultSize, 0 );
	appGuessText->Wrap( -1 );
	appGuessSizer->Add( appGuessText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appMainPanelSizer->Add( appGuessSizer, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	appBinaryTextCtrl = new wxTextCtrl( applicationNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	appMainPanelSizer->Add( appBinaryTextCtrl, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	appBinaryModifyButton = new wxButton( applicationNbPanel, wxID_ANY, wxT("modify"), wxDefaultPosition, wxDefaultSize, 0 );
	appMainPanelSizer->Add( appBinaryModifyButton, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	appMainPanelSizer->Add( 35, 0, 1, wxEXPAND, 5 );
	
	appBinaryAddButton = new wxButton( applicationNbPanel, wxID_ANY, wxT("+"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appMainPanelSizer->Add( appBinaryAddButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appInheritanceText = new wxStaticText( applicationNbPanel, wxID_ANY, wxT("Inheritance:"), wxDefaultPosition, wxDefaultSize, 0 );
	appInheritanceText->Wrap( -1 );
	appMainPanelSizer->Add( appInheritanceText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	appInheritanceTextCtrl = new wxTextCtrl( applicationNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	appMainPanelSizer->Add( appInheritanceTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	appInheritanceModifyButton = new wxButton( applicationNbPanel, wxID_ANY, wxT("modify"), wxDefaultPosition, wxDefaultSize, 0 );
	appMainPanelSizer->Add( appInheritanceModifyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	
	appMainPanelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	appInheritanceAddButton = new wxButton( applicationNbPanel, wxID_ANY, wxT("+"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	appMainPanelSizer->Add( appInheritanceAddButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	applicationNbPanel->SetSizer( appMainPanelSizer );
	applicationNbPanel->Layout();
	appMainPanelSizer->Fit( applicationNbPanel );
	ruleEditNotebook->AddPage( applicationNbPanel, wxT("Application"), false );
	alfNbPanel = new wxScrolledWindow( ruleEditNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	alfNbPanel->SetScrollRate( 5, 5 );
	wxBoxSizer* alfPanelMainSizer;
	alfPanelMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* alfOptionSizer;
	alfOptionSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
	alfOptionSizer->SetFlexibleDirection( wxBOTH );
	alfOptionSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	alfActionText = new wxStaticText( alfNbPanel, wxID_ANY, wxT("Action:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfActionText->Wrap( -1 );
	alfOptionSizer->Add( alfActionText, 0, wxALL, 5 );
	
	alfAllowRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, wxT("allow"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	alfOptionSizer->Add( alfAllowRadioButton, 0, wxALL, 5 );
	
	alfDenyRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, wxT("deny"), wxDefaultPosition, wxDefaultSize, 0 );
	alfOptionSizer->Add( alfDenyRadioButton, 0, wxALL, 5 );
	
	alfAskRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, wxT("ask"), wxDefaultPosition, wxDefaultSize, 0 );
	alfOptionSizer->Add( alfAskRadioButton, 0, wxALL, 5 );
	
	alfTypeText = new wxStaticText( alfNbPanel, wxID_ANY, wxT("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfTypeText->Wrap( -1 );
	alfOptionSizer->Add( alfTypeText, 0, wxALL, 5 );
	
	alfFilterRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, wxT("filter"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	alfOptionSizer->Add( alfFilterRadioButton, 0, wxALL, 5 );
	
	alfCapRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, wxT("capability"), wxDefaultPosition, wxDefaultSize, 0 );
	alfOptionSizer->Add( alfCapRadioButton, 0, wxALL, 5 );
	
	alfDefaultRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, wxT("default"), wxDefaultPosition, wxDefaultSize, 0 );
	alfOptionSizer->Add( alfDefaultRadioButton, 0, wxALL, 5 );
	
	alfPanelMainSizer->Add( alfOptionSizer, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* alfConnectionBox;
	alfConnectionBox = new wxStaticBoxSizer( new wxStaticBox( alfNbPanel, -1, wxT("Connection") ), wxVERTICAL );
	
	wxFlexGridSizer* alfConnectionSizer;
	alfConnectionSizer = new wxFlexGridSizer( 2, 6, 0, 0 );
	alfConnectionSizer->SetFlexibleDirection( wxBOTH );
	alfConnectionSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	alfProtocolText = new wxStaticText( alfNbPanel, wxID_ANY, wxT("Protocol:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfProtocolText->Wrap( -1 );
	alfConnectionSizer->Add( alfProtocolText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* alfProtocolSizer;
	alfProtocolSizer = new wxBoxSizer( wxHORIZONTAL );
	
	alfTcpCheckBox = new wxCheckBox( alfNbPanel, wxID_ANY, wxT("tcp"), wxDefaultPosition, wxDefaultSize, 0 );
	alfTcpCheckBox->SetValue(true);
	
	alfProtocolSizer->Add( alfTcpCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfUdpCheckBox = new wxCheckBox( alfNbPanel, wxID_ANY, wxT("udp"), wxDefaultPosition, wxDefaultSize, 0 );
	
	alfProtocolSizer->Add( alfUdpCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfConnectionSizer->Add( alfProtocolSizer, 1, wxEXPAND, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	alfSrcAddrText = new wxStaticText( alfNbPanel, wxID_ANY, wxT("Source address:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfSrcAddrText->Wrap( -1 );
	alfConnectionSizer->Add( alfSrcAddrText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcAddrComboBox = new wxComboBox( alfNbPanel, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	alfConnectionSizer->Add( alfSrcAddrComboBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcAddrDelimiterText = new wxStaticText( alfNbPanel, wxID_ANY, wxT(" / "), wxDefaultPosition, wxDefaultSize, 0 );
	alfSrcAddrDelimiterText->Wrap( -1 );
	alfConnectionSizer->Add( alfSrcAddrDelimiterText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcAddrNetSpinCtrl = new wxSpinCtrl( alfNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	alfConnectionSizer->Add( alfSrcAddrNetSpinCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	alfConnectionSizer->Add( 35, 0, 1, wxEXPAND, 5 );
	
	alfSrcAddrAddButton = new wxButton( alfNbPanel, wxID_ANY, wxT("+"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	alfConnectionSizer->Add( alfSrcAddrAddButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfDstAddrText = new wxStaticText( alfNbPanel, wxID_ANY, wxT("Destination address:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfDstAddrText->Wrap( -1 );
	alfConnectionSizer->Add( alfDstAddrText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfDstAddrComboBox = new wxComboBox( alfNbPanel, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	alfConnectionSizer->Add( alfDstAddrComboBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfDstAddrDelimiterText = new wxStaticText( alfNbPanel, wxID_ANY, wxT(" / "), wxDefaultPosition, wxDefaultSize, 0 );
	alfDstAddrDelimiterText->Wrap( -1 );
	alfConnectionSizer->Add( alfDstAddrDelimiterText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfDstAddrNetSpinCtrl = new wxSpinCtrl( alfNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	alfConnectionSizer->Add( alfDstAddrNetSpinCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	alfDstAddrAddButton = new wxButton( alfNbPanel, wxID_ANY, wxT("+"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	alfConnectionSizer->Add( alfDstAddrAddButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcPortText = new wxStaticText( alfNbPanel, wxID_ANY, wxT("Source Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfSrcPortText->Wrap( -1 );
	alfConnectionSizer->Add( alfSrcPortText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfSrcPortComboBox = new wxComboBox( alfNbPanel, wxID_ANY, wxT("53"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	alfSrcPortComboBox->Append( wxT("80") );
	alfSrcPortComboBox->Append( wxT("443") );
	alfSrcPortComboBox->Append( wxT("$www") );
	alfSrcPortComboBox->Append( wxT("21,22") );
	alfConnectionSizer->Add( alfSrcPortComboBox, 0, wxALL, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	alfDstPortText = new wxStaticText( alfNbPanel, wxID_ANY, wxT("Destination Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfDstPortText->Wrap( -1 );
	alfConnectionSizer->Add( alfDstPortText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	alfDstPortComboBox = new wxComboBox( alfNbPanel, wxID_ANY, wxT("53"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	alfDstPortComboBox->Append( wxT("80") );
	alfDstPortComboBox->Append( wxT("443") );
	alfDstPortComboBox->Append( wxT("$www") );
	alfDstPortComboBox->Append( wxT("21,22") );
	alfConnectionSizer->Add( alfDstPortComboBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	alfConnectionSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	alfAddrFamilyText = new wxStaticText( alfNbPanel, wxID_ANY, wxT("Address family:"), wxDefaultPosition, wxDefaultSize, 0 );
	alfAddrFamilyText->Wrap( -1 );
	alfConnectionSizer->Add( alfAddrFamilyText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* alfAddrFamilySizer;
	alfAddrFamilySizer = new wxBoxSizer( wxHORIZONTAL );
	
	alfInetRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, wxT("inet"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	alfAddrFamilySizer->Add( alfInetRadioButton, 0, wxALL, 5 );
	
	alfInet6RadionButton = new wxRadioButton( alfNbPanel, wxID_ANY, wxT("inet6"), wxDefaultPosition, wxDefaultSize, 0 );
	alfAddrFamilySizer->Add( alfInet6RadionButton, 0, wxALL, 5 );
	
	alfAnyRadioButton = new wxRadioButton( alfNbPanel, wxID_ANY, wxT("any"), wxDefaultPosition, wxDefaultSize, 0 );
	alfAddrFamilySizer->Add( alfAnyRadioButton, 0, wxALL, 5 );
	
	alfConnectionSizer->Add( alfAddrFamilySizer, 1, wxEXPAND, 5 );
	
	alfConnectionBox->Add( alfConnectionSizer, 1, wxEXPAND, 5 );
	
	alfPanelMainSizer->Add( alfConnectionBox, 0, 0, 5 );
	
	alfNbPanel->SetSizer( alfPanelMainSizer );
	alfNbPanel->Layout();
	alfPanelMainSizer->Fit( alfNbPanel );
	ruleEditNotebook->AddPage( alfNbPanel, wxT("ALF"), true );
	sfsNbPanel = new wxScrolledWindow( ruleEditNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	sfsNbPanel->SetScrollRate( 5, 5 );
	wxFlexGridSizer* sfsSizer;
	sfsSizer = new wxFlexGridSizer( 3, 3, 0, 0 );
	sfsSizer->SetFlexibleDirection( wxBOTH );
	sfsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	sfsBinaryLabelText = new wxStaticText( sfsNbPanel, wxID_ANY, wxT("Binary:"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsBinaryLabelText->Wrap( -1 );
	sfsSizer->Add( sfsBinaryLabelText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sfsBinaryTextCtrl = new wxTextCtrl( sfsNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	sfsSizer->Add( sfsBinaryTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sfsBinaryModifyButton = new wxButton( sfsNbPanel, wxID_ANY, wxT("modify"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsSizer->Add( sfsBinaryModifyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sfsRegisteredSumLabelText = new wxStaticText( sfsNbPanel, wxID_ANY, wxT("Checksum (registered):"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsRegisteredSumLabelText->Wrap( -1 );
	sfsSizer->Add( sfsRegisteredSumLabelText, 0, wxALL, 5 );
	
	sfsRegisteredSumValueText = new wxStaticText( sfsNbPanel, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsRegisteredSumValueText->Wrap( -1 );
	sfsSizer->Add( sfsRegisteredSumValueText, 0, wxALL, 5 );
	
	
	sfsSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	sfsCurrentSumLabelText = new wxStaticText( sfsNbPanel, wxID_ANY, wxT("Checksum (current):"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsCurrentSumLabelText->Wrap( -1 );
	sfsSizer->Add( sfsCurrentSumLabelText, 0, wxALL, 5 );
	
	sfsCurrentSumValueText = new wxStaticText( sfsNbPanel, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsCurrentSumValueText->Wrap( -1 );
	sfsSizer->Add( sfsCurrentSumValueText, 0, wxALL, 5 );
	
	
	sfsSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	sfsStatusLabelText = new wxStaticText( sfsNbPanel, wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsStatusLabelText->Wrap( -1 );
	sfsSizer->Add( sfsStatusLabelText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sfsStatusValueText = new wxStaticText( sfsNbPanel, wxID_ANY, wxT("mismatch"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsStatusValueText->Wrap( -1 );
	sfsSizer->Add( sfsStatusValueText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sfsUpdateChkSumButton = new wxButton( sfsNbPanel, wxID_ANY, wxT("update"), wxDefaultPosition, wxDefaultSize, 0 );
	sfsSizer->Add( sfsUpdateChkSumButton, 0, wxALL, 5 );
	
	sfsNbPanel->SetSizer( sfsSizer );
	sfsNbPanel->Layout();
	sfsSizer->Fit( sfsNbPanel );
	ruleEditNotebook->AddPage( sfsNbPanel, wxT("SFS"), false );
	macroNbPanel = new wxScrolledWindow( ruleEditNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	macroNbPanel->SetScrollRate( 5, 5 );
	wxFlexGridSizer* macroSizer;
	macroSizer = new wxFlexGridSizer( 3, 2, 0, 0 );
	macroSizer->SetFlexibleDirection( wxBOTH );
	macroSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	macroTypeLabelText = new wxStaticText( macroNbPanel, wxID_ANY, wxT("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	macroTypeLabelText->Wrap( -1 );
	macroSizer->Add( macroTypeLabelText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString macroTypeChoiceChoices[] = { wxT("VAR_APPLICATION"), wxT("VAR_RULE"), wxT("VAR_DEFAULT"), wxT("VAR_HOST"), wxT("VAR_PORT"), wxT("VAR_FILENAME") };
	int macroTypeChoiceNChoices = sizeof( macroTypeChoiceChoices ) / sizeof( wxString );
	macroTypeChoice = new wxChoice( macroNbPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, macroTypeChoiceNChoices, macroTypeChoiceChoices, 0 );
	macroSizer->Add( macroTypeChoice, 0, wxALL, 5 );
	
	macroValueLabelText = new wxStaticText( macroNbPanel, wxID_ANY, wxT("Value:"), wxDefaultPosition, wxDefaultSize, 0 );
	macroValueLabelText->Wrap( -1 );
	macroSizer->Add( macroValueLabelText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	macroValueextCtrl = new wxTextCtrl( macroNbPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	macroSizer->Add( macroValueextCtrl, 0, wxALL|wxEXPAND, 5 );
	
	macroNbPanel->SetSizer( macroSizer );
	macroNbPanel->Layout();
	macroSizer->Fit( macroNbPanel );
	ruleEditNotebook->AddPage( macroNbPanel, wxT("Macro"), false );
	
	sz_main->Add( ruleEditNotebook, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( sz_main );
	this->Layout();
	sz_main->Fit( this );
	
	// Connect Events
	controlOptionButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnTableOptionButtonClick ), NULL, this );
	appBinaryModifyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnBinaryModifyButtonClick ), NULL, this );
	sfsBinaryModifyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgRuleEditorBase::OnBinaryModifyButtonClick ), NULL, this );
}
