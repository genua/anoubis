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

#include "DlgBackupPolicyBase.h"

///////////////////////////////////////////////////////////////////////////

DlgBackupPolicyBase::DlgBackupPolicyBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* sz_main;
	sz_main = new wxBoxSizer( wxVERTICAL );
	
	
	sz_main->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_info;
	sz_info = new wxBoxSizer( wxHORIZONTAL );
	
	
	sz_info->Add( 0, 0, 2, wxEXPAND, 5 );
	
	bm_info = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	sz_info->Add( bm_info, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sz_info->Add( 0, 0, 1, wxEXPAND, 5 );
	
	tx_info = new wxStaticText( this, wxID_ANY, wxT("A new policy was loaded and your version of that policy has local modifications. You can backup your modified version or discard your changes."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	tx_info->Wrap( 400 );
	sz_info->Add( tx_info, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	sz_info->Add( 0, 0, 2, wxEXPAND, 5 );
	
	sz_main->Add( sz_info, 0, wxEXPAND, 5 );
	
	
	sz_main->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_policy;
	sz_policy = new wxBoxSizer( wxHORIZONTAL );
	
	
	sz_policy->Add( 0, 0, 1, wxEXPAND, 5 );
	
	tx_policyleft = new wxStaticText( this, wxID_ANY, wxT("Affected Policy:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_policyleft->Wrap( -1 );
	sz_policy->Add( tx_policyleft, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	tx_policyspace = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	tx_policyspace->Wrap( -1 );
	sz_policy->Add( tx_policyspace, 0, wxALL, 5 );
	
	tx_policyright = new wxStaticText( this, wxID_ANY, wxT("User Policy of %ls"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_policyright->Wrap( -1 );
	sz_policy->Add( tx_policyright, 0, wxALL, 5 );
	
	
	sz_policy->Add( 0, 0, 1, wxEXPAND, 5 );
	
	sz_main->Add( sz_policy, 0, wxEXPAND, 5 );
	
	
	sz_main->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sz_main->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_buttons;
	sz_buttons = new wxBoxSizer( wxHORIZONTAL );
	
	
	sz_buttons->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bt_discard = new wxButton( this, wxID_ANY, wxT("Discard"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_buttons->Add( bt_discard, 0, wxALL, 5 );
	
	
	sz_buttons->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bt_save = new wxButton( this, wxID_ANY, wxT("Save..."), wxDefaultPosition, wxDefaultSize, 0 );
	bt_save->SetDefault(); 
	sz_buttons->Add( bt_save, 0, wxALL, 5 );
	
	
	sz_buttons->Add( 0, 0, 1, wxEXPAND, 5 );
	
	sz_main->Add( sz_buttons, 0, wxEXPAND, 5 );
	
	
	sz_main->Add( 0, 0, 1, wxEXPAND, 5 );
	
	this->SetSizer( sz_main );
	this->Layout();
	
	// Connect Events
	bt_discard->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgBackupPolicyBase::onDiscardButton ), NULL, this );
	bt_save->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgBackupPolicyBase::onSaveButton ), NULL, this );
}
