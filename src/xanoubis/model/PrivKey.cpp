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

#include <config.h>

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include <wx/stdpaths.h>

#include "PrivKey.h"

PrivKey::PrivKey(void)
{
	const wxString defFile =
	    wxStandardPaths::Get().GetUserDataDir() + wxT("/default.key");

	privKey_ = 0;
	validity_ = 0;
	tries_ = 0;
	referenced_ = false;

	/* Assign default keyfile, if already existing */
	if (wxFileExists(defFile))
		this->keyFile_ = defFile;
	else
		this->keyFile_ = wxEmptyString;
}

PrivKey::~PrivKey(void)
{
	unload();
}

wxString
PrivKey::getFile(void) const
{
	return (keyFile_);
}

int
PrivKey::getTries(void)
{
	return(tries_);
}

void
PrivKey::resetTries(void)
{
	tries_ = 0;
}

void
PrivKey::setFile(const wxString &file)
{
	this->keyFile_ = file;
}

int
PrivKey::getValidity(void) const
{
	return (validity_);
}

void
PrivKey::setValidity(int validity)
{
	this->validity_ = validity;
	if (validity_) {
		startTimer();
	} else {
		stopTimer();
	}
}

bool
PrivKey::canLoad(void) const
{
	return (wxFileExists(keyFile_));
}

PrivKey::PrivKeyResult
PrivKey::load(pem_password_cb *xpass_cb)
{
	int err = 0;

	if (privKey_ == 0) {
		do {
			err = anoubis_sig_create(&privKey_, keyFile_.fn_str(),
			    NULL, xpass_cb);
			if (err < 0) {
				if (err == -EPERM)
					tries_++;
				else if (err == -ECANCELED)
					break;
				else
					break;
			} else
				this->resetTries();
		} while (err && tries_ < 3);

		if (err < 0) {
			if (tries_ >= 3)
				return ERR_PRIV_WRONG_PASS;
			else if (err == -ECANCELED)
				return ERR_PRIV_ABORT;
			else
				return ERR_PRIV_ERR;
		}

		if (privKey_ != 0) {
			/*
			 * Loading the key was successful.
			 * Now start the timer to unload the key again.
			 */
			if (!startTimer()) {
				/* Resets privKey_ to 0 */
				unload();
			}
		}
		return ERR_PRIV_OK;
	} else {
		/* A key is already loaded */
		return ERR_PRIV_OK;
	}
}

bool
PrivKey::unload(void)
{
	if (this->privKey_ != 0) {
		/* A key is loaded, unload it and stop the timer. */
		anoubis_sig_free(this->privKey_);
		this->privKey_ = 0;

		stopTimer();

		return (true);
	} else
		return (false);
}

bool
PrivKey::isLoaded(void) const
{
	return (this->privKey_ != 0);
}

struct anoubis_sig *
PrivKey::getKey(void)
{
	referenced_ = true;
	return (this->privKey_);
}

bool
PrivKey::startTimer(void)
{
	if (validity_ > 0)
		return Start(validity_ * 1000, true);
	else
		return (true);
}

void
PrivKey::stopTimer(void)
{
	Stop();
}

void
PrivKey::Notify(void)
{
	if (referenced_) {
		referenced_ = false;
		startTimer();
	} else {
		unload();
	}
}
