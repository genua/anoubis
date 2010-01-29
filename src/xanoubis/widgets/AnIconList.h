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

#include "Singleton.h"

/**
 * Central image-list.
 *
 * Provides all icons, which are used all over the application.
 *
 * AnListCtrl is assigning the image-list to receive its icon-data.
 */
class AnIconList : public wxImageList, public Singleton<AnIconList>
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
			ICON_OK_48,		/*!< Big ok icon */
			ICON_WARNING_48,	/*!< Big warning icon */
			ICON_ERROR_48,		/*!< Big error icon */
			ICON_ALERT_48,		/*!< Big alert icon */
			ICON_QUESTION_48,	/*!< Big question icon */
			ICON_PROBLEM_48,	/*!< Big exclamation icon */
			ICON_SYMLINK,		/*!< Symlink icon */
			ICON_ANOUBIS_BLACK,	/*!< Basic Anoubis icon */
			ICON_ANOUBIS_ALERT,	/*!< Anoubis alert-icon */
			ICON_ANOUBIS_QUESTION	/*!< Anoubis question-icon */
		};

		/**
		 * Returns the singleton-instance of the class.
		 *
		 * @return Singleton instanceof the image-list.
		 */
		static AnIconList *getInstance(void);

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

	friend class Singleton<AnIconList>;
};

#endif	/* _ANICONLIST_H_ */
