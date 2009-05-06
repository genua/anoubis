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
libanoubischeck_a_CFLAGS = @CHECK_CFLAGS@

#
# For unit tests of the components, we build a shared library.
# This library is for testing purpose only. We tear apart the unittest and
# the test candidate. Thus we are able to run tests and baseline-tests
# propperly.
# To reduce the compile time and to test what's in the final product, we
# reuse the already compiled objects. Ensure those are built with -fPIC.
#
noinst_PROGRAMS = libanoubischeck.so
libanoubischeck_so_SOURCES =
libanoubischeck_so_DEPENDENCIES = $(libanoubischeck_a_OBJECTS)
libanoubischeck_so_LINK = \
	$(CXX) -fstack-protector -shared -Wl,-soname,$@ \
	    -o $@ $(libanoubischeck_a_OBJECTS)

#
# Test the library
#
TESTS = test_libanoubischeck
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=.
noinst_PROGRAMS += test_libanoubischeck
test_libanoubischeck_CFLAGS = @CHECK_CFLAGS@
test_libanoubischeck_DEPENDENCIES = libanoubischeck.so
test_libanoubischeck_LDADD = @CHECK_LIBS@ -L. -lanoubischeck
test_libanoubischeck_SOURCES = test_libanoubischeck.c
