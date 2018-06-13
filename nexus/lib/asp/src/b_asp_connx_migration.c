/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
#include <linux/tcp.h>

#include <ifaddrs.h>
#include <netpacket/packet.h>

#include <net/if.h>
#include <linux/rtnetlink.h>

#include <stdint.h>
#include <stddef.h>
#include <linux/sockios.h>
#include <endian.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/ethtool.h>

#if 0
ETHTOOL_SRXCLSRLINS
Receive (RX) Classification (CLS) Rule (RL) Inserting (INS)
#endif

#include "bstd.h"
#include "bkni.h"
#include "b_asp_priv.h"
#include "b_asp_connection_migration.h"
#include "asp_netfilter_drv.h"

#define TEMPORARILY_DISABLE_REVERSE_MIGRATION

BDBG_MODULE(b_asp_connx);

static int tcpSelectQueue(
    int socketFd,
    int queue
    )
{
    int rc;
    int savedErrno;

    rc = setsockopt(socketFd, IPPROTO_TCP, TCP_REPAIR_QUEUE, (void*)&queue, sizeof(queue));
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: Failed to set TCP_REPAIR_QUEUE option, error:", socketFd), error, rc, rc);
    return 0;

error:
    perror("setsockopt");
    errno = savedErrno;
    return -EINVAL;
}

static int tcpGetQueueSeq(int socketFd, unsigned *seq)
{
    int rc;
    int savedErrno;

    socklen_t len = sizeof(len);
    rc = getsockopt(socketFd, IPPROTO_TCP, TCP_QUEUE_SEQ, (void*)seq, &len);
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: Failed to get TCP_QUEUE_SEQ option, error: ", socketFd), error, rc, rc);
    return 0;

error:
    perror("getsockopt");
    errno = savedErrno;
    return -EINVAL;
}

#ifndef TEMPORARILY_DISABLE_REVERSE_MIGRATION
static int tcpSetQueueSeq(int socketFd, unsigned seq)
{
    int rc;
    int len = sizeof(len);
    int savedErrno;

    rc = setsockopt(socketFd, IPPROTO_TCP, TCP_QUEUE_SEQ, (void*)&seq, sizeof(seq));
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: Failed to get TCP_QUEUE_SEQ option, error: ", socketFd), error, rc, rc);
    return 0;

error:
    perror("setsockopt");
    errno = savedErrno;
    return -EINVAL;
}
#endif /* TEMPORARILY_DISABLE_REVERSE_MIGRATION */

static int tcpGetMaxSegSize(int socketFd, unsigned *maxSegSize)
{
    int rc;
    unsigned len = sizeof(len);
    int savedErrno;

    rc = getsockopt(socketFd, IPPROTO_TCP, TCP_MAXSEG, (void*)maxSegSize, &len);
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: Failed to get TCP_MAXSEG option, error: ", socketFd), error, rc, rc);
    return 0;

error:
    perror("getsockopt");
    errno = savedErrno;
    return -EINVAL;
}

static int tcpGetInfo(int socketFd, struct tcp_info *tcpInfo)
{
    int rc;
    socklen_t len = sizeof(struct tcp_info);
    int savedErrno;

    rc = getsockopt(socketFd, IPPROTO_TCP, TCP_INFO, (void*)tcpInfo, &len);
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: Failed to get TCP_INFO option, error: ", socketFd), error, rc, rc);
    BDBG_MSG((B_ASP_MSG_PRE_FMT "TCP State: --> state=%d ca_state=%d" B_ASP_MSG_PRE_ARG,
                tcpInfo->tcpi_state,
                tcpInfo->tcpi_ca_state
                ));
    BDBG_MSG(("options: timestamps=%s sack=%s wscale=%s ecn=%s",
                tcpInfo->tcpi_options & TCPI_OPT_TIMESTAMPS ? "Y":"N",
                tcpInfo->tcpi_options & TCPI_OPT_SACK ? "Y":"N",
                tcpInfo->tcpi_options & TCPI_OPT_WSCALE ? "Y":"N",
                tcpInfo->tcpi_options & TCPI_OPT_ECN ? "Y":"N"
             ));
    BDBG_MSG(("snd: mss=%u cwnd=%u wscale=%u ssthresh=%u",
                tcpInfo->tcpi_snd_mss,
                tcpInfo->tcpi_snd_cwnd,
                tcpInfo->tcpi_snd_wscale,
                tcpInfo->tcpi_snd_ssthresh
             ));
    BDBG_MSG(("rcv: space=%u wscale=%u ssthresh=%u rtt=%u mss=%u",
                tcpInfo->tcpi_rcv_space,
                tcpInfo->tcpi_rcv_wscale,
                tcpInfo->tcpi_rcv_ssthresh,
                tcpInfo->tcpi_rcv_rtt,
                tcpInfo->tcpi_rcv_mss
             ));
    return 0;

error:
    perror("getsockopt");
    errno = savedErrno;
    return -EINVAL;
}

/* TODO: need to see why compiler doesn't find this definition in our default tcp.h file. */
#define TCP_TIMESTAMP           24
static int tcpGetTimestamp(int socketFd, unsigned *pSenderTimestamp)
{
    int rc;
    socklen_t len = sizeof(len);
    int savedErrno;

    rc = getsockopt(socketFd, IPPROTO_TCP, TCP_TIMESTAMP, (void*)pSenderTimestamp, &len);
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: Failed to get TCP_TIMESTAMP option, error: ", socketFd), error, rc, rc);
    BDBG_MSG(("senderTimestamp=%u", *pSenderTimestamp));
    return 0;

error:
    perror("getsockopt");
    errno = savedErrno;
    return -EINVAL;
}

static int tcpFreezeSocket(int socketFd)
{
    int rc;
    int freeze = 1;
    int savedErrno;

    rc = setsockopt(socketFd, IPPROTO_TCP, TCP_REPAIR, (void*)&freeze, sizeof(freeze) );
    savedErrno = errno;
    if (rc != 0 && errno == EPERM)      /* EPERM => Socket probably closing. */
    {
        BDBG_WRN(("%s : %d setsockopt(TCP_REPAIR), errno=EPERM, Assume socket closing" B_ASP_MSG_PRE_ARG ));
        return EPERM;          /* Socket not in closed or established state. */
    }
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: Failed to set TCP_REPAIR option, error: ", socketFd), error, rc, rc);

    return 0;
error:
    perror("setsockopt");
    errno = savedErrno;
    return -EINVAL;
}

