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

#include <anoubis_client.h>
#include <anoubis_playground.h>
#include <anoubis_transaction.h>

#include "anoubis_errno.h"
#include "PlaygroundUnlinkTask.h"

PlaygroundUnlinkTask::PlaygroundUnlinkTask(uint64_t pgId)
{
	reset();
	pgId_ = pgId;
	state_ = INFO;
	considerMatchList_ = false;
	no_progress_ = 0;
}

PlaygroundUnlinkTask::~PlaygroundUnlinkTask(void)
{
	reset();
}

void
PlaygroundUnlinkTask::addMatchList(std::vector<PlaygroundFileEntry *> & list)
{
	std::vector<PlaygroundFileEntry *>::iterator	eIt;

	considerMatchList_ = true;
	for (eIt=list.begin(); eIt!=list.end(); eIt++) {
		addFile(*eIt);
	}
}

bool
PlaygroundUnlinkTask::addFile(PlaygroundFileEntry *entry)
{
	devInodePair dip;

	dip = devInodePair(entry->getDevice(), entry->getInode());
	matchList_[dip] = NULL;
	considerMatchList_ = true;

	return (true);
}

wxEventType
PlaygroundUnlinkTask::getEventType(void) const
{
	return (anTASKEVT_PG_UNLINK);
}

void
PlaygroundUnlinkTask::exec(void)
{
	switch (getType()) {
	case Task::TYPE_COM:
		execCom();
		break;
	case Task::TYPE_FS:
		execFs();
		break;
	}
}

bool
PlaygroundUnlinkTask::done(void)
{
	struct anoubis_msg *message = NULL;

	if (getComTaskResult() != RESULT_INIT) {
		/* An error occured. We are done. */
		return true;
	}

	if (transaction_ && (transaction_->flags & ANOUBIS_T_DONE) == 0) {
		/* Not finished yet */
		return false;
	} else if (transaction_ && transaction_->result) {
		/* Finished with an error */
		setComTaskResult(RESULT_REMOTE_ERROR);
		setResultDetails(transaction_->result);
		anoubis_transaction_destroy(transaction_);
		transaction_ = NULL;
		return true;
	} else if (transaction_ && (transaction_->msg == NULL ||
	    !VERIFY_LENGTH(transaction_->msg, sizeof(Anoubis_PgReplyMessage)) ||
	    get_value(transaction_->msg->u.pgreply->error) != 0)) {
		/* Message verification failed */
		setComTaskResult(RESULT_REMOTE_ERROR);
		setResultDetails(get_value(
		    transaction_->msg->u.pgreply->error));
		anoubis_transaction_destroy(transaction_);
		transaction_ = NULL;
		return true;
	}

	/* Success */
	message = transaction_->msg;
	transaction_->msg = 0;

	anoubis_transaction_destroy(transaction_);
	transaction_ = NULL;

	switch (state_) {
	case INFO:
		if (isPlaygroundActive(message)) {
			setComTaskResult(RESULT_REMOTE_ERROR);
			setResultDetails(EINPROGRESS);
			anoubis_msg_free(message);
			return true;
		}

		anoubis_msg_free(message);
		state_ = FILEFETCH;

		transaction_ = anoubis_client_pglist_start(getClient(),
		    ANOUBIS_PGREC_FILELIST, pgId_);
		if (transaction_ == NULL) {
			setComTaskResult(RESULT_LOCAL_ERROR);
			setResultDetails(ENOMEM);
			return true;
		}
		return false;
		break;
	case FILEFETCH:
		extractFileList(message);
		type_ = Task::TYPE_FS;
		break;
	}

	anoubis_msg_free(message);
	return true;
}

uint64_t
PlaygroundUnlinkTask::getPgId(void) const
{
	return pgId_;
}

bool
PlaygroundUnlinkTask::hasMatchList(void) const
{
	return (considerMatchList_);
}

bool
PlaygroundUnlinkTask::hasErrorList(void) const
{
	return (!errorList_.empty());
}

std::map<devInodePair, int> &
PlaygroundUnlinkTask::getErrorList(void)
{
	return (errorList_);
}

