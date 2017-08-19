/*
 * RSOCK: Remote Sockets Client Library
 *
 *   Implemented: TCP, UDP, RAW
 *   Tested: TCP, UDP
 *   Not implemented: OOB data, SIGPIPE, SIGIO, select
 *
 * Note:  These calls operate only on rsock socket descriptors.
 *        None of them are intended to work on regular file descriptors.
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

#include <stdarg.h>
#include "os.h"
#include <rsock/socket.h>
#include <rsock/proto.h>
#include <rsock/plumb.h>
#include <rsock/endian.h>

/* Max outstanding requests for all sockets; preferably a power of 2 */
#define RSOCK_NSLOTS		16

/* Sequence number range; must have RSOCK_NSLOTS * (RSOCK_SEQ_MAX + 1) <= 65536 */
#define RSOCK_SEQ_MIN		1               /* Seq 0 reserved for future use */
#define RSOCK_SEQ_MAX		255

#define RSOCK_PVER		1		/* Protocol version implemented in this file */

/* Bus protocol byte order conversion */

#if (RSOCK_LE == RSOCK_BUS_LE)
#define htobl(x)		(x)
#define btohl(x)		(x)
#define htobs(x)		(x)
#define btohs(x)		(x)
#else
#define htobl(x)		rsock_swap32(x)
#define btohl(x)		rsock_swap32(x)
#define htobs(x)		rsock_swap16(x)
#define btohs(x)		rsock_swap16(x)
#endif

/*
 * The request handle is composed of the request slot number in the
 * lower bits, and a sequence number in the upper bits.  This scheme
 * only matters here on the client side since the server side just
 * echoes the request handles in the responses.
 */

#define HANDLE(seq, slot)	((seq) * RSOCK_NSLOTS + (slot))
#define HANDLE_SEQ(handle)	((handle) / RSOCK_NSLOTS)
#define HANDLE_SLOT(handle)	((handle) % RSOCK_NSLOTS)

/* Timeout for remote operations that should be quick */

#define TIMEOUT_SLOT		5000	/* msec */
#define TIMEOUT_NORMAL		3000	/* msec */
#define TIMEOUT_CONNECT		5000	/* msec */

#undef TIMEOUT_NORMAL
#define TIMEOUT_NORMAL		OS_MSEC_FOREVER
#undef TIMEOUT_CONNECT
#define TIMEOUT_CONNECT		OS_MSEC_FOREVER

#ifdef BCMDBG
#define TIMEOUT_NONE		30000	/* Prevent hangs while debugging in kernel */
#else
#define TIMEOUT_NONE		OS_MSEC_FOREVER
#endif

#define RETRY_CLOSE_USEC	4000000

#define RSOCK_MIN(x, y)		((x) <= (y) ? (x) : (y))
#define RSOCK_MAX(x, y)		((x) > (y) ? (x) : (y))

#define RECV_HDRLEN(type) \
	((type) == SOCK_STREAM ? RSOCK_RDATA_STREAM_HDRLEN : RSOCK_RDATA_DGRAM_HDRLEN);

/* Conversion back and forth between socket fd and socket structure */
#define SI(s)			(&sockinfo[s])
#define S(si)			((int)((si) - sockinfo))

/* Receive circular queue management */
#define RECV_Q_SIZE		(RSOCK_BUS_RECV_WIN + 1)
#define RECV_Q_INC(ptr)		(((ptr) + 1) % RECV_Q_SIZE)
#define RECV_Q_EMPTY(si)	((si)->recv_q_rptr == (si)->recv_q_wptr)
#define RECV_Q_FULL(si)		(RECV_Q_INC((si)->recv_q_wptr) == (si)->recv_q_rptr)
#define RECV_Q_WRITE(si, pkt)	{ (si)->recv_q[(si)->recv_q_wptr] = (pkt); \
				  (si)->recv_q_wptr = RECV_Q_INC((si)->recv_q_wptr); }
#define RECV_Q_READ(si, pkt)	{ (pkt) = (si)->recv_q[(si)->recv_q_rptr]; \
				  (si)->recv_q[(si)->recv_q_rptr] = NULL; \
				  (si)->recv_q_rptr = RECV_Q_INC((si)->recv_q_rptr); }
#define RECV_Q_PEEK(si, pkt)	{ (pkt) = (si)->recv_q[(si)->recv_q_rptr]; }

/* Socket state flags */
#define STATE_CONN		0x1	/* Socket connected */
#define STATE_CLOSE_LOCAL	0x2	/* Connection closed locally */
#define STATE_CLOSE_PEER	0x4	/* Connection closed by peer */
#define STATE_ERROR		0x8	/* Fatal error has occurred  */
#define STATE_NONBLOCK		0x10	/* Socket has been marked as non-blocking */

/* Client-side state per socket */
struct sockinfo {
	int domain;			/* PF_xx, or -1 if sockinfo entry unallocated */
	int type;			/* SOCK_RAW/DGRAM/STREAM */
	int proto;			/* Used for SOCK_RAW */

	uint32 state;			/* STATE_xx flags */

	/* Queue of outstanding sends */
	os_sem_t send_lock;		/* Serialize multiple sending tasks */
	int send_head;			/* Linked list of outstanding send slots */
	int send_tail;			/* Index of last slot on send list */
	int send_count;			/* Number of outstanding send requests */

	/* Receive state */
	os_sem_t recv_lock;		/* Serialize multiple tasks receiving */
	os_sem_t recv_signal;		/* Signal for receive event */

	/* Circular queue of receive data packets so rsock_input doesn't need to take a lock */
	os_pkt_t *recv_q[RECV_Q_SIZE];
	int recv_q_wptr;		/* recv_q write pointer */
	int recv_q_rptr;		/* recv_q read pointer */
	int recv_q_hoff;		/* Offset to next unread data in recv_q next packet */
};

struct sockinfo sockinfo[RSOCK_SOCKET_MAX];

/*
 * One slot per outstanding request.
 * Free slots are kept linked together for quick allocation.
 */

#define RSOCK_SLOT_USED			0x1

struct rsock_slot {
	int16 next;			/* Next slot pointer for lists (null pointer is -1) */
	uint16 flags;			/* RSOCK_SLOT_xx flags */
	rsock_handle_t handle;		/* Handle of outstanding request associated w/ this slot */
	os_sem_t completion;		/* Semaphore to wake when matching response is received */
	os_pkt_t *resp_pkt;		/* Response packet that woke the completion semaphore */
};

