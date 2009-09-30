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

#include <sys/types.h>
#include <sys/wait.h>

#include <check.h>
#include <errno.h>
#include <stdio.h>

#include <wx/app.h>
#include <wx/ffile.h>
#include <wx/filename.h>

#include <AnConfig.h>
#include <AnEvents.h>

#include "AnEventsSpy.h"

static wxApp *app = 0;
static char tempDir[PATH_MAX];

static const wxString defaultConf = wxString::FromAscii("[test]\n") +
    wxT("stroption=strvalue\n") +
    wxT("longoption=4711\n") +
    wxT("doubleoption=1.3\n") +
    wxT("booloption=0\n");

static wxString
getConfFileContent(void)
{
	wxString confFile = wxString::FromAscii(tempDir) +
	    wxT("/.xanoubis/xanoubis.conf");
	wxFFile f;
	wxString content;

	if (!f.Open(confFile, wxT("r"))) {
		return (wxEmptyString);
	}

	if (f.ReadAll(&content))
		return (content);
	else
		return (wxEmptyString);
}

static bool
initializeConfFile(void)
{
	wxString confFile = wxString::FromAscii(tempDir) +
	    wxT("/.xanoubis/xanoubis.conf");
	wxFFile f;

	if (!f.Open(confFile, wxT("w")))
		return (false);

	if (f.Write(defaultConf)) {
		f.Flush();
		return (true);
	} else
		return (false);
}

static void
setup(void)
{
	char *charResult;
	bool boolResult;

	/* Temp. directory is HOME-directory */
	strcpy(tempDir, "/tmp/suite_Config_XXXXXX");
	charResult = mkdtemp(tempDir);
	fail_unless(charResult != 0,
	    "Failed to create temp. directory (%s)", strerror(errno));

	/* Create applications data-directory */
	boolResult = wxMkdir(wxString::FromAscii(tempDir) + wxT("/.xanoubis"));
	fail_unless(boolResult, "Failed to create .xanoubis");

	/* Assign new home-directory */
	setenv("HOME", tempDir, 1);

	/* wxWidgets-infrastructure */
	app = new wxApp;
	wxApp::SetInstance(app);
	wxPendingEventsLocker = new wxCriticalSection;

	/* Object to be tested */
	boolResult = initializeConfFile();
	fail_unless(boolResult, "Failed to create initial config-file");
	wxConfig *config = new AnConfig(wxT("xanoubis"));
	wxConfig::Set(config);
}

static void
teardown(void)
{
	int result;

	/* Destry test-object */
	wxConfigBase *config = wxConfig::Get();
	wxConfig::Set(0);
	delete config;

	/* Destroy wxWidgets-infrastructure */
	wxApp::SetInstance(0);
	delete app;
	delete wxPendingEventsLocker;
	wxPendingEventsLocker = 0;

	/* Remove temp. HOME-directory */
	char cmd[PATH_MAX];
	sprintf(cmd, "rm -rf %s", tempDir);
	result = system(cmd);
	fail_unless(WIFEXITED(result) && WEXITSTATUS(result) == 0,
	    "Failed to remove temp. HOME-directory");
}

START_TEST(tc_Config_Read)
{
	bool result;
	wxString strValue;
	long longValue;
	double doubleValue;
	bool boolValue;
	AnEventsSpy spy(anEVT_ANOUBISOPTIONS_UPDATE);

	/* Read string */

	result = wxConfig::Get()->Read(wxT("/test/stroption"), &strValue);
	fail_unless(result, "Unexpected read-result");
	fail_unless(strValue == wxT("strvalue"), "Unexpected value");

	strValue = wxEmptyString;
	result = wxConfig::Get()->Read(wxT("/test/stroption"), &strValue,
	    wxT("12345"));
	fail_unless(result, "Unexpected read-result");
	fail_unless(strValue == wxT("strvalue"), "Unexpected value");

	strValue = wxConfig::Get()->Read(wxT("/test/stroption"),
	    wxT("abcdef"));
	fail_unless(strValue == wxT("strvalue"), "Unexpected value");

	/* Read long */

	longValue = -1;
	result = wxConfig::Get()->Read(wxT("/test/longoption"), &longValue);
	fail_unless(result, "Unexpected read-result");
	fail_unless(longValue == 4711, "Unexpected value");

	longValue = -1;
	result = wxConfig::Get()->Read(wxT("/test/longoption"),
	    &longValue, 9244);
	fail_unless(result, "Unexpected read-result");
	fail_unless(longValue == 4711, "Unexpected value");

	longValue = wxConfig::Get()->Read(wxT("/test/longoption"), 9422);
	fail_unless(longValue == 4711, "Unexpected value");

	/* Read double */

	doubleValue = 0.0;
	result = wxConfig::Get()->Read(wxT("/test/doubleoption"), &doubleValue);
	fail_unless(result, "Unexpected read-result");
	fail_unless(doubleValue == 1.3, "Unexpected value");

	doubleValue = 0.0;
	result = wxConfig::Get()->Read(wxT("/test/doubleoption"),
	    &doubleValue, M_PI);
	fail_unless(result, "Unexpected read-result");
	fail_unless(doubleValue == 1.3, "Unexpected value");

	/* Read bool */

	boolValue = true;
	result = wxConfig::Get()->Read(wxT("/test/booloption"), &boolValue);
	fail_unless(result, "Unexpected read-result");
	fail_unless(boolValue == false, "Unexpected value");

	boolValue = true;
	result = wxConfig::Get()->Read(wxT("/test/booloption"),
	    &boolValue, true);
	fail_unless(result, "Unexpected read-result");
	fail_unless(boolValue == false, "Unexpected value");

	/* General checks */

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 0,
	    "Unexpected number of events");
	fail_unless(getConfFileContent() == defaultConf,
	    "Unexpected content of conf-file");
}
END_TEST

