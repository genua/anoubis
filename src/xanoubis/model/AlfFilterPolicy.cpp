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

#include "AlfFilterPolicy.h"

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include "PolicyUtils.h"
#include "PolicyVisitor.h"
#include "PolicyRuleSet.h"

IMPLEMENT_CLASS(AlfFilterPolicy, FilterPolicy);

AlfFilterPolicy::AlfFilterPolicy(AppPolicy *parentPolicy, struct apn_rule *rule)
    : FilterPolicy(parentPolicy, rule)
{
}

AlfFilterPolicy::AlfFilterPolicy(AlfAppPolicy *parentPolicy)
    : FilterPolicy(parentPolicy, AlfFilterPolicy::createApnRule())
{
}

wxString
AlfFilterPolicy::getTypeIdentifier(void) const
{
	return (wxT("ALF"));
}

void
AlfFilterPolicy::accept(PolicyVisitor &visitor)
{
	visitor.visitAlfFilterPolicy(this);
}

struct apn_rule *
AlfFilterPolicy::createApnRule(void)
{
	struct apn_rule *rule;

	rule = FilterPolicy::createApnRule();
	if (rule != NULL) {
		rule->apn_type = APN_ALF_FILTER;
		rule->rule.afilt.filtspec.proto = IPPROTO_TCP;
		rule->rule.afilt.filtspec.netaccess = APN_CONNECT;
	}

	return (rule);
}

struct apn_host *
AlfFilterPolicy::createApnHost(wxString host, int af)
{
	wxString	 ipString;
	wxString	 netString;
	long		 netmask;
	int		 negate;
	int		 rc;
	char		 ipBuffer[256];
	struct apn_host *apnHost;

	netmask = 0;
	negate = 0;
	rc = 0;
	memset(ipBuffer, 0, sizeof(ipBuffer));

	/* Split host into ip and netmask  */
	ipString = host.BeforeFirst('/');
	netString = host.AfterLast('/');
	if (netString.IsSameAs(ipString)) {
		netmask = (af == AF_INET) ? 32 : 128;
	} else {
		netString.ToLong(&netmask);
	}

	/* Is host negated */
	if (ipString.StartsWith(wxT("!"), &ipString)) {
		negate = 1;
		ipString.Trim(false);
	}

	apnHost = (struct apn_host *)calloc(1, sizeof(struct apn_host));
	if (apnHost != NULL) {
		apnHost->addr.len = (u_int8_t)netmask;
		apnHost->addr.af = af;
		apnHost->negate = negate;

		strlcpy(ipBuffer, (const char*)ipString.mb_str(wxConvUTF8),
		    sizeof(ipBuffer));
		rc = inet_pton(af, ipBuffer, &(apnHost->addr.apa.addr32));
		if (rc <= 0) {
			free(apnHost);
			apnHost = NULL;
		}
	}

	return (apnHost);
}

struct apn_port *
AlfFilterPolicy::createApnPort(wxString port)
{
	int		 lcnt, rcnt;
	long		 p1=0, p2=0;
	struct apn_port *apnPort;

	apnPort = (struct apn_port *)calloc(1, sizeof(struct apn_port));
	if (apnPort != NULL) {
		lcnt = port.Find(wxT("-"));
		if (lcnt == wxNOT_FOUND) {
			if (port.ToLong(&p1)) {
				apnPort->port = htons((int)p1);
				apnPort->port2 = 0;
			} else {
				free(apnPort);
				apnPort = NULL;
			}
		} else {
			rcnt = port.Len() - lcnt - 1;
			wxString	left = port.Left(lcnt);
			wxString	right = port.Right(rcnt);

			left.Trim(true);
			left.Trim(false);
			right.Trim(true);
			right.Trim(false);
			if (left.ToLong(&p1) && right.ToLong(&p2)) {
				apnPort->port = htons((int)p1);
				apnPort->port2 = htons((int)p2);
			} else {
				free(apnPort);
				return NULL;
			}
		}
	}

	return (apnPort);
}

