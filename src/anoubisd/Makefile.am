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

bsdcompatdir = src/bsdcompat
libbsdcompat = $(top_builddir)/$(bsdcompatdir)/libbsdcompat.a
anoubischatdir = src/libanoubischat
anoubissigdir = src/libanoubissig
libanoubischat = $(top_builddir)/$(anoubischatdir)/libanoubischat.a
libanoubissig = $(top_builddir)/$(anoubissigdir)/libanoubissig.a
anoubispsdir = src/libanoubisprotocol
libanoubisps = $(top_builddir)/$(anoubispsdir)/libanoubisps.a
libapndir = src/libapn
libapn = $(top_builddir)/$(libapndir)/libapn.a


if STATIC_LINKING
STATIC = -static
endif

#CFLAGS += $(SSL_CFLAGS)
LIBS += $(SSL_LIBS)

CPPFLAGS = \
	   -I$(top_builddir)/src \
	   -I$(top_srcdir)/include \
	   -I$(top_srcdir)/src/anoubisd \
	   -I$(top_srcdir)/$(anoubischatdir) \
	   -I$(top_srcdir)/$(anoubissigdir) \
	   -I$(top_srcdir)/$(anoubispsdir) \
	   -I$(top_srcdir)/$(anoubispsdir)/server \
	   -I$(top_srcdir)/$(libapndir)

anoubisd_LDADD = $(libanoubisps) $(libanoubischat) $(libapn) $(libanoubissig)

if NEEDBSDCOMPAT
CPPFLAGS += -I$(top_srcdir)/$(bsdcompatdir)
anoubisd_LDADD    += $(libbsdcompat)
endif

anoubisd_LDFLAGS = $(AM_LDFLAGS) $(STATIC)

anoubisd_SOURCES= \
	main.c \
	policy.c \
	pe.c \
	pe_proc.c \
	pe_user.c \
	pe_context.c \
	pe_alf.c \
	pe_ipc.c \
	pe_sfs.c \
	pe_sfscache.c \
	pe_prefixhash.c \
	pe_sandbox.c \
	cert.c \
	session.c \
	log.c \
	amsg.c \
	aqueue.c \
	sfs.c \
	kernelcache.c \
	anoubisd.h \
	amsg.h \
	aqueue.h \
	sfs.h \
	pe.h \
	cert.h \
	kernelcache.h

anoubisd_SOURCES_flawfinder= \
	main.c_flawfinder \
	policy.c_flawfinder \
	pe.c_flawfinder \
	pe_proc.c_flawfinder \
	pe_user.c_flawfinder \
	pe_context.c_flawfinder \
	pe_alf.c_flawfinder \
	pe_sfs.c_flawfinder \
	pe_sfscache.c_flawfinder \
	pe_prefixhash.c_flawfinder \
	pe_sandbox.c_flawfinder \
	cert.c_flawfinder \
	session.c_flawfinder \
	log.c_flawfinder \
	amsg.c_flawfinder \
	aqueue.c_flawfinder \
	sfs.c_flawfinder \
	kernelcache.c_flawfinder \
	anoubisd.h_flawfinder \
	amsg.h_flawfinder \
	aqueue.h_flawfinder \
	sfs.h_flawfinder \
	cert.h_flawfinder \
	kernelcache.h_flawfinder

anoubisd_SOURCES_lint= \
	main.c_lint \
	policy.c_lint \
	pe.c_lint \
	pe_proc.c_lint \
	pe_user.c_lint \
	pe_context.c_lint \
	pe_alf.c_lint \
	pe_sfs.c_lint \
	pe_sfscache.c_lint \
	pe_prefixhash.c_lint \
	pe_sandbox.c_lint \
	cert.c_lint \
	session.c_lint \
	log.c_lint \
	sfs.c_lint \
	kernelcache.c_lint

anoubisd_SOURCES_splint= \
	main.c_splint \
	policy.c_splint \
	pe.c_splint \
	pe_proc.c_splint \
	pe_user.c_splint \
	pe_context.c_splint \
	pe_alf.c_splint \
	pe_sfs.c_splint \
	pe_sfscache.c_splint \
	pe_prefixhash.c_splint \
	pe_sandbox.c_splint \
	cert.c_splint \
	session.c_splint \
	log.c_splint \
	amsg.c_splint \
	aqueue.c_splint \
	sfs.c_splint \
	kernelcache.c_splint


flawfinder_FLAGS=
# XXX tartler: -compdestoy, because it causes too many false positives.
# Will be revisited in a later change
splint_FLAGS=$(CPPFLAGS) -compdestroy
lint_FLAGS=$(CPPFLAGS)

flawfinder:	$(anoubisd_SOURCES_flawfinder)
splint:		$(anoubisd_SOURCES_splint)
lint:		$(anoubisd_SOURCES_lint)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(anoubisd_SOURCES_lint)
	@rm -f $(anoubisd_SOURCES_splint)
	@rm -f $(anoubisd_SOURCES_flawfinder)
