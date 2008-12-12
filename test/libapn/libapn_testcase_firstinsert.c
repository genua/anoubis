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

#include <sys/param.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <apn.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

static struct apn_ruleset *
create_empty_ruleset(void)
{
	struct iovec		 iv;
	struct apn_ruleset	*rs;

	iv.iov_base = (void *)"\nalf {\n }\nsandbox {\n }\n";
	iv.iov_len  = 34;

	if (apn_parse_iovec("<iov>", &iv, 1, &rs, 0) != 0) {
		apn_print_errors(rs, stderr);
		fail("Couldn't create empty apn rule set.");
		return (NULL);
	}

	return (rs);
}

static struct apn_rule *
create_empty_app_rule(int type)
{
	struct apn_rule	*rule;

	rule = calloc(1, sizeof(struct apn_rule));
	fail_if(rule == NULL, "Couldn't create new apn rule");
	rule->apn_type = type;
	rule->apn_id = 0;
	rule->app = NULL;
	return rule;
}

static struct apn_rule *
create_empty_filter(int type)
{
	struct apn_rule	*filter;

	filter = calloc(1, sizeof(struct apn_rule));
	fail_if(filter == NULL, "Couldn't create new alf filter");
	filter->apn_id = 0;

	switch(type) {
	case APN_ALF:
		filter->apn_type = APN_ALF_FILTER;
		filter->rule.afilt.filtspec.proto = IPPROTO_TCP;
		filter->rule.afilt.filtspec.af = AF_INET;
		filter->rule.afilt.filtspec.netaccess = APN_CONNECT;
		break;
	case APN_SFS:
		filter->apn_type = APN_SFS_ACCESS;
		filter->rule.sfsaccess.path = NULL;
		filter->rule.sfsaccess.subject.type = APN_CS_NONE;
		filter->rule.sfsaccess.valid.action = APN_ACTION_DENY;
		filter->rule.sfsaccess.valid.log = APN_LOG_NONE;
		filter->rule.sfsaccess.invalid.action = POLICY_ALLOW;
		filter->rule.sfsaccess.invalid.log = APN_LOG_NONE;
		filter->rule.sfsaccess.unknown.action = POLICY_ALLOW;
		filter->rule.sfsaccess.unknown.log = APN_LOG_NONE;
		break;
	case APN_CTX:
		filter->apn_type = APN_CTX_RULE;
		filter->rule.apncontext.type = APN_CTX_NEW;
		filter->rule.apncontext.application = NULL;
		break;
	case APN_SB:
		filter->apn_type = APN_SB_ACCESS;
		filter->rule.sbaccess.path = NULL;
		filter->rule.sbaccess.cs.type = APN_CS_NONE;
		filter->rule.sbaccess.amask = APN_SBA_READ;
		filter->rule.sbaccess.action = APN_ACTION_ALLOW;
		filter->rule.sbaccess.log = APN_LOG_NONE;
		break;
	default:
		free(filter);
		return NULL;
	}

	return (filter);
}

static int
add2app(int type, struct apn_ruleset *rs, struct apn_rule *rule, int id)
{
	switch(type) {
	case APN_ALF:
		return apn_add2app_alfrule(rs, rule, id);
	case APN_SB:
		return apn_add2app_sbrule(rs, rule, id);
	case APN_CTX:
		return apn_add2app_ctxrule(rs, rule, id);
	}
	return 1;
}


START_TEST(firstinsert_std)
{
	static const int	types[] = { APN_CTX, APN_ALF, APN_SB, -1 };
	int			i;

	for (i=0; types[i] >= 0; ++i) {
		int			 type = types[i];
		int			 rc;
		struct apn_ruleset	*rs;
		struct apn_rule		*rule;
		struct apn_rule		*filter1;
		struct apn_rule		*filter2;

		rs      = create_empty_ruleset();
		rule    = create_empty_app_rule(type);
		filter1 = create_empty_filter(type);
		filter2 = create_empty_filter(type);

		rc = apn_insert(rs, rule, 1);
		fail_if(rc == 0, "Could insert first rule (fail expected)!");

		rc = apn_add(rs, rule);
		fail_if(rc != 0, "Couldn't add first rule.");

		rc = add2app(type, rs, filter1, 333);
		fail_if(rc == 0, "Could insert first filter (fail expected)!");

		rc = add2app(type, rs, filter1, rule->apn_id);
		fail_if(rc != 0, "Couldn't add first filter "
		    "(i=%d, type=%d, apn_id=%d)", i, type, rule->apn_id);

		rc = add2app(type, rs, filter2, 444);
		fail_if(rc == 0, "Could insert second filter (fail expected)!");

		rc = add2app(type, rs, filter2, rule->apn_id);
		fail_if(rc != 0, "Couldn't add second filter "
		    "(i=%d, type=%d, apn_id=%d)", i, type, rule->apn_id);
	}
}
END_TEST

START_TEST(firstinsert_sfs)
{
	int			 rc;
	struct apn_ruleset	*rs;
	struct apn_rule		*rule;
	struct apn_rule		*filter1;
	struct apn_rule		*filter2;

	rs      = create_empty_ruleset();
	filter1 = create_empty_filter(APN_SFS);
	filter2 = create_empty_filter(APN_SFS);

	fail_if(filter1 == NULL, "Failed to create sfs filter");
	fail_if(filter2 == NULL, "Failed to create sfs filter");
	rule = TAILQ_FIRST(&(rs->sfs_queue));
	fail_if(rule == NULL, "Ruleset not initialized");

	rc = apn_add2app_sfsrule(rs, filter1, 333);
	fail_if(rc == 0, "Could insert first filter (fail expected)!");

	rc = apn_add2app_sfsrule(rs, filter1, rule->apn_id);
	fail_if(rc != 0, "Couldn't add first filter (rc=%d, rs=%p, id=%d).", rc,
	    rs, rule->apn_id);

	rc = apn_add2app_sfsrule(rs, filter2, 444);
	fail_if(rc == 0, "Could insert second filter (fail expected)!");

	rc = apn_add2app_sfsrule(rs, filter2, rule->apn_id);
	fail_if(rc != 0, "Couldn't add second filter.");
}
END_TEST

TCase *
libapn_testcase_firstinsert(void)
{
	TCase *testcase = tcase_create("First insert");

	tcase_add_test(testcase, firstinsert_std);
	tcase_add_test(testcase, firstinsert_sfs);

	return (testcase);
}
