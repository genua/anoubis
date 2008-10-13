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
#include <wx/arrstr.h>
#include <wx/intl.h>
#include <wx/utils.h>

#include <apn.h>

#include "main.h"
#include "Policy.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(PolicyList);

IMPLEMENT_CLASS(Policy, wxObject)

Policy::Policy(PolicyRuleSet *rsParent)
{
	parent_ = NULL;
	index_ = 0;
	rsParent_ = rsParent;
}

Policy::Policy(PolicyRuleSet *rsParent, Policy *parent)
{
	parent_ = parent;
	index_ = 0;
	rsParent_ = rsParent;
}

Policy::~Policy(void)
{
}

Policy *
Policy::getParent(void)
{
	return (parent_);
}

PolicyRuleSet *
Policy::getRsParent(void)
{
	return (rsParent_);
}

wxString
Policy::getActionName(int action)
{
	wxString result;

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
		result = _("(unknown)");
		break;
	}

	return (result);
}

wxString
Policy::getDirectionName(int direction)
{
	wxString result;

	switch (direction) {
	case APN_CONNECT:
		result = wxT("connect");
		break;
	case APN_ACCEPT:
		result = wxT("accept");
		break;
	case APN_SEND:
		result = wxT("send");
		break;
	case APN_RECEIVE:
		result = wxT("receive");
		break;
	case APN_BOTH:
		result = wxT("");
		break;
	default:
		result = _("(unknown)");
		break;
	}

	return (result);
}

wxString
Policy::getLogName(int log)
{
	wxString result;

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
		result = _("(unknown)");
		break;
	}

	return (result);
}

wxString
Policy::getVarTypeName(int type)
{
	wxString result;

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
		result = _("(unknown)");
		break;
	}

	return (result);
}

wxString
Policy::getRuleTypeName(int type)
{
	wxString result;

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
		result = _("(unknown)");
		break;
	}

	return (result);
}

wxString
Policy::guessAppName(wxString binary)
{
	wxString        result;
	wxString        command;
	wxArrayString   output;
	wxArrayString   errors;

	command = wxT("sh ");
	command += wxGetApp().getUtilsPath() + wxT("/xanoubis_guessApp.sh ");
	command += binary;

	if (wxExecute(command, output, errors) == 0) {
		result = output.Item(0).BeforeFirst('#');
	} else {
		result = _("unknown");
	}
	return (result);
}

unsigned long
Policy::getIndex(void)
{
	return (index_);
}

void
Policy::setIndex(unsigned long index)
{
	index_ = index;
}
