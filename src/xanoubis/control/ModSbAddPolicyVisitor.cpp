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

#include "ModSbAddPolicyVisitor.h"

#include "main.h"
#include "PolicyRuleSet.h"

ModSbAddPolicyVisitor::ModSbAddPolicyVisitor(ModSbMainPanelImpl *sbPanel)
{
	sbPanel_ = sbPanel;
}

void
ModSbAddPolicyVisitor::visitAlfAppPolicy(AlfAppPolicy *)
{
	/* ModSb does not deal with AlfAppPolicies. */
	setPropagation(false);
}

void
ModSbAddPolicyVisitor::visitAlfCapabilityFilterPolicy(
    AlfCapabilityFilterPolicy *)
{
	/* ModSb does not deal with AlfCapabilityFilterPolicies. */
	setPropagation(false);
}

void
ModSbAddPolicyVisitor::visitAlfFilterPolicy(AlfFilterPolicy *)
{
	/* ModSb does not deal with AlfFilterPolicies. */
	setPropagation(false);
}

void
ModSbAddPolicyVisitor::visitContextAppPolicy(ContextAppPolicy *)
{
	/* ModSb does not deal with ContextAppPolicies. */
	setPropagation(false);
}

void
ModSbAddPolicyVisitor::visitContextFilterPolicy(ContextFilterPolicy *)
{
	/* ModSb does not deal with ContextFilterPolicies. */
	setPropagation(false);
}

void
ModSbAddPolicyVisitor::visitDefaultFilterPolicy(DefaultFilterPolicy* policy)
{
	setPropagation(true);
	sbPanel_->addDefaultFilterPolicy(policy);
}

void
ModSbAddPolicyVisitor::visitSbAccessFilterPolicy(SbAccessFilterPolicy *policy)
{
	setPropagation(true);
	sbPanel_->addSbAccessFilterPolicy(policy);
}

void
ModSbAddPolicyVisitor::visitSbAppPolicy(SbAppPolicy *policy)
{
	setPropagation(true);
	sbPanel_->addSbAppPolicy(policy);
}

void
ModSbAddPolicyVisitor::visitSfsAppPolicy(SfsAppPolicy *)
{
	/* ModSb does not deal with SfsAppPolicy. */
	setPropagation(false);
}

void
ModSbAddPolicyVisitor::visitSfsFilterPolicy(SfsFilterPolicy *)
{
	/* ModSb does not deal with SfsFilterPolicy. */
	setPropagation(false);
}

void
ModSbAddPolicyVisitor::visitSfsDefaultFilterPolicy(SfsDefaultFilterPolicy *)
{
	/* ModSb does not deal with SfsDefaultFilterPolicy. */
	setPropagation(false);
}
