dnl $Id: aclocal.m4,v 1.2 2001/06/19 11:03:50 kcr Exp $

dnl Copyright 1996 by the Massachusetts Institute of Technology.
dnl
dnl Permission to use, copy, modify, and distribute this
dnl software and its documentation for any purpose and without
dnl fee is hereby granted, provided that the above copyright
dnl notice appear in all copies and that both that copyright
dnl notice and this permission notice appear in supporting
dnl documentation, and that the name of M.I.T. not be used in
dnl advertising or publicity pertaining to distribution of the
dnl software without specific, written prior permission.
dnl M.I.T. makes no representations about the suitability of
dnl this software for any purpose.  It is provided "as is"
dnl without express or implied warranty.

dnl This file provides local macros for packages which use specific
dnl external libraries.  The public macros are:
dnl
dnl	ATHENA_UTIL_COM_ERR
dnl		Generates error if com_err not found.
dnl	ATHENA_UTIL_SS
dnl		Generates error if ss not found.
dnl	ATHENA_REGEXP
dnl		Sets REGEX_LIBS if rx library used; ensures POSIX
dnl		regexp support.
dnl	ATHENA_MOTIF
dnl		Sets MOTIF_LIBS and defines HAVE_MOTIF if Motif used.
dnl	ATHENA_MOTIF_REQUIRED
dnl		Generates error if Motif not found.
dnl	ATHENA_AFS
dnl		Sets AFS_LIBS and defines HAVE_AFS if AFS used.  Pass
dnl		in an argument giving the desired AFS libraries;
dnl		AFS_LIBS will be set to that value if AFS is found.
dnl		AFS_DIR will be set to the prefix given.
dnl	ATHENA_AFS_REQUIRED
dnl		Generates error if AFS libraries not found.  AFS_DIR
dnl		will be set to the prefix given.
dnl	ATHENA_KRB4
dnl		Sets KRB4_LIBS and defines HAVE_KRB4 if krb4 used.
dnl	ATHENA_KRB4_REQUIRED
dnl		Generates error if krb4 not found.  Sets KRB4_LIBS
dnl		otherwise.  (Special behavior because krb4 libraries
dnl		may be different if using krb4 compatibility libraries
dnl		from krb5.)
dnl	ATHENA_KRB5
dnl		Sets KRB5_LIBS and defines HAVE_KRB5 if krb5 used.
dnl	ATHENA_KRB5_REQUIRED
dnl		Generates error if krb5 not found.
dnl	ATHENA_HESIOD
dnl		Sets HESIOD_LIBS and defines HAVE_HESIOD if Hesiod
dnl		used.
dnl	ATHENA_HESIOD_REQUIRED
dnl		Generates error if Hesiod not found.
dnl	ATHENA_ARES
dnl		Sets ARES_LIBS and defines HAVE_ARES if libares
dnl		used.
dnl	ATHENA_ARES_REQUIRED
dnl		Generates error if libares not found.
dnl	ATHENA_ZEPHYR
dnl		Sets ZEPHYR_LIBS and defines HAVE_ZEPHYR if zephyr
dnl		used.
dnl	ATHENA_ZEPHYR_REQUIRED
dnl		Generates error if zephyr not found.
dnl
dnl All of the macros may extend CPPFLAGS and LDFLAGS to let the
dnl compiler find the requested libraries.  Put ATHENA_UTIL_COM_ERR
dnl and ATHENA_UTIL_SS before ATHENA_AFS or ATHENA_AFS_REQUIRED; there
dnl is a com_err library in the AFS libraries which requires -lutil.

dnl ----- com_err -----

AC_DEFUN(ATHENA_UTIL_COM_ERR,
[AC_ARG_WITH(com_err,
	[  --with-com_err=PREFIX   Specify location of com_err],
	[com_err="$withval"], [com_err=yes])
if test "$com_err" != no; then
	if test "$com_err" != yes; then
		CPPFLAGS="$CPPFLAGS -I$com_err/include"
		LDFLAGS="$LDFLAGS -L$com_err/lib"
	fi
	AC_CHECK_LIB(com_err, com_err, :,
		     [AC_MSG_ERROR(com_err library not found)])
else
	AC_MSG_ERROR(This package requires com_err.)
fi])

