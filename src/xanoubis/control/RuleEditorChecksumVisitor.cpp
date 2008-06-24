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

#include "RuleEditorChecksumVisitor.h"

#include <wx/intl.h>
#include <apn.h>

RuleEditorChecksumVisitor::RuleEditorChecksumVisitor()
{
	mismatch_ = false;
	state_ = -1;
}

RuleEditorChecksumVisitor::RuleEditorChecksumVisitor(int state)
{
	mismatch_ = false;
	state_ = state;
}

RuleEditorChecksumVisitor::~RuleEditorChecksumVisitor()
{
	/* Destructor */
}

void
RuleEditorChecksumVisitor::compare(Policy *policy)
{
	SfsPolicy *sfsPolicy;
	AppPolicy *appPolicy;
	wxString   regHash;
	wxString   curHash;
	wxString   unknown;
	unsigned char csum[MAX_APN_HASH_LEN];

	unknown = _("unknown");

	/* first match strategie */
	if (!mismatch_ && policy) {
		if (policy->IsKindOf(CLASSINFO(SfsPolicy))) {
			sfsPolicy = (SfsPolicy *)policy;
			if (sfsPolicy->isModified()) {
				if (!sfsPolicy->getCurrentHash().Cmp(unknown)) {
					sfsPolicy->calcCurrentHash(csum);
				}
				curHash = sfsPolicy->getCurrentHash();
				regHash = sfsPolicy->getHashValue();
			}
		} else {
			appPolicy = (AppPolicy *)policy;

			if (appPolicy->isDefault())
				return;

			if (appPolicy->isModified()) {
				if (!appPolicy->getCurrentHash().Cmp(unknown)) {
					appPolicy->calcCurrentHash(csum);
				}
				curHash = appPolicy->getCurrentHash();
				regHash = appPolicy->getHashValue();
			}
		}

		if (regHash.Cmp(curHash))
			mismatch_ = true;
	}
}

void
RuleEditorChecksumVisitor::setModifiedTo(Policy *policy)
{
	SfsPolicy *sfsPolicy;
	AppPolicy *appPolicy;

	if (policy) {
		if (policy->IsKindOf(CLASSINFO(SfsPolicy))) {
			sfsPolicy = (SfsPolicy *)policy;
			sfsPolicy->setModified(state_);
		} else {
			appPolicy = (AppPolicy *)policy;
			appPolicy->setModified(state_);
		}
	}
}

bool
RuleEditorChecksumVisitor::hasMismatch(void)
{
	return (mismatch_);
}

void
RuleEditorChecksumVisitor::visitAppPolicy(AppPolicy *appPolicy)
{
	if (state_ == -1)
		compare((Policy *)appPolicy);
	else
		setModifiedTo((Policy *)appPolicy);
}

void
RuleEditorChecksumVisitor::visitAlfPolicy(AlfPolicy *alfPolicy)
{
	/* There are no Hash sums in AlfPolicy */
}

void
RuleEditorChecksumVisitor::visitSfsPolicy(SfsPolicy *sfsPolicy)
{
	if (state_ == -1)
		compare((Policy *)sfsPolicy);
	else
		setModifiedTo((Policy *)sfsPolicy);

}

void
RuleEditorChecksumVisitor::visitVarPolicy(VarPolicy *varPolicy)
{
	/* There are no Hash sums in VarPolicy */
}
