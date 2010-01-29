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

#ifndef _SERVICE_H_
#define _SERVICE_H_

#include <AnListClass.h>

class ServiceList;

/**
 * A network-service.
 *
 * A network-service is identified by a name (getName()), is listening on a
 * port (getPort()) and is using a protocol (getProtocol()). The default-flag
 * (getDefault()) shows, if the service comes from the default-list
 * (ServiceList::assignDefaultServices()).
 *
 * Instances of Service as managed by ServiceList.
 */
class Service : public AnListClass
{
	public:
		/**
		 * Protocol-enumeration.
		 */
		enum Protocol {
			TCP,	/**< TCP */
			UDP	/**< UDP */
		};

		/**
		 * Default-c'tor.
		 *
		 * Creates a new service. The instance still needs to be
		 * assigned to a service-list.
		 *
		 * @param name Name of service
		 * @param port Port of service
		 * @param protocol Underlaying protocol
		 * @param def Set to true, if this service was loaded by the
		 *            default-configuration.
		 */
		Service(const wxString &, unsigned int, Protocol, bool);

		/**
		 * D'tor.
		 *
		 * If the service is assigned at a service-list, it instance
		 * is removed from there before.
		 */
		~Service(void);

		/**
		 * Returns the parent list-instance.
		 *
		 * If the service is linked with a ServiceList, this method
		 * returns the parent list-instance. But if the service is
		 * unlinked, NULL is returned.
		 *
		 * @return The parent service-list (if any).
		 */
		ServiceList *getParent(void) const;

		/**
		 * Returns the name of the service.
		 * @return Name of service
		 */
		wxString getName(void) const;

		/**
		 * Returns the port of the service.
		 * @return Port of service
		 */
		unsigned int getPort(void) const;

		/**
		 * Returns the protocol of the service.
		 *
		 * @return The protocol
		 */
		Protocol getProtocol(void) const;

		/**
		 * Flags shows, if the service comes from the
		 * default-configuration.
		 *
		 * @return true, if this service was created from the
		 *         default-configuration.
		 * @see ServiceList::assignDefaultServices()
		 */
		bool isDefault(void) const;

	private:
		/**
		 * The parent service-list (if any).
		 */
		ServiceList *parent_;

		/**
		 * Name of service
		 */
		wxString name_;

		/**
		 * Port of service
		 */
		unsigned int port_;

		/**
		 * Protocol of service
		 */
		Protocol protocol_;

		/**
		 * Default-configuration flag
		 */
		bool default_;

	friend class ServiceList;
};

#endif	/* _SERVICE_H_ */
