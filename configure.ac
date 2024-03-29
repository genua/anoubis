dnl Copyright (c) 2010 GeNUA mbH <info@genua.de>
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
dnl XXX: this automake project must be more configurable at compilation time
dnl      see #311 for details.

dnl Define version as provided by VERSION
AC_INIT(anoubis, esyscmd([echo -n `head -1 VERSION`]), info@genua.de)

AC_DEFINE(PACKAGE_BUILD, esyscmd([echo -n "\"`head -2 VERSION|tail -1`\""]), \
	"Package build number")

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_HEADERS([src/version.h])

dnl needed by later used: 'AM_GNU_GETTEXT([external])'
AC_GNU_SOURCE

dnl os specific library replacements
AC_SEARCH_LIBS([inet_net_pton], [resolv], \
	AC_DEFINE([__USE_MISC], [1], [for inet_net_pton]))
AC_CONFIG_LIBOBJ_DIR([src/libanoubis/bsdcompat])
AC_REPLACE_FUNCS([strlcat strlcpy strtonum getpeereid \
	inet_net_pton setproctitle])
AM_CONDITIONAL([NEEDBSDCOMPAT], [test -n "$LIBOBJS"])

dnl provide the variable $host, and set some defines depending on that
AC_CANONICAL_HOST
case $host in
	*-*-linux*)
	AC_DEFINE(LINUX, 1, "Define LINUX")
	AC_DEFINE(_GNU_SOURCE, 1, "Define _GNU_SOURCE")
	AM_LDFLAGS="-Wl,-z,relro $AM_LDFLAGS"
	linux=yes
	;;
	*-*-openbsd*)
	AC_DEFINE(OPENBSD, 1, "Define OpenBSD")
	obsd=yes
	;;
esac

if test -n "$LIBOBJS"; then
	AC_DEFINE(NEEDBSDCOMPAT, 1, "need a libbsdcompat.a")
fi

dnl enable large file support
AC_SYS_LARGEFILE

dnl check for presence of the stack protector
AX_CHECK_COMPILER_FLAGS(-fstack-protector,
[
	AM_CFLAGS="-fstack-protector $AM_CFLAGS"
	AM_CXXFLAGS="-fstack-protector $AM_CXXFLAGS"
])

dnl enable automatic fortification
dnl
dnl on systems that do not support this (bsd, older linux), this is
dnl effectively a noop. Since it doesn't harm, we enable it
dnl unconditionally here.

AM_CFLAGS="-D_FORTIFY_SOURCE=2 $AM_CFLAGS"
AM_CXXFLAGS="-D_FORTIFY_SOURCE=2 $AM_CXXFLAGS"


dnl checks for programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_LEX
AC_PROG_LN_S
dnl normal yacc does not know %destructor, use bison instead
AC_CHECK_PROG(YACC, bison, [bison -y])

AC_PATH_PROG(FLAWFINDER, flawfinder)
AC_PATH_PROG(SPLINT, splint)
AC_PATH_PROG(LINT, lint)

AM_CONDITIONAL(HAVE_FLAWFINDER, [test "x$FLAWFINDER" != "x"])
AM_CONDITIONAL(HAVE_SPLINT, [test "x$SPLINT" != "x"])
AM_CONDITIONAL(HAVE_LINT, [test "x$LINT" != "x"])

dnl checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_PID_T

dnl needed library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([bzero backtrace])

dnl checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([stdlib.h strings.h unistd.h])
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h stdlib.h string.h sys/socket.h])
AC_CHECK_HEADERS([sys/inotify.h])
if [ test "x$linux" = "xyes" ]; then
    AC_CHECK_HEADER([attr/xattr.h], [], AC_MSG_ERROR([xattr.h not found]))
fi

