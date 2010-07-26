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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <wx/intl.h>

#include "DefaultConversions.h"
#include "MainUtils.h"
#include "PlaygroundFileEntry.h"
#include "PlaygroundInfoEntry.h"
#include "PlaygroundListProperty.h"


PlaygroundListProperty::PlaygroundListProperty(PropertyType type)
{
	type_ = type;
}

wxString
PlaygroundListProperty::getHeader(void) const
{
	wxString header;

	switch (type_) {
	case PROPERTY_ID:		header = _("Id");		break;
	case PROPERTY_USER:		header = _("User");		break;
	case PROPERTY_STAT:		header = _("Status");		break;
	case PROPERTY_FILES:		header = _("Files");		break;
	case PROPERTY_TIME:		header = _("Start Time");	break;
	case PROPERTY_COMMAND:		header = _("Command");		break;
	case PROPERTY_ATTENTION:	header = wxEmptyString;		break;
	case PROPERTY_DEV:		header = _("Device");		break;
	case PROPERTY_INODE:		header = _("Inode");		break;
	case PROPERTY_FILENAME:		header = _("File name");	break;
	}
	return (header);
}

wxString
PlaygroundListProperty::getText(AnListClass *item) const
{
	wxString text;
	wxDateTime startDate;
	PlaygroundInfoEntry* pgInfo;
	PlaygroundFileEntry* pgFile;

	text   = wxEmptyString;
	pgInfo = dynamic_cast<PlaygroundInfoEntry*>(item);
	pgFile = dynamic_cast<PlaygroundFileEntry*>(item);

	if (pgInfo != NULL) {
		switch (type_) {
		case PROPERTY_ID:
			text = wxString::Format(wxT("%"PRIx64),
			    pgInfo->getPgid());
			break;
		case PROPERTY_USER:
			text = MainUtils::instance()->getUserNameById(
			    pgInfo->getUid());
			break;
		case PROPERTY_STAT:
			if (pgInfo->isActive()) {
				text = _("active");
			} else {
				text = _("inactive");
			}
			break;
		case PROPERTY_FILES:
			text = wxString::Format(wxT("%d"),
			    pgInfo->getNumFiles());
			break;
		case PROPERTY_TIME:
			text = DefaultConversions::toString(
			    pgInfo->getStarttime());
			break;
		case PROPERTY_COMMAND:
			text = pgInfo->getPath();
			break;
		case PROPERTY_ATTENTION:
			text = wxEmptyString;
			break;
		default:
			text = wxEmptyString;
			break;
		}
	} else if (pgFile != NULL) {
		switch (type_) {
		case PROPERTY_ID:
			text = wxString::Format(wxT("%"PRIx64),
			    pgFile->getPgid());
			break;
		case PROPERTY_DEV:
			text = wxString::Format(wxT("%"PRIx64),
			    pgFile->getDevice());
			break;
		case PROPERTY_INODE:
			text = wxString::Format(wxT("%"PRIx64),
			    pgFile->getInode());
			break;
		case PROPERTY_FILENAME:
			text = DefaultConversions::toString(
			    pgFile->getPaths());
			break;
		default:
			text = wxEmptyString;
			break;
		}
	}

	return (text);
}

AnIconList::IconId
PlaygroundListProperty::getIcon(AnListClass *item) const
{
	PlaygroundInfoEntry* pgInfo;

	pgInfo = dynamic_cast<PlaygroundInfoEntry*>(item);
	if ((type_ == PROPERTY_ATTENTION) && (pgInfo != NULL) &&
	    !pgInfo->isActive() && (pgInfo->getUid() == geteuid())) {
		return AnIconList::ICON_WARNING;
	} else {
		return AnIconList::ICON_NONE;
	}
}
