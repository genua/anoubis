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

#ifndef _RULEWIZARDSANDBOXREADFILESPAGE_H_
#define _RULEWIZARDSANDBOXREADFILESPAGE_H_

#include <wx/wizard.h>

#include "RuleWizardPanelsBase.h"
#include "RuleWizardHistory.h"

/**
 *
 */
class RuleWizardSandboxReadFilesPage : public RuleWizardSandboxFilesPageBase
{
	public:
		/**
		 * Constructor of this page.
		 */
		RuleWizardSandboxReadFilesPage(wxWindow *,
		    RuleWizardHistory *);

	private:
		/**
		 * Use these indices for the columns of the file list.
		 */
		enum fileColumns {
			COLUMN_PATH = 0,	/**< Column for path */
			COLUMN_FILE,		/**< Column for filename */
			COLUMN_STD,		/**< Column for std-flag */
		};

		/**
		 * Store the input here.
		 */
		RuleWizardHistory *history_;

		/**
		 * Handle events from wizard.
		 * We became the current page. Update view.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onPageChanged(wxWizardEvent &);

		/**
		 * Handle events from addFileButton.
		 * This will open a file chooser and on 'ok' the choosen
		 * file is added to the list.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAddFileButton(wxCommandEvent &);

		/**
		 * Handle events from addDirectoryButton.
		 * This will open a directory chooser and on 'ok' the choosen
		 * directory is added to the list.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAddDirectoryButton(wxCommandEvent &);

		/**
		 * Handle events form defaultsButton.
		 * This will load the defaults and add them to the list.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onDefaultsButton(wxCommandEvent &);

		/**
		 * Handle events from deleteButton.
		 * This will delete the selected entries from the list.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onDeleteButton(wxCommandEvent &);

		/**
		 * Handle select events from fileListCtrl.
		 * This will enable the deleteButton if something is selected.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onFileListSelect(wxListEvent &);

		/**
		 * Handle deselect events from fileListCtrl.
		 * This will disable the deleteButton if the last seleciton
		 * is removed.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onFileListDeselect(wxListEvent &);

		/**
		 * Handle events from askCheckBox.
		 * This will store the value of the checkbox to the history.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAskCheckBox(wxCommandEvent &);

		/**
		 * Handle events from validCheckBox.
		 * This will store the value of the checkbox to the history.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onValidCheckBox(wxCommandEvent &);

		/**
		 * Store file list.
		 * This will take the input of the file list and stores
		 * it to the history.
		 * @param None.
		 * @return Nothing.
		 */
		void storeFileList(void) const;

		/**
		 * Update navigation.
		 * @param None.
		 * @return Nothing.
		 */
		void updateNavi(void);
};

#endif	/* _RULEWIZARDSANDBOXREADFILESPAGE_H_ */
