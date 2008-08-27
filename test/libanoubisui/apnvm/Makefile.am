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

TESTS = test_apnvm

bsdcompatdir	= src/bsdcompat
libbsdcompat	= $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
apnvmdir	= src/libanoubisui/apnvm
apnvm		= $(top_builddir)/$(apnvmdir)/libapnvm.a

# set TESTS_ENVIRONMENT for checks
RPATH = $(top_builddir)/$(apnvmdir):$(top_builddir)/$(bsdcompatdir)
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(RPATH)

noinst_PROGRAMS = test_apnvm

test_apnvm_CFLAGS = $(AM_CFLAGS) @CHECK_CFLAGS@
test_apnvm_CPPFLAGS = -I$(top_srcdir)/$(apnvmdir)
test_apnvm_LDFLAGS = \
	$(AM_LDFLAGS) -dynamic \
	-L$(top_builddir)/$(apnvmdir)

# conditionally static link of libraries to accomplish correct Code Coverage
test_apnvm_DEPENDENCIES = $(apnvm)
test_apnvm_LDADD = @CHECK_LIBS@ $(test_apnvm_DEPENDENCIES) -lfl

if NEEDBSDCOMPAT
test_apnvm_DEPENDENCIES += $(libbsdcompat)
test_apnvm_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
test_apnvm_LDADD += $(libbsdcompat)
endif

test_apnvm_SOURCES =		\
	apnvm_testrunner.c	\
	apnvm_tc_cvs.c