static int rsock_inited;
static os_sem_t rsock_lock;		/* Protects global rsock state */
static int rserv_pver;			/* Protocol version from saved from socket() call */

struct rsock_slot rsock_slots[RSOCK_NSLOTS];
static os_sem_t rsock_freeslot_count;	/* Counting semaphore to allocate slots */
static int rsock_freeslot_list;		/* List of free slots */
static int rsock_seq;			/* Next request sequence number */

static int rsock_send_wait(int s, int cleanup);

/*
 * Translation of rsock error code (RSOCK_E*) to O/S errno (E*).
 * Table must be kept in sync with RSOCK_E* definitions in <rsock/proto.h>
 */

static const uint8 rsock_errno_map[] = {
	0,
	EBADF,
	ENOMEM,
	ENOBUFS,
	ECONNABORTED,
	ECONNRESET,
	ESHUTDOWN,
	ENOTCONN,
	EINVAL,
	EIO,
	EHOSTUNREACH,
	EADDRINUSE,
	ENOPROTOOPT,
	ESOCKTNOSUPPORT,
	ENOSYS,
	EAGAIN,
	EINTR
};

#define RSOCK_ERRNO_MAP_COUNT \
	((int)((sizeof(rsock_errno_map) / sizeof(rsock_errno_map[0]))))

/*
 * In this file, err_no is always the rsock error number (RSOCK_Exx), and errno is the
 * operating system error number.  This routine converts err_no to errno.  If the result
 * is zero, it doesn't do anything and returns 0.  Otherwise, it sets the operating system
 * errno and returns -1.
 */
static int
rsock_errno_set(int err_no)
{
	ASSERT(RSOCK_ERRNO_MAP_COUNT == RSOCK_E__COUNT);

	if (err_no == 0)
		return 0;

	if (err_no < 0 || err_no >= RSOCK_E__COUNT)
		os_errno_set(EIO);
	else
		os_errno_set((int)rsock_errno_map[err_no]);

	return -1;
}

/* Close all sockets; application should call this before exiting */

void
rsock_term(void)
{
	int s;

	OS_DEBUG("rsock_term: called\n");

	for (s = 0; s < RSOCK_SOCKET_MAX; s++)
		if (SI(s)->domain >= 0)
			(void)rsock_close(s);
}

int
rsock_init(void)
{
	int i, s;

	OS_DEBUG("rsock_init: called\n");

	ASSERT(sizeof(struct sockaddr) == sizeof(struct sockaddr_in));

	os_sem_init(&rsock_freeslot_count, RSOCK_NSLOTS);

	os_memset(rsock_slots, 0, sizeof(rsock_slots));

	rsock_freeslot_list = -1;

	for (i = RSOCK_NSLOTS - 1; i >= 0; i--) {
		rsock_slots[i].next = rsock_freeslot_list;
		rsock_freeslot_list = i;
	}

	rsock_seq = RSOCK_SEQ_MIN;

	os_sem_init(&rsock_lock, 1);

	os_memset(sockinfo, 0, sizeof(sockinfo));

	for (s = 0; s < RSOCK_SOCKET_MAX; s++)
		SI(s)->domain = PF__INVAL;

	rsock_inited = 1;

	return 0;
}

/* Allocate a free slot */

static int
rsock_slot_alloc(int *slot_ptr)
{
	struct rsock_slot *sl;
	int slot, seq, err;

	/* Wait until one is available */

	if ((err = os_sem_take(&rsock_freeslot_count, TIMEOUT_SLOT, 0)) != 0) {
		OS_ERROR("rsock_slot_alloc: timed out waiting for slot\n");
		os_errno_set(err);
		return -1;
	}

	/* Remove from free list */

	if ((err = os_sem_take(&rsock_lock, OS_MSEC_FOREVER, 0)) != 0) {
		os_errno_set(err);
		return -1;
	}

	slot = rsock_freeslot_list;

	ASSERT(slot >= 0);

	sl = &rsock_slots[slot];

	rsock_freeslot_list = sl->next;

	ASSERT(!(sl->flags & RSOCK_SLOT_USED));

	seq = rsock_seq++;
	if (rsock_seq > RSOCK_SEQ_MAX)
		rsock_seq = RSOCK_SEQ_MIN;

	os_sem_give(&rsock_lock);

	/* Initialize slot */

	sl->next = -1;
	sl->flags = RSOCK_SLOT_USED;
	sl->handle = HANDLE(seq, slot);
	os_sem_init(&sl->completion, 0);
	sl->resp_pkt = NULL;

	*slot_ptr = slot;

	OS_DEBUG("rsock_slot_alloc: slot=%d\n", slot);

	return 0;
}

static void
rsock_slot_free(int slot)
{
	struct rsock_slot *sl;

	OS_DEBUG("rsock_slot_free: slot=%d\n", slot);

	sl = &rsock_slots[slot];

	sl->handle = (rsock_handle_t)-1;
	sl->flags = 0;

	while (os_sem_take(&rsock_lock, OS_MSEC_FOREVER, 0) != 0)
		;

	/* Put on free list */
	sl->next = rsock_freeslot_list;
	rsock_freeslot_list = slot;

	os_sem_give(&rsock_lock);

	/* Increment available slots */
	os_sem_give(&rsock_freeslot_count);
}

/*
 * rsock_request allocates a slot, sends a request, and returns the slot
 * number without waiting for it to finish.  The request must have been
 * completely filled in except for the handle.
 *
 * The request packet is freed regardless of success or failure.  The
 * data output function must be thread safe and must free the output packet
 * regardless of success or failure.
 *
 * On failure, errno is set to ENFILE and -1 is returned.
 * On success, slot number is returned.
 */

static int
rsock_request(os_pkt_t *req_pkt)
{
	int slot;
	union rsock_req *req;

	if (rsock_slot_alloc(&slot)) {
		os_pkt_free(req_pkt);
		return -1;
	}

	req = os_pkt_data(req_pkt);
	req->header.handle = htobs(rsock_slots[slot].handle);

	if (os_pkt_output(req_pkt) < 0) {
		rsock_slot_free(slot);
		os_errno_set(EIO);
		return -1;
	}

	return slot;
}

/*
 * rsock_wait waits for a response to an outstanding operation.
 *
 * On timeout, -1 is returned, errno is set to ETIMEDOUT, and
 * if tmout_cancel is true, the request is canceled and the
 * slot is freed (if tmout_cancel is false, the caller can retry).
 *
 * On success, the response packet is returned, the pointer to
 * the resp data is returned, the slot is freed, and 0 is returned.
 */