#ifndef TEMPORARILY_DISABLE_REVERSE_MIGRATION
static int tcpUnfreezeSocket(int socketFd)
{
    int rc;
    int freeze = 0;
    int savedErrno;

    rc = setsockopt(socketFd, IPPROTO_TCP, TCP_REPAIR, (void*)&freeze, sizeof(freeze) );
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: Failed to set TCP_REPAIR option, error: ", socketFd), error, rc, rc);
    return 0;

error:
    perror("setsockopt");
    errno = savedErrno;
    return -EINVAL;
}
#endif /* TEMPORARILY_DISABLE_REVERSE_MIGRATION */


static int getIpAddressForAnInterface(
    const char          *pInterfacename,
    struct sockaddr_in  *pFindAddr
    )
{
    int fd = -1;
    struct ifreq ifr;
    int rc ;
    int savedErrno;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    savedErrno = errno;
    if (fd == -1)
    {
        errno = savedErrno;
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "ioctl() socket() failed, errno=%d" B_ASP_MSG_PRE_ARG, savedErrno));
        return -1;
    }

    /* TODO: IPv6 support. */
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy( ifr.ifr_name, pInterfacename, IFNAMSIZ-1);
    rc = ioctl( fd, SIOCGIFADDR, &ifr);
    savedErrno = errno;
    if (rc < 0)
    {
        /* no ip address available for this interface */
        close(fd);
        errno = savedErrno;
        return -1;
    }

    pFindAddr->sin_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
    BDBG_MSG((B_ASP_MSG_PRE_FMT "interfaceName=%s ipAddress=%s" B_ASP_MSG_PRE_ARG, pInterfacename, inet_ntoa(pFindAddr->sin_addr)));
    close(fd);
    errno = savedErrno;
    return rc;
}

static int getInterfaceNameAndLocalMacAddress(
    B_AspSocketState *pSocketState
    )
{
    struct ifaddrs *ifaddr = NULL;
    struct ifaddrs *ifeach = NULL;
    struct sockaddr_in findAddr;
    int rc;
    struct sockaddr_in *localIpAddr = &pSocketState->localIpAddr;
    struct sockaddr_in *remoteIpAddr = &pSocketState->remoteIpAddr;
    int savedErrno;


    BKNI_Memset(&findAddr, 0, sizeof( struct sockaddr_in));

    rc = getifaddrs(&ifaddr);
    savedErrno = errno;
    if (rc == -1)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "Error: getifaddrs() failed, errno=%d" B_ASP_MSG_PRE_ARG, errno));
        errno = savedErrno;
        return (rc);
    }
    else
    {
        for (ifeach = ifaddr; ifeach != NULL; ifeach = ifeach->ifa_next)
        {
            if (ifeach->ifa_addr && ifeach->ifa_addr->sa_family == AF_PACKET )
            {
                struct sockaddr_ll *sockaddr = (struct sockaddr_ll *)ifeach->ifa_addr;

                rc = getIpAddressForAnInterface(ifeach->ifa_name, &findAddr);
                if (rc < 0)
                {
                   BDBG_MSG(( B_ASP_MSG_PRE_FMT "No IP Address define this interface, so skipping interface=%s, interfaceIndex=%d.." B_ASP_MSG_PRE_ARG, ifeach->ifa_name, sockaddr->sll_ifindex));
                   continue;
                }
                if ( localIpAddr->sin_addr.s_addr == findAddr.sin_addr.s_addr /*&& TODO (localIpAddr->sin_family == ifeach->ifa_addr->sa_family) */)
                {
                    /* Save the interface name. */
                    strncpy(pSocketState->interfaceName, ifeach->ifa_name, IFNAMSIZ);
                    pSocketState->interfaceIndex = sockaddr->sll_ifindex;

                    /* Save the MAC address associated with this interface name. */
                    {
                        pSocketState->localMacAddress[0] = sockaddr->sll_addr[0];
                        pSocketState->localMacAddress[1] = sockaddr->sll_addr[1];
                        pSocketState->localMacAddress[2] = sockaddr->sll_addr[2];
                        pSocketState->localMacAddress[3] = sockaddr->sll_addr[3];
                        pSocketState->localMacAddress[4] = sockaddr->sll_addr[4];
                        pSocketState->localMacAddress[5] = sockaddr->sll_addr[5];
                        BDBG_MSG((B_ASP_MSG_PRE_FMT "Local Host: interfacename=%s interfaceIndex=%d MAC address=%02X:%02X:%02X:%02X:%02X:%02X\n" B_ASP_MSG_PRE_ARG,
                               pSocketState->interfaceName, pSocketState->interfaceIndex,
                               pSocketState->localMacAddress[0], pSocketState->localMacAddress[1],
                               pSocketState->localMacAddress[2], pSocketState->localMacAddress[3],
                               pSocketState->localMacAddress[4], pSocketState->localMacAddress[5]
                               ));
                    }
                }
                if ( remoteIpAddr->sin_addr.s_addr == findAddr.sin_addr.s_addr /*&& TODO (remoteIpAddr->sin_family == ifeach->ifa_addr->sa_family) */)
                {
                    /* Remote IP happens to match one of the local interface's IP, so remote must be on the same node too. */
                    strncpy(pSocketState->remoteInterfaceName, ifeach->ifa_name, IFNAMSIZ);
                    pSocketState->remoteInterfaceIndex = sockaddr->sll_ifindex;
                    pSocketState->remoteOnLocalHost = true;
                    BDBG_MSG((B_ASP_MSG_PRE_FMT "Remote IP=%s is on same host on interface=%s interfaceIndex=%d" B_ASP_MSG_PRE_ARG,
                                inet_ntoa(remoteIpAddr->sin_addr), pSocketState->remoteInterfaceName, pSocketState->remoteInterfaceIndex ));
                }
            }
        }
        freeifaddrs(ifaddr);
    }

    return 0;
}

