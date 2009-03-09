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

#include <netdb.h>

#include "RuleWizardAlfDlgAddService.h"
#include "main.h"

RuleWizardAlfDlgAddService::RuleWizardAlfDlgAddService(wxWindow *parent)
    : RuleWizardAlfDlgAddServiceBase(parent)
{
	int		 width;
	long		 index;
	wxString	 text;
	struct servent	*entry;

	/* Create columns */
	serviceListCtrl->InsertColumn(COLUMN_NAME, _("Servicename"));
	serviceListCtrl->InsertColumn(COLUMN_PORT, _("Portnumber"));
	serviceListCtrl->InsertColumn(COLUMN_PROT, _("Protocol"));

	/* Set initial column width */
	width = serviceListCtrl->GetClientSize().GetWidth() / 3;
	serviceListCtrl->SetColumnWidth(COLUMN_NAME, width);
	serviceListCtrl->SetColumnWidth(COLUMN_PORT, width);
	serviceListCtrl->SetColumnWidth(COLUMN_PROT, width);

	/* Open /etc/services */
	entry = getservent();

	while (entry != NULL) {
		/* Only show protocol tcp and udp. */
		text = wxString::From8BitData(entry->s_proto);
		if (text == wxT("tcp") || text == wxT("udp")) {
			index = serviceListCtrl->GetItemCount();
			serviceListCtrl->InsertItem(index, wxEmptyString);

			/* Protocol */
			serviceListCtrl->SetItem(index, COLUMN_PROT, text);

			/* Servicename */
			text = wxString::From8BitData(entry->s_name);
			serviceListCtrl->SetItem(index, COLUMN_NAME, text);

			/* Portnumber */
			text.Printf(wxT("%d"), ntohs(entry->s_port));
			serviceListCtrl->SetItem(index, COLUMN_PORT, text);
		}

		entry = getservent();
	}

	/* Close /etc/services */
	endservent();

	/*
	 * Update column width.
	 * Substract 10 here, because GetClientSize() does not deliver
	 * size without scrollbars.
	 */
	width = (serviceListCtrl->GetClientSize().GetWidth() / 3) - 10;
	serviceListCtrl->SetColumnWidth(COLUMN_NAME, width);
	serviceListCtrl->SetColumnWidth(COLUMN_PORT, width);
	serviceListCtrl->SetColumnWidth(COLUMN_PROT, width);

	searchTextCtrl->SetFocus();
	Layout();
	Refresh();
}

RuleWizardAlfDlgAddService::~RuleWizardAlfDlgAddService(void)
{
	selection_.Clear();
}

wxArrayString
RuleWizardAlfDlgAddService::getSelection(void) const
{
	return (selection_);
}

void
RuleWizardAlfDlgAddService::onAddButton(wxCommandEvent &)
{
	long		index;
	wxListItem	line;

	selection_.Empty();

	/* Get the first selection */
	index = serviceListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL,
	    wxLIST_STATE_SELECTED);

	while (index != wxNOT_FOUND) {
		/* Get servicename */
		line.SetId(index);
		line.SetColumn(COLUMN_NAME);
		serviceListCtrl->GetItem(line);
		selection_.Add(line.GetText());

		/* Get portnumber */
		line.SetId(index);
		line.SetColumn(COLUMN_PORT);
		serviceListCtrl->GetItem(line);
		selection_.Add(line.GetText());

		/* Get protocol */
		line.SetId(index);
		line.SetColumn(COLUMN_PROT);
		serviceListCtrl->GetItem(line);
		selection_.Add(line.GetText());

		/* Proceed to next selection */
		index = serviceListCtrl->GetNextItem(index, wxLIST_NEXT_ALL,
		    wxLIST_STATE_SELECTED);
	};

	EndModal(wxID_OK);
}

void
RuleWizardAlfDlgAddService::onCustomAddButton(wxCommandEvent &)
{
	selection_.Empty();

	selection_.Add(_("custom"));
	selection_.Add(portTextCtrl->GetValue());
	if (tcpRadioButton->GetValue()) {
		selection_.Add(wxT("tcp"));
	} else {
		selection_.Add(wxT("udp"));
	}

	EndModal(wxID_OK);
}

void
RuleWizardAlfDlgAddService::onCancelButton(wxCommandEvent &)
{
	selection_.Clear();
	EndModal(wxID_CANCEL);
}

void
RuleWizardAlfDlgAddService::onServiceListSelect(wxListEvent &)
{
	addButton->Enable();
}

void
RuleWizardAlfDlgAddService::onServiceListDeselect(wxListEvent &)
{
	/* Did we deselect the last one? */
	if (serviceListCtrl->GetSelectedItemCount() == 0) {
		addButton->Disable();
	}
}

void
RuleWizardAlfDlgAddService::onSearchTextEnter(wxCommandEvent &event)
{
	long		 index;
	wxString	 search;
	wxListItem	 line;
	wxIcon		*icon;

	search = event.GetString();

	/* Deselect first */
	index = serviceListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL,
	    wxLIST_STATE_SELECTED);
	while (index != wxNOT_FOUND) {
		serviceListCtrl->SetItemState(index, 0, wxLIST_STATE_SELECTED);
		/* Proceed to next selection */
		index = serviceListCtrl->GetNextItem(index, wxLIST_NEXT_ALL,
		    wxLIST_STATE_SELECTED);
	}

	for (index=serviceListCtrl->GetItemCount() - 1; index>=0; index--) {
		line.SetId(index);
		if (search.IsNumber()) {
			/* compare port number */
			line.SetColumn(COLUMN_PORT);
		} else {
			/* compare service name */
			line.SetColumn(COLUMN_NAME);
		}
		serviceListCtrl->GetItem(line);
		if (search.Cmp(line.GetText()) != 0) {
			continue;
		}

		serviceListCtrl->SetItemState(index, wxLIST_STATE_SELECTED,
		    wxLIST_STATE_SELECTED);
		serviceListCtrl->EnsureVisible(index);
	}

	/* Did we find something? */
	if (serviceListCtrl->GetSelectedItemCount() > 0) {
		icon = wxGetApp().loadIcon(wxT("General_ok_16.png"));
		addButton->SetFocus();
	} else {
		icon = wxGetApp().loadIcon(wxT("General_alert_16.png"));
	}
	searchIcon->SetIcon(*icon);
	searchIcon->Show();
	Layout();
	Refresh();
}
