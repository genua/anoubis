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

#include "ComCsumHandler.h"
#include "ComTask.h"
#include <anoubis_transaction.h>

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
 * If no certificate is deposited for the requested key-id, getResultDetails()
 * will return EINVAL, getCsum() will not copy a checksum (returns 0) and
 * getCsumStr() will return an empty string.
 *
 * If no checksum exists for the requested file, getResultDetails() will return
 * ENOENT, getCsum() will not copy a checksum (returns 0) and getCsumStr() will
 * return an empty string.
 *
 * Supported error-codes:
 * - <code>RESULT_COM_ERROR</code> Communication error. Failed to create a
 *   transaction or to fetch the answer-message.
 * - <code>RESULT_LOCAL_ERROR</code> Failed to resolve path in case of a
 *   symlink is specified.
 * - <code>RESULT_REMOTE_ERROR</code> Operation(s) performed by anoubisd
 *   failed. getResultDetails() will return the remote error-code and can be
 *   evaluated by strerror(3) or similar.
 */
class ComCsumGetTask : public ComTask, public ComCsumHandler
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
		 * Returns the size of the requested checksum/signature.
		 *
		 * If a checksum was requested, the size is ANOUBIS_CS_LEN but
		 * if a signature was requested, the size is not constant.
		 *
		 * @return Size of requested checksum/signature.
		 */
		size_t getCsumLen(void) const;

		/**
		 * Returns the size of the upgrade checksum if present.
		 * The size should always be ANOUBIS_CS_LEN if the upgrade
		 * marker is present or 0 if no upgrade marker was found.
		 *
		 * @return Size of requested checksum/signature.
		 */
		size_t getUpgradeCsumLen(void) const;

		/**
		 * Returns the requested checksum.
		 *
		 * If the request-operation was successful, you can use this
		 * method to receive the requested checksum.
		 *
		 * @param csum Destination buffer, where the resulting checksum
		 *             is written.
		 * @param size Size of destination buffer <code>csum</code>.
		 * @return On success, the length of the checksum is
		 *         returned. Use getCsumLen() to get the length of the
		 *         checksum. A return-code of 0 means, that nothing was
		 *         written. It might happen, if the requested file does
		 *         not have a registered checksum or the destination
		 *         buffer is not large enough to hold the whole
		 *         checksum.
		 * @see getCsumLen()
		 */
		size_t getCsum(u_int8_t *, size_t) const;

		/**
		 * Returns the upgrade checksum if present.
		 *
		 * If the request-operation was successful and contained an
		 * upgrade checksum, you can use this method to receive the
		 * upgrade checksum.
		 *
		 * @param csum Destination buffer, where the resulting checksum
		 *             is written.
		 * @param size Size of destination buffer <code>csum</code>.
		 * @return On success, the length of the checksum is
		 *         returned. Otherwise 0 is returned.
		 * @see getUpgradeCsumLen()
		 */
		size_t getUpgradeCsum(u_int8_t *, size_t) const;

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
		u_int8_t			*cs_, *upcs_;
		size_t				cs_len_, upcs_len_;
		struct anoubis_transaction	*ta_;

		void resetCsum(void);
};

#endif	/* _COMCSUMGETTASK_H_ */