bool
AlfFilterPolicy::createApnInserted(AppPolicy *parent, unsigned int id)
{
	int		 rc;
	struct apn_rule *rule;
	PolicyRuleSet	*ruleSet;

	if (parent == NULL) {
		return (false);
	}

	ruleSet = parent->getParentRuleSet();
	if (ruleSet == NULL) {
		return (false);
	}

	rule = AlfFilterPolicy::createApnRule();
	if (rule == NULL) {
		return (false);
	}

	/* No 'insert-before'-id given: insert on top by using block-id . */
	if (id == 0) {
		id = parent->getApnRuleId();
	}

	rc = apn_insert_alfrule(ruleSet->getApnRuleSet(), rule, id);

	if (rc != 0) {
		apn_free_one_rule(rule, NULL);
		return (false);
	}

	return (true);
}

bool
AlfFilterPolicy::setLogNo(int log)
{
	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();
	getApnRule()->rule.afilt.filtspec.log = log;
	setModified();
	finishChange();

	return (true);
}

int
AlfFilterPolicy::getLogNo(void) const
{
	int log = APN_LOG_NONE;

	if (getApnRule() != NULL) {
		log = getApnRule()->rule.afilt.filtspec.log;
	}

	return (log);
}

bool
AlfFilterPolicy::setActionNo(int action)
{
	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();
	getApnRule()->rule.afilt.action = action;
	setModified();
	finishChange();

	return (true);
}

int
AlfFilterPolicy::getActionNo(void) const
{
	int action = APN_ACTION_DENY;

	if (getApnRule() != NULL) {
		action = getApnRule()->rule.afilt.action;
	}

	return (action);
}

bool
AlfFilterPolicy::setDirectionNo(int direction)
{
	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();
	if (getProtocolNo() == IPPROTO_UDP) {
		switch (direction) {
		case APN_CONNECT:
			direction = APN_SEND;
			break;
		case APN_ACCEPT:
			direction = APN_RECEIVE;
			break;
		default:
			break;
		}
	} else {
		switch (direction) {
		case APN_SEND:
			direction = APN_CONNECT;
			break;
		case APN_RECEIVE:
			direction = APN_ACCEPT;
			break;
		default:
			break;
		}
	}
	getApnRule()->rule.afilt.filtspec.netaccess = direction;
	setModified();
	finishChange();

	return (true);
}

int
AlfFilterPolicy::getDirectionNo(void) const
{
	int direction = -1;

	if (getApnRule() != NULL) {
		direction = getApnRule()->rule.afilt.filtspec.netaccess;
	}

	return (direction);
}

wxString
AlfFilterPolicy::getDirectionName(void) const
{
	wxString direction;

	switch (getDirectionNo()) {
	case APN_CONNECT:
		direction = wxT("connect");
		break;
	case APN_ACCEPT:
		direction = wxT("accept");
		break;
	case APN_SEND:
		direction = wxT("send");
		break;
	case APN_RECEIVE:
		direction = wxT("receive");
		break;
	case APN_BOTH:
		direction = wxT("both");
		break;
	default:
		direction = _("(unknown)");
		break;
	}

	return (direction);
}

bool
AlfFilterPolicy::setProtocol(int protocol)
{
	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();
	getApnRule()->rule.afilt.filtspec.proto = protocol;
	/* Adjust direction to new protocol. */
	setDirectionNo(getDirectionNo());
	setModified();
	finishChange();

	return (true);
}

int
AlfFilterPolicy::getProtocolNo(void) const
{
	int protocol = -1;

	if (getApnRule() != NULL) {
		protocol = getApnRule()->rule.afilt.filtspec.proto;
	}

	return (protocol);
}

wxString
AlfFilterPolicy::getProtocolName(void) const
{
	wxString protocol;

	switch (getProtocolNo()) {
	case 0:
		protocol = wxT("any");
		break;
	case IPPROTO_TCP:
		protocol = wxT("tcp");
		break;
	case IPPROTO_UDP:
		protocol = wxT("udp");
		break;
	default:
		protocol = _("(unknown");
		break;
	}

	return (protocol);
}

