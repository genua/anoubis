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
#include "AppPolicy.h"

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
	for (size_t i=0; i<len*2 && i<workOn.length(); i++) {
		c = workOn.GetChar(i);

		if (('0' <= c) && (c <= '9')) {
			d = c - '0';
		} else if (('A' <= c) && (c <= 'F')) {
			d = c - 'A' + 10;
		} else if (('a' <= c) && (c <= 'f')) {
			d = c - 'a' + 10;
		} else {
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
PolicyUtils::addToAppListAt(struct apn_app **appList, unsigned int pos,
    wxString name)
{
	struct apn_app *app;

	if (appList == NULL)
		return false;
	while(pos && (*appList)) {
		appList = &(*appList)->next;
		pos--;
	}
	app = AppPolicy::createApnApp();
	if (!app)
		return false;
	app->name = strdup(name.To8BitData());
	if (!app->name) {
		free(app);
		return false;
	}
	app->next = *appList;
	*appList = app;
	return true;
}

bool
PolicyUtils::removeFromAppListAt(struct apn_app **appList, unsigned int pos)
{
	struct apn_app	*app;

	if (appList == NULL)
		return false;
	while(pos && (*appList)) {
		appList = &(*appList)->next;
		pos--;
	}
	app = *appList;
	if (app == NULL)
		return false;
	*appList = app->next;
	app->next = NULL;
	apn_free_app(app);
	return true;
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

bool
PolicyUtils::fileToCsum(const wxString file, wxString &csum)
{
	u_int8_t	csraw[MAX_APN_HASH_LEN];
	int		cslen = MAX_APN_HASH_LEN;

	csum = wxT("");
	if (anoubis_csum_calc(file.fn_str(), csraw, &cslen) == 0) {
		return csumToString(csraw, cslen, csum);
	}
	if (anoubis_csum_calc_userspace(file.fn_str(), csraw, &cslen) == 0) {
		return csumToString(csraw, cslen, csum);
	}
	return false;
}
