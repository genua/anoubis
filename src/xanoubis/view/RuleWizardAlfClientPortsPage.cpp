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

#include "RuleWizardAlfClientPortsPage.h"

#include "RuleWizardAlfDlgAddService.h"

RuleWizardAlfClientPortsPage::RuleWizardAlfClientPortsPage(wxWindow *parent,
    RuleWizardHistory *history) : RuleWizardAlfServicePageBase(parent)
{
	int	width;

	history_ = history;

	/* Create columns */
	portListCtrl->InsertColumn(COLUMN_NAME, _("Servicename"));
	portListCtrl->InsertColumn(COLUMN_PORT, _("Portnumber"));
	portListCtrl->InsertColumn(COLUMN_PROT, _("Protocol"));
	portListCtrl->InsertColumn(COLUMN_STD, _("Standard"));

	/* Set initial column width */
	width = portListCtrl->GetClientSize().GetWidth() / 4;
	portListCtrl->SetColumnWidth(COLUMN_NAME, width);
	portListCtrl->SetColumnWidth(COLUMN_PORT, width);
	portListCtrl->SetColumnWidth(COLUMN_PROT, width);
	portListCtrl->SetColumnWidth(COLUMN_STD, width);

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardAlfClientPortsPage::onPageChanged),
	    NULL, this);
}

void
RuleWizardAlfClientPortsPage::onPageChanged(wxWizardEvent &)
{
	wxString text;

	text.Printf(_("Options for restricted network access\n"
	    "of application \"%ls\":"),
	    history_->getProgram().c_str());
	questionLabel->SetLabel(text);

	updateNavi();
}

void
RuleWizardAlfClientPortsPage::onAddButton(wxCommandEvent &)
{
	long				index;
	wxArrayString			list;
	RuleWizardAlfDlgAddService	dlg(this);

	if (dlg.ShowModal() == wxID_OK) {
		list = dlg.getSelection();
		for (size_t i=0; i<list.GetCount(); i=i+3) {
			index = portListCtrl->GetItemCount();
			portListCtrl->InsertItem(index, wxEmptyString);
			portListCtrl->SetItem(index, COLUMN_NAME,
			    list.Item(i));
			portListCtrl->SetItem(index, COLUMN_PORT,
			    list.Item(i+1));
			portListCtrl->SetItem(index, COLUMN_PROT,
			    list.Item(i+2));
		}
		storePortList();
	}
}

void
RuleWizardAlfClientPortsPage::onDeleteButton(wxCommandEvent &)
{
	long index;

	index = portListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL,
	    wxLIST_STATE_SELECTED);
	while (index != wxNOT_FOUND) {
		portListCtrl->SetItemState(index, 0, wxLIST_STATE_SELECTED);
		portListCtrl->DeleteItem(index);
		index = portListCtrl->GetNextItem(index - 1, wxLIST_NEXT_ALL,
		    wxLIST_STATE_SELECTED);
	}
	storePortList();
}

void
RuleWizardAlfClientPortsPage::onPortListSelect(wxListEvent &)
{
	deleteButton->Enable();
}

void
RuleWizardAlfClientPortsPage::onPortListDeselect(wxListEvent &)
{
	/* Was the last one deselected? */
	if (portListCtrl->GetSelectedItemCount() == 0) {
		deleteButton->Disable();
	}
}

void
RuleWizardAlfClientPortsPage::onAskCheckBox(wxCommandEvent & event)
{
	history_->setAlfClientAsk(event.GetInt());
	updateNavi();
}

void
RuleWizardAlfClientPortsPage::onRawCheckBox(wxCommandEvent & event)
{
	history_->setAlfClientRaw(event.GetInt());
	updateNavi();
}

void
RuleWizardAlfClientPortsPage::storePortList(void) const
{
	long		index;
	wxListItem	line;
	wxArrayString	list;

	for (index=0; index<portListCtrl->GetItemCount(); index++) {
		/* Get servicename */
		line.SetId(index);
		line.SetColumn(COLUMN_NAME);
		portListCtrl->GetItem(line);
		list.Add(line.GetText());

		/* Get portnumber */
		line.SetId(index);
		line.SetColumn(COLUMN_PORT);
		portListCtrl->GetItem(line);
		list.Add(line.GetText());

		/* Get protocol */
		line.SetId(index);
		line.SetColumn(COLUMN_PROT);
		portListCtrl->GetItem(line);
		list.Add(line.GetText());
	}

	history_->setAlfClientPortList(list);
}

void
RuleWizardAlfClientPortsPage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, false);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillAlfNavi(this, naviSizer, true);
	history_->fillSandboxNavi(this, naviSizer, false);
	Layout();
	Refresh();
}
