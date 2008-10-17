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
 * If the task was successfully executed, you can ask for the checksum using
 * ComCsumGetTask::getCsum().
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
		 *             Specifies max. number of byte to be written.
		 * @return Number of bytes written to the destination buffer.
		 *         Number of bytes cannot be greater than
		 *         <code>ANOUBIS_CS_LEN</code>.
		 */
		size_t getCsum(u_int8_t *, size_t) const;

		/**
		 * Returns an hex-string representation of getCsum().
		 * @return Hex-string of getCsum().
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
		u_int8_t	cs_[ANOUBIS_CS_LEN];
};

#endif	/* _COMCSUMGETTASK_H_ */
