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

#ifndef __ModAnoubisPanelsBase__
#define __ModAnoubisPanelsBase__

class ProfileListCtrl;
class VersionListCtrl;

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/radiobut.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/scrolwin.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/notebook.h>
#include <wx/statbmp.h>
#include <wx/combobox.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

#define MAIN_CONNECTBUTTON 13100

///////////////////////////////////////////////////////////////////////////////
/// Class ModAnoubisMainPanelBase
///////////////////////////////////////////////////////////////////////////////
class ModAnoubisMainPanelBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* sz_MainAnoubisMain;
		wxStaticText* tx_MainHeadline;
		wxNotebook* tb_MainAnoubisNotify;
		wxScrolledWindow* tb_MainAnoubisNotification;
		wxStaticText* tx_type;
		wxChoice* ch_type;
		
		wxButton* bt_first;
		wxButton* bt_previous;
		wxStaticText* tx_currNumber;
		wxStaticText* tx_delimiter;
		wxStaticText* tx_maxNumber;
		wxButton* bt_next;
		wxButton* bt_last;
		
		wxStaticText* slotLabelText1;
		wxStaticText* slotValueText1;
		wxStaticText* slotLabelText2;
		wxStaticText* slotValueText2;
		wxStaticText* slotLabelText3;
		wxStaticText* slotValueText3;
		wxStaticText* slotLabelText4;
		wxStaticText* slotValueText4;
		wxStaticText* slotLabelText5;
		wxStaticText* slotValueText5;
		wxStaticText* slotLabelText6;
		wxStaticText* slotValueText6;
		wxStaticText* tx_answerValue;
		wxPanel* pn_Escalation;
		wxPanel* pn_EscalationOptions;
		
		wxRadioButton* rb_EscalationOnce;
		wxRadioButton* rb_EscalationProcess;
		wxRadioButton* rb_EscalationTime;
		wxSpinCtrl* spin_EscalationTime;
		wxChoice* ch_EscalationTimeUnit;
		wxRadioButton* rb_EscalationAlways;
		
		wxStaticLine* m_staticline2;
		wxPanel* pn_EscalationAlf;
		
		wxRadioButton* rb_EscalationAlf1;
		wxRadioButton* rb_EscalationAlf2;
		wxRadioButton* rb_EscalationAlf3;
		wxRadioButton* rb_EscalationAlf4;
		
		wxPanel* pn_EscalationSb;
		
		wxStaticText* tx_EscalationSbDesc;
		wxButton* bt_EscalationSbPathLeft;
		
		wxStaticText* tx_EscalationSbPath;
		
		wxButton* bt_EscalationSbPathRight;
		
		wxCheckBox* ck_EscalationSbRead;
		
		wxCheckBox* ck_EscalationSbWrite;
		
		wxCheckBox* ck_EscalationSbExec;
		
		
		wxPanel* pn_EscalationSfs;
		
		wxStaticText* tx_EscalationSfsDesc;
		wxButton* bt_EscalationSfsPathLeft;
		
		wxStaticText* tx_EscalationSfsPath;
		
		wxButton* bt_EscalationSfsPathRight;
		
		
		wxCheckBox* ck_EscalationEditor;
		
		wxButton* bt_EscalationAllow;
		
		wxButton* bt_EscalationDeny;
		
		wxScrolledWindow* tb_Profiles;
		ProfileListCtrl* profileListCtrl;
		wxStaticText* m_staticText35;
		wxStaticText* selectedProfileText;
		
		wxButton* profileDeleteButton;
		wxStaticText* m_staticText38;
		wxStaticText* loadedProfileText;
		wxButton* profileLoadButton;
		wxButton* profileSaveButton;
		
		wxStaticLine* m_staticline1;
		wxStaticText* m_staticText40;
		wxButton* profileActivateButton;
		wxScrolledWindow* tb_MainAnoubisVersions;
		wxStaticText* m_staticText41;
		wxRadioButton* VersionActivePolicyRadioButton;
		wxRadioButton* VersionProfilePolicyRadioButton;
		wxChoice* VersionProfileChoice;
		VersionListCtrl* versionListCtrl;
		wxTextCtrl* VersionShowCommentTextCtrl;
		
		wxButton* VersionRestoreButton;
		wxButton* VersionExportButton1;
		wxButton* VersionDeleteButton;
		wxButton* VersionShowButton;
		
		wxScrolledWindow* tb_MainAnoubisOptions;
		wxCheckBox* cb_SendEscalations;
		
		wxCheckBox* cb_NoEscalationTimeout;
		
		wxSpinCtrl* m_spinEscalationNotifyTimeout;
		wxStaticText* tx_EscalationNotifyTimeoutLabel;
		wxCheckBox* cb_SendAlerts;
		
		wxCheckBox* cb_NoAlertTimeout;
		
		wxSpinCtrl* m_spinAlertNotifyTimeout;
		wxStaticText* tx_AlertNotifyTimeoutLabel;
		wxCheckBox* cb_ShowUpgradeMsg;
		wxCheckBox* cb_ShowKernelMsg;
		wxCheckBox* cb_ShowKeyGenInfoMsg;
		wxCheckBox* cb_DoAutostart;
		
		wxCheckBox* autoConnectBox;
		wxCheckBox* toolTipCheckBox;
		wxSpinCtrl* toolTipSpinCtrl;
		wxStaticText* m_staticText411;
		wxScrolledWindow* tb_PsBrowser;
		
		wxButton* psReloadButton;
		wxButton* psColumnButton;
		wxListCtrl* psList;
		wxNotebook* psInfoBook;
		wxScrolledWindow* psDetailsPage;
		wxStaticText* psDetailsCommandLabel;
		wxStaticText* psDetailsCommandText;
		
		
		wxStaticText* psDetailsPidLabel;
		wxStaticText* psDetailsPidText;
		wxStaticText* psDetailsPpidLabel;
		wxStaticText* psDetailsPpidText;
		wxStaticText* psDetailsRealUidLabel;
		wxStaticText* psDetailsRealUidText;
		wxStaticText* psDetailsRealGidLabel;
		wxStaticText* psDetailsRealGidText;
		wxStaticText* psDetailsEffectiveUidLabel;
		wxStaticText* psDetailsEffectiveUidText;
		wxStaticText* psDetailsEffectiveGidLabel;
		wxStaticText* psDetailsEffectiveGidText;
		wxStaticText* psDetailsSecureExecLabel;
		wxStaticText* psDetailsSecureExecText;
		wxStaticText* psDetailsPlaygroundLabel;
		wxStaticText* psDetailsPlaygroundText;
		wxScrolledWindow* psPathPage;
		wxStaticText* psPathAppLabel;
		wxStaticText* psPathAppText;
		wxStaticText* psPathCsumLabel;
		wxStaticText* psPathCsumText;
		wxStaticLine* psPathStaticLine1;
		wxStaticLine* psPathStaticLine2;
		wxStaticText* psPathUserCtxPathLabel;
		wxStaticText* psPathUserCtxPathText;
		wxStaticText* psPathUserCtxCsumLabel;
		wxStaticText* psPathUserCtxCsumText;
		wxStaticLine* psPathStaticLine3;
		wxStaticLine* psPathStaticLine4;
		wxStaticText* psPathAdminCtxPathLabel;
		wxStaticText* psPathAdminCtxPathText;
		wxStaticText* psPathAdminCtxCsumLabel;
		wxStaticText* psPathAdminCtxCsumText;
		wxScrolledWindow* psAlfPage;
		wxStaticText* psAlfUserLabel;
		
		wxButton* alfUserEditButton;
		wxStaticLine* m_staticline7;
		wxStaticText* psAlfAdminLabel;
		
		wxButton* alfAdminEditButton;
		wxTextCtrl* psAlfUserPolicy;
		wxStaticLine* m_staticline8;
		wxTextCtrl* psAlfAdminPolicy;
		wxScrolledWindow* psSbPage;
		wxStaticText* psSbUserLabel;
		
		wxButton* sbUserEditButton;
		wxStaticLine* m_staticline9;
		wxStaticText* psSbAdminLabel;
		
		wxButton* sbAdminEditButton;
		wxTextCtrl* psSbUserPolicy;
		wxStaticLine* m_staticline10;
		wxTextCtrl* psSbAdminPolicy;
		wxScrolledWindow* psCtxPage;
		wxStaticText* psCtxUserLabel;
		
		wxButton* ctxUserEditButton;
		wxStaticLine* m_staticline11;
		wxStaticText* psCtxAdminLabel;
		
		wxButton* ctxAdminEditButton;
		wxTextCtrl* psCtxUserPolicy;
		wxStaticLine* m_staticline12;
		wxTextCtrl* psCtxAdminPolicy;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnTypeChoosen( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnFirstBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnPreviousBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnNextBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnLastBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEscalationOnceButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEscalationProcessButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEscalationTimeoutButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEscalationAlwaysButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEscalationSbPathLeft( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEscalationSbPathRight( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEscalationSfsPathLeft( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEscalationSfsPathRight( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAllowBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDenyBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnProfileSelectionChanged( wxListEvent& event ){ event.Skip(); }
		virtual void OnProfileDeleteClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnProfileLoadClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnProfileSaveClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnProfileActivateClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnVersionActivePolicyClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnVersionProfilePolicyClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnVersionProfileChoice( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnVersionListCtrlSelected( wxListEvent& event ){ event.Skip(); }
		virtual void OnVersionRestoreButtonClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnVersionExportButtonClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnVersionDeleteButtonClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnVersionShowButtonClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEscalationDisable( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEscalationNoTimeout( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEscalationTimeout( wxSpinEvent& event ){ event.Skip(); }
		virtual void OnAlertDisable( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlertNoTimeout( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAlertTimeout( wxSpinEvent& event ){ event.Skip(); }
		virtual void OnEnableUpgradeMsg( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEnableKernelMsg( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEnableInformationMsg( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDoAutostart( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnToolTipCheckBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnToolTipSpinCtrl( wxSpinEvent& event ){ event.Skip(); }
		virtual void OnToolTipSpinCtrlText( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAlfUserEditClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void onAlfAdminEditClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void onSbUserEditClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void onSbAdminEditClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void onCtxUserEditClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void onCtxAdminEditClicked( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		ModAnoubisMainPanelBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ModAnoubisOverviewPanelBase
///////////////////////////////////////////////////////////////////////////////
class ModAnoubisOverviewPanelBase : public wxPanel 
{
	private:
	
	protected:
		
		wxStaticBitmap* anoubisStatusIcon;
		
		wxStaticText* m_staticText35;
		
		wxButton* connectButton;
		wxButton* disconnectButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnConnectClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDisconnectClicked( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		ModAnoubisOverviewPanelBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ModAnoubisProfileSelectionDialogBase
///////////////////////////////////////////////////////////////////////////////
class ModAnoubisProfileSelectionDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText40;
		wxComboBox* profilesCombo;
		wxStdDialogButtonSizer* buttonSizer;
		wxButton* buttonSizerOK;
		wxButton* buttonSizerCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnTextChanged( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		ModAnoubisProfileSelectionDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Select a profile"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
	
};

#endif //__ModAnoubisPanelsBase__
