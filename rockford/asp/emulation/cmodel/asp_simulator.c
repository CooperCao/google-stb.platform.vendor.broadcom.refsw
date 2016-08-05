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
#include <sys/types.h>
#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "netinet/ip.h"
#include "linux/tcp.h"
/*#include "linux/in.h"*/

#include "asp_utils.h"

#include "asp_proxy_server_api.h" /* TODO: Later this will be removed now it is added to access SocketState definition.*/
#include "basp_aspsim_api.h"
#include "asp_xpt_api.h"

extern BASP_P_MessageFifo  gAspMessageFifo;
extern ASP_XptMcpb_BuffInfo    gXptMcpbBuffInfo;

#define ASP_RAW_FRAME_BUFFER_SIZE   4096


typedef enum ASP_Simulator_ChannelType
{
    ASP_Simulator_ChannelType_eStreamOut,
    ASP_Simulator_ChannelType_eStreamIn,
}ASP_Simulator_ChannelType;

typedef struct ASP_Simulator_Channel
{
    bool                        channelStarted;
    ASP_Simulator_ChannelType   channelType;

    int                         channelIndex;
    int                         rawFrameBufferSize;
    char                        *pRawframeBuffer;

    SocketState                 sAspSimulatorConnectionState;
} ASP_Simulator_Channel;

ASP_Simulator_Channel   *gpSimulatorChannel = NULL;/* this will only work for a single channel mode */

static ASP_Simulator_Channel *createAspSimulatorChannel()
{
    ASP_Simulator_Channel   *pSimulatorChannel = NULL;

    pSimulatorChannel = calloc(1, sizeof(ASP_Simulator_Channel));

    pSimulatorChannel->rawFrameBufferSize = ASP_RAW_FRAME_BUFFER_SIZE;
    pSimulatorChannel->pRawframeBuffer = calloc(1,ASP_RAW_FRAME_BUFFER_SIZE );

    return pSimulatorChannel;
}

static void destroyAspSimulatorChannel(
    ASP_Simulator_Channel *pSimulatorChannel
    )
{
    if(pSimulatorChannel)
    {
        if(pSimulatorChannel->pRawframeBuffer)
        {
            free(pSimulatorChannel->pRawframeBuffer);
        }
        free(pSimulatorChannel);
        pSimulatorChannel = NULL;
    }
}

static void BASP_SendMessage2Pi(
    BASP_P_MessageHeader *pMsgHeader,
    void *pMessagePayload,
    int payLoadSize,
    BASP_P_MessageFifo  *pMsgFifo
    )
{
    /* write start message to the Pi2Fw fifo.*/
    int msgHeaderSize = sizeof(BASP_P_MessageHeader);
    memcpy(pMsgFifo->fw2PiMessage, pMsgHeader, msgHeaderSize);

    memcpy((pMsgFifo->fw2PiMessage+msgHeaderSize), pMessagePayload, payLoadSize);
    pMsgFifo->fw2PiMessageAvailable = true;
}


static void sendStartResponse(
    ASP_Simulator_Channel *pAspSimChannel,
    BASP_P_MessageFifo *pAspMessageFifo ,
    ASP_Simulator_ChannelType channelType,
    int success
    )
{
    BASP_P_MessageHeader sMsgHeader;
    BASP_P_ChannelStartResponse sStartResponse;
    int headerSize = sizeof(BASP_P_MessageHeader);
    sMsgHeader.channelIndex = pAspSimChannel->channelIndex;
    if(ASP_Simulator_ChannelType_eStreamOut == channelType)
    {
        sMsgHeader.messageType = BASP_P_MessageType_FW2PI_eChannelStartStreamOutResponse;
    }
    else
    {
        sMsgHeader.messageType = BASP_P_MessageType_FW2PI_eChannelStartStreamInResponse;
    }

    sStartResponse.success = success;

    BASP_SendMessage2Pi(&sMsgHeader, &sStartResponse, sizeof(BASP_P_ChannelStartResponse),pAspMessageFifo);
}
static void sendStopResponse(
    ASP_Simulator_Channel *pAspSimChannel,
    BASP_P_MessageFifo *pAspMessageFifo ,
    int success
    )
{
    BASP_P_MessageHeader        sMsgHeader;
    BASP_P_ChannelStopResponse sStopResponse;

    int headerSize = sizeof(BASP_P_MessageHeader);

    sMsgHeader.channelIndex = pAspSimChannel->channelIndex;
    sMsgHeader.messageType = BASP_P_MessageType_FW2PI_eChannelStopResponse;

    sStopResponse.success = success;
    sStopResponse.sSocketState = pAspSimChannel->sAspSimulatorConnectionState;

    BASP_SendMessage2Pi(&sMsgHeader, &sStopResponse, sizeof(BASP_P_ChannelStopResponse),pAspMessageFifo);
}


