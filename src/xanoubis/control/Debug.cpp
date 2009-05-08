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

#include "Debug.h"

Debug::~Debug()
{
}

Debug*
Debug::instance(void)
{
	return Singleton<Debug>::instance();
}

void
Debug::log(wxString message, int level)
{
	if (level_ >= level)
		switch (level) {
		case DEBUG_ALERT:
			fprintf(stderr, "Alert\t%s\n",
			    (const char*)message.mb_str(wxConvUTF8));
			break;
		case DEBUG_LOG:
			fprintf(stderr, "Log\t%s\n",
			    (const char*)message.mb_str(wxConvUTF8));
			break;
		case DEBUG_STATUS:
			fprintf(stderr, "Status\t%s\n",
			    (const char*)message.mb_str(wxConvUTF8));
			break;
		case DEBUG_CHAT:
			fprintf(stderr, "Chat\t%s\n",
			    (const char*)message.mb_str(wxConvUTF8));
			break;
		default:
			break;
		}
}

void
Debug::setLevel(int level)
{
	level_ = level;
}

int
Debug::checkLevel(int level)
{
	return (level_ >= level);
}

Debug::Debug(void) : Singleton<Debug>()
{
}
