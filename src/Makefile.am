bin_PROGRAMS = fnprobe
NULL = 

fnprobe_headers = \
 lib/config_parse.h \
 debug.h \
 err.h \
 lib/fileman.h \
 header-parse.h \
 inaddr_none.h \
 io.h \
 io/filelog.h \
 io/syslog.h \
 io/stdout.h \
 lib/collection.h \
 lib/dispatch.h \
 lib/libares/ares.h \
 lib/libares/ares_dns.h \
 lib/libares/ares_private.h \
 lib/task.h \
 net-modules/dns.h \
 net-modules/http.h \
 net-modules/http-recurse.h \
 net-modules/icmp.h \
 lib/nv_pair.h \
 options.h \
 p_regex.h \
 p_time.h \
 p_types.h \
 lib/proto.h \
 compat.h \
 probe.h \
 sys/compat.h \
 sys/unix.h \
 url-extract.h \
 url-parse.h \
 urlencode.h \
 urldecode.h \
 util.h \
 lib/wmips.h \
 work.h \
 crypt/dsa/pdsa.h \
 crypt/base64/base64.h \
 lib/cookie.h \
 crypt/ossl_util.h \
 lib/probecast.h \
 setup.h \
 caps.h \
 lib/p_sysinfo.h \
 $(NULL)

fnprobe_SOURCES = \
 $(fnprobe_headers) \
 lib/config_parse.c \
 lib/fileman.c \
 header-parse.c \
 io.c \
 io/filelog.c \
 io/syslog.c \
 io/stdout.c \
 lib/collection.c \
 lib/dispatch.c \
 lib/libares/ares__close_sockets.c \
 lib/libares/ares__get_hostent.c \
 lib/libares/ares__read_line.c \
 lib/libares/ares_destroy.c \
 lib/libares/ares_expand_name.c \
 lib/libares/ares_fds.c \
 lib/libares/ares_free_errmem.c \
 lib/libares/ares_free_hostent.c \
 lib/libares/ares_free_string.c \
 lib/libares/ares_gethostbyaddr.c \
 lib/libares/ares_gethostbyname.c \
 lib/libares/ares_init.c \
 lib/libares/ares_mkquery.c \
 lib/libares/ares_parse_a_reply.c \
 lib/libares/ares_parse_ptr_reply.c \
 lib/libares/ares_process.c \
 lib/libares/ares_query.c \
 lib/libares/ares_search.c \
 lib/libares/ares_send.c \
 lib/libares/ares_strerror.c \
 lib/libares/ares_timeout.c \
 lib/task.c \
 main.c \
 net-modules/dns.c \
 net-modules/http.c \
 net-modules/http-recurse.c \
 net-modules/icmp.c \
 lib/nv_pair.c \
 options.c \
 p_regex.c \
 p_time.c \
 lib/proto.c \
 sys/unix/daemon.c \
 sys/unix/strnlen.c \
 url-extract.c \
 url-parse.c \
 urlencode.c \
 urldecode.c \
 lib/wmips.c \
 work.c \
 crypt/dsa/pdsa.c \
 crypt/base64/base64.c \
 lib/cookie.c \
 crypt/ossl_util.c \
 lib/probecast.c \
 setup.c \
 caps.c \
 lib/p_sysinfo.c \
 $(NULL)

fnprobe_CFLAGS = $(PROBE_CFLAGS)


