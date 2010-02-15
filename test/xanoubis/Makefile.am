# Copyright (c) 2007 GeNUA mbH <info@genua.de>
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

if GCOV_ENABLED
#	These test suites were disabled when configuring with GCOV_ENABLED
#	as they break the code coverage report. For details see BUG #837
TESTS = \
	pidtest
else
# XXX ST: #423
#         These tests were disabled due to the fact that they heavily rely on
#         on a running anoubisd which can't be expected to be run during
#         'make distcheck'.
#         Nethertheless that test is quite useful and still should be shipped.
# TESTS = test_xanoubis_server testSuite_JobCtrl testSuite_Patterns \
#	  testSuite_Profiles testSuite_Sfs
TESTS = \
	testSuite_Patterns \
	testSuite_Profiles \
	testSuite_Policies \
	testSuite_Services \
	testSuite_Sandbox \
	testSuite_Sfs \
	testSuite_StringList \
	testSuite_Key \
	testSuite_Notifications \
	testSuite_Config \
	pidtest \
	wxStringTest
noinst_PROGRAMS = test_xanoubis_server testSuite_JobCtrl $(TESTS)
endif

xanoubisdir       = src/xanoubis
xanoubisbuilddir  = $(top_builddir)/$(xanoubisdir)
xanoubissrcdir    = $(top_srcdir)/$(xanoubisdir)
libGUIcomponents  = $(xanoubisbuilddir)/libGUIcomponents.so

include $(top_srcdir)/src/Makefile.include

RPATH = $(test_ldpath):$(xanoubisbuilddir)
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(RPATH)

AM_CPPFLAGS = $(test_cppflags) \
	-I$(xanoubissrcdir) \
	-I$(xanoubissrcdir)/model \
	-I$(xanoubissrcdir)/view \
	-I$(xanoubissrcdir)/control \
	-I$(xanoubissrcdir)/pattern \
	-I$(xanoubissrcdir)/job \
	-I$(xanoubissrcdir)/widgets \
	$(WX_CPPFLAGS)

AM_CXXFLAGS = \
	-fno-strict-aliasing \
	$(prog_AM_CXXFLAGS) \
	$(WX_CXXFLAGS) \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(NOTIFY_CFLAGS) \
	$(SSL_CFLAGS)


AM_LDFLAGS = $(test_ldflags) \
	-L$(xanoubisbuilddir)

LDADD = $(test_ldadd) \
	@WX_LIBS@ \
	@GTK_LIBS@ \
	@GLIB_LIBS@ \
	@NOTIFY_LIBS@ \
	$(SSL_LIBS)


dependencies = \
	$(libGUIcomponents) \
	$(test_dependencies)

test_xanoubis_server_DEPENDENCIES = $(test_dependencies)
test_xanoubis_server_LDADD = $(test_ldadd)
test_xanoubis_server_SOURCES = \
	xanoubis_dummyserver_testrunner.c \
	xanoubis_dummyserver_testsuite.c \
	xanoubis_testcase_dummyserver.c

#
# Since the source files in the sub directories have conflicting names, the
# created object file names must be distinct. To achieve this, we set the
# CPPFLAGS fariable for each test (which would otherwise be unnecessary).
# An alternative would be to create a Makefile.am per subdir.
#

#
# Testsuite: Pattern
#
testSuite_Patterns_CPPFLAGS = $(AM_CPPFLAGS)
testSuite_Patterns_DEPENDENCIES = $(dependencies)
testSuite_Patterns_LDADD = $(LDADD) -lGUIcomponents

testSuite_Patterns_SOURCES = \
	testrunner.cpp \
	suite_Patterns/testSuite.cpp \
	suite_Patterns/tc_Singleton.cpp \
	suite_Patterns/tc_IndexTree.cpp \
	suite_Patterns/tc_Observer.cpp

#
# Testsuite: Profile
#
testSuite_Profiles_CPPFLAGS = $(AM_CPPFLAGS)
testSuite_Profiles_DEPENDENCIES = $(dependencies)
testSuite_Profiles_LDADD = $(LDADD) -lGUIcomponents

testSuite_Profiles_SOURCES = \
	testrunner.cpp \
	suite_Profiles/testSuite.cpp \
	suite_Profiles/tc_PolicyCtrl.cpp

#
# Testsuite: Policy
#
testSuite_Policies_CPPFLAGS = $(AM_CPPFLAGS)
testSuite_Policies_DEPENDENCIES = $(dependencies)
testSuite_Policies_LDADD = $(LDADD) -lGUIcomponents

