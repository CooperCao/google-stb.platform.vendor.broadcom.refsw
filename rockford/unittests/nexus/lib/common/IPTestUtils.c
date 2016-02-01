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
 * Module Description: Utils source for DTCP unit test app
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/syscall.h>
#include <netdb.h>

#include <sys/ioctl.h>
#include <net/if.h>

#include <assert.h>
#include <stdbool.h>

#include <linux/igmp.h>
#include <netinet/ip.h>
#include <ifaddrs.h>

#include "TestUtils.h"
#include "IPTestUtils.h"

#ifdef HAS_NEXUS /* TBD: Need to find the appropriate define */
	#include "nexus_platform.h"
	BDBG_MODULE(IpTestUtils);
#else
	#define	BDBG_ERR(A) 	\
		do {				\
			printf A;		\
			printf("\n");	\
		} while (0)

	#define BDBG_WRN	BDBG_ERR
	#define BDBG_LOG	BDBG_ERR
#endif

/* This functions waits for a network event on a specified socket
 * descriptor for a specified timeout. Returns -1 is no event occurred */
int waitForNetEvent(int sd, int timeout)
{
	fd_set rfds;
	struct timeval tv;

	while (timeout--) {
		FD_ZERO(&rfds);
		FD_SET(sd, &rfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		if ( select(sd +1, &rfds, NULL, NULL, &tv) < 0 ) {
			perror("ERROR: select(): exiting...");
			break;
		}

		if (!FD_ISSET(sd, &rfds))
		{
			/* No request from Client yet, go back to select loop */
			continue;
		}
		else
		{
			return NO_ERROR;
		}
	}

    return ERROR;
}


/* This function opens a tcp socket, given the ip address and port number,
 * binds to it and listens on it. It returns the socket descriptor. */
int tcpListen(int port)
{
    int sd;
    int reuse_flag = 1;
    struct sockaddr_in localAddr;

    if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        /* Socket Create Error */
        BDBG_ERR(("Socket Open Err"));
        return SOCKET_ERROR;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(port);

    if(setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_flag, sizeof(reuse_flag) ) < 0 ) {
        BDBG_ERR(("REUSE Socket Error"));
        goto error;
    }

    if (bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr))) {
        BDBG_ERR(("Socket Bind Err"));
        goto error;
    }

    if (listen(sd, 4)) {
        BDBG_ERR(("Socket listen Err"));
        goto error;
    }

    return sd;

error:
    close(sd);
    return SOCKET_ERROR;
}

/* This function waits for connection on the given socket and accepts them. It returns the
 * remote ip address and the packet payload, once a packet is received on that socket.
 * It times out after 'timeout' seconds if no input connections are seen from the client */
int tcpAccept(int sd, int timeout, unsigned long *ipaddr, unsigned char *recvbuf)
{
	int nsd = -1;
	int ret;
	int addrlen;
	char remoteIp[100];
	struct sockaddr_in remoteAddr;

	*ipaddr = 0;

	/* Now we wait for an input connection */
	ret = waitForNetEvent(sd, timeout);
	if (ret == NO_ERROR)
	{
		/* Accept the connection */
		addrlen = sizeof(remoteAddr);
		if ((nsd = accept(sd, (struct sockaddr *)&remoteAddr, (socklen_t *)&addrlen)) < 0 )
		{
			BDBG_ERR(("ERROR: accept(): Could not accept input connection"));
		}
		else
		{
			snprintf (remoteIp, 100, "%d.%d.%d.%d", (remoteAddr.sin_addr.s_addr & 0xFF), ((remoteAddr.sin_addr.s_addr >> 8) & 0xFF), ((remoteAddr.sin_addr.s_addr >> 16) & 0xFF), ((remoteAddr.sin_addr.s_addr >> 24) & 0xFF));
			BDBG_LOG(("Accepted Connection from %s:%d", remoteIp, (int)ntohs(remoteAddr.sin_port)));
			waitForNetEvent(nsd, timeout);

			if ((ret = read(nsd, recvbuf, 1024)) <= 0)
			{
				BDBG_ERR(("ERROR: failed reading the data from the client"));
			}
			else
			{
				BDBG_LOG(("Received message from client %s: size %d", remoteIp, ret));
				/*recvbuf[ret] = '\0';
				for (i = 0; i < ret; i++)
				{
					printf ("%02x", recvbuf[i]);
				}
				printf ("\n");*/
				*ipaddr = (unsigned long)remoteAddr.sin_addr.s_addr;
			}
		}
	}

	if (*ipaddr == 0)
	{
		return SOCKET_ERROR;
	}

	return nsd;
}

