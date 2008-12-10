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

noinst_LIBRARIES = libcsum.a
libcsum_a_SOURCES =	\
	csum.c	\
	csum.h

LIBS += $(SSL_LIBS)

bsdcompatdir   = src/bsdcompat
libbsdcompat   = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a

# Extra includes & dependencies
csum_includes = -I$(top_srcdir)/src \
		-I$(top_builddir)/src \
		-I$(top_srcdir)/include \
		-I$(top_srcdir)/src/libanoubisprotocol \
		-I$(top_srcdir)/src/libanoubischat
csum_dependencies =

if NEEDBSDCOMPAT
csum_includes += -I$(top_srcdir)/$(bsdcompatdir)
csum_dependencies += $(libbsdcompat)
endif

libcsum_a_CFLAGS = -std=gnu99 $(AM_CFLAGS) $(SSL_CFLAGS)
libcsum_a_CPPFLAGS = $(csum_includes) $(AM_CPPFLAGS)
libcsum_a_DEPENDENCIES	= $(csum_dependencies)

libcsum_a_SOURCES_lint =	\
	csum.c_lint

libcsum_a_SOURCES_splint =	\
	csum.c_splint		\
	csum.h_splint

libcsum_a_SOURCES_flawfinder =	\
	csum.c_flawfinder	\
	csum.h_flawfinder

flawfinder_FLAGS=
splint_FLAGS=$(libcsum_a_CPPFLAGS)
lint_FLAGS=$(libcsum_a_CPPFLAGS)

lint:		$(libcsum_a_SOURCES_lint)
splint:		$(libcsum_a_SOURCES_splint)
flawfinder:	$(libcsum_a_SOURCES_flawfinder)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(libcsum_a_SOURCES_lint)
	@rm -f $(libcsum_a_SOURCES_splint)
	@rm -f $(libcsum_a_SOURCES_flawfinder)

# As we're in maintainer mode we have to remove generated source files
# manually.
dist-hook:
	$(MAKE) maintainer-clean-generic
