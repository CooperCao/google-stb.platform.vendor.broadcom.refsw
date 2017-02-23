/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description: simple test app to demonstrate UDP/RTP receiving using Normal & Accelerated Sockets
 *
 *************************************************************/
#include "nexus_platform.h"
#include <stdio.h>

#if defined(LINUX) && NEXUS_HAS_PLAYBACK && NEXUS_HAS_AUDIO
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_video_decoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_audio_dac.h"
#include "nexus_spdif_output.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/time.h>

#include "bnetaccel_info.h"

/* the following define the input and its characteristics -- these will vary by input */
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 69
#define AUDIO_PID 68

/* Simple test App to demonstrate IP tuning using Nexus Playpump Interface */
/* It supports mode to read from standard DGRAM socket and Broadcom's accelerated socket type */

/* **************************************************************************** */
/*       !!!!!!!!!!!!!!!!!!!!              NOTE         !!!!!!!!!!!!!!!!!!!!!!! */
/* **************************************************************************** */
/* This example **doesn't** provide an example of a Nexus Configuration for supporting high n/w jitter */
/* which is typically present in the IP Networks. */

/* For a complete IP Application, please use the NEXUS IP Applib. There is an example app, */
/* ip_applib_test.c, in rockford/unittests/applibs/playback_ip to demonstrate its usage. */
/* This example app should be used to write customer application. */

#define ALIGN_4096   (4096 - 1)
#define MEM_SIZE 4*1024*1024
#define UDP_HDR_SIZE 8
#define RTP_HDR_SIZE 12

typedef enum {
    eProtoUDP,
    eProtoRTP,
    eProtoMAX
} eProtos;

typedef struct {
    int pktsPerRead;
    eProtos proto;
    int writeToDisk;
    int doSockAccel;
    int timeout;
    int tsSize;
    int iphVersion;
    short port;
    char *host;
    char *service;
    char *filename;
    int pktSize;
    int decode;
}ExampleIpCfg;

typedef struct {
    int sfd;
    int wfd;
    int bufSize;
    int hdrSize;
    struct iovec *vector;
    uint8_t *buf;
    int bytesRecv;
    int bytesToFeed;
    NEXUS_PlaypumpHandle playpump;
    BKNI_EventHandle event;
    unsigned char sockaddr_storage[NI_MAXHOST];
}ExampleIpCtx;


int feedToPlayback = 0;

#define DEBUG
#ifdef DEBUG
struct udphdr {
    unsigned short   source;
    unsigned short   dest;
    unsigned short   len;
    unsigned short   checksum;
};

typedef struct rtphdr {
    uint16_t word0;
#define VERSION_MASK 0xc000
#define VERSION_SHIFT 14
#define PADDING_MASK 0x2000
#define PADDING_SHIFT 13
#define EXTENSION_MASK 0x1000
#define EXTENSION_SHIFT 12
#define CSRC_MASK 0x0f00
#define CSRC_SHIFT 8
#define MARKER_MASK 0x0080
#define MARKER_SHIFT 7
#define TYPE_MASK 0x007f
#define TYPE_SHIFT 0
    uint16_t sequenceNum;
    uint32_t timestamp;
    uint32_t ssrc;
} rtphdr;

void printRtp(rtphdr *rtp)
{
    uint16_t word0;
    word0 = ntohs(rtp->word0);
    printf("RTP Hdr: Version %d Pad %d Ext %d CRSC count %d Marker %d Type %d ",
            (word0 & VERSION_MASK) >> VERSION_SHIFT,
            (word0 & PADDING_MASK) >> PADDING_SHIFT,
            (word0 & EXTENSION_MASK) >> EXTENSION_SHIFT,
            (word0 & CSRC_MASK) >> CSRC_SHIFT,
            (word0 & MARKER_MASK) >> MARKER_SHIFT,
            (word0 & TYPE_MASK) >> TYPE_SHIFT
          );
    printf("Seq %d, TS %u, SSRC %u\n",
            ntohs(rtp->sequenceNum),
            ntohl(rtp->timestamp),
            ntohl(rtp->ssrc));
}

void printTS(unsigned char *ptr)
{
    printf("Transport Header Fields are:\n");
    printf("[0] 0x%x, [1] 0x%x, [2] 0x%x, [3] 0x%x\n", ptr[0], ptr[1], ptr[2], ptr[3]);
}

