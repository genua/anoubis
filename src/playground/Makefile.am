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

bin_PROGRAMS = playground

LIBS += $(SSL_LIBS)

if STATIC_LINKING
STATIC = -static
endif

include $(top_srcdir)/src/Makefile.include

INCLUDES = $(anoubis_cppflags)

playground_DEPENDENCIES = $(libanoubis)

playground_LDADD = $(playground_DEPENDENCIES) $(intl_ldadd)

playground_LDFLAGS= $(AM_LDFLAGS) $(STATIC)

sources= main.c
headers = playground.h

playground_SOURCES = $(sources) $(headers)

sources_lint   = \
	$(sources:%=%_lint)
sources_splint = \
	$(sources:%=%_splint) \
	$(headers:%=%_splint)
sources_flawfinder = \
	$(sources:%=%_flawfinder) \
	$(headers:%=%_flawfinder)

flawfinder_FLAGS=
# XXX tartler: the playground source needs annotations to
#     enable splint memory checking effektively. We disable
#     memory checking for part of anoubis until proper annotations
#     have been added.
splint_FLAGS=$(INCLUDES) -memchecks
lint_FLAGS=$(INCLUDES)

include $(top_srcdir)/lint/Makefile.lint
