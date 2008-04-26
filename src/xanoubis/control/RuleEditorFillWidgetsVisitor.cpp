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

#include <wx/arrstr.h>

#include "DlgRuleEditor.h"
#include "RuleEditorFillWidgetsVisitor.h"

#define RULEDITOR_DEFAULT	-1
#define RULEDITOR_DISABLE	-2

RuleEditorFillWidgetsVisitor::RuleEditorFillWidgetsVisitor(DlgRuleEditor *re)
{
	ruleEditor_ = re;
}

RuleEditorFillWidgetsVisitor::~RuleEditorFillWidgetsVisitor(void)
{
}

void
RuleEditorFillWidgetsVisitor::clear(void)
{
	ruleEditor_->applicationNbPanel->Disable();
	ruleEditor_->alfNbPanel->Disable();
	ruleEditor_->sfsNbPanel->Disable();

	showAction(RULEDITOR_DISABLE);
	showLog(RULEDITOR_DISABLE);
	showProtocol(RULEDITOR_DISABLE);

	showAddrFamily(RULEDITOR_DISABLE);
	showCapability(RULEDITOR_DISABLE);
	showDirection(RULEDITOR_DISABLE);

	ruleEditor_->alfSrcAddrText->Disable();
	ruleEditor_->alfSrcAddrComboBox->Disable();
	ruleEditor_->alfSrcAddrDelimiterText->Disable();
	ruleEditor_->alfSrcAddrNetSpinCtrl->Disable();
	ruleEditor_->alfSrcAddrDelButton->Disable();
	ruleEditor_->alfSrcAddrAddButton->Disable();
	ruleEditor_->alfDstAddrText->Disable();
	ruleEditor_->alfDstAddrComboBox->Disable();
	ruleEditor_->alfDstAddrDelimiterText->Disable();
	ruleEditor_->alfDstAddrNetSpinCtrl->Disable();
	ruleEditor_->alfDstAddrDelButton->Disable();
	ruleEditor_->alfDstAddrAddButton->Disable();

	ruleEditor_->alfSrcPortText->Disable();
	ruleEditor_->alfSrcPortComboBox->Disable();
	ruleEditor_->alfDstPortText->Disable();
	ruleEditor_->alfDstPortComboBox->Disable();

	for (AddrLineList::iterator i=ruleEditor_->extraSrcAddrList.begin();
	    i != ruleEditor_->extraSrcAddrList.end();
	    i++) {
		(*i)->remove();
		delete (*i);
	}
	ruleEditor_->extraSrcAddrList.Clear();//DeleteContents(true);
	ruleEditor_->Layout();

	for (AddrLineList::iterator i=ruleEditor_->extraDstAddrList.begin();
	    i != ruleEditor_->extraDstAddrList.end();
	    i++) {
		(*i)->remove();
		delete (*i);
	}
	ruleEditor_->extraDstAddrList.Clear();//DeleteContents(true);

	ruleEditor_->sfsBinaryTextCtrl->Clear();
	ruleEditor_->sfsRegisteredSumValueText->SetLabel(wxT("0"));

	ruleEditor_->Layout();
}

void
RuleEditorFillWidgetsVisitor::showAction(int action)
{
	ruleEditor_->alfActionText->Enable();
	ruleEditor_->alfAllowRadioButton->Enable();
	ruleEditor_->alfDenyRadioButton->Enable();
	ruleEditor_->alfAskRadioButton->Enable();

	switch (action) {
	case APN_ACTION_ALLOW:
		ruleEditor_->alfAllowRadioButton->SetValue(true);
		break;
	case APN_ACTION_DENY:
		ruleEditor_->alfDenyRadioButton->SetValue(true);
		break;
	case RULEDITOR_DISABLE:
		ruleEditor_->alfActionText->Disable();
		ruleEditor_->alfAllowRadioButton->Disable();
		ruleEditor_->alfDenyRadioButton->Disable();
		ruleEditor_->alfAskRadioButton->Disable();
		/* FALLTHROUGH */
	default:
		ruleEditor_->alfAskRadioButton->SetValue(true);
		break;
	}
}

