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

#include <wx/string.h>

#include "NotifyAnswer.h"

NotifyAnswer::NotifyAnswer(enum notifyAnswerType type, bool allow)
{
	type_ = type;
	wasAllowed_ = allow;
}

NotifyAnswer::NotifyAnswer(enum notifyAnswerType type, bool allow, int value,
    enum timeUnit unit)
{
	type_ = type;
	wasAllowed_ = allow;
	timeValue_ = value;
	timeUnit_ = unit;
}

wxString
NotifyAnswer::getAnswer(void)
{
	wxString s = wxT("Diese Nachricht wurde ");;

	switch (type_) {
	case NOTIFY_ANSWER_ONCE:
		break;
	case NOTIFY_ANSWER_PROCEND:
		s += wxT("bis Prozess Ende ");
		break;
	case NOTIFY_ANSWER_TIME:
		s += wxString::Format(wxT("fuer %d "), timeValue_);
		switch (timeUnit_) {
		case TIMEUNIT_SECOND:
			if (timeValue_ < 2)
				s += wxT("Sekunde ");
			else
				s += wxT("Sekunden ");
			break;
		case TIMEUNIT_MINUTE:
			if (timeValue_ < 2)
				s += wxT("Minute ");
			else
				s += wxT("Minuten ");
			break;
		case TIMEUNIT_HOUR:
			if (timeValue_ < 2)
				s += wxT("Stunde ");
			else
				s += wxT("Stunden ");
			break;
		case TIMEUNIT_DAY:
			if (timeValue_ < 2)
				s += wxT("Tag ");
			else
				s += wxT("Tage ");
			break;
		}
		break;
	case NOTIFY_ANSWER_FOREVER:
		break;
	case NOTIFY_ANSWER_NONE:
		/* FALLTHROUGH*/
	default:
		s += wxT("fuer immer ");
		break;
	}

	if (wasAllowed_) {
		s += wxT("erlaubt.");
	} else {
		s += wxT("verboten.");
	}

	return (s);
}

bool
NotifyAnswer::wasAllowed(void)
{
	return (wasAllowed_);
}