static int getRemoteMacAddress(
    B_AspSocketState *pSocketState,
    int                      aspNetFilterDrvFd
    )
{
    int sockFd = -1;
    struct arpreq arpreq;
    unsigned char *eap;
    struct sockaddr_in *soutTemp;
    struct sockaddr_in nextIpAddr;   /* Either remote IP addr or gateway's IP addr. */
    int rc;
    ASP_DeviceGetGateway    getGateway;
    int savedErrno;

    nextIpAddr = pSocketState->remoteIpAddr;    /* Assume no gateway. */

    memset(&getGateway, 0, sizeof getGateway);
    getGateway.remoteIpAddr[0] = (uint32_t)pSocketState->remoteIpAddr.sin_addr.s_addr;
    rc = ioctl(aspNetFilterDrvFd, ASP_DEVICE_IOC_GET_GATEWAY, (unsigned long)(&getGateway));
    if (rc == 0)
    {
        if (getGateway.routeUsesGateway)
        {
            nextIpAddr.sin_addr.s_addr = getGateway.gatewayIpAddr[0];   /* Set gateway's IP address. */
        }
    }
    else
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "ASP_DEVICE_IOC_GET_GATEWAY failed, errno=%d" B_ASP_MSG_PRE_ARG, errno));
        /* Print error message and try to continue... */
    }

    if (pSocketState->localIpAddr.sin_addr.s_addr == nextIpAddr.sin_addr.s_addr)
    {
        pSocketState->remoteMacAddress[0] = pSocketState->localMacAddress[0];
        pSocketState->remoteMacAddress[1] = pSocketState->localMacAddress[1];
        pSocketState->remoteMacAddress[2] = pSocketState->localMacAddress[2];
        pSocketState->remoteMacAddress[3] = pSocketState->localMacAddress[3];
        pSocketState->remoteMacAddress[4] = pSocketState->localMacAddress[4];
        pSocketState->remoteMacAddress[5] = pSocketState->localMacAddress[5];
    }
    else
    {
        BKNI_Memset(&arpreq, 0, sizeof(arpreq));

        strncpy(arpreq.arp_dev, pSocketState->interfaceName, IFNAMSIZ-1);

        soutTemp = (struct sockaddr_in *) &arpreq.arp_pa;
        soutTemp->sin_family = nextIpAddr.sin_family;
        soutTemp->sin_addr = nextIpAddr.sin_addr;
        sockFd = socket(AF_INET, SOCK_DGRAM, 0);
        savedErrno = errno;
        if (sockFd == -1)
        {
            int myerrno = errno;
            BDBG_ERR(( B_ASP_MSG_PRE_FMT "ioctl() socket() failed, errno=%d" B_ASP_MSG_PRE_ARG, myerrno));
            goto error;
        }

        rc = ioctl(sockFd, SIOCGARP, &arpreq);
        savedErrno = errno;
        if (rc < 0)
        {
            BDBG_ERR(( B_ASP_MSG_PRE_FMT "Not able to find remote mac address, ioctl failed, errno=%d" B_ASP_MSG_PRE_ARG, errno));
            goto error;
        }

        if (arpreq.arp_flags & ATF_COM) /* Complete entry */
        {
            eap = (unsigned char *) &arpreq.arp_ha.sa_data[0];

            pSocketState->remoteMacAddress[0] = eap[0];
            pSocketState->remoteMacAddress[1] = eap[1];
            pSocketState->remoteMacAddress[2] = eap[2];
            pSocketState->remoteMacAddress[3] = eap[3];
            pSocketState->remoteMacAddress[4] = eap[4];
            pSocketState->remoteMacAddress[5] = eap[5];
        }
        else
        {
            BDBG_ERR(( B_ASP_MSG_PRE_FMT "didn't find a complete ARP entry for nextIpAddr=%s" B_ASP_MSG_PRE_ARG, inet_ntoa(nextIpAddr.sin_addr)));
            goto error;
        }

        close(sockFd);
    }

    BDBG_MSG((B_ASP_MSG_PRE_FMT "Remote client's MAC address=%02X:%02X:%02X:%02X:%02X:%02X\n" B_ASP_MSG_PRE_ARG,
                pSocketState->remoteMacAddress[0], pSocketState->remoteMacAddress[1],
                pSocketState->remoteMacAddress[2], pSocketState->remoteMacAddress[3],
                pSocketState->remoteMacAddress[4], pSocketState->remoteMacAddress[5]
             ));

    return 0;

error:
    if(sockFd != -1)
    {
        close(sockFd);
    }
    errno = savedErrno;
    return -1;
}

int B_Asp_GetInterfaceNameAndMacAddress(
    B_AspSocketState     *pSocketState,       /* Interface name & MAC address' is returned in the pSocketState */
    int                          aspNetFilterDrvFd
    )
{
    int rc;

    rc = getInterfaceNameAndLocalMacAddress(pSocketState);
    if (rc == 0)
    {
        rc = getRemoteMacAddress(pSocketState, aspNetFilterDrvFd);
    }
    return rc;
}

