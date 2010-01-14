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

#include <wx/stdpaths.h>
#include <wx/datetime.h>

#include <anoubis_sig.h>
#include <keygen.h>
#include <time.h>

#include "LocalCertificate.h"

LocalCertificate::LocalCertificate(void)
{
	const wxString defFile =
	    wxStandardPaths::Get().GetUserDataDir() + wxT("/default.crt");

	this->disName_ = wxEmptyString;
	this->keyId_ = wxEmptyString;
	this->subject_ = NULL;
	this->cert_ = 0;

	/* Assign default certificate, if already existing */
	if (wxFileExists(defFile))
		this->certFile_ = defFile;
	else
		this->certFile_ = wxEmptyString;
}

LocalCertificate::~LocalCertificate(void)
{
	unload();
}

wxString
LocalCertificate::getFile(void) const
{
	return (this->certFile_);
}

void
LocalCertificate::setFile(const wxString &file)
{
	this->certFile_ = file;
}

wxString
LocalCertificate::getKeyId(void) const
{
	return (this->keyId_);
}

wxString
LocalCertificate::getFingerprint(void) const
{
	wxString fingerprint = wxEmptyString;
	/* Fingerprint is equal to key-id but the representation differs */

	if (cert_ != 0) {
		for (int i = 0; i < cert_->idlen; i++) {
			if (i == 0)
				fingerprint = wxString::Format(wxT("%2.2X"),
				    cert_->keyid[i]);
			else
				fingerprint += wxString::Format(wxT(":%2.2X"),
				    cert_->keyid[i]);
		}
	}

	return (fingerprint);
}

wxDateTime
LocalCertificate::getValidity(void) const
{
	int		rc;
	struct tm	time;
	wxDateTime	wxTime;

	if (cert_ != NULL) {
		rc = anoubis_sig_cert_validity(cert_->cert, &time);
		if (rc == 0) {
			wxTime.Set(time);
		}
	}

	return (wxTime);
}

wxString
LocalCertificate::getDistinguishedName(void) const
{
	if (cert_ != 0) {
		return (disName_);
	} else
		return (wxEmptyString);
}

wxString
LocalCertificate::getCountry(void) const
{
	if (subject_ && subject_->country)
		return wxString::FromAscii(subject_->country);
	return wxEmptyString;
}

wxString
LocalCertificate::getState(void) const
{
	if (subject_ && subject_->state)
		return wxString::FromAscii(subject_->state);
	return wxEmptyString;
}

wxString
LocalCertificate::getLocality(void) const
{
	if (subject_ && subject_->locality)
		return wxString::FromAscii(subject_->locality);
	return wxEmptyString;
}

wxString
LocalCertificate::getOrganization(void) const
{
	if (subject_ && subject_->organization)
		return wxString::FromAscii(subject_->organization);
	return wxEmptyString;
}

wxString
LocalCertificate::getOrganizationalUnit(void) const
{
	if (subject_ && subject_->orgunit)
		return wxString::FromAscii(subject_->orgunit);
	return wxEmptyString;
}

wxString
LocalCertificate::getCommonName(void) const
{
	if (subject_ && subject_->name)
		return wxString::FromAscii(subject_->name);
	return wxEmptyString;
}

wxString
LocalCertificate::getEmailAddress(void) const
{
	if (subject_ && subject_->email)
		return wxString::FromAscii(subject_->email);
	return wxEmptyString;
}

struct anoubis_sig *
LocalCertificate::getCertificate(void) const
{
	return (this->cert_);
}

bool
LocalCertificate::canLoad(void) const
{
	return (wxFileExists(certFile_));
}

bool
LocalCertificate::isLoaded(void) const
{
	return (cert_ != 0);
}

bool
LocalCertificate::load(void)
{
	/* XXX Error handling? */
	int error = 0;

	char *dname = NULL;

	/* Read certificate */
	struct anoubis_sig *cert = anoubis_sig_pub_init(
	    0, certFile_.fn_str(), 0, &error);


	unload(); /* Destroy previously loaded certificate */

	if (cert != 0) {
		/* Extract keyid from certificate */
		char *keyid = anoubis_sig_key2char(cert->idlen, cert->keyid);

		if (keyid != 0) {
			this->keyId_ = wxString::FromAscii(keyid);
			this->cert_ = cert;

			free(keyid);

		} else {
			/* Failed to obtain keyid from certificate */
			anoubis_sig_free(cert);
			return (false);
		}
		dname = anoubis_sig_cert_name(cert_->cert);
		if (!dname)
			return (false);
		disName_ = wxString::FromAscii(dname);

		/* get and assign string tokens of subject */
		subject_ = anoubis_keysubject_fromstring(dname);
		if (!subject_)
			return (false);
		free(dname);
	} else {
		/* Failed to load certificate */
		return (false);
	}
	return (true);
}

void
LocalCertificate::unload(void)
{
	if (this->cert_ != 0) {
		anoubis_sig_free(this->cert_);
		this->keyId_ = wxEmptyString;
		this->cert_ = 0;

		if (this->subject_ != NULL) {
			anoubis_keysubject_destroy(this->subject_);
			this->subject_ = NULL;
		}
	}
}
