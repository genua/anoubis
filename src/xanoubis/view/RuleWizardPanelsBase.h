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

#ifndef __RuleWizardPanelsBase__
#define __RuleWizardPanelsBase__

class AnDetails;
class AnListCtrl;
class AnPickFromFs;

#include <wx/sizer.h>
#include <wx/gdicmn.h>
#include <wx/statline.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/checkbox.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardOverwritePolicyPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardOverwritePolicyPageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
		wxPanel* helpPage;
		wxStaticText* helpLabel;
		wxStaticText* questionLabel;
		wxRadioButton* yesRadioButton;
		wxRadioButton* noRadioButton;
		wxStaticBitmap* alertIcon;
		wxStaticText* alertLabel;
		
		wxStaticText* policyLabel;
		wxTextCtrl* policyTextCtrl;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onYesRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onNoRadioButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardOverwritePolicyPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 980,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardAlfDlgAddServiceBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardAlfDlgAddServiceBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* headLineLabel;
		
		wxTextCtrl* searchTextCtrl;
		wxStaticBitmap* searchIcon;
		AnListCtrl* serviceListCtrl;
		
		wxButton* addButton;
		wxStaticText* borderLabel;
		wxStaticLine* borderLine;
		wxStaticText* protocolLabel;
		wxRadioButton* tcpRadioButton;
		wxRadioButton* udpRadioButton;
		
		wxStaticText* portLabel;
		wxTextCtrl* portTextCtrl;
		wxButton* customAddButton;
		
		wxStaticText* portHelpLabel;
		wxStaticLine* tailLine;
		
		wxButton* cancelButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onSearchTextEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void onServiceListDeselect( wxListEvent& event ){ event.Skip(); }
		virtual void onServiceListSelect( wxListEvent& event ){ event.Skip(); }
		virtual void onAddButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onCustomAddButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onCancelButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardAlfDlgAddServiceBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Rule wizard add service"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,600 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardProgramPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardProgramPageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
		wxStaticText* staticTextCsum;
		wxStaticText* programLabel;
		AnPickFromFs* programPicker;
		
	
	public:
		RuleWizardProgramPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 980,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardContextPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardContextPageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
		wxPanel* helpPage;
		wxStaticText* helpLabel;
		wxStaticText* questionLabel;
		
		wxRadioButton* yesRadioButton;
		
		wxCheckBox* exceptionsCheckBox;
		wxRadioButton* noRadioButton;
		
		AnDetails* detailsPanel;
		wxCheckBox* noSfsCheckbox;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onYesRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onExceptionsCheckBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onNoRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onSfsDisable( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardContextPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 980,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardContextExceptionPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardContextExceptionPageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
		wxStaticText* questionLabel;
		
		wxStaticText* exceptionListLabel;
		AnListCtrl* exceptionList;
		wxButton* addButton;
		
		wxButton* deleteButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onAddButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDeleteButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardContextExceptionPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 980,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardAlfPermissionPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardAlfPermissionPageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
		wxStaticText* questionLabel;
		
		wxRadioButton* yesRadioButton;
		wxRadioButton* defaultRadioButton;
		
		wxStaticText* defaultLabel;
		wxRadioButton* restrictedRadioButton;
		
		wxStaticText* restrictedLabel;
		wxRadioButton* noRadioButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onYesRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDefaultRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onRestrictedRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onNoRadioButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardAlfPermissionPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 980,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardAlfServicePageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardAlfServicePageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
		wxStaticText* questionLabel;
		wxStaticText* portListLabel;
		AnListCtrl* portListCtrl;
		wxButton* addButton;
		
		wxButton* defaultsButton;
		
		wxButton* deleteButton;
		AnDetails* detailsPanel;
		wxCheckBox* askCheckBox;
		wxCheckBox* rawCheckBox;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onPortListDeselect( wxListEvent& event ){ event.Skip(); }
		virtual void onPortListSelect( wxListEvent& event ){ event.Skip(); }
		virtual void onAddButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDefaultsButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDeleteButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAskCheckBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onRawCheckBox( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardAlfServicePageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 980,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardSandboxPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardSandboxPageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
		wxPanel* helpPage;
		wxStaticText* helpLabel;
		wxStaticText* questionLabel;
		
		wxRadioButton* yesWizardRadioButton;
		wxRadioButton* yesDefaultsRadioButton;
		wxRadioButton* noRadioButton;
		
		
		// Virtual event handlers, overide them in your derived class
		virtual void onYesWizardRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onYesDefaultsRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onNoRadioButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardSandboxPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 980,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardSandboxPermissionPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardSandboxPermissionPageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
		wxStaticText* questionLabel;
		
		wxRadioButton* allowAllRadioButton;
		wxRadioButton* defaultRadioButton;
		
		wxStaticText* defaultLabel;
		wxRadioButton* restrictedRadioButton;
		
		wxStaticText* restrictedLabel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onAllowAllRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDefaultRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onRestrictedRadioButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardSandboxPermissionPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 980,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardSandboxFilesPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardSandboxFilesPageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
		wxStaticText* questionLabel;
		wxStaticText* fileListLabel;
		AnListCtrl* fileListCtrl;
		wxStaticText* add_label;
		wxButton* addFileButton;
		wxButton* addDirectoryButton;
		wxButton* defaultsButton;
		
		wxButton* deleteButton;
		AnDetails* detailsPanel;
		wxCheckBox* askCheckBox;
		wxCheckBox* validCheckBox;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onFileListDeselect( wxListEvent& event ){ event.Skip(); }
		virtual void onFileListSelect( wxListEvent& event ){ event.Skip(); }
		virtual void onAddFileButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAddDirectoryButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDefaultsButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDeleteButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAskCheckBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onValidCheckBox( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardSandboxFilesPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 980,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardFinalPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardFinalPageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
		wxStaticText* finalLabel;
		wxCheckBox* activatePolicyCheckbox;
		
		
		// Virtual event handlers, overide them in your derived class
		virtual void onActivatePolicyCheckBox( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardFinalPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 980,600 ), long style = wxTAB_TRAVERSAL );
	
};

#endif //__RuleWizardPanelsBase__
