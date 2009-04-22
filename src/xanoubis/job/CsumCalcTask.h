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
 *
 * You need to setup the file (setPath()) before the tasks gets scheduled!
 *
 * If the file is a symlink, then the calcLink() flag becomes relevant. The
 * flag specifies the source of the checksum-calculation. If the flag is set,
 * the checksum is calculated over the resolved path of the link, otherwise
 * the source is the content of the resolved path.
 *
 * Example: A symlink <code>foo</code> points to a plain file <code>bar</code>.
 * The content of <code>bar</code> is <i>Hello World!</i>. Some pseudo code:
 *
 * <pre>
 * > ls -l foo
 * foo -> bar
 *
 * > cat foo
 * Hello World!
 *
 * > cat bar
 * Hello World!
 * </pre>
 *
 * If calcLink() is set, the checksum is calculated over the string
 * <code>bar</code>.
 *
 * If calcLink() is unset, the checksum is calculated over the string
 * <code>Hello World!</code>.
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
		 * Tests whether the checksum is calculated over the resolved
		 * path.
		 *
		 * On plain files the checksum is always calculated over the
		 * content of the file. But if you want to calculate the
		 * checksum over a symlink, you have two possibilities:
		 *
		 * -# Calculate the checksum over the content of the referenced
		 *    file.
		 * -# Calculate the checksum over the resolved path.
		 *
		 * The first case makes sure, that the content you reference
		 * with a symlink is not changed. The second case tells you,
		 * whether the link-destination has changed.
		 *
		 * By default the first case is activated and the checksum is
		 * calculated over the content of the referenced file.
		 *
		 * @return true is returned, if the checksum should be
		 *         calculated over the link-path (if you specify a
		 *         symlink). false is returned, if the checksum is
		 *         calculated over the content of the referenced file.
		 * @note This option only affects symlinks. If you specify a
		 *       plain file, the checksum is always calculated over the
		 *       content of the file.
		 */
		bool calcLink(void) const;

		/**
		 * Specifies the symlink calculation mode.
		 *
		 * For a detailed explanation visit the calcLink() method.
		 *
		 * @param enable Set to true, if yoou want to calculate the
		 *               checksum over the resolved path.
		 * @see calcLink()
		 */
		void setCalcLink(bool);

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
		 * @return On success, 0 is returned. On error an errno is
		 *         returned.
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
		/**
		 * Path of file to be checksumed
		 */
		wxString	path_;

		/**
		 * Link-calculation-flag.
		 * Specifies, if the link-destination should be checksumed
		 */
		bool		calcLink_;

		/**
		 * Result of calculation.
		 */
		int		result_;

		/**
		 * Calculated checksum
		 */
		u_int8_t	cs_[ANOUBIS_CS_LEN];

		/**
		 * Resets the task.
		 * Removes the calculation-result from the task, so it can be
		 * reused.
		 */
		void reset();

		/**
		 * Calculates the checksum over the content of the file.
		 *
		 * The calculation-method depends on the existance of
		 * /dev/anoubis. If the device exists, the method asks the
		 * kernel of the checksum, otherwise the calculation is
		 * performed in userspace.
		 *
		 * @param path Path of file to be checksumed
		 * @param csum Buffer, where the calculated checksum is written
		 *             to. Needs to be large enough, at least
		 *             ANOUBIS_CS_LEN.
		 * @param csum_len Holds the initial length of csum. If the
		 *                 calculation was performed, the number of
		 *                 bytes written to csum is written into the
		 *                 variable.
		 * @return 0 on success, otherwise a negative unix error code
		 *         is returned.
		 */
		static int calculateCsum(const char *, u_int8_t *, int *);
};

#endif	/* _CSUMCALCTASK_H_ */
