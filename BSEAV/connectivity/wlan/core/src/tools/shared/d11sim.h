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
 * IEEE 802.11b Medium Simulation server and client defs
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

#ifndef _D11SIM_H
#define _D11SIM_H

#define D11_SERVERPORT 18888

#define D11_MSG_SIGNATURE	0x80211

#define D11_MSG_ACCEPT		0
#define D11_MSG_REJECT		1
#define D11_MSG_WAKEUP		2
#define D11_MSG_PKT		3
#define D11_MSG_NOPKT		4
#define D11_MSG_POLL_SIFS	5
#define D11_MSG_POLL_DIFS	6
#define D11_MSG_POLL_TIMER	7
	
#define D11SIM_MSGSZ 20
#define D11SIM_MAXDATASZ	(16*1024)

typedef struct {
	uint32 sig;
	int32 id;
	uint32 time;
	uint32 tx_time;
	int32 data_len;
	unsigned char data[D11SIM_MAXDATASZ];
} d11_msg_t;

extern int d11verbose;
extern char* d11_msg_name[];

int  d11_create_client(char *host, int port);
int  d11_msg_available(int s, int timeout_sec);
void d11_send_msg(int s, uint32 id, uint32 data_len, void* data);
int  d11_read_msg(int s, d11_msg_t* msg);
void d11_write_msg(int s, d11_msg_t* msg);
void d11_msg_hton(d11_msg_t* msg);
void d11_msg_ntoh(d11_msg_t* msg);

#endif