dnl ----- ss -----

AC_DEFUN(ATHENA_UTIL_SS,
[AC_ARG_WITH(ss,
	[  --with-ss=PREFIX        Specify location of ss (requires com_err)],
	[ss="$withval"], [ss=yes])
if test "$ss" != no; then
	if test "$ss" != yes; then
		CPPFLAGS="$CPPFLAGS -I$ss/include"
		LDFLAGS="$LDFLAGS -L$ss/lib"
	fi
	AC_CHECK_LIB(ss, ss_perror, :,
		     [AC_MSG_ERROR(ss library not found)], -lcom_err)
else
	AC_MSG_ERROR(This package requires ss.)
fi])

dnl ----- Regular expressions -----

AC_DEFUN(ATHENA_REGEXP,
[AC_ARG_WITH(regex,
	[  --with-regex=PREFIX     Use installed regex library],
	[regex="$withval"], [regex=no])
if test "$regex" != no; then
	if test "$regex" != yes; then
		CPPFLAGS="$CPPFLAGS -I$regex/include"
		LDFLAGS="$LDFLAGS -L$regex/lib"
	fi
	AC_CHECK_LIB(regex, regcomp, REGEX_LIBS=-lregex,
		     [AC_MSG_ERROR(regex library not found)])
else
	AC_CHECK_FUNC(regcomp, :,
		      [AC_MSG_ERROR(can't find POSIX regexp support)])
fi
AC_SUBST(REGEX_LIBS)])

dnl ----- Motif -----

AC_DEFUN(ATHENA_MOTIF_CHECK,
[if test "$motif" != yes; then
	CPPFLAGS="$CPPFLAGS -I$motif/include"
	LDFLAGS="$LDFLAGS -L$motif/lib"
fi
AC_CHECK_LIB(Xm, XmStringFree, :, [AC_MSG_ERROR(Motif library not found)])])

AC_DEFUN(ATHENA_MOTIF,
[AC_ARG_WITH(motif,
	[  --with-motif=PREFIX     Use Motif],
	[motif="$withval"], [motif=no])
if test "$motif" != no; then
	ATHENA_MOTIF_CHECK
	MOTIF_LIBS=-lXm
	AC_DEFINE(HAVE_MOTIF)
fi
AC_SUBST(MOTIF_LIBS)])

AC_DEFUN(ATHENA_MOTIF_REQUIRED,
[AC_ARG_WITH(motif,
	[  --with-motif=PREFIX     Specify location of Motif],
	[motif="$withval"], [motif=yes])
if test "$motif" != no; then
	ATHENA_MOTIF_CHECK
else
	AC_MSG_ERROR(This package requires Motif.)
fi])

dnl ----- AFS -----

AC_DEFUN(ATHENA_AFS_CHECK,
[AC_CHECK_FUNC(insque, :, AC_CHECK_LIB(compat, insque))
AC_CHECK_FUNC(gethostbyname, :, AC_CHECK_LIB(nsl, gethostbyname))
AC_CHECK_FUNC(socket, :, AC_CHECK_LIB(socket, socket))
if test "$afs" != yes; then
	CPPFLAGS="$CPPFLAGS -I$afs/include"
	LDFLAGS="$LDFLAGS -L$afs/lib -L$afs/lib/afs"
fi
AC_CHECK_LIB(sys, pioctl, :, [AC_MSG_ERROR(AFS libraries not found)],
	     -lrx -llwp -lsys)
AFS_DIR=$afs
AC_SUBST(AFS_DIR)])

dnl Specify desired AFS libraries as a parameter.
AC_DEFUN(ATHENA_AFS,
[AC_ARG_WITH(afs,
	[  --with-afs=PREFIX       Use AFS libraries],
	[afs="$withval"], [afs=no])
if test "$afs" != no; then
	ATHENA_AFS_CHECK
	AFS_LIBS=$1
	AC_DEFINE(HAVE_AFS)
fi
AC_SUBST(AFS_LIBS)])

AC_DEFUN(ATHENA_AFS_REQUIRED,
[AC_ARG_WITH(afs,
	[  --with-afs=PREFIX       Specify location of AFS libraries],
	[afs="$withval"], [afs=/usr/afsws])
if test "$afs" != no; then
	ATHENA_AFS_CHECK
else
	AC_MSG_ERROR(This package requires AFS libraries.)
fi])

dnl ----- Kerberos 4 -----

AC_DEFUN(ATHENA_KRB4_CHECK,
[AC_CHECK_FUNC(gethostbyname, :, AC_CHECK_LIB(nsl, gethostbyname))
AC_CHECK_FUNC(socket, :, AC_CHECK_LIB(socket, socket))
AC_CHECK_LIB(gen, compile)
if test "$krb4" != yes; then
	CPPFLAGS="$CPPFLAGS -I$krb4/include"
	if test -d "$krb4/include/kerberosIV"; then
		CPPFLAGS="$CPPFLAGS -I$krb4/include/kerberosIV"
	fi
	LDFLAGS="$LDFLAGS -L$krb4/lib"
fi
AC_CHECK_LIB(krb4, krb_rd_req,
	     [KRB4_LIBS="-lkrb4 -ldes425 -lkrb5 -lk5crypto -lcom_err"],
	     [AC_CHECK_LIB(krb, krb_rd_req,
			   [KRB4_LIBS="-lkrb -ldes"],
			   [AC_MSG_ERROR(Kerberos 4 libraries not found)],
			   -ldes)],
	     -ldes425 -lkrb5 -lk5crypto -lcom_err)])

AC_DEFUN(ATHENA_KRB4,
[AC_ARG_WITH(krb4,
	[  --with-krb4=PREFIX      Use Kerberos 4],
	[krb4="$withval"], [krb4=no])
if test "$krb4" != no; then
	ATHENA_KRB4_CHECK
	AC_DEFINE(HAVE_KRB4)
fi
AC_SUBST(KRB4_LIBS)])

AC_DEFUN(ATHENA_KRB4_REQUIRED,
[AC_ARG_WITH(krb4,
	[  --with-krb4=PREFIX      Specify location of Kerberos 4],
	[krb4="$withval"], [krb4=yes])
if test "$krb4" != no; then
	ATHENA_KRB4_CHECK
	AC_SUBST(KRB4_LIBS)
else
	AC_MSG_ERROR(This package requires Kerberos 4.)
fi])

dnl ----- Kerberos 5 -----

AC_DEFUN(ATHENA_KRB5_CHECK,
[AC_SEARCH_LIBS(gethostbyname, nsl)
AC_SEARCH_LIBS(socket, socket)
AC_CHECK_LIB(gen, compile)
if test "$krb5" != yes; then
	CPPFLAGS="$CPPFLAGS -I$krb5/include"
	LDFLAGS="$LDFLAGS -L$krb5/lib"
fi
AC_CHECK_LIB(krb5, krb5_init_context, :,
	     [AC_MSG_ERROR(Kerberos 5 libraries not found)],
	     -lk5crypto -lcom_err)])

AC_DEFUN(ATHENA_KRB5,
[AC_ARG_WITH(krb5,
	[  --with-krb5=PREFIX      Use Kerberos 5],
	[krb5="$withval"], [krb5=no])
if test "$krb5" != no; then
	ATHENA_KRB5_CHECK
	KRB5_LIBS="-lkrb5 -lk5crypto -lcom_err"
	AC_DEFINE(HAVE_KRB5)
fi
AC_SUBST(KRB5_LIBS)])

AC_DEFUN(ATHENA_KRB5_REQUIRED,
[AC_ARG_WITH(krb5,
	[  --with-krb5=PREFIX      Specify location of Kerberos 5],
	[krb5="$withval"], [krb5=yes])
if test "$krb5" != no; then
	ATHENA_KRB5_CHECK
else
	AC_MSG_ERROR(This package requires Kerberos 5.)
fi])

dnl ----- Hesiod -----

AC_DEFUN(ATHENA_HESIOD_CHECK,
[AC_CHECK_FUNC(res_send, :, AC_CHECK_LIB(resolv, res_send))
if test "$hesiod" != yes; then
	CPPFLAGS="$CPPFLAGS -I$hesiod/include"
	LDFLAGS="$LDFLAGS -L$hesiod/lib"
fi
AC_CHECK_LIB(hesiod, hes_resolve, :,
	     [AC_MSG_ERROR(Hesiod library not found)])])

AC_DEFUN(ATHENA_HESIOD,
[AC_ARG_WITH(hesiod,
	[  --with-hesiod=PREFIX    Use Hesiod],
	[hesiod="$withval"], [hesiod=no])
if test "$hesiod" != no; then
	ATHENA_HESIOD_CHECK
	HESIOD_LIBS="-lhesiod"
	AC_DEFINE(HAVE_HESIOD)
fi
AC_SUBST(HESIOD_LIBS)])

AC_DEFUN(ATHENA_HESIOD_REQUIRED,
[AC_ARG_WITH(hesiod,
	[  --with-hesiod=PREFIX    Specify location of Hesiod],
	[hesiod="$withval"], [hesiod=yes])
if test "$hesiod" != no; then
	ATHENA_HESIOD_CHECK
else
	AC_MSG_ERROR(This package requires Hesiod.)
fi])

dnl ----- libares -----

AC_DEFUN(ATHENA_ARES_CHECK,
[AC_CHECK_FUNC(res_send, :, AC_CHECK_LIB(resolv, res_send))
if test "$ares" != yes; then
	CPPFLAGS="$CPPFLAGS -I$ares/include"
	LDFLAGS="$LDFLAGS -L$ares/lib"
fi
AC_CHECK_LIB(ares, ares_init, :, [AC_MSG_ERROR(libares not found)])])

AC_DEFUN(ATHENA_ARES,
[AC_ARG_WITH(ares,
	[  --with-ares=PREFIX      Use libares],
	[ares="$withval"], [ares=no])
if test "$ares" != no; then
	ATHENA_ARES_CHECK
	ARES_LIBS="-lares"
	AC_DEFINE(HAVE_ARES)
fi
AC_SUBST(ARES_LIBS)])

AC_DEFUN(ATHENA_ARES_REQUIRED,
[AC_ARG_WITH(ares,
	[  --with-ares=PREFIX      Specify location of libares],
	[ares="$withval"], [ares=yes])
if test "$ares" != no; then
	ATHENA_ARES_CHECK
else
	AC_MSG_ERROR(This package requires libares.)
fi])
dnl ----- zephyr -----

AC_DEFUN(ATHENA_ZEPHYR_CHECK,
[if test "$zephyr" != yes; then
	CPPFLAGS="$CPPFLAGS -I$zephyr/include"
	LDFLAGS="$LDFLAGS -L$zephyr/lib"
fi
AC_CHECK_LIB(zephyr, ZFreeNotice, :, [AC_MSG_ERROR(zephyr not found)])])

AC_DEFUN(ATHENA_ZEPHYR,
[AC_ARG_WITH(zephyr,
	[  --with-zephyr=PREFIX      Use zephyr],
	[zephyr="$withval"], [zephyr=no])
if test "$zephyr" != no; then
	ATHENA_ZEPHYR_CHECK
	ZEPHYR_LIBS="-lzephyr"
	AC_DEFINE(HAVE_ZEPHYR)
fi
AC_SUBST(ZEPHYR_LIBS)])

AC_DEFUN(ATHENA_ZEPHYR_REQUIRED,
[AC_ARG_WITH(zephyr,
	[  --with-zephyr=PREFIX      Specify location of zephyr],
	[zephyr="$withval"], [zephyr=yes])
if test "$zephyr" != no; then
	ATHENA_ZEPHYR_CHECK
else
	AC_MSG_ERROR(This package requires zephyr.)
fi])
## libtool.m4 - Configure libtool for the target system. -*-Shell-script-*-
## Copyright (C) 1996-1999, 2000 Free Software Foundation, Inc.
## Originally by Gordon Matzigkeit <gord@gnu.ai.mit.edu>, 1996
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##
## As a special exception to the GNU General Public License, if you
## distribute this file as part of a program that contains a
## configuration script generated by Autoconf, you may include it under
## the same distribution terms that you use for the rest of that program.

# serial 40 AC_PROG_LIBTOOL
AC_DEFUN(AC_PROG_LIBTOOL,
[AC_REQUIRE([AC_LIBTOOL_SETUP])dnl

# Save cache, so that ltconfig can load it
AC_CACHE_SAVE

# Actually configure libtool.  ac_aux_dir is where install-sh is found.
CC="$CC" CFLAGS="$CFLAGS" CPPFLAGS="$CPPFLAGS" \
LD="$LD" LDFLAGS="$LDFLAGS" LIBS="$LIBS" \
LN_S="$LN_S" NM="$NM" RANLIB="$RANLIB" \
DLLTOOL="$DLLTOOL" AS="$AS" OBJDUMP="$OBJDUMP" \
${CONFIG_SHELL-/bin/sh} $ac_aux_dir/ltconfig --no-reexec \
$libtool_flags --no-verify $ac_aux_dir/ltmain.sh $lt_target \
|| AC_MSG_ERROR([libtool configure failed])

# Reload cache, that may have been modified by ltconfig
AC_CACHE_LOAD

# This can be used to rebuild libtool when needed
LIBTOOL_DEPS="$ac_aux_dir/ltconfig $ac_aux_dir/ltmain.sh"

# Always use our own libtool.
LIBTOOL='$(SHELL) $(top_builddir)/libtool'
AC_SUBST(LIBTOOL)dnl

# Redirect the config.log output again, so that the ltconfig log is not
# clobbered by the next message.
exec 5>>./config.log
])

AC_DEFUN(AC_LIBTOOL_SETUP,
[AC_PREREQ(2.13)dnl
AC_REQUIRE([AC_ENABLE_SHARED])dnl
AC_REQUIRE([AC_ENABLE_STATIC])dnl
AC_REQUIRE([AC_ENABLE_FAST_INSTALL])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_CANONICAL_BUILD])dnl
AC_REQUIRE([AC_PROG_RANLIB])dnl
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_LD])dnl
AC_REQUIRE([AC_PROG_NM])dnl
AC_REQUIRE([AC_PROG_LN_S])dnl
dnl

case "$target" in
NONE) lt_target="$host" ;;
*) lt_target="$target" ;;
esac

