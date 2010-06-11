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

#include "AnPickFromFs.h"

#include "ModPlaygroundPanelsBase.h"

///////////////////////////////////////////////////////////////////////////

ModPlaygroundMainPanelBase::ModPlaygroundMainPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	mainHeadlineLabel = new wxStaticText( this, wxID_ANY, _("PG - Playground"), wxDefaultPosition, wxDefaultSize, 0 );
	mainHeadlineLabel->Wrap( -1 );
	mainHeadlineLabel->SetFont( wxFont( 16, 70, 90, 90, false, wxEmptyString ) );
	
	mainSizer->Add( mainHeadlineLabel, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxStaticBoxSizer* startSizer;
	startSizer = new wxStaticBoxSizer( new wxStaticBox( this, -1, _("New Playground") ), wxVERTICAL );
	
	programPicker = new AnPickFromFs( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	startSizer->Add( programPicker, 0, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	startButton = new wxButton( this, wxID_ANY, _("Start Playground"), wxDefaultPosition, wxDefaultSize, 0 );
	startSizer->Add( startButton, 0, wxALL, 5 );
	
	mainSizer->Add( startSizer, 0, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( mainSizer );
	this->Layout();
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
