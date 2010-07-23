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

#include "ModPlaygroundPanelsBase.h"

///////////////////////////////////////////////////////////////////////////

ModPlaygroundMainPanelBase::ModPlaygroundMainPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	mainHeadlineLabel = new wxStaticText( this, wxID_ANY, _("PG - Playground"), wxDefaultPosition, wxDefaultSize, 0 );
	mainHeadlineLabel->Wrap( -1 );
	mainHeadlineLabel->SetFont( wxFont( 16, 70, 90, 90, false, wxEmptyString ) );
	
	mainSizer->Add( mainHeadlineLabel, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	pgNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	pgPage = new wxPanel( pgNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* pgPageSizer;
	pgPageSizer = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* startSizer;
	startSizer = new wxStaticBoxSizer( new wxStaticBox( pgPage, -1, _("New Playground") ), wxVERTICAL );
	
	applicationLabel = new wxStaticText( pgPage, wxID_ANY, _("Application:"), wxDefaultPosition, wxDefaultSize, 0 );
	applicationLabel->Wrap( -1 );
	startSizer->Add( applicationLabel, 0, wxALL, 5 );
	
	applicationComboBox = new wxComboBox( pgPage, PG_APP_COMBOBOX, wxEmptyString, wxDefaultPosition, wxSize( 350,-1 ), 0, NULL, wxTE_PROCESS_ENTER ); 
	startSizer->Add( applicationComboBox, 0, wxALL, 5 );
	
	applicationStartButton = new wxButton( pgPage, PG_APP_STARTBUTTON, _("Start Playground"), wxDefaultPosition, wxDefaultSize, 0 );
	startSizer->Add( applicationStartButton, 0, wxALL, 5 );
	
	pgPageSizer->Add( startSizer, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* pgOverviewListSizer;
	pgOverviewListSizer = new wxStaticBoxSizer( new wxStaticBox( pgPage, -1, _("Playground Overview:") ), wxVERTICAL );
	
	pgGrid = new AnGrid( pgPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	pgGrid->CreateGrid( 1, 2 );
	pgGrid->EnableEditing( false );
	pgGrid->EnableGridLines( true );
	pgGrid->EnableDragGridSize( false );
	pgGrid->SetMargins( 0, 0 );
	
	// Columns
	pgGrid->SetColSize( 0, 80 );
	pgGrid->SetColSize( 1, 80 );
	pgGrid->EnableDragColMove( false );
	pgGrid->EnableDragColSize( true );
	pgGrid->SetColLabelSize( 30 );
	pgGrid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Rows
	pgGrid->EnableDragRowSize( true );
	pgGrid->SetRowLabelSize( 0 );
	pgGrid->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	pgGrid->SetLabelFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	// Cell Defaults
	pgGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	pgOverviewListSizer->Add( pgGrid, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* pgButtonSizer;
	pgButtonSizer = new wxBoxSizer( wxHORIZONTAL );
	
	pgRefreshButton = new wxButton( pgPage, wxID_ANY, _("Refresh view"), wxDefaultPosition, wxDefaultSize, 0 );
	pgRefreshButton->Enable( false );
	
	pgButtonSizer->Add( pgRefreshButton, 0, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 5 );
	
	
	pgButtonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	pgCommitButton = new wxButton( pgPage, wxID_ANY, _("Commit files..."), wxDefaultPosition, wxDefaultSize, 0 );
	pgCommitButton->Enable( false );
	
	pgButtonSizer->Add( pgCommitButton, 0, wxALL, 5 );
	
	pgDeleteButton = new wxButton( pgPage, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	pgDeleteButton->Enable( false );
	
	pgButtonSizer->Add( pgDeleteButton, 0, wxALIGN_BOTTOM|wxALIGN_RIGHT|wxALL, 5 );
	
	pgOverviewListSizer->Add( pgButtonSizer, 0, wxALIGN_BOTTOM|wxALIGN_RIGHT|wxEXPAND, 5 );
	
	pgPageSizer->Add( pgOverviewListSizer, 1, wxALL|wxEXPAND, 5 );
	
	pgPage->SetSizer( pgPageSizer );
	pgPage->Layout();
	pgPageSizer->Fit( pgPage );
	pgNotebook->AddPage( pgPage, _("Playground"), false );
	
	mainSizer->Add( pgNotebook, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( mainSizer );
	this->Layout();
	
	// Connect Events
	pgNotebook->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING, wxNotebookEventHandler( ModPlaygroundMainPanelBase::onPgNotebookChanging ), NULL, this );
	applicationComboBox->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( ModPlaygroundMainPanelBase::onAppStartEnter ), NULL, this );
	applicationStartButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModPlaygroundMainPanelBase::onAppStart ), NULL, this );
	pgGrid->Connect( wxEVT_GRID_SELECT_CELL, wxGridEventHandler( ModPlaygroundMainPanelBase::OnCellSelect ), NULL, this );
	pgRefreshButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModPlaygroundMainPanelBase::onPgListRefreshClicked ), NULL, this );
}

