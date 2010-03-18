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

#ifndef _COMCSUMGETTASK_H_
#define _COMCSUMGETTASK_H_

#include <config.h>

#include <sys/types.h>
#include <vector>

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include "ComTask.h"
#include <anoubis_transaction.h>

/**
 * Task to receive a registered checksum from anoubisd.
 *
 * Before the task gets scheduled, you have to specify the files you are
 * requesting (ComCsumGetTask::addFile()).
 *
 * You can add a keyid to the request. In this case, unsigend checksums and
 * signatures for the files are retrieved.
 *
 * If the task was successfully executed, you can ask for the checksum
 * data  using ComCsumGetTask::getChecksumData().
 *
 * If no checksum exists for the requested file ENOENT is returend for this
 * file and getChecksumData will not copy checksum data.
 *
 * Supported error-codes:
 * - <code>RESULT_COM_ERROR</code> Communication error. Failed to create a
 *   transaction or to fetch the answer-message.
 * - <code>RESULT_LOCAL_ERROR</code> Failed to resolve path in case of a
 *   symlink is specified.
 * - <code>RESULT_REMOTE_ERROR</code> Operation(s) performed by anoubisd
 *   failed. getResultDetails() will return the remote error-code and can be
 *   evaluated by anoubis_strerror.
 */
class ComCsumGetTask : public ComTask
{
	public:
		/**
		 * Default constructor.
		 */
		ComCsumGetTask(void);

		/**
		 * Destructor.
		 */
		~ComCsumGetTask(void);

		/**
		 * Add a path to the GET request.
		 *
		 * @param 1st The file.
		 * @return none.
		 */
		void addPath(const wxString &);

		/**
		 * Return the path associated with the given index.
		 *
		 * @param 1st The index.
		 * @return The path.
		 */
		wxString getPath(unsigned int idx) const;

		/**
		 * Return the total number of paths in this request.
		 */
		size_t getPathCount(void) const;

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
		 * Returns the result of the checksum get request for
		 * the path with the given index.
		 *
		 * @param idx The index.
		 * @return The error code. Zero if a checksum was received.
		 */
		int getChecksumError(unsigned int idx) const;

		/**
		 * Return the result of the signature get request for
		 * the path with the given index.
		 *
		 * @param idx The index.
		 * @return The error code. Zero if a signature was received.
		 */
		int getSignatureError(unsigned int idx) const;

		/**
		 * Returns the size of a checksum/signature.
		 *
		 * @param 1st The index of the path.
		 * @param 2nd The checksum type.
		 * @return The length of the checksum/signature of the
		 *     given type. Zero if no such checksum was received.
		 */
		size_t getChecksumLen(unsigned int idx, int type) const;

		/**
		 * Returns a pointer to the checksum data in the task.
		 * The caller is not allowed to modify this data.
		 *
		 * @param 1st The index of the corresponding path.
		 * @param 2nd The checksum/signature type.
		 * @param 3rd A pointer to the destination buffer.
		 * @param 4th The length of the destination buffer.
		 * @return True if data was returned.
		 */
		bool getChecksumData(unsigned int idx, int type,
		    const u_int8_t *&buffer, size_t &len) const;

		/**
		 * Assign a keyid to the request.
		 *
		 * @param 1st The keyid.
		 * @param 2nd The length of the keyid.
		 * @return True if successful.
		 */
		bool setKeyId(const u_int8_t *keyid, unsigned int kidlen);

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
		u_int8_t			*keyid_;
		unsigned int			 kidlen_;
		anoubis_transaction		*ta_;

		/**
		 * Create the neccessary checksum and signature request
		 * if they do not exist.
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

		/**
		 * Return a pointer to the checksum entry structure of the
		 * specified type or NULL if no checksum of that type was
		 * received.
		 * This function will automatically search for the checksum
		 * in the appropriate request, depending on the type.
		 *
		 * @param 1st The index of the file.
		 * @param 2nd The checksum type (ANOUBIS_SIG_TYPE_*)
		 * @return The anoubis_csentry structure or NULL.
		 */
		struct anoubis_csentry *get_checksum_entry(
		    unsigned int idx, int type) const;
};

#endif	/* _COMCSUMGETTASK_H_ */
