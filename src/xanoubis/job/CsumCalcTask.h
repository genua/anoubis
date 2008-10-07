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

#ifndef _CSUMCALCTASK_H_
#define _CSUMCALCTASK_H_

#include <config.h>

#include <sys/types.h>

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include <wx/string.h>

#include "Task.h"

/**
 * Task to calculate the checksum of a file.
 */
class CsumCalcTask : public Task
{
	public:
		CsumCalcTask(void);

		/**
		 * Returns the path of the file, where the checksum should
		 * be calculated.
		 *
		 * @return Path of file to be checksumed
		 */
		wxString getPath(void) const;

		/**
		 * Updates the path of the, where the checksum should be
		 * calculated.
		 *
		 * This method needs to be called before the task is scheduled!
		 *
		 * @param path Path of file to be checksumed
		 */
		void setPath(const wxString &);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

		/**
		 * Returns the state of the latest checksum-calculation.
		 *
		 * @return On success, 0 is returned. On error a result < 0
		 *         is returned.
		 */
		int getResult(void) const;

		/**
		 * Returns the result (a checksum) of the latest
		 * checksum-calculation.
		 *
		 * @return The calculated checksum. Size of array is
		 *         ANOUBIS_CS_LEN.
		 */
		const u_int8_t *getCsum(void) const;

		/**
		 * Returns a hex-string representation of the latest
		 * checksum-calculation. Letters are all lowercase.
		 *
		 * @return Hex-string of checksum
		 */
		wxString getCsumStr(void) const;

	private:
		wxString	path_;
		int		result_;
		u_int8_t	cs_[ANOUBIS_CS_LEN];

		void reset();
};

#endif	/* _CSUMCALCTASK_H_ */
