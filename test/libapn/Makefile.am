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

TESTS = test_libapn test_printing test_insertcopy test_rbtree

noinst_PROGRAMS = test_libapn test_printing test_insertcopy test_rbtree \
	apnedit_helper

include $(top_srcdir)/src/Makefile.include

TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(test_ldpath)

AM_CPPFLAGS = $(test_cppflags)
AM_LDFLAGS = $(test_ldflags)
LDADD = $(test_ldadd)

test_libapn_DEPENDENCIES = $(test_dependencies)
test_libapn_SOURCES = \
	libapn_testcase_errorcodes.c \
	libapn_testcase_crash1.c \
	libapn_testsuite.c \
	libapn_testrunner.c \
	libapn_testcase_iov.c \
	libapn_testcase_firstinsert.c

apnedit_helper_DEPENDENCIES = $(test_dependencies)
apnedit_helper_SOURCES = \
	apnedit_helper.c

test_printing_DEPENDENCIES = $(test_dependencies)
test_printing_SOURCES = \
	printing_testcase_ruleset.c \
	printing_testsuite.c \
	printing_testrunner.c

test_rbtree_DEPENDENCIES = $(test_dependencies)
test_rbtree_SOURCES = test_rbtree.c

test_insertcopy_DEPENDENCIES = $(test_dependencies)
test_insertcopy_SOURCES = \
	insertcopy_testcase_copy.c \
	insertcopy_testsuite.c \
	insertcopy_testrunner.c
