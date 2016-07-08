/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/tcp.h>
#include <netinet/ip.h>
#include "bip_server.h"
#include "tcp_connx_migration.h"

static unsigned short int checksum (unsigned short int *addr, int len)
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

// Build IPv4 TCP pseudo-header and call checksum function.
static unsigned short int tcp4_checksum(struct iphdr iphdr, struct tcphdr tcphdr, unsigned char *payload, int payloadlen)
{
    unsigned short int svalue;
    char buf[1000], cvalue;
    char *ptr;
    int i, chksumlen = 0;

    // ptr points to beginning of buffer buf
    ptr = &buf[0];

    // Copy source IP address into buf (32 bits)
    memcpy (ptr, &iphdr.saddr, sizeof (iphdr.saddr));
    ptr += sizeof (iphdr.saddr);
    chksumlen += sizeof (iphdr.saddr);

    // Copy destination IP address into buf (32 bits)
    memcpy (ptr, &iphdr.daddr, sizeof (iphdr.daddr));
    ptr += sizeof (iphdr.daddr);
    chksumlen += sizeof (iphdr.daddr);

    // Copy zero field to buf (8 bits)
    *ptr = 0; ptr++;
    chksumlen += 1;

    // Copy transport layer protocol to buf (8 bits)
    memcpy (ptr, &iphdr.protocol, sizeof (iphdr.protocol));
    ptr += sizeof (iphdr.protocol);
    chksumlen += sizeof (iphdr.protocol);

    // Copy TCP length to buf (16 bits)
    svalue = htons (sizeof (tcphdr) + payloadlen);
    memcpy (ptr, &svalue, sizeof (svalue));
    ptr += sizeof (svalue);
    chksumlen += sizeof (svalue);

    // Copy TCP source port to buf (16 bits)
    memcpy (ptr, &tcphdr.source, sizeof (tcphdr.source));
    ptr += sizeof (tcphdr.source);
    chksumlen += sizeof (tcphdr.source);

    // Copy TCP destination port to buf (16 bits)
    memcpy (ptr, &tcphdr.dest, sizeof (tcphdr.dest));
    ptr += sizeof (tcphdr.dest);
    chksumlen += sizeof (tcphdr.dest);

    // Copy sequence number to buf (32 bits)
    memcpy (ptr, &tcphdr.seq, sizeof (tcphdr.seq));
    ptr += sizeof (tcphdr.seq);
    chksumlen += sizeof (tcphdr.seq);

    // Copy acknowledgement number to buf (32 bits)
    memcpy (ptr, &tcphdr.ack_seq, sizeof (tcphdr.ack_seq));
    ptr += sizeof (tcphdr.ack_seq);
    chksumlen += sizeof (tcphdr.ack_seq);

    // Copy data offset to buf (4 bits) and
    // copy reserved bits to buf (4 bits)
    cvalue = (tcphdr.doff << 4) + tcphdr.res1;
    memcpy (ptr, &cvalue, sizeof (cvalue));
    ptr += sizeof (cvalue);
    chksumlen += sizeof (cvalue);

    // Copy TCP flags to buf (8 bits)
    cvalue = (tcphdr.fin)      +
        (tcphdr.syn << 1) +
        (tcphdr.rst << 2) +
        (tcphdr.psh << 3) +
        (tcphdr.ack << 4) +
        (tcphdr.urg << 5) +
        (tcphdr.ece << 6) +
        (tcphdr.cwr << 7);

    memcpy (ptr, &cvalue, sizeof (cvalue));
    ptr += sizeof (cvalue);
    chksumlen += sizeof (cvalue);


    // Copy TCP window size to buf (16 bits)
    memcpy (ptr, &tcphdr.window, sizeof (tcphdr.window));
    ptr += sizeof (tcphdr.window);
    chksumlen += sizeof (tcphdr.window);

    // Copy TCP checksum to buf (16 bits)
    // Zero, since we don't know it yet
    *ptr = 0; ptr++;
    *ptr = 0; ptr++;
    chksumlen += 2;

    // Copy urgent pointer to buf (16 bits)
    memcpy (ptr, &tcphdr.urg_ptr, sizeof (tcphdr.urg_ptr));
    ptr += sizeof (tcphdr.urg_ptr);
    chksumlen += sizeof (tcphdr.urg_ptr);


    if (payloadlen)
    {
        // Copy payload to buf
        memcpy (ptr, payload, payloadlen);
        ptr += payloadlen;
        chksumlen += payloadlen;

        // Pad to the next 16-bit boundary
        for (i=0; i<payloadlen%2; i++, ptr++) {
            *ptr = 0;
            ptr++;
            chksumlen++;
        }
    }
    return checksum ((unsigned short int *) buf, chksumlen);
}

TcpState gAspTcpState;
static void aspSimulator_getTcpState(TcpState *tcpState)
{
    *tcpState = gAspTcpState;
    /* TODO: may need to update ACK number - */
}

