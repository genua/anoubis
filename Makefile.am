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

SUBDIRS = src test include lint

# the following is needed to add man pages to the dist file
man_MANS = man/anoubisd.8 man/xanoubis.1 man/anoubischat.3 man/anoubisctl.8 \
	man/apn.3 man/anoubis_msg.3 man/anoubis_transaction.3 \
	man/anoubis_client.3 man/anoubis_server.3 man/anoubis_auth.3 \
	man/anoubis_notify.3

EXTRA_DIST = $(man_MANS) @WXFB_PROJECT_FILES@ autogen.sh

MAINTAINERCLEANFILES= \
	aclocal.m4 \
	Makefile.in\
	configure \
	config.log \
	config.guess \
	config.sub

flawfinder:
	for t in $(SUBDIRS); do cd $$t && make $@ && cd ..; done

lint:
	for t in $(SUBDIRS); do cd $$t && make $@ && cd ..; done

splint:
	for t in $(SUBDIRS); do cd $$t && make $@ && cd ..; done

linting: flawfinder lint splint

.PHONY: flawfinder lint splint