int Asp_Simulator_ProcessMessages( void )
{
    BASP_P_MessageHeader sMsgHeader;
    int headerSize = sizeof(BASP_P_MessageHeader);
    ASP_Simulator_Channel   *pSimulatorChannel = NULL;
    BASP_P_ChannelStartMessage  sStartMessage;
    BASP_P_ChannelStopMessage   sStopMessage;
    BASP_P_MessageFifo *pAspMessageFifo = &gAspMessageFifo;

    /* Check first whether the Pi2Fw message fifo has any message. */
    if(pAspMessageFifo->pi2FwMessageAvailable == true)
    {
        memcpy(&sMsgHeader, pAspMessageFifo->pi2FwMessage, headerSize );

        fprintf(stdout,"\n%s: received Pi2Fw message of type=%d ========================\n", __FUNCTION__, sMsgHeader.messageType);
        switch(sMsgHeader.messageType)
        {
        case BASP_P_MessageType_PI2FW_eChannelStartStreamOut:
            pSimulatorChannel = createAspSimulatorChannel();
            pSimulatorChannel->channelType = ASP_Simulator_ChannelType_eStreamOut;
            pSimulatorChannel->channelIndex = sMsgHeader.channelIndex;

            /* Read the actual message */
            memcpy(&sStartMessage, (pAspMessageFifo->pi2FwMessage+headerSize), sizeof(BASP_P_ChannelStartMessage) );
            pAspMessageFifo->pi2FwMessageAvailable =  false; /* This indicates that the message has been consumed.*/

            pSimulatorChannel->sAspSimulatorConnectionState = sStartMessage.sSocketState;

            /* Send StreamOut Start response */
            sendStartResponse( pSimulatorChannel, pAspMessageFifo, pSimulatorChannel->channelType , 1);

            fprintf(stdout,"\n%s: Got BASP_P_MessageType_PI2FW_eChannelStartStreamOut ========================\n", __FUNCTION__);

            pSimulatorChannel->channelStarted = true;
            /* maintain the pointer in the global pointer */
            gpSimulatorChannel = pSimulatorChannel;
            break;

        case BASP_P_MessageType_PI2FW_eChannelStartStreamIn:
            pSimulatorChannel = createAspSimulatorChannel();
            pSimulatorChannel->channelType = ASP_Simulator_ChannelType_eStreamIn;

            /* Send StreamIn Start response */
            pSimulatorChannel->channelIndex = sMsgHeader.channelIndex;

            fprintf(stdout,"\n%s: Got BASP_P_MessageType_PI2FW_eChannelStartStreamIn ========================\n", __FUNCTION__);

            /* Read the actual message */
            memcpy(&sStartMessage, (pAspMessageFifo->pi2FwMessage+headerSize), sizeof(BASP_P_ChannelStartMessage) );
            pAspMessageFifo->pi2FwMessageAvailable =  false; /* This indicates that the message has been consumed.*/

            pSimulatorChannel->sAspSimulatorConnectionState = sStartMessage.sSocketState;

            /* Send StreamOut Start response */
            sendStartResponse( pSimulatorChannel, pAspMessageFifo, pSimulatorChannel->channelType , 1);

            pSimulatorChannel->channelStarted = true;

            break;

        case BASP_P_MessageType_PI2FW_eChannelStop:

            pSimulatorChannel = gpSimulatorChannel;
            memcpy(&sStopMessage, (pAspMessageFifo->pi2FwMessage+headerSize), sizeof(BASP_P_ChannelStopMessage) );

            pAspMessageFifo->pi2FwMessageAvailable =  false;
            pSimulatorChannel->channelStarted = false;

            /* Now send the response with latest socket state before detroying the asp channel object.*/
            sendStopResponse(pSimulatorChannel, pAspMessageFifo , 1);
            fprintf(stdout,"\n%s: Got BASP_P_MessageType_PI2FW_eChannelStop ========================\n", __FUNCTION__);

            /* Now destroy the channelObject*/
            destroyAspSimulatorChannel(pSimulatorChannel);

            /* set global pointer back to NULL */
            gpSimulatorChannel = NULL;

            break;
        }
    }
}

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
        (tcphdr.urg << 5) /* +
        (tcphdr.ece << 6) +
        (tcphdr.cwr << 7)*/;

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

