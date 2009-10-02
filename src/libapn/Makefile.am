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

BUILT_SOURCES = parse.h parse.c parse_v042.c parse_v042.h
AM_YFLAGS = -d
libapn_a_CFLAGS = -std=gnu99 $(AM_CFLAGS)

lib_LIBRARIES = libapn.a

noinst_HEADERS = apn.h apninternals.h rbtree.h

bsdcompatdir = src/bsdcompat
libbsdcompat = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a

INCLUDES = -I $(top_builddir)/src -I $(top_srcdir)/include \
    -I $(top_srcdir)/src/libanoubisprotocol

if NEEDBSDCOMPAT
INCLUDES += -I$(top_srcdir)/$(bsdcompatdir)
endif

libapn_a_SOURCES = \
	apnparser.c  \
	apnescalations.c  \
	rbtree.c  \
	parse.y \
	parse_v042.y \
	$(noinst_HEADERS)

# For testing with aegis (especially against the baseline) we need to run
# the test-binary (containing the current tests), operating on the old/baseline
# library. Thus we build a dynamic link-able library for testing purpose only.
noinst_PROGRAMS = libapn.so
libapn_so_SOURCES = $(libapn_a_SOURCES)
libapn_so_CFLAGS = -fPIC -shared -std=gnu99 $(shared_AM_CFLAGS)
libapn_so_LDFLAGS = -nostartfiles -shared $(shared_AM_LDFLAGS)

if NEEDBSDCOMPAT
LDADD = -lbsdcompat
libapn_so_LDFLAGS += -L$(top_builddir)/$(bsdcompatdir)
endif

libapn_a_SOURCES_lint = \
	apnparser.c_lint \
	apnescalations.c_lint  \
	rbtree.c_lint

libapn_a_SOURCES_splint = \
	apnparser.c_splint  \
	apnescalations.c_splint  \
	rbtree.c_splint  \
	apn.h_splint \
	apninternals.h_splint \
	rbtree.h_splint

libapn_a_SOURCES_flawfinder = \
	apnparser.c_flawfinder  \
	apnescalations.c_flawfinder  \
	rbtree.c_flawfinder  \
	rbtree.h_flawfinder  \
	apn.h_flawfinder \
	apninternals.h_flawfinder

flawfinder_FLAGS=
# XXX tartler: the libapn source needs annotations to enable splint
#              memory checking efficiently. We disable memory checking
#              for this source file for now.
splint_FLAGS=$(INCLUDES) -memchecks
lint_FLAGS=$(INCLUDES)

lint:		$(libapn_a_SOURCES_lint)
splint:		$(libapn_a_SOURCES_splint)
flawfinder:	$(libapn_a_SOURCES_flawfinder)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(libapn_a_SOURCES_lint)
	@rm -f $(libapn_a_SOURCES_splint)
	@rm -f $(libapn_a_SOURCES_flawfinder)

# As we're in maintainer mode we have to remove generated source files
# manually.
dist-hook:
	$(MAKE) maintainer-clean-generic
