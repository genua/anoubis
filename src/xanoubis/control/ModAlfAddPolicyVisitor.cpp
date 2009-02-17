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

#include "ModAlfAddPolicyVisitor.h"

#include "main.h"
#include "PolicyRuleSet.h"

ModAlfAddPolicyVisitor::ModAlfAddPolicyVisitor(ModAlfMainPanelImpl *alfPanel)
{
	alfPanel_ = alfPanel;
}

void
ModAlfAddPolicyVisitor::visitAlfAppPolicy(AlfAppPolicy *policy)
{
	alfPanel_->addAlfAppPolicy(policy);
}

void
ModAlfAddPolicyVisitor::visitAlfCapabilityFilterPolicy(
    AlfCapabilityFilterPolicy *policy)
{
	alfPanel_->addAlfCapabilityFilterPolicy(policy);
}

void
ModAlfAddPolicyVisitor::visitAlfFilterPolicy(AlfFilterPolicy *policy)
{
	alfPanel_->addAlfFilterPolicy(policy);
}

void
ModAlfAddPolicyVisitor::visitContextAppPolicy(ContextAppPolicy *)
{
	/* ModAlf does not deal with ContextAppPolicies. */
}

void
ModAlfAddPolicyVisitor::visitContextFilterPolicy(
    ContextFilterPolicy *)
{
	/* ModAlf does not deal with ContextFilterPolicies. */
}


void
ModAlfAddPolicyVisitor::visitDefaultFilterPolicy(DefaultFilterPolicy *policy)
{
	alfPanel_->addDefaultFilterPolicy(policy);
}

void
ModAlfAddPolicyVisitor::visitSbAccessFilterPolicy(SbAccessFilterPolicy *)
{
	/* ModAlf does not deal with SbAccessFilterPolicies. */
}

void
ModAlfAddPolicyVisitor::visitSbAppPolicy(SbAppPolicy *)
{
	/* ModAlf does not deal with SbAppPolicies. */
}

void
ModAlfAddPolicyVisitor::visitSfsAppPolicy(SfsAppPolicy *)
{
	/* ModAlf does not deal with SfsAppPolicies. */
}

void
ModAlfAddPolicyVisitor::visitSfsFilterPolicy(SfsFilterPolicy *)
{
	/* ModAlf does not deal with SfsFilterPolicies. */
}

void
ModAlfAddPolicyVisitor::visitSfsDefaultFilterPolicy(SfsDefaultFilterPolicy *)
{
	/* ModAlf does not deal with SfsFilterPolicies. */
}
