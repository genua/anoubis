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

noinst_LIBRARIES = libkey.a

libkey_a_SOURCES = keygen.h keygen.c

libkey_a_SOURCES_lint =	\
	keygen.c_lint

libkey_a_SOURCES_splint =	\
	keygen.c_splint

libkey_a_SOURCES_flawfinder =	\
	keygen.c_flawfinder

noinst_PROGRAMS = libkey.so
libkey_so_CFLAGS = -fPIC -shared -std=gnu99 $(shared_AM_CFLAGS)
libkey_so_SOURCES = $(libkey_a_SOURCES)
libkey_so_LDFLAGS = -nostartfiles -shared $(shared_AM_LDFLAGS)

flawfinder_FLAGS=
splint_FLAGS=$(libkey_a_CPPFLAGS)
lint_FLAGS=$(libkey_a_CPPFLAGS)

lint:		$(libkey_a_SOURCES_lint)
splint:		$(libkey_a_SOURCES_splint)
flawfinder:	$(libkey_a_SOURCES_flawfinder)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(libkey_a_SOURCES_lint)
	@rm -f $(libkey_a_SOURCES_splint)
	@rm -f $(libkey_a_SOURCES_flawfinder)

# As we're in maintainer mode we have to remove generated source files
# manually.
dist-hook:
	$(MAKE) maintainer-clean-generic
