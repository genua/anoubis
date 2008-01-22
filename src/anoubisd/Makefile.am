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

sbin_PROGRAMS = anoubisd

if STATIC_LINKING
AM_LDFLAGS = -static
endif

INCLUDES = -I$(top_srcdir)/include

anoubisd_SOURCES= \
	main.c \
	policy.c \
	printconf.c \
	session.c \
	anoubisd.h \
	anoubis_alf.h

anoubisd_SOURCES_flawfinder= \
	main.c_flawfinder \
	policy.c_flawfinder \
	printconf.c_flawfinder \
	session.c_flawfinder \
	anoubisd.h_flawfinder \
	anoubis_alf.h_flawfinder

anoubisd_SOURCES_lint= \
	main.c_lint \
	policy.c_lint \
	printconf.c_lint \
	session.c_lint

anoubisd_SOURCES_splint= \
	main.c_splint \
	policy.c_splint \
	printconf.c_splint \
	session.c_splint

lint: $(anoubisd_SOURCES_lint) $(anoubisd_SOURCES_splint) \
	$(anoubisd_SOURCES_flawfinder)

flawfinder_FLAGS=--minlevel=4 --allowlink --context
splint_FLAGS=+posixlib
lint_FLAGS=""

flawfinder:	$(anoubisd_SOURCES_flawfinder)
splint:		$(anoubisd_SOURCES_splint)
lint:		$(anoubisd_SOURCES_lint)

.c.c_flawfinder .h.h_flawfinder:
	flawfinder $(flawfinder_FLAGS) $< > $@

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
	@rm -f $(anoubisd_SOURCES_lint)
	@rm -f $(anoubisd_SOURCES_splint)
	@rm -f $(anoubisd_SOURCES_flawfinder)
