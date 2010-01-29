/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#ifndef _SBENTRY_H_
#define _SBENTRY_H_

#include <AnListClass.h>

class SbModel;

/**
 * An sandbox-entry.
 *
 * This entry is able to assign file-permissions to a file. This is used
 * by rulewizard. There you are able to assign permissions to files.
 *
 * SbEntry-instances are maintained by SbModel.
 */
class SbEntry : public AnListClass
{
	public:
		/**
		 * Access-rights.
		 *
		 * The enumeration defined access-rights, which can be assigned
		 * to the sandbox-entry.
		 */
		enum Permission
		{
			READ = 0x1,	/*!< Read-permission. */
			WRITE = 0x2,	/*!< Write-permission. */
			EXECUTE = 0x4	/*!< Execute-permission. */
		};

		/**
		 * Returns the path.
		 *
		 * @return The path
		 */
		wxString getPath(void) const;

		/**
		 * Updates the path of this sandbox-entry.
		 *
		 * @param path The new path
		 */
		void setPath(const wxString &);

		/**
		 * Tests whether the sandbox-entry has the given access-right.
		 *
		 * @param permission The requested permission
		 * @return true if the sandbox-entry has the requested
		 *         access-right, false otherwise.
		 */
		bool hasPermission(Permission) const;

		/**
		 * Assigns an access-right to the sandbox-entry.
		 *
		 * @param permission The permission to be updated
		 * @param value If set to true, the permission is assigned,
		 *              if set to false, it is removed from the
		 *              sandbox-entry.
		 */
		void setPermission(Permission, bool);

		/**
		 * Flag shows if the sandbox-entry was created from the
		 * default-configuration.
		 *
		 * @return true is returned, if this is a default-entry.
		 * @see SbModel::assignDefaults()
		 */
		bool isDefault(void) const;

	private:
		/**
		 * Std-c'tor.
		 * Made private because only SbModel should be able to maintain
		 * instances of this class.
		 */
		SbEntry(void) {}

		/**
		 * Creates a SbEntry-instance.
		 * The instance is assigned to the given model.
		 *
		 * @param model The model to be assigned
		 */
		SbEntry(SbModel *);

		/**
		 * Copy-c'tor.
		 * Made private because only SbModel should be able to maintain
		 * instances of this class.
		 */
		SbEntry(const SbEntry &) : AnListClass() {}

		/**
		 * D'tor.
		 * Made private because only SbModel should be able to maintain
		 * instances of this class.
		 */
		~SbEntry(void) {}

		/**
		 * The parent model-instance.
		 */
		SbModel *parent_;

		/**
		 * The path.
		 */
		wxString path_;

		/**
		 * Permissions bit-field.
		 */
		int permissions_;

		/**
		 * Comes the entry from the default-configuration?
		 */
		bool default_;

	friend class SbModel;
};

#endif	/* _SBENTRY_H_ */