# Check for any special flags to pass to ltconfig.
libtool_flags="--cache-file=$cache_file"
test "$enable_shared" = no && libtool_flags="$libtool_flags --disable-shared"
test "$enable_static" = no && libtool_flags="$libtool_flags --disable-static"
test "$enable_fast_install" = no && libtool_flags="$libtool_flags --disable-fast-install"
test "$ac_cv_prog_gcc" = yes && libtool_flags="$libtool_flags --with-gcc"
test "$ac_cv_prog_gnu_ld" = yes && libtool_flags="$libtool_flags --with-gnu-ld"
ifdef([AC_PROVIDE_AC_LIBTOOL_DLOPEN],
[libtool_flags="$libtool_flags --enable-dlopen"])
ifdef([AC_PROVIDE_AC_LIBTOOL_WIN32_DLL],
[libtool_flags="$libtool_flags --enable-win32-dll"])
AC_ARG_ENABLE(libtool-lock,
  [  --disable-libtool-lock  avoid locking (might break parallel builds)])
test "x$enable_libtool_lock" = xno && libtool_flags="$libtool_flags --disable-lock"
test x"$silent" = xyes && libtool_flags="$libtool_flags --silent"

# Some flags need to be propagated to the compiler or linker for good
# libtool support.
case "$lt_target" in
*-*-irix6*)
  # Find out which ABI we are using.
  echo '[#]line __oline__ "configure"' > conftest.$ac_ext
  if AC_TRY_EVAL(ac_compile); then
    case "`/usr/bin/file conftest.o`" in
    *32-bit*)
      LD="${LD-ld} -32"
      ;;
    *N32*)
      LD="${LD-ld} -n32"
      ;;
    *64-bit*)
      LD="${LD-ld} -64"
      ;;
    esac
  fi
  rm -rf conftest*
  ;;