bool
AlfFilterPolicy::setAddrFamilyNo(int af)
{
	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();
	getApnRule()->rule.afilt.filtspec.af = af;
	setModified();
	finishChange();

	return (true);

}

int
AlfFilterPolicy::getAddrFamilyNo(void) const
{
	int af = -1;

	if (getApnRule() != NULL) {
		af = getApnRule()->rule.afilt.filtspec.af;
	}

	return (af);
}

wxString
AlfFilterPolicy::getAddrFamilyName(void) const
{
	wxString af;

	switch (getAddrFamilyNo()) {
	case 0:
		af = wxT("any");
		break;
	case AF_INET:
		af = wxT("inet");
		break;
	case AF_INET6:
		af = wxT("inet6");
		break;
	default:
		af = _("(unknown)");
		break;
	}

	return (af);
}

bool
AlfFilterPolicy::setFromHostName(wxString fromHost)
{
	return (setFromHostList(PolicyUtils::stringToList(fromHost)));
}

bool
AlfFilterPolicy::setFromHostList(wxArrayString hostList)
{
	struct apn_host **fromHostPtr;
	int		  af = getAddrFamilyNo();

	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();
	apn_free_host(getApnRule()->rule.afilt.filtspec.fromhost);
	getApnRule()->rule.afilt.filtspec.fromhost = NULL;
	fromHostPtr = &(getApnRule()->rule.afilt.filtspec.fromhost);

	if ((hostList.GetCount() == 1) &&
	    (hostList.Item(0).Cmp(wxT("any")) == 0)) {
		/* The only element is any, which is already set. */
		setModified();
		finishChange();
		return (true);
	}

	if (!hostList.IsEmpty()) {
		for (size_t i = 0; i < hostList.GetCount(); i++) {
			struct apn_host		*tmp = NULL;

			switch (af) {
			case AF_UNSPEC:
			case AF_INET:
				tmp = createApnHost(hostList.Item(i), AF_INET);
				if (tmp || af == AF_INET)
					break;
			case AF_INET6:
				tmp = createApnHost(hostList.Item(i), AF_INET6);
				break;
			}
			if (tmp == NULL) {
				setModified();
				finishChange();
				return (false);
			}
			tmp->next = NULL;
			(*fromHostPtr) = tmp;
			fromHostPtr = &tmp->next;
		}
	}

	setModified();
	finishChange();

	return (true);
}

wxString
AlfFilterPolicy::getFromHostName(void) const
{
	return (PolicyUtils::listToString(getFromHostList()));
}

wxArrayString
AlfFilterPolicy::getFromHostList(void) const
{
	wxArrayString	 hostList;
	struct apn_host *fromHost;

	hostList.Clear();

	if (getApnRule() == NULL) {
		return (hostList);
	}

	fromHost = getApnRule()->rule.afilt.filtspec.fromhost;
	do {
		hostList.Add(hostToString(fromHost));
		if ((fromHost == NULL) || (fromHost->next == NULL)) {
			break;
		}
		fromHost = fromHost->next;
	} while (fromHost != NULL);

	return (hostList);
}

bool
AlfFilterPolicy::setFromPortName(wxString fromPort)
{
	return (setFromPortList(PolicyUtils::stringToList(fromPort)));
}

bool
AlfFilterPolicy::setFromPortList(wxArrayString portList)
{
	struct apn_port **fromPortPtr;

	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();
	apn_free_port(getApnRule()->rule.afilt.filtspec.fromport);
	getApnRule()->rule.afilt.filtspec.fromport = NULL;
	fromPortPtr = &(getApnRule()->rule.afilt.filtspec.fromport);

	if ((portList.GetCount() == 1) &&
	    (portList.Item(0).Cmp(wxT("any")) == 0)) {
		/* The only element is any, which is already set. */
		setModified();
		finishChange();
		return (true);
	}

	if (!portList.IsEmpty()) {
		for(size_t i=0; i<portList.GetCount(); ++i) {
			struct apn_port	*tmp;

			tmp = createApnPort(portList.Item(i));
			if (tmp == NULL) {
				setModified();
				finishChange();
				return (false);
			}
			tmp->next = NULL;
			(*fromPortPtr) = tmp;
			fromPortPtr = &tmp->next;
		}
	}

	setModified();
	finishChange();

	return (true);
}

