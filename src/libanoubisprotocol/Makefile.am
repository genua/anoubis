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

bsdcompatdir = $(top_srcdir)/src/bsdcompat
libbsdcompat = $(top_builddir)/src/bsdcompat/libbsdcompat.a

lib_LIBRARIES = libanoubisps.a libanoubispc.a
include_HEADERS_common =		\
	anoubis_auth.h			\
	anoubis_dump.h			\
	anoubis_msg.h			\
	anoubis_errno.h			\
	anoubis_protocol.h		\
	crc32.h

include_HEADERS_server =		\
	server/anoubis_server.h		\
	server/anoubis_notify.h		\
	server/anoubis_policy.h
include_HEADERS_client =		\
	client/anoubis_client.h		\
	client/anoubis_transaction.h

include_HEADERS =			\
	$(include_HEADERS_common	\
	$(include_HEADERS_server)	\
	$(include_HEADERS_client)


anoubisprotocoldir	= src/libanoubisprotocol
anoubischatdir		= src/libanoubischat
apndir			= src/libapn

libanoubisps_a_CPPFLAGS = \
	-I$(top_srcdir)/$(anoubisprotocoldir) \
	-I$(top_srcdir)/$(anoubisprotocoldir)/server \
	-I$(top_srcdir)/$(anoubischatdir) \
	-I$(top_srcdir)/$(apndir) \
	-I$(top_srcdir)/include
libanoubisps_so_CPPFLAGS = $(libanoubisps_a_CPPFLAGS)
libanoubisps_so_LDFLAGS	=
libanoubispc_so_LDFLAGS	=
libanoubisps_so_LDADD =
libanoubisps_so_DEPENDENCIES =
if NEEDBSDCOMPAT
libanoubisps_a_CPPFLAGS  += -I$(bsdcompatdir)
libanoubisps_so_CPPFLAGS += -I$(bsdcompatdir)
libanoubisps_so_LDFLAGS	 += -L$(top_builddir)/src/bsdcompat
libanoubisps_a_LIBADD	 = $(libbsdcompat)
libanoubisps_so_LDADD	 += -lbsdcompat
libanoubisps_a_DEPENDENCIES = $(libbsdcompat)
libanoubisps_so_DEPENDENCIES += $(libbsdcompat)
endif
libanoubisps_so_LDADD += -lapn
libanoubisps_so_DEPENDENCIES += $(libapn)
libanoubisps_so_LDFLAGS += -L$(top_builddir)/src/libapn

libanoubisps_a_SOURCES =		\
	server/anoubis_server.c		\
	server/anoubis_notify.c		\
	server/anoubis_policy.c		\
	anoubis_msg.c			\
	anoubis_dump.c			\
	anoubis_auth.c			\
	anoubis_msg_verify.c		\
	$(include_HEADERS_common)	\
	$(include_HEADERS_server)

libanoubispc_a_CPPFLAGS = \
	-I$(top_srcdir)/$(anoubisprotocoldir) \
	-I$(top_srcdir)/$(anoubisprotocoldir)/client \
	-I$(top_srcdir)/$(anoubischatdir) \
	-I$(top_srcdir)/include
libanoubispc_so_CPPFLAGS = $(libanoubispc_a_CPPFLAGS)
if NEEDBSDCOMPAT
libanoubispc_a_CPPFLAGS  += -I$(bsdcompatdir)
libanoubispc_so_CPPFLAGS += -I$(bsdcompatdir)
libanoubispc_so_LDFLAGS	+= -L$(top_builddir)/src/bsdcompat
libanoubispc_a_LIBADD	= $(libbsdcompat)
libanoubispc_so_LDADD	= -lbsdcompat
libanoubispc_a_DEPENDENCIES = $(libbsdcompat)
libanoubispc_so_DEPENDENCIES = $(libbsdcompat)
endif

libanoubispc_a_SOURCES = \
	client/anoubis_client.c		\
	client/anoubis_transaction.c	\
	anoubis_msg.c			\
	anoubis_msg_verify.c		\
	anoubis_dump.c			\
	anoubis_auth.c			\
	$(include_HEADERS_common)	\
	$(include_HEADERS_client)

# For testing with aegis (especially against the baseline) we need to run
# the test-binary (containing the current tests), operating on the old/baseline
# library. Thus we build a dynamic link-able library for testing purpose only.
noinst_PROGRAMS = libanoubisps.so libanoubispc.so

libanoubisps_so_SOURCES = $(libanoubisps_a_SOURCES)
libanoubisps_so_CFLAGS = -fPIC -shared
libanoubisps_so_LDFLAGS += -nostartfiles -shared	\
	-L$(top_builddir)/$(anoubischatdir) -lanoubischat

libanoubispc_so_SOURCES = $(libanoubispc_a_SOURCES)
libanoubispc_so_CFLAGS = -fPIC -shared
libanoubispc_so_LDFLAGS += -nostartfiles -shared	\
	-L$(top_builddir)/$(anoubischatdir) -lanoubischat \
	-L$(top_builddir)/$(apndir) -lapn

libanoubisps_a_SOURCES_flawfinder =		\
	server/anoubis_server.c_flawfinder	\
	server/anoubis_notify.c_flawfinder	\
	server/anoubis_policy.c_flawfinder	\
	anoubis_msg.c_flawfinder		\
	anoubis_msg_verify.c_flawfinder		\
	anoubis_dump.c_flawfinder		\
	anoubis_auth.c_flawfinder		\
	server/anoubis_server.h_flawfinder	\
	server/anoubis_notify.h_flawfinder	\
	server/anoubis_policy.h_flawfinder	\
	anoubis_msg.h_flawfinder		\
	anoubis_dump.h_flawfinder		\
	anoubis_auth.h_flawfinder		\
	anoubis_protocol.h_flawfinder		\
	anoubis_errno.h_flawfinder		\
	crc32.h_flawfinder

libanoubispc_a_SOURCES_flawfinder =		\
	client/anoubis_client.c_flawfinder	\
	client/anoubis_transaction.c_flawfinder	\
	anoubis_msg.c_flawfinder		\
	anoubis_msg_verify.c_flawfinder		\
	client/anoubis_client.h_flawfinder	\
	client/anoubis_transaction.h_flawfinder

libanoubisps_a_SOURCES_lint =		\
	server/anoubis_server.c_lint	\
	server/anoubis_notify.c_lint	\
	server/anoubis_policy.c_lint	\
	anoubis_msg.c_lint		\
	anoubis_msg_verify.c_lint	\
	anoubis_dump.c_lint		\
	anoubis_auth.c_lint

libanoubispc_a_SOURCES_lint =			\
	client/anoubis_client.c_lint		\
	client/anoubis_transaction.c_lint

libanoubisps_a_SOURCES_splint =		\
	server/anoubis_server.c_splint	\
	server/anoubis_notify.c_splint	\
	server/anoubis_policy.c_splint	\
	anoubis_msg.c_splint		\
	anoubis_msg_verify.c_splint	\
	anoubis_dump.c_splint		\
	anoubis_auth.c_splint		\
	server/anoubis_server.h_splint	\
	server/anoubis_notify.h_splint	\
	server/anoubis_policy.h_splint	\
	anoubis_msg.h_splint		\
	anoubis_dump.h_splint		\
	anoubis_auth.h_splint		\
	anoubis_protocol.h_splint	\
	anoubis_errno.h_splint		\
	crc32.h_splint

libanoubispc_a_SOURCES_splint =			\
	client/anoubis_client.c_splint		\
	client/anoubis_transaction.c_splint	\
	client/anoubis_client.h_splint		\
	client/anoubis_transaction.h_splint

flawfinder_FLAGS=
splint_FLAGS=$(libanoubispc_a_CPPFLAGS) $(libanoubisps_a_CPPFLAGS) -I..
lint_FLAGS=$(libanoubispc_a_CPPFLAGS) $(libanoubisps_a_CPPFLAGS) -I..

mksubdirs:
	mkdir -p client server
.PHONY: mksubdirs

lint: mksubdirs $(libanoubisps_a_SOURCES_lint)	\
	$(libanoubispc_a_SOURCES_lint)

splint: mksubdirs $(libanoubisps_a_SOURCES_splint)\
	$(libanoubispc_a_SOURCES_splint)

flawfinder: mksubdirs $(libanoubisps_a_SOURCES_flawfinder)\
	$(libanoubispc_a_SOURCES_flawfinder)

include $(top_srcdir)/lint/Makefile.lint

clean-local:
	@rm -f $(libanoubisps_a_SOURCES_lint) $(libanoubispc_a_SOURCES_lint)
	@rm -f $(libanoubisps_a_SOURCES_splint) $(libanoubispc_a_SOURCES_splint)
	@rm -f $(libanoubisps_a_SOURCES_flawfinder) \
		$(libanoubispc_a_SOURCES_flawfinder)
