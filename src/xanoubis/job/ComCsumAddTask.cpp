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

#include <sys/types.h>

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include <client/anoubis_client.h>
#include <client/anoubis_transaction.h>
#include <csum/csum.h>
#include <anoubis_protocol.h>

#include "ComCsumAddTask.h"
#include "ComHandler.h"
#include "TaskEvent.h"

ComCsumAddTask::ComCsumAddTask(void)
{
	setFile(wxT(""));
}

ComCsumAddTask::ComCsumAddTask(const wxString &file)
{
	setFile(file);
}

wxString
ComCsumAddTask::getFile(void) const
{
	return (this->file_);
}

void
ComCsumAddTask::setFile(const wxString &file)
{
	this->file_ = file;
}

wxEventType
ComCsumAddTask::getEventType(void) const
{
	return (anTASKEVT_CSUM_ADD);
}

void
ComCsumAddTask::exec(void)
{
	/* Request to add a checksum for a file */
	struct anoubis_transaction	*ta;
	u_int8_t			cs[ANOUBIS_CS_LEN];
	int				len, ret;
	char				path[file_.Len() + 1];

	resetComTaskResult();

	strlcpy(path, file_.fn_str(), sizeof(path));

	/* Calculate checksum */
	len = ANOUBIS_CS_LEN;
	ret = anoubis_csum_calc(path, cs, &len);
	if (ret < 0) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		return;
	}

	/* Send request to daemon */
	ta =  anoubis_client_csumrequest_start(getComHandler()->getClient(),
	    ANOUBIS_CHECKSUM_OP_ADDSUM, path, cs, len, 0, ANOUBIS_CSUM_NONE);
	if(!ta) {
		setComTaskResult(RESULT_COM_ERROR);
		return;
	}

	/* Wait for completition */
	while (!(ta->flags & ANOUBIS_T_DONE)) {
		if (!getComHandler()->waitForMessage()) {
			setComTaskResult(RESULT_COM_ERROR);
			return;
		}
	}

	/* Result */
	if (ta->result)
		setComTaskResult(RESULT_REMOTE_ERROR);
	else
		setComTaskResult(RESULT_SUCCESS);

	anoubis_transaction_destroy(ta);
}
