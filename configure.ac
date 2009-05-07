AC_PREREQ(2.52)
# Note that you need to change the binary version number in probe.h as well.
AC_INIT(freenote-probe, 1.0.0rc11, freenote@petta-tech.com)
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)

AM_MAINTAINER_MODE

AC_PROG_CC
AC_PROG_INSTALL
AC_C_BIGENDIAN

AC_SUBST(PROBE_CFLAGS)
AC_SUBST(PROBE_LDFLAGS)
PROBE_CFLAGS=""

AM_CONFIG_HEADER(config.h)

AC_CHECK_LIB(socket,socket,LIBS="$LIBS -lsocket",)
AC_CHECK_LIB(nsl,gethostbyname,LIBS="$LIBS -lnsl",)

AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)

AC_CHECK_HEADER(locale.h,AC_DEFINE(HAVE_LOCALE,1,[Define for locale]),)

AC_ARG_WITH(rpath,
	[  --without-rpath         Disable auto-added -R linker paths],
	[
		if test "x$withval" = "xno" ; then	
			need_dash_r=""
		fi
		if test "x$withval" = "xyes" ; then
			need_dash_r=1
		fi
	]
)

# taken from the OpenSSL configure.ac:
# Search for OpenSSL
PROBE_CPPFLAGS="$CPPFLAGS"
PROBE_LDFLAGS="$LDFLAGS"
AC_ARG_WITH(ssl-dir,
	[  --with-ssl-dir=PATH     Specify path to OpenSSL installation ],
	[
		if test "x$withval" != "xno" ; then
			if test -d "$withval/lib"; then
				if test -n "${need_dash_r}"; then
					LDFLAGS="-L${withval}/lib -R${withval}/lib ${LDFLAGS}"
				else
					LDFLAGS="-L${withval}/lib ${LDFLAGS}"
				fi
			else
				if test -n "${need_dash_r}"; then
					LDFLAGS="-L${withval} -R${withval} ${LDFLAGS}"
				else
					LDFLAGS="-L${withval} ${LDFLAGS}"
				fi
			fi
			if test -d "$withval/include"; then
				CPPFLAGS="-I${withval}/include ${CPPFLAGS}"
			else
				CPPFLAGS="-I${withval} ${CPPFLAGS}"
			fi
		fi
	]
)
LIBS="-lcrypto -lz -lssl $LIBS"
AC_TRY_LINK_FUNC(RAND_add, AC_DEFINE([HAVE_OPENSSL],1,[Define to enable OpenSSL]),
	[
		dnl Check default openssl install dir
		if test -n "${need_dash_r}"; then
			LDFLAGS="-L/usr/local/ssl/lib -R/usr/local/ssl/lib ${PROBE_LDFLAGS}"
		else
			LDFLAGS="-L/usr/local/ssl/lib ${PROBE_LDFLAGS}"
		fi
		CPPFLAGS="-I/usr/local/ssl/include ${PROBE_CPPFLAGS}"
		AC_TRY_LINK_FUNC(RAND_add, AC_DEFINE([HAVE_OPENSSL],1,[Define to enable OpenSSL]),
			[
				AC_MSG_ERROR([*** Can't find recent OpenSSL libcrypto (see config.log for details) ***])
			]
		)
	]
)

# Determine OpenSSL header version
AC_MSG_CHECKING([OpenSSL header version])
AC_TRY_RUN(
	[
#include <stdio.h>
#include <string.h>
#include <openssl/opensslv.h>
#define DATA "conftest.sslincver"
int main(void) {
	FILE *fd;
	int rc;

	fd = fopen(DATA,"w");
	if(fd == NULL)
		exit(1);

	if ((rc = fprintf(fd ,"%x (%s)\n", OPENSSL_VERSION_NUMBER, OPENSSL_VERSION_TEXT)) <0)
		exit(1);

	exit(0);
}
	],
	[
		ssl_header_ver=`cat conftest.sslincver`
		AC_MSG_RESULT($ssl_header_ver)
	],
	[
		AC_MSG_RESULT(not found)
		AC_MSG_ERROR(OpenSSL version header not found.)
	]
)

# Determine OpenSSL library version
AC_MSG_CHECKING([OpenSSL library version])
AC_TRY_RUN(
	[
#include <stdio.h>
#include <string.h>
#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#define DATA "conftest.ssllibver"
int main(void) {
	FILE *fd;
	int rc;

	fd = fopen(DATA,"w");
	if(fd == NULL)
		exit(1);

	if ((rc = fprintf(fd ,"%x (%s)\n", SSLeay(), SSLeay_version(SSLEAY_VERSION))) <0)
		exit(1);

	exit(0);
}
	],
	[
		ssl_library_ver=`cat conftest.ssllibver`
		AC_MSG_RESULT($ssl_library_ver)
	],
	[
		AC_MSG_RESULT(not found)
		AC_MSG_ERROR(OpenSSL library not found.)
	]
)

# Sanity check OpenSSL headers
AC_MSG_CHECKING([whether OpenSSL's headers match the library])
AC_TRY_RUN(
	[
#include <string.h>
#include <openssl/opensslv.h>
int main(void) { exit(SSLeay() == OPENSSL_VERSION_NUMBER ? 0 : 1); }
	],
	[
		AC_MSG_RESULT(yes)
	],
	[
		AC_MSG_RESULT(no)
		AC_MSG_ERROR([Your OpenSSL headers do not match your library.
Check config.log for details.
Also see contrib/findssl.sh for help identifying header/library mismatches.])
	]
)

# Some systems want crypt() from libcrypt, *not* the version in OpenSSL,
# because the system crypt() is more featureful.
if test "x$check_for_libcrypt_before" = "x1"; then
	AC_CHECK_LIB(crypt, crypt)
fi

# Some Linux systems (Slackware) need crypt() from libcrypt, *not* the
# version in OpenSSL.
if test "x$check_for_libcrypt_later" = "x1"; then
	AC_CHECK_LIB(crypt, crypt, LIBS="$LIBS -lcrypt")
fi
# /end of stuff taken from the OpenSSL configure.ac

dnl AC_OUTPUT(doc/Makefile src/Makefile Makefile)
AC_OUTPUT(doc/Makefile src/Makefile Makefile)



