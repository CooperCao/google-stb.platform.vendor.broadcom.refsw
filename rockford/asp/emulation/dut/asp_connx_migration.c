/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <ifaddrs.h>
#include <netpacket/packet.h>

#include <net/if.h>

#include "error_macro.h"
#include "asp_connx_migration_api.h"


static void myPrint(char *string)
{
    printf("\n**********************************************************************************\n");
    printf("%s\n", string);
    printf("**********************************************************************************\n");
}

static int setNetFilterRule(
    SocketState *pSocketState
    )
{
    int rc;
#define RULE_BUF_SIZE 512
    char *pRule = NULL;

    pRule = calloc(1, RULE_BUF_SIZE);
    assert(pRule);
    snprintf(pRule, RULE_BUF_SIZE-1, "iptables -A INPUT --protocol tcp --dport %d --tcp-flags ALL ACK -j DROP", htons(pSocketState->localIpAddr.sin_port));
    /* setup a netfilter rule to divert all incoming packets for this session to a queue serviced by ASP Manager */
    /* TODO: refine this rule: first two rules will catch all the fragmented but later reassemled packets */
    /* - add src port to make it it specific to this socket */
    /* - add rule to divert incoming packets to a queue */
    /* - add rule to catch ICMP Path MTU packets */
    rc = system("iptables -F");
    /*rc = system("iptables -A INPUT --protocol tcp --dport 5000 --tcp-flags ALL ACK -j DROP");*/
    rc = system(pRule);
    CHECK_ERR_NZ_GOTO("Failed to setup iptables rule to divert all incoming packets from the network stack to ASP Manager Q ...", rc, error);
    printf("Added the iptables rule to divert all incoming packets from the network stack to ASP Manager Q ...");

    rc = system("iptables -L");
    CHECK_ERR_NZ_GOTO("Failed to list iptables rules ...", rc, error);

    if (pRule) free(pRule);
    return 0;
error:
    if (pRule) free(pRule);
    return -1;
}

static int removeNetFilterRules(void)
{
    int rc;
    /* setup a netfilter rule to divert all incoming packets for this session to a queue serviced by ASP Manager */
    /* TODO: refine this rule: first two rules will catch all the fragmented but later reassemled packets */
    /* - add src port to make it it specific to this socket */
    /* - add rule to divert incoming packets to a queue */
    /* - add rule to catch ICMP Path MTU packets */
    rc = system("iptables -F");
    myPrint("Removed the iptables rule to divert all incoming packets from the network stack to ASP Manager Q ...");

    rc = system("iptables -L");
    CHECK_ERR_NZ_GOTO("Failed to list iptables rules ...", rc, error);

    return 0;
error:
    return -1;
}

static void printLinuxConnxState(void)
{
    int rc;
    printf("Linux connx status:\n");
    rc = system("netstat -an | grep 5000 | grep -v TIME | grep -v ACK");
    CHECK_ERR_NZ_GOTO("netstat Failed ", rc, error);
error:
    return;
}

static int tcpSelectQueue(int socketFd, int queue)
{
    int rc;
    rc = setsockopt(socketFd, IPPROTO_TCP, TCP_REPAIR_QUEUE, (void*)&queue, sizeof(queue));
    CHECK_ERR_NZ_GOTO("Failed to set TCP_REPAIR_QUEUE option, error:", rc, error);
    return 0;
error:
    perror("setsockopt");
    return -EINVAL;
}

static int tcpGetQueueSeq(int socketFd, unsigned *seq)
{
    int rc;
    socklen_t len = sizeof(len);
    rc = getsockopt(socketFd, IPPROTO_TCP, TCP_QUEUE_SEQ, (void*)seq, &len);
    CHECK_ERR_NZ_GOTO("Failed to get TCP_QUEUE_SEQ option, error: ", rc, error);
    return 0;
error:
    perror("getsockopt");
    return -EINVAL;
}

static int tcpSetQueueSeq(int socketFd, unsigned seq)
{
    int rc;
    int len = sizeof(len);
    rc = setsockopt(socketFd, IPPROTO_TCP, TCP_QUEUE_SEQ, (void*)&seq, sizeof(seq));
    CHECK_ERR_NZ_GOTO("Failed to get TCP_QUEUE_SEQ option, error: ", rc, error);
    return 0;
error:
    perror("setsockopt");
    return -EINVAL;
}

