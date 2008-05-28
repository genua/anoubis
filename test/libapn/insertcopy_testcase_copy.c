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

char *
generate_file(void)
{
	char	*sfn;
	FILE	*sfp;
	int	 fd;

	if (asprintf(&sfn, "/tmp/copyinsert.XXXXXX") == -1)
		fail_if(1, "asprintf");
	if ((fd = mkstemp(sfn)) == -1)
		fail_if(1, "mkstemp");
	if ((sfp = fdopen(fd, "w+")) == NULL) {
		unlink(sfn);
		fail_if(1, "fdopen");
	}

	/* To avoid races, we keep the file open. */

	fprintf(sfp, "alf {\n");
	fprintf(sfp, "/bin/sh sha256 \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "allow connect tcp all\n");
	fprintf(sfp, "allow accept tcp from any to any\n");
	fprintf(sfp, "allow connect tcp from any to 1.2.3.4\n");
	fprintf(sfp, "allow accept tcp from any to 1.2.3.4 port www\n");
	fprintf(sfp, "allow connect tcp from 1.2.3.4 to any\n");
	fprintf(sfp, "allow accept tcp from 1.2.3.4 port www to any\n");
	fprintf(sfp, "allow connect tcp from 1.2.3.0/24 to 4.0.0.0/8\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "context new any\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "/usr/bin/ssh sha256 \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "allow connect tcp all\n");
	fprintf(sfp, "context new { /sbin/dhclient sha256 \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef,\n");
	fprintf(sfp, "/bin/sh \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef }\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "/sbin/dhclient sha256 \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "allow raw\n");
	fprintf(sfp, "allow send udp all\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "\n");
	fprintf(sfp, "/bin/sh \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "context new any\n");
	fprintf(sfp, "allow connect tcp from any to 1.2.3.4 port www\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "/usr/bin/ssh \\\n");	/* This rule has ID 24 */
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "default deny\n");		/* This is rule 22 */
	fprintf(sfp, "allow connect alert tcp all\n"); /* This is rule 23 */
	fprintf(sfp, "}\n");
	fprintf(sfp, "/sbin/dhclient \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "allow raw\n");
	fprintf(sfp, "allow send log udp all\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "any {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "context new any\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "}\n");

	fflush(sfp);

	return(sfn);
}

struct apn_alfrule *
generate_alfrule(void)
{
	struct apn_alfrule	*alf;

	if ((alf = calloc(1, sizeof(struct apn_alfrule))) == NULL)
		return (NULL);

	alf->type = APN_ALF_FILTER;
	alf->rule.afilt.action = APN_ACTION_ALLOW;
	alf->rule.afilt.filtspec.log = 0;
	alf->rule.afilt.filtspec.af = AF_INET;
	alf->rule.afilt.filtspec.proto = IPPROTO_TCP;
	alf->rule.afilt.filtspec.netaccess = APN_CONNECT;

	return (alf);
}


struct apn_rule *
generate_rule(void)
{
	struct apn_alfrule	*alf;
	struct apn_rule		*rule;
	struct apn_app		*app;

	if ((alf = generate_alfrule()) == NULL)
		return (NULL);
	if ((rule = calloc(1, sizeof(struct apn_alfrule))) == NULL) {
		free(alf);
		return (NULL);
	}
	if ((app = calloc(1, sizeof(struct apn_app))) == NULL) {
		free(alf);
		free(rule);
		return (NULL);
	}

	if ((app->name = strdup("/bin/sh")) == NULL) {
		free(alf);
		free(rule);
		free(app);
		return (NULL);
	}
	app->hashtype = APN_HASH_SHA256;
	*(unsigned long *)app->hashvalue = htonl(0xdeadbeef);

	rule->type = APN_ALF;
	rule->rule.alf = alf;
	rule->app = app;

	return (rule);
}

START_TEST(tc_Insert1)
{
	struct apn_rule		*rule;
	struct apn_ruleset	*rs;
	char			*file;
	int			 ret;

	file = generate_file();
	ret = apn_parse(file, &rs, 0);
	unlink(file);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Parsing failed");

	/*
	 * Dump the rule set, so the perl wrapper can check the output.
	 */
	ret = apn_print_ruleset(rs, APN_FLAG_VERBOSE|APN_FLAG_VERBOSE2,
	    stdout);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Printing failed");

	fail_if(rs->maxid != 32, "Maximum ID in ruleset is %d, but should be"
	    " 32!", rs->maxid);

	rule = generate_rule();
	fail_if(rule == NULL, "generate_rule() failed");

	/*
	 * Put in before application rule with ID 24, see comment in
	 * generate_file() above! The inserted rule must have the new ID 33.
	 */
	ret = apn_insert(rs, rule, 24);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Insert failed");
	fail_if(rule->id != 33, "ID update failed, got %d expected 33",
	    rule->id);

	/*
	 * Dump the new rule set, so the perl wrapper can check the
	 * output.
	 */
	ret = apn_print_ruleset(rs, APN_FLAG_VERBOSE|APN_FLAG_VERBOSE2,
	    stdout);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Printing failed");

	/* Try to put in before application rule with bogus ID 1234 */
	ret = apn_insert(rs, rule, 1234);
	fail_if(ret != 1, "Insert did not fail, but should have!");
	fail_if(rule->id != 33, "Rule ID was modified!");

	/*
	 * Try to put in before rule with the ID 22, which is not an
	 * application rule.
	 */
	ret = apn_insert(rs, rule, 22);
	fail_if(ret != 1, "Insert did not fail, but should have!");
	fail_if(rule->id != 33, "Rule ID was modified!");

	apn_free_ruleset(rs);
}
END_TEST

START_TEST(tc_Insert2)
{
	struct apn_alfrule	*rule;
	struct apn_ruleset	*rs;
	char			*file;
	int			 ret;

	file = generate_file();
	ret = apn_parse(file, &rs, 0);
	unlink(file);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Parsing failed");

	/*
	 * Dump the rule set, so the perl wrapper can check the output.
	 */
	ret = apn_print_ruleset(rs, APN_FLAG_VERBOSE|APN_FLAG_VERBOSE2,
	    stdout);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Printing failed");

	/*
	 * Put in before alf filter rule with ID 23, see comment in
	 * generate_file() above!
	 * Rule 23 is in the middle of a rule chain.
	 */
	rule = generate_alfrule();
	fail_if(rule == NULL, "generate_alfrule() failed");

	ret = apn_insert_alfrule(rs, rule, 23);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Insert failed");
	fail_if(rule->id != 32, "ID update failed, got %d expected 32",
	    rule->id);

	/*
	 * Put in before alf filter rule with ID 22, see comment in
	 * generate_file() above!
	 * Rule 22 is the head of a rule chain.
	 */
	rule = generate_alfrule();
	fail_if(rule == NULL, "generate_alfrule() failed");

	ret = apn_insert_alfrule(rs, rule, 22);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Insert failed");
	fail_if(rule->id != 33, "ID update failed, got %d expected 33",
	    rule->id);

	/*
	 * Dump the new rule set, so the perl wrapper can check the
	 * output.
	 */
	ret = apn_print_ruleset(rs, APN_FLAG_VERBOSE|APN_FLAG_VERBOSE2,
	    stdout);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Printing failed");

	/* Try to put in before alf rule with bogus ID 1234 */
	ret = apn_insert_alfrule(rs, rule, 1234);
	fail_if(ret != 1, "Insert did not fail, but should have!");
	fail_if(rule->id != 33, "Rule ID was modified!");

	/* Try to put in before application rule 24 */
	ret = apn_insert_alfrule(rs, rule, 24);
	fail_if(ret != 1, "Insert did not fail, but should have!");
	fail_if(rule->id != 33, "Rule ID was modified!");

	apn_free_ruleset(rs);
}
END_TEST

START_TEST(tc_Copy1)
{
	u_int8_t		 fakecs[APN_HASH_SHA256_LEN];
	struct apn_alfrule	*rule;
	struct apn_ruleset	*rs;
	char			*file;
	int			 ret;

	file = generate_file();
	ret = apn_parse(file, &rs, 0);
	unlink(file);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Parsing failed");

	/*
	 * Dump the rule set, so the perl wrapper can check the output.
	 */
	ret = apn_print_ruleset(rs, APN_FLAG_VERBOSE|APN_FLAG_VERBOSE2,
	    stdout);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Printing failed");


	/*
	 * First, insert+copy for a rule in the first application rule.
	 * The application rule has ID 9, the alf rule we want to be
	 * inserted before has ID 6.
	 */
	rule = generate_alfrule();
	fail_if(rule == NULL, "generate_rule() failed");

	memset(fakecs, 0xaa, sizeof(fakecs));
	ret = apn_copyinsert(rs, rule, 6, "/bin/foobar", fakecs,
	    APN_HASH_SHA256);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Copy + insert failed");
	fail_if(rule->id != 38, "Rule should have new id 38, but has %d",
	    rule->id);
	fail_if(rs->maxid != 42, "Wrong maxid, is %d should be 42", rs->maxid);

	/*
	 * Second, insert+copy for a default rule.
	 * The application rule has ID 13, the alf rule we want to be
	 * inserted before has ID 10.
	 */
	rule = generate_alfrule();
	fail_if(rule == NULL, "generate_rule() failed");

	memset(fakecs, 0xaa, sizeof(fakecs));
	ret = apn_copyinsert(rs, rule, 10, "/bin/foobar", fakecs,
	    APN_HASH_SHA256);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Copy + insert failed");
	fail_if(rule->id != 42, "Rule should have new id 42, but has %d",
	    rule->id);
	fail_if(rs->maxid != 46, "Wrong maxid, is %d should be 46", rs->maxid);

	/*
	 * Dump the new rule set, so the perl wrapper can check the
	 * output.
	 */
	ret = apn_print_ruleset(rs, APN_FLAG_VERBOSE|APN_FLAG_VERBOSE2,
	    stdout);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Printing failed");


	/* Try to copy+insert using the bogus ID 1234 */
	ret = apn_copyinsert(rs, rule, 1234, "/bin/foobar", fakecs,
	    APN_HASH_SHA256);
	fail_if(ret != 1, "Copy+insert did not fail, but should have!");

	apn_free_ruleset(rs);
}
END_TEST

TCase *
insertcopy_testcase_insert1(void)
{
	/* sessions test case */
	TCase *tc_insert1 = tcase_create("Insert test 1");

	tcase_add_test(tc_insert1, tc_Insert1);

	return (tc_insert1);
}

TCase *
insertcopy_testcase_insert2(void)
{
	/* sessions test case */
	TCase *tc_insert2 = tcase_create("Insert test 2");

	tcase_add_test(tc_insert2, tc_Insert2);

	return (tc_insert2);
}

TCase *
insertcopy_testcase_copy1(void)
{
	/* sessions test case */
	TCase *tc_copy1 = tcase_create("Copy test 1");

	tcase_add_test(tc_copy1, tc_Copy1);

	return (tc_copy1);
}
