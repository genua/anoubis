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

#ifndef _COMTHREAD_H_
#define _COMTHREAD_H_

#include "ComHandler.h"
#include "JobThread.h"
#include "Notification.h"

/**
 * The thread, which is responsible for tasks of the Task::TYPE_COM.
 */
class ComThread : public JobThread, protected ComHandler
{
	public:
		ComThread(JobCtrl *, const wxString &);

		/**
		 * Connects to anoubisd.
		 * This method needs to be called before the thread is started!
		 *
		 * @return true on success, false otherwise.
		 */
		bool connect(void);

		/**
		 * Disconnects from anoubisd.
		 */
		void disconnect(void);

		/**
		 * Tests weather a connection to anoubisd is established.
		 * @return true if you have a connection to anoubisd, false
		 *         otherwise.
		 */
		bool isConnected(void) const;

		void *Entry(void);

	protected:
		/**
		 * Implementation of ComHandler::getClient().
		 */
		struct anoubis_client *getClient(void) const;

		/**
		 * Implementation of ComHandler::waitForMessage().
		 */
		bool waitForMessage(void);

	private:
		wxString		socketPath_;
		struct achat_channel	*channel_;
		struct anoubis_client	*client_;
		NotifyList		answerList_;

		bool checkNotify(struct anoubis_msg *);
		void sendEscalationAnswer(Notification *);
};

#endif	/* _COMTHREAD_H_ */