dnl checks for libraries
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
AC_CHECK_LIB(check_pic, suite_create, check_pic=yes, check_pic=no)
AC_CHECK_LIB(event, event_init)
AC_CHECK_LIB(kvm, kvm_open)
AX_CHECK_SSL
AS_IF(test -e /usr/lib/libwxGuiTestingd.a,
	[PKG_CHECK_MODULES([CPPUNIT], [cppunit >= 1.12])
	 AM_CONDITIONAL(HAVE_LIBWXGUITESTING, [true])   ],
	[AM_CONDITIONAL(HAVE_LIBWXGUITESTING, [false])  ] )

dnl checks for libnotify and its dependencies (xanoubis)
PKG_CHECK_MODULES([GTK], [gtk+-2.0 >= 2.4.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.2])
PKG_CHECK_MODULES([NOTIFY], [libnotify >= 0.4.2])
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(NOTIFY_LIBS)
AC_SUBST(NOTIFY_CFLAGS)

AC_CONFIG_FILES([
	Makefile
	contrib/Makefile
	src/Makefile
	src/anoubisd/Makefile
	src/libanoubis/Makefile
	src/xanoubis/Makefile
	src/anoubisctl/Makefile
	src/sfssig/Makefile
	src/playground/Makefile
	support/Makefile
	test/Makefile
	test/libanoubischeck/Makefile
	test/libanoubischat/Makefile
	test/libanoubisprotocol/Makefile
	test/xanoubis/Makefile
	test/libanoubissig/Makefile
	test/libanoubiserrorcode/Makefile
	test/anoubisd/Makefile
	test/bsdcompat/Makefile
	test/libapn/Makefile
	test/libanoubisui/Makefile
	test/libanoubisui/apnvm/Makefile
	test/libanoubisui/csum/Makefile
	test/libanoubisui/key/Makefile
	test/libanoubisui/auth/Makefile
	test/libanoubisui/procinfo/Makefile
	include/Makefile
	lint/Makefile
	lint/flawfinder
	lint/splint
	lint/lint
	po/Makefile.in
])

AC_CHECK_PROGS(LCOV, lcov, false)
AC_CHECK_PROGS(GENHTML, genhtml, false)


dnl KERNELHEADERS/KERNELFLAGS, preprocessor-flags to kernel-headers
KERNELFLAGS=
AC_ARG_VAR([KERNELHEADERS], [Location to Anoubis kernel headers])
if [ test -n "$KERNELHEADERS" ]; then
	for dir in $KERNELHEADERS; do
		if [ test -d "$dir" ]; then
			AC_MSG_CHECKING([$dir])
			if [ test -r "$dir/linux/anoubis.h" ]; then
				# Assume that this is a Linux-kernel
				if [ test "x$linux" = "xyes" ]; then
					KERNELFLAGS="$KERNELFLAGS -I $dir"
					AC_MSG_RESULT([include])
				else
					AC_MSG_RESULT([contrib])
				fi
			elif [ test -r "$dir/dev/anoubis.h" ]; then
				# Assume that this is an OpenBSD-kernel
				if [ test "x$obsd" = "xyes" ]; then
					KERNELFLAGS="$KERNELFLAGS -I $dir"
					AC_MSG_RESULT([include])
				else
					AC_MSG_RESULT([contrib])
				fi
			else
				AC_MSG_ERROR([Unsupported kernel: $dir])
			fi
		else
			AC_MSG_ERROR([No such directory: $dir])
		fi
	done
fi
AC_SUBST(KERNELFLAGS)

dnl conditionally enable code coverage
AC_ARG_ENABLE(gcov,
	AC_HELP_STRING([--enable-gcov], [turn on code coverage analysis tools]),
	[case "${enableval}" in
		yes) gcovenabled=yes;
		     AM_CFLAGS="$AM_CFLAGS -fprofile-arcs -ftest-coverage" ;
		     AM_CXXFLAGS="$AM_CXXFLAGS -fprofile-arcs -ftest-coverage" ;
		     AM_LDFLAGS="$AM_LDFLAGS -lgcov" ;;
		no)  gcovenabled=no ;;
		*)
		    AC_MSG_ERROR(bad value ${enableval} for
				 --enable-gcov) ;;
	esac],
	[gcovenabled=no])
