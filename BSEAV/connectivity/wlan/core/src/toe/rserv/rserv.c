/*
 * RSERV: Remote Sockets Server
 *
 * This code is assumed to run in a non-threaded environment.
 * Requests are processed serially in the order they are received.
 * No locking is needed.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include "os.h"

#include "lwip/opt.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/ip.h"
#include "lwip/raw.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "lwip/stats.h"
#include "lwip/netif.h"

#include <rsock/types.h>
#include <rsock/proto.h>
#include <rsock/endian.h>

/* Define O/S-specific shim macros */

#include "rserv_sim.h"

#include "rserv.h"

#ifdef EXT_CBALL
extern void RateStat(char *module, char *stat, int count);
extern void RateStatTrigger(void);
static int RateStat_send = 0;
static int RateStat_recv = 0;
#define STAMP(type)		(*(uint32 *)0x18009014 = (type))
#else
#define STAMP(type)
#endif

#define DEBUG_ENABLE		0
#define DPRINTF			if (DEBUG_ENABLE) printf
#define EPRINTF			printf

/* Protocol version implemented in this file */
#define RSERV_PVER		1

/* Max number of outstanding blocking operations permitted on a given socket */
#define MAX_REQ_PER_SOCKET	4

/* Private socket options */
#define SO_NONBLOCK		0x1000

/* Conversion back and forth between socket fd and socket structure */
#define SI(s)			(&sockinfo[s])
#define S(si)			((int)((si) - sockinfo))

#define IP_ADDR_ZERO(d)		memset(d, 0, 4)
#define IP_ADDR_COPY(d, s)	memcpy(d, s, 4)

#define SOCKADDR_ZERO(d)	(((uint32 *)(d))[0] = ((uint32 *)(d))[1] = \
				 ((uint32 *)(d))[2] = ((uint32 *)(d))[3] = 0)
#define SOCKADDR_COPY(d, s)	(((uint32 *)(d))[0] = ((uint32 *)(s))[0], \
				 ((uint32 *)(d))[1] = ((uint32 *)(s))[1], \
				 ((uint32 *)(d))[2] = ((uint32 *)(s))[2], \
				 ((uint32 *)(d))[3] = ((uint32 *)(s))[3])

#define RECV_Q_ENABLE		0x1
#define RECV_Q_CLOSING		0x2
#define RECV_Q_CLOSED		0x4

#define SOCKET_CHECK(s) {							\
	if ((s) < 0 || (s) >= RSOCK_SOCKET_MAX || sockinfo[s].domain < 0) {	\
		err_no = RSOCK_EBADF;						\
		goto done;								\
	}									\
}

#define SEND_HDRLEN(type) \
	((type) == SOCK_STREAM ? RSOCK_SEND_STREAM_REQ_HDRLEN : RSOCK_SEND_DGRAM_REQ_HDRLEN)

#define RESP_HEADER(resp, req, _len, _err_no) {					\
	(resp)->header.flags = htobs(RSOCK_FLAG_TYPE_RESP |			\
				     (RSERV_PVER << RSOCK_FLAG_PVER_SHIFT));	\
	(resp)->header.len = htobs((uint16)(_len));				\
	(resp)->header.handle = (req)->header.handle;				\
	(resp)->header.err_no = htobs((uint16)(_err_no));			\
}

#define DOMAIN_TYPE(d, t)	((d) << 8 | (t))

/* Receive buffer list */
struct recv_buf {
	struct recv_buf *next;
	struct pbuf *p;
	struct ip_addr fromaddr;  /* Not used for stream */
	u16_t fromport;		/* For raw socket, lwIP uses fromport to return protocol */
};

/* Server side state per socket */
struct sockinfo {
	int domain;		/* PF_xx, or -1 if sockinfo entry unallocated */
	int type;		/* SOCK_RAW/DGRAM/STREAM */
	int proto;		/* Protocol; used for SOCK_RAW */

	union {
		void *ptr;
		struct tcp_pcb *tcp;
		struct udp_pcb *udp;
		struct raw_pcb *raw;
	} pcb;			/* lwIP state specific to connection type */

	int err;		/* lwIP persistent error */
	int options;		/* Flags from SO_xx */

	/* Accept backlog list */
	int backlog_max;	/* Maximum sockets allowed on backlog list */
	int backlog_next;	/* List of sockets representing connections not yet accepted */

	/* Request queue */
	RSERV_PKT *req_pend;	/* Queue of client requests that can't be satisfied immediately */
	RSERV_PKT *req_pend_tail;  /* Pointer to tail packet in req_pend */
	int req_pend_count;	/* Number of entries in req_pend */

	/* Send data queue for TCP */
	RSERV_PKT *send_q_head;	/* Queue to hold TCP send data before passing to tcp_write() */
	RSERV_PKT *send_q_tail;	/* Pointer to tail packet in send_q */
	int send_q_len;		/* Number of entries on send_q */
	int send_q_aoff;	/* Offset to next unacked data within send_q_head packet */
	RSERV_PKT *send_q_spkt;	/* Pointer to next packet on send_q containing unsent data */
	int send_q_soff;	/* Offset to next unsent data within send_q_spkt packet */

	/* Receive data queue for TCP, UDP, Raw */
	struct recv_buf *recv_q_head;  /* Queue holding receive data not yet sent to client */
	struct recv_buf *recv_q_tail;  /* Pointer to tail packet in recv_q */
	int recv_q_unacked;	/* Number of outstanding rdata messages sent to client */
	int recv_q_total;	/* Total bytes of data in recv_q */
	int recv_q_hoff;	/* Offset to next unread data in recv_q head pbuf (stream only) */
	uint32 recv_q_flags;	/* RECV_Q_xx */
};

struct sockinfo sockinfo[RSOCK_SOCKET_MAX];

/*
 * Translation of lwIP error code to RSOCK error code
 */

static const uint8
err_xlate_table[] = {
	0,			/* ERR_OK    0      No error.                */
	RSOCK_ENOMEM,		/* ERR_MEM  -1      Out of memory error.     */
	RSOCK_ENOBUFS,		/* ERR_BUF  -2      Buffer error.            */
	RSOCK_ECONNABORTED,	/* ERR_ABRT -3      Connection aborted.      */
	RSOCK_ECONNRESET,	/* ERR_RST  -4      Connection reset.        */
	RSOCK_ESHUTDOWN,	/* ERR_CLSD -5      Connection closed.       */
	RSOCK_ENOTCONN,		/* ERR_CONN -6      Not connected.           */
	RSOCK_EINVAL,		/* ERR_VAL  -7      Illegal value.           */
	RSOCK_EIO,		/* ERR_ARG  -8      Illegal argument.        */
	RSOCK_EHOSTUNREACH,	/* ERR_RTE  -9      Routing problem.         */
	RSOCK_EADDRINUSE	/* ERR_USE  -10     Address in use.          */
};

#define ERR_XLATE_TABLE_COUNT \
	((int)((sizeof(err_xlate_table) / sizeof(err_xlate_table[0]))))

static int
err_xlate(int err)
{
	int idx = -err;

	return ((idx < 0 || idx >= ERR_XLATE_TABLE_COUNT) ? RSOCK_EIO :
	        err_xlate_table[idx]);
}

static void
sockaddr_set(struct sockaddr *sa, struct ip_addr *ipaddr, u16_t port)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)sa;

	sin->sin_family = (u8_t)AF_INET;
	sin->sin_port = htons(port);
	IP_ADDR_COPY(&sin->sin_addr, ipaddr);
}

static void
sockaddr_get(struct sockaddr *sa, struct ip_addr *ipaddr, u16_t *port)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)sa;

	*port = ntohs(sin->sin_port);
	IP_ADDR_COPY(ipaddr, &sin->sin_addr);
}

/*
 * Copy data out of a pbuf chain starting at a given offset for a given
 * length.  Returns amount of data copied, which may be less than length
 * if the amount requested was not available.
 */

static int
pbuf_copyout(struct pbuf *p, void *buf, int offset, int length)
{
	int outptr = 0;

	while (p && offset >= p->len) {
		offset -= p->len;
		p = p->next;
	}

	do {
		int n = p->len - offset;
		if (n > length)
			n = length;
		/* STAMP(250); */
		memcpy((uint8 *)buf + outptr, (uint8 *)p->payload + offset, n);
		/* STAMP(251); */
		outptr += n;
		p = p->next;
		offset = 0;
		length -= n;
	} while (length && p != NULL);

	return outptr;
}

