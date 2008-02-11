dnl Copyright (c) 2007 GeNUA mbH <info@genua.de>
dnl
dnl All rights reserved.
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions
dnl are met:
dnl 1. Redistributions of source code must retain the above copyright
dnl    notice, this list of conditions and the following disclaimer.
dnl 2. Redistributions in binary form must reproduce the above copyright
dnl    notice, this list of conditions and the following disclaimer in the
dnl    documentation and/or other materials provided with the distribution.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
dnl "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
dnl LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
dnl A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
dnl OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
dnl SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
dnl TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
dnl PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
dnl LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
dnl NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
dnl SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

AC_PREREQ(2.59)
dnl XXX: the version must be generated, see BUG #163
dnl XXX: this bugzilla reference seems outdated. we should update it with a
dnl      new bug number, if the versioning style is wrong, or remove this
dnl      reference altogether

dnl
dnl XXX: this automake project must be more configurable at compilation time
dnl      see #311 for details.

dnl Define version
AC_INIT(anoubis, esyscmd([echo -n `aesub '${version}'`]), info@genua.de)

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_CONFIG_HEADER([src/config.h])

dnl checks for programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_YACC

dnl checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_PID_T

dnl needed library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([bzero])

dnl checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([stdlib.h strings.h unistd.h])
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h stdlib.h string.h sys/socket.h])

dnl checks for libraries
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
AC_CHECK_LIB(event, event_init)

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/anoubisd/Makefile
	src/libanoubischat/Makefile
	src/xanoubis/Makefile
	test/Makefile
	test/libanoubischat/Makefile
	test/xanoubis/Makefile
	include/Makefile
	include/linux/Makefile
	include/dev/Makefile
])

dnl on some systems like openbsd, anoubisd must by linked statically
AC_ARG_ENABLE(static-anoubisd,
	AC_HELP_STRING([--enable-static-anoubisd], [link anoubisd statically]),
	[case "${enableval}" in
		yes) linkanoubisdstatic=yes ;;
		no)  linkanoubisdstatic=no ;;
		*)
		    AC_MSG_ERROR(bad value ${enableval} for
				 --enable-static-anoubisd) ;;
	esac],
	[linkanoubisdstatic=no])
AM_CONDITIONAL(STATIC_LINKING, [test "x$linkanoubisdstatic" = "xyes"])

dnl determine wx-specific settings
dnl
dnl since wxwidgets does not provide a proper pkg-config interface, we have to
dnl implement the functionality of pkg-config we need ourselves.

WXCONFIG=wx-config
AC_ARG_WITH(wx-config,
	[[  --with-wx-config=FILE	path to wx-config to determine]],
[
	if test "$withval" != "yes" -a "$withval" != ""; then
		WXCONFIG=$withval
	fi
])

AC_MSG_CHECKING([wxWidgets version])
if wxversion=`$WXCONFIG --version`; then
	AC_MSG_RESULT([$wxversion])
else
	AC_MSG_RESULT([not found])
	AC_MSG_ERROR([wxWidgets is required. Try --with-wx-config.])
fi

WX_CPPFLAGS="`$WXCONFIG --cppflags`"
WX_CXXFLAGS="`$WXCONFIG --cxxflags`"
WX_LIBS="`$WXCONFIG --libs`"

CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CXXFLAGS="$CXXFLAGS $WX_CPPFLAGS"

AC_SUBST(WX_LIBS)
AC_SUBST(WX_CPPFLAGS)

dnl Configured prefix has to be propagated to wxStandardPaths::SetInstallPrefix
AS_IF([test "$prefix" = "NONE"],
	[generalprefix=$ac_default_prefix],
	[generalprefix=$prefix])
AC_DEFINE_UNQUOTED(GENERALPREFIX, ["$generalprefix"], "configured prefix")

AC_MSG_CHECKING([for number of anoubis modules])
WXFB_PROJECT_FILES=`ls -1 ${srcdir}/src/xanoubis/view/*.fbp`
ANOUBIS_MODULESNO=`echo "$WXFB_PROJECT_FILES" | $GREP -c -E "Mod.*\.fbp$"`
WXFB_PROJECT_FILES=`echo "$WXFB_PROJECT_FILES" | tr '\n' ' '`
AS_IF([test $ANOUBIS_MODULESNO -gt 0],
	[AC_MSG_RESULT([$ANOUBIS_MODULESNO])],
	[AC_MSG_ERROR([cannot find anoubis modules in \${srcdir}/src/view/])])
AC_SUBST(WXFB_PROJECT_FILES)
AC_DEFINE_UNQUOTED(ANOUBIS_MODULESNO, [$ANOUBIS_MODULESNO],
    "Number of anoubis modules")

dnl provide the variable $host, and set some defines depending on that
AC_CANONICAL_HOST
case $host in
	*-*-linux*)
	AC_DEFINE(LINUX, 1, "Define LINUX")
	AC_DEFINE(_GNU_SOURCE, 1, "Define _GNU_SOURCE")
	;;
	*-*-openbsd*)
	AC_DEFINE(OPENBSD, 1, "Define OpenBSD")
	;;
esac

AC_OUTPUT
