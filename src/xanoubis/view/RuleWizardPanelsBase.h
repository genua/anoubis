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

#ifndef __RuleWizardPanelsBase__
#define __RuleWizardPanelsBase__

class AnDetails;

#include <wx/sizer.h>
#include <wx/gdicmn.h>
#include <wx/statline.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/listbox.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/statbmp.h>
#include <wx/listctrl.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


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
		wxStaticText* programLabel;
		wxTextCtrl* programTextCtrl;
		wxButton* pickButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onProgramTextKillFocus( wxFocusEvent& event ){ event.Skip(); }
		virtual void onProgramTextEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void onPickButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardProgramPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,600 ), long style = wxTAB_TRAVERSAL );
	
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
		
		wxStaticText* questionLabel;
		
		wxRadioButton* yesRadioButton;
		
		wxCheckBox* yesExceptionsCheckBox;
		wxRadioButton* noRadioButton;
		
		wxCheckBox* noExceptionsCheckBox;
		wxPanel* contextHelpPage;
		wxStaticText* contextHelpLabel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onYesRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onYesExceptionsCheckBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onNoRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onNoExceptionsCheckBox( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardContextPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,600 ), long style = wxTAB_TRAVERSAL );
	
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
		wxListBox* exceptionListBox;
		wxButton* addButton;
		
		wxButton* deleteButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onAddButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDeleteButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardContextExceptionPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardAlfKeepPolicyPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardAlfKeepPolicyPageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
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
		RuleWizardAlfKeepPolicyPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardAlfClientPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardAlfClientPageBase : public wxPanel 
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
		RuleWizardAlfClientPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,600 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RuleWizardAlfClientPortsPageBase
///////////////////////////////////////////////////////////////////////////////
class RuleWizardAlfClientPortsPageBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* naviSizer;
		wxStaticLine* naviMainDelimiter;
		
		wxStaticText* headLineLabel;
		
		wxStaticText* questionLabel;
		wxStaticText* portListLabel;
		wxListCtrl* portListCtrl;
		wxButton* addButton;
		
		wxButton* defaultsButton;
		
		wxButton* deleteButton;
		AnDetails* detailsPanel;
		wxCheckBox* askCheckBox;
		wxCheckBox* rawCheckBox;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onAddButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDefaultsButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDeleteButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAskCheckBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onRawCheckBox( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardAlfClientPortsPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,600 ), long style = wxTAB_TRAVERSAL );
	
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
		wxListCtrl* serviceListCtrl;
		
		wxButton* addButton;
		wxStaticText* borderLabel;
		wxStaticLine* borderLine;
		wxStaticText* protocolLabel;
		wxRadioButton* tcpRadioButton;
		wxStaticText* portLabel;
		
		
		wxRadioButton* udpRadioButton;
		wxTextCtrl* portTextCtrl;
		wxButton* customAddButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onAddButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		RuleWizardAlfDlgAddServiceBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Rule wizard add service"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 600,400 ), long style = wxDEFAULT_DIALOG_STYLE|wxSTAY_ON_TOP );
	
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
		
	
	public:
		RuleWizardFinalPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,600 ), long style = wxTAB_TRAVERSAL );
	
};

#endif //__RuleWizardPanelsBase__