wxString
AlfFilterPolicy::getFromPortName(void) const
{
	return (PolicyUtils::listToString(getFromPortList()));
}

wxArrayString
AlfFilterPolicy::getFromPortList(void) const
{
	wxArrayString	 portList;
	struct apn_port *fromPort;

	portList.Clear();

	if (getApnRule() == NULL) {
		return (portList);
	}

	fromPort = getApnRule()->rule.afilt.filtspec.fromport;
	do {
		portList.Add(portToString(fromPort));
		if ((fromPort == NULL) || (fromPort->next == NULL)) {
			break;
		}
		fromPort = fromPort->next;
	} while (fromPort != NULL);

	return (portList);
}

bool
AlfFilterPolicy::setToHostName(wxString toHost)
{
	return (setToHostList(PolicyUtils::stringToList(toHost)));
}

bool
AlfFilterPolicy::setToHostList(wxArrayString hostList)
{
	struct apn_host **toHostPtr;
	int		  af = getAddrFamilyNo();

	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();
	apn_free_host(getApnRule()->rule.afilt.filtspec.tohost);
	getApnRule()->rule.afilt.filtspec.tohost = NULL;
	toHostPtr = &(getApnRule()->rule.afilt.filtspec.tohost);

	if ((hostList.GetCount() == 1) &&
	    (hostList.Item(0).Cmp(wxT("any")) == 0)) {
		/* The only element is any, which is already set. */
		setModified();
		finishChange();
		return (true);
	}

	if (!hostList.IsEmpty()) {
		for (size_t i = 0; i < hostList.GetCount(); i++) {
			struct apn_host		*tmp = NULL;

			switch (af) {
			case AF_UNSPEC:
			case AF_INET:
				tmp = createApnHost(hostList.Item(i), AF_INET);
				if (tmp || af == AF_INET)
					break;
			case AF_INET6:
				tmp = createApnHost(hostList.Item(i), AF_INET6);
				break;
			}
			if (tmp == NULL) {
				setModified();
				finishChange();
				return (false);
			}
			tmp->next = NULL;
			(*toHostPtr) = tmp;
			toHostPtr = &tmp->next;
		}
	}

	setModified();
	finishChange();

	return (true);
}

wxString
AlfFilterPolicy::getToHostName(void) const
{
	return (PolicyUtils::listToString(getToHostList()));
}

wxArrayString
AlfFilterPolicy::getToHostList(void) const
{
	wxArrayString	 hostList;
	struct apn_host *toHost;

	hostList.Clear();

	if (getApnRule() == NULL) {
		return (hostList);
	}

	toHost = getApnRule()->rule.afilt.filtspec.tohost;
	do {
		hostList.Add(hostToString(toHost));
		if ((toHost == NULL) || (toHost->next == NULL)) {
			break;
		}
		toHost = toHost->next;
	} while (toHost != NULL);

	return (hostList);
}

bool
AlfFilterPolicy::setToPortName(wxString toPort)
{
	return (setToPortList(PolicyUtils::stringToList(toPort)));
}

bool
AlfFilterPolicy::setToPortList(wxArrayString portList)
{
	struct apn_port **toPortPtr;

	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();
	apn_free_port(getApnRule()->rule.afilt.filtspec.toport);
	getApnRule()->rule.afilt.filtspec.toport = NULL;
	toPortPtr = &(getApnRule()->rule.afilt.filtspec.toport);

	if ((portList.GetCount() == 1) &&
	    (portList.Item(0).Cmp(wxT("any")) == 0)) {
		/* The only element is any, which is already set. */
		setModified();
		finishChange();
		return (true);
	}

	if (!portList.IsEmpty()) {
		for(size_t i=0; i<portList.GetCount(); ++i) {
			struct apn_port	*tmp;

			tmp = createApnPort(portList.Item(i));
			if (tmp == NULL) {
				setModified();
				finishChange();
				return (false);
			}
			tmp->next = NULL;
			(*toPortPtr) = tmp;
			toPortPtr = &tmp->next;
		}
	}

	setModified();
	finishChange();

	return (true);
}

