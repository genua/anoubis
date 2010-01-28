/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#ifndef _PROFILE_H_
#define _PROFILE_H_

#include "AnListClass.h"

/**
 * This is the base class for proflies. It carries the name and the type
 * of the Profile and their getter methods.
 */
class Profile : public AnListClass {
	public:
		/**
		 * Different types of profiles.
		 */
		enum ProfileSpec
		{
			NO_PROFILE = 0,		/*!< No valid profile */
			DEFAULT_PROFILE,	/*!< A default profile. Visible
						  for every user, but cannot
						  be overwritten. */
			USER_PROFILE		/*!< A user-specific profile.
						  Visible only for the user
						  and can be overwritten. */
		};

		/**
		 * Constructor of a profile
		 * @param[in] 1st The name of the profile
		 * @param[in] 2nd The type of the profile
		 */
		Profile(const wxString &, ProfileSpec);

		/**
		 * D'tor of a profile
		 */
		~Profile(void);

		/**
		 * Get the name of the proflie.
		 * @return name of the profile.
		 */
		wxString	getProfileName(void) const;

		/**
		 * Get the type of the proflie.
		 * @return type of the profile.
		 */
		ProfileSpec	getProfileType(void) const;

	private:
		wxString	name_;	/**< Name of the profile. */
		ProfileSpec	type_;  /**< Type of the profile. */
};

#endif	/* _PROFILE_H_ */
