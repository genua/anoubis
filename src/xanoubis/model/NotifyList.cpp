/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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
#include "NotifyList.h"
#include "NotifyAnswer.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(NotifyList);

NotifyListEntry::NotifyListEntry(bool isAnswerAble)
{
	isAnswered_ = false;
	isAnswerAble_ = isAnswerAble;
}

NotifyListEntry::NotifyListEntry(unsigned int id, wxString module,
    bool isAnswerAble)
{
	isAnswered_ = false;
	isAnswerAble_ = isAnswerAble;
	id_ = id;
	module_ = module;
}

bool
NotifyListEntry::isAnswered(void)
{
	return (isAnswered_);
}

bool
NotifyListEntry::isAnswerAble(void)
{
	return (isAnswerAble_);
}

void
NotifyListEntry::answer(NotifyAnswer *answer)
{
	isAnswered_ = true;
	answer_ = answer;
}

unsigned int
NotifyListEntry::getId(void)
{
	return (id_);
}

wxString
NotifyListEntry::getModule(void)
{
	return (module_);
}

NotifyAnswer *
NotifyListEntry::getAnswer(void)
{
	return (answer_);
}