#endif /* DEBUG */

void usage(void)
{
    printf("Usage: playpump_ip -d <ip> -p <port> [-f <content-file>] [-a] [-s <num>] [-v <num>] [-n <num>] [-t <num>] [-w] [-h] \n");
    printf("options are:\n");
    printf(" -a            # use accelerated socket type, default is regular socket\n");
    printf(" -d <ip>       # IP address of Live IP Service (e.g. 192.168.1.110)\n");
    printf(" -p <port>     # Port of Live IP Service (e.g. 5000)\n");
    printf(" -f <file>     # file to write content to (default rec0.mpg\n");
    printf(" -s <num>      # size of MPEG2 TS packets, default 188 bytes\n");
    printf(" -v            # IP version to use: IPv4: 4, IPv6: 6, default is IPv4\n");
    printf(" -t <0|1>      # protocol: UDP(0)|RTP(1), default is RTP\n");
    printf(" -n <num>      # pkts to receive per read/recvfrom call, default is 32\n");
    printf(" -z <num>      # Per Datagram size, default hdr sz + 7 *TS size\n");
    printf(" -w            # write To Disk, default is no write\n");
    printf(" -h            # prints playpump_ip usage\n");
    printf("\n");
}

void update_stats(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
    static int pktCnt = 0;
    static int total;
    int port;
    char hostbuf[NI_MAXHOST] = {0};
    struct sockaddr *saddr;

    saddr = (struct sockaddr *)ipCtx->sockaddr_storage;
    total += ipCtx->bytesRecv;
    pktCnt += ipCfg->pktsPerRead;
    if (pktCnt % (ipCfg->pktsPerRead * 100) == 0) {
        if (saddr->sa_family == AF_INET) {
            inet_ntop(saddr->sa_family, &((struct sockaddr_in *)saddr)->sin_addr, hostbuf, NI_MAXHOST);
            port = ntohs(((struct sockaddr_in *)saddr)->sin_port);
        }
        else {
            inet_ntop(saddr->sa_family, &((struct sockaddr_in6 *)saddr)->sin6_addr, hostbuf, NI_MAXHOST);
            port = ntohs(((struct sockaddr_in6 *)saddr)->sin6_port);
        }
        printf("Rcvd %u bytes in %u pkts from %s:%d, total bytes rcvd %u\n",
            ipCtx->bytesRecv, pktCnt, hostbuf, port, total);
    }
}

void set_default_ip_cfg(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
    BSTD_UNUSED(ipCtx);
    ipCfg->pktsPerRead = 32;
    ipCfg->proto = eProtoRTP;
    ipCfg->writeToDisk = 0;
    ipCfg->doSockAccel = 0;
    ipCfg->timeout = 20;
    ipCfg->tsSize = 188;
    ipCfg->iphVersion = 4;
    ipCfg->filename = "./rec0.mpg";
    ipCfg->decode = 1;
}

