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

/*
 * Event if only used internally. That's why, it not not part of the
 * public interface.
 *
 * anEVT_LOAD_PRIVKEY is fired, when the private key should be loaded.
 * This needs to be performed in the GUI-thread because the user might
 * enter the passphrase of the private key.
 */
BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_LOCAL_EVENT_TYPE(anEVT_LOAD_PRIVKEY, wxNewEventType())
END_DECLARE_EVENT_TYPES()

DEFINE_LOCAL_EVENT_TYPE(anEVT_LOAD_PRIVKEY)

/* This function is needed by libanoubissig to call a passphrase
 * Reader just in need.
 * Since libanoubissig is using openssl it has to be in this
 * form.
 * */
extern "C" int
xpass_cb(char *buf, int size, int, void *err)
{
	wxString result = wxEmptyString;
	bool ok = true;
	int pass_len;
	int *error = (int *)err;

	PassphraseReader *pr = KeyCtrl::getInstance()->getPassphraseReader();
	if (pr != 0) {
		result = pr->readPassphrase(&ok);
		if (!ok) {
			*error = 1;
			return (0);
		}
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

	this->loadCondition_ = new wxCondition(loadMutex_);
	this->loadResult_ = RESULT_KEY_ERROR;

	Connect(anEVT_LOAD_PRIVKEY,
	    wxCommandEventHandler(KeyCtrl::onLoadPrivateKeyRequest),
	    NULL, this);
}

KeyCtrl::~KeyCtrl(void)
{
	Disconnect(anEVT_LOAD_PRIVKEY,
	    wxCommandEventHandler(KeyCtrl::onLoadPrivateKeyRequest),
	    NULL, this);

	delete this->loadCondition_;
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

KeyCtrl::KeyResult
KeyCtrl::loadPrivateKey(void)
{
	if (wxIsMainThread()) {
		/* This is the GUI-thread, simply load the key */
		return (doLoadPrivateKey());
	} else {
		/*
		 * This current thread differs from the GUI-thread, but the
		 * private key needs to be loaded from the GUI-thread because
		 * the user might enter the passphrase.
		 *
		 * Post an event to the GUI-thread.
		 */
		loadMutex_.Lock();

		wxCommandEvent event(anEVT_LOAD_PRIVKEY);
		wxPostEvent(this, event);

		/* Wait for the load-result from the GUI-thread */
		loadCondition_->Wait();

		loadMutex_.Unlock();

		return (this->loadResult_);
	}
}

KeyCtrl::KeyResult
KeyCtrl::doLoadPrivateKey(void)
{
	PrivKey::PrivKeyResult keyRes;

	if (!privKey_.isLoaded()) {
		keyRes = privKey_.load(xpass_cb);
		privKey_.resetTries();
		switch (keyRes) {
		case PrivKey::ERR_PRIV_WRONG_PASS:
			return (RESULT_KEY_WRONG_PASS);
			/* NOTREACHED */
		case PrivKey::ERR_PRIV_ERR:
			return (RESULT_KEY_ERROR);
			/* NOTREACHED */
		case PrivKey::ERR_PRIV_ABORT:
			return (RESULT_KEY_ABORT);
			/* NOTREACHED */
		case PrivKey::ERR_PRIV_OK:
			return (RESULT_KEY_OK);
			/* NOTREACHED */
		}
		return (RESULT_KEY_ERROR);
	} else
		return (RESULT_KEY_OK); /* Already loaded */
}

void
KeyCtrl::onLoadPrivateKeyRequest(wxCommandEvent &)
{
	assert(wxIsMainThread());

	wxMutexLocker locker(loadMutex_);
	loadResult_ = doLoadPrivateKey();

	/* Inform the calling thread that the key is loaded */
	loadCondition_->Signal();
}
