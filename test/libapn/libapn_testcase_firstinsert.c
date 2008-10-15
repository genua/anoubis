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

	iv.iov_base = (void *)"\nalf {\n }\n sfs {\n }\n sandbox {\n }\n";
	iv.iov_len  = 34;

	if (apn_parse_iovec("<iov>", &iv, 1, &rs, 0) != 0) {
		apn_print_errors(rs, stderr);
		fail("Couldn't create empty apn rule set.");
		return (NULL);
	}

	return (rs);
}

static struct apn_rule *
create_empty_alf_rule(void)
{
	struct apn_rule	*rule;

	rule = (struct apn_rule *)calloc(1, sizeof(struct apn_rule));
	fail_if(rule == NULL, "Couldn't create new apn rule");

	rule->apn_type = APN_ALF;

	rule->app = (struct apn_app *)calloc(1, sizeof(struct apn_app));
	fail_if(rule->app == NULL, "Couldn't create app for new rule");

	rule->app->hashtype = APN_HASH_NONE;

	return (rule);
}

static struct apn_rule *
create_empty_alf_filter(void)
{
	struct apn_rule	*filter;

	filter = (struct apn_rule *)calloc(1, sizeof(struct apn_rule));
	fail_if(filter == NULL, "Couldn't create new alf filter");

	filter->apn_type = APN_ALF_FILTER;
	filter->rule.afilt.filtspec.proto = IPPROTO_TCP;
	filter->rule.afilt.filtspec.af = AF_INET;
	filter->rule.afilt.filtspec.netaccess = APN_CONNECT;

	return (filter);
}

static struct apn_rule *
create_empty_sfs_filter(void)
{
	struct apn_rule	*filter;

	filter = (struct apn_rule *)calloc(1, sizeof(struct apn_rule));
	fail_if(filter == NULL, "Couldn't create new sfs filter");

	filter->apn_id = 0;
	filter->apn_type = APN_SFS_CHECK;

	filter->rule.sfscheck.app = (struct apn_app *)calloc(1,
	    sizeof(struct apn_app));
	if (filter->rule.sfscheck.app == NULL) {
		fail("Couldn't create app for new rule");
	}

	filter->rule.sfscheck.app->hashtype = APN_HASH_NONE;

	return (filter);
}

START_TEST(firstinsert_alf)
{
	int			 rc;
	struct apn_ruleset	*rs;
	struct apn_rule		*rule;
	struct apn_rule		*filter1;
	struct apn_rule		*filter2;

	rs      = create_empty_ruleset();
	rule    = create_empty_alf_rule();
	filter1 = create_empty_alf_filter();
	filter2 = create_empty_alf_filter();

	rc = apn_insert(rs, rule, 1);
	fail_if(rc == 0, "Could insert first rule (fail expected)!");

	rc = apn_add(rs, rule);
	fail_if(rc != 0, "Couldn't add first rule.");

	rc = apn_add2app_alfrule(rs, filter1, 333);
	fail_if(rc == 0, "Could insert first filter (fail expected)!");

	rc = apn_add2app_alfrule(rs, filter1, rule->apn_id);
	fail_if(rc != 0, "Couldn't add first filter.");

	rc = apn_add2app_alfrule(rs, filter2, 444);
	fail_if(rc == 0, "Could insert second filter (fail expected)!");

	rc = apn_add2app_alfrule(rs, filter2, rule->apn_id);
	fail_if(rc != 0, "Couldn't add second filter.");
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
	filter1 = create_empty_sfs_filter();
	filter2 = create_empty_sfs_filter();

	rule = TAILQ_FIRST(&(rs->sfs_queue));
	fail_if(rule == NULL, "Ruleset not initialized");

	rc = apn_add2app_sfsrule(rs, filter1, 333);
	fail_if(rc == 0, "Could insert first filter (fail expected)!");

	rc = apn_add2app_sfsrule(rs, filter1, rule->apn_id);
	fail_if(rc != 0, "Couldn't add first filter.");

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

	tcase_add_test(testcase, firstinsert_alf);
	tcase_add_test(testcase, firstinsert_sfs);

	 return (testcase);
}