int B_Asp_GetSocketStateFromLinux(
    int                         socketFd,           /* in: fd of socket to be offloaded from host to ASP. */
    B_AspSocketState            *pSocketState       /* out: associated socket state. */
    )
{
    int rc;
    int savedErrno;

    rc = tcpFreezeSocket(socketFd);
    savedErrno = errno;
    if (rc == EPERM)     /* EPERM => Socket probably closing. */
    {
        pSocketState->connectionLost = true;
        return 0; /* return success... losing connection is normal if client changes channel. */
    }
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpFreezeSocket Failed...", socketFd), error, rc, rc);

    rc = tcpSelectQueue(socketFd, TCP_RECV_QUEUE);
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpSelectQueu Failed ...", socketFd), error, rc, rc);

    rc = tcpGetQueueSeq(socketFd, &pSocketState->tcpState.ack);
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpGetQueueSeq Failed ...", socketFd), error, rc, rc);

    rc = tcpGetTimestamp(socketFd, &pSocketState->tcpState.senderTimestamp);
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpGetTimestamp Failed ...", socketFd), error, rc, rc);

    rc = tcpSelectQueue(socketFd, TCP_SEND_QUEUE);
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpSelectQueue Failed ...", socketFd), error, rc, rc);

    rc = tcpGetQueueSeq(socketFd, &pSocketState->tcpState.seq);
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpGetQueueSeq Failed ...", socketFd), error, rc, rc);

    rc = tcpGetInfo(socketFd, &pSocketState->tcpState.tcpInfo);
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpGetInfo Failed ...", socketFd), error, rc, rc);

    rc = tcpGetMaxSegSize(socketFd, &pSocketState->tcpState.maxSegSize);
    savedErrno = errno;
    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpGetMaxSegSize Failed ...", socketFd), error, rc, rc);


    BDBG_MSG((B_ASP_MSG_PRE_FMT "TCP State before Offload: seq %u, ack %u, mss %d" B_ASP_MSG_PRE_ARG,
            pSocketState->tcpState.seq, pSocketState->tcpState.ack, pSocketState->tcpState.maxSegSize));
#if ENDIANNESS_DEBUG
    {
        uint8_t *pByte = (uint8_t *)&pSocketState->tcpState.seq;
        BDBG_WRN(("Seq# in hex=0x%x , Linux returned in BE, shown in LE on ARM), byte[0]=0x%x byte[1]=0x%x byte[2]=0x%x byte[3]=0x%x", *(uint32_t *)pByte, pByte[0], pByte[1], pByte[2], pByte[3]));
    }
#endif

    return 0;
error:
    errno = savedErrno;
    return -1;
}

/* creates a new TCP socket and updates its state to the one obtained from ASP */
int B_Asp_SetSocketStateToLinux(
    B_AspSocketState *pSocketState,             /* in:  socket state to use to create the new socket. */
    int fdMigratedConnx,                        /* in: fd associated w/ the migrated connection. */
    int *pOutSocketFd                           /* out: fd of newly created socket. */
    )
{

#ifdef TEMPORARILY_DISABLE_REVERSE_MIGRATION
    BSTD_UNUSED(pSocketState);
    BSTD_UNUSED(fdMigratedConnx);
    BSTD_UNUSED(pOutSocketFd);
    BDBG_MSG((B_ASP_MSG_PRE_FMT "Not YET migrating the connection state back to Linux!!!" B_ASP_MSG_PRE_ARG));
    return 0;

#else /*  TEMPORARILY_DISABLE_REVERSE_MIGRATION */

    int rc;
    int socketFd;
    int reuse_flag = 1;

    /* 2/1/2017: TODO: this code needs more thought... */
    /* Currently, Linux kernel seems to require following on connection migration:
     * -doesn't allow us to re-use the existing socket (fdMigratedConnx) for migrating the socket from ASP.
     *  It expects the socket to be the TCP_CLOSE state and the migrated socket is not in that state.
     * -also, kernel doesn't allow the new socket to be connected to w/o closing the current one.
     *
     * Given this, we are not going to YET migrate the connection to Linux and defer this work to
     * after ASP bringup complete.
     */

    /* Close the  previously migrated socket. It was deferred until now */
    /* so that its local port # can be reserved & not reused by other connections. */
    {
        shutdown(fdMigratedConnx, SHUT_RDWR);
        close(fdMigratedConnx);
        BDBG_WRN(("%s: Closed migratedSocketFd=%d" B_ASP_MSG_PRE_ARG, fdMigratedConnx));
    }


    /* Create a socket to which we can migrate the connection state back. */
    {
        socketFd = socket(AF_INET, SOCK_STREAM, 0);
        B_ASP_CHECK_GOTO( rc == 0, "Failed to create new socket: ", strerror(errno), socketFd, error);
        BDBG_WRN(("%s: OutSocketFd=%d" B_ASP_MSG_PRE_ARG, socketFd));
    }

    /* Enable reuse address option */
    {
        rc = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_flag, sizeof(reuse_flag));
        B_ASP_CHECK_GOTO( rc == 0, "Failed to set socket option: ", strerror(errno), socketFd), error, rc, rc);
    }

    /* Now migrate the current state back to this socket. */
    {
        rc = tcpFreezeSocket(socketFd);
        B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpFreezeSocket Failed...", socketFd), error, rc, rc);

        /* update the send seq # */
        rc = tcpSelectQueue(socketFd, TCP_SEND_QUEUE);
        B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpSelectQueue Failed ...", socketFd), error, rc, rc);
        rc = tcpSetQueueSeq(socketFd, pSocketState->tcpState.seq);
        B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpSetQueueSeq Failed ...", socketFd), error, rc, rc);

        /* update the recv ack # */
        rc = tcpSelectQueue(socketFd, TCP_RECV_QUEUE);
        B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpSelectQueu Failed ...", socketFd), error, rc, rc);
        rc = tcpSetQueueSeq(socketFd, pSocketState->tcpState.ack);
        B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpGetQueueSeq Failed ...", socketFd), error, rc, rc);
    }

    /* bind to this socket: Note: we reuse the socket addr & port info from the saved socket state */
    {
        BDBG_WRN(("%s: Bind to localIpAddr:port %s:%d" B_ASP_MSG_PRE_ARG, inet_ntoa(pSocketState->localIpAddr.sin_addr), ntohs(pSocketState->localIpAddr.sin_port)));
        rc = bind(socketFd, (struct sockaddr *) &pSocketState->localIpAddr, sizeof(pSocketState->localIpAddr));
        B_ASP_CHECK_GOTO( rc == 0, "Failed to bind to socket: ", strerror(errno), socketFd), error, rc, rc);
    }

    /* Connect to the remote socket address (this simply attaches the remote IP & port). */
    {
        BDBG_WRN(("%s: remoteIpAddr:port %s:%d" B_ASP_MSG_PRE_ARG, inet_ntoa(pSocketState->remoteIpAddr.sin_addr), ntohs(pSocketState->remoteIpAddr.sin_port)));
        rc = connect(socketFd, (struct sockaddr *) &pSocketState->remoteIpAddr, sizeof(pSocketState->remoteIpAddr));
        B_ASP_CHECK_GOTO( rc == 0, "Connect Error: ", strerror(errno), socketFd), error, rc, rc);
    }

    /* Unfreeze the socket as it is ready to go! */
    {
        rc = tcpUnfreezeSocket(socketFd);
        B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: tcpUnfreezeSocket Failed...", socketFd), error, rc, rc);
        BDBG_MSG(("%s: TCP State after migration back to Linux: seq %u, ack %u, mss %d" B_ASP_MSG_PRE_ARG,
                pSocketState->tcpState.seq, pSocketState->tcpState.ack, pSocketState->tcpState.maxSegSize));
    }

    if (pOutSocketFd)
    {
        BDBG_WRN(("%s: Caller doesn't want us to close the migrated socket & needs access to it, returning=%d as newSocketFd!" B_ASP_MSG_PRE_ARG, socketFd));
        *pOutSocketFd = socketFd;
        goto out;
    }
    else
    {
        shutdown(socketFd, SHUT_RDWR);
        close(socketFd);
        BDBG_WRN(("%s: Closed migratedSocketFd=%d" B_ASP_MSG_PRE_ARG, socketFd));
    }

