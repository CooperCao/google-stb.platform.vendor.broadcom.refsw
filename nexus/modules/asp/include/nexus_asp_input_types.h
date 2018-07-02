/***************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_ASP_INPUT_TYPES_H
#define NEXUS_ASP_INPUT_TYPES_H

#include "nexus_types.h"
#include "nexus_asp_types.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Declarations of various types used by AspInput Interface.
**/

typedef struct NEXUS_AspInput *NEXUS_AspInputHandle;

/**
Summary:
NEXUS_AspInput_Create related settings.
**/
typedef struct NEXUS_AspInputCreateBufferSettings
{
    unsigned                                size;                   /* Buffer size in bytes. */
    NEXUS_HeapHandle                        heap;                   /* Optional heap handle for buffer allocation. */
    NEXUS_MemoryBlockHandle                 memory;                 /* Optional external allocation of Buffer. Set 'size' above to reflect its size. */
    unsigned                                memoryOffset;           /* offset into 'memory' block. 'size' should be counted from the start of the memoryOffset, not the memory block size. */
} NEXUS_AspInputCreateBufferSettings;

/**
Summary:
NEXUS_AspInput_Create settings.
**/
typedef struct NEXUS_AspInputCreateSettings
{
    NEXUS_AspInputCreateBufferSettings     writeFifo;              /* FIFO for storing the outgoing HTTP Request provided by the host for transmission over the network */
                                                                   /* Default size is 4K, app can increase it if it wants AspInput to send a larger chunk of data to the network. */
                                                                   /* This FIFO memory must be accessible by host user space (for caller to fill-in data to be sent out) & by ASP Device. */

    NEXUS_AspInputCreateBufferSettings     reassembledFifo;        /* FIFO where reassembled frames provided by the host via the NEXUS_AspInput_GetReassembledWriteBuffer() are written for firmware to continue processing. */
                                                                   /* Default size is 16K, caller can increase it if it expects to feed larger size of reassembled IP Frames to the firmware. */
                                                                   /* This FIFO memory must be accessible by host user space (by caller to fill-in reassembled frames) & by ASP Device. */

    NEXUS_AspInputCreateBufferSettings     reTransmitFifo;         /* FIFO ASP uses to buffer data sent on the network until ACKed by the remote side. */
                                                                   /* Default size is 1MB, caller can increase it if it wants to have larger TCP retransmit queue. */
                                                                   /* This FIFO memory must be accessible by ASP Device only. */

    NEXUS_AspInputCreateBufferSettings     receiveFifo;            /* FIFO firmware uses to receive the incoming data from the network. */
                                                                   /* Default size is 1MB, caller can increase it if it wants to have larger receive queue. */                                                                   /* When AspChannel is being used for HTTP & Streaming hasn't been started, this buffer will hold the incoming HTTP Response */
                                                                   /* dataReadyCallback will be issued to notify caller about this buffered up data. */
                                                                   /* This FIFO memory must be accessible by host user space (by caller to read the received payload)) & by ASP Device. */

    NEXUS_AspInputCreateBufferSettings     miscBuffer;             /* Misc buffer used by FW for sending control packets. Default size is 4K. */
                                                                   /* This memory must be accessible by ASP Device only. */

    NEXUS_AspInputCreateBufferSettings     m2mDmaDescBuffer;       /* Buffer used by ASP FW to allocate XPT's M2M DMA buffer for Stream-in usages. */
                                                                   /* This memory must be accessible by ASP Device only. */
} NEXUS_AspInputCreateSettings;

/**
Summary:
NEXUS_AspInput_Get/_Set related settings.
**/
typedef struct NEXUS_AspInputSettings
{
    NEXUS_CallbackDesc                      endOfStreaming;         /* Invoked when FIN/RST is received, RTO/keepalive timeout happens. */
                                                                    /* Call NEXUS_AspInput_GetStatus() to find out the specific event causing this callback. */

    NEXUS_CallbackDesc                      bufferReady;            /* Invoked when there is buffer to be gotten by the app using the NEXUS_AspInput_GetBufferWithWrap() API. */
                                                                    /* Only applies if NEXUS_AspInputStartSettings.feedMode == NEXUS_AspInputFeedMode_eHost || */
                                                                    /*              if NEXUS_AspInputStartSettings.feedMode == NEXUS_AspInputFeedMode_eAutoWithHostEncrytion. */

    NEXUS_CallbackDesc                      httpResponseDataReady;  /* Invoked when some data belonging to a HTTP Response is received from network. */
                                                                    /* The app can receive this data using NEXUS_AspOuput_GetHttpResponseData(). */
} NEXUS_AspInputSettings;

