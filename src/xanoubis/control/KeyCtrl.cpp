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

#include "KeyCtrl.h"
#include "Singleton.cpp"

int
xpass_cb(char *buf, int size, int, void *)
{
	bool ok = true;
	int pass_len;
	wxString result = wxEmptyString;
	PassphraseReader *pr = KeyCtrl::getInstance()->getPassphraseReader();
	if (pr != 0) {
		result = pr->readPassphrase(&ok);
		if (!ok)
			return (0);
	}

	pass_len = result.length();
	if (pass_len > size)
		pass_len = size;

	memcpy(buf, (const char *)result.mb_str(wxConvUTF8), pass_len);
	result.Clear();

	return (pass_len);
}

KeyCtrl::KeyCtrl(void)
{
	this->passphraseReader_ = 0;
}

KeyCtrl::~KeyCtrl(void)
{
}

KeyCtrl *
KeyCtrl::getInstance(void)
{
	return (Singleton<KeyCtrl>::instance());
}

PassphraseReader *
KeyCtrl::getPassphraseReader(void) const
{
	return (this->passphraseReader_);
}

void
KeyCtrl::setPassphraseReader(PassphraseReader *reader)
{
	this->passphraseReader_ = reader;
}

PrivKey &
KeyCtrl::getPrivateKey(void)
{
	return (privKey_);
}

LocalCertificate &
KeyCtrl::getLocalCertificate(void)
{
	return (cert_);
}

bool
KeyCtrl::canUseLocalKeys(void) const
{
	return (privKey_.canLoad() && cert_.isLoaded());
}

bool
KeyCtrl::loadPrivateKey(void)
{
	if (!privKey_.isLoaded()) {
		return privKey_.load(xpass_cb);
	} else
		return (true); /* Already loaded */
}
