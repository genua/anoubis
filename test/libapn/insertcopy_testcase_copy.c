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
	fprintf(sfp, "9: /bin/sh sha256 \\\n");
	fprintf(sfp, "a123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "1: allow connect tcp all\n");
	fprintf(sfp, "2: allow accept tcp from any to any\n");
	fprintf(sfp, "3: allow connect tcp from any to 1.2.3.4\n");
	fprintf(sfp, "4: allow accept tcp from any to 1.2.3.4 port www\n");
	fprintf(sfp, "5: allow connect tcp from 1.2.3.4 to any\n");
	fprintf(sfp, "6: allow accept tcp from 1.2.3.4 port www to any\n");
	fprintf(sfp, "7: allow connect tcp from 1.2.3.0/24 to 4.0.0.0/8\n");
	fprintf(sfp, "8: default deny\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "12: /usr/bin/ssh sha256 \\\n");
	fprintf(sfp, "b123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "10: default deny\n");
	fprintf(sfp, "11: allow connect tcp all\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "16: /sbin/dhclient sha256 \\\n");
	fprintf(sfp, "c123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "13: default deny\n");
	fprintf(sfp, "14: allow raw\n");
	fprintf(sfp, "15: allow send udp all\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "\n");
	fprintf(sfp, "19: /bin/sh \\\n");
	fprintf(sfp, "d123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "17: default deny\n");
	fprintf(sfp, "18: allow connect tcp from any to 1.2.3.4 port www\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "22: /usr/bin/ssh \\\n");
	fprintf(sfp, "e123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "20: default deny\n");		/* This is rule 20 */
	fprintf(sfp, "21: allow connect alert tcp all\n"); /* This is rule 21 */
	fprintf(sfp, "}\n");
	fprintf(sfp, "26: /sbin/dhclient \\\n");
	fprintf(sfp, "f123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "23: default deny\n");
	fprintf(sfp, "24: allow raw\n");
	fprintf(sfp, "25: allow send log udp all\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "28: any {\n");
	fprintf(sfp, "27: default deny\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "}\n");

	fflush(sfp);

	return(sfn);
}

struct apn_rule *
generate_alfrule(void)
{
	struct apn_rule	*alf;

	if ((alf = calloc(1, sizeof(struct apn_rule))) == NULL)
		return (NULL);

	alf->apn_type = APN_ALF_FILTER;
	alf->rule.afilt.action = APN_ACTION_ALLOW;
	alf->rule.afilt.filtspec.log = 0;
	alf->rule.afilt.filtspec.proto = IPPROTO_TCP;
	alf->rule.afilt.filtspec.netaccess = APN_CONNECT;

	return (alf);
}


struct apn_rule *
generate_rule(void)
{
	struct apn_rule		*alf;
	struct apn_rule		*rule;
	struct apn_app		*app;

	if ((alf = generate_alfrule()) == NULL)
		return (NULL);
	if ((rule = calloc(1, sizeof(struct apn_rule))) == NULL) {
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
	app->subject.type = APN_CS_CSUM;
	app->subject.value.csum = calloc(APN_HASH_SHA256_LEN, sizeof(u_int8_t));
	*(unsigned long *)app->subject.value.csum = htonl(0xdeadbeef);

	rule->apn_type = APN_ALF;
	rule->apn_id = 0;
	TAILQ_INIT(&rule->rule.chain);
	TAILQ_INSERT_TAIL(&rule->rule.chain, alf, entry);
	rule->app = app;

	return (rule);
}

START_TEST(tc_Insert1)
{
	struct apn_rule		*rule, *rule2;
	struct apn_ruleset	*rs;
	char			*file;
	int			 ret;
	unsigned int		 saved;

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

	rule = generate_rule();
	fail_if(rule == NULL, "generate_rule() failed");

	/*
	 * Put in before application rule with ID 22. *
	 */
	ret = apn_insert(rs, rule, 22);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Insert failed with %d", ret);

	/*
	 * Dump the new rule set, so the perl wrapper can check the
	 * output.
	 */
	ret = apn_print_ruleset(rs, APN_FLAG_VERBOSE|APN_FLAG_VERBOSE2,
	    stdout);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Printing failed");

	saved = rule->apn_id;
	rule2 = generate_rule();
	/* Try to put in before application rule with bogus ID 1234 */
	ret = apn_insert(rs, rule2, 1234);
	fail_if(ret != 1, "Insert did not fail, but should have!");
	fail_if(rule->apn_id != saved, "Rule ID was modified (%d => %d)!",
	    saved, rule->apn_id);

	/*
	 * Try to put in before rule with the ID 23, which is not an
	 * application rule.
	 */
	ret = apn_insert(rs, rule2, 23);
	fail_if(ret != 1, "Insert did not fail, but should have!");
	fail_if(rule->apn_id != saved, "Rule ID was modified (%d => %d)!",
	    saved, rule->apn_id);

	apn_free_ruleset(rs);
}
END_TEST

START_TEST(tc_Insert2)
{
	struct apn_rule		*rule1, *rule2, *rule3;
	struct apn_ruleset	*rs;
	char			*file;
	int			 ret;
	unsigned int		 s1, s2;

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
	 * Put in before alf filter rule with ID 24, see comment in
	 * generate_file() above!
	 * Rule 24 is in the middle of a rule chain.
	 */
	rule1 = generate_alfrule();
	fail_if(rule1 == NULL, "generate_alfrule() failed");

	ret = apn_insert_alfrule(rs, rule1, 21);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Insert failed");
	fail_if(rule1->apn_id < 29, "ID update failed, got %d expected >= 29",
	    rule1->apn_id);

	/*
	 * Put in before alf filter rule with ID 23, see comment in
	 * generate_file() above!
	 * Rule 23 is the head of a rule chain.
	 */
	rule2 = generate_alfrule();
	fail_if(rule2 == NULL, "generate_alfrule() failed");

	ret = apn_insert_alfrule(rs, rule2, 20);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Insert failed");
	fail_if(rule2->apn_id < 29, "ID update failed, got %d expected >= 29",
	    rule2->apn_id);

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
	s1 = rule1->apn_id; s2 = rule2->apn_id;
	rule3 = generate_alfrule();
	ret = apn_insert_alfrule(rs, rule3, 1234);
	fail_if(ret != 1, "Insert did not fail, but should have!");
	fail_if(rule1->apn_id != s1, "Rule ID was modified!");
	fail_if(rule2->apn_id != s2, "Rule ID was modified!");

	/* Try to put in before application rule 22 */
	ret = apn_insert_alfrule(rs, rule3, 22);
	fail_if(ret != 0, "Insert with application block failed");

	apn_free_ruleset(rs);
}
END_TEST

START_TEST(tc_Copy1)
{
	u_int8_t		 fakecs[APN_HASH_SHA256_LEN];
	struct apn_rule		*rule;
	struct apn_ruleset	*rs;
	char			*file;
	int			 ret;
	struct apn_subject	subject;

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
	 * The application rule has ID 10, the alf rule we want to be
	 * inserted before has ID 7.
	 */
	rule = generate_alfrule();
	fail_if(rule == NULL, "generate_rule() failed");

	subject.type = APN_CS_CSUM;
	subject.value.csum = fakecs;
	memset(fakecs, 0xaa, sizeof(fakecs));
	ret = apn_copyinsert_alf(rs, rule, 7, "/bin/foobar", &subject);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Copy + insert failed with %d", ret);

	/*
	 * Second, insert+copy for a default rule.
	 * The application rule has ID 14, the alf rule we want to be
	 * inserted before has ID 11.
	 */
	rule = generate_alfrule();
	fail_if(rule == NULL, "generate_rule() failed");

	ret = apn_copyinsert_alf(rs, rule, 11, "/bin/foobar", &subject);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Copy + insert failed");

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
	ret = apn_copyinsert_alf(rs, rule, 1234, "/bin/foobar", &subject);
	fail_if(ret == 0, "Copy+insert did not fail, but should have!");
	fail_if(ret < 0, "Copy+insert should fail with ret > 0 but returned %d",
	    ret);

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
