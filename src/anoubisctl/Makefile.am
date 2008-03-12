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

sbin_PROGRAMS = anoubisctl

if STATIC_LINKING
STATIC = -static
endif

libapndir	= src/libapn
libapn		= $(top_builddir)/$(libapndir)/libapn.a
bsdcompatdir	= src/bsdcompat
libbsdcompat	= $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/$(libapndir)

anoubisctl_LDADD = $(libapn)

anoubisctl_DEPENDENCIES = $(libapn)

anoubisctl_LDFLAGS= $(AM_LDFLAGS) $(STATIC)

if NEEDBSDCOMPAT
INCLUDES += -I$(top_srcdir)/$(bsdcompatdir)
anoubisctl_LDADD += $(libbsdcompat)
endif

anoubisctl_SOURCES= \
	main.c \
	anoubisctl.h

anoubisctl_SOURCES_flawfinder= \
	main.c_flawfinder \
	anoubisctl.h_flawfinder

anoubisctl_SOURCES_lint= \
	main.c_lint

anoubisctl_SOURCES_splint= \
	main.c_splint

flawfinder_FLAGS=
splint_FLAGS=+unixlib $(INCLUDES) -I..
lint_FLAGS=

flawfinder:	$(anoubisctl_SOURCES_flawfinder)
splint:		$(anoubisctl_SOURCES_splint)
lint:		$(anoubisctl_SOURCES_lint)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(anoubisctl_SOURCES_lint)
	@rm -f $(anoubisctl_SOURCES_splint)
	@rm -f $(anoubisctl_SOURCES_flawfinder)
