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

#include <check.h>
#include <LogNotify.h>
#include <NotificationCtrl.h>
#include <Subject.h>
#include <Observer.h>

#include <wx/app.h>

/*
 * Define a perspectiveObserver
 */
class PerspectiveObserver : public Observer
{
	public:
		PerspectiveObserver(Subject *);
		int getNotifyCount(void);
		int getDelNotifyCount(void);
		void resetNotifyCount(void);
		void update(Subject *);
		virtual void updateDelete(Subject *);

	private:
		int notifyCount_;
		int delNotifyCount_;
};

PerspectiveObserver::PerspectiveObserver(Subject *subject) : Observer(subject)
{
	resetNotifyCount();
}

int
PerspectiveObserver::getNotifyCount(void)
{
	return (notifyCount_);
}

int
PerspectiveObserver::getDelNotifyCount(void)
{
	return (delNotifyCount_);
}

void
PerspectiveObserver::resetNotifyCount(void)
{
	notifyCount_ = 0;
	delNotifyCount_ = 0;
}

void
PerspectiveObserver::update(Subject * WXUNUSED(subject))
{
	notifyCount_++;
}

void
PerspectiveObserver::updateDelete(Subject * WXUNUSED(subject))
{
	delNotifyCount_++;
}

#define CHECK_OBSERVER_NOTIFY(observer, count) \
	do { \
		if (observer->getNotifyCount() == count) { \
			/* notify count ok - reset */ \
			observer->resetNotifyCount(); \
		} else { \
			fail("Observer not notified as expected."); \
		} \
	} while (0)

static wxApp *tcApp = 0;

static void
setup(void)
{
	tcApp = new wxApp;
	wxApp::SetInstance(tcApp);
}

static void
teardown(void)
{
	wxApp::SetInstance(0);
	delete tcApp;
}

/*
 * The unit tests itself
 */
START_TEST(notificationCtrl_add)
{
	LogNotify		*logNotify1;
	LogNotify		*logNotify2;
	LogNotify		*logNotify3;
	NotificationCtrl	*ctrl;
	PerspectiveObserver	*observer;

	logNotify1 = new LogNotify(wxT("LogNotify #1"));
	logNotify2 = new LogNotify(wxT("LogNotify #2"));
	logNotify3 = new LogNotify(wxT("LogNotify #3"));

	ctrl = NotificationCtrl::instance();

	observer = new PerspectiveObserver(ctrl->getPerspective(
	    NotificationCtrl::LIST_ALL));
	observer->resetNotifyCount();

	ctrl->addNotification(logNotify1);
	ctrl->addNotification(logNotify2);
	CHECK_OBSERVER_NOTIFY(observer, 2);

	ctrl->addNotification(logNotify3);
	CHECK_OBSERVER_NOTIFY(observer, 1);
}
END_TEST

TCase *
getTc_NotificationCtrl(void)
{
	TCase *testCase;

	testCase = tcase_create("NotificationCtrl");
	tcase_add_checked_fixture(testCase, setup, teardown);
	tcase_add_test(testCase, notificationCtrl_add);

	return (testCase);
}
