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

noinst_PROGRAMS = test_xanoubis_server

bsdcompatdir	   = src/bsdcompat
anoubischatdir	   = src/libanoubischat
anoubisprotocoldir = src/libanoubisprotocol

libbsdcompat	  = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
libanoubischat	  = $(top_builddir)/$(anoubischatdir)/libanoubischat.a
libprotocolserver = $(top_builddir)/$(anoubisprotocoldir)/libanoubisps.a

test_xanoubis_server_CFLAGS = $(AM_CFLAGS) @CHECK_CFLAGS@
test_xanoubis_server_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/$(anoubischatdir) \
	-I$(top_srcdir)/$(anoubisprotocoldir) \
	-I$(top_srcdir)/$(anoubisprotocoldir)/server

test_xanoubis_server_DEPENDENCIES = \
	$(libanoubischat) \
	$(libprotocolserver)

if NEEDBSDCOMPAT
test_xanoubis_server_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
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