int allocate_buffers(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
    int i;
    uint8_t *xbuf;

    /*
     * Note: on NetAccel copies UDP & RTP headers along with the each video payload
     * This allows applications do any RTP speoific processing (e.g FEC).
     * In some instances, the payload size can vary from packet to packet (upto 1-7 MPEG2 TS packets)
     * For this reason, UDP header is also copied along w/ RTP header. This allows apps to determine
     * per packet payload length (udp length field
     */

    /* Calculate the protocol header length: Add RTP Header Size for RTP Protocol */
    if (ipCfg->doSockAccel)
        (ipCfg->proto == eProtoUDP) ? (ipCtx->hdrSize = UDP_HDR_SIZE) : (ipCtx->hdrSize = UDP_HDR_SIZE + RTP_HDR_SIZE);
    else
        /* standard linux network stack doesn't provide access to UDP header */
        (ipCfg->proto == eProtoUDP) ? (ipCtx->hdrSize = 0) : (ipCtx->hdrSize = RTP_HDR_SIZE);

    /* if user didn't specify per packet size, calculate it */
    if (!ipCfg->pktSize)
        ipCfg->pktSize = ipCtx->hdrSize + (7*ipCfg->tsSize);

    /* and use that to determine buffer needed to hold pktsPerRead worth of data */
    ipCtx->bufSize = ipCfg->pktSize * ipCfg->pktsPerRead;
    printf("Options used are --> proto %d, hdrSize = %d, bufSize %d, writeToDisk %d, pktsPerRead %d, sockAccel %d\n",
            ipCfg->proto, ipCtx->hdrSize, ipCtx->bufSize, ipCfg->writeToDisk, ipCfg->pktsPerRead, ipCfg->doSockAccel);

    /* allocate receive buffer */
    if ( (xbuf = malloc(ipCtx->bufSize + ALIGN_4096)) == NULL ) {
        perror("malloc failure():");
        goto exit;
    }
    ipCtx->buf = (uint8_t *)(((unsigned long)xbuf + ALIGN_4096) & ~ALIGN_4096);
    memset(xbuf, (ipCtx->bufSize + ALIGN_4096), 0);
    printf("Successfully malloced (addr = %p)\n", ipCtx->buf);

    /* setup the vector arrary: each vector corresponds to a pkt */
    if ( (ipCtx->vector = malloc(sizeof(struct iovec) * ipCfg->pktsPerRead)) == NULL ) {
        perror("malloc failure():");
        goto exit;
    }
    for (i=0; i < ipCfg->pktsPerRead; i++) {
        ipCtx->vector[i].iov_base = ipCtx->buf + i*ipCfg->pktSize;
        ipCtx->vector[i].iov_len = ipCfg->pktSize;
    }
    return 0;

exit:
    return -1;
}

