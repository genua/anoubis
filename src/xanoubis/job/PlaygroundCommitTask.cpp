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

#include <config.h>

#include <sys/types.h>

#ifdef LINUX
#include <attr/xattr.h>
#endif

#include "PlaygroundCommitTask.h"
#include "PlaygroundFileNotify.h"
#include "NotificationCtrl.h"
#include "anoubis_client.h"
#include "anoubis_playground.h"

PlaygroundCommitTask::PlaygroundCommitTask(uint64_t pgid,
    std::vector<uint64_t>devs, std::vector<uint64_t>inos)
{
	listta_ = NULL;
	committa_ = NULL;
	inos_ = inos;
	devs_ = devs;
	pgid_ = pgid;
	states_.resize(inos.size(), STATE_TODO);
	errs_.resize(inos.size(), EPERM);
	scanresults_.resize(inos.size());
	idx_ = -1;
	names_ = NULL;
	fullname_ = NULL;
	progress_ = false;
	forceOverwrite_ = false;
	noscan_ = false;
}

PlaygroundCommitTask::~PlaygroundCommitTask(void)
{
	if (listta_)
		anoubis_transaction_destroy(listta_);
	if (committa_)
		anoubis_transaction_destroy(committa_);
	if (names_)
		free(names_);
	if (fullname_)
		free(fullname_);
}

void
PlaygroundCommitTask::createFileListTransaction(void)
{
	if (listta_)
		anoubis_transaction_destroy(listta_);
	listta_ = anoubis_client_pglist_start(getClient(),
	    ANOUBIS_PGREC_FILELIST, pgid_);
	if (listta_ == NULL) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(ENOMEM);
		return;
	}
}

void
PlaygroundCommitTask::createCommitTransaction(void)
{
	char		*cleanname;

	if (committa_)
		anoubis_transaction_destroy(committa_);
	committa_ = anoubis_client_pgcommit_start(getClient(),
	    pgid_, fullname_, noscan_);
	if (committa_ == NULL) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(ENOMEM);
		return;
	}
	cleanname = strdup(fullname_);
	pgfile_normalize_file(cleanname);
	progress(wxString::Format(_("Committing %hs"), cleanname));
	free(cleanname);
}

void
PlaygroundCommitTask::execCom(void)
{
	/* Request a new file list. */
	if (getComTaskResult() != RESULT_INIT)
		return;
	if (idx_ == -1) {
		createFileListTransaction();
		return;
	}
	if (states_[idx_] != STATE_DO_COMMIT)
		return;
	gotCommitNotify_ = false;
}

void
PlaygroundCommitTask::parseScanResults(void)
{
	const char		**str;

	str = anoubis_client_parse_pgcommit_reply(committa_->msg);
	if (str == NULL) {
		errs_[idx_] = ENOMEM;
		return;
	}
	scanresults_[idx_].clear();
	for (int i=0; str[i]; i+= 2) {
		scanresults_[idx_].push_back(
		    wxString::Format(wxT("%hs"), str[i]));
		if (str[i+1] == NULL) {
			scanresults_[idx_].push_back(
			    _("No scanner output available!"));
			break;
		} else {
			scanresults_[idx_].push_back(
			    wxString::Format(wxT("%hs"), str[i+1]));
		}
	}
	free(str);
}

bool
PlaygroundCommitTask::done(void)
{
	if (getComTaskResult() != RESULT_INIT)
		return true;
	if (idx_ == -1) {
		if ((listta_->flags & ANOUBIS_T_DONE) == 0)
			return false;
		if (listta_->result == ESRCH) {
			/* Playground no longer exists. This is success. */
			setComTaskResult(RESULT_SUCCESS);
			return true;
		} else if (listta_->result) {
			setComTaskResult(RESULT_REMOTE_ERROR);
			setResultDetails(listta_->result);
			return true;
		}
		type_ = Task::TYPE_FS;
		return true;
	}
	if (states_[idx_] != STATE_DO_COMMIT) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(EINVAL);
		return true;
	}
	if (!gotCommitNotify_) {
		PlaygroundFileNotify		*notify;
		NotificationCtrl		*notifyCtrl;

		/* XXX CEH: Implement and handle timeouts ... */
		notifyCtrl = NotificationCtrl::existingInstance();
		if (notifyCtrl == NULL)
			return false;
		notify = notifyCtrl->getPlaygroundFileNotify();
		if (notify == NULL)
			return false;
		if (notify->getDevice() != devs_[idx_])
			return false;
		if (notify->getInode() != inos_[idx_])
			return false;
		if (notify->getPgId() != pgid_)
			return false;
		gotCommitNotify_ = true;
		createCommitTransaction();
		return false;
	}
	if ((committa_->flags & ANOUBIS_T_DONE) == 0)
		return false;
	if (committa_->result) {
		states_[idx_] = STATE_SCAN_FAILED;
		errs_[idx_] = committa_->result;
		parseScanResults();
	} else {
		states_[idx_] = STATE_LABEL_REMOVED;
	}
	type_ = Task::TYPE_FS;
	return true;
}

