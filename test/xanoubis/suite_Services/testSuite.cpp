/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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
#include <errno.h>
#include <stdio.h>

#include <wx/app.h>
#include <wx/apptrait.h>
#include <wx/stdpaths.h>

#include <Service.h>
#include <ServiceList.h>

class ServiceObserver : public Observer
{
	public:
		ServiceObserver(Service *service) : Observer(service)
		{
			destroyed_ = false;
		}

		bool isDestroyed(void)
		{
			return (destroyed_);
		}

		void update(Subject *)
		{
		}

		void updateDelete(Subject *)
		{
			destroyed_ = true;
		}

	private:
		bool destroyed_;
};

class ServicesAppTraits : public wxGUIAppTraits
{
	public:
		ServicesAppTraits(const wxString &prefix)
		{
			paths.SetInstallPrefix(prefix);
		}

		virtual wxStandardPaths &GetStandardPaths()
		{
			return (paths);
		}

	private:
		wxStandardPaths paths;
};

class ServicesApp : public wxApp
{
	public:
		ServicesApp(const wxString &prefix)
		{
			this->prefix = prefix;
			SetAppName(wxT("ServicesApp"));
		}

		virtual wxAppTraits *CreateTraits()
		{
			return new ServicesAppTraits(prefix);
		}

	private:
		wxString prefix;
};

static ServicesApp	*app;
static char		tmp_prefix[PATH_MAX];

