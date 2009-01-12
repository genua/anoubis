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

#ifndef __ModSfsPanelsBase__
#define __ModSfsPanelsBase__

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/dirctrl.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/notebook.h>
#include <wx/statbmp.h>

///////////////////////////////////////////////////////////////////////////

#define wxID_RulesOperationSep 7500

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
		wxListCtrl* lst_Rules;
		wxStaticText* tx_RulesOperation1stHeader;
		wxStaticLine* ln_RulesOperationSep;
		wxStaticText* tx_RulesOperation2ndHeader;
		wxPanel* pan_SfsMain;
		wxStaticText* m_staticText12;
		wxStaticText* SfsMainCurrPathLabel;
		wxGenericDirCtrl* SfsMainDirCtrl;
		wxStaticText* m_staticText10;
		wxTextCtrl* SfsMainFilterTextCtrl;
		wxButton* SfsMainFilterButton;
		wxCheckBox* SfsMainFilterInvertCheckBox;
		
		wxButton* SfsMainFilterValidateButton;
		wxListCtrl* SfsMainListCtrl;
		wxCheckBox* SfsMainSignFilesCheckBox;
		wxButton* SfsMainSearchOrphanedButton;
		wxButton* SfsMainShowChecksumButton;
		wxButton* SfsMainImportButton;
		
		
		wxButton* SfsMainShowChangedButton;
		wxButton* SfsMainExportButton;
		
		wxStaticText* m_staticText101;
		wxChoice* SfsMainActionChoice;
		wxButton* SfsMainActionButton;
	
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

#endif //__ModSfsPanelsBase__
