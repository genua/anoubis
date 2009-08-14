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

#include "ComUpgradeListGetTask.h"

ComUpgradeListGetTask::ComUpgradeListGetTask(void)
{
	uid_ = 0 - 1;
	fileList_.Clear();
}

ComUpgradeListGetTask::ComUpgradeListGetTask(uid_t uid)
{
	uid_ = uid;
	fileList_.Clear();
}

ComUpgradeListGetTask::~ComUpgradeListGetTask(void)
{
	/* XXX ch: Currently nothing needs to be done here. */
}

uid_t
ComUpgradeListGetTask::getUid(void) const
{
	return (uid_);
}

void
ComUpgradeListGetTask::setRequestParameter(uid_t uid)
{
	uid_ = uid;
}

bool
ComUpgradeListGetTask::haveKeyId(void) const
{
	return ((keyId_ != 0) && (keyIdLen_ > 0));
}

bool
ComUpgradeListGetTask::setKeyId(const u_int8_t *keyId, int len)
{
	if ((keyId != 0) && (len > 0)) {
		u_int8_t *newKeyId = (u_int8_t *)malloc(len);

		if (newKeyId != 0)
			memcpy(newKeyId, keyId, len);
		else
			return (false);

		if (this->keyId_ != 0)
			free(this->keyId_);

		this->keyId_ = newKeyId;
		this->keyIdLen_ = len;

		return (true);
	} else
		return (false);
}

wxEventType
ComUpgradeListGetTask::getEventType(void) const
{
	return (anTASKEVT_UPGRADE_LIST);
}

void
ComUpgradeListGetTask::exec(void)
{
	resetComTaskResult();
}

bool
ComUpgradeListGetTask::done(void)
{
	/*
	 * XXX ch: Currently we are not talking to the daemon,
	 * XXX ch: because the protocol is not implemented.
	 * XXX ch: To proceed development we generate a fake list.
	 */
	fileList_.Add(wxT("/home/obaum/fileA"));
	fileList_.Add(wxT("/home/obaum/fileB"));
	fileList_.Add(wxT("/home/obaum/fileC"));
	fileList_.Add(wxT("/home/obaum/fileD"));
	fileList_.Add(wxT("/home/obaum/foo.pl"));
	setComTaskResult(RESULT_SUCCESS);

	return (true);
}

wxArrayString
ComUpgradeListGetTask::getFileList(void) const
{
	return (fileList_);
}

void
ComUpgradeListGetTask::resetComTaskResult(void)
{
	ComTask::resetComTaskResult();
	fileList_.Clear();
}
