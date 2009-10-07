/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#ifndef _COMVERSIONTASK_H_
#define _COMVERSIONTASK_H_

#include "ComTask.h"

/**
 * Task to receive version-information from the daemon.
 *
 * The versions needs to be compared against the client-versions to prevent
 * incompatibility.
 *
 * Supported error-codes:
 * - <code>RESULT_COM_ERROR</code> Communication error. Failed to create a
 *   transaction or to fetch the answer-message.
 * - <code>RESULT_REMOTE_ERROR</code> The daemin could not collect version
 *   information. getResultDetails() will return the remote error-code and can
 *   be evaluated by strerror(3) or similar.
 */
class ComVersionTask : public ComTask
{
	public:
		/**
		 * Default c'tor.
		 */
		ComVersionTask(void);

		/**
		 * Returns the version of the Anoubis protocol used by the
		 * daemon.
		 * @return Version of remote Anoubis protocol
		 */
		int getProtocolVersion(void) const;

		/**
		 * Returns the version of the APN parser used by the daemon.
		 * @return Version of remote APN parser
		 */
		int getApnVersion(void) const;

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

	protected:
		/**
		 * @see ComTask::resetComTaskResult()
		 */
		void resetComTaskResult(void);

	private:
		/**
		 * The transaction.
		 */
		struct anoubis_transaction *ta_;

		/**
		 * The Anoubis protocol version of the daemon.
		 */
		int protocolVersion_;

		/**
		 * The APN parser version of the daemon.
		 */
		int apnVersion_;

		/**
		 * Method extracts version information from the given message.
		 * @return true on success, false of the message is invalid.
		 */
		bool processVersionMessage(struct anoubis_msg *);
};

#endif	/* _COMVERSIONTASK_H_ */
