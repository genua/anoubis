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

	defsubj = anoubis_keysubject_defaults();
	if (defsubj) {

/* Macro to make the folloing repeated code more readable. */
#define SET_DEFAULT(TEXTCTRL, FIELD)					\
	if (defsubj->FIELD)						\
		(TEXTCTRL)->ChangeValue(wxString::FromAscii(defsubj->FIELD))

	SET_DEFAULT(CountryOfCertTxtCtrl, country);
	SET_DEFAULT(StateOfCertTxtCtrl, state);
	SET_DEFAULT(LocalityOfCertTxtCtrl, locality);
	SET_DEFAULT(OrgaOfCertTxtCtrl, organization);
	SET_DEFAULT(OrgaunitOfCertTxtCtrl, orgunit);
	SET_DEFAULT(ComNameOfCertTxtCtrl, name);
	SET_DEFAULT(EmailOfCertTxtCtrl, email);

#undef SET_DEFAULT

		anoubis_keysubject_destroy(defsubj);
	}

	pathPrivKeyTxtCtrl->ChangeValue(pathPrefix + wxT(".key"));
	pathToCertTxtCtrl->ChangeValue(pathPrefix + wxT(".crt"));
}

void
ModSfsGenerateKeyDlg::OnOkButton(wxCommandEvent &event)
{
	wxString			 pass;
	struct anoubis_keysubject	 subject;
	char				*subjstr;
	int				 error;


	pass = PassphrPrivKeyTxtCtrl->GetValue();
	if (pass != PassphrRepeatPrivKeyTxtCtrl->GetValue()) {
		PassphrMismatchTxt->Show();
		PassphrMisMatchIcon->Show();
		Layout();
		Refresh();
		return;
	}
	PassphrMismatchTxt->Hide();
	PassphrMisMatchIcon->Hide();
	Layout();
	Refresh();
	subject.country = strdup(CountryOfCertTxtCtrl->GetValue().fn_str());
	subject.state = strdup(StateOfCertTxtCtrl->GetValue().fn_str());
	subject.locality = strdup(LocalityOfCertTxtCtrl->GetValue().fn_str());
	subject.organization = strdup(OrgaOfCertTxtCtrl->GetValue().fn_str());
	subject.orgunit = strdup(OrgaunitOfCertTxtCtrl->GetValue().fn_str());
	subject.name = strdup(ComNameOfCertTxtCtrl->GetValue().fn_str());
	subject.email = strdup(EmailOfCertTxtCtrl->GetValue().fn_str());
	subjstr = anoubis_keysubject_tostring(&subject);
	if (subjstr == NULL) {
		error = -ENOMEM;
	} else {
		error = anoubis_keygen(pathPrivKeyTxtCtrl->GetValue().fn_str(),
		    pathToCertTxtCtrl->GetValue().fn_str(), pass.fn_str(),
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
	PassphrMismatchTxt->Hide();
	PassphrMisMatchIcon->Hide();
	Layout();
	Refresh();
}
