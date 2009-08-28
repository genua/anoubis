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

#include "ComSfsListTask.h"
#include "TaskEvent.h"

ComSfsListTask::ComSfsListTask(void) : dirqueue_()
{
	this->recursive_ = false;
	this->orphaned_ = false;
	this->keyId_ = 0;
	this->keyIdLen_ = 0;
	basepath_ = NULL;
	this->upgraded_ = false;
}

ComSfsListTask::ComSfsListTask(uid_t uid, const wxString &dir) : dirqueue_()
{
	this->recursive_ = false;
	this->orphaned_ = false;
	this->keyId_ = 0;
	this->keyIdLen_ = 0;
	basepath_ = NULL;
	this->upgraded_ = false;

	setRequestParameter(uid, dir);
}

ComSfsListTask::~ComSfsListTask(void)
{
	if (keyId_ != 0)
		free(keyId_);
	while(!dirqueue_.empty()) {
		char	*path;
		path = dirqueue_.front();
		dirqueue_.pop();
		free(path);
	}
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
	this->req_flags_ = ANOUBIS_CSUM_UID;

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
		this->req_flags_ = ANOUBIS_CSUM_KEY;

		return (true);
	} else
		return (false);
}

void
ComSfsListTask::setFetchUpgraded(bool upgrade)
{
	this->upgraded_ = upgrade;
}

bool
ComSfsListTask::fetchUpgraded(void) const
{
	return this->upgraded_;
}

wxEventType
ComSfsListTask::getEventType(void) const
{
	return (anTASKEVT_SFS_LIST);
}

void
ComSfsListTask::exec(void)
{
	if (req_flags_ & ANOUBIS_CSUM_UID) {
		const uid_t	cur_uid = getuid();
		if (uid_ != cur_uid && cur_uid != 0)
			return;
		req_uid_ = uid_;
	} else {
		req_uid_ = 0;
	}
	resetComTaskResult();
	ta_ = NULL;

	dirqueue_.push(strdup(""));
	done();
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

bool
ComSfsListTask::fetchSfsList(const char *path)
{
	char				*fullpath;
	unsigned int			 flags = ANOUBIS_CSUM_UID;

	int result;
	if (directory_ != wxT("/"))
		result = asprintf(&fullpath, "%s/%s",
		    (const char *)directory_.fn_str(), path);
	else
		result = asprintf(&fullpath, "/%s", path);

	if (result == -1) {
		setComTaskResult(RESULT_OOM);
		free(fullpath);
		return false;
	} else {
		/* Remove trailing slash, if any */
		if (strlen(fullpath) > 1 &&
		    *(fullpath + strlen(fullpath) - 1) == '/')
			*(fullpath + strlen(fullpath) -1) = '\0';
	}

	/* Create request */
	if (this->keyIdLen_) {
		flags = ANOUBIS_CSUM_KEY;
		if (this->upgraded_) {
			flags |= ANOUBIS_CSUM_UPGRADED;
			this->uid_ = 0;
		}
	}

	ta_ = anoubis_client_csumrequest_start(getClient(),
	    ANOUBIS_CHECKSUM_OP_GENERIC_LIST, fullpath, this->keyId_, 0,
	    this->keyIdLen_, this->uid_, flags);
	free(fullpath);

	if(!ta_) {
		setComTaskResult(RESULT_COM_ERROR);
		return (false);
	}
	return (true);
}

bool
ComSfsListTask::done(void)
{
	char		**list;
	int		  listSize;

	while(1) {
		if (ta_ == NULL && !dirqueue_.empty()) {
			basepath_ = dirqueue_.front();
			dirqueue_.pop();
			if (!fetchSfsList(basepath_)) {
				free(basepath_);
				break;
			}
		}
		/* At this point ta_ == NULL implies an empty dirqueue_. */
		if (ta_ == NULL)
			return (true);
		if ((ta_->flags & ANOUBIS_T_DONE) == 0)
			return (false);

		if (ta_->result) {
			setComTaskResult(RESULT_REMOTE_ERROR);
			setResultDetails(ta_->result);
			anoubis_transaction_destroy(ta_);
			ta_ = NULL;
			free(basepath_);
			break;
		} else {
			setComTaskResult(RESULT_SUCCESS);
		}

		/* Extract filelist from response */
		list = anoubis_csum_list(ta_->msg, &listSize);
		anoubis_transaction_destroy(ta_);
		ta_ = NULL;

		/* Put filelist from transaction into result of the task */
		if (list != 0) {
			for (int i = 0; i < listSize; i++) {
				wxString f = wxString::FromAscii(list[i]);

				/* Prepend the basepath, if exists */
				if (strlen(basepath_) > 0) {
					wxString pp;
					pp = wxString::FromAscii(basepath_);

					if (!pp.EndsWith(wxT("/")))
						pp += wxT("/");
					f.Prepend(pp);
				}
				if (canFetch(f))
					fileList_.Add(f);
				/*
				 * If this is a directory and we are recursive
				 * queue the directory.
				 */
				if (this->recursive_ && f.EndsWith(wxT("/"))) {
					dirqueue_.push(strdup(f.fn_str()));
				}
				free(list[i]);
			}
			free(list);
		}
	}
	/*
	 * We only reach this point in case of an error and the error
	 * code of the task is already set. Free the list of pending
	 * directories to make sure that subsequent calls to done do
	 * not restart the task.
	 */
	while(!dirqueue_.empty()) {
		char	*path;
		path = dirqueue_.front();
		dirqueue_.pop();
		free(path);
	}
	return (true);
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