static int tcpGetMaxSegSize(int socketFd, unsigned *maxSegSize)
{
    int rc;
    unsigned len = sizeof(len);
    rc = getsockopt(socketFd, IPPROTO_TCP, TCP_MAXSEG, (void*)maxSegSize, &len);
    CHECK_ERR_NZ_GOTO("Failed to get TCP_MAXSEG option, error: ", rc, error);
    return 0;
error:
    perror("getsockopt");
    return -EINVAL;
}

static int tcpGetInfo(int socketFd, struct tcp_info *tcpInfo)
{
    int rc;
    socklen_t len = sizeof(struct tcp_info);
    rc = getsockopt(socketFd, IPPROTO_TCP, TCP_INFO, (void*)tcpInfo, &len);
    CHECK_ERR_NZ_GOTO("Failed to get TCP_INFO option, error: ", rc, error);
    return 0;
error:
    perror("getsockopt");
    return -EINVAL;
}

static int tcpFreezeSocket(int socketFd)
{
    int rc;
    int freeze = 1;
    rc = setsockopt(socketFd, IPPROTO_TCP, TCP_REPAIR, (void*)&freeze, sizeof(freeze) );
    CHECK_ERR_NZ_GOTO("Failed to set TCP_REPAIR option, error: ", rc, error);

    return 0;
error:
    perror("setsockopt");
    return -EINVAL;
}

static int tcpUnfreezeSocket(int socketFd)
{
    int rc;
    int freeze = 0;
    rc = setsockopt(socketFd, IPPROTO_TCP, TCP_REPAIR, (void*)&freeze, sizeof(freeze) );
    CHECK_ERR_NZ_GOTO("Failed to set TCP_REPAIR option, error: ", rc, error);
    return 0;
error:
    perror("setsockopt");
    return -EINVAL;
}

#if 0
/* the following flag should be defined in if_arp.h file.
   TODO: later removed them.*/
#define ATF_INUSE   0x01    /* entry in use */
#define ATF_COM     0x02    /* completed entry (enaddr valid) */
#define ATF_PERM    0x04    /* permanent entry */
#define ATF_PUBL    0x08    /* publish entry (respond for other host) */
#define ATF_USETRAILERS 0x10    /* has requested trailers */

#endif

#define ATF_PROXY   0x20    /* Do PROXY arp */

static int getIpAddressForAnInterface(
    const char *pInterfacename,
    struct sockaddr_in *pFindAddr
    )
{
    int fd = -1;
    struct ifreq ifr;
    int rc ;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* TODO:Right now I only support IPv4 ip address. */
    ifr.ifr_addr.sa_family = AF_INET;

    strncpy( ifr.ifr_name, pInterfacename, IFNAMSIZ-1);

    rc = ioctl( fd, SIOCGIFADDR, &ifr);
    if (rc < 0)
    {
        /* no ip address available for this interface*/
        if(fd != -1)
        {
            close(fd);
        }
        return -1;
    }

    pFindAddr->sin_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;

    fprintf(stdout, "\n Interface name is %s and corresponding ip address is %s\n", pInterfacename, inet_ntoa(pFindAddr->sin_addr));

    close(fd);
    return 0;
}

