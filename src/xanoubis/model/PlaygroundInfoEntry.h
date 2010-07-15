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

#ifndef _PLAYGROUNDINFOENTRY_H_
#define _PLAYGROUNDINFOENTRY_H_

#include <unistd.h>
#include <wx/datetime.h>
#include <wx/string.h>

#include "AnListClass.h"
#include "anoubis_protocol.h"
#include "config.h"


/**
 * A single Playground (info) entry.
 *
 * Represents one existing playground.
 */
class PlaygroundInfoEntry : public AnListClass
{
	public:
		/**
		 * c'tor with initial values
		 * @param pgid      playground id
		 * @param uid       user id
		 * @param starttime playground creation time
		 * @param isactive  active flag
		 * @param numfiles  number of files in playground
		 * @param path      file/command from playground creation
		 */
		PlaygroundInfoEntry(uint64_t pgid, uid_t uid,
		    wxDateTime starttime, bool isactive,
		    unsigned int numfiles, wxString path);

		/**
		 * Returns the ID of the playground.
		 * @return Playground ID
		 */
		uint64_t getPgid(void) const;

		/**
		 * Returns the ID of the user that created the playground.
		 * @return User ID
		 */
		uid_t getUid(void) const;

		/**
		 * Returns the creation time of the playground (seconds since
		 * epoch)
		 * @return Playground creation time
		 */
		wxDateTime getStarttime(void) const;

		/**
		 * Returns true if the playground is active and has processes.
		 * @return true if playground is active, otherwise false
		 */
		bool isActive(void) const;

		/**
		 * Returns the number of files in this playground.
		 * @return number of files
		 */
		unsigned int getNumFiles(void) const;

		/**
		 * Returns the command that was initially run in this PG.
		 * @return initial playground command
		 */
		 wxString getPath(void ) const;

	private:
		uint64_t      pgid_;       /**< Playground ID  */
		uid_t         uid_;        /**< User ID */
		wxDateTime    starttime_;  /**< creation time of playground */
		bool          isactive_;   /**< true if playground is active */
		unsigned int  numfiles_;   /**< number of files in PG */
		wxString      path_;       /**< command that created PG */

};

#endif	/* _PLAYGROUNDINFOENTRY_H_ */
