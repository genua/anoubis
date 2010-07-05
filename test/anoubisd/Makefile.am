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
		  anoubisd_wblock anoubisd_rblock anoubisd_csmulti

TESTS = test_peunit

anoubisddir = src/anoubisd
anoubisdsrcdir   = $(top_srcdir)/$(anoubisddir)
anoubisdbuilddir = $(top_builddir)/$(anoubisddir)

include $(top_srcdir)/src/Makefile.include

TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(test_ldpath)

AM_CPPFLAGS = $(test_cppflags) -I$(anoubisdsrcdir) -I$(anoubisdbuilddir)
AM_LDFLAGS = $(test_ldflags)
LDADD = $(test_ldadd)

test_peunit_objs= \
	$(anoubisdbuilddir)/pe_alf.o \
	$(anoubisdbuilddir)/pe_context.o \
	$(anoubisdbuilddir)/pe_ipc.o \
	$(anoubisdbuilddir)/pe.o \
	$(anoubisdbuilddir)/pe_prefixhash.o \
	$(anoubisdbuilddir)/pe_proc.o \
	$(anoubisdbuilddir)/pe_sandbox.o \
	$(anoubisdbuilddir)/pe_sfscache.o \
	$(anoubisdbuilddir)/pe_sfs.o \
	$(anoubisdbuilddir)/pe_filetree.o \
	$(anoubisdbuilddir)/pe_playground.o \
	$(anoubisdbuilddir)/anoubis_alloc.o

test_peunit_LDADD = \
	$(test_ldadd) \
	$(test_peunit_objs)

test_peunit_DEPENDENCIES = \
	$(test_dependencies) \
	$(test_peunit_objs)

test_peunit_SOURCES = \
	anoubisd_testcase_pe.c \
	anoubisd_testcase_pe_filetree.c \
	anoubisd_testcase_upgrade.c \
	anoubisd_unit.h \
	test_peunit.c

test_anoubisd_DEPENDENCIES = $(test_dependencies)
test_anoubisd_SOURCES = \
	anoubisd_testcase_sessions.c \
	anoubisd_testsuite.c \
	anoubisd_testrunner.c

anoubisd_policy_dos_DEPENDENCIES = $(test_dependencies)
anoubisd_policy_dos_SOURCES = \
	anoubisd_policy_dos.c \
	protocol_utils.c \
	protocol_utils.h

anoubisd_wblock_DEPENDENCIES = $(test_dependencies)
anoubisd_wblock_SOURCES = \
	anoubisd_wblock.c \
	protocol_utils.c\
	protocol_utils.h

anoubisd_rblock_DEPENDENCIES = $(test_dependencies)
anoubisd_rblock_SOURCES = \
	anoubisd_rblock.c \
	protocol_utils.c\
	protocol_utils.h

anoubisd_csmulti_DEPENDENCIES = $(test_dependencies)
anoubisd_csmulti_SOURCES = \
	anoubisd_csmulti.c \
	protocol_utils.c \
	protocol_utils.h