void
RuleEditorFillWidgetsVisitor::showLog(int log)
{
	ruleEditor_->commonDoLogRadioButton->Enable();
	ruleEditor_->commonAlertLogRadioButton->Enable();
	ruleEditor_->commonNoneLogRadioButton->Enable();

	switch (log) {
	case APN_LOG_NORMAL:
		ruleEditor_->commonDoLogRadioButton->SetValue(true);
		break;
	case APN_LOG_ALERT:
		ruleEditor_->commonAlertLogRadioButton->SetValue(true);
		break;
	case RULEDITOR_DISABLE:
		ruleEditor_->commonDoLogRadioButton->Disable();
		ruleEditor_->commonAlertLogRadioButton->Disable();
		ruleEditor_->commonNoneLogRadioButton->Disable();
		/* FALLTHROUGH */
	default:
		ruleEditor_->commonNoneLogRadioButton->SetValue(true);
		break;
	}
}

void
RuleEditorFillWidgetsVisitor::showProtocol(int protocol)
{
	ruleEditor_->alfProtocolText->Enable();
	ruleEditor_->alfTcpRadioButton->Enable();
	ruleEditor_->alfUdpRadioButton->Enable();

	switch (protocol) {
	case IPPROTO_TCP:
		ruleEditor_->alfTcpRadioButton->SetValue(true);
		break;
	case IPPROTO_UDP:
		ruleEditor_->alfUdpRadioButton->SetValue(true);
		break;
	case RULEDITOR_DISABLE:
		ruleEditor_->alfProtocolText->Disable();
		ruleEditor_->alfTcpRadioButton->Disable();
		ruleEditor_->alfUdpRadioButton->Disable();
		/* FALLTHROUGH */
	default:
		ruleEditor_->alfTcpRadioButton->SetValue(true);
		break;
	}
}

void
RuleEditorFillWidgetsVisitor::showAddrFamily(int af)
{
	ruleEditor_->alfAddrFamilyText->Enable();
	ruleEditor_->alfInetRadioButton->Enable();
	ruleEditor_->alfInet6RadioButton->Enable();
	ruleEditor_->alfAnyRadioButton->Enable();

	switch (af) {
	case AF_INET:
		ruleEditor_->alfInetRadioButton->SetValue(true);
		break;
	case AF_INET6:
		ruleEditor_->alfInet6RadioButton->SetValue(true);
		break;
	case RULEDITOR_DISABLE:
		ruleEditor_->alfAddrFamilyText->Disable();
		ruleEditor_->alfInetRadioButton->Disable();
		ruleEditor_->alfInet6RadioButton->Disable();
		ruleEditor_->alfAnyRadioButton->Disable();
		/* FALLTHROUGH */
	case 0:
		/* FALLTHROUGH */
	default:
		ruleEditor_->alfAnyRadioButton->SetValue(true);
		break;
	}
}

void
RuleEditorFillWidgetsVisitor::showCapability(int cap)
{
	ruleEditor_->alfCapText->Enable();
	ruleEditor_->alfRawCapRadioButton->Enable();
	ruleEditor_->alfOtherCapRadioButton->Enable();
	ruleEditor_->alfAllCapRadioButton->Enable();

	switch (cap) {
	case APN_ALF_CAPRAW:
		ruleEditor_->alfRawCapRadioButton->SetValue(true);
		break;
	case APN_ALF_CAPOTHER:
		ruleEditor_->alfOtherCapRadioButton->SetValue(true);
		break;
	case RULEDITOR_DISABLE:
		ruleEditor_->alfCapText->Disable();
		ruleEditor_->alfRawCapRadioButton->Disable();
		ruleEditor_->alfOtherCapRadioButton->Disable();
		ruleEditor_->alfAllCapRadioButton->Disable();
		/* FALLTHROUGH */
	case APN_ALF_CAPALL:
		/* FALLTHROUGH */
	default:
		ruleEditor_->alfAllCapRadioButton->SetValue(true);
		break;
	}
}

