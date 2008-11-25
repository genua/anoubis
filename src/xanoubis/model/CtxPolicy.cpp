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

#include <sys/types.h>
#include <sys/stat.h>

#ifndef LINUX
#include <sys/queue.h>
#include <sha2.h>
#else
#include <queue.h>
#endif

#include <openssl/sha.h>

#include <wx/string.h>
#include <wx/intl.h>
#include <wx/datetime.h>
#include <wx/file.h>

#include <apn.h>
#include <csum/csum.h>

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include "Policy.h"
#include "AppPolicy.h"
#include "CtxPolicy.h"
#include "PolicyVisitor.h"

IMPLEMENT_DYNAMIC_CLASS(CtxPolicy, Policy)

CtxPolicy::CtxPolicy(PolicyRuleSet *parent) : Policy(parent)
{

}

CtxPolicy::CtxPolicy(void) : Policy(NULL, NULL)
{
}

CtxPolicy::CtxPolicy(AppPolicy *parent, struct apn_rule *ctxRule,
    PolicyRuleSet *rsParent) : Policy(rsParent, parent)
{
	ctxRule_ = ctxRule;
	currHash_ = wxEmptyString;
	currSum_ = NULL;
	modified_ = false;
}

CtxPolicy::~CtxPolicy(void)
{
	/*
	 * The parent can destroy itself and the apn structure is cleand by
	 * PolicyRuleSet.
	 */
}

void
CtxPolicy::accept(PolicyVisitor &visitor)
{
	visitor.visitCtxPolicy(this);
}

int
CtxPolicy::getId(void)
{
	return (ctxRule_->apn_id);
}

int
CtxPolicy::getTypeNo(void)
{
	return ctxRule_->apn_type;
}

wxString
CtxPolicy::getBinaryName(void)
{
	if (ctxRule_->rule.apncontext.application == NULL)
		return wxString::From8BitData("any");
	return wxString::From8BitData(
	    ctxRule_->rule.apncontext.application->name);
}

wxString
CtxPolicy::getHashTypeName(void)
{
	wxString result;

	if (ctxRule_->rule.apncontext.application == NULL)
		return wxString::From8BitData("");
	switch (ctxRule_->rule.apncontext.application->hashtype) {
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
CtxPolicy::getHashValue(void)
{
	wxString	ret;

	if (ctxRule_->rule.apncontext.application == NULL)
		return wxString::From8BitData("");
	ret = convertToString(
	    ctxRule_->rule.apncontext.application->hashvalue);

	return (ret);
}

wxString
CtxPolicy::convertToString(unsigned char *csum)
{
	wxString	 result;
	unsigned int	 length = 0;

	result = wxT("0x");

	if (ctxRule_->rule.apncontext.application == NULL)
		return (_("any"));
	switch (ctxRule_->rule.apncontext.application->hashtype) {
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

void
CtxPolicy::setBinaryName(wxString name)
{
	struct apn_app  *app;

	app = ctxRule_->rule.apncontext.application;

	if (name == wxT("any")) {
		ctxRule_->rule.apncontext.application = NULL;
		if (app)
			apn_free_app(app);
		modified_ = true;
		return;
	}
	if(app == NULL) {
		app = (struct apn_app *)calloc(1, sizeof(struct apn_app));
		if (app == NULL)
			return;

		ctxRule_->rule.apncontext.application = app;
		app->hashtype = APN_HASH_SHA256;
	} else
		free(app->name);

	app->name = strdup(name.fn_str());
	modified_ = true;
}

bool
CtxPolicy::isAny(void)
{
	return (ctxRule_->rule.apncontext.application == NULL);
}

bool
CtxPolicy::isModified(void)
{
	return modified_;
}

void
CtxPolicy::setModified(bool value)
{
	modified_ = value;
}

void
CtxPolicy::setHashValue(unsigned char csum[MAX_APN_HASH_LEN])
{
	int		 len;
	struct apn_app	*app;

	/* XXX: KM there should be a propper error handling */
	app = ctxRule_->rule.apncontext.application;
	if (app == NULL)
		return;

	switch (app->hashtype) {
	case APN_HASH_SHA256:
		len = APN_HASH_SHA256_LEN;
		break;
	case APN_HASH_NONE:
		app->hashtype = APN_HASH_SHA256;
		len = APN_HASH_SHA256_LEN;
		break;
	default:
		len = 0;
		break;
	}

	bcopy(csum, app->hashvalue, len);
	modified_ = true;
}

unsigned char*
CtxPolicy::getCurrentSum(void)
{
	return (currSum_);
}

void
CtxPolicy::setCurrentSum(unsigned char *csum)
{
	int len;
	struct apn_app	*app;

	if (!currSum_) {
		currSum_ = (unsigned char*)malloc(MAX_APN_HASH_LEN);
		if (!currSum_)
			return;
	}
	app = ctxRule_->rule.apncontext.application;
	switch (app->hashtype) {
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
CtxPolicy::getCurrentHash(void)
{
	if (currHash_.IsEmpty()) {
		return _("unknown");
	} else {
		return (currHash_);
	}
}

void
CtxPolicy::setCurrentHash(wxString currHash)
{
	struct apn_app	*app = ctxRule_->rule.apncontext.application;
	if (app)
		app->hashtype = APN_HASH_SHA256;
	currHash_ = currHash;
}

int
CtxPolicy::calcCurrentHash(unsigned char csum[MAX_APN_HASH_LEN])
{
	SHA256_CTX	 shaCtx;
	u_int8_t	 buf[4096];
	size_t		 ret;
	wxFile		*file;
	struct stat	 fileStat;

	/* At first we looking for any UNIX file permissions	*/
	if (stat((const char *)getBinaryName().mb_str(wxConvLocal),
	    &fileStat) < 0)
	    return -1;

	if (! (fileStat.st_mode & S_IRUSR))
		return -2;

	/* Now we looking if Sfs let us access the file		*/
	if (wxFile::Exists(getBinaryName().c_str())) {
		if (!wxFile::Access(getBinaryName().c_str(), wxFile::read))
			return 0;
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

	ctxRule_->rule.apncontext.application->hashtype = APN_HASH_SHA256;
	currHash_ = convertToString(csum);

	this->setCurrentSum(csum);

	return (1);
}
