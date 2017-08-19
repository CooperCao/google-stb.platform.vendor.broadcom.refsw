/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <assert.h>
#include <unistd.h>
#include "zigbee_socket.h"
#include "zigbee_rpc_frame.h"

/*#define SRV_IP "127.0.0.1"*/
/*#define SRV_IP "10.13.134.212"*/

static int s_client;
#ifdef SOCKET_UDP
static struct sockaddr_in server;

#define PORT 9930
#define SRV_IP "10.13.135.196"
int Zigbee_Socket_ClientOpen(char *hostIpAddr)
{
    if ((s_client=socket(AF_INET, SOCK_DGRAM, 0))==-1) {
        perror("opening stream socket");
        exit(1);
    }

    memset((char *) &server, 0, sizeof(server));
    server.sin_family = AF_INET;

    #ifdef SRV_IP
        if (inet_aton(SRV_IP, &server.sin_addr)==0) {
            printf("inet_aton error\n");
        }
    #else
        hp = gethostbyname(hostIpAddr);
        if (hp == 0) {
            fprintf(stderr, "%s: unknown host\n", argv[2]);
            exit(2);
        }
        memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
    #endif

    #ifdef PORT
        server.sin_port = htons(PORT);
    #else
        server.sin_port = htons(atoi(argv[3]));
    #endif
}
#else
int Zigbee_Socket_ClientOpen(char *hostIpAddr)
{
    int s;
    struct addrinfo hints, *servinfo, *p;
    int connect_err=0;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

#ifdef SRV_IP
    s = getaddrinfo(SRV_IP, PORT_STR, &hints, &servinfo);
#else
    s = getaddrinfo(hostIpAddr, PORT_STR, &hints, &servinfo);
#endif

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((s_client = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        if (connect(s_client, p->ai_addr, p->ai_addrlen) == -1) {
            close(s_client);
            //perror("connect");
            connect_err=1;
            continue;
        }

        break; // if we get here, we must have connected successfully
    }

    freeaddrinfo(servinfo);
    return (connect_err ? -1 : 0);
}
#endif

void Zigbee_Socket_ClientSend(unsigned int *buf, int len, int socket)
{
    int i;
    (void)socket; /* unused */

#ifdef SOCKET_UDP
    int slen=sizeof(server);
#endif

    if ((unsigned int)len != buf[RPC_FRAME_LENGTH_OFFSET]) {
        printf("len=%d, buf[RPC_FRAME_LENGTH_OFFSET]=%d\n", len, buf[RPC_FRAME_LENGTH_OFFSET]);
        assert(0);
    }

    #ifdef DEBUG
        printf("ZIGBEE_SOCKET_CLIENT:  send %d words to server:", len);
        {
            int i;
            for (i=0; i<len; i++) {
                if (i%4==0) printf("\n");
                printf("%08x ", buf[i]);
            }
            printf("\n");
        }
    #endif

    /* Convert to network byte order */
    for (i=0; i<len; i++) {
        buf[i] = htonl(buf[i]);
    }

#ifdef SOCKET_UDP
    if (sendto(s_client, buf, len*4, 0, (struct sockaddr*)&server, slen)==-1)
        printf("ZIGBEE_SOCKET_CLIENT:  write error\n");
#else
    if (write(s_client, (char *)&buf[0], len*4) < 0)
        printf("ZIGBEE_SOCKET_CLIENT:  write error\n");
#endif
}

int Zigbee_Socket_ClientRecv(unsigned int *buf, int *socket, unsigned int words)
{
    int i;
    int bytes;
    (void)socket; /* unused */

#ifdef SOCKET_UDP
    int slen=sizeof(server);
    bytes = recvfrom(s_client, buf, MAX_MSG_SIZE_IN_WORDS*4, 0, (struct sockaddr*)&server, &slen);
#else
    bytes = read(s_client, (char *)buf, words*4);
#endif
    if (bytes < 0) {
        printf("ZIGBEE_SOCKET_CLIENT:  read error\n");
        return -1;
    }

    /* Convert from network byte order */
    for (i=0; i<bytes/4; i++) {
        buf[i] = ntohl(buf[i]);
    }

    #ifdef DEBUG
        printf("ZIGBEE_SOCKET_CLIENT:  receive %d words from server:", bytes/4);
        {
            int i;
            for (i=0; i<bytes/4; i++) {
                if (i%4==0) printf("\n");
                printf("%08x ", buf[i]);
            }
            printf("\n");
        }
    #endif
    return bytes;
}

void Zigbee_Socket_ClientClose(void)
{
    close(s_client);
}

/* eof zigbee_socket_client.c */