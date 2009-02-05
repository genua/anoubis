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

#include <config.h>

#include <sys/types.h>

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include <errno.h>

#include <csum/csum.h>

#include "CsumCalcTask.h"
#include "JobCtrl.h"

CsumCalcTask::CsumCalcTask(void) : Task(Task::TYPE_CSUMCALC)
{
	reset();
}

wxString
CsumCalcTask::getPath(void) const
{
	return (this->path_);
}

void
CsumCalcTask::setPath(const wxString &path)
{
	this->path_ = path;
}

bool
CsumCalcTask::calcLink(void) const
{
	return (this->calcLink_);
}

void
CsumCalcTask::setCalcLink(bool enable)
{
	this->calcLink_ = enable;
}

wxEventType
CsumCalcTask::getEventType(void) const
{
	return (anTASKEVT_CSUMCALC);
}

void
CsumCalcTask::exec(void)
{
	int		cslen = ANOUBIS_CS_LEN;
	struct stat	fstat;

	reset();

	if (lstat(path_.fn_str(), &fstat) == 0) {
		if (S_ISLNK(fstat.st_mode) && calcLink_)
			this->result_ = anoubis_csum_link_calc(
			    path_.fn_str(), this->cs_, &cslen);
		else
			this->result_ = anoubis_csum_calc(
			    path_.fn_str(), this->cs_, &cslen);

		this->result_ *= -1;
	} else
		this->result_ = errno;
}

int
CsumCalcTask::getResult(void) const
{
	return (this->result_);
}

const u_int8_t *
CsumCalcTask::getCsum(void) const
{
	return (this->cs_);
}

wxString
CsumCalcTask::getCsumStr(void) const
{
	wxString str;

	for (int i = 0; i < ANOUBIS_CS_LEN; i++)
		str += wxString::Format(wxT("%.2x"), this->cs_[i]);

	return (str);
}

void
CsumCalcTask::reset(void)
{
	this->result_ = -99;

	for (int i = 0; i < ANOUBIS_CS_LEN; i++)
		this->cs_[i] = 0;
}