/**
Summary:
DRMs supported by AspInput.
**/
typedef enum NEXUS_AspInputDrmType
{
    NEXUS_AspInputDrmType_eNone,
    NEXUS_AspInputDrmType_eDtcpIp,
    NEXUS_AspInputDrmType_eMax
} NEXUS_AspInputDrmType;

typedef struct NEXUS_AspInputDtcpIpSettings
{
#define NEXUS_ASP_DTCP_IP_EXCH_KEY_IN_BYTES 12
    uint8_t                                 exchKey[NEXUS_ASP_DTCP_IP_EXCH_KEY_IN_BYTES]; /* This key is in clear. ASP FW derives content key using Exchange key, EMI, and Nonce value. */
    unsigned                                exchKeyLabel;
    unsigned                                emiModes;
#define NEXUS_ASP_DTCP_IP_NONCE_IN_BYTES 8
    uint8_t                                 initialNonce[NEXUS_ASP_DTCP_IP_NONCE_IN_BYTES];
    unsigned                                pcpPayloadSize;
} NEXUS_AspInputDtcpIpSettings;

/**
Summary:
NEXUS_AspInput_Connect* related settings.
**/
typedef struct NEXUS_AspInputConnectHttpSettings
{
    NEXUS_TransportType                     transportType;
    NEXUS_PlaypumpHandle                    hPlaypump;
    NEXUS_DmaHandle                         hDma;
} NEXUS_AspInputConnectHttpSettings;

/**
Summary:
NEXUS_AspInput_Start related settings.
**/
typedef enum NEXUS_AspInputFeedMode
{
    NEXUS_AspInputFeedMode_eAuto,                     /* Incoming data is automatically fed to transport.  */
    NEXUS_AspInputFeedMode_eHost,                     /* Incoming data is consumed by the host. It gets a buffer of received data using NEXUS_AspInput_GetBufferWithWrap() API, */
                                                      /* processes it, & then releases it using the NEXUS_AspInput_BufferComplete() API. */
    NEXUS_AspInputFeedMode_eAutoWithHostDecryption,   /* Incoming data is decrypted by host.  It gets a buffer of received data using  NEXUS_AspInput_GetBufferWithWrap() API, */
                                                      /* then decrypts it, & then submits it back by using the NEXUS_AspInput_BufferComplete() API. */
    NEXUS_AspInputFeedMode_eMax
} NEXUS_AspInputFeedMode;

typedef struct NEXUS_AspInputStartSettings
{
    NEXUS_AspInputFeedMode          feedMode;
} NEXUS_AspInputStartSettings;

typedef enum NEXUS_AspInputState
{
    NEXUS_AspInputState_eIdle,                        /* enters on _AspInput_Create() or after _AspInput_Disconnect(). */
    NEXUS_AspInputState_eConnected,                   /* enters on _AspInput_Connect*(), after _AspInput_Stop(). */
    NEXUS_AspInputState_eStreaming,                   /* enters on _AspInput_Start(). */
    NEXUS_AspInputState_eMax
} NEXUS_AspInputState;

/**
Summary:
NEXUS_AspInput_GetStatus.
**/
typedef struct NEXUS_AspInputStatus
{
    int                     aspChannelIndex;
    NEXUS_AspInputState     state;

    bool                    remoteDoneSending;        /* When network peer has closed its connection by sending a TCP FIN. */
                                                      /* AspInput will issue endOfStreaming callback.  */
                                                      /* AspInput state will remain in _eStreaming state until app calls AspInput_Stop API. */

    bool                    connectionReset;          /* When network peer has sent a TCP RST. */
                                                      /* AspInput will issue endOfStreaming callback.  */
                                                      /* AspInput state will remain in _eStreaming state until app calls AspInput_Stop API. */

    bool                    networkTimeout;           /* When ASP exceeds the transmission retries and can't send anymore data (HTTP Request or keepalive probes, if enabled) to remote. */
                                                      /* AspInput will issue endOfStreaming callback.  */
                                                      /* AspInput state will remain in _eStreaming state until app calls AspInput_Stop API. */

} NEXUS_AspInputStatus;

