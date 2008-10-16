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
	
	wxBoxSizer* sz_RulesOperations;
	sz_RulesOperations = new wxBoxSizer( wxVERTICAL );
	
	tx_RulesOperation1stHeader = new wxStaticText( pan_Rules, wxID_ANY, _("Rule:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_RulesOperation1stHeader->Wrap( -1 );
	sz_RulesOperations->Add( tx_RulesOperation1stHeader, 0, wxALL, 5 );
	
	ln_RulesOperationSep = new wxStaticLine( pan_Rules, wxID_RulesOperationSep, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	sz_RulesOperations->Add( ln_RulesOperationSep, 0, wxEXPAND | wxALL, 5 );
	
	tx_RulesOperation2ndHeader = new wxStaticText( pan_Rules, wxID_ANY, _("Information:\n\nHere you can see the\nrules of the SFS. They are displayed\ndepending on their priority."), wxDefaultPosition, wxDefaultSize, 0 );
	tx_RulesOperation2ndHeader->Wrap( -1 );
	sz_RulesOperations->Add( tx_RulesOperation2ndHeader, 0, wxALL, 5 );
	
	sz_RulesWE->Add( sz_RulesOperations, 34, wxEXPAND, 5 );
	
	sz_RulesNS->Add( sz_RulesWE, 1, wxEXPAND, 5 );
	
	sz_SfsRules->Add( sz_RulesNS, 1, wxEXPAND, 5 );
	
	pan_Rules->SetSizer( sz_SfsRules );
	pan_Rules->Layout();
	sz_SfsRules->Fit( pan_Rules );
	note_MainSfs->AddPage( pan_Rules, _("Rules"), false );
	pan_SfsMain = new wxPanel( note_MainSfs, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );
	
	wxGridBagSizer* gbSizer2;
	gbSizer2 = new wxGridBagSizer( 0, 0 );
	gbSizer2->SetFlexibleDirection( wxBOTH );
	gbSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	SfsMainCurrPathLabel = new wxStaticText( pan_SfsMain, wxID_ANY, _("/usr/local/bin/"), wxDefaultPosition, wxDefaultSize, 0 );
	SfsMainCurrPathLabel->Wrap( -1 );
	gbSizer2->Add( SfsMainCurrPathLabel, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL, 5 );
	
	SfsMainDirCtrl = new wxGenericDirCtrl( pan_SfsMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDIRCTRL_DIR_ONLY|wxDIRCTRL_SHOW_FILTERS|wxSUNKEN_BORDER, wxEmptyString, 0 );
	
	SfsMainDirCtrl->ShowHidden( false );
	SfsMainDirCtrl->SetMinSize( wxSize( 200,-1 ) );
	
	gbSizer2->Add( SfsMainDirCtrl, wxGBPosition( 1, 0 ), wxGBSpan( 4, 1 ), wxEXPAND | wxALL, 5 );
	
	SfsMainListCtrl = new wxListCtrl( pan_SfsMain, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL );
	SfsMainListCtrl->SetMinSize( wxSize( 500,280 ) );
	
	gbSizer2->Add( SfsMainListCtrl, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 1, 5, 0, 0 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText10 = new wxStaticText( pan_SfsMain, wxID_ANY, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	fgSizer5->Add( m_staticText10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxGridSizer* gSizer3;
	gSizer3 = new wxGridSizer( 1, 5, 0, 0 );
	
	SfsMainFilterTextCtrl = new wxTextCtrl( pan_SfsMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	gSizer3->Add( SfsMainFilterTextCtrl, 0, wxALL|wxEXPAND, 5 );
	
	SfsMainFilterButton = new wxButton( pan_SfsMain, wxID_ANY, _("Filter"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( SfsMainFilterButton, 0, wxALL, 5 );
	
	SfsMainFilterInvertCheckBox = new wxCheckBox( pan_SfsMain, wxID_ANY, _("Invert"), wxDefaultPosition, wxDefaultSize, 0 );
	
	gSizer3->Add( SfsMainFilterInvertCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	gSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
	
	SfsMainFilterValidateButton = new wxButton( pan_SfsMain, wxID_ANY, _("Validate all"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( SfsMainFilterValidateButton, 0, wxALL, 5 );
	
	fgSizer5->Add( gSizer3, 1, wxEXPAND, 5 );
	
	gbSizer2->Add( fgSizer5, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), 0, 5 );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString SfsMainActionChoiceChoices[] = { _("Register"), _("Unregister"), _("Validate"), _("Update") };
	int SfsMainActionChoiceNChoices = sizeof( SfsMainActionChoiceChoices ) / sizeof( wxString );
	SfsMainActionChoice = new wxChoice( pan_SfsMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, SfsMainActionChoiceNChoices, SfsMainActionChoiceChoices, 0 );
	fgSizer6->Add( SfsMainActionChoice, 0, wxALL, 5 );
	
	SfsMainActionButton = new wxButton( pan_SfsMain, wxID_ANY, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( SfsMainActionButton, 0, wxALL, 5 );
	
	gbSizer2->Add( fgSizer6, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), 0, 5 );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( pan_SfsMain, -1, _("Details") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer51;
	fgSizer51 = new wxFlexGridSizer( 2, 4, 0, 0 );
	fgSizer51->SetFlexibleDirection( wxBOTH );
	fgSizer51->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	SfsMainSignFilesCheckBox = new wxCheckBox( pan_SfsMain, wxID_ANY, _("Sign Files"), wxDefaultPosition, wxDefaultSize, 0 );
	
	fgSizer51->Add( SfsMainSignFilesCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SfsMainShowChecksumButton = new wxButton( pan_SfsMain, wxID_ANY, _("Show all Checksums"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer51->Add( SfsMainShowChecksumButton, 0, wxALL, 5 );
	
	SfsMainSearchOrphanedButton = new wxButton( pan_SfsMain, wxID_ANY, _("Search Orphaned"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer51->Add( SfsMainSearchOrphanedButton, 0, wxALL, 5 );
	
	SfsMainShowChangedButton = new wxButton( pan_SfsMain, wxID_ANY, _("Show Changed"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer51->Add( SfsMainShowChangedButton, 0, wxALL, 5 );
	
	
	fgSizer51->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer51->Add( 0, 0, 1, wxEXPAND, 5 );
	
	SfsMainImportButton = new wxButton( pan_SfsMain, wxID_ANY, _("Import..."), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer51->Add( SfsMainImportButton, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	SfsMainExportButton = new wxButton( pan_SfsMain, wxID_ANY, _("Export..."), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer51->Add( SfsMainExportButton, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	sbSizer1->Add( fgSizer51, 1, wxEXPAND, 5 );
	
	gbSizer2->Add( sbSizer1, wxGBPosition( 4, 1 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	bSizer10->Add( gbSizer2, 0, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 5 );
	
	pan_SfsMain->SetSizer( bSizer10 );
	pan_SfsMain->Layout();
	bSizer10->Fit( pan_SfsMain );
	note_MainSfs->AddPage( pan_SfsMain, _("Browser"), false );
	
	sz_MainSFSMain->Add( note_MainSfs, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( sz_MainSFSMain );
	this->Layout();
	sz_MainSFSMain->Fit( this );
}

ModSfsOverviewPanelBase::ModSfsOverviewPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetToolTip( _("The level of restrictions increases with the\nprofile chosen.\nSettings of a lower profile\nare automatically valid in all profiles above.\nAll settings you choose are contingent upon\nthe chosen profile.\nIt may occur that you can't switch off the\nmodule because of restrictions.") );
	
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
