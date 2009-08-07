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
		void nextTest();

		void setupTestConnect(void);
		void onTestConnect(wxCommandEvent &);

		void setupTestPolicyRequest(void);
		void onTestPolicyRequest(TaskEvent &);

		void setupTestPolicySendEmpty(void);
		void onTestPolicySendEmpty(TaskEvent &);

		void setupTestPolicySend(void);
		void onTestPolicySend(TaskEvent &);

		void setupTestCsumAdd(void);
		void onTestCsumAdd(TaskEvent &);

		void setupTestCsumAddSymlink(void);
		void onTestCsumAddSymlink(TaskEvent &);

		void setupTestCsumAddSymlinkLink(void);
		void onTestCsumAddSymlinkLink(TaskEvent &);

		void setupTestCsumAddPipe(void);
		void onTestCsumAddPipe(TaskEvent &);

		void setupTestCsumAddPipeLink(void);
		void onTestCsumAddPipeLink(TaskEvent &);

		void setupTestCsumGet(void);
		void onTestCsumGet(TaskEvent &);

		void setupTestCsumGetNoSuchFile(void);
		void onTestCsumGetNoSuchFile(TaskEvent &);

		void setupTestCsumGetOrphaned(void);
		void onTestCsumGetOrphaned(TaskEvent &);

		void setupTestCsumGetSymlink(void);
		void onTestCsumGetSymlink(TaskEvent &);

		void setupTestCsumGetSymlinkLink(void);
		void onTestCsumGetSymlinkLink(TaskEvent &);

		void setupTestSfsListNotEmpty(void);
		void onTestSfsListNotEmpty(TaskEvent &);

		void setupTestCsumDel(void);
		void onTestCsumDel(TaskEvent &);

		void setupTestCsumDelSymlink(void);
		void onTestCsumDelSymlink(TaskEvent &);

		void setupTestCsumDelSymlinkLink(void);
		void onTestCsumDelSymlinkLink(TaskEvent &);

		void setupTestSfsListEmpty(void);
		void onTestSfsListEmpty(TaskEvent &);

		void setupTestSfsListRecursive(void);
		void onTestSfsListRecursive(TaskEvent &);

		void setupTestUpgradeList(void);
		void onTestUpgradeList(TaskEvent &);

		void setupTestSigAdd(void);
		void onTestSigAdd(TaskEvent &);

		void setupTestSigGet(void);
		void onTestSigGet(TaskEvent &);

		void setupTestSigListNotEmpty(void);
		void onTestSigListNotEmpty(TaskEvent &);

		void setupTestSigDel(void);
		void onTestSigDel(TaskEvent &);

		void setupTestSigListEmpty(void);
		void onTestSigListEmpty(TaskEvent &);

	private:
		int			testCounter_;
		bool			exit_;
		int			result_;

		static wxString getFileContent(const wxString &);
		static wxString getPolicyAsString(struct apn_ruleset *);
		static struct apn_ruleset *getPolicyFromFile(const wxString &);
};

#endif	/* _TCCOMTASK_H_ */
