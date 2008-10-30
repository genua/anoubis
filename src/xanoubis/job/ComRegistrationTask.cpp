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

#include <config.h>

#include <sys/types.h>

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include <client/anoubis_client.h>
#include <client/anoubis_transaction.h>

#include "ComHandler.h"
#include "ComRegistrationTask.h"
#include "TaskEvent.h"

static long long
getToken(void)
{
	static long long token = 0;
	return (++token);
}

ComRegistrationTask::ComRegistrationTask(void)
{
	this->action_ = ACTION_REGISTER;
}

ComRegistrationTask::ComRegistrationTask(Action action)
{
	this->action_ = action;
}

ComRegistrationTask::Action
ComRegistrationTask::getAction(void) const
{
	return (this->action_);
}

void
ComRegistrationTask::setAction(Action action)
{
	this->action_ = action;
}

wxEventType
ComRegistrationTask::getEventType(void) const
{
	return (anTASKEVT_REGISTER);
}

void
ComRegistrationTask::exec(void)
{
	struct anoubis_client	*client = getComHandler()->getClient();
	RegState		state = STATE_DONE;

	resetComTaskResult();

	if (client == 0) {
		/*
		 * You don't have a connection
		 * Leave task-state on RESULT_INIT.
		 */
		return;
	}

	/* Start-state depends on action */
	switch (action_) {
	case ACTION_REGISTER:
		state = STATE_REGISTER;
		break;
	case ACTION_UNREGISTER:
		state = STATE_UNREGISTER;
		break;
	}

	while (state != STATE_DONE) {
		struct anoubis_transaction *ta = 0;

		/* Dummy initialization to keep compiler happy */
		RegState next = STATE_DONE;

		switch (state) {
		case STATE_REGISTER:
			ta = anoubis_client_register_start(client,
			    getToken(), geteuid(), 0, 0);
			next = STATE_STAT_REGISTER;
			break;
		case STATE_UNREGISTER:
			ta = anoubis_client_unregister_start(client,
			    getToken(), geteuid(), 0, 0);
			next = STATE_STAT_UNREGISTER;
			break;
		case STATE_STAT_REGISTER:
			ta = anoubis_client_register_start(client,
			    getToken(), 0, 0, ANOUBIS_SOURCE_STAT);
			next = STATE_DONE;
			break;
		case STATE_STAT_UNREGISTER:
			ta = anoubis_client_unregister_start(client,
			    getToken(), 0, 0, ANOUBIS_SOURCE_STAT);
			next = STATE_DONE;
			break;
		case STATE_DONE:
			break;
		}

		if (state != STATE_DONE) {
			if (ta == 0) {
				/* Failed to create transaction */
				setComTaskResult(RESULT_COM_ERROR);
				return;
			}

			/* Wait for completition */
			while (!(ta->flags & ANOUBIS_T_DONE)) {
				if (!getComHandler()->waitForMessage()) {
					anoubis_transaction_destroy(ta);
					setComTaskResult(RESULT_COM_ERROR);
					return;
				}
			}

			/* Done */
			anoubis_transaction_destroy(ta);
			state = next;
		}
	}

	setComTaskResult(RESULT_SUCCESS);
}
