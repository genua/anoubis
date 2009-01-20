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

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include "ComTask.h"

/**
 * Task to receive a registered checksum from anoubisd.
 *
 * Before the task gets scheduled, you have to specify the file you are
 * requesting (ComCsumGetTask::setFile()).
 *
 * You can bind the operation to a certificate. In this case, the checksum of
 * the file is returned, which is signed by the configured certificate.
 *
 * If the task was successfully executed, you can ask for the checksum using
 * ComCsumGetTask::getCsum().
 *
 * If no checksum exists for the requested file, getResultDetails() will return
 * ENOENT, getCsum() will not copy a checksum (returns 0) and getCsumStr() will
 * return an empty string.
 *
 * Supported error-codes:
 * - <code>RESULT_COM_ERROR</code> Communication error. Failed to create a
 *   transaction or to fetch the answer-message.
 * - <code>RESULT_REMOTE_ERROR</code> Operation(s) performed by anoubisd
 *   failed. getResultDetails() will return the remote error-code and can be
 *   evaluated by strerror(3) or similar.
 */
class ComCsumGetTask : public ComTask
{
	public:
		/**
		 * Default c'tor.
		 *
		 * You explicity need to set the filename by calling setFile().
		 */
		ComCsumGetTask(void);

		/**
		 * Constructs a ComCsumGetTask with an already assigned file.
		 *
		 * @param file The requested filename
		 * @see setFile()
		 */
		ComCsumGetTask(const wxString &);

		/**
		 * D'tor.
		 */
		~ComCsumGetTask(void);

		/**
		 * Returns the requested file.
		 * @return The file you are requesting
		 */
		wxString getFile(void);

		/**
		 * Updates the requested file.
		 *
		 * Specifies the filename, which is sent to anoubisd.
		 * This method needs to be called <i>before</i> the task is
		 * executed!
		 *
		 * @param file The requested filename
		 */
		void setFile(const wxString &);

		/**
		 * Provides a key-id used by the operation.
		 *
		 * Once configured, the checksum of the file, which is signed
		 * with the certificate behind the key-id is returned.
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
		 * Returns the requested checksum.
		 *
		 * If the request-operation was successful, you can use this
		 * method to receive the requested checksum.
		 *
		 * @param csum Destination buffer, where the resulting checksum
		 *             is written.
		 * @param size Size of destination buffer <code>csum</code>.
		 * @return On success, <code>ANOUBIS_CS_LEN</code> is returned.
		 *         <code>ANOUBIS_CS_LEN</code>. A return-code of 0
		 *         means, that nothing was written. It might happen,
		 *         if the requested file does not have a registered
		 *         checksum or the destination buffer is not large
		 *         enough to hold the whole checksum.
		 */
		size_t getCsum(u_int8_t *, size_t) const;

		/**
		 * Returns an hex-string representation of getCsum().
		 * @return Hex-string of getCsum(). An empty string is
		 *         returned, if the requested file foes not have a
		 *         registered checksum.
		 * @note The resulting string does <b>not</b> start with the
		 *       leading hex-indicator <code>0x</code>!
		 * @see getCsum()
		 */
		wxString getCsumStr(void) const;

	protected:
		/**
		 * @see ComTask::resetComTaskResult()
		 */
		void resetComTaskResult(void);

	private:
		wxString	file_;
		u_int8_t	*keyId_;
		int		keyIdLen_;
		u_int8_t	cs_[ANOUBIS_CS_LEN];
};

#endif	/* _COMCSUMGETTASK_H_ */
