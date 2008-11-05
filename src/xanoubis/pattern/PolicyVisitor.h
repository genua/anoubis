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

#ifndef _POLICYVISITOR_H_
#define _POLICYVISITOR_H_

#include "AppPolicy.h"
#include "AlfPolicy.h"
#include "CtxPolicy.h"
#include "SfsPolicy.h"
#include "VarPolicy.h"

class PolicyVisitor
{
	private:
		bool	propagate_;
		bool	isAdmin_;

	public:
		PolicyVisitor(void);
		virtual ~PolicyVisitor(void);

		virtual void setPropagation(bool);
		virtual bool shallBeenPropagated(void);
		virtual void setAdmin(bool);
		virtual bool isAdmin(void);

		virtual void visitAppPolicy(AppPolicy *) = 0;
		virtual void visitAlfPolicy(AlfPolicy *) = 0;
		virtual void visitCtxPolicy(CtxPolicy *) = 0;
		virtual void visitSfsPolicy(SfsPolicy *) = 0;
		virtual void visitVarPolicy(VarPolicy *) = 0;
};

#endif	/* _POLICYVISITOR_H_ */
