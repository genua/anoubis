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

#ifndef _PLAYGROUNDFILENOTIFY_H_
#define _PLAYGROUNDFILENOTIFY_H_

#include <anoubis_msg.h>
#include <wx/string.h>
#include <typeinfo>

#include "Notification.h"

/**
 * A special notification message for type ANOUBIS_N_LOGNOTIFY and subsystem
 * ANOUBIS_SOURCE_PLAYGROUNDFILE. This notification is used during the
 * playground file commit process.
 */
class PlaygroundFileNotify : public Notification {
	private:
		struct pg_file_message *filemsg_;  /**< kernel event part */
		wxString path_;              /**< the expanded path value */

	public:
		/**
		 * Constructor, create a new notification instance from a
		 * notification message received from the daemon.
		 * @param msg  The anoubis notification message that this
		 *             instance represents
		 */
		PlaygroundFileNotify(struct anoubis_msg *);

		/**
		 * Destructor.
		 */
		~PlaygroundFileNotify(void);

		/**
		 * Get the log message for this event class.
		 * @return The log message as wxstring.
		 */
		wxString    getLogMessage(void);

		/**
		 * Get the path from the notification. This is the
		 * userfriendly version with the device prefix and
		 * no playground references.
		 * @return the path as wxstring.
		 */
		wxString    getPath(void);

		/**
		 * Get the Playground ID from the notification.
		 * @return The ID of the playground.
		 */
		uint64_t     getPgId(void);

		/**
		 * Get the device from the notification.
		 * @return The device id.
		 */
		uint64_t     getDevice(void);

		/**
		 * Get the inode from the notification.
		 * @return The inode.
		 */
		uint64_t     getInode(void);

		/**
		 * Get the (raw) path from the notification. This is the
		 * original path value from the kernel message for internal
		 * use. It includes playground tags and is missing a device
		 * prefix string.
		 * @return The file path.
		 */
		const char*  getRawPath(void);
};

#endif	/* _PLAYGROUNDFILENOTIFY_H_ */
