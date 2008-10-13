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

#ifndef _SFSPOLICY_H_
#define _SFSPOLICY_H_

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

class SfsPolicy : public Policy
{
	DECLARE_DYNAMIC_CLASS(SfsPolicy)

	private:
		struct apn_rule		*sfsRule_;
		wxString		 currHash_;
		unsigned char		*currSum_;
		bool			 modified_;

	public:
		SfsPolicy(void);
		SfsPolicy(PolicyRuleSet *);
		SfsPolicy(AppPolicy *, struct apn_rule *, PolicyRuleSet *);
		~SfsPolicy(void);

		virtual void accept(PolicyVisitor&);

		int		 getId(void);

		void		 setBinaryName(wxString);
		wxString	 getBinaryName(void);
		wxString	 getHashTypeName(void);

		void		 setCurrentSum(unsigned char*);
		unsigned char	*getCurrentSum(void);
		wxString	 getCurrentHash(void);
		void		 setCurrentHash(wxString);
		int		 calcCurrentHash(unsigned char *);
		void		 setHashValue(unsigned char *);
		wxString	 getHashValue(void);

		bool		 isModified(void);
		void		 setModified(bool);
		wxString	 convertToString(unsigned char*);
		bool		 isDefault(void);
		int		 getLogNo(void);
		wxString	 getLogName(void);
		void		 setLogNo(int);
};

#endif	/* _SFSPOLICY_H_ */
