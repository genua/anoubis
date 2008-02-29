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

# XXX CH: Deactivate this test; see Bug #368
TESTS =
#test_xanoubis

# XXX CH: Deactivate this test; see Bug #368
# noinst_PROGRAMS = test_xanoubis

bsdcompatdir   = src/bsdcompat
libbsdcompat   = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
xanoubisdir    = src/xanoubis
anoubischatdir = src/libanoubischat
libanoubischat = $(top_builddir)/$(anoubischatdir)/libanoubischat.a
anoubisprotocoldir = src/libanoubisprotocol
libanoubisprotocol = $(top_builddir)/$(anoubisprotocoldir)/libanoubispc.a

# XXX CH: Deactivate this test; see Bug #368
test_xanoubis_CPPFLAGS_hidden = \
	-I$(top_srcdir)/$(anoubischatdir) \
	-I$(top_srcdir)/$(anoubisprotocoldir) \
	-I$(top_srcdir)/$(anoubisprotocoldir)/client \
	@CHECK_CFLAGS@ \
	-I$(top_srcdir)/$(xanoubisdir) \
	-I$(top_srcdir)/$(xanoubisdir)/model \
	-I$(top_srcdir)/$(xanoubisdir)/view \
	-I$(top_srcdir)/$(xanoubisdir)/control \
	$(AM_CPPFLAGS) $(WX_CPPFLAGS)

# XXX CH: Deactivate this test; see Bug #368
test_xanoubis_CXXFLAGS_hide = -fno-strict-aliasing $(AM_CXXFLAGS) $(WX_CXXFLAGS)

# XXX CH: Deactivate this test; see Bug #368
test_xanoubis_LDADD_hidden = \
	$(top_builddir)/$(xanoubisdir)/xanoubis-Communicator.o \
	$(top_builddir)/$(xanoubisdir)/xanoubis-CommunicatorException.o \
	@CHECK_LIBS@ \
	$(libanoubischat) \
	$(libanoubisprotocol) \
	@WX_LIBS@

if NEEDBSDCOMPAT
# XXX CH: Deactivate this test; see Bug #368
test_xanoubis_LDADD_hidden += $(libbsdcompat)
endif

# XXX CH: Deactivate this test; see Bug #368
test_xanoubis_SOURCES_hidden = \
	xanoubis_testcase_Communicator.cpp \
	xanoubis_testcase_CommunicatorException.cpp \
	xanoubis_testsuite.cpp \
	xanoubis_testrunner.cpp
