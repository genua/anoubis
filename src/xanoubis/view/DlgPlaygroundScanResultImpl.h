/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#ifndef _DLGPLAYGROUNDSCANRESULTIMPL_H_
#define _DLGPLAYGROUNDSCANRESULTIMPL_H_

#include "ModPlaygroundPanelsBase.h"

/**
 * Implementation of dialog to show content scanner results.
 */
class DlgPlaygroundScanResultImpl : public DlgPlaygroundScanResultBase
{
	public:
		/**
		 * Constructor.
		 */
		DlgPlaygroundScanResultImpl(void);

		/**
		 * Add scanner result.
		 * This method adds the given scanner result with the given
		 * title as details to the dialog.
		 * @param[in] 1st The title (aka name of content scanner).
		 * @param[in] 2nd The result description.
		 * @return Nothing.
		 */
		void addResult(const wxString &, const wxString &);

		/**
		 * Set required content scanners.
		 * Use this method to tell the dialog to switch state
		 * of buttons and show concerning question text.
		 * @param[in] 1st True for a required scanner.
		 * @return Nothing.
		 */
		void setRequired(bool);

		/**
		 * Set file name.
		 * Use this method do set the name of the file in question.
		 * @param[in] 1st The name of the file.
		 * @return Nothing.
		 */
		void setFileName(const wxString &);

	protected:
		/**
		 * Event handler for size change events of details.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onSize(wxSizeEvent &);

		/**
		 * Handle Ok-Button click.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onOkButtonClick(wxCommandEvent &);

		/**
		 * Handle Skip-Button click.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onSkipButtonClick(wxCommandEvent &);

		/**
		 * Handle Commit-Button click.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onCommitButtonClick(wxCommandEvent &);
};

#endif	/* _DLGPLAYGROUNDSCANRESULTIMPL_H_ */