static int getMacAddressesForLocalHost(
            SocketState *pSocketState,
            char        **ppInterfacename   /* out: this function will alllocate memeory and populate this memeber.*/
            )
{
    struct ifaddrs *ifaddr = NULL;
    struct ifaddrs *ifeach = NULL;
    struct sockaddr_in findAddr;
    int rc;

    struct sockaddr_in *sin = &pSocketState->localIpAddr;

    memset(&findAddr, 0, sizeof( struct sockaddr_in));

    rc = getifaddrs(&ifaddr);
    if(rc == -1)
    {
        fprintf(stdout,"%s: getifaddrs() failed.", __FUNCTION__);
    }
    else
    {
        for (ifeach = ifaddr; ifeach != NULL; ifeach = ifeach->ifa_next)
        {
            if( (ifeach->ifa_addr) && (ifeach->ifa_addr->sa_family == AF_PACKET) )
            {
                struct sockaddr_ll *sockaddr = (struct sockaddr_ll *)ifeach->ifa_addr;

                rc = getIpAddressForAnInterface(
                                           ifeach->ifa_name,
                                           &findAddr
                                           );
                if(rc < 0)
                {
                   fprintf(stdout, "\n No Ip address for this interface so skip this interface=%s..\n", ifeach->ifa_name);

                }
                else if( (sin->sin_addr.s_addr == findAddr.sin_addr.s_addr) /*&& (sin->sin_family == ifeach->ifa_addr->sa_family) */)
                {
                    /* Also save the interface name which will be needed to determine remote mac address.
                       This says local ip got a request on this interface.*/
                    *ppInterfacename  = calloc((strlen(ifeach->ifa_name) + 2), 1);

                    *ppInterfacename = strncpy(*ppInterfacename, ifeach->ifa_name, (strlen(ifeach->ifa_name) + 1));

                    fprintf(stdout, "\n =============== Interface name = |%s|======\n", *ppInterfacename);

                    {
                        pSocketState->localMacAddress[0] = sockaddr->sll_addr[0];
                        pSocketState->localMacAddress[1] = sockaddr->sll_addr[1];
                        pSocketState->localMacAddress[2] = sockaddr->sll_addr[2];
                        pSocketState->localMacAddress[3] = sockaddr->sll_addr[3];
                        pSocketState->localMacAddress[4] = sockaddr->sll_addr[4];
                        pSocketState->localMacAddress[5] = sockaddr->sll_addr[5];
                        pSocketState->localMacAddress[6] = '\0';
                        fprintf(stdout, "\n%s:  MAC address of Local host  is ===============> %02X:%02X:%02X:%02X:%02X:%02X\n", __FUNCTION__,
                               pSocketState->localMacAddress[0], pSocketState->localMacAddress[1],
                               pSocketState->localMacAddress[2], pSocketState->localMacAddress[3],
                               pSocketState->localMacAddress[4], pSocketState->localMacAddress[5]
                               );

                    }
                }
            }
        }

        freeifaddrs(ifaddr);
    }

    return 0;
}

static int getRemoteMacAddress(
    SocketState *pSocketState,
    const char *pInterfacename, /* in */
    unsigned char *pRemoteHostMacAddr   /* out. this a part of asp_managerHandle structure memeber of unsigned char remoteHostMacAddr[8] ,
                                            so no need of any memory allocation. We just need to populate it.*/
    )
{
    int sockFd = -1;
    struct arpreq arpreq;
    unsigned char *eap;
    struct sockaddr_in *soutTemp;
    int rc;
    struct in_addr ipaddr;

    memset(&arpreq, 0, sizeof(arpreq));

    /*strcpy(arpreq.arp_dev,"eth0");*/
    strncpy(arpreq.arp_dev, pInterfacename, (strlen(pInterfacename )+1)); /* without interface name in this case we get the errno 19 , which is #define ENODEV      19  No such device */

    soutTemp = (struct sockaddr_in *) &arpreq.arp_pa;
    soutTemp->sin_family = AF_INET;
    /*ipaddr = inet_addr("192.168.10.141");*/
    /*inet_aton("192.168.10.191", &ipaddr);*/
    ipaddr = pSocketState->remoteIpAddr.sin_addr;
    soutTemp->sin_addr = ipaddr;/*sin->sin_addr.s_addr; arp table doesn't show any entry for the anvl dummy address 192.168.10.191 */

    sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockFd < 0)
    {
       fprintf(stdout, "\n Not able to create socket, errno=%d\n",errno);
       goto error;
    }

    rc = ioctl( sockFd, SIOCGARP, &arpreq);
    if (rc < 0)
    {
       fprintf(stdout, "\n Not able to find remote mac address, ioctl failed, errno=%d\n",errno);
       goto error;
    }

    if (arpreq.arp_flags & ATF_COM)
    {
       eap = (unsigned char *) &arpreq.arp_ha.sa_data[0];

       pRemoteHostMacAddr[0] = pSocketState->remoteMacAddress[0] = eap[0];
       pRemoteHostMacAddr[1] = pSocketState->remoteMacAddress[1] = eap[1];
       pRemoteHostMacAddr[2] = pSocketState->remoteMacAddress[2] = eap[2];
       pRemoteHostMacAddr[3] = pSocketState->remoteMacAddress[3] = eap[3];
       pRemoteHostMacAddr[4] = pSocketState->remoteMacAddress[4] = eap[4];
       pRemoteHostMacAddr[5] = pSocketState->remoteMacAddress[5] = eap[5];
       pRemoteHostMacAddr[6] = pSocketState->remoteMacAddress[6] = '\0';

       fprintf(stdout, "\n%s: MAC address of Remote client is ================>: %02X:%02X:%02X:%02X:%02X:%02X\n", __FUNCTION__,
               pSocketState->remoteMacAddress[0], pSocketState->remoteMacAddress[1],
               pSocketState->remoteMacAddress[2], pSocketState->remoteMacAddress[3],
               pSocketState->remoteMacAddress[4], pSocketState->remoteMacAddress[5]
               );

       if (arpreq.arp_flags & ATF_PERM) fprintf(stdout," PERM\n");
       if (arpreq.arp_flags & ATF_PUBL) fprintf(stdout, "PUBLISHED\n");
       if (arpreq.arp_flags & ATF_USETRAILERS) fprintf(stdout," TRAILERS\n");
       if (arpreq.arp_flags & ATF_PROXY) fprintf(stdout," PROXY\n");
       printf("\n");
    }

    close(sockFd);

    return 0;
