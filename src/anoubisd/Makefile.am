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

m4headers = \
    apnrules_array.h \
    prefixhash_entry_array.h \
    sfs_csmulti_record_array.h

EXTRA_DIST = README.privsep anoubis_array.m4
BUILT_SOURCES = $(m4headers)
CLEANFILES = $(m4headers)

if STATIC_LINKING
STATIC = -static
endif

#CFLAGS += $(SSL_CFLAGS)
LIBS += $(SSL_LIBS)

include $(top_srcdir)/src/Makefile.include

AM_CPPFLAGS = $(anoubis_cppflags) \
	   -I$(top_srcdir)/src/anoubisd -I.

LDADD = $(libanoubis) $(intl_ldadd)
AM_LDFLAGS += $(STATIC)

lint_sources = \
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
	pe_filetree.c \
	pe_playground.c \
	cert.c \
	session.c \
	log.c \
	amsg.c \
	kernel_compat.c \
	aqueue.c \
	sfs.c \
	cfg.c \
	anoubis_alloc.c \
	upgrade.c

nolint_sources = amsg_verify.c

headers = \
	anoubisd.h \
	amsg.h \
	aqueue.h \
	sfs.h \
	pe.h \
	pe_proc_internals.h \
	cfg.h \
	cert.h \
	anoubis_alloc.h \
	pe_filetree.h \
	compat_openat.h

anoubisd_SOURCES = $(lint_sources) $(nolint_sources) $(headers)

sources_lint   = \
	$(lint_sources:%=%_lint)
sources_splint = \
	$(lint_sources:%=%_splint) \
	$(headers:%=%_splint)
sources_flawfinder = \
	$(lint_sources:%=%_flawfinder) \
	$(headers:%=%_flawfinder)

apnrules_array.h:	anoubis_array.m4 Makefile
	m4 -DTYPE="struct apn_rule *" -DPREFIX=apnarr \
	    $(srcdir)/anoubis_array.m4 >$@
prefixhash_entry_array.h:	anoubis_array.m4 Makefile
	m4 -DTYPE="struct entry *" -DPREFIX=entryarr \
	    $(srcdir)/anoubis_array.m4 >$@
sfs_csmulti_record_array.h:	anoubis_array.m4 Makefile
	m4 -DTYPE="struct sfs_csmulti_record" -DPREFIX="csmultiarr" \
	    $(srcdir)/anoubis_array.m4 >$@

flawfinder_FLAGS=
# XXX tartler: -compdestoy, because it causes too many false positives.
# Will be revisited in a later change
splint_FLAGS=$(AM_CPPFLAGS) -compdestroy
lint_FLAGS=$(AM_CPPFLAGS)

include $(top_srcdir)/lint/Makefile.lint
