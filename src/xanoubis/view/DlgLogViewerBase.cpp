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

#include "DlgLogViewerBase.h"

///////////////////////////////////////////////////////////////////////////

DlgLogViewerBase::DlgLogViewerBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxDialog( parent, id, title, pos, size, style, name )
{
	this->SetSizeHints( wxSize( 700,400 ), wxDefaultSize );
	
	wxBoxSizer* sz_main;
	sz_main = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* sz_filePicker;
	sz_filePicker = new wxBoxSizer( wxHORIZONTAL );
	
	tx_logFile = new wxStaticText( this, wxID_ANY, wxT("Viewed log file:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_logFile->Wrap( -1 );
	sz_filePicker->Add( tx_logFile, 0, wxALIGN_CENTER|wxALL, 5 );
	
	fp_logFile = new wxFilePickerCtrl( this, wxID_ANY, wxT("/var/log/syslog"), wxT("Select a file"), wxT("*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE|wxFLP_FILE_MUST_EXIST );
	sz_filePicker->Add( fp_logFile, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5 );
	
	sz_main->Add( sz_filePicker, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );
	
	tx_logList = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_BESTWRAP|wxTE_DONTWRAP|wxTE_LEFT|wxTE_MULTILINE|wxTE_READONLY|wxTE_WORDWRAP );
	sz_main->Add( tx_logList, 1, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( sz_main );
	this->Layout();
	sz_main->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_CHAR, wxKeyEventHandler( DlgLogViewerBase::OnKeyPressed ) );
	fp_logFile->Connect( wxEVT_COMMAND_FILEPICKER_CHANGED, wxFileDirPickerEventHandler( DlgLogViewerBase::OnFileSelected ), NULL, this );
}