/* This function opens a tcp socket, given the ip address and port number,
 * binds and connects to it. It returns the socket descriptor */
int tcpConnect(char *server,     /* In: Server IP address in numbers-and-dots format */
			   int server_port,  /* In: Server port number */
			   int client_port   /* In: Client port number */
			   )
{
    int sd,rc;

    struct sockaddr_in localAddr;
    char portString[16];
    struct addrinfo hints;
    struct addrinfo *addrInfo = NULL;
    struct addrinfo *addrInfoNode = NULL;

    printf("TCP - Connection to %s:%d ...\n",server, server_port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* we dont know the family */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    memset(portString, 0, sizeof(portString));  /* getaddrinfo() requires port # in the string form */
    snprintf(portString, sizeof(portString)-1, "%d", server_port);

    if (getaddrinfo(server, portString, &hints, &addrInfo) != 0) {
        printf("\nERROR: getaddrinfo failed for server:port: %s:%d\n", server, server_port);
        perror("getaddrinfo");
        return INVALID_ARGUMENT;
    }

    for (addrInfoNode = addrInfo; addrInfoNode != NULL; addrInfoNode = addrInfo->ai_next) {
        if (addrInfoNode->ai_family == AF_INET)
            break;
    }
    if (!addrInfoNode) {
        perror("%s: ERROR: no IPv4 address available for this server, no support for IPv6 yet");
        return INVALID_ARGUMENT;
    }

    addrInfo = addrInfoNode;

    /* Open Socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd<0) {
        perror("Socket Open Err");
        return INVALID_ARGUMENT;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(client_port);

    if (bind(sd,(struct sockaddr *) &localAddr, sizeof(localAddr))) {
        perror("Socket Bind Err");
        close(sd);
        return INVALID_ARGUMENT;
    }

    rc = connect(sd, addrInfo->ai_addr, addrInfo->ai_addrlen);

    if (rc<0) {
        perror("Connect Error: Server busy or unavailable:");
        close(sd);
        return SOCKET_ERROR;
    }

    printf("TCP: Connection Success\n");
    return sd;
}

int tcpPayloadSendReceive(int sd,       /* In: TCP Socket Descriptor */
                          char *buf,    /* In: Message to send over the socket; If null, only receive */
                          char *recv,   /* In: Buffer to receive response from socket */
                          int size,     /* In: Receive buffer size - If null, only transmit */
                          char *msg     /* In: Message type */
                          )
{
    int ret;

    if (msg == NULL)
    {
        msg = "";
    }

    if ((buf == NULL) && (recv == NULL))
    {
        return INVALID_ARGUMENT;
    }

    /* send the command */
    if (buf)
    {
        ret = write(sd, buf, strlen(buf));
        if ((unsigned)ret != strlen(buf)) {
            printf("Failed to send %s Request: rc %d\n", msg, ret);
            return SOCKET_IO_ERROR;
        }
    }

    /* receive response */
    if (recv)
    {
        memset(recv, 0, size);
        if ( (ret = read(sd, recv, size)) <= 0) {
            printf("Failed to read response to %s command : ret = %d\n", msg, ret);
            return SOCKET_IO_ERROR;
        }
    }

    return NO_ERROR;
}

int getFirstIPV4Address(void)
{
    struct ifaddrs *ifaddr, *ifa;
    int addr = -1, ip;

    getifaddrs(&ifaddr);
    ifa = ifaddr;
    while (ifa)
    {
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            ip = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
            if ((unsigned int)ip != htonl(0x7F000001))
            {
                /*printf ("ipaddr = %s:0x%08x\n", inet_ntoa(*(struct in_addr *)&(ip)), ip);*/
                addr = ip;
                break;
            }
        }
        ifa = ifa->ifa_next;
    }

    freeifaddrs(ifaddr);

    return addr;
}