out:
    return 0;

error:
    return NEXUS_OS_ERROR;
#endif  /* TEMPORARILY_DISABLE_REVERSE_MIGRATION */
}

int B_Asp_GetSocketAddrInfo(
    int                         socketFd,           /* in: fd of socket to be offloaded from host to ASP. */
    B_AspSocketState            *pSocketState       /* out: associated socket address info. */
    )
{
    int rc;
    int addrLen = sizeof(pSocketState->localIpAddr);
    int savedErrno;

    BDBG_ASSERT(pSocketState);

    addrLen = sizeof(pSocketState->localIpAddr);
    /* Get local ip_addr and port details.*/
    rc = getsockname(socketFd, (struct sockaddr *)&pSocketState->localIpAddr, (socklen_t *)&addrLen);
    savedErrno = errno;
    B_ASP_CHECK_GOTO(rc==0, ("socketFd=%d: failed to get local address info..", socketFd), error, rc, rc);
    BDBG_MSG((B_ASP_MSG_PRE_FMT "LocalIpAddr:Port %s:%d" B_ASP_MSG_PRE_ARG,
                inet_ntoa(pSocketState->localIpAddr.sin_addr), ntohs(pSocketState->localIpAddr.sin_port)));

#if ENDIANNESS_DEBUG
    {
        uint8_t *pByte = (uint8_t *)&pSocketState->localIpAddr.sin_port;
        BDBG_MSG(("local port in hex=0x%x (Linux returned in BE, shown in LE on ARM), byte[0]=0x%x byte[1]=0x%x", pSocketState->localIpAddr.sin_port, pByte[0], pByte[1]));
        pByte = (uint8_t *)&pSocketState->localIpAddr.sin_addr.s_addr;
        BDBG_MSG(("local IP in hex=0x%x , Linux returned in BE, shown in LE on ARM), byte[0]=0x%x byte[1]=0x%x byte[2]=0x%x byte[3]=0x%x", pSocketState->localIpAddr.sin_addr.s_addr, pByte[0], pByte[1], pByte[2], pByte[3]));
    }
#endif
    /* Get remote ip_addr and port details.*/
    addrLen = sizeof(pSocketState->remoteIpAddr);
    rc = getpeername(socketFd, (struct sockaddr *)&pSocketState->remoteIpAddr, (socklen_t *)&addrLen);
    savedErrno = errno;
    if (rc != 0 && errno == ENOTCONN)           /* ENOTCONN => Socket not connected. */
    {
        BDBG_WRN((B_ASP_MSG_PRE_FMT "getpeername(), errno=ENOTCONN, Assume socket closed" B_ASP_MSG_PRE_ARG ));
        pSocketState->connectionLost = true;    /* Indicate connection lost. */
        return 0;     /* return success... losing connection is normal if client changes channel. */
    }

    B_ASP_CHECK_GOTO( rc == 0, ("socketFd=%d: failed to get peer address info..", socketFd), error, rc, rc);
    BDBG_MSG((B_ASP_MSG_PRE_FMT "RemoteIpAddr:Port %s:%d" B_ASP_MSG_PRE_ARG,
                inet_ntoa(pSocketState->remoteIpAddr.sin_addr), ntohs(pSocketState->remoteIpAddr.sin_port)));
    return 0;

error:
    errno = savedErrno;
    return (rc);
} /* B_Asp_GetSocketAddrInfo */

static int getInterfaceNameUsingIfIndex(
    int                     ifIndex,
    char *                  pInterfaceName
    )
{
    struct ifaddrs *ifaddr = NULL;
    struct ifaddrs *ifeach = NULL;
    int rc;
    int savedErrno;

    rc = getifaddrs(&ifaddr);
    savedErrno = errno;
    if (rc == -1)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "Error: getifaddrs() failed, errno=%d" B_ASP_MSG_PRE_ARG, errno));
        errno = savedErrno;
        return (rc);
    }
    else
    {
        for (ifeach = ifaddr; ifeach != NULL; ifeach = ifeach->ifa_next)
        {
            if (ifeach->ifa_addr && ifeach->ifa_addr->sa_family == AF_PACKET )
            {
                struct sockaddr_ll *sockaddr = (struct sockaddr_ll *)ifeach->ifa_addr;

                if (sockaddr->sll_ifindex == ifIndex)
                {
                    /* Found matching interface, save the interface name. */
                    strncpy(pInterfaceName, ifeach->ifa_name, IFNAMSIZ);
                    BDBG_MSG((B_ASP_MSG_PRE_FMT "Found InterfaceIndex=%d Interfacename=%s" B_ASP_MSG_PRE_ARG, sockaddr->sll_ifindex, pInterfaceName));
                    break;
                }
            }
        }
        freeifaddrs(ifaddr);
    }

    return 0;
}