static int
rsock_wait(int slot, os_msec_t tmout_msec, int tmout_cancel,
           os_pkt_t **resp_pkt, union rsock_resp **resp)
{
	struct rsock_slot *sl;
	int err;

	OS_DEBUG("rsock_wait: slot=%d\n", slot);

	ASSERT(slot >= 0 && slot < RSOCK_NSLOTS);

	sl = &rsock_slots[slot];

	/* Slot should no longer be on any list */
	ASSERT(sl->next < 0);

	if ((err = os_sem_take(&sl->completion, tmout_msec, 1)) != 0) {
		if (err == ETIMEDOUT && tmout_cancel) {
			rsock_slot_free(slot);
			OS_ERROR("rsock_wait: timed out waiting for slot %d completion\n", slot);
		}
		os_errno_set(err);
		return -1;
	}

	if (sl->resp_pkt == NULL) {
		OS_ERROR("rsock_wait: slot %d semaphore taken illegally\n", slot);
		os_errno_set(EIO);
		return -1;
	}

	*resp_pkt = sl->resp_pkt;
	*resp = os_pkt_data(sl->resp_pkt);

	sl->resp_pkt = NULL;

	rsock_slot_free(slot);

	return 0;
}

/*
 * rsock_call combines rsock_request and rsock_wait for the common case.
 * On error, errno has been set.
 */

static int
rsock_call(os_pkt_t *req_pkt, os_msec_t tmout_msec,
           os_pkt_t **resp_pkt, union rsock_resp **resp)
{
	int slot;

	if ((slot = rsock_request(req_pkt)) < 0)
		return -1;

	if (rsock_wait(slot, tmout_msec, 1, resp_pkt, resp) < 0)
		return -1;

	return 0;
}

/*
 * rsock_input is called by the bus transport layer when a response
 * message is received from rserv on the transport channel.  The handle
 * in the response is matched up to a request slot.  The input message
 * is freed later when rsock calls the application-supplied data_free
 * routine.
 *
 * Note: May be called from interrupt context.
 */

void
rsock_input(os_pkt_t *pkt)
{
	union rsock_header *hdr;
	unsigned int flags;
	rsock_handle_t handle;
	int slot, len, s, hoff;
	struct rsock_slot *sl;
	struct sockinfo *si;

	ASSERT(pkt != NULL);

	hdr = os_pkt_data(pkt);
	len = os_pkt_len(pkt);

	flags = btohs(hdr->common.flags);

	if ((int)btohs(hdr->common.len) != len) {
		OS_ERROR("rsock_input: response len field %d does not match actual len %d\n",
		         (int)btohs(hdr->common.len), len);
		goto drop;
	}

	switch (flags & RSOCK_FLAG_TYPE_MASK) {
	case RSOCK_FLAG_TYPE_RESP:
		handle = btohs(hdr->resp.handle);

		slot = HANDLE_SLOT(handle);

		if (slot < 0 || slot >= RSOCK_NSLOTS) {
			OS_ERROR("rsock_input: response with bad slot %d\n", slot);
			goto drop;
		}

		sl = &rsock_slots[slot];

		if (!(sl->flags & RSOCK_SLOT_USED)) {
			OS_ERROR("rsock_input: response to idle slot %d\n", slot);
			goto drop;
		}

		if (sl->handle != handle) {
			OS_ERROR("rsock_input: response to canceled handle 0x%x ignored\n", handle);
			goto drop;
		}

		sl->resp_pkt = pkt;

		os_sem_give(&sl->completion);

		return;

	case RSOCK_FLAG_TYPE_RDATA:
		s = (int)btohs(hdr->rdata.s);

		if (s < 0 || s >= RSOCK_SOCKET_MAX || SI(s)->domain < 0) {
			OS_ERROR("rsock_input: rdata s=%d bad socket\n", s);
			goto drop;
		}

		si = SI(s);

		hoff = RECV_HDRLEN(si->type);

		if (len < hoff) {
			OS_ERROR("rsock_input: rdata len=%d too short\n", len);
			goto drop;
		}

		len -= hoff;

		OS_DEBUG("rsock_input: rdata s=%d len=%d state=%d\n", s, len, si->state);

		if (si->state & STATE_CLOSE_LOCAL)
			goto drop;

		/* Handle stream closed condition, as indicated by zero-length data */
		if (len == 0) {
			OS_DEBUG("rsock_input: rdata s=%d recv closed\n", s);
			si->state |= STATE_CLOSE_PEER;
			/* Notify possible pending task */
			os_sem_give(&si->recv_signal);
			goto drop;
		}

		if (RECV_Q_FULL(si)) {
			OS_ERROR("rsock_input: rdata s=%d overflow\n", s);
			si->state |= STATE_ERROR;
			os_sem_give(&si->recv_signal);
			goto drop;
		}

		if (RECV_Q_EMPTY(si))
			si->recv_q_hoff = hoff;

		RECV_Q_WRITE(si, pkt);

		OS_DEBUG("rsock_input: rdata s=%d len=%d wptr=%d rptr=%d\n",
		         s, len, si->recv_q_wptr, si->recv_q_rptr);

		/* Notify pending or future receive */
		os_sem_give(&si->recv_signal);
		return;

	default:
		OS_ERROR("rsock_input: unknown message type, flags=0x%x\n", flags);
		goto drop;
	}

drop:
	os_pkt_free(pkt);
}

/* On error, errno has been set */
static os_pkt_t *
rsock_req_alloc(union rsock_req **req_ptr, int fn, int req_len)
{
	os_pkt_t *req_pkt;
	union rsock_req *req;

	if ((req_pkt = os_pkt_alloc(req_len)) == NULL) {
		OS_ERROR("rsock: out of memory (%d)\n", req_len);
		os_errno_set(ENOMEM);
		return NULL;
	}

	req = os_pkt_data(req_pkt);

	/* Fill in header, except handle which is filled in when request is sent */
	req->header.flags = htobs(RSOCK_FLAG_TYPE_REQ | (RSOCK_PVER << RSOCK_FLAG_PVER_SHIFT));
	req->header.len = htobs(req_len);
	req->header.fn = htobs(fn);

	*req_ptr = req;

	return req_pkt;
}

/* Initialize local socket structure after rserv has allocated a socket */

