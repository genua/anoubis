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

#include <config.h>

#include <sys/types.h>

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include <wx/datetime.h>
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
		 * List of checksum types managed by the SfsEntry.
		 */
		enum ChecksumType
		{
			SFSENTRY_CHECKSUM = 0,	/*!< Denotes a plain
						     checksum. */
			SFSENTRY_SIGNATURE /*!< Denotes a signed checksum */
		};

		/**
		 * List of possible checksum-states.
		 */
		enum ChecksumState
		{
			SFSENTRY_NOT_VALIDATED = 0,	/*!< The checksum is
							     not validated. */
			SFSENTRY_MISSING,	/*!< The entry has no
						     checksum. */
			SFSENTRY_INVALID,	/*!< Invalid checksum or
						     dependency-problem
						     (checksums not
						     compared).*/
			SFSENTRY_NOMATCH,	/*!< The checksum does not
						     match */
			SFSENTRY_MATCH		/*!< The checksum matches */
		};

		/**
		 * Returns the path of the file.
		 *
		 * This includes the complete path and the filename.
		 */
		wxString getPath() const;

		/**
		 * Returns the path of the SfsEntry relative to the specified
		 * base-path.
		 *
		 * Appending the returned path to the base-path again will
		 * result into the absolute path speciefied with setPath().
		 *
		 * @param basePath The base-path. Removed from the absolute
		 *        path.
		 * @return The path relative to the absolute path specified
		 *         with setPath().
		 * @see setPath()
		 */
		wxString getRelativePath(const wxString &) const;

		/**
		 * Returns the filename.
		 *
		 * This is the filename without the path.
		 *
		 * @return Filename <i>without</i> path.
		 */
		wxString getFileName(void) const;

		/**
		 * Updates the path of the SfsEntry.
		 *
		 * @param path Complete path of entry.
		 */
		void setPath(const wxString &);

		/**
		 * Tests whether the file exists in the local filesystem.
		 *
		 * @return true if such a file exists, false otherwise.
		 */
		bool fileExists(void) const;

		/**
		 * Tests whether the file exists and is a symbolic link.
		 *
		 * @return true if the file is a symblic link.
		 */
		bool isSymlink(void) const;

		/**
		 * Resolves the symlink.
		 *
		 * @return The resolved path of the link-destination is
		 *         returned, if the SfsEntry represents a symlink. An
		 *         empty string is returned, if the entry is a plain
		 *         file or resolving failed.
		 */
		wxString resolve(void) const;

		/**
		 * Tests whether the file can have a checksum.
		 *
		 * A checksum over a file can only be calculated, if the file
		 * is a symbolic link or a regular file. If the
		 * resolve-argument is set to true, the symlink is resolved and
		 * the reference needs to be a regular file.
		 *
		 * @param resolve If the file is a symlink and the argument is
		 *                set to true, the symlink is resolved. Then,
		 *                the destination needs to be regular file.
		 * @return true if a checksum can be calculated for the file,
		 *         false otherwise.
		 */
		bool canHaveChecksum(bool) const;

		/**
		 * Returns the modification timestamp of the file.
		 *
		 * If the file does not exist, ther returned datetime-value
		 * is unset.
		 *
		 * @return Last modification timestamp of the file
		 */
		wxDateTime getLastModified(void) const;

		/**
		 * Returns the state of the specified checksum-type.
		 *
		 * The state can change depending on the checksums you assigned
		 * to the SfsEntry.
		 *
		 * @param type Type of requested checksum
		 * @return State of requested checksum-type
		 */
		ChecksumState getChecksumState(ChecksumType) const;

		/**
		 * Tests whether the checksum of the specified type is
		 * assigned.
		 *
		 * @param type Type of requested checksum
		 * @return true is returned, if a checksum of the requested
		 *         type is assigned, false otherwise.
		 */
		bool haveChecksum(ChecksumType) const;

		/**
		 * Tests if at least one checksum (of any type) is assigned.
		 *
		 * @return true is returned, if at least one checksum is
		 *         assigned, false otherwise.
		 * @see haveChecksum(ChecksumType)
		 */
		bool haveChecksum(void) const;

		/**
		 * Tests whether the checksum of the specified type differs
		 * from the local calculated checksum.
		 *
		 * For comparison you need:
		 * - a local checksum
		 * - the checksum of the specified type
		 *
		 * If one of the conditions fails, comparison is not performed
		 * and false is retured.
		 *
		 * @param type Type of requested checksum
		 * @return true is returned, if the requested checksum differs
		 *         from the local checksum, false otherwise.
		 */
		bool isChecksumChanged(ChecksumType) const;

		/**
		 * Tests if at least one checksum (of any type) has changed.
		 *
		 * @return true is returned, if at least one checksum has
		 *         changed, false otherwise.
		 * @see isChecksumChanged(ChecksumType)
		 */
		bool isChecksumChanged() const;

		/**
		 * Returns the length of a checksum.
		 *
		 * A plain checksum has a length of ANOUBIS_CS_LEN, but the
		 * length of a signature is not constant.
		 *
		 * @param type Type of requested checksum
		 * @return Length of checksum. 0 is returned, if the requested
		 *         checksum is not assigned.
		 */
		size_t getChecksumLength(ChecksumType) const;

		/**
		 * Returns the checksum of the specified type.
		 *
		 * @param type Type of requested checksum
		 * @param csum Destination buffer. Checksum is written into the
		 *             buffer. You need at least ANOUBIS_CS_LEN bytes.
		 * @param size Size of csum-argument.
		 * @return Number of bytes written. If the requested checksum
		 *         is not assigned or the destination buffer is not
		 *         large enough to hold the complete checksum, 0 is
		 *         returned and the destination buffer is not used.
		 */
		size_t getChecksum(ChecksumType, u_int8_t *, size_t) const;

		/**
		 * Returns the hex-string of a checksum.
		 *
		 * If the requested checksum is not assigned to the entry, an
		 * empty string is returned.
		 *
		 * @param type Type of requested checksum
		 * @return An hex-string representing the requested checksum.
		 */
		wxString getChecksum(ChecksumType) const;

		/**
		 * Assings a checksum to the SfSEntry.
		 *
		 * The assigned checksum is compared with the local checksum
		 * (if any). Depending on the compare-result, the
		 * checksum-state can change.
		 *
		 * @param type Type of requested checksum
		 * @param cs The checksum to be assigned
		 * @return true is returned, if the checksum-state has changed.
		 *         The return-code can be used to track changes of the
		 *         model.
		 */
		bool setChecksum(ChecksumType, const u_int8_t *, size_t);

		/**
		 * Removes an assigned checksum from the SfsEntry.
		 *
		 * The checksum-state is updated to SfsEntry::SFSENTRY_MISSING.
		 *
		 * @param type Type of requested checksum
		 * @return true is returned, if the checksum-state has changed.
		 *         The return-code can be used to track changes of the
		 *         model.
		 */
		bool setChecksumMissing(ChecksumType);

		/**
		 * Invalidates an assigned checksum.
		 *
		 * The checksum is removed and the checksum-state is updated to
		 * SfsEntry::SFSENTRY_INVALID.
		 *
		 * @param type Type of requested checksum
		 * @return true is returned, if the checksum-state has changed.
		 *         The return-code can be used to track changes of the
		 *         model.
		 */
		bool setChecksumInvalid(ChecksumType);

		/**
		 * Tests whether a local checksum is assigned to the SfsEntry.
		 *
		 * @return true is returned, if a local checksum is assigned,
		 *         false otherwise.
		 */
		bool haveLocalCsum(void) const;

		/**
		 * Returns the hex-string of the local checksum.
		 *
		 * If no checksum is assigned to the entry, an empty string is
		 * returned.
		 *
		 * @return An hex-string representing the local checksum.
		 */
		wxString getLocalCsum(void) const;

		/**
		 * Copies the locally calculated checksum into the SfsEntry.
		 *
		 * The assigned checksum is compared with the other checkums
		 * (if any). Depending on the compare-result, the
		 * (checksum-states can change.
		 *
		 * @param cs The local calculated checksum.
		 * @return true is returned, if at least a checksum-state has
		 *         changed. The return-code can be used to track
		 *         changes of the model.
		 * @see setDaemonCsum()
		 * @see getChecksumAttr()
		 */
		bool setLocalCsum(const u_int8_t *);

		/**
		 * Resets the SfsEntry.
		 *
		 * Checksums are cleared, attributes are resetted.
		 *
		 * @return true is returned, if at least a checksum-state has
		 *         changed. The return-code can be used to track
		 *         changes of the model.
		 */
		bool reset(void);

		/**
		 * Resets the specified checksum.
		 *
		 * The assigned checksum (if any) is removed and the
		 * checksum-state is updated to
		 * SfsEntry::SFSENTRY_NOT_VALIDATED.
		 *
		 * @param type Type of requested checksum
		 * @return true is returned, if the checksum-state has changed.
		 *         The return-code can be used to track changes of the
		 *         model.
		 */
		bool reset(ChecksumType);

	private:
		wxString	path_;
		wxString	filename_;
		bool		haveLocalCsum_;
		u_int8_t	localCsum_[ANOUBIS_CS_LEN];
		size_t		csumLen_[2];
		u_int8_t	*csum_[2];
		ChecksumState	state_[2];

		/**
		 * Default-c'tor.
		 *
		 * Creates an empty, resetted SfsEntry.
		 */
		SfsEntry(void);

		/**
		 * Creates an SfsEntry with the specified path.
		 *
		 * @param path Path of SfsEntry
		 * @see setPath()
		 */
		SfsEntry(const wxString &);

		/**
		 * Copy-c'tor.
		 */
		SfsEntry(const SfsEntry &) {}

		/**
		 * D'tor.
		 */
		~SfsEntry(void);

		void copyChecksum(ChecksumType, const u_int8_t *, size_t);
		void releaseChecksum(ChecksumType);
		bool validateChecksum(ChecksumType);
		static wxString cs2str(const u_int8_t *, size_t);

	friend class SfsDirectory;
};

#endif	/* _SFSENTRY_H_ */
