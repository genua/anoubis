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

#include <syslog.h>

#include "AlertNotify.h"
#include "Debug.h"
#include "LogNotify.h"
#include "NotificationCtrl.h"

#include <wx/string.h>

Debug::Level Debug::level_;

void
Debug::initialize(void)
{
	level_ = FATAL;
	openlog("xanoubis", LOG_PID, LOG_USER);
}

void
Debug::uninitialize(void)
{
	closelog();
}

Debug::Level
Debug::getLevel(void)
{
	return (level_);
}

void
Debug::setLevel(Debug::Level level)
{
	level_ = level;
}

void
Debug::setLevel(long level)
{
	if (level < 0)
		setLevel(FATAL);
	else if (level > TRACE)
		setLevel(TRACE);
	else
		setLevel((Level)level);
}

bool
Debug::checkLevel(Level level)
{
	return (level_ >= level);
}

void
Debug::fatal(const wxChar *format, ...)
{
	va_list ap;

	va_start(ap, format);
	wxString message = wxString::FormatV(format, ap);
	va_end(ap);

	fprintf(stderr, "FATAL: %ls\n", message.c_str());
	syslog(LOG_CRIT, "%ls", message.c_str());

	exit(1);
}

void
Debug::err(const wxChar *format, ...)
{
	AlertNotify		*notify;
	NotificationCtrl	*notifyCtrl;
	va_list			 ap;

	va_start(ap, format);
	wxString message = wxString::FormatV(format, ap);
	va_end(ap);

	notify = new AlertNotify(message);

	notifyCtrl = NotificationCtrl::instance();
	notifyCtrl->addNotification(notify);

	syslog(LOG_ERR, "%ls", message.c_str());

	if (level_ >= ERR)
		fprintf(stderr, "ERR: %ls\n", message.c_str());
}

void
Debug::warn(const wxChar *format, ...)
{
	AlertNotify		*notify;
	NotificationCtrl	*notifyCtrl;
	va_list			 ap;

	va_start(ap, format);
	wxString message = wxString::FormatV(format, ap);
	va_end(ap);

	notify = new AlertNotify(message);

	notifyCtrl = NotificationCtrl::instance();
	notifyCtrl->addNotification(notify);

	syslog(LOG_WARNING, "%ls", message.c_str());

	if (level_ >= WARN)
		fprintf(stderr, "WARN: %ls\n", message.c_str());
}

void
Debug::info(const wxChar *format, ...)
{
	LogNotify		*notify;
	NotificationCtrl	*notifyCtrl;
	va_list			 ap;

	va_start(ap, format);
	wxString message = wxString::FormatV(format, ap);
	va_end(ap);

	notify = new LogNotify(message);

	notifyCtrl = NotificationCtrl::instance();
	notifyCtrl->addNotification(notify);

	if (level_ >= INFO)
		fprintf(stderr, "INFO: %ls\n", message.c_str());
}

void
Debug::chat(const wxChar *format, ...)
{
	if (level_ >= CHAT) {
		va_list ap;

		va_start(ap, format);
		wxString message = wxString::FormatV(format, ap);
		va_end(ap);

		fprintf(stderr, "CHAT: %ls\n", message.c_str());
	}
}

void
Debug::trace(const wxChar *format, ...)
{
	if (level_ >= TRACE) {
		va_list ap;

		va_start(ap, format);
		wxString message = wxString::FormatV(format, ap);
		va_end(ap);

		fprintf(stderr, "TRACE: %ls\n", message.c_str());
	}
}
