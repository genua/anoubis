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

DlgRuleEditorBase::DlgRuleEditorBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxDialog( parent, id, title, pos, size, style, name )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* sz_main;
	sz_main = new wxBoxSizer( wxVERTICAL );
	
	wxString rb_modSelectorChoices[] = { wxT("ALF"), wxT("SFS") };
	int rb_modSelectorNChoices = sizeof( rb_modSelectorChoices ) / sizeof( wxString );
	rb_modSelector = new wxRadioBox( this, wxID_ANY, wxT("Choose module"), wxDefaultPosition, wxDefaultSize, rb_modSelectorNChoices, rb_modSelectorChoices, 1, wxRA_SPECIFY_ROWS );
	rb_modSelector->SetSelection( 0 );
	rb_modSelector->SetToolTip( wxT("Select the module you want to create a rule for") );
	
	sz_main->Add( rb_modSelector, 0, wxALL|wxEXPAND, 5 );
	
	pa_common = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( 200,100 ), wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sz_common;
	sz_common = new wxStaticBoxSizer( new wxStaticBox( pa_common, -1, wxT("Common settings") ), wxVERTICAL );
	
	pa_common->SetSizer( sz_common );
	pa_common->Layout();
	sz_main->Add( pa_common, 1, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( sz_main );
	this->Layout();
	
	// Connect Events
	this->Connect( wxEVT_CHAR, wxKeyEventHandler( DlgRuleEditorBase::OnKeyPressed ) );
	rb_modSelector->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DlgRuleEditorBase::OnModSelected ), NULL, this );
}

DlgRuleEditorAlfPanelBase::DlgRuleEditorAlfPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxStaticBoxSizer* sz_mainAlf;
	sz_mainAlf = new wxStaticBoxSizer( new wxStaticBox( this, -1, wxT("Application level firewall") ), wxVERTICAL );
	
	this->SetSizer( sz_mainAlf );
	this->Layout();
}

DlgRuleEditorSfsPanelBase::DlgRuleEditorSfsPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxStaticBoxSizer* sz_mainSfs;
	sz_mainSfs = new wxStaticBoxSizer( new wxStaticBox( this, -1, wxT("Secure file system") ), wxVERTICAL );
	
	this->SetSizer( sz_mainSfs );
	this->Layout();
}
