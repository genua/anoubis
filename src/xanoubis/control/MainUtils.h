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

#ifndef _MAINUTILS_H_
#define _MAINUTILS_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/param.h>

#include <wx/icon.h>
#include <wx/stdpaths.h>
#include <wx/string.h>

#include "ctassert.h"
#include "Singleton.h"
#include "MainFrame.h"
#include "Module.h"
#include "ModOverview.h"
#include "ModAlf.h"
#include "ModSfs.h"
#include "ModSb.h"
#include "ModPlayground.h"
#include "ModAnoubis.h"

enum moduleIdx {
	OVERVIEW = 0,
	ALF,
	SFS,
	SB,
	PG,
	ANOUBIS,
	LAST_MODULE_INDEX
};

compile_time_assert((LAST_MODULE_INDEX == ANOUBIS_MODULESNO), \
    MODULE_INDEX_mismatch_ANOUBIS_MODULESNO);


/**
 * Class to collect utilities formerly based in main.
 */
class MainUtils : public Singleton<MainUtils>
{
	public:
		/**
		 * Destructor.
		 */
		~MainUtils(void);

		uid_t getUserIdByName(wxString) const;
		wxString getUserNameById(uid_t) const;
		wxString getIconPath(wxString);
		wxIcon *loadIcon(wxString);
		wxString getDataDir(void);
		void status(wxString);
		void initModules(MainFrame *);
		Module *getModule(enum moduleIdx);
		bool connectCommunicator(bool);
		void checkBootConf(void);
		wxString getGrubPath(void);
		void autoStart(bool);

	protected:
		/**
		 * Constructor.
		 * You can't call it from anywhere. Use instance().
		 */
		MainUtils(void);

	private:
		wxStandardPaths paths_;
		Module *modules_[ANOUBIS_MODULESNO];
		wxString grubPath_;

	friend class Singleton<MainUtils>;
};

#endif	/* _MAINUTILS_H_ */
