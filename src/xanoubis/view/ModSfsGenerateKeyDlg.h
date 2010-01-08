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

#ifndef _MODSFSGENERATEKEYDLG_H_
#define _MODSFSGENERATEKEYDLG_H_

#include "ModSfsPanelsBase.h"

/**
 * A dialog providing details to configure private key and certificate
 * as follows:
 *
 * - path to private key file
 * - passphrase of the private key
 *
 * - path to certificate file
 * - certificate details like COUNTRY, STATE, ORGANIZATION, EMAIL and the likes
 */
class ModSfsGenerateKeyDlg : public ModSfsGenerateKeyDlgBase
{
public:
	/**
	 * Constructor: Intialize dialog with default values.
	 * @param[in] 1st The parent window.
	 */
	ModSfsGenerateKeyDlg(wxWindow *);

	/**
	 * Returns the path of the file, where the private key is stored.
	 *
	 * The path is configured in the dialog.
	 *
	 * @return Path of private key
	 */
	wxString getPathPrivateKey(void) const;

	/**
	 * Returns the path of the file, where the certificate is stored.
	 *
	 * The path is configured in the dialog.
	 *
	 * @return Path of certificate
	 */
	wxString getPathCertificate(void) const;

protected:
	/**
	 * Handler that is called if the OK button is pressed.
	 * @param[in] 1st The button event.
	 * @return None.
	 */
	virtual void OnOkButton(wxCommandEvent& event);

	/**
	 * This is called after the construction of the dialog but
	 * before the user inputs data. We can hide some elements of the
	 * dialog that are only display in response to user input here.
	 *
	 * @param[in] 1st The init event.
	 * @return None.
	 */
	virtual void InitDialog(wxInitDialogEvent & event);

private:
	/**
	 * Generate a name prefix for a new private/public key pair.
	 * Make sure that neither prefix + wxT(".crt") nor
	 * prefix + wxT(".key") exists.
	 * @param None.
	 * @return The new prefix.
	 */
	wxString	newDefaultName(void);
};



#endif	/* _MODSFSGENERATEKEYDLG_H_ */
