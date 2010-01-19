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

noinst_PROGRAMS = libanoubisui_key_tester test_subject
TESTS = test_subject

include $(top_srcdir)/src/Makefile.include

TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(test_ldpath) KEYDIR=$(srcdir)

AM_CPPFLAGS = $(test_cppflags)
AM_LDFLAGS = $(test_ldflags)
LDADD = $(test_ldadd)

libanoubisui_key_tester_DEPENDENCIES = $(test_dependencies)
libanoubisui_key_tester_SOURCES = libanoubisui_key_tester.c

test_subject_DEPENDENCIES = $(test_dependencies)
test_subject_SOURCES = test_subject.c

EXTRA_DIST = keys/complete.key keys/complete.cert \
	keys/no_state.key keys/no_state.cert \
	keys/umlauts.key keys/umlauts.cert
