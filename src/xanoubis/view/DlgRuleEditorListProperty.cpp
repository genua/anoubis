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

#include "main.h"
#include "Debug.h"
#include "DlgRuleEditorListProperty.h"

#define EXTRACT_ID(text, policy) \
	do { \
		text = wxString::Format(wxT("%d:"), policy->getApnRuleId()); \
	} while (0)

#define EXTRACT_TYPE(text, ruleSet, policy) \
	do { \
		text = policy->getTypeIdentifier(); \
		if (ruleSet && ruleSet->isAdmin()) { \
			text.Append(wxT("(A)")); \
		} \
	} while (0)


DlgRuleEditorListProperty::DlgRuleEditorListProperty(PropertyType type)
{
	type_ = type;
}

wxString
DlgRuleEditorListProperty::getHeader(void) const
{
	wxString header;

	switch (type_) {
	case PROPERTY_ID:	header = _("Id");		break;
	case PROPERTY_TYPE:	header = _("Type");		break;
	case PROPERTY_USER:	header = _("User");		break;
	case PROPERTY_DETAILS:	header = _("Details");		break;
	case PROPERTY_BINARY:	header = _("Binary");		break;
	case PROPERTY_ACTION:	header = _("Action");		break;
	case PROPERTY_LOG:	header = _("Log");		break;
	case PROPERTY_SCOPE:	header = _("Scope");		break;
	case PROPERTY_PATH:	header = _("Path");		break;
	case PROPERTY_SUB:	header = _("Subject");		break;
	case PROPERTY_CAP:	header = _("Capability");	break;
	case PROPERTY_DIR:	header = _("Direction");	break;
	case PROPERTY_PROT:	header = _("Protocol");		break;
	case PROPERTY_FHOST:	header = _("From host");	break;
	case PROPERTY_FPORT:	header = _("From port");	break;
	case PROPERTY_THOST:	header = _("To host");		break;
	case PROPERTY_TPORT:	header = _("To port");		break;
	case PROPERTY_VALACT:	header = _("Valid action");	break;
	case PROPERTY_VALLOG:	header = _("Valid log");	break;
	case PROPERTY_INVALACT:	header = _("Invalid action");	break;
	case PROPERTY_INVALLOG:	header = _("Invalid log");	break;
	case PROPERTY_UNKACT:	header = _("Unknown action");	break;
	case PROPERTY_UNKLOG:	header = _("Unknown log");	break;
	case PROPERTY_MASK:	header = _("Access mask");	break;
	}

	return (header);
}

wxString
DlgRuleEditorListProperty::getText(AnListClass *item) const
{
	wxString			 text;
	AppPolicy			*app;
	AlfFilterPolicy			*alfFilter;
	AlfCapabilityFilterPolicy	*alfCapability;
	SfsDefaultFilterPolicy		*sfsDefault;
	SfsFilterPolicy			*sfsFilter;
	DefaultFilterPolicy		*defaultFilter;
	ContextFilterPolicy		*ctxFilter;
	SbAccessFilterPolicy		*sbFilter;

	app = dynamic_cast<AppPolicy*>(item);
	alfFilter = dynamic_cast<AlfFilterPolicy*>(item);
	alfCapability = dynamic_cast<AlfCapabilityFilterPolicy*>(item);
	sfsDefault = dynamic_cast<SfsDefaultFilterPolicy*>(item);
	sfsFilter = dynamic_cast<SfsFilterPolicy*>(item);
	defaultFilter = dynamic_cast<DefaultFilterPolicy*>(item);
	ctxFilter = dynamic_cast<ContextFilterPolicy*>(item);
	sbFilter = dynamic_cast<SbAccessFilterPolicy*>(item);

	if (app != NULL) {
		text = examineAppPolicy(app);
	} else if (alfFilter != NULL) {
		text = examineAlfFilterPolicy(alfFilter);
	} else if (alfCapability!= NULL) {
		text = examineAlfCapabilityFilterPolicy(alfCapability);
	} else if (sfsDefault != NULL) {
		text = examineSfsDefaultFilterPolicy(sfsDefault);
	} else if (sfsFilter != NULL) {
		text = examineSfsFilterPolicy(sfsFilter);
	} else if (defaultFilter != NULL) {
		text = examineDefaultFilterPolicy(defaultFilter);
	} else if (ctxFilter != NULL) {
		text = examineContextFilterPolicy(ctxFilter);
	} else if (sbFilter != NULL) {
		text = examineSbAccessFilterPolicy(sbFilter);
	} else {
		text = _("???");
	}

	return (text);
}

