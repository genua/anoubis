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

#ifndef _PRIVKEY_H_
#define _PRIVKEY_H_

#include <wx/timer.h>
#include <anoubis_sig.h>

/**
 * A private key.
 *
 * The private key needs to be loaded explicitly from a file. For security
 * reasons the key can be removed from memory after a configurable period of
 * time. When the validity-period is expired, the key needs to be loaded
 * another time.
 *
 * The default-location for the private-key is
 * <code>~/.xanoubis/default.key</code>. If such a file exists, the
 * file-attribute is automatically filled with the path on creation. But you
 * are still able to change the path afterwards.
 */
class PrivKey : private wxTimer
{
	public:

		/**
		 * Result-Codes of loading key
		 */
		enum PrivKeyResult {
			ERR_PRIV_WRONG_PASS,	/*<! Wrong password was
						 * entered. */
			ERR_PRIV_ABORT,		/*<! Abort entering password */
			ERR_PRIV_ERR,		/*<! A an error occured. */
			ERR_PRIV_OK		/*<! Everything worked fine. */
		};

		/**
		 * Default-c'tor.
		 *
		 * If the file <code>~/.xanoubis/default.key</code> exists,
		 * the file-attribute is automatically filled with the path.
		 *
		 * Sets the key-validity to infinity.
		 * You explicitly need to call load() to load a key!
		 *
		 * @see setValidity()
		 */
		PrivKey(void);

		/**
		 * D'tor.
		 *
		 * Unloads the key if still loaded.
		 */
		~PrivKey(void);

		/**
		 * Returns the path of the file, where the key is stored.
		 *
		 * The default-path is <code>~/.xanoubis/default.key</code>.
		 * If such a file exists, the path is automatically filled
		 * during creation of the instance.
		 *
		 * @return Path of file, where the key is stored.
		 */
		wxString getFile(void) const;

		/**
		 * Updates the path of the file, where the key is stored.
		 *
		 * Make sure to call this method before you load a key (using
		 * the load()-method). A new path takes affect with the next
		 * call of load().
		 *
		 * @param file Path of file, where the key is stored.
		 */
		void setFile(const wxString &);

		/**
		 * Returns the key-validity.
		 *
		 * A value greater than 0 means, that the key is automatically
		 * unloaded after the number of seconds. A value less or equal
		 * to 0 sets the key-validity to infinity. It means, that the
		 * key is available until the d'tor is invoked.
		 *
		 * @return Key-validity in seconds.
		 */
		int getValidity(void) const;

		/**
		 * Updates the key-validity.
		 *
		 * The new validity takes affect with the next call of load().
		 *
		 * @param validity The new validity in seconds.
		 */
		void setValidity(int);

		/**
		 * Tests whether the key can be loaded.
		 *
		 * You need at least a configured filename to be able to load
		 * the private key.
		 *
		 * @return true is returned, if you are able to load the key,
		 *         false otherwise.
		 */
		bool canLoad(void) const;

		/**
		 * Returns the number of incorrect password tries
		 *
		 * @return number of incorrect password attempts
		 */
		int getTries(void);

		/**
		 * resets the number of tries to zero
		 */
		void resetTries(void);

		/**
		 * Loads a private key protected by the specified passphrase.
		 *
		 * The key is read from the file configured with setFile().
		 *
		 * If a validity greater than 0 is configured, the key is
		 * automatically unloaded after the number of seconds.
		 *
		 * If loading of a key was successful and the key is still
		 * loaded, isLoaded() will return true. If loading of a key
		 * was not successful or the key was unloaded manually or
		 * automatically, isLoaded() will return false.
		 *
		 * @param call back function to request passphrase
		 *
		 * @return a PrivKeyResult.
		 *
		 * @see isLoaded()
		 * @see unload()
		 */
		PrivKeyResult load(pem_password_cb *);

		/**
		 * Unloads the key again.
		 *
		 * @return On success, true is returned. A return value of
		 *         false means, that no key was loaded.
		 */
		bool unload(void);

		/**
		 * Tests whether a key is loaded.
		 *
		 * There are several reasons, why no key is loaded:
		 * - No key is loaded (using the load()-method)
		 * - The key was unloaded manually (using the unload()-method)
		 * - The key was automatically unloaded after the
		 *   validity-period has expired.
		 *
		 * @return true if a key is currently loaded, false otherwise.
		 */
		bool isLoaded(void) const;

		/**
		 * Returns the raw-key.
		 *
		 * The method will return the result of a successful
		 * load-operation.
		 *
		 * @return The raw-key. If no key is loaded, 0 is returned.
		 */
		struct anoubis_sig *getKey(void) const;

	private:
		/**
		 * Data structure representing the private key.
		 */
		struct anoubis_sig *privKey_;

		/**
		 * Path to the file, where the key is stored.
		 */
		wxString keyFile_;

		/**
		 * Key validity.
		 */
		int validity_;

		/**
		 * Tries to type in a correct passphrase
		 */
		int tries_;

		/**
		 * Starts a timer if necessary.
		 * If you need to start a timer depends on the value of
		 * _validity.
		 *
		 * @return true is returned if the timer was started
		 *         successfully or there is no need to start a timer.
		 */
		bool startTimer(void);

		/**
		 * Stops the timer again.
		 */
		void stopTimer(void);

		/**
		 * Re-implementation of wxTimer::Notify().
		 *
		 * The method is invoked if the timer is expired.
		 */
		void Notify(void);
};

#endif	/* _PRIVKEY_H_ */