*-*-sco3.2v5*)
  # On SCO OpenServer 5, we need -belf to get full-featured binaries.
  SAVE_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -belf"
  AC_CACHE_CHECK([whether the C compiler needs -belf], lt_cv_cc_needs_belf,
    [AC_TRY_LINK([],[],[lt_cv_cc_needs_belf=yes],[lt_cv_cc_needs_belf=no])])
  if test x"$lt_cv_cc_needs_belf" != x"yes"; then
    # this is probably gcc 2.8.0, egcs 1.0 or newer; no need for -belf
    CFLAGS="$SAVE_CFLAGS"
  fi
  ;;

ifdef([AC_PROVIDE_AC_LIBTOOL_WIN32_DLL],
[*-*-cygwin* | *-*-mingw*)
  AC_CHECK_TOOL(DLLTOOL, dlltool, false)
  AC_CHECK_TOOL(AS, as, false)
  AC_CHECK_TOOL(OBJDUMP, objdump, false)
  ;;
])
esac
])

# AC_LIBTOOL_DLOPEN - enable checks for dlopen support
AC_DEFUN(AC_LIBTOOL_DLOPEN, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])])

# AC_LIBTOOL_WIN32_DLL - declare package support for building win32 dll's
AC_DEFUN(AC_LIBTOOL_WIN32_DLL, [AC_BEFORE([$0], [AC_LIBTOOL_SETUP])])

