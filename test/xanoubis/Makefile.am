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

# XXX ST: #423
#         This test was disabled due to the fact that it heavily depends on
#         on a running anoubisd which can't be expected to be run during
#         'make distcheck'.
#         Nethertheless that test is quite useful and still should be shipped.
# TESTS = test_xanoubis_server

TESTS = testSuite_Patterns
noinst_PROGRAMS = test_xanoubis_server $(TESTS)

uidir		   = src/libanoubisui
apndir		   = src/libapn
xanoubisdir	   = src/xanoubis
bsdcompatdir	   = src/bsdcompat
anoubischatdir	   = src/libanoubischat
anoubisprotocoldir = src/libanoubisprotocol

libanoubisui	  = $(top_builddir)/$(uidir)/libanoubisui.a
libapn		  = $(top_builddir)/$(apndir)/libapn.a
libbsdcompat	  = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
libanoubischat	  = $(top_builddir)/$(anoubischatdir)/libanoubischat.a
libprotocolserver = $(top_builddir)/$(anoubisprotocoldir)/libanoubisps.a
libprotocolclient = $(top_builddir)/$(anoubisprotocoldir)/libanoubispc.a
libGUIcomponents  = $(top_builddir)/$(xanoubisdir)/libGUIcomponents.so

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
libGUIcomponents_CPPFLAGS = \
	-I$(top_srcdir)/$(xanoubisdir) \
	-I$(top_srcdir)/$(xanoubisdir)/model \
	-I$(top_srcdir)/$(xanoubisdir)/view \
	-I$(top_srcdir)/$(xanoubisdir)/control \
	-I$(top_srcdir)/$(xanoubisdir)/pattern \
	$(WX_CPPFLAGS)
libGUIcomponents_CXXFLAGS = \
	-fno-strict-aliasing \
	$(AM_CXXFLAGS) \
	$(WX_CXXFLAGS) \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(NOTIFY_CFLAGS) \
	$(SSL_CFLAGS)
libGUIcomponents_DEPENDENCIES = \
	$(libGUIcomponents) \
	$(libanoubischat) \
	$(libprotocolclient) \
	$(libapn) \
	$(libanoubisui)
libGUIcomponents_LDADD = \
	$(libanoubischat) \
	$(libprotocolclient) \
	$(libapn) \
	$(libanoubisui) \
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
	@CHECK_LIBS@

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
testSuite_Patterns_DEPENDENCIES = $(libGUIcomponents_DEPENDENCIES)
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
	suite_Patterns/tc_Singleton.cpp
