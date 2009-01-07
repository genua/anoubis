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

BUILT_SOURCES = apncvslog.c date.c apnmd.c

AM_YFLAGS = -papnvm
AM_LFLAGS = -olex.yy.c

noinst_LIBRARIES = libapnvm.a

libapnvm_a_SOURCES =	\
	apncvs.c	\
	apncvslog.l	\
	apnmd.l		\
	apnvm.c		\
	apnvmutil.c	\
	date.y

noinst_HEADERS = apncvs.h apnmd.h apnvm.h apnvmutil.h

bsdcompatdir   = src/bsdcompat
libapndir      = src/libapn
libbsdcompat   = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a

# Extra includes & dependencies
apnvm_includes = -I$(top_builddir)/src -I$(top_srcdir)/include
apnvm_dependencies =

if NEEDBSDCOMPAT
apnvm_includes += -I$(top_srcdir)/$(bsdcompatdir)
apnvm_dependencies += $(libbsdcompat)
endif

# -Wno-error required with flex 2.5.34 on Ubuntu hardy
libapnvm_a_CFLAGS = -std=gnu99 $(AM_CFLAGS) -Wno-error
libapnvm_a_CPPFLAGS = $(apnvm_includes) $(AM_CPPFLAGS)
libapnvm_a_DEPENDENCIES	= $(apnvm_dependencies)
libapnvm_a_LIBADD =

libapnvm_a_SOURCES_lint =	\
	apncvs.c_lint		\
	apncvs.h_lint		\
	apnvm.c_lint		\
	apnvm.h_lint		\
	apnvmutil.c_lint	\
	apnvmutil.h_lint

libapnvm_a_SOURCES_splint =	\
	apncvs.c_splint		\
	apncvs.h_splint		\
	apnvm.c_splint		\
	apnvm.h_splint		\
	apnvmutil.c_splint	\
	apnvmutil.h_splint

libapnvm_a_SOURCES_flawfinder =	\
	apncvs.c_flawfinder	\
	apncvs.h_flawfinder	\
	apnvm.c_flawfinder	\
	apnvm.h_flawfinder	\
	apnvmutil.c_flawfinder	\
	apnvmutil.h_flawfinder

#
# For unit tests of the components of apnvm, we build a shared library.
# This library is for testing purpose only. We tear apart the unittest and
# the test candidate. Thus we are able to run tests and baseline-tests
# propperly.
# To reduce the compile time and to test what's in the final product, we
# reuse the already compiled objects of apnvm. Ensure those are
# built with -fPIC.
#
noinst_PROGRAMS = libapnvm.so
libapnvm_so_CFLAGS = -fPIC -shared -std=gnu99 $(shared_AM_CFLAGS) -Wno-error
libapnvm_so_CPPFLAGS = $(libapnvm_a_CPPFLAGS)
libapnvm_so_SOURCES = $(libapnvm_a_SOURCES)
libapnvm_so_LDFLAGS = -nostartfiles -shared $(shared_AM_LDFLAGS) \
	-L$(top_builddir)/$(libapndir) -lapn


flawfinder_FLAGS=
splint_FLAGS=$(libapnvm_a_CPPFLAGS)
lint_FLAGS=$(libapnvm_a_CPPFLAGS)

lint:		$(libapnvm_a_SOURCES_lint)
splint:		$(libapnvm_a_SOURCES_splint)
flawfinder:	$(libapnvm_a_SOURCES_flawfinder)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(libapnvm_a_SOURCES_lint)
	@rm -f $(libapnvm_a_SOURCES_splint)
	@rm -f $(libapnvm_a_SOURCES_flawfinder)

# As we're in maintainer mode we have to remove generated source files
# manually.
dist-hook:
	$(MAKE) maintainer-clean-generic
