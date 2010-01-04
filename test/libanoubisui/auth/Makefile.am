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

TESTS = test_auth

bsdcompatdir	= src/bsdcompat
libbsdcompat	= $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
chatdir		= src/libanoubischat
apndir		= src/libapn
protocoldir	= src/libanoubisprotocol
authdir		= src/libanoubisui/auth
sigdir		= src/libanoubissig
auth		= $(top_builddir)/$(authdir)/libauth.a
anoubischeckdir = test/libanoubischeck
libanoubischeck = $(top_builddir)/$(anoubischeckdir)/libanoubischeck.a
libanoubisprotocol = $(top_builddir)/$(protocoldir)/libanoubispc.so
libanoubischat = $(top_builddir)/$(chatdir)/libanoubischat.so
libanoubissig = $(top_builddir)/$(sigdir)/libanoubisig.so
libapn = $(top_builddir)/$(apndir)/libapn.so
authtestdir = test/libanoubisui/auth

# set TESTS_ENVIRONMENT for checks
RPATH = $(top_builddir)/$(bsdcompatdir):$(top_builddir)/$(authdir):$(top_builddir)/$(protocoldir):$(top_builddir)/$(chatdir):$(top_builddir)/$(apndir)

TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(RPATH) KEYDIR=$(top_srcdir)/$(authtestdir)

noinst_PROGRAMS = test_auth

test_auth_CFLAGS = \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/include \
	$(AM_CFLAGS) \
	@CHECK_CFLAGS@

test_auth_CPPFLAGS = \
	-I$(top_srcdir)/$(authdir) \
	-I$(top_srcdir)/$(chatdir) \
	-I$(top_srcdir)/$(protocoldir) \
	-I$(top_srcdir)/$(protocoldir)/client \
	-I$(top_srcdir)/$(sigdir) \
	-I$(top_srcdir)/$(anoubischeckdir)

test_auth_DEPENDENCIES =

# conditionally static link of libraries to accomplish correct Code Coverage
if GCOV_ENABLED
test_auth_LDADD = @CHECK_LIBS@ $(auth) $(libanoubischeck) -lfl -lssl
test_auth_LDFLAGS = \
	$(AM_LDFLAGS) \
	-L$(top_builddir)/$(authdir)
test_auth_CPPFLAGS += -DGCOV
else
test_auth_LDADD = @CHECK_LIBS@ $(libanoubischeck) -lfl -lssl \
    -L$(top_builddir)/$(authdir) -lauth \
    -L$(top_builddir)/$(protocoldir) -lanoubispc \
    -L$(top_builddir)/$(chatdir) -lanoubischat \
    -L$(top_builddir)/$(sigdir) -lanoubissig \
    -L$(top_builddir)/$(apndir) -lapn
test_auth_LDFLAGS = $(AM_LDFLAGS) -dynamic
endif

if NEEDBSDCOMPAT
test_auth_DEPENDENCIES += $(libbsdcompat)
test_auth_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
test_auth_LDADD += $(libbsdcompat)
endif

test_auth_SOURCES = \
	auth_testrunner.c

EXTRA_DIST = test.key test.crt

install-exec-hook:
