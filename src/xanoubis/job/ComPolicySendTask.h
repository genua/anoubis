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

#ifndef _COMPOLICYSENDTASK_H_
#define _COMPOLICYSENDTASK_H_

#include "ComTask.h"

class PolicyRuleSet;

/**
 * Task to send a policy to anoubisd.
 *
 * Before the task is scheduled, you need to provide the policy and its
 * priority to be send by calling ComPolicySendTask::setPolicy() and
 * ComPolicySendTask::setPriority().
 *
 * The policy is signed, if you provide a private key (setPrivateKey()). Then,
 * the signature together with the policy is send to Aenoubis-daemon.
 *
 * The task provides both policy-formats:
 * - GUI's <code>PolicyRuleSet</code>
 * - raw <code>struct apn_ruleset</code>
 * Thus one of the getter (getPolicy(), getPolicyApn()) will return a valid
 * (<code>!= 0</code>) pointer.
 *
 * Supported error-codes:
 * - <code>RESULT_OOM</code> Out of memory
 * - <code>RESULT_COM_ERROR</code> Communication error. Failed to create a
 *   transaction or to fetch the answer-message.
 * - <code>RESULT_REMOTE_ERROR</code> Operation(s) performed by anoubisd
 *   failed. getResultDetails() will return the remote error-code and can be
 *   evaluated by anoubis_strerror or similar.
 * - <code>RESULT_LOCAL_ERROR</code> Failed to sign the policy (if requested).
 *   getResultDetails() will return the remote error-code and can be evaluated
 *   by anoubis_strerror or similar.
 */
class ComPolicySendTask : public ComTask
{
	public:
		/**
		 * Default-c'tor.
		 *
		 * You explicity need to set the policy!
		 *
		 * @see setPolicy()
		 */
		ComPolicySendTask(void);

		/**
		 * Destroy a ComPolicySendTask.
		 * @param None.
		 */
		~ComPolicySendTask(void);

		/**
		 * Provides the policy to be sent.
		 *
		 * Uid and priority are extracted from the given policy. The
		 * ruleset is serialized and specifies the return-code of the
		 * method.
		 *
		 * @param policy The policy to be send to anoubisd
		 * @return On success true is returned. If serialization of the
		 *         ruleset was not successful, false is returned.
		 */
		bool setPolicy(PolicyRuleSet *);

		/**
		 * Provides the policy to be sent.
		 *
		 * You also need to specify uid and priority of the policy! The
		 * ruleset is serialized and specifies the return-code of the
		 * method.
		 *
		 * @param policy The policy to be send to anoubisd
		 * @param uid The uid of the policy
		 * @param prio The priority of the policy
		 * @return On success true is returned. If serialization of the
		 *         ruleset was not successful, false is returned.
		 */
		bool setPolicy(struct apn_ruleset *, uid_t, int);

		/**
		 * Returns the assigned uid of the policy.
		 *
		 * @return Assigned uid
		 */
		uid_t getUid(void) const;

		/**
		 * Returns the assigned priority of the policy.
		 *
		 * - prio = 0: admin policy
		 * - prio = 1: user policy
		 *
		 * @return The assigned priority
		 */
		int getPriority(void) const;

		/**
		 * Tests whether a private key is assigned to the task.
		 * @return true is returned, if a private key is assigned,
		 *         false otherwise.
		 */
		bool havePrivateKey(void) const;

		/**
		 * Configures a private-key.
		 *
		 * If a private key is assigned, the policy is signed with the
		 * private key before. Then, the policy together with the
		 * resulting signature is send to Anoubis-daemon.
		 *
		 * As soon as the signature is calculated, the internal
		 * assignment to the private key is removed. Thus, if you want
		 * to re-use the same instance again, don't forget to call this
		 * method again!
		 *
		 * @param key The private key
		 *
		 * @see PrivKey
		 */
		void setPrivateKey(struct anoubis_sig *);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

		/**
		 * Implementation of Task::done().
		 */
		bool done(void);

	private:
		char				*policy_content_;
		uid_t				 uid_;
		int				 prio_;
		struct anoubis_sig		*privKey_;
		struct anoubis_transaction	*ta_;

		/**
		 * Store the id of the ruleset currently been sent.
		 */
		long ruleSetId_;

		static bool getPolicyContent(struct apn_ruleset *, wxString &);
};

#endif	/* _COMPOLICYSENDTASK_H_ */
