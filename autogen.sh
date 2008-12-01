#!/bin/sh
# a quick hack script to generate needed autotools files
# $Id: 2309b5,v 1.13 2008/12/01 09:47:01 fritsch Exp $

# map used commands
: ${ACLOCAL=aclocal}
: ${AUTOHEADER=autoheader}
: ${AUTOMAKE=automake}
: ${AUTOCONF=autoconf}

{
	echo "running aclocal" >&2
	if [ -n "$CHECK_DIR" ]; then
		$ACLOCAL --version
		$ACLOCAL -I m4 -I $CHECK_DIR
	else
		$ACLOCAL -I m4
	fi
} && {
	$AUTOHEADER --force
} && {
	touch AUTHORS INSTALL NEWS COPYING README ChangeLog
	echo "running automake" >&2
	$AUTOMAKE --foreign -a -c --add-missing
} && {
	echo "running autoconf" >&2
	$AUTOCONF --force
} &&
	echo "autogen complete" >&2 || {
	echo "ERROR: autogen.sh failed, autogen is incomplete" >&2
	exit 1
}

echo
echo "Ready to run configure, for instance:"
echo " ./configure --prefix=$HOME"
