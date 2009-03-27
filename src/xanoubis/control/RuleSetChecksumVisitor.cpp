/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#include "RuleSetChecksumVisitor.h"

RuleSetChecksumVisitor::RuleSetChecksumVisitor(void)
{
	mismatchPolicy_.Clear();
	setPropagation(true);
}

RuleSetChecksumVisitor::~RuleSetChecksumVisitor(void)
{
	mismatchPolicy_.DeleteContents(false);
	mismatchPolicy_.Clear();
}

const PolicyList
RuleSetChecksumVisitor::getPolicyMismatchList(void) const
{
	return (mismatchPolicy_);
}

void
RuleSetChecksumVisitor::compare(AppPolicy *policy)
{
	wxArrayString	_registeredList;
	wxArrayString	_currentList;

	_registeredList = policy->getHashValueList();
	_currentList = policy->calculateCurrentChecksums();

	if (_registeredList != _currentList) {
		mismatchPolicy_.Append(policy);
	}
}

void
RuleSetChecksumVisitor::compare(ContextFilterPolicy *policy)
{
	wxArrayString	_registeredList;
	wxArrayString	_currentList;

	_registeredList = policy->getHashValueList();
	_currentList = policy->calculateCurrentChecksums();

	if (_registeredList != _currentList) {
		mismatchPolicy_.Append(policy);
	}
}

bool
RuleSetChecksumVisitor::havePolicyMismatch(void) const
{
	return (!mismatchPolicy_.IsEmpty());
}

void
RuleSetChecksumVisitor::visitAlfAppPolicy(AlfAppPolicy *policy)
{
	this->compare(policy);
}

void
RuleSetChecksumVisitor::visitAlfCapabilityFilterPolicy(
    AlfCapabilityFilterPolicy *)
{
	/* Nothing to do for filter policies. */
}

void
RuleSetChecksumVisitor::visitAlfFilterPolicy(AlfFilterPolicy *)
{
	/* Nothing to do for filter policies. */
}

void
RuleSetChecksumVisitor::visitContextAppPolicy(ContextAppPolicy *policy)
{
	this->compare(policy);
}

void
RuleSetChecksumVisitor::visitContextFilterPolicy(
    ContextFilterPolicy *policy)
{
	this->compare(policy);
}

void
RuleSetChecksumVisitor::visitDefaultFilterPolicy(
    DefaultFilterPolicy *)
{
	/* Nothing to do for filter policies. */
}

void
RuleSetChecksumVisitor::visitSbAccessFilterPolicy(
    SbAccessFilterPolicy *)
{
	/* Nothing to do for filter policies. */
}

void
RuleSetChecksumVisitor::visitSbAppPolicy(SbAppPolicy *policy)
{
	this->compare(policy);
}

void
RuleSetChecksumVisitor::visitSfsAppPolicy(SfsAppPolicy *policy)
{
	this->compare(policy);
}

void
RuleSetChecksumVisitor::visitSfsFilterPolicy(SfsFilterPolicy *)
{
	/* Nothing to do for filter policies. */
}

void
RuleSetChecksumVisitor::visitSfsDefaultFilterPolicy(
    SfsDefaultFilterPolicy *)
{
	/* Nothing to do for filter policies. */
}
