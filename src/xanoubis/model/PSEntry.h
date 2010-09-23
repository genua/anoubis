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

#ifndef _PSENTRY_H_
#define _PSENTRY_H_

#include <unistd.h>
#include <wx/string.h>

#include "AnListClass.h"
#include "anoubis_protocol.h"
#include "anoubis_procinfo.h"
#include "config.h"

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif


/**
 * A single Process entry in the process browser.
 *
 * Represents one process with information from the daemon, references to
 * the relevant rules and additional information from the operating system.
 */
class PSEntry : public AnListClass
{
	public:
		/**
		 * c'tor with initial values.
		 * @param 1st  A process record from the daemon message.
		 * @param 2nd  The corresponding process info from the OS,
		 *             (obtained through anoubis_procinfo.h). If the
		 *             value is NULL the process is not known by the
		 *             OS. This may happen due to race conditions.
		 */
		PSEntry(const Anoubis_ProcRecord* procrec,
		    const struct anoubis_proc* procinfo);

		/**
		 * Get process id.
		 * @return the process id
		 */
		const wxString getProcessId(void) const;

		/**
		 * Get parent process id.
		 * @return the parent process id,
		 *     empty string if process does not exist anymore
		 */
		const wxString getParentProcessId(void) const;

		/**
		 * Get real user id.
		 * @return the real uid of the process,
		 *     empty string if process does not exist anymore
		 */
		const wxString getUID(void) const;

		/**
		 * Get effective user id.
		 * @return the effective uid of the process,
		 *     empty string if process does not exist anymore
		 */
		const wxString getEUID(void) const;

		/**
		 * Get real group id.
		 * @return the real gid of the process,
		 *     empty string if process does not exist anymore
		 */
		const wxString getGID(void) const;

		/**
		 * Get effective group id.
		 * @return the effective gid of the process,
		 *     empty string if process does not exist anymore
		 */
		const wxString getEGID(void) const;

		/**
		 * Get the process name (as in 'ps').
		 * @return the process name
		 */
		const wxString getProcessName(void) const;

		/**
		 * Get SecureExec flag.
		 * @return the secure exec flag
		 */
		bool getSecureExec(void) const;

		/**
		 * Get the playground ID of this process.
		 * @return the playground id, 0 if process is not in playground
		 */
		uint64_t getPlaygroundId(void) const;

		/**
		 * Get path with full name for the process.
		 * @return the process path.
		 */
		const wxString getPathProcess(void) const;

		/**
		 * Get path with full name for the admin context
		 * @return the admin context path.
		 */
		const wxString getPathAdminContext(void) const;

		/**
		 * Get path with full name for the user context
		 * @return the user context path.
		 */
		const wxString getPathUserContext(void) const;

		/**
		 * Get checksum for the process.
		 * @return the checksum for the process.
		 */
		const wxString getChecksumProcess(void) const;

		/**
		 * Get checksum for the admin context.
		 * @return the checksum for the admin context.
		 */
		const wxString getChecksumUserContext(void) const;

		/**
		 * Get checksum for the user context.
		 * @return the checksum for the user context.
		 */
		const wxString getChecksumAdminContext(void) const;

		/**
		 * Return the rule id of the ALF admin rules.
		 * @param None.
		 * @return The rule id.
		 */
		unsigned long getAlfAdminRule(void) const {
			return rule_alf_[0];
		}

		/**
		 * Return the rule id of the ALF user rules.
		 * @param None.
		 * @return The rule id.
		 */
		unsigned long getAlfUserRule(void) const {
			return rule_alf_[1];
		}

		/**
		 * Return the rule id of the sandbox admin rules.
		 * @param None.
		 * @return The rule id.
		 */
		unsigned long getSbAdminRule(void) const {
			return rule_sb_[0];
		}

		/**
		 * Return the rule id of the sandbox user rules.
		 * @param None.
		 * @return The rule id.
		 */
		unsigned long getSbUserRule(void) const {
			return rule_sb_[1];
		}

		/**
		 * Return the rule id of the CTX admin rules.
		 * @param None.
		 * @return The rule id.
		 */
		unsigned long getCtxAdminRule(void) const {
			return rule_ctx_[0];
		}

		/**
		 * Return the rule id of the CTX user rules.
		 * @param None.
		 * @return The rule id.
		 */
		unsigned long getCtxUserRule(void) const {
			return rule_ctx_[1];
		}

	private:
		long pid_;	/**< Process id */
		long ppid_;	/**< Parent pid */
		long uid_;	/**< real uid */
		long euid_;	/**< effective uid */
		long gid_;	/**< real gid */
		long egid_;	/**< effective gid */

		wxString process_name_;	/**< Process name as in 'ps' */

		uint64_t pgid_;		/**< Playground ID, 0 if not in PG */
		bool     secure_exec_;	/**< Secure exec flag */

		/**
		 * Full path of binary for admin context[0],
		 * user context[1] and process itself[2].
		 */
		wxString path_[3];

		/**
		 * Stringified checksum of binary for admin context[0],
		 * user context[1] and process itself[2].
		 */
		wxString checksum_[3];

		/**
		 * Relevant ALF rule for admin[0] and user[1]
		 */
		uint32_t rule_alf_[2];

		/**
		 * Relevant SB rule for admin[0] and user[1]
		 */
		uint32_t rule_sb_[2];

		/**
		 * Relevant CTX rule for admin[0] and user[1]
		 */
		uint32_t rule_ctx_[2];
};

#endif	/* _PSENTRY_H_ */
