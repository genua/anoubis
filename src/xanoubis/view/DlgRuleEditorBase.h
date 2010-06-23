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

#ifndef __DlgRuleEditorBase__
#define __DlgRuleEditorBase__

class AnGrid;
class AnPickFromFs;
class AnPolicyNotebook;
class DlgRuleEditorAppPage;
class DlgRuleEditorFilterSubjectPage;

#include <wx/string.h>
#include <wx/radiobut.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/grid.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/splitter.h>
#include <wx/frame.h>
#include <wx/checkbox.h>
#include <wx/scrolwin.h>
#include <wx/radiobox.h>

///////////////////////////////////////////////////////////////////////////

#define ID_APP_PAGE_ADD 1000
#define ID_APP_PAGE_DELETE 1001

///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorBase : public wxFrame 
{
	private:
	
	protected:
		wxRadioButton* rb_userMe;
		wxRadioButton* rb_userSelect;
		wxTextCtrl* tx_userSelect;
		wxRadioButton* rb_userDefault;
		wxSplitterWindow* splitterWindow;
		wxPanel* appPanel;
		wxStaticText* appListTypeLabel;
		wxChoice* appListTypeChoice;
		wxButton* appListCreateButton;
		
		wxButton* appListColumnsButton;
		AnGrid* appGrid;
		
		wxStaticText* appListPolicyLabel;
		wxStaticText* appListPolicyText;
		wxButton* appListUpButton;
		wxButton* appListDownButton;
		
		wxButton* appListDeleteButton;
		AnPolicyNotebook* appPolicyPanels;
		wxPanel* filterPanel;
		wxStaticText* filterListTypeLabel;
		wxChoice* filterListTypeChoice;
		wxButton* filterListCreateButton;
		
		wxButton* filterListColumnsButton;
		AnGrid* filterGrid;
		
		wxStaticText* filterListPolicyLabel;
		wxStaticText* filterListPolicyText;
		wxButton* filterListUpButton;
		wxButton* filterListDownButton;
		
		wxButton* filterListDeleteButton;
		AnPolicyNotebook* filterPolicyPanels;
		wxStaticText* footerRuleSetLabel;
		wxStaticText* footerRuleSetText;
		
		wxStaticText* footerStatusLabel;
		wxStaticText* footerStatusText;
		wxButton* footerImportButton;
		wxButton* footerReloadButton;
		
		wxButton* footerExportButton;
		wxButton* footerActivateButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onClose( wxCloseEvent& event ){ event.Skip(); }
		virtual void onRbUserMe( wxCommandEvent& event ){ event.Skip(); }
		virtual void onRbUserSelect( wxCommandEvent& event ){ event.Skip(); }
		virtual void onUserSelectKillFocus( wxFocusEvent& event ){ event.Skip(); }
		virtual void onUserSelectTextEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void onRbUserDefault( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAppListCreateButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAppListColumnsButtonClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAppGridCellSelect( wxGridEvent& event ){ event.Skip(); }
		virtual void onAppListUpClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAppListDownClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAppListDeleteClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void onFilterListCreateButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onFilterListColumnsButtonClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void onFilterGridCellSelect( wxGridEvent& event ){ event.Skip(); }
		virtual void onFilterListUpClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void onFilterListDownClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void onFilterListDeleteClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void onFooterImportButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onFooterReloadButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onFooterExportButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onFooterActivateButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Anoubis Rule Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1000,900 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		void splitterWindowOnIdle( wxIdleEvent& )
		{
		splitterWindow->SetSashPosition( 482 );
		splitterWindow->Disconnect( wxEVT_IDLE, wxIdleEventHandler( DlgRuleEditorBase::splitterWindowOnIdle ), NULL, this );
		}
		
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorAppPageBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorAppPageBase : public wxPanel 
{
	private:
	
	protected:
		wxScrolledWindow* mainPage;
		DlgRuleEditorFilterSubjectPage* subjPage;
		wxCheckBox* noSfsCheckbox;
		wxButton* addButton;
		wxButton* deleteButton;
		
		
		// Virtual event handlers, overide them in your derived class
		virtual void onNoSfsClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAddButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDeleteButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorAppPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,320 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorFilterActionPageBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorFilterActionPageBase : public wxPanel 
{
	private:
	
	protected:
		wxScrolledWindow* mainPage;
		wxStaticText* actionLabel;
		wxRadioButton* allowRadioButton;
		wxRadioButton* denyRadioButton;
		wxRadioButton* askRadioButton;
		wxStaticText* logLabel;
		wxRadioButton* noneRadioButton;
		wxRadioButton* normalRadioButton;
		wxRadioButton* alertRadioButton;
		AnPickFromFs* defaultPathPicker;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onAllowRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDenyRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAskRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onNoneRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onNormalRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAlertRadioButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorFilterActionPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,320 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorFilterNetworkPageBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorFilterNetworkPageBase : public wxPanel 
{
	private:
	
	protected:
		wxScrolledWindow* mainPage;
		wxStaticText* directionLabel;
		wxRadioButton* inRadioButton;
		wxRadioButton* outRadioButton;
		wxRadioButton* bothRadioButton;
		wxStaticText* protocolLabel;
		wxRadioButton* tcpRadioButton;
		wxRadioButton* udpRadioButton;
		wxRadioButton* sctpRadioButton;
		
		
		// Virtual event handlers, overide them in your derived class
		virtual void onInRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onOutRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onBothRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onTcpRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onUdpRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onSctpRadioButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorFilterNetworkPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,320 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorFilterAddressPageBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorFilterAddressPageBase : public wxPanel 
{
	private:
	
	protected:
		wxScrolledWindow* mainPage;
		wxFlexGridSizer* mainSizer;
		wxStaticText* sourceAddressLabel;
		wxTextCtrl* sourceAddressTextCtrl;
		wxStaticText* sourcePortLabel;
		wxTextCtrl* sourcePortTextCtrl;
		wxStaticText* destinationAddressLabel;
		wxTextCtrl* destinationAddressTextCtrl;
		wxStaticText* destinationPortLabel;
		wxTextCtrl* destinationPortTextCtrl;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onSourceAddressTextKillFocus( wxFocusEvent& event ){ event.Skip(); }
		virtual void onSourceAddressTextEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void onSourcePortTextKillFocus( wxFocusEvent& event ){ event.Skip(); }
		virtual void onSourcePortTextEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDestinationAddressTextKillFocus( wxFocusEvent& event ){ event.Skip(); }
		virtual void onDestinationAddressTextEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void onDestinationPortTextKillFocus( wxFocusEvent& event ){ event.Skip(); }
		virtual void onDestinationPortTextEnter( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorFilterAddressPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,320 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorFilterCapabilityPageBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorFilterCapabilityPageBase : public wxPanel 
{
	private:
	
	protected:
		wxScrolledWindow* mainPage;
		wxStaticText* capabilityLabel;
		wxRadioButton* rawRadioButton;
		wxRadioButton* otherRadioButton;
		wxRadioButton* allRadioButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onRawRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onOtherRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAllRadioButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorFilterCapabilityPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,320 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorFilterSubjectPageBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorFilterSubjectPageBase : public wxPanel 
{
	private:
	
	protected:
		wxScrolledWindow* scrollPanel;
		AnPickFromFs* pathPicker;
		wxStaticText* subjectLabel;
		wxChoice* subjectChoice;
		
		
		wxTextCtrl* subjectTextCtrl;
		
		
		// Virtual event handlers, overide them in your derived class
		virtual void onSubjectSelected( wxCommandEvent& event ){ event.Skip(); }
		virtual void onSubjectTextKillFocus( wxFocusEvent& event ){ event.Skip(); }
		virtual void onSubjectTextEnter( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorFilterSubjectPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorFilterSfsPageBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorFilterSfsPageBase : public wxPanel 
{
	private:
	
	protected:
		wxScrolledWindow* mainPage;
		wxStaticText* validLabel;
		
		wxRadioBox* validActionRadioBox;
		wxRadioBox* validLogRadioBox;
		
		
		wxStaticText* invalidLabel;
		
		wxRadioBox* invalidActionRadioBox;
		wxRadioBox* invalidLogRadioBox;
		
		
		wxStaticText* unknownLabel;
		
		wxRadioBox* unknownActionRadioBox;
		wxRadioBox* unknownLogRadioBox;
		
		
		
		// Virtual event handlers, overide them in your derived class
		virtual void onValidActionRadioBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onValidLogRadioBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onInvalidActionRadioBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onInvalidLogRadioBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onUnknownActionRadioBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onUnknownLogRadioBox( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorFilterSfsPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,320 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorFilterContextPageBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorFilterContextPageBase : public wxPanel 
{
	private:
	
	protected:
		wxScrolledWindow* scrollPanel;
		wxStaticText* typeLabel;
		wxRadioButton* newRadioButton;
		wxRadioButton* openRadioButton;
		DlgRuleEditorAppPage* appPage;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onNewRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onOpenRadioButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorFilterContextPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorFilterPermissionPageBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorFilterPermissionPageBase : public wxPanel 
{
	private:
	
	protected:
		wxScrolledWindow* mainPage;
		wxStaticText* permissionLabel;
		wxCheckBox* readCheckBox;
		wxCheckBox* writeCheckBox;
		wxCheckBox* executeCheckBox;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onReadCheckBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onWriteCheckBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void onExecuteCheckBox( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorFilterPermissionPageBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,320 ), long style = wxTAB_TRAVERSAL );
	
};

#endif //__DlgRuleEditorBase__