testSuite_Policies_SOURCES = \
	testrunner.cpp \
	suite_Policies/testSuite.cpp \
	suite_Policies/PolicyObserver.cpp \
	suite_Policies/PolicyObserver.h \
	suite_Policies/policyChecks.h \
	suite_Policies/tc_AlfAppPolicy.cpp \
	suite_Policies/tc_AlfCapabilityFilterPolicy.cpp \
	suite_Policies/tc_AlfFilterPolicy.cpp \
	suite_Policies/tc_ContextAppPolicy.cpp \
	suite_Policies/tc_ContextFilterPolicy.cpp \
	suite_Policies/tc_DefaultFilterPolicy.cpp \
	suite_Policies/tc_SbAccessFilterPolicy.cpp \
	suite_Policies/tc_SbAppPolicy.cpp \
	suite_Policies/tc_SfsAppPolicy.cpp \
	suite_Policies/tc_SfsFilterPolicy.cpp \
	suite_Policies/tc_SfsDefaultFilterPolicy.cpp \
	suite_Policies/tc_PolicyRuleSet.cpp \
	suite_Policies/tc_PolicyUtils.cpp \
	suite_Policies/tc_PolicyChecks.cpp

#
# Testsuite: Services
#
testSuite_Services_CPPFLAGS = $(AM_CPPFLAGS) -D_PATH_SERVICES=\"/tmp/foobar\"
testSuite_Services_DEPENDENCIES = $(dependencies)
testSuite_Services_LDADD = $(LDADD) -lGUIcomponents

testSuite_Services_SOURCES = \
	testrunner.cpp \
	suite_Services/testSuite.cpp

#
# Testsuite: Sandbox
#
testSuite_Sandbox_CPPFLAGS = $(AM_CPPFLAGS)
testSuite_Sandbox_DEPENDENCIES = $(dependencies)
testSuite_Sandbox_LDADD = $(LDADD) -lGUIcomponents

testSuite_Sandbox_SOURCES = \
	testrunner.cpp \
	suite_Sandbox/testSuite.cpp

#
# Testsuite: Sfs
#
testSuite_Sfs_CPPFLAGS = $(AM_CPPFLAGS)
testSuite_Sfs_DEPENDENCIES = $(dependencies)
testSuite_Sfs_LDADD = $(LDADD) -lGUIcomponents

testSuite_Sfs_SOURCES = \
	testrunner.cpp \
	suite_Sfs/testSuite.cpp \
	suite_Sfs/tc_SfsDir.cpp

#
# Testsuite: StringList
#
testSuite_StringList_CPPFLAGS = $(AM_CPPFLAGS)
testSuite_StringList_DEPENDENCIES = $(dependencies)
testSuite_StringList_LDADD = $(LDADD) -lGUIcomponents

testSuite_StringList_SOURCES = \
	testrunner.cpp \
	suite_StringList/testSuite.cpp

#
# Testsuite: Key
#
testSuite_Key_CPPFLAGS = $(AM_CPPFLAGS)
testSuite_Key_DEPENDENCIES = $(dependencies)
testSuite_Key_LDADD = $(LDADD) -lGUIcomponents

testSuite_Key_SOURCES = \
	testrunner.cpp \
	suite_Key/testSuite.cpp \
	suite_Key/tc_KeyCtrl.cpp \
	suite_Key/tc_LocalCertificate.cpp \
	suite_Key/tc_PrivKey.cpp

#
# Testsuite: JobCtrl
#
testSuite_JobCtrl_CPPFLAGS = $(AM_CPPFLAGS)
testSuite_JobCtrl_DEPENDENCIES = $(dependencies)
testSuite_JobCtrl_LDADD = $(LDADD) -lGUIcomponents

testSuite_JobCtrl_SOURCES = \
	testrunner.cpp \
	suite_JobCtrl/JobCtrlEventSpy.cpp \
	suite_JobCtrl/JobCtrlEventSpy.h \
	suite_JobCtrl/TaskEventSpy.cpp \
	suite_JobCtrl/TaskEventSpy.h \
	suite_JobCtrl/tc_Disconnected.cpp \
	suite_JobCtrl/tc_Connected.cpp \
	suite_JobCtrl/tc_SfsList.cpp \
	suite_JobCtrl/testSuite.cpp \
	suite_JobCtrl/utils.cpp \
	suite_JobCtrl/utils.h

#
# Testsuite: Notification
#
testSuite_Notifications_CPPFLAGS = $(AM_CPPFLAGS)
testSuite_Notifications_DEPENDENCIES = $(dependencies)
testSuite_Notifications_LDADD = $(LDADD) -lGUIcomponents

testSuite_Notifications_SOURCES = \
	testrunner.cpp \
	suite_Notifications/testSuite.cpp \
	suite_Notifications/tc_NotificationCtrl.cpp

#
# Testsuite: Config
#
testSuite_Config_CPPFLAGS = $(AM_CPPFLAGS)
testSuite_Config_DEPENDENCIES = $(dependencies)
testSuite_Config_LDADD = $(LDADD) -lGUIcomponents

testSuite_Config_SOURCES = \
	testrunner.cpp \
	suite_Config/testSuite.cpp \
	suite_Config/tc_Config.cpp \
	suite_Config/AnEventsSpy.cpp \
	suite_Config/AnEventsSpy.h

#
# pidtest
#
pidtest_SOURCES = pidtest.cpp

#
# wxStringTest
#
wxStringTest_SOURCES = wxStringTest.cpp
