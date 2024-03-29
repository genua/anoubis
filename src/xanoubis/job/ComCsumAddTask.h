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

#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include "ComCSMultiTask.h"
#include <anoubis_transaction.h>

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
 *   anoubis_strerror(3) or similar.
 * - <code>RESULT_COM_ERROR</code> Communication error. Failed to create a
 *   transaction or to fetch the answer-message.
 * - <code>RESULT_REMOTE_ERROR</code> Operation(s) performed by anoubisd
 *   failed. getResultDetails() will return the remote error-code and can be
 *   evaluated by anoubis_strerror(3) or similar.
 */
class ComCsumAddTask : public ComCSMultiTask
{
	public:
		/**
		 * Default constructor.
		 */
		ComCsumAddTask(void);

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
		void setPrivateKey(PrivKey *);

		/**
		 * Assignes the contents of an sfs_entry-structure to the task.
		 * This function can be used to add pre-calculated checksums
		 * and/or signatures instead of caculating them on the fly.
		 *
		 * @param entry The sfs_entry to by assigned
		 */
		void setSfsEntry(const struct sfs_entry *);

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
		 * The checksum that was calculated during the runtime
		 * of the task and sent to the anoubis daemon is returned.
		 *
		 * @param idx The index of the path in the path list.
		 * @param csum Destination buffer, where the resulting checksum
		 *             is written.
		 * @param size Size of destination buffer <code>csum</code>.
		 * @return On success, the length of the checksum is returned.
		 *     A return-code of 0 means, that nothing was written.
		 *     This might happen, if the result of the task is
		 *     not success or the destination buffer is not large
		 *     enough to hold the whole checksum.
		 */
		size_t getCsum(unsigned int idx, u_int8_t *, size_t) const;

		/**
		 * Returns true if the checksums returned by getCsum is
		 * calculated from the local file system. False if it
		 * was specified explicitly.
		 * The return value should be true for normal register
		 * jobs and false in case of an import that specifies
		 * its own checksum.
		 * @param None.
		 * @return True if the checksums were calculated locally.
		 */
		bool isLocalChecksum(void) const;

	private:
		/**
		 * Add an error record to both the checksum and the
		 * signature request.
		 *
		 * @param 1st The path name.
		 * @param 2nd The error code.
		 * @return Zero if a record was added, a negative error
		 *     code in case of an error.
		 * NOTE: The only valid error should be ENOMEM.
		 */
		int addErrorPath(const char *path, int error);

		PrivKey				*privKey_;
		bool				 checksumsCalculated_;
};

#endif	/* _COMCSUMADDTASK_H_ */
