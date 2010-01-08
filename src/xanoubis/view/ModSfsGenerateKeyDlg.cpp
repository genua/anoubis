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

#include "ModSfsGenerateKeyDlg.h"

#include <cstring>
#include <cerrno>
#include <cstdlib>

#include <keygen.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <AnMessageDialog.h>

#include "AnIconList.h"
#include "AnPickFromFs.h"

wxString
ModSfsGenerateKeyDlg::newDefaultName(void)
{
	wxString	datadir = wxStandardPaths::Get().GetUserDataDir();
	wxString	prefix = datadir + wxT("/default");
	int		count = 0;

	while (1) {
		if (!wxFileExists(prefix + wxT(".crt"))
		    && !wxFileExists(prefix + wxT(".key")))
			break;
		count++;
		prefix = wxString::Format(wxT("%ls/default%d"),
		    datadir.c_str(), count);
	}
	return prefix;
}


ModSfsGenerateKeyDlg::ModSfsGenerateKeyDlg(wxWindow *parent)
    : ModSfsGenerateKeyDlgBase(parent)
{
	struct anoubis_keysubject	*defsubj;
	wxString			 pathPrefix = newDefaultName();
	wxSize				 indent;
	wxIcon				 icon;
	wxSizerItem			*spacer;

	defsubj = anoubis_keysubject_defaults();
	if (defsubj) {

/* Macro to make the folloing repeated code more readable. */
#define SET_DEFAULT(TEXTCTRL, FIELD)					\
	if (defsubj->FIELD)						\
		(TEXTCTRL)->ChangeValue(wxString::FromAscii(defsubj->FIELD))

	SET_DEFAULT(certCountryTextCtrl, country);
	SET_DEFAULT(certStateTextCtrl, state);
	SET_DEFAULT(certLocalityTextCtrl, locality);
	SET_DEFAULT(certOrgaTextCtrl, organization);
	SET_DEFAULT(certOrgaUnitTextCtrl, orgunit);
	SET_DEFAULT(certCnTextCtrl, name);
	SET_DEFAULT(certEmailTextCtrl, email);

#undef SET_DEFAULT

		anoubis_keysubject_destroy(defsubj);
	}

	keyPicker->setTitle(_("Configure private key:"));
	keyPicker->setButtonLabel(_("Browse ..."));
	keyPicker->setMode(AnPickFromFs::MODE_NEWFILE);
	keyPicker->setFileName(pathPrefix + wxT(".key"));

	certificatePicker->setTitle(_("Configure certificate:"));
	certificatePicker->setButtonLabel(_("Browse ..."));
	certificatePicker->setMode(AnPickFromFs::MODE_NEWFILE);
	certificatePicker->setFileName(pathPrefix + wxT(".crt"));

	/* Get initial indent size. */
	spacer = certDetailsIndentSizer->GetItem((size_t)0);
	indent = keyPicker->getTitleSize();
	indent.IncBy(30, 0);

	/* Set indentation. */
	keyPicker->setTitleMinSize(indent);
	certificatePicker->setTitleMinSize(indent);
	passphraseLabel->SetMinSize(indent);

	/* Adjust indent by indentation of details block. */
	indent.DecBy(spacer->GetSize().GetWidth(), 0);
	certValidityLabel->SetMinSize(indent);

	icon = AnIconList::getInstance()->GetIcon(AnIconList::ICON_ERROR);
	passphraseMismatchIcon->SetIcon(icon);
}

wxString
ModSfsGenerateKeyDlg::getPathPrivateKey(void) const
{
	return keyPicker->getFileName();
}

wxString
ModSfsGenerateKeyDlg::getPathCertificate(void) const
{
	return certificatePicker->getFileName();
}

void
ModSfsGenerateKeyDlg::OnOkButton(wxCommandEvent &event)
{
	wxString			 pass;
	struct anoubis_keysubject	 subject;
	char				*subjstr;
	int				 error;


	pass = passphraseTextCtrl->GetValue();
	if (pass != passphraseRepeatTextCtrl->GetValue()) {
		passphraseMismatchText->Show();
		passphraseMismatchIcon->Show();
		Layout();
		Refresh();
		return;
	}
	passphraseMismatchText->Hide();
	passphraseMismatchIcon->Hide();
	Layout();
	Refresh();
	subject.country = strdup(certCountryTextCtrl->GetValue().fn_str());
	subject.state = strdup(certStateTextCtrl->GetValue().fn_str());
	subject.locality = strdup(certLocalityTextCtrl->GetValue().fn_str());
	subject.organization = strdup(certOrgaTextCtrl->GetValue().fn_str());
	subject.orgunit = strdup(certOrgaUnitTextCtrl->GetValue().fn_str());
	subject.name = strdup(certCnTextCtrl->GetValue().fn_str());
	subject.email = strdup(certEmailTextCtrl->GetValue().fn_str());
	subjstr = anoubis_keysubject_tostring(&subject);
	if (subjstr == NULL) {
		error = -ENOMEM;
	} else {
		error = anoubis_keygen(keyPicker->getFileName().fn_str(),
		    certificatePicker->getFileName().fn_str(), pass.fn_str(),
		    subjstr, 2048);
	}
	free(subject.country);
	free(subject.state);
	free(subject.locality);
	free(subject.organization);
	free(subject.orgunit);
	free(subject.name);
	free(subject.email);
	free(subjstr);
	if (error < 0) {
		wxString	errmsg;

		errmsg = wxString::Format(_("Failed to create keypair (%hs)"),
		    strerror(-error));

		anMessageBox(errmsg, _("Error"), wxICON_ERROR, this);
	} else {
		/*
		 * If we skip the event the default handler will close
		 * the dialog box.
		 */
		event.Skip();
	}
}

void
ModSfsGenerateKeyDlg::InitDialog(wxInitDialogEvent & event)
{
	/* Make sure that other handlers run as wall. */
	event.Skip();

	/* Hide the passphrase mismatch text and icon. */
	passphraseMismatchText->Hide();
	passphraseMismatchIcon->Hide();
	Layout();
	Refresh();
}