START_TEST(tc_Config_Read_NoOptions)
{
	bool result;
	wxString strValue;
	long longValue = -1;
	double doubleValue = 0.0;
	bool boolValue = true;
	AnEventsSpy spy(anEVT_ANOUBISOPTIONS_UPDATE);

	/* Read string */

	result = wxConfig::Get()->Read(wxT("/xxx/stroption"), &strValue);
	fail_unless(!result, "Unexpected read-result");
	fail_unless(strValue.IsEmpty(), "Unexpected value");

	result = wxConfig::Get()->Read(wxT("/xxx/stroption"), &strValue,
	    wxT("12345"));
	fail_unless(!result, "Unexpected read-result");
	fail_unless(strValue == wxT("12345"), "Unexpected value");

	strValue = wxConfig::Get()->Read(wxT("/xxx/stroption"),
	    wxT("abcdef"));
	fail_unless(strValue == wxT("abcdef"), "Unexpected value");

	/* Read long */

	result = wxConfig::Get()->Read(wxT("/xxx/longoption"), &longValue);
	fail_unless(!result, "Unexpected read-result");
	fail_unless(longValue == -1, "Unexpected value");

	result = wxConfig::Get()->Read(wxT("/xxx/longoption"),
	    &longValue, 9244);
	fail_unless(!result, "Unexpected read-result");
	fail_unless(longValue == 9244, "Unexpected value");

	longValue = wxConfig::Get()->Read(wxT("/xxx/longoption"), 5);
	fail_unless(longValue == 5, "Unexpected value");

	/* Read double */

	result = wxConfig::Get()->Read(wxT("/xxx/doubleoption"), &doubleValue);
	fail_unless(!result, "Unexpected read-result");
	fail_unless(doubleValue == 0.0, "Unexpected value");

	result = wxConfig::Get()->Read(wxT("/xxx/doubleoption"),
	    &doubleValue, M_PI);
	fail_unless(!result, "Unexpected read-result");
	fail_unless(doubleValue == M_PI, "Unexpected value");

	/* Read bool */

	result = wxConfig::Get()->Read(wxT("/xxx/booloption"), &boolValue);
	fail_unless(!result, "Unexpected read-result");
	fail_unless(boolValue, "Unexpected value");

	result = wxConfig::Get()->Read(wxT("/xxx/booloption"),
	    &boolValue, false);
	fail_unless(!result, "Unexpected read-result");
	fail_unless(boolValue == false, "Unexpected value");

	/* General checks */

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 0,
	    "Unexpected number of events");
	fail_unless(getConfFileContent() == defaultConf,
	    "Unexpected content of conf-file");
}
END_TEST

START_TEST(tc_Config_Write)
{
	bool result;
	wxString conf = defaultConf;
	AnEventsSpy spy(anEVT_ANOUBISOPTIONS_UPDATE);

	/* Write string */

	result = wxConfig::Get()->Write(wxT("/xxx/1"), wxT("yyy"));
	fail_unless(result, "Unexpected write-result");

	conf += wxT("[xxx]\n1=yyy\n");
	fail_unless(getConfFileContent() == conf,
	    "Unexpected content of conf-file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 1,
	    "Unexpected number of events");

	/* Write long */

	result = wxConfig::Get()->Write(wxT("/xxx/2"), 4711);
	fail_unless(result, "Unexpected write-result");

	conf += wxT("2=4711\n");
	fail_unless(getConfFileContent() == conf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 2,
	    "Unexpected number of events");

	/* Write double */

	result = wxConfig::Get()->Write(wxT("/xxx/3"), 1.3);
	fail_unless(result, "Unexpected write-result");

	conf += wxT("3=1.3\n");
	fail_unless(getConfFileContent() == conf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 3,
	    "Unexpected number of events");

	/* Write bool */

	result = wxConfig::Get()->Write(wxT("/xxx/4"), true);
	fail_unless(result, "Unexpected write-result");

	conf += wxT("4=1\n");
	fail_unless(getConfFileContent() == conf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 4,
	    "Unexpected number of events");

	/* General checks */

	result = wxConfig::Get()->Flush();
	fail_unless(!result, "Unexpected flush-result");

	fail_unless(getConfFileContent() == conf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 4,
	    "Unexpected number of events");
}
END_TEST