static void
setup(void)
{
	char path[PATH_MAX];
	char *s;

	strcpy(tmp_prefix, "/tmp/tc_Services_XXXXXX");
	s = mkdtemp(tmp_prefix);
	setenv("HOME", tmp_prefix, 1);

	snprintf(path, sizeof(path), "%s/share", tmp_prefix);
	fail_unless(mkdir(path, S_IRWXU) == 0,
	    "Failed to create %s: %s", path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share/ServicesApp", tmp_prefix);
	fail_unless(mkdir(path, S_IRWXU) == 0,
	    "Failed to create %s: %s", path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share/ServicesApp/profiles",
	    tmp_prefix);
	fail_unless(mkdir(path, S_IRWXU) == 0,
	    "Failed to create %s: %s", path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share/ServicesApp/profiles/wizard",
	    tmp_prefix);
	fail_unless(mkdir(path, S_IRWXU) == 0,
	    "Failed to create %s: %s", path, strerror(errno));

	snprintf(path, sizeof(path),
	    "%s/share/ServicesApp/profiles/wizard/alf", tmp_prefix);

	FILE *f = fopen(path, "w");
	fail_unless(f != 0);
	fail_unless(fwrite("udp/53\ntcp/53\n", 14, 1, f) == 1);
	fclose(f);

	app = new ServicesApp(wxString(tmp_prefix, wxConvFile));
	wxApp::SetInstance(app);
}

static void
teardown(void)
{
	char path[PATH_MAX];

	wxApp::SetInstance(0);
	delete app;

	snprintf(path, sizeof(path),
	"%s/share/ServicesApp/profiles/wizard/alf", tmp_prefix);
	unlink(path); /* Ignore error, file can be removed by a test */

	snprintf(path, sizeof(path), "%s/share/ServicesApp/profiles/wizard",
	    tmp_prefix);
	fail_unless(rmdir(path) == 0, "Failed to remove %s: %s",
	    path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share/ServicesApp/profiles",
	    tmp_prefix);
	fail_unless(rmdir(path) == 0, "Failed to remove %s: %s",
	    path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share/ServicesApp", tmp_prefix);
	fail_unless(rmdir(path) == 0, "Failed to remove %s: %s",
	    path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share", tmp_prefix);
	fail_unless(rmdir(path) == 0, "Failed to remove %s: %s",
	    path, strerror(errno));

	fail_unless(rmdir(tmp_prefix) == 0, "Failed to remove %s: %s",
	    tmp_prefix, strerror(errno));
}

START_TEST(add_service)
{
	ServiceList serviceList;
	Service *s = new Service(wxEmptyString, 4711, Service::TCP, false);

	serviceList.addService(s);

	fail_unless(s->getParent() == &serviceList);

	fail_unless(serviceList.getServiceCount() == 1);
	fail_unless(serviceList.getServiceAt(0) == s);
	fail_unless(serviceList.getServiceAt(1) == 0);
	fail_unless(serviceList.getIndexOf(s) == 0);
	fail_unless(serviceList.getIndexOf(0) == -1);
}
END_TEST

START_TEST(remove_service)
{
	ServiceList serviceList;
	Service *s = new Service(wxEmptyString, 4711, Service::TCP, false);
	ServiceObserver observer(s);

	serviceList.addService(s);

	fail_unless(s->getParent() == &serviceList);
	fail_unless(serviceList.getServiceCount() == 1);

	fail_unless(serviceList.removeService(s));

	fail_unless(!observer.isDestroyed());
	fail_unless(s->getParent() == 0);

	fail_unless(serviceList.getServiceCount() == 0);
	fail_unless(serviceList.getServiceAt(0) == 0);
}
END_TEST

START_TEST(clear_notdestroy)
{
	ServiceList serviceList;
	Service *s1 = new Service(wxEmptyString, 4711, Service::TCP, false);
	Service *s2 = new Service(wxEmptyString, 4711, Service::TCP, false);
	ServiceObserver o1(s1);
	ServiceObserver o2(s2);

	serviceList.addService(s1);
	serviceList.addService(s2);
	fail_unless(serviceList.getServiceCount() == 2);

	serviceList.clearServiceList(false);

	fail_unless(serviceList.getServiceCount() == 0);
	fail_unless(serviceList.getServiceAt(0) == 0);

	fail_unless(!o1.isDestroyed());
	fail_unless(!o2.isDestroyed());
	fail_unless(s1->getParent() == 0);
	fail_unless(s2->getParent() == 0);
}
END_TEST

START_TEST(clear_destroy)
{
	ServiceList serviceList;
	Service *s1 = new Service(wxEmptyString, 4711, Service::TCP, false);
	Service *s2 = new Service(wxEmptyString, 4711, Service::TCP, false);
	ServiceObserver o1(s1);
	ServiceObserver o2(s2);

	serviceList.addService(s1);
	serviceList.addService(s2);
	fail_unless(serviceList.getServiceCount() == 2);

	serviceList.clearServiceList(true);

	fail_unless(serviceList.getServiceCount() == 0);
	fail_unless(serviceList.getServiceAt(0) == 0);

	fail_unless(o1.isDestroyed());
	fail_unless(o2.isDestroyed());
}
END_TEST

START_TEST(system_services)
{
	ServiceList serviceList;
	serviceList.assignSystemServices();

	/*
	 * Since you don't know how many service-entries you have,
	 * this check should be enough.
	 */
	fail_unless(serviceList.getServiceCount() > 0);
}
END_TEST

START_TEST(default_service)
{
	ServiceList serviceList;
	Service *s;

	fail_unless(serviceList.canHaveDefaultServices());

	serviceList.assignDefaultServices();

	fail_unless(serviceList.getServiceCount() == 2);

	s = serviceList.getServiceAt(0);
	fail_unless(s != 0);
	fail_unless(s->getPort() == 53);
	fail_unless(s->getProtocol() == Service::UDP);

	s = serviceList.getServiceAt(1);
	fail_unless(s != 0);
	fail_unless(s->getPort() == 53);
	fail_unless(s->getProtocol() == Service::TCP);
}
END_TEST

START_TEST(no_default_service)
{
	ServiceList serviceList;
	char path[PATH_MAX];
	Service *s;

	snprintf(path, sizeof(path),
	"%s/share/ServicesApp/profiles/wizard/alf", tmp_prefix);
	fail_unless(unlink(path) == 0, "Failed to remove %s: %s",
	    path, strerror(errno));

	fail_unless(!serviceList.canHaveDefaultServices());

	serviceList.assignDefaultServices();

	fail_unless(serviceList.getServiceCount() == 0);

	s = serviceList.getServiceAt(0);
	fail_unless(s == 0);
}
END_TEST

Suite *
getTestSuite(void)
{
	Suite *testSuite;
	TCase *testCase;

	testSuite = suite_create("Services");
	testCase = tcase_create("tc_Services");

	tcase_add_checked_fixture(testCase, setup, teardown);
	tcase_add_test(testCase, add_service);
	tcase_add_test(testCase, remove_service);
	tcase_add_test(testCase, clear_notdestroy);
	tcase_add_test(testCase, clear_destroy);
	tcase_add_test(testCase, system_services);
	tcase_add_test(testCase, default_service);
	tcase_add_test(testCase, no_default_service);

	suite_add_tcase(testSuite, testCase);

	return (testSuite);
}
