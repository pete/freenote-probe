# Generated automatically from Makefile.in by configure.
# $Id: Makefile.in,v 1.3 2000/01/05 16:29:43 ghudson Exp $

SHELL=/bin/sh
INSTALL=/usr/bin/install -c
RANLIB=ranlib
srcdir=.
top_srcdir=.
prefix=/usr/local
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include
mandir=${prefix}/man
top_builddir=.

CC=gcc
CPPFLAGS=
CFLAGS=-g -O2 ${WARN_CFLAGS} ${ERROR_CFLAGS}
DEFS= 
LDFLAGS=
LIBS=
ALL_CFLAGS=${CPPFLAGS} ${CFLAGS} ${DEFS}
LIBTOOL=$(SHELL) $(top_builddir)/libtool

OBJS=	ares__close_sockets.lo ares__get_hostent.lo ares__read_line.lo \
	ares_destroy.lo ares_expand_name.lo ares_fds.lo ares_free_errmem.lo \
	ares_free_hostent.lo ares_free_string.lo ares_gethostbyaddr.lo \
	ares_gethostbyname.lo ares_init.lo ares_mkquery.lo \
	ares_parse_a_reply.lo \
	ares_parse_ptr_reply.lo ares_process.lo ares_query.lo ares_search.lo \
	ares_send.lo ares_strerror.lo ares_timeout.lo

.SUFFIXES: .c .lo

all: libares.la adig ahost

libares.la: ${OBJS}
	${LIBTOOL} ${CC} ${LDFLAGS} -o $@ ${OBJS} \
		-version-info 0 -rpath ${libdir}

adig: adig.lo libares.la
	${LIBTOOL} ${CC} ${LDFLAGS} -o $@ adig.lo libares.la ${LIBS}

ahost: ahost.lo libares.la
	${LIBTOOL} ${CC} ${LDFLAGS} -o $@ ahost.lo libares.la ${LIBS}

${OBJS}: ares.h ares_dns.h ares_private.h

.c.lo:
	${LIBTOOL} ${CC} -c ${ALL_CFLAGS} $<

check:

install:
	${top_srcdir}/mkinstalldirs ${DESTDIR}${libdir}
	${top_srcdir}/mkinstalldirs ${DESTDIR}${includedir}
	${top_srcdir}/mkinstalldirs ${DESTDIR}${mandir}/man3
	${LIBTOOL} ${INSTALL} -m 644 libares.la ${DESTDIR}${libdir}/libares.la
	${INSTALL} -m 444 ${srcdir}/ares.h ${DESTDIR}${includedir}
	${INSTALL} -m 444 ${srcdir}/ares_destroy.3 ${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_expand_name.3 ${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_fds.3 ${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_free_errmem.3 ${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_free_hostent.3 \
		${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_free_string.3 \
		${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_gethostbyaddr.3 \
		${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_gethostbyname.3 \
		${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_init.3 ${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_init_options.3 \
		${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_mkquery.3 ${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_parse_a_reply.3 \
		${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_parse_ptr_reply.3 \
		${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_process.3 ${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_query.3 ${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_search.3 ${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_send.3 ${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_strerror.3 ${DESTDIR}${mandir}/man3
	${INSTALL} -m 444 ${srcdir}/ares_timeout.3 ${DESTDIR}${mandir}/man3
	${LIBTOOL} --finish ${DESTDIR}/${libdir}

clean:
	rm -f *.o *.lo libares.la adig ahost
	rm -rf .libs _libs

distclean: clean
	rm -f libtool config.cache config.log config.status Makefile
