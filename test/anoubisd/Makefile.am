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

# The tests "test_anoubisd" and "anoubisd_polciy_dos" are disabled due to
# the fact that they depends on a running anoubisd which can't be expected
# during a 'make distcheck'.
# Nethertheless the tests are useful and should be shipped.

noinst_PROGRAMS = test_anoubisd test_peunit anoubisd_policy_dos \
		  anoubisd_wblock anoubisd_rblock

TESTS = test_peunit

anoubisddir     = src/anoubisd
anoubischatdir  = src/libanoubischat
bsdcompatdir    = src/bsdcompat
anoubisprotodir = src/libanoubisprotocol
anoubischeckdir = test/libanoubischeck
anoubisapn	= src/libapn
anoubissigdir	= src/libanoubissig
libapndir	= src/libapn
libanoubischat  = $(top_builddir)/$(anoubischatdir)/libanoubischat.a
libanoubispc    = $(top_builddir)/$(anoubisprotodir)/libanoubispc.a
libanoubisps	= $(top_builddir)/$(anoubisprotodir)/libanoubisps.a
libbsdcompat    = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
libanoubischeck = $(top_builddir)/$(anoubischeckdir)/libanoubischeck.a
libanoubissig	= $(top_builddir)/$(anoubissigdir)/libanoubissig.a
libapn		= $(top_builddir)/$(libapndir)/libapn.a

test_anoubisd_CPPFLAGS = \
	-I$(top_srcdir)/$(anoubischatdir) \
	-I$(top_srcdir)/$(anoubisddir) \
	-I$(top_srcdir)/$(anoubisprotodir) \
	-I$(top_srcdir)/$(anoubisprotodir)/client \
	-I$(top_srcdir)/$(anoubisprotodir)/server \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/$(bsdcompatdir) \
	-I$(top_srcdir)/$(anoubischeckdir) \
	-I$(top_srcdir)/$(anoubisapn) \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src/anoubisd \
	-I$(top_srcdir)/$(anoubissigdir) \
	@CHECK_CFLAGS@

test_peunit_CPPFLAGS = $(test_anoubisd_CPPFLAGS)
anoubisd_policy_dos_CPPFLAGS = $(test_anoubisd_CPPFLAGS)
anoubisd_wblock_CPPFLAGS = $(test_anoubisd_CPPFLAGS)
anoubisd_rblock_CPPFLAGS = $(test_anoubisd_CPPFLAGS)

# We link with check.a which is compiled without -fPIE, therefore
# we must use shared_AM_LDFLAGS in order not to link with -pie.
test_anoubisd_LDFLAGS = $(shared_AM_LDFLAGS)

test_peunit_objs= \
	$(top_builddir)/$(anoubisddir)/pe_alf.o \
	$(top_builddir)/$(anoubisddir)/pe_context.o \
	$(top_builddir)/$(anoubisddir)/pe_ipc.o \
	$(top_builddir)/$(anoubisddir)/pe.o \
	$(top_builddir)/$(anoubisddir)/pe_prefixhash.o \
	$(top_builddir)/$(anoubisddir)/pe_proc.o \
	$(top_builddir)/$(anoubisddir)/pe_sandbox.o \
	$(top_builddir)/$(anoubisddir)/pe_sfscache.o \
	$(top_builddir)/$(anoubisddir)/pe_sfs.o \
	$(top_builddir)/$(anoubisddir)/pe_filetree.o

test_peunit_LDADD = \
	@CHECK_LIBS@ \
	@LIBS@ \
	@SSL_LIBS@ \
	$(test_peunit_objs) \
	$(libanoubischeck) \
	$(libanoubispc) \
	$(libanoubisps) \
	$(libanoubissig) \
	$(libapn) \
	$(libanoubischat)

test_anoubisd_LDADD = \
	@CHECK_LIBS@ \
	$(libanoubischeck) \
	$(libanoubispc) \
	$(libanoubischat)

anoubisd_policy_dos_LDADD = \
	$(libanoubispc) \
	$(libanoubischat)

anoubisd_wblock_LDADD = \
	$(libanoubispc) \
	$(libanoubischat)

anoubisd_rblock_LDADD = \
	$(libanoubispc) \
	$(libanoubischat)

if NEEDBSDCOMPAT
test_anoubisd_LDADD += $(libbsdcompat)
test_peunit_LDADD += $(libbsdcompat)
anoubisd_policy_dos_LDADD += $(libbsdcompat)
anoubisd_wblock_LDADD += $(libbsdcompat)
anoubisd_rblock_LDADD += $(libbsdcompat)
endif

test_anoubisd_SOURCES = \
	anoubisd_testcase_sessions.c \
	anoubisd_testsuite.c \
	anoubisd_testrunner.c

test_peunit_SOURCES = \
	anoubisd_testcase_pe.c \
	anoubisd_testcase_pe_filetree.c \
	anoubisd_testcase_upgrade.c \
	anoubisd_unit.h \
	test_peunit.c

anoubisd_policy_dos_SOURCES = \
	anoubisd_policy_dos.c \
	protocol_utils.c

anoubisd_wblock_SOURCES = \
	anoubisd_wblock.c \
	protocol_utils.c

anoubisd_rblock_SOURCES = \
	anoubisd_rblock.c \
	protocol_utils.c
