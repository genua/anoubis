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

#ifndef _RULEWIZARD_H_
#define _RULEWIZARD_H_

#include <wx/wizard.h>

#include "RuleWizardHistory.h"
#include "PolicyRuleSet.h"

/**
 * This is the main wizard class.
 *
 * \dot
 * digraph G {
 *	center=true;
 *	remincross=true;
 *
 *	node [shape=box, fontsize=10];
 *	PAGE_PROGRAM [label="Choose Program"];
 *	PAGE_CTX [label="Context with same permissions"];
 *	PAGE_CTX_EXCEPT [label="Context Exceptions List"];
 *	PAGE_ALF_OVERWRITE [label="ALF overwrite policy"];
 *	PAGE_ALF_CLIENT [label="ALF client permissions"];
 *	PAGE_ALF_CLIENT_PORTS [label="ALF client ports"];
 *	PAGE_ALF_SERVER [label="ALF server permissions"];
 *	PAGE_ALF_SERVER_PORTS [label="ALF server ports"];
 *	PAGE_SB [label="SB general"];
 *	PAGE_SB_OVERWRITE [label="SB overwrite policy"];
 *	PAGE_SB_READ [label="SB read permissions"];
 *	PAGE_SB_READ_FILES [label="SB read files"];
 *	PAGE_SB_WRITE [label="SB write permissions"];
 *	PAGE_SB_WRITE_FILES [label="SB write files"];
 *	PAGE_SB_EXECUTE [label="SB execute permissions"];
 *	PAGE_SB_EXECUTE_FILES [label="SB execute files"];
 *	PAGE_FINAL [label="Final"];
 *
 *	node [shape=diamond, fontsize=10];
 *	haveContextPolicy;
 *	haveContextException;
 *	haveAlfPolicy;
 *	overwriteAlf;
 *	alfClientUserDefines;
 *	alfServerUserDefines;
 *	haveSandboxPolicy;
 *	overwriteSandbox;
 *	sbDefines;
 *	sbReadUserDefines;
 *	sbWriteUserDefines;
 *	sbExecuteUserDefines;
 *
 *	{ rank="same"; haveContextPolicy; PAGE_CTX; }
 *	{ rank="same"; haveContextException; PAGE_CTX_EXCEPT; }
 *	{ rank="same"; PAGE_CTX_EXCEPT; haveContextException;  }
 *	{ rank="same"; haveAlfPolicy; PAGE_ALF_OVERWRITE; }
 *	{ rank="same"; alfClientUserDefines; PAGE_ALF_CLIENT_PORTS; }
 *	{ rank="same"; alfServerUserDefines; PAGE_ALF_SERVER_PORTS; }
 *	{ rank="same"; haveSandboxPolicy; PAGE_SB_OVERWRITE; }
 *	{ rank="same"; sbReadUserDefines; PAGE_SB_READ_FILES; }
 *	{ rank="same"; sbWriteUserDefines; PAGE_SB_WRITE_FILES; }
 *	{ rank="same"; sbExecuteUserDefines; PAGE_SB_EXECUTE_FILES; }
 *
 *	edge [fontsize=10];
 *
 *	PAGE_PROGRAM -> haveContextPolicy;
 *	haveContextPolicy -> PAGE_CTX [taillabel="no", minlen=2.00];
 *	haveContextPolicy:s -> haveAlfPolicy:n [taillabel="yes"];
 *
 *	PAGE_CTX -> haveContextException:n;
 *	haveContextException -> PAGE_CTX_EXCEPT [taillabel="yes"];
 *	haveContextException -> haveAlfPolicy [taillabel="no"];
 *	PAGE_CTX_EXCEPT:s -> haveAlfPolicy:n;
 *
 *	haveAlfPolicy -> PAGE_ALF_OVERWRITE [taillabel="yes"];
 *	haveAlfPolicy:s -> PAGE_ALF_CLIENT [taillabel="no"];
 *
 *	PAGE_ALF_OVERWRITE -> overwriteAlf;
 *	overwriteAlf:w -> PAGE_ALF_CLIENT [taillabel="yes"];
 *	overwriteAlf:s -> haveSandboxPolicy:n [taillabel="no"];
 *
 *	PAGE_ALF_CLIENT -> alfClientUserDefines:n;
 *	alfClientUserDefines:e -> PAGE_ALF_CLIENT_PORTS:w
 *	    [taillabel="PERM_RESTRICT_USER"];
 *	alfClientUserDefines:s -> PAGE_ALF_SERVER
 *	    [taillabel="PERM_ALLOW_ALL\nPERM_RESTRICT_DEFAULT\nPERM_DENY_ALL"];
 *
 *	PAGE_ALF_SERVER -> alfServerUserDefines:n;
 *	alfServerUserDefines:e -> PAGE_ALF_SERVER_PORTS:w [taillabel="yes"];
 *	alfServerUserDefines:s -> haveSandboxPolicy:n
 *	    [taillabel="no or defaults"];
 *
 *	PAGE_ALF_CLIENT_PORTS:s -> PAGE_ALF_SERVER:n;
 *	PAGE_ALF_SERVER_PORTS -> haveSandboxPolicy:n;
 *
 *	haveSandboxPolicy -> PAGE_SB_OVERWRITE [taillabel="yes"];
 *	haveSandboxPolicy:s -> PAGE_SB [taillabel="no"];
 *
 *	PAGE_SB -> sbDefines:n;
 *	sbDefines:s -> PAGE_SB_READ:n [taillabel="user"];
 *	sbDefines:w -> PAGE_FINAL [taillabel="no or defaults"];
 *
 *	PAGE_SB_OVERWRITE -> overwriteSandbox:n;
 *	overwriteSandbox:w -> PAGE_SB_READ [taillabel="yes"];
 *	overwriteSandbox:s -> PAGE_FINAL;
 *
 *	PAGE_SB_READ -> sbReadUserDefines:n;
 *	sbReadUserDefines:e -> PAGE_SB_READ_FILES:w;
 *	sbReadUserDefines:s -> PAGE_SB_WRITE;
 *	PAGE_SB_READ_FILES:s -> PAGE_SB_WRITE:n;
 *
 *	PAGE_SB_WRITE -> sbWriteUserDefines:n;
 *	sbWriteUserDefines:e -> PAGE_SB_WRITE_FILES:w;
 *	sbWriteUserDefines:s -> PAGE_SB_EXECUTE;
 *	PAGE_SB_WRITE_FILES:s -> PAGE_SB_EXECUTE:n;
 *
 *	PAGE_SB_EXECUTE -> sbExecuteUserDefines:n;
 *	sbExecuteUserDefines:e -> PAGE_SB_EXECUTE_FILES:w;
 *	sbExecuteUserDefines:s -> PAGE_FINAL;
 *	PAGE_SB_EXECUTE_FILES -> PAGE_FINAL;
 * }
 * \enddot
 */
