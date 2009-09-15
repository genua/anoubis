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
PolicyUtils::getSubjectName(const struct apn_subject *subject)
{
	wxString	 subjectName = wxEmptyString;

	subjectName = wxEmptyString;
	if (subject != NULL) {
		switch (subject->type) {
		case APN_CS_KEY_SELF:
			subjectName = wxT("signed-self");
			break;
		case APN_CS_UID_SELF:
			subjectName = wxT("self");
			break;
		case APN_CS_KEY:
			subjectName = wxString::From8BitData(
			    subject->value.keyid);
			subjectName.Prepend(wxT("key "));
			break;
		case APN_CS_UID:
			subjectName.Printf(wxT("uid %d"),
			    subject->value.uid);
			break;
		case APN_CS_NONE:
			subjectName = wxT("none");
			break;
		default:
			break;
		}
	}

	return (subjectName);
}

void
PolicyUtils::cleanSubject(struct apn_subject *subject)
{
	if (!subject)
		return;
	switch (subject->type) {
	case APN_CS_KEY:
		free(subject->value.keyid);
		subject->value.keyid = NULL;
		break;
	case APN_CS_UID:
		subject->value.uid = 0 - 1;
		break;
	default:
		break;
	}
	subject->type = APN_CS_NONE;
}
