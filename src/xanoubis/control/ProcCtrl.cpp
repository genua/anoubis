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

#include <sys/types.h>
#include <unistd.h>

#include "ProcCtrl.h"

#include "Singleton.cpp"
template class Singleton<ProcCtrl>;

ProcCtrl::ProcCtrl(void) : mutex_(), pids_(0)
{
	mutex_.Lock();
	pids_.push_back(getpid());
	mutex_.Unlock();
}

void
ProcCtrl::addPid(pid_t pid)
{
	mutex_.Lock();
	pids_.push_back(pid);
	mutex_.Unlock();
}

void
ProcCtrl::removePid(pid_t pid)
{
	std::vector<int>::iterator	it;
	mutex_.Lock();
	for (it=pids_.begin(); it != pids_.end(); ++it) {
		if (*it == pid) {
			pids_.erase(it);
			break;
		}
	}
	mutex_.Unlock();
}

bool
ProcCtrl::findPid(pid_t pid)
{
	std::vector<int>::iterator	it;
	mutex_.Lock();
	for (it=pids_.begin(); it != pids_.end(); ++it) {
		if (*it == pid)
			break;
	}
	mutex_.Unlock();
	return (it != pids_.end());
}

void
procctrl_pidcallback(pid_t pid, int add)
{
	if (add) {
		ProcCtrl::instance()->addPid(pid);
	} else {
		ProcCtrl::instance()->removePid(pid);
	}
}