/*
 * Receive buffer data type
 *
 * Routines to maintain a list of received packets along with their
 * source addresses, and to pull off whole packets or just a specified
 * number of bytes from the beginning (for streams).
 */

static int
rserv_recvdata_enq(struct sockinfo *si, struct pbuf *p,
                   struct ip_addr *fromaddr, u16_t fromport)
{
	struct recv_buf *rb;

	DPRINTF("rserv_recvdata_enq: s=%d p=%p tot_len=%d\n", S(si), (void *)p, p->tot_len);

	if ((rb = RSERV_MALLOC(sizeof(struct recv_buf))) == NULL) {
		printf("rserv_recv_data_enq: out of memory\n");		/* actually fatal */
		return -1;
	}

	rb->p = p;

	if (fromaddr) {
		IP_ADDR_COPY(&rb->fromaddr, fromaddr);
		rb->fromport = fromport;
	} else {
		IP_ADDR_ZERO(&rb->fromaddr);
		rb->fromport = 0;
	}

	rb->next = NULL;

	if (si->recv_q_head == NULL)
		si->recv_q_head = rb;
	else
		si->recv_q_tail->next = rb;

	si->recv_q_tail = rb;
	si->recv_q_total += p->tot_len;

	DPRINTF("rserv_recvdata_enq: s=%d recv_q_total=%d\n", S(si), si->recv_q_total);

	return 0;
}

static int
rserv_recvdata_deq_packet(struct sockinfo *si, void *data, int len_max,
                          struct ip_addr *fromaddr, u16_t *fromport)
{
	struct recv_buf *rb;
	int len;
	struct pbuf *p;

	/* Dequeue one recv_buf (one packet) */

	if ((rb = si->recv_q_head) == NULL)
		return 0;

	si->recv_q_head = rb->next;

	if (si->recv_q_head == NULL)
		si->recv_q_tail = NULL;

	p = rb->p;

	si->recv_q_total -= p->tot_len;

	/* Return data and free recv_buf */

	len = p->tot_len;
	if (len > len_max)
		len = len_max;

	if (data) {
		int clen = pbuf_copyout(p, data, 0, len);
		if (clen != len)
			ASSERT(0);	/* (Assert on expr results in unused var clen) */
	}

	if (fromaddr)
		IP_ADDR_COPY(fromaddr, &rb->fromaddr);

	if (fromport)
		*fromport = rb->fromport;

	pbuf_free(p);

	RSERV_FREE(rb, sizeof(struct recv_buf));

	return len;
}

#if LWIP_TCP
static int
rserv_recvdata_deq_bytes(struct sockinfo *si, void *buf, int len)
{
	int n, clen;
	struct recv_buf *rb;
	struct pbuf *p;
	int outptr = 0;

	while (len > 0 && (rb = si->recv_q_head) != NULL) {
		p = rb->p;

		if ((n = p->tot_len - si->recv_q_hoff) > len)
			n = len;

		clen = pbuf_copyout(p, (uint8 *)buf + outptr, si->recv_q_hoff, n);
		ASSERT(clen == n);

		outptr += n;
		len -= n;

		si->recv_q_hoff += n;
		if (si->recv_q_hoff == p->tot_len) {
			rserv_recvdata_deq_packet(si, NULL, 0, NULL, NULL);
			si->recv_q_hoff = 0;
		}
	}

	return outptr;
}

/*
 * Simple queue to hold blocking requests.
 * Efficient enough for a small number of requests.
 */

static int
rserv_req_enq(struct sockinfo *si, RSERV_PKT *req_pkt)
{
	DPRINTF("rserv_req_enq: s=%d req_pkt=%p\n", S(si), (void *)req_pkt);

	if (si->req_pend_count == MAX_REQ_PER_SOCKET)
		return 0;

	if (si->req_pend != NULL)
		RSERV_PKTSETLINK(si->req_pend_tail, req_pkt);
	else
		si->req_pend = req_pkt;

	si->req_pend_tail = req_pkt;

	si->req_pend_count++;

	return 1;
}

static RSERV_PKT *
rserv_req_deq(struct sockinfo *si, int fn)
{
	RSERV_PKT *prev, *req_pkt;

	/* Search for first request with matching fn */

	prev = NULL;

	for (req_pkt = si->req_pend; req_pkt != NULL; req_pkt = RSERV_PKTLINK(req_pkt)) {
		union rsock_req *req = RSERV_PKTDATA(req_pkt);
		if (btohs(req->header.fn) == fn)
			break;
		prev = req_pkt;
	}

	if (req_pkt == NULL) {
		DPRINTF("rserv_req_deq: s=%d fn=%d not found\n", S(si), fn);
		return NULL;
	}

	/* Dequeue and return */
	if (prev == NULL)
		si->req_pend = NULL;
	else
		RSERV_PKTSETLINK(prev, RSERV_PKTLINK(req_pkt));

	if (req_pkt == si->req_pend_tail)
		si->req_pend_tail = prev;

	si->req_pend_count--;

	RSERV_PKTSETLINK(req_pkt, NULL);

	DPRINTF("rserv_req_deq: s=%d fn=%d req_pkt=%p\n", S(si), fn, (void *)req_pkt);

	return req_pkt;
}

/*
 * Allocate a new socket, place descriptor in s_new.
 * Returns err_no; if non-zero, s_new will be set to -1.
 */

static int
rserv_socket_alloc(int *s_new, int domain, int type, int proto)
{
	struct sockinfo *si;
	int s;

	*s_new = -1;

	for (s = 0; s < RSOCK_SOCKET_MAX; s++)
		if (sockinfo[s].domain < 0)
			break;

	if (s == RSOCK_SOCKET_MAX)
		return RSOCK_ENOMEM;

	si = SI(s);

	memset(si, 0, sizeof(*si));

	si->domain = domain;
	si->type = type;
	si->proto = proto;
	si->backlog_next = -1;

	*s_new = s;

	return 0;
}

/*
 * lwip callback functions for TCP connection
 */

