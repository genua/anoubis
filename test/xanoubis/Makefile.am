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
	testSuite_Sfs \
	testSuite_Key \
	pidtest
noinst_PROGRAMS = test_xanoubis_server testSuite_JobCtrl $(TESTS)
endif

uidir		   = src/libanoubisui
apndir		   = src/libapn
xanoubisdir	   = src/xanoubis
bsdcompatdir	   = src/bsdcompat
anoubischatdir	   = src/libanoubischat
anoubisprotocoldir = src/libanoubisprotocol
anoubissigdir      = src/libanoubissig
anoubischeckdir    = test/libanoubischeck

libanoubisui	  = $(top_builddir)/$(uidir)/libanoubisui.a
libapn		  = $(top_builddir)/$(apndir)/libapn.a
libbsdcompat	  = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
libanoubischat	  = $(top_builddir)/$(anoubischatdir)/libanoubischat.a
libprotocolserver = $(top_builddir)/$(anoubisprotocoldir)/libanoubisps.a
libprotocolclient = $(top_builddir)/$(anoubisprotocoldir)/libanoubispc.a
libanoubissig     = $(top_builddir)/$(anoubissigdir)/libanoubissig.a
libGUIcomponents  = $(top_builddir)/$(xanoubisdir)/libGUIcomponents.so
libanoubischeck   = $(top_builddir)/$(anoubischeckdir)/libanoubischeck.a

libanouibisui_CPPFLAGS = -I$(top_srcdir)/$(uidir)
libapn_CPPFLAGS = -I$(top_srcdir)/$(apndir)
libbsdcompat_CPPFLAGS = -I$(top_srcdir)/$(bsdcompatdir)
libanoubischat_CPPFLAGS = -I$(top_srcdir)/$(anoubischatdir)
libprotocolserver_CPPFLAGS = \
	-I$(top_srcdir)/$(anoubisprotocoldir) \
	-I$(top_srcdir)/$(anoubisprotocoldir)/server
libprotocolclient_CPPFLAGS = \
	-I$(top_srcdir)/$(anoubisprotocoldir) \
	-I$(top_srcdir)/$(anoubisprotocoldir)/client
