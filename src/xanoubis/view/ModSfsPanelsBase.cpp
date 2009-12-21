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
