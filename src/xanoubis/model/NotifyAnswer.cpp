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

#include <wx/intl.h>
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
	wxString s = _("this message was ");;

	switch (type_) {
	case NOTIFY_ANSWER_ONCE:
		break;
	case NOTIFY_ANSWER_PROCEND:
		s += wxT("till process ends ");
		break;
	case NOTIFY_ANSWER_TIME:
		s += wxString::Format(_("for %d "), timeValue_);
		switch (timeUnit_) {
		case TIMEUNIT_SECOND:
			if (timeValue_ < 2)
				s += _("second ");
			else
				s += _("seconds ");
			break;
		case TIMEUNIT_MINUTE:
			if (timeValue_ < 2)
				s += _("minute ");
			else
				s += _("minutes ");
			break;
		case TIMEUNIT_HOUR:
			if (timeValue_ < 2)
				s += wxT("hour ");
			else
				s += wxT("hours ");
			break;
		case TIMEUNIT_DAY:
			if (timeValue_ < 2)
				s += _("day ");
			else
				s += _("days ");
			break;
		}
		break;
	case NOTIFY_ANSWER_FOREVER:
		break;
	case NOTIFY_ANSWER_NONE:
		/* FALLTHROUGH*/
	default:
		s += _("forever ");
		break;
	}

	if (wasAllowed_) {
		s += _("allowed.");
	} else {
		s += _("forbidden.");
	}

	return (s);
}

bool
NotifyAnswer::wasAllowed(void)
{
	return (wasAllowed_);
}
