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

TESTS = test_libapn test_printing test_insertcopy

noinst_PROGRAMS = test_libapn test_printing test_insertcopy

libapndir       = src/libapn
bsdcompatdir    = src/bsdcompat
libapn		= $(top_builddir)/$(libapndir)/libapn.so
libbsdcompat    = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a

# set TESTS_ENVIRONMENT for checks
RPATH = $(top_builddir)/$(libapndir):$(top_builddir)/$(bsdcompatdir)
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(RPATH)

test_libapn_CPPFLAGS = \
	-I$(top_srcdir)/$(libapndir) \
	-I$(top_srcdir)/$(bsdcompatdir) \
	-I$(top_srcdir)/include \
	@CHECK_CFLAGS@

test_libapn_LDFLAGS = \
	-L$(top_builddir)/$(libapndir) -dynamic
test_libapn_LDADD = @CHECK_LIBS@ -lapn

test_libapn_DEPENDENCIES = $(libapn)

if NEEDBSDCOMPAT
test_libapn_LDADD += $(libbsdcompat)
test_libapn_DEPENDENCIES += $(libbsdcompat)
endif

test_libapn_SOURCES = \
	libapn_testcase_errorcodes.c \
	libapn_testcase_crash1.c \
	libapn_testsuite.c \
	libapn_testrunner.c \
	libapn_testcase_iov.c

test_printing_CPPFLAGS = \
	-I$(top_srcdir)/$(libapndir) \
	-I$(top_srcdir)/$(bsdcompatdir) \
	-I$(top_srcdir)/include \
	@CHECK_CFLAGS@

test_printing_LDFLAGS = \
	-L$(top_builddir)/$(libapndir) -dynamic
test_printing_LDADD = @CHECK_LIBS@ -lapn

test_printing_DEPENDENCIES = $(libapn)

if NEEDBSDCOMPAT
test_printing_LDADD += $(libbsdcompat)
test_printing_DEPENDENCIES += $(libbsdcompat)
endif

test_printing_SOURCES = \
	printing_testcase_ruleset.c \
	printing_testsuite.c \
	printing_testrunner.c

test_insertcopy_CPPFLAGS = \
	-I$(top_srcdir)/$(libapndir) \
	-I$(top_srcdir)/$(bsdcompatdir) \
	-I$(top_srcdir)/include \
	@CHECK_CFLAGS@

test_insertcopy_LDFLAGS = \
	-L$(top_builddir)/$(libapndir) -dynamic
test_insertcopy_LDADD = @CHECK_LIBS@ -lapn
test_insertcopy_DEPENDENCIES = $(libapn)

if NEEDBSDCOMPAT
test_insertcopy_LDADD += $(libbsdcompat)
test_insertcopy_DEPENDENCIES += $(libbsdcompat)
endif

test_insertcopy_SOURCES = \
	insertcopy_testcase_copy.c \
	insertcopy_testsuite.c \
	insertcopy_testrunner.c
