/*
 * Broadcom DHCP Server
 * Socket routines for Win32 
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: ce_socket.c,v 1.2 2010-04-01 17:38:36 $
 */

#include <winsock.h>

#include "dhcp.h"
#include "osl.h"
#include "packet.h"
#include "dhcpdebug.h"
#include "config.h"

#pragma comment( lib,"ws2.lib" )

struct sockaddr_in SenderAddr;
struct sockaddr_in serverAddr;	// Socket address for receive
struct sockaddr_in serv_addr;	// Socket address for sending (broadcast)

SOCKET hSocket;

int SenderAddrSize = sizeof(SenderAddr);

int socketBind() {
	int bcast = 1;
	int i;

    // Set up the socket.
	i=0;
	while(i++ < 10) {
		if (bind( hSocket, (const struct sockaddr *) &serverAddr , sizeof( serverAddr ) ) == SOCKET_ERROR)
		{
			DHCPLOG(("DHCP : failed to bind listener on a specific interface(0x%lx) and port, err=%d, trying again\n", htonl(serverAddr.sin_addr.s_addr), WSAGetLastError()));
			Sleep(1000);
		}
		else
		{
			DHCPLOG(("DHCP : success to bind listener on a specific interface(0x%lx) and port\n",htonl(serverAddr.sin_addr.s_addr)));
			break;
		}
	}

	// enable 'broadcast' to allow sending to a broadcast address(or sends will fail)
    if(setsockopt(hSocket, SOL_SOCKET, SO_BROADCAST, (const char *)&bcast, sizeof(bcast)) < 0){
  	  DHCPLOG(("DHCP : setsockopt SO_BROADCAST failed, err=%d\n",  WSAGetLastError()));
    }
	else
		DHCPLOG(("DHCP : setsockopt SO_BROADCAST succeeded\n"));

	// on linux, use SO_BINDTODEVICE via setsockopt() to bind the socket with a specific
	// network interface

	return DDOK;


}

int socketInit() {
	int status;

	// Initiate use of the WinSock DLL.
    WSADATA wsaData = { 0 };
    // Prefer version 2.2.
    if ((status = WSAStartup( MAKEWORD( 2, 2 ), &wsaData )) != 0)
	{
		DHCPLOG(("DHCP : socketInit failed to initialize WinSocket Dll, status=%d\n", status));
		return DDFAIL;
	}
 
    // Create a socket and store the handle to it.
    hSocket = socket( AF_INET, SOCK_DGRAM, 0 );

    // RECEIVE related variables
	//
	// Socket address for receive 
    // serverAddr = { 0 };
	memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons( DHCP_RECV_SOCKET );
    serverAddr.sin_addr.s_addr = htonl(gConfig.Subnet + 1); // INADDR_ANY; inet_addr("192.168.16.1");

	// Receive buffer & pointers
    // unsigned char buffer[1024] = { 0 };
	// struct bootp_frame *pRequest;

	// SEND related variables
	//
	// Socket address for send.
	memset(&serv_addr, '\0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	serv_addr.sin_port = htons(DHCP_SEND_SOCKET); 

	// Send buffer & pointers
	// struct bootp_frame *pOffer = (struct bootp_frame *) dhcp_offer_template;
	// struct bootp_frame *pAck = (struct bootp_frame *) dhcp_ack_template;


	return DDOK;
}

int socketDeinit() {
	closesocket(hSocket);

	if (WSACleanup() == SOCKET_ERROR)
		DHCPLOG(("DHCP : socketDeinit failed to cleanup WinSocket Dll, err=%d\n", WSAGetLastError()));

	return DDOK;
}

int socketGet(struct Packet **p) {
	int rVal;
	fd_set fd;
	struct timeval tm;

	ASSERT(p);

	if (DDOK != (rVal = packetAlloc(MAXBUFFERSIZE, p)))
		return rVal;


	/* sleep one second then check for data or shutdown request. */
	/* if no data or shutdown, sleep and check again */

	for (rVal = 0; !rVal; ) {
		FD_ZERO(&fd);
		FD_SET(hSocket, &fd);

		tm.tv_sec = 1;
		tm.tv_usec = 0;

		rVal = select(hSocket + 1, &fd, NULL, NULL, &tm);

		if (gDHCPStatus & DS_EXITPENDING)
		{
			DHCPLOG(("DHCP : socketGet ignore the packet before shutdown\n"));
			packetFree(*p);
			*p = NULL;
			return DDSHUTDOWN;
		}
	}

	SenderAddrSize = sizeof(SenderAddr);		// always reset it
	rVal = recvfrom( hSocket, (char *) (*p)->Data, (*p)->maxSize, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize );

	if (rVal < 0) {
		int err = WSAGetLastError();
		packetFree(*p);
		*p = NULL;
		return DDFAIL;
	}

	(*p)->Size = rVal;

	//DHCPLOG(("DHCP : receive %d bytes via socket from %s\n", rVal, inet_ntoa(SenderAddr.sin_addr)));

	return DDOK;
}

int socketSend(struct Packet *p) {
	int rVal;

	ASSERT(p);

	// set up for sending
	rVal = sendto( hSocket, (const char *) p->Data, p->Size, 0,(SOCKADDR *)&serv_addr, sizeof(serv_addr));
	DHCPLOG(("DHCP : tx via socketSend to %s(port#=%d, txSize = %d, actualTxSize = %d)\n", inet_ntoa(serv_addr.sin_addr), htons(serv_addr.sin_port), p->Size, rVal));

	if (rVal < 0)
	{
		DHCPLOG(("DHCP : tx via socketSend failed (txSize = %d, actualTxSize = %d)\n",
					p->Size, rVal));
		return DDFAIL;
	}

	return DDOK;
}
