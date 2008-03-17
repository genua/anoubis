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

#include <anoubis_msg.h>
#include <wx/string.h>

#include "Notification.h"
#include "NotifyAnswer.h"
#include "EscalationNotify.h"

EscalationNotify::EscalationNotify(struct anoubis_msg *msg) : Notification(msg)
{
	answer_ = NULL;
}

EscalationNotify::~EscalationNotify(void)
{
	if (isAnswered()) {
		delete answer_;
	}
}

void
EscalationNotify::assembleLogMessage(void)
{
	if (!logMessage_.IsEmpty()) {
		return;
	}

	if (notify_ == NULL) {
		logMessage_ = wxT("No information available.");
		return;
	}

	/* XXX: create log message from notify -- ch */
	logMessage_ = wxT("No information extracted yet.");

}

bool
EscalationNotify::isAnswered(void)
{
	if (answer_ == NULL) {
		return (false);
	} else {
		return (true);
	}
}

void
EscalationNotify::answer(NotifyAnswer *answer)
{
	answer_ = answer;
}

NotifyAnswer *
EscalationNotify::getAnswer(void)
{
	return (answer_);
}

wxString
EscalationNotify::getLogMessage(void)
{
	assembleLogMessage();
	return (logMessage_);
}
