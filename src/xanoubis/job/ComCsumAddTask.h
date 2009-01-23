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

#ifndef _COMCSUMADDTASK_H_
#define _COMCSUMADDTASK_H_

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include "ComTask.h"

/**
 * Task to register the checksum of a file at anoubisd.
 *
 * You have to specify the filename with ComCsumAddTask::setFile(). The
 * checksum of this file is calculated and send to anoubisd.
 *
 * You can bind the operation to a certificate. In this case, the checksum is
 * calculated and signed with the key specified with setPrivateKey(). Then the
 * signed checksum is assigned to the certificate behind the key-id specified
 * with setKeyId() and sent back to anoubisd.
 *
 * Supported error-codes:
 * - <code>RESULT_LOCAL_ERROR</code> Calculation of checksum failed.
 *   getResultDetails() returns the error-code and can be evaluated by
 *   strerror(3) or similar.
 * - <code>RESULT_COM_ERROR</code> Communication error. Failed to create a
 *   transaction or to fetch the answer-message.
 * - <code>RESULT_REMOTE_ERROR</code> Operation(s) performed by anoubisd
 *   failed. getResultDetails() will return the remote error-code and can be
 *   evaluated by strerror(3) or similar.
 */
class ComCsumAddTask : public ComTask
{
	public:
		/**
		 * Default c'tor.
		 *
		 * You explicity need to set the filename by calling setFile().
		 */
		ComCsumAddTask(void);

		/**
		 * Constructs a ComCsumAddTask with an assigned filename.
		 *
		 * @param file The source-file
		 * @see setFile()
		 */
		ComCsumAddTask(const wxString &);

		/**
		 * D'tor.
		 */
		~ComCsumAddTask(void);

		/**
		 * Returns the source-file.
		 *
		 * The this file the checksum is calculated and send to
		 * anoubisd.
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
		 * Once configured, the signed checksum of the file is assigned
		 * to the certificate behind the configured key-id.
		 *
		 * @param keyId The key-id of the certificate
		 * @param keyIdLen Length of keyId
		 * @return true if you specified a correct key-id, false
		 *         otherwise.
		 *
		 * @note Furthermore you need to configure a private-key to use
		 *       signed checksums.
		 * @see LocalCertificate
		 * @see setPrivateKey()
		 */
		bool setKeyId(const u_int8_t *, int);

		/**
		 * Tests whether a private key is assigned to the task.
		 * @return true is returned, if a private key is assigned,
		 *         false otherwise.
		 */
		bool havePrivateKey(void) const;

		/**
		 * Configures a private-key.
		 *
		 * The key is used to sign the checksum of the file. As soon as
		 * the signature is calculated, the internal assignment to the
		 * private key is removed. Thus, if you want to re-use the same
		 * instance again, don't forget to call this method again!
		 *
		 * @param key The private key
		 *
		 * @note Furthermore you need to configure the key-id to use
		 *       signed checksums.
		 * @see PrivKey
		 * @see setKeyId()
		 */
		void setPrivateKey(struct anoubis_sig *);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

		/**
		 * Returns the calculated checksum.
		 *
		 * The checksum was calculated during the runtime of the task
		 * and sent to anoubisd.
		 *
		 * @param csum Destination buffer, where the resulting checksum
		 *             is written.
		 * @param size Size of destination buffer <code>csum</code>.
		 * @return On success, <code>ANOUBIS_CS_LEN</code> is returned.
		 *         <code>ANOUBIS_CS_LEN</code>. A return-code of 0
		 *         means, that nothing was written. It might happen,
		 *         if the result of the task is not success or the
		 *         destination buffer is not large enough to hold the
		 *         whole checksum.
		 */
		size_t getCsum(u_int8_t *, size_t) const;

	protected:
		/**
		 * @see ComTask::resetComTaskResult()
		 */
		void resetComTaskResult(void);

	private:
		wxString		file_;
		u_int8_t		*keyId_;
		int			keyIdLen_;
		struct anoubis_sig	*privKey_;
		u_int8_t		cs_[ANOUBIS_CS_LEN];

		/**
		 * Creates the payload sent to anoubisd in case of unsigned
		 * checksums.
		 *
		 * The checksum is calculated and assigned to the payload.
		 *
		 * @param path Path to file to be checksumed
		 * @param payload_len Length of returned payload-buffer is
		 *                    written into this argument.
		 */
		u_int8_t *createCsMsg(const char *, int *);

		/**
		 * Creates the payload sent to anoubisd in case of signed
		 * checksums.
		 *
		 * The checksum is calculated and signed. Together with the
		 * key-id, there are appended to the payload.
		 *
		 * @param path Path to file to be checksumed
		 * @param payload_len Length of returned payload-buffer is
		 *                    written into this argument.
		 */
		u_int8_t *createSigMsg(const char *, int *);
};

#endif	/* _COMCSUMADDTASK_H_ */