static void
rserv_do_accept_response(struct sockinfo *si, RSERV_PKT *req_pkt, int err_no)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;
	struct tcp_pcb *pcb;
	int s_new = -1;
	struct sockinfo *si_new;

	req = RSERV_PKTDATA(req_pkt);

	resp_pkt = RSERV_PKTGET((int)sizeof(resp->accept), TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	memset(resp, 0, sizeof(resp->accept));

	if (err_no != 0)
		goto done;

	/* Remove the first socket from the backlog queue */
	s_new = si->backlog_next;
	ASSERT(s_new >= 0);

	si_new = SI(s_new);

	si->backlog_next = si_new->backlog_next;
	si_new->backlog_next = -1;

	pcb = si_new->pcb.tcp;

	DPRINTF("rserv_do_accept_response: s=%d s_new=%d pcb=%p remote_ip=0x%x port=%d\n",
	        S(si), s_new, (void *)pcb,
	        *(unsigned int *)&pcb->remote_ip, pcb->remote_port);

	sockaddr_set(&resp->accept.addr, &pcb->remote_ip, pcb->remote_port);
	resp->accept.addrlen = htobl(sizeof(struct sockaddr));

done:
	RESP_HEADER(resp, req, sizeof(resp->accept), err_no);

	resp->accept.s = htobl(s_new);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_connect_response(struct sockinfo *si, RSERV_PKT *req_pkt, int err_no)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;

	req = RSERV_PKTDATA(req_pkt);

	resp_pkt = RSERV_PKTGET((int)sizeof(resp->connect), TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	RESP_HEADER(resp, req, sizeof(resp->connect), err_no);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_send_response(int type, RSERV_PKT *req_pkt, int err_no)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;

	DPRINTF("rserv_do_send_response: req_pkt=%p err_no=%d\n",
	        (void *)req_pkt, err_no);

	req = RSERV_PKTDATA(req_pkt);

	if (type == SOCK_STREAM) {
		int sent_len;

		/* Returned sent_len is all or nothing, and currently ignored by client */
		sent_len = ((int)(btohs(req->send_stream.header.len) -
		                  sizeof(req->send_stream)));

		resp_pkt = RSERV_PKTGET((int)sizeof(resp->send_stream), TRUE);
		resp = RSERV_PKTDATA(resp_pkt);

		RESP_HEADER(resp, req, sizeof(resp->send_stream), err_no);

		resp->send_stream.sent_len = htobl(sent_len);
	} else {
		resp_pkt = RSERV_PKTGET((int)sizeof(resp->send_dgram), TRUE);
		resp = RSERV_PKTDATA(resp_pkt);

		RESP_HEADER(resp, req, sizeof(resp->send_dgram), err_no);
	}

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

#if DEBUG_ENABLE
static void
rserv_senddata_dump(struct sockinfo *si)
{
	RSERV_PKT *p;
	int hdrlen = SEND_HDRLEN(si->type);
	int total_pkts, total_bytes;

	printf("SENDDATA:");

	total_pkts = 0;
	total_bytes = 0;

	for (p = si->send_q_head; p != NULL; p = RSERV_PKTLINK(p)) {
		total_pkts++;
		total_bytes += RSERV_PKTLEN(p) - hdrlen;
		printf(" %d", RSERV_PKTLEN(p) - hdrlen);
		if (p == si->send_q_head)
			printf("!%d", si->send_q_aoff - hdrlen);
		if (p == si->send_q_spkt)
			printf("@%d", si->send_q_soff - hdrlen);
	}

	printf(" total %d\n", total_bytes);
}
#endif /* DEBUG_ENABLE */

/* Each queue entry is an entire send request, including the rsock protocol header */
static void
rserv_senddata_enq(struct sockinfo *si, RSERV_PKT *req_pkt)
{
	STAMP(60);
	DPRINTF("rserv_senddata_enq: s=%d req_pkt=%p len=%d\n",
	        S(si), (void *)req_pkt, RSERV_PKTLEN(req_pkt));

	if (si->send_q_head == NULL) {
		si->send_q_head = req_pkt;
		si->send_q_aoff = SEND_HDRLEN(si->type);
	} else
		RSERV_PKTSETLINK(si->send_q_tail, req_pkt);

	if (si->send_q_spkt == NULL) {
		si->send_q_spkt = req_pkt;
		si->send_q_soff = SEND_HDRLEN(si->type);
	}

	si->send_q_tail = req_pkt;
	si->send_q_len++;

#if DEBUG_ENABLE
	rserv_senddata_dump(si);
#endif
}

/* Return pointer to next unsent data and length of contiguous amount available */
static void *
rserv_senddata_unsent(struct sockinfo *si, int *len)
{
	if (si->send_q_spkt == NULL) {
		*len = 0;
		return NULL;
	}

	ASSERT(si->send_q_soff >= SEND_HDRLEN(si->type));

	*len = RSERV_PKTLEN(si->send_q_spkt) - si->send_q_soff;

	ASSERT(*len > 0);

	return (void *)((char *)RSERV_PKTDATA(si->send_q_spkt) + si->send_q_soff);
}

/*
 * Mark a given number of bytes in the send queue as having been sent.
 * len must be less than or equal to that returned by rserv_senddata_unsent().
 */
static void
rserv_senddata_sent(struct sockinfo *si, int len)
{
	DPRINTF("rserv_senddata_sent: s=%d len=%d\n", S(si), len);

	if (len == 0)
		return;

	ASSERT(si->send_q_spkt != NULL);
	ASSERT(si->send_q_soff + len <= RSERV_PKTLEN(si->send_q_spkt));

	if (si->send_q_soff + len < RSERV_PKTLEN(si->send_q_spkt))
		si->send_q_soff += len;
	else {
		si->send_q_spkt = RSERV_PKTLINK(si->send_q_spkt);
		si->send_q_soff = SEND_HDRLEN(si->type);
	}

#if DEBUG_ENABLE
	rserv_senddata_dump(si);
#endif
}

/*
 * Remove len bytes of data from the head of the send queue.
 * Adjusts aoff pointer.  Whenever an entire request packet is
 * consumed, a send response is sent and the request is freed.
 */
static void
rserv_senddata_acked(struct sockinfo *si, int len)
{
	RSERV_PKT *req_pkt;
	int req_pkt_len, n;

	DPRINTF("rserv_senddata_acked: s=%d len=%d\n", S(si), len);

	while (len > 0) {
		req_pkt = si->send_q_head;

		/* Underflow would be caused by acking more than is in the queue */
		ASSERT(req_pkt != NULL);

		req_pkt_len = RSERV_PKTLEN(req_pkt);

		/* Find number of unacked data bytes in head request packet */
		n = req_pkt_len - si->send_q_aoff;

		/* Finish up if len is less than remaining bytes in head request packet */
		if (len < n) {
			si->send_q_aoff += len;
			break;
		}

		/* Should never free packet containing unsent data */
		ASSERT(si->send_q_spkt != req_pkt);

		/* Remove head request packet from the queue */
		if ((si->send_q_head = RSERV_PKTLINK(req_pkt)) == NULL)
			si->send_q_tail = NULL;

		si->send_q_len--;

		RSERV_PKTSETLINK(req_pkt, NULL);

		/* Send response and free request */
		rserv_do_send_response(si->type, req_pkt, 0);

		/* Reset aoff to beginning of next packet */
		si->send_q_aoff = SEND_HDRLEN(si->type);

		len -= n;
	}

#if DEBUG_ENABLE
	rserv_senddata_dump(si);
#endif
}

/* Try to write as much send data as possible */
static void
rserv_senddata_start(struct sockinfo *si)
{
	int tcp_max, len, data_len, err;
	void *data;

	for (;;) {
		tcp_max = tcp_sndbuf(si->pcb.tcp);
		data = rserv_senddata_unsent(si, &data_len);

		DPRINTF("rserv_senddata_start: s=%d tcp_max=%d data_len=%d\n",
		        S(si), tcp_max, data_len);

		/* No more data or room to put it? */
		if ((len = LWIP_MIN(data_len, tcp_max)) == 0)
			break;

		/*
		 * Even though the amount was checked with tcp_sndbuf, tcp_write may
		 * still fail if the outgoing segment queue is full.  In any case,
		 * senddata will be retried the next time lwIP calls rserv_tcp_sent.
		 */

		DPRINTF("rserv_senddata_start: s=%d tcp_write(%d)\n", S(si), len);

		STAMP(70);
		if ((err = tcp_write(si->pcb.tcp, data, len, 0)) != ERR_OK) {
			if (err != ERR_MEM)
				si->err = err;
			break;
		}

#ifdef SHOW_PKTS
		printf("wrote(%d)\n", len);
#endif


		rserv_senddata_sent(si, len);

		/*
		 * Nagle algorithm: inhibit the sending of new TCP segments if
		 * there is an outstanding ACK and there is less than one full
		 * segment of data on the send queue.
		 */

		if (si->pcb.tcp->unacked == NULL ||
		    (si->pcb.tcp->flags & TF_NODELAY) ||
		    (si->pcb.tcp->snd_queuelen > 1)) {
			STAMP(80);
			tcp_output(si->pcb.tcp);
		}
	}
}
#endif /* LWIP_TCP */

/*
 * Send a receive data message to the client.
 *
 * If a TCP connection has been closed, that is indicated to the client by sending a
 * zero-length packet.
 */

static void
rserv_do_rdata(struct sockinfo *si)
{
	RSERV_PKT *rdata_pkt;
	int rdata_pkt_len;
	union rsock_rdata *rdata = NULL;
	int ret_len = 0, recv_len;
#if (LWIP_RAW || LWIP_UDP)
	struct ip_addr fromaddr;
	u16_t fromport = 0;
#endif

	DPRINTF("rserv_do_rdata: type=%d unacked=%d flags=0x%x\n",
	        si->type, si->recv_q_unacked, si->recv_q_flags);

	/* Cap the number of outstanding responses */
	if (si->recv_q_unacked >= RSOCK_BUS_RECV_WIN)
		return;

	/* Do nothing if a "connection closed" indication has already been sent */
	if (si->recv_q_flags & RECV_Q_CLOSED)
		return;

	/* Do nothing if receive queue not yet enabled */
	if (!(si->recv_q_flags & RECV_Q_ENABLE))
		return;

	/*
	 * If the connection is closing and all data has been sent, send
	 * an empty response to indicate "connection closed."
	 */
	if ((si->recv_q_flags & RECV_Q_CLOSING) && si->recv_q_total == 0) {
		si->recv_q_flags |= RECV_Q_CLOSED;
		recv_len = 0;
	} else {
		/* Send the next data.  Do nothing if no data is queued. */
		if (si->recv_q_total == 0)
			return;

		if (si->type == SOCK_STREAM)
			recv_len = si->recv_q_total - si->recv_q_hoff;
		else {
			ASSERT(si->recv_q_head != NULL);
			recv_len = si->recv_q_head->p->tot_len;
		}
	}

	switch (si->type) {
#if (LWIP_RAW || LWIP_UDP)
	case SOCK_RAW:
	case SOCK_DGRAM:
		recv_len = LWIP_MIN(recv_len, RSOCK_RDATA_DGRAM_LIMIT);
		DPRINTF("rserv_do_rdata: dgram s=%d recv_len=%d\n", S(si), recv_len);

		rdata_pkt_len = (int)sizeof(rdata->dgram) + recv_len;
		rdata_pkt = RSERV_PKTGET(rdata_pkt_len, TRUE);
		rdata = RSERV_PKTDATA(rdata_pkt);

		/* Packet is truncated to recv_max, if longer than that */
		ret_len = rserv_recvdata_deq_packet(si,
		                                    &rdata->dgram + 1, recv_len,
		                                    &fromaddr, &fromport);
		ASSERT(ret_len == recv_len);

		sockaddr_set(&rdata->dgram.from, &fromaddr, fromport);
		rdata->dgram.fromlen = htobl((int)sizeof(fromaddr));

		break;
#endif /* (LWIP_RAW || LWIP_UDP) */

#if LWIP_TCP
	case SOCK_STREAM:
		recv_len = LWIP_MIN(recv_len, RSOCK_RDATA_STREAM_LIMIT);
		DPRINTF("rserv_do_rdata: stream s=%d recv_len=%d\n", S(si), recv_len);

		rdata_pkt_len = (int)sizeof(rdata->stream) + recv_len;
		rdata_pkt = RSERV_PKTGET(rdata_pkt_len, TRUE);
		rdata = RSERV_PKTDATA(rdata_pkt);

		/* Return as many bytes as available, up to len_max */
		ret_len = rserv_recvdata_deq_bytes(si, &rdata->stream + 1, recv_len);
		ASSERT(ret_len == recv_len);

		DPRINTF("rserv_do_rdata: stream s=%d deq %d bytes ==> got %d\n",
		        S(si), recv_len, ret_len);

		/* Notify lwIP data is taken */
		if (ret_len > 0)
			tcp_recved(si->pcb.tcp, ret_len);

		break;
#endif /* LWIP_TCP */

	default:
		rdata_pkt_len = 0;
		rdata_pkt = NULL;
		ASSERT(0);
		break;
	}

	rdata->header.flags = htobs(RSOCK_FLAG_TYPE_RDATA |
	                            (RSERV_PVER << RSOCK_FLAG_PVER_SHIFT));
	rdata->header.len = htobs((uint16)rdata_pkt_len);
	rdata->header.s = htobs(S(si));
	rdata->header._pad = 0;

#ifdef EXT_CBALL
	RateStat("rserv", "recv", ret_len);
	if ((RateStat_recv += ret_len) > 10000)
		RateStatTrigger();
#endif

	RSERV_OUTPUT(rdata_pkt);

	/* Keep track of how many rdata messages are outstanding */
	si->recv_q_unacked++;
}

static int
rserv_socket_close(int s, int force)
{
	struct sockinfo *si = SI(s);
	struct recv_buf *rb;
	RSERV_PKT *req_pkt;
	int err_no = 0;

	/* Error out any pending requests on the socket (accept, connect) */

	while ((req_pkt = rserv_req_deq(si, RSOCK_FN_ACCEPT)) != NULL)
		rserv_do_accept_response(si, req_pkt, RSOCK_EBADF);

	while ((req_pkt = rserv_req_deq(si, RSOCK_FN_CONNECT)) != NULL)
		rserv_do_connect_response(si, req_pkt, RSOCK_EBADF);

	/* Discard pending receive data */
	while ((rb = si->recv_q_head) != NULL) {
		si->recv_q_head = rb->next;
		pbuf_free(rb->p);
		RSERV_FREE(rb, sizeof(struct recv_buf));
	}

	si->recv_q_tail = NULL;
	si->recv_q_total = 0;
	si->recv_q_flags |= RECV_Q_CLOSED;

	switch (si->type) {
#if LWIP_RAW
	case SOCK_RAW:
		raw_recv(si->pcb.raw, NULL, NULL);
		raw_remove(si->pcb.raw);
		break;
#endif /* LWIP_RAW */

	case SOCK_DGRAM:
#if LWIP_UDP
		if (si->domain == PF_INET) {
			si->pcb.udp->recv_arg = NULL;
			udp_recv(si->pcb.udp, NULL, NULL);
			udp_remove(si->pcb.udp);
		}
#endif /* LWIP_UDP */
		break;

#if LWIP_TCP
	case SOCK_STREAM:
		{
			int bs, *pbs, err;

			/* Close any socket(s) on backlog list, dequeueing from tail to head */
			while ((bs = *(pbs = &si->backlog_next)) >= 0) {
				while (SI(bs)->backlog_next >= 0)
					bs = *(pbs = &SI(bs)->backlog_next);
				*pbs = -1;
				if ((err_no = rserv_socket_close(bs, force)) != 0)
					return err_no;
			}

			/*
			 * Check if there is still pending send data.  This would
			 * normally never happen since rsock_close() waits for all
			 * send responses.
			 */
			if (si->send_q_head != NULL) {
				RSERV_PKT *p;

				/* If not forcing a close, have client retry the close */
				if (!force) {
					DPRINTF("rserv_socket_close: s=%d send_q_len=%d\n",
					        s, si->send_q_len);
					return RSOCK_EAGAIN;
				}

				/* Free rest of send queue */
				while ((p = si->send_q_head) != NULL) {
					si->send_q_head = RSERV_PKTLINK(si->send_q_head);
					RSERV_PKTSETLINK(p, NULL);
					RSERV_PKTFREE(p, 0);
				}

				si->send_q_tail = NULL;
				si->send_q_len = 0;
			}

			/*
			 * lwIP may still keep the pcb allocated and in use internally
			 * even after tcp_close() is called, and may continue to make
			 * callbacks as long as it exists.
			 */
			tcp_arg(si->pcb.tcp, NULL);
			tcp_accept(si->pcb.tcp, NULL);
			tcp_recv(si->pcb.tcp, NULL);
			tcp_sent(si->pcb.tcp, NULL);
			tcp_poll(si->pcb.tcp, NULL, 0);
			tcp_err(si->pcb.tcp, NULL);

			/*
			 * Memory errors can happen during tcp_close() and the close must
			 * be retried (but because we're waiting for all transmit data to
			 * be acked with tcp_sent(), this probably won't be likely).
			 */

			if ((err = tcp_close(si->pcb.tcp)) == ERR_MEM) {
				if (!force) {
					DPRINTF("rserv_socket_close: s=%d mem err retry\n", s);
					return RSOCK_EAGAIN;
				}

				tcp_abort(si->pcb.tcp);
				err = 0;
			}

			err_no = err_xlate(err);
		}
		break;
#endif /* LWIP_TCP */

	default:
		ASSERT(0);
		break;
	}

	DPRINTF("rserv_socket_close: clear socket\n");

	memset(si, 0, sizeof(*si));

	si->domain = PF__INVAL;

	return err_no;
}

#if LWIP_TCP
static err_t
rserv_tcp_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	struct sockinfo *si = arg;

	DPRINTF("rserv_tcp_recv: s=%d pcb=%p p=%p\n", S(si), (void *)pcb, (void *)p);

	ASSERT(si != NULL);
	ASSERT(si->type == SOCK_STREAM);

	if (p == NULL)
		si->recv_q_flags |= RECV_Q_CLOSING;
	else
		rserv_recvdata_enq(si, p, NULL, 0);

	rserv_do_rdata(si);

	return ERR_OK;
}

/*
 * This callback is called from lwIP whenever a connection is accepted.
 * Allocate a new socket and put it on the backlog queue.
 * Then if an accept() request is pending, complete the request.
 */

static err_t
rserv_tcp_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	struct sockinfo *si = arg;
	struct sockinfo *si_new;
	RSERV_PKT *req_pkt;
	int s_new, n;
	int *pnext;

	DPRINTF("rserv_tcp_accept: s=%d err=%d newpcb=%p\n", S(si), err, (void *)newpcb);

	/* Count connections in backlog and find tail pointer */
	n = 0;
	for (pnext = &si->backlog_next; *pnext != -1; pnext = &SI(*pnext)->backlog_next)
		n++;

	/* Check if backlog is full */
	if (n >= si->backlog_max)
		return ERR_MEM;

	/* Open a new socket */
	if (rserv_socket_alloc(&s_new, si->domain, si->type, si->proto) != 0)
		return ERR_MEM;

	/* Enqueue the connection at end of backlog list. */
	DPRINTF("rserv_tcp_accept: s=%d new socket %d on backlog\n", S(si), s_new);

	*pnext = s_new;

	si_new = SI(s_new);

	si_new->pcb.tcp = newpcb;

	tcp_arg(newpcb, si_new);
	tcp_recv(newpcb, rserv_tcp_recv);

	DPRINTF("rserv_tcp_accept: s=%d check for pending accept\n", S(si));

	/* If there is an outstanding ACCEPT request, send a response for it immediately */
	if ((req_pkt = rserv_req_deq(si, RSOCK_FN_ACCEPT)) != NULL)
		rserv_do_accept_response(si, req_pkt, 0);

	return ERR_OK;
}

