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

#include "PolicyUtils.h"

#include <wx/tokenzr.h>

bool
PolicyUtils::stringToCsum(wxString str,
    unsigned char csum[MAX_APN_HASH_LEN], size_t len)
{
	wxString workOn;

	memset(csum, 0, len);

	/* Ensure no leading '0x' or '0X' */
	if (!str.StartsWith(wxT("0x"), &workOn) &&
	    !str.StartsWith(wxT("0X"), &workOn)) {
		workOn = str;
	}

	memcpy(csum, workOn.To8BitData(), len);

	return (true);
}

bool
PolicyUtils::csumToString(unsigned char csum[MAX_APN_HASH_LEN],
    size_t len, wxString str)
{
	str.Clear();

	str = wxT("0x");
	for (size_t i = 0; i < len; i++) {
		str += wxString::Format(wxT("%2.2x"),
		    (unsigned char)csum[i]);
	}

	return (true);
}

/*
 * XXX ch: Is this method still needed?
 * XXX ch: anoubis_csum_calc() requests csum from kernel
 */
int
PolicyUtils::calculateHash(wxString binary,
    unsigned char csum[MAX_APN_HASH_LEN], size_t length)
{
	size_t		 ret;
	struct stat	 fileStat;
	SHA256_CTX	 shaCtx;
	u_int8_t	 buf[4096];
	wxFile		*file;

	/* At first we looking for any UNIX file permissions */
	/* XXX ch: this was before: (const char *)binary.mb_str(wxConvLocal) */
	if (stat(binary.To8BitData(), &fileStat) < 0) {
		return (-1);
	}

	if (! (fileStat.st_mode & S_IRUSR)) {
		return (-2);
	}

	/* Now we looking if Sfs let us access the file	*/
	if (wxFileExists(binary)) {
		/*if (!wxFile::Access(getBinaryName().c_str(), wxFile::read))*/
		if (!wxFileName::IsFileReadable(binary)) {
			return (0);
		}
	} else {
		return (-1);
	}

	file = new wxFile(binary.c_str());
	memset(csum, 0, length);

	if (file->IsOpened()) {
		SHA256_Init(&shaCtx);
		while (1) {
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

	return (1);
}

bool
PolicyUtils::setAppList(struct apn_app **appList, wxArrayString list)
{
	struct apn_app *app;
	const size_t	listSize = list.GetCount() - 1;

	/* Check parameters */
	if (list.IsEmpty() || (appList == NULL)) {
		return (false);
	}

	/* If there's already a list, remove the elements. */
	if (*appList != NULL) {
		apn_free_app(*appList);
		*appList = NULL;
	}

	/* If we shall set 'any', we're done. */
	if ((list.GetCount() == 1) && (list.Item(0).Cmp(wxT("any")) == 0)) {
		return (true);
	}

	*appList = AppPolicy::createApnApp();
	if (*appList == NULL) {
		return (false);
	}
	app = *appList;
	for (size_t i=0; i <= listSize; i++) {
		app->name = strdup(list.Item(i).To8BitData());
		/* We have to create a 'next', except for the last one. */
		if (i < listSize) {
			app->next = AppPolicy::createApnApp();
			if (app->next == NULL) {
				return (false);
			}
			app = app->next;
		}
	}

	return (true);
}

wxArrayString
PolicyUtils::getAppList(struct apn_app *appList)
{
	wxArrayString	 list;
	struct apn_app	*app;

	list.Clear();

	/* Is list element == 'any'? */
	if (appList == NULL) {
		list.Add(wxT("any"));
	} else {
		app = appList;
		/* Walk down the list ... */
		while (app != NULL) {
			list.Add(wxString::From8BitData(app->name));
			app = app->next;
		}
	}

	return (list);
}

wxString
PolicyUtils::listToString(wxArrayString list)
{
	wxString result;

	if (list.IsEmpty()) {
		result = wxEmptyString;
	} else {
		result = list.Item(0);
	}

	for (size_t i = 1; i < list.GetCount(); i++) {
		result += wxT(", ");
		result += list.Item(i);
	}
	if (list.GetCount() > 1) {
		result.Prepend(wxT("{"));
		result.Append(wxT("}"));
	}

	return (result);
}

wxArrayString
PolicyUtils::stringToList(wxString str)
{
	wxString		workOn = str;
	wxString		token;
	wxArrayString		list;
	wxStringTokenizer	tokenizer;

	/* Remove leading and trailing whithe space char's */
	workOn.Trim(true);
	workOn.Trim(false);

	if (workOn.StartsWith(wxT("{")) && workOn.EndsWith(wxT("}"))) {
		/* This is a truely list. */
		tokenizer.SetString(workOn.Mid(1,workOn.Len() - 1), wxT(","));
		while (tokenizer.HasMoreTokens()) {
			token = tokenizer.GetNextToken();
			token.Trim(true);
			token.Trim(false);
			list.Add(token);
		}
	} else {
		list.Add(str);
	}

	return (list);
}
