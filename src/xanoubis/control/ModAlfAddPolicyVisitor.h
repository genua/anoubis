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

#ifndef _MODALFADDPOLICYVISITOR_H_
#define _MODALFADDPOLICYVISITOR_H_

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

#include "ModAlfMainPanelImpl.h"
#include "Policy.h"
#include "PolicyVisitor.h"
#include "AppPolicy.h"
#include "AlfPolicy.h"
#include "VarPolicy.h"

class ModAlfAddPolicyVisitor : public PolicyVisitor
{
	private:
		ModAlfMainPanelImpl	*alfPanel_;

		long		ruleListAppend(Policy *);

	public:
		ModAlfAddPolicyVisitor(ModAlfMainPanelImpl *);
		~ModAlfAddPolicyVisitor(void);

		virtual void visitAppPolicy(AppPolicy *);
		virtual void visitAlfPolicy(AlfPolicy *);
		virtual void visitSfsPolicy(SfsPolicy *);
		virtual void visitVarPolicy(VarPolicy *);
};

#endif	/* _MODALFADDPOLICYVISITOR_H_ */
