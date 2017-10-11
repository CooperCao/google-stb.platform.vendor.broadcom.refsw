/*
 * RSOCK (Remote Sockets) - protocol version 1
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

#ifndef __RSOCK_PROTO_H__
#define __RSOCK_PROTO_H__

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>


#define RSOCK_SOCKET_MAX	10	/* Maximum open descriptors */

#ifndef RSOCK_BUS_MTU
#define RSOCK_BUS_MTU		1500	/* Optimal MTU >= MSS(1460) + hdr(36) */
#endif

#define RSOCK_BUS_SEND_WIN	3	/* Max outstanding send requests */
#define RSOCK_BUS_RECV_WIN	3	/* Max outstanding rdata messages */

/*
 * The RSOCK handle consists of a unique number per remote request.
 * Responses coming back are matched up with the request number.
 */

typedef uint16 rsock_handle_t;

/*
 * Request function codes
 */

enum rsock_fn {
	RSOCK_FN_SOCKET = 0,
	RSOCK_FN_ACCEPT = 1,
	RSOCK_FN_FLOW = 2,
	RSOCK_FN_BIND = 3,
	RSOCK_FN_SHUTDOWN = 4,
	RSOCK_FN_GETNAME = 5,
	RSOCK_FN_GETSOCKOPT = 6,
	RSOCK_FN_SETSOCKOPT = 7,
	RSOCK_FN_CLOSE = 8,
	RSOCK_FN_CONNECT = 9,
	RSOCK_FN_LISTEN = 10,
	RSOCK_FN_SEND_DGRAM = 11,
	RSOCK_FN_SEND_STREAM = 12,
	RSOCK_FN_IOCTL = 13,
	RSOCK_FN__COUNT = 14		/* last */
};

/*
 * Error codes
 */

enum rsock_err {
	RSOCK_OK = 0,
	RSOCK_EBADF = 1,
	RSOCK_ENOMEM = 2,
	RSOCK_ENOBUFS = 3,
	RSOCK_ECONNABORTED = 4,
	RSOCK_ECONNRESET = 5,
	RSOCK_ESHUTDOWN = 6,
	RSOCK_ENOTCONN = 7,
	RSOCK_EINVAL = 8,
	RSOCK_EIO = 9,
	RSOCK_EHOSTUNREACH = 10,
	RSOCK_EADDRINUSE = 11,
	RSOCK_ENOPROTOOPT = 12,
	RSOCK_ESOCKTNOSUPPORT = 13,
	RSOCK_ENOSYS = 14,
	RSOCK_EAGAIN = 15,
	RSOCK_EINTR = 16,
	RSOCK_E__COUNT = 17		/* last */
};

/* Message header flags */

#define RSOCK_FLAG_TYPE_MASK	0x7
#define RSOCK_FLAG_TYPE_REQ	1	/* Request type */
#define RSOCK_FLAG_TYPE_RESP	2	/* Response type */
#define RSOCK_FLAG_TYPE_RDATA	3	/* Receive data type (uses resp_recv format) */
#define RSOCK_FLAG_TYPE_RACK	4	/* Receive ack type (uses req_recv format) */
#define RSOCK_FLAG_FORCE	0x10	/* Option for RSOCK_FN_CLOSE */
#define RSOCK_FLAG_PVER_MASK	0xf000	/* Protocol version mask */
#define RSOCK_FLAG_PVER_SHIFT	12	/* Protocol version shift */

/* Common header for all rsock messages detailed below */

