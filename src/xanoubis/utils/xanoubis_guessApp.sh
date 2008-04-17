#!/bin/sh

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

set -eu

# first of all check for non-standard characters within all parameters
for arg do
	# strip all non-standard characters
	# XXX: this needs bash, how to do it in POSIX?
	tmp=${arg//[^a-zA-Z0-9_\-\.\/,]/}

	# abort if length is different than before
	if [ ${#arg} != ${#tmp} ] ; then
		echo "Invalid character within parameter!" >&2
		exit 1
	fi
done

usage() {
	cat >&2 << EOF
usage: $0 [-b <binary>] [-dh] [-s <path>] [pattern]
	-b	searches for a specified binary (field Exec)
	-h	display help
	-d	enable debug messages
	-s	add an additional directory to search in

	This script searches for Desktop Entry files (*.desktop), specified
	at www.freedesktop.org/wiki/Specifications/desktop-entry-spec.
	Each found file is inspected for containing the given pattern.
	The packaging system is asked for further information also.
	Finally a single line (fields seperated by #) is printed for
	each matching file.
	If the pattern is omitted, a line for each found desktop entry file
	will been printed.
	The following directories will been inspected:
	  - /usr/share/applications/
	  - $HOME/.kde*
	  - $HOME/.gnome*
	  - $HOME/Desktop/
EOF
}

function debug () {
	if $DEBUG; then
		echo "DEBUG: $*" >&2
	fi;
}

set +e
PARAMS=`getopt b:dhs: "$@"`
if test $? -ne 0; then usage; exit 1; fi;
set -e

eval set -- "$PARAMS"

DEBUG=false
search_paths=""
search_binary=""

while test $# -gt 0
do
	case $1 in
		-b)
			search_binary="$2";
			shift;
			;;
		-h)
			usage;
			exit 1;
			;;
		-d)
			DEBUG=true
			;;
		-s)
			search_paths+="$2 ";
			shift;
			;;
		--)
			shift;
			break;
			;;
		*)
			echo "Internal error!";
			exit 1;
			;;
	esac
	shift
done

search_paths+="/usr/share/applications/ "
search_paths+="$HOME/.kde* "
search_paths+="$HOME/.gnome* "
search_paths+="$HOME/Desktop/ "
search_pattern=${*:-".*"}

debug "Searching in: [$search_paths]"
if [ -n "$search_binary" ]; then
	search_binary=`basename $search_binary`;
	debug "Searching for binary: [$search_binary]"
else
	debug "Searching for pattern: [$search_pattern]"
fi;

find $search_paths -name \*.desktop -type f -print0 2>/dev/null \
    | xargs -0 -L 1 awk -F= \
    -v pattern="$search_pattern" -v binary="$search_binary" '
# only inspect non-comment lines
$0 !~ /^( |\t)*#/ {
	field[$1] = $2;
	if ($0 ~ "Type=Application") hasType = 1;
	if (field["Exec"] != "") {
		# chop arguments
		split(field["Exec"], A, " ");
		field["Exec"] = A[1];
		hasExec = 1;
	}
	if (binary != "") {
		if (field["Exec"] ~ "^" binary ) matchesPattern = 1;
	} else {
		if ($2 ~ pattern ) matchesPattern = 1;
	}
}
END {
	FS = ":";

	if (matchesPattern && hasType && hasExec) {
		cmdAbsPath = "which " field["Exec"];
		cmdAbsPath | getline absPath;

		cmdPkgName = "dpkg -S " absPath " 2>/dev/null";
		cmdPkgName | getline;
		pkgName = $1;

		cmdPkgDescription = "dpkg -s " pkgName " 2>/dev/null";
		if (pkgName != "##") {
			do {
				cmdPkgDescription | getline;
			} while ($1 !~ "Description");
			pkgDescription = $2;
		} else {
			pkgName = "(no package/name available)";
			pkgDescription = "(no package/description available)";
		}

		OFS = " # ";
		print field["Name"], absPath, pkgName, pkgDescription;

		exit 0;
	}
} '
