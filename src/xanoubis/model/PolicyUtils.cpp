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
	wxChar c;
	unsigned char d;

	memset(csum, 0, len);

	/* Ensure no leading '0x' or '0X' */
	if (!str.StartsWith(wxT("0x"), &workOn) &&
	    !str.StartsWith(wxT("0X"), &workOn)) {
		workOn = str;
	}

	/* XXX ch: this is a quick-hack and should been improved */
	for (size_t i=0; i<len*2; i++) {
		c = workOn.GetChar(i);

		if ((47 < c) && (c < 58)) {
			// is digit
			d = c - 48;
		} else if ((64 < c) && (c < 71)) {
			// is A - F
			d = c - 55;
		} else if ((96 < c) && (c < 103)) {
			// is a - f
			d = c - 87;
		} else {
			// not in range
			d = 0;
		}
		if ((i%2) == 0) {
			csum[i/2] = d << 4;
		} else {
			csum[i/2] += d;
		}
	}

	return (true);
}

bool
PolicyUtils::csumToString(unsigned char csum[MAX_APN_HASH_LEN],
    size_t len, wxString &str)
{
	if (len == 0 || len > MAX_APN_HASH_LEN)
		return (false);
	str.Clear();
	for (size_t i = 0; i < len; i++) {
		str += wxString::Format(wxT("%2.2x"),
		    (unsigned char)csum[i]);
	}

	return (true);
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

	/* Remove leading and trailing white space char's */
	workOn.Trim(true);
	workOn.Trim(false);

	/* Accept list both with and without curly braces. */
	if (workOn.StartsWith(wxT("{")) && workOn.EndsWith(wxT("}"))) {
		tokenizer.SetString(workOn.Mid(1,workOn.Len() - 2), wxT(","));
	} else {
		tokenizer.SetString(workOn, wxT(","));
	}
	while (tokenizer.HasMoreTokens()) {
		token = tokenizer.GetNextToken();
		token.Trim(true);
		token.Trim(false);
		list.Add(token);
	}
	return (list);
}

wxString
PolicyUtils::hashTypeToString(int hashType)
{
	wxString hashTypeString;

	switch (hashType) {
	case APN_HASH_SHA256:
		hashTypeString = wxT("SHA256");
		break;
	case APN_HASH_NONE:
		hashTypeString = wxT("NONE");
		break;
	default:
		hashTypeString = _("(unknown)");
		break;
	}

	return (hashTypeString);
}
