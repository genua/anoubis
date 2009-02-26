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

#ifndef _RULEWIZARDHISTORY_H_
#define _RULEWIZARDHISTORY_H_

#include <wx/arrstr.h>
#include <wx/font.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/window.h>

/**
 */
class RuleWizardHistory
{
	public:
		/**
		 */
		enum alfPermissions {
			ALF_NONE = 0,
			ALF_CLIENT_ALLOW_ALL,
			ALF_CLEINT_RESTRICT_DEFAULT,
			ALF_CLEINT_RESTRICT_USER,
			ALF_CLIENT_DENY_ALL
		};

		/**
		 */
		RuleWizardHistory(void);

		/**
		 */
		void setProgram(const wxString &);

		/**
		 */
		wxString getProgram(void) const;

		/**
		 */
		bool getCtxHavePolicy(void) const;

		/**
		 */
		void setCtxSame(bool);

		/**
		 */
		bool getCtxSame(void) const;

		/**
		 */
		void setCtxException(bool);

		/**
		 */
		bool getCtxException(void) const;

		/**
		 */
		void setCtxExceptionList(const wxArrayString &);

		/**
		 */
		wxArrayString getCtxExceptionList(void) const;

		/**
		 */
		bool getAlfHavePolicy(void) const;

		/**
		 */
		void setAlfKeepPolicy(bool);

		/**
		 */
		bool getAlfKeepPolicy(void) const;

		/**
		 */
		void setAlfClientPermission(enum alfPermissions);

		/**
		 */
		RuleWizardHistory::alfPermissions getAlfClientPermission(void)
		    const;

		/**
		 */
		void fillProgramNavi(wxWindow *, wxSizer *, bool) const;

		/**
		 */
		void fillContextNavi(wxWindow *, wxSizer *, bool) const;

		/**
		 */
		void fillAlfNavi(wxWindow *, wxSizer *, bool) const;

	private:
		/**
		 */
		wxFont normalSectionFont_;

		/**
		 */
		wxFont activeSectionFont_;

		/**
		 */
		wxString program_;

		/**
		 */
		bool ctxHavePolicy_;

		/**
		 */
		bool ctxSame_;

		/**
		 */
		bool ctxException_;

		/**
		 */
		wxArrayString ctxExceptionList_;

		/**
		 */
		bool alfHavePolicy_;

		/**
		 */
		bool alfKeepPolicy_;

		/**
		 */
		enum alfPermissions alfClientPermission_;

		/**
		 */
		void addTitle(wxWindow *, wxSizer *, bool, wxString) const;

		/**
		 */
		void addValue(wxWindow *, wxSizer *, wxString) const;
};

#endif	/* _RULEWIZARDHISTORY_H_ */
