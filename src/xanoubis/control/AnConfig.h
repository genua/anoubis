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

#ifndef _ANCONFIG_H_
#define _ANCONFIG_H_

#include <wx/config.h>

/**
 * Anoubis configuration class.
 *
 * The configuration-options are stored under
 * <code>$HOME/.xanoubis/xanoubis.conf</code>.
 *
 * This class automatically @link Flush() flushes @endlink the configuration
 * into the configuration file after each successful
 * @link Write() Write()-operation @endlink.
 *
 * Furthermore the class emits a wxCommandEvent of type
 * anEVT_ANOUBISOPTIONS_UPDATE after each successful
 * @link Write() Write()-operation @endlink. So, any lister can react on a
 * configuration change.
 */
class AnConfig : public wxFileConfig
{
	public:
		/**
		 * Constructs an AnConfig-instance.
		 * @param appName Name of application
		 */
		AnConfig(const wxString &);

		/**
		 * Destructor.
		 * Flushes the configuration a last time.
		 */
		~AnConfig(void);

		/**
		 * Flushes the configuration (if necessary).
		 *
		 * On success a wxCommandEvent of type
		 * anEVT_ANOUBISOPTIONS_UPDATE is emitted.
		 *
		 * @param bCurrentOnly Passed to wxFileConfig::Flush()
		 * @return true on success, false otherwise.
		 */
		virtual bool Flush(bool bCurrentOnly = false);

	protected:
		//@{
		/**
		 * Writes a configuration value.
		 *
		 * If the new value differs from the old one, Flush() is
		 * automatically called.
		 *
		 * @param key The key of the option
		 * @param value The new value
		 * @return true on success, false otherwise.
		 */
		virtual bool DoWriteString(const wxString &, const wxString &);
		virtual bool DoWriteLong(const wxString &, long);
		virtual bool DoWriteInt(const wxString &, int);
		virtual bool DoWriteDouble(const wxString &, double);
		virtual bool DoWriteBool(const wxString &, bool);
		//@}

	private:
		/**
		 * Flag defines, if the configuration needs to be flushed.
		 */
		bool needFlush_;
};

#endif	/* _ANCONFIG_H_ */
