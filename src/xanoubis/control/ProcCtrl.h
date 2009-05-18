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

#ifndef _PROCCTRL_H_
#define _PROCCTRL_H_

#include <vector>
#include <wx/thread.h>

#include "Singleton.h"

/*
 * Maintain a list of processes that cannot trigger ASK events in the GUI
 * because they are part of the GUI. E.g. The GUI threads themselves or
 * the cvs processes started by the apnvm library.
 *
 * The communication process will ask this class if the current event
 * belongs to one of the processes registered here and will immediately
 * OK these events if that is the case.
 */

class ProcCtrl : public Singleton<ProcCtrl>
{
	public:
		/**
		 * Constructor of ProcCtrl.
		 * @param None.
		 * @return Nothing.
		 */
		ProcCtrl(void);

		/**
		 * Implementation of GetInstance from Singleton.
		 */
		static ProcCtrl *getInstance(void);

		/*
		 * Add the given PID to the list of processes
		 * @param[in] 1st The pid.
		 * @return None.
		 */
		void addPid(pid_t);

		/**
		 * Remove the given PID from the list of processes
		 * @param[in] 1st The pid.
		 * @return None.
		 */
		void removePid(pid_t);

		/**
		 * Return true if the given PID is in the list of processes
		 * maintained by ProcCtrl, false otherwise.
		 * @param[in] 1st The PID.
		 * @return True if the pid was found.
		 */
		bool findPid(pid_t);

	private:
		/* Synchronize access to PID list. */
		wxMutex			mutex_;
		std::vector<int>	pids_;
};

extern "C" {
	/**
	 * A callback function that can be used from a "C" context to
	 * register a new PID or delete an existing one.
	 * @param[in] 1st The PID.
	 * @param[in] 2nd True if the pid should be added, false if the
	 *     PID should be deleted.
	 * @return None.
	 */
	extern void procctrl_pidcallback(pid_t, int);
};

#endif	/* _PROCCTRL_H_ */
