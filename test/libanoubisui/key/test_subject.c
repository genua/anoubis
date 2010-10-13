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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <check.h>
#include <anoubischeck.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <anoubis_keygen.h>
#include <anoubis_errno.h>

#include <openssl/err.h>
#include <openssl/pem.h>

START_TEST(subject_default)
{
	struct anoubis_keysubject	*subject = NULL;
	int				 i;
	struct passwd			*pw;

	/* only run the test if username lookups work */
	pw = getpwuid(getuid());
	if (pw == NULL)
		return;

	/* Do it several times to catch more potential memory errors. */
	for (i=0; i<50; ++i) {
		subject = anoubis_keysubject_defaults();
		fail_if(subject == NULL, "Failed to allocate subject");
		fail_if(subject->country == NULL
		    || strlen(subject->country) != 2,
		    "Invalid country in default subject");
		fail_if(subject->name == NULL || strlen(subject->name) <= 0,
		    "No name in default subject");
		int len = strlen(subject->name) - 1;
		fail_if(subject->name[len] == ',',
		    "Unexpected comma at the end of name (%s)", subject->name);
		fail_if(subject->email == NULL || strlen(subject->email) <= 0,
		    "No email in default subject");
		anoubis_keysubject_destroy(subject);
	}
}
END_TEST

#define CMPONE(S1, S2, FIELD) do {					\
		fail_if(((S1)->FIELD == NULL) != ((S2)->FIELD == NULL),	\
		    "Different fields in subject (%s)", #FIELD);	\
		fail_if((S1)->FIELD != NULL				\
		    && strcmp((S1)->FIELD, (S2)->FIELD) != 0,		\
		    "String for subject field " #FIELD " differ "	\
		    "(%s vs %s)", (S1)->FIELD, (S2)->FIELD);		\
	} while (0)

#define assert_subject_equal(SJ1, SJ2)			\
	CMPONE((SJ1), (SJ2), country);			\
	CMPONE((SJ1), (SJ2), state);			\
	CMPONE((SJ1), (SJ2), locality);			\
	CMPONE((SJ1), (SJ2), organization);			\
	CMPONE((SJ1), (SJ2), orgunit);			\
	CMPONE((SJ1), (SJ2), name);				\
	CMPONE((SJ1), (SJ2), email);

struct test_case_pair {
	const char			*str;
	const struct anoubis_keysubject	 key;
};


static struct test_case_pair testcases[] = {
{
	.str = "/C=DE/ST=Some-State/L=Muc/O=Internet Widgits Pty Ltd/"
	    "CN=Christian Ehrhardt",
	.key = {
		"DE", "Some-State", "Muc", "Internet Widgits Pty Ltd",
		NULL, "Christian Ehrhardt", NULL,
	},
},
{
	.str = "/C=DE/ST=Some-State/L=Muc/O=Internet Widgits Pty Ltd/"
	    "OU=Some Department/CN=Christian Ehrhardt/"
	    "emailAddress=foo@bar.baz.com",
	.key = {
		"DE", "Some-State", "Muc", "Internet Widgits Pty Ltd",
		"Some Department", "Christian Ehrhardt", "foo@bar.baz.com",
	},
},
{
	.str = "/ST=Some-State/L=Muc/CN=Christian Ehrhardt",
	.key = {
		NULL, "Some-State", "Muc", NULL, NULL,
		"Christian Ehrhardt", NULL,
	},
},
{
	.str = "/C=DE/O=Internet Widgits Pty Ltd/emailAddress=foo@bar.baz.com",
	.key = {
		"DE", NULL, NULL, "Internet Widgits Pty Ltd", NULL, NULL,
		"foo@bar.baz.com"
	},
},
};

static X509 *
read_certificate(const char *name)
{
	char	*crtfile;
	FILE	*fh;
	X509	*cert;

	fail_unless(asprintf(&crtfile, "%s/%s", getenv("KEYDIR"), name) > 0);
	fh = fopen(crtfile, "r");
	fail_unless(fh != NULL, "Failed to open \"%s\" for reading: %s",
	    crtfile, anoubis_strerror(errno));

	cert = PEM_read_X509(fh, NULL, NULL, NULL);
	fail_unless(cert != NULL, "Failed to load certificate: %li",
	    ERR_get_error());

	fclose(fh);
	free(crtfile);

	return (cert);
}

START_TEST(subject_fromX509_complete)
{
	X509				*cert;
	struct anoubis_keysubject	*subject;

	cert = read_certificate("keys/complete.cert");
	subject = anoubis_keysubject_fromX509(cert);
	fail_unless(subject != NULL, "Failed to extract subject");

	fail_unless(strcmp(subject->country, "AU") == 0,
	    "Wrong country (\"%s\" vs. \"AU\")", subject->country);
	fail_unless(strcmp(subject->state, "Some-State") == 0,
	    "Wrong state (\"%s\" vs. \"Some-State\")", subject->state);
	fail_unless(strcmp(subject->locality, "Some-City") == 0,
	    "Wrong locality (\"%s\" vs. \"Some-City\")", subject->locality);
	fail_unless(strcmp(subject->organization, "Some-Org") == 0,
	    "Wrong organization (\"%s\" vs. \"Some-Org\")",
	    subject->organization);
	fail_unless(strcmp(subject->orgunit, "Some-OrgUnit") == 0,
	    "Wrong orgunit (\"%s\" vs. \"Some-OrgUnit\")", subject->orgunit);
	fail_unless(strcmp(subject->name, "Some-Name") == 0,
	    "Wrong name (\"%s\" vs. \"Some-Name\")", subject->name);
	fail_unless(strcmp(subject->email, "Some-Email") == 0,
	    "Wrong email (\"%s\" vs. \"Some-Email\")", subject->email);

	anoubis_keysubject_destroy(subject);
	X509_free(cert);
}
END_TEST

