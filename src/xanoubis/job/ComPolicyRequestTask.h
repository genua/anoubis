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

#ifndef _COMPOLICYREQUESTTASK_H_
#define _COMPOLICYREQUESTTASK_H_

#include "ComTask.h"

class PolicyRuleSet;

/**
 * Task to receive a policy from anoubisd.
 *
 * The requested policy belongs to a specific user
 * (ComPolicyRequestTask::getUid()) and has a priority
 * (ComPolicyRequestTask::getPriority()). Both information have to be specified
 * before the task is executed.
 *
 * When the task was successfully executed, you can fetch the requested policy
 * with ComPolicyRequestTask::getPolicy().
 *
 * Supported error-codes:
 * - <code>RESULT_COM_ERROR</code> Communication error. Failed to create a
 *   transaction or to fetch the answer-message.
 * - <code>RESULT_REMOTE_ERROR</code> Operation(s) performed by anoubisd
 *   failed. getResultDetails() will return the remote error-code and can be
 *   evaluated by strerror(3) or similar.
 */
class ComPolicyRequestTask : public ComTask
{
	public:
		/**
		 * Default-c'tor.
		 *
		 * You explicity need to set the priority and uid.
		 *
		 * @see setRequestParameter()
		 */
		ComPolicyRequestTask(void);

		/**
		 * Constructs a ComPolicyRequestTask with an already assigned
		 * priority and uid.
		 *
		 * @param priority Priority of requested policy
		 * @param uid User of requested policy
		 *
		 * @see setRequestParameter()
		 */
		ComPolicyRequestTask(int, uid_t);

		/**
		 * D'tor.
		 */
		~ComPolicyRequestTask();

		/**
		 * Returns the priority of the requested policy.
		 *
		 * - prio = 0: admin policy
		 * - prio = 1: user policy
		 *
		 * @return Priority of requested policy
		 */
		int getPriority(void) const;

		/**
		 * Returns the uid of the corresponding user.
		 * @return User of requested policy.
		 */
		uid_t getUid(void) const;

		/**
		 * Assigns request-parameter to the task.
		 *
		 * @param prio The priority of the requested policy
		 * @param uid The uid of the requested policy
		 */
		void setRequestParameter(int, uid_t);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

		/**
		 * Implementation of Task::exec().
		 */
		bool done(void);

		/**
		 * Returns the requested policy after the task was executed.
		 * On error (e.g. communication error, unknown user, ...) 0 is
		 * returned.
		 *
		 * @return The requested policy.
		 * @note Memory allocated for the policy needs to be explicitly
		 *       destroyed. The task does not take cake about the
		 *       memory!
		 * @see apn_free_ruleset()
		 */
		struct apn_ruleset *getPolicyApn(void) const;

		/**
		 * Returns the requested policy after the task was executed.
		 * On error (e.g. communication error, unknown user, ...) 0 is
		 * returned.
		 *
		 * @return The requested policy.
		 * @note Memory allocated for the policy needs to be explicitly
		 *       destroyed. The task does not take cake about the
		 *       memory!
		 */
		PolicyRuleSet *getPolicy(void);

	private:
		int				 prio_;
		uid_t				 uid_;
		struct anoubis_msg		*msg_;
		struct anoubis_transaction	*ta_;

		/**
		 * @see ComTask::resetComTaskResult()
		 */
		void resetComTaskResult(void);
};

#endif	/* _COMPOLICYREQUESTTASK_H_ */
