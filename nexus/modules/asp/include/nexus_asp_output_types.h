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
#ifndef NEXUS_ASP_OUTPUT_TYPES_H
#define NEXUS_ASP_OUTPUT_TYPES_H

#include "nexus_types.h"
#include "nexus_asp_types.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Declarations of various types used by AspOutput Interface.
**/

typedef struct NEXUS_AspOutput *NEXUS_AspOutputHandle;

/**
Summary:
NEXUS_AspOutput_Create related settings.
**/
typedef struct NEXUS_AspOutputCreateBufferSettings
{
    unsigned                                size;                   /* Buffer size in bytes. */
    NEXUS_HeapHandle                        heap;                   /* Optional heap handle for buffer allocation. */
    NEXUS_MemoryBlockHandle                 memory;                 /* Optional external allocation of Buffer. Set 'size' above to reflect its size. */
    unsigned                                memoryOffset;           /* offset into 'memory' block. 'size' should be counted from the start of the memoryOffset, not the memory block size. */
} NEXUS_AspOutputCreateBufferSettings;

/**
Summary:
NEXUS_AspOutput_Create settings.
**/
typedef struct NEXUS_AspOutputCreateSettings
{
    NEXUS_AspOutputCreateBufferSettings     writeFifo;              /* FIFO where AspOutput will buffer the HTTP Response proivided in the NEXUS_AspOutput_SendHttpResponse() API. */
                                                                    /* Default size is 4K, app can increase it if it wants AspOutput to send larger HTTP Response to the network. */
                                                                    /* This FIFO memory must be accessible by host user space (by caller to fill-in data to be sent out) & by ASP Device. */

    NEXUS_AspOutputCreateBufferSettings     reTransmitFifo;         /* FIFO ASP uses to buffer data sent on the network until ACKed by the remote side. */
                                                                    /* Default size is 1MB, caller can increase it if it wants to have larger TCP retransmit queue. */
                                                                    /* This FIFO memory must be accessible by ASP Device only. */

    NEXUS_AspOutputCreateBufferSettings     receiveFifo;            /* FIFO ASP uses to receive the incoming data from the network. */
                                                                    /* Default size is 4KB, caller can increase it if it wants to have larger TCP receive queue. */
                                                                    /* When AspOutput is being used for HTTP & Streaming hasn't been started, this buffer can hold the subsequent HTTP Request. */
                                                                    /* requestDataReady callback will be issued to notify caller about this buffered-up data. */
                                                                    /* This FIFO memory must be accessible by host user space (by caller to read the received payload)) & by ASP Device. */

    NEXUS_AspOutputCreateBufferSettings     miscBuffer;             /* Misc buffer used by FW for sending control packets. Default size is 4K. */
                                                                    /* This memory must be accessible by ASP Device only. */
} NEXUS_AspOutputCreateSettings;

/**
Summary:
NEXUS_AspOutput_Get/_Set related settings.
**/
typedef struct NEXUS_AspOutputSettings
{
    NEXUS_CallbackDesc                      endOfStreaming;         /* Invoked when RST is received, RTO timeout happens, or work completes for _Finish(). */
                                                                    /* Call NEXUS_AspOutput_GetStatus() to find out the specific event causing this callback. */

    NEXUS_CallbackDesc                      remoteDoneSending;      /* Invoked when remote has closed its sending side of the connection (using shutdown() API.). */
                                                                    /* This is an informational type callback and streaming will continue. */

    NEXUS_CallbackDesc                      bufferReady;            /* Invoked when there is buffer to be gotten by the app using the NEXUS_AspOutput_GetBufferWithWrap() API. */
                                                                    /* Only applies if NEXUS_AspOutputStartSettings.feedMode == NEXUS_AspOutputFeedMode_eHost || */
                                                                    /*              if NEXUS_AspOutputStartSettings.feedMode == NEXUS_AspOutputFeedMode_eAutoWithHostEncrytion. */

    NEXUS_CallbackDesc                      httpRequestDataReady;   /* Invoked when some data belonging to a HTTP Request is received from network. */
                                                                    /* The app can receive this data using NEXUS_AspOuput_GetHttpRequestData(). */
} NEXUS_AspOutputSettings;

/**
Summary:
DRMs supported by AspOutput.
**/
typedef enum NEXUS_AspOutputDrmType
{
    NEXUS_AspOutputDrmType_eNone,
    NEXUS_AspOutputDrmType_eDtcpIp,
    NEXUS_AspOutputDrmType_eMax
} NEXUS_AspOutputDrmType;

typedef struct NEXUS_AspOutputDtcpIpSettings
{
#define NEXUS_ASP_DTCP_IP_EXCH_KEY_IN_BYTES 12
    uint8_t                                 exchKey[NEXUS_ASP_DTCP_IP_EXCH_KEY_IN_BYTES]; /* This key is in clear. ASP FW derives content key using Exchange key, EMI, and Nonce value. */
    unsigned                                exchKeyLabel;
    unsigned                                emiModes;
#define NEXUS_ASP_DTCP_IP_NONCE_IN_BYTES 8
    uint8_t                                 initialNonce[NEXUS_ASP_DTCP_IP_NONCE_IN_BYTES];
    unsigned                                pcpPayloadSize;
} NEXUS_AspOutputDtcpIpSettings;

