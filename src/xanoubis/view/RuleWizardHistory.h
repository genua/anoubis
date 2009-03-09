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
		 * Answers to the most permission questions.
		 */
		enum permissionAnswer {
			PERM_NONE = 0,		/**< invalid in most cases */
			PERM_ALLOW_ALL,		/**< allow all */
			PERM_RESTRICT_DEFAULT,	/**< load defaults */
			PERM_RESTRICT_USER,	/**< user settings */
			PERM_DENY_ALL,		/**< allow nothing */
		};

		/**
		 * Answers to the overwrite question.
		 */
		enum overwriteAnswer {
			OVERWRITE_NONE = 0, /**< invalid in most cases */
			OVERWRITE_YES,	    /**< overwrite and ask with what. */
			OVERWRITE_DEFAULTS, /**< ovwerwrite with defaults. */
			OVERWRITE_NO,	    /**< do not overwrite. */
		};

		/**
		 * Constructor of this wizard history.
		 * @param None.
		 */
		RuleWizardHistory(void);

		/**
		 * Store the given program.
		 * This will store the given program and determine already
		 * existing alf, context and sandbox policies.
		 * @param[in] 1st The program to store.
		 * @return Nothing.
		 */
		void setProgram(const wxString &);

		/**
		 * Get the stored program.
		 * @param None.
		 * @return The stored program.
		 */
		wxString getProgram(void) const;

		/**
		 * Store the given checksum.
		 * This will store the given checksum.
		 * @param[in] 1st The checksum to store.
		 * @return Nothing.
		 */
		void setChecksum(const wxString &);

		/**
		 * Get the stored checksum.
		 * @param None.
		 * @return The stored checksum.
		 */
		wxString getChecksum(void) const;

		/**
		 * Does context policies exist for the stored program?
		 * @param None.
		 * @return True if policies already exist, false otherwise.
		 */
		bool haveContextPolicy(void) const;

		/**
		 * Store the answer to the question: on executing other
		 * programs, those have same restrictions?
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setSameContext(bool);

		/**
		 * On executing other programs, those have same restrictions?
		 * @param None.
		 * @return On true (aka yes) we have to create an empty
		 *	context block.\n
		 *	On false (aka no) we have to create a\n
		 *	'context new any'
		 */
		bool isSameContext(void) const;

		/**
		 * Store answer to the question: does the user want to set
		 * exceptions to 'context new any'?
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setContextException(bool);

		/**
		 * Does the user want to set exceptions to 'context new any'?
		 * @param None.
		 * @return True if exceptions shall be defined.
		 */
		bool haveContextException(void) const;

		/**
		 * Store the list of context exceptions.
		 * @param[in] 1st The list of exceptions.
		 * @return Nothing.
		 */
		void setContextExceptionList(const wxArrayString &);

		/**
		 * Get the list of context exceptions.
		 * @param None.
		 * @return The list of exceptions.
		 */
		wxArrayString getContextExceptionList(void) const;

		/**
		 * Does alf policies exist for the stored program?
		 * @param None.
		 * @return True if policies already exist, false otherwise.
		 */
		bool haveAlfPolicy(void) const;

		/**
		 * Store the answer of the question: shall we overwrite
		 * existing alf policies?
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setOverwriteAlfPolicy(enum overwriteAnswer);

		/**
		 * Get the answer of the question: shall we overwrite existing
		 * alf policies?
		 * @param None.
		 * @return The answer.
		 */
		enum overwriteAnswer shallOverwriteAlfPolicy(void) const;

		/**
		 * Store the answer to the question about the alf client
		 * permissions.
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setAlfClientPermission(enum permissionAnswer);

		/**
		 * Get the answer to the question about the alf client
		 * permissions.
		 * @param None.
		 * @return The answer.
		 */
		enum permissionAnswer getAlfClientPermission(void) const;

		/**
		 * Store the list of alf client port.
		 * @param[in] 1st The list of client port.
		 * @return Nothing.
		 */
		void setAlfClientPortList(const wxArrayString &);

		/**
		 * Get the list of alf client port.
		 * @param None.
		 * @return The list of client port.
		 */
		wxArrayString getAlfClientPortList(void) const;

		/**
		 * Store the answer to the question: ask on any other
		 * network access?
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setAlfClientAsk(bool);

		/**
		 * Shall we ask on any other client network access?
		 * @param None.
		 * @return True or false.
		 */
		bool getAlfClientAsk(void) const;

		/**
		 * Store the answer to the question: allow raw access?
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setAlfClientRaw(bool);

		/**
		 * Shall we allow raw network access?
		 * @param None.
		 * * @return True or false.
		 */
		bool getAlfClientRaw(void) const;

		/**
		 * Store the answer to the question about the alf server
		 * permissions.
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setAlfServerPermission(enum permissionAnswer);

		/**
		 * Get the answer to the question about the alf server
		 * permissions.
		 * @param None.
		 * @return The answer.
		 */
		enum permissionAnswer getAlfServerPermission(void) const;

		/**
		 * Store answer to the question: sandbox wizard, sandbox
		 * defaults or no sandbox?
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setSandbox(enum permissionAnswer);

		/**
		 * What does the user want about the sandbox?
		 * @param None.
		 * @return The answer.
		 */
		enum permissionAnswer haveSandbox(void) const;

		/**
		 * Does sandbox policies exist for the stored program?
		 * @param None.
		 * @return True if policies already exist, false otherwise.
		 */
		bool haveSandboxPolicy(void) const;

		/**
		 * Store the answer of the question: shall we overwrite
		 * existing sandbox policies?
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setOverwriteSandboxPolicy(enum overwriteAnswer);

		/**
		 * Get the answer of the question: shall we overwrite existing
		 * sandbox policies?
		 * @param None.
		 * @return The answer.
		 */
		enum overwriteAnswer shallOverwriteSandboxPolicy(void) const;

		/**
		 * Store the answer to the question about the sandbox read
		 * permissions.
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setSandboxReadPermission(enum permissionAnswer);

		/**
		 * Get the answer to the question about the sandbox read
		 * permissions.
		 * @param None.
		 * @return The answer.
		 */
		enum permissionAnswer getSandboxReadPermission(void) const;

		/**
		 * Store the answer to the question about the sandbox write
		 * permissions.
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setSandboxWritePermission(enum permissionAnswer);

		/**
		 * Get the answer to the question about the sandbox write
		 * permissions.
		 * @param None.
		 * @return The answer.
		 */
		enum permissionAnswer getSandboxWritePermission(void) const;

		/**
		 * Store the answer to the question about the sandbox execute
		 * permissions.
		 * @param[in] 1st The answer.
		 * @return Nothing.
		 */
		void setSandboxExecutePermission(enum permissionAnswer);

		/**
		 * Get the answer to the question about the sandbox execute
		 * permissions.
		 * @param None.
		 * @return The answer.
		 */
		enum permissionAnswer getSandboxExecutePermission(void) const;

		/**
		 * Fill navigation sizer with all items for a program page.
		 * @param[in] 1st The parent (aka caller).
		 * @param[in] 2nd The navigation sizer.
		 * @param[in] 3rd True on active page.
		 * @return Nothing.
		 */
		void fillProgramNavi(wxWindow *, wxSizer *, bool) const;

		/**
		 * Fill navigation sizer with all items for a context page.
		 * @param[in] 1st The parent (aka caller).
		 * @param[in] 2nd The navigation sizer.
		 * @param[in] 3rd True on active page.
		 * @return Nothing.
		 */
		void fillContextNavi(wxWindow *, wxSizer *, bool) const;

		/**
		 * Fill navigation sizer with all items for an alf page.
		 * @param[in] 1st The parent (aka caller).
		 * @param[in] 2nd The navigation sizer.
		 * @param[in] 3rd True on active page.
		 * @return Nothing.
		 */
		void fillAlfNavi(wxWindow *, wxSizer *, bool) const;

		/**
		 * Fill navigation sizer with all items for an sandbox page.
		 * @param[in] 1st The parent (aka caller).
		 * @param[in] 2nd The navigation sizer.
		 * @param[in] 3rd True on active page.
		 * @return Nothing.
		 */
		void fillSandboxNavi(wxWindow *, wxSizer *, bool) const;

	private:
		/**
		 * Use this font for the \b inactive section titles been
		 * pushed into the navigation sizer.
		 */
		wxFont normalSectionFont_;

		/**
		 * Use this font for the \b active section titles been
		 * pushed into the navigation sizer.
		 */
		wxFont activeSectionFont_;

		/**
		 * Run the wizard for this program.
		 */
		wxString program_;

		/**
		 * This is the current checksum of the given program.
		 */
		wxString csum_;

		/**
		 * Does the given program already has context policies?
		 */
		bool haveContextPolicy_;

		/**
		 * On executing other programs, those have same restrictions?
		 * Yes -> create empty context block\n
		 * No -> create 'context new any'
		 */
		bool isSameContext_;

		/**
		 * Are context exceptions defined?
		 */
		bool haveContextException_;

		/**
		 * The list of context exceptions.
		 */
		wxArrayString contextExceptionList_;

		/**
		 * Does the given program already has alf policies?
		 */
		bool haveAlfPolicy_;

		/**
		 * Shall we overwrite the existing alf policies?
		 */
		enum overwriteAnswer overwriteAlfPolicy_;

		/**
		 * Alf client permissions.
		 */
		enum permissionAnswer alfClientPermission_;

		/**
		 * The list of alf client ports.
		 */
		wxArrayString alfClientPortList_;

		/**
		 * Shall we ask on any other client network access?
		 */
		bool alfClientAsk_;

		/**
		 * Shall we allow raw network access?
		 */
		bool alfClientRaw_;

		/**
		 * Alf server permissions.
		 */
		enum permissionAnswer alfServerPermission_;

		/**
		 * What does the user want about the sandbox?
		 */
		enum permissionAnswer haveSandbox_;

		/**
		 * Does the given program already has sandbox policies?
		 */
		bool haveSandboxPolicy_;

		/**
		 * Shall we overwrite the existing sandbox policies?
		 */
		enum overwriteAnswer overwriteSandboxPolicy_;

		/**
		 * Sandbox read permissions.
		 */
		enum permissionAnswer sandboxReadPermission_;

		/**
		 * Sandbox write permissions.
		 */
		enum permissionAnswer sandboxWritePermission_;

		/**
		 * Sandbox execute permissions.
		 */
		enum permissionAnswer sandboxExecutePermission_;

		/**
		 * Push a section title to the navigation sizer.
		 * @param[in] 1st The parent (aka caller).
		 * @param[in] 2nd The navigation sizer.
		 * @param[in] 3rd True if this section is active.
		 * @param[in] 4th The section title to add.
		 * @return Nothing.
		 */
		void addTitle(wxWindow *, wxSizer *, bool, wxString) const;

		/**
		 * Push a value line to the navigation sizer.
		 * @param[in] 1st The parent (aka caller).
		 * @param[in] 2nd The navigation sizer.
		 * @param[in] 3rd The value to push.
		 * @return Nothing.
		 */
		void addValue(wxWindow *, wxSizer *, wxString) const;
};

#endif	/* _RULEWIZARDHISTORY_H_ */
