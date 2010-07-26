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

#ifndef _DEFAULTCONVERSIONS_H
#define _DEFAULTCONVERSIONS_H

#include <vector>
#include <wx/string.h>
#include <wx/datetime.h>

#include "AnIconList.h"

/**
 * This class contains a number of default conversion functions
 * that can be used to convert different data types to wxStrings
 * or icon IDs.
 *
 * The main use of these functions is in list properties that are
 * generated via the AnFmtListProperty template. However, the conversion
 * functions themselves are stand alone.
 *
 * Nameing conventions:
 * A function called toString should convert its single argument
 * to a wxString in a "natural" way. Other ways to convert the same
 * data type can be given as special names:
 * e.g.: The natural conversion of a wxDateTime to a string is a string
 * that includes both the date and the time. This is done by the
 * corresponding toString function.
 * Similarly, the function toIcon returns the natural conversion of a
 * type to an Icon (if there is any).
 */
class DefaultConversions {
public:
	/**
	 * Returns the argument string unmodified.
	 */
	static wxString toString(const wxString str) {
		return str;
	}
	/**
	 * Returns the decimal representation of the value.
	 */
	static wxString toString(long long val) {
		return wxString::Format(wxT("%lld"), val);
	}
	/**
	 * Returns the date and time of the argument in the default time
	 * zone as a string.
	 */
	static wxString toString(const wxDateTime dt) {
		return dt.Format(wxT("%x %X"));
	}
	/**
	 * Returns the vector elements as singl line string.
	 */
	static wxString toString(const std::vector<wxString> & list) {
		wxString line;
		std::vector<wxString>::const_iterator it;
		if (list.size() == 0) {
			return wxEmptyString;
		}
		for (it=list.begin(); it!=list.end(); it++) {
			if (!line.IsEmpty()) {
				line.Append(wxT(", "));
			}
			line.Append(*it);
		}
		return line;
	}
	/**
	 * Returns the date of the argument in the default time zone as
	 * a string.
	 */
	static wxString toDate(const wxDateTime dt) {
		return dt.FormatDate();
	}
	/**
	 * Returns the time of the argument in the default time zone as
	 * a string.
	 */
	static wxString toTime(const wxDateTime dt) {
		return dt.FormatTime();
	}
	/**
	 * Return an empty string regardless of the argument value.
	 */
	template <typename T>
	static wxString toEmpty(T) {
		return wxEmptyString;
	}

	/**
	 * Returns the argument Icon ID unmodified.
	 */
	static AnIconList::IconId toIcon(AnIconList::IconId iconid) {
		return iconid;
	}
	static AnIconList::IconId toIcon(bool val) {
		return val ? AnIconList::ICON_OK : AnIconList::ICON_ERROR;
	}
};

#endif	/* _DEFAULTCONVERSIONS_H_ */
