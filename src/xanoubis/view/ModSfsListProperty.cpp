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

#include <wx/intl.h>

#include <SfsEntry.h>

#include "ModSfsListProperty.h"

ModSfsListProperty::ModSfsListProperty(Type type)
{
	this->type_ = type;
}

wxString
ModSfsListProperty::getHeader(void) const
{
	switch (type_) {
	case PATH:
		return _("File");
	case CHECKSUM:
		return _("Checksum");
	case SIGNATURE:
		return _("Signature");
	}

	return (wxEmptyString);
}

wxString
ModSfsListProperty::getText(AnListClass *obj) const
{
	SfsEntry *entry = dynamic_cast<SfsEntry *>(obj);

	if (entry == 0)
		return _("(null)");

	switch (type_) {
	case PATH:
		return (getTextPath(entry));
	case CHECKSUM:
		return (getTextChecksum(entry));
	case SIGNATURE:
		return (getTextSignature(entry));
	}

	return (wxEmptyString);
}

AnIconList::IconId
ModSfsListProperty::getIcon(AnListClass *obj) const
{
	SfsEntry *entry = dynamic_cast<SfsEntry *>(obj);

	if (entry == 0)
		return (AnIconList::ICON_NONE);

	switch (type_) {
	case PATH:
		return (getIconPath(entry));
	case CHECKSUM:
		return (getIconChecksum(entry));
	case SIGNATURE:
		return (getIconSignature(entry));
	}

	return (AnIconList::ICON_NONE);
}

wxString
ModSfsListProperty::getTextPath(SfsEntry *entry) const
{
	return (entry->getRelativePath());
}

wxString
ModSfsListProperty::getTextChecksum(SfsEntry *entry) const
{
	switch (entry->getChecksumState(SfsEntry::SFSENTRY_CHECKSUM)) {
	case SfsEntry::SFSENTRY_NOT_VALIDATED:
		return _("???");
	case SfsEntry::SFSENTRY_MISSING:
		return _("not registered");
	case SfsEntry::SFSENTRY_INVALID:
		return _("invalid");
	case SfsEntry::SFSENTRY_NOMATCH:
	case SfsEntry::SFSENTRY_MATCH:
		return (wxEmptyString);
	case SfsEntry::SFSENTRY_ORPHANED:
		return _("orphaned");
	}

	return _("(null)");
}

wxString
ModSfsListProperty::getTextSignature(SfsEntry *entry) const
{
	switch (entry->getChecksumState(SfsEntry::SFSENTRY_SIGNATURE)) {
	case SfsEntry::SFSENTRY_NOT_VALIDATED:
		return _("???");
	case SfsEntry::SFSENTRY_MISSING:
		return _("not registered");
	case SfsEntry::SFSENTRY_INVALID:
		return _("invalid");
	case SfsEntry::SFSENTRY_MATCH:
		return (wxEmptyString);
	case SfsEntry::SFSENTRY_ORPHANED:
		return _("orphaned");
	case SfsEntry::SFSENTRY_NOMATCH:
		switch(entry->getChecksumState(SfsEntry::SFSENTRY_UPGRADE)) {
		case SfsEntry::SFSENTRY_MATCH:
			return _("upgraded");
		default:
			return (wxEmptyString);
		}
		break;
	}

	return _("(null)");
}

AnIconList::IconId
ModSfsListProperty::getIconPath(SfsEntry *entry) const
{
	if (entry->isSymlink())
		return (AnIconList::ICON_SYMLINK);
	else
		return (AnIconList::ICON_NONE);
}

AnIconList::IconId
ModSfsListProperty::getIconChecksum(SfsEntry *entry) const
{
	switch (entry->getChecksumState(SfsEntry::SFSENTRY_CHECKSUM)) {
	case SfsEntry::SFSENTRY_NOT_VALIDATED:
	case SfsEntry::SFSENTRY_MISSING:
		return (AnIconList::ICON_NONE);
	case SfsEntry::SFSENTRY_INVALID:
	case SfsEntry::SFSENTRY_ORPHANED:
		return (AnIconList::ICON_WARNING);
	case SfsEntry::SFSENTRY_NOMATCH:
		return (AnIconList::ICON_ERROR);
	case SfsEntry::SFSENTRY_MATCH:
		return (AnIconList::ICON_OK);
	}

	return (AnIconList::ICON_NONE);
}

AnIconList::IconId
ModSfsListProperty::getIconSignature(SfsEntry *entry) const
{
	switch (entry->getChecksumState(SfsEntry::SFSENTRY_SIGNATURE)) {
	case SfsEntry::SFSENTRY_NOT_VALIDATED:
	case SfsEntry::SFSENTRY_MISSING:
		return (AnIconList::ICON_NONE);
	case SfsEntry::SFSENTRY_INVALID:
	case SfsEntry::SFSENTRY_ORPHANED:
		return (AnIconList::ICON_WARNING);
	case SfsEntry::SFSENTRY_MATCH:
		return (AnIconList::ICON_OK);
	case SfsEntry::SFSENTRY_NOMATCH:
		switch(entry->getChecksumState(SfsEntry::SFSENTRY_UPGRADE)) {
		case SfsEntry::SFSENTRY_MATCH:
			return (AnIconList::ICON_WARNING);
		default:
			return (AnIconList::ICON_ERROR);
		}
		break;
	}

	return (AnIconList::ICON_NONE);
}