class RuleWizard : public wxWizard
{
	public:
		/**
		 * The wizard knows about these pages. The pages are organized
		 * as array and these are the used indices.
		 */
		enum wizardPages {
			PAGE_PROGRAM = 0,
			PAGE_CTX,
			PAGE_CTX_EXCEPT,
			PAGE_ALF_OVERWRITE,
			PAGE_ALF_CLIENT,
			PAGE_ALF_CLIENT_PORTS,
			PAGE_ALF_SERVER,
			PAGE_ALF_SERVER_PORTS,
			PAGE_SB,
			PAGE_SB_OVERWRITE,
			PAGE_SB_READ,
			PAGE_SB_READ_FILES,
			PAGE_SB_WRITE,
			PAGE_SB_WRITE_FILES,
			PAGE_SB_EXECUTE,
			PAGE_SB_EXECUTE_FILES,
			PAGE_FINAL,
			PAGE_EOL
		};

		/**
		 * Constructor of Wizard.
		 * @param None.
		 */
		RuleWizard(wxWindow *);

		/**
		 * Destructor of the Wizard.
		 * @param None.
		 */
		~RuleWizard(void);

		/**
		 * Get a special page.
		 * This will deliver the specified page if the given index
		 * meets the need of wizardPages range.
		 * @param[in] 1st Page index.
		 * @return Requested page or NULL in case of error.
		 */
		wxWizardPage *getPage(enum wizardPages);

