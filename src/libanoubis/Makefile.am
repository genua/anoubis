##########################################################################
# Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

lib_LIBRARIES = libanoubis.a
noinst_DATA = potfiles.frag

CLEANFILES = $(noinst_DATA)

include $(top_srcdir)/src/Makefile.include

AM_CFLAGS += -std=gnu99
shared_AM_CFLAGS += -std=gnu99

built_sources = \
	parse.c \
	parse_v042.c \
	apncvslog.c \
	date.c \
	apnmd.c\
	csum_import.c \
	fname.c

built_headers = \
	parse.h \
	parse_v042.h \
	csum_import.h

BUILT_SOURCES = $(built_sources) $(built_headers)

AM_YFLAGS = -d
AM_LFLAGS = -olex.yy.c

headers = \
	include/accbuffer.h \
	include/accutils.h \
	include/anoubis_protocol_auth.h \
	include/anoubis_chat.h \
	include/anoubis_client.h \
	include/anoubis_crc32.h \
	include/anoubis_dump.h \
	include/anoubis_errno.h \
	include/anoubis_msg.h \
	include/anoubis_notify.h \
	include/anoubis_policy.h \
	include/anoubis_protocol.h \
	include/anoubis_rbtree.h \
	include/anoubis_server.h \
	include/anoubis_sig.h \
	include/anoubis_transaction.h \
	include/anoubis_apnvm_cmd.h \
	include/anoubis_apnvm_cvs.h \
	include/anoubis_apnvm_md.h \
	include/anoubis_apnvm.h \
	include/anoubis_apnvm_util.h \
	include/anoubis_auth.h \
	include/anoubis_csum.h \
	include/anoubis_csumtoken.h \
	include/anoubis_keygen.h \
	include/anoubis_playground.h \
	include/apn.h \
	include/apninternals.h

libanoubis_a_CPPFLAGS = $(anoubis_cppflags)

csources = \
	anoubischat/accbuffer.c \
	anoubischat/accconnect.c \
	anoubischat/accsetup.c \
	anoubischat/acctransmission.c \
	anoubisprotocol/anoubis_auth.c \
	anoubisprotocol/anoubis_dump.c \
	anoubisprotocol/anoubis_msg.c \
	anoubisprotocol/anoubis_msg_verify.c \
	anoubisprotocol/client/anoubis_client.c \
	anoubisprotocol/client/anoubis_transaction.c \
	anoubisprotocol/server/anoubis_notify.c \
	anoubisprotocol/server/anoubis_policy.c \
	anoubisprotocol/server/anoubis_server.c \
	anoubissig/anoubis_sig.c \
	anoubiserrorcode/anoubis_errno.c \
	anoubisui/apnvm/apncmd.c \
	anoubisui/apnvm/apncvs.c \
	anoubisui/apnvm/apnvm.c \
	anoubisui/apnvm/apnvmutil.c \
	anoubisui/auth/auth.c \
	anoubisui/csum/csum.c \
	anoubisui/key/keygen.c \
	anoubisui/playground/playground.c \
	apn/apnescalations.c \
	apn/apnparser.c \
	apn/rbtree.c

ylsources = \
	anoubisui/apnvm/date.y \
	anoubisui/apnvm/apncvslog.l \
	anoubisui/apnvm/apnmd.l \
	anoubisui/csum/csum_import.y \
	anoubisui/csum/fname.l \
	apn/parse.y \
	apn/parse_v042.y

if NEEDBSDCOMPAT
headers += \
	include/bsdcompat/bsdcompat.h \
	include/bsdcompat/sys/queue.h \
	include/bsdcompat/sys/tree.h
csources += \
	bsdcompat/getpeereid.c \
	bsdcompat/strtonum.c \
	bsdcompat/strlcat.c \
	bsdcompat/strlcpy.c \
	bsdcompat/inet_net_pton.c \
	bsdcompat/setproctitle.c
endif

libanoubis_a_SOURCES = $(csources) $(ylsources) $(headers)

sources_lint   = \
	$(csources:%=%_lint)
sources_splint = \
	$(csources:%=%_splint) \
	$(built_sources:%=%_splint) \
	$(headers:%=%_splint) \
	$(built_headers:%=%_splint)
sources_flawfinder = \
	$(csources:%=%_flawfinder) \
	$(built_sources:%=%_flawfinder) \
	$(headers:%=%_flawfinder) \
	$(built_headers:%=%_flawfinder)

flawfinder_FLAGS=
splint_FLAGS=$(libanoubis_a_CPPFLAGS)
lint_FLAGS=$(libanoubis_a_CPPFLAGS)

# For testing with aegis (especially against the baseline) we need to run
# the test-binary (containing the current tests), operating on the old/baseline
# library. Thus we build a dynamic link-able library for testing purpose only.
noinst_PROGRAMS = libanoubis.so
libanoubis_so_SOURCES = $(libanoubis_a_SOURCES)
libanoubis_so_CFLAGS = $(shared_AM_CFLAGS)
libanoubis_so_CPPFLAGS = $(libanoubis_a_CPPFLAGS)
libanoubis_so_LDFLAGS = $(shared_AM_LDFLAGS) -lcrypto
libanoubis_so_LDADD = @LIBINTL@

potfiles.frag:
	echo "src/libanoubis/anoubiserrorcode/anoubis_errno.c" \
	    > potfiles.frag

include $(top_srcdir)/lint/Makefile.lint