int setup_ip_socket(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
    int sfd;
    int ret;
    struct addrinfo  hints;
    struct addrinfo *addrInfo;
    char hostbuf[NI_MAXHOST];
    char serverbuf[NI_MAXSERV];
    struct ip_mreq mreq;
    struct ipv6_mreq ipv6mreq;
    STRM_SockRecvParams_t sockRecvParams;


    /* prep up fields for the getaddrinfo() */
    memset(&hints, 0, sizeof(hints));

    /* set socket family */
    (ipCfg->iphVersion == 4) ? (hints.ai_family = AF_INET) : (hints.ai_family = AF_INET6);
    hints.ai_protocol = IPPROTO_UDP;

    /* set socket type */
    /* set it to standard socktype as getaddrinfo doesn't understand our new sock type */
    /* after getaddrinfo() call, we change the type to new Broadcom accelerated socket type */
    hints.ai_socktype = SOCK_DGRAM;
    if ((ret = getaddrinfo(ipCfg->host, ipCfg->service, &hints, &addrInfo)) != 0) {
        printf("getaddrinfo() Failed: error code = %d\n", ret);
        goto exit;
    }

    /* now update the sock type to our new type */
    /* this is a hack until we update the uclibc to understand the new socket type */
    if (ipCfg->doSockAccel)
            addrInfo->ai_socktype = SOCK_BRCM_DGRAM;

    if ( ipCfg->iphVersion == 6 && ((struct sockaddr_in6 *)addrInfo->ai_addr)->sin6_scope_id == 0) {
        printf("IPv6 scope-ID is not set\n");
        ((struct sockaddr_in6 *)addrInfo->ai_addr)->sin6_scope_id = 1;
    }

    printf("IP Address Info:\n"
            "   family       = %2d (PF_INET = %d, PF_INET6 = %d)\n"
            "   socktype     = %2d (SOCK_DRAM = %d, SOCK_BRCM_DGRAM %d, SOCK_STREAM = %d, SOCK_BRCM_STREAM %d)\n"
            "   protocol     = %2d (IPPROTO_UDP = %d, IPPROTO_TCP %d)\n"
            "   addrlen      = %2d\n"
            "   flags        = %d\n",
            addrInfo->ai_family,
            PF_INET, PF_INET6,
            addrInfo->ai_socktype,
            SOCK_DGRAM, SOCK_BRCM_DGRAM,
            SOCK_STREAM, SOCK_BRCM_STREAM,
            addrInfo->ai_protocol,
            IPPROTO_UDP, IPPROTO_TCP,
            addrInfo->ai_addrlen,
            addrInfo->ai_flags);

    getnameinfo(addrInfo->ai_addr, addrInfo->ai_addrlen, hostbuf, sizeof(hostbuf),
                serverbuf, sizeof(serverbuf), NI_NUMERICHOST | NI_NUMERICSERV);

    printf("   IP Addr      = %s\n" "   Port         = %s\n", hostbuf, serverbuf);

    /* Create Datagram socket */
    if ( (sfd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol)) < 0 ) {
        perror("socket():");
        goto exit;
    }
    printf("Socket Created\n");
    ipCtx->sfd = sfd;

    /* Bind to the Receiving IP Address & Port */
    if (bind(sfd, addrInfo->ai_addr, addrInfo->ai_addrlen) < 0) {
        perror("bind():");
        goto exit;
    }
    printf("Socket bind done\n");

    /* Note: */
    /* Apps can choose to either read "n" bytes of data or "m" IP packets at a time */
    /* by default, i/f is designed to wait for "n" bytes and only then return */
    /* To instead read only "m" packets at a time, Apps can set per this number */
    /* via a socket option. read/recvfrom calls can block until these conditions are met */

    /* by default, read/recvfrom i/f returns lower of (bytes to be read or bytes in 1 pkt) */
    /* this is needed to allow app only read the essential protocol headers in the begining. */
    /* E.g. HTTP & DTCP/IP protocols headers can be read by app by reading 1 pkt at a time */
    /* once headers are read, apps can increase the # of pkts (thus read more payload data) */
    /* by setting a socket option */

    /* set the number of pkts to wait before returning from recvfrom */
    if (ipCfg->doSockAccel) {
        sockRecvParams.pktsPerRecv = ipCfg->pktsPerRead;
        /* copy packets back to back (instead of each packet begining at a fixed offset) */
        sockRecvParams.pktOffset = 0;
        /* received data starts from UDP header. To skip it, set the hdrOffset to the udp header size */
        sockRecvParams.hdrOffset = 0;
        if (setsockopt(sfd, SOCK_BRCM_DGRAM, STRM_SOCK_RECV_PARAMS, &sockRecvParams, sizeof(sockRecvParams)) != 0) {
            perror("setsockopt():recv options:");
            goto exit;
        }
        printf("Modified the default pkts per recvfrom to %d, pkt offset to %d\n", sockRecvParams.pktsPerRecv, sockRecvParams.pktOffset);
    }
    else {
        /* increase the receive buffer size for UDP socket, network
           stack will drop pkts at >14Mbps rate */
        int size = 250000;
        if (setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &size, 4) != 0) {
            perror("setsockopt():");
            goto exit;
        }
        printf("Increased the Socket RCVBUF size to %d bytes\n", size);
    }

    /* Note on each packet payload offset */
    /* read/recvfrom api also provides flexibility to copy eash packet at a fixed offset */
    /* instead of copying payloads of multiple packets back to back. */
    /* this is mainly useful for RTP processing where individual pkts can be of variable lengths */
    /* and applications wants each packet to begin at a fixed offset */
    /* set the per pkt size, kernel then copies each pkt at fixed offset */

    /* Join the Multicast Group */
    if (ipCfg->iphVersion == 4) {
        if (IN_MULTICAST(ntohl(((struct sockaddr_in *)(addrInfo->ai_addr))->sin_addr.s_addr))) {
            printf("Joining IPv4 Multicast Address Group\n");
            mreq.imr_multiaddr.s_addr = ((struct sockaddr_in *)(addrInfo->ai_addr))->sin_addr.s_addr;
            mreq.imr_interface.s_addr = INADDR_ANY;
            if (setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0) {
                perror("setsockopt():");
                goto exit;
            }
        }
    }
    else {
        /* IPv6 case */
        if ( IN6_IS_ADDR_MULTICAST(((struct sockaddr_in6 *)(addrInfo->ai_addr))->sin6_addr.s6_addr)) {
            printf("Joining IPv6 Multicast Address Group\n");
            ipv6mreq.ipv6mr_multiaddr = ((struct sockaddr_in6 *)(addrInfo->ai_addr))->sin6_addr;
            ipv6mreq.ipv6mr_interface = 0;  /* chooses default interface */
            if (setsockopt(sfd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &ipv6mreq, sizeof(ipv6mreq)) < 0) {
                perror("setsockopt():");
                goto exit;
            }
        }
    }
    return 0;

exit:
    return -1;
}

