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
#include <anoubis_protocol.h>

#include "ComCsumGetTask.h"
#include "TaskEvent.h"


wxEventType
ComCsumGetTask::getEventType(void) const
{
	return anTASKEVT_CSUM_GET;
}

void
ComCsumGetTask::exec(void)
{
	int		ret;

	createRequests(ANOUBIS_CHECKSUM_OP_GET2);
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

struct anoubis_csentry *
ComCsumGetTask::get_checksum_entry(unsigned int idx, int type) const
{
	struct anoubis_csmulti_record	*record;
	struct anoubis_csmulti_request	*request = NULL;
	struct anoubis_csentry		*entry = NULL;

	switch (type) {
	case ANOUBIS_SIG_TYPE_CS:
		request = csreq_;
		break;
	case ANOUBIS_SIG_TYPE_SIG:
	case ANOUBIS_SIG_TYPE_UPGRADECS:
		request = sigreq_;
		break;
	}
	if (!request)
		return NULL;
	record = anoubis_csmulti_find(request, idx);
	if (!record)
		return NULL;
	switch (type) {
	case ANOUBIS_SIG_TYPE_CS:
		entry = record->u.get.csum;
		break;
	case ANOUBIS_SIG_TYPE_SIG:
		entry = record->u.get.sig;
		break;
	case ANOUBIS_SIG_TYPE_UPGRADECS:
		entry = record->u.get.upgrade;
		break;
	}
	return entry;
}

size_t
ComCsumGetTask::getChecksumLen(unsigned int idx, int type) const
{
	anoubis_csentry	*entry = get_checksum_entry(idx, type);

	if (!entry)
		return 0;
	return get_value(entry->cslen);
}

bool
ComCsumGetTask::getChecksumData(unsigned int idx, int type,
    const u_int8_t *&buffer, size_t &buflen) const
{
	anoubis_csentry	*entry = get_checksum_entry(idx, type);
	size_t			 len;

	if (!entry) {
		buffer = NULL;
		buflen = 0;
		return false;
	}
	len = get_value(entry->cslen);
	if (!len) {
		buffer = NULL;
		buflen = 0;
		return false;
	}
	buflen = len;
	buffer = (const u_int8_t *)entry->csdata;
	return true;
}
