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

#ifndef _ALFFILTERPOLICY_H_
#define _ALFFILTERPOLICY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FilterPolicy.h"
class AlfAppPolicy;

/**
 * This is an Alf filter policy.
 */
class AlfFilterPolicy : public FilterPolicy
{
	DECLARE_CLASS(AlfFilterPolicy)

	public:
		/**
		 * Constructor of a AlfFilterPolicy.
		 * @param[in] 1st The parent alf application policy.
		 * @param[in] 2nd The apn_rule this policy should represent.
		 */
		AlfFilterPolicy(AppPolicy *, struct apn_rule *);

		/**
		 * Constructor of an empty AlfFilterPolicy.
		 * It's the duty of the caller to add this policy to the
		 * parent AlfAppPolicy by prependFilterPolicy().
		 * @param[in] 1st The parent alf application policy.
		 */
		AlfFilterPolicy(AlfAppPolicy *);

		/**
		 * Get the policy type as string.
		 * @param None.
		 * @return String with the policy type.
		 */
		virtual wxString getTypeIdentifier(void) const;

		/**
		 * Accept a visitor.
		 * @param[in] 1st The visitor.
		 * @return Nothing.
		 */
		virtual void accept(PolicyVisitor &);

		/**
		 * Create native / apn rule.
		 * @param None.
		 * @return A single apn rule.
		 */
		static struct apn_rule *createApnRule(void);

		/**
		 * Create native / apn host structure.
		 * @param None.
		 * @return A single apn structure.
		 */
		static struct apn_host *createApnHost(wxString, int);

		/**
		 * Create native / apn port structure.
		 * @param None.
		 * @return A single apn structure.
		 */
		static struct apn_port *createApnPort(wxString);

		/**
		 * Create apn rule and insert to apn ruleset.
		 * This will create a new apn_rule (by createApnRule()).
		 * This new rule is inserted to the apn_ruleset been
		 * determined by the given parent. On success the apn_ruleset
		 * is modified and out of sync with the policy object tree.
		 * It's the responsibility of the caller to re-create the
		 * object tree.
		 * @param[in] 1st The parent policy.
		 * @param[in] 2nd Insert before this id (on top if id=0)
		 * @return True on success.
		 */
		static bool createApnInserted(AppPolicy *, unsigned int);

		/**
		 * Set the apn log type.
		 * @param[in] 1st The apn log type.
		 * @return True on success.
		 */
		virtual bool setLogNo(int);

		/**
		 * Get the apn log type.
		 * @param None.
		 * @return The apn log type.
		 */
		virtual int getLogNo(void) const;

		/**
		 * Set the apn action type.
		 * @param[in] 1st The apn action type.
		 * @return True on success.
		 */
		virtual bool setActionNo(int);

		/**
		 * Get the apn action type.
		 * @param None.
		 * @return The apn action type.
		 */
		virtual int getActionNo(void) const;

		/**
		 * Set the apn direction type.
		 * @param[in] 1st The apn direction type.
		 * @return True on success.
		 */
		bool setDirectionNo(int);

		/**
		 * Get the apn direction type.
		 * @param None.
		 * @return The apn direction type.
		 */
		int getDirectionNo(void) const;

		/**
		 * Get the string representation of direction type.
		 * @param None.
		 * @return The string of direction type.
		 */
		wxString getDirectionName(void) const;

		/**
		 * Set the apn protocol type.
		 * @param[in] 1st The apn protocol type.
		 * @return True on success.
		 */
		bool setProtocol(int);

		/**
		 * Get the apn protocol type.
		 * @param None.
		 * @return The apn protocol type.
		 */
		int getProtocolNo(void) const;

		/**
		 * Get the string representation of protocol type.
		 * @param None.
		 * @return The string of protocol type.
		 */
		wxString getProtocolName(void) const;

		/**
		 * Set IP of from host.
		 * @param[in] 1st String with ip address.
		 * @return True on success.
		 */
		bool setFromHostName(wxString);

		/**
		 * Set list of IPs of from hosts.
		 * @param[in] 1st List of strings with ip addresses.
		 * @return True on success.
		 */
		bool setFromHostList(wxArrayString);

		/**
		 * Get the from host ip address.
		 * @param None.
		 * @return From host ip (may be a list).
		 */
		wxString getFromHostName(void) const;

		/**
		 * Get the form host ip address list.
		 * @param None.
		 * @return List of from host ip's.
		 */
		wxArrayString getFromHostList(void) const;

		/**
		 * Set IP of from port.
		 * @param[in] 1st String with port.
		 * @return True on success.
		 */
		bool setFromPortName(wxString);

		/**
		 * Set list of from ports.
		 * @param[in] 1st List of strings with ports.
		 * @return True on success.
		 */
		bool setFromPortList(wxArrayString);

		/**
		 * Get the from port.
		 * @param None.
		 * @return From port (may be a list).
		 */
		wxString getFromPortName(void) const;

		/**
		 * Get the form ports list.
		 * @param None.
		 * @return List of from ports.
		 */
		wxArrayString getFromPortList(void) const;

		/**
		 * Set IP of to host.
		 * @param[in] 1st String with ip address.
		 * @return True on success.
		 */
		bool setToHostName(wxString);

		/**
		 * Set list of IPs of to hosts.
		 * @param[in] 1st List of strings with ip addresses.
		 * @return True on success.
		 */
		bool setToHostList(wxArrayString);

		/**
		 * Get the to host ip address.
		 * @param None.
		 * @return To host ip (may be a list).
		 */
		wxString getToHostName(void) const;

		/**
		 * Get the to host ip address list.
		 * @param None.
		 * @return List of to host ip's.
		 */
		wxArrayString getToHostList(void) const;

		/**
		 * Set IP of to port.
		 * @param[in] 1st String with port.
		 * @return True on success.
		 */
		bool setToPortName(wxString);

		/**
		 * Get the to ports list.
		 * @param None.
		 * @return List of to ports.
		 */
		bool setToPortList(wxArrayString);

		/**
		 * Get the to port.
		 * @param None.
		 * @return To port (may be a list).
		 */
		wxString getToPortName(void) const;

		/**
		 * Get the to ports list.
		 * @param None.
		 * @return List of to ports.
		 */
		wxArrayString getToPortList(void) const;

		/**
		 * Get the role of this policy.
		 * Easy representation of direction (client/server).
		 * @param None.
		 * @return Easy string.
		 */
		wxString getRoleName(void) const;

		/**
		 * Get the service of this policy.
		 * Easy representation of form/to ip and port.
		 * @param None.
		 * @return Easy string.
		 */
		wxString getServiceName(void) const;

	private:
		/**
		 * Converts data of native apn host structure to string.
		 * @param[in] 1st The host structure in question.
		 * @return String representation of the given host.
		 */
		wxString hostToString(struct apn_host *) const;

		/**
		 * Converts data of native apn port structure to string.
		 * @param[in] 1st The port structure in question.
		 * @return String representation of the given port.
		 */
		wxString portToString(struct apn_port *) const;
};

#endif	/* _ALFFILTERPOLICY_H_ */
