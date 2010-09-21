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

#include "PSEntry.h"
#include "PolicyUtils.h"

PSEntry::PSEntry(Anoubis_ProcRecord* psrec,
    struct anoubis_proc* psinfo)
{
	int i;

	/* values from daemon */
	pid_ = get_value(psrec->pid);
	pgid_ = get_value(psrec->pgid);
	secure_exec_ = get_value(psrec->secureexec);

	for (i=0; i<2; i++) {
		rule_alf_[i] = get_value(psrec->alfrule[i]);
		rule_sb_[i] = get_value(psrec->sbrule[i]);
		rule_ctx_[i] = get_value(psrec->ctxrule[i]);
	}

	/* process payload (payload is already verified) */
	char* payload = psrec->payload;
	for (i=0; i<3; i++) {
		/* checksum */
		PolicyUtils::csumToString((unsigned char*)payload,
		    ANOUBIS_CS_LEN, checksum_[i]);
		payload = payload+ANOUBIS_CS_LEN;

		/* path */
		path_[i] = wxString::FromAscii(payload);
		while (*payload != 0) {
			payload++;
		}
		payload++;
	}

	/* values from operating system */
	if (psinfo != NULL) {
		ppid_ = psinfo->ppid;
		uid_ =  psinfo->ruid;
		euid_ = psinfo->euid;
		gid_ =  psinfo->rgid;
		egid_ = psinfo->egid;
		process_name_ = wxString::FromAscii(psinfo->comm);
	} else {
		ppid_ = -1;
		uid_ = -1;
		euid_ = -1;
		gid_ = -1;
		egid_ = -1;
		process_name_ = wxEmptyString;
	}
}

const wxString
PSEntry::getProcessId(void)
{
	if (pid_ == -1) {
		return wxEmptyString;
	}
	return wxString::Format(wxT("%d"), pid_);
}

const wxString
PSEntry::getParentProcessId(void)
{
	if (ppid_ == -1) {
		return wxEmptyString;
	}
	return wxString::Format(wxT("%d"), ppid_);
}

const wxString
PSEntry::getUID(void)
{
	if (uid_ == -1) {
		return wxEmptyString;
	}
	return wxString::Format(wxT("%d"), uid_);
}

const wxString
PSEntry::getEUID(void)
{
	if (euid_ == -1) {
		return wxEmptyString;
	}
	return wxString::Format(wxT("%d"), euid_);
}

const wxString
PSEntry::getGID(void)
{
	if (gid_ == -1) {
		return wxEmptyString;
	}
	return wxString::Format(wxT("%d"), gid_);
}

const wxString
PSEntry::getEGID(void)
{
	if (egid_ == -1) {
		return wxEmptyString;
	}
	return wxString::Format(wxT("%d"), egid_);
}

const wxString
PSEntry::getProcessName(void)
{
	return process_name_;
}

bool
PSEntry::getSecureExec(void)
{
	return secure_exec_;
}

uint64_t
PSEntry::getPlaygroundId(void)
{
	return pgid_;
}

const wxString
PSEntry::getPathProcess(void)
{
	return path_[2];
}

const wxString
PSEntry::getPathAdminContext(void)
{
	return path_[0];
}

const wxString
PSEntry::getPathUserContext(void)
{
	return path_[1];
}

const wxString
PSEntry::getChecksumProcess(void)
{
	return checksum_[2];
}

const wxString
PSEntry::getChecksumAdminContext(void)
{
	return checksum_[0];
}

const wxString
PSEntry::getChecksumUserContext(void)
{
	return checksum_[1];
}
