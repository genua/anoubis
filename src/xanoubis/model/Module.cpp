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

#include "AnIconList.h"
#include "MainUtils.h"
#include "Module.h"

Module::Module(void)
{
	name_ = wxString(_("unknown"));
	nick_ = wxString(_("unknown"));
	state_ = wxString(_("unknown"));
	mainPanel_ = NULL;
	overviewPanel_ = NULL;
}

Module::~Module(void)
{
	mainPanel_ = NULL;
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
