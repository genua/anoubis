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

#include "DlgUpgradeAskBase.h"

///////////////////////////////////////////////////////////////////////////

DlgUpgradeAskBase::DlgUpgradeAskBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxDialog( parent, id, title, pos, size, style, name )
{
	this->SetSizeHints( wxDefaultSize, wxSize( -1,-1 ) );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* warningSizer;
	warningSizer = new wxBoxSizer( wxHORIZONTAL );
	
	warningIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 48,48 ), 0 );
	warningSizer->Add( warningIcon, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	warningLabel = new wxStaticText( this, wxID_ANY, _("The system has been upgraded. Therefore the checksums of all related files have changed. Please check the checksums of your signed files in the SFS Browser!"), wxPoint( -1,-1 ), wxSize( -1,-1 ), 0 );
	warningLabel->Wrap( 350 );
	warningSizer->Add( warningLabel, 1, wxALL, 5 );
	
	mainSizer->Add( warningSizer, 0, wxALL|wxEXPAND, 0 );
	
	
	mainSizer->Add( 0, 15, 0, 0, 0 );
	
	wxBoxSizer* buttonSizer;
	buttonSizer = new wxBoxSizer( wxHORIZONTAL );
	
	closeButton = new wxButton( this, wxID_ANY, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
	buttonSizer->Add( closeButton, 0, wxALIGN_CENTER|wxALIGN_CENTER_HORIZONTAL|wxALL, 10 );
	
	
	buttonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	openButton = new wxButton( this, wxID_ANY, _("Open SFS Browser now!"), wxDefaultPosition, wxDefaultSize, 0 );
	buttonSizer->Add( openButton, 0, wxALIGN_CENTER|wxALIGN_CENTER_HORIZONTAL|wxALL, 10 );
	
	mainSizer->Add( buttonSizer, 1, wxEXPAND, 5 );
	
	showAgainCheckBox = new wxCheckBox( this, wxID_ANY, _("Don't show this message again!"), wxDefaultPosition, wxDefaultSize, 0 );
	
	mainSizer->Add( showAgainCheckBox, 0, wxALL|wxEXPAND, 10 );
	
	this->SetSizer( mainSizer );
	this->Layout();
	mainSizer->Fit( this );
	
	// Connect Events
	closeButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgUpgradeAskBase::onClose ), NULL, this );
	openButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgUpgradeAskBase::onSfsBrowserShow ), NULL, this );
	showAgainCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DlgUpgradeAskBase::onUpgradeNotifyCheck ), NULL, this );
}