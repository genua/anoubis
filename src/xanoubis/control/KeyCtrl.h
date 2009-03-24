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

#ifndef _KEYCTRL_H_
#define _KEYCTRL_H_

#include "LocalCertificate.h"
#include "PrivKey.h"
#include "Singleton.h"

/**
 * Interface used by KeyCtrl to fetch a passphrase.
 */
class PassphraseReader
{
	public:
		virtual ~PassphraseReader(void) {}

		/**
		 * Reads a passphrase.
		 *
		 * @param ok Set to true, if the passphrase was successfully
		 *           read by the method. Set to false if the
		 *           read-operation was canceled. In this case the
		 *           returned passphrase is ignored.
		 * @return A passphrase (can be an empty string), if ok is set
		 *         to true. If ok == false, the result is ignored.
		 */
		virtual wxString readPassphrase(bool *) = 0;
};

/**
 * Controller responsible for managing keys and certificates.
 *
 * You can obtain the private key of the user by calling getPrivateKey(). Note,
 * that the private key is not loaded by default. You need to call
 * PrivKey::load()! Depending on the validity-settings of the private key, the
 * key might be removed from memory automatically!
 *
 * You can get the certificate of the user by calling getLocalCertificate().
 * You need to load the certificate explicity from a file by calling
 * Certificate::load()! Once loaded, the certificate remains in memory until
 * the application quits.
 */
class KeyCtrl : public Singleton<KeyCtrl>
{
	public:
		/**
		 * Destructor of KeyCtrl.
		 * This will clean up the whole mess. It needs to be public,
		 * but you are not allowed to use it (delete).
		 */
		~KeyCtrl(void);

		/**
		 * Get object.
		 * This returns the (only) object of this class
		 * (see singleton pattern).
		 * @return It self.
		 */
		static KeyCtrl *getInstance(void);

		/**
		 * Returns the assigned passphrase reader.
		 *
		 * This interface is used to ask for a passphrase, whenever
		 * a passphrase is needed. Currently loadPrivateKey() is
		 * using the interface to receive the passphrase is the
		 * private key is not loaded.
		 *
		 * @return The assigned passphrase reader
		 */
		PassphraseReader *getPassphraseReader(void) const;

		/**
		 * Assigns a passphrase reader to the controller.
		 *
		 * Then, the assigned passphrase reader is used to receive a
		 * passphrase.
		 *
		 * @param reader The passphrase reader to be assigned
		 */
		void setPassphraseReader(PassphraseReader *);

		/**
		 * Returns the private key of the user.
		 *
		 * @return Instance of the users private key.
		 * @note Obtaining the instance of the PrivKey-class does not
		 *       mean, that the private key is loaded into the memory!
		 */
		PrivKey &getPrivateKey(void);

		/**
		 * Returns the certificate of the user.
		 *
		 * @return Instance of the users certificate.
		 * @note Obtaining the instance of the certificate does not
		 *       mean, that the certificate is loaded into the memory!
		 */
		LocalCertificate &getLocalCertificate(void);

		/**
		 * Tests whether local keys are both ready for usage.
		 *
		 * Private key (getPrivateKey() and certificate
		 * (getLocalCertificate()) has a strong binding. This method
		 * tests, if the private key can be loaded and the certificate
		 * is loaded.
		 *
		 * @return true is returned, if the private key can be loaded
		 *         and the certificate is loaded.
		 */
		bool canUseLocalKeys(void) const;

		/**
		 * Loads the private key.
		 *
		 * When the private key is not loaded, the method tries to
		 * fetch the passphrase using the assigned PassphraseReader. If
		 * no passphrase reader is assigned, the method uses an empty
		 * string (wxEmptyString) as a passphrase.
		 *
		 * @return true is returned, if the private key is already
		 *         loaded, or the key was successfully loaded.
		 *
		 * @see getPassphraseReader()
		 * @see PrivKey
		 */
		bool loadPrivateKey(void);

	protected:
		/**
		 * Constructor of KeyCtrl.
		 */
		KeyCtrl(void);

	private:
		/**
		 * Passphrase reader assigned to the controller.
		 */
		PassphraseReader *passphraseReader_;

		/**
		 * Instance of the private key.
		 */
		PrivKey privKey_;

		/**
		 * Instance of users certificate.
		 */
		LocalCertificate cert_;

	friend class Singleton<KeyCtrl>;
};

#endif	/* _KEYCTRL_H_ */