int B_Asp_UpdateInterfaceName(
    B_AspSocketState       *pSocketState,
    char                          *pInterfaceName /* out: fill-in the updated interface name here */
    )
{
    int rc;
    int netLinkFd = -1;
    unsigned char *pRespBuffer = NULL;
    int savedErrno;

    BDBG_MSG((B_ASP_MSG_PRE_FMT "pInterfaceName=%s Remote's MAC address=%02X:%02X:%02X:%02X:%02X:%02X\n" B_ASP_MSG_PRE_ARG,
                pInterfaceName,
                pSocketState->remoteMacAddress[0], pSocketState->remoteMacAddress[1],
                pSocketState->remoteMacAddress[2], pSocketState->remoteMacAddress[3],
                pSocketState->remoteMacAddress[4], pSocketState->remoteMacAddress[5]
             ));

    /* Open NetLink Socket to communicate w/ Kernel's Neighbor table logic. */
    {
        struct sockaddr_nl netLinkLocalAddr;

        netLinkFd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE); /* Note: AF_NETLINK family is NETLINK_ROUTE. */
        savedErrno = errno;
        B_ASP_CHECK_GOTO( netLinkFd == 0, ("Failed to create new AF_NETLINK socket: "), error, rc, -1);

        /* We call bind so that kernel can assign a unique nlmsg_pid to this socket. */
        memset(&netLinkLocalAddr, 0, sizeof(netLinkLocalAddr));
        netLinkLocalAddr.nl_family = AF_NETLINK;
        rc = bind(netLinkFd, (struct sockaddr*)&netLinkLocalAddr, sizeof(netLinkLocalAddr));
        savedErrno = errno;
        B_ASP_CHECK_GOTO(rc == 0, ("failed to bind() to AF_NETLINK socket."), error, rc, rc);

        BDBG_MSG((B_ASP_MSG_PRE_FMT "netLinkFd=%d" B_ASP_MSG_PRE_ARG, netLinkFd));
    }

    /* Send request to fetch the Neighbor table from Linux. */
    {
        struct
        {
            struct nlmsghdr netLinkMsg;
            struct ifinfomsg ifInfoMsg;
        } reqMsg;

        /* Prepare the Request where we send message of type RTM_GETNEIGH to the NETLINK_ROUTE family of NetLink. */
        BKNI_Memset(&reqMsg, 0, sizeof(reqMsg));
        reqMsg.netLinkMsg.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
        reqMsg.netLinkMsg.nlmsg_type = RTM_GETNEIGH;
        reqMsg.netLinkMsg.nlmsg_flags = NLM_F_DUMP|NLM_F_REQUEST;
        reqMsg.netLinkMsg.nlmsg_seq = 0;
        reqMsg.netLinkMsg.nlmsg_pid = 0;
        /* Further tell the NETLINK_ROUTE family that this request applies to the PF_BRIDGE protocol. */
        reqMsg.ifInfoMsg.ifi_family = PF_BRIDGE;

        rc = send(netLinkFd, (void *)&reqMsg, sizeof(reqMsg), 0);
        savedErrno = errno;
        B_ASP_CHECK_GOTO( rc == 0, ("Failed to send():"), error, rc, rc );

        BDBG_MSG((B_ASP_MSG_PRE_FMT "Request to get Neighbor table sent, netLinkFd=%d" B_ASP_MSG_PRE_ARG, netLinkFd));
    }

    /* Recv the response message containing the Neighbor table fields & parse for entry matching the remote's MAC address. */
    {
#define NETLINK_RESPONSE_SIZE 8192
        ssize_t respMsgLength;

        pRespBuffer = BKNI_Malloc(NETLINK_RESPONSE_SIZE);
        B_ASP_CHECK_GOTO(pRespBuffer, ("BKNI_Malloc() Failed for size=%d", NETLINK_RESPONSE_SIZE), error, rc, -1);
        BKNI_Memset(pRespBuffer, 0, NETLINK_RESPONSE_SIZE);

        /* Keep reading until we find the L2 entry matching w/ the local MAC address. */
        while (true)
        {
            respMsgLength = recv(netLinkFd, pRespBuffer, NETLINK_RESPONSE_SIZE, MSG_WAITALL);
            savedErrno = errno;
            if (respMsgLength < 0)
            {
                if (errno == EINTR || errno == EAGAIN) continue;
                BDBG_ERR(( B_ASP_MSG_PRE_FMT "Failed to recv() response for Neighbor table request on netLinkFd=%d error=%s errno=%d" B_ASP_MSG_PRE_ARG, netLinkFd, strerror(errno), errno));
                goto error;
            }
            else if (respMsgLength == 0)
            {
                BDBG_WRN((B_ASP_MSG_PRE_FMT "Got EOF on recv() while reading response for Neighbor table request on netLinkFd=%d" B_ASP_MSG_PRE_ARG, netLinkFd));
                goto error;
            }
            else
            {
                /* Response msg contains multiple NetLink headers & associated payloads. */
                /* Each NetLink header & its payload has following layout: */
                /* struct nlmsghdr + struct ndmsg + struct rtattr + rta_type specific data */
                /* We will parse each of these NetLink headers to find the one matching w/ remote's MAC address. */
                struct nlmsghdr *pNetLinkMsg;

                pNetLinkMsg = (struct nlmsghdr*)pRespBuffer; /* First netLink header */
                while (true)
                {
                    if (NLMSG_OK(pNetLinkMsg, respMsgLength) == 0)
                    {
                        BDBG_ERR(( B_ASP_MSG_PRE_FMT "NLMSG_OK returned 0, so breaking, respMsgLength=%zd" B_ASP_MSG_PRE_ARG, respMsgLength));
                        goto done;
                    }
                    if (pNetLinkMsg->nlmsg_type == NLMSG_DONE)
                    {
                        /* We got the NetLink header w/ type DONE, so that is the last header in the current response buffer, We are done! */
                        BDBG_MSG((B_ASP_MSG_PRE_FMT "Got NLMSG_DONE while parsing Neighbor table netLinkFd=%d" B_ASP_MSG_PRE_ARG, netLinkFd));
                        goto done;
                    }
                    else if (pNetLinkMsg->nlmsg_type == NLMSG_ERROR)
                    {
                        struct nlmsgerr *pNlMsgError = (struct nlmsgerr*)NLMSG_DATA(pNetLinkMsg);
                        if (pNetLinkMsg->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr)))
                        {
                            BDBG_ERR(( B_ASP_MSG_PRE_FMT "Got NLMSG_ERROR while parsing Neighbor table: NetLink Error header (size=%zu) is partial=%u"
                                        B_ASP_MSG_PRE_ARG, NLMSG_LENGTH(sizeof(struct nlmsgerr)), pNetLinkMsg->nlmsg_len));
                        }
                        else
                        {
                            errno = -pNlMsgError->error;
                            BDBG_ERR(( B_ASP_MSG_PRE_FMT "Got NLMSG_ERROR while parsing Neighbor table: error=%s errno=%d" B_ASP_MSG_PRE_ARG, strerror(errno), errno));
                        }
                        goto error;
                    }
                    else
                    {
                        /* Got a valid NetLink Msg. Now parse the Neighbor header (ndmsg), */
                        /* which will further contain the route attributes (rtattr) related header. */
                        /* This layout looks like this: */
                        /* struct ndmsg + [struct rtattr + rta_type specific data]* */
                        /* Note: there can be 1 or more rtattr attributes after the ndmsg structure. */
                        /* The MAC address is in the NDA_LLADDR type rt attribute!. */
                        struct ndmsg *pNdMsg;
                        ssize_t rtaMsgLength;
                        struct rtattr *pRtaMsg;
                        int matchedIfIndex = -1;
                        int masterIfIndex = -1;

                        if (pNetLinkMsg->nlmsg_type != RTM_NEWNEIGH )
                        {
                            BDBG_MSG((B_ASP_MSG_PRE_FMT "Got NetLink Msg w/ type=%d != RTM_NEWNEIGH, ignore it!" B_ASP_MSG_PRE_ARG, pNetLinkMsg->nlmsg_type));
                            goto nextNlHeader;
                        }

                        /* Now parse the Neighbor Discovery Msg header, it is the payload of the NetLink msg. */
                        pNdMsg = NLMSG_DATA(pNetLinkMsg);
                        if (pNdMsg->ndm_family != AF_BRIDGE)
                        {
                            BDBG_MSG((B_ASP_MSG_PRE_FMT "Got NDM Msg w/ family=%d != AF_BRIDGE, ignore it!" B_ASP_MSG_PRE_ARG, pNetLinkMsg->nlmsg_type));
                            goto nextNlHeader;
                        }

                        /* We have valid Neighbord Discovery (ndm) header. */
                        /* Check if the ndmsg contains the valid rt attribute. */
                        rtaMsgLength = pNetLinkMsg->nlmsg_len - NLMSG_LENGTH(sizeof(*pNdMsg));
                        if (rtaMsgLength < 0)
                        {
                            BDBG_ERR(( B_ASP_MSG_PRE_FMT "rtaMsgLength=%zd is invalid, nlmsg_len=%u nl+nd hdr length=%zu"
                                        B_ASP_MSG_PRE_ARG, rtaMsgLength, pNetLinkMsg->nlmsg_len, NLMSG_LENGTH(sizeof(*pNdMsg))));
                            goto error;
                        }
                        pRtaMsg = (struct rtattr*)( ((char*)pNdMsg) + NLMSG_ALIGN(sizeof(struct ndmsg)) );

                        while (RTA_OK(pRtaMsg, rtaMsgLength))
                        {
                            /* Note: same MAC address may have multiple entries in the neighbor table. */
                            /* We are interested in finding the one whose masterIndex matches the master index */
                            /* associated with given MAC. masterIndex refers to the index of the bridge interface */
                            /* to which this real interface belongs to. */
#if 0
                            BDBG_MSG(("rt attribute: type=%d len=%d", pRtaMsg->rta_type, pRtaMsg->rta_len));
#endif
                            if (pRtaMsg->rta_type == NDA_LLADDR)
                            {
                                unsigned char *macAddress = RTA_DATA(pRtaMsg);
                                if (memcmp(pSocketState->remoteMacAddress, macAddress, sizeof(pSocketState->remoteMacAddress)) == 0)
                                {
                                    BDBG_MSG((B_ASP_MSG_PRE_FMT "MAC address matched: ifIdx=%d l2Hdr: size=%lu value=0x%02X%02X%02X%02X%02X%02X\n" B_ASP_MSG_PRE_ARG,
                                            pNdMsg->ndm_ifindex,
                                            (unsigned long)RTA_PAYLOAD(pRtaMsg),
                                            macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]
                                          ));
                                    matchedIfIndex = pNdMsg->ndm_ifindex;
                                }
                            }
                            if (pRtaMsg->rta_type == NDA_MASTER)
                            {
                                masterIfIndex = *((uint32_t *)RTA_DATA(pRtaMsg));
                            }
                            if (matchedIfIndex >=0 && pSocketState->masterInterfaceIndex == masterIfIndex)
                            {
                                BDBG_MSG((B_ASP_MSG_PRE_FMT "Found the matching entry: matchedIfIndex=%d masterIfIndex=%d" B_ASP_MSG_PRE_ARG, matchedIfIndex, masterIfIndex));
                                /* Find the interface name associated with this index. */
                                if ( getInterfaceNameUsingIfIndex(matchedIfIndex, pInterfaceName) != 0)
                                {
                                    BDBG_WRN((B_ASP_MSG_PRE_FMT "Invalid interfaceIndex=%d, failed to map it to any existing interface!.." B_ASP_MSG_PRE_ARG, pNdMsg->ndm_ifindex));
                                    goto error;
                                }
                                goto done;
                            }
                            pRtaMsg = RTA_NEXT(pRtaMsg, rtaMsgLength);
                        } /* while loop for processing rt attributes. */
                    } /* else for valid NetLink msg. */
nextNlHeader:
                    pNetLinkMsg = NLMSG_NEXT(pNetLinkMsg, respMsgLength); /* this updates the respMsgLength */
                } /* while loop for processing NetLink headers. */
            } /* valid recv() response */
        }
    }
