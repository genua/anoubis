/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#ifndef _MODULE_H_
#define _MODULE_H_

#include <wx/icon.h>
#include <wx/panel.h>
#include <wx/string.h>

#define MODULE_ID_ENTRY(moduleName,entryName) \
	MOD##moduleName##_ID_##entryName = MOD##moduleName##_ID_BASE + \
	    MODULEID_OFFSET_##entryName

enum ModuleIdOffset {
	MODULEID_OFFSET_MAINPANEL = 1,
	MODULEID_OFFSET_OVERVIEWPANEL,
	MODULEID_OFFSET_TOOLBAR
};

class Module {
	protected:
		wxString	 name_;
		wxString	 nick_;
		wxString	 state_;
		wxPanel		*mainPanel_;
		wxPanel		*overviewPanel_;

	public:
		Module(void);
		virtual ~Module(void);

		virtual wxString	getName(void);
		virtual wxString	getNick(void);
		virtual wxString	getState(void);
		virtual void		setState(const wxString& state);
		virtual wxPanel		*getMainPanel(void);
		virtual wxPanel		*getOverviewPanel(void);
		virtual wxIcon		*getIcon(void) const = 0;
		virtual int		getBaseId(void) = 0;
		virtual int		getToolbarId(void) = 0;
		virtual void		update(void) = 0;
};

#endif	/* _MODULE_H_ */
