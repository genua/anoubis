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

#include <client/anoubis_client.h>
#include <client/anoubis_transaction.h>

#include <csum/csum.h>

#include "ComHandler.h"
#include "ComSfsListTask.h"
#include "TaskEvent.h"

ComSfsListTask::ComSfsListTask(void)
{
}

ComSfsListTask::ComSfsListTask(uid_t uid, const wxString &dir)
{
	setRequestParameter(uid, dir);
}

uid_t
ComSfsListTask::getUid(void) const
{
	return (this->uid_);
}

wxString
ComSfsListTask::getDirectory(void) const
{
	return (this->directory_);
}

void
ComSfsListTask::setRequestParameter(uid_t uid, const wxString &dir)
{
	this->uid_ = uid;

	/*
	 * Remove the trailing slash
	 * but leave the trailing slash, if the root-directory is specified.
	 */
	if ((dir != wxT("/")) && dir.EndsWith(wxT("/")))
		this->directory_ = dir.Mid(0, dir.Len() - 1);
	else
		this->directory_ = dir;
}

wxEventType
ComSfsListTask::getEventType(void) const
{
	return (anTASKEVT_SFS_LIST);
}

void
ComSfsListTask::exec(void)
{
	const uid_t			cur_uid = getuid();
	struct anoubis_transaction	*ta;
	char				path[directory_.Len() + 1];
	char				**list;
	int				listSize;

	resetComTaskResult();

	strlcpy(path, directory_.fn_str(), sizeof(path));

	/* Create request */
	if (uid_ == cur_uid) {
		/* Ask for your own list */
		ta = anoubis_client_csumrequest_start(
		    getComHandler()->getClient(), ANOUBIS_CHECKSUM_OP_LIST,
		    path, NULL, 0, 0, 0, ANOUBIS_CSUM_NONE);
	} else if (cur_uid == 0) {
		/* root is allowed to request list for another user, too */
		ta = anoubis_client_csumrequest_start(
		    getComHandler()->getClient(), ANOUBIS_CHECKSUM_OP_LIST,
		    path, NULL, 0, 0, uid_, ANOUBIS_CSUM_UID);
	} else {
		/* Non-root user is requesting the list for another user */
		return;
	}

	if(!ta) {
		setComTaskResult(RESULT_COM_ERROR);
		return;
	}

	/* Wait for completition */
	while (!(ta->flags & ANOUBIS_T_DONE)) {
		if (!getComHandler()->waitForMessage()) {
			anoubis_transaction_destroy(ta);
			setComTaskResult(RESULT_COM_ERROR);
			return;
		}
	}

	if (ta->result) {
		setComTaskResult(RESULT_REMOTE_ERROR);
		setResultDetails(ta->result);
		anoubis_transaction_destroy(ta);
		return;
	}

	/* Extract filelist from response */
	list = anoubis_csum_list(ta->msg, &listSize);
	anoubis_transaction_destroy(ta);

	/* Put filelist from transaction into result of the task */
	if (list != 0) {
		for (int i = 0; i < listSize; i++) {
			wxString f = wxString(list[i], wxConvFile);
			fileList_.Add(f);

			free(list[i]);
		}

		free(list);
	}

	setComTaskResult(RESULT_SUCCESS);
}

wxArrayString
ComSfsListTask::getFileList(void) const
{
	return (this->fileList_);
}

void
ComSfsListTask::resetComTaskResult(void)
{
	ComTask::resetComTaskResult();

	fileList_.Clear();
}
