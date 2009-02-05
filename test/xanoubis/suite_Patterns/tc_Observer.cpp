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

#include <wx/utils.h>

#include <Subject.h>
#include <Observer.h>

/*
 * Define a concrete subject class
 */
class ConcreteSubject : public Subject
{
	public:
		ConcreteSubject(void);
		long getId(void)	{ return (id_); }
		long getIndex(void)	{ return (index_); }
		void setId(long);
		void setIndex(long);
		void setParams(long, long);

	private:
		long id_;
		long index_;
};

ConcreteSubject::ConcreteSubject(void)
{
	id_ = wxNewId();
	index_ = wxNewId();
}

void
ConcreteSubject::setId(long id)
{
	startChange();
	id_ = id;
	finishChange();
}

void
ConcreteSubject::setIndex(long index)
{
	startChange();
	index_ = index;
	finishChange();
}

void
ConcreteSubject::setParams(long id, long index)
{
	startChange();
	setId(id);
	setIndex(index);
	finishChange();
}

#define SET_VALUE_ID(subject) \
	do { \
		long _newId = wxNewId(); \
		if (subject->getId() == _newId) { \
			fail("ID: _newId not new!"); \
		} \
		subject->setId(_newId); \
		if (subject->getId() != _newId) { \
			fail("ID: Couldn't set id."); \
		} \
	} while (0)

#define SET_VALUE_INDEX(subject) \
	do { \
		long _newIndex = wxNewId(); \
		if (subject->getIndex() == _newIndex) { \
			fail("INDEX: _newIndex isn't new!"); \
		} \
		subject->setIndex(_newIndex); \
		if (subject->getIndex() != _newIndex) { \
			fail("INDEX: Couldn't set index."); \
		} \
	} while (0)

#define SET_VALUE_PARAMS(subject) \
	do { \
		long _newId = wxNewId(); \
		long _newIndex = wxNewId(); \
		if (subject->getId() == _newId) { \
			fail("PARAMS: _newId not new!"); \
		} \
		if (subject->getIndex() == _newIndex) { \
			fail("PARAMS: _newIndex isn't new!"); \
		} \
		subject->setParams(_newId, _newIndex); \
		if (subject->getId() != _newId) { \
			fail("PARAMS: Couldn't set id."); \
		} \
		if (subject->getIndex() != _newIndex) { \
			fail("PARAMS: Couldn't set index."); \
		} \
	} while (0)


/*
 * Define a concrete observer
 */
class ConcreteObserver : public Observer
{
	public:
		ConcreteObserver(Subject *);
		int getNotifyCount(void);
		int getDelNotifyCount(void);
		void resetNotifyCount(void);
		void update(Subject *);
		virtual void updateDelete(Subject *);

	private:
		int notifyCount_;
		int delNotifyCount_;
};

ConcreteObserver::ConcreteObserver(Subject *subject) : Observer(subject)
{
	resetNotifyCount();
}

int
ConcreteObserver::getNotifyCount(void)
{
	return (notifyCount_);
}

int
ConcreteObserver::getDelNotifyCount(void)
{
	return (delNotifyCount_);
}

void
ConcreteObserver::resetNotifyCount(void)
{
	notifyCount_ = 0;
	delNotifyCount_ = 0;
}

void
ConcreteObserver::update(Subject * WXUNUSED(subject))
{
	notifyCount_++;
}

void
ConcreteObserver::updateDelete(Subject * WXUNUSED(subject))
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

/*
 * The unit tests itself
 */
START_TEST(observer_s1_o0)
{
	ConcreteSubject		*cs;

	cs = new ConcreteSubject();
	fail_if(cs == NULL,  "Couldn't create concrete subject.");

	SET_VALUE_ID(cs);

	if (cs->addObserver(NULL) == true) {
		fail("Successfully added not-existing observer.");
	}

	SET_VALUE_ID(cs);
}
END_TEST

