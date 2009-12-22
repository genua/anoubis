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

noinst_LIBRARIES = libauth.a

libauth_a_SOURCES = \
	auth.c	\
	auth.h

LIBS += $(SSL_LIBS)

bsdcompatdir   = src/bsdcompat
libbsdcompat   = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a

# Extra includes & dependencies
auth_includes = -I$(top_srcdir)/src \
		-I$(top_builddir)/src \
		-I$(top_srcdir)/include \
		-I$(top_builddir)/include \
		-I$(top_srcdir)/src/libanoubisprotocol \
		-I$(top_srcdir)/src/libanoubisprotocol/client \
		-I$(top_srcdir)/src/libanoubischat \
		-I$(top_srcdir)/src/libanoubissig
auth_dependencies =

if NEEDBSDCOMPAT
auth_includes += -I$(top_srcdir)/$(bsdcompatdir)
auth_dependencies += $(libbsdcompat)
endif

libauth_a_CFLAGS = -std=gnu99 $(AM_CFLAGS) $(SSL_CFLAGS)
libauth_a_CPPFLAGS = $(auth_includes) $(AM_CPPFLAGS)
libauth_a_DEPENDENCIES	= $(auth_dependencies)

libauth_a_SOURCES_lint =	\
	auth.c_lint

libauth_a_SOURCES_splint =	\
	auth.c_splint		\
	auth.h_splint

libauth_a_SOURCES_flawfinder =	\
	auth.c_flawfinder	\
	auth.h_flawfinder

noinst_PROGRAMS = libauth.so
libauth_so_CFLAGS = -fPIC -shared -std=gnu99 $(shared_AM_CFLAGS)
libauth_so_CPPFLAGS = $(libauth_a_CPPFLAGS)
libauth_so_SOURCES = $(libauth_a_SOURCES)
libauth_so_LDFLAGS = -nostartfiles -shared $(shared_AM_LDFLAGS)
libauth_so_LDADD = -lfl
if NEEDBSDCOMPAT
libauth_so_LDADD += -lbsdcompat -L$(top_builddir)/$(bsdcompatdir)
endif

flawfinder_FLAGS=
splint_FLAGS=$(libauth_a_CPPFLAGS)
lint_FLAGS=$(libauth_a_CPPFLAGS)

lint:		$(libauth_a_SOURCES_lint)
splint:		$(libauth_a_SOURCES_splint)
flawfinder:	$(libauth_a_SOURCES_flawfinder)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(libauth_a_SOURCES_lint)
	@rm -f $(libauth_a_SOURCES_splint)
	@rm -f $(libauth_a_SOURCES_flawfinder)

# As we're in maintainer mode we have to remove generated source files
# manually.
dist-hook:
	$(MAKE) maintainer-clean-generic
