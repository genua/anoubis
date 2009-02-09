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

#ifndef _DLGRULEEDITORFILTERPAGE_H_
#define _DLGRULEEDITORFILTERPAGE_H_

#include "Observer.h"
#include "Subject.h"
#include "FilterPolicy.h"

/**
 * This is the base class to the pages of the filter notebook from
 * the RuleEditor.
 *
 * In general the filter pages hold the widgets to modify the values
 * of a policy. The concerning policy has to be selected in the list
 * of filter policies.\n
 * On filter policy select, the RuleEditor iterates through the list
 * of pages and calls select() on each page. On filter policy deselect,
 * the RuleEditor will call deselect() in each page.\n
 *
 * The inherited pages have to implement theire own select()/deselect()
 * and chek, if they're responible for this kind of policy. They also
 * have to call Show()/Hide() by themself, but may use the base
 * implementation of this base class for adding and removind the policy
 * to the list of observed subjects.
 */
class DlgRuleEditorPage : public Observer
{
	public:
		/**
		 * Constructor for DlgRuleEditorPage.
		 * @param None.
		 */
		DlgRuleEditorPage(void);

		/**
		 * Select this page.
		 * This will add the given policy to the list of observed
		 * subjects and run update(). Use it from derrived classes.
		 * This does not change the appearance/visability.
		 * @param[in] 1st The selected filter policy.
		 * @return Nothing.
		 */
		virtual void select(FilterPolicy *);

		/**
		 * Deselect this page.
		 * This will remove the stored policy and stops observing it.
		 * @param None.
		 * @return Noting.
		 */
		virtual void deselect(void);

	private:
		/**
		 * This holds the policy been edited by this page.
		 */
		FilterPolicy *filterPolicy_;
};

#endif	/* _DLGRULEEDITORFILTERPAGE_H_ */