done:
    if (netLinkFd >= 0) close(netLinkFd);
    if (pRespBuffer) BKNI_Free(pRespBuffer);
    return 0;

error:
    if (netLinkFd >= 0) close(netLinkFd);
    if (pRespBuffer) BKNI_Free(pRespBuffer);
    errno = savedErrno;
    return -1;
}

int B_Asp_DeleteFlowClassificationRuleFromNwSwitch(
    B_AspSocketState     *pSocketState,
    int                          ruleIndex
    )
{
    int sock;
    struct ifreq ifr;
    int rc;
    struct ethtool_rxnfc rule;
    int savedErrno;

    BKNI_Memset(&rule, 0, sizeof(rule));
    rule.cmd = ETHTOOL_SRXCLSRLDEL;
    rule.flow_type = TCP_V4_FLOW;

    rule.fs.flow_type = TCP_V4_FLOW;
    rule.fs.location = ruleIndex;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    savedErrno = errno;
    if (sock == -1)
    {
        int myerrno = errno;
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "socket() failed, errno=%d" B_ASP_MSG_PRE_ARG, myerrno));
        errno = savedErrno;
        return NEXUS_OS_ERROR;
    }

    BKNI_Memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pSocketState->interfaceName, IF_NAMESIZE-1);
    ifr.ifr_data = (void *)&rule;
    rc = ioctl(sock, SIOCETHTOOL, &ifr);
    savedErrno = errno;
    if (rc < 0) BDBG_WRN((B_ASP_MSG_PRE_FMT "ETHTOOL_GRXCLSRULE ioctl failed: errno=%d" B_ASP_MSG_PRE_ARG, errno));

    BDBG_MSG((B_ASP_MSG_PRE_FMT "ruleIdx=%d deleted" B_ASP_MSG_PRE_ARG, rule.fs.location));

    close(sock);
    errno = savedErrno;
    return 0;
}