START_TEST(observer_s0_o1)
{
	ConcreteObserver	*co;

	co = new ConcreteObserver(NULL);
	fail_if(co == NULL, "Couldn't create observer.");

	delete co;
}
END_TEST

START_TEST(observer_s1_o3)
{
	bool			 rc;
	ConcreteSubject		*cs;
	ConcreteObserver	*co1;
	ConcreteObserver	*co2;
	ConcreteObserver	*co3;

	cs  = new ConcreteSubject();
	co1 = new ConcreteObserver(cs);
	co2 = new ConcreteObserver(NULL);
	co3 = new ConcreteObserver(NULL);

	fail_if(cs  == NULL, "Couldn't create concrete subject.");
	fail_if(co1 == NULL, "Couldn't create concrete observer 1.");
	fail_if(co2 == NULL, "Couldn't create concrete observer 2.");
	fail_if(co3 == NULL, "Couldn't create concrete observer 3.");

	rc = cs->addObserver(co1);
	fail_if(rc == true, "Could add already mapped observer 1 to subject.");

	rc = co1->addSubject(cs);
	fail_if(rc == true, "Could add already mapped subject to observer 1.");

	/*
	 * Mapping:
	 *	cs -- co1
	 * not mapped: co2, co3
	 */
	SET_VALUE_ID(cs);
	SET_VALUE_INDEX(cs);
	SET_VALUE_PARAMS(cs);
	CHECK_OBSERVER_NOTIFY(co1, 3);
	CHECK_OBSERVER_NOTIFY(co2, 0);
	CHECK_OBSERVER_NOTIFY(co3, 0);

	/*
	 * Mapping:
	 *	cs -- co1, co2
	 * not mapped: co3
	 */
	rc = co2->addSubject(cs);
	fail_if(rc == false, "Couldn't add subject to observer 2.");

	SET_VALUE_ID(cs);
	SET_VALUE_INDEX(cs);
	SET_VALUE_PARAMS(cs);
	CHECK_OBSERVER_NOTIFY(co1, 3);
	CHECK_OBSERVER_NOTIFY(co2, 3);
	CHECK_OBSERVER_NOTIFY(co3, 0);

	/*
	 * Mapping:
	 *	cs -- co1, co2, co3
	 */
	rc = co3->addSubject(cs);
	fail_if(rc == false, "Couldn't add subject to observer 3.");

	SET_VALUE_ID(cs);
	SET_VALUE_INDEX(cs);
	SET_VALUE_PARAMS(cs);
	CHECK_OBSERVER_NOTIFY(co1, 3);
	CHECK_OBSERVER_NOTIFY(co2, 3);
	CHECK_OBSERVER_NOTIFY(co3, 3);

	delete cs;
	delete co1;
	delete co2;
	delete co3;
}
END_TEST

