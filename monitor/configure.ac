dnl
dnl  Libmonitor top-level configure.ac.
dnl
dnl  Copyright (c) 2019-2020, Rice University.
dnl  All rights reserved.
dnl
dnl  Redistribution and use in source and binary forms, with or without
dnl  modification, are permitted provided that the following conditions are
dnl  met:
dnl
dnl  * Redistributions of source code must retain the above copyright
dnl    notice, this list of conditions and the following disclaimer.
dnl
dnl  * Redistributions in binary form must reproduce the above copyright
dnl    notice, this list of conditions and the following disclaimer in the
dnl    documentation and/or other materials provided with the distribution.
dnl
dnl  * Neither the name of Rice University (RICE) nor the names of its
dnl    contributors may be used to endorse or promote products derived from
dnl    this software without specific prior written permission.
dnl
dnl  This software is provided by RICE and contributors "as is" and any
dnl  express or implied warranties, including, but not limited to, the
dnl  implied warranties of merchantability and fitness for a particular
dnl  purpose are disclaimed. In no event shall RICE or contributors be
dnl  liable for any direct, indirect, incidental, special, exemplary, or
dnl  consequential damages (including, but not limited to, procurement of
dnl  substitute goods or services; loss of use, data, or profits; or
dnl  business interruption) however caused and on any theory of liability,
dnl  whether in contract, strict liability, or tort (including negligence
dnl  or otherwise) arising in any way out of the use of this software, even
dnl  if advised of the possibility of such damage.
dnl

AC_INIT([libmonitor], [0.x], [hpctoolkit-forum@rice.edu])
AC_COPYRIGHT([libmonitor is Copyright (c) 2019-2020, Rice University.
See the file LICENSE for details.])

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])
AC_CONFIG_HEADERS([src/monitor-config.h])

AM_INIT_AUTOMAKE([1.15.1 foreign no-dist -Wall])
AM_MAINTAINER_MODE
AC_PREREQ(2.69)

# Disable building the static version of libtool libraries (we don't
# use them).  This must come before PROG_AR and PROG_LIBTOOL.
#
enable_static=no

AM_PROG_AR
AC_PROG_LIBTOOL
LT_PREREQ(2.4.6)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_CPP

AC_PROG_AWK
AC_PROG_GREP
AC_PROG_SED

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/monitor-run])
AC_CONFIG_FILES([src/monitor-link])

AC_LANG([C])

#------------------------------------------------------------
# CPU type
#------------------------------------------------------------

host_cpu_ppc=no
case "$host_cpu" in
    [*[pP][pP][cC]*] | [*[pP][oO][wW][eE][rR][pP][cC]*] )
	host_cpu_ppc=yes
	;;
esac

#------------------------------------------------------------
# Compiler path, version and wordsize
#------------------------------------------------------------

# Find the location and version of the C compiler and warn if not
# using GNU gcc.  This is mainly for display in the summary.  We don't
# really do anything differently based on the version.
#
# The --version option works with GNU and Intel.  We could extend this
# to PGI, IBM, etc, but we probably don't build cleanly with those
# compilers.

CC_VERSION='unknown non-gnu'
version=`$CC --version 2>/dev/null`
if echo "$version" | $GREP -i copyright >/dev/null ; then
    CC_VERSION=`echo "$version" | head -1`
fi

