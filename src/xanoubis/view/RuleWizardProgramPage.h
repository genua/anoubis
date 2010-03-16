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

#ifndef _RULEWIZARDPROGRAMPAGE_H_
#define _RULEWIZARDPROGRAMPAGE_H_

#include <wx/wizard.h>

#include "CsumCalcTask.h"
#include "ComCsumAddTask.h"
#include "ComCsumGetTask.h"
#include "RuleWizardPanelsBase.h"
#include "RuleWizardHistory.h"

class TaskEvent;

/**
 * This is the first page of the wizard.
 * The user is asked for the name of the program this wizard should work for.
 * After the user made his choice, the checksum of the given program is
 * calculated.
 *
 * The next page is only reached if the program exists and calculating the
 * checksum was successfull.
 */
class RuleWizardProgramPage : public Observer, public RuleWizardProgramPageBase
{
	public:
		/**
		 * Constructor of this page.
		 */
		RuleWizardProgramPage(wxWindow *, RuleWizardHistory *);

		/**
		 * Destructor of this page.
		 */
		~RuleWizardProgramPage(void);

		/**
		 * This is called by the programPicker (a subject)
		 * to inform us about a picked file.
		 * @param[in] 1st The changed subject itself.
		 * @return Nothing.
		 */
		virtual void update(Subject *);

		/**
		 * This is called when the observed subject is destroyed.
		 * This should never happen, but we had to implement this.
		 * @param[in] 1st The subject be destroyed.
		 * @return Nothing.
		 */
		virtual void updateDelete(Subject *);

	private:
		/**
		 * Store the input here.
		 */
		RuleWizardHistory *history_;

		/**
		 * Tasks fetches a checksum from the daemon.
		 */
		ComCsumGetTask	*csumGetTask_;

		/**
		 * Task registers a checksum at the daemon.
		 */
		ComCsumAddTask	*csumAddTask_;

		/**
		 * Task calculates a checksum.
		 */
		CsumCalcTask	*csumCalcTask_;

		/**
		 * Handle events from wizard.
		 * The page change is in progress we block this
		 * if no program was set.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onPageChanging(wxWizardEvent &);

		/**
		 * Handle events from wizard.
		 * We became the current page. Update view.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onPageChanged(wxWizardEvent &);

		/**
		 * Handles events from daemon - a checksum was requested.
		 * A checksum needs to be registered, ff the requested file
		 * does not have a checksum. If the file has a checksum, you
		 * need to verify it.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onCsumGet(TaskEvent &);

		/**
		 * Handles events from daemon - a checksum appended to the
		 * shadowtree.
		 * If the operation was successful, you are are allowed to
		 * continue with the wizard.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onCsumAdd(TaskEvent &);

		/**
		 * Handles events from checksum-calculation.
		 * If the operation was successful, you are allowed to continue
		 * with the wizard.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onCsumCalc(TaskEvent &);

		/**
		 * Store the chosen program.
		 * This will store the given program to the history and
		 * start the calculation of the checksum.
		 * @param[in] 1st The program.
		 * @return Nothing.
		 */
		void setProgram(const wxString &);

		/**
		 * Update navigation.
		 * @param None.
		 * @return Nothing.
		 */
		void updateNavi(void);

		/**
		 * Returns the parent RuleWizardPage.
		 * @param None.
		 * @return Parent RuleWizardPage
		 */
		RuleWizardPage *getWizardPage(void) const;
};

#endif	/* _RULEWIZARDPROGRAMPAGE_H_ */