int recv_av_data_from_socket(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
    int i;
    int ret;
    uint8_t *ptr;
    fd_set rfds;
    struct timeval tv;
    socklen_t saddrLen;
    struct sockaddr *saddr;
    int sfd = ipCtx->sfd;

    FD_ZERO(&rfds);
    FD_SET(sfd, &rfds);
    tv.tv_sec = ipCfg->timeout;
    tv.tv_usec = 0;

    if ( (ret = select(sfd +1, &rfds, NULL, NULL, &tv)) < 0 ) {
        perror("select(): exiting...");
        goto exit;
    }

    if (!FD_ISSET(sfd, &rfds)) {
        printf("**** No Input From Server, EXITING.... \n");
        goto exit;
    }

    /* Use recvfrom() to receive the RTP header (if present) + payload */
    saddrLen = NI_MAXHOST;
    saddr = (struct sockaddr *)ipCtx->sockaddr_storage;

    if (ipCfg->doSockAccel) {
        if ( (ipCtx->bytesRecv = recvfrom(sfd, ipCtx->buf, ipCtx->bufSize, 0, saddr, &saddrLen)) < 0) {
            perror("recvfrom():");
            goto exit;
        }
        /*printf("read %d in %d\n", ipCtx->bytesRecv, ipCtx->bufSize); */
    }
    else {
        /* need to make multiple recvfrom calls to read multiple packets */
        ptr = ipCtx->buf;
        for (i=0; i < ipCfg->pktsPerRead; i++) {
            if ( (ipCtx->bytesRecv = recvfrom(sfd, ptr, ipCtx->bufSize, 0, saddr, &saddrLen)) < 0) {
                perror("recvfrom():");
                goto exit;
            }
            ipCtx->vector[i].iov_base = ptr;
            ipCtx->vector[i].iov_len = ipCtx->bytesRecv;
            ptr += ipCfg->pktSize;
            /* printf("read %d in 0x%x, ", ipCtx->bytesRecv, vector[i].iov_base);*/
        }
        /*printf("\n");*/
    }
    return 0;

exit:
    return -1;
}

void process_ip_pkts(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
    int i;
    int num = 0;
    uint8_t *ptr;
    uint16_t seq =0;
    static uint16_t expectedSeq = 0;
    uint16_t udpLen = 0;
    struct udphdr *udp;
    struct rtphdr *rtp;
    static int firstTime = 1;

    /* Adjust payload pointers & optionally verify RTP Sequence */
    ptr = ipCtx->buf;
    ipCtx->bytesToFeed = 0;
    for (i=0; i< ipCfg->pktsPerRead; i++) {
        num++;
        expectedSeq++;
        if (ipCfg->doSockAccel) {
            udp = (struct udphdr *)ptr;
            udpLen = ntohs(udp->len);
            rtp = (rtphdr *) (ptr + sizeof(struct udphdr));
            seq = ntohs(rtp->sequenceNum);
        }
        else {
            /* standard socket recvfrom: kernel returns only one pkt starting at RTP header */
            udpLen = ipCtx->vector[i].iov_len;
            rtp = (rtphdr *) ptr;
            seq = ntohs(rtp->sequenceNum);
        }

        /* adjust the payload pointer */
        ipCtx->vector[i].iov_base = ptr + ipCtx->hdrSize;
        ipCtx->vector[i].iov_len = udpLen - ipCtx->hdrSize;
        ipCtx->bytesToFeed += udpLen - ipCtx->hdrSize;
        if (ipCfg->proto == eProtoRTP) {
            if (!firstTime) {
                if ( seq != expectedSeq ) {
                    printf("********* Break in RTP Seq (expected seq %u, current seq %u, pktnum %d ***********\n", expectedSeq-1, seq, num);
                    printRtp(rtp);
                    expectedSeq = seq;
                    /* goto exit; */
                }
            }
            else {
                expectedSeq = ntohs(rtp->sequenceNum);
                firstTime = 0;
                printf("########## Initial RTP Seq %d, pktsize %d, pktnum %d ###########\n", expectedSeq, udpLen, num);
                printRtp(rtp);
            }
        } /* if RTP */

        /* next pkt */
        /*TODO ptr += udpLen;*/
        /* */ptr += ipCfg->pktSize;
    }

    update_stats(ipCtx, ipCfg);
}

