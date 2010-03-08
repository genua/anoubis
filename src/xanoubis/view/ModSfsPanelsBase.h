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

#ifndef __ModSfsPanelsBase__
#define __ModSfsPanelsBase__

class AnDetails;
class AnGrid;
class AnPickFromFs;
class ModSfsListCtrl;

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
#include <wx/textctrl.h>
#include <wx/dirctrl.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/scrolwin.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/statbmp.h>
#include <wx/notebook.h>
#include <wx/dialog.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class ModSfsMainPanelBase
///////////////////////////////////////////////////////////////////////////////
class ModSfsMainPanelBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* sz_MainSFSMain;
		wxStaticText* tx_MainHeadline;
		wxNotebook* note_MainSfs;
		wxPanel* pan_Rules;
		AnGrid* lst_Rules;
		wxPanel* pan_SfsMain;
		wxStaticText* SfsMainDirectoryLabel;
		wxTextCtrl* SfsMainPathCtrl;
		wxGenericDirCtrl* SfsMainDirCtrl;
		wxScrolledWindow* browserListPanel;
		wxStaticText* SfsMainDirViewLabel;
		wxChoice* SfsMainDirViewChoice;
		wxCheckBox* SfsMainDirTraversalCheckbox;
		wxStaticText* SfsMainDirFilterLabel;
		wxTextCtrl* SfsMainFilterTextCtrl;
		wxButton* SfsMainFilterButton;
		wxCheckBox* SfsMainFilterInvertCheckBox;
		ModSfsListCtrl* SfsMainListCtrl;
		AnDetails* SfsMainDetailsPanel;
		wxCheckBox* SfsMainSignFilesCheckBox;
		
		wxButton* SfsMainImportButton;
		wxButton* SfsMainExportButton;
		wxStaticText* m_staticText101;
		wxChoice* SfsMainActionChoice;
		wxButton* SfsMainActionButton;
		
		wxButton* SfsMainFilterValidateButton;
		wxScrolledWindow* keysTab;
		AnPickFromFs* keyPicker;
		wxStaticText* passphraseValidityLabel;
		wxChoice* privKeyValidityChoice;
		wxSpinCtrl* privKeyValiditySpinCtrl;
		wxStaticText* privKeyValidityText;
		AnPickFromFs* certificatePicker;
		wxStaticText* certDetailsLabel;
		wxStaticLine* certDetailsLine;
		wxBoxSizer* certDetailsIndentSizer;
		
		wxStaticText* certFingerprintLabel;
		wxStaticText* certFingerprintText;
		wxStaticText* certValidityLabel;
		wxStaticText* certValidityText;
		wxStaticText* certCountryLabel;
		wxStaticText* certCountryText;
		wxStaticText* certStateLabel;
		wxStaticText* certStateText;
		wxStaticText* certLocalityLabel;
		wxStaticText* certLocalityText;
		wxStaticText* certOrgaLabel;
		wxStaticText* certOrgaText;
		wxStaticText* certOrgaUnitLabel;
		wxStaticText* certOrgaUnitText;
		wxStaticText* certCnLabel;
		wxStaticText* certCnText;
		wxStaticText* certEmailLabel;
		wxStaticText* certEmailText;
		wxButton* generateKeyPairButton;
		wxPanel* keyWarningPanel;
		wxStaticBitmap* keyWarningIcon;
		wxStaticText* keyWarningText;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onSfsTabChange( wxNotebookEvent& event ){ event.Skip(); }
		virtual void OnGridCellLeftDClick( wxGridEvent& event ){ event.Skip(); }
		virtual void OnSfsPathChanged( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSfsMainDirCtrlSelChanged( wxTreeEvent& event ){ event.Skip(); }
		virtual void OnSfsMainDirViewChoiceSelected( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSfsMainDirTraversalChecked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSfsMainFilterButtonClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSfsMainInverseCheckboxClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSfsListDeselected( wxListEvent& event ){ event.Skip(); }
		virtual void OnSfsListSelected( wxListEvent& event ){ event.Skip(); }
		virtual void OnSfsMainSigEnabledClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSfsMainImportClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSfsMainExportClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSfsMainApplyButtonClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSfsMainValidateButtonClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void onPrivKeyValidityChanged( wxCommandEvent& event ){ event.Skip(); }
		virtual void onPrivKeyValidityPeriodChanged( wxSpinEvent& event ){ event.Skip(); }
		virtual void onGenerateKeyPairButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		ModSfsMainPanelBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ModSfsOverviewPanelBase
///////////////////////////////////////////////////////////////////////////////
class ModSfsOverviewPanelBase : public wxPanel 
{
	private:
	
	protected:
		
		wxStaticBitmap* sfsStatusIcon;
		wxStaticText* txt_status;
		wxStaticText* txt_nachfragen;
		wxStaticText* txt_statusValue;
		wxStaticText* txt_requestValue;
	
	public:
		ModSfsOverviewPanelBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ModSfsDetailsDlgBase
///////////////////////////////////////////////////////////////////////////////
class ModSfsDetailsDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText20;
		wxTextCtrl* pathTextCtrl;
		wxStaticText* linkLabel;
		wxTextCtrl* linkTextCtrl;
		wxStaticText* m_staticText21;
		wxTextCtrl* modifiedTextCtrl;
		wxStaticText* m_staticText22;
		wxTextCtrl* checksumTextCtrl;
		wxStaticText* m_staticText23;
		wxTextCtrl* regChecksumTextCtrl;
		wxStaticText* regSigLabel;
		wxTextCtrl* regSigTextCtrl;
		wxStaticText* m_staticText24;
		wxStaticText* checksumStateLabel;
		wxStaticText* m_staticText26;
		wxStaticText* signatureStateLabel;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
	
	public:
		ModSfsDetailsDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ModSfsGenerateKeyDlgBase
///////////////////////////////////////////////////////////////////////////////
class ModSfsGenerateKeyDlgBase : public wxDialog 
{
	private:
	
	protected:
		AnPickFromFs* keyPicker;
		wxStaticText* passphraseLabel;
		wxTextCtrl* passphraseTextCtrl;
		
		
		wxStaticText* passphraseRepeatLabel;
		wxTextCtrl* passphraseRepeatTextCtrl;
		wxStaticBitmap* passphraseMismatchIcon;
		wxStaticText* passphraseMismatchText;
		AnPickFromFs* certificatePicker;
		wxStaticText* certDetailsLabel;
		wxStaticLine* certDetailsLine;
		wxBoxSizer* certDetailsIndentSizer;
		
		wxStaticText* certValidityLabel;
		wxDatePickerCtrl* certValidityDatePicker;
		wxStaticText* certCountryLabel;
		wxTextCtrl* certCountryTextCtrl;
		wxStaticText* certStateLabel;
		wxTextCtrl* certStateTextCtrl;
		wxStaticText* certLocalityLabel;
		wxTextCtrl* certLocalityTextCtrl;
		wxStaticText* certOrgaLabel;
		wxTextCtrl* certOrgaTextCtrl;
		wxStaticText* certOrgaUnitLabel;
		wxTextCtrl* certOrgaUnitTextCtrl;
		wxStaticText* certCnLabel;
		wxTextCtrl* certCnTextCtrl;
		wxStaticText* certEmailLabel;
		wxTextCtrl* certEmailTextCtrl;
		wxStdDialogButtonSizer* buttonSizer;
		wxButton* buttonSizerOK;
		wxButton* buttonSizerCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void InitDialog( wxInitDialogEvent& event ){ event.Skip(); }
		virtual void onPassphraseFocusLost( wxFocusEvent& event ){ event.Skip(); }
		virtual void onPassphraseEnter( wxCommandEvent& event ){ event.Skip(); }
		virtual void onCancelButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void onOkButton( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		ModSfsGenerateKeyDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Generate Keypair"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("GenerateKeypair") );
	
};

#endif //__ModSfsPanelsBase__
