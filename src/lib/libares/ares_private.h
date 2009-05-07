/* $Id: ares_private.h,v 1.3 1998/09/22 01:46:11 ghudson Exp $ */

/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#include <stdio.h>

#ifdef WIN32
# include <winsock.h>
#else
# include <sys/types.h>
# include <netinet/in.h>
#endif
#define	DEFAULT_TIMEOUT		5
#define DEFAULT_TRIES		4
#ifndef INADDR_NONE
#define	INADDR_NONE 0xffffffff
#endif

#define PATH_RESOLV_CONF	"/etc/resolv.conf"
#ifdef ETC_INET
#define PATH_HOSTS		"/etc/inet/hosts"
#else
#define PATH_HOSTS		"/etc/hosts"
#endif

struct send_request {
  /* Remaining data to send */
  const char *data;
  int len;

  /* Next request in queue */
  struct send_request *next;
};

/*	/p	*/
/*
struct server_state {
  struct in_addr addr;
  int udp_socket;
  int tcp_socket;

  unsigned char tcp_lenbuf[2];
  int tcp_lenbuf_pos;
  int tcp_length;

  unsigned char *tcp_buffer;
  int tcp_buffer_pos;

  struct send_request *qhead;
  struct send_request *qtail;
};
*/

struct query {
  /* Query ID from qbuf, for faster lookup, and current timeout */
  unsigned short qid;
  time_t timeout;

  /* Query buf with length at beginning, for TCP transmission */
  char *tcpbuf;
  int tcplen;

  /* Arguments passed to ares_send() (qbuf points into tcpbuf) */
  const char *qbuf;
  int qlen;
  ares_callback callback;
  void *arg;

  /* Query status */
  int try;
  int server;
  int *skip_server;
  int using_tcp;
  int error_status;

  /* Next query in chain */
  struct query *next;
};

/* An IP address pattern; matches an IP address X if X & mask == addr */
struct apattern {
  struct in_addr addr;
  struct in_addr mask;
};

void ares__send_query(ares_channel channel, struct query *query, time_t now);
void ares__close_sockets(struct server_state *server);
int ares__get_hostent(FILE *fp, struct hostent **host);
int ares__read_line(FILE *fp, char **buf, int *bufsize);
