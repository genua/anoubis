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

#include "PrivKey.h"
#include "Singleton.h"

/**
 * Controller responsible for managing keys and certificates.
 *
 * You can obtain the private key of the user by calling getPrivateKey(). Note,
 * that the private key is not loaded by default. You need to call
 * PrivKey::load()! Depending on the validity-settings of the private key, the
 * key might be removed from memory automatically!
 */
class KeyCtrl : public Singleton<KeyCtrl>
{
	public:
		/**
		 * Destructor of ProfileCtrl.
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
		 * Returns the private key of the user.
		 *
		 * @return Instance of the users private key.
		 * @note Obtaining the instance of the PrivKey-class does not
		 *       mean, that the private key is loaded into the memory!
		 */
		PrivKey &getPrivateKey(void);

	protected:
		/**
		 * Constructor of KeyCtrl.
		 */
		KeyCtrl(void);

	private:
		/**
		 * Instance of the private key.
		 */
		PrivKey privKey_;

	friend class Singleton<KeyCtrl>;
};

#endif	/* _KEYCTRL_H_ */
