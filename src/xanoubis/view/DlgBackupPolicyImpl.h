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

#ifndef _DLGBACKUPPOLICYIMPL_H_
#define _DLGBACKUPPOLICYIMPL_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/progdlg.h>
#include <wx/config.h>
#include "DlgBackupPolicyBase.h"
#include "PolicyRuleSet.h"

class DlgBackupPolicy : public DlgBackupPolicyBase
{
	public:
		/**
		 * Constructor of the Dialog.
		 * @param[in] 1st The icon to show in the Dialog.
		 * @param[in] 2nd The locked ruleset that should be saved.
		 */
		DlgBackupPolicy(wxIcon *, PolicyRuleSet *);

	protected:
		/**
		 * Handler for the Discard button.
		 */
		void onDiscardButton(wxCommandEvent &);

		/**
		 * Handler for the Save button.
		 */
		void onSaveButton(wxCommandEvent &);

	private:
		wxString	 homedir_;
		PolicyRuleSet	*ruleset_;
};

#endif	/* _DLGBACKUPPOLICYIMPL_H_ */
