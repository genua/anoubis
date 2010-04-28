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

#ifndef _COMCSUMDELTASK_H_
#define _COMCSUMDELTASK_H_

#include "ComTask.h"
#include <anoubis_transaction.h>

#include <sys/types.h>
#include <vector>

/**
 * Task to delete the checksum of a file from anoubisd.
 *
 * Before the task gets schedule, you mstu specify the files names you of
 * the checksum that should be deleted using ComCsumDelTask::addFile().
 *
 * You can add a keyid to the request, too. In this case unsigned checksums
 * and signatures fore the files are removed.
 *
 * If the task was successfully executed, you can ask for the results for
 * each file.
 *
 * Supported error-codes:
 * - <code>RESULT_COM_ERROR</code> Communication error. Failed to create a
 *   transaction or to fetch the answer-message.
 * - <code>RESULT_LOCAL_ERROR</code> Failed to resolve path in case of a
 *   symlink is specified.
 * - <code>RESULT_REMOTE_ERROR</code> Operation(s) performed by anoubisd
 *   failed. getResultDetails() will return the remote error-code and can be
 *   evaluated by anoubis_strerror(3) or similar.
 */
class ComCsumDelTask : public ComTask
{
	public:
		/**
		 * Default constructor.
		 */
		ComCsumDelTask(void);

		/**
		 * Destructor.
		 */
		~ComCsumDelTask(void);

		/**
		 * Add a path to the DEL request.
		 *
		 * @param 1st The path.
		 * @retrun None.
		 */
		void addPath(const wxString);

		/**
		 * Return the number of paths in this CsumDelTask.
		 *
		 * @return The number of paths.
		 */
		unsigned int getPathCount(void) const;

		/**
		 * Return the path associated with an index in the request.
		 *
		 * @param idx the index.
		 * @return The path.
		 */
		wxString getPath(unsigned int idx) const;

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

		/**
		 * Implementation of ComTask::done().
		 */
		bool done(void);

		/**
		 * Returns the result of the checksum del request for the
		 * path with the given index.
		 *
		 * @param idx The index.
		 * @return The error code. Zero if the checksum was removed.
		 */
		int getChecksumError(unsigned int idx) const;

		/**
		 * Returns the result of the signature del requst for the
		 * path with the given index.
		 *
		 * @param idx The index.
		 * @return The error code. Zero if the signature was removed.
		 */
		int getSignatureError(unsigned int idx) const;

		/**
		 * Assign a keyid to the request.
		 *
		 * @param 1st The keyid.
		 * @param 2nd The length of the keyid.
		 * @return True if successful.
		 */
		bool setKeyId(const uint8_t *keyid, unsigned int kidlen);

		/**
		 * Return true if a keyid is assigned to the request.
		 *
		 * @param None.
		 * @return True if there is a keyid.
		 */
		bool haveKeyId(void) const;

	private:
		std::vector<wxString>		 paths_;
		struct anoubis_csmulti_request	*csreq_;
		struct anoubis_csmulti_request	*sigreq_;
		uint8_t				*keyid_;
		unsigned int			 kidlen_;
		anoubis_transaction		*ta_;

		/**
		 * Create the neccessary checksum and signature requests
		 * if the do not exist.
		 */
		void createRequests(void);

		/**
		 * Add a new path to a single request. If this fails add
		 * an error request.
		 *
		 * @param 1st The request.
		 * @param 2nd The path.
		 * @return Zero if something was added to the request.
		 *     An error code (ENOMEM) if neither the request nor
		 *     an error request could be added.
		 */
		int addPathToRequest(struct anoubis_csmulti_request *,
		    const char *path);
};

#endif	/* _COMCSUMDELTASK_H_ */
