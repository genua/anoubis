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

LINUX_HEADER = linux/anoubis.h linux/eventdev.h linux/anoubis_sfs.h \
    linux/anoubis_alf.h linux/anoubis_playground.h
OBSD_HEADER = dev/anoubis.h dev/eventdev.h sys/anoubis_sfs.h sys/anoubis_alf.h
ALL_HEADER = $(LINUX_HEADER) $(OBSD_HEADER)

dist-hook:
	for dir in @KERNELHEADERS@; do \
		for hdr in $(ALL_HEADER); do \
			if [ -r $$dir/$$hdr ]; then \
				df=$(distdir)/$$hdr; \
				dd=`dirname $$df`; \
				[ ! -d $$dd ] && mkdir -p $$dd; \
				cp -fp $$dir/$$hdr $$dd; \
			fi \
		done \
	done
