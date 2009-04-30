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
#include "ModSfsListCtrl.h"

#include "ModSfsPanelsBase.h"

///////////////////////////////////////////////////////////////////////////

ModSfsMainPanelBase::ModSfsMainPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	sz_MainSFSMain = new wxBoxSizer( wxVERTICAL );
	
	tx_MainHeadline = new wxStaticText( this, wxID_ANY, _("Main Panel of Module SFS"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_MainHeadline->Wrap( -1 );
	tx_MainHeadline->SetFont( wxFont( 16, 70, 90, 90, false, wxEmptyString ) );
	
	sz_MainSFSMain->Add( tx_MainHeadline, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	note_MainSfs = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	pan_Rules = new wxPanel( note_MainSfs, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_SfsRules;
	sz_SfsRules = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* sz_RulesNS;
	sz_RulesNS = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* sz_RulesWE;
	sz_RulesWE = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* sz_Rules;
	sz_Rules = new wxBoxSizer( wxVERTICAL );
	
	lst_Rules = new wxListCtrl( pan_Rules, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL );
	sz_Rules->Add( lst_Rules, 1, wxALL|wxEXPAND, 5 );
	
	sz_RulesWE->Add( sz_Rules, 66, wxEXPAND, 5 );
	
	sz_RulesNS->Add( sz_RulesWE, 1, wxEXPAND, 5 );
	
	sz_SfsRules->Add( sz_RulesNS, 1, wxEXPAND, 5 );
	
	pan_Rules->SetSizer( sz_SfsRules );
	pan_Rules->Layout();
	sz_SfsRules->Fit( pan_Rules );
	note_MainSfs->AddPage( pan_Rules, _("Rules"), true );
	pan_SfsMain = new wxPanel( note_MainSfs, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText12 = new wxStaticText( pan_SfsMain, wxID_ANY, _("Directory:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	bSizer11->Add( m_staticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SfsMainCurrPathLabel = new wxStaticText( pan_SfsMain, wxID_ANY, _("/usr/local/bin/"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainCurrPathLabel->Wrap( -1 );
	bSizer11->Add( SfsMainCurrPathLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer12->Add( bSizer11, 0, wxEXPAND, 5 );
	
	SfsMainDirCtrl = new wxGenericDirCtrl( pan_SfsMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDIRCTRL_DIR_ONLY|wxDIRCTRL_SHOW_FILTERS|wxSUNKEN_BORDER, wxEmptyString, 0 );
	
	SfsMainDirCtrl->ShowHidden( false );
	SfsMainDirCtrl->SetMinSize( wxSize( 200,-1 ) );
	
	bSizer12->Add( SfsMainDirCtrl, 1, wxEXPAND | wxALL, 5 );
	
	SfsMainDirTraversalCheckbox = new wxCheckBox( pan_SfsMain, wxID_ANY, _("Recursive traversal"), wxDefaultPosition, wxDefaultSize, 0 );
	
	SfsMainDirTraversalCheckbox->SetToolTip( _("Enables recursive traversal through filesystem") );
	
	bSizer12->Add( SfsMainDirTraversalCheckbox, 0, wxALL, 5 );
	
	bSizer10->Add( bSizer12, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText10 = new wxStaticText( pan_SfsMain, wxID_ANY, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	bSizer15->Add( m_staticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SfsMainFilterTextCtrl = new wxTextCtrl( pan_SfsMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	bSizer15->Add( SfsMainFilterTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SfsMainFilterButton = new wxButton( pan_SfsMain, wxID_ANY, _("Filter"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainFilterButton->SetToolTip( _("Apply the filter") );
	
	bSizer15->Add( SfsMainFilterButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SfsMainFilterInvertCheckBox = new wxCheckBox( pan_SfsMain, wxID_ANY, _("Invert"), wxDefaultPosition, wxDefaultSize, 0 );
	
	SfsMainFilterInvertCheckBox->SetToolTip( _("Invert the filter") );
	
	bSizer15->Add( SfsMainFilterInvertCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer15->Add( 0, 0, 1, 0, 5 );
	
	SfsMainFilterValidateButton = new wxButton( pan_SfsMain, wxID_ANY, _("Validate all"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainFilterValidateButton->Enable( false );
	SfsMainFilterValidateButton->SetToolTip( _("Validates all displayed files") );
	
	bSizer15->Add( SfsMainFilterValidateButton, 0, wxALL, 5 );
	
	bSizer14->Add( bSizer15, 0, wxEXPAND, 5 );
	
	SfsMainListCtrl = new ModSfsListCtrl( pan_SfsMain, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxLC_HRULES|wxLC_REPORT );
	SfsMainListCtrl->SetMinSize( wxSize( 500,280 ) );
	
	bSizer14->Add( SfsMainListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	SfsMainDetailsPanel = new AnDetails( pan_SfsMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL, wxT("Details") );
	wxFlexGridSizer* fgSizer51;
	fgSizer51 = new wxFlexGridSizer( 2, 4, 0, 0 );
	fgSizer51->SetFlexibleDirection( wxBOTH );
	fgSizer51->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	SfsMainSignFilesCheckBox = new wxCheckBox( SfsMainDetailsPanel, wxID_ANY, _("Sign Files"), wxDefaultPosition, wxDefaultSize, 0 );
	
	SfsMainSignFilesCheckBox->Enable( false );
	SfsMainSignFilesCheckBox->SetToolTip( _("Enables signature support") );
	
	fgSizer51->Add( SfsMainSignFilesCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SfsMainSearchOrphanedButton = new wxButton( SfsMainDetailsPanel, wxID_ANY, _("Search Orphaned"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainSearchOrphanedButton->Enable( false );
	SfsMainSearchOrphanedButton->SetToolTip( _("Searches for orphaned files") );
	
	fgSizer51->Add( SfsMainSearchOrphanedButton, 0, wxALL, 5 );
	
	SfsMainShowChecksumButton = new wxButton( SfsMainDetailsPanel, wxID_ANY, _("Show all Checksums"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainShowChecksumButton->Enable( false );
	SfsMainShowChecksumButton->SetToolTip( _("Show files with checksum") );
	
	fgSizer51->Add( SfsMainShowChecksumButton, 0, wxALL, 5 );
	
	SfsMainImportButton = new wxButton( SfsMainDetailsPanel, wxID_ANY, _("Import..."), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainImportButton->Enable( false );
	SfsMainImportButton->SetToolTip( _("Import from file") );
	
	fgSizer51->Add( SfsMainImportButton, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	
	fgSizer51->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer51->Add( 0, 0, 1, wxEXPAND, 5 );
	
	SfsMainShowChangedButton = new wxButton( SfsMainDetailsPanel, wxID_ANY, _("Show Changed"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainShowChangedButton->Enable( false );
	SfsMainShowChangedButton->SetToolTip( _("Show files with modified checksum") );
	
	fgSizer51->Add( SfsMainShowChangedButton, 0, wxALL, 5 );
	
	SfsMainExportButton = new wxButton( SfsMainDetailsPanel, wxID_ANY, _("Export..."), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainExportButton->Enable( false );
	SfsMainExportButton->SetToolTip( _("Export to file") );
	
	fgSizer51->Add( SfsMainExportButton, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	SfsMainDetailsPanel->SetSizer( fgSizer51 );
	SfsMainDetailsPanel->Layout();
	fgSizer51->Fit( SfsMainDetailsPanel );
	bSizer14->Add( SfsMainDetailsPanel, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer151;
	bSizer151 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer151->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText101 = new wxStaticText( pan_SfsMain, wxID_ANY, _("Current selection:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText101->Wrap( -1 );
	bSizer151->Add( m_staticText101, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString SfsMainActionChoiceChoices[] = { _("Register"), _("Unregister"), _("Validate") };
	int SfsMainActionChoiceNChoices = sizeof( SfsMainActionChoiceChoices ) / sizeof( wxString );
	SfsMainActionChoice = new wxChoice( pan_SfsMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, SfsMainActionChoiceNChoices, SfsMainActionChoiceChoices, 0 );
	bSizer151->Add( SfsMainActionChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SfsMainActionButton = new wxButton( pan_SfsMain, wxID_ANY, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainActionButton->Enable( false );
	SfsMainActionButton->SetToolTip( _("Executes the selected action") );
	
	bSizer151->Add( SfsMainActionButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer14->Add( bSizer151, 0, wxEXPAND, 5 );
	
	bSizer10->Add( bSizer14, 2, wxEXPAND, 5 );
	
	pan_SfsMain->SetSizer( bSizer10 );
	pan_SfsMain->Layout();
	bSizer10->Fit( pan_SfsMain );
	note_MainSfs->AddPage( pan_SfsMain, _("Browser"), false );
	pan_Options = new wxPanel( note_MainSfs, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( pan_Options, -1, _("Private Key") ), wxVERTICAL );
	
	keyPicker = new AnPickFromFs( pan_Options, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	sbSizer1->Add( keyPicker, 0, wxEXPAND|wxALL, 5 );
	
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer3->AddGrowableCol( 1 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText13 = new wxStaticText( pan_Options, wxID_ANY, _("Passphrase validity:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	fgSizer3->Add( m_staticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxHORIZONTAL );
	
	wxString PrivKeyValidityChoiceChoices[] = { _("Until end of session"), _("Until specified time") };
	int PrivKeyValidityChoiceNChoices = sizeof( PrivKeyValidityChoiceChoices ) / sizeof( wxString );
	PrivKeyValidityChoice = new wxChoice( pan_Options, wxID_ANY, wxDefaultPosition, wxDefaultSize, PrivKeyValidityChoiceNChoices, PrivKeyValidityChoiceChoices, 0 );
	bSizer18->Add( PrivKeyValidityChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	PrivKeyValiditySpinCtrl = new wxSpinCtrl( pan_Options, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 86400, 0 );
	PrivKeyValiditySpinCtrl->Enable( false );
	
	bSizer18->Add( PrivKeyValiditySpinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	PrivKeyValidityText = new wxStaticText( pan_Options, wxID_ANY, _("Validity end in seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	PrivKeyValidityText->Wrap( -1 );
	PrivKeyValidityText->Enable( false );
	
	bSizer18->Add( PrivKeyValidityText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	fgSizer3->Add( bSizer18, 1, wxEXPAND, 5 );
	
	sbSizer1->Add( fgSizer3, 0, wxEXPAND, 5 );
	
	bSizer16->Add( sbSizer1, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( pan_Options, -1, _("Certificate") ), wxVERTICAL );
	
	certificatePicker = new AnPickFromFs( pan_Options, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	sbSizer2->Add( certificatePicker, 1, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer4->AddGrowableCol( 1 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText16 = new wxStaticText( pan_Options, wxID_ANY, _("Fingerprint:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	fgSizer4->Add( m_staticText16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	CertFingerprintText = new wxStaticText( pan_Options, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, 0 );
	CertFingerprintText->Wrap( -1 );
	fgSizer4->Add( CertFingerprintText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticText18 = new wxStaticText( pan_Options, wxID_ANY, _("Distinguished Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	fgSizer4->Add( m_staticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	CertDnText = new wxStaticText( pan_Options, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, 0 );
	CertDnText->Wrap( -1 );
	fgSizer4->Add( CertDnText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	sbSizer2->Add( fgSizer4, 0, wxEXPAND, 5 );
	
	bSizer16->Add( sbSizer2, 0, wxEXPAND, 5 );
	
	pan_Options->SetSizer( bSizer16 );
	pan_Options->Layout();
	bSizer16->Fit( pan_Options );
	note_MainSfs->AddPage( pan_Options, _("Options"), false );
	
	sz_MainSFSMain->Add( note_MainSfs, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( sz_MainSFSMain );
	this->Layout();
	sz_MainSFSMain->Fit( this );
	
	// Connect Events
	SfsMainDirCtrl->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( ModSfsMainPanelBase::OnSfsMainDirCtrlSelChanged ), NULL, this );
	SfsMainDirTraversalCheckbox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainDirTraversalChecked ), NULL, this );
	SfsMainFilterTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainFilterButtonClicked ), NULL, this );
	SfsMainFilterButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainFilterButtonClicked ), NULL, this );
	SfsMainFilterInvertCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainInverseCheckboxClicked ), NULL, this );
	SfsMainFilterValidateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainValidateButtonClicked ), NULL, this );
	SfsMainListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( ModSfsMainPanelBase::OnSfsListDeselected ), NULL, this );
	SfsMainListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( ModSfsMainPanelBase::OnSfsListSelected ), NULL, this );
	SfsMainSignFilesCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainSigEnabledClicked ), NULL, this );
	SfsMainSearchOrphanedButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainSearchOrphanedClicked ), NULL, this );
	SfsMainShowChecksumButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainShowAllChecksumsClicked ), NULL, this );
	SfsMainImportButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainImportClicked ), NULL, this );
	SfsMainShowChangedButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainShowChangedClicked ), NULL, this );
	SfsMainExportButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainExportClicked ), NULL, this );
	SfsMainActionButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainApplyButtonClicked ), NULL, this );
	PrivKeyValidityChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ModSfsMainPanelBase::OnPrivKeyValidityChanged ), NULL, this );
	PrivKeyValiditySpinCtrl->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( ModSfsMainPanelBase::OnPrivKeyValidityPeriodChanged ), NULL, this );
}

ModSfsOverviewPanelBase::ModSfsOverviewPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetToolTip( _("Displays the status of the Secure File System Module (SFS) while connected.") );
	
	wxBoxSizer* sz_OverviewSFSMain;
	sz_OverviewSFSMain = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* sz_OVSFS;
	sz_OVSFS = new wxFlexGridSizer( 2, 4, 0, 0 );
	sz_OVSFS->SetFlexibleDirection( wxBOTH );
	sz_OVSFS->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	
	sz_OVSFS->Add( 20, 0, 1, wxEXPAND, 5 );
	
	sfsStatusIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	sz_OVSFS->Add( sfsStatusIcon, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* sz_lables;
	sz_lables = new wxBoxSizer( wxVERTICAL );
	
	txt_status = new wxStaticText( this, wxID_ANY, _("status (SFS):"), wxDefaultPosition, wxDefaultSize, 0 );
	txt_status->Wrap( -1 );
	sz_lables->Add( txt_status, 0, wxALL, 5 );
	
	txt_nachfragen = new wxStaticText( this, wxID_ANY, _("open requests (SFS):"), wxDefaultPosition, wxDefaultSize, 0 );
	txt_nachfragen->Wrap( -1 );
	sz_lables->Add( txt_nachfragen, 0, wxALL, 5 );
	
	sz_OVSFS->Add( sz_lables, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_values;
	sz_values = new wxBoxSizer( wxVERTICAL );
	
	txt_statusValue = new wxStaticText( this, wxID_ANY, _("everything OK"), wxDefaultPosition, wxDefaultSize, 0 );
	txt_statusValue->Wrap( -1 );
	sz_values->Add( txt_statusValue, 0, wxALL, 5 );
	
	txt_requestValue = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	txt_requestValue->Wrap( -1 );
	sz_values->Add( txt_requestValue, 0, wxALL, 5 );
	
	sz_OVSFS->Add( sz_values, 1, wxEXPAND, 5 );
	
	sz_OverviewSFSMain->Add( sz_OVSFS, 1, wxEXPAND, 5 );
	
	this->SetSizer( sz_OverviewSFSMain );
	this->Layout();
	sz_OverviewSFSMain->Fit( this );
}

ModSfsDetailsDlgBase::ModSfsDetailsDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer6->AddGrowableCol( 1 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText20 = new wxStaticText( this, wxID_ANY, _("Path:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText20->Wrap( -1 );
	fgSizer6->Add( m_staticText20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	pathTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 250,-1 ), wxTE_READONLY );
	fgSizer6->Add( pathTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	linkLabel = new wxStaticText( this, wxID_ANY, _("Link Target:"), wxDefaultPosition, wxDefaultSize, 0 );
	linkLabel->Wrap( -1 );
	fgSizer6->Add( linkLabel, 0, wxALL, 5 );
	
	linkTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 250,-1 ), wxTE_READONLY );
	fgSizer6->Add( linkTextCtrl, 0, wxALL, 5 );
	
	m_staticText21 = new wxStaticText( this, wxID_ANY, _("Last modified:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText21->Wrap( -1 );
	fgSizer6->Add( m_staticText21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	modifiedTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 250,-1 ), wxTE_READONLY );
	fgSizer6->Add( modifiedTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	m_staticText22 = new wxStaticText( this, wxID_ANY, _("Checksum:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText22->Wrap( -1 );
	fgSizer6->Add( m_staticText22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	checksumTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 250,-1 ), wxTE_READONLY );
	fgSizer6->Add( checksumTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	m_staticText23 = new wxStaticText( this, wxID_ANY, _("Registered checksum:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText23->Wrap( -1 );
	fgSizer6->Add( m_staticText23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	regChecksumTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 250,-1 ), wxTE_READONLY );
	fgSizer6->Add( regChecksumTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	m_staticText24 = new wxStaticText( this, wxID_ANY, _("Checksum status:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText24->Wrap( -1 );
	fgSizer6->Add( m_staticText24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	checksumStateLabel = new wxStaticText( this, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, 0 );
	checksumStateLabel->Wrap( -1 );
	fgSizer6->Add( checksumStateLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticText26 = new wxStaticText( this, wxID_ANY, _("Signature status:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText26->Wrap( -1 );
	fgSizer6->Add( m_staticText26, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	signatureStateLabel = new wxStaticText( this, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, 0 );
	signatureStateLabel->Wrap( -1 );
	fgSizer6->Add( signatureStateLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer20->Add( fgSizer6, 1, wxALL|wxEXPAND, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1->Realize();
	bSizer20->Add( m_sdbSizer1, 0, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( bSizer20 );
	this->Layout();
	bSizer20->Fit( this );
}
