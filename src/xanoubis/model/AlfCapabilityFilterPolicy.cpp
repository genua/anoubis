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

#include "AlfCapabilityFilterPolicy.h"
#include "PolicyVisitor.h"

IMPLEMENT_CLASS(AlfCapabilityFilterPolicy, FilterPolicy);

AlfCapabilityFilterPolicy::AlfCapabilityFilterPolicy(AppPolicy *parent,
    struct apn_rule *rule) : FilterPolicy(parent, rule)
{
	/* Nothing special to do here. */
}

wxString
AlfCapabilityFilterPolicy::getTypeIdentifier(void) const
{
	return (wxT("Capability"));
}

struct apn_rule *
AlfCapabilityFilterPolicy::createApnRule(void)
{
	struct apn_rule *rule;

	rule = FilterPolicy::createApnRule();
	if (rule != NULL) {
		rule->apn_type = APN_ALF_CAPABILITY;
	}

	return (rule);
}

void
AlfCapabilityFilterPolicy::accept(PolicyVisitor &visitor)
{
	visitor.visitAlfCapabilityFilterPolicy(this);
}

bool
AlfCapabilityFilterPolicy::setLogNo(int log)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.acap.log = log;
	setModified();
	finishChange();

	return (true);
}

int
AlfCapabilityFilterPolicy::getLogNo(void) const
{
	int		 log;
	struct apn_rule	*rule;

	log  = -1;
	rule = getApnRule();

	if (rule != NULL) {
		log = rule->rule.acap.log;
	}

	return (log);
}

bool
AlfCapabilityFilterPolicy::setActionNo(int action)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.acap.action = action;
	setModified();
	finishChange();

	return (true);
}

int
AlfCapabilityFilterPolicy::getActionNo(void) const
{
	int		 action;
	struct apn_rule	*rule;

	action = -1;
	rule   = getApnRule();

	if (rule != NULL) {
		action = rule->rule.acap.action;
	}

	return (action);
}

bool
AlfCapabilityFilterPolicy::setCapabilityTypeNo(int capability)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.acap.capability = capability;
	setModified();
	finishChange();

	return (true);
}

int
AlfCapabilityFilterPolicy::getCapabilityTypeNo(void) const
{
	int		 capability;
	struct apn_rule	*rule;

	capability = -1;
	rule	   = getApnRule();

	if (rule != NULL) {
		capability = rule->rule.acap.capability;
	}

	return (capability);
}

wxString
AlfCapabilityFilterPolicy::getCapabilityTypeName(void) const
{
	wxString capability;

	switch (getCapabilityTypeNo()) {
	case APN_ALF_CAPRAW:
		capability = wxT("raw");
		break;
	case APN_ALF_CAPOTHER:
		capability = wxT("other");
		break;
	case APN_ALF_CAPALL:
		capability = wxT("all");
		break;
	default:
		capability = _("(unknown)");
		break;
	}

	return (capability);
}
