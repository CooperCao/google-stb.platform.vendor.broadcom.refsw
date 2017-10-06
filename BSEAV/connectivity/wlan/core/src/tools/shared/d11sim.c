/*
 * Copyright 2001, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 *
 * IEEE 802.11b Medium simulation server and client routines.
 *
 * Server listens on a TCP port for client connections. Keeps track
 * of clienets and polls for packets at 802.11b defined inter-packet
 * intervals. If more than one client requests medium, the server will
 * simulate medium access by choosing one client to succeed and failing
 * others. Server will go idle when there is no activity.
 * 
 * Clients connect to a server and respond to polls with packets if 
 * packets are ready to be sent. When the server is idle, Clients may
 * send unsolicited packets.
 *
 * $Id$
 *
 */

#include <unistd.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typedefs.h>
#include <utils.h>
#include <sys_xx.h>
#include <d11sim.h>

/* Globals */

int d11verbose = 0;

char* d11_msg_name[] = {
	"Packet Accepted",
	"Packet Rejected",
	"Wake Up",
	"Packet",
	"No Packet",
	"Poll SIFS",
	"Poll DIFS"
};

void
d11_msg_hton(d11_msg_t* msg)
{
	msg->sig = htonl(msg->sig);
	msg->id = htonl(msg->id);
	msg->time = htonl(msg->time);
	msg->tx_time = htonl(msg->tx_time);
	msg->data_len = htonl(msg->data_len);
}

void
d11_msg_ntoh(d11_msg_t* msg)
{
	msg->sig = ntohl(msg->sig);
	msg->id = ntohl(msg->id);
	msg->time = ntohl(msg->time);
	msg->tx_time = ntohl(msg->tx_time);
	msg->data_len = ntohl(msg->data_len);
}

int
d11_create_client(char *host, int port)
{
	struct sockaddr_in server_sin;
	struct hostent *hp = NULL;
	int csocket;
	
	if (*host != '\0' && (hp = gethostbyname(host)) == NULL) {
		fprintf(stderr, "gethostbyname error: %s\n", host);
		return -1;
	}

	bzero(&server_sin, sizeof (struct sockaddr_in));
	server_sin.sin_family = AF_INET;
	if (*host == '\0') {
		server_sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	} else {
		server_sin.sin_addr.s_addr = *(int *)(hp->h_addr);
	}
	server_sin.sin_port = htons((ushort)port);

	if ((csocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		syserr("socket call failed");

	if (connect(csocket, (struct sockaddr *) &server_sin, sizeof (struct sockaddr)) < 0)
		syserr ("connect");

	return csocket;
}

int
d11_msg_available(int s, int timeout_sec)
{
	fd_set rfds;
	struct timeval tv;
	struct timeval* timeout;
	int rc;
	
	/* Check for pending read IO on socket */
#ifdef __KERNEL__
	FD_ZERO((void *)&rfds);
	FD_SET(s, (void *)&rfds);
#else
	FD_ZERO(&rfds);
	FD_SET(s, &rfds);
#endif

	if (timeout_sec == -1) {
		timeout = NULL;
	} else {
		/* Set timeout as specified */
		tv.tv_sec = timeout_sec;
		tv.tv_usec = 0;
		timeout = &tv;
	}
	
	rc = select(s+1, &rfds, NULL, NULL, timeout);
	
	if (rc < 0) {
		syserr("select");
	}

	return rc > 0;
}

void
d11_send_msg(int s, uint32 id, uint32 data_len, void* data)
{
	d11_msg_t msg;
	
	ASSERT(data_len < D11SIM_MAXDATASZ);
	
	msg.id = id;
	msg.data_len = data_len;
	if (data_len > 0) {
		bcopy(data, msg.data, data_len);
	}
	d11_msg_hton(&msg);
	d11_write_msg(s, &msg);
}

void
d11_write_msg(int s, d11_msg_t* msg)
{
	int bytes;
	int len = D11SIM_MSGSZ + ntohl(msg->data_len);
	
	msg->sig = htonl(D11_MSG_SIGNATURE);

	bytes = write(s, (char*)msg, len);

	if (bytes != len)
		syserr("d11_write_msg: write");
}

int
d11_read_msg(int s, d11_msg_t* msg)
{
	int bytes;
	
	bytes = read(s, (char *)msg, D11SIM_MSGSZ);

	if (bytes < 0)
		syserr("d11_read_msg: read");
	if (bytes == 0) {
		return 0;
	}

	ASSERT(bytes == D11SIM_MSGSZ);

	d11_msg_ntoh(msg);

	if (msg->sig != D11_MSG_SIGNATURE) {
		fprintf(stderr, "d11_read_msg: message signature %08X did not match %08X\n",
			msg->sig, D11_MSG_SIGNATURE);
		exit(1);
	}
	
	if (msg->data_len > 0) {
		bytes = read(s, (char *)msg->data, msg->data_len);
		if (bytes < 0)
			syserr("d11_read_msg: data read");
		if (bytes == 0) {
			return 0;
		}
	}

	return 1;
}
