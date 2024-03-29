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

#ifndef _POLICYUTILS_H_
#define _POLICYUTILS_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/stat.h>
#include <wx/arrstr.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/string.h>

#ifndef LINUX
#include <sha2.h>
#endif
#include <openssl/sha.h>

#include "libapn.h"
#include "AppPolicy.h"

/**
 * This abstract class collects utilities for policies.
 */
class PolicyUtils
{
	public:
		/**
		 * Convert checksum to string.
		 * @param[in] 1st Checksum to convert.
		 * @param[in] 2nd Size of checksum.
		 * @param[out] 3rd Converted string.
		 * @return True on success.
		 */
		static bool csumToString(unsigned char *, size_t, wxString &);

		/**
		 * Add an application to a list of apn_app structures.
		 * @param[in] 1st The address of the location where
		 *     the start pointer of the list is stored.
		 * @param[in] 2nd A zero based index into the list. The
		 *     new application is added before the application given
		 *     by this index.
		 * @param[in] 3nd The name of the application.
		 * @return True on success.
		 */
		static bool addToAppListAt(struct apn_app **, unsigned int,
		    wxString);

		/**
		 * Remove an application froma  lit of apn_app structures.
		 * @param[in] 1st The address of the location where
		 *     the start point of the is stored.
		 * @param[in] 2nd A zero basedindex into the list. The
		 *     application given by this index is removed.
		 * @return True on success.
		 */
		static bool removeFromAppListAt(struct apn_app **,
		    unsigned int);

		/**
		 * Get the list of apn_app's.
		 * @param[in] 1st The linked list of app's.
		 * @return The list of binaries.
		 */
		static wxArrayString getAppList(struct apn_app *);

		/**
		 * Convert a list to a (comma seperated) string.
		 * @param[in] 1st The list of strings.
		 * @return The single string with all list elements.
		 */
		static wxString listToString(wxArrayString);

		/**
		 * Convert a (comma seperated) string to a list.
		 * @param[in] 1st The string to convert.
		 * @return The list with the elements of the string.
		 */
		static wxArrayString stringToList(wxString);

		/**
		 * Get readable subject type.
		 * @param[in] 1st A pointer to the subject.
		 * @return The type of subject.
		 */
		static wxString getSubjectName(const struct apn_subject *);

		/**
		 * Clean a subject data structure and set its type to
		 * APN_CS_NONE.
		 * @param[in] 1st A pointer to the subject.
		 * @return None.
		 */
		static void cleanSubject(struct apn_subject *);
};

#endif	/* _POLICYUTILS_H_ */