static void
rsock_socket_init(int s, int domain, int type, int proto)
{
	struct sockinfo *si = SI(s);

	os_memset(si, 0, sizeof(struct sockinfo));

	si->domain = domain;
	si->type = type;
	si->proto = proto;
	si->state = 0;

	os_sem_init(&si->send_lock, 1);
	si->send_head = -1;
	si->send_tail = -1;

	os_sem_init(&si->recv_lock, 1);
	os_sem_init(&si->recv_signal, 0);
}

/* Enable/disable rserv sending receive data packets for a socket */

static int
rsock_socket_flow(int s, int enable)
{
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no;

	req_len = (uint16)sizeof(req->flow);

	if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_FLOW, req_len)) == NULL)
		return -1;

	req->flow.s = htobl(s);
	req->flow.status = htobl(enable ? RSOCK_FLOW_XON : RSOCK_FLOW_XOFF);

	if (rsock_call(req_pkt, TIMEOUT_NONE, &resp_pkt, &resp) < 0)
		return -1;

	if ((err_no = btohs(resp->flow.header.err_no)) != 0) {
		os_pkt_free(resp_pkt);
		return rsock_errno_set(err_no);
	}

	os_pkt_free(resp_pkt);

	return 0;
}

int
rsock_accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no;
	int s_new = -1;		/* Descriptor for new socket */

	OS_DEBUG("rsock_accept: s=%d\n", s);

	req_len = (uint16)sizeof(req->accept);

	if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_ACCEPT, req_len)) == NULL)
		return -1;

	req->accept.s = htobl(s);

	if (rsock_call(req_pkt, TIMEOUT_NONE, &resp_pkt, &resp) < 0)
		return -1;

	if ((err_no = btohs(resp->accept.header.err_no)) != 0) {
		os_pkt_free(resp_pkt);
		return rsock_errno_set(err_no);
	}

	if (addr != NULL)
		os_memcpy(addr, &resp->accept.addr, sizeof(*addr));

	if (addrlen != NULL)
		*addrlen = btohl(resp->accept.addrlen);

	s_new = btohl(resp->accept.s);

	os_pkt_free(resp_pkt);

	OS_DEBUG("rsock_accept: s_new=%d\n", s_new);

	if (s_new < 0 || s_new >= RSOCK_SOCKET_MAX) {
		os_errno_set(EIO);
		return -1;
	}

	/* Initialize local structure for new socket */
	rsock_socket_init(s_new, SI(s)->domain, SI(s)->type, SI(s)->proto);

	SI(s_new)->state |= STATE_CONN;

	/* Enable flow of receive data */
	if (rsock_socket_flow(s_new, 1) < 0) {
		(void)rsock_close(s_new);
		return -1;
	}

	return s_new;
}

int
rsock_bind(int s, struct sockaddr *name, socklen_t namelen)
{
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no;

	OS_DEBUG("rsock_bind: s=%d\n", s);

	req_len = (uint16)sizeof(req->bind);

	if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_BIND, req_len)) == NULL)
		return -1;

	req->bind.s = htobl(s);
	os_memcpy(&req->bind.addr, name, sizeof(req->bind.addr));
	req->bind.addrlen = htobl(namelen);

	if (rsock_call(req_pkt, TIMEOUT_NORMAL, &resp_pkt, &resp) < 0)
		return -1;

	err_no = btohs(resp->bind.header.err_no);

	os_pkt_free(resp_pkt);

	return rsock_errno_set(err_no);
}

int
rsock_close(int s)
{
	struct sockinfo *si;
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no, err, retry_usec;


	OS_DEBUG("rsock_close: s=%d\n", s);

	if (s < 0 || s >= RSOCK_SOCKET_MAX || SI(s)->domain < 0) {
		os_errno_set(EBADF);
		return -1;
	}

	si = SI(s);

	/* Prevent new send and receive requests from being made */
	si->state |= STATE_CLOSE_LOCAL;
	os_sem_give(&si->recv_signal);

	os_sem_take(&si->recv_lock, OS_MSEC_FOREVER, 0);

	/* Wait for all outstanding send requests to complete */
	while (si->send_count > 0)
		(void)rsock_send_wait(s, 1);

	/* Prevent additional receive data from being queued */
	si->state |= STATE_CLOSE_PEER;

	/* Toss all buffered receive data */
	while (!RECV_Q_EMPTY(si)) {
		RECV_Q_READ(si, resp_pkt);
		os_pkt_free(resp_pkt);
	}

	os_sem_give(&si->recv_signal);

	os_sem_give(&si->recv_lock);

	/* Retry close in a loop until successful (see rserv) */
	retry_usec = 1;

	for (;;) {
		req_len = (uint16)sizeof(req->close);

		if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_CLOSE, req_len)) == NULL)
			return -1;

		req->close.s = htobl(s);

		if (rsock_call(req_pkt, TIMEOUT_NORMAL, &resp_pkt, &resp) < 0)
			return -1;

		err_no = btohs(resp->close.header.err_no);

		if (err_no == 0) {
			/* On success, clear/invalidate sockinfo entry */
			rsock_socket_init(s, PF__INVAL, 0, 0);
		}

		os_pkt_free(resp_pkt);

		if (err_no != RSOCK_EAGAIN)
			break;

		/* After a few fast retries, start inserting an exponential backoff delay */
		if (retry_usec >= 1000 && (err = os_msleep(retry_usec / 1000)) != 0) {
			os_errno_set(err);
			return -1;
		}

		if ((retry_usec *= 2) > RETRY_CLOSE_USEC)
			retry_usec = RETRY_CLOSE_USEC;
	}

	return rsock_errno_set(err_no);
}

int
rsock_connect(int s, struct sockaddr *name, socklen_t namelen)
{
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no;

	OS_DEBUG("rsock_connect: s=%d\n", s);

	req_len = (uint16)sizeof(req->connect);

	if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_CONNECT, req_len)) == NULL)
		return -1;

	req->connect.s = htobl(s);
	os_memcpy(&req->connect.addr, name, sizeof(req->connect.addr));
	req->connect.addrlen = htobl(namelen);

	if (rsock_call(req_pkt, TIMEOUT_CONNECT, &resp_pkt, &resp) < 0)
		return -1;

	err_no = btohs(resp->connect.header.err_no);

	os_pkt_free(resp_pkt);

	if (rsock_errno_set(err_no))
		return -1;

	SI(s)->state |= STATE_CONN;
	return 0;
}

