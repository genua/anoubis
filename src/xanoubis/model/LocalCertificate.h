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

#ifndef _LOCALCERTIFICATE_H_
#define _LOCALCERTIFICATE_H_

#include <anoubis_keygen.h>
#include <wx/string.h>
#include <wx/datetime.h>

/**
 * A local certificate.
 *
 * A local certificate is selected and loaded by the user to verify signed
 * checksums.
 *
 * The default-location for the certificate is
 * <code>~/.xanoubis/default.crt</code>. If such a file exists, the
 * file-attribute is automatically filled with the path on creation. But you
 * are still able to change the path afterwards.
 *
 * You explicity need to load the certificate by calling load()!
 */
class LocalCertificate
{
	public:
		/**
		 * Default c'tor.
		 *
		 * If the file <code>~/.xanoubis/default.crt</code> exists,
		 * the file-attribute is automatically filled with the path.
		 *
		 * You explicitly need to call load() to load the certificate
		 * from the file!
		 */
		LocalCertificate(void);

		/**
		 * D'tor.
		 *
		 * Unloads the certificate if still loaded.
		 */
		~LocalCertificate(void);

		/**
		 * Returns the path of the file, where the certificate is
		 * stored.
		 *
		 * The default-path is <code>~/.xanoubis/default.crt</code>.
		 * If such a file exists, the path is automatically filled
		 * during creation of the instance.
		 *
		 * @return Path of file, where the certificate is stored.
		 */
		wxString getFile(void) const;

		/**
		 * Updates the path of the file, where the certificate is
		 * stored.
		 *
		 * Make sure to call this method before you load a key (using
		 * the load()-method). A new path takes affect with the next
		 * call of load().
		 *
		 * @param file Path of file, where the certificate is stored.
		 */
		void setFile(const wxString &);

		/**
		 * Returns the key-id of the certificate.
		 *
		 * This is an unique identifier of a certificate.
		 *
		 * @return The key-id of the certificate if a certificate is
		 *         loaded.
		 * @see load()
		 */
		wxString getKeyId(void) const;

		/**
		 * Returns the fingerprint of the certificate.
		 *
		 * The method only returns a valid fingerprint if a certificate
		 * is loaded!
		 *
		 * @return Fingerprint of the currently loaded certificate.
		 * @see load()
		 */
		wxString getFingerprint(void) const;

		/**
		 * Returns the validity.
		 *
		 * The method only returns the validity if a certificate
		 * is loaded.
		 *
		 * @return The validity of the currently loaded certificate.
		 * @see load()
		 */
		wxDateTime getValidity(void) const;

		/**
		 * Returns the distinguished name assigned to the certificate.
		 *
		 * The method only returns a valid DN if a certificate
		 * is loaded.
		 *
		 * @return The DN of the currently loaded certificate.
		 * @see load()
		 */
		wxString getDistinguishedName(void) const;

		/**
		 * Returns the country name of the certificate in question.
		 *
		 * @return The country name of the currently loaded certificate
		 * or wxEmptyString.
		 * @see load()
		 */
		wxString getCountry(void) const;

		/**
		 * Returns the state or province of the certificate in question.
		 *
		 * @return The state or province of the currently loaded
		 *	   certificate or wxEmptyString.
		 * @see load()
		 */
		wxString getState(void) const;

		/**
		 * Returns the locality of the certificate in question.
		 *
		 * @return The locality of the currently loaded certificate
		 * or wxEmptyString.
		 * @see load()
		 */
		wxString getLocality(void) const;

		/**
		 * Returns the organization of the certificate in question.
		 *
		 * @return The organization of the currently loaded certificate
		 * or wxEmptyString.
		 * @see load()
		 */
		wxString getOrganization(void) const;

		/**
		 * Returns the organizational unit of the certificate in
		 * question.
		 *
		 * @return The organizational unit of the currently loaded
		 *	   certificate or wxEmptyString.
		 * @see load()
		 */
		wxString getOrganizationalUnit(void) const;

		/**
		 * Returns the common name of the certificate in question.
		 *
		 * @return The common name of the currently loaded certificate
		 * or wxEmptyString.
		 * @see load()
		 */
		wxString getCommonName(void) const;

		/**
		 * Returns the email address of the certificate in question.
		 *
		 * @return The email address of the currently loaded
		 *	   certificate or wxEmptyString.
		 * @see load()
		 */
		wxString getEmailAddress(void) const;

		/**
		 * Returns the raw-certificate.
		 *
		 * The method will return the result of a successful
		 * load-operation.
		 *
		 * @return The raw-certificate. If no certificate is loaded, 0
		 *         is returned.
		 */
		struct anoubis_sig *getCertificate(void) const;

		/**
		 * Tests whether a certificate can be loaded.
		 *
		 * A certificate can be loaded, if a file is assigned to the
		 * class. If the method returns true, you cab proceed with
		 * load().
		 *
		 * @return true if a certificate can be loaded.
		 */
		bool canLoad(void) const;

		/**
		 * Tests whether a certificate is currently loaded.
		 *
		 * A certificate is loaded if a load()-call was successful.
		 *
		 * @return true is returned, if a certificate is currently
		 *         loaded.
		 */
		bool isLoaded(void) const;

		/**
		 * Loads the X.509-certificate
		 *
		 * The certificate is read from the file configured with
		 * setFile().
		 *
		 * A previously loaded certificate is destroyed before.
		 *
		 * @return true on success, false otherwise.
		 */
		bool load(void);

		/**
		 * Unloads a certificate.
		 *
		 * @return Nothing.
		 */
		void unload(void);

	private:
		/**
		 * To protect the copy consturcter
		 */
		LocalCertificate(const LocalCertificate &) {}

		/**
		 * Path to file, where the certificate is stored.
		 */
		wxString certFile_;

		/**
		 * Distinguished Name of certificate
		 */
		wxString disName_;

		/**
		 * Struct containg fields provided by Distinguished Name
		 * of certificate
		 */
		struct anoubis_keysubject *subject_;

		/**
		 * The certificate itself. Can be 0.
		 */
		struct anoubis_sig *cert_;

		/**
		 * The unique key-id.
		 */
		wxString keyId_;
};

#endif	/* _LOCALCERTIFICATE_H_ */
