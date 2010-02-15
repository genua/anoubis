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

#include <StringListModel.h>

class EventSpy : public wxEvtHandler
{
	public:
		EventSpy(StringListModel *model)
		{
			numSizeChanges = 0;
			numRowUpdates = 0;
			model_ = model;

			model_->Connect(anEVT_ROW_SIZECHANGE,
			    wxCommandEventHandler(EventSpy::onSizeChange),
			    NULL, this);
			model_->Connect(anEVT_ROW_UPDATE,
			    wxCommandEventHandler(EventSpy::onRowUpdate),
			    NULL, this);
		}

		~EventSpy(void)
		{
			model_->Disconnect(anEVT_ROW_SIZECHANGE,
			    wxCommandEventHandler(EventSpy::onSizeChange),
			    NULL, this);
			model_->Disconnect(anEVT_ROW_UPDATE,
			    wxCommandEventHandler(EventSpy::onRowUpdate),
			    NULL, this);
		}

		unsigned int numSizeChanges;
		unsigned int numRowUpdates;

	private:
		StringListModel *model_;

		void onSizeChange(wxCommandEvent &event)
		{
			numSizeChanges++;
			event.Skip();
		}

		void onRowUpdate(wxCommandEvent &event)
		{
			numRowUpdates++;
			event.Skip();
		}
};

static StringListModel *model = 0;
static EventSpy *spy = 0;

static void
assert_StringProxy(unsigned int idx, const wxString &str)
{
	fail_unless(model->getRow(idx) != 0);

	StringWrapper *wrapper =
	    dynamic_cast<StringWrapper *>(model->getRow(idx));
	fail_unless(wrapper != 0);
	fail_unless(wrapper->str() == str);
}

static void
assert_EventSpy(unsigned int sizeChanges, unsigned int rowUpdates)
{
	model->ProcessPendingEvents();
	fail_unless(spy->numSizeChanges == sizeChanges);
	fail_unless(spy->numRowUpdates == rowUpdates);
}

static void
setup(void)
{
	model = new StringListModel;

	model->add(wxT("a"));
	model->add(wxT("b"));
	model->add(wxT("c"));

	fail_unless(model->count() == 3);
	fail_unless(model->get(0) == wxT("a"));
	fail_unless(model->get(1) == wxT("b"));
	fail_unless(model->get(2) == wxT("c"));
	fail_unless(model->get(3) == wxEmptyString);

	spy = new EventSpy(model);
}

static void
teardown()
{
	delete spy;
	spy = 0;

	delete model;
	model = 0;
}

START_TEST(add)
{
	model->add(wxT("x"));
	fail_unless(model->count() == 4);
	fail_unless(model->getSize() == 4);
	assert_EventSpy(1, 0);

	model->add(wxT("y"));
	fail_unless(model->count() == 5);
	fail_unless(model->getSize() == 5);
	assert_EventSpy(2, 0);

	model->add(wxT("z"));
	fail_unless(model->count() == 6);
	fail_unless(model->getSize() == 6);
	assert_EventSpy(3, 0);

	fail_unless(model->get(0) == wxT("a"));
	fail_unless(model->get(1) == wxT("b"));
	fail_unless(model->get(2) == wxT("c"));
	fail_unless(model->get(3) == wxT("x"));
	fail_unless(model->get(4) == wxT("y"));
	fail_unless(model->get(5) == wxT("z"));
	fail_unless(model->get(6) == wxEmptyString);

	assert_StringProxy(0, wxT("a"));
	assert_StringProxy(1, wxT("b"));
	assert_StringProxy(2, wxT("c"));
	assert_StringProxy(3, wxT("x"));
	assert_StringProxy(4, wxT("y"));
	assert_StringProxy(5, wxT("z"));
	fail_unless(model->getRow(6) == 0);

	assert_EventSpy(3, 0);
}
END_TEST

