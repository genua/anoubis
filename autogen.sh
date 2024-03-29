#!/bin/sh
# a quick hack script to generate needed autotools files

# map used commands
: ${ACLOCAL=aclocal}
: ${AUTOHEADER=autoheader}
: ${AUTOMAKE=automake}
: ${AUTOCONF=autoconf}

# Create a VERSION-file
if [ ! -f VERSION ]; then
	echo "0.9.4-dev" > VERSION
	date >> VERSION
fi

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
	perl split_config_h_in.pl
} && {
	touch AUTHORS INSTALL NEWS COPYING README
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