AnIconList::IconId
DlgRuleEditorListProperty::getIcon(AnListClass *) const
{
	return (AnIconList::ICON_NONE);
}

wxString
DlgRuleEditorListProperty::examineAppPolicy(AppPolicy *app) const
{
	wxString	 text;
	PolicyRuleSet	*ruleSet;

	ruleSet = app->getParentRuleSet();

	switch (type_) {
	case PROPERTY_ID:
		EXTRACT_ID(text, app);
		break;
	case PROPERTY_TYPE:
		EXTRACT_TYPE(text, ruleSet, app);
		break;
	case PROPERTY_USER:
		if (ruleSet && (ruleSet->getUid() != (uid_t)-1)) {
			text = wxGetApp().getUserNameById(ruleSet->getUid());
		} else {
			text = wxT("default");
		}
		break;
	case PROPERTY_DETAILS:
		text = wxEmptyString;
		if (app->getFlag(APN_RULE_NOSFS)) {
			text = wxT("nosfs");
		}
		if (app->getFlag(APN_RULE_PLAYGROUND)) {
			if (!text.IsEmpty()) {
				text.Append(wxT(", "));
			}
			text.Append(wxT("playground"));
		}
		break;
	case PROPERTY_BINARY:
		text = app->getBinaryName();
		break;
	default:
		text = wxEmptyString;
		break;
	}

	return (text);
}

wxString
DlgRuleEditorListProperty::examineAlfFilterPolicy(AlfFilterPolicy
    *filter) const
{
	wxString	 text;
	PolicyRuleSet	*ruleSet;

	ruleSet = filter->getParentRuleSet();

	switch (type_) {
	case PROPERTY_ID:
		EXTRACT_ID(text, filter);
		break;
	case PROPERTY_TYPE:
		EXTRACT_TYPE(text, ruleSet, filter);
		break;
	case PROPERTY_ACTION:
		text = filter->getActionName();
		break;
	case PROPERTY_LOG:
		text = filter->getLogName();
		break;
	case PROPERTY_SCOPE:
		text = filter->getScopeName();
		break;
	case PROPERTY_DIR:
		text = filter->getDirectionName();
		break;
	case PROPERTY_PROT:
		text = filter->getProtocolName();
		break;
	case PROPERTY_FHOST:
		text = filter->getFromHostName();
		break;
	case PROPERTY_FPORT:
		text = filter->getFromPortName();
		break;
	case PROPERTY_THOST:
		text = filter->getToHostName();
		break;
	case PROPERTY_TPORT:
		text = filter->getToPortName();
		break;
	default:
		text = wxEmptyString;
		break;
	}

		return (text);
}

wxString
DlgRuleEditorListProperty::examineAlfCapabilityFilterPolicy(
    AlfCapabilityFilterPolicy *filter) const
{
	wxString	 text;
	PolicyRuleSet	*ruleSet;

	ruleSet = filter->getParentRuleSet();

	switch (type_) {
	case PROPERTY_ID:
		EXTRACT_ID(text, filter);
		break;
	case PROPERTY_TYPE:
		EXTRACT_TYPE(text, ruleSet, filter);
		break;
	case PROPERTY_CAP:
		text = filter->getCapabilityTypeName();
		break;
	case PROPERTY_SCOPE:
		text = filter->getScopeName();
		break;
	default:
		text = wxEmptyString;
		break;
	}

	return (text);
}

wxString
DlgRuleEditorListProperty::examineSfsDefaultFilterPolicy(
    SfsDefaultFilterPolicy *filter) const
{
	wxString	 text;
	PolicyRuleSet	*ruleSet;

	ruleSet = filter->getParentRuleSet();

	switch (type_) {
	case PROPERTY_ID:
		EXTRACT_ID(text, filter);
		break;
	case PROPERTY_TYPE:
		EXTRACT_TYPE(text, ruleSet, filter);
		break;
	case PROPERTY_PATH:
		text = filter->getPath();
		break;
	case PROPERTY_VALACT:
		text = filter->getActionName();
		break;
	case PROPERTY_VALLOG:
		text = filter->getLogName();
		break;
	default:
		text = wxEmptyString;
		break;
	}

	return (text);
}