BWL_PRE_PACKED_STRUCT struct rsock_common_header {
	uint16 flags;			/* RSOCK_FLAG_xx */
	uint16 len;			/* Length of entire packet */
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_req_header {		/* Request message header */
	uint16 flags;			/* RSOCK_FLAG_xx */
	uint16 len;			/* Length of entire packet */
	rsock_handle_t handle;		/* Unique transaction ID */
	uint16 fn;			/* enum rsock_fn (except 16 bits) */
	/* Request arguments follow the header, and are specific to each fn */
	/* For send requests, the send data then follows the arguments */
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_header {		/* Response message header */
	uint16 flags;			/* RSOCK_FLAG_xx */
	uint16 len;			/* Length of entire packet */
	rsock_handle_t handle;		/* Unique transaction ID echoed back */
	uint16 err_no;			/* RSOCK_E_xx; 0 if no error */
	/* Return values follow the header, and are specific to each fn */
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_rdata_header {		/* Receive data header */
	uint16 flags;			/* RSOCK_FLAG_xx */
	uint16 len;			/* Length of entire packet */
	uint16 s;			/* Socket number */
	uint16 _pad;			/* Reserved */
	/* Receive params follow the header, and are specific to the socket type */
	/* Variable-length receive data follows the params */
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_rack_header {		/* Receive acknowledgement header */
	uint16 flags;			/* RSOCK_FLAG_xx */
	uint16 len;			/* Length of entire packet */
	uint16 s;			/* Socket number */
	uint16 _pad;			/* Reserved */
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT union rsock_header {
	struct rsock_common_header common;
	struct rsock_req_header req;
	struct rsock_resp_header resp;
	struct rsock_rdata_header rdata;
	struct rsock_rack_header rack;
} BWL_POST_PACKED_STRUCT;

/*
 * Format of specific messages of each type.
 */

#define RSOCK_IOCTL_MAX		128	/* Size limited by IOCPARM_MASK */
#define RSOCK_SOCKOPT_MAX	4

BWL_PRE_PACKED_STRUCT struct rsock_req_accept {		/* ACCEPT */
	struct rsock_req_header header;
	int32 s;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_accept {
	struct rsock_resp_header header;
	int32 s;
	struct sockaddr addr;
	int32 addrlen;
} BWL_POST_PACKED_STRUCT;

#define RSOCK_FLOW_XOFF		0
#define RSOCK_FLOW_XON		1

BWL_PRE_PACKED_STRUCT struct rsock_req_flow {			/* FLOW */
	struct rsock_req_header header;
	int32 s;
	uint32 status;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_flow {
	struct rsock_resp_header header;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_req_bind {			/* BIND */
	struct rsock_req_header header;
	int32 s;
	struct sockaddr addr;
	int32 addrlen;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_bind {
	struct rsock_resp_header header;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_req_connect {		/* CONNECT */
	struct rsock_req_header header;
	int32 s;
	struct sockaddr addr;
	int32 addrlen;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_connect {
	struct rsock_resp_header header;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_req_shutdown {		/* SHUTDOWN */
	struct rsock_req_header header;
	int32 s;
	int32 how;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_shutdown {
	struct rsock_resp_header header;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_req_getname {		/* GETNAME */
	struct rsock_req_header header;
	int32 s;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_getname {
	struct rsock_resp_header header;
	struct sockaddr local_addr;
	int32 local_addrlen;
	struct sockaddr remote_addr;
	int32 remote_addrlen;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_req_getsockopt {		/* GETSOCKOPT */
	struct rsock_req_header header;
	int32 s;
	int32 level;
	int32 optname;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_getsockopt {
	struct rsock_resp_header header;
	int32 optlen;
	uint8 optval[RSOCK_SOCKOPT_MAX];
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_req_setsockopt {		/* SETSOCKOPT */
	struct rsock_req_header header;
	int32 s;
	int32 level;
	int32 optname;
	int32 optlen;
	uint8 optval[RSOCK_SOCKOPT_MAX];
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_setsockopt {
	struct rsock_resp_header header;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_req_close {		/* CLOSE */
	struct rsock_req_header header;
	int32 s;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_close {
	struct rsock_resp_header header;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_req_listen {		/* LISTEN */
	struct rsock_req_header header;
	int32 s;
	int32 backlog;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_listen {
	struct rsock_resp_header header;
} BWL_POST_PACKED_STRUCT;

#define RSOCK_SEND_DGRAM_REQ_HDRLEN	((int)sizeof(struct rsock_req_send_dgram))
#define RSOCK_SEND_DGRAM_REQ_LIMIT	(RSOCK_BUS_MTU - RSOCK_SEND_DGRAM_REQ_HDRLEN)

BWL_PRE_PACKED_STRUCT struct rsock_req_send_dgram {		/* SEND_DGRAM */
	struct rsock_req_header header;
	int32 s;
	uint32 flags;			/* MSG_xx */
	struct sockaddr to;
	int32 tolen;
	/* Variable-length send data follows */
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_send_dgram {
	struct rsock_resp_header header;
} BWL_POST_PACKED_STRUCT;

#define RSOCK_SEND_STREAM_REQ_HDRLEN	((int)sizeof(struct rsock_req_send_stream))
#define RSOCK_SEND_STREAM_REQ_LIMIT	(RSOCK_BUS_MTU - RSOCK_SEND_STREAM_REQ_HDRLEN)

BWL_PRE_PACKED_STRUCT struct rsock_req_send_stream {		/* SEND_STREAM */
	struct rsock_req_header header;
	int32 s;
	uint32 flags;			/* MSG_xx */
	/* Variable-length send data follows */
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_send_stream {
	struct rsock_resp_header header;
	int32 sent_len;
} BWL_POST_PACKED_STRUCT;

/* Receive data messages sent from server to client */
#define RSOCK_RDATA_DGRAM_HDRLEN	((int)sizeof(struct rsock_rdata_dgram))
#define RSOCK_RDATA_DGRAM_LIMIT		(RSOCK_BUS_MTU - RSOCK_RDATA_DGRAM_HDRLEN)

BWL_PRE_PACKED_STRUCT struct rsock_rdata_dgram {
	struct rsock_rdata_header header;
	struct sockaddr from;
	int32 fromlen;
	/* Variable-length receive data follows */
} BWL_POST_PACKED_STRUCT;

#define RSOCK_RDATA_STREAM_HDRLEN	((int)sizeof(struct rsock_rdata_stream))
#define RSOCK_RDATA_STREAM_LIMIT	(RSOCK_BUS_MTU - RSOCK_RDATA_STREAM_HDRLEN)

BWL_PRE_PACKED_STRUCT struct rsock_rdata_stream {
	struct rsock_rdata_header header;
	/* Variable-length receive data follows */
} BWL_POST_PACKED_STRUCT;

/* Receive data acknowledgement message sent from client to server */
BWL_PRE_PACKED_STRUCT struct rsock_rack_dgram {
	struct rsock_rack_header header;
	int32 count;			/* Number of rdata packets being acknowledged */
} BWL_POST_PACKED_STRUCT;

/* (the protocol and code are written to allow stream/dgram to differ in the future) */
BWL_PRE_PACKED_STRUCT struct rsock_rack_stream {
	struct rsock_rack_header header;
	int32 count;			/* Number of rdata packets being acknowledged */
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_req_socket {		/* SOCKET */
	struct rsock_req_header header;
	int32 domain;
	int32 type;
	int32 proto;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_socket {
	struct rsock_resp_header header;
	int32 s;
} BWL_POST_PACKED_STRUCT;

#define RSOCK_IOCTL_LIMIT \
	(RSOCK_IOCTL_MAX - (int)sizeof(struct rsock_req_ioctl))

BWL_PRE_PACKED_STRUCT struct rsock_req_ioctl {		/* IOCTL */
	struct rsock_req_header header;
	int32 s;
	int32 cmd;
	/* Variable-length input argument follows */
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_resp_ioctl {
	struct rsock_resp_header header;
	/* Variable-length output argument follows */
} BWL_POST_PACKED_STRUCT;

/*
 * Composite messages
 */

BWL_PRE_PACKED_STRUCT union rsock_req {
	struct rsock_req_header header;
	struct rsock_req_accept accept;
	struct rsock_req_flow flow;
	struct rsock_req_bind bind;
	struct rsock_req_connect connect;
	struct rsock_req_shutdown shutdown;
	struct rsock_req_getname getname;
	struct rsock_req_getsockopt getsockopt;
	struct rsock_req_setsockopt setsockopt;
	struct rsock_req_close close;
	struct rsock_req_listen listen;
	struct rsock_req_send_dgram send_dgram;
	struct rsock_req_send_stream send_stream;
	struct rsock_req_socket socket;
	struct rsock_req_ioctl ioctl;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT union rsock_resp {
	struct rsock_resp_header header;
	struct rsock_resp_accept accept;
	struct rsock_resp_flow flow;
	struct rsock_resp_bind bind;
	struct rsock_resp_connect connect;
	struct rsock_resp_shutdown shutdown;
	struct rsock_resp_getname getname;
	struct rsock_resp_getsockopt getsockopt;
	struct rsock_resp_setsockopt setsockopt;
	struct rsock_resp_close close;
	struct rsock_resp_listen listen;
	struct rsock_resp_send_dgram send_dgram;
	struct rsock_resp_send_stream send_stream;
	struct rsock_resp_socket socket;
	struct rsock_resp_ioctl ioctl;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT union rsock_rdata {
	struct rsock_rdata_header header;
	struct rsock_rdata_dgram dgram;
	struct rsock_rdata_stream stream;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT union rsock_rack {
	struct rsock_rack_header header;
	struct rsock_rack_dgram dgram;
	struct rsock_rack_stream stream;
} BWL_POST_PACKED_STRUCT;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>


#endif /* __RSOCK_PROTO_H__ */
