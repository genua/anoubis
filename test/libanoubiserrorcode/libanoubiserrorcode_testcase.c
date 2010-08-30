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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define CATSTRING	"LC_MESSAGES"

#ifdef OPENBSD
#define LOCALE	"de"
#else
#define	LOCALE	"de_DE"
#endif

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <anoubis_errno.h>
#include <anoubischeck.h>

char *tmpdir;

void
libanoubiserrorcode_tc_setup(void)
{
	/*
	* create temporary directory here
	 * with: anoubis/de_DE/LC_MESSAGES/anoubis.mo
	 * in it, this catalog-file will be used by
	 * testcase_de
	 */
	char cmd[200], *tmp, *catalog;
	int errno;

	catalog = getenv("CATALOG");
	if (catalog == NULL)
		catalog = "src/anoubis/po/de.gmo";

	tmpdir = strdup("/tmp/tc_de_testXXXXXX");
	tmp = mkdtemp(tmpdir);

	snprintf(cmd, 200, "mkdir -p %s/%s/%s",
		tmpdir, LOCALE, CATSTRING);
	errno = system(cmd);
	snprintf(cmd, 200, "ln -s %s %s/%s.utf8", LOCALE, tmpdir, LOCALE);
	errno = system(cmd);
	snprintf(cmd, 200, "cp %s %s/%s/%s/anoubis.mo",
		catalog, tmpdir, LOCALE, CATSTRING);
	errno = system(cmd);
}

void
libanoubiserrorcode_tc_teardown(void)
{
	/*
	 * temporary directory created within setup should
	 * be removed here. to do so, the directory/file
	 * anoubis/de_DE/LC_MESSAGES/anoubis.mo has to be 'rm -rf'ed
	 */
	char cmd[200];
	int errno;

	snprintf(cmd, 200, "rm -rf %s", tmpdir);
	errno = system(cmd);
}

static void
run_test(char * predicted_error_string[])
{
	char *teststring;

	/* case errnum < 0 */
	teststring = anoubis_strerror(-1);
	fail_if(strcmp(teststring, predicted_error_string[0]) != 0,
		"errorcode < 0 didn't match error_text: \"%s\" vs \"%s\"",
		teststring, predicted_error_string[0]);

	/* case error from errno */
	teststring = anoubis_strerror(2);
	fail_if(strcmp(teststring, predicted_error_string[1]) != 0,
		"errorcode 2 didn't match error_text: \"%s\" vs \"%s\"",
		teststring, predicted_error_string[1]);

	/* case unspecified anoubis error */
	teststring = anoubis_strerror(1226);
	fail_if(strcmp(teststring, predicted_error_string[2]) != 0,
		"errorcode 1226 didn't match error_text: \"%s\"",
		teststring);

	/* case anoubis error out of range*/
	teststring = anoubis_strerror(4242);
	fail_if(strcmp(teststring, predicted_error_string[3]) != 0,
		"errorcode 4242 didn't match error_text: \"%s\"",
		teststring);

	/* case specified anoubis error */
	teststring = anoubis_strerror(1024);
	fail_if(strcmp(teststring, predicted_error_string[4]) != 0,
		"errorcode 1024 didn't match error_text: \"%s\"",
		teststring);
}

START_TEST(testcase_en)
{
	char *predicted_error_string[] = {
		"invalid error code",
		"No such file or directory",
		"undefined anoubis error code",
		"undefined anoubis error code",
		"Operation not permitted: no certificate found"
	};

	printf("testing english error codes\n");
	run_test(predicted_error_string);
}
END_TEST

START_TEST(testcase_de)
{
	char	*predicted_error_string_iso[] = {
		"Ung\xfcltige Fehlernummer",
		"Datei oder Verzeichnis nicht gefunden",
		"Unbekannte Anoubis-Fehlernummer",
		"Unbekannte Anoubis-Fehlernummer",
		"Operation nicht erlaubt: Kein Zertifikat gefunden"
	};
	char	*predicted_error_string_utf8[] = {
		"Ung\xc3\xbcltige Fehlernummer",
		"Datei oder Verzeichnis nicht gefunden",
		"Unbekannte Anoubis-Fehlernummer",
		"Unbekannte Anoubis-Fehlernummer",
		"Operation nicht erlaubt: Kein Zertifikat gefunden"
	};

	char	*locale = setlocale(LC_ALL, NULL);
	char	 *domain = textdomain(NULL);
	char	*dir;
	char	*loc;
	int	 is_utf8 = 0;

	printf("testing german error codes\n");
	printf("domain: %s\n", textdomain("anoubis"));

	dir = bindtextdomain("anoubis", tmpdir);
#ifndef OPENBSD
	loc = setlocale(LC_ALL, LOCALE);
	if (loc == NULL) {
		is_utf8 = 1;
		loc = setlocale(LC_ALL, LOCALE ".utf8");
	}
	printf("locale: %s\n", loc);
#else
	setenv("LANG", "de", 1);
	dir = bindtextdomain("libc", "/usr/share/nls/");
	loc = setlocale(LC_ALL, "de_DE.ISO8859-15");
	printf("locale: %s\n", loc);
#endif

	if (is_utf8 == 1)
		run_test(predicted_error_string_utf8);
	else
		run_test(predicted_error_string_iso);
	setlocale(LC_ALL, (const char *) locale);
	textdomain( (const char *) domain);
}
END_TEST

TCase *
libanoubiserrorcode_testcase(void)
{
	TCase *testcase = tcase_create("Error Code");

	tcase_add_checked_fixture(testcase, libanoubiserrorcode_tc_setup,
	    libanoubiserrorcode_tc_teardown);
	tcase_add_test(testcase, testcase_en);
	tcase_add_test(testcase, testcase_de);

	return (testcase);
}