wxString
DlgRuleEditorListProperty::examineSfsFilterPolicy(
    SfsFilterPolicy *filter) const
{
	wxString	 text;
	PolicyRuleSet	*ruleSet;

	ruleSet = filter->getParentRuleSet();

	switch (type_) {
	case PROPERTY_ID:
		EXTRACT_ID(text, filter);
		break;
	case PROPERTY_TYPE:
		EXTRACT_TYPE(text, ruleSet, filter);
		break;
	case PROPERTY_SCOPE:
		text = filter->getScopeName();
		break;
	case PROPERTY_PATH:
		text = filter->getPath();
		break;
	case PROPERTY_SUB:
		text = filter->getSubjectName();
		break;
	case PROPERTY_VALACT:
		text = filter->getValidActionName();
		break;
	case PROPERTY_VALLOG:
		text = filter->getValidLogName();
		break;
	case PROPERTY_INVALACT:
		text = filter->getInvalidActionName();
		break;
	case PROPERTY_INVALLOG:
		text = filter->getInvalidLogName();
		break;
	case PROPERTY_UNKACT:
		text = filter->getUnknownActionName();
		break;
	case PROPERTY_UNKLOG:
		text = filter->getUnknownLogName();
		break;
	default:
		text = wxEmptyString;
		break;
	}

	return (text);
}

wxString
DlgRuleEditorListProperty::examineDefaultFilterPolicy(
    DefaultFilterPolicy *filter) const
{
	wxString	 text;
	PolicyRuleSet	*ruleSet;

	ruleSet = filter->getParentRuleSet();

	switch (type_) {
	case PROPERTY_ID:
		EXTRACT_ID(text, filter);
		break;
	case PROPERTY_TYPE:
		EXTRACT_TYPE(text, ruleSet, filter);
		break;
	case PROPERTY_SCOPE:
		text = filter->getScopeName();
		break;
	case PROPERTY_ACTION:
		text = filter->getActionName();
		break;
	case PROPERTY_LOG:
		text = filter->getLogName();
		break;
	default:
		text = wxEmptyString;
		break;
	}

	return (text);
}

wxString
DlgRuleEditorListProperty::examineContextFilterPolicy(
    ContextFilterPolicy *filter) const
{
	wxString	 text;
	PolicyRuleSet	*ruleSet;

	ruleSet = filter->getParentRuleSet();

	switch (type_) {
	case PROPERTY_ID:
		EXTRACT_ID(text, filter);
		break;
	case PROPERTY_TYPE:
		text = filter->getContextTypeName();
		if (ruleSet && ruleSet->isAdmin()) {
			text.Append(wxT("(A)"));
		}
		break;
	case PROPERTY_SCOPE:
		text = filter->getScopeName();
		break;
	case PROPERTY_BINARY:
		text = filter->getBinaryName();
		break;
	default:
		text = wxEmptyString;
		break;
	}

	return (text);
}

wxString
DlgRuleEditorListProperty::examineSbAccessFilterPolicy(
    SbAccessFilterPolicy *filter) const
{
	wxString	 text;
	PolicyRuleSet	*ruleSet;

	ruleSet = filter->getParentRuleSet();

	switch (type_) {
	case PROPERTY_ID:
		EXTRACT_ID(text, filter);
		break;
	case PROPERTY_TYPE:
		EXTRACT_TYPE(text, ruleSet, filter);
		break;
	case PROPERTY_SCOPE:
		text = filter->getScopeName();
		break;
	case PROPERTY_ACTION:
		text = filter->getActionName();
		break;
	case PROPERTY_LOG:
		text = filter->getLogName();
		break;
	case PROPERTY_PATH:
		text = filter->getPath();
		break;
	case PROPERTY_SUB:
		text = filter->getSubjectName();
		break;
	case PROPERTY_MASK:
		text = filter->getAccessMaskName();
		break;
	default:
		text = wxEmptyString;
		break;
	}

	return (text);
}
