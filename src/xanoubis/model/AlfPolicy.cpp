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

#include <sys/param.h>
#include <sys/socket.h>

#ifndef LINUX
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>
#include <wx/string.h>

#include <apn.h>

#include "Policy.h"
#include "AppPolicy.h"
#include "AlfPolicy.h"
#include "PolicyVisitor.h"

AlfPolicy::AlfPolicy(AppPolicy *parent, struct apn_alfrule *alfRule)
    : Policy(parent)
{
	alfRule_ = alfRule;
}

AlfPolicy::~AlfPolicy(void)
{
	/*
	 * The parent can destroy itself and the apn structure is cleand by
	 * PolicyRuleSet.
	 */
}

wxString
AlfPolicy::getHostName(struct apn_host *host)
{
	wxString	result;
	char		ipAddrBuffer[256];

	if (host == NULL) {
		result = wxT("any");
	} else {
		if (host->negate) {
			result += wxT("! ");
		}
		inet_ntop(host->addr.af, &(host->addr.apa.addr32),
		    ipAddrBuffer, sizeof(ipAddrBuffer));
		result += wxString::From8BitData(ipAddrBuffer);
		if (host->addr.len != 32 && host->addr.len != 128) {
			result += wxString::Format(wxT("/%u"), host->addr.len);
		}
	}

	return (result);
}

wxString
AlfPolicy::getPortName(struct apn_port *port)
{
	wxString result;

	if (port == NULL) {
		result = wxT("any");
	} else {
		result = wxString::Format(wxT("%hu"), ntohs(port->port));
	}

	return (result);
}

wxString
AlfPolicy::listToString(wxArrayString list)
{
	wxString	result;

	if (list.IsEmpty()) {
		result = wxEmptyString;
	} else {
		result = list.Item(0);
	}

	for (size_t i=1; i<list.GetCount(); i++) {
		result += wxT(", ");
		result += list.Item(i);
	}
	if (list.GetCount() > 1) {
		result.Prepend(wxT("{"));
		result.Append(wxT("}"));
	}

	return (result);
}

void
AlfPolicy::accept(PolicyVisitor &visitor)
{
	visitor.visitAlfPolicy(this);
}

void
AlfPolicy::setType(int type)
{
	alfRule_->type = type;
}

int
AlfPolicy::getTypeNo(void)
{
	return (alfRule_->type);
}