START_TEST(observer_s2_o3)
{
	bool			 rc;
	ConcreteSubject		*cs1;
	ConcreteSubject		*cs2;
	ConcreteObserver	*co1;
	ConcreteObserver	*co2;
	ConcreteObserver	*co3;

	cs1 = new ConcreteSubject();
	cs2 = new ConcreteSubject();
	co1 = new ConcreteObserver(cs1);
	co2 = new ConcreteObserver(cs2);
	co3 = new ConcreteObserver(NULL);

	fail_if(cs1 == NULL, "Couldn't create concrete subject  1.");
	fail_if(cs2 == NULL, "Couldn't create concrete subject  2.");
	fail_if(co1 == NULL, "Couldn't create concrete observer 1.");
	fail_if(co2 == NULL, "Couldn't create concrete observer 2.");
	fail_if(co3 == NULL, "Couldn't create concrete observer 3.");

	/*
	 * Mapping:
	 *	cs1 -- co1
	 *	cs2 -- co2
	 * not mapped: co3
	 */
	SET_VALUE_ID(cs1);
	SET_VALUE_INDEX(cs1);
	SET_VALUE_PARAMS(cs1);
	SET_VALUE_ID(cs2);
	SET_VALUE_INDEX(cs2);
	SET_VALUE_PARAMS(cs2);
	CHECK_OBSERVER_NOTIFY(co1, 3);
	CHECK_OBSERVER_NOTIFY(co2, 3);
	CHECK_OBSERVER_NOTIFY(co3, 0);

	/*
	 * Mapping:
	 *	cs1 -- co1, co2
	 *	cs2 -- co2, co1
	 * not mapped: co3
	 */
	rc = co1->addSubject(cs2);
	fail_if(rc == false, "Couldn't add subject 2 to observer 1.");
	rc = co2->addSubject(cs1);
	fail_if(rc == false, "Couldn't add subject 1 to observer 2.");

	SET_VALUE_ID(cs1);
	SET_VALUE_INDEX(cs1);
	SET_VALUE_PARAMS(cs1);
	SET_VALUE_ID(cs2);
	SET_VALUE_INDEX(cs2);
	SET_VALUE_PARAMS(cs2);
	CHECK_OBSERVER_NOTIFY(co1, 6);
	CHECK_OBSERVER_NOTIFY(co2, 6);
	CHECK_OBSERVER_NOTIFY(co3, 0);

	/*
	 * Mapping:
	 *	cs1 -- co1, co3
	 *	cs2 -- co2
	 */
	rc = co1->removeSubject(cs2);
	fail_if(rc == false, "Couldn't remove subject 2 from observer 1.");
	rc = co2->removeSubject(cs1);
	fail_if(rc == false, "Couldn't remove subject 1 to observer 2.");
	rc = co3->addSubject(cs1);
	fail_if(rc == false, "Couldn't add subject 1 to observer 3.");

	SET_VALUE_ID(cs1);
	SET_VALUE_INDEX(cs1);
	SET_VALUE_PARAMS(cs1);
	SET_VALUE_ID(cs2);
	SET_VALUE_INDEX(cs2);
	SET_VALUE_PARAMS(cs2);
	CHECK_OBSERVER_NOTIFY(co1, 3);
	CHECK_OBSERVER_NOTIFY(co2, 3);
	CHECK_OBSERVER_NOTIFY(co3, 3);

	/*
	 * Mapping:
	 *	cs1 -- co1, co3
	 *	cs2 -- co2, co3
	 */
	rc = co3->addSubject(cs2);
	fail_if(rc == false, "Couldn't add subject 2 to observer 3.");

	SET_VALUE_ID(cs1);
	SET_VALUE_INDEX(cs1);
	SET_VALUE_PARAMS(cs1);
	SET_VALUE_ID(cs2);
	SET_VALUE_INDEX(cs2);
	SET_VALUE_PARAMS(cs2);
	CHECK_OBSERVER_NOTIFY(co1, 3);
	CHECK_OBSERVER_NOTIFY(co2, 3);
	CHECK_OBSERVER_NOTIFY(co3, 6);

	delete cs1;
	delete cs2;
	delete co1;
	delete co2;
	delete co3;
}
END_TEST