START_TEST(subject_fromX509_incomplete)
{
	X509				*cert;
	struct anoubis_keysubject	*subject;

	cert = read_certificate("keys/no_state.cert");
	subject = anoubis_keysubject_fromX509(cert);
	fail_unless(subject != NULL, "Failed to extract subject");

	fail_unless(strcmp(subject->country, "AU") == 0,
	    "Wrong country (\"%s\" vs. \"AU\")", subject->country);
	fail_unless(subject->state == NULL,
	    "Wrong state (\"%s\" vs. \"Some-State\")", subject->state);
	fail_unless(strcmp(subject->locality, "Some-City") == 0,
	    "Wrong locality (\"%s\" vs. \"Some-City\")", subject->locality);
	fail_unless(strcmp(subject->organization, "Some-Org") == 0,
	    "Wrong organization (\"%s\" vs. \"Some-Org\")",
	    subject->organization);
	fail_unless(strcmp(subject->orgunit, "Some-OrgUnit") == 0,
	    "Wrong orgunit (\"%s\" vs. \"Some-OrgUnit\")", subject->orgunit);
	fail_unless(strcmp(subject->name, "Some-Name") == 0,
	    "Wrong name (\"%s\" vs. \"Some-Name\")", subject->name);
	fail_unless(strcmp(subject->email, "Some-Email") == 0,
	    "Wrong email (\"%s\" vs. \"Some-Email\")", subject->email);

	anoubis_keysubject_destroy(subject);
	X509_free(cert);
}
END_TEST

START_TEST(subject_fromX509_umlauts)
{
	X509				*cert;
	struct anoubis_keysubject	*subject;

	cert = read_certificate("keys/umlauts.cert");
	subject = anoubis_keysubject_fromX509(cert);
	fail_unless(subject != NULL, "Failed to extract subject");

	fail_unless(strcmp(subject->country, "AU") == 0,
	    "Wrong country (\"%s\" vs. \"AU\")", subject->country);
	fail_unless(strcmp(subject->state, "Some-State") == 0,
	    "Wrong state (\"%s\" vs. \"Some-State\")", subject->state);
	fail_unless(strcmp(subject->locality, "üüü") == 0,
	    "Wrong locality (\"%s\" vs. \"Some-City\")", subject->locality);
	fail_unless(strcmp(subject->organization, "Some-Org") == 0,
	    "Wrong organization (\"%s\" vs. \"Some-Org\")",
	    subject->organization);
	fail_unless(strcmp(subject->orgunit, "Some-OrgUnit") == 0,
	    "Wrong orgunit (\"%s\" vs. \"Some-OrgUnit\")", subject->orgunit);
	fail_unless(strcmp(subject->name, "Some-Name") == 0,
	    "Wrong name (\"%s\" vs. \"Some-Name\")", subject->name);
	fail_unless(strcmp(subject->email, "Some-Email") == 0,
	    "Wrong email (\"%s\" vs. \"Some-Email\")", subject->email);

	anoubis_keysubject_destroy(subject);
	X509_free(cert);
}
END_TEST

START_TEST(subject_tostring)
{
	char		*name;
	int		 i, j, cnt;

	cnt = sizeof(testcases) / sizeof(testcases[0]);
	for (i=0; i<50; ++i) {
		for (j=0; j<cnt; ++j) {
			name = anoubis_keysubject_tostring(&testcases[j].key);
			fail_if(name == NULL, "Cannot convert subject %d", j);
			fail_if(strcmp(name, testcases[j].str) != 0,
			    "Result of anoubis_keysubject_tostring differs "
			    "from expected string (%s vs %s)",
			    name, testcases[j].str);
			free(name);
		}
	}
}
END_TEST

static TCase *
subject_tcase(void)
{
	TCase		*tc = tcase_create("SubjectTcase");

	tcase_add_test(tc, subject_default);
	tcase_add_test(tc, subject_fromX509_complete);
	tcase_add_test(tc, subject_fromX509_incomplete);
	tcase_add_test(tc, subject_fromX509_umlauts);
	tcase_add_test(tc, subject_tostring);

	return tc;
}

static Suite *
subject_testsuite(void)
{
	Suite		*s = suite_create("SubjectSuite");

	suite_add_tcase(s, subject_tcase());
	return s;
}

int
main(void)
{
	int		 result;
	SRunner		*suiterunner = srunner_create(subject_testsuite());

	srunner_run_all(suiterunner, CK_NORMAL);
	result = check_eval_srunner(suiterunner);
	srunner_free(suiterunner);

	return result;
}
