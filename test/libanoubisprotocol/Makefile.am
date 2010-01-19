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

AM_CPPFLAGS = $(test_cppflags) -I$(top_srcdir)/test/libanoubisprotocol
AM_LDFLAGS = $(test_ldflags) -L$(top_builddir)/test/libanoubisprotocol

include $(top_srcdir)/src/Makefile.include

TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(test_ldpath):.

if GCOV_ENABLED
noinst_PROGRAMS = test_libanoubisproto test_libanoubisnotify
noinst_LIBRARIES = libanoubischat_dummy.a
libanoubischat_dummy = libanoubischat_dummy.a
else
noinst_PROGRAMS = test_libanoubisproto test_libanoubisnotify \
	libanoubischat_dummy.so
libanoubischat_dummy = libanoubischat_dummy.so
endif


libanoubischat_dummy_so_CFLAGS = $(shared_AM_CFLAGS)
libanoubischat_dummy_so_LDFLAGS = $(test_so_ldflags)

libanoubischat_dummy_so_SOURCES =		\
	anoubischat_dummy.h		\
	anoubischat_dummy.c

libanoubischat_dummy_a_SOURCES = $(libanoubischat_dummy_so_SOURCES)

test_libanoubisproto_LDADD = $(test_ldadd)
test_libanoubisnotify_LDADD = $(libanoubischat_dummy) $(test_ldadd)
test_libanoubisproto_DEPENDENCIES = $(test_dependencies)
test_libanoubisnotify_DEPENDENCIES = $(test_dependencies) \
	libanoubischat_dummy.so

test_libanoubisproto_SOURCES = \
	testrunner.c \
	libanoubisproto_testsuite.c \
	libanoubisproto_testcase_core.c

test_libanoubisnotify_SOURCES = \
	testrunner_notify.c \
	libanoubisprotonotify_testsuite.c \
	libanoubisprotonotify_testcase.c
