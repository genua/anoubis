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

#ifndef __ModSfsMainPanelImpl__
#define __ModSfsMainPanelImpl__

#include "AnEvents.h"
#include "ModSfsPanelsBase.h"

enum modSfsListColumns {
	MODSFS_LIST_COLUMN_PRIO = 0,
	MODSFS_LIST_COLUMN_PROG,
	MODSFS_LIST_COLUMN_HASHT,
	MODSFS_LIST_COLUMN_HASH,
	MODSFS_LIST_COLUMN_EOL
};

enum modSfsMainFileListColumns {
	MODSFSMAIN_FILELIST_COLUMN_FILE = 0,
	MODSFSMAIN_FILELIST_COLUMN_CHECKSUM,
	MODSFSMAIN_FILELIST_COLUMN_SIGNATURE,
	MODSFSMAIN_FILELIST_COLUMN_EOL
};

class IndexArray;
class SfsCtrl;
class SfsEntry;

class ModSfsMainPanelImpl : public ModSfsMainPanelBase
{
	private:
		/**
		 * An Sfs-operation.
		 *
		 * Post-processing on the view might differ from operation to
		 * operation. That's why you have to keep in mind the
		 * operation, which was initiated.
		 */
		enum SfSOperation {
			OP_NOP,	/*!< No operation. The view is in initial
				     state. */
			OP_SHOW_ALL,	/*!< A new directory, filter ect. was
					     selected, which forces an update
					     of the complete entry-list. */
			OP_SHOW_CHANGED,	/*!< Show changed checksums. */
			OP_SHOW_CHECKSUMS,	/*!< Show checksums only */
			OP_SHOW_ORPHANED,	/*!< Show orphaned files
						     only */
			OP_APPLY	/*!< Apply was clicked to perform an
					     operation on the current
					     selection */
		};

		wxString	 columnNames_[MODSFS_LIST_COLUMN_EOL];
		long		 userRuleSetId_;
		long		 adminRuleSetId_;
		SfsCtrl		*sfsCtrl_;

		void OnLoadRuleSet(wxCommandEvent&);

		SfSOperation	currentOperation_;
		wxIcon		*sfsListOkIcon_;
		wxIcon		*sfsListWarnIcon_;
		wxIcon		*sfsListErrorIcon_;
		wxImageList	sfsListImageList_;

		void initSfsMain();
		void destroySfsMain();
		void updateSfsList();
		void updateSfsEntry(int);
		void applySfsAction(const IndexArray &);
		void applySfsValidateAll(bool);
		bool canDisplay(const SfsEntry &) const;

		void initSfsOptions(void);
		void certificateParamsUpdate(void);

		void OnSfsMainDirCtrlSelChanged(wxTreeEvent&);
		void OnSfsOperationFinished(wxCommandEvent&);
		void OnSfsDirChanged(wxCommandEvent&);
		void OnSfsMainListItemActivated(wxListEvent&);
		void OnSfsMainDirTraversalChecked(wxCommandEvent&);
		void OnSfsEntryChanged(wxCommandEvent&);
		void OnSfsError(wxCommandEvent&);
		void OnSfsMainFilterButtonClicked(wxCommandEvent&);
		void OnSfsMainInverseCheckboxClicked(wxCommandEvent&);
		void OnSfsMainValidateButtonClicked(wxCommandEvent&);
		void OnSfsMainApplyButtonClicked(wxCommandEvent&);
		void OnSfsMainSigEnabledClicked(wxCommandEvent&);
		void OnSfsMainSearchOrphanedClicked(wxCommandEvent&);
		void OnSfsMainShowAllChecksumsClicked(wxCommandEvent&);
		void OnSfsMainShowChangedClicked(wxCommandEvent&);
		void OnSfsMainKeyLoaded(wxCommandEvent&);

		void OnPrivKeyValidityChanged(wxCommandEvent&);
		void OnPrivKeyChooseClicked(wxCommandEvent&);
		void OnPrivKeyValidityPeriodChanged(wxSpinEvent&);
		void OnCertChooseClicked(wxCommandEvent&);

	public:
		ModSfsMainPanelImpl(wxWindow*, wxWindowID);
		~ModSfsMainPanelImpl();

		friend class ModSfsAddPolicyVisitor;
};

#endif /* __ModSfsMainPanelImpl__ */