static void aspSimulator_setTcpState(TcpState *tcpState, int bytesSent)
{
    /* update ASP TCP state */
    tcpState->seq += bytesSent;

    /* TODO: may need to update ACK number - */
}

int aspSimulator_Stop(SocketStateHandle hSocketState)
{
    /* TODO: check if we need to update any fields in the tcp_info structure (most likely no!) */

    aspSimulator_getTcpState(&hSocketState->tcpState);

    return 0;
}

int aspSimulator_Start(SocketStateHandle hSocketState, char *initialData, int initialDataLen)
{
    int rc;
    int aspSocketFd;
    struct iphdr ip_hdr;
    struct tcphdr tcp_hdr;
    int one = 1;
    const int *val = &one;
    int totalbytes;
    int packet_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + 2048;
    char packet[packet_len];

    gAspTcpState = hSocketState->tcpState;
    printf("%s: Create a raw socket to simulate ASP and send the response: ini len %d\n", __FUNCTION__, initialDataLen);
    printf("%s: initial data len %d: \n%s\n", __FUNCTION__, initialDataLen, initialData);
    aspSocketFd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
    CHECK_ERR_LEZ_GOTO("Failed to create asp raw socket: ", strerror(errno), aspSocketFd, error);

    myPrint("aspSimulator_Start(): Press any key to proceed with Sending Data via ASP Simulator!!!");
    getchar();

    memset(&ip_hdr, 0, sizeof(struct iphdr));
    memset(&tcp_hdr, 0, sizeof(struct tcphdr));
    totalbytes = sizeof(struct iphdr) + sizeof(struct tcphdr)+initialDataLen;

    ip_hdr.ihl = 5;
    ip_hdr.version = 4;
    ip_hdr.tos = 0;

    ip_hdr.tot_len = htons(totalbytes);
    ip_hdr.id = htons(12345);/*check if id has to be from the original socket*/
    ip_hdr.frag_off = 0x40;
    ip_hdr.ttl = 64;
    ip_hdr.protocol = IPPROTO_TCP;
    ip_hdr.check = 0;
    ip_hdr.saddr = hSocketState->localIpAddr.sin_addr.s_addr;
    ip_hdr.daddr = hSocketState->remoteIpAddr.sin_addr.s_addr;

    tcp_hdr.source = hSocketState->localIpAddr.sin_port;
    tcp_hdr.dest = hSocketState->remoteIpAddr.sin_port;
    tcp_hdr.seq = ntohl(hSocketState->tcpState.seq);
    tcp_hdr.ack_seq = ntohl(hSocketState->tcpState.ack);
    tcp_hdr.res1 = 0;
    tcp_hdr.doff = sizeof(struct tcphdr)/4;
    tcp_hdr.fin = 0;
    tcp_hdr.syn = 0;
    tcp_hdr.rst = 0;
    tcp_hdr.psh = 0;
    tcp_hdr.ack = 1;
    tcp_hdr.urg = 0;
    tcp_hdr.window = htons(114);
    tcp_hdr.check = 0;
    tcp_hdr.urg_ptr = htons(0);

    ip_hdr.check = checksum((unsigned short int *) &ip_hdr, sizeof(struct iphdr));
    tcp_hdr.check = tcp4_checksum(ip_hdr, tcp_hdr, (unsigned char *)initialData, initialDataLen);

    //printf("checksum %x\n",tcp_hdr.check);

    memset(packet,0,packet_len);
    memcpy(packet,&ip_hdr,sizeof(struct iphdr));
    memcpy((packet+sizeof(struct iphdr)),&tcp_hdr,sizeof(struct tcphdr));
    memcpy((packet+sizeof(struct iphdr)+sizeof(struct tcphdr)), initialData, initialDataLen);

    rc = setsockopt(aspSocketFd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one));
    CHECK_ERR_NZ_GOTO("setsocketopt failed to set IP_HDRINCL ...", rc, error);

    printf("%s: sending to %s:%d ", __FUNCTION__, inet_ntoa(hSocketState->remoteIpAddr.sin_addr), ntohs(hSocketState->remoteIpAddr.sin_port));
    printf("from %s:%d \n", inet_ntoa(hSocketState->localIpAddr.sin_addr), ntohs(hSocketState->localIpAddr.sin_port));

    rc = sendto(aspSocketFd, packet, totalbytes, 0, (struct sockaddr *)&hSocketState->remoteIpAddr, sizeof(hSocketState->remoteIpAddr));
    CHECK_ERR_LEZ_GOTO("sendto failed ...", strerror(errno), rc, error);

    printf("%s: Sent %d bytes of initial Data \n", __FUNCTION__, initialDataLen);

    aspSimulator_setTcpState(&gAspTcpState, initialDataLen);

    printf("%s: Done\n", __FUNCTION__);
    return 0;

error:
    return -1;
}
