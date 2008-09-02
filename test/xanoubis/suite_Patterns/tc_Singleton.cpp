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
#include <Singleton.h>
#include <Singleton.cpp>

class SingleObjTest : public Singleton<SingleObjTest>
{
	public:
		~SingleObjTest(void);

		static SingleObjTest	*instance(void);
		void			 setId(int);
		int			 getId(void) const;

	protected:
		SingleObjTest(void);

	private:
		int	id_;

	friend class Singleton<SingleObjTest>;
};

SingleObjTest::SingleObjTest(void) : Singleton<SingleObjTest>()
{
	id_ = 0;
}

SingleObjTest::~SingleObjTest(void)
{
	id_ = -1;
}

SingleObjTest *
SingleObjTest::instance(void)
{
	return (Singleton<SingleObjTest>::instance());
}

void
SingleObjTest::setId(int id)
{
	id_ = id;
}

int
SingleObjTest::getId(void) const
{
	return (id_);
}

START_TEST(singleton_create)
{
	SingleObjTest	*so;

	so = SingleObjTest::instance();
	fail_if(so == NULL, "Couldn't claim an instance.");

	delete so;
}
END_TEST

START_TEST(singleton_once)
{
	SingleObjTest	*so1;
	SingleObjTest	*so2;

	so1 = SingleObjTest::instance();
	so1->setId(22);
	fail_if(so1->getId() != 22, "Couldn't set id.");

	so2 = SingleObjTest::instance();
	fail_if(so2->getId() != 22, "Not the same instance.");

	so2->setId(33);
	fail_if(so2->getId() != 33, "Couldn't set id.");
	fail_if(so1->getId() != 33, "Not the same instance.");

	delete so1; /* This also deletes so2, 'cause it's the same object. */
}
END_TEST

START_TEST(singleton_init)
{
	SingleObjTest	*so;

	so = SingleObjTest::instance();
	fail_if(so == NULL, "Couldn't claim an instance.");
	fail_if(so->getId() != 0, "Not initialized.");

	so->setId(44);
	fail_if(so->getId() != 44, "Couldn't set id.");

	delete so;
	so = NULL;

	so = SingleObjTest::instance();
	fail_if(so == NULL, "Couldn't claim 2nd instance.");
	fail_if(so->getId() != 0, "Not a new object and not initialized.");

	delete so;
}
END_TEST

TCase *
getTc_Singleton(void)
{
	TCase *testCase;

	testCase = tcase_create("Singleton");
	tcase_add_test(testCase, singleton_create);
	tcase_add_test(testCase, singleton_once);
	tcase_add_test(testCase, singleton_init);

	return (testCase);
}
