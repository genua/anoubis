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

#include "AnListClass.h"
#include "AppPolicy.h"
#include "FilterPolicy.h"
#include "PolicyRuleSet.h"
#include "SimpleOverviewRow.h"

SimpleOverviewRow::SimpleOverviewRow(PolicyRuleSet *ruleSet,
    unsigned int appIdx, AppPolicy *appPol,
    unsigned int filterIdx, FilterPolicy *filterPol)
    : Observer(0)
{
	ruleSet_ = ruleSet;
	applicationIndex_ = appIdx;
	applicationPolicy_ = appPol;
	filterIndex_ = filterIdx;
	filterPolicy_ = filterPol;

	if (ruleSet != 0)
		addSubject(ruleSet);
	if (appPol != 0)
		addSubject(appPol);
	if (filterPol != 0)
		addSubject(filterPol);
}

SimpleOverviewRow::~SimpleOverviewRow(void)
{
	if (ruleSet_ != 0)
		removeSubject(ruleSet_);
	if (applicationPolicy_ != 0)
		removeSubject(applicationPolicy_);
	if (filterPolicy_ != 0)
		removeSubject(filterPolicy_);
}

PolicyRuleSet *
SimpleOverviewRow::getRuleSet(void) const
{
	return ruleSet_;
}

FilterPolicy *
SimpleOverviewRow::getFilterPolicy(void) const
{
	return filterPolicy_;
}

unsigned int
SimpleOverviewRow::getFilterPolicyIndex(void) const
{
	return filterIndex_;
}

AppPolicy *
SimpleOverviewRow::getApplicationPolicy(void) const
{
	return applicationPolicy_;
}

unsigned int
SimpleOverviewRow::getApplicationPolicyIndex(void) const
{
	return applicationIndex_;
}

bool
SimpleOverviewRow::haveBinary(void) const
{
	if (applicationPolicy_ != 0) {
		if (applicationPolicy_->isAnyBlock())
			return (filterIndex_ == 0);
		else
			return (filterIndex_ <
			    applicationPolicy_->getBinaryCount());
	} else
		return false;
}

void
SimpleOverviewRow::update(Subject *)
{
}

void
SimpleOverviewRow::updateDelete(Subject *subject)
{
	if (subject == ruleSet_)
		ruleSet_ = 0;
	else if (subject == applicationPolicy_)
		applicationPolicy_ = 0;
	else if (subject == filterPolicy_)
		filterPolicy_ = 0;
}
