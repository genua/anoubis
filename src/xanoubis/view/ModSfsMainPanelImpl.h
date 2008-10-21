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

class SfsCtrl;

class ModSfsMainPanelImpl : public ModSfsMainPanelBase
{
	private:
		wxString	 columnNames_[MODSFS_LIST_COLUMN_EOL];
		long		 userRuleSetId_;
		long		 adminRuleSetId_;
		SfsCtrl		*sfsCtrl_;

		void OnLoadRuleSet(wxCommandEvent&);

		wxIcon		*sfsListOkIcon_;
		wxIcon		*sfsListWarnIcon_;
		wxIcon		*sfsListErrorIcon_;
		wxImageList	sfsListImageList_;
		void initSfsMain();
		void destroySfsMain();
		void updateSfsList();
		void OnSfsMainDirCtrlSelChanged(wxTreeEvent&);
		void OnSfsDirChanged(wxCommandEvent&);
		void OnSfsMainFilterButtonClicked(wxCommandEvent&);
		void OnSfsMainInverseCheckboxClicked(wxCommandEvent&);

	public:
		ModSfsMainPanelImpl(wxWindow*, wxWindowID);
		~ModSfsMainPanelImpl();

		friend class ModSfsAddPolicyVisitor;
};

#endif /* __ModSfsMainPanelImpl__ */
