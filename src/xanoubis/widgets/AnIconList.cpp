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

#include "AnIconList.h"
#include "MainUtils.h"

#include "Singleton.cpp"
template class Singleton<AnIconList>;

AnIconList::AnIconList(void)
{
	/* Add icons in correct ordner, as defined in enum IconId */
	addIcon(wxT("General_ok_16.png"));
	addIcon(wxT("General_problem_16.png"));
	addIcon(wxT("General_error_16.png"));
	addIcon(wxT("General_alert_16.png"));
	addIcon(wxT("General_ok_48.png"));
	addIcon(wxT("General_problem_48.png"));
	addIcon(wxT("General_error_48.png"));
	addIcon(wxT("General_alert_48.png"));
	addIcon(wxT("General_question_48.png"));
	addIcon(wxT("General_problem_48.png"));
	addIcon(wxT("General_symlink_16.png"));

	addIcon(wxT("ModAnoubis_black_16.png"));
	addIcon(wxT("ModAnoubis_black_20.png"));
	addIcon(wxT("ModAnoubis_black_24.png"));
	addIcon(wxT("ModAnoubis_black_32.png"));
	addIcon(wxT("ModAnoubis_black_48.png"));

	addIcon(wxT("ModAnoubis_alert_16.png"));
	addIcon(wxT("ModAnoubis_alert_20.png"));
	addIcon(wxT("ModAnoubis_alert_24.png"));
	addIcon(wxT("ModAnoubis_alert_32.png"));
	addIcon(wxT("ModAnoubis_alert_48.png"));

	addIcon(wxT("ModAnoubis_question_16.png"));
	addIcon(wxT("ModAnoubis_question_20.png"));
	addIcon(wxT("ModAnoubis_question_24.png"));
	addIcon(wxT("ModAnoubis_question_32.png"));
	addIcon(wxT("ModAnoubis_question_48.png"));
}

void
AnIconList::addIcon(const wxString &name)
{
	wxIcon *icon = MainUtils::instance()->loadIcon(name);

	Add(*icon);

	delete icon;
}