int record_av_data(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
    int nwritten;

    if (ipCfg->proto == eProtoUDP && !ipCfg->doSockAccel) {
        /* Plain UDP, do a block write */
        if ( (nwritten = write(ipCtx->wfd, ipCtx->buf, ipCfg->pktsPerRead * ipCtx->bytesRecv)) < 0 ) {
            perror("write():");
            goto exit;
        }
        if (nwritten != ipCtx->bytesRecv * ipCfg->pktsPerRead) {
            printf("Write Error: only wrote %u our of %u received bytes\n", nwritten, ipCtx->bytesRecv);
            goto exit;
        }
    }
    else {
        /* RTP + UDP, use vectored write to skip the RTP header */
        if ( (nwritten = writev(ipCtx->wfd, ipCtx->vector, ipCfg->pktsPerRead)) < 0 ) {
            perror("write():");
            goto exit;
        }
    }
    return 0;
exit:
    return -1;
}

int feed_to_playpump(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
    unsigned char *playbackBuf;
    size_t playbackBufSize = 0;
    int bytesToFeed = 0;
    int i, feedNow = 0;
    int len;
    static int total =0;

    /*printf("*** need to feed %d bytes from %d pkts\n", ipCtx->bytesToFeed, ipCfg->pktsPerRead);*/
    i=0;
    len = ipCtx->bytesToFeed;
    while (i < ipCfg->pktsPerRead) {
        if (playbackBufSize <=0) {
            if (NEXUS_Playpump_GetBuffer(ipCtx->playpump, (void **)&playbackBuf, &playbackBufSize)) {
                printf("$$$$$$ ERROR: failed to get buffer from playpump\n");
                return -1;
            }
            if (playbackBufSize == 0) {
                BKNI_WaitForEvent(ipCtx->event, BKNI_INFINITE);
                continue;
            }
            /*printf("#### [%d] allocated buffer %p of %d bytes, need to feed %d bytes\n",
                i, playbackBuf, playbackBufSize, len);*/
        }

        /* now copy data into the playback buffer */
        if (ipCtx->vector[i].iov_len < playbackBufSize) {
            /* feed the complete packet */
            memcpy(playbackBuf, ipCtx->vector[i].iov_base, ipCtx->vector[i].iov_len);
            playbackBufSize -= ipCtx->vector[i].iov_len;
            bytesToFeed += ipCtx->vector[i].iov_len;
            playbackBuf += ipCtx->vector[i].iov_len;
            i++;
            if (i == ipCfg->pktsPerRead) {
                feedNow = 1;
            }
        }
        else {
            /* can only feed the partial packet */
            /*printf("^^^^^ [%d] feeding partial buffer, need %d, have %d, base %x\n",
                    i, vector[i].iov_len, playbackBufSize, vector[i].iov_base);*/
            memcpy(playbackBuf, ipCtx->vector[i].iov_base, playbackBufSize);
            bytesToFeed += playbackBufSize;
            ipCtx->vector[i].iov_base = (unsigned char *)ipCtx->vector[i].iov_base + playbackBufSize;
            ipCtx->vector[i].iov_len -= playbackBufSize;
            playbackBufSize = 0;

            /* reached end of playback buffer, so feed it now */
            feedNow = 1;
        }
        if (feedNow) {
            /* feed the data to the playpump */
            if (NEXUS_Playpump_WriteComplete(ipCtx->playpump, 0, bytesToFeed)) {
                printf("$$$$$ ERROR: bplaypump_read_complete() failed\n");
                return -1;
            }
            len -= bytesToFeed;
            total += bytesToFeed;
            /* printf("#####[%d] Fed %d bytes (total %d), remaining %d \n", i, bytesToFeed, total, len);*/
            bytesToFeed = 0;
            feedNow = 0;
        }
    }
    return 0;
}