int B_Asp_AddFlowClassificationRuleToNwSwitch(
    B_AspSocketState            *pSocketState,
    int                         aspSwitchPortNumber,
    int                         aspSwitchQueueNumber,
    int                         *ruleIndex              /* out */
    )
{
    int sock;
    int savedErrno;
    struct ifreq ifr;
    int rc;
    struct ethtool_rxnfc rule;

    /*
     * Example ethtool string (converted to ioctl below):
     * ethtool --config-nfc gphy flow-type udp4 src-ip 192.168.1.1 dst-ip 192.168.1.201 src-port 41262 dst-port 5001 action 7
     * Note: gphy is the interface/device for which this rule applies! Kernel will map this name to the switch port # on which to set this CFP rule.
     * action: implies the port number to which the CFP classified packets are forwarded to (it is 7 in this example above).
     *
     * Also, note that rule is setup with view of incoming packet in mind. The rule instructs the switch to redirect those  packets to ASP.
     * Such packets will have local IP & Port as the destination & remote IP & Port as the source.
     */
    BKNI_Memset(&rule, 0, sizeof(rule));
    rule.cmd = ETHTOOL_SRXCLSRLINS;
    rule.flow_type = TCP_V4_FLOW;

#define PORT_NUMBER_MULTIPLER   8   /* Since we only have one field (ring_cookie) to indicate both Port & Q #s to Linux, we use this multiplier for identifying the port number. */
    rule.fs.ring_cookie = aspSwitchPortNumber * PORT_NUMBER_MULTIPLER + aspSwitchQueueNumber;
    rule.fs.location = RX_CLS_LOC_ANY;
    rule.fs.flow_type = TCP_V4_FLOW;
    rule.fs.h_u.usr_ip4_spec.ip_ver = ETH_RX_NFC_IP4;
    rule.fs.h_u.tcp_ip4_spec.ip4src = pSocketState->remoteIpAddr.sin_addr.s_addr;

    if (pSocketState->remoteIpAddr.sin_addr.s_addr == pSocketState->localIpAddr.sin_addr.s_addr)
    {
        rule.fs.h_u.tcp_ip4_spec.ip4dst = pSocketState->aspIpAddr.sin_addr.s_addr;
    }
    else
    {
        rule.fs.h_u.tcp_ip4_spec.ip4dst = pSocketState->localIpAddr.sin_addr.s_addr;
    }
    rule.fs.h_u.tcp_ip4_spec.psrc = pSocketState->remoteIpAddr.sin_port;
    rule.fs.h_u.tcp_ip4_spec.pdst = pSocketState->localIpAddr.sin_port;

    /* Set the masks. */
    rule.fs.m_u.tcp_ip4_spec.ip4src = 0xffffffff;
    rule.fs.m_u.tcp_ip4_spec.ip4dst = 0xffffffff;
    rule.fs.m_u.tcp_ip4_spec.psrc = 0xffff;
    rule.fs.m_u.tcp_ip4_spec.pdst = 0xffff;
    rule.fs.m_u.usr_ip4_spec.ip_ver = ETH_RX_NFC_IP4;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    savedErrno = errno;
    if (sock == -1)
    {
        savedErrno = errno;
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "ioctl() socket() failed, errno=%d" B_ASP_MSG_PRE_ARG, savedErrno));
        return -1;
    }

    BKNI_Memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pSocketState->interfaceName, IF_NAMESIZE-1);
    ifr.ifr_data = (void *)&rule;
    rc = ioctl(sock, SIOCETHTOOL, &ifr);
    savedErrno = errno;
    if (rc < 0)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "ioctl() SIOCETHTOOL Failed to add the Classification rule to network switch, errno=%d" B_ASP_MSG_PRE_ARG, savedErrno));
        perror("ioctl: ");
    }
    BDBG_MSG((B_ASP_MSG_PRE_FMT "IP:Port local=0x%x:%d, remote=0x%x:%d, switchFwdPort#=%d, switchFwdPortQ#=%d ring_cookie=%d ruleIdx=%d" B_ASP_MSG_PRE_ARG,
                ntohl(rule.fs.h_u.tcp_ip4_spec.ip4src),
                ntohs(rule.fs.h_u.tcp_ip4_spec.psrc),
                ntohl(rule.fs.h_u.tcp_ip4_spec.ip4dst),
                ntohs(rule.fs.h_u.tcp_ip4_spec.pdst),
                aspSwitchPortNumber,
                aspSwitchQueueNumber,
                (int)rule.fs.ring_cookie,
                rule.fs.location
             ));
    if (rc >= 0)
    {
        *ruleIndex = rule.fs.location;
    }

    close(sock);
    errno = savedErrno;
    return rc;
}
