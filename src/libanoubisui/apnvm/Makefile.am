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

BUILT_SOURCES = apncvslog.c date.c
#AM_YFLAGS = -d
libapnvm_a_CFLAGS = -std=gnu99 $(AM_CFLAGS)

noinst_LIBRARIES = libapnvm.a

libapnvm_a_SOURCES =	\
	apncvs.c	\
	apncvslog.l	\
	apnvm.c		\
	date.y

noinst_HEADERS = apncvs.h apnvm.h

bsdcompatdir   = src/bsdcompat
libbsdcompat   = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a

libapnvm_a_CFLAGS += -I$(top_builddir)/src -I$(top_srcdir)/include
libapnvm_a_LIBADD =

if NEEDBSDCOMPAT
libapnvm_a_CFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
endif

libapnvm_a_CPPFLAGS = $(libapnvm_a_CFLAGS)

libapnvm_a_SOURCES_lint =	\
	apncvs.c_lint		\
	apncvs.h_lint		\
	apnvm.c_lint		\
	apnvm.h_lint

libapnvm_a_SOURCES_splint =	\
	apncvs.c_splint		\
	apncvs.h_splint		\
	apnvm.c_splint		\
	apnvm.h_splint

libapnvm_a_SOURCES_flawfinder =	\
	apncvs.c_flawfinder	\
	apncvs.h_flawfinder	\
	apnvm.c_flawfinder	\
	apnvm.h_flawfinder

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