int
rsock_listen(int s, int backlog)
{
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no;

	OS_DEBUG("rsock_listen: s=%d backlog=%d\n", s, backlog);

	req_len = (uint16)sizeof(req->listen);

	if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_LISTEN, req_len)) == NULL)
		return -1;

	req->listen.s = htobl(s);
	req->listen.backlog = htobl(backlog);

	if (rsock_call(req_pkt, TIMEOUT_NORMAL, &resp_pkt, &resp) < 0)
		return -1;

	err_no = btohs(resp->listen.header.err_no);

	os_pkt_free(resp_pkt);

	return rsock_errno_set(err_no);
}

/* On success, returns 0.  On error, returns -1 and sets errno. */
static int
rsock_recv_ack(int s, int count)
{
	struct sockinfo *si = SI(s);
	uint16 rack_len;
	os_pkt_t *rack_pkt;
	union rsock_rack *rack;

	OS_DEBUG("rsock_recv_ack: s=%d\n", s);

	if (si->type == SOCK_STREAM)
		rack_len =  (uint16)sizeof(struct rsock_rack_stream);
	else
		rack_len =  (uint16)sizeof(struct rsock_rack_dgram);

	if ((rack_pkt = os_pkt_alloc(rack_len)) == NULL) {
		os_errno_set(ENOMEM);
		return -1;
	}

	rack = os_pkt_data(rack_pkt);

	rack->header.flags = htobs(RSOCK_FLAG_TYPE_RACK | (RSOCK_PVER << RSOCK_FLAG_PVER_SHIFT));
	rack->header.len = htobs(rack_len);
	rack->header.s = htobs((uint16)s);
	rack->header._pad = 0;

	if (si->type == SOCK_STREAM)
		rack->stream.count = htobl(count);
	else
		rack->dgram.count = htobl(count);

	if (os_pkt_output(rack_pkt) < 0) {
		os_errno_set(EIO);
		return -1;
	}

	return 0;
}

int
rsock_recvfrom(int s, void *mem, int len, unsigned int flags,
               struct sockaddr *from, socklen_t *fromlen)
{
	struct sockinfo *si;
	os_pkt_t *rdata_pkt;
	union rsock_rdata *rdata;
	int rdata_len;
	int n, ret_len;
	int ack_count = 0;

	if (s < 0 || s >= RSOCK_SOCKET_MAX || SI(s)->domain < 0) {
		os_errno_set(EBADF);
		return -1;
	}

	si = SI(s);

	if (si->type == SOCK_STREAM && !(si->state & STATE_CONN)) {
		os_errno_set(ENOTCONN);
		return -1;
	}

	OS_DEBUG("rsock_recvfrom: s=%d len=%d flags=0x%x\n", s, len, flags);

	os_sem_take(&si->recv_lock, OS_MSEC_FOREVER, 0);

	/* Wait for receive event */
	for (;;) {
		/* See if connection was locally closed */
		if (si->state & STATE_CLOSE_LOCAL) {
			os_sem_give(&si->recv_lock);
			os_errno_set(EBADF);
			return -1;
		}

		/* See if an error occurred */
		if (si->state & STATE_ERROR) {
			os_sem_give(&si->recv_lock);
			os_errno_set(EIO);
			return -1;
		}

		/* See if data is available (before checking if closed by peer) */
		if (!RECV_Q_EMPTY(si))
			break;

		/* See if connection was closed by peer */
		if (si->state & STATE_CLOSE_PEER) {
			OS_DEBUG("rsock_recvfrom: s=%d connection closed\n", s);
			os_sem_give(&si->recv_lock);
			return 0;
		}

		if (si->state & STATE_NONBLOCK) {
			os_sem_give(&si->recv_lock);
			os_errno_set(EAGAIN);
			return -1;
		}

		OS_DEBUG("rsock_recvfrom: s=%d wait for receive event\n", s);
		os_sem_take(&si->recv_signal, OS_MSEC_FOREVER, 0);
	}

	/*
	 * For stream sockets: consume up to 'len' bytes from recv_q.
	 * For datagram sockets: consume one packet from recv_q.  The data
	 *	is truncated to the size of the memory buffer.
	 * Either way, one receive acknowledgement is sent for each packet
	 *	when it is fully consumed and dequeued.
	 */

	if (si->type == SOCK_STREAM) {
		ret_len = 0;

		while (len > 0) {
			if (RECV_Q_EMPTY(si))
				break;

			RECV_Q_PEEK(si, rdata_pkt);
			rdata = os_pkt_data(rdata_pkt);
			rdata_len = os_pkt_len(rdata_pkt);

			/* Consume as much as requested, or as much as left in head packet */
			n = RSOCK_MIN(len, rdata_len - si->recv_q_hoff);

			OS_DEBUG("rsock_recvfrom: s=%d n=%d stream\n", s, n);

			os_memcpy(mem, (const char *)rdata + si->recv_q_hoff, n);

			mem = (char *)mem + n;
			len -= n;
			ret_len += n;

			/* If head packet fully consumed, dequeue and ack it */
			if ((si->recv_q_hoff += n) == rdata_len) {
				RECV_Q_READ(si, rdata_pkt);

				si->recv_q_hoff = RSOCK_RDATA_STREAM_HDRLEN;

				os_pkt_free(rdata_pkt);

				ack_count++;
			}
		}
	} else {				/* RAW, DGRAM */
		RECV_Q_READ(si, rdata_pkt);
		rdata = os_pkt_data(rdata_pkt);
		rdata_len = os_pkt_len(rdata_pkt);

		OS_DEBUG("rsock_recvfrom: dgram s=%d len=%d\n",
		         s, rdata_len - si->recv_q_hoff);

		ret_len = RSOCK_MIN(len, rdata_len - si->recv_q_hoff);
		ASSERT(ret_len > 0);

		os_memcpy(mem, (const char *)rdata + si->recv_q_hoff, ret_len);

		if (from != NULL)
			os_memcpy(from, &rdata->dgram.from, sizeof(*from));

		if (fromlen != NULL)
			*fromlen = btohl(rdata->dgram.fromlen);

		os_pkt_free(rdata_pkt);

		ack_count++;
	}

	if (ack_count > 0 && rsock_recv_ack(s, ack_count) < 0)
		ret_len = -1;

	os_sem_give(&si->recv_lock);

	OS_DEBUG("rsock_recvfrom: s=%d ret_len=%d\n", s, ret_len);

	return ret_len;
}

int
rsock_read(int s, void *mem, int len)
{
	return rsock_recvfrom(s, mem, len, 0, NULL, NULL);
}

int
rsock_recv(int s, void *mem, int len, unsigned int flags)
{
	return rsock_recvfrom(s, mem, len, flags, NULL, NULL);
}