error:
    if(sockFd != -1)
    {
        close(sockFd);
    }
    return -1;
}


int ASP_ConnxMigration_GetTcpStateFromLinux(
    int             socketFd,           /* in: fd of socket to be offloaded from host to ASP. */
    SocketState     *pSocketState,      /* out: associated socket state. */
    char            **ppInterfacename,    /* This is requuired to be preserved since it will be rewuired at the time of sendRawFrameToNw.*/
    unsigned char   *pRemoteHostMacAddr /* This is requuired to be preserved since it will be rewuired at the time of sendRawFrameToNw.*/
    )
{
    int i;
    int rc;
    char string[256];
    int addrLen = sizeof(pSocketState->localIpAddr);

    /* First get the remoteIpAddr , port and local ip_addr and port details.*/
    rc = getsockname(socketFd, (struct sockaddr *)&pSocketState->localIpAddr, (socklen_t *)&addrLen);
    CHECK_ERR_NZ_GOTO("failed to get local address info..", rc, error);
    printf("%s: LocalIpAddr:Port %s:%d \n", __FUNCTION__,inet_ntoa(pSocketState->localIpAddr.sin_addr), ntohs(pSocketState->localIpAddr.sin_port));

    addrLen = sizeof(pSocketState->remoteIpAddr);
    rc = getpeername(socketFd, (struct sockaddr *)&pSocketState->remoteIpAddr, (socklen_t *)&addrLen);
    CHECK_ERR_NZ_GOTO("failed to get peer address info..", rc, error);
    printf("%s: RemoteIpAddr:Port %s:%d \n", __FUNCTION__, inet_ntoa(pSocketState->remoteIpAddr.sin_addr), ntohs(pSocketState->remoteIpAddr.sin_port));

    setNetFilterRule(pSocketState);

    rc = getMacAddressesForLocalHost( pSocketState, ppInterfacename);

    rc = getRemoteMacAddress(pSocketState, *ppInterfacename, pRemoteHostMacAddr );

    rc = tcpFreezeSocket(socketFd);
    CHECK_ERR_NZ_GOTO("tcpFreezeSocket Failed...", rc, error);

    rc = tcpSelectQueue(socketFd, TCP_RECV_QUEUE);
    CHECK_ERR_NZ_GOTO("tcpSelectQueu Failed ...", rc, error);

    rc = tcpGetQueueSeq(socketFd, &pSocketState->tcpState.ack);
    CHECK_ERR_NZ_GOTO("tcpGetQueueSeq Failed ...", rc, error);

    rc = tcpSelectQueue(socketFd, TCP_SEND_QUEUE);
    CHECK_ERR_NZ_GOTO("tcpSelectQueue Failed ...", rc, error);

    rc = tcpGetQueueSeq(socketFd, &pSocketState->tcpState.seq);
    CHECK_ERR_NZ_GOTO("tcpGetQueueSeq Failed ...", rc, error);

    rc = tcpGetInfo(socketFd, &pSocketState->tcpState.tcpInfo);
    CHECK_ERR_NZ_GOTO("tcpGetInfo Failed ...", rc, error);

    rc = tcpGetMaxSegSize(socketFd, &pSocketState->tcpState.maxSegSize);
    CHECK_ERR_NZ_GOTO("tcpGetMaxSegSize Failed ...", rc, error);


    i = snprintf(string, sizeof(string)-1, "TCP State before Offload: seq %u, ack %u, mss %d",
            pSocketState->tcpState.seq, pSocketState->tcpState.ack, pSocketState->tcpState.maxSegSize);
    string[i] = '\0';
    myPrint(string);

    return 0;
error:
    return -1;
}