/**
Summary:
NEXUS_AspOutput_Connect* related settings.
**/
typedef struct NEXUS_AspOutputConnectHttpSettings
{
    NEXUS_TransportType                     transportType;
    unsigned                                maxBitRate;         /* Maximum rate at which ASP will send out the AV payloads. */
    NEXUS_RecpumpHandle                     hRecpump;

    /* HTTP Specific Settings. */
    struct {
        unsigned                            major;
        unsigned                            minor;
    } version;
    bool                                    enableChunkTransferEncoding;
    unsigned                                chunkSize;
} NEXUS_AspOutputConnectHttpSettings;

/**
Summary:
NEXUS_AspOutput_Start related settings.
**/
typedef enum NEXUS_AspOutputFeedMode
{
    NEXUS_AspOutputFeedMode_eAuto,                    /* Data is automatically available in Recpump for ASP to output from.  */
    NEXUS_AspOutputFeedMode_eHost,                    /* Data is fed by host. It gets a buffer using NEXUS_AspOutput_GetBufferWithWrap() API, */
                                                      /* copies data into it, & then submits the buffer back using the NEXUS_AspOutput_BufferSubmit() API. */
    NEXUS_AspOutputFeedMode_eAutoWithHostEncryption,  /* Data is available in Recpump but host encrypts it before letting ASP send it out on the network. */
                                                      /* Host accesses buffer using the NEXUS_AspOutput_GetBufferWithWrap() API and submits it back using NEXUS_AspOutput_BufferSubmit() API. */
    NEXUS_AspOutputFeedMode_eMax
} NEXUS_AspOutputFeedMode;

typedef struct NEXUS_AspOutputStartSettings
{
    NEXUS_AspOutputFeedMode                 feedMode;
} NEXUS_AspOutputStartSettings;

typedef enum NEXUS_AspOutputState
{
    NEXUS_AspOutputState_eIdle,                                             /* enters on _AspOutput_Create() or after _AspOutput_Disconnect(). */
    NEXUS_AspOutputState_eConnected,                                        /* enters on _AspOutput_Connect*(), after _AspOutput_Stop(), or when _AspOutput_Finish() completes. */
    NEXUS_AspOutputState_eStreaming,                                        /* enters on _AspOutput_Start(). */
    NEXUS_AspOutputState_eMax
} NEXUS_AspOutputState;

/**
Summary:
NEXUS_AspOutput_GetStatus.
**/
typedef struct NEXUS_AspOutputStatus
{
    int                     aspChannelIndex;
    NEXUS_AspOutputState    state;

    bool                    finishRequested;                        /* Gets set when app has called _AspOutput_Finish() and AspOutput is working on finish completion. */
                                                                    /* AspOutput will set finishCompleted flag & issue endOfStreaming callback to reflect this completion.  */

    bool                    finishCompleted;                        /* Gets set when previously started finish sequence completes. */
                                                                    /* AspOutput will issue endOfStreaming callback to reflect this completion.  */
                                                                    /* AspOutput state will transition back to Connected state. */

    bool                    connectionReset;                        /* When network peer has sent a TCP RST. */
                                                                    /* AspOutput will issue endOfStreaming callback.  */
                                                                    /* AspOutput state will remain in _eStreaming state until app calls AspOutput_Stop/AspOutput_Finish APIs. */

    bool                    networkTimeout;                         /* When ASP exceeds the transmission retries and can't send anymore data to remote. */
                                                                    /* AspOutput will issue endOfStreaming callback.  */
                                                                    /* AspOutput state will remain in _eStreaming state until app calls AspOutput_Stop/AspOutput_Finish APIs. */

    bool                    remoteDoneSending;                      /* When network peer has closed its half of connection by sending a TCP FIN. */
                                                                    /* AspOutput will issue remoteDoneStreaming callback.  */
                                                                    /* AspOutput state will remain in _eStreaming state, and streaming will continue. */
} NEXUS_AspOutputStatus;

typedef struct NEXUS_AspOutputTcpStatus
{
    bool statsValid;
    unsigned raveCtxDepthInBytes;
    unsigned raveCtxSizeInBytes;
    uint64_t mcpbConsumedInBytes;
    uint64_t mcpbConsumedInTsPkts;
    uint64_t mcpbConsumedInIpPkts;
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
    } fwStats;
} NEXUS_AspOutputTcpStatus;

typedef struct NEXUS_AspOutputHttpStatus
{
    NEXUS_AspOutputTcpStatus    tcp;
    unsigned                    httpRequestsReceived;
    unsigned                    httpResponsesSent;
} NEXUS_AspOutputHttpStatus;

typedef struct NEXUS_AspOutputDisconnectStatus
{
    struct
    {
        unsigned  finalSendSequenceNumber;
        unsigned  finalRecvSequenceNumber;
    } tcpState;
} NEXUS_AspOutputDisconnectStatus;
#ifdef __cplusplus
}
#endif

#endif