static void aspSimulator_setTcpState(TcpState *pTcpState, int bytesSent)
{
    /* update ASP TCP state */
    pTcpState->seq += bytesSent;

    /* TODO: may need to update ACK number - */
}

void generateEthernetFrameFromPayload(
    ASP_Simulator_Channel *pAspSimChannel,
    char *pPlayload,
    int payloadSize
    )
{
    struct iphdr ip_hdr;
    struct tcphdr tcp_hdr;
    int totalbytes;
    SocketState *pSocketState = &pAspSimChannel->sAspSimulatorConnectionState;
    int packet_len = pAspSimChannel->rawFrameBufferSize;
    char *pFrameBuffer = pAspSimChannel->pRawframeBuffer;

    printf("%s: Create a raw socket to simulate ASP and send the raw frame for payload size:%d \n", __FUNCTION__, payloadSize);

    memset(&ip_hdr, 0, sizeof(struct iphdr));
    memset(&tcp_hdr, 0, sizeof(struct tcphdr));
    totalbytes = sizeof(struct iphdr) + sizeof(struct tcphdr)+payloadSize;

    ip_hdr.ihl = 5;
    ip_hdr.version = 4;
    ip_hdr.tos = 0;

    ip_hdr.tot_len = htons(totalbytes);
    ip_hdr.id = htons(12345);/*check if id has to be from the original socket*/
    ip_hdr.frag_off = 0x40;
    ip_hdr.ttl = 74;
    ip_hdr.protocol = IPPROTO_TCP;
    ip_hdr.check = 0;
    ip_hdr.saddr = pSocketState->localIpAddr.sin_addr.s_addr;
    ip_hdr.daddr = pSocketState->remoteIpAddr.sin_addr.s_addr;

    tcp_hdr.source = pSocketState->localIpAddr.sin_port;
    tcp_hdr.dest = pSocketState->remoteIpAddr.sin_port;

    tcp_hdr.seq = ntohl(pSocketState->tcpState.seq);
    tcp_hdr.ack_seq = ntohl(pSocketState->tcpState.ack);

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
    tcp_hdr.check = tcp4_checksum(ip_hdr, tcp_hdr, (unsigned char *)pPlayload, payloadSize);

    //printf("checksum %x\n",tcp_hdr.check);

    memset(pFrameBuffer,0,packet_len);
    memcpy(pFrameBuffer,&ip_hdr,sizeof(struct iphdr));
    memcpy((pFrameBuffer+sizeof(struct iphdr)),&tcp_hdr,sizeof(struct tcphdr));
    memcpy((pFrameBuffer+sizeof(struct iphdr)+sizeof(struct tcphdr)), pPlayload, payloadSize);

    printf("%s: ==========+++++++++++++++++++++> Payload |%s| and payload size = %d and pFrameBuffer|%s|",__FUNCTION__, pPlayload, payloadSize, pFrameBuffer);

    /* Now call ASP_NwSwFlow_FeedRawFrameFromAspSimulator(
    void *pRawFrame,
    size_t frameLength
    ) to send the raw frame to network.*/
    ASP_NwSwFlow_FeedRawFrameFromAspSimulator( pFrameBuffer,totalbytes);

    /* Now set the tcp state to update sequence number.*/
    aspSimulator_setTcpState(&pAspSimChannel->sAspSimulatorConnectionState.tcpState, payloadSize);
}

int Asp_Simulator_ProcessPayloadData( )
{
    if (gXptMcpbBuffInfo.payloadSize)
    {
        /* consume the payload and generate ethernet frame for it.*/
        generateEthernetFrameFromPayload(gpSimulatorChannel, gXptMcpbBuffInfo.xptMcpbBuffer, gXptMcpbBuffInfo.payloadSize );

        gXptMcpbBuffInfo.payloadSize = 0; /* Data consumed.*/
    }

}
