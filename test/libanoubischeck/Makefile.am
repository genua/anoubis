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

#
# Build the library
#
noinst_LIBRARIES = libanoubischeck.a
noinst_HEADERS = anoubischeck.h

libanoubischeck_a_SOURCES = anoubischeck.c
libanoubischeck_a_CFLAGS = @CHECK_CFLAGS@ $(AM_CFLAGS)

# A libanoubischeck.so would have to be linked with libcheck_pic.a instead of
# libcheck.a. Unfortunately libcheck_pic.a is not available under openbsd.
# The required autoconf/automake magic to determine if we have libcheck_pic.a
# is not worth the hassle. We just skip libanoubischeck.so and accept that we
# can't do real baseline tests with test_libanoubischeck.

#
# Test the library
#
TESTS = test_libanoubischeck
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=.
noinst_PROGRAMS = test_libanoubischeck
test_libanoubischeck_CFLAGS = @CHECK_CFLAGS@ $(AM_CFLAGS)
test_libanoubischeck_LDADD = @CHECK_LIBS@ libanoubischeck.a
test_libanoubischeck_SOURCES = test_libanoubischeck.c