void
PlaygroundUnlinkTask::reset(void)
{
	cleanFileMap(unlinkList_);
	cleanFileMap(matchList_);
	setComTaskResult(RESULT_INIT);
}

void
PlaygroundUnlinkTask::execCom(void)
{
	transaction_ = anoubis_client_pglist_start(getClient(),
	    ANOUBIS_PGREC_PGLIST, pgId_);

	if (transaction_ == NULL) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(ENOMEM);
	}
}

void
PlaygroundUnlinkTask::execFs(void)
{
	unsigned int count = 0;

	count = unlinkLoop();
	if (count != 0 && count != unlinkList_.size()) {
		/* We deleted something but not all. Do another loop. */
		no_progress_ = 0;
		type_ = Task::TYPE_COM;
		state_ = INFO;
	} else if (count == 0) {
		/* We deleted nothing. */
		no_progress_++;

		if (no_progress_ == 2) {
			setComTaskResult(RESULT_LOCAL_ERROR);
			setResultDetails(EBUSY);
		} else {
			/* try once more, daemon might not have catched up */
			type_ = Task::TYPE_COM;
			state_ = INFO;
		}
	} else {
		/* everything deleted */
		setComTaskResult(RESULT_SUCCESS);
	}
}

void
PlaygroundUnlinkTask::extractFileList(struct anoubis_msg *message)
{
	int			 i = 0;
	int			 offset = 0;
	Anoubis_PgFileRecord	*record = NULL;
	char			*path = NULL;
	uint64_t		 dev = 0;
	uint64_t		 ino = 0;
	devInodePair		 dip;

	cleanFileMap(unlinkList_);
	for(; message; message = message->next) {
		for (i=offset=0; i<get_value(message->u.pgreply->nrec); ++i) {
			record = (Anoubis_PgFileRecord *)
			    (message->u.pgreply->payload + offset);
			offset += get_value(record->reclen);

			dev = get_value(record->dev);
			ino = get_value(record->ino);
			if (record->path[0] == 0 || pgfile_composename(&path,
			    dev, ino, record->path) < 0) {
				continue;
			}

			dip = devInodePair(dev,ino);
			if (considerMatchList_) {
				if (matchList_.find(dip) != matchList_.end()) {
					unlinkList_[dip] = path;
				}
			} else {
				unlinkList_[dip] = path;
			}
		}
	}
}

int
PlaygroundUnlinkTask::unlinkLoop(void)
{
	int			 count = 0;
	char			*path = NULL;
	fileMap::iterator	 it;

	std::map<devInodePair, int>::iterator eIt;

	setResultDetails(0);
	for (it=unlinkList_.begin(); it!=unlinkList_.end(); it++) {
		path = it->second;
		if (unlink(path) == 0 ||
		    (errno == EISDIR && rmdir(path) == 0)) {
			count++;
			/* Erase from errorList if prev. reported. */
			eIt = errorList_.find(it->first);
			if (eIt != errorList_.end()) {
				errorList_.erase(eIt);
			}
		} else {
			setResultDetails(errno);
			errorList_[it->first] = errno;
		}
	}

	return (count);
}

bool
PlaygroundUnlinkTask::isPlaygroundActive(struct anoubis_msg *message)
{
	int			 i = 0;
	int			 offset = 0;
	Anoubis_PgInfoRecord	*record = NULL;

	for(; message; message = message->next) {
		for (i=offset=0; i<get_value(message->u.pgreply->nrec); ++i) {
			record = (Anoubis_PgInfoRecord *)
			    (message->u.pgreply->payload + offset);
			offset += get_value(record->reclen);
			if (get_value(record->pgid) != pgId_) {
				continue;
			}
			if (get_value(record->nrprocs) == 0) {
				return false;
			} else {
				return true;
			}
		}
	}

	return false;
}

void
PlaygroundUnlinkTask::cleanFileMap(fileMap & list)
{
	fileMap::iterator it;

	while (!list.empty()) {
		it = list.begin();
		if (it->second != NULL) {
			free(it->second);
		}
		list.erase(it);
	}
}
