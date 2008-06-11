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

#ifndef _APPPOLICY_H_
#define _APPPOLICY_H_

#include <wx/string.h>

#include "Policy.h"
class AlfPolicy;

class AppPolicy : public Policy
{
	private:
		PolicyList	 ruleList_;
		wxString	 appName_;
		AlfPolicy	*context_;
		struct apn_rule	*appRule_;

		wxString	 currHash_;

	public:
		AppPolicy(struct apn_rule *);
		~AppPolicy(void);

		virtual void accept(PolicyVisitor&);

		wxString	 getApplicationName(void);
		void		 setApplicationName(wxString);
		wxString	 getBinaryName(void);
		void		 setBinaryName(wxString name);
		wxString	 getCurrentHash(void);
		void		 setCurrentHash(wxString);
		bool		 calcCurrentHash(unsigned char *);
		wxString	 getHashTypeName(void);
		void		 setHashValue(unsigned char *);
		wxString	 getHashValue(void);
		AlfPolicy	*getContext(void);
		bool		 hasContext(void);
		int		 getType(void);
};

#endif	/* _APPPOLICY_H_ */
