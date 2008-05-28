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
#include <wx/gbsizer.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/notebook.h>

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
		wxNotebook* tb_MainAnoubisNotify;
		wxPanel* tb_MainAnoubisNotification;
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
		wxPanel* pn_question;
		wxStaticText* tx_question;
		wxRadioButton* rb_number;
		
		
		wxRadioButton* rb_procend;
		
		
		wxRadioButton* rb_time;
		wxSpinCtrl* sc_time;
		wxChoice* ch_time;
		wxButton* bt_allow;
		
		wxRadioButton* rb_always;
		wxButton* bt_deny;
		wxPanel* tb_MainAnoubisOptions;
		wxCheckBox* cb_SystemNotification;
		wxSpinCtrl* m_spinSystemNotificationTimeout;
		wxStaticText* m_staticText21;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnTypeChoosen( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnFirstBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnPreviousBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnNextBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnLastBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAllowBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDenyBtnClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnToggleNotification( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnNotificationTimeout( wxSpinEvent& event ){ event.Skip(); }
		
	
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
