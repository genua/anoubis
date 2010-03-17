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

#include <queue>
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
 * You can enable a recursive fetch-operation. If enabled, the task fetches
 * files and directories by asking for a sfs-list for each detected directory.
 * By default, the feature is disabled.
 *
 * When the operation was successful, you can receive the filelist with
 * getFileList(). The task checks the each file for existence. Only if a file
 * exists, it is reported back. You can disable the check, by activating the
 * orphaned-fetch-mode (setFetchOrphaned()). If enabled, all files (independent
 * from existence) are reported back.
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
		 * @see getDirectory()
		 */
		void setRequestParameter(uid_t, const wxString &);

		/**
		 * Tests whether the recursive fetch is enabled.
		 *
		 * If enabled, the task fetches all files and directory
		 * recursive below the specified directory.
		 *
		 * @return true if recursive fetch is enabled, false otherwise.
		 * @see setRecursive()
		 */
		bool isRecursive(void) const;

		/**
		 * Enables or disables the recursive fetch.
		 *
		 * @param recursive Set to true, if you want to enable the
		 *                  recursive fetch-operation.
		 * @see isRecursive()
		 */
		void setRecursive(bool);

		/**
		 * Tests whether the orphaned fetch is activated.
		 *
		 * If disabled (by default) each file is tested for existence
		 * before it is returned. If disabled, you will get every file
		 * (independent from existence).
		 *
		 * @return true if the orphaned fetch is activated, false
		 *         otherwise.
		 */
		bool fetchOrphaned(void) const;

		/**
		 * Enables or disables the orphaned fetch.
		 *
		 * @param enabled Set to true, if you want to activate the
		 *                feature.
		 * @see fetchOrphaned()
		 */
		void setFetchOrphaned(bool);

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
		 * Tests whether the upgraded fetch is activated.
		 *
		 * If disabled, you will get every file.
		 * @return true if the upgraded fetch is activated, false
		 *         otherwise.
		 */
		bool fetchUpgraded(void) const;

		/**
		 * Enables or disables the upgraded fetch.
		 *
		 * @param enabled Set to true, if you want to activate the
		 *               feature.
		 */
		void setFetchUpgraded(bool);

		/**
		 * Enables or disables a special list mode that aborts
		 * the listing once the first real file has been added
		 * to the list. This is useful if we only need to know if
		 * the list is empty or not.
		 * @param enabled Set to ture if you want to activate the
		 *     feature.
		 */
		void setOneFile(bool);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

		/**
		 * Implementation of Task::done().
		 */
		bool done(void);

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

	private:
		uid_t				 uid_;
		wxString			 directory_;
		bool				 recursive_;
		bool				 orphaned_;
		wxArrayString			 fileList_;
		u_int8_t			*keyId_;
		int				 keyIdLen_;
		struct anoubis_transaction	*ta_;
		char				*basepath_;
		std::queue<char *>		 dirqueue_;
		bool				 upgraded_;
		bool				 oneFile_;

		/**
		 * Performs a single fetch-operation.
		 *
		 * The recursive fetch is resolved, if necessary.
		 *
		 * @param op Operation to be send to anoubisd
		 * @param basepath Path relative to directory_ send to anoubisd
		 * @param uid Uid to be send to anoubisd
		 * @param flags Flags to be send to anoubisd
		 * @return True if a new transaction was created, falls
		 *     on error.
		 */
		bool fetchSfsList(const char *);

		/**
		 * Tests whether the given file can be fetched.
		 *
		 * If the fetch-orphaned-feature is disabled, the file must
		 * exist.
		 *
		 * @param file Relative path of file to check. Relative path
		 *             to the basedirectory specified with directory_.
		 * @return true, if you are allowed to fetch the file, false
		 *         otherwise.
		 */
		bool canFetch(const wxString &) const;
};

#endif	/* _COMSFSLISTTASK_H_ */
