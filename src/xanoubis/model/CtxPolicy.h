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

#ifndef _CTXPOLICY_H_
#define _CTXPOLICY_H_

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

class CtxPolicy : public Policy
{
	DECLARE_DYNAMIC_CLASS(CtxPolicy)

	private:
		struct apn_rule		*ctxRule_;
		bool			 modified_;
		wxString		 currHash_;
		unsigned char		*currSum_;

	public:
		CtxPolicy(void);
		CtxPolicy(PolicyRuleSet *);
		CtxPolicy(AppPolicy *, struct apn_rule *, PolicyRuleSet *);
		~CtxPolicy(void);

		virtual void	accept(PolicyVisitor&);
		virtual bool	isDefault(void) { return false; }

		int		getId(void);
		int		getTypeNo(void);
		wxString	getBinaryName(void);
		wxString	getHashTypeName(void);
		wxString	getHashValue(void);
		wxString	convertToString(unsigned char *);
		void		setBinaryName(wxString name);
		bool		isModified(void);
		void		setModified(bool);
		void		setHashValue(unsigned char *);
		unsigned char*	getCurrentSum(void);
		void		setCurrentSum(unsigned char *);
		wxString	getCurrentHash(void);
		void		setCurrentHash(wxString currHash);
		int		calcCurrentHash(unsigned char *);
};

#endif	/* _CTXPOLICY_H_ */