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

#ifndef _ALFPOLICY_H_
#define _ALFPOLICY_H_

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
#include <wx/arrstr.h>

#include <apn.h>

#include "Policy.h"
#include "AppPolicy.h"
#include "PolicyVisitor.h"

class AlfPolicy : public Policy
{
	DECLARE_DYNAMIC_CLASS(AlfPolicy)

	private:
		struct apn_rule		*alfRule_;

		wxString	getHostName(struct apn_host *);
		wxString        getPortName(struct apn_port *);
		wxString        listToString(wxArrayString);

	public:
		AlfPolicy(void);
		AlfPolicy(PolicyRuleSet *);
		AlfPolicy(AppPolicy *, struct apn_rule *, PolicyRuleSet *);
		~AlfPolicy(void);

		virtual void		 accept(PolicyVisitor&);
		struct apn_rule		*cloneRule(void);

		int		getId(void);
		bool		isDefault(void);

		void		setType(int);
		int		getTypeNo(void);
		wxString	getTypeName(void);

		void		setAction(int);
		int		getActionNo(void);
		wxString	getActionName(void);

		wxString	getRoleName(void);
		wxString	getServiceName(void);

		bool		hasScope(void);
		wxString	getScopeName(void);

		/* valid for type one of APN_ALF_{FILTER,CAPABILITY,DEFAULT} */
		void		setLogNo(int);
		int		getLogNo(void);
		wxString	getLogName(void);

		/* valid for type == APN_ALF_FILTER */
		void		setDirection(int);
		int		getDirectionNo(void);
		wxString	getDirectionName(void);

		void		setProtocol(int);
		int		getProtocolNo(void);
		wxString	getProtocolName(void);

		void		setAddrFamily(int);
		int		getAddrFamilyNo(void);
		wxString	getAddrFamilyName(void);

		wxString	getFromHostName(void);
		wxArrayString	getFromHostList(void);

		wxArrayString	getFromPortList(void);
		wxString	getFromPortName(void);

		wxString	getToHostName(void);
		wxArrayString	getToHostList(void);

		wxArrayString	getToPortList(void);
		wxString	getToPortName(void);

		void		setStateTimeout(int);
		wxString	getStateTimeout(void);

		/* valid for type == APN_ALF_CAPABILITY */
		void		setCapType(int);
		int		getCapTypeNo(void);
		wxString	getCapTypeName(void);

		wxString        getSrcNetmask(void);
		wxString        getDstNetmask(void);

		void		setAlfSrcAddress(wxString, int, int);
		void		setAlfDstAddress(wxString, int, int);

		void		setAlfSrcPort(int);
		void		setAlfDstPort(int);
};

#endif	/* _ALFPOLICY_H_ */
