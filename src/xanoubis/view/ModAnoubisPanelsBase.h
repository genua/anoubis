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

#ifndef __ModAnoubisPanelsBase__
#define __ModAnoubisPanelsBase__

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
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class ModAnoubisMainPanelBase
///////////////////////////////////////////////////////////////////////////////
class ModAnoubisMainPanelBase : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* sz_MainAnoubisMain;
		wxStaticText* tx_MainHeadline;
		wxStaticText* tx_type;
		wxChoice* ch_type;
		
		wxButton* bt_first;
		wxButton* bt_previous;
		wxStaticText* tx_currNumber;
		wxStaticText* tx_delimiter;
		wxStaticText* tx_maxNumber;
		wxButton* bt_next;
		wxButton* bt_last;
		
		wxStaticText* tx_fieldSlot1;
		wxStaticText* tx_valueSlot1;
		wxStaticText* tx_fieldSlot2;
		wxStaticText* tx_valueSlot2;
		wxStaticText* tx_fieldSlot3;
		wxStaticText* tx_valueSlot3;
		wxStaticText* tx_answerValue;
		wxPanel* pn_question;
		wxStaticText* tx_question;
		wxRadioButton* rb_number;
		wxSpinCtrl* sc_number;
		
		
		wxRadioButton* rb_procend;
		
		
		wxRadioButton* rb_time;
		wxSpinCtrl* sc_time;
		wxChoice* ch_time;
		wxButton* bt_allow;
		
		wxRadioButton* rb_always;
		wxButton* bt_deny;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnTypeChoosen( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnFirstBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnPreviousBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnNextBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnLastBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAllowBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDenyBtnClick( wxCommandEvent& event ){ event.Skip(); }
		
	
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
		wxStaticText* tx_OVMainHeadline;
	
	public:
		ModAnoubisOverviewPanelBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
	
};

#endif //__ModAnoubisPanelsBase__