int parse_options(int argc, char *argv[], ExampleIpCfg *ipCfg)
{
    int c;
    while ( (c = getopt(argc, argv, "d:p:f:z:s:v:wn:t:ah")) != -1) {
        switch (c) {
        case 'd':
            ipCfg->host = optarg;
            break;
        case 'p':
            ipCfg->service = optarg;
            break;
        case 'f':
            ipCfg->filename = optarg;
            break;
        case 'v':
            ipCfg->iphVersion = atoi(optarg);
            if (ipCfg->iphVersion != 4 && ipCfg->iphVersion != 6) {
                printf("Incorrect IP Version (%d)\n", ipCfg->iphVersion);
                goto usage;
            }
            break;
        case 't':
            ipCfg->proto = atoi(optarg);
            if (ipCfg->proto > eProtoMAX) {
                printf("Incorrect Protocol Option (%d)\n", ipCfg->proto);
                goto usage;
            }
            break;
        case 's':
            ipCfg->tsSize = atoi(optarg);
            if (ipCfg->tsSize != 188 && ipCfg->tsSize != 192 && ipCfg->tsSize != 130) {
                printf("Not supported transport Size (%d)\n", ipCfg->tsSize);
                goto usage;
            }
            break;
        case 'w':
            ipCfg->writeToDisk = 1;
            break;
        case 'n':
            ipCfg->pktsPerRead = atoi(optarg);
            if (ipCfg->pktsPerRead > 128) {
                printf("Incorrect Pkts Per Read (%d)\n", ipCfg->pktsPerRead);
                goto usage;
            }
            break;
        case 'z':
            ipCfg->pktSize = atoi(optarg);
            if (ipCfg->pktSize > 2000) {
                printf("Per UDP Datagram size %d is too large?\n", ipCfg->pktSize);
                goto usage;
            }
            break;
        case 'a':
            ipCfg->doSockAccel = 1;
            break;
        case 'h':
        default:
usage:
            usage();
            return -1;
        }
    }
    if (ipCfg->host == NULL || ipCfg->service == NULL) {
        usage();
        return -1;
    }
    return 0;
}

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

ExampleIpCtx ipCtx;
ExampleIpCfg ipCfg;

int main(int argc, char *argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_AudioDecoderHandle audioDecoder, audioPassthrough;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&ipCtx.event);

    playpump = NEXUS_Playpump_Open(0, NULL);

    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = ipCtx.event;
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);

    NEXUS_Playpump_Start(playpump);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, VIDEO_PID, NULL);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = NEXUS_Playpump_OpenPidChannel(playpump, AUDIO_PID, NULL);
    audioProgram.stcChannel = stcChannel;

    /* Bring up video decoder and display. No audio to keep the example simple. */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
		}
    }
#endif

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    audioPassthrough = NEXUS_AudioDecoder_Open(1, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), 
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
    NEXUS_AudioDecoder_Start(audioPassthrough, &audioProgram);

    printf("Playpump is ready to Receive Data...\n");

    /* set the IP socket related defults for program */
    set_default_ip_cfg(&ipCtx, &ipCfg);

    /* parse command line options */
    if (parse_options(argc, argv, &ipCfg)) {
        printf("ERROR: Incorrect Options\n");
        goto error;
    }

    /* allocate buffer to receive UDP/RTP Headers + Payload from Network */
    if (allocate_buffers(&ipCtx, &ipCfg)) {
        printf("ERROR: Failed to Allocate Memory\n");
        goto error;
    }

    /* create the socket to receive AV Data over UDP socket */
    if (setup_ip_socket(&ipCtx, &ipCfg)) {
        printf("ERROR: Failed to Setup the socket\n");
        goto error;
    }

    /* Open the temp file where received content is written to */
    if (ipCfg.writeToDisk && (ipCtx.wfd = open(ipCfg.filename, O_RDWR|O_CREAT|O_TRUNC)) < 0) {
        perror("open():");
        goto error;
    }

    ipCtx.playpump = playpump;
    printf("Socket is ready to Receive Data...\n");

    /* loops forever */
    while (1) {
        /* read av data from the socket */
        if (recv_av_data_from_socket(&ipCtx, &ipCfg))
            goto error;

        /* process IP packets: verify RTP Header (optionally decrypt AV payload) */
        process_ip_pkts(&ipCtx, &ipCfg);

        /* now write it to disk either using write or writev*/
        if (ipCfg.writeToDisk) {
            if (record_av_data(&ipCtx, &ipCfg))
                goto error;
        }

        if (ipCfg.decode) {
            if (feed_to_playpump(&ipCtx, &ipCfg))
                goto error;
        }
    }

    close(ipCtx.sfd);
    return 0;

error:
    return 1;
}
#else
int main(void)
{
    printf("This application is not supported on this OS and/or this platform\n");
    return -1;
}
#endif