libanoubissig_CPPFLAGS = -I$(top_srcdir)/$(anoubissigdir)
libGUIcomponents_CPPFLAGS = \
	$(libanouibisui_CPPFLAGS) \
	$(libapn_CPPFLAGS) \
	$(libanoubischat_CPPFLAGS) \
	$(libprotocolclient_CPPFLAGS) \
	$(libanoubissig_CPPFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/$(xanoubisdir) \
	-I$(top_srcdir)/$(xanoubisdir)/model \
	-I$(top_srcdir)/$(xanoubisdir)/view \
	-I$(top_srcdir)/$(xanoubisdir)/control \
	-I$(top_srcdir)/$(xanoubisdir)/pattern \
	-I$(top_srcdir)/$(xanoubisdir)/job \
	-I$(top_srcdir)/$(anoubischeckdir) \
	$(WX_CPPFLAGS)
libGUIcomponents_CXXFLAGS = \
	-fno-strict-aliasing \
	$(AM_CXXFLAGS) \
	$(WX_CXXFLAGS) \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(NOTIFY_CFLAGS) \
	$(SSL_CFLAGS)
libGUIcomponents_DEPENDS = \
	$(libGUIcomponents) \
	$(libanoubischat) \
	$(libprotocolclient) \
	$(libanoubissig) \
	$(libapn) \
	$(libanoubisui)
libGUIcomponents_LDADD = \
	$(libanoubischat) \
	$(libprotocolclient) \
	$(libanoubissig) \
	$(libapn) \
	$(libanoubisui) \
	$(libanoubischeck) \
	@WX_LIBS@ \
	@GTK_LIBS@ \
	@GLIB_LIBS@ \
	@NOTIFY_LIBS@ \
	$(SSL_LIBS)

RPATH = $(top_builddir)/$(xanoubisdir)
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(RPATH)

test_xanoubis_server_CFLAGS = $(AM_CFLAGS) @CHECK_CFLAGS@
test_xanoubis_server_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/$(anoubischeckdir) \
	$(libanoubischat_CPPFLAGS) \
	$(libprotocolserver_CPPFLAGS)

test_xanoubis_server_DEPENDENCIES = \
	$(libanoubischat) \
	$(libprotocolserver)

if NEEDBSDCOMPAT
test_xanoubis_server_CPPFLAGS += $(libbsdcompat_CPPFLAGS)
test_xanoubis_server_DEPENDENCIES += $(libbsdcompat)
endif

test_xanoubis_server_LDADD = \
	$(test_xanoubis_server_DEPENDENCIES) \
	$(libanoubischeck) @CHECK_LIBS@

# We link with check.a which is compiled without -fPIE, therefore
# we must use shared_AM_LDFLAGS in order not to link with -pie.
test_xanoubis_server_LDFLAGS = $(shared_AM_LDFLAGS)

test_xanoubis_server_SOURCES = \
	xanoubis_dummyserver_testrunner.c \
	xanoubis_dummyserver_testsuite.c \
	xanoubis_testcase_dummyserver.c

#
# Testsuite: Pattern
#
testSuite_Patterns_CPPFLAGS = $(libGUIcomponents_CPPFLAGS)
testSuite_Patterns_CXXFLAGS = $(libGUIcomponents_CXXFLAGS) @CHECK_CFLAGS@
testSuite_Patterns_LDFLAGS = $(shared_AM_LDFLAGS) -dynamic -L $(RPATH)
testSuite_Patterns_DEPENDENCIES = $(libGUIcomponents_DEPENDS)
testSuite_Patterns_LDADD = \
	-lGUIcomponents \
	$(libGUIcomponents_LDADD) \
	@CHECK_LIBS@

if NEEDBSDCOMPAT
testSuite_Patterns_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
testSuite_Patterns_DEPENDENCIES += $(libbsdcompat)
testSuite_Patterns_LDADD += $(libbsdcompat)
endif

testSuite_Patterns_SOURCES = \
	testrunner.cpp \
	suite_Patterns/testSuite.cpp \
	suite_Patterns/tc_Singleton.cpp \
	suite_Patterns/tc_Observer.cpp

#
# Testsuite: Profile
#
testSuite_Profiles_CPPFLAGS = $(libGUIcomponents_CPPFLAGS)
testSuite_Profiles_CXXFLAGS = $(libGUIcomponents_CXXFLAGS) @CHECK_CFLAGS@
testSuite_Profiles_LDFLAGS = $(shared_AM_LDFLAGS) -dynamic -L $(RPATH)
testSuite_Profiles_DEPENDENCIES = $(libGUIcomponents_DEPENDS)
testSuite_Profiles_LDADD = \
	-lGUIcomponents \
	$(libGUIcomponents_LDADD) \
	@CHECK_LIBS@

if NEEDBSDCOMPAT
testSuite_Profiles_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
testSuite_Profiles_DEPENDENCIES += $(libbsdcompat)
testSuite_Profiles_LDADD += $(libbsdcompat)
endif

testSuite_Profiles_SOURCES = \
	testrunner.cpp \
	suite_Profiles/testSuite.cpp \
	suite_Profiles/tc_ProfileCtrl.cpp

#
# Testsuite: Policy
#
testSuite_Policies_CPPFLAGS = $(libGUIcomponents_CPPFLAGS)
testSuite_Policies_CXXFLAGS = $(libGUIcomponents_CXXFLAGS) @CHECK_CFLAGS@
testSuite_Policies_LDFLAGS = $(shared_AM_LDFLAGS) -dynamic -L $(RPATH)
testSuite_Policies_DEPENDENCIES = $(libGUIcomponents_DEPENDS)
testSuite_Policies_LDADD = \
	-lGUIcomponents \
	$(libGUIcomponents_LDADD) \
	@CHECK_LIBS@

if NEEDBSDCOMPAT
testSuite_Policies_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
testSuite_Policies_DEPENDENCIES += $(libbsdcompat)
testSuite_Policies_LDADD += $(libbsdcompat)
endif

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
	suite_Policies/tc_PolicyUtils.cpp

#
# Testsuite: Sfs
#
testSuite_Sfs_CPPFLAGS = $(libGUIcomponents_CPPFLAGS)
testSuite_Sfs_CXXFLAGS = $(libGUIcomponents_CXXFLAGS) @CHECK_CFLAGS@
testSuite_Sfs_LDFLAGS = $(shared_AM_LDFLAGS) -dynamic -L $(RPATH)
testSuite_Sfs_DEPENDENCIES = $(libGUIcomponents_DEPENDS)
testSuite_Sfs_LDADD = \
	-lGUIcomponents \
	$(libGUIcomponents_LDADD) \
	@CHECK_LIBS@

if NEEDBSDCOMPAT
testSuite_Sfs_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
testSuite_Sfs_DEPENDENCIES += $(libbsdcompat)
testSuite_Sfs_LDADD += $(libbsdcompat)
endif

testSuite_Sfs_SOURCES = \
	testrunner.cpp \
	suite_Sfs/testSuite.cpp \
	suite_Sfs/tc_SfsDir.cpp

#
# Testsuite: Key
#
testSuite_Key_CPPFLAGS = $(libGUIcomponents_CPPFLAGS)
testSuite_Key_CXXFLAGS = $(libGUIcomponents_CXXFLAGS) @CHECK_CFLAGS@
testSuite_Key_LDFLAGS = $(shared_AM_LDFLAGS) -dynamic -L $(RPATH)
testSuite_Key_DEPENDENCIES = $(libGUIcomponents_DEPENDS)
testSuite_Key_LDADD = \
	-lGUIcomponents \
	$(libGUIcomponents_LDADD) \
	@CHECK_LIBS@

if NEEDBSDCOMPAT
testSuite_Key_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
testSuite_Key_DEPENDENCIES += $(libbsdcompat)
testSuite_Key_LDADD += $(libbsdcompat)
endif

testSuite_Key_SOURCES = \
	testrunner.cpp \
	suite_Key/testSuite.cpp \
	suite_Key/tc_KeyCtrl.cpp \
	suite_Key/tc_LocalCertificate.cpp \
	suite_Key/tc_PrivKey.cpp

#
# Testsuite: JobCtrl
#
testSuite_JobCtrl_CPPFLAGS = $(libGUIcomponents_CPPFLAGS)
testSuite_JobCtrl_CXXFLAGS = $(libGUIcomponents_CXXFLAGS) @CHECK_CFLAGS@
testSuite_JobCtrl_LDFLAGS = $(shared_AM_LDFLAGS) -dynamic -L $(RPATH)
testSuite_JobCtrl_DEPENDENCIES = $(libGUIcomponents_DEPENDS)
testSuite_JobCtrl_LDADD = \
	-lGUIcomponents \
	$(libGUIcomponents_LDADD) \
	@CHECK_LIBS@

if NEEDBSDCOMPAT
testSuite_JobCtrl_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
testSuite_JobCtrl_DEPENDENCIES += $(libbsdcompat)
testSuite_JobCtrl_LDADD += $(libbsdcompat)
endif

testSuite_JobCtrl_SOURCES = \
	testrunner.cpp \
	suite_JobCtrl/JobCtrlApp.cpp \
	suite_JobCtrl/JobCtrlApp.h \
	suite_JobCtrl/TestHandler.h \
	suite_JobCtrl/TcComTask.cpp \
	suite_JobCtrl/TcComTask.h \
	suite_JobCtrl/TcCsumCalcTask.cpp \
	suite_JobCtrl/TcCsumCalcTask.h \
	suite_JobCtrl/testSuite.cpp \
	suite_JobCtrl/tc_JobCtrl.cpp

#
# pidtest
#
pidtest_SOURCES = pidtest.cpp
pidtest__CPPFLAGS = $(WX_CPPFLAGS)
pidtest_CXXFLAGS = $(WX_CXXFLAGS)
pidtest_LDADD = @WX_LIBS@
