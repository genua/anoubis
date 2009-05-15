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

#include "NotificationPerspective.h"

#include <wx/arrimpl.cpp>

NotificationPerspective::NotificationPerspective(void)
{
	ids_.Empty();
}

long
NotificationPerspective::getSize(void) const
{
	return (ids_.GetCount());
}

long
NotificationPerspective::getId(long index) const
{
	return (ids_.Item(index));
}

wxArrayLong::const_iterator
NotificationPerspective::begin(void) const
{
	return (ids_.begin());
}

wxArrayLong::const_iterator
NotificationPerspective::end(void) const
{
	return (ids_.end());
}

NotificationPerspective::~NotificationPerspective(void)
{
	ids_.Clear();
}

void
NotificationPerspective::addId(long id)
{
	startChange();
	ids_.Add(id);
	finishChange();
}

void
NotificationPerspective::removeId(long id)
{
	startChange();
	ids_.Remove(id);
	finishChange();
}
