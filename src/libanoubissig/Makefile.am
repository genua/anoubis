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


lib_LIBRARIES = libanoubissig.a
libapndir	= src/libapn

LIBS += $(SSL_LIBS)

INCLUDES = \
	   -I$(top_builddir)/src	\
	   -I$(top_srcdir)/include	\
	   -I$(top_srcdir)/$(libapndir) \
	   -I$(top_srcdir)/$(bsdcompatdir)

# Make auto-tooling happy and yourself!
libanoubissig_a_SOURCES = anoubis_sig.c anoubis_sig.h
libanoubissig_a_SOURCES_lint = anoubis_sig.c_lint anoubis_sig.h_lint
libanoubissig_a_SOURCES_splint = anoubis_sig.c_splint anoubis_sig.h_splint
libanoubissig_a_SOURCES_flawfinder = anoubis_sig.c_flawfinder \
				     anoubis_sig.h_flawfinder

flawfinder_FLAGS=
splint_FLAGS=$(INCLUDES)
lint_FLAGS=$(INCLUDES)

lint: $(libanoubissig_a_SOURCES_lint)
splint: $(libanoubissig_a_SOURCES_splint)
flawfinder: $(libanoubissig_a_SOURCES_flawfinder)

linting: lint splint flawfinder

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(libanoubissig_a_SOURCES_lint)
	@rm -f $(libanoubissig_a_SOURCES_flawfinder)
	@rm -f $(libanoubissig_a_SOURCES_splint)
