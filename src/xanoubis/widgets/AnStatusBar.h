/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#ifndef __ANSTATUSBAR_H__
#define __ANSTATUSBAR_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/panel.h>
#include <wx/statusbr.h>

#include "AnEvents.h"

#define ID_STATUSBAR 1001

enum FIELD_IDX {
	FIELD_IDX_STATUS = 0,
	FIELD_IDX_WIZARD,
	FIELD_IDX_SPACER,
	FIELD_IDX_LOG,
	FIELD_IDX_RULEEDITOR,
	FIELD_IDX_RESIZER,
	FIELD_IDX_LAST
};

class AnStatusBar : public wxStatusBar
{
	private:
		bool	 isWizardPressed_;
		bool	 isLogViewerPressed_;
		bool	 isRuleEditorPressed_;
		wxPanel *raisedWizardPanel_;
		wxPanel *sunkenWizardPanel_;
		wxPanel *raisedLogViewerPanel_;
		wxPanel *sunkenLogViewerPanel_;
		wxPanel *raisedRuleEditorPanel_;
		wxPanel *sunkenRuleEditorPanel_;

		void enterWizardPanel(bool);
		void redrawWizardPanel(void);
		void enterLogViewerPanel(bool);
		void redrawLogViewerPanel(void);
		void enterRuleEditorPanel(bool);
		void redrawRuleEditorPanel(void);

		void OnSize(wxSizeEvent&);
		void OnWizardClick(wxMouseEvent&);
		void OnWizardEnter(wxMouseEvent&);
		void OnLogViewerClick(wxMouseEvent&);
		void OnLogViewerEnter(wxMouseEvent&);
		void OnRuleEditorClick(wxMouseEvent&);
		void OnRuleEditorEnter(wxMouseEvent&);

		void onWizardShow(wxCommandEvent&);
		void onLogViewerShow(wxCommandEvent&);
		void onRuleEditorShow(wxCommandEvent&);

		void onConnectionStateChange(wxCommandEvent &);

		ANEVENTS_IDENT_BCAST_METHOD_DECLARATION;

	public:
		AnStatusBar(wxWindow *);
		~AnStatusBar(void);
};

#endif /* __ANSTATUSBAR_H__ */
