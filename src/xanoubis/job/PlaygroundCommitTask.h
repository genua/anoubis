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

#ifndef _PLAYGROUNDCOMMITTASK_H_
#define _PLAYGROUNDCOMMITTASK_H_

#include <vector>

#include <ComTask.h>
#include <TaskEvent.h>
#include <anoubis_transaction.h>

/**
 * This task is used to commit file from a playground to the production
 * system in the GUI. This task is derived from ComTask but it is in fact
 * both a ComTask and an FsTask. After each step the task changes its type
 * and is rescheduled to the other thread.
 *
 * The task should not be reused. Allocate a new task for each commit.
 */
class PlaygroundCommitTask : public ComTask {
public:

	/**
	 * Constructor. The file that should be committed must be provided
	 * to this constructor.
	 *
	 * @param pgid The playground ID of the files.
	 * @param devs The device numbers of the files that will be committed.
	 *     This array must have the same number of elements as the
	 *     inos array.
	 * @param inos The inode numbers of the files that will be committed.
	 */
	PlaygroundCommitTask(uint64_t pgid, std::vector<uint64_t>devs,
	    std::vector<uint64_t>inos);

	/**
	 * Destructor.
	 */
	~PlaygroundCommitTask(void);

	/**
	 * Implementation of ComTask::exec and Task::exec.
	 *
	 * @param None.
	 * @return None.
	 */
	void exec(void);

	/**
	 * Implementation of ComTask::done.
	 *
	 * @param None.
	 * @return True once the task is done.
	 */
	bool done(void);

	/**
	 * Return the playground ID of the task.
	 *
	 * @param None.
	 * @return The playground ID.
	 */
	uint64_t getPgid(void) {
		return pgid_;
	};

	/**
	 * These values are used to track the commit state of the
	 * files in this task. The caller can check the state of each
	 * file by using getFileState(). Possible values are:
	 *
	 * STATE_TODO: File should be committed (it wasn't really tried yet,
	 *     or the initial link count check failed).
	 * STATE_NEED_OVERWRITE: Committing failed because the target file
	 *     exists. This user should confirm that the commit is wanted.
	 *     If so a new task with the forceOverwrite_ flag set to true
	 *     must be scheduled.
	 * STATE_DO_COMMIT: The pre-commit checks of the file were successful
	 *     and the ComThread should no try to commit the file.
	 * STATE_LABEL_REMOVED: The file's security label was removed, the
	 *     file should now be renamed.
	 * STATE_SCAN_FAILED: Scanning of the file was tried but the daemon
	 *     reported that scanning deteced a problem with the file.
	 * STATE_RENAME_FAILED: The file's security label was removed but
	 *     renaming the file failed. This is a serious issue and must
	 *     be reported to the user.
	 * STATE_COMPLETE: Commit was successful.
	 */
	enum FileState {
		STATE_TODO,
		STATE_NEED_OVERWRITE,
		STATE_DO_COMMIT,
		STATE_LABEL_REMOVED,
		STATE_SCAN_FAILED,
		STATE_RENAME_FAILED,
		STATE_COMPLETE,
	};

	/**
	 * Return the current state of the file with the given index.
	 *
	 * @param The index of the file in the device/inode list.
	 * @return The current file state.
	 */
	int getFileState(int idx) {
		return states_[idx];
	}

	/**
	 * Return the detailed anoubis error code for a file that
	 * could not be committed.
	 *
	 * @param The index of the file.
	 * @return The error code.
	 */
	int getFileError(int idx) {
		return errs_[idx];
	}

	/**
	 * Return the total number of files in this task.
	 *
	 * @param None.
	 * @return The number of elements in the device/inode arrays.
	 */
	int getFileCount(void) {
		return states_.size();
	}

	/**
	 * Return the device number of the file with the given index.
	 *
	 * @param idx The index.
	 * @return The file's device number.
	 */
	uint64_t getDevice(int idx) {
		return devs_[idx];
	}

	/**
	 * Return the inode number of the file with the given index.
	 *
	 * @param idx The index.
	 * @return The file's inode number.
	 */
	uint64_t getInode(int idx) {
		return inos_[idx];
	}

