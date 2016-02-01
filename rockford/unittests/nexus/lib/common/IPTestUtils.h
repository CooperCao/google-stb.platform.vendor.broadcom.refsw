/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Utils source for IP related unit test application
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *************************************************************/

/* Error Codes */
#define HTTP_ERROR                  -20
#define SOCKET_ERROR                -21
#define SOCKET_IO_ERROR             -22

#define MODE_IS_INCLUDE             1
#define MODE_IS_EXCLUDE             2
#define CHANGE_TO_INCLUDE_MODE      3
#define CHANGE_TO_EXCLUDE_MODE      4

#define JOIN_IGMP_GROUP    CHANGE_TO_EXCLUDE_MODE
#define LEAVE_IGMP_GROUP   CHANGE_TO_INCLUDE_MODE

#define IGMP_V3_GROUP_MEMBERSHIP_DEST_ADDRESS "224.0.0.22"

/* This functions waits for a network event on a specified socket
 * descriptor for a specified timeout. Returns -1 is no event occurred */
int waitForNetEvent(int sd, int timeout);

/* This function opens a tcp socket, given the ip address and port number,
 * binds to it and listens on it. It returns the socket descriptor. */
int tcpListen(int port);

/* This function waits for connection on the given socket and accepts them. It returns the
 * remote ip address and the packet payload, once a packet is received on that socket.
 * It times out after 'timeout' seconds if no input connections are seen from the client */
int tcpAccept(int sd, int timeout, unsigned long *ipaddr, unsigned char *recvbuf);

/* This function opens a tcp socket, given the ip address and port number,
 * binds and connects to it. It returns the socket descriptor */
int tcpConnect(char *server,     /* In: Server IP address in numbers-and-dots format */
              int server_port,  /* In: Server port number */
              int client_port   /* In: Client port number */
              );

/* This functions sends and/or receives data to/from a socket descriptor */
int tcpPayloadSendReceive(int sd,       /* In: TCP Socket Descriptor */
                          char *buf,    /* In: Message to send over the socket; If null, only receive */
                          char *recv,   /* Out: Buffer to receive response from socket */
                          int size,     /* In: Receive buffer size - If null, only transmit */
                          char *msg     /* In: Message type */
                          );

/* This function returns the ip address of the first non-local-host IPV4 interface */
int getFirstIPV4Address(void
                        );

/* Closes the tcp/udp connection */
int closeConnection(int sd /* TCP/UDP Socket Descriptor */
                    );

/* Computes the checksum to be populated in IP/IGMP headers */
unsigned short int checksum (unsigned short int *addr,	/* In: Buffer address */
							 int len /* In: Buffer length */
							 );

/* Sends an IGMP V43 membership report to the specified multicast group address */
int sendIGMPMembershipReport(int src_address, /* In: Source IP address */
                             int src_port, /* In: Source port number*/
                             unsigned int group_address, /* In: IGMP group address */
                             unsigned short group_type /* In:  IGMP group record type */
                             );