wxString
AlfFilterPolicy::getToPortName(void) const
{
	return (PolicyUtils::listToString(getToPortList()));
}

wxArrayString
AlfFilterPolicy::getToPortList(void) const
{
	wxArrayString	 portList;
	struct apn_port *toPort;

	portList.Clear();

	if (getApnRule() == NULL) {
		return (portList);
	}

	toPort = getApnRule()->rule.afilt.filtspec.toport;
	do {
		portList.Add(portToString(toPort));
		if ((toPort == NULL) || (toPort->next == NULL)) {
			break;
		}
		toPort = toPort->next;
	} while (toPort != NULL);

	return (portList);
}

bool
AlfFilterPolicy::setStateTimeout(int timeout)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.afilt.filtspec.statetimeout = timeout;
	setModified();
	finishChange();

	return (true);
}

int
AlfFilterPolicy::getStateTimeoutNo(void) const
{
	int		 timeout;
	struct apn_rule	*rule;

	timeout = -1;
	rule = getApnRule();
	if (rule != NULL) {
		timeout = rule->rule.afilt.filtspec.statetimeout;
	}

	return (timeout);
}

wxString
AlfFilterPolicy::getStateTimeoutName(void) const
{
	return (wxString::Format(wxT("%d"), getStateTimeoutNo()));
}

wxString
AlfFilterPolicy::getRoleName(void) const
{
	wxString role;

	switch (getDirectionNo()) {
	case APN_SEND:
		/* FALLTHROUGH */
	case APN_CONNECT:
		role = wxT("client");
		break;
	case APN_RECEIVE:
		/* FALLTHROUGH */
	case APN_ACCEPT:
		role = wxT("server");
		break;
	case APN_BOTH:
		role = wxT("both");
		break;
	default:
		role = _("(unknown)");
		break;
	}

	return (role);
}

wxString
AlfFilterPolicy::getServiceName(void) const
{
	wxString service;

	switch (getDirectionNo()) {
	case APN_SEND:
		/* FALLTHROUGH */
	case APN_CONNECT:
		service.Printf(_("to %s"), getToHostName().c_str());
		break;
	case APN_RECEIVE:
		/* FALLTHROUGH */
	case APN_ACCEPT:
		service.Printf(_("from %s"), getFromHostName().c_str());
		break;
	case APN_BOTH:
		service.Printf(_("both from %s to %s"),
		    getFromHostName().c_str(), getToHostName().c_str());
		break;
	default:
		service = _("(unknown)");
		break;
	}

	return (service);
}

wxString
AlfFilterPolicy::hostToString(struct apn_host *host) const
{
	wxString	hostString;
	char		ipAddrBuffer[256];

	if (host == NULL) {
		hostString = wxT("any");
	} else {
		if (host->negate) {
			hostString += wxT("! ");
		}
		inet_ntop(host->addr.af, &(host->addr.apa.addr32),
		    ipAddrBuffer, sizeof(ipAddrBuffer));
		hostString += wxString::From8BitData(ipAddrBuffer);
		if (host->addr.len != 32 && host->addr.len != 128) {
			hostString += wxString::Format(wxT("/%u"),
			    host->addr.len);
		}
	}

	return (hostString);
}

wxString
AlfFilterPolicy::portToString(struct apn_port *port) const
{
	wxString portString;

	if (port == NULL) {
		portString = wxT("any");
	} else if (port->port2) {
		portString = wxString::Format(wxT("%hu - %hu"),
		    ntohs(port->port), ntohs(port->port2));
	} else {
		portString = wxString::Format(wxT("%hu"), ntohs(port->port));
	}

	return (portString);
}
