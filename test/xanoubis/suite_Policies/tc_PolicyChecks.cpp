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

#include <check.h>
#include "PolicyChecks.h"

#define ENUM_FAIL_UPPER_BOUND 32

START_TEST(PolicyChecks_apnLogLevel)
{
	unsigned int lowerBound = 3;

	/* Test the valid apn_log_level Types */
	for (unsigned int i=APN_LOG_NONE; i <= APN_LOG_NORMAL; i++) {
		apn_log_level level = (apn_log_level)i;
		switch (level) {
		case APN_LOG_NONE:
			if (!PolicyChecks::checkApnLogLevel(APN_LOG_NONE))
				fail("ERROR: APN_LOG_NONE is a valid type.");
			break;
		case APN_LOG_NORMAL:
			if (!PolicyChecks::checkApnLogLevel(APN_LOG_NORMAL))
				fail("ERROR: APN_LOG_NORMAL is a valid type.");
			break;
		case APN_LOG_ALERT:
			if (!PolicyChecks::checkApnLogLevel(APN_LOG_ALERT))
				fail("ERROR: APN_LOG_ALERT is a valid type.");
			break;
		}
	}

	/* Test invalid apn_log_level Types which are out of range */
	for (unsigned int i=lowerBound; i < ENUM_FAIL_UPPER_BOUND; i++) {
		if (PolicyChecks::checkApnLogLevel((apn_log_level)i))
			fail("ERROR: accepted an invalid type.");
	}
}
END_TEST

START_TEST(PolicyChecks_apnActionType)
{
	unsigned int lowerBound = 4;

	/* Test the valid apn_action_type Types */
	for (unsigned int i=APN_ACTION_ALLOW; i <= APN_ACTION_CONTINUE; i++) {
		apn_action_type action = (apn_action_type)i;
		switch (action) {
		case APN_ACTION_ALLOW:
			if (!PolicyChecks::checkApnActionType(APN_ACTION_ALLOW))
				fail("APN_ACTION_ALLOW: is a valid type.");
			break;
		case APN_ACTION_DENY:
			if (!PolicyChecks::checkApnActionType(APN_ACTION_DENY))
				fail("APN_ACTION_DENY: is a valid type.");
			break;
		case APN_ACTION_ASK:
			if (!PolicyChecks::checkApnActionType(APN_ACTION_ASK))
				fail("APN_ACTION_ASK: is a valid type.");
			break;
		case APN_ACTION_CONTINUE:
			if (!PolicyChecks::checkApnActionType(APN_ACTION_CONTINUE))
				fail("APN_ACTION_CONTINUE: is a valid type.");
			break;
		}
	}

	/* Test invalid apn_action_type Types which are out of range */
	for (unsigned int i=lowerBound; i < ENUM_FAIL_UPPER_BOUND; i++) {
		if (PolicyChecks::checkApnActionType((apn_action_type)i))
			fail("ERROR: accepted an invalid type.");
	}
}
END_TEST

START_TEST(PolicyChecks_apnDirection)
{
	unsigned int lowerBound = 5;

	/* Test the valid apn_direction Types */
	for (unsigned int i=APN_CONNECT; i <= APN_BOTH; i++) {
		apn_direction direction = (apn_direction)i;
		switch (direction) {
		case APN_CONNECT:
			if (!PolicyChecks::checkApnDirection(APN_CONNECT))
				fail("APN_CONNECT: is a valid type.");
			break;
		case APN_ACCEPT:
			if (!PolicyChecks::checkApnDirection(APN_ACCEPT))
				fail("APN_ACCEPT: is a valid type.");
			break;
		case APN_SEND:
			if (!PolicyChecks::checkApnDirection(APN_SEND))
				fail("APN_SEND: is a valid type.");
			break;
		case APN_RECEIVE:
			if (!PolicyChecks::checkApnDirection(APN_RECEIVE))
				fail("APN_RECEIVE: is valid type.");
			break;
		case APN_BOTH:
			if (!PolicyChecks::checkApnDirection(APN_BOTH))
				fail("APN_BOTH: is a valid type.");
			break;
		}
	}

	/* Test invalid apn_direction Types which are out of range */
	for (unsigned int i=lowerBound; i < ENUM_FAIL_UPPER_BOUND; i++) {
		if (PolicyChecks::checkApnDirection((apn_direction)i))
			fail("ERROR: accepted an invalid type.");
	}
}
END_TEST

START_TEST(PolicyChecks_apnHashType)
{
	unsigned int lowerBound = 2;

	/* Test the valid apn_hash_type Types */
	for (unsigned int i=APN_HASH_NONE; i <= APN_HASH_SHA256; i++) {
		apn_hash_type type = (apn_hash_type)i;
		switch (type) {
		case APN_HASH_NONE:
			if (!PolicyChecks::checkApnHashType(APN_HASH_NONE))
				fail("APN_HASH_NONE: is a valid type.");
			break;
		case APN_HASH_SHA256:
			if (!PolicyChecks::checkApnHashType(APN_HASH_SHA256))
				fail("APN_HASH_SHA256: is a valid type.");
			break;
		}
	}

	/* Test invalid apn_hash_type Types which are out of range */
	for (unsigned int i=lowerBound; i < ENUM_FAIL_UPPER_BOUND; i++) {
		if (PolicyChecks::checkApnHashType((apn_hash_type)i))
			fail("ERROR: accepted an invalid type.");
	}
}
END_TEST

TCase *
getTc_PolicyChecks(void)
{
	TCase *testCase;

	testCase = tcase_create("PolicyChecks");
	tcase_add_test(testCase, PolicyChecks_apnLogLevel);
	tcase_add_test(testCase, PolicyChecks_apnActionType);
	tcase_add_test(testCase, PolicyChecks_apnDirection);
	tcase_add_test(testCase, PolicyChecks_apnHashType);

	return (testCase);
}