# AC_ENABLE_SHARED - implement the --enable-shared flag
# Usage: AC_ENABLE_SHARED[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_SHARED, [dnl
define([AC_ENABLE_SHARED_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(shared,
changequote(<<, >>)dnl
<<  --enable-shared[=PKGS]  build shared libraries [default=>>AC_ENABLE_SHARED_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_shared=yes ;;
no) enable_shared=no ;;
*)
  enable_shared=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_shared=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_shared=AC_ENABLE_SHARED_DEFAULT)dnl
])

# AC_DISABLE_SHARED - set the default shared flag to --disable-shared
AC_DEFUN(AC_DISABLE_SHARED, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_SHARED(no)])

# AC_ENABLE_STATIC - implement the --enable-static flag
# Usage: AC_ENABLE_STATIC[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_STATIC, [dnl
define([AC_ENABLE_STATIC_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(static,
changequote(<<, >>)dnl
<<  --enable-static[=PKGS]  build static libraries [default=>>AC_ENABLE_STATIC_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_static=yes ;;
no) enable_static=no ;;
*)
  enable_static=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_static=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_static=AC_ENABLE_STATIC_DEFAULT)dnl
])

# AC_DISABLE_STATIC - set the default static flag to --disable-static
AC_DEFUN(AC_DISABLE_STATIC, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_STATIC(no)])


