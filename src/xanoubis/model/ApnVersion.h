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

#ifndef _APNVERSION_H_
#define _APNVERSION_H_

#include "AnListClass.h"

#include <wx/datetime.h>

#include <apnvm.h>

/**
 * Detail information about a single version.
 *
 * VersionCtrl::getVersion() is used to receive some detail information
 * about a specific version. This class holds all the information.
 */
class ApnVersion : public AnListClass
{
	public:
		ApnVersion(const ApnVersion &);

		/**
		 * Returns the version-number of the version.
		 */
		int getVersionNo(void) const;

		/**
		 * Returns the date/time, when the version was created.
		 */
		wxDateTime getTimestamp(void) const;

		/**
		 * Returns the name of the user which belongs to the version.
		 */
		wxString getUsername(void) const;

		/**
		 * Returns the comment of the version.
		 */
		wxString getComment(void) const;

		/**
		 * Checks weather the version was created automatically by
		 * the system or manually by the user.
		 */
		bool isAutoStore(void) const;

	private:
		ApnVersion(void) {}
		ApnVersion(struct apnvm_version *, const char *);

		int		no_;
		wxDateTime	tstamp_;
		wxString	user_;
		wxString	comment_;
		bool		auto_store_;

	friend class VersionCtrl;
};

#endif	/* _APNVERSION_H_ */
