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

#include <config.h>

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include <anoubis_client.h>
#include <anoubis_transaction.h>

#include "ComCsumDelTask.h"
#include "TaskEvent.h"

wxEventType
ComCsumDelTask::getEventType(void) const
{
	return anTASKEVT_CSUM_DEL;
}

void
ComCsumDelTask::exec(void)
{
	int			ret;

	createRequests(ANOUBIS_CHECKSUM_OP_DEL);
	if (getComTaskResult() != RESULT_INIT)
		return;
	for (unsigned int idx = 0; idx < paths_.size(); ++idx) {
		ret = addPathToRequest(csreq_, paths_[idx].fn_str());
		if (ret < 0)
			goto fatal;
		if (sigreq_) {
			ret = addPathToRequest(sigreq_, paths_[idx].fn_str());
			if (ret < 0)
				goto fatal;
		}
	}
	done();
	return;
fatal:
	setComTaskResult(RESULT_LOCAL_ERROR);
	setResultDetails(-ret);
}
