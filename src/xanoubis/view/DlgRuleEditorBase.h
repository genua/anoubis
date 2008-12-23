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
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/scrolwin.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/notebook.h>
#include <wx/radiobut.h>
#include <wx/spinctrl.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DlgRuleEditorBase
///////////////////////////////////////////////////////////////////////////////
class DlgRuleEditorBase : public wxFrame 
{
	private:
	
	protected:
		wxStaticText* appListTypeLabel;
		wxChoice* appListTypeChoice;
		wxButton* appListCreateButton;
		
		wxButton* appListColumnsButton;
		wxListCtrl* appPolicyListCtrl;
		
		wxStaticText* appListPolicyLabel;
		wxStaticText* appListPolicyText;
		wxButton* appListUpButton;
		wxButton* appListDownButton;
		wxButton* appListDeleteButton;
		wxStaticText* filterListTypeLabel;
		wxChoice* filterListTypeChoice;
		wxButton* filterListCreateButton;
		
		wxButton* filterListColumnsButton;
		wxListCtrl* filterPolicyListCtrl;
		
		wxStaticText* filterListPolicyLabel;
		wxStaticText* filterListPolicyText;
		wxButton* filterListUpButton;
		wxButton* filterListDownButton;
		wxButton* filterListDeleteButton;
		wxNotebook* appPolicyPanels;
		wxScrolledWindow* appBinaryPage;
		wxStaticText* appBinaryLabel;
		wxTextCtrl* appBinaryTextCtrl;
		wxButton* appBinaryModifyButton;
		wxStaticText* appBinaryCsumRegLabel;
		wxStaticText* appBinaryCsumRegText;
		wxStaticText* appBinaryCsumCurLabel;
		wxStaticText* appBinaryCsumCurText;
		wxStaticText* appBinaryStatusLabel;
		wxStaticText* appBinaryStatusText;
		wxButton* appBinaryValidateButton;
		wxButton* appBinaryUpdateButton;
		wxNotebook* filterPolicyPanels;
		wxScrolledWindow* filterCommonPage;
		wxStaticText* filterCommonActionText;
		wxRadioButton* filterCommonAllowRadioButton;
		wxRadioButton* filterCommonDenyRadioButton;
		wxRadioButton* filterCommonAskRadioButton;
		wxStaticText* filterCommonLogText;
		wxRadioButton* filterCommonLogNoneRadioButton;
		wxRadioButton* filterCommonLogNormalRadioButton;
		wxRadioButton* filterCommonLogAlertRadioButton;
		wxScrolledWindow* filterNetworkPage;
		wxStaticText* filterNetworkDirectionLabel;
		wxRadioButton* filterNetworkInRadioButton;
		wxRadioButton* filterNetworkOutRadionButton;
		wxRadioButton* filterNetworkBothRadioButton;
		wxStaticText* filterNetworkAddrFamilyLabel;
		wxRadioButton* filterNetworkInetRadioButton;
		wxRadioButton* filterNetworkInet6RadioButton;
		wxRadioButton* filterNetworkAnyRadioButton;
		wxStaticText* filterNetworkProtocolLabel;
		wxRadioButton* filterNetworkTcpRadioButton;
		wxRadioButton* filterNetworkUdpRadioButton;
		