AM_CONDITIONAL(GCOV_ENABLED, [test "x$gcovenabled" = "xyes"])

dnl on some systems like openbsd, anoubis-tools must be linked statically
AC_ARG_ENABLE(static,
	AC_HELP_STRING([--enable-static], [link anoubis-tools statically]),
	[case "${enableval}" in
		yes) linkstatic=yes ;;
		no)  linkstatic=no ;;
		*)
		    AC_MSG_ERROR(bad value ${enableval} for
				 --enable-static) ;;
	esac],
	[linkstatic=no])
AM_CONDITIONAL(STATIC_LINKING, [test "x$linkstatic" = "xyes"])

AC_ARG_ENABLE(pie,
	AC_HELP_STRING([--disable-pie], [disable position independent executables]),
	[ use_pie=$enableval ],
	[ use_pie=yes ])

dnl provide configure option for conditional "xanoubis"
dnl the implicit default is to enable "xanoubis"
AC_ARG_ENABLE(xanoubis,
	AC_HELP_STRING([--enable-xanoubis], [build Anoubis-GUI]),
	[case "${enableval}" in
		yes) with_xanoubis=yes ;;
		no)  with_xanoubis=no ;;
		*)
			AC_MSG_ERROR(bad value ${enableval} for
				--enable-xanoubis) ;;
	esac],
	[with_xanoubis=yes])
AM_CONDITIONAL(XANOUBIS, [test "x$with_xanoubis" = "xyes"])

dnl provide configure option for conditional "tests"
dnl the implicit default is to enable "tests"
AC_ARG_ENABLE(tests,
	AC_HELP_STRING([--enable-tests], [build tests]),
	[case "${enableval}" in
		yes) with_tests=yes ;;
		no)  with_tests=no ;;
		*)
			AC_MSG_ERROR(bad value ${enableval} for
				--enable-tests) ;;
	esac],
	[with_tests=yes])
AM_CONDITIONAL(WITH_TESTS, [test "x$with_tests" = "xyes"])

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

AC_SUBST(WX_LIBS)
AC_SUBST(WX_CPPFLAGS)
AC_SUBST(WX_CXXFLAGS)

dnl Configured prefix has to be propagated to wxStandardPaths::SetInstallPrefix
AC_DEFINE_DIR(PACKAGE_PREFIX, ["$prefix"], "configured prefix")

dnl Configured sbindir has to be propagated to anoubisctl
AC_DEFINE_DIR(PACKAGE_SBINDIR, ["$sbindir"], "configured sbindir")

dnl Configured datadir has to be propagated to anoubis-keygen
AC_DEFINE_DIR(PACKAGE_DATADIR, ["$datadir"], "configured datadir")

AC_DEFINE(PACKAGE_DAEMON, PACKAGE_NAME "d", "name of the daemon")
AC_SUBST(PACKAGE_DAEMON, "${PACKAGE_NAME}d")

AS_IF([test -n "$policy_dir"],
	[AC_DEFINE_UNQUOTED(PACKAGE_POLICYDIR, "$policy_dir",
		"location of daemon policies")],
	[AC_DEFINE(PACKAGE_POLICYDIR, "/var/lib/" PACKAGE_NAME,
		"location of daemon policies")]
)
AS_IF([test -n "$pid_dir"],
	[AC_DEFINE_UNQUOTED(PACKAGE_PIDDIR, "$pid_dir",
		"location of pid file")],
	[AC_DEFINE(PACKAGE_PIDDIR, "/var/run",
		"location of pid file")]
)
AC_DEFINE(PACKAGE_PIDFILE, PACKAGE_PIDDIR "/" PACKAGE_DAEMON ".pid",
	[daemon pidfile])
AC_DEFINE(PACKAGE_SOCKET, PACKAGE_PIDDIR "/" PACKAGE_DAEMON ".sock",
	[daemon socket])

