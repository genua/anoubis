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

#include "SimpleOverviewRow.h"
#include "AnListClass.h"
#include "AppPolicy.h"
#include "FilterPolicy.h"

SimpleOverviewRow::SimpleOverviewRow(int appIdx, AppPolicy *appPol,
	int filterIdx, FilterPolicy *filterPol)
{
	applicationIndex_ = appIdx;
	applicationPolicy_ = appPol;
	filterIndex_ = filterIdx;
	filterPolicy_ = filterPol;
}

FilterPolicy *
SimpleOverviewRow::getFilterPolicy(void) const
{
	return filterPolicy_;
}

int
SimpleOverviewRow::getFilterPolicyIndex(void) const
{
	return filterIndex_;
}

AppPolicy *
SimpleOverviewRow::getApplicationPolicy(void) const
{
	return applicationPolicy_;
}

int
SimpleOverviewRow::getApplicationPolicyIndex(void) const
{
	return applicationIndex_;
}