/* creates a new TCP socket and updates its state to the one obtained from ASP */
int ASP_ConnxMigration_SetTcpStateToLinux(
    SocketState *pSocketState,      /* in:  socket state to use to create the new socket. */
    int fdMigratedConnx,            /* in: fd associated w/ the migrated connection. */
    int *pOutSocketFd               /* out: fd of newly created socket. */
    )
{
    int i;
    int rc;
    int socketFd;
    int reuse_flag = 1;
    char string[256];

    shutdown(fdMigratedConnx, SHUT_RDWR);
    close(fdMigratedConnx);

    fprintf(stdout, "\n%s: Inside close sock fd value ====================%d\n", __FUNCTION__, fdMigratedConnx);

    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_ERR_LZ_GOTO("Failed to create new socket: ", strerror(errno), socketFd, error);
    *pOutSocketFd = socketFd;

    fprintf(stdout, "\n%s: New open sock Fd value ================+++++++++++++====%d\n", __FUNCTION__, socketFd);

    /* enable reuse address option */
    rc = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_flag, sizeof(reuse_flag));
    CHECK_ERR_LZ_GOTO("Failed to set socket option: ", strerror(errno), rc, error);

    rc = tcpFreezeSocket(socketFd);
    CHECK_ERR_NZ_GOTO("tcpFreezeSocket Failed...", rc, error);

    /* update the send seq # */
    rc = tcpSelectQueue(socketFd, TCP_SEND_QUEUE);
    CHECK_ERR_NZ_GOTO("tcpSelectQueue Failed ...", rc, error);
    rc = tcpSetQueueSeq(socketFd, pSocketState->tcpState.seq);
    CHECK_ERR_NZ_GOTO("tcpGetQueueSeq Failed ...", rc, error);

    /* update the recv ack # */
    rc = tcpSelectQueue(socketFd, TCP_RECV_QUEUE);
    CHECK_ERR_NZ_GOTO("tcpSelectQueu Failed ...", rc, error);
    rc = tcpSetQueueSeq(socketFd, pSocketState->tcpState.ack);
    CHECK_ERR_NZ_GOTO("tcpGetQueueSeq Failed ...", rc, error);

    /* bind to this socket: Note: we reuse the socket addr & port info from the saved socket state */
    printf("localIpAddr:port %s:%d\n", inet_ntoa(pSocketState->localIpAddr.sin_addr), ntohs(pSocketState->localIpAddr.sin_port));

    rc = bind(socketFd, (struct sockaddr *) &pSocketState->localIpAddr, sizeof(pSocketState->localIpAddr));
    CHECK_ERR_LZ_GOTO("Failed to bind to socket: ", strerror(errno), rc, error);

    printf("remoteIpAddr:port %s:%d\n", inet_ntoa(pSocketState->remoteIpAddr.sin_addr), ntohs(pSocketState->remoteIpAddr.sin_port));
    rc = connect(socketFd, (struct sockaddr *) &pSocketState->remoteIpAddr, sizeof(pSocketState->remoteIpAddr));
    CHECK_ERR_LZ_GOTO("Connect Error: ", strerror(errno), rc, error);

    rc = tcpUnfreezeSocket(socketFd);
    CHECK_ERR_NZ_GOTO("tcpUnfreezeSocket Failed...", rc, error);

    myPrint("Linux socket status: server side TCP connection should now be back in the ESTABLISHED state !!!!!");
    printLinuxConnxState();
    i = snprintf(string, sizeof(string)-1, "TCP State after migration back to Linux: seq %u, ack %u, mss %d",
            pSocketState->tcpState.seq, pSocketState->tcpState.ack, pSocketState->tcpState.maxSegSize);
    string[i] = '\0';
    myPrint(string);

    /* new socket is ready, so stop filtering packets. */
    removeNetFilterRules();

    return 0;
error:
    printLinuxConnxState();
    return -1;
}
