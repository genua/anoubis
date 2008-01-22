/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#include <anoubischat.h>
#include <errno.h>
#include <exception>
#include <string.h>
#include <wx/string.h>

#include "CommunicatorException.h"


CommunicatorException::CommunicatorException(void)
{
	reason_ = ACHAT_RC_ERROR;
}

CommunicatorException::CommunicatorException(enum achat_rc reason)
{
	reason_ = reason;
}

wxString *
CommunicatorException::explain(void)
{
	const wxChar *reason = NULL;
	wxString	*msg;

	switch(reason_) {
	case ACHAT_RC_OK:
		reason = _T("ACHAT_RC_OK - anything is ok");
		break;
	case ACHAT_RC_ERROR:
		reason = _T("ACHAT_RC_ERROR - a general error occured");
		break;
	case ACHAT_RC_NYI:
		reason = _T("ACHAT_RC_NYI - method is not implemented");
		break;
	case ACHAT_RC_INVALPARAM:
		reason = _T("ACHAT_RC_INVALPARAM - invalid parameter");
		break;
	case ACHAT_RC_WRONGSTATE:
		reason = _T("ACHAT_RC_WRONGSTATE - wrong state");
		break;
	case ACHAT_RC_OOMEM:
		reason = _T("ACHAT_RC_OOMEM - out of memory");
		break;
	default:
		reason = _T("Thrown with unknown reason!");
		break;
	}

	msg = new wxString(reason);
	*msg += wxT("[");
	*msg += (wxChar*)std::strerror(errno);
	*msg += wxT("]");

	return (msg);
}

achat_rc
 CommunicatorException::getReason(void)
{
	return (reason_);
}
