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

lib_LIBRARIES = libanoubisps.a libanoubispc.a
include_HEADERS = \
	anoubisprotocol.h \
	anoubisprotocolserver.h \
	anoubisprotocolclient.h

anoubisprotocoldir	= src/libanoubisprotocol
anoubischatdir		= src/libanoubischat
libanoubischat		= $(top_builddir)/$(anoubischatdir)/libanoubischat.a

libanoubisps_a_CPPFLAGS = \
	-I$(top_srcdir)/$(anoubisprotocoldir)/server \
	-I$(top_builddir)/$(anoubischatdir)

libanoubisps_a_SOURCES = \
	server/apssetup.c \
	server/apscore.c \
	$(include_HEADERS)

libanoubispc_a_CPPFLAGS = \
	-I$(top_srcdir)/$(anoubisprotocoldir)/client \
	-I$(top_builddir)/$(anoubischatdir)

libanoubispc_a_SOURCES = \
	client/apcsetup.c \
	client/apccore.c \
	$(include_HEADERS)
#
# For testing with aegis (especially against the baseline) we need to run
# the test-binary (containing the current tests), operating on the old/baseline
# library. Thus we build a dynamic link-able library for testing purpose only.
noinst_PROGRAMS = libanoubisps.so libanoubispc.so

libanoubisps_so_SOURCES = $(libanoubisps_a_SOURCES)
libanoubisps_so_CFLAGS = -fPIC -shared
libanoubisps_so_LDFLAGS = -nostartfiles -shared

libanoubispc_so_SOURCES = $(libanoubispc_a_SOURCES)
libanoubispc_so_CFLAGS = -fPIC -shared
libanoubispc_so_LDFLAGS = -nostartfiles -shared


libanoubisps_a_SOURCES_flawfinder = \
	server/apssetup.c_flawfinder \
	server/apscore.c_flawfinder \
	anoubisprotocolserver.h_flawfinder \
	anoubisprotocol.h_flawfinder

libanoubispc_a_SOURCES_flawfinder = \
	client/apcsetup.c_flawfinder \
	client/apccore.c_flawfinder \
	anoubisprotocolclient.h_flawfinder \
	anoubisprotocol.h_flawfinder

libanoubisps_a_SOURCES_lint = \
	server/apssetup.c_lint \
	server/apscore.c_lint \
	anoubisprotocolserver.h_lint \
	anoubisprotocol.h_lint

libanoubispc_a_SOURCES_lint = \
	client/apcsetup.c_lint \
	client/apccore.c_lint \
	anoubisprotocolclient.h_lint \
	anoubisprotocol.h_lint

libanoubisps_a_SOURCES_splint = \
	server/apssetup.c_splint \
	server/apscore.c_splint \
	anoubisprotocolserver.h_splint \
	anoubisprotocol.h_splint

libanoubispc_a_SOURCES_splint = \
	client/apcsetup.c_splint \
	client/apccore.c_splint \
	anoubisprotocolclient.h_splint \
	anoubisprotocol.h_splint

flawfinder_FLAGS=--minlevel=4 --allowlink --context
splint_FLAGS=+posixlib
lint_FLAGS=""

lint: $(libanoubisps_a_SOURCES_lint) $(libanoubispc_a_SOURCES_lint)
splint: $(libanoubisps_a_SOURCES_splint) $(libanoubispc_a_SOURCES_splint)
flawfinder: $(libanoubisps_a_SOURCES_flawfinder) \
	$(libanoubispc_a_SOURCES_flawfinder)

# in some cases, flawfinder terminates with internal errors on openbsd.
# Don't fail the build for now in this case.
.c.c_flawfinder .h.h_flawfinder:
	-flawfinder $(flawfinder_FLAGS) $< > $@

.SUFFIXES= .lint

.c.c_lint .h.h_lint:
	@if expr $(build_os) : "openbsd" >/dev/null; then \
		lint $< > $@; \
		echo [lint] $<; \
	else \
		touch $@; \
	fi

# splint returns an bad exitcode on every source code hit. We need to
# ignore these exits for a successful build
.c.c_splint .h.h_splint:
	@echo splint stub
	-splint $(splint_FLAGS) $< > $@

clean-local:
	@rm -f $(libanoubisps_a_SOURCES_lint) $(libanoubispc_a_SOURCES_lint)
	@rm -f $(libanoubisps_a_SOURCES_splint) $(libanoubispc_a_SOURCES_splint)
	@rm -f $(libanoubisps_a_SOURCES_flawfinder) \
		$(libanoubispc_a_SOURCES_flawfinder)