	/**
	 * Set the force overwrite flag to true. If this is set the
	 * task will commit files even if the target file exists.
	 */
	void setForceOverwrite(void) {
		forceOverwrite_ = true;
	}

private:

	/**
	 * Create a new file list transaction for the files in the
	 * current playground.
	 *
	 * @param None.
	 * @return None.
	 */
	void		createFileListTransaction(void);

	/**
	 * Create a new file commit transaction. The file name is in
	 * the field fullname_. Device and inode number are taken from
	 * the respective arrays using the current index. The current index
	 * is stored in the field idx_
	 *
	 * @param None.
	 * @return None.
	 */
	void		createCommitTransaction(void);

	/**
	 * This implements Task::exec for the case where the task is
	 * currently acting as a ComTask. It is called be exec().
	 *
	 * @param None.
	 * @return None.
	 */
	void		execCom(void);

	/**
	 * This implements Task::exec for the case where the task is
	 * currently acting an FsTask. It is called by exec().
	 *
	 * @param None.
	 * @return None.
	 */
	void		execFs(void);

	/**
	 * This function tries to process the current file (as given
	 * by the field idx_. Actions taken depend on the current
	 * state of the file. This function tries to continue file processing
	 * up to the point where daemon communication is required. If it
	 * succeeds it returns true and the caller should reschedule the
	 * jobs as a ComTask. Otherwise it returns false. The caller should
	 * proceed to the next file.
	 *
	 * @param None.
	 * @return True if processing should proceed in the ComTask.
	 */
	bool		execFsFile(void);

	/**
	 * Initialize the names_ array for the current file. This function
	 * takes the current file as given by idx_ and iterates over all
	 * files in the current file list. File name that match the current
	 * device and inode are collected and stored in a NULL terminated
	 * array that can be passed to the file processing function in
	 * libanoubisui. The names_ array is malloced, the names themselves
	 * point directly to the file list message.
	 *
	 * @param None.
	 * @return None.
	 */
	void		createNames(void);

	/**
	 * Implementation of Task::getEventType.
	 *
	 * @param None.
	 * @return The event type to send when this task completes.
	 */
	wxEventType	getEventType(void) const;

	/**
	 * The playground ID of this task. Set by teh constructor, read only.
	 */
	uint64_t			  pgid_;

	/**
	 * The device numbers of the files. Set by the constructor, read only.
	 */
	std::vector<uint64_t>		  devs_;

	/**
	 * The inode numbers of the files. Set by the constructor, read only.
	 */
	std::vector<uint64_t>		  inos_;

	/**
	 * The current state of each file. Can be retrieved by getFileState().
	 */
	std::vector<int>		  states_;

	/**
	 * The detailed error codes. Can be retrieved by getFileError().
	 */
	std::vector<int>		  errs_;

	/**
	 * The transaction for the current/last file list request.
	 */
	struct anoubis_transaction	 *listta_;

	/**
	 * The transaction for the current/last commit request.
	 */
	struct anoubis_transaction	 *committa_;

	/**
	 * In index of the current file.
	 */
	int				  idx_;

	/**
	 * Potential file names of the current file (device relative).
	 */
	const char			**names_;

	/**
	 * The complete file name of the current file. This is passed to
	 * the daemon in a commit request.
	 */
	char				 *fullname_;

	/**
	 * True if we received a commit request from the daemon for the
	 * current file. This request is triggered by the call to
	 * lremovexattr.
	 */
	bool				  gotCommitNotify_;

	/**
	 * True if some progress was made in the current iteration over
	 * the files. If progress was made we request a new file list
	 * an retry all files that could not be committed due to a failure
	 * in the initial link count check. This catches directories, that
	 * are not empty, too.
	 */
	bool				  progress_;

	/**
	 * True if the commit should be done in force mode, i.e.
	 * overwriting production system files is allowed.
	 */
	bool				  forceOverwrite_;
};

#endif	/* _PLAYGROUNDCOMMITTASK_H_ */
