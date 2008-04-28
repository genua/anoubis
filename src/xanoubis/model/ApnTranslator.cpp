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

#include "ApnTranslator.h"

ApnTranslator::ApnTranslator(void)
{
	/* Nothing needs to be done here. */
}

ApnTranslator::~ApnTranslator(void)
{
	/* Nothing needs to be done here. */
}

wxString
ApnTranslator::direction(int direction)
{
	wxString	result;

	switch (direction) {
	case APN_CONNECT:
		result = wxT("connect");
		break;
	case APN_ACCEPT:
		result = wxT("accept");
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}

int
ApnTranslator::direction(wxString direction)
{
	int	result;

	if (direction.Cmp(wxT("accept")) == 0) {
		result = APN_ACCEPT;
	} else {
		result = APN_CONNECT;
	}

	return (result);
}

wxString
ApnTranslator::action(int action)
{
	wxString	result;

	switch (action) {
	case APN_ACTION_ALLOW:
		result = wxT("allow");
		break;
	case APN_ACTION_DENY:
		result = wxT("deny");
		break;
	case APN_ACTION_ASK:
		result = wxT("ask");
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}

int
ApnTranslator::action(wxString action)
{
	int	result;

	if (action.Cmp(wxT("allow")) == 0) {
		result = APN_ACTION_ALLOW;
	} else if (action.Cmp(wxT("ask")) == 0) {
		result = APN_ACTION_ASK;
	} else {
		result = APN_ACTION_DENY;
	}

	return (result);
}

wxString
ApnTranslator::log(int log)
{
	wxString	result;

	switch (log) {
	case APN_LOG_NONE:
		result = wxT("none");
		break;
	case APN_LOG_NORMAL:
		result = wxT("normal");
		break;
	case APN_LOG_ALERT:
		result = wxT("alert");
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}

int
ApnTranslator::log(wxString log)
{
	int	result;

	if (log.Cmp(wxT("none")) == 0) {
		result = APN_LOG_NONE;
	} else if (log.Cmp(wxT("normal")) == 0) {
		result = APN_LOG_NORMAL;
	} else {
		result = APN_LOG_ALERT;
	}

	return (result);
}

wxString
ApnTranslator::varType(int type)
{
	wxString	result;

	switch (type) {
	case VAR_APPLICATION:
		result = wxT("Application");
		break;
	case VAR_RULE:
		result = wxT("Rule");
		break;
	case VAR_DEFAULT:
		result = wxT("Default");
		break;
	case VAR_HOST:
		result = wxT("Host");
		break;
	case VAR_PORT:
		result = wxT("Port");
		break;
	case VAR_FILENAME:
		result = wxT("Filename");
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}

int
ApnTranslator::varType(wxString type)
{
	int	result;

	if (type.Cmp(wxT("Application")) == 0) {
		result = VAR_APPLICATION;
	} else if (type.Cmp(wxT("Rule")) == 0) {
		result = VAR_RULE;
	} else if (type.Cmp(wxT("Default")) == 0) {
		result = VAR_DEFAULT;
	} else if (type.Cmp(wxT("Host")) == 0) {
		result = VAR_HOST;
	} else if (type.Cmp(wxT("Port")) == 0) {
		result = VAR_PORT;
	} else {
		result = VAR_FILENAME;
	}

	return (result);
}

wxString
ApnTranslator::ruleType(int type)
{
	wxString	result;

	switch (type) {
	case APN_ALF:
		result = wxT("alf");
		break;
	case APN_SFS:
		result = wxT("sfs");
		break;
	case APN_SB:
		result = wxT("sb");
		break;
	case APN_VS:
		result = wxT("vs");
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}

int
ApnTranslator::ruleType(wxString type)
{
	int	result;

	if (type.Cmp(wxT("alf")) == 0) {
		result = APN_ALF;
	} else if (type.Cmp(wxT("sfs")) == 0) {
		result = APN_SFS;
	} else if (type.Cmp(wxT("sb")) == 0) {
		result = APN_SB;
	} else {
		result = APN_VS;
	}

	return (result);
}

wxString
ApnTranslator::alfCap(int cap)
{
	wxString	result;

	switch (cap) {
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

int
ApnTranslator::alfCap(wxString cap)
{
	int	result;

	if (cap.Cmp(wxT("raw")) == 0) {
		result = APN_ALF_CAPRAW;
	} else if (cap.Cmp(wxT("other")) == 0) {
		result = APN_ALF_CAPOTHER;
	} else {
		result = APN_ALF_CAPALL;
	}

	return (result);
}

wxString
ApnTranslator::alfType(int type)
{
	wxString	result;

	switch (type) {
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

int
ApnTranslator::alfType(wxString type)
{
	int	result;

	if (type.Cmp(wxT("filter")) == 0) {
		result = APN_ALF_FILTER;
	} else if (type.Cmp(wxT("capability")) == 0) {
		result = APN_ALF_CAPABILITY;
	} else if (type.Cmp(wxT("default")) == 0) {
		result = APN_ALF_DEFAULT;
	} else {
		result = APN_ALF_CTX;
	}

	return (result);
}

wxString
ApnTranslator::hashType(int type)
{
	wxString	result;

	switch (type) {
	case APN_HASH_SHA256:
		result = wxT("SHA256");
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}

int
ApnTranslator::hashType(wxString type)
{
	int	result;

	result = APN_HASH_SHA256;

	return (result);
}

wxString
ApnTranslator::prettyPrintHash(int type, char *hash)
{
	wxString	result;
	unsigned int	length;

	length = 0;
	result = wxT("0x");

	switch (type) {
	case APN_HASH_SHA256:
		length = APN_HASH_SHA256_LEN;
		break;
	default:
		length = 0;
		result = wxT("(unknown hash type)");
		break;
	}

	for (unsigned int i=0; i<length; i++) {
		result += wxString::Format(wxT("%02x"), hash[i]);
	}

	return (result);
}

wxString
ApnTranslator::ipProtocol(int proto)
{
	wxString result;

	switch (proto) {
	case 0:
		result = wxT("any");
		break;
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

int
ApnTranslator::ipProtocol(wxString protocol)
{
	int result;

	if (protocol.Cmp(wxT("tcp")) == 0) {
		result = IPPROTO_TCP;
	} else if (protocol.Cmp(wxT("udp")) == 0) {
		 result = IPPROTO_UDP;
	} else {
		result = 0;
	}

	return (result);
}

wxString
ApnTranslator::addrFamily(int af)
{
	wxString result;

	switch (af) {
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
		result = wxT("(unknown");
		break;
	}

	return (result);
}

int
ApnTranslator::addrFamily(wxString af)
{
	int result;

	if (af.Cmp(wxT("inet6")) == 0) {
		result = AF_INET6;
	} else if (af.Cmp(wxT("inet")) == 0) {
		result = AF_INET;
	} else {
		result = 0;
	}

	return (result);
}