CC_PATH=
base=`echo $CC | $AWK '{ print $1 }'`
case "$base" in
    /* ) CC_PATH="$base" ;;
    * )  AC_PATH_PROG([CC_PATH], [$base]) ;;
esac
if test "x$CC_PATH" = x ; then
    CC_PATH=unknown
fi

AC_MSG_NOTICE([C compiler: $CC_VERSION])

warn_non_gcc=no
echo $version | $GREP -E -i -e 'copy.*free.*soft.*found' >/dev/null
if test $? -ne 0 ; then
    warn_non_gcc=yes
    AC_MSG_WARN([not using the GNU C compiler])
fi

#------------------------------------------------------------
# GNU atomic ops
#------------------------------------------------------------

# Test if CC supports GNU atomic ops: __sync_val_compare_and_swap()

AC_MSG_CHECKING([if CC supports GNU atomic ops])

AC_LINK_IFELSE([
AC_LANG_SOURCE([[
#include <stdio.h>
int main(int argc, char **argv)
{
    long x = (long) argc;
    x = __sync_val_compare_and_swap(&x, 2L, 3L);
    printf("%ld", x);
    return 0;
}
]])], [gnu_atomic_ops=yes], [gnu_atomic_ops=no])

AC_MSG_RESULT([$gnu_atomic_ops])

if test "$gnu_atomic_ops" != yes ; then
    AC_MSG_ERROR([CC does not support GNU atomic ops: $CC])
fi

#------------------------------------------------------------
# Option: --with-gotcha=PATH
#------------------------------------------------------------

multilib_path='lib64 lib'

GOTCHA=no

AC_ARG_WITH([gotcha],
    [AS_HELP_STRING([--with-gotcha=PATH],
        [path to gotcha install directory])],
    [GOTCHA="$withval"],
    [])

GOTCHA_IFLAGS=
GOTCHA_LFLAGS=
GOTCHA_LIBDIR=

AC_MSG_CHECKING([for gotcha])

case "$GOTCHA" in
    /* )
        if test -f "${GOTCHA}/include/gotcha/gotcha.h" ; then :
	else
	    AC_MSG_ERROR([unable to find gotcha.h in: $GOTCHA])
	fi
	GOTCHA_IFLAGS="-I${GOTCHA}/include"

	for lib in $multilib_path ; do
	    if test -f "${GOTCHA}/${lib}/libgotcha.so" ; then
	        GOTCHA_LIBDIR="${GOTCHA}/$lib"
		break
	    fi
	done
	if test "x$GOTCHA_LIBDIR" = x ; then
	    AC_MSG_ERROR([unable to find libgotcha.so in: $GOTCHA])
	fi
	GOTCHA_LFLAGS="-L$GOTCHA_LIBDIR -lgotcha"
	;;
    no )
	;;
    * )
        AC_MSG_ERROR([gotcha directory must be absolute path: $GOTCHA])
	;;
esac

AC_MSG_RESULT([$GOTCHA])

if test "x$GOTCHA_IFLAGS" = x ; then
    AC_MSG_WARN([gotcha is required for libmonitor])
    AC_MSG_ERROR([must use --with-gotcha=PATH to specify path to gotcha])
fi

AC_SUBST([GOTCHA_IFLAGS])
AC_SUBST([GOTCHA_LFLAGS])
AC_SUBST([GOTCHA_LIBDIR])

#------------------------------------------------------------
# Option: --enable-dlfcn=yes
#------------------------------------------------------------

# Note: autoconf uses 'enable_dlopen', so we use dlfcn.

AC_ARG_ENABLE([dlfcn],
    [AS_HELP_STRING([--enable-dlfcn],
	[include support for dlopen and friends (default=yes)])],
    [],
    [enable_dlfcn=yes])

AC_MSG_NOTICE([enable dlopen: $enable_dlfcn])

case "$enable_dlfcn" in
     yes | no ) ;;
     * ) AC_MSG_ERROR([invalid value for enable dlfcn: $enable_dlfcn]) ;;
esac

if test "$enable_dlfcn" = yes ; then
    AC_DEFINE([MONITOR_USE_DLOPEN], [1], [Include support for dlopen.])
fi

AM_CONDITIONAL([MONITOR_COND_USE_DLOPEN], [test x$enable_dlfcn = xyes])

#------------------------------------------------------------
# Option: --enable-start-main=TYPE
#------------------------------------------------------------

AC_ARG_ENABLE([start-main],
    [AS_HELP_STRING([--enable-start-main=TYPE],
        [type of __libc_start_main() function (x86, ppc)])],
    [],
    [enable_start_main=x86
     if test "x$host_cpu_ppc" = xyes ; then
	 enable_start_main=ppc
     fi])

AC_MSG_CHECKING([type of __libc_start_main() function])

case "$enable_start_main" in
    ppc )
	AC_DEFINE([MONITOR_START_MAIN_PPC], [1],
	    [libc start main type is ppc.])
	;;
    x86 )
	;;
    * )
	AC_MSG_ERROR([invalid start main type (x86, ppc): $enable_start_main])
	;;
esac

AC_MSG_RESULT([$enable_start_main])

#------------------------------------------------------------
# Display a summary of the config options.
#------------------------------------------------------------

AC_OUTPUT

AC_MSG_NOTICE([-------------------])
AC_MSG_NOTICE([Libmonitor summary])
AC_MSG_NOTICE([-------------------])

AC_MSG_NOTICE([prefix: $prefix])
AC_MSG_NOTICE([build: $build])
AC_MSG_NOTICE([host:  $host])

AC_MSG_NOTICE([C compiler: $CC_VERSION])
AC_MSG_NOTICE([path: $CC_PATH])
AC_MSG_NOTICE([CC:   $CC])
AC_MSG_NOTICE([CFLAGS:   $CFLAGS])
AC_MSG_NOTICE([CPPFLAGS: $CPPFLAGS])

AC_MSG_NOTICE([gotcha: $GOTCHA])

AC_MSG_NOTICE([start main type: $enable_start_main])
