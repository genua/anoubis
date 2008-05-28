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

#ifndef __DlgRuleEditorBase__
#define __DlgRuleEditorBase__

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/spinctrl.h>
#include "AnFader.h"
#include <wx/statbox.h>
#include <wx/scrolwin.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/combobox.h>
#include <wx/notebook.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorBase : public wxFrame 
{
	private:
	
	protected:
		wxStaticText* controlRuleText;
		wxChoice* controlCreationChoice;
		wxButton* controlRuleCreateButton;
		wxStaticText* controlRuleText1;
		wxButton* controlRuleDeleteButton;
		
		wxStaticText* controlFilterText;
		wxTextCtrl* controlFilterTextCtrl;
		wxStaticText* controlFilterInText;
		wxChoice* controlFilterChoice;
		wxStaticText* controlRuleSetText;
		wxButton* controlRuleSetSaveButton;
		
		wxStaticText* controlOptionText;
		wxButton* controlOptionButton;
		wxListCtrl* ruleListCtrl;
		wxNotebook* ruleEditNotebook;
		wxScrolledWindow* commonNbPanel;
		wxStaticText* commonModuleText;
		wxChoice* commonModuleChoice;
		wxStaticText* commonStateText;
		wxRadioButton* commonActiveRadioButton;
		wxRadioButton* commonDeactiveRadioButton;
		wxStaticText* commonNameText;
		wxTextCtrl* commonNameTextCtrl;
		wxStaticText* commonPriorityText;
		wxSpinCtrl* commonPrioritySpinCtrl;
		wxStaticText* commonFaderText;
		AnFader *commonFader;
		wxStaticText* commonCommentText;
		wxTextCtrl* commonCommentTextCtrl;
		wxRadioButton* commonProcEndRadioButton;
		wxRadioButton* commonTimeRadioButton;
		wxSpinCtrl* commonTimeSpinCtrl;
		wxChoice* commonTimeUnitChoice;
		wxRadioButton* commonAlwaysRadioButton;
		wxRadioButton* commonNoneLogRadioButton;
		wxRadioButton* commonDoLogRadioButton;
		wxRadioButton* commonAlertLogRadioButton;
		wxStaticText* commonCreatedText;
		wxStaticText* commonCreateTextValue;
		wxStaticText* commonModifiedText;
		wxStaticText* commonModifiedTextValue;
		wxStaticText* commonModificatorText;
		wxStaticText* commonModificatorTextValue;
		wxScrolledWindow* applicationNbPanel;
		wxStaticText* appNameText;
		wxComboBox* appNameComboBox;
		
		
		
		wxStaticText* appBinaryText;
		wxButton* appGuessButton;
		wxStaticText* appGuessText;
		
		
		
		
		wxTextCtrl* appBinaryTextCtrl;
		wxButton* appBinaryModifyButton;
		
		wxButton* appBinaryAddButton;
		wxStaticText* appInheritanceText;
		wxTextCtrl* appInheritanceTextCtrl;
		wxButton* appInheritanceModifyButton;
		
		wxButton* appInheritanceAddButton;
		wxScrolledWindow* alfNbPanel;
		wxStaticText* alfActionText;
		wxRadioButton* alfAllowRadioButton;
		wxRadioButton* alfDenyRadioButton;
		wxRadioButton* alfAskRadioButton;
		wxStaticText* alfTypeText;
		wxRadioButton* alfFilterRadioButton;
		wxRadioButton* alfCapRadioButton;
		wxRadioButton* alfDefaultRadioButton;
		wxStaticText* alfProtocolText;
		wxRadioButton* alfTcpRadioButton;
		wxRadioButton* alfUdpRadioButton;
		
		wxStaticText* alfAddrFamilyText;
		wxRadioButton* alfInetRadioButton;
		wxRadioButton* alfInet6RadioButton;
		wxRadioButton* alfAnyRadioButton;
		wxStaticText* alfCapText;
		wxRadioButton* alfRawCapRadioButton;
		wxRadioButton* alfOtherCapRadioButton;
		wxRadioButton* alfAllCapRadioButton;
		wxStaticText* alfDirectionText;
		wxRadioButton* alfAcceptRadioButton;
		wxRadioButton* alfConnectRadioButton;
		wxFlexGridSizer* alfConnectAddrSizer;
		wxStaticText* alfSrcAddrText;
		wxComboBox* alfSrcAddrComboBox;
		wxStaticText* alfSrcAddrDelimiterText;
		wxSpinCtrl* alfSrcAddrNetSpinCtrl;
		wxButton* alfSrcAddrDelButton;
		wxButton* alfSrcAddrAddButton;
		wxStaticText* alfDstAddrText;
		wxComboBox* alfDstAddrComboBox;
		wxStaticText* alfDstAddrDelimiterText;
		wxSpinCtrl* alfDstAddrNetSpinCtrl;
		wxButton* alfDstAddrDelButton;
		wxButton* alfDstAddrAddButton;
		wxStaticText* alfSrcPortText;
		wxComboBox* alfSrcPortComboBox;
		
		
		
		
		wxStaticText* alfDstPortText;
		wxComboBox* alfDstPortComboBox;
		
		
		
		
		wxStaticText* alfStateTimeoutText;
		wxSpinCtrl* alfStateTimeoutSpinCtrl;
		wxScrolledWindow* sfsNbPanel;
		wxStaticText* sfsBinaryLabelText;
		wxTextCtrl* sfsBinaryTextCtrl;
		wxButton* sfsBinaryModifyButton;
		wxStaticText* sfsRegisteredSumLabelText;
		wxStaticText* sfsRegisteredSumValueText;
		
		wxStaticText* sfsCurrentSumLabelText;
		wxStaticText* sfsCurrentSumValueText;
		
		wxStaticText* sfsStatusLabelText;
		wxStaticText* sfsStatusValueText;
		wxButton* sfsUpdateChkSumButton;
		wxScrolledWindow* macroNbPanel;
		wxStaticText* macroTypeLabelText;
		wxChoice* macroTypeChoice;
		wxStaticText* macroValueLabelText;
		wxTextCtrl* macroValueextCtrl;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ){ event.Skip(); }
		virtual void OnRuleCreateButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRuleDeleteButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRuleSetSave( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnTableOptionButtonClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnLineSelected( wxListEvent& event ){ event.Skip(); }
		virtual void OnBinaryModifyButtonClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfAllowRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfDenyRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfAskRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfFilterRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfCapRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfDefaultRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfTcpRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfUdpRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfInetRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfInet6RadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfAnyRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfRawCapRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfOtherCapRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfAllCapRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfAcceptRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfConnectRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSrcAddrAddButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfStateTimeoutChange( wxSpinEvent& event ){ event.Skip(); }
		virtual void OnSfsBinaryModifyButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSfsUpdateChkSumButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Anoubis Rule Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	
};

#endif //__DlgRuleEditorBase__