START_TEST(add_idx)
{
	model->add(wxT("x"), 4711); // Appended
	fail_unless(model->count() == 4);
	fail_unless(model->getSize() == 4);
	assert_EventSpy(1, 0);

	model->add(wxT("y"), 0); // Prepended
	fail_unless(model->count() == 5);
	fail_unless(model->getSize() == 5);
	assert_EventSpy(2, 0);

	model->add(wxT("z"), 3); // Between "b" and "c"
	fail_unless(model->count() == 6);
	fail_unless(model->getSize() == 6);
	assert_EventSpy(3, 0);

	fail_unless(model->get(0) == wxT("y"));
	fail_unless(model->get(1) == wxT("a"));
	fail_unless(model->get(2) == wxT("b"));
	fail_unless(model->get(3) == wxT("z"));
	fail_unless(model->get(4) == wxT("c"));
	fail_unless(model->get(5) == wxT("x"));
	fail_unless(model->get(6) == wxEmptyString);

	assert_StringProxy(0, wxT("y"));
	assert_StringProxy(1, wxT("a"));
	assert_StringProxy(2, wxT("b"));
	assert_StringProxy(3, wxT("z"));
	assert_StringProxy(4, wxT("c"));
	assert_StringProxy(5, wxT("x"));
	fail_unless(model->getRow(6) == 0);

	assert_EventSpy(3, 0);
}
END_TEST

START_TEST(remove_str)
{
	model->remove(wxT("something_not_existing"));

	fail_unless(model->count() == 3);
	fail_unless(model->get(0) == wxT("a"));
	fail_unless(model->get(1) == wxT("b"));
	fail_unless(model->get(2) == wxT("c"));
	fail_unless(model->get(3) == wxEmptyString);
	assert_EventSpy(0, 0);

	model->remove(wxT("b"));

	fail_unless(model->count() == 2);
	fail_unless(model->get(0) == wxT("a"));
	fail_unless(model->get(1) == wxT("c"));
	fail_unless(model->get(2) == wxEmptyString);
	assert_EventSpy(1, 0);

	model->remove(wxT("c"));

	fail_unless(model->count() == 1);
	fail_unless(model->get(0) == wxT("a"));
	fail_unless(model->get(1) == wxEmptyString);
	assert_EventSpy(2, 0);

	model->remove(wxT("a"));

	fail_unless(model->count() == 0);
	fail_unless(model->get(0) == wxEmptyString);
	assert_EventSpy(3, 0);

	model->remove(wxT("something_else"));

	fail_unless(model->count() == 0);
	fail_unless(model->get(0) == wxEmptyString);
	assert_EventSpy(3, 0);
}
END_TEST

START_TEST(remove_idx)
{
	model->remove(3); /* Out of range */

	fail_unless(model->count() == 3);
	fail_unless(model->get(0) == wxT("a"));
	fail_unless(model->get(1) == wxT("b"));
	fail_unless(model->get(2) == wxT("c"));
	fail_unless(model->get(3) == wxEmptyString);
	assert_EventSpy(0, 0);

	model->remove(1);

	fail_unless(model->count() == 2);
	fail_unless(model->get(0) == wxT("a"));
	fail_unless(model->get(1) == wxT("c"));
	fail_unless(model->get(2) == wxEmptyString);
	assert_EventSpy(1, 0);

	model->remove(1);

	fail_unless(model->count() == 1);
	fail_unless(model->get(0) == wxT("a"));
	fail_unless(model->get(1) == wxEmptyString);
	assert_EventSpy(2, 0);

	model->remove(0);

	fail_unless(model->count() == 0);
	fail_unless(model->get(0) == wxEmptyString);
	assert_EventSpy(3, 0);

	model->remove(4711); /* Out of range */

	fail_unless(model->count() == 0);
	fail_unless(model->get(0) == wxEmptyString);
	assert_EventSpy(3, 0);
}
END_TEST

START_TEST(clear)
{
	model->clear();

	fail_unless(model->count() == 0);
	fail_unless(model->get(0) == wxEmptyString);
	assert_EventSpy(1, 0);

	model->clear();
	assert_EventSpy(1, 0);
}
END_TEST

START_TEST(find)
{
	fail_unless(model->find(wxT("a")) == 0);
	fail_unless(model->find(wxT("b")) == 1);
	fail_unless(model->find(wxT("c")) == 2);
	fail_unless(model->find(wxT("sonething_completely_different")) == -1);
	assert_EventSpy(0, 0);
}
END_TEST

Suite *
getTestSuite(void)
{
	Suite *testSuite;
	TCase *testCase;

	testSuite = suite_create("StringList");
	testCase = tcase_create("tc_StringList");

	tcase_add_checked_fixture(testCase, setup, teardown);
	tcase_add_test(testCase, add);
	tcase_add_test(testCase, add_idx);
	tcase_add_test(testCase, remove_str);
	tcase_add_test(testCase, remove_idx);
	tcase_add_test(testCase, clear);
	tcase_add_test(testCase, find);

	suite_add_tcase(testSuite, testCase);

	return (testSuite);
}
