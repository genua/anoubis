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

#include <wx/icon.h>
#include <wx/stdpaths.h>
#include <wx/string.h>

#include "TrayIcon.h"

TrayIcon::TrayIcon(void)
{
	wxString	iconFileName;
	wxStandardPaths paths;

	iconFileName = paths.GetDataDir();
	if (!::wxDirExists(iconFileName))
		iconFileName = ::wxPathOnly(paths.GetExecutablePath());
	iconFileName += _T("/icons/tray_normal.png");
	iconNormal_ = new wxIcon(iconFileName, wxBITMAP_TYPE_PNG);

	iconFileName = paths.GetDataDir();
	if (!::wxDirExists(iconFileName))
		iconFileName = ::wxPathOnly(paths.GetExecutablePath());
	iconFileName += _T("/icons/tray_msgByHand.png");
	iconMsgByHand_ = new wxIcon(iconFileName, wxBITMAP_TYPE_PNG);

	messageByHandNo_ = 0;
	update();
}

TrayIcon::~TrayIcon(void)
{
	delete iconNormal_;
	delete iconMsgByHand_;
}

void
TrayIcon::SetMessageByHand(unsigned int number)
{
	messageByHandNo_ = number;
	update();
}

void
TrayIcon::update(void)
{
	if (messageByHandNo_ > 0) {
		wxString tooltip = wxT("Messages: ");
		tooltip += wxString::Format(wxT("%d\n"), messageByHandNo_);
		tooltip += wxT("connected with localhost");
		SetIcon(*iconMsgByHand_, tooltip);
	} else {
		SetIcon(*iconNormal_, wxString(wxT(
		    "No messages\nconnected with localhost")));
	}
}
