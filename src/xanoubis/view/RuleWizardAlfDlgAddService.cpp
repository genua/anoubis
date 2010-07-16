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

#include <AnListColumn.h>
#include <AnListCtrl.h>
#include <Service.h>
#include <ServiceList.h>

#include "RuleWizardAlfDlgAddService.h"
#include "main.h"

static wxString
protoToString(unsigned int proto)
{
	return proto == Service::TCP ? wxT("tcp") : wxT("udp");
}

RuleWizardAlfDlgAddService::RuleWizardAlfDlgAddService(wxWindow *parent)
    : RuleWizardAlfDlgAddServiceBase(parent)
{
	int		 width;

	serviceListCtrl->addColumn(new AnFmtListProperty<Service>(
	     _("Servicename"), &Service::getName));
	serviceListCtrl->addColumn(
	    new AnFmtListProperty<Service, unsigned int>(
	    _("Portnumber"), &Service::getPort));
	serviceListCtrl->addColumn(
	    new AnFmtListProperty<Service, unsigned int>(
	    _("Protocol"), &Service::getPort, NULL, &protoToString));

	/* The model, fill with system-services */
	serviceList_ = new ServiceList;
	serviceList_->assignSystemServices();
	serviceListCtrl->setRowProvider(serviceList_);

	/*
	 * List of selected services. You can disable the event-handler because
	 * the object is not used as a model. It is only a storage-container.
	 */
	selection_ = new ServiceList;
	selection_->SetEvtHandlerEnabled(false);

	/*
	 * Update column width.
	 * Substract 10 here, because GetClientSize() does not deliver
	 * size without scrollbars.
	 */
	width = (serviceListCtrl->GetClientSize().GetWidth() / 3) - 10;
	serviceListCtrl->getColumn(0)->setWidth(width);
	serviceListCtrl->getColumn(1)->setWidth(width);
	serviceListCtrl->getColumn(2)->setWidth(width);

	searchTextCtrl->SetFocus();
	Layout();
	Refresh();
}

RuleWizardAlfDlgAddService::~RuleWizardAlfDlgAddService(void)
{
	delete selection_;
	delete serviceList_;
}

ServiceList *
RuleWizardAlfDlgAddService::getSelection(void) const
{
	return (selection_);
}

void
RuleWizardAlfDlgAddService::onAddButton(wxCommandEvent &)
{
	int index = -1;

	selection_->clearServiceList(true);

	while (true) {
		if ((index = serviceListCtrl->getNextSelection(index)) == -1)
			break;

		/* Get servicename */
		Service *service = serviceList_->getServiceAt(index);
		Service *selected = new Service(service->getName(),
		    service->getPort(), service->getProtocol(),
		    service->isDefault());

		selection_->addService(selected);
	};

	EndModal(wxID_OK);
}

void
RuleWizardAlfDlgAddService::onCustomAddButton(wxCommandEvent &)
{
	long port = 0;
	Service::Protocol protocol;

	portTextCtrl->GetValue().ToLong(&port);
	protocol = tcpRadioButton->GetValue() ? Service::TCP : Service::UDP;

	selection_->clearServiceList(true);

	Service *custom = new Service(_("custom"), port, protocol, false);

	selection_->addService(custom);

	EndModal(wxID_OK);
}

void
RuleWizardAlfDlgAddService::onCancelButton(wxCommandEvent &)
{
	selection_->clearServiceList(true);
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
	addButton->Enable(serviceListCtrl->hasSelection());
}

void
RuleWizardAlfDlgAddService::onSearchTextEnter(wxCommandEvent &event)
{
	wxString search = event.GetString();

	/* Deselect first */
	while (serviceListCtrl->hasSelection()) {
		int index = serviceListCtrl->getFirstSelection();
		serviceListCtrl->SetItemState(index, 0, wxLIST_STATE_SELECTED);
	}

	for (unsigned int i = 0; i < serviceList_->getServiceCount(); i++) {
		Service *service = serviceList_->getServiceAt(i);
		wxString other;

		if (search.IsNumber()) {
			/* compare port number */
			other = wxString::Format(wxT("%d"),
			    service->getPort());
		} else {
			/* compare service name */
			other = service->getName();
		}

		if (search.Cmp(other) != 0)
			continue;

		serviceListCtrl->SetItemState(i, wxLIST_STATE_SELECTED,
		    wxLIST_STATE_SELECTED);
		serviceListCtrl->EnsureVisible(i);
	}

	/* Did we find something? */
	if (serviceListCtrl->hasSelection()) {
		searchIcon->SetIcon(AnIconList::getInstance()->GetIcon(
		    AnIconList::ICON_OK));
		addButton->SetFocus();
	} else {
		searchIcon->SetIcon(AnIconList::getInstance()->GetIcon(
		    AnIconList::ICON_ALERT));
	}
	searchIcon->Show();
	Layout();
	Refresh();
}
