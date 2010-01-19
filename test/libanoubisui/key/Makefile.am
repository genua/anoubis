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

keydir		= src/libanoubisui/key
keytestdir	= test/libanoubisui/key
key		= $(top_builddir)/$(keydir)/libkey.a
anoubischeckdir	= test/libanoubischeck
anoubischeck	= $(top_builddir)/$(anoubischeckdir)/libanoubischeck.a
bsdcompatdir	= src/bsdcompat

noinst_PROGRAMS = libanoubisui_key_tester test_subject
TESTS = test_subject

# Make sure that test find their libraries when run with make check
RPATH = $(top_builddir)/$(bsdcompatdir)
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(RPATH) KEYDIR=$(top_srcdir)/$(keytestdir)

libanoubisui_key_tester_CPPFLAGS = -I$(top_srcdir)/$(keydir)
test_subject_CPPFLAGS =	\
	-I$(top_srcdir)/$(keydir) \
	-I$(top_srcdir)/$(anoubischeckdir)
test_subject_CFLAGS = $(AM_CFLAGS) @CHECK_CFLAGS@ @SSL_CFLAGS@

libanoubisui_key_tester_CFLAGS = $(AM_CFLAGS) @SSL_CFLAGS@
libanoubisui_key_tester_LDADD = $(key) @SSL_LIBS@
libanoubisui_key_tester_LDFLAGS = $(AM_LDFLAGS) -L$(top_builddir)/$(keydir)
libanoubisui_key_tester_CPPFLAGS += -DGCOV

test_subject_LDADD = $(key) $(anoubischeck) @CHECK_LIBS@ @SSL_LIBS@
test_subject_LDFLAGS = $(AM_LDFLAGS) -L$(top_builddir)/$(keydir)
test_subject_CPPFLAGS += -DGCOV

if NEEDBSDCOMPAT
libanoubisui_key_tester_LDFLAGS += -L$(top_builddir)/src/bsdcompat -lbsdcompat
test_subject_LDFLAGS += -L$(top_builddir)/src/bsdcompat -lbsdcompat
endif

libanoubisui_key_tester_SOURCES = libanoubisui_key_tester.c
test_subject_SOURCES = test_subject.c

EXTRA_DIST = keys/complete.key keys/complete.cert \
	keys/no_state.key keys/no_state.cert \
	keys/umlauts.key keys/umlauts.cert