		wxStaticText* filterNetworkStateTimeoutLabel;
		wxSpinCtrl* filterNetworkStateTimeoutSpinCtrl;
		wxScrolledWindow* filterAddressPage;
		wxFlexGridSizer* filterAddressMainSizer;
		wxStaticText* filterAddressSourceLabel;
		wxTextCtrl* filterAddressSourceTextCtrl;
		wxStaticText* filterAddressSourceDelimiterLabel;
		wxSpinCtrl* filterAddressSoruceNetSpinCtrl;
		wxStaticText* filterAddressSourcePortLabel;
		wxTextCtrl* filterAddressSorucePortTextCtrl;
		
		
		wxStaticText* filterAddressDestinationLabel;
		wxTextCtrl* filterAddressDestinationTextCtrl;
		wxStaticText* filterAddressDestinationDelimiterLabel;
		wxSpinCtrl* filterAddressDestinationNetSpinCtrl;
		wxStaticText* filterAddressDestinationPortLabel;
		wxTextCtrl* filterAddressDestinationPortTextCtrl;
		wxScrolledWindow* filterCapabilityPage;
		wxStaticText* filterCapabilityLabel;
		wxRadioButton* filterCapabilityRawRadioButton;
		wxRadioButton* filterCapabilityOtherRadioButton;
		wxRadioButton* filterCapabilityAllRadioButton;
		wxScrolledWindow* filterSubjectPage;
		wxStaticText* filterSubjectPathLabel;
		wxTextCtrl* filterSubjectPathTextCtrl;
		wxButton* filterSubjectPathModifyButton;
		wxStaticText* filterSubjectLabel;
		wxRadioButton* filterSubjectAnyRadioButton;
		
		
		wxRadioButton* filterSubjectSelfRadioButton;
		
		
		wxRadioButton* filterSubjectSelfSignedRadioButton;
		
		
		wxRadioButton* filterSubjectUidRadioButton;
		wxTextCtrl* filterSubjectUidTextCtrl;
		
		
		wxRadioButton* filterSubjectKeyLRadioButton;
		wxTextCtrl* filterSubjectKeyTextCtrl;
		
		
		wxRadioButton* filterSubjectCsumRadioButton;
		wxTextCtrl* filterSubjectCsumTextCtrl;
		wxScrolledWindow* filterSfsPage;
		wxStaticText* filterSfsValidLabel;
		
		
		
		wxRadioBox* filterSfsValidActionRadioBox;
		wxRadioBox* filterSfsValidLogRadioBox;
		wxStaticText* filterSfsInvalidLabel;
		
		
		
		wxRadioBox* filterSfsInvalidActionRadioBox;
		wxRadioBox* filterSfsInvalidLogRadioBox;
		wxStaticText* filterSfsUnknownLabel;
		
		
		
		wxRadioBox* filterSfsUnknownActionRadioBox;
		wxRadioBox* filterSfsUnknownLogRadioBox;
		wxScrolledWindow* filterContextPage;
		wxStaticText* filterContextTypeLabel;
		wxRadioButton* filterContextNewRadioButton;
		wxRadioButton* filterContextOpenRadioButton;
		
		
		wxStaticText* filterContextBinaryLabel;
		wxTextCtrl* filterContextBinaryTextCtrl;
		wxButton* filterContextBinaryModifyButton;
		wxStaticText* filterContextCsumRegLabel;
		wxStaticText* filterContextCsumRegText;
		wxStaticText* filterContextCsumCurLabel;
		wxStaticText* filterContextCsumCurText;
		wxStaticText* filterContextStatusLabel;
		wxStaticText* filterContextStatusText;
		wxButton* filterContextValidateButton;
		wxButton* filterContextUpdateButton;
		wxScrolledWindow* filterPermissionPage;
		wxStaticText* filterPermissionLabel;
		wxCheckBox* filterPermissionReadCheckBox;
		wxCheckBox* filterPermissionWriteCheckBox;
		wxCheckBox* filterPermissionExecuteCheckBox;
		wxStaticText* mainFooterRuleSetLabel;
		wxStaticText* mainFooterRuleSetText;
		wxButton* mainFooterReloadButton;
		
		wxStaticText* mainFooterStatusLabel;
		wxStaticText* mainFooterStatusText;
		wxButton* mainFooterSaveButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnAppBinaryTextCtrl( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAppBinaryModifyButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAppValidateChkSumButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAppUpdateChkSumButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfAllowRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfDenyRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfAskRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfFilterRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfCapRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfDefaultRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfAcceptRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfConnectRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfInetRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfInet6RadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfAnyRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfTcpRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfUdpRadioButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfStateTimeoutChange( wxSpinEvent& event ){ event.Skip(); }
		virtual void onAlfSrcAddrTextCtrlEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfSrcNetmaskSpinCtrl( wxSpinEvent& event ){ event.Skip(); }
		virtual void onAlfSrcPortTextCtrlEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAlfDstAddrTextCtrlEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlfDstNetmaskSpinCtrl( wxSpinEvent& event ){ event.Skip(); }
		virtual void onAlfDstPortTextCtrlEnter( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DlgRuleEditorBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Anoubis Rule Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1000,700 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	
};

#endif //__DlgRuleEditorBase__
