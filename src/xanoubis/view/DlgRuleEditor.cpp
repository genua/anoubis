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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/filedlg.h>
#include <wx/choicdlg.h>

#include "AnShortcuts.h"
#include "DlgRuleEditor.h"

DlgRuleEditor::DlgRuleEditor(wxWindow* parent) : DlgRuleEditorBase(parent)
{
	shortcuts_ = new AnShortcuts(this);

	ruleListCtrl->InsertColumn(0, wxT("Id"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	ruleListCtrl->InsertColumn(1, wxT("Application"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	ruleListCtrl->InsertColumn(2, wxT("Context"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	ruleListCtrl->InsertColumn(3, wxT("Binary"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	ruleListCtrl->InsertColumn(4, wxT("Hash-Type"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	ruleListCtrl->InsertColumn(5, wxT("Hash"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	ruleListCtrl->InsertColumn(6, wxT("Action"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
}

DlgRuleEditor::~DlgRuleEditor(void)
{
	delete shortcuts_;
}

void
DlgRuleEditor::OnTableOptionButtonClick(wxCommandEvent& event)
{
	wxArrayString	 choices;
	wxMultiChoiceDialog	*multiChoiceDlg;

	choices.Add(wxT("Id"));
	choices.Add(wxT("Application"));
	choices.Add(wxT("Context"));
	choices.Add(wxT("Binary"));
	choices.Add(wxT("Hash-Type"));
	choices.Add(wxT("Hash"));
	choices.Add(wxT("Action"));

	multiChoiceDlg = new wxMultiChoiceDialog(this, wxT("Table columns"),
	    wxT("Please select the columns you're interested in"), choices);

	if (multiChoiceDlg->ShowModal() == wxID_OK) {
		/* XXX: alter table columns here -- ch */
	}

	delete multiChoiceDlg;
}

void
DlgRuleEditor::OnBinaryModifyButtonClick(wxCommandEvent& event)
{
	wxString	caption = wxT("Choose a binary");
	wxString	wildcard = wxT("*");
	wxString	defaultDir = wxT("/usr/bin/");
	wxString	defaultFilename = wxEmptyString;
	wxFileDialog	fileDlg(NULL, caption, defaultDir, defaultFilename,
			    wildcard, wxOPEN);

	if (fileDlg.ShowModal() == wxID_OK) {
		appBinaryTextCtrl->Clear();
		appBinaryTextCtrl->AppendText(fileDlg.GetPath());
	}
}
