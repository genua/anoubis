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

RuleSetSearchPolicyVisitor::RuleSetSearchPolicyVisitor(wxString hash)
{
	seekId_ = 0;
	seekHash_ = hash;
	matchingPolicy_ = NULL;
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
RuleSetSearchPolicyVisitor::visitAlfAppPolicy(AlfAppPolicy *policy)
{
	compare(policy);
	compareHash(policy);
}

void
RuleSetSearchPolicyVisitor::visitAlfCapabilityFilterPolicy(
    AlfCapabilityFilterPolicy *policy)
{
	compare(policy);
}

void
RuleSetSearchPolicyVisitor::visitAlfFilterPolicy(AlfFilterPolicy *policy)
{
	compare(policy);
}

void
RuleSetSearchPolicyVisitor::visitContextAppPolicy(ContextAppPolicy *policy)
{
	compare(policy);
	compareHash(policy);
}

void
RuleSetSearchPolicyVisitor::visitContextFilterPolicy(
    ContextFilterPolicy *policy)
{
	compare(policy);
	// XXX ch: do we have to compareHash((AppPolicy *)policy);
}

void
RuleSetSearchPolicyVisitor::visitDefaultFilterPolicy(
    DefaultFilterPolicy *policy)
{
	compare(policy);
}

void
RuleSetSearchPolicyVisitor::visitSbAccessFilterPolicy(
    SbAccessFilterPolicy *policy)
{
	compare(policy);
}

void
RuleSetSearchPolicyVisitor::visitSbAppPolicy(SbAppPolicy *policy)
{
	compare(policy);
	compareHash(policy);
}

void
RuleSetSearchPolicyVisitor::visitSfsAppPolicy(SfsAppPolicy *policy)
{
	compare(policy);
	compareHash(policy);
}

void
RuleSetSearchPolicyVisitor::visitSfsFilterPolicy(SfsFilterPolicy *policy)
{
	compare(policy);
}

void
RuleSetSearchPolicyVisitor::visitSfsDefaultFilterPolicy(
    SfsDefaultFilterPolicy *policy)
{
	compare(policy);
}

void
RuleSetSearchPolicyVisitor::compare(Policy *policy)
{
	/* first match strategy */
	if (matchingPolicy_ == NULL) {
		if (policy->getApnRuleId() == seekId_) {
			matchingPolicy_ = policy;
		}
	}
}

void
RuleSetSearchPolicyVisitor::compareHash(AppPolicy *appPolicy)
{
	/* first match strategy */
	/* XXX ch: does not work for lists */
	if (matchingPolicy_ == NULL) {
		if (appPolicy->getHashValueName(0).Cmp(seekHash_) == 0) {
			matchingPolicy_ = appPolicy;
		}
	}
}
