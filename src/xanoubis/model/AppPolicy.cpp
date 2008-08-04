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

#include "Policy.h"
#include "AlfPolicy.h"
#include "PolicyVisitor.h"

#include "AppPolicy.h"

IMPLEMENT_DYNAMIC_CLASS(AppPolicy, Policy)

AppPolicy::AppPolicy(void)
{
}

AppPolicy::AppPolicy(struct apn_rule *appRule) : Policy(NULL)
{
	struct apn_alfrule	*alfRule;
	struct apn_sfsrule	*sfsRule;
	AlfPolicy		*newAlf;
	SfsPolicy		*newSfs;

	context_ = NULL;
	appRule_ = appRule;
	currHash_ = wxEmptyString;
	currSum_ = NULL;
	appName_ = wxEmptyString;
	modified_ = false;

	switch (appRule_->type) {
	case APN_ALF:
		alfRule = appRule_->rule.alf;
		while (alfRule) {
			newAlf = new AlfPolicy(this, alfRule);
			ruleList_.Append(newAlf);
			if (alfRule->type == APN_ALF_CTX) {
				context_ = newAlf;
			}
			alfRule = alfRule->next;
		}
		break;
	case APN_SFS:
		sfsRule = appRule_->rule.sfs;
		while (sfsRule) {
			newSfs = new SfsPolicy(this, sfsRule);
			ruleList_.Append(newSfs);
			sfsRule = sfsRule->next;
		}
		break;
	case APN_SB:
	case APN_VS:
	default:
		break;
	}
}

AppPolicy::~AppPolicy(void)
{
	ruleList_.DeleteContents(true);
	/* the apn structure will be deleted by PolicyRuleSet */
}

void
AppPolicy::accept(PolicyVisitor& visitor)
{
	PolicyList::iterator	 i;

	visitor.visitAppPolicy(this);

	if (visitor.shallBeenPropagated() == true) {
		for (i=ruleList_.begin(); i != ruleList_.end(); ++i) {
			(*i)->accept(visitor);
		}
	}
}

wxString
AppPolicy::getApplicationName(void)
{
	return (appName_);
}

void
AppPolicy::setApplicationName(wxString appName)
{
	appName_ = appName;
	modified_ = true;
}

void
AppPolicy::setBinaryName(wxString name)
{
	struct apn_app  *app;

	app = appRule_->app;

	if(app == NULL) {
		app = (struct apn_app *)calloc(1, sizeof(struct apn_app));
		if (app == NULL)
			return;

		appRule_->app = app;
		app->hashtype = APN_HASH_SHA256;
	} else
		free(app->name);

	app->name = strdup(name.fn_str());
	modified_ = true;
}

wxString
AppPolicy::getBinaryName(void)
{
	if (appRule_->app == NULL)
		return wxString::From8BitData("any");
	return wxString::From8BitData(appRule_->app->name);
}

unsigned char*
AppPolicy::getCurrentSum(void)
{
	return (currSum_);
}

void
AppPolicy::setCurrentSum(unsigned char *csum)
{
	int len;

	if (!currSum_) {
		currSum_ = (unsigned char*)malloc(MAX_APN_HASH_LEN);
		if (!currSum_)
			return;
	}

	switch (appRule_->app->hashtype) {
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
AppPolicy::getCurrentHash(void)
{
	if (currHash_.IsEmpty()) {
		return _("unknown");
	} else {
		return (currHash_);
	}
}

void
AppPolicy::setCurrentHash(wxString currHash)
{
	appRule_->app->hashtype = APN_HASH_SHA256;
	currHash_ = currHash;
}

int
AppPolicy::calcCurrentHash(unsigned char csum[MAX_APN_HASH_LEN])
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

	appRule_->app->hashtype = APN_HASH_SHA256;
	currHash_ = convertToString(csum);

	this->setCurrentSum(csum);

	return (1);
}

wxString
AppPolicy::getHashTypeName(void)
{
	wxString result;

	if (appRule_->app == NULL)
		return wxString::From8BitData("");
	switch (appRule_->app->hashtype) {
	case APN_HASH_SHA256:
		result = wxT("SHA256");
		break;
	default:
		result = _("(unknown)");
		break;
	}

	return (result);
}

void
AppPolicy::setHashValue(unsigned char csum[MAX_APN_HASH_LEN])
{
	int len;

	/* XXX: KM there should be a propper error handling */
	if (appRule_->app == NULL)
		return;

	switch (appRule_->app->hashtype) {
	case APN_HASH_SHA256:
		len = APN_HASH_SHA256_LEN;
		break;
	case APN_HASH_NONE:
		appRule_->app->hashtype = APN_HASH_SHA256;
		len = APN_HASH_SHA256_LEN;
		break;
	default:
		len = 0;
		break;
	}

	bcopy(csum, appRule_->app->hashvalue, len);
	modified_ = true;
}

wxString
AppPolicy::getHashValue(void)
{
	wxString	result;

	result = convertToString(appRule_->app->hashvalue);

	return (result);
}

AlfPolicy *
AppPolicy::getContext(void)
{
	return (context_);
}

bool
AppPolicy::hasContext(void)
{
	if (context_ == NULL) {
		return (false);
	} else {
		return (true);
	}
}

int
AppPolicy::getType(void)
{
	return(appRule_->type);
}

bool
AppPolicy::isModified(void)
{
	return (modified_);
}

void
AppPolicy::setModified(bool modified)
{
	modified_ = modified;
}

int
AppPolicy::getId(void)
{
	return(appRule_->id);
}

wxString
AppPolicy::convertToString(unsigned char *csum)
{
	wxString	result;
	unsigned int	length;

	length = 0;
	result = wxT("0x");

	if (appRule_->app == NULL)
		return (_("any"));

	switch (appRule_->app->hashtype) {
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
AppPolicy::isDefault(void)
{
	return (appRule_->app == NULL); /* aka any rule */
}
