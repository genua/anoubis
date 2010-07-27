/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#include "AnConfig.h"
#include "AnEvents.h"

AnConfig::AnConfig(const wxString &appName)
    : wxFileConfig(appName, wxEmptyString, wxEmptyString, wxEmptyString,
    wxCONFIG_USE_SUBDIR | wxCONFIG_USE_LOCAL_FILE)
{
	needFlush_ = false;
}

AnConfig::~AnConfig(void)
{
	Flush();
}

bool
AnConfig::Flush(bool bCurrentOnly)
{
	/* Flush only if you really have some changes */
	if (needFlush_) {
		bool result = wxFileConfig::Flush(bCurrentOnly);

		if (result) {
			/* Inform any listener about new configuration */
			wxCommandEvent event(anEVT_ANOUBISOPTIONS_UPDATE);
			wxPostEvent(AnEvents::instance(), event);
		}

		needFlush_ = false; /* Reset flush-flag */

		return (result);
	} else
		return (false);
}

bool
AnConfig::DoWriteString(const wxString &key, const wxString &value)
{
	wxString oldValue;
	bool readResult = DoReadString(key, &oldValue);

	if (readResult && oldValue == value) {
		/* No difference between old and new value, nothing to do */
		return (false);
	}

	bool writeResult = wxFileConfig::DoWriteString(key, value);

	if (writeResult) {
		needFlush_ = true;
		Flush();
	}

	return (writeResult);
}

bool
AnConfig::DoWriteLong(const wxString &key, long value)
{
	return (DoWriteString(key, wxString::Format(wxT("%li"), value)));
}

bool
AnConfig::DoWriteInt(const wxString &key, int value)
{
	return (DoWriteLong(key, (long)value));
}

bool
AnConfig::DoWriteDouble(const wxString &key, double value)
{
	return (DoWriteString(key, wxString::Format(_T("%g"), value)));
}

bool
AnConfig::DoWriteBool(const wxString &key, bool value)
{
	return (DoWriteLong(key, value ? 1l : 0l));
}
