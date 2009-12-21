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

#ifndef __ModSbPanelsBase__
#define __ModSbPanelsBase__

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/notebook.h>
#include <wx/statbmp.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class ModSbMainPanelBase
///////////////////////////////////////////////////////////////////////////////
class ModSbMainPanelBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* sz_MainSbMain;
		wxStaticText* tx_MainHeadline;
		wxNotebook* note_MainSb;
		wxPanel* pan_Rules;
		wxGrid* lst_Rules;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnGridCellLeftDClick( wxGridEvent& event ){ event.Skip(); }
		
	
	public:
		ModSbMainPanelBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1067,-1 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ModSbOverviewPanelBase
///////////////////////////////////////////////////////////////////////////////
class ModSbOverviewPanelBase : public wxPanel 
{
	private:
	
	protected:
		
		wxStaticBitmap* sbStatusIcon;
		wxStaticText* txt_status;
		wxStaticText* txt_nachfragen;
		wxStaticText* txt_statusValue;
		wxStaticText* txt_requestValue;
	
	public:
		ModSbOverviewPanelBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 600,-1 ), long style = wxTAB_TRAVERSAL );
	
};

#endif //__ModSbPanelsBase__
