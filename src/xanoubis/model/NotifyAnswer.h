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

#ifndef _NOTIFYANSWER_H_
#define _NOTIFYANSWER_H_

#include <wx/string.h>

enum notifyAnswerType {
	NOTIFY_ANSWER_NONE = 0,
	NOTIFY_ANSWER_ONCE,
	NOTIFY_ANSWER_PROCEND,
	NOTIFY_ANSWER_TIME,
	NOTIFY_ANSWER_FOREVER
};

enum timeUnit {
	TIMEUNIT_SECOND = 0,
	TIMEUNIT_MINUTE,
	TIMEUNIT_HOUR,
	TIMEUNIT_DAY
};

class NotifyAnswer {
	private:
		enum notifyAnswerType	type_;
		bool			wasAllowed_;
		int			timeValue_;
		enum timeUnit		timeUnit_;
		unsigned long		flags_;
		wxString		prefix_;
		bool			openEditor_;

	public:
		NotifyAnswer(enum notifyAnswerType, bool);
		NotifyAnswer(enum notifyAnswerType, bool, int, enum timeUnit);

		wxString		getAnswer(void);
		bool			wasAllowed(void);
		bool			causeTmpRule(void);
		bool			causePermRule(void);
		time_t			getTime(void);
		enum notifyAnswerType	getType(void);
		wxString		getPrefix(void);
		void			setPrefix(wxString);
		unsigned long		getFlags(void);
		void			setFlags(unsigned long);
		void			setEditor(bool);
		bool			getEditor(void);
};

#endif	/* _NOTIFYANSWER_H_ */
