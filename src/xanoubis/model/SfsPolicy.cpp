/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/param.h>
#include <sys/socket.h>


#ifndef LINUX
#include <sys/queue.h>
#include <sha2.h>
#else
#include <queue.h>
#include <openssl/sha.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wx/file.h>
#include <wx/intl.h>

#include <apn.h>

#include "Policy.h"
#include "PolicyVisitor.h"
#include "AppPolicy.h"

#include "SfsPolicy.h"

IMPLEMENT_DYNAMIC_CLASS(SfsPolicy, Policy)

SfsPolicy::SfsPolicy(void)
{
}

SfsPolicy::SfsPolicy(AppPolicy *parent, struct apn_sfsrule *sfsRule)
    : Policy(parent)
{
	sfsRule_ = sfsRule;
	currHash_ = wxEmptyString;
	currSum_ = NULL;
	modified_ = false;
}

SfsPolicy::~SfsPolicy()
{
}

void
SfsPolicy::accept(PolicyVisitor& visitor)
{
	visitor.visitSfsPolicy(this);
}

int
SfsPolicy::getId(void)
{
	return (sfsRule_->apn_id);
}

void
SfsPolicy::setBinaryName(wxString name)
{
	struct apn_app	*app;

	app = sfsRule_->rule.sfscheck.app;

	free(app->name);
	app->name = strdup(name.fn_str());
	modified_ = true;
}

wxString
SfsPolicy::getBinaryName(void)
{
	struct apn_app	*app;

	app = sfsRule_->rule.sfscheck.app;

	if ((app != NULL) && (app->name != NULL)) {
		return (wxString::From8BitData(app->name));
	} else {
		return (wxEmptyString);
	}
}

unsigned char*
SfsPolicy::getCurrentSum(void)
{
	return (currSum_);
}

void
SfsPolicy::setCurrentSum(unsigned char *csum)
{
	int len;

	if (!currSum_) {
		currSum_ = (unsigned char*)malloc(MAX_APN_HASH_LEN);
		if (!currSum_)
			return;
	}

	switch (sfsRule_->rule.sfscheck.app->hashtype) {
	case APN_HASH_SHA256:
		len = APN_HASH_SHA256_LEN;
		break;
	default:
		len = 0;
		break;
	}

	bcopy(csum, currSum_, len);
}

wxString
SfsPolicy::getHashTypeName(void)
{
	wxString	 result;
	struct apn_app	*app;

	app = sfsRule_->rule.sfscheck.app;

	switch (app->hashtype) {
	case APN_HASH_SHA256:
		result = wxT("SHA256");
		break;
	default:
		result = _("(unknown)");
		break;
	}

	return (result);
}

wxString
SfsPolicy::getCurrentHash(void)
{
	if (currHash_.IsEmpty()) {
		return _("unknown");
	} else {
		return (currHash_);
	}
}

void
SfsPolicy::setCurrentHash(wxString currHash)
{
	sfsRule_->rule.sfscheck.app->hashtype = APN_HASH_SHA256;
	currHash_ = currHash;
}

int
SfsPolicy::calcCurrentHash(unsigned char csum[MAX_APN_HASH_LEN])
{
	SHA256_CTX	 shaCtx;
	u_int8_t	 buf[4096];
	size_t		 ret;
	wxFile		*file;
	struct stat	fileStat;

	/* At first we looking for any UNIX file permissions	*/
	if (stat((const char *)getBinaryName().mb_str(wxConvLocal),
	    &fileStat) < 0)
		return -1;

	if (! (fileStat.st_mode & S_IRUSR))
		return -2;

	/* Now we looking if Sfs let us access the file		*/
	if (wxFile::Exists(getBinaryName().c_str())) {
		if (!wxFile::Access(getBinaryName().c_str(), wxFile::read))
			return (0);
	} else {
		return -1;
	}

	file = new wxFile(getBinaryName().c_str());
	bzero(csum, MAX_APN_HASH_LEN);

	if (file->IsOpened()) {
		SHA256_Init(&shaCtx);
		while(1) {
			ret = file->Read(buf, sizeof(buf));
			if (ret == 0) {
				break;
			}
			if (ret == (size_t)wxInvalidOffset) {
				return (-1);
			}
			SHA256_Update(&shaCtx, buf, ret);
		}
		SHA256_Final(csum, &shaCtx);
		file->Close();
	}

	sfsRule_->rule.sfscheck.app->hashtype = APN_HASH_SHA256;
	currHash_ = this->convertToString(csum);
	this->setCurrentSum(csum);

	return (1);
}

void
SfsPolicy::setHashValue(unsigned char csum[MAX_APN_HASH_LEN])
{
	int len;

	switch (sfsRule_->rule.sfscheck.app->hashtype) {
	case APN_HASH_SHA256:
		len = APN_HASH_SHA256_LEN;
		break;
	case APN_HASH_NONE:
		len = APN_HASH_SHA256_LEN;
		break;
	default:
		len = 0;
		break;
	}

	bcopy(csum, sfsRule_->rule.sfscheck.app->hashvalue, len);
	modified_ = true;
}

wxString
SfsPolicy::getHashValue(void)
{
	wxString	 result;
	struct apn_app	*app;

	app = sfsRule_->rule.sfscheck.app;

	result = convertToString(app->hashvalue);

	return (result);
}

bool
SfsPolicy::isModified(void)
{
	return (modified_);
}

void
SfsPolicy::setModified(bool modified)
{
	modified_ = modified;
}

wxString
SfsPolicy::convertToString(unsigned char *csum)
{
	wxString	result;
	unsigned int	length;

	length = 0;
	result = wxT("0x");

	switch (sfsRule_->rule.sfscheck.app->hashtype) {
	case APN_HASH_SHA256:
		length = APN_HASH_SHA256_LEN;
		break;
	case APN_HASH_NONE:
		length = 0;
		result = _("unknown");
		break;
	default:
		length = 0;
		result = _("(unknown hash type)");
		break;
	}

	for (unsigned int i=0; i<length; i++) {
		result += wxString::Format(wxT("%2.2x"),
		    (unsigned char)csum[i]);
	}

	return (result);
}

bool
SfsPolicy::isDefault(void)
{
	/* sfs policies are never default */
	return (false);
}

void
SfsPolicy::setLogNo(int logNo)
{
	sfsRule_->rule.sfscheck.log = logNo;
}

int
SfsPolicy::getLogNo(void)
{
	return sfsRule_->rule.sfscheck.log;
}

wxString
SfsPolicy::getLogName(void)
{
	return (SUPER(Policy)->getLogName(getLogNo()));
}
