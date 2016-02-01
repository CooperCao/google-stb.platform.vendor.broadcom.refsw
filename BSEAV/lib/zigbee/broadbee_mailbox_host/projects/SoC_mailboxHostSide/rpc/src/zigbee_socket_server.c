/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <assert.h>
#include "zigbee_socket.h"
#include "zigbee_rpc_frame.h"

static int listener;
static int s_server;
static fd_set read_fds;
static fd_set master;
static int fdmax; /* maximum file descriptor number */
static int current_client_fd; /* the old msgsock */

static void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

#ifdef SOCKET_UDP
#define PORT 9930
int Zigbee_Socket_ServerOpen(void)
{
    struct sockaddr_in si_me;
    int yes=1;        // for setsockopt() SO_REUSEADDR, below

    /* Create socket */
    if ((s_server = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("opening socket");
        exit(1);
    }

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    #ifdef PORT
        si_me.sin_port = htons(PORT);   /* If port is set to zero, it will be selected automatically? */
    #else
        si_me.sin_port = 0;   /* If port is set to zero, it will be selected automatically? */
    #endif

    // lose the pesky "address already in use" error message
    setsockopt(s_server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(s_server, (struct sockaddr*)&si_me, sizeof(si_me))==-1) {
        perror("binding stream socket");
        exit(1);
    }

    #ifndef PORT
        /* Find out assigned port number and print it out */
        length = sizeof(si_me);
        if (getsockname(s_server, (struct sockaddr *)&si_me, &length)) {
            perror("getting socket name");
            exit(1);
        }
        printf("Socket has port #%d\n", ntohs(si_me.sin_port));
    #endif

//    return sock;
}
#else
void Zigbee_Socket_ServerOpen(void)
{
    struct sockaddr_in si_me;
    int length;
    struct sockaddr_in si_client;
    int slen=sizeof(si_client);
    int yes=1;        /* for setsockopt() SO_REUSEADDR, below */

    /* Create socket */
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("opening socket");
        exit(1);
    }

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    #ifdef PORT
        si_me.sin_port = htons(PORT);   /* If port is set to zero, it will be selected automatically? */
    #else
        si_me.sin_port = 0;   /* If port is set to zero, it will be selected automatically? */
    #endif

    /* lose the pesky "address already in use" error message */
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listener, (struct sockaddr*)&si_me, sizeof(si_me))==-1) {
        perror("binding stream socket");
        exit(1);
    }

    #ifndef PORT
        /* Find out assigned port number and print it out */
        length = sizeof(si_me);
        if (getsockname(listener, (struct sockaddr *)&si_me, &length)) {
            perror("getting socket name");
            exit(1);
        }
        printf("ZIGBEE_SOCKET_SERVER:  Socket has port #%d\n", ntohs(si_me.sin_port));
    #endif

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    /* listen */
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    /* add the listener to the master set */
    FD_SET(listener, &master);

    /* keep track of the biggest file descriptor */
    fdmax = listener; /* so far, it's this one */
}
#endif

void Zigbee_Socket_ServerSend(unsigned int *buf, int len, int socket)
{
    int i;

    if (len != buf[RPC_FRAME_LENGTH_OFFSET]) {
        printf("len=%d, buf[RPC_FRAME_LENGTH_OFFSET]=%d\n", len, buf[RPC_FRAME_LENGTH_OFFSET]);
        assert(0);
    }

    #ifdef DEBUG
        printf("ZIGBEE_SOCKET_SERVER:  send %d words to client:", len);
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
    if (sendto(s_server, buf, len*4, 0, (struct sockaddr*)&si_client, slen)==-1)
        printf("sendto error\n");
#else
    if (write(socket, buf, len*4) < 0)
        printf("ZIGBEE_SOCKET_SERVER:  sendto error\n");
#endif
}

#ifdef SOCKET_UDP
int Zigbee_Socket_ServerRecv(unsigned int *buf, struct sockaddr* si_client)
{
    int bytes;
    bytes = recvfrom(s_server, buf, MAX_MSG_SIZE_IN_WORDS*4, 0, (struct sockaddr*)si_client, &slen);
    if (bytes < 0) {
        printf("ZIGBEE_SOCKET_SERVER:  read error\n");
    }

    /* Convert from network byte order */
    for (i=0; i<bytes/4; i++) {
        buf[i] = ntohl(buf[i]);
    }

    #ifdef DEBUG
        printf("ZIGBEE_SOCKET_SERVER:  received buffer from client:\n");
        {
            int i;
            for (i=0; i<bytes/4; i++) {
                if (i%4==0) printf("\n");
                printf("%08x ", p[i]);
            }
            printf("\n");
        }
    #endif

    return bytes;
}
#else
int Zigbee_Socket_ServerRecv(unsigned int *buf, int *socket, int words)
{
    int data_received = 0;
    socklen_t addrlen;
    struct sockaddr_storage remoteaddr; /* client address */
    int newfd;        /* newly accept()ed socket descriptor */
    char remoteIP[INET6_ADDRSTRLEN];
    int nbytes;
    int i;

    for (;;) {
        if (data_received) break;
        read_fds = master;

        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        for (i=0; i<=fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i==listener) {
                    /* handle new connection */
                    addrlen = sizeof(remoteaddr);
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        current_client_fd = newfd;
                        FD_SET(newfd, &master);
                        if (newfd > fdmax) {
                            fdmax = newfd;
                        }
                        printf("ZIGBEE_SOCKET_SERVER:  selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                    }
                } else {
                    /* handle data from a client. */
//                    memset(buf, 0, BUFLEN/*sizeof(buf)*/);
                    *socket = i;
                    if ((nbytes  = read(i, buf,  words*4)) <= 0) {
                        close(i);
                        FD_CLR(i, &master); /* remove from master set */
                        if (nbytes == 0) {
                            /* connection closed */
                            /* *socket = i; */
                            printf("ZIGBEE_SOCKET_SERVER:  selectserver: socket %d hung up\n", i);
                            return -1;
                        } else {
                            perror("reading stream message");
                        }
                    } else {
                        /*printf("ZIGBEE_SOCKET_SERVER:  received packet from socket=%d, bytes=%d\n", i, nbytes);*/
                        /* *socket = i; */ /* TBD - what if we receive data from more than one socket at a time ? */
                        data_received = 1;
                    }
                }
            }
        }
    }

    /* Convert from network byte order */
    for (i=0; i<words; i++) {
        buf[i] = ntohl(buf[i]);
    }

    #ifdef DEBUG
        printf("ZIGBEE_SOCKET_SERVER:  received %d words from client:", nbytes/4);
        {
            int i;
            for (i=0; i<words; i++) {
                if (i%4==0) printf("\n");
                printf("%08x ", buf[i]);
            }
            printf("\n");
        }
    #endif

    return nbytes;
}
#endif

#ifdef SOCKET_UDP
void Zigbee_Socket_ServerClose(void)
{
    close(s_server);
    return;
}
#else
void Zigbee_Socket_ServerClose(void)
{
    close(listener);
    return;
}
#endif