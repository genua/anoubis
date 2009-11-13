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

#include "Debug.h"
#include "SimpleOverviewRow.h"
#include "SimpleOverviewVisitor.h"

SimpleOverviewVisitor::SimpleOverviewVisitor(wxClassInfo *classInfo)
{
	classInfo_ = classInfo;
	currentRuleSet_ = 0;
	currentApp_ = 0;
	appIndex_ = 0;
	filterIndex_ = 0;
}

SimpleOverviewVisitor::~SimpleOverviewVisitor(void)
{
	clear();
}

const std::vector<SimpleOverviewRow *> &
SimpleOverviewVisitor::getResult(void) const
{
	return (filterList_);
}

void
SimpleOverviewVisitor::reset(PolicyRuleSet *ruleSet)
{
	currentRuleSet_ = ruleSet;
	setPropagation(true);
}

void
SimpleOverviewVisitor::clear(void)
{
	while (!filterList_.empty()) {
		SimpleOverviewRow *row = filterList_.back();

		filterList_.pop_back();
		delete row;
	}

	currentApp_ = 0;
	appIndex_ = 0;
	filterIndex_ = 0;
}

void
SimpleOverviewVisitor::visitAlfAppPolicy(AlfAppPolicy *policy)
{
	visitAppPolicy(policy);
}

void
SimpleOverviewVisitor::visitContextAppPolicy(ContextAppPolicy *policy)
{
	visitAppPolicy(policy);
}

void
SimpleOverviewVisitor::visitSbAppPolicy(SbAppPolicy *policy)
{
	visitAppPolicy(policy);
}

void
SimpleOverviewVisitor::visitSfsAppPolicy(SfsAppPolicy *policy)
{
	visitAppPolicy(policy);
}

void
SimpleOverviewVisitor::visitAlfCapabilityFilterPolicy(
    AlfCapabilityFilterPolicy *policy)
{
	visitFilterPolicy(policy);
}

void
SimpleOverviewVisitor::visitAlfFilterPolicy(AlfFilterPolicy *policy)
{
	visitFilterPolicy(policy);
}

void
SimpleOverviewVisitor::visitContextFilterPolicy(ContextFilterPolicy *policy)
{
	visitFilterPolicy(policy);
}

void
SimpleOverviewVisitor::visitDefaultFilterPolicy(DefaultFilterPolicy *policy)
{
	visitFilterPolicy(policy);
}

void
SimpleOverviewVisitor::visitSbAccessFilterPolicy(SbAccessFilterPolicy *policy)
{
	visitFilterPolicy(policy);
}

void
SimpleOverviewVisitor::visitSfsFilterPolicy(SfsFilterPolicy *policy)
{
	visitFilterPolicy(policy);
}

void
SimpleOverviewVisitor::visitSfsDefaultFilterPolicy(
    SfsDefaultFilterPolicy *policy)
{
	visitFilterPolicy(policy);
}

void
SimpleOverviewVisitor::visitAppPolicy(AppPolicy *policy)
{
	if (policy->IsKindOf(classInfo_)) {
		Debug::trace(wxT("(%ls) Visiting application %ls"),
		    classInfo_->GetClassName(),
		    policy->GetClassInfo()->GetClassName());

		/* This is an application-policies you are looking for */
		currentApp_ = policy;
		appIndex_++;
		filterIndex_ = 0;
		setPropagation(true);

		if (policy->getFilterPolicyCount() == 0) {
			/*
			 * This application-policy has no assigned filter.
			 * Nevertheless append a row (with not assigned filter
			 */
			unsigned int count = currentApp_->getBinaryCount();

			if (count == 0) {
				/* No binary assigned. Anyway, insert a row */
				count = 1;
			}

			for (unsigned int i = 0; i < count; i++) {
				Debug::trace(
				    wxT("(%ls) Appending application"),
				    classInfo_->GetClassName());

				SimpleOverviewRow *row = new SimpleOverviewRow(
				    currentRuleSet_, appIndex_ - 1,
				    currentApp_, filterIndex_ + i, 0);
				filterList_.push_back(row);
			}
		}
	} else {
		/*
		 * You are not interested in such an app-policies. Ignore the
		 * corresponding filter.
		 */
		Debug::trace(wxT("(%ls) Skipping application %ls"),
		    classInfo_->GetClassName(),
		    policy->GetClassInfo()->GetClassName());

		setPropagation(false);
	}
}

void
SimpleOverviewVisitor::visitFilterPolicy(FilterPolicy *policy)
{
	Debug::trace(wxT("(%ls) Visiting filter %ls"),
	    classInfo_->GetClassName(),
	    policy->GetClassInfo()->GetClassName());

	SimpleOverviewRow *row = new SimpleOverviewRow(currentRuleSet_,
	    appIndex_ - 1, currentApp_, filterIndex_, policy);
	filterList_.push_back(row);
	Debug::trace(wxT("(%ls) Appending filter"),
	    classInfo_->GetClassName());

	filterIndex_++;

	if (currentApp_->getFilterPolicyCount() == filterIndex_) {
		/*
		 * This is the last filter-policy of the current application.
		 * But maybe the application-policy has more binaries than
		 * filter. In this case we have to add some more rows.
		 */
		while (filterIndex_ < currentApp_->getBinaryCount()) {
			SimpleOverviewRow *row = new SimpleOverviewRow(
			    currentRuleSet_, appIndex_ - 1, currentApp_,
			    filterIndex_, 0);
			filterList_.push_back(row);

			Debug::trace(wxT("(%ls) Appending extra filter"),
			    classInfo_->GetClassName());

			filterIndex_++;
		}
	}
}
