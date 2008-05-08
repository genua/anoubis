# Copyright (c) 2008 GeNUA mbH <info@genua.de>
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

TESTS = test_libanoubisproto test_libanoubisnotify
anoubisprotocoldir	= src/libanoubisprotocol
anoubischatdir		= src/libanoubischat
bsdcompatdir		= src/bsdcompat
libanoubischat = $(top_builddir)/$(anoubischatdir)/libanoubischat.so
libbsdcompat = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
libanoubisps = $(top_builddir)/$(anoubisprotocoldir)/libanoubisps.so
libanoubispsa = $(top_builddir)/$(anoubisprotocoldir)/libanoubisps.a
libanoubispc = $(top_builddir)/$(anoubisprotocoldir)/libanoubispc.so

# set TESTS_ENVIRONMENT for checks
RPATH = $(top_builddir)/$(anoubisprotocoldir):$(top_builddir)/$(anoubischatdir):$(top_builddir)/$(bsdcompatdir)
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(RPATH)

noinst_PROGRAMS = test_libanoubisproto test_libanoubisnotify libanoubischat.so
CPPFLAGS =						\
	-I$(top_srcdir)/$(anoubischatdir)		\
	-I$(top_srcdir)/$(anoubisprotocoldir)/client	\
	-I$(top_srcdir)/$(anoubisprotocoldir)/server	\
	-I$(top_srcdir)/$(anoubisprotocoldir)

libanoubischat_so_CFLAGS = -fPIC -shared
libanoubischat_so_LDFLAGS = -nostartfiles -shared

test_libanoubisproto_LDFLAGS = \
	-L$(top_builddir)/$(anoubisprotocoldir) \
	-L$(top_builddir)/$(anoubischatdir) -dynamic
test_libanoubisnotify_LDFLAGS = \
	-L. -L$(top_builddir)/$(anoubisprotocoldir)
test_libanoubisproto_LDADD = @CHECK_LIBS@ -lanoubisps -lanoubispc \
	-lanoubischat
test_libanoubisnotify_LDADD = @CHECK_LIBS@ -lanoubisps -lanoubischat

test_libanoubisproto_DEPENDENCIES =	\
	$(libanoubisps) $(libanoubispc) $(libanoubischat)
test_libanoubisnotify_DEPENDENCIES = $(libanoubisps) libanoubischat.so

if NEEDBSDCOMPAT
CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
test_libanoubisproto_LDFLAGS += -L$(top_builddir)/$(bsdcompatdir)
test_libanoubisnotify_LDFLAGS += -L$(top_builddir)/$(bsdcompatdir)
test_libanoubisproto_LDADD += -lbsdcompat
test_libanoubisnotify_LDADD += -lbsdcompat
test_libanoubisproto_DEPENDENCIES += $(libbsdcompat)
test_libanoubisnotify_DEPENDENCIES += $(libbsdcompat)
endif

test_libanoubisproto_CPPFLAGS = @CHECK_CFLAGS@ $(AM_CPPFLAGS)
test_libanoubisnotify_CPPFLAGS = @CHECK_CFLAGS@ $(AM_CPPFLAGS)

test_libanoubisproto_SOURCES = \
	testrunner.c \
	libanoubisproto_testsuite.c \
	libanoubisproto_testcase_core.c

test_libanoubisnotify_SOURCES = \
	testrunner_notify.c \
	libanoubisprotonotify_testsuite.c \
	libanoubisprotonotify_testcase.c \
	anoubischat_dummy.c \
	anoubischat_dummy.h
libanoubischat_so_SOURCES =		\
	anoubischat_dummy.h		\
	anoubischat_dummy.c
