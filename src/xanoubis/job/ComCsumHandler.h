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

#ifndef _COMCSUMHANDLER_H_
#define _COMCSUMHANDLER_H_

#include <wx/string.h>

/**
 * A checksum-handler provides an interface, which is used by all
 * checksum-related tasks.
 *
 * A checksum-related communication-tasks are derivated from the class, and
 * thus inherit the methods. In general, there are used by all checksum-tasks.
 * So it makes sense the generalise and centralise the functionality.
 */
class ComCsumHandler
{
	public:
		/**
		 * Returns the path to the source-file, where the operation is
		 * working on.
		 *
		 * The task is using the file to perform an operation on it.
		 * The operation highly depends on the task!
		 *
		 * @return Path of file, where the operation is working on.
		 */
		wxString getPath(void) const;

		/**
		 * Updates the path of the source-file.
		 *
		 * Use this method to give the task a file. Then, the task is
		 * using this file to perform the task-operation.
		 *
		 * @param path The new path
		 */
		void setPath(const wxString &);

		/**
		 * Tests whether a key-id is assigned.
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
		 * @see LocalCertificate
		 */
		bool setKeyId(const u_int8_t *, int);

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

	protected:
		ComCsumHandler(void);
		~ComCsumHandler(void);

		/**
		 * Returns the assigned key-id.
		 *
		 * The array has the size getKeyIdLen().
		 *
		 * @return The assigned key-id. If no key-id is assigned, 0 is
		 *         returned.
		 */
		u_int8_t *getKeyId(void) const;

		/**
		 * Returns the length of the assigned key-id.
		 *
		 * This is the length of the array returned by getKeyId().
		 *
		 * @return Length of key-id returned by getKeyId(). If no
		 *         key-id is assigned, 0 is returned.
		 */
		int getKeyIdLen(void);

		/**
		 * Calculates the checksum over the assigned file.
		 *
		 * The way of checksum-calculation depends many conditions,
		 * that's why it makes sence to provide an own method.
		 *
		 * Depending on:
		 * - the file-type (plain file, symlink)
		 * - the calcLink() flag
		 *
		 * a checksum is either calculated over the content of the file
		 * or the name of the link-destination.
		 *
		 * If calkLink() is enabled and a symlink is assigned
		 * (getPath()), the symlink is resolved and the checksum is
		 * calculated over the resulting path. In any other cases the
		 * checksum is calculated over the content of the (referenced)
		 * file.
		 *
		 * @param cs Destination, where the calculated checksum is
		 *           written to. The array needs be to large enough to
		 *           hold the complete checksum (at least
		 *           ANOUBIS_CS_LEN bytes).
		 * @param cslen Length of destination buffer.
		 * @return On success 0 is returned. On error, an standard unix
		 *         error-code is returned.
		 */
		int csumCalc(u_int8_t *, int *) const;

		/**
		 * Resolves the path to be send to anoubisd.
		 *
		 * Path to be send to anoubisd depends on filetype (symlink)
		 * and calcLink()-flag. If the flag is disabled and the
		 * requested file is a symlink, then send resolved path to
		 * anoubisd.
		 *
		 * @param resolved_path Destination, where there the resolved
		 *                      path is written to. The string needs to
		 *                      have a size of PATH_MAX.
		 * @return true on success, false if the resolve-operation
		 *         failed.
		 */
		bool resolvePath(char *) const;

	private:
		wxString	path_;
		u_int8_t	*keyId_;
		int		keyIdLen_;
		bool		calcLink_;
};

#endif	/* _COMCSUMHANDLER_H_ */
