///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep 28 2007)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "ModAnoubisPanelsBase.h"

///////////////////////////////////////////////////////////////////////////

ModAnoubisMainPanelBase::ModAnoubisMainPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	sz_MainAnoubisMain = new wxBoxSizer( wxVERTICAL );
	
	tx_MainHeadline = new wxStaticText( this, wxID_ANY, _("Main Panel of Module Anoubis"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_MainHeadline->Wrap( -1 );
	tx_MainHeadline->SetFont( wxFont( 16, 70, 90, 90, false, wxEmptyString ) );
	
	sz_MainAnoubisMain->Add( tx_MainHeadline, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 5 );
	
	tb_MainAnoubisNotify = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	tb_MainAnoubisNotification = new wxPanel( tb_MainAnoubisNotify, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_MainAnoubisNotify;
	sz_MainAnoubisNotify = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* sz_type;
	sz_type = new wxBoxSizer( wxHORIZONTAL );
	
	tx_type = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("Typ:"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_type->Wrap( -1 );
	sz_type->Add( tx_type, 0, wxALIGN_CENTER|wxALL|wxFIXED_MINSIZE, 5 );
	
	wxString ch_typeChoices[] = { _("current requests"), _("messages"), _("closed requests"), _("all") };
	int ch_typeNChoices = sizeof( ch_typeChoices ) / sizeof( wxString );
	ch_type = new wxChoice( tb_MainAnoubisNotification, wxID_ANY, wxDefaultPosition, wxDefaultSize, ch_typeNChoices, ch_typeChoices, 0 );
	sz_type->Add( ch_type, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sz_MainAnoubisNotify->Add( sz_type, 0, wxALIGN_CENTER|wxALL|wxFIXED_MINSIZE, 5 );
	
	wxBoxSizer* sz_navigate;
	sz_navigate = new wxBoxSizer( wxHORIZONTAL );
	
	
	sz_navigate->Add( 40, 0, 1, wxALL|wxEXPAND, 5 );
	
	bt_first = new wxButton( tb_MainAnoubisNotification, wxID_ANY, _("<<"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_first->Enable( false );
	
	sz_navigate->Add( bt_first, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bt_previous = new wxButton( tb_MainAnoubisNotification, wxID_ANY, _("<"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_previous->Enable( false );
	
	sz_navigate->Add( bt_previous, 0, wxALIGN_CENTER|wxALL, 5 );
	
	tx_currNumber = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("00000000"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_currNumber->Wrap( -1 );
	sz_navigate->Add( tx_currNumber, 0, wxALIGN_CENTER|wxALL, 5 );
	
	tx_delimiter = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(" / "), wxDefaultPosition, wxDefaultSize, 0 );
	tx_delimiter->Wrap( -1 );
	sz_navigate->Add( tx_delimiter, 0, wxALIGN_CENTER|wxALL, 5 );
	
	tx_maxNumber = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("00000000"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_maxNumber->Wrap( -1 );
	sz_navigate->Add( tx_maxNumber, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bt_next = new wxButton( tb_MainAnoubisNotification, wxID_ANY, _(">"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_next->Enable( false );
	
	sz_navigate->Add( bt_next, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bt_last = new wxButton( tb_MainAnoubisNotification, wxID_ANY, _(">>"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT|wxNO_BORDER );
	bt_last->Enable( false );
	
	sz_navigate->Add( bt_last, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sz_navigate->Add( 40, 0, 1, wxALL|wxEXPAND, 5 );
	
	sz_MainAnoubisNotify->Add( sz_navigate, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* slotSizer;
	slotSizer = new wxFlexGridSizer( 6, 2, 0, 0 );
	slotSizer->AddGrowableCol( 0 );
	slotSizer->AddGrowableCol( 1 );
	slotSizer->SetFlexibleDirection( wxHORIZONTAL );
	slotSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );
	
	slotLabelText1 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText1->Wrap( -1 );
	slotSizer->Add( slotLabelText1, 0, wxALL, 5 );
	
	slotValueText1 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText1->Wrap( -1 );
	slotSizer->Add( slotValueText1, 0, wxALL, 5 );
	
	slotLabelText2 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText2->Wrap( -1 );
	slotSizer->Add( slotLabelText2, 0, wxALL, 5 );
	
	slotValueText2 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText2->Wrap( -1 );
	slotSizer->Add( slotValueText2, 0, wxALL, 5 );
	
	slotLabelText3 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText3->Wrap( -1 );
	slotSizer->Add( slotLabelText3, 0, wxALL, 5 );
	
	slotValueText3 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText3->Wrap( -1 );
	slotSizer->Add( slotValueText3, 0, wxALL, 5 );
	
	slotLabelText4 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText4->Wrap( -1 );
	slotSizer->Add( slotLabelText4, 0, wxALL, 5 );
	
	slotValueText4 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText4->Wrap( -1 );
	slotSizer->Add( slotValueText4, 0, wxALL, 5 );
	
	slotLabelText5 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText5->Wrap( -1 );
	slotSizer->Add( slotLabelText5, 0, wxALL, 5 );
	
	slotValueText5 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText5->Wrap( -1 );
	slotSizer->Add( slotValueText5, 0, wxALL, 5 );
	
	slotLabelText6 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _(":"), wxDefaultPosition, wxSize( 140,-1 ), 0 );
	slotLabelText6->Wrap( -1 );
	slotSizer->Add( slotLabelText6, 0, wxALL, 5 );
	
	slotValueText6 = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	slotValueText6->Wrap( -1 );
	slotSizer->Add( slotValueText6, 0, wxALL, 5 );
	
	bSizer10->Add( slotSizer, 0, 0, 5 );
	
	sz_MainAnoubisNotify->Add( bSizer10, 0, wxALL, 5 );
	
	tx_answerValue = new wxStaticText( tb_MainAnoubisNotification, wxID_ANY, _("This message was answered."), wxDefaultPosition, wxDefaultSize, 0 );
	tx_answerValue->Wrap( -1 );
	sz_MainAnoubisNotify->Add( tx_answerValue, 0, wxALL, 5 );
	
	wxBoxSizer* sz_question_top;
	sz_question_top = new wxBoxSizer( wxVERTICAL );
	
	pn_question = new wxPanel( tb_MainAnoubisNotification, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_message;
	sz_message = new wxBoxSizer( wxVERTICAL );
	
	wxGridBagSizer* sz_question;
	sz_question = new wxGridBagSizer( 0, 0 );
	sz_question->SetFlexibleDirection( wxBOTH );
	sz_question->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	tx_question = new wxStaticText( pn_question, wxID_ANY, _("This message"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_question->Wrap( -1 );
	tx_question->SetMinSize( wxSize( 140,-1 ) );
	
	sz_question->Add( tx_question, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL, 5 );
	
	wxGridSizer* sz_question_sub;
	sz_question_sub = new wxGridSizer( 4, 3, 0, 0 );
	
	rb_number = new wxRadioButton( pn_question, wxID_ANY, _("once"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	sz_question_sub->Add( rb_number, 0, wxALL, 1 );
	
	
	sz_question_sub->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sz_question_sub->Add( 0, 0, 1, wxEXPAND, 5 );
	
	rb_procend = new wxRadioButton( pn_question, wxID_ANY, _("till process ends"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question_sub->Add( rb_procend, 0, wxALL, 1 );
	
	
	sz_question_sub->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sz_question_sub->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* sz_time;
	sz_time = new wxBoxSizer( wxHORIZONTAL );
	
	rb_time = new wxRadioButton( pn_question, wxID_ANY, _("for "), wxDefaultPosition, wxDefaultSize, 0 );
	rb_time->SetMinSize( wxSize( 100,-1 ) );
	
	sz_time->Add( rb_time, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1 );
	
	sc_time = new wxSpinCtrl( pn_question, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxSP_WRAP, 1, 999, 1 );
	sc_time->SetMinSize( wxSize( 50,-1 ) );
	
	sz_time->Add( sc_time, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	
	wxString ch_timeChoices[] = { _("seconds"), _("minutes"), _("hours"), _("days") };
	int ch_timeNChoices = sizeof( ch_timeChoices ) / sizeof( wxString );
	ch_time = new wxChoice( pn_question, wxID_ANY, wxDefaultPosition, wxDefaultSize, ch_timeNChoices, ch_timeChoices, 0 );
	sz_time->Add( ch_time, 0, wxALIGN_CENTER|wxALL, 1 );
	
	sz_question_sub->Add( sz_time, 0, 0, 1 );
	
	bt_allow = new wxButton( pn_question, wxID_ANY, _("allow"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question_sub->Add( bt_allow, 0, wxALL|wxALIGN_RIGHT, 1 );
	
	
	sz_question_sub->Add( 0, 0, 1, wxEXPAND, 5 );
	
	rb_always = new wxRadioButton( pn_question, wxID_ANY, _("always"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question_sub->Add( rb_always, 0, wxALL, 1 );
	
	bt_deny = new wxButton( pn_question, wxID_ANY, _("deny"), wxDefaultPosition, wxDefaultSize, 0 );
	sz_question_sub->Add( bt_deny, 0, wxALL|wxALIGN_RIGHT, 1 );
	
	sz_question->Add( sz_question_sub, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	sz_message->Add( sz_question, 1, wxALL, 5 );
	
	pn_question->SetSizer( sz_message );
	pn_question->Layout();
	sz_message->Fit( pn_question );
	sz_question_top->Add( pn_question, 0, wxALIGN_TOP|wxALL, 5 );
	
	sz_MainAnoubisNotify->Add( sz_question_top, 0, wxEXPAND, 5 );
	
	tb_MainAnoubisNotification->SetSizer( sz_MainAnoubisNotify );
	tb_MainAnoubisNotification->Layout();
	sz_MainAnoubisNotify->Fit( tb_MainAnoubisNotification );
	tb_MainAnoubisNotify->AddPage( tb_MainAnoubisNotification, _("Notifications"), true );
	tb_MainAnoubisOptions = new wxPanel( tb_MainAnoubisNotify, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sz_MainAnoubisOptions;
	sz_MainAnoubisOptions = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( tb_MainAnoubisOptions, -1, _("settings") ), wxVERTICAL );
	
	cb_SystemNotification = new wxCheckBox( tb_MainAnoubisOptions, wxID_ANY, _("Send System Notification"), wxDefaultPosition, wxDefaultSize, 0 );
	
	cb_SystemNotification->SetToolTip( _("Get Alert and Escalation Notification via Popup") );
	
	sbSizer1->Add( cb_SystemNotification, 0, wxALL, 5 );
	
	m_spinSystemNotificationTimeout = new wxSpinCtrl( tb_MainAnoubisOptions, wxID_ANY, wxT("10"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 60, 10 );
	m_spinSystemNotificationTimeout->SetToolTip( _("Set System Notification timeout in Seconds") );
	
	sbSizer1->Add( m_spinSystemNotificationTimeout, 0, wxALL, 5 );
	
	m_staticText21 = new wxStaticText( tb_MainAnoubisOptions, wxID_ANY, _("Notification Timeout"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText21->Wrap( -1 );
	sbSizer1->Add( m_staticText21, 0, wxALL, 5 );
	
	sz_MainAnoubisOptions->Add( sbSizer1, 1, wxEXPAND, 5 );
	
	tb_MainAnoubisOptions->SetSizer( sz_MainAnoubisOptions );
	tb_MainAnoubisOptions->Layout();
	sz_MainAnoubisOptions->Fit( tb_MainAnoubisOptions );
	tb_MainAnoubisNotify->AddPage( tb_MainAnoubisOptions, _("Options"), false );
	
	sz_MainAnoubisMain->Add( tb_MainAnoubisNotify, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( sz_MainAnoubisMain );
	this->Layout();
	sz_MainAnoubisMain->Fit( this );
	
	// Connect Events
	ch_type->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnTypeChoosen ), NULL, this );
	bt_first->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnFirstBtnClick ), NULL, this );
	bt_previous->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnPreviousBtnClick ), NULL, this );
	bt_next->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnNextBtnClick ), NULL, this );
	bt_last->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnLastBtnClick ), NULL, this );
	bt_allow->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnAllowBtnClick ), NULL, this );
	bt_deny->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnDenyBtnClick ), NULL, this );
	cb_SystemNotification->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ModAnoubisMainPanelBase::OnToggleNotification ), NULL, this );
	m_spinSystemNotificationTimeout->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( ModAnoubisMainPanelBase::OnNotificationTimeout ), NULL, this );
}

ModAnoubisOverviewPanelBase::ModAnoubisOverviewPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* sz_OverviewAnoubisMain;
	sz_OverviewAnoubisMain = new wxBoxSizer( wxVERTICAL );
	
	tx_OVMainHeadline = new wxStaticText( this, wxID_ANY, _("Show overview information of module Anoubis"), wxDefaultPosition, wxDefaultSize, 0 );
	tx_OVMainHeadline->Wrap( -1 );
	sz_OverviewAnoubisMain->Add( tx_OVMainHeadline, 0, wxALIGN_TOP|wxALL, 5 );
	
	this->SetSizer( sz_OverviewAnoubisMain );
	this->Layout();
	sz_OverviewAnoubisMain->Fit( this );
}