void
RuleEditorFillWidgetsVisitor::showSrcAddr(wxArrayString hosts)
{
	wxString host;
	wxString net;
	AddrLine	*addrLine;
	wxWindow	*window;

	ruleEditor_->alfSrcAddrText->Enable();
	ruleEditor_->alfSrcAddrComboBox->Enable();
	ruleEditor_->alfSrcAddrDelimiterText->Enable();
	ruleEditor_->alfSrcAddrNetSpinCtrl->Enable();
	ruleEditor_->alfSrcAddrDelButton->Enable();
	ruleEditor_->alfSrcAddrAddButton->Enable();

	for (size_t i=0; i<hosts.GetCount(); i++) {
		host = hosts.Item(i).BeforeFirst('/');
		net  = hosts.Item(i).AfterFirst('/');
		if (net.IsEmpty()) {
			net = wxT("0");
		}
		if (i == 0) {
			ruleEditor_->alfSrcAddrComboBox->SetValue(host);
			ruleEditor_->alfSrcAddrNetSpinCtrl->SetValue(net);
		} else {
			window = ruleEditor_->alfNbPanel;
			addrLine = new AddrLine(window, host, net);
			addrLine->add(ruleEditor_->alfConnectAddrSizer, i * 6);
			ruleEditor_->extraSrcAddrList.Append(addrLine);
		}
	}
}

void
RuleEditorFillWidgetsVisitor::showDstAddr(wxArrayString hosts)
{
	wxString host;
	wxString net;
	AddrLine	*addrLine;
	wxWindow	*window;

	ruleEditor_->alfDstAddrText->Enable();
	ruleEditor_->alfDstAddrComboBox->Enable();
	ruleEditor_->alfDstAddrDelimiterText->Enable();
	ruleEditor_->alfDstAddrNetSpinCtrl->Enable();
	ruleEditor_->alfDstAddrDelButton->Enable();
	ruleEditor_->alfDstAddrAddButton->Enable();

	for (size_t i=0; i<hosts.GetCount(); i++) {
		host = hosts.Item(i).BeforeFirst('/');
		net  = hosts.Item(i).AfterFirst('/');
		if (net.IsEmpty()) {
			net = wxT("0");
		}
		if (i == 0) {
			ruleEditor_->alfDstAddrComboBox->SetValue(host);
			ruleEditor_->alfDstAddrNetSpinCtrl->SetValue(net);
		} else {
			window = ruleEditor_->alfNbPanel;
			addrLine = new AddrLine(window, host, net);
			addrLine->add(ruleEditor_->alfConnectAddrSizer,
			    6 + (i * 6));
			ruleEditor_->extraDstAddrList.Append(addrLine);
		}
	}
}

void
RuleEditorFillWidgetsVisitor::showSrcPort(wxString port)
{
	ruleEditor_->alfSrcPortText->Enable();
	ruleEditor_->alfSrcPortComboBox->Enable();
	ruleEditor_->alfSrcPortComboBox->SetValue(port);
}

void
RuleEditorFillWidgetsVisitor::showDstPort(wxString port)
{
	ruleEditor_->alfDstPortText->Enable();
	ruleEditor_->alfDstPortComboBox->Enable();
	ruleEditor_->alfDstPortComboBox->SetValue(port);
}

void
RuleEditorFillWidgetsVisitor::showDirection(int direction)
{
	ruleEditor_->alfDirectionText->Enable();
	ruleEditor_->alfAcceptRadioButton->Enable();
	ruleEditor_->alfConnectRadioButton->Enable();

	switch (direction) {
	case APN_CONNECT:
		ruleEditor_->alfConnectRadioButton->SetValue(true);
		break;
	case APN_ACCEPT:
		ruleEditor_->alfAcceptRadioButton->SetValue(true);
		break;
	case RULEDITOR_DISABLE:
		ruleEditor_->alfDirectionText->Disable();
		ruleEditor_->alfAcceptRadioButton->Disable();
		ruleEditor_->alfConnectRadioButton->Disable();
		/* FALLTHROUGH */
	default:
		ruleEditor_->alfConnectRadioButton->SetValue(true);
		break;
	}
}