int closeConnection(int sd /* TCP/UDP Socket Descriptor */
                    )
{
    return shutdown(sd, 2);
}

unsigned short int checksum (unsigned short int *addr,	/* In: Buffer address */
							 int len /* In: Buffer length */
							 )
{
    int nleft = len;
    int sum = 0;
    unsigned short int *w = addr;
    unsigned short int answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= sizeof (unsigned short int);
    }

    if (nleft == 1) {
        *(unsigned char *) (&answer) = *(unsigned char *) w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}

int sendIGMPMembershipReport(int src_address, /* In: Source IP address */
                             int src_port, /* In: Source port number*/
                             unsigned int group_addr, /* In: IGMP group address */
                             unsigned short group_type /* In:  IGMP group record type */
                             )
{
    int ret = NO_ERROR, dest_address, sd;
	struct iphdr ip_hdr;
	struct igmpv3_report* igmp_report_hdr;
	struct sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	int one = 1;
	const int *val = &one;
    int igmp_hdr_size, packet_len;
	char packet[sizeof(struct iphdr) + sizeof(struct igmpv3_report)+ sizeof(struct igmpv3_grec)];

    igmp_hdr_size = sizeof(struct igmpv3_report)+ sizeof(struct igmpv3_grec);
    packet_len = sizeof(struct iphdr) + igmp_hdr_size;

	local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(src_port);

	dest_address = inet_addr(IGMP_V3_GROUP_MEMBERSHIP_DEST_ADDRESS);

	remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = dest_address;
    remote_addr.sin_port = htons(5000);

    sd = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
    if (sd < 0)
    {
        printf ("Error opening socket for sending IGMP message\n");
        return SOCKET_ERROR;
    }

    memset(&ip_hdr, 0, sizeof(struct iphdr));

    ip_hdr.ihl = 5;
    ip_hdr.version = 4;
    ip_hdr.tos = 0;

    ip_hdr.tot_len = htons(packet_len);
    ip_hdr.id = htons(3); /*check if id has to be from the original socket*/
    ip_hdr.frag_off = 0x00;
    ip_hdr.ttl = 1;
    ip_hdr.protocol = IPPROTO_IGMP;
    ip_hdr.check = 0;
	ip_hdr.saddr = src_address;
	ip_hdr.daddr = dest_address;

	igmp_report_hdr = malloc(igmp_hdr_size);
	igmp_report_hdr->type = 0x22;
	igmp_report_hdr->resv1 =0;
	igmp_report_hdr->csum = 0;
	igmp_report_hdr->resv2 =0;
	igmp_report_hdr->ngrec = htons(1);
	igmp_report_hdr->grec[0].grec_type = group_type;
	igmp_report_hdr->grec[0].grec_auxwords = 0;
	igmp_report_hdr->grec[0].grec_nsrcs =0;
	igmp_report_hdr->grec[0].grec_mca = group_addr;
	igmp_report_hdr->grec[0].grec_src[0] = 0;
	igmp_report_hdr->csum = checksum((unsigned short int *) igmp_report_hdr, igmp_hdr_size);

    ip_hdr.check = checksum((unsigned short int *) &ip_hdr, sizeof(struct iphdr));

    memset(packet, 0, packet_len);
    memcpy(packet, &ip_hdr, sizeof(struct iphdr));
	memcpy(packet + sizeof(struct iphdr), igmp_report_hdr, igmp_hdr_size);

    ret = setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one));
    if (ret < 0)
    {
        printf ("Error sending IGMP group membership report\n");
        ret = SOCKET_IO_ERROR;
        goto close_socket;
    }

    /*printf("Sending to %s:%d ", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
    printf("from %s:%d \n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));*/

    ret = sendto(sd, packet, packet_len, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
    if (ret < 0)
    {
        printf ("Error sending IGMP group membership report\n");
        ret = SOCKET_IO_ERROR;
    }

close_socket:
    shutdown(sd, 2);
    close(sd);

    free(igmp_report_hdr);

    return ret;
}
