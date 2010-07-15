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

#ifndef _PLAYGROUNDFILEENTRY_H_
#define _PLAYGROUNDFILEENTRY_H_

#include <unistd.h>
#include <wx/string.h>

#include "AnListClass.h"
#include "anoubis_protocol.h"
#include "config.h"


/**
 * A single file within a playground.
 */
class PlaygroundFileEntry : public AnListClass
{
	public:
		/**
		 * public c'tor.
		 * @param pgid  playground id
		 * @param path  full qualified filename
		 */
		PlaygroundFileEntry(uint64_t, wxString);

		/**
		 * Returns the ID of the playground that this file belongs to.
		 * @return playground id
		 */
		uint64_t getPgid(void) const;

		/**
		 * Returns the absolute path of the file.
		 * Note: backend will calculate absolute filename from
		 * relative path, device and inode.
		 * @return absolute path
		 */
		wxString getPath(void) const;

	private:
		uint64_t  pgid_;    /**< playground ID */
		wxString  path_;    /**< file path */
};

#endif	/* _PLAYGROUNDFILEENTRY_H_ */
