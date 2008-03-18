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

lib_LIBRARIES = libanoubischat.a

include_HEADERS = anoubischat.h

bsdcompatdir = src/bsdcompat

libanoubischat_a_SOURCES = \
	accsetup.c  \
	acctransmission.c \
	accconnect.c \
	accutils.c \
	accutils.h \
	accbuffer.c \
	accbuffer.h \
	$(include_HEADERS)

INCLUDES = \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/include

if NEEDBSDCOMPAT
INCLUDES += -I$(top_srcdir)/$(bsdcompatdir)
endif

# For testing with aegis (especially against the baseline) we need to run
# the test-binary (containing the current tests), operating on the old/baseline
# library. Thus we build a dynamic link-able library for testing purpose only.
noinst_PROGRAMS = libanoubischat.so
libanoubischat_so_SOURCES = $(libanoubischat_a_SOURCES)
libanoubischat_so_CFLAGS = -fPIC -shared $(shared_AM_CFLAGS)
libanoubischat_so_LDFLAGS = -nostartfiles -shared $(shared_AM_LDFLAGS)

if NEEDBSDCOMPAT
LDADD = -lbsdcompat
libanoubischat_so_LDFLAGS += -L$(top_builddir)/$(bsdcompatdir)
endif

libanoubischat_a_SOURCES_lint = \
	accsetup.c_lint  \
	acctransmission.c_lint \
	accconnect.c_lint \
	accutils.c_lint \
	accbuffer.c_lint

libanoubischat_a_SOURCES_splint = \
	accsetup.c_splint  \
	acctransmission.c_splint \
	accconnect.c_splint \
	accutils.c_splint \
	accutils.h_splint \
	accbuffer.c_splint \
	accbuffer.h_splint

libanoubischat_a_SOURCES_flawfinder = \
	accsetup.c_flawfinder  \
	acctransmission.c_flawfinder \
	accconnect.c_flawfinder \
	accutils.c_flawfinder \
	accutils.h_flawfinder \
	accbuffer.c_flawfinder \
	accbuffer.h_flawfinder

flawfinder_FLAGS=
splint_FLAGS=$(INCLUDES)
lint_FLAGS=$(INCLUDES)

lint:		$(libanoubischat_a_SOURCES_lint)
splint:		$(libanoubischat_a_SOURCES_splint)
flawfinder:	$(libanoubischat_a_SOURCES_flawfinder)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(libanoubischat_a_SOURCES_lint)
	@rm -f $(libanoubischat_a_SOURCES_splint)
	@rm -f $(libanoubischat_a_SOURCES_flawfinder)
