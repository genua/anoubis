##########################################################################
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
##########################################################################

TESTS = test_csum

bsdcompatdir	= src/bsdcompat
libbsdcompat	= $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
chatdir		= src/libanoubischat
apndir		= src/libapn
protocoldir	= src/libanoubisprotocol
csumdir		= src/libanoubisui/csum
csum		= $(top_builddir)/$(csumdir)/libcsum.a
anoubischeckdir = test/libanoubischeck
libanoubischeck = $(top_builddir)/$(anoubischeckdir)/libanoubischeck.a
libanoubisprotocol = $(top_builddir)/$(protocoldir)/libanoubispc.so
libanoubischat = $(top_builddir)/$(chatdir)/libanoubischat.so
libapn = $(top_builddir)/$(apndir)/libapn.so

# set TESTS_ENVIRONMENT for checks
RPATH = $(top_builddir)/$(bsdcompatdir):$(top_builddir)/$(csumdir):$(top_builddir)/$(protocoldir):$(top_builddir)/$(chatdir):$(top_builddir)/$(apndir)
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(RPATH)

noinst_PROGRAMS = test_csum

test_csum_CFLAGS = \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/include \
	$(AM_CFLAGS) \
	@CHECK_CFLAGS@

test_csum_CPPFLAGS = \
	-I$(top_srcdir)/$(csumdir) \
	-I$(top_srcdir)/$(chatdir) \
	-I$(top_srcdir)/$(protocoldir) \
	-I$(top_srcdir)/$(anoubischeckdir)

test_csum_DEPENDENCIES =

# conditionally static link of libraries to accomplish correct Code Coverage
if GCOV_ENABLED
test_csum_LDADD = @CHECK_LIBS@ $(csum) $(libanoubischeck) -lfl -lssl
test_csum_LDFLAGS = \
	$(AM_LDFLAGS) \
	-L$(top_builddir)/$(csumdir)
test_csum_CPPFLAGS += -DGCOV
else
test_csum_LDADD = @CHECK_LIBS@ $(libanoubischeck) -lfl -lssl \
    -L$(top_builddir)/$(csumdir) -lcsum \
    -L$(top_builddir)/$(protocoldir) -lanoubispc \
    -L$(top_builddir)/$(chatdir) -lanoubischat \
    -L$(top_builddir)/$(apndir) -lapn
test_csum_LDFLAGS = $(AM_LDFLAGS) -dynamic
endif

if NEEDBSDCOMPAT
test_csum_DEPENDENCIES += $(libbsdcompat)
test_csum_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
test_csum_LDADD += $(libbsdcompat)
endif

test_csum_SOURCES = \
	csum_testrunner.c
