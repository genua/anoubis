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

#include "ModSfsDetailsDlg.h"
#include "SfsEntry.h"

ModSfsDetailsDlg::ModSfsDetailsDlg(SfsEntry *entry, wxWindow *parent)
    : ModSfsDetailsDlgBase(parent)
{
	/* Path */
	wxString path = entry->getPath();
	SetTitle(wxString::Format(_("Details for %ls"), path.c_str()));
	pathTextCtrl->SetValue(path);

	if (entry->isSymlink()) {
		linkLabel->Show();
		linkTextCtrl->Show();
		linkTextCtrl->SetValue(entry->resolve());
	} else {
		linkLabel->Hide();
		linkTextCtrl->Hide();
	}

	/* Last modification timestamp */
	wxDateTime modified = entry->getLastModified();
	if (modified.IsValid())
		modifiedTextCtrl->SetValue(modified.Format(wxT("%c")));
	else
		modifiedTextCtrl->SetValue(_("n/a"));

	/* Local checksum */
	wxString localCsum = entry->getLocalCsum();
	if (!localCsum.IsEmpty())
		checksumTextCtrl->SetValue(localCsum);
	else
		checksumTextCtrl->SetValue(_("n/a"));

	/* Remote checksum */
	wxString remoteCsum = entry->getChecksum(SfsEntry::SFSENTRY_CHECKSUM);
	if (!remoteCsum.IsEmpty())
		regChecksumTextCtrl->SetValue(remoteCsum);
	else
		regChecksumTextCtrl->SetValue(_("n/a"));

	/* Checksum state */
	switch (entry->getChecksumState(SfsEntry::SFSENTRY_CHECKSUM)) {
	case SfsEntry::SFSENTRY_NOT_VALIDATED:
		checksumStateLabel->SetLabel(_("not validated"));
		break;
	case SfsEntry::SFSENTRY_MISSING:
		checksumStateLabel->SetLabel(_("not registered"));
		break;
	case SfsEntry::SFSENTRY_INVALID:
		checksumStateLabel->SetLabel(_("invalid"));
		break;
	case SfsEntry::SFSENTRY_NOMATCH:
		checksumStateLabel->SetLabel(_("not matching"));
		break;
	case SfsEntry::SFSENTRY_MATCH:
		checksumStateLabel->SetLabel(_("matching"));
		break;
	case SfsEntry::SFSENTRY_ORPHANED:
		checksumStateLabel->SetLabel(_("orphaned"));
		break;
	case SfsEntry::SFSENTRY_UPGRADED:
		checksumStateLabel->SetLabel(_("upgraded"));
		break;
	}

	/* Signature state */
	switch (entry->getChecksumState(SfsEntry::SFSENTRY_SIGNATURE)) {
	case SfsEntry::SFSENTRY_NOT_VALIDATED:
		signatureStateLabel->SetLabel(_("not validated"));
		break;
	case SfsEntry::SFSENTRY_MISSING:
		signatureStateLabel->SetLabel(_("not registered"));
		break;
	case SfsEntry::SFSENTRY_INVALID:
		signatureStateLabel->SetLabel(_("invalid"));
		break;
	case SfsEntry::SFSENTRY_NOMATCH:
		signatureStateLabel->SetLabel(_("not matching"));
		break;
	case SfsEntry::SFSENTRY_MATCH:
		signatureStateLabel->SetLabel(_("matching"));
		break;
	case SfsEntry::SFSENTRY_ORPHANED:
		signatureStateLabel->SetLabel(_("orphaned"));
		break;
	case SfsEntry::SFSENTRY_UPGRADED:
		signatureStateLabel->SetLabel(_("upgraded"));
		break;
	}
}
