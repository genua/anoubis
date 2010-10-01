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

#ifndef __ModPlaygroundPanelsBase__
#define __ModPlaygroundPanelsBase__

class AnListCtrl;

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/notebook.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

#define PG_APP_COMBOBOX 8500
#define PG_APP_STARTBUTTON 8501
#define PG_VIEW_LIST 8502
#define PG_VIEW_REFRESHBUTTON 8503
#define PG_VIEW_COMMITBUTTON 8504
#define PG_COMMIT_LIST 8505
#define PG_FILE_COMMITBUTTON 8506

///////////////////////////////////////////////////////////////////////////////
/// Class ModPlaygroundMainPanelBase
///////////////////////////////////////////////////////////////////////////////
class ModPlaygroundMainPanelBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* mainSizer;
		wxStaticText* mainHeadlineLabel;
		wxNotebook* pgNotebook;
		wxPanel* pgPage;
		wxStaticText* applicationLabel;
		wxComboBox* applicationComboBox;
		wxButton* applicationStartButton;
		AnListCtrl* pgList;
		wxButton* pgRefreshButton;
		
		wxButton* pgCommitButton;
		wxButton* pgDeleteButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onPgNotebookChanging( wxNotebookEvent& event ){ event.Skip(); }
		virtual void onAppPathEntered( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAppStartEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAppStart( wxCommandEvent& event ){ event.Skip(); }
		virtual void onPgListItemActivate( wxListEvent& event ){ event.Skip(); }
		virtual void onPgListItemDeselect( wxListEvent& event ){ event.Skip(); }
		virtual void onPgListItemSelect( wxListEvent& event ){ event.Skip(); }
		virtual void onPgListRefreshClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void onCommitFiles( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDeleteFiles( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		ModPlaygroundMainPanelBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1067,-1 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ModPlaygroundOverviewPanelBase
///////////////////////////////////////////////////////////////////////////////
class ModPlaygroundOverviewPanelBase : public wxPanel 
{
	private:
	
	protected:
		
		wxStaticBitmap* statusIcon;
		wxStaticText* statusLabel;
		wxStaticText* countLabel;
		wxStaticText* statusText;
		wxStaticText* countText;
	
	public:
		ModPlaygroundOverviewPanelBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 600,-1 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgPlaygroundCommitFileListBase
///////////////////////////////////////////////////////////////////////////////
class DlgPlaygroundCommitFileListBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* descriptionText;
		wxStaticText* listTitleText;
		
		wxStaticText* m_staticText13;
		wxTextCtrl* listSearchEntry;
		
		wxButton* columnButton;
		AnListCtrl* fileList;
		wxStaticText* progressText;
		wxButton* commitButton;
		wxButton* delButton;
		
		wxButton* closeButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onCommitSearchKillFocus( wxFocusEvent& event ){ event.Skip(); }
		virtual void onCommitSearchEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void onColumnButtonClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void onCommitListColClick( wxListEvent& event ){ event.Skip(); }
		virtual void onCommitClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDeleteClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void onCloseClicked( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgPlaygroundCommitFileListBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Commit Files"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 640,480 ), long style = wxCAPTION|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgPlaygroundScanResultBase
///////////////////////////////////////////////////////////////////////////////
class DlgPlaygroundScanResultBase : public wxDialog 
{
	private:
	
	protected:
		
		wxStaticBitmap* alertIcon;
		wxStaticText* descriptionLabel;
		wxStaticText* fileNameLabel;
		wxStaticText* questionLabel;
		
		
		wxButton* okButton;
		wxButton* skipButton;
		wxButton* commitButton;
		
		wxBoxSizer* resultSizer;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onOkButtonClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void onSkipButtonClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void onCommitButtonClick( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgPlaygroundScanResultBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Scan results"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 640,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER );
	
};

#endif //__ModPlaygroundPanelsBase__
