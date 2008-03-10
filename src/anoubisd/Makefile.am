# Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

BUILT_SOURCES = parse.h parse.c
AM_YFLAGS = -d

sbin_PROGRAMS = anoubisd

bsdcompatdir = src/bsdcompat
libbsdcompat = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
anoubischatdir = src/libanoubischat
libanoubischat = $(top_builddir)/$(anoubischatdir)/libanoubischat.a
anoubispsdir = src/libanoubisprotocol
libanoubisps = $(top_builddir)/$(anoubispsdir)/libanoubisps.a


if STATIC_LINKING
STATIC = -static
endif

INCLUDES = \
	   -I. \
	   -I$(top_builddir)/src \
	   -I$(top_srcdir)/include \
	   -I$(top_srcdir)/$(anoubischatdir) \
	   -I$(top_srcdir)/$(anoubispsdir) \
	   -I$(top_srcdir)/$(anoubispsdir)/server

anoubisd_LDADD = $(libanoubisps) $(libanoubischat)

if NEEDBSDCOMPAT
INCLUDES += -I$(top_srcdir)/$(bsdcompatdir)
anoubisd_LDADD += $(libbsdcompat)
endif

anoubisd_LDFLAGS = $(AM_LDFLAGS) $(STATIC)

anoubisd_SOURCES= \
	main.c \
	policy.c \
	printconf.c \
	session.c \
	log.c \
	anoubisd.h \
	parse.y \
	queue.h

anoubisd_SOURCES_flawfinder= \
	main.c_flawfinder \
	policy.c_flawfinder \
	printconf.c_flawfinder \
	session.c_flawfinder \
	log.c_flawfinder \
	anoubisd.h_flawfinder

anoubisd_SOURCES_lint= \
	main.c_lint \
	policy.c_lint \
	printconf.c_lint \
	session.c_lint \
	log.c_lint

anoubisd_SOURCES_splint= \
	main.c_splint \
	policy.c_splint \
	printconf.c_splint \
	session.c_splint \
	log.c_splint


flawfinder_FLAGS=--minlevel=4 --allowlink --context
splint_FLAGS=-Du_int64_t='unsigned long long' -Du_int32_t='unsigned int' \
	-D__signed__=signed +unixlib -imptype $(INCLUDES)
lint_FLAGS=$(INCLUDES)

linting: flawfinder splint lint

flawfinder:	$(anoubisd_SOURCES_flawfinder)
splint:		$(anoubisd_SOURCES_splint)
lint:		$(anoubisd_SOURCES_lint)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(anoubisd_SOURCES_lint)
	@rm -f $(anoubisd_SOURCES_splint)
	@rm -f $(anoubisd_SOURCES_flawfinder)

# As we're in maintainer mode we have to remove generated source files
# manually.
dist-hook:
	$(MAKE) maintainer-clean-generic
