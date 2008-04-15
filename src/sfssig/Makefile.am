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
#

sbin_PROGRAMS = sfssig

if STATIC_LINKING
STATIC = -static
endif

CFLAGS += $(SSL_CFLAGS)

LIBS += $(SSL_LIBS)

sfssig_LDFLAGS = $(AM_LDFLAGS) $(SSL_LDFLAGS)

sfssig_SOURCES= \
	main.c

sfssig_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/src
if NEEDBSDCOMPAT
sfssig_CPPFLAGS += -I$(top_srcdir)/src/bsdcompat
endif

sfssig_SOURCES_flawfinder= \
	main.c_flawfinder

sfssig_SOURCES_lint= \
	main.c_lint

sfssig_SOURCES_splint= \
	main.c_splint

flawfinder_FLAGS=

# sfssig uses parts of the openbsd library, which has no proper
# annotations for splint. We therefore have to disable checking for
# complete definitions, as splint cannot know the internal semantics of
# OpenSSL at this point.
splint_FLAGS=+unixlib $(sfssig_CPPFLAGS) -compdef
lint_FLAGS=$(sfssig_CPPFLAGS)

flawfinder:	$(sfssig_SOURCES_flawfinder)
splint:		$(sfssig_SOURCES_splint)
lint:		$(sfssig_SOURCES_lint)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(sfssig_SOURCES_lint)
	@rm -f $(sfssig_SOURCES_splint)
	@rm -f $(sfssig_SOURCES_flawfinder)

# As we're in maintainer mode we have to remove generated source files
# manually.
dist-hook:
	$(MAKE) maintainer-clean-generic
