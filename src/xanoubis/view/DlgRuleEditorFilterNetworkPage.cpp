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

#include "DlgRuleEditorFilterNetworkPage.h"

DlgRuleEditorFilterNetworkPage::DlgRuleEditorFilterNetworkPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorPage(),
    DlgRuleEditorFilterNetworkPageBase(parent, id, pos, size, style)
{
	filterPolicy_ = NULL;
}

void
DlgRuleEditorFilterNetworkPage::update(Subject *subject)
{
	if (subject == filterPolicy_) {
		/* This is our policy. */
		showDirection();
		showAddressFamily();
		showProtocol();
		showStateTimeout();
	}
}

void
DlgRuleEditorFilterNetworkPage::select(Policy *policy)
{
	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		filterPolicy_ = wxDynamicCast(policy, AlfFilterPolicy);
		DlgRuleEditorPage::select(policy);
		Enable(enable_);
		Show();
	}
}

void
DlgRuleEditorFilterNetworkPage::deselect(void)
{
	filterPolicy_ = NULL;
	DlgRuleEditorPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterNetworkPage::showDirection(void)
{
	if (filterPolicy_ == NULL) {
		return;
	}

	switch (filterPolicy_->getDirectionNo()) {
	case APN_SEND:
		inRadioButton->SetLabel(wxT("receive"));
		outRadioButton->SetLabel(wxT("send"));
		outRadioButton->SetValue(true);
		break;
	case APN_CONNECT:
		inRadioButton->SetLabel(wxT("accept"));
		outRadioButton->SetLabel(wxT("connect"));
		outRadioButton->SetValue(true);
		break;
	case APN_RECEIVE:
		outRadioButton->SetLabel(wxT("send"));
		inRadioButton->SetLabel(wxT("receive"));
		inRadioButton->SetValue(true);
		break;
	case APN_ACCEPT:
		outRadioButton->SetLabel(wxT("connect"));
		inRadioButton->SetLabel(wxT("accept"));
		inRadioButton->SetValue(true);
		break;
	case APN_BOTH:
		bothRadioButton->SetValue(true);
		break;
	default:
		inRadioButton->SetValue(false);
		outRadioButton->SetValue(false);
		bothRadioButton->SetValue(false);
	}

	Layout();
	Refresh();
}

void
DlgRuleEditorFilterNetworkPage::showAddressFamily(void)
{
	if (filterPolicy_ == NULL) {
		return;
	}

	switch (filterPolicy_->getAddrFamilyNo()) {
	case AF_INET:
		inetRadioButton->SetValue(true);
		break;
	case AF_INET6:
		inet6RadioButton->SetValue(true);
		break;
	case 0: /* ANY */
		anyRadioButton->SetValue(true);
		break;
	default:
		inetRadioButton->SetValue(false);
		inet6RadioButton->SetValue(false);
		anyRadioButton->SetValue(false);
	}
}

void
DlgRuleEditorFilterNetworkPage::showProtocol(void)
{
	if (filterPolicy_ == NULL) {
		return;
	}

	switch (filterPolicy_->getProtocolNo()) {
	case IPPROTO_TCP:
		tcpRadioButton->SetValue(true);
		break;
	case IPPROTO_UDP:
		udpRadioButton->SetValue(true);
		break;
	default:
		tcpRadioButton->SetValue(false);
		udpRadioButton->SetValue(false);
	}
}

void
DlgRuleEditorFilterNetworkPage::showStateTimeout(void)
{
	if (filterPolicy_ != NULL) {
		stateTimeoutSpinCtrl->SetValue(
		    filterPolicy_->getStateTimeoutNo());
	}
}

void
DlgRuleEditorFilterNetworkPage::onInRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		/* In case of udp, policy will change this to APN_RECEIVE. */
		filterPolicy_->setDirectionNo(APN_ACCEPT);
	}
}

void
DlgRuleEditorFilterNetworkPage::onOutRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		/* In case of udp, policy will change this to APN_SEND. */
		filterPolicy_->setDirectionNo(APN_CONNECT);
	}
}

void
DlgRuleEditorFilterNetworkPage::onBothRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setDirectionNo(APN_BOTH);
	}
}

void
DlgRuleEditorFilterNetworkPage::onInetRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setAddrFamilyNo(AF_INET);
	}
}

void
DlgRuleEditorFilterNetworkPage::onInet6RadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setAddrFamilyNo(AF_INET6);
	}
}

void
DlgRuleEditorFilterNetworkPage::onAnyRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setAddrFamilyNo(0);
	}
}

void
DlgRuleEditorFilterNetworkPage::onTcpRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setProtocol(IPPROTO_TCP);
	}
}

void
DlgRuleEditorFilterNetworkPage::onUdpRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setProtocol(IPPROTO_UDP);
	}
}

void
DlgRuleEditorFilterNetworkPage::onStateTimeoutSpinCtrl(wxSpinEvent & event)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setStateTimeout((int)event.GetPosition());
	}
}
