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

#include "SbAccessFilterPolicy.h"
#include "PolicyVisitor.h"

IMPLEMENT_CLASS(SbAccessFilterPolicy, FilterPolicy);

SbAccessFilterPolicy::SbAccessFilterPolicy(AppPolicy *parentPolicy,
    struct apn_rule *rule) : FilterPolicy(parentPolicy, rule)
{
}

wxString
SbAccessFilterPolicy::getTypeIdentifier(void) const
{
	return (wxT("SB"));
}

void
SbAccessFilterPolicy::accept(PolicyVisitor &visitor)
{
	visitor.visitSbAccessFilterPolicy(this);
}

struct apn_rule *
SbAccessFilterPolicy::createApnRule(void)
{
	struct apn_rule *rule;

	rule = FilterPolicy::createApnRule();
	if (rule != NULL) {
		rule->apn_type = APN_SB_ACCESS;
	}

	return (rule);
}

bool
SbAccessFilterPolicy::setActionNo(int)
{
	/* SB filter policies has no action. */
	return (false);
}

int
SbAccessFilterPolicy::getActionNo(void) const
{
	/* SB filter policies has no action. */
	return (-1);
}

bool
SbAccessFilterPolicy::setLogNo(int)
{
	/* SB filter policies has no log. */
	return (false);
}

int
SbAccessFilterPolicy::getLogNo(void) const
{
	/* SB filter policies has no log. */
	return (-1);
}