bool
PlaygroundCommitTask::execFsFile(void)
{
	int		 err;

	switch(states_[idx_]) {
	case STATE_TODO:
		createNames();
		if (names_ == NULL)
			return false;
		err = pgfile_check(devs_[idx_], inos_[idx_], names_,
		    !forceOverwrite_);
		if (err < 0) {
			errs_[idx_] = -err;
			if (errs_[idx_] == EMLINK || errs_[idx_] == EEXIST)
				states_[idx_] = STATE_NEED_OVERWRITE;
			return false;
		}
		if (fullname_)
			free(fullname_);
		fullname_ = NULL;
		for (unsigned int i=0; fullname_ == NULL && names_[i]; ++i) {
			if (pgfile_composename(&fullname_,
			    devs_[idx_], inos_[idx_], names_[i]) < 0)
				fullname_ = NULL;
		}
		if (fullname_ == NULL)
			return false;
#ifdef LINUX
		err = lremovexattr(fullname_, "security.anoubis_pg");
#else
		errno = ENOSYS;
		err = -1;
#endif
		/* Should not happen! */
		if (err == 0) {
			states_[idx_] = STATE_LABEL_REMOVED;
			progress_ = true;
			return execFsFile();
		}
		switch (errno) {
		case EINPROGRESS:
			states_[idx_] = STATE_DO_COMMIT;
			return true;
		case ENOTEMPTY:
			errs_[idx_] = ENOTEMPTY;
			return false;
		default:
			return false;
		}
	case STATE_LABEL_REMOVED:
		progress_ = true;
		errs_[idx_] = -pgfile_process(devs_[idx_], inos_[idx_], names_);
		if (errs_[idx_]) {
			states_[idx_] = STATE_RENAME_FAILED;
		} else {
			states_[idx_] = STATE_COMPLETE;
			errs_[idx_] = 0;
		}
		break;
	case STATE_DO_COMMIT:
		states_[idx_] = STATE_SCAN_FAILED;
		errs_[idx_] = EPERM;
		break;
	}
	return false;
}

void
PlaygroundCommitTask::execFs(void)
{
	if (idx_ >= 0) {
		if (execFsFile()) {
			type_ = Task::TYPE_COM;
			return;
		}
	}
	while (1) {
		idx_++;
		if (idx_ >= (int)states_.size())
			break;
		if (states_[idx_] == STATE_TODO && execFsFile()) {
			type_ = Task::TYPE_COM;
			return;
		}
	}
	if (progress_) {
		progress_ = 0;
		idx_ = -1;
		type_ = Task::TYPE_COM;
	} else {
		setComTaskResult(RESULT_SUCCESS);
	}
}

void
PlaygroundCommitTask::exec(void)
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

void
PlaygroundCommitTask::createNames(void)
{
	struct anoubis_msg	*m;
	std::vector<char *>	 tmp;

	for (m=listta_->msg; m; m = m->next) {
		unsigned int		i, offset;

		for (i=offset=0; i<get_value(m->u.pgreply->nrec); ++i) {
			Anoubis_PgFileRecord	*rec;
			uint64_t		 dev, ino;

			rec = (Anoubis_PgFileRecord *)
			    (m->u.pgreply->payload + offset);
			offset += get_value(rec->reclen);
			dev = get_value(rec->dev);
			ino = get_value(rec->ino);
			if (dev != devs_[idx_] || ino != inos_[idx_])
				continue;
			tmp.push_back(rec->path);
		}
	}
	tmp.push_back(NULL);
	if (names_)
		free(names_);
	names_ = (const char **)malloc(tmp.size() * sizeof(char *));
	if (names_ == NULL) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(ENOMEM);
		return;
	}
	for (unsigned int i=0; i<tmp.size(); ++i) {
		names_[i] = tmp[i];
	}
}

wxEventType
PlaygroundCommitTask::getEventType(void) const
{
	return anTASKEVT_PG_COMMIT;
}