void
RuleEditorFillWidgetsVisitor::visitAppPolicy(AppPolicy *appPolicy)
{
	wxString name;

	clear();

	ruleEditor_->applicationNbPanel->Enable();
//	ruleEditor_->ruleEditNotebook->ChangeSelection(1);
	ruleEditor_->ruleEditNotebook->SetSelection(1);


	name = appPolicy->getBinaryName();
	ruleEditor_->appBinaryTextCtrl->Clear();
	ruleEditor_->appBinaryTextCtrl->AppendText(name);
	ruleEditor_->appNameComboBox->SetValue(appPolicy->getAppName());
	if (appPolicy->hasContext()) {
		name = appPolicy->getContext()->getContextName();
		ruleEditor_->appInheritanceTextCtrl->Clear();
		ruleEditor_->appInheritanceTextCtrl->AppendText(name);
	}
	ruleEditor_->ruleListCtrl->SetFocus();
}

void
RuleEditorFillWidgetsVisitor::visitAlfPolicy(AlfPolicy *alfPolicy)
{
	int type;

	clear();

	ruleEditor_->alfNbPanel->Enable();
	ruleEditor_->ruleEditNotebook->ChangeSelection(2);

	type = alfPolicy->getTypeNo();

	switch (type) {
	case APN_ALF_FILTER:
		ruleEditor_->alfFilterRadioButton->SetValue(true);
		showAction(alfPolicy->getActionNo());
		showLog(alfPolicy->getLogNo());
		showProtocol(alfPolicy->getProtocolNo());
		showAddrFamily(alfPolicy->getAddrFamilyNo());
		showSrcAddr(alfPolicy->getFromHostList());
		showDstAddr(alfPolicy->getToHostList());
		showSrcPort(alfPolicy->getFromPortName());
		showDstPort(alfPolicy->getToPortName());
		showDirection(alfPolicy->getDirectionNo());
		break;
	case APN_ALF_CAPABILITY:
		ruleEditor_->alfCapRadioButton->SetValue(true);
		showAction(alfPolicy->getActionNo());
		showLog(alfPolicy->getLogNo());
		showCapability(alfPolicy->getCapTypeNo());
		break;
	case APN_ALF_DEFAULT:
		ruleEditor_->alfDefaultRadioButton->SetValue(true);
		showAction(alfPolicy->getActionNo());
		showLog(alfPolicy->getLogNo());
		break;
	default:
		break;
	}
	ruleEditor_->ruleListCtrl->SetFocus();
}

void
RuleEditorFillWidgetsVisitor::visitSfsPolicy(SfsPolicy *sfsPolicy)
{
	wxString	currHash;
	wxString	regHash;
	unsigned char	csum[MAX_APN_HASH_LEN];

	clear();

	ruleEditor_->sfsNbPanel->Enable();
	ruleEditor_->ruleEditNotebook->ChangeSelection(3);

	if (sfsPolicy->calcCurrentHash(csum)) {
		currHash = wxT("0x");
		for (unsigned int i=0; i<MAX_APN_HASH_LEN; i++) {
			currHash += wxString::Format(wxT("%2.2x"),
			    (unsigned char)csum[i]);
		}
	} else {
		currHash = _("unable to calculate checksum");
	}
	regHash = sfsPolicy->getHashValue();

	ruleEditor_->sfsBinaryTextCtrl->AppendText(sfsPolicy->getBinaryName());
	ruleEditor_->sfsRegisteredSumValueText->SetLabel(regHash);
	ruleEditor_->sfsCurrentSumValueText->SetLabel(currHash);

	if (regHash.Cmp(currHash) == 0) {
		ruleEditor_->sfsStatusValueText->SetLabel(_("match"));
		ruleEditor_->sfsUpdateChkSumButton->Disable();
	} else {
		ruleEditor_->sfsStatusValueText->SetLabel(_("mismatch"));
		ruleEditor_->sfsUpdateChkSumButton->Enable();
	}

	ruleEditor_->ruleListCtrl->SetFocus();
}

void
RuleEditorFillWidgetsVisitor::visitVarPolicy(VarPolicy *variable)
{
	ruleEditor_->applicationNbPanel->Disable();
	ruleEditor_->alfNbPanel->Disable();
	ruleEditor_->sfsNbPanel->Enable();
}
