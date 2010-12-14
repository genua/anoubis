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
	force_ = false;
	state_ = INFO;
	considerMatchList_ = false;
	no_progress_ = 0;
	matchList_.clear();
	unlinkList_.clear();
}

PlaygroundUnlinkTask::PlaygroundUnlinkTask(uint64_t pgId, bool force)
{
	reset();
	pgId_ = pgId;
	force_ = force;
	state_ = INFO;
	considerMatchList_ = false;
	no_progress_ = 0;
	matchList_.clear();
	unlinkList_.clear();
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
	matchList_[dip] = true;
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
	struct anoubis_msg	*message = NULL, *tmp;
	bool			 ret;

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
	} else if (transaction_ && state_ != FORCEUNLINK &&
	    (transaction_->msg == NULL ||
	    !VERIFY_LENGTH(transaction_->msg, sizeof(Anoubis_ListMessage)) ||
	    get_value(transaction_->msg->u.listreply->error) != 0)) {
		/* Message verification failed */
		setComTaskResult(RESULT_REMOTE_ERROR);
		setResultDetails(get_value(
		    transaction_->msg->u.listreply->error));
		anoubis_transaction_destroy(transaction_);
		transaction_ = NULL;
		return true;
	}

	/* Success */
	message = transaction_->msg;
	transaction_->msg = 0;

	anoubis_transaction_destroy(transaction_);
	transaction_ = NULL;

	ret = true;
	switch (state_) {
	case INFO:
		if (isPlaygroundActive(message)) {
			setComTaskResult(RESULT_REMOTE_ERROR);
			setResultDetails(EBUSY);
			break;
		}

		state_ = FILEFETCH;

		transaction_ = anoubis_client_list_start(getClient(),
		    ANOUBIS_REC_PGFILELIST, pgId_);
		if (transaction_ == NULL) {
			setComTaskResult(RESULT_LOCAL_ERROR);
			setResultDetails(ENOMEM);
			break;
		}
		ret = false;
		break;
	case FILEFETCH:
		if (!force_) {
			extractFileList(message);
			type_ = Task::TYPE_FS;
		} else {
			extractForceUnlinkList(message);

			if (!forceUnlink()) {
				ret = false;
				state_ = FORCEUNLINK;
			} else
				setComTaskResult(RESULT_SUCCESS);
		}

		break;
	case FORCEUNLINK:
		if ((ret = forceUnlink()))
			setComTaskResult(RESULT_SUCCESS);
		break;
	}
	while ((tmp = message) != NULL) {
		message = tmp->next;
		anoubis_msg_free(tmp);
	}
	return ret;
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
	cleanUnlinkList();
	matchList_.clear();
	setComTaskResult(RESULT_INIT);
}

void
PlaygroundUnlinkTask::execCom(void)
{
	transaction_ = anoubis_client_list_start(getClient(),
	    ANOUBIS_REC_PGLIST, pgId_);

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
			if (!hasMatchList()) {
				setComTaskResult(RESULT_LOCAL_ERROR);
				setResultDetails(EBUSY);
			} else {
				setComTaskResult(RESULT_SUCCESS);
			}
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

	cleanUnlinkList();
	for(; message; message = message->next) {
		for (i=offset=0; i<get_value(message->u.listreply->nrec); ++i) {
			record = (Anoubis_PgFileRecord *)
			    (message->u.listreply->payload + offset);
			offset += get_value(record->reclen);

			dev = get_value(record->dev);
			ino = get_value(record->ino);
			if (record->path[0] == 0 || pgfile_composename(&path,
			    dev, ino, record->path) < 0 || path == NULL) {
				continue;
			}

			dip = devInodePair(dev,ino);
			if (considerMatchList_) {
				if (matchList_.find(dip) != matchList_.end()) {
					unlinkList_[path] = dip;
				} else {
					free(path);
				}
			} else {
				unlinkList_[path] = dip;
			}
		}
	}
}

void
PlaygroundUnlinkTask::extractForceUnlinkList(struct anoubis_msg *message)
{
	int			 i = 0;
	int			 offset = 0;
	Anoubis_PgFileRecord	*record = NULL;
	uint64_t		 dev = 0;
	uint64_t		 ino = 0;
	devInodePair		 dip;

	forceUnlinkList_.clear();
	for(; message; message = message->next) {
		for (i=offset=0; i<get_value(message->u.listreply->nrec); ++i) {
			record = (Anoubis_PgFileRecord *)
			    (message->u.listreply->payload + offset);
			offset += get_value(record->reclen);

			dev = get_value(record->dev);
			ino = get_value(record->ino);
			dip = devInodePair(dev,ino);

			if (considerMatchList_) {
				if (matchList_.find(dip) != matchList_.end())
					forceUnlinkList_.insert(dip);
			} else
				forceUnlinkList_.insert(dip);
		}
	}
}

int
PlaygroundUnlinkTask::unlinkLoop(void)
{
	int						 count = 0;
	char						*path = NULL;
	std::map<char *, devInodePair>::iterator	 it;
	std::map<devInodePair, int>::iterator		 eIt;

	setResultDetails(0);
	for (it=unlinkList_.begin(); it!=unlinkList_.end(); it++) {
		path = it->first;
		if (unlink(path) == 0 ||
		    (errno == EISDIR && rmdir(path) == 0)) {
			count++;
			/* Erase from errorList if prev. reported. */
			eIt = errorList_.find(it->second);
			if (eIt != errorList_.end()) {
				errorList_.erase(eIt);
			}
		} else {
			setResultDetails(errno);
			errorList_[it->second] = errno;
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
		for (i=offset=0; i<get_value(message->u.listreply->nrec); ++i) {
			record = (Anoubis_PgInfoRecord *)
			    (message->u.listreply->payload + offset);
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
PlaygroundUnlinkTask::cleanUnlinkList(void)
{
	std::map<char *, devInodePair>::iterator	it;

	while (!unlinkList_.empty()) {
		it = unlinkList_.begin();
		if (it->first != NULL) {
			free(it->first);
		}
		unlinkList_.erase(it);
	}
}

bool
PlaygroundUnlinkTask::forceUnlink(void)
{
	if (forceUnlinkList_.empty()) {
		/* Done */
		return true;
	}

	/* Unlink first element */
	std::set<devInodePair>::iterator it = forceUnlinkList_.begin();
	uint64_t dev = (*it).first;
	uint64_t ino = (*it).second;

	forceUnlinkList_.erase(it);

	transaction_ =
	    anoubis_client_pgunlink_start(getClient(), pgId_, dev, ino);
	if (transaction_ == NULL) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(ENOMEM);

		return true;
	}

	return false;
}