typedef struct NEXUS_AspInputTcpStatus
{
    /* TODO: Clean out these for stream in. */
    bool statsValid;
    unsigned raveCtxDepthInBytes;
    unsigned raveCtxSizeInBytes;
    uint64_t mcpbConsumedInBytes;
    uint64_t mcpbConsumedInTsPkts;
    uint64_t mcpbConsumedInIpPkts;
    uint64_t xptMcpbConsumedInBytes;
    uint64_t xptMcpbConsumedInTsPkts;
    uint64_t xptMcpbConsumedInIpPkts;
    unsigned ePktConsumedInBytes;
    unsigned unimacConsumedInUnicastIpPkts;     /* IP packets sent to the switch (from ASP EPKT). */
    unsigned unimacConsumedInUnicastIpBytes;
    unsigned unimacTxUnicastIpPkts;             /* IP packets sent to the switch (from ASP EPKT). */
    unsigned unimacTxUnicastIpBytes;
    unsigned nwSwRxFmAspInBytes;
    unsigned nwSwRxFmAspInUnicastIpPkts;
    unsigned nwSwTxToAspInUnicastIpPkts;
    unsigned nwSwTxToP0InUnicastIpPkts;
    unsigned nwSwTxToHostInUnicastIpPkts;
    unsigned nwSwRxP0InUnicastIpPkts;
    unsigned nwSwRxP8InUnicastIpPkts;
    unsigned unimacRxUnicastIpPkts;             /* IP packets received from the Switch. */
    unsigned nwSwRxP0InDiscards;
    unsigned eDpktRxIpPkts;
    unsigned eDpktPendingPkts;                  /* Packets which are received by EDPKT but not yet pushed to FW. */
    unsigned mcpbSendWindow;                    /* Current Send Window: how many bytes of data ASP can send before waiting for TCP ACK. */
    bool     mcpbChEnabled;
    bool     mcpbDescFifoEmpty;                 /* Set if FW has currently no descriptors programmed for ASP Pipe. */
    unsigned mcpbPendingBufferDepth;            /* Instantaneous Buffer Depth in bytes: # of bytes committed by FW to MCPB which haven't yet been read by MCPB. */
    bool     mcpbAvPaused;                      /* Set if AV is paused for this channel. AV is paused when pkts are retransmitted and then cleared. */
    bool     mcpbStalled;                       /* Set if MCPB gets stalled during streaming, indicated by ASP_MCPB_CORE_ASP_MCPB_DEBUG_14: bit10 */
    struct
    {
        unsigned congestionWindow;
        unsigned receiveWindow;
        unsigned sendWindow;                    /* minimum of congestion & receive windows. */
        unsigned sendSequenceNumber;
        unsigned rcvdAckNumber;
        uint64_t pktsSent;
        uint64_t pktsRcvd;
        unsigned pktsRetx;                      /* total retransmitted IP packets. */
        unsigned retxSequenceNumber;
        unsigned pktsDropped;
        unsigned dataPktsDropped;
        unsigned rcvdSequenceNumber;
        unsigned descriptorsFedToXpt;
        unsigned bytesFedToXpt;
        unsigned outOfOrderPacketsRcvd;         /* total out-of-order packets received. */
        unsigned outOfOrderEventsRcvd;          /* total out-of-order events received. */
        unsigned curOutOfOrderEventsRcvd;       /* current out-of-order events received. */
    } fwStats;
} NEXUS_AspInputTcpStatus;

typedef struct NEXUS_AspInputHttpStatus
{
    NEXUS_AspInputTcpStatus     tcp;
    unsigned                    httpRequestsSent;
    unsigned                    httpResponsesReceived;
} NEXUS_AspInputHttpStatus;

typedef struct NEXUS_AspInputDisconnectStatus
{
    struct
    {
        unsigned  finalSendSequenceNumber;
        unsigned  finalRecvSequenceNumber;
    } tcpState;
} NEXUS_AspInputDisconnectStatus;
#ifdef __cplusplus
}
#endif

#endif
