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
	this->recursive_ = false;
	this->orphaned_ = false;
	this->keyId_ = 0;
	this->keyIdLen_ = 0;
}

ComSfsListTask::ComSfsListTask(uid_t uid, const wxString &dir)
{
	this->recursive_ = false;
	this->orphaned_ = false;
	this->keyId_ = 0;
	this->keyIdLen_ = 0;

	setRequestParameter(uid, dir);
}

ComSfsListTask::~ComSfsListTask(void)
{
	if (keyId_ != 0)
		free(keyId_);
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

bool
ComSfsListTask::isRecursive(void) const
{
	return (this->recursive_);
}

void
ComSfsListTask::setRecursive(bool recursive)
{
	this->recursive_ = recursive;
}

bool
ComSfsListTask::fetchOrphaned(void) const
{
	return (this->orphaned_);
}

void
ComSfsListTask::setFetchOrphaned(bool enabled)
{
	this->orphaned_ = enabled;
}

bool
ComSfsListTask::haveKeyId(void) const
{
	return ((keyId_ != 0) && (keyIdLen_ > 0));
}

bool
ComSfsListTask::setKeyId(const u_int8_t *keyId, int len)
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
ComSfsListTask::getEventType(void) const
{
	return (anTASKEVT_SFS_LIST);
}

void
ComSfsListTask::exec(void)
{
	const uid_t	cur_uid = getuid();
	int		req_op;
	uid_t		req_uid;
	int		req_flags;

	resetComTaskResult();

	/* Operation depends on key-id. If set assume signature-operation */
	if ((this->keyId_ != 0) && (this->keyIdLen_ > 0))
		req_op = ANOUBIS_CHECKSUM_OP_SIG_LIST;
	else
		req_op = ANOUBIS_CHECKSUM_OP_LIST;

	if (uid_ == cur_uid) {
		/* Ask for your own list */
		req_uid = 0;
		req_flags = ANOUBIS_CSUM_NONE;
	} else if (cur_uid == 0) {
		/* root is allowed to request list for another user, too */
		req_uid = uid_;
		req_flags = ANOUBIS_CSUM_UID;
	} else {
		/* Non-root user is requesting the list for another user */
		return;
	}

	fetchSfsList(req_op, "", req_uid, req_flags);
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

void
ComSfsListTask::fetchSfsList(int op, const char *basepath, uid_t uid, int flags)
{
	struct anoubis_transaction	*ta;
	char				*path;
	char				**list;
	int				listSize;

	int result;
	if (directory_ != wxT("/"))
		result = asprintf(&path, "%s/%s",
		    (const char *)directory_.fn_str(), basepath);
	else
		result = asprintf(&path, "/%s", basepath);

	if (result == -1) {
		setComTaskResult(RESULT_OOM);
		free(path);
		return;
	} else {
		/* Remove trailing slash, if any */
		if (strlen(path) > 1 && *(path + strlen(path) - 1) == '/')
			*(path + strlen(path) -1) = '\0';
	}

	/* Create request */
	ta = anoubis_client_csumrequest_start(getComHandler()->getClient(),
	    op, path, this->keyId_, 0, this->keyIdLen_, uid, flags);

	if(!ta) {
		setComTaskResult(RESULT_COM_ERROR);
		free(path);
		return;
	}

	/* Wait for completition */
	while (!(ta->flags & ANOUBIS_T_DONE)) {
		if (!getComHandler()->waitForMessage()) {
			anoubis_transaction_destroy(ta);
			setComTaskResult(RESULT_COM_ERROR);
			free(path);
			return;
		}
	}

	if (ta->result) {
		setComTaskResult(RESULT_REMOTE_ERROR);
		setResultDetails(ta->result);
		anoubis_transaction_destroy(ta);
		free(path);
		return;
	}

	/* Extract filelist from response */
	list = anoubis_csum_list(ta->msg, &listSize);
	anoubis_transaction_destroy(ta);

	/* Put filelist from transaction into result of the task */
	if (list != 0) {
		for (int i = 0; i < listSize; i++) {
			wxString f = wxString::FromAscii(list[i]);

			if (strlen(basepath) > 0) {
				/* Prepend the basepath, if exists */
				wxString pp = wxString::FromAscii(basepath);

				if (!pp.EndsWith(wxT("/")))
					pp += wxT("/");

				f.Prepend(pp);
			}

			if (canFetch(f))
				fileList_.Add(f);

			if (this->recursive_ && f.EndsWith(wxT("/"))) {
				/* This is a directory, go down if requested */
				fetchSfsList(op, f.fn_str(), uid, flags);

				if (getComTaskResult() != RESULT_SUCCESS) {
					/* Can stop here */
					for (int j = i; j < listSize; j++)
						free(list[j]);

					free(list);
					free(path);

					return;
				}
			}

			free(list[i]);
		}

		free(list);
	}

	free(path);
	setComTaskResult(RESULT_SUCCESS);
}

bool
ComSfsListTask::canFetch(const wxString &file) const
{
	wxString path = wxT("/") + file;

	if (directory_ != wxT("/"))
		path.Prepend(directory_);

	if (!orphaned_) {
		/* Orphaned fetch disabled, thus only fetch exising files */
		struct stat fstat;
		return (stat(path.fn_str(), &fstat) == 0);
	} else
		return (true);
}