static err_t
rserv_tcp_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
	struct sockinfo *si = arg;

#ifdef SHOW_PKTS
	printf("sent(%d)\n", len);
#endif

	STAMP(130);
	DPRINTF("rserv_tcp_sent: len=%d\n", len);

	rserv_senddata_acked(si, len);
	rserv_senddata_start(si);

	return ERR_OK;
}

static void
rserv_tcp_err(void *arg, err_t err)
{
	struct sockinfo *si = arg;

	if (si != NULL)
		si->err = err;
}

static err_t
rserv_tcp_poll(void *arg, struct tcp_pcb *pcb)
{
	struct sockinfo *si = arg;

	if (si == NULL)
		return ERR_VAL;

	return tcp_output(si->pcb.tcp);
}

static err_t
rserv_tcp_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
	struct sockinfo *si = arg;
	RSERV_PKT *req_pkt;

	if (si == NULL)
		return ERR_VAL;

	si->err = err;

	req_pkt = rserv_req_deq(si, RSOCK_FN_CONNECT);
	ASSERT(req_pkt != NULL);

	rserv_do_connect_response(si, req_pkt, err_xlate(err));

	return ERR_OK;
}
#endif /* LWIP_TCP */

#if LWIP_RAW
/*
 * lwip callback functions for raw connection
 */

