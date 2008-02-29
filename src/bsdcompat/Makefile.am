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

# We build the bsdcompat library in two flavors: One suitable for
# executables, and one suitable for including in shared objects.

lib_LIBRARIES = libbsdcompat.a libbsdcompat_pic.a
include_HEADERS = bsdcompat.h queue.h

# for LLONG_MIN/LLONG_MAX
AM_CFLAGS += -std=gnu99

libbsdcompat_a_SOURCES = $(include_HEADERS)
libbsdcompat_a_LIBADD = $(LIBOBJS)

libbsdcompat_pic_a_SOURCES = $(libbsdcompat_a_SOURCES)
libbsdcompat_pic_a_LIBADD  = $(libbsdcompat_a_LIBADD)
libbsdcompat_pic_a_CFLAGS = -fPIC -std=gnu99 $(shared_AM_CFLAGS)

# no linting is done here, as we are not developing new source here. The
# functions here are rather source imports from OpenBSD.

splint:
flawfinder:
lint:
