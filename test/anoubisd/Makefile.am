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
#	  This test was disabled due to the fact that it heavily depends on
#	  on a running anoubisd which can't be expected to be run during
#	  'make distcheck'.
#	  Nethertheless that test is quite useful and still should be shipped.
# TESTS = test_anoubisd

noinst_PROGRAMS = test_anoubisd

anoubisddir     = src/anoubisd
anoubischatdir  = src/libanoubischat
bsdcompatdir    = src/bsdcompat
anoubisprotodir = src/libanoubisprotocol
libanoubischat  = $(top_builddir)/$(anoubischatdir)/libanoubischat.a
libanoubispc    = $(top_builddir)/$(anoubisprotodir)/libanoubispc.a
libbsdcompat    = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a

test_anoubisd_CPPFLAGS = \
	-I$(top_srcdir)/$(anoubischatdir) \
	-I$(top_srcdir)/$(anoubisddir) \
	-I$(top_srcdir)/$(anoubisprotodir) \
	-I$(top_srcdir)/$(anoubisprotodir)/client \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/$(bsdcompatdir) \
	-I$(top_builddir)/src \
	@CHECK_CFLAGS@

# We link with check.a which is compiled without -fPIE, therefore
# we must use shared_AM_LDFLAGS in order not to link with -pie.
test_anoubisd_LDFLAGS = $(shared_AM_LDFLAGS)

test_anoubisd_LDADD = \
	@CHECK_LIBS@ \
	$(libanoubispc) \
	$(libanoubischat)

if NEEDBSDCOMPAT
test_anoubisd_LDADD += $(libbsdcompat)
endif

test_anoubisd_SOURCES = \
	anoubisd_testcase_sessions.c \
	anoubisd_testsuite.c \
	anoubisd_testrunner.c
