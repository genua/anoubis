/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include <wx/intl.h>

#include <Service.h>

#include "RuleWizardServiceProperty.h"

RuleWizardServiceProperty::RuleWizardServiceProperty(Type type)
{
	this->type_ = type;
}

wxString
RuleWizardServiceProperty::getHeader(void) const
{
	switch (type_) {
	case NAME: return _("Servicename");
	case PORT: return _("Portnumber");
	case PROTOCOL: return _("Protocol");
	case DEFAULT: return _("Standard");
	}

	return _("(null)");
}

wxString
RuleWizardServiceProperty::getText(AnListClass *obj) const
{
	Service *service = dynamic_cast<Service *>(obj);

	if (service == 0)
		return _("(null)");

	switch (type_) {
	case NAME:
		return (service->getName());
	case PORT:
		return (wxString::Format(wxT("%d"), service->getPort()));
	case PROTOCOL:
		return (service->getProtocol() == Service::TCP ?
		    wxT("tcp") : wxT("udp"));
	case DEFAULT:
		return (wxEmptyString);
	}

	return _("(null)");
}

AnIconList::IconId
RuleWizardServiceProperty::getIcon(AnListClass *obj) const
{
	Service *service = dynamic_cast<Service *>(obj);

	if (service != 0 && type_ == DEFAULT && service->isDefault())
		return (AnIconList::ICON_OK);
	else
		return (AnIconList::ICON_NONE);
}