wxString
AlfPolicy::getTypeName(void)
{
	wxString result;

	switch (alfRule_->type) {
	case APN_ALF_FILTER:
		result = wxT("filter");
		break;
	case APN_ALF_CAPABILITY:
		result = wxT("capability");
		break;
	case APN_ALF_DEFAULT:
		result = wxT("default");
		break;
	case APN_ALF_CTX:
		result = wxT("context");
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}

void
AlfPolicy::setAction(int action)
{
	switch (alfRule_->type) {
	case APN_ALF_FILTER:
		alfRule_->rule.afilt.action = action;
		break;
	case APN_ALF_CAPABILITY:
		alfRule_->rule.acap.action = action;
		break;
	case APN_ALF_DEFAULT:
		alfRule_->rule.apndefault.action = action;
		break;
	default:
		break;
	}
}

int
AlfPolicy::getActionNo(void)
{
	int result = APN_ACTION_DENY;

	switch (alfRule_->type) {
	case APN_ALF_FILTER:
		result = alfRule_->rule.afilt.action;
		break;
	case APN_ALF_CAPABILITY:
		result = alfRule_->rule.acap.action;
		break;
	case APN_ALF_DEFAULT:
		result = alfRule_->rule.apndefault.action;
		break;
	default:
		break;
	}

	return (result);
}

wxString
AlfPolicy::getActionName(void)
{
	return (SUPER(Policy)->getActionName(getActionNo()));
}

wxArrayString
AlfPolicy::getContextList(void)
{
	wxArrayString	 result;
	struct apn_app	*app;

	if (alfRule_->type == APN_ALF_CTX) {
		app = alfRule_->rule.apncontext.application;
		do {
			if (app == NULL) {
				result.Add(wxT("any"));
			} else {
				result.Add(wxString::From8BitData(app->name));
			}
			if ((app == NULL) || (app->next == NULL)) {
				break;
			}
			app = app->next;
		} while (app);
	}

	return (result);
}

wxString
AlfPolicy::getContextName(void)
{
	return (listToString(getContextList()));
}

wxString
AlfPolicy::getRoleName(void)
{
	wxString result;

	switch (getDirectionNo()) {
	case APN_SEND:
		/* FALLTHROUGH */
	case APN_CONNECT:
		result = wxT("client");
		break;
	case APN_RECEIVE:
		/* FALLTHROUGH */
	case APN_ACCEPT:
		result = wxT("server");
		break;
	case APN_BOTH:
		result = wxT("both");
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}

wxString
AlfPolicy::getServiceName(void)
{
	wxString result;

	switch (getDirectionNo()) {
	case APN_SEND:
		/* FALLTHROUGH */
	case APN_CONNECT:
		result = wxT("to ") + getToHostName();
		break;
	case APN_RECEIVE:
		/* FALLTHROUGH */
	case APN_ACCEPT:
		result = wxT("from ") + getFromHostName();
		break;
	case APN_BOTH:
		result = wxT("both from ") + getFromHostName();
		result += wxT(" to ") + getToHostName();
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}

int
AlfPolicy::getLogNo(void)
{
	int result = APN_LOG_NONE;

	switch (alfRule_->type) {
	case APN_ALF_FILTER:
		result = alfRule_->rule.afilt.filtspec.log;
		break;
	case APN_ALF_CAPABILITY:
		result = alfRule_->rule.acap.log;
		break;
	case APN_ALF_DEFAULT:
		result = alfRule_->rule.apndefault.log;
		break;
	default:
		break;
	}

	return (result);
}

wxString
AlfPolicy::getLogName(void)
{
	return (SUPER(Policy)->getLogName(getLogNo()));
}

void
AlfPolicy::setDirection(int direction)
{
	if (alfRule_->type == APN_ALF_FILTER) {
		alfRule_->rule.afilt.filtspec.netaccess = direction;
	}
}

int
AlfPolicy::getDirectionNo(void)
{
	int result = -1;

	if (alfRule_->type == APN_ALF_FILTER) {
		result = alfRule_->rule.afilt.filtspec.netaccess;
	}

	return (result);
}

wxString
AlfPolicy::getDirectionName(void)
{
	return (SUPER(Policy)->getDirectionName(getDirectionNo()));
}

void
AlfPolicy::setProtocol(int protocol)
{
	if (alfRule_->type == APN_ALF_FILTER) {
		alfRule_->rule.afilt.filtspec.proto = protocol;
	}
}

int
AlfPolicy::getProtocolNo(void)
{
	int result = -1;

	if (alfRule_->type == APN_ALF_FILTER) {
		result = alfRule_->rule.afilt.filtspec.proto;
	}

	return (result);
}

wxString
AlfPolicy::getProtocolName(void)
{
	wxString result;

	switch (getProtocolNo()) {
	case IPPROTO_TCP:
		result = wxT("tcp");
		break;
	case IPPROTO_UDP:
		result = wxT("udp");
		break;
	default:
		result = wxT("(unknown");
		break;
	}

	return (result);
}

void
AlfPolicy::setAddrFamily(int addrFamily)
{
	if (alfRule_->type == APN_ALF_FILTER) {
		alfRule_->rule.afilt.filtspec.af = addrFamily;
	}
}

int
AlfPolicy::getAddrFamilyNo(void)
{
	int result = -1;

	if (alfRule_->type == APN_ALF_FILTER) {
		result = alfRule_->rule.afilt.filtspec.af;
	}

	return (result);
}

wxString
AlfPolicy::getAddrFamilyName(void)
{
	wxString result;

	switch (getAddrFamilyNo()) {
	case 0:
		result = wxT("any");
		break;
	case AF_INET:
		result = wxT("inet");
		break;
	case AF_INET6:
		result = wxT("inet6");
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}

wxArrayString
AlfPolicy::getFromHostList(void)
{
	wxArrayString	 result;
	struct apn_host	*fromHost;

	if (alfRule_->type == APN_ALF_FILTER) {
		fromHost = alfRule_->rule.afilt.filtspec.fromhost;
		do {
			result.Add(getHostName(fromHost));
			if ((fromHost == NULL) || (fromHost->next == NULL)) {
				break;
			}
			fromHost = fromHost->next;
		} while (fromHost);
	}

	return (result);
}

wxString
AlfPolicy::getFromHostName(void)
{
	return (listToString(getFromHostList()));
}

wxArrayString
AlfPolicy::getFromPortList(void)
{
	wxArrayString	 result;
	struct apn_port	*fromPort;

	if (alfRule_->type == APN_ALF_FILTER) {
		fromPort = alfRule_->rule.afilt.filtspec.fromport;
		do {
			result.Add(getPortName(fromPort));
			if ((fromPort == NULL) || (fromPort->next == NULL)) {
				break;
			}
			fromPort = fromPort->next;
		} while (fromPort);
	}

	return (result);
}

wxString
AlfPolicy::getFromPortName(void)
{
	return (listToString(getFromPortList()));
}

wxArrayString
AlfPolicy::getToHostList(void)
{
	wxArrayString	 result;
	struct apn_host	*toHost;

	if (alfRule_->type == APN_ALF_FILTER) {
		toHost = alfRule_->rule.afilt.filtspec.tohost;
		do {
			result.Add(getHostName(toHost));
			if ((toHost == NULL) || (toHost->next == NULL)) {
				break;
			}
			toHost = toHost->next;
		} while (toHost);
	}

	return (result);
}

wxString
AlfPolicy::getToHostName(void)
{
	return (listToString(getToHostList()));
}

wxArrayString
AlfPolicy::getToPortList(void)
{
	wxArrayString	 result;
	struct apn_port	*toPort;

	if (alfRule_->type == APN_ALF_FILTER) {
		toPort = alfRule_->rule.afilt.filtspec.toport;
		do {
			result.Add(getPortName(toPort));
			if ((toPort == NULL) || (toPort->next == NULL)) {
				break;
			}
			toPort = toPort->next;
		} while (toPort);
	}

	return (result);
}

wxString
AlfPolicy::getToPortName(void)
{
	return (listToString(getToPortList()));
}

void
AlfPolicy::setCapType(int capability)
{
	if (alfRule_->type == APN_ALF_CAPABILITY) {
		alfRule_->rule.acap.capability = capability;
	}
}

int
AlfPolicy::getCapTypeNo(void)
{
	int result = -1;

	if (alfRule_->type == APN_ALF_CAPABILITY) {
		result = alfRule_->rule.acap.capability;
	}

	return (result);
}

wxString
AlfPolicy::getCapTypeName(void)
{
	wxString result;

	switch (getCapTypeNo()) {
	case APN_ALF_CAPRAW:
		result = wxT("raw");
		break;
	case APN_ALF_CAPOTHER:
		result = wxT("other");
		break;
	case APN_ALF_CAPALL:
		result = wxT("all");
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}