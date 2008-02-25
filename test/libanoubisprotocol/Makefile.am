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

TESTS = test_libanoubisps test_libanoubispc

anoubisprotocoldir	= src/libanoubisprotocol
libanoubisps = $(top_builddir)/$(anoubisprotocoldir)/libanoubisps.so
libanoubispc = $(top_builddir)/$(anoubisprotocoldir)/libanoubispc.so

noinst_PROGRAMS = test_libanoubisps test_libanoubispc
INCLUDES = -I$(top_srcdir)/$(anoubisprotocoldir)

AM_LDFLAGS = -L$(top_builddir)/$(anoubisprotocoldir) -dynamic
test_libanoubisps_LDADD = @CHECK_LIBS@ -lanoubisps
test_libanoubispc_LDADD = @CHECK_LIBS@ -lanoubispc
test_libanoubisps_CPPFLAGS = @CHECK_CFLAGS@ $(AM_CPPFLAGS)
test_libanoubispc_CPPFLAGS = @CHECK_CFLAGS@ $(AM_CPPFLAGS)

test_libanoubisps_DEPENDENCIES = $(libanoubisps)
test_libanoubispc_DEPENDENCIES = $(libanoubispc)

test_libanoubisps_SOURCES = \
	testrunner.c \
	libanoubisps_testsuite.c \
	libanoubisps_testcase_core.c

test_libanoubispc_SOURCES = \
	testrunner.c \
	libanoubispc_testsuite.c \
	libanoubispc_testcase_core.c