# AC_ENABLE_FAST_INSTALL - implement the --enable-fast-install flag
# Usage: AC_ENABLE_FAST_INSTALL[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_FAST_INSTALL, [dnl
define([AC_ENABLE_FAST_INSTALL_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(fast-install,
changequote(<<, >>)dnl
<<  --enable-fast-install[=PKGS]  optimize for fast installation [default=>>AC_ENABLE_FAST_INSTALL_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_fast_install=yes ;;
no) enable_fast_install=no ;;
*)
  enable_fast_install=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_fast_install=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_fast_install=AC_ENABLE_FAST_INSTALL_DEFAULT)dnl
])

# AC_ENABLE_FAST_INSTALL - set the default to --disable-fast-install
AC_DEFUN(AC_DISABLE_FAST_INSTALL, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_FAST_INSTALL(no)])

# AC_PROG_LD - find the path to the GNU or non-GNU linker
AC_DEFUN(AC_PROG_LD,
[AC_ARG_WITH(gnu-ld,
[  --with-gnu-ld           assume the C compiler uses GNU ld [default=no]],
test "$withval" = no || with_gnu_ld=yes, with_gnu_ld=no)
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_CANONICAL_BUILD])dnl
ac_prog=ld
if test "$ac_cv_prog_gcc" = yes; then
  # Check if gcc -print-prog-name=ld gives a path.
  AC_MSG_CHECKING([for ld used by GCC])
  ac_prog=`($CC -print-prog-name=ld) 2>&5`
  case "$ac_prog" in
    # Accept absolute paths.
changequote(,)dnl
    [\\/]* | [A-Za-z]:[\\/]*)
      re_direlt='/[^/][^/]*/\.\./'