dnl
dnl Specify location of anoubisd configuration file
dnl
anoubisd_conf="$sysconfdir/anoubis/anoubisd.conf"
AC_ARG_WITH([conffile],
    [AS_HELP_STRING([--with-conffile=FILE],
      [Specifies location of configuration file.])],
    [anoubisd_conf=$withval], [])
AC_DEFINE_DIR(ANOUBISD_CONF, ["$anoubisd_conf"],
    "Location of configuration file")

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


dnl makefile.include should know that platform is openbsd, to inlcude
dnl libintl.h
AM_CONDITIONAL(OPENBSD,[test "x$obsd" = xyes ])

dnl Enable ugly hack to save some compile time on ix86 (32bit)
dnl this is only used for the libGUIcomponents.so which is only
dnl used for tests.
dnl We cannot use $host here because autoconf will always detect
dnl x86_64 on machines with 64bit kernel and 32bit userland.
case `$CC -dumpmachine 2>/dev/null` in
	i?86-*)
		fpic_hack=yes
	;;
	*)
		fpic_hack=no
esac
AM_CONDITIONAL(FPIC_HACK,[test "x$fpic_hack" = xyes ])

dnl
dnl enable some warnings and treat them as errors
dnl
AM_CFLAGS="-Werror -Wformat-security -W -Wall $AM_CFLAGS"
AM_CXXFLAGS="-Werror -Wformat-security -W -Wall $AM_CXXFLAGS"

dnl
dnl resolve symbols at startup
dnl
AM_LDFLAGS="-Wl,-z,now $AM_LDFLAGS"

dnl
dnl save modified AM_*FLAGS for use in shared libraries
dnl
shared_AM_CFLAGS="-fPIC -shared $AM_CFLAGS"
shared_AM_CXXFLAGS="-fPIC -shared $AM_CXXFLAGS"
shared_AM_LDFLAGS="-shared $AM_LDFLAGS"

dnl check if compiler can create position independent executables
dnl this must be the last change of AM_*FLAGS
dnl
if [ test "x$use_pie" = "xyes" ]; then
	AX_CHECK_COMPILER_FLAGS(-fPIE,
	[
		AM_CFLAGS="-fPIE $AM_CFLAGS"
		AM_CXXFLAGS="-fPIE $AM_CXXFLAGS"
		AM_LDFLAGS="-pie $AM_LDFLAGS"
	])
fi

dnl check if compiler can create static position independent executables
if [ test "x$use_pie" = "xyes" -a "x$linkstatic" = "xyes" ]; then
	AX_CHECK_COMPILER_FLAGS(-pie -static, [],
	[
		AC_MSG_ERROR([-pie and -static cannot be combined.
		    Either disable static linking or disable pie-code
		    (--disable-pie).])
	])
fi

dnl If we use -fPIE, we need to link our tests with a PIC version of libcheck
dnl (unless we are on i386).
AM_CONDITIONAL(NEED_LIBCHECK_PIC,
	[test "x$fpic_hack" != xyes && test "x$use_pie" = xyes && \
	 test "x$check_pic" = xyes])

dnl save copy of standard compiler flags, needed for automake in some
dnl cases to avoid recursion
dnl
prog_AM_CFLAGS="$AM_CFLAGS"
prog_AM_CXXFLAGS="$AM_CXXFLAGS"
prog_AM_LDFLAGS="$AM_LDFLAGS"

dnl needed by automake gettext
AM_GNU_GETTEXT([external])

AC_SUBST(prog_AM_CFLAGS)
AC_SUBST(prog_AM_CXXFLAGS)
AC_SUBST(prog_AM_LDFLAGS)
AC_SUBST(shared_AM_CFLAGS)
AC_SUBST(shared_AM_CXXFLAGS)
AC_SUBST(shared_AM_LDFLAGS)
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_LDFLAGS)

AC_OUTPUT
