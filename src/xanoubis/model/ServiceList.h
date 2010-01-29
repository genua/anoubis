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

#ifndef _SERVICELIST_H_
#define _SERVICELIST_H_

#include <vector>

#include <AnRowProvider.h>

class Service;

class ServiceList : public AnRowProvider
{
	public:
		~ServiceList(void);

		/**
		 * Returns the number of assigned services.
		 *
		 * @return Number of assigned services
		 */
		unsigned int getServiceCount(void) const;

		/**
		 * Returns the service at the given index.
		 *
		 * @param idx The requeste index
		 * @return The service is the requested index. If the index is
		 *         out of range, NULL is returned.
		 */
		Service *getServiceAt(unsigned int) const;

		/**
		 * Returns the index, where the given service is assigned.
		 *
		 * @param service The requested service
		 * @return The index of the service. -1 is returned, of the
		 *         requested service is not assigned.
		 */
		int getIndexOf(Service *) const;

		/**
		 * Links the given service at the list.
		 *
		 * If the service is already appended to another list, it is
		 * unlinked before.
		 *
		 * @param service Service to be appended
		 */
		void addService(Service *);

		/**
		 * Unlinks the given service from the list.
		 *
		 * The instance is not destroyed! The caller has the
		 * responsibility to destroy the instance manually!
		 *
		 * @param service The instance to be unlinked
		 */
		bool removeService(Service *);

		/**
		 * Unlinks (any possibly destroys) all services from the list.
		 *
		 * @param destroy If set to true the instances are additionally
		 *                destroyed.
		 */
		void clearServiceList(bool);

		/**
		 * Tests whether you are able to load services from the
		 * default-configuration.
		 *
		 * If the related configuration-file exists, the method
		 * returnes true.
		 *
		 * @return true if you can load the default-services.
		 * @see assignDefaultServices()
		 */
		bool canHaveDefaultServices(void) const;

		/**
		 * Assigns services from the default-configuration.
		 *
		 * The administrator has the possibility to provide a list with
		 * default-services. The file is usually located under
		 * <code>/usr/share/xanoubis/profiles/wizard/alf</code>.
		 *
		 * If the file exists, previously assigned services are
		 * destroyed, the content of the file parsed, and the services
		 * are assigned to the list.
		 */
		void assignDefaultServices(void);

		/**
		 * Assigns system-services.
		 *
		 * Usually system-services are configured in
		 * <code>/etc/services</code>.
		 *
		 * Previously assigned service are destroyed and the
		 * system-services are assigned to the list.
		 */
		void assignSystemServices(void);

		/**
		 * Implementation of AnRowProvider::getRow().
		 *
		 * Simply calls getServiceAt().
		 * @see getServiceAt()
		 */
		AnListClass *getRow(unsigned int idx) const;

		/**
		 * Implementation of AnRowProvider::getSize().
		 *
		 * Simply calls getServiceCount().
		 *
		 * @see getServiceCount()
		 */
		int getSize(void) const;

	private:
		/**
		 * Internal list of services
		 */
		std::vector<Service *> serviceList_;

	friend class Service;
};

#endif	/* _SERVICELIST_H_ */