/*
 * Wait for the oldest outstanding send to complete.
 * On success, returns 0.  On failure, returns -1 and sets errno.
 *
 * si->send_lock must be held before calling and the send queue must be non-empty.
 */

static int
rsock_send_wait(int s, int cleanup)
{
	struct sockinfo *si = SI(s);
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no, slot, tmout;

	/* Dequeue oldest outstanding send request */
	slot = si->send_head;
	ASSERT(slot >= 0);

	OS_DEBUG("rsock_send_wait: s=%d slot=%d cleanup=%d\n", s, slot, cleanup);

	if ((si->send_head = rsock_slots[slot].next) < 0)
		si->send_tail = -1;

	rsock_slots[slot].next = -1;

	tmout = (cleanup || !(si->state & STATE_NONBLOCK)) ? OS_MSEC_FOREVER : 0;

	if (rsock_wait(slot, tmout, 0, &resp_pkt, &resp) < 0) {
		/* Put failed send request back at head of queue */
		rsock_slots[slot].next = si->send_head;
		si->send_head = slot;
		return -1;
	}

	si->send_count--;

	if (si->type == SOCK_STREAM)
		err_no = btohs(resp->send_stream.header.err_no);
	else
		err_no = btohs(resp->send_dgram.header.err_no);

	os_pkt_free(resp_pkt);

	return rsock_errno_set(err_no);
}

/*
 * rsock_send_raw sends a burst of data in one request.  For SOCK_RAW or
 * SOCK_DGRAM, this is one datagram.  For SOCK_STREAM, the caller must
 * already have already allocated send buffer space to make sure no data
 * will be dropped.
 *
 * Normally, rsock_send_raw doesn't wait for the send to complete, but
 * puts the send slot on a queue of outstanding sends.  If the queue is
 * full, it blocks unless the socket is non-blocking.
 *
 * On success, returns 0.
 * On error, returns -1 and sets errno.
 */

static int
rsock_send_raw(int s, void *data, int size, unsigned int flags,
               struct sockaddr *to, socklen_t tolen)
{
	struct sockinfo *si = SI(s);
	int slot;
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;

	OS_DEBUG("rsock_send_raw: s=%d data=%p size=%d flags=0x%x\n",
	         s, data, size, flags);

	if (size == 0)
		return 0;

	/* Exclude multiple tasks from sending at the same time */
	os_sem_take(&si->send_lock, OS_MSEC_FOREVER, 0);

	/* If too many sends are outstanding, wait for the oldest one to finish */
	if (si->send_count == RSOCK_BUS_SEND_WIN)
		if (rsock_send_wait(s, 0) < 0) {
			if (os_errno == ETIMEDOUT)
				os_errno_set(EAGAIN);	/* Non-blocking operation support */
			os_sem_give(&si->send_lock);
			return -1;
		}

	/* Send a new request */
	if (si->type == SOCK_STREAM) {
		if (size > RSOCK_SEND_STREAM_REQ_LIMIT) {
			os_sem_give(&si->send_lock);
			os_errno_set(EMSGSIZE);
			return -1;
		}

		req_len = (uint16)(sizeof(req->send_stream) + size);

		if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_SEND_STREAM, req_len)) == NULL) {
			os_sem_give(&si->send_lock);
			return -1;
		}

		req->send_stream.s = htobl(s);
		req->send_stream.flags = htobl(flags);

		os_memcpy(&req->send_stream + 1, data, size);
	} else {
		if (size > RSOCK_SEND_DGRAM_REQ_LIMIT) {
			os_sem_give(&si->send_lock);
			os_errno_set(EMSGSIZE);
			return -1;
		}

		req_len = (uint16)(sizeof(req->send_dgram) + size);

		if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_SEND_DGRAM, req_len)) == NULL) {
			os_sem_give(&si->send_lock);
			return -1;
		}

		req->send_dgram.s = htobl(s);
		req->send_dgram.flags = htobl(flags);

		if (to) {
			os_memcpy(&req->send_dgram.to, to, sizeof(req->send_dgram.to));
			req->send_dgram.tolen = htobl(tolen);
		} else {
			os_memset(&req->send_dgram.to, 0, sizeof(req->send_dgram.to));
			req->send_dgram.tolen = 0;
		}

		os_memcpy(&req->send_dgram + 1, data, size);
	}

	if ((slot = rsock_request(req_pkt)) < 0) {
		os_sem_give(&si->send_lock);
		return -1;
	}

	/* Put slot on end of socket sends list */

	rsock_slots[slot].next = -1;

	if (si->send_head < 0)
		si->send_head = slot;
	else
		rsock_slots[si->send_tail].next = slot;

	si->send_tail = slot;
	si->send_count++;

	os_sem_give(&si->send_lock);

	return size;
}

static int
rsock_send_stream(int s, void *data, int size, unsigned int flags)
{
	int total_sent = 0;

	while (size > 0) {
		int n;

		/* Break data into MTU-size chunks */
		n = RSOCK_MIN(size, RSOCK_SEND_STREAM_REQ_LIMIT);

		if ((n = rsock_send_raw(s, data, n, flags, NULL, 0)) < 0) {
			if (os_errno == EAGAIN && total_sent > 0)
				break;	/* Wrote part of data to non-blocking socket */
			return -1;
		}

		data = (void *)((uint8 *)data + n);
		size -= n;
		total_sent += n;
	}

	return total_sent;
}

/*
 * On success, returns the number of bytes sent.
 * For locally detected errors, returns -1 and set errno.
 * For remote errors, datagrams are silently dropped, and
 * stream write errors are (eventually) detected.
 */

int
rsock_send(int s, void *data, int size, unsigned int flags)
{
	if (s < 0 || s >= RSOCK_SOCKET_MAX ||
	    SI(s)->domain < 0 || (SI(s)->state & STATE_CLOSE_LOCAL)) {
		os_errno_set(EBADF);
		return -1;
	}

	if (SI(s)->type == SOCK_STREAM) {
		if (!(SI(s)->state & STATE_CONN)) {
			os_errno_set(ENOTCONN);
			return -1;
		}

		return rsock_send_stream(s, data, size, flags);
	} else
		return rsock_send_raw(s, data, size, flags, NULL, 0);
}

int
rsock_sendto(int s, void *data, int size, unsigned int flags,
             struct sockaddr *to, socklen_t tolen)
{
	if (s < 0 || s >= RSOCK_SOCKET_MAX ||
	    SI(s)->domain < 0 || (SI(s)->state & STATE_CLOSE_LOCAL)) {
		os_errno_set(EBADF);
		return -1;
	}

