# Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

bsdcompatdir   = src/bsdcompat
libbsdcompat   = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a

# Evil hack to build a binary into the library-dir
# automake rejects lib_PROGRAMS, but mylib_PROGRAMS is accepted
mylibdir=$(libdir)
mylib_PROGRAMS = libanoubispreload.so

libanoubispreload_so_SOURCES = preload.c
libanoubispreload_so_CFLAGS = $(shared_AM_CFLAGS) -fPIC -shared
libanoubispreload_so_CPPFLAGS =
libanoubispreload_so_LDFLAGS = -nostartfiles -shared $(shared_AM_LDFLAGS)
libanoubispreload_so_LDADD = $(DL_LIB)

if NEEDBSDCOMPAT
libanoubispreload_so_CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
libanoubispreload_so_LDADD += $(libbsdcompat)
libanoubispreload_so_DEPENDENCIES = $(libbsdcompat)
endif

libanoubispreload_so_SOURCES_lint = \
	preload.c_lint

libanoubispreload_so_SOURCES_splint = \
	preload.c_splint

libanoubispreload_so_SOURCES_flawfinder = \
	preload.c_flawfinder

flawfinder_FLAGS=
splint_FLAGS=$(INCLUDES)
lint_FLAGS=$(INCLUDES)

lint: $(libanoubispreload_so_SOURCES_lint)
splint: $(libanoubispreload_so_SOURCES_splint)
flawfinder: $(libanoubispreload_so_SOURCES_flawfinder)

linting: lint splint flawfinder

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(libanoubispreload_so_SOURCES_lint)
	@rm -f $(libanoubispreload_so_SOURCES_flawfinder)
	@rm -f $(libanoubispreload_so_SOURCES_splint)
