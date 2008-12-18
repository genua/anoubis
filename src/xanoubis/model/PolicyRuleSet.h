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

#ifndef _POLICYRULESET_H_
#define _POLICYRULESET_H_

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

#include "EscalationNotify.h"
#include "Policy.h"
#include "AlfPolicy.h"

class PolicyRuleSet
{
	private:
		int			 priority_;
		wxString		 origin_;
		long			 id_;
		uid_t			 uid_;
		bool			 hasErrors_;
		bool			 isModified_;
		struct apn_ruleset	*ruleSet_;
		PolicyList		 alfList_;
		PolicyList		 sfsList_;
		PolicyList		 ctxList_;
		PolicyList		 varList_;

		void clean(void);
		void create(wxString, bool);
		void create(struct apn_ruleset *);
		bool hasLocalHost(wxArrayString);
		int createAppPolicy(int type, int id);
		void log(const wxString &);
		void status(const wxString &);

		struct apn_rule *assembleAlfPolicy(AlfPolicy *,
		   EscalationNotify *);

	public:
		PolicyRuleSet(int, uid_t, struct apn_ruleset *);
		PolicyRuleSet(int, uid_t, wxString, bool);
		~PolicyRuleSet(void);

		void accept(PolicyVisitor&);
		void exportToFile(wxString);

		void createAnswerPolicy(EscalationNotify *);

		int createAlfAppPolicy(int id) {
			return createAppPolicy(APN_ALF, id);
		};
		int createSBAppPolicy(int id) {
			return createAppPolicy(APN_SB, id);
		};
		int createCtxAppPolicy(int id) {
			return createAppPolicy(APN_CTX, id);
		};
		int createAlfPolicy(int);
		int createCtxNewPolicy(int);
		int createSfsPolicy(int);

		void clearModified(void);
		bool findMismatchHash(void);
		bool deletePolicy(int);
		bool isEmpty(void);
		bool isAdmin(void);

		wxString getOrigin(void);
		wxString toString(void) const;
		long getId(void) const;
		uid_t getUid(void) const;
		int getPriority(void) const;
		bool hasErrors(void) const;
		bool isModified(void) const;
		void setModified(bool);
};

#endif	/* _POLICYRULESET_H_ */
