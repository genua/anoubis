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

TESTS = test_libanoubissig

bsdcompatdir	= src/bsdcompat
libbsdcompat	= $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
anoubissigdir	= src/libanoubissig
libanoubissig	= $(top_builddir)/$(anoubissigdir)/libanoubissig.a

# set TESTS_ENVIRONMENT for checks
RPATH = $(top_builddir)/$(anoubissigdir):$(top_builddir)/$(bsdcompatdir)
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(RPATH)

noinst_PROGRAMS = test_libanoubissig

test_libanoubissig_CFLAGS = $(AM_CFLAGS) @CHECK_CFLAGS@ \
			    -I$(top_srcdir)/include
test_libanoubissig_CPPFLAGS = -I$(top_srcdir)/$(anoubissigdir) \
			      -I$(top_srcdir)/include

# We link with check.a which is compiled without -fPIE, therefore
# we must use shared_AM_LDFLAGS in order not to link with -pie.
test_libanoubissig_LDFLAGS = \
	$(shared_AM_LDFLAGS) -dynamic \
	-L$(top_builddir)/$(anoubissigdir)

# conditionally static link of libraries to accomplish correct Code Coverage
if GCOV_ENABLED
test_libanoubissig_LDADD = \
	@CHECK_LIBS@ \
	$(SSL_LIBS) \
	$(top_builddir)/$(anoubissigdir)/libanoubissig.a
test_libanoubissig_DEPENDENCIES = \
	$(top_builddir)/$(anoubissigdir)/libanoubissig.a
else
test_libanoubissig_LDADD = @CHECK_LIBS@ -lanoubissig $(SSL_LIBS)
test_libanoubissig_DEPENDENCIES = $(libanoubissig)
endif


if NEEDBSDCOMPAT
test_libanoubissig_DEPENDENCIES += $(libbsdcompat)
test_libanoubissig_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
test_libanoubissig_LDADD += $(libbsdcompat)
endif

test_libanoubissig_SOURCES =                  \
	libanoubissig_testcase_signature.c    \
	libanoubissig_testsuite.c             \
	libanoubissig_testrunner.c
