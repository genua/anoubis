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

#include "AlfAppPolicy.h"
#include "PolicyVisitor.h"

IMPLEMENT_CLASS(AlfAppPolicy, AppPolicy);

AlfAppPolicy::AlfAppPolicy(PolicyRuleSet *ruleSet, struct apn_rule *rule)
    : AppPolicy(ruleSet, rule)
{
	struct apn_rule *filter;

	if (rule == NULL) {
		return;
	}

	TAILQ_FOREACH(filter, &rule->rule.chain, entry) {
		switch (filter->apn_type) {
		case APN_ALF_FILTER:
			filterList_.Append(new AlfFilterPolicy(this, filter));
			break;
		case APN_ALF_CAPABILITY:
			filterList_.Append(
			    new AlfCapabilityFilterPolicy(this, filter));
			break;
		case APN_DEFAULT:
			filterList_.Append(
			    new DefaultFilterPolicy(this, filter));
			break;
		default:
			/* Unknown filter type of AlfApp, do nothing. */
			break;
		}
	}
}

AlfAppPolicy::AlfAppPolicy(PolicyRuleSet *ruleSet)
    : AppPolicy(ruleSet, AlfAppPolicy::createApnRule())
{
}

wxString
AlfAppPolicy::getTypeIdentifier(void) const
{
	return (wxT("ALF"));
}

struct apn_rule *
AlfAppPolicy::createApnRule(void)
{
	struct apn_rule *rule;

	rule = AppPolicy::createApnRule();
	if (rule != NULL) {
		rule->apn_type = APN_ALF;
	}

	return (rule);
}

void
AlfAppPolicy::accept(PolicyVisitor &visitor)
{
	visitor.visitAlfAppPolicy(this);
	acceptOnFilter(visitor);
}
