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

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <wxGuiTest/WxGuiTestHelper.h>
#include <wxGuiTest/CppUnitWarningAsserter.h>
#include <wxGuiTest/InitWxGuiTest.h>

#ifdef __WXGTK__
#define OUTPUT std::cout
#else
#define OUTPUT std::cerr
#endif

#include <stdlib.h>
#include <main.h>

int
main (int, char**)
{
	bool				 wasSucessful;
	AnoubisGuiApp			*myApp;
	CPPUNIT_NS::TestSuite		*suite;
	CPPUNIT_NS::TextUi::TestRunner	 runner;

	/* Configure unit testing */
	wxTst::WxGuiTestHelper::SetShowModalDialogsNonModalFlag(true);
	wxTst::WxGuiTestHelper::SetShowPopupMenusFlag(false);
	wxTst::WxGuiTestHelper::SetDisableTestInteractivity(true);
	wxTst::WxGuiTestHelper::SetPopupWarningForFailingAssert(false);
	/* Check for warnings and set needed asserter. */
	wxTst::WxGuiTestHelper::SetCheckForProvokedWarnings(true);
	wxTst::WxGuiTestHelper::SetWarningAsserter(
	    new wxTst::CppUnitWarningAsserter());

	/* Create test traget application. */
	myApp = new AnoubisGuiApp();

	/* Create and register test suite. */
	suite = new CPPUNIT_NS::TestSuite("All Tests");
	suite->addTest(wxTst::InitWxGuiTest::suite());
	runner.addTest(suite);

	/* Change output style to meet compiler error format. */
	runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
	    OUTPUT));

	/* Run the test */
	wasSucessful = runner.run();

	return wasSucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
