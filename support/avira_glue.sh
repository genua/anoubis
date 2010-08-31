#!/bin/sh

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

function leave {
	rm -f $TMPFILE
}

trap leave EXIT

if [ -z "$HOME" ]; then
	echo 'ERROR: $HOME is not set'
	exit 1
fi

if [ ! -d "$HOME" ]; then
	echo "ERROR: Home-directory $HOME does not exist"
	exit 1
fi

AVSCAN=$(which avscan)
if [ $? -ne 0 ]; then
	echo "ERROR: Could not find avscan-binary"
	exit 1
fi

TMPFILE=$(mktemp -p $HOME)
if [ $? -ne 0 ]; then
	echo "ERROR: Could not create temporary file"
	exit 1
fi

# Copy content from stdin to temporary file
cat <&0 > $TMPFILE

# Scan temp. file
out=$($AVSCAN --scan-mode=all --alert-action=ignore --batch $TMPFILE 2>&1)
rc=$?

if [ $rc -eq 0 ]; then
	# Success
	exit 0
fi

# Print basic error
case $rc in
	1) echo "ERROR: Found concerning file";;
	3) echo "ERROR: Suspicious file found";;
	4) echo "ERROR: Warnings were issued";;
	255) echo "ERROR: Internal error";;
	254) echo "ERROR: Configuration error";;
	253) echo "ERROR: Error while preparing on-demand scan";;
	252) echo "ERROR: The avguard daemon is not running";;
	251) echo "ERROR: The avguard daemon is not accessible";;
	250) echo "ERROR: Cannot initialize scan process";;
	249) echo "ERROR: Scan process not completed";;
	*) echo "ERROR: Unexpected return-code: $rc";;
esac

# Extract ALERT messages, this might be some useful information
IFS=\n
echo $out | grep ALERT | sed -e "s/^\s*//"

# Exit with non-zero
exit $rc