		/**
		 * Get next page index.
		 * Based on the current page (given index) and the input
		 * already made, this will decide about the next page and
		 * return it's index.
		 * @param[in] 1st Index of the current page.
		 * @return Index of the next page.
		 */
		RuleWizard::wizardPages forwardTransition(enum wizardPages)
		    const;

		/**
		 * Get previous page index.
		 * Based on the current page (given index) and the input
		 * already made, this will decide about the previous page and
		 * return it's index.
		 * @param[in] 1st Index of the current page.
		 * @return Index of the previous page.
		 */
		RuleWizard::wizardPages backwardTransition(enum wizardPages)
		    const;

	private:
		/**
		 * The array of pages.
		 */
		wxWizardPage *pages_[PAGE_EOL];

		/**
		 * Store made input within this class.
		 */
		RuleWizardHistory *history_;

		/**
		 * Handle event of finished wizard.
		 * With this we start creating the (new) policies.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onWizardFinished(wxWizardEvent &);

		/**
		 * Helper method to create context policies.
		 * @param[in] 1st The ruleset of policies.
		 * @return Nothind.
		 */
		void createContextPolicy(PolicyRuleSet *) const;

		/**
		 * Helper method to create alf policies.
		 * @param[in] 1st The ruleset of policies.
		 * @return Nothind.
		 */
		void createAlfPolicy(PolicyRuleSet *) const;

		/**
		 * Helper method to create sandbox policies.
		 * @param[in] 1st The ruleset of policies.
		 * @return Nothind.
		 */
		void createSandboxPolicy(PolicyRuleSet *) const;

		/**
		 * Create the filter policies by list of services.
		 * @param[in] 1st The alf application policy (aka parent).
		 * @param[in] 2nd The direction.
		 * @return Nothing.
		 */
		void createAlfPortList(AlfAppPolicy *, int) const;

		/**
		 * Create a rule with the given permission for the root
		 * directory. If the permission is DENY the rule will log.
		 * @param[in] 1st The sandbox application policy (aka parent)
		 * @param[in] 2nd The access mask (read/write/access)
		 * @param[in] 3rd The action (allow/deny/ask)
		 */
		void createSbPermissionRoot(SbAppPolicy *, int, int) const;

		/**
		 * Create sandbox policies.
		 * @param[in] 1st The sandbox application policy (aka parent).
		 * @return Nothing.
		 */
		void createSbPermissions(SbAppPolicy *) const;

		/**
		 * First step of creation of sandbox-permission-rules.
		 *
		 * Here, the user can choose, if default-sandbox-policies
		 * should be created.
		 */
		void createSbPermissionsStage1(void) const;

		/**
		 * Second step of creation of sandbox-permission-rules.
		 *
		 * For each permission (read, write, execute) the user can
		 * choose, if default-policies should be created or the access
		 * can be allowed for everything.
		 *
		 * @param[in] 1st The sandbox application policy (aka parent).
		 * @param[in] 2nd The permission
		 */
		void createSbPermissionsStage2(SbAppPolicy *,
		    SbEntry::Permission) const;

		/**
		 * Third step of creation of sandbox-permission-rules.
		 *
		 * For each permission (read, write, execute) the user can
		 * specify an own list of allowed files.
		 *
		 * @param[in] 1st The sandbox application policy (aka parent).
		 * @param[in] 2nd The permission
		 */
		void createSbPermissionsStage3(SbAppPolicy *,
		    SbEntry::Permission) const;

		/**
		 * Converts a SbEntry::Permission-value into an
		 * APN_SBA_*-value.
		 *
		 * @param [in] The permission
		 * @return The related SbEntry::Permission-value
		 */
		static int toAPN_SBA(SbEntry::Permission);
};

#endif	/* _RULEWIZARD_H_ */