	if (SI(s)->type != SOCK_RAW && SI(s)->type != SOCK_DGRAM) {
		os_errno_set(EPROTOTYPE);
		return -1;
	}

	return rsock_send_raw(s, data, size, flags, to, tolen);
}

int
rsock_socket(int domain, int type, int proto)
{
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no, s_new = -1;

	OS_DEBUG("rsock_socket: domain=%d type=%d proto=%d\n", domain, type, proto);

	req_len = (uint16)sizeof(req->socket);

	if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_SOCKET, req_len)) == NULL)
		return -1;

	req->socket.domain = htobl(domain);
	req->socket.type = htobl(type);
	req->socket.proto = htobl(proto);

	if (rsock_call(req_pkt, TIMEOUT_NORMAL, &resp_pkt, &resp) < 0)
		return -1;

	err_no = btohs(resp->socket.header.err_no);

	if (err_no == 0) {
		rserv_pver = (btohl(resp->header.flags) &
		              RSOCK_FLAG_PVER_MASK) >> RSOCK_FLAG_PVER_SHIFT;

		s_new = btohl(resp->socket.s);

		OS_DEBUG("rsock_socket: pver=%d s_new=%d\n", rserv_pver, s_new);

		if (s_new < 0 || s_new >= RSOCK_SOCKET_MAX) {
			os_errno_set(EIO);
			return -1;
		}

		/* Initialize local structure for new socket */
		rsock_socket_init(s_new, domain, type, proto);

		/* Enable flow of receive data */
		if (rsock_socket_flow(s_new, 1) < 0) {
			(void)rsock_close(s_new);
			return -1;
		}
	}

	os_pkt_free(resp_pkt);

	return (rsock_errno_set(err_no) ? -1 : s_new);
}

int
rsock_write(int s, void *data, int size)
{
	return rsock_send(s, data, size, 0);
}

int
rsock_shutdown(int s, int how)
{
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no;

	OS_DEBUG("rsock_shutdown: s=%d how=%d\n", s, how);

	req_len = (uint16)sizeof(req->shutdown);

	if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_SHUTDOWN, req_len)) == NULL)
		return -1;

	req->shutdown.s = htobl(s);
	req->shutdown.how = htobl(how);

	if (rsock_call(req_pkt, TIMEOUT_NORMAL, &resp_pkt, &resp) < 0)
		return -1;

	err_no = btohs(resp->shutdown.header.err_no);

	os_pkt_free(resp_pkt);

	return rsock_errno_set(err_no);
}

static int
rsock_getnames(int s,
               struct sockaddr *local_name, int *local_name_len,
               struct sockaddr *remote_name, int *remote_name_len)
{
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no;

	req_len = (uint16)sizeof(req->getname);

	if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_GETNAME, req_len)) == NULL)
		return -1;

	req->getname.s = htobl(s);

	if (rsock_call(req_pkt, TIMEOUT_NORMAL, &resp_pkt, &resp) < 0)
		return -1;

	err_no = btohs(resp->socket.header.err_no);

	if (local_name != NULL)
		os_memcpy(local_name, &resp->getname.local_addr, sizeof(*local_name));
	if (local_name_len != NULL)
		*local_name_len = btohl(resp->getname.local_addrlen);
	if (remote_name != NULL)
		os_memcpy(remote_name, &resp->getname.remote_addr, sizeof(*remote_name));
	if (remote_name_len != NULL)
		*remote_name_len = btohl(resp->getname.remote_addrlen);

	os_pkt_free(resp_pkt);

	return rsock_errno_set(err_no);
}

int
rsock_getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
	return rsock_getnames(s, NULL, NULL, name, namelen);
}

int
rsock_getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
	return rsock_getnames(s, name, namelen, NULL, NULL);
}

int
rsock_getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)
{
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no;

	req_len = (uint16)sizeof(req->getsockopt);

	if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_GETSOCKOPT, req_len)) == NULL)
		return -1;

	req->getsockopt.s = htobl(s);
	req->getsockopt.level = htobl(level);
	req->getsockopt.optname = htobl(optname);

	if (rsock_call(req_pkt, TIMEOUT_NORMAL, &resp_pkt, &resp) < 0)
		return -1;

	err_no = btohs(resp->getsockopt.header.err_no);

	/* Currently, every supported option returns a single 'int' value */

	*optlen = btohl(resp->getsockopt.optlen);
	*(uint32 *)optval = btohl(*(uint32 *)resp->getsockopt.optval);

	os_pkt_free(resp_pkt);

	return rsock_errno_set(err_no);
}

int
rsock_setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no;

	req_len = (uint16)sizeof(req->setsockopt);

	if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_SETSOCKOPT, req_len)) == NULL)
		return -1;

	req->setsockopt.s = htobl(s);
	req->setsockopt.level = htobl(level);
	req->setsockopt.optname = htobl(optname);
	req->setsockopt.optlen = htobl(optlen);

	/* Currently, every supported option requires a single 'int' value */

	if (optlen < sizeof(int)) {
		os_errno_set(EINVAL);
		return -1;
	}

	*(uint32 *)req->setsockopt.optval = htobl(*(uint32 *)optval);

	if (rsock_call(req_pkt, TIMEOUT_NORMAL, &resp_pkt, &resp) < 0)
		return -1;

	err_no = btohs(resp->setsockopt.header.err_no);

	os_pkt_free(resp_pkt);

	return rsock_errno_set(err_no);
}

