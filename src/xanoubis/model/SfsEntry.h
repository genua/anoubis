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

#ifndef _SFSENTRY_H_
#define _SFSENTRY_H_

#include <wx/string.h>

/**
 * A single SFS-entry.
 *
 * Represents a file with some SFS-related attributes. A file can have an
 * assigned checksum and/or signature.
 */
class SfsEntry
{
	public:
		/**
		 * List of possible checksum-states.
		 */
		enum ChecksumAttr
		{
			/*!< The entry has no checksum. */
			SFSENTRY_CHECKSUM_UNKNOWN = 0,

			/*!< The checksum does not match */
			SFSENTRY_CHECKSUM_NOMATCH,

			/*!< The checksum matches */
			SFSENTRY_CHECKUM_MATCH
		};

		enum SignatureAttr
		{
			/*!< The entry has no signature */
			SFSENTRY_SIGNATURE_UNKNOWN = 0,

			/*!< The signature is invalid */
			SFSENTRY_SIGNATURE_INVALID,

			/*!< The signature/checksum does not match */
			SFSENTRY_SIGNATURE_NOMATCH,

			/*!< The signature/checksum matches */
			SFSENTRY_SIGNATURE_MATCH
		};

		SfsEntry(const SfsEntry &);

		/**
		 * Returns the path of the file.
		 *
		 * This includes the complete path and the filename.
		 */
		wxString getPath() const;

		/**
		 * Returns the checksum-attribute of the file.
		 */
		ChecksumAttr getChecksumAttr() const;

		/**
		 * Returns the signature-attribute of the file.
		 */
		SignatureAttr getSignatureAttr() const;

	private:
		wxString path_;
		ChecksumAttr checksumAttr_;
		SignatureAttr signatureAttr_;

		SfsEntry();
		SfsEntry(const wxString &, ChecksumAttr, SignatureAttr);
		SfsEntry(const wxString &);

		void setPath(const wxString &);
		void setChecksumAttr(ChecksumAttr);
		void setSignatureAttr(SignatureAttr);

	friend class SfsDirectory;
};

#endif	/* _SFSENTRY_H_ */