ModPlaygroundOverviewPanelBase::ModPlaygroundOverviewPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetToolTip( _("Displays the status of the Playground (PG) while connected.") );
	
	wxFlexGridSizer* mainSizer;
	mainSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
	mainSizer->SetFlexibleDirection( wxBOTH );
	mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	
	mainSizer->Add( 20, 0, 1, wxEXPAND, 5 );
	
	statusIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	mainSizer->Add( statusIcon, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* labelSizer;
	labelSizer = new wxBoxSizer( wxVERTICAL );
	
	labelSizer->SetMinSize( wxSize( 200,-1 ) ); 
	statusLabel = new wxStaticText( this, wxID_ANY, _("status (PG):"), wxDefaultPosition, wxDefaultSize, 0 );
	statusLabel->Wrap( -1 );
	labelSizer->Add( statusLabel, 0, wxALL, 5 );
	
	countLabel = new wxStaticText( this, wxID_ANY, _("playgrounds (PG):"), wxDefaultPosition, wxDefaultSize, 0 );
	countLabel->Wrap( -1 );
	labelSizer->Add( countLabel, 0, wxALL, 5 );
	
	mainSizer->Add( labelSizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* valueSizer;
	valueSizer = new wxBoxSizer( wxVERTICAL );
	
	statusText = new wxStaticText( this, wxID_ANY, _("everything OK"), wxDefaultPosition, wxDefaultSize, 0 );
	statusText->Wrap( -1 );
	valueSizer->Add( statusText, 0, wxALL, 5 );
	
	countText = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	countText->Wrap( -1 );
	valueSizer->Add( countText, 0, wxALL, 5 );
	
	mainSizer->Add( valueSizer, 1, wxEXPAND, 5 );
	
	this->SetSizer( mainSizer );
	this->Layout();
}

modPlaygroundCommitDialog::modPlaygroundCommitDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* textSizer;
	textSizer = new wxBoxSizer( wxVERTICAL );
	
	descriptionText = new wxStaticText( this, wxID_ANY, _("This is a list of files that have been used or created within the current playground. All selected files will be committed to the file system. Existing files won't be replaced without further permission."), wxDefaultPosition, wxDefaultSize, 0 );
	descriptionText->Wrap( 522 );
	textSizer->Add( descriptionText, 0, wxALL|wxEXPAND, 10 );
	
	wxStaticBoxSizer* mainSizer;
	mainSizer = new wxStaticBoxSizer( new wxStaticBox( this, -1, _("Playground Files") ), wxVERTICAL );
	
	commitFilelistGrid = new wxGrid( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxVSCROLL );
	
	// Grid
	commitFilelistGrid->CreateGrid( 1, 1 );
	commitFilelistGrid->EnableEditing( true );
	commitFilelistGrid->EnableGridLines( true );
	commitFilelistGrid->EnableDragGridSize( false );
	commitFilelistGrid->SetMargins( 0, 0 );
	
	// Columns
	commitFilelistGrid->EnableDragColMove( false );
	commitFilelistGrid->EnableDragColSize( true );
	commitFilelistGrid->SetColLabelSize( 30 );
	commitFilelistGrid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Rows
	commitFilelistGrid->EnableDragRowSize( true );
	commitFilelistGrid->SetRowLabelSize( 0 );
	commitFilelistGrid->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	
	// Cell Defaults
	commitFilelistGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	mainSizer->Add( commitFilelistGrid, 0, wxALL|wxEXPAND, 0 );
	
	wxGridSizer* footerSizer;
	footerSizer = new wxGridSizer( 1, 2, 0, 0 );
	
	commitScanFilesCheckbox = new wxCheckBox( this, wxID_ANY, _("Scan files"), wxDefaultPosition, wxDefaultSize, 0 );
	
	footerSizer->Add( commitScanFilesCheckbox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	comittButtonSizer = new wxStdDialogButtonSizer();
	comittButtonSizerOK = new wxButton( this, wxID_OK );
	comittButtonSizer->AddButton( comittButtonSizerOK );
	comittButtonSizerCancel = new wxButton( this, wxID_CANCEL );
	comittButtonSizer->AddButton( comittButtonSizerCancel );
	comittButtonSizer->Realize();
	footerSizer->Add( comittButtonSizer, 1, wxEXPAND, 5 );
	
	mainSizer->Add( footerSizer, 1, wxEXPAND, 5 );
	
	textSizer->Add( mainSizer, 1, wxEXPAND, 5 );
	
	this->SetSizer( textSizer );
	this->Layout();
}
