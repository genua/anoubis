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
#include <time.h>

#include "NotifyAnswer.h"

NotifyAnswer::NotifyAnswer(enum notifyAnswerType type, bool allow)
{
	type_ = type;
	wasAllowed_ = allow;
	openEditor_ = false;
	flags_ = 0;
	prefix_ = wxEmptyString;
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
	switch (type_) {
	case NOTIFY_ANSWER_ONCE:
		return getAnswerOnce();
	case NOTIFY_ANSWER_PROCEND:
		return getAnswerProcEnd();
	case NOTIFY_ANSWER_TIME:
		return getAnswerTime();
	case NOTIFY_ANSWER_FOREVER:
		return getAnswerForever();
	case NOTIFY_ANSWER_NONE:
		return getAnswerNone();
	default:
		return getAnswerDefault();
	}

	return (wxEmptyString); /* Should never be reached */
}

bool
NotifyAnswer::wasAllowed(void)
{
	return (wasAllowed_);
}

bool
NotifyAnswer::causeTmpRule(void)
{
	bool result;

	if ((type_ == NOTIFY_ANSWER_PROCEND) ||
	    (type_ == NOTIFY_ANSWER_TIME)) {
		result = true;
	} else {
		result = false;
	}

	return (result);
}

bool
NotifyAnswer::causePermRule(void)
{
	return (type_ == NOTIFY_ANSWER_FOREVER);
}

time_t
NotifyAnswer::getTime(void)
{
	time_t	now;
	long	factor;
	time_t	result;

	now = time(NULL); /* number of seconds since ... 1970 */
	factor = 1;
	result = 0;

	switch (timeUnit_) {
	case TIMEUNIT_DAY:
		factor *= 24;
	case TIMEUNIT_HOUR:
		factor *= 60;
	case TIMEUNIT_MINUTE:
		factor *= 60;
	case TIMEUNIT_SECOND:
		result = timeValue_ * factor;
	}

	return (now + result);
}

enum notifyAnswerType
NotifyAnswer::getType(void)
{
	return (type_);
}

void
NotifyAnswer::setEditor(bool value)
{
	openEditor_ = value;
}

bool
NotifyAnswer::getEditor(void)
{
	return openEditor_;
}

void
NotifyAnswer::setFlags(unsigned long value)
{
	flags_ = value;
}

unsigned long
NotifyAnswer::getFlags(void)
{
	return flags_;
}

void
NotifyAnswer::setPrefix(wxString value)
{
	prefix_ = value;
}

wxString
NotifyAnswer::getPrefix(void)
{
	return prefix_;
}

inline wxString
NotifyAnswer::getAnswerOnce(void) const
{
	if (wasAllowed_)
		return (_("this message was allowed."));
	else
		return (_("this message was forbidden."));
}

inline wxString
NotifyAnswer::getAnswerProcEnd(void) const
{
	if (wasAllowed_)
		return _("this message was allowed till process ends.");
	else
		return _("this message was forbidden till process ends.");
}

inline wxString
NotifyAnswer::getAnswerTime(void) const
{
	if (wasAllowed_) {
		switch (timeUnit_) {
		case TIMEUNIT_SECOND:
			return wxString::Format(wxPLURAL(
			    "this message was allowed for %d second.",
			    "this message was allowed for %d seconds.",
			    timeValue_), timeValue_);
		case TIMEUNIT_MINUTE:
			return wxString::Format(wxPLURAL(
			    "this message was allowed for %d minute.",
			    "this message was allowed for %d minutes.",
			    timeValue_), timeValue_);
		case TIMEUNIT_HOUR:
			return wxString::Format(wxPLURAL(
			    "this message was allowed for %d hour.",
			    "this message was allowed for %d hours.",
			    timeValue_), timeValue_);
		case TIMEUNIT_DAY:
			return wxString::Format(wxPLURAL(
			    "this message was allowed for %d day.",
			    "this message was allowed for %d days.",
			    timeValue_), timeValue_);
		}
	} else {
		switch (timeUnit_) {
		case TIMEUNIT_SECOND:
			return wxString::Format(wxPLURAL(
			    "this message was forbidden for %d second.",
			    "this message was forbidden for %d seconds.",
			    timeValue_), timeValue_);
		case TIMEUNIT_MINUTE:
			return wxString::Format(wxPLURAL(
			    "this message was forbidden for %d minute.",
			    "this message was forbidden for %d minutes.",
			    timeValue_), timeValue_);
		case TIMEUNIT_HOUR:
			return wxString::Format(wxPLURAL(
			    "this message was forbidden for %d hour.",
			    "this message was forbidden for %d hours.",
			    timeValue_), timeValue_);
		case TIMEUNIT_DAY:
			return wxString::Format(wxPLURAL(
			    "this message was forbidden for %d day.",
			    "this message was forbidden for %d days.",
			    timeValue_), timeValue_);
		}
	}

	return (wxEmptyString); /* Should never be reached */
}

inline wxString
NotifyAnswer::getAnswerForever(void) const
{
	if (wasAllowed_)
		return _("this message was allowed.");
	else
		return _("this message was forbidden.");
}

inline wxString
NotifyAnswer::getAnswerNone(void) const
{
	if (wasAllowed_)
		return _("this message was allowed forever.");
	else
		return _("this message was forbidden forever.");
}

inline wxString
NotifyAnswer::getAnswerDefault(void) const
{
	if (wasAllowed_)
		return _("this message was allowed forever.");
	else
		return _("this message was forbidden forever.");
}