changequote([,])dnl
      # Canonicalize the path of ld
      ac_prog=`echo $ac_prog| sed 's%\\\\%/%g'`
      while echo $ac_prog | grep "$re_direlt" > /dev/null 2>&1; do
	ac_prog=`echo $ac_prog| sed "s%$re_direlt%/%"`
      done
      test -z "$LD" && LD="$ac_prog"
      ;;
  "")
    # If it fails, then pretend we aren't using GCC.
    ac_prog=ld
    ;;
  *)
    # If it is relative, then search for the first ld in PATH.
    with_gnu_ld=unknown
    ;;
  esac
elif test "$with_gnu_ld" = yes; then
  AC_MSG_CHECKING([for GNU ld])
else
  AC_MSG_CHECKING([for non-GNU ld])
fi
AC_CACHE_VAL(ac_cv_path_LD,
[if test -z "$LD"; then
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH; do
    test -z "$ac_dir" && ac_dir=.
    if test -f "$ac_dir/$ac_prog" || test -f "$ac_dir/$ac_prog$ac_exeext"; then
      ac_cv_path_LD="$ac_dir/$ac_prog"
      # Check to see if the program is GNU ld.  I'd rather use --version,
      # but apparently some GNU ld's only accept -v.
      # Break only if it was the GNU/non-GNU ld that we prefer.
      if "$ac_cv_path_LD" -v 2>&1 < /dev/null | egrep '(GNU|with BFD)' > /dev/null; then
	test "$with_gnu_ld" != no && break
      else
	test "$with_gnu_ld" != yes && break
      fi
    fi
  done
  IFS="$ac_save_ifs"
else
  ac_cv_path_LD="$LD" # Let the user override the test with a path.
fi])
LD="$ac_cv_path_LD"
if test -n "$LD"; then
  AC_MSG_RESULT($LD)
else
  AC_MSG_RESULT(no)
fi
test -z "$LD" && AC_MSG_ERROR([no acceptable ld found in \$PATH])
AC_PROG_LD_GNU
])

AC_DEFUN(AC_PROG_LD_GNU,
[AC_CACHE_CHECK([if the linker ($LD) is GNU ld], ac_cv_prog_gnu_ld,
[# I'd rather use --version here, but apparently some GNU ld's only accept -v.
if $LD -v 2>&1 </dev/null | egrep '(GNU|with BFD)' 1>&5; then
  ac_cv_prog_gnu_ld=yes
else
  ac_cv_prog_gnu_ld=no
fi])
])

# AC_PROG_NM - find the path to a BSD-compatible name lister
AC_DEFUN(AC_PROG_NM,
[AC_MSG_CHECKING([for BSD-compatible nm])
AC_CACHE_VAL(ac_cv_path_NM,
[if test -n "$NM"; then
  # Let the user override the test.
  ac_cv_path_NM="$NM"
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH /usr/ccs/bin /usr/ucb /bin; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/nm || test -f $ac_dir/nm$ac_exeext ; then
      # Check to see if the nm accepts a BSD-compat flag.
      # Adding the `sed 1q' prevents false positives on HP-UX, which says:
      #   nm: unknown option "B" ignored
      if ($ac_dir/nm -B /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
	ac_cv_path_NM="$ac_dir/nm -B"
	break
      elif ($ac_dir/nm -p /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
	ac_cv_path_NM="$ac_dir/nm -p"
	break
      else
	ac_cv_path_NM=${ac_cv_path_NM="$ac_dir/nm"} # keep the first match, but
	continue # so that we can try to find one that supports BSD flags
      fi
    fi
  done
  IFS="$ac_save_ifs"
  test -z "$ac_cv_path_NM" && ac_cv_path_NM=nm
fi])
NM="$ac_cv_path_NM"
AC_MSG_RESULT([$NM])
])

# AC_CHECK_LIBM - check for math library
AC_DEFUN(AC_CHECK_LIBM,
[AC_REQUIRE([AC_CANONICAL_HOST])dnl
LIBM=
case "$lt_target" in
*-*-beos* | *-*-cygwin*)
  # These system don't have libm
  ;;
*-ncr-sysv4.3*)
  AC_CHECK_LIB(mw, _mwvalidcheckl, LIBM="-lmw")
  AC_CHECK_LIB(m, main, LIBM="$LIBM -lm")
  ;;
*)
  AC_CHECK_LIB(m, main, LIBM="-lm")
  ;;
esac
])