int
rsock_ioctl(int s, long cmd, void *argp)
{
	struct sockinfo *si;
	uint16 req_len;
	os_pkt_t *req_pkt;
	union rsock_req *req;
	os_pkt_t *resp_pkt;
	union rsock_resp *resp;
	int err_no;
	int retval_len;

	OS_DEBUG("rsock_ioctl: s=%d cmd=0x%lx\n", s, (unsigned long)cmd);

	if (s < 0 || s >= RSOCK_SOCKET_MAX || SI(s)->domain < 0) {
		os_errno_set(EBADF);
		return -1;
	}

	si = SI(s);

	if (cmd == FIONREAD) {
		uint32 *n_out = (uint32 *)argp;
		int i, len, hoff;

		/* Return amount buffered locally */

		*n_out = 0;
		hoff = si->recv_q_hoff;

		for (i = si->recv_q_rptr; i != si->recv_q_wptr; i = RECV_Q_INC(i)) {
			len = os_pkt_len(si->recv_q[i]);
			*n_out += len - hoff;
			hoff = RECV_HDRLEN(si->type);
		}

		return 0;
	}

	req_len = (uint16)sizeof(req->ioctl);

	switch (cmd) {
	case FIONBIO:
		req_len += 4;
		break;
	case SIOCSIFCONFIG:
		req_len += (int)sizeof(struct rsock_ifconfig);
		break;
	case SIOCGIFSTATS:
		req_len += IFNAMSIZ;
		break;
	}

	if ((req_pkt = rsock_req_alloc(&req, RSOCK_FN_IOCTL, req_len)) == NULL)
		return -1;

	req->ioctl.s = htobl(s);
	req->ioctl.cmd = htobl(cmd);

	/* Copy out args */

	switch (cmd) {
	case FIONBIO:
		{
			uint32 *flg_in = (uint32 *)argp;
			uint32 *flg_out = (uint32 *)(&req->ioctl + 1);

			ASSERT(flg_in != NULL);
			*flg_out = htobl(*flg_in);
		}
		break;
	case SIOCSIFCONFIG:
		{
			struct rsock_ifconfig *ifc_in =
			        (struct rsock_ifconfig *)argp;
			struct rsock_ifconfig *ifc_out =
			        (struct rsock_ifconfig *)(&req->ioctl + 1);

			ASSERT(ifc_in != NULL);
			ifc_out->ifc_flags = htobl(ifc_in->ifc_flags);
			ifc_out->ifc_index = htobl(ifc_in->ifc_index);
			ifc_out->ifc_metric = htobl(ifc_in->ifc_metric);
			ifc_out->ifc_mtu = htobl(ifc_in->ifc_mtu);
			os_memcpy(ifc_out->ifc_name, ifc_in->ifc_name,
			       sizeof(ifc_out->ifc_name));
			os_memcpy(&ifc_out->ifc_addr, &ifc_in->ifc_addr,
			       sizeof(struct sockaddr));
			os_memcpy(&ifc_out->ifc_netmask, &ifc_in->ifc_netmask,
			       sizeof(struct sockaddr));
			os_memcpy(&ifc_out->ifc_broadaddr, &ifc_in->ifc_broadaddr,
			       sizeof(struct sockaddr));
			os_memcpy(&ifc_out->ifc_hwaddr, &ifc_in->ifc_hwaddr,
			       sizeof(struct sockaddr));
			os_memcpy(&ifc_out->ifc_p2p, &ifc_in->ifc_p2p,
			       sizeof(struct sockaddr));
		}
		break;
	case SIOCGIFSTATS:
		{
			char *name_in = (char *)argp;
			char *name_out = (char *)(&req->ioctl + 1);

			ASSERT(name_in != NULL);
			os_memcpy(name_out, name_in, IFNAMSIZ);
		}
		break;
	}

	if (rsock_call(req_pkt, TIMEOUT_NORMAL, &resp_pkt, &resp) < 0)
		return -1;

	err_no = btohs(resp->ioctl.header.err_no);

	retval_len = (int)(btohs(resp->ioctl.header.len) -
	                   sizeof(resp->ioctl));

	if (err_no == 0) {
		/* Copy in return value and make local settings */

		switch (cmd) {
		case FIONBIO:
			{
				uint32 *flg_in = (uint32 *)argp;

				/* Maintain local flag also */
				if (*flg_in)
					si->state |= STATE_NONBLOCK;
				else
					si->state &= ~STATE_NONBLOCK;
			}
			break;
		case SIOCGIFCONFIG:
			{
				struct rsock_ifconfig *ifc_in =
				        (struct rsock_ifconfig *)(&resp->ioctl + 1);
				struct rsock_ifconfig *ifc_out =
				        (struct rsock_ifconfig *)argp;

				ASSERT(ifc_out != NULL);

				if (retval_len < sizeof(struct rsock_ifconfig)) {
					err_no = EIO;
					break;
				}

				ifc_out->ifc_flags = btohl(ifc_in->ifc_flags);
				ifc_out->ifc_index = btohl(ifc_in->ifc_index);
				ifc_out->ifc_metric = btohl(ifc_in->ifc_metric);
				ifc_out->ifc_mtu = btohl(ifc_in->ifc_mtu);
				os_memcpy(ifc_out->ifc_name, ifc_in->ifc_name,
				       sizeof(ifc_out->ifc_name));
				os_memcpy(&ifc_out->ifc_addr, &ifc_in->ifc_addr,
				       sizeof(struct sockaddr));
				os_memcpy(&ifc_out->ifc_netmask, &ifc_in->ifc_netmask,
				       sizeof(struct sockaddr));
				os_memcpy(&ifc_out->ifc_broadaddr, &ifc_in->ifc_broadaddr,
				       sizeof(struct sockaddr));
				os_memcpy(&ifc_out->ifc_hwaddr, &ifc_in->ifc_hwaddr,
				       sizeof(struct sockaddr));
				os_memcpy(&ifc_out->ifc_p2p, &ifc_in->ifc_p2p,
				       sizeof(struct sockaddr));
			}
			break;
		case SIOCGIFSTATS:
			{
				uint32 *ifs_in = (uint32 *)(&resp->ioctl + 1);
				uint32 *ifs_out = (uint32 *)argp;
				int i;

				if (retval_len < sizeof(struct rsock_ifstats)) {
					err_no = EIO;
					break;
				}

				for (i = 0; i < RSOCK_IFSTATS_COUNT; i++)
					ifs_out[i] = btohl(ifs_in[i]);
			}
			break;
		}
	}

	os_pkt_free(resp_pkt);

	return rsock_errno_set(err_no);
}

/* fcntl only supports manipulating O_NONBLOCK via F_SETFL/GETFL. */
int
rsock_fcntl(int s, int cmd, ...)
{
	va_list		ap;
	int		*arg_ptr, nb;

	if (s < 0 || s >= RSOCK_SOCKET_MAX || SI(s)->domain < 0) {
		os_errno_set(EBADF);
		return -1;
	}

	va_start(ap, cmd);

	switch (cmd) {
	case F_GETFL:
		arg_ptr = va_arg(ap, void *);
		*arg_ptr = ((SI(s)->state & STATE_NONBLOCK) ? O_NONBLOCK : 0);
		return 0;

	case F_SETFL:
		arg_ptr = va_arg(ap, void *);
		nb = (*arg_ptr & O_NONBLOCK) ? 1 : 0;
		return rsock_ioctl(s, FIONBIO, &nb);
	}

	os_errno_set(EINVAL);
	return -1;
}
