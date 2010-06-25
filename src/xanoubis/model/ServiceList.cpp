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

#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/textfile.h>

#include <netdb.h>

#include "Service.h"
#include "ServiceList.h"

ServiceList::~ServiceList(void)
{
	clearServiceList(true);
}

unsigned int
ServiceList::getServiceCount(void) const
{
	return (serviceList_.size());
}

Service *
ServiceList::getServiceAt(unsigned int idx) const
{
	if (!serviceList_.empty() && idx < serviceList_.size())
		return (serviceList_[idx]);
	else
		return (0);
}

int
ServiceList::getIndexOf(Service *service) const
{
	if (service == 0)
		return (-1);

	for (unsigned int i = 0; i < serviceList_.size(); i++) {
		if (serviceList_[i] == service)
			return (i);
	}

	return (-1);
}

void
ServiceList::addService(Service *service)
{
	if (service == 0)
		return;

	if (service->parent_ != 0) {
		/*
		 * A parent is already assigned. Unlink from there.
		 */
		service->parent_->removeService(service);
	}

	service->parent_ = this;
	serviceList_.push_back(service);

	sizeChangeEvent(serviceList_.size());
}

bool
ServiceList::removeService(Service *service)
{
	if (service == 0)
		return (false);

	std::vector<Service *>::iterator it = serviceList_.begin();

	for (; it != serviceList_.end(); ++it) {
		Service *s (*it);
		if (service == s) {
			serviceList_.erase(it);
			service->parent_ = 0;

			sizeChangeEvent(serviceList_.size());
			return (true);
		}
	}

	return (false);
}

void
ServiceList::clearServiceList(bool destroy)
{
	bool empty = serviceList_.empty();

	while (!serviceList_.empty()) {
		Service *s = serviceList_.back();
		serviceList_.pop_back();

		if (destroy)
			delete s;
		else
			s->parent_ = 0;
	}

	if (!empty)
		sizeChangeEvent(0);
}

bool
ServiceList::canHaveDefaultServices(void) const
{
	wxString file =
	    wxStandardPaths::Get().GetDataDir() + wxT("/profiles/wizard/alf");
	return (wxFileName::IsFileReadable(file));
}

void
ServiceList::assignDefaultServices(void)
{
	unsigned int	portNr;
	wxString	name;
	wxString	port;
	wxString	prot;
	wxString	line;
	wxTextFile	file;
	struct servent *entry;

	clearServiceList(true);

	file.Open(
	    wxStandardPaths::Get().GetDataDir() + wxT("/profiles/wizard/alf"));

	if (!file.IsOpened()) {
		/* alf defaults file does not exists - return empty list */
		return;
	}

	setservent(1);
	portNr = 0;

	line = file.GetFirstLine().BeforeFirst('#');
	line.Trim(true);
	line.Trim(false);
	while (!file.Eof()) {
		if (!line.IsEmpty()) {
			/* Slice the string */
			prot = line.BeforeFirst('/'); /* protocol name */
			prot.Trim(true);
			prot.Trim(false);

			port = line.AfterFirst('/');  /* port number */
			port.Trim(true);
			port.Trim(false);
			port.ToLong((long *)&portNr);

			/* Fetch service name */
			entry = getservbyport(htons((int)portNr),
			    prot.fn_str());
			if (entry != NULL) {
				name = wxString::From8BitData(entry->s_name);
			} else {
				name = _("(unknown)");
			}

			/* Create new service */
			Service *service = new Service(name, portNr,
			    (prot == wxT("tcp") ? Service::TCP : Service::UDP),
			    true);

			addService(service);
		}
		line = file.GetNextLine().BeforeFirst('#');
		line.Trim(true);
		line.Trim(false);
	}

	endservent();
	file.Close();

	sizeChangeEvent(serviceList_.size());
}

void
ServiceList::assignSystemServices(void)
{
	struct servent	*entry;

	clearServiceList(true);

	/* Open /etc/services */
	entry = getservent();

	while (entry != NULL) {
		/* Only show protocol tcp and udp. */
		wxString prot = wxString::From8BitData(entry->s_proto);
		if (prot == wxT("tcp") || prot == wxT("udp")) {
			Service *service = new Service(
			    wxString::From8BitData(entry->s_name),
			    entry->s_port,
			    (prot == wxT("tcp") ? Service::TCP : Service::UDP),
			    false);

			addService(service);
		}

		entry = getservent();
	}

	/* Close /etc/services */
	endservent();

	sizeChangeEvent(serviceList_.size());
}

AnListClass *
ServiceList::getRow(unsigned int idx) const
{
	return (getServiceAt(idx));
}

int
ServiceList::getSize(void) const
{
	return (getServiceCount());
}