# AC_LIBLTDL_CONVENIENCE[(dir)] - sets LIBLTDL to the link flags for
# the libltdl convenience library and INCLTDL to the include flags for
# the libltdl header and adds --enable-ltdl-convenience to the
# configure arguments.  Note that LIBLTDL and INCLTDL are not
# AC_SUBSTed, nor is AC_CONFIG_SUBDIRS called.  If DIR is not
# provided, it is assumed to be `libltdl'.  LIBLTDL will be prefixed
# with '${top_builddir}/' and INCLTDL will be prefixed with
# '${top_srcdir}/' (note the single quotes!).  If your package is not
# flat and you're not using automake, define top_builddir and
# top_srcdir appropriately in the Makefiles.
AC_DEFUN(AC_LIBLTDL_CONVENIENCE, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
  case "$enable_ltdl_convenience" in
  no) AC_MSG_ERROR([this package needs a convenience libltdl]) ;;
  "") enable_ltdl_convenience=yes
      ac_configure_args="$ac_configure_args --enable-ltdl-convenience" ;;
  esac
  LIBLTDL='${top_builddir}/'ifelse($#,1,[$1],['libltdl'])/libltdlc.la
  INCLTDL='-I${top_srcdir}/'ifelse($#,1,[$1],['libltdl'])
])

# AC_LIBLTDL_INSTALLABLE[(dir)] - sets LIBLTDL to the link flags for
# the libltdl installable library and INCLTDL to the include flags for
# the libltdl header and adds --enable-ltdl-install to the configure
# arguments.  Note that LIBLTDL and INCLTDL are not AC_SUBSTed, nor is
# AC_CONFIG_SUBDIRS called.  If DIR is not provided and an installed
# libltdl is not found, it is assumed to be `libltdl'.  LIBLTDL will
# be prefixed with '${top_builddir}/' and INCLTDL will be prefixed
# with '${top_srcdir}/' (note the single quotes!).  If your package is
# not flat and you're not using automake, define top_builddir and
# top_srcdir appropriately in the Makefiles.
# In the future, this macro may have to be called after AC_PROG_LIBTOOL.
AC_DEFUN(AC_LIBLTDL_INSTALLABLE, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
  AC_CHECK_LIB(ltdl, main,
  [test x"$enable_ltdl_install" != xyes && enable_ltdl_install=no],
  [if test x"$enable_ltdl_install" = xno; then
     AC_MSG_WARN([libltdl not installed, but installation disabled])
   else
     enable_ltdl_install=yes
   fi
  ])
  if test x"$enable_ltdl_install" = x"yes"; then
    ac_configure_args="$ac_configure_args --enable-ltdl-install"
    LIBLTDL='${top_builddir}/'ifelse($#,1,[$1],['libltdl'])/libltdl.la
    INCLTDL='-I${top_srcdir}/'ifelse($#,1,[$1],['libltdl'])
  else
    ac_configure_args="$ac_configure_args --enable-ltdl-install=no"
    LIBLTDL="-lltdl"
    INCLTDL=
  fi
])

dnl old names
AC_DEFUN(AM_PROG_LIBTOOL, [indir([AC_PROG_LIBTOOL])])dnl
AC_DEFUN(AM_ENABLE_SHARED, [indir([AC_ENABLE_SHARED], $@)])dnl
AC_DEFUN(AM_ENABLE_STATIC, [indir([AC_ENABLE_STATIC], $@)])dnl
AC_DEFUN(AM_DISABLE_SHARED, [indir([AC_DISABLE_SHARED], $@)])dnl
AC_DEFUN(AM_DISABLE_STATIC, [indir([AC_DISABLE_STATIC], $@)])dnl
AC_DEFUN(AM_PROG_LD, [indir([AC_PROG_LD])])dnl
AC_DEFUN(AM_PROG_NM, [indir([AC_PROG_NM])])dnl

dnl This is just to silence aclocal about the macro not being used
ifelse([AC_DISABLE_FAST_INSTALL])dnl