static u8_t
rserv_raw_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p,
               struct ip_addr *addr)
{
	struct sockinfo *si = arg;

	if (si == NULL)
		return ERR_VAL;

	/* For raw socket, lwIP uses fromport to return protocol */
	rserv_recvdata_enq(si, p, addr, pcb->protocol);

	/* Do not eat the packet; it's next passed to other protocols */
	pbuf_ref(p);

	rserv_do_rdata(si);

	return 0;
}
#endif /* LWIP_RAW */

#if LWIP_UDP
/*
 * lwip callback functions for UDP connection
 */

static void
rserv_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
               struct ip_addr *addr, u16_t port)
{
	struct sockinfo *si = arg;

	if (si != NULL)
		rserv_recvdata_enq(si, p, addr, port);

	rserv_do_rdata(si);

}
#endif /* LWIP_UDP */

/*
 * Remote function service handlers
 */

static void
rserv_do_accept(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	int s;
	struct sockinfo *si = NULL;
	int err_no = 0;

	req = RSERV_PKTDATA(req_pkt);

	s = btohl(req->accept.s);

	SOCKET_CHECK(s);

	si = SI(s);

	switch (si->type) {
#if LWIP_RAW
	case SOCK_RAW:
		err_no = RSOCK_EINVAL;
		break;
#endif

#if LWIP_UDP
	case SOCK_DGRAM:
		err_no = RSOCK_EINVAL;
		break;
#endif

#if LWIP_TCP
	case SOCK_STREAM:
		/*
		 * If there's a connection waiting in the backlog, accept it immediately.
		 * Else, if the socket is SO_NONBLOCK, return EAGAIN.  Else, place the
		 * request on pending list to block until rserv_tcp_accept() is called back.
		 */
		DPRINTF("rserv_do_accept: s=%d backlog_next=%d\n", S(si), si->backlog_next);

		if (si->backlog_next < 0) {
			if (si->options & SO_NONBLOCK)
				err_no = RSOCK_EAGAIN;
			else if (rserv_req_enq(si, req_pkt))
				return;		/* block */
			else
				err_no = RSOCK_ENOMEM;
		}
		break;
#endif /* LWIP_TCP */

	default:
		ASSERT(0);
		break;
	}

done:
	rserv_do_accept_response(si, req_pkt, err_no);
}