START_TEST(tc_Config_Overwrite)
{
	bool result;
	wxString conf;
	AnEventsSpy spy(anEVT_ANOUBISOPTIONS_UPDATE);

	/* Overwrite string */

	result = wxConfig::Get()->Write(wxT("/test/stroption"), wxT("foobar"));
	fail_unless(result, "Unexpected write-result");

	conf = wxT("[test]\nstroption=foobar\nlongoption=4711\n"
	    "doubleoption=1.3\nbooloption=0\n");
	fail_unless(getConfFileContent() == conf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 1,
	    "Unexpected number of events");

	/* Overwrite long */

	result = wxConfig::Get()->Write(wxT("/test/longoption"), 9244);
	fail_unless(result, "Unexpected write-result");

	conf = wxT("[test]\nstroption=foobar\nlongoption=9244\n"
	    "doubleoption=1.3\nbooloption=0\n");
	fail_unless(getConfFileContent() == conf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 2,
	    "Unexpected number of events");

	/* Overwrite double */

	result = wxConfig::Get()->Write(wxT("/test/doubleoption"), 4.5);
	fail_unless(result, "Unexpected write-result");

	conf = wxT("[test]\nstroption=foobar\nlongoption=9244\n"
	    "doubleoption=4.5\nbooloption=0\n");
	fail_unless(getConfFileContent() == conf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 3,
	    "Unexpected number of events");

	/* Overwrite bool */

	result = wxConfig::Get()->Write(wxT("/test/booloption"), true);
	fail_unless(result, "Unexpected write-result");

	conf = wxT("[test]\nstroption=foobar\nlongoption=9244\n"
	    "doubleoption=4.5\nbooloption=1\n");
	fail_unless(getConfFileContent() == conf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 4,
	    "Unexpected number of events");

	/* General checks */

	result = wxConfig::Get()->Flush();
	fail_unless(!result, "Unexpected flush-result");

	fail_unless(getConfFileContent() == conf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 4,
	    "Unexpected number of events");
}
END_TEST

START_TEST(tc_Config_Overwrite_Same)
{
	bool result;
	AnEventsSpy spy(anEVT_ANOUBISOPTIONS_UPDATE);

	/* Overwrite string */

	result = wxConfig::Get()->Write(wxT("/test/stroption"),
	    wxT("strvalue"));
	fail_unless(!result, "Unexpected write-result");

	fail_unless(getConfFileContent() == defaultConf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 0,
	    "Unexpected number of events");

	/* Overwrite long */

	result = wxConfig::Get()->Write(wxT("/test/longoption"), 4711);
	fail_unless(!result, "Unexpected write-result");

	fail_unless(getConfFileContent() == defaultConf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 0,
	    "Unexpected number of events");

	/* Overwrite double */

	result = wxConfig::Get()->Write(wxT("/test/doubleoption"), 1.3);
	fail_unless(!result, "Unexpected write-result");

	fail_unless(getConfFileContent() == defaultConf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 0,
	    "Unexpected number of events");

	/* Overwrite bool */

	result = wxConfig::Get()->Write(wxT("/test/booloption"), false);
	fail_unless(!result, "Unexpected write-result");

	fail_unless(getConfFileContent() == defaultConf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 0,
	    "Unexpected number of events");

	/* General checks */

	result = wxConfig::Get()->Flush();
	fail_unless(!result, "Unexpected flush-result");

	fail_unless(getConfFileContent() == defaultConf,
	    "Unexpected content of configuration file");

	AnEvents::getInstance()->ProcessPendingEvents();
	fail_unless(spy.getNumInvocations() == 0,
	    "Unexpected number of events");
}
END_TEST

TCase *
getTc_Config(void)
{
	TCase *testCase;

	testCase = tcase_create("Config");
	tcase_add_checked_fixture(testCase, setup, teardown);
	tcase_add_test(testCase, tc_Config_Read);
	tcase_add_test(testCase, tc_Config_Read_NoOptions);
	tcase_add_test(testCase, tc_Config_Write);
	tcase_add_test(testCase, tc_Config_Overwrite);
	tcase_add_test(testCase, tc_Config_Overwrite_Same);

	return (testCase);
}
