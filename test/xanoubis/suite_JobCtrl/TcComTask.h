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

#ifndef _TCCOMTASK_H_
#define _TCCOMTASK_H_

#include "TestHandler.h"

class TcComTask : public TestHandler
{
	public:
		TcComTask();
		~TcComTask();

		void startTest();
		void taskEvent(const TaskEvent &);
		bool canExitTest() const;
		int getTestResult() const;

	protected:
		void OnRegister(TaskEvent &);
		void OnPolicyReceived(TaskEvent &);
		void OnPolicySend(TaskEvent &);
		void OnCsumAdd(TaskEvent &);
		void OnCsumGet(TaskEvent &);
		void OnCsumDel(TaskEvent &);
		void OnSfsList(TaskEvent &);

	private:
		int			policyRequestCounter_;
		int			csumListCounter_;
		bool			exit_;
		int			result_;

		static wxString getFileContent(const wxString &);
		static wxString getPolicyAsString(struct apn_ruleset *);
		static struct apn_ruleset *getPolicyFromFile(const wxString &);
};

#endif	/* _TCCOMTASK_H_ */