static void
rserv_do_flow(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;
	int s;
	struct sockinfo *si = NULL;
	int err_no = 0;
	uint32 status;

	req = RSERV_PKTDATA(req_pkt);

	resp_pkt = RSERV_PKTGET((int)sizeof(resp->flow), TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	memset(resp, 0, sizeof(resp->flow));

	s = btohl(req->flow.s);

	SOCKET_CHECK(s);

	si = SI(s);

	status = btohl(req->flow.status);

	if (status & RSOCK_FLOW_XON)
		si->recv_q_flags |= RECV_Q_ENABLE;
	else
		si->recv_q_flags &= ~RECV_Q_ENABLE;

	/* Kick off the first rdata; rack's will kick off the rest */
	rserv_do_rdata(si);

	err_no = 0;

done:
	RESP_HEADER(resp, req, sizeof(resp->flow), err_no);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_bind(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;
	int s;
	struct sockinfo *si = NULL;
	int err_no = 0;
	struct ip_addr ipaddr;
	uint16 port;

	req = RSERV_PKTDATA(req_pkt);

	resp_pkt = RSERV_PKTGET((int)sizeof(resp->bind), TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	memset(resp, 0, sizeof(resp->bind));

	s = btohl(req->bind.s);

	SOCKET_CHECK(s);

	si = SI(s);

	if (btohl(req->bind.addrlen) != sizeof(struct sockaddr)) {
		err_no = RSOCK_EINVAL;
		goto done;
	}

	sockaddr_get(&req->bind.addr, &ipaddr, &port);

	switch (si->type) {
#if LWIP_RAW
	case SOCK_RAW:
		err_no = err_xlate(raw_bind(si->pcb.raw, &ipaddr));
		break;
#endif

	case SOCK_DGRAM:
#if LWIP_UDP
		if (si->domain == PF_INET)
			err_no = err_xlate(udp_bind(si->pcb.udp, &ipaddr, port));
		else
			err_no = RSOCK_EINVAL;
#endif
		break;

#if LWIP_TCP
	case SOCK_STREAM:
		err_no = err_xlate(tcp_bind(si->pcb.tcp, &ipaddr, port));
		break;
#endif

	default:
		ASSERT(0);
		break;
	}

done:
	RESP_HEADER(resp, req, sizeof(resp->bind), err_no);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_shutdown(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;
	int s;
	int err_no = 0;

	req = RSERV_PKTDATA(req_pkt);

	resp_pkt = RSERV_PKTGET((int)sizeof(resp->shutdown), TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	memset(resp, 0, sizeof(resp->shutdown));

	s = btohl(req->shutdown.s);

	SOCKET_CHECK(s);

	/* lwip currently does not implement shutdown */
	err_no = RSOCK_ENOSYS;

done:
	RESP_HEADER(resp, req, sizeof(resp->shutdown), err_no);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_getname(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;
	int s;
	struct sockinfo *si = NULL;
	int err_no = 0;

	req = RSERV_PKTDATA(req_pkt);

	resp_pkt = RSERV_PKTGET((int)sizeof(resp->getname), TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	memset(resp, 0, sizeof(resp->getname));

	s = btohl(req->getname.s);

	SOCKET_CHECK(s);

	si = SI(s);

	switch (si->type) {
#if LWIP_RAW
	case SOCK_RAW:
		/* port is being used to return protocol, and there is no remote address */
		sockaddr_set(&resp->getname.local_addr,
		             &si->pcb.raw->local_ip, si->pcb.raw->protocol);
		resp->getname.local_addrlen = htobl(sizeof(resp->getname.local_addr));
		break;
#endif

	case SOCK_DGRAM:
#if LWIP_UDP
		if (si->domain != PF_INET)
			break;
		sockaddr_set(&resp->getname.local_addr,
		             &si->pcb.udp->local_ip, si->pcb.udp->local_port);
		resp->getname.local_addrlen = htobl(sizeof(resp->getname.local_addr));
		sockaddr_set(&resp->getname.remote_addr,
		             &si->pcb.udp->remote_ip, si->pcb.udp->remote_port);
		resp->getname.remote_addrlen = htobl(sizeof(resp->getname.remote_addr));
#endif
		break;

#if LWIP_TCP
	case SOCK_STREAM:
		sockaddr_set(&resp->getname.local_addr,
		             &si->pcb.tcp->local_ip, si->pcb.tcp->local_port);
		resp->getname.local_addrlen = htobl(sizeof(resp->getname.local_addr));
		sockaddr_set(&resp->getname.remote_addr,
		             &si->pcb.tcp->remote_ip, si->pcb.tcp->remote_port);
		resp->getname.remote_addrlen = htobl(sizeof(resp->getname.remote_addr));
		break;
#endif

	default:
		ASSERT(0);
		break;
	}

done:
	RESP_HEADER(resp, req, sizeof(resp->getname), err_no);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_getsockopt(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;
	int s, level, optname, optval = 0;
	struct sockinfo *si = NULL;
	int err_no = RSOCK_ENOPROTOOPT;

	req = RSERV_PKTDATA(req_pkt);

	resp_pkt = RSERV_PKTGET((int)sizeof(resp->getsockopt), TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	memset(resp, 0, sizeof(resp->getsockopt));

	s = btohl(req->getsockopt.s);

	SOCKET_CHECK(s);

	/* Currently, every supported option returns a single 'int' value */

	level = btohl(req->getsockopt.level);
	optname = btohl(req->getsockopt.optname);

	si = SI(s);

	switch (level) {
	case SOL_SOCKET:
		switch (optname) {
		case SO_BROADCAST:
		case SO_KEEPALIVE:
		case SO_REUSEADDR:
		case SO_REUSEPORT:
			/* so_options is in the same place in each pcb type */
			optval = (si->pcb.tcp->so_options & optname);
			err_no = 0;
			break;
		case SO_TYPE:
			optval = si->type;
			err_no = 0;
			break;
		case SO_ERROR:
			optval = err_xlate(si->err);
			err_no = 0;
			break;
		}
		break;

	case IPPROTO_IP:
		switch (optname) {
		case IP_TTL:
			/* ttl is in the same place in each pcb type */
			optval = si->pcb.tcp->ttl;
			err_no = 0;
			break;

		case IP_TOS:
			/* tos is in the same place in each pcb type */
			optval = si->pcb.tcp->tos;
			err_no = 0;
			break;
		}
		break;

#if LWIP_TCP
	case IPPROTO_TCP:
		if (si->type != SOCK_STREAM)
			break;

		switch (optname) {
		case TCP_NODELAY:
			optval = (si->pcb.tcp->flags & TF_NODELAY) != 0;
			err_no = 0;
			break;

		case TCP_KEEPALIVE:
			optval = si->pcb.tcp->keepalive;
			err_no = 0;
			break;
		}

		break;
#endif /* LWIP_TCP */
	}

	*(uint32 *)resp->getsockopt.optval = htobl(optval);
	resp->getsockopt.optlen = htobl(sizeof(optval));

done:
	RESP_HEADER(resp, req, sizeof(resp->getsockopt), err_no);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_setsockopt(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;
	int s, level, optname, optval;
	struct sockinfo *si = NULL;
	int err_no = RSOCK_ENOPROTOOPT;

	req = RSERV_PKTDATA(req_pkt);

	resp_pkt = RSERV_PKTGET((int)sizeof(resp->setsockopt), TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	memset(resp, 0, sizeof(resp->setsockopt));

	s = btohl(req->setsockopt.s);

	SOCKET_CHECK(s);

	/* Currently, every supported option requires a single 'int' value */

	level = btohl(req->setsockopt.level);
	optname = btohl(req->setsockopt.optname);
	optval = btohl(*(uint32 *)req->setsockopt.optval);

	si = SI(s);

	switch (level) {
	case SOL_SOCKET:
		switch (optname) {
		case SO_BROADCAST:
		case SO_KEEPALIVE:
		case SO_REUSEADDR:
		case SO_REUSEPORT:
			/* so_options is in the same place in each pcb type */
			if (optval)
				si->pcb.tcp->so_options |= optname;
			else
				si->pcb.tcp->so_options &= ~optname;
			err_no = 0;
			break;
		}
		break;

	case IPPROTO_IP:
		switch (optname) {
		case IP_TTL:
			/* ttl is in the same place in each pcb type */
			si->pcb.tcp->ttl = (u8_t)optval;
			err_no = 0;
			break;

		case IP_TOS:
			/* tos is in the same place in each pcb type */
			si->pcb.tcp->tos = (u8_t)optval;
			err_no = 0;
			break;
		}
		break;

#if LWIP_TCP
	case IPPROTO_TCP:
		if (si->type != SOCK_STREAM)
			break;

		switch (optname) {
		case TCP_NODELAY:
			if (optval)
				si->pcb.tcp->flags |= TF_NODELAY;
			else
				si->pcb.tcp->flags &= ~TF_NODELAY;
			err_no = 0;
			break;

		case TCP_KEEPALIVE:
			si->pcb.tcp->keepalive = (u32_t)optval;
			err_no = 0;
			break;
		}

		break;
#endif /* LWIP_TCP */
	}

done:
	RESP_HEADER(resp, req, sizeof(resp->setsockopt), err_no);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_close(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;
	int s;
	struct sockinfo *si = NULL;
	int err_no = 0;
	uint16 flags;

	req = RSERV_PKTDATA(req_pkt);

	resp_pkt = RSERV_PKTGET((int)sizeof(resp->close), TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	memset(resp, 0, sizeof(resp->close));

	s = btohl(req->close.s);

	SOCKET_CHECK(s);

	si = SI(s);

	flags = btohs(req->header.flags);

	err_no = rserv_socket_close(s, flags & RSOCK_FLAG_FORCE);


done:
	RESP_HEADER(resp, req, sizeof(resp->close), err_no);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_connect(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	int s;
	struct sockinfo *si = NULL;
	int err_no = 0;
	struct ip_addr ipaddr;
	u16_t port;

	req = RSERV_PKTDATA(req_pkt);

	s = btohl(req->connect.s);

	SOCKET_CHECK(s);

	si = SI(s);

	sockaddr_get(&req->connect.addr, &ipaddr, &port);

	switch (si->type) {
#if LWIP_RAW
	case SOCK_RAW:
		raw_connect(si->pcb.raw, &ipaddr);
		break;
#endif

#if LWIP_UDP
	case SOCK_DGRAM:
		if (si->domain == PF_INET)
			udp_connect(si->pcb.udp, &ipaddr, port);
		else
			err_no = RSOCK_EINVAL;
		break;
#endif

#if LWIP_TCP
	case SOCK_STREAM:
		/* rserv_tcp_connected() will send the response once connected. */
		tcp_connect(si->pcb.tcp, &ipaddr, port, rserv_tcp_connected);

		/* Place request on pending list to block until completion of connection */
		if (rserv_req_enq(si, req_pkt))
			return;

		err_no = RSOCK_ENOMEM;
		break;
#endif

	default:
		ASSERT(0);
		break;
	}

done:
	rserv_do_connect_response(si, req_pkt, err_no);
}

static void
rserv_do_listen(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;
	int s;
	struct sockinfo *si = NULL;
	int err_no = 0;

	req = RSERV_PKTDATA(req_pkt);

	resp_pkt = RSERV_PKTGET((int)sizeof(resp->listen), TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	memset(resp, 0, sizeof(resp->listen));

	s = btohl(req->listen.s);

	SOCKET_CHECK(s);

	si = SI(s);

	switch (si->type) {
#if LWIP_RAW
	case SOCK_RAW:
		err_no = RSOCK_EINVAL;
		break;
#endif

#if LWIP_UDP
	case SOCK_DGRAM:
		err_no = RSOCK_EINVAL;
		break;
#endif

#if LWIP_TCP
	case SOCK_STREAM:
		{
			struct tcp_pcb *pcb_new;

			si->backlog_max = btohl(req->listen.backlog);

			if ((pcb_new = tcp_listen(si->pcb.tcp)) == NULL)
				err_no = RSOCK_ENOMEM;
			else {
				si->pcb.tcp = pcb_new;
				tcp_accept(si->pcb.tcp, rserv_tcp_accept);
			}
		}
		break;
#endif

	default:
		ASSERT(0);
		break;
	}

done:
	RESP_HEADER(resp, req, sizeof(resp->listen), err_no);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_send_dgram(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	struct sockinfo *si = NULL;
	int data_len = 0;
	int err_no = 0;
	struct pbuf *p;
	void *data;
	int s;

	STAMP(51);
	req = RSERV_PKTDATA(req_pkt);

	s = btohl(req->send_dgram.s);

	SOCKET_CHECK(s);

	si = SI(s);

	if (si->type != SOCK_RAW && si->type != SOCK_DGRAM) {
		err_no = RSOCK_ESOCKTNOSUPPORT;
		goto done;
	}

	/* Send data follows request header */
	data_len = (int)(btohs(req->send_dgram.header.len) - sizeof(req->send_dgram));

	DPRINTF("rserv_do_send_dgram: s=%d data_len=%d\n", s, data_len);

	/* Check for a persistent socket error */
	if (si->err) {
		err_no = err_xlate(si->err);
		goto done;
	}

	/* Check to prevent wayward client from exceeding RSOCK_BUS_SEND_WIN */
	if (si->send_q_len == RSOCK_BUS_SEND_WIN) {
		err_no = RSOCK_ENOBUFS;
		goto done;
	}

	data = &req->send_dgram + 1;

	if ((p = pbuf_alloc(PBUF_TRANSPORT, data_len, PBUF_RAM)) == NULL)
		goto done;		/* Discard */

	memcpy(p->payload, data, data_len);

	switch (DOMAIN_TYPE(si->domain, si->type)) {
	case DOMAIN_TYPE(PF_RSOCK, SOCK_DGRAM):
		RSERV_IFCONTROL(p->payload, p->len);	/* Multi-buffer payload not supported */
		rserv_recvdata_enq(si, p, NULL, 0);
		pbuf_ref(p);
		rserv_do_rdata(si);
		break;

#if LWIP_RAW
	case DOMAIN_TYPE(PF_INET, SOCK_RAW):
		raw_send(si->pcb.raw, p);
		break;
#endif

#if LWIP_UDP
	case DOMAIN_TYPE(PF_INET, SOCK_DGRAM):
		if (req->send_dgram.tolen == 0)
			udp_send(si->pcb.udp, p);
		else {
			struct ip_addr ipaddr;
			uint16 port;
			sockaddr_get(&req->send_dgram.to, &ipaddr, &port);
			udp_sendto(si->pcb.udp, p, &ipaddr, port);
		}
		break;
#endif

	default:
		ASSERT(0);
	}

#ifdef EXT_CBALL
	RateStat("rserv", "send", data_len);
	if ((RateStat_send += data_len) > 10000)
		RateStatTrigger();
#endif

	pbuf_free(p);

done:
	rserv_do_send_response(SOCK_DGRAM, req_pkt, err_no);
}

static void
rserv_do_send_stream(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	struct sockinfo *si = NULL;
	int data_len = 0;
	int err_no = 0;
	int s;

	STAMP(50);
	req = RSERV_PKTDATA(req_pkt);

	s = btohl(req->send_stream.s);

	SOCKET_CHECK(s);

	si = SI(s);

	if (si->type != SOCK_STREAM) {
		err_no = RSOCK_ESOCKTNOSUPPORT;
		goto done;
	}

	/* Send data follows request header */
	data_len = (int)(btohs(req->send_stream.header.len) - sizeof(req->send_stream));

	DPRINTF("rserv_do_send_stream: s=%d data_len=%d\n", s, data_len);

	/* Check for a persistent socket error */
	if (si->err) {
		err_no = err_xlate(si->err);
		goto done;
	}

	/* Check to prevent rogue client from exceeding RSOCK_BUS_SEND_WIN */
	if (si->send_q_len == RSOCK_BUS_SEND_WIN) {
		err_no = RSOCK_ENOBUFS;
		goto done;
	}

#if LWIP_TCP
	rserv_senddata_enq(si, req_pkt);
	rserv_senddata_start(si);
#endif

#ifdef EXT_CBALL
	RateStat("rserv", "send", data_len);
	if ((RateStat_send += data_len) > 10000)
		RateStatTrigger();
#endif

	/* Request enqueued; response will be sent by rserv_senddata_acked */
	return;

done:
	rserv_do_send_response(SOCK_STREAM, req_pkt, err_no);
}

static void
rserv_do_rack(RSERV_PKT *req_pkt)
{
	union rsock_rack *rack;
	int s, count;
	struct sockinfo *si = NULL;

	rack = RSERV_PKTDATA(req_pkt);

	s = (int)btohs(rack->header.s);

	if (s < 0 || s >= RSOCK_SOCKET_MAX || sockinfo[s].domain < 0) {
		EPRINTF("rserv_do_rack: "
		        "got ack for illegal socket %d\n", s);
		goto done;
	}

	si = SI(s);

	if (si->type == SOCK_STREAM)
		count = btohl(rack->stream.count);
	else
		count = btohl(rack->dgram.count);

	if (count < 0 || count > si->recv_q_unacked) {
		EPRINTF("rserv_do_rack: "
		        "s=%d got %d acks for recv data with %d outstanding\n",
		        s, count, si->recv_q_unacked);
		/* Try to recover */
		si->recv_q_unacked = count = 1;
	}

	si->recv_q_unacked -= count;

	/* Send more data responses, if any */
	while (count--)
		rserv_do_rdata(si);

done:
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_socket(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	union rsock_resp *resp;
	int domain, type, proto;
	int s_new = -1;
	struct sockinfo *si_new;
	int err_no = 0;

	req = RSERV_PKTDATA(req_pkt);

	domain = htobl(req->socket.domain);
	type = htobl(req->socket.type);
	proto = htobl(req->socket.proto);

	resp_pkt = RSERV_PKTGET((int)sizeof(resp->socket), TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	memset(resp, 0, sizeof(resp->socket));

	if ((err_no = rserv_socket_alloc(&s_new, domain, type, proto)) != 0)
		goto done;

	si_new = SI(s_new);

	switch (DOMAIN_TYPE(domain, type)) {
#if LWIP_RAW
	case DOMAIN_TYPE(PF_INET, SOCK_RAW):
		if ((si_new->pcb.raw = raw_new(proto)) == NULL)
			err_no = RSOCK_ENOMEM;
		else
			raw_recv(si_new->pcb.raw, rserv_raw_recv, si_new);
		break;
#endif /* LWIP_RAW */

#if LWIP_UDP
	case DOMAIN_TYPE(PF_INET, SOCK_DGRAM):
		if ((si_new->pcb.udp = udp_new()) == NULL)
			err_no = RSOCK_ENOMEM;
		else
			udp_recv(si_new->pcb.udp, rserv_udp_recv, si_new);
		break;
#endif /* LWIP_UDP */

#if LWIP_TCP
	case DOMAIN_TYPE(PF_INET, SOCK_STREAM):
		if ((si_new->pcb.tcp = tcp_new()) == NULL)
			err_no = RSOCK_ENOMEM;
		else {
			tcp_arg(si_new->pcb.tcp, si_new);
			tcp_recv(si_new->pcb.tcp, rserv_tcp_recv);
			tcp_sent(si_new->pcb.tcp, rserv_tcp_sent);
			tcp_poll(si_new->pcb.tcp, rserv_tcp_poll, 4);
			tcp_err(si_new->pcb.tcp, rserv_tcp_err);
		}
		break;
#endif /* LWIP_TCP */

	case DOMAIN_TYPE(PF_RSOCK, SOCK_DGRAM):
		break;

	default:
		err_no = RSOCK_ESOCKTNOSUPPORT;
		break;
	}

done:
	if (err_no != 0 && s_new >= 0) {
		rserv_socket_close(s_new, 1);
		s_new = -1;
	}

	RESP_HEADER(resp, req, sizeof(resp->socket), err_no);

	resp->socket.s = htobl(s_new);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

static void
rserv_do_ioctl(RSERV_PKT *req_pkt)
{
	union rsock_req *req;
	RSERV_PKT *resp_pkt;
	int resp_pkt_len;
	union rsock_resp *resp = NULL;
	int s;
	long cmd = 0;
	int err_no = 0;

	req = RSERV_PKTDATA(req_pkt);

	resp_pkt_len = (int)sizeof(resp->ioctl);

	s = btohl(req->ioctl.s);

	if (s < 0 || s >= RSOCK_SOCKET_MAX || sockinfo[s].domain < 0)
		err_no = RSOCK_EBADF;
	else {
		cmd = btohl(req->ioctl.cmd);

		switch (cmd) {
		case SIOCGIFCONFIG:
			resp_pkt_len += (int)sizeof(struct rsock_ifconfig);
			break;
		case SIOCGIFSTATS:
			resp_pkt_len += (int)sizeof(struct rsock_ifstats);
			break;
		}
	}

	resp_pkt = RSERV_PKTGET(resp_pkt_len, TRUE);
	resp = RSERV_PKTDATA(resp_pkt);

	memset(resp, 0, resp_pkt_len);

	if (err_no != 0)
		goto done;

	switch (cmd) {
	case FIONBIO:
		{
			uint32 flg_in = btohl(*(uint32 *)(&req->ioctl + 1));
			if (flg_in)
				SI(s)->options |= SO_NONBLOCK;
			else
				SI(s)->options &= ~SO_NONBLOCK;
		}
		break;
	case SIOCSIFCONFIG:
		{
			struct rsock_ifconfig *ifc_in =
			        (struct rsock_ifconfig *)(&req->ioctl + 1);
			struct rsock_ifconfig ifc;

			ifc.ifc_flags = btohl(ifc_in->ifc_flags);
			ifc.ifc_index = btohl(ifc_in->ifc_index);
			ifc.ifc_metric = btohl(ifc_in->ifc_metric);
			ifc.ifc_mtu = btohl(ifc_in->ifc_mtu);
			memcpy(ifc.ifc_name, ifc_in->ifc_name, sizeof(ifc.ifc_name));
			SOCKADDR_COPY(&ifc.ifc_addr, &ifc_in->ifc_addr);
			SOCKADDR_COPY(&ifc.ifc_netmask, &ifc_in->ifc_netmask);
			SOCKADDR_COPY(&ifc.ifc_broadaddr, &ifc_in->ifc_broadaddr);
			SOCKADDR_COPY(&ifc.ifc_hwaddr, &ifc_in->ifc_hwaddr);
			SOCKADDR_COPY(&ifc.ifc_p2p, &ifc_in->ifc_p2p);

			err_no = RSERV_IFCONFIG_SET(&ifc);
		}
		break;
	case SIOCGIFCONFIG:
		{
			struct rsock_ifconfig *ifc_out =
			        (struct rsock_ifconfig *)(&resp->ioctl + 1);
			struct rsock_ifconfig ifc;

			if ((err_no = RSERV_IFCONFIG_GET(&ifc)) != 0)
				goto done;

			ifc_out->ifc_flags = htobl(ifc.ifc_flags);
			ifc_out->ifc_index = htobl(ifc.ifc_index);
			ifc_out->ifc_metric = htobl(ifc.ifc_metric);
			ifc_out->ifc_mtu = htobl(ifc.ifc_mtu);
			memcpy(ifc_out->ifc_name, ifc.ifc_name, sizeof(ifc.ifc_name));
			SOCKADDR_COPY(&ifc_out->ifc_addr, &ifc.ifc_addr);
			SOCKADDR_COPY(&ifc_out->ifc_netmask, &ifc.ifc_netmask);
			SOCKADDR_COPY(&ifc_out->ifc_broadaddr, &ifc.ifc_broadaddr);
			SOCKADDR_COPY(&ifc_out->ifc_hwaddr, &ifc.ifc_hwaddr);
			SOCKADDR_COPY(&ifc_out->ifc_p2p, &ifc.ifc_p2p);
		}
		break;
	case SIOCGIFSTATS:
		{
			uint32 *ifs = (uint32 *)(&resp->ioctl + 1);
			int i;

			if ((err_no = RSERV_IFSTATS_GET((struct rsock_ifstats *)ifs)) != 0)
				goto done;

			for (i = 0; i < RSOCK_IFSTATS_COUNT; i++)
				ifs[i] = htobl(ifs[i]);
		}
		break;
	default:
		err_no = RSOCK_ENOSYS;
		break;
	}

done:
	RESP_HEADER(resp, req, resp_pkt_len, err_no);

	RSERV_OUTPUT(resp_pkt);
	RSERV_PKTFREE(req_pkt, FALSE);
}

/*
 * Function dispatch table
 */

static void (*rserv_do_fn[])(RSERV_PKT *req_pkt) = {
	/* This table and the next must be kept in sync with RSOCK_FN_xx values */
	rserv_do_socket,
	rserv_do_accept,
	rserv_do_flow,
	rserv_do_bind,
	rserv_do_shutdown,
	rserv_do_getname,
	rserv_do_getsockopt,
	rserv_do_setsockopt,
	rserv_do_close,
	rserv_do_connect,
	rserv_do_listen,
	rserv_do_send_dgram,
	rserv_do_send_stream,
	rserv_do_ioctl
};

static u8_t rserv_do_reqsize[] = {
	/* This table and the preceding must be kept in sync with RSOCK_FN_xx values */
	(u8_t)sizeof(struct rsock_req_socket),
	(u8_t)sizeof(struct rsock_req_accept),
	(u8_t)sizeof(struct rsock_req_flow),
	(u8_t)sizeof(struct rsock_req_bind),
	(u8_t)sizeof(struct rsock_req_shutdown),
	(u8_t)sizeof(struct rsock_req_getname),
	(u8_t)sizeof(struct rsock_req_getsockopt),
	(u8_t)sizeof(struct rsock_req_setsockopt),
	(u8_t)sizeof(struct rsock_req_close),
	(u8_t)sizeof(struct rsock_req_connect),
	(u8_t)sizeof(struct rsock_req_listen),
	(u8_t)sizeof(struct rsock_req_send_dgram),
	(u8_t)sizeof(struct rsock_req_send_stream),
	(u8_t)sizeof(struct rsock_req_ioctl)
};

/*
 * Whenever a message is received from the transport bus, it's passed into the RSOCK
 * server by calling rserv_input.  The RSOCK server processes the message and may
 * send an immediate response or a delayed response at a later time.  The input
 * message is freed eventually inside rserv.
 */

void
rserv_input(RSERV_PKT *pkt)
{
	uint32 flags;
	union rsock_header *hdr;
	int len, fn;

	STAMP(40);
	hdr = RSERV_PKTDATA(pkt);

	DPRINTF("rserv_input: pkt=%p len=%d\n", (void *)hdr, RSERV_PKTLEN(pkt));

	ASSERT(sizeof(rserv_do_fn) / sizeof(rserv_do_fn[0]) == RSOCK_FN__COUNT);
	ASSERT(sizeof(rserv_do_reqsize) / sizeof(rserv_do_reqsize[0]) == RSOCK_FN__COUNT);

	flags = btohs(hdr->common.flags);

	switch (flags & RSOCK_FLAG_TYPE_MASK) {
	case RSOCK_FLAG_TYPE_REQ:
		fn = btohs(hdr->req.fn);

		DPRINTF("rserv_input: req fn=%d\n", fn);

		if (fn < 0 || fn >= RSOCK_FN__COUNT) {
			EPRINTF("rserv_input: bad function\n");
			RSERV_PKTFREE(pkt, FALSE);
			return;
		}

		if ((len = btohs(hdr->req.len)) < rserv_do_reqsize[fn]) {
			EPRINTF("rserv_input: request too short (%d)\n", len);
			RSERV_PKTFREE(pkt, FALSE);
			return;
		}

		(*rserv_do_fn[fn])(pkt);
		break;

	case RSOCK_FLAG_TYPE_RACK:
		rserv_do_rack(pkt);
		break;

	default:
		EPRINTF("rserv_input: unknown message type, flags=0x%x\n", flags);
		break;
	}
}

/*
 * Initialization
 */

void
rserv_init(void)
{
	int s;

	/*
	 * Reserve socket 0 for rsock protocol internal use.
	 * Socket 0 is used only for the socket() call.
	 */

	for (s = 0; s < RSOCK_SOCKET_MAX; s++)
		SI(s)->domain = PF__INVAL;

	s = rserv_socket_alloc(&s, PF_RSOCK, AF_UNSPEC, 0);
	ASSERT(s == 0);
}
