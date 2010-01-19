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

#include <sys/types.h>
#include <sys/stat.h>

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include <errno.h>

#include <anoubis_csum.h>

#include "ComCsumHandler.h"

ComCsumHandler::ComCsumHandler(void)
{
	this->path_ = wxEmptyString;
	this->keyId_ = 0;
	this->keyIdLen_ = 0;
	this->calcLink_ = false;
}

ComCsumHandler::~ComCsumHandler(void)
{
	if (keyId_ != 0)
		free(keyId_);
}

wxString
ComCsumHandler::getPath(void) const
{
	return (this->path_);
}

void
ComCsumHandler::setPath(const wxString &path)
{
	this->path_ = path;
}

bool
ComCsumHandler::haveKeyId(void) const
{
	return ((keyId_ != 0) && (keyIdLen_ > 0));
}

bool
ComCsumHandler::setKeyId(const u_int8_t *keyId, int len)
{
	if ((keyId != 0) && (len > 0)) {
		u_int8_t *newKeyId = (u_int8_t *)malloc(len);

		if (newKeyId != 0)
			memcpy(newKeyId, keyId, len);
		else
			return (false);

		if (this->keyId_ != 0)
			free(this->keyId_);

		this->keyId_ = newKeyId;
		this->keyIdLen_ = len;

		return (true);
	} else
		return (false);
}

bool
ComCsumHandler::calcLink(void) const
{
	return (this->calcLink_);
}

void
ComCsumHandler::setCalcLink(bool enable)
{
	this->calcLink_ = enable;
}

u_int8_t *
ComCsumHandler::getKeyId(void) const
{
	return (this->keyId_);
}

int
ComCsumHandler::getKeyIdLen(void)
{
	return (this->keyIdLen_);
}

int
ComCsumHandler::csumCalc(u_int8_t *cs, int *cslen) const
{
	struct stat	fstat;

	if (lstat(path_.fn_str(), &fstat) == 0) {
		int result;

		if (S_ISLNK(fstat.st_mode) && calcLink_) {
			/*
			 * This is a symlink and calculation over linkname is
			 * requested.
			 */
			result = anoubis_csum_link_calc(
			    path_.fn_str(), cs, cslen);
		} else if (S_ISREG(fstat.st_mode) || S_ISLNK(fstat.st_mode)) {
			if (S_ISLNK(fstat.st_mode)) {
				/*
				 * If you have a symlink make sure that only
				 * regular files are referenced
				 */
				struct stat link_stat;

				if (stat(path_.fn_str(), &link_stat) == 0) {
					if (!S_ISREG(link_stat.st_mode))
						return (EINVAL);
				} else
					return (errno);
			}

			result = anoubis_csum_calc(path_.fn_str(), cs, cslen);
		} else {
			/*
			 * Operation supported only on regular files and
			 * symbolic links
			 */
			return (EINVAL);
		}

		/* Toggle sign of result-code, this is the correct errno */
		return (-result);
	} else
		return (errno);
}

bool
ComCsumHandler::resolvePath(char *resolved_path, bool noent) const
{
	struct stat fstat;

	if (lstat(path_.fn_str(), &fstat) == 0) {
		if (S_ISLNK(fstat.st_mode) && !calcLink_) {
			/*
			 * This is a symlink and linkname-calculation is
			 * disabled. In this case you want to register the
			 * checksum for the referenced file. The checksum is
			 * also associated with the resolved path at anoubisd.
			 */
			if (realpath(path_.fn_str(), resolved_path) == 0)
				return (false);
		} else
			strlcpy(resolved_path, path_.fn_str(), PATH_MAX);

		return (true);
	} else if (noent) {
		strlcpy(resolved_path, path_.fn_str(), PATH_MAX);
		return (true);
	} else
		return (false);
}
