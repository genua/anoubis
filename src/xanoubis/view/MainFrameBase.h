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

#ifndef __MainFrameBase__
#define __MainFrameBase__

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/toolbar.h>
#include <wx/scrolwin.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

#define ID_MAINFRAME 1500
#define ID_MENUBAR 1501
#define ID_MIFILECONNECT 1502
#define ID_MIFILEIMPORT 1503
#define ID_MIFILEEXPORT 1504
#define ID_MIFILECLOSE 1505
#define ID_MIFILEQUIT 1506
#define ID_MIEDITPREFERENCES 1507
#define ID_MITOOLSRULEEDITOR 1508
#define ID_MITOOLSLOGVIEWER 1509
#define ID_MITOOLSWIZARD 1510
#define ID_MIHELPABOUT 1511
#define ID_TOOLBAR 1512

///////////////////////////////////////////////////////////////////////////////
/// Class MainFrameBase
///////////////////////////////////////////////////////////////////////////////
class MainFrameBase : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* an_menubar;
		wxMenu* me_menubarFile;
		wxMenu* me_menubarEdit;
		wxMenu* me_menubarTools;
		wxMenu* me_menubarHelp;
		wxBoxSizer* sz_mainframeMain;
		wxStaticBitmap* statusBoxMsgIcon;
		wxStaticText* statusBoxMsgLable;
		wxStaticText* statusBoxMsgText;
		wxStaticBitmap* statusBoxComIcon;
		wxStaticText* statusBoxComLabel;
		wxStaticText* statusBoxComText;
		wxStaticLine* li_MainLineLeft;
		wxScrolledWindow* sw_MainLeftToolbar;
		wxToolBar* tb_LeftToolbarModule;
		
		wxStaticLine* li_MainLineRight;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ){ event.Skip(); }
		virtual void OnIdle( wxIdleEvent& event ){ event.Skip(); }
		virtual void OnMbFileConnectSelect( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMbFileImportSelect( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMbFileExportSelect( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMbFileCloseSelect( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMbFileQuitSelect( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMbEditPreferencesSelect( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMbToolsRuleEditorSelect( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMbToolsLogViewerSelect( wxCommandEvent& event ){ event.Skip(); }
		virtual void onMbToolsWizardSelect( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMbHelpAboutSelect( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		MainFrameBase( wxWindow* parent, wxWindowID id = ID_MAINFRAME, const wxString& title = _("Anoubis"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 950,700 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	
};

#endif //__MainFrameBase__
