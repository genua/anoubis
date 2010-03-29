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

#include <cerrno>
#include "ComTask.h"
#include "KeyCtrl.h"

ComTask::ComTask(void) : Task(Task::TYPE_COM)
{
	client_ = NULL;
	this->result_ = RESULT_INIT;
	this->resultDetails_ = 0;
}

ComTask::ComTaskResult
ComTask::getComTaskResult(void) const
{
	return (this->result_);
}

void
ComTask::setComTaskResult(ComTaskResult result)
{
	this->result_ = result;
}

int
ComTask::getResultDetails(void) const
{
	return (this->resultDetails_);
}

void
ComTask::setResultDetails(int result)
{
	this->resultDetails_ = result;
}

void
ComTask::setClient(struct anoubis_client *client)
{
	client_ = client;
}

struct anoubis_client *
ComTask::getClient(void) const
{
	return client_;
}

void
ComTask::setTaskResultAbort(void)
{
	setComTaskResult(RESULT_LOCAL_ERROR);
	setResultDetails(EINTR);
}

struct anoubis_sig *
ComTask::comLoadPrivateKey(PrivKey *privkey) const
{
	if (!privkey->isLoaded()) {
		KeyCtrl		*keyctrl = KeyCtrl::existingInstance();

		if (keyctrl)
			keyctrl->loadPrivateKey();
	}
	return privkey->getKey();
}
