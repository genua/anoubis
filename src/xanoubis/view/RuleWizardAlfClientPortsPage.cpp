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

#include <AnListCtrl.h>
#include <AnListProperty.h>
#include <Service.h>
#include <ServiceList.h>

#include "RuleWizardAlfClientPortsPage.h"
#include "RuleWizardAlfDlgAddService.h"

static wxString
protoToString(unsigned int proto)
{
	return proto == Service::TCP ? wxT("tcp") : wxT("udp");
}

static AnIconList::IconId
defaultToIcon(bool val)
{
	return val ? AnIconList::ICON_OK : AnIconList::ICON_NONE;
}

RuleWizardAlfClientPortsPage::RuleWizardAlfClientPortsPage(wxWindow *parent,
    RuleWizardHistory *history) : RuleWizardAlfServicePageBase(parent)
{
	int		width;

	history->get();
	history_ = history;

	/* Initial column width */
	width = portListCtrl->GetClientSize().GetWidth() / 4;

	/* Create columns */
	portListCtrl->addColumn(new AnFmtListProperty<Service>(
	    _("Servicename"), &Service::getName), width);
	portListCtrl->addColumn(
	    new AnFmtListProperty<Service, unsigned int>(
	    _("Portnumber"), &Service::getPort), width);
	portListCtrl->addColumn(
	    new AnFmtListProperty<Service, unsigned int>(
	    _("Protocol"), &Service::getPort, NULL, &protoToString), width);
	portListCtrl->addColumn(
	    new AnFmtListProperty<Service, bool, bool>(
	    _("Standard"), &Service::isDefault, &Service::isDefault,
	    &DefaultConversions::toEmpty, &defaultToIcon), width);

	/* Assign model */
	portListCtrl->setRowProvider(history_->getAlfClientPortList());

	defaultsButton->Enable
	    (history_->getAlfClientPortList()->canHaveDefaultServices());

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardAlfClientPortsPage::onPageChanged),
	    NULL, this);
}

RuleWizardAlfClientPortsPage::~RuleWizardAlfClientPortsPage(void)
{
	RuleWizardHistory::put(history_);
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
	RuleWizardAlfDlgAddService	dlg(this);

	if (dlg.ShowModal() == wxID_OK) {
		ServiceList *list = dlg.getSelection();

		while (list->getServiceCount() > 0) {
			/*
			 * Move selected service into model.
			 * This service-instance is moved into another model.
			 * With each move list->getServiceCount() is decreased.
			 * That's why you cannot iterate over list with a
			 * for-loop.
			 */
			Service *service = list->getServiceAt(0);
			history_->getAlfClientPortList()->addService(service);
		}

		updateNavi();
	}
}

void
RuleWizardAlfClientPortsPage::onDefaultsButton(wxCommandEvent &)
{
	history_->getAlfClientPortList()->assignDefaultServices();
	updateNavi();
}

void
RuleWizardAlfClientPortsPage::onDeleteButton(wxCommandEvent &)
{
	ServiceList *list = history_->getAlfClientPortList();
	std::list<Service *> removeList;
	int index = -1;

	/*
	 * AnListCtrl restores selected internally, if the content of the list
	 * changes. That's why you have to collect the selected services
	 * before removing them from the model.
	 */
	while (true) {
		if ((index = portListCtrl->getNextSelection(index)) == -1)
			break;
		removeList.push_back(list->getServiceAt(index));
	}

	/*
	 * Now it's save to remove the services from the model as you are
	 * independent from the selection
	 */
	while (!removeList.empty()) {
		Service *service = removeList.back();
		removeList.pop_back();
		delete service;
	}

	updateNavi();
}

void
RuleWizardAlfClientPortsPage::onPortListSelect(wxListEvent &)
{
	deleteButton->Enable();
}

void
RuleWizardAlfClientPortsPage::onPortListDeselect(wxListEvent &)
{
	deleteButton->Enable(portListCtrl->hasSelection());
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
