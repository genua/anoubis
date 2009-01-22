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

#include "SfsDefaultFilterPolicy.h"
#include "PolicyVisitor.h"

IMPLEMENT_CLASS(SfsDefaultFilterPolicy, FilterPolicy);

SfsDefaultFilterPolicy::SfsDefaultFilterPolicy(AppPolicy *parentPolicy,
    struct apn_rule *rule) : FilterPolicy(parentPolicy, rule)
{
}

wxString
SfsDefaultFilterPolicy::getTypeIdentifier(void) const
{
	return (wxT("Default"));
}

struct apn_rule *
SfsDefaultFilterPolicy::createApnRule(void)
{
	struct apn_rule *rule;

	rule = FilterPolicy::createApnRule();
	if (rule != NULL) {
		rule->apn_type = APN_SFS_DEFAULT;
	}

	return (rule);
}

void
SfsDefaultFilterPolicy::accept(PolicyVisitor &visitor)
{
	visitor.visitSfsDefaultFilterPolicy(this);
}

bool
SfsDefaultFilterPolicy::setLogNo(int log)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.sfsdefault.log = log;
	setModified();
	finishChange();

	return (true);
}

int
SfsDefaultFilterPolicy::getLogNo(void) const
{
	int		 log;
	struct apn_rule	*rule;

	log  = -1;
	rule = getApnRule();

	if (rule != NULL) {
		log = rule->rule.sfsdefault.log;
	}

	return (log);
}

bool
SfsDefaultFilterPolicy::setActionNo(int action)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.sfsdefault.action = action;
	setModified();
	finishChange();

	return (true);
}

int
SfsDefaultFilterPolicy::getActionNo(void) const
{
	int		 action;
	struct apn_rule	*rule;

	action = -1;
	rule   = getApnRule();

	if (rule != NULL) {
		action = rule->rule.sfsdefault.action;
	}

	return (action);
}

bool
SfsDefaultFilterPolicy::setPath(wxString path)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if ((rule == NULL) || path.IsEmpty()){
		return (false);
	}

	startChange();
	if (rule->rule.sfsdefault.path != NULL) {
		free(rule->rule.sfsdefault.path);
		rule->rule.sfsdefault.path = NULL;
		setModified();
	}

	if (path.Cmp(wxT("any")) != 0) {
		rule->rule.sfsdefault.path = strdup(path.fn_str());
		setModified();
	}

	finishChange();
	return (true);
}

wxString
SfsDefaultFilterPolicy::getPath(void) const
{
	wxString	 path;
	struct apn_rule *rule;

	path = wxEmptyString;
	rule = getApnRule();
	if (rule != NULL) {
		if (rule->rule.sfsdefault.path == NULL) {
			path = wxT("any");
		} else {
			path = wxString::From8BitData(
			    rule->rule.sfsdefault.path);
		}
	}

	return (path);
}
