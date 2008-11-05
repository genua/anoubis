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

#include "RuleSetSearchPolicyVisitor.h"

RuleSetSearchPolicyVisitor::RuleSetSearchPolicyVisitor(int id)
{
	seekId_ = id;
	seekHash_ = wxEmptyString;
	matchingPolicy_ = NULL;
}

RuleSetSearchPolicyVisitor::RuleSetSearchPolicyVisitor(wxString binary)
{
	seekId_ = 0;
	seekHash_ = binary;
	matchingPolicy_ = NULL;
}

RuleSetSearchPolicyVisitor::~RuleSetSearchPolicyVisitor(void)
{
}

void
RuleSetSearchPolicyVisitor::compare(Policy *policy)
{
	/* first match strategy */
	if (matchingPolicy_ == NULL) {
		if (policy->getId() == seekId_) {
			matchingPolicy_ = policy;
		}
	}
}

void
RuleSetSearchPolicyVisitor::compareHash(AppPolicy *appPolicy)
{
	/* first match strategy */
	if (matchingPolicy_ == NULL) {
		if (appPolicy->getHashValue().Cmp(seekHash_) == 0) {
			matchingPolicy_ = appPolicy;
		}
	}
}

Policy *
RuleSetSearchPolicyVisitor::getMatchingPolicy(void)
{
	return (matchingPolicy_);
}

bool
RuleSetSearchPolicyVisitor::hasMatchingPolicy(void)
{
	return (matchingPolicy_ != NULL);
}

void
RuleSetSearchPolicyVisitor::visitAppPolicy(AppPolicy *appPolicy)
{
	/* Never visit the SFS dummy application block itself! */
	if (appPolicy->getType() == APN_SFS)
		return;
	compareHash(appPolicy);
	compare((Policy *)appPolicy);
}

void
RuleSetSearchPolicyVisitor::visitAlfPolicy(AlfPolicy *alfPolicy)
{
	compare((Policy *)alfPolicy);
}

void
RuleSetSearchPolicyVisitor::visitCtxPolicy(CtxPolicy *ctxPolicy)
{
	compare((Policy *)ctxPolicy);
}

void
RuleSetSearchPolicyVisitor::visitSfsPolicy(SfsPolicy *sfsPolicy)
{
	compare((Policy *)sfsPolicy);
}

void
RuleSetSearchPolicyVisitor::visitVarPolicy(VarPolicy*)
{
	/* var rules don't have an id -- do  nothing */
}
