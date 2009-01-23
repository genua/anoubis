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

#ifndef _COMSFSLISTTASK_H_
#define _COMSFSLISTTASK_H_

#include "ComTask.h"

/**
 * Task to retrieve the sfs-list from anoubisd.
 *
 * The sfs-list is bind to a user or a certificate.
 *
 * If a bindiung to a user is configured, the operation lists all checksumed
 * files under a directory, which belongs to the user.
 *
 * If a bindung to a certificate is configured, the operation lists all
 * checksumed files under a directory, which are signed by the certificate.
 *
 * Base directory and uid of the requested user are configured using
 * setRequestParameter(). The bindung to a certificate is enabled by providing
 * the key-id if the certificate (setKeyId()). Unless the key-id is configured,
 * the uid-bindung is used.
 *
 * When the operation was successful, you can receive the filelist with
 * getFileList().
 *
 * Supported error-codes:
 * - <code>RESULT_INIT</code> The task remains in RESULT_INIT state, if a
 *   non-root-user tries to fetch the sfslist of another user.
 * - <code>RESULT_COM_ERROR</code> Communication error. Failed to create a
 *   transaction or to fetch the answer-message.
 * - <code>RESULT_REMOTE_ERROR</code> Operation(s) performed by anoubisd
 *   failed. getResultDetails() will return the remote error-code and can be
 *   evaluated by strerror(3) or similar.
 */
class ComSfsListTask : public ComTask
{
	public:
		/**
		 * Std-c'tor.
		 *
		 * You explicity need to set the parameter by calling
		 * setRequestParameter().
		 */
		ComSfsListTask(void);

		/**
		 * Constructs a ComSfsListTask with an already assigned
		 * base-directory.
		 *
		 * @param uid The checksum-tree of the user with the specified
		 *            uid is scanned.
		 * @param dir The base-directory. This is the directory, which
		 *            is scanned for checksums.
		 */
		ComSfsListTask(uid_t, const wxString &);

		/**
		 * D'tor.
		 */
		~ComSfsListTask(void);

		/**
		 * Returns the requested uid.
		 *
		 * The checksum-tree of the user with the returned uid is
		 * scanned.
		 *
		 * @note An unprivileged (non-root) user is only allowed to
		 *       request its own sfs-list. If he tries to fetch the
		 *       sfs-list of another user, the task exited with
		 *       ComTask::RESULT_INIT (not executed).
		 */
		uid_t getUid(void) const;

		/**
		 * Returns the base-directory.
		 *
		 * The Anoubis daemon scans this directory for registered
		 * checksums. Each file, which has a checksum, is reported back
		 * to the client.
		 *
		 * @return Base search directory
		 */
		wxString getDirectory(void) const;

		/**
		 * Updates the input-parameter of the task.
		 *
		 * This method needs to be called <i>before</i> the task is
		 * executed!
		 *
		 * @param uid The checksum-tree of the user with the specified
		 *            uid is scanned.
		 * @param dir The base-directory. This is the directory, which
		 *            is scanned for checksums.
		 * @see getUid()
		 * @see getDirectory()
		 */
		void setRequestParameter(uid_t, const wxString &);

		/**
		 * Tests whether a key-id is assigned to the task.
		 * @return true is returned, if a key-id is assigned, false
		 *         otherwise.
		 */
		bool haveKeyId(void) const;

		/**
		 * Provides a key-id used by the list-operation.
		 *
		 * Once configured, the checksum-tree of the certificate behind
		 * the key-id is scanned.
		 *
		 * @param keyId The key-id of the certificate
		 * @param keyIdLen Length of keyId
		 * @return true if you specified a correct key-id, false
		 *         otherwise.
		 *
		 * @see LocalCertificate
		 */
		bool setKeyId(const u_int8_t *, int);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

		/**
		 * Returns a list of filenames, which has a registered
		 * checksum.
		 *
		 * After successful completition of the task you can receive
		 * the result of the task.
		 *
		 * @return List of filenames, which has a registered checksum.
		 */
		wxArrayString getFileList(void) const;

	protected:
		/**
		 * @see ComTask::resetComTaskResult()
		 */
		void resetComTaskResult(void);

	private:
		uid_t		uid_;
		wxString	directory_;
		wxArrayString	fileList_;
		u_int8_t	*keyId_;
		int		keyIdLen_;
};

#endif	/* _COMSFSLISTTASK_H_ */
