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
	
	tx_MainHeadline = new wxStaticText( this, wxID_ANY, _("SFS - Secure File System"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_MainHeadline->Wrap( -1 );
	tx_MainHeadline->SetFont( wxFont( 16, 70, 90, 90, false, wxEmptyString ) );
	
	sz_MainSFSMain->Add( tx_MainHeadline, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	note_MainSfs = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	pan_Rules = new wxPanel( note_MainSfs, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_SfsRules;
	sz_SfsRules = new wxBoxSizer( wxHORIZONTAL );
	
	lst_Rules = new wxGrid( pan_Rules, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	lst_Rules->CreateGrid( 5, 5 );
	lst_Rules->EnableEditing( false );
	lst_Rules->EnableGridLines( false );
	lst_Rules->EnableDragGridSize( false );
	lst_Rules->SetMargins( 0, 0 );
	
	// Columns
	lst_Rules->EnableDragColMove( false );
	lst_Rules->EnableDragColSize( true );
	lst_Rules->SetColLabelSize( 30 );
	lst_Rules->SetColLabelAlignment( wxALIGN_LEFT, wxALIGN_CENTRE );
	
	// Rows
	lst_Rules->EnableDragRowSize( true );
	lst_Rules->SetRowLabelSize( 0 );
	lst_Rules->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	lst_Rules->SetLabelFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	// Cell Defaults
	lst_Rules->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	sz_SfsRules->Add( lst_Rules, 1, wxALL|wxEXPAND, 5 );
	
	pan_Rules->SetSizer( sz_SfsRules );
	pan_Rules->Layout();
	sz_SfsRules->Fit( pan_Rules );
	note_MainSfs->AddPage( pan_Rules, _("Rules"), true );
	pan_SfsMain = new wxPanel( note_MainSfs, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* browserMainSizer;
	browserMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* pathSizer;
	pathSizer = new wxBoxSizer( wxHORIZONTAL );
	
	SfsMainDirectoryLabel = new wxStaticText( pan_SfsMain, wxID_ANY, _("Directory:"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainDirectoryLabel->Wrap( -1 );
	pathSizer->Add( SfsMainDirectoryLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SfsMainPathCtrl = new wxTextCtrl( pan_SfsMain, wxID_ANY, _("/usr/local/bin"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	pathSizer->Add( SfsMainPathCtrl, 1, wxALIGN_CENTER|wxALL, 5 );
	
	browserMainSizer->Add( pathSizer, 0, wxEXPAND, 5 );
	
	wxBoxSizer* controlSizer;
	controlSizer = new wxBoxSizer( wxHORIZONTAL );
	
	SfsMainDirCtrl = new wxGenericDirCtrl( pan_SfsMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDIRCTRL_DIR_ONLY|wxDIRCTRL_SHOW_FILTERS|wxSUNKEN_BORDER, wxEmptyString, 0 );
	
	SfsMainDirCtrl->ShowHidden( false );
	controlSizer->Add( SfsMainDirCtrl, 2, wxEXPAND | wxALL, 5 );
	
	browserListPanel = new wxScrolledWindow( pan_SfsMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	browserListPanel->SetScrollRate( 5, 5 );
	wxBoxSizer* browserListSizer;
	browserListSizer = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* SfsMainDirListHeadSizer;
	SfsMainDirListHeadSizer = new wxFlexGridSizer( 2, 3, 0, 0 );
	SfsMainDirListHeadSizer->SetFlexibleDirection( wxBOTH );
	SfsMainDirListHeadSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	SfsMainDirViewLabel = new wxStaticText( browserListPanel, wxID_ANY, _("View:"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainDirViewLabel->Wrap( -1 );
	SfsMainDirListHeadSizer->Add( SfsMainDirViewLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString SfsMainDirViewChoiceChoices[] = { _("Standard file browsing"), _("Files with checksum"), _("Changed files"), _("Orphaned checksums"), _("Upgraded files") };
	int SfsMainDirViewChoiceNChoices = sizeof( SfsMainDirViewChoiceChoices ) / sizeof( wxString );
	SfsMainDirViewChoice = new wxChoice( browserListPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, SfsMainDirViewChoiceNChoices, SfsMainDirViewChoiceChoices, 0 );
	SfsMainDirViewChoice->Enable( false );
	
	SfsMainDirListHeadSizer->Add( SfsMainDirViewChoice, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	SfsMainDirTraversalCheckbox = new wxCheckBox( browserListPanel, wxID_ANY, _("Recursive traversal"), wxDefaultPosition, wxDefaultSize, 0 );
	
	SfsMainDirTraversalCheckbox->SetToolTip( _("Enables recursive traversal through filesystem") );
	
	SfsMainDirListHeadSizer->Add( SfsMainDirTraversalCheckbox, 0, wxALL, 5 );
	
	SfsMainDirFilterLabel = new wxStaticText( browserListPanel, wxID_ANY, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainDirFilterLabel->Wrap( -1 );
	SfsMainDirListHeadSizer->Add( SfsMainDirFilterLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* SfsMainDirFilterSizer;
	SfsMainDirFilterSizer = new wxBoxSizer( wxHORIZONTAL );
	
	SfsMainFilterTextCtrl = new wxTextCtrl( browserListPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	SfsMainDirFilterSizer->Add( SfsMainFilterTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SfsMainFilterButton = new wxButton( browserListPanel, wxID_ANY, _("Filter"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainFilterButton->SetToolTip( _("Apply the filter") );
	
	SfsMainDirFilterSizer->Add( SfsMainFilterButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SfsMainDirListHeadSizer->Add( SfsMainDirFilterSizer, 1, wxEXPAND, 5 );
	
	SfsMainFilterInvertCheckBox = new wxCheckBox( browserListPanel, wxID_ANY, _("Invert"), wxDefaultPosition, wxDefaultSize, 0 );
	
	SfsMainFilterInvertCheckBox->SetToolTip( _("Invert the filter") );
	
	SfsMainDirListHeadSizer->Add( SfsMainFilterInvertCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	browserListSizer->Add( SfsMainDirListHeadSizer, 0, wxEXPAND, 5 );
	
	SfsMainListCtrl = new ModSfsListCtrl( browserListPanel, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxLC_HRULES|wxLC_REPORT );
	browserListSizer->Add( SfsMainListCtrl, 1, wxALL|wxEXPAND, 5 );
	
	SfsMainDetailsPanel = new AnDetails( browserListPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxRAISED_BORDER|wxTAB_TRAVERSAL, wxT("Details") );
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );
	
	SfsMainSignFilesCheckBox = new wxCheckBox( SfsMainDetailsPanel, wxID_ANY, _("Sign Files / Check Signatures"), wxDefaultPosition, wxDefaultSize, 0 );
	
	SfsMainSignFilesCheckBox->Enable( false );
	SfsMainSignFilesCheckBox->SetToolTip( _("Enables signature support") );
	
	bSizer20->Add( SfsMainSignFilesCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer20->Add( 0, 0, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	SfsMainImportButton = new wxButton( SfsMainDetailsPanel, wxID_ANY, _("Import..."), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainImportButton->Enable( false );
	SfsMainImportButton->SetToolTip( _("Import from file") );
	
	bSizer20->Add( SfsMainImportButton, 0, wxALIGN_RIGHT|wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	SfsMainExportButton = new wxButton( SfsMainDetailsPanel, wxID_ANY, _("Export..."), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainExportButton->Enable( false );
	SfsMainExportButton->SetToolTip( _("Export to file") );
	
	bSizer20->Add( SfsMainExportButton, 0, wxALIGN_RIGHT|wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	SfsMainDetailsPanel->SetSizer( bSizer20 );
	SfsMainDetailsPanel->Layout();
	bSizer20->Fit( SfsMainDetailsPanel );
	browserListSizer->Add( SfsMainDetailsPanel, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* browserListFootSizer;
	browserListFootSizer = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText101 = new wxStaticText( browserListPanel, wxID_ANY, _("Current selection:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText101->Wrap( -1 );
	browserListFootSizer->Add( m_staticText101, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString SfsMainActionChoiceChoices[] = { _("Register"), _("Unregister"), _("Validate") };
	int SfsMainActionChoiceNChoices = sizeof( SfsMainActionChoiceChoices ) / sizeof( wxString );
	SfsMainActionChoice = new wxChoice( browserListPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, SfsMainActionChoiceNChoices, SfsMainActionChoiceChoices, 0 );
	SfsMainActionChoice->Enable( false );
	
	browserListFootSizer->Add( SfsMainActionChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SfsMainActionButton = new wxButton( browserListPanel, wxID_ANY, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainActionButton->Enable( false );
	SfsMainActionButton->SetToolTip( _("Executes the selected action") );
	
	browserListFootSizer->Add( SfsMainActionButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	browserListFootSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	SfsMainFilterValidateButton = new wxButton( browserListPanel, wxID_ANY, _("Validate all"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainFilterValidateButton->Enable( false );
	SfsMainFilterValidateButton->SetToolTip( _("Validates all displayed files") );
	
	browserListFootSizer->Add( SfsMainFilterValidateButton, 0, wxALL, 5 );
	
	browserListSizer->Add( browserListFootSizer, 0, wxEXPAND, 5 );
	
	browserListPanel->SetSizer( browserListSizer );
	browserListPanel->Layout();
	browserListSizer->Fit( browserListPanel );
	controlSizer->Add( browserListPanel, 5, wxEXPAND | wxALL, 5 );
	
	browserMainSizer->Add( controlSizer, 1, wxEXPAND, 5 );
	
	pan_SfsMain->SetSizer( browserMainSizer );
	pan_SfsMain->Layout();
	browserMainSizer->Fit( pan_SfsMain );
	note_MainSfs->AddPage( pan_SfsMain, _("Browser"), false );
	pan_Options = new wxScrolledWindow( note_MainSfs, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	pan_Options->SetScrollRate( 5, 5 );
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
	
	m_staticText16 = new wxStaticText( pan_Options, wxID_ANY, _("KeyID:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	fgSizer4->Add( m_staticText16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	CertFingerprintText = new wxStaticText( pan_Options, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CertFingerprintText->Wrap( -1 );
	fgSizer4->Add( CertFingerprintText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticText26 = new wxStaticText( pan_Options, wxID_ANY, _("Country:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText26->Wrap( -1 );
	fgSizer4->Add( m_staticText26, 0, wxALL, 5 );
	
	CertCountryText = new wxStaticText( pan_Options, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CertCountryText->Wrap( -1 );
	fgSizer4->Add( CertCountryText, 0, wxALL, 5 );
	
	m_staticText28 = new wxStaticText( pan_Options, wxID_ANY, _("State or Province:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText28->Wrap( -1 );
	fgSizer4->Add( m_staticText28, 0, wxALL, 5 );
	
	CertStateText = new wxStaticText( pan_Options, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CertStateText->Wrap( -1 );
	fgSizer4->Add( CertStateText, 0, wxALL, 5 );
	
	m_staticText30 = new wxStaticText( pan_Options, wxID_ANY, _("Locality:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText30->Wrap( -1 );
	fgSizer4->Add( m_staticText30, 0, wxALL, 5 );
	
	CertLocalityText = new wxStaticText( pan_Options, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CertLocalityText->Wrap( -1 );
	fgSizer4->Add( CertLocalityText, 0, wxALL, 5 );
	
	m_staticText32 = new wxStaticText( pan_Options, wxID_ANY, _("Organization:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText32->Wrap( -1 );
	fgSizer4->Add( m_staticText32, 0, wxALL, 5 );
	
	CertOrgaText = new wxStaticText( pan_Options, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CertOrgaText->Wrap( -1 );
	fgSizer4->Add( CertOrgaText, 0, wxALL, 5 );
	
	m_staticText34 = new wxStaticText( pan_Options, wxID_ANY, _("Organizational Unit:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText34->Wrap( -1 );
	fgSizer4->Add( m_staticText34, 0, wxALL, 5 );
	
	CertOrgaunitText = new wxStaticText( pan_Options, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CertOrgaunitText->Wrap( -1 );
	fgSizer4->Add( CertOrgaunitText, 0, wxALL, 5 );
	
	m_staticText36 = new wxStaticText( pan_Options, wxID_ANY, _("Common Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText36->Wrap( -1 );
	fgSizer4->Add( m_staticText36, 0, wxALL, 5 );
	
	CertCnText = new wxStaticText( pan_Options, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CertCnText->Wrap( -1 );
	fgSizer4->Add( CertCnText, 0, wxALL, 5 );
	
	m_staticText38 = new wxStaticText( pan_Options, wxID_ANY, _("Email Address:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText38->Wrap( -1 );
	fgSizer4->Add( m_staticText38, 0, wxALL, 5 );
	
	CertEmailText = new wxStaticText( pan_Options, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CertEmailText->Wrap( -1 );
	fgSizer4->Add( CertEmailText, 0, wxALL, 5 );
	
	sbSizer2->Add( fgSizer4, 0, wxEXPAND, 5 );
	
	bSizer16->Add( sbSizer2, 0, wxEXPAND, 5 );
	
	btn_GenerateKeyPair = new wxButton( pan_Options, wxID_ANY, _("Generate Keypair"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer16->Add( btn_GenerateKeyPair, 0, wxALL, 5 );
	
	pan_Options->SetSizer( bSizer16 );
	pan_Options->Layout();
	bSizer16->Fit( pan_Options );
	note_MainSfs->AddPage( pan_Options, _("Options"), false );
	
	sz_MainSFSMain->Add( note_MainSfs, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( sz_MainSFSMain );
	this->Layout();
	sz_MainSFSMain->Fit( this );
	
	// Connect Events
	lst_Rules->Connect( wxEVT_GRID_CELL_LEFT_DCLICK, wxGridEventHandler( ModSfsMainPanelBase::OnGridCellLeftDClick ), NULL, this );
	SfsMainPathCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsPathChanged ), NULL, this );
	SfsMainDirCtrl->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( ModSfsMainPanelBase::OnSfsMainDirCtrlSelChanged ), NULL, this );
	SfsMainDirViewChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainDirViewChoiceSelected ), NULL, this );
	SfsMainDirTraversalCheckbox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainDirTraversalChecked ), NULL, this );
	SfsMainFilterTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainFilterButtonClicked ), NULL, this );
	SfsMainFilterButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainFilterButtonClicked ), NULL, this );
	SfsMainFilterInvertCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainInverseCheckboxClicked ), NULL, this );
	SfsMainListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( ModSfsMainPanelBase::OnSfsListDeselected ), NULL, this );
	SfsMainListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( ModSfsMainPanelBase::OnSfsListSelected ), NULL, this );
	SfsMainSignFilesCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainSigEnabledClicked ), NULL, this );
	SfsMainImportButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainImportClicked ), NULL, this );
	SfsMainExportButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainExportClicked ), NULL, this );
	SfsMainActionButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainApplyButtonClicked ), NULL, this );
	SfsMainFilterValidateButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnSfsMainValidateButtonClicked ), NULL, this );
	PrivKeyValidityChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ModSfsMainPanelBase::OnPrivKeyValidityChanged ), NULL, this );
	PrivKeyValiditySpinCtrl->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( ModSfsMainPanelBase::OnPrivKeyValidityPeriodChanged ), NULL, this );
	btn_GenerateKeyPair->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModSfsMainPanelBase::OnGenerateKeyPair ), NULL, this );
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
	
	regSigLabel = new wxStaticText( this, wxID_ANY, _("Signature:"), wxDefaultPosition, wxDefaultSize, 0 );
	regSigLabel->Wrap( -1 );
	fgSizer6->Add( regSigLabel, 0, wxALL, 5 );
	
	regSigTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 250,-1 ), wxTE_READONLY );
	fgSizer6->Add( regSigTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
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

ModSfsGenerateKeyDlgBase::ModSfsGenerateKeyDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxDialog( parent, id, title, pos, size, style, name )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sz_PrivateKey;
	sz_PrivateKey = new wxStaticBoxSizer( new wxStaticBox( this, -1, _("Private Key") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer111;
	fgSizer111 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizer111->AddGrowableCol( 1 );
	fgSizer111->SetFlexibleDirection( wxBOTH );
	fgSizer111->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText52 = new wxStaticText( this, wxID_ANY, _("Configure private key:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText52->Wrap( -1 );
	fgSizer111->Add( m_staticText52, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	pathPrivKeyTxtCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pathPrivKeyTxtCtrl->SetMinSize( wxSize( 220,-1 ) );
	
	fgSizer111->Add( pathPrivKeyTxtCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	pathPrivKeyBrowseBtn = new wxButton( this, wxID_ANY, _("Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer111->Add( pathPrivKeyBrowseBtn, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	m_staticText53 = new wxStaticText( this, wxID_ANY, _("Passphrase:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText53->Wrap( -1 );
	fgSizer111->Add( m_staticText53, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	PassphrPrivKeyTxtCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer111->Add( PassphrPrivKeyTxtCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	PassphrMisMatchIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer111->Add( PassphrMisMatchIcon, 0, wxALL, 5 );
	
	PassphrMismatchTxt = new wxStaticText( this, wxID_ANY, _("Passphrase (repeat):"), wxDefaultPosition, wxDefaultSize, 0 );
	PassphrMismatchTxt->Wrap( -1 );
	fgSizer111->Add( PassphrMismatchTxt, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	PassphrRepeatPrivKeyTxtCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer111->Add( PassphrRepeatPrivKeyTxtCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	m_staticText55 = new wxStaticText( this, wxID_ANY, _("Passphrase mismatch!"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText55->Wrap( -1 );
	m_staticText55->Enable( false );
	
	fgSizer111->Add( m_staticText55, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	sz_PrivateKey->Add( fgSizer111, 1, wxEXPAND, 5 );
	
	bSizer18->Add( sz_PrivateKey, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sz_Certificate;
	sz_Certificate = new wxStaticBoxSizer( new wxStaticBox( this, -1, _("Certificate") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizer11->AddGrowableCol( 1 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText50 = new wxStaticText( this, wxID_ANY, _("Configure Certificate:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText50->Wrap( -1 );
	fgSizer11->Add( m_staticText50, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	pathToCertTxtCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pathToCertTxtCtrl->SetMinSize( wxSize( 220,-1 ) );
	
	fgSizer11->Add( pathToCertTxtCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	pathCertBrowseBtn = new wxButton( this, wxID_ANY, _("Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( pathCertBrowseBtn, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	m_staticText42 = new wxStaticText( this, wxID_ANY, _("Country:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText42->Wrap( -1 );
	fgSizer11->Add( m_staticText42, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	CountryOfCertTxtCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,-1 ), 0 );
	fgSizer11->Add( CountryOfCertTxtCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	fgSizer11->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText43 = new wxStaticText( this, wxID_ANY, _("State or Province:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText43->Wrap( -1 );
	fgSizer11->Add( m_staticText43, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	StateOfCertTxtCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( StateOfCertTxtCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	fgSizer11->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText44 = new wxStaticText( this, wxID_ANY, _("Locality:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText44->Wrap( -1 );
	fgSizer11->Add( m_staticText44, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	LocalityOfCertTxtCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( LocalityOfCertTxtCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	fgSizer11->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText45 = new wxStaticText( this, wxID_ANY, _("Organization:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText45->Wrap( -1 );
	fgSizer11->Add( m_staticText45, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	OrgaOfCertTxtCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( OrgaOfCertTxtCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	fgSizer11->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText46 = new wxStaticText( this, wxID_ANY, _("Organizational Unit:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText46->Wrap( -1 );
	fgSizer11->Add( m_staticText46, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	OrgaunitOfCertTxtCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( OrgaunitOfCertTxtCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	fgSizer11->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText47 = new wxStaticText( this, wxID_ANY, _("Common Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText47->Wrap( -1 );
	fgSizer11->Add( m_staticText47, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ComNameOfCertTxtCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( ComNameOfCertTxtCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	
	fgSizer11->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText48 = new wxStaticText( this, wxID_ANY, _("Email Address:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText48->Wrap( -1 );
	fgSizer11->Add( m_staticText48, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	EmailOfCertTxtCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( EmailOfCertTxtCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	sz_Certificate->Add( fgSizer11, 1, wxALL|wxEXPAND, 5 );
	
	bSizer18->Add( sz_Certificate, 0, wxEXPAND, 5 );
	
	m_sdbSizer4 = new wxStdDialogButtonSizer();
	m_sdbSizer4OK = new wxButton( this, wxID_OK );
	m_sdbSizer4->AddButton( m_sdbSizer4OK );
	m_sdbSizer4Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer4->AddButton( m_sdbSizer4Cancel );
	m_sdbSizer4->Realize();
	bSizer18->Add( m_sdbSizer4, 0, wxEXPAND, 5 );
	
	this->SetSizer( bSizer18 );
	this->Layout();
	bSizer18->Fit( this );
}
