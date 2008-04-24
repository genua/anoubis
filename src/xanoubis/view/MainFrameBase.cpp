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

#include "MainFrameBase.h"

///////////////////////////////////////////////////////////////////////////

MainFrameBase::MainFrameBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	an_menubar = new wxMenuBar( 0 );
	me_menubarFile = new wxMenu();
	wxMenuItem* mi_mbFileConnect = new wxMenuItem( me_menubarFile, ID_MIFILECONNECT, wxString( _("Connect") ) , wxEmptyString, wxITEM_CHECK );
	me_menubarFile->Append( mi_mbFileConnect );
	wxMenuItem* mi_mbFileImport = new wxMenuItem( me_menubarFile, ID_MIFILEIMPORT, wxString( _("Import policy file...") ) , wxEmptyString, wxITEM_NORMAL );
	me_menubarFile->Append( mi_mbFileImport );
	
	me_menubarFile->AppendSeparator();
	wxMenuItem* mi_mbFileClose = new wxMenuItem( me_menubarFile, ID_MIFILECLOSE, wxString( _("Close") ) , wxEmptyString, wxITEM_NORMAL );
	me_menubarFile->Append( mi_mbFileClose );
	wxMenuItem* mi_mbFileQuit = new wxMenuItem( me_menubarFile, ID_MIFILEQUIT, wxString( _("Quit") ) , wxEmptyString, wxITEM_NORMAL );
	me_menubarFile->Append( mi_mbFileQuit );
	an_menubar->Append( me_menubarFile, _("File") );
	
	me_menubarEdit = new wxMenu();
	wxMenuItem* mi_mbEditPreferences = new wxMenuItem( me_menubarEdit, ID_MIEDITPREFERENCES, wxString( _("Preferences") ) , wxEmptyString, wxITEM_NORMAL );
	me_menubarEdit->Append( mi_mbEditPreferences );
	an_menubar->Append( me_menubarEdit, _("Edit") );
	
	me_menubarTools = new wxMenu();
	wxMenuItem* mi_mbToolsRuleEditor = new wxMenuItem( me_menubarTools, ID_MITOOLSRULEEDITOR, wxString( _("Rule Editor") ) , wxEmptyString, wxITEM_CHECK );
	me_menubarTools->Append( mi_mbToolsRuleEditor );
	wxMenuItem* mi_mbToolsLogViewer = new wxMenuItem( me_menubarTools, ID_MITOOLSLOGVIEWER, wxString( _("Log Viewer") ) , wxEmptyString, wxITEM_CHECK );
	me_menubarTools->Append( mi_mbToolsLogViewer );
	an_menubar->Append( me_menubarTools, _("Tools") );
	
	me_menubarHelp = new wxMenu();
	wxMenuItem* mi_mbHelpHelp = new wxMenuItem( me_menubarHelp, ID_MIHELPHELP, wxString( _("Help") ) , wxEmptyString, wxITEM_NORMAL );
	me_menubarHelp->Append( mi_mbHelpHelp );
	wxMenuItem* mi_mbHelpAbout = new wxMenuItem( me_menubarHelp, ID_MIHELPABOUT, wxString( _("About") ) , wxEmptyString, wxITEM_NORMAL );
	me_menubarHelp->Append( mi_mbHelpAbout );
	an_menubar->Append( me_menubarHelp, _("Help") );
	
	this->SetMenuBar( an_menubar );
	
	sz_mainframeMain = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* sz_mfMainLeft;
	sz_mfMainLeft = new wxBoxSizer( wxVERTICAL );
	
	pa_MainLeftStatus = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pa_MainLeftStatus->SetMinSize( wxSize( 150,-1 ) );
	pa_MainLeftStatus->SetMaxSize( wxSize( 150,-1 ) );
	
	wxStaticBoxSizer* bz_statuswindow;
	bz_statuswindow = new wxStaticBoxSizer( new wxStaticBox( pa_MainLeftStatus, -1, _("Status") ), wxVERTICAL );
	
	tx_messages = new wxStaticText( pa_MainLeftStatus, wxID_ANY, _("No messages"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_messages->Wrap( -1 );
	bz_statuswindow->Add( tx_messages, 0, wxALL, 5 );
	
	
	bz_statuswindow->Add( 0, 20, 1, wxEXPAND, 5 );
	
	tx_connected = new wxStaticText( pa_MainLeftStatus, wxID_ANY, _("connected with localhost"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_connected->Wrap( -1 );
	tx_connected->SetMaxSize( wxSize( 150,-1 ) );
	
	bz_statuswindow->Add( tx_connected, 0, wxALL, 5 );
	
	pa_MainLeftStatus->SetSizer( bz_statuswindow );
	pa_MainLeftStatus->Layout();
	bz_statuswindow->Fit( pa_MainLeftStatus );
	sz_mfMainLeft->Add( pa_MainLeftStatus, 0, wxEXPAND|wxFIXED_MINSIZE|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* sz_MainLeftLine;
	sz_MainLeftLine = new wxBoxSizer( wxHORIZONTAL );
	
	li_MainLineLeft = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	sz_MainLeftLine->Add( li_MainLineLeft, 0, wxEXPAND|wxLEFT, 5 );
	
	sw_MainLeftToolbar = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL );
	sw_MainLeftToolbar->SetScrollRate( 5, 5 );
	wxBoxSizer* sz_LeftToolbar;
	sz_LeftToolbar = new wxBoxSizer( wxVERTICAL );
	
	tb_LeftToolbarModule = new wxToolBar( sw_MainLeftToolbar, ID_TOOLBAR, wxDefaultPosition, wxSize( -1,-1 ), wxTB_TEXT|wxTB_VERTICAL|wxNO_BORDER ); 
	tb_LeftToolbarModule->Realize();
	
	sz_LeftToolbar->Add( tb_LeftToolbarModule, 1, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	sw_MainLeftToolbar->SetSizer( sz_LeftToolbar );
	sw_MainLeftToolbar->Layout();
	sz_LeftToolbar->Fit( sw_MainLeftToolbar );
	sz_MainLeftLine->Add( sw_MainLeftToolbar, 1, wxEXPAND | wxALL, 3 );
	
	
	sz_MainLeftLine->Add( 0, 0, 0, wxLEFT, 1 );
	
	li_MainLineRight = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	sz_MainLeftLine->Add( li_MainLineRight, 0, wxEXPAND|wxRIGHT, 5 );
	
	sz_mfMainLeft->Add( sz_MainLeftLine, 1, wxEXPAND, 5 );
	
	sz_mainframeMain->Add( sz_mfMainLeft, 0, wxEXPAND, 5 );
	
	this->SetSizer( sz_mainframeMain );
	this->Layout();
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MainFrameBase::OnClose ) );
	this->Connect( mi_mbFileConnect->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::OnMbFileConnectSelect ) );
	this->Connect( mi_mbFileImport->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::OnMbFileImportSelect ) );
	this->Connect( mi_mbFileClose->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::OnMbFileCloseSelect ) );
	this->Connect( mi_mbFileQuit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::OnMbFileQuitSelect ) );
	this->Connect( mi_mbEditPreferences->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::OnMbEditPreferencesSelect ) );
	this->Connect( mi_mbToolsRuleEditor->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::OnMbToolsRuleEditorSelect ) );
	this->Connect( mi_mbToolsLogViewer->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::OnMbToolsLogViewerSelect ) );
	this->Connect( mi_mbHelpHelp->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::OnMbHelpHelpSelect ) );
	this->Connect( mi_mbHelpAbout->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::OnMbHelpAboutSelect ) );
}
