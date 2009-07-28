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

#ifndef _POLICYCHECKS_H_
#define _POLICYCHECKS_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libapn.h"

/**
 * This class implements validators for data types of policies
 */
class PolicyChecks
{
	public:
		/**
		 * Check that apn_log_level is valid
		 * @param[in] 1st apn_log_level
		 * @return True if of valid type
		 */
		static bool checkApnLogLevel(apn_log_level);

		/**
		 * Check that apn_action_type is valid
		 * @param[in] 1st apn_action_type
		 * @return True if of valid type
		 */
		static bool checkApnActionType(apn_action_type);

		/**
		 * Check that apn_direction is valid
		 * @param[in] 1st apn_direction
		 * @return True if of valid type
		 */
		static bool checkApnDirection(apn_direction);
};

#endif	/* _POLICYCHECKS_H_ */
