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

#ifndef _ANICONLIST_H_
#define _ANICONLIST_H_

#include <wx/imaglist.h>
#include <vector>
#include <map>

#include "Singleton.h"

/**
 * Central image-list.
 *
 * Provides all icons, which are used all over the application.
 *
 * AnListCtrl is assigning the image-list to receive its icon-data.
 */
class AnIconList : public Singleton<AnIconList>
{
	public:
		/**
		 * Enumeration lists all available icons.
		 */
		enum IconId
		{
			ICON_NONE = -1,		/*!< No icon is used. */
			ICON_OK = 0,		/*!< Ok icon */
			ICON_WARNING,		/*!< Warning icon */
			ICON_ERROR,		/*!< Error icon */
			ICON_ALERT,		/*!< Alert icon */
			ICON_QUESTION,		/*!< Question icon */
			ICON_DOWN,		/*!< Down-arrow icon */
			ICON_RIGHT,		/*!< Right-arrow icon */
			ICON_OK_48,		/*!< Big ok icon */
			ICON_WARNING_48,	/*!< Big warning icon */
			ICON_ERROR_48,		/*!< Big error icon */
			ICON_ALERT_48,		/*!< Big alert icon */
			ICON_QUESTION_48,	/*!< Big question icon */
			ICON_PROBLEM_48,	/*!< Big exclamation icon */
			ICON_SYMLINK,		/*!< Symlink icon */
			ICON_ANOUBIS_BLACK,	/*!< Basic Anoubis icon */
			ICON_ANOUBIS_BLACK_20,	/*!< Basic Anoubis icon 20x20 */
			ICON_ANOUBIS_BLACK_24,	/*!< Basic Anoubis icon 24x24 */
			ICON_ANOUBIS_BLACK_32,	/*!< Basic Anoubis icon 32x32 */
			ICON_ANOUBIS_BLACK_48,	/*!< Basic Anoubis icon 48x48 */
			ICON_ANOUBIS_ALERT,	/*!< Anoubis alert-icon */
			ICON_ANOUBIS_ALERT_20,	/*!< Anoubis alert-icon 20x20 */
			ICON_ANOUBIS_ALERT_24,	/*!< Anoubis alert-icon 24x24 */
			ICON_ANOUBIS_ALERT_32,	/*!< Anoubis alert-icon 32x32 */
			ICON_ANOUBIS_ALERT_48,	/*!< Anoubis alert-icon 48x48 */
			ICON_ANOUBIS_QUESTION,	 /*!< Anoubis question-icon */
			ICON_ANOUBIS_QUESTION_20,/*!< Anoubis question-icon */
			ICON_ANOUBIS_QUESTION_24,/*!< Anoubis question-icon */
			ICON_ANOUBIS_QUESTION_32,/*!< Anoubis question-icon */
			ICON_ANOUBIS_QUESTION_48,/*!< Anoubis question-icon */
			ICON_ALF_BLACK_48,	/*!< Basic Alf-icon */
			ICON_ALF_OK_48,		/*!< Alf-ok icon */
			ICON_ALF_ERROR_48,	/*!< Alf-error icon */
			ICON_OVERVIEW_48,	/*!< Basic Overview-icon */
			ICON_PG_BLACK_48,	/*!< Basic playground-icon */
			ICON_PG_OK_48,		/*!< Playground-ok icon */
			ICON_PG_ERROR_48,	/*!< Playground-error icon */
			ICON_SB_BLACK_48,	/*!< Basic playground-icon */
			ICON_SB_OK_48,		/*!< Playground-ok icon */
			ICON_SB_ERROR_48,	/*!< Playground-error icon */
			ICON_SFS_BLACK_48,	/*!< Basic Sfs-icon */
			ICON_SFS_OK_48,		/*!< Sfs-ok icon */
			ICON_SFS_ERROR_48,	/*!< Sfs-error icon */
			ICON_ANOUBIS_MAX
		};

		~AnIconList(void);

		/**
		 * Returns the icon-data with the given id.
		 * @param id The requested id
		 * @return Icon-data
		 */
		wxIcon	*getIcon(AnIconList::IconId id) {
			return icons_[id].second;
		}

		/**
		 * Returns the path where the icon with the given id is stored.
		 * @param id The requested id
		 * @return Path of requested icon
		 */
		wxString getPath(AnIconList::IconId);

		wxIcon	GetIcon(int id) {
			return *icons_[id].second;
		}
	protected:
		/**
		 * Std-c'tor.
		 */
		AnIconList(void);

	private:
		/**
		 * Appends an icon to the list.
		 *
		 * @param name Name of icon-file
		 */
		void addIcon(const wxString &);

		/**
		 * Returns the path of the picture with the given name.
		 * @param iconName The name of the picture
		 * @return Path, where the picture is stored.
		 */
		static wxString getIconPath(const wxString &);

		std::vector<std::pair<wxString, wxIcon *> > icons_;

	friend class Singleton<AnIconList>;
};

class AnDynamicIconList : public wxImageList {
public:
	AnDynamicIconList(void) {
	}
	int	loadIcon(AnIconList::IconId id) {
		if (idmap_.find(id) == idmap_.end()) {
			wxIcon	*icon = AnIconList::instance()->getIcon(id);
			idmap_[id] = Add(*icon);
		}
		return idmap_[id];
	};
private:
	std::map<int, int>	idmap_;
};

#endif	/* _ANICONLIST_H_ */
