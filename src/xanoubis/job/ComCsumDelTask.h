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

/**
 * Task to delete the checksum of a file from anoubisd.
 *
 * You have to specify the filename with ComCsumDelTask::setFile().
 *
 * You can bind the operation to a certificate. In this case, the checksum of
 * the file is removed, which is signed by the configured certificate.
 *
 * Supported error-codes:
 * - <code>RESULT_COM_ERROR</code> Communication error. Failed to create a
 *   transaction or to fetch the answer-message.
 * - <code>RESULT_REMOTE_ERROR</code> Operation(s) performed by anoubisd
 *   failed. getResultDetails() will return the remote error-code and can be
 *   evaluated by strerror(3) or similar.
 */
class ComCsumDelTask : public ComTask
{
	public:
		/**
		 * Default c'tor.
		 *
		 * You explicity need to set the filename by calling setFile().
		 */
		ComCsumDelTask(void);

		/**
		 * Constructs a ComCsumDelTask with an assigned filename.
		 *
		 * @param file The source-file
		 * @see setFile()
		 */
		ComCsumDelTask(const wxString &);

		/**
		 * D'tor.
		 */
		~ComCsumDelTask(void);

		/**
		 * Returns the source-file.
		 *
		 * The checksum of the file is removed.
		 *
		 * @return The source-file
		 */
		wxString getFile(void) const;

		/**
		 * Updates the source-file.
		 *
		 * This method needs to be called <i>before</i> the tesk is
		 * executed!
		 *
		 * @param file The new source-file
		 */
		void setFile(const wxString &);

		/**
		 * Tests whether a key-id is assigned to the task.
		 * @return true is returned, if a key-id is assigned, false
		 *         otherwise.
		 */
		bool haveKeyId(void) const;

		/**
		 * Provides a key-id used by the operation.
		 *
		 * Once configured, the checksum of the file, which is signed
		 * with the certificate behind the key-id is removed.
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

	private:
		wxString	file_;
		u_int8_t	*keyId_;
		int		keyIdLen_;
};

#endif	/* _COMCSUMDELTASK_H_ */
