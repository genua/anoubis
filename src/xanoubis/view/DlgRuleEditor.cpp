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

#include "AnShortcuts.h"
#include "DlgRuleEditor.h"

DlgRuleEditor::DlgRuleEditor(wxWindow* parent) : DlgRuleEditorBase(parent)
{
	alfPanel_ = new DlgRuleEditorAlfPanelBase(this);
	sfsPanel_ = new DlgRuleEditorSfsPanelBase(this);
	shortcuts_  = new AnShortcuts(this);

	alfPanel_->Show();
	sfsPanel_->Hide();

	this->GetSizer()->Add(alfPanel_, 1, wxALL|wxEXPAND, 5);
	this->GetSizer()->Layout();
}

DlgRuleEditor::~DlgRuleEditor(void)
{
	delete alfPanel_;
	delete sfsPanel_;
	delete shortcuts_;
}

void
DlgRuleEditor::OnModSelected(wxCommandEvent& event)
{
	wxPanel *modulePanel;
	wxSizer *mainSizer;

	mainSizer = this->GetSizer();
	mainSizer->GetItem(2)->GetWindow()->Hide();
	mainSizer->Detach(2);

	switch (this->rb_modSelector->GetSelection()) {
	default:
		fprintf(stderr, "INTERNAL ERROR: default for modSelector\n");
		/* FALLTHROUGH */
	case 0:
		modulePanel = alfPanel_;
		break;
	case 1:
		modulePanel = sfsPanel_;
		break;
	}
	mainSizer->Add(modulePanel, 1, wxALL|wxEXPAND, 5);
	modulePanel->Show();
	mainSizer->Layout();
	event.Skip();
}
