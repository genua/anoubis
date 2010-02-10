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

#include <wx/filename.h>
#include <wx/intl.h>

#include <SbEntry.h>

#include "RuleWizardSandboxProperty.h"

RuleWizardSandboxProperty::RuleWizardSandboxProperty(Type type)
{
	this->type_ = type;
}

wxString
RuleWizardSandboxProperty::getHeader(void) const
{
	switch (type_) {
	case PATH: return _("Path");
	case FILE: return _("File");
	case STD:  return _("Standard");
	}

	return _("(null)");
}

wxString
RuleWizardSandboxProperty::getText(AnListClass *obj) const
{
	SbEntry *entry = dynamic_cast<SbEntry *>(obj);

	if (entry == 0)
		return _("(null)");

	if (wxFileName::DirExists(entry->getPath())) {
		if (type_ == PATH)
			return (entry->getPath());
		else if (type_ == FILE)
			return _("(all files)");
	} else {
		wxFileName path(entry->getPath());

		if (type_ == PATH)
			return (path.GetPath());
		else if (type_ == FILE)
			return (path.GetFullName());
	}

	return (wxEmptyString);
}

AnIconList::IconId
RuleWizardSandboxProperty::getIcon(AnListClass *obj) const
{
	SbEntry *entry = dynamic_cast<SbEntry *>(obj);

	if (entry != 0 && entry->isDefault() && type_ == STD)
		return (AnIconList::ICON_OK);
	else
		return (AnIconList::ICON_NONE);
}