START_TEST(observer_s3_o2)
{
	bool			 rc;
	ConcreteSubject		*cs1;
	ConcreteSubject		*cs2;
	ConcreteSubject		*cs3;
	ConcreteObserver	*co1;
	ConcreteObserver	*co2;

	cs1 = new ConcreteSubject();
	cs2 = new ConcreteSubject();
	cs3 = new ConcreteSubject();
	co1 = new ConcreteObserver(cs1);
	co2 = new ConcreteObserver(NULL);

	fail_if(cs1 == NULL, "Couldn't create concrete subject  1.");
	fail_if(cs2 == NULL, "Couldn't create concrete subject  2.");
	fail_if(cs3 == NULL, "Couldn't create concrete subject  3.");
	fail_if(co1 == NULL, "Couldn't create concrete observer 1.");
	fail_if(co2 == NULL, "Couldn't create concrete observer 2.");

	/*
	 * Mapping:
	 *	cs1 -- co1
	 *	cs2 -- co1
	 *	cs3 -- co1
	 */
	rc = co1->addSubject(cs2);
	fail_if(rc == false, "Couldn't add subject 2 to observer 1.");
	rc = co1->addSubject(cs3);
	fail_if(rc == false, "Couldn't add subject 3 to observer 1.");

	SET_VALUE_ID(cs1);
	SET_VALUE_INDEX(cs1);
	SET_VALUE_PARAMS(cs1);
	SET_VALUE_ID(cs2);
	SET_VALUE_INDEX(cs2);
	SET_VALUE_PARAMS(cs2);
	SET_VALUE_ID(cs3);
	SET_VALUE_INDEX(cs3);
	SET_VALUE_PARAMS(cs3);
	CHECK_OBSERVER_NOTIFY(co1, 9);
	CHECK_OBSERVER_NOTIFY(co2, 0);

	/*
	 * Mapping:
	 *	cs1 -- co1
	 *	cs2 -- co1, co2
	 *	cs3 -- co1, co2
	 */
	rc = co2->addSubject(cs2);
	fail_if(rc == false, "Couldn't add subject 2 to observer 2.");
	rc = co2->addSubject(cs3);
	fail_if(rc == false, "Couldn't add subject 3 to observer 2.");

	SET_VALUE_ID(cs1);
	SET_VALUE_INDEX(cs1);
	SET_VALUE_PARAMS(cs1);
	SET_VALUE_ID(cs2);
	SET_VALUE_INDEX(cs2);
	SET_VALUE_PARAMS(cs2);
	SET_VALUE_ID(cs3);
	SET_VALUE_INDEX(cs3);
	SET_VALUE_PARAMS(cs3);
	CHECK_OBSERVER_NOTIFY(co1, 9);
	CHECK_OBSERVER_NOTIFY(co2, 6);

	delete cs1;
	delete cs2;
	delete cs3;
	delete co1;
	delete co2;
}
END_TEST

START_TEST(observer_delete)
{
	ConcreteSubject		*cs1;
	ConcreteSubject		*cs2;
	ConcreteSubject		*cs3;
	ConcreteObserver	*co1;
	ConcreteObserver	*co2;

	cs1 = new ConcreteSubject();
	cs2 = new ConcreteSubject();
	cs3 = new ConcreteSubject();
	co1 = new ConcreteObserver(cs1);
	co2 = new ConcreteObserver(NULL);

	co1->addSubject(cs2);
	co1->addSubject(cs3);
	co2->addSubject(cs1);
	co2->addSubject(cs2);
	co2->addSubject(cs3);
	co1->removeSubject(cs1);
	delete cs1;
	delete cs2;

	fail_if(co1->getDelNotifyCount() != 1,
	    "Notify count is %d (should be 1)", co1->getDelNotifyCount());
	fail_if(co2->getDelNotifyCount() != 2,
	    "Notify count is %d (should be 2)", co2->getDelNotifyCount());

	co2->removeSubject(cs3);
	delete cs3;

	fail_if(co1->getDelNotifyCount() != 2,
	    "Notify count is %d (should be 2)", co1->getDelNotifyCount());
	fail_if(co2->getDelNotifyCount() != 2,
	    "Notify count is %d (should be 2)", co2->getDelNotifyCount());

	delete co1;
	delete co2;
}
END_TEST

TCase *
getTc_Observer(void)
{
	TCase *testCase;

	testCase = tcase_create("Observer");

	tcase_add_test(testCase, observer_s1_o0);
	tcase_add_test(testCase, observer_s0_o1);
	tcase_add_test(testCase, observer_s1_o3);
	tcase_add_test(testCase, observer_s2_o3);
	tcase_add_test(testCase, observer_s3_o2);
	tcase_add_test(testCase, observer_delete);

	return (testCase);
}
