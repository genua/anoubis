/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#include <wx/icon.h>
#include <wx/string.h>

#include "main.h"
#include "Module.h"

Module::Module(void)
{
	name_ = wxString(wxT("unknown"));
	nick_ = wxString(wxT("unknown"));
	state_ = wxString(wxT("unknown"));
	mainPanel_ = NULL;
	overviewPanel_ = NULL;
	icon_ = NULL;
}

Module::~Module(void)
{
	if (mainPanel_ != NULL)
		delete mainPanel_;
	if (overviewPanel_ != NULL)
		delete overviewPanel_;
	if (icon_ != NULL)
		delete icon_;
}

wxString
Module::getName(void)
{
	return (name_);
}

wxString
Module::getNick(void)
{
	return (nick_);
}

wxString
Module::getState(void)
{
	return (state_);
}

void
Module::setState(const wxString& state)
{
	if(!state.IsEmpty())
		state_ = state;
}

wxPanel *
Module::getMainPanel(void)
{
	return (mainPanel_);
}

wxPanel *
Module::getOverviewPanel(void)
{
	return (overviewPanel_);
}

wxIcon *
Module::getIcon(void)
{
	return (icon_);
}

void
Module::loadIcon(wxString iconName)
{
	icon_ = wxGetApp().loadIcon(iconName);
}
