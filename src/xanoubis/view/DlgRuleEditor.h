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

#ifndef __DlgRuleEditor__
#define __DlgRuleEditor__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/list.h>
#include <wx/string.h>
#include <wx/filename.h>

#include "AnEvents.h"
#include "AnShortcuts.h"
#include "DlgRuleEditorBase.h"
#include "PolicyRuleSet.h"

enum ruleEditorListColumns {
	RULEDITOR_LIST_COLUMN_PRIO = 0,
	RULEDITOR_LIST_COLUMN_RULE,
	RULEDITOR_LIST_COLUMN_APP,
	RULEDITOR_LIST_COLUMN_BIN,
	RULEDITOR_LIST_COLUMN_HASHT,
	RULEDITOR_LIST_COLUMN_HASH,
	RULEDITOR_LIST_COLUMN_CTX,
	RULEDITOR_LIST_COLUMN_TYPE,
	RULEDITOR_LIST_COLUMN_ACTION,
	RULEDITOR_LIST_COLUMN_LOG,
	RULEDITOR_LIST_COLUMN_AF,
	RULEDITOR_LIST_COLUMN_CAP,
	RULEDITOR_LIST_COLUMN_PROTO,
	RULEDITOR_LIST_COLUMN_DIR,
	RULEDITOR_LIST_COLUMN_FHOST,
	RULEDITOR_LIST_COLUMN_FPORT,
	RULEDITOR_LIST_COLUMN_THOST,
	RULEDITOR_LIST_COLUMN_TPORT,
	RULEDITOR_LIST_COLUMN_STATETIMEOUT,
	RULEDITOR_LIST_COLUMN_EOL
};

class AddrLine
{
	private:
		wxSizer		*sizer_;
		wxWindow	*parent_;
		wxStaticText	*lead_;
		wxComboBox	*addr_;
		wxStaticText	*delimiter_;
		wxSpinCtrl	*net_;
		wxButton	*remove_;
		wxButton	*add_;

	public:
		AddrLine(wxWindow *, wxString, wxString);
		~AddrLine(void);
		void add(wxSizer *, size_t);
		void remove(void);
};
WX_DECLARE_LIST(AddrLine, AddrLineList);

class DlgRuleEditor : public DlgRuleEditorBase
{
	private:
		unsigned long	selectedId_;
		PolicyRuleSet	*ruleSet_;
		wxString	columnNames_[RULEDITOR_LIST_COLUMN_EOL];
		int		columnWidths_[RULEDITOR_LIST_COLUMN_EOL];
		AddrLineList	extraSrcAddrList;
		AddrLineList	extraDstAddrList;

		void OnShow(wxCommandEvent&);

	protected:
		AnShortcuts	*shortcuts_;

		void OnClose(wxCloseEvent& event);
		void OnTableOptionButtonClick(wxCommandEvent&);
		void OnBinaryModifyButtonClick(wxCommandEvent&);
		void OnLoadRuleSet(wxCommandEvent&);
		void OnLineSelected(wxListEvent&);
		void OnRuleSetSave(wxCommandEvent&);

		void OnAlfAllowRadioButton(wxCommandEvent&);
		void OnAlfDenyRadioButton(wxCommandEvent&);
		void OnAlfAskRadioButton(wxCommandEvent&);

		void OnAlfFilterRadioButton(wxCommandEvent&);
		void OnAlfCapRadioButton(wxCommandEvent&);
		void OnAlfDefaultRadioButton(wxCommandEvent&);

		void OnAlfTcpRadioButton(wxCommandEvent&);
		void OnAlfUdpRadioButton(wxCommandEvent&);

		void OnAlfInetRadioButton(wxCommandEvent&);
		void OnAlfInet6RadioButton(wxCommandEvent&);
		void OnAlfAnyRadioButton(wxCommandEvent&);

		void OnAlfRawCapRadioButton(wxCommandEvent&);
		void OnAlfOtherCapRadioButton(wxCommandEvent&);
		void OnAlfAllCapRadioButton(wxCommandEvent&);

		void OnAlfAcceptRadioButton(wxCommandEvent&);
		void OnAlfConnectRadioButton(wxCommandEvent&);

		void OnSfsBinaryModifyButton(wxCommandEvent&);
		void OnSfsUpdateChkSumButton(wxCommandEvent&);

		void loadRuleSet(PolicyRuleSet*);
		void OnCreationChoice(wxCommandEvent&);
		void OnSrcAddrAddButton(wxCommandEvent&);

		void OnAlfStateTimeoutChange(wxSpinEvent&);

	public:
		DlgRuleEditor(wxWindow *);
		~DlgRuleEditor(void);

		friend class RuleEditorAddPolicyVisitor;
		friend class RuleEditorFillWidgetsVisitor;
};

#endif /* __DlgRuleEditor__ */
