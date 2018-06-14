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
#ifndef B_ASP_OUTPUT_TYPES_H__
#define B_ASP_OUTPUT_TYPES_H__

#include "nexus_types.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Declarations of various types used by AspOutput Interface.
**/

/**
Summary:
B_AspOutput_Create related settings.
**/
typedef struct B_AspOutputCreateBufferSettings
{
    unsigned                                size;                   /* Buffer size in bytes. */
    NEXUS_HeapHandle                        heap;                   /* Optional heap handle for buffer allocation. */
    NEXUS_MemoryBlockHandle                 memory;                 /* Optional external allocation of Buffer. Set 'size' above to reflect its size. */
    unsigned                                memoryOffset;           /* offset into 'memory' block. 'size' should be counted from the start of the memoryOffset, not the memory block size. */
} B_AspOutputCreateBufferSettings;

/**
Summary:
B_AspOutput_Create settings.
**/
typedef struct B_AspOutputCreateSettings
{
    B_AspOutputCreateBufferSettings         writeFifo;              /* FIFO where AspOutput will buffer the HTTP Response proivided in the B_AspOutput_SendHttpResponse() API. */
                                                                    /* Default size is 4K, app can increase it if it wants AspOutput to send larger HTTP Response to the network. */

    B_AspOutputCreateBufferSettings         reTransmitFifo;         /* FIFO ASP uses to buffer data sent on the network until ACKed by the remote side. */
                                                                    /* Default size is 1MB, caller can increase it if it wants to have larger TCP retransmit queue. */
                                                                    /* This FIFO memory will need to be only ASP Device accessible. */

    B_AspOutputCreateBufferSettings         receiveFifo;            /* FIFO ASP uses to receive the incoming data from the network. */
                                                                    /* Default size is 4KB, caller can increase it if it wants to have larger TCP receive queue. */
                                                                    /* When AspOutput is being used for HTTP & Streaming hasn't been started, this buffer can hold up the subsequent HTTP Request. */
                                                                    /* requestDataReady callback will be issued to notify caller about this buffered-up data. */
                                                                    /* This FIFO memory will need to be host user space accessible (by caller to read the received payload) & by ASP Device accessible also. */

    B_AspOutputCreateBufferSettings         miscBuffer;             /* Misc buffer used by FW for sending control packets. Default size is 4K. */
                                                                    /* This memory will need to be only ASP Device accessible. */
} B_AspOutputCreateSettings;

/**
Summary:
B_AspOutput_Get/_Set related settings.
**/
typedef struct B_AspOutputSettings
{
    NEXUS_CallbackDesc                      endOfStreaming;         /* Invoked when RST is received, RTO timeout happens, or work completes for _Finish(). */
                                                                    /* Call B_AspOutput_GetStatus() to find out the specific event causing this callback. */

    NEXUS_CallbackDesc                      remoteDoneSending;      /* Invoked when remote has closed its sending side of the connection (using shutdown() API.). */
                                                                    /* This is an informational type callback and streaming will continue. */

    NEXUS_CallbackDesc                      bufferReady;            /* Invoked when there is buffer to be gotten by the app using the B_AspOutput_GetBufferWithWrap() API. */
                                                                    /* Only applies if B_AspOutputStartSettings.feedMode == B_AspOutputFeedMode_eHost || */
                                                                    /*              if B_AspOutputStartSettings.feedMode == B_AspOutputFeedMode_eAutoWithHostEncrytion. */

    NEXUS_CallbackDesc                      httpRequestDataReady;   /* Invoked when some data belonging to a HTTP Request is received from network. */
                                                                    /* The app can receive this data using B_AspOuput_GetHttpRequestData(). */
} B_AspOutputSettings;

/**
Summary:
DRMs supported by AspOutput.
**/
typedef enum B_AspOutputDrmType
{
    B_AspOutputDrmType_eNone,
    B_AspOutputDrmType_eDtcpIp,
    B_AspOutputDrmType_eMax
} B_AspOutputDrmType;

typedef struct B_AspOutputDtcpIpSettings
{
#define B_ASP_DTCP_IP_EXCH_KEY_IN_BYTES 12
    uint8_t                                 exchKey[B_ASP_DTCP_IP_EXCH_KEY_IN_BYTES]; /* This key is in clear. ASP FW derives content key using Exchange key, EMI, and Nonce value. */
    unsigned                                exchKeyLabel;
    unsigned                                emiModes;
#define B_ASP_DTCP_IP_NONCE_IN_BYTES 8
    uint8_t                                 initialNonce[B_ASP_DTCP_IP_NONCE_IN_BYTES];
    unsigned                                pcpPayloadSize;
} B_AspOutputDtcpIpSettings;

/**
Summary:
B_AspOutput_Connect* related settings.
**/
typedef struct B_AspOutputConnectHttpSettings
{
    NEXUS_TransportType                     transportType;
    NEXUS_RecpumpHandle                     hRecpump;               /* Recpump from where to output the AV data. */
                                                                    /* Only applies if B_AspOutputStartSettings.feedMode == B_AspOutputFeedMode_eAuto || */
                                                                    /*              if B_AspOutputStartSettings.feedMode == B_AspOutputFeedMode_eAutoWithHostEncrytion. */
    NEXUS_PlaypumpHandle                    hPlaypump;              /* Optional: Playpump where Host is feeding data into. Needed for debug purposes only. */
    unsigned                                maxBitRate;             /* Maximum rate at which ASP will send out the AV payloads. */
    /* HTTP Specific Settings. */
    struct {
        unsigned                            major;
        unsigned                            minor;
    } version;
    bool                                    enableChunkTransferEncoding;
    unsigned                                chunkSize;
} B_AspOutputConnectHttpSettings;

/**
Summary:
B_AspOutput_Start related settings.
**/
typedef enum B_AspOutputFeedMode
{
    B_AspOutputFeedMode_eAuto,                                      /* Data is automatically available in Recpump for ASP to output from.  */
    B_AspOutputFeedMode_eHost,                                      /* Data is fed by host. It gets a buffer using B_AspOutput_GetBufferWithWrap() API, */
                                                                    /* copies data into it, & then submits the buffer back using the B_AspOutput_BufferSubmit() API. */
    B_AspOutputFeedMode_eAutoWithHostEncryption,                    /* Data is available in Recpump but host encrypts it before letting ASP send it out on the network. */
                                                                    /* Host accesses buffer using the B_AspOutput_GetBufferWithWrap() API and submits it back using B_AspOutput_BufferSubmit() API. */
    B_AspOutputFeedMode_eMax
} B_AspOutputFeedMode;

typedef struct B_AspOutputStartSettings
{
    B_AspOutputFeedMode                     feedMode;
} B_AspOutputStartSettings;


typedef enum B_AspOutputState
{
    B_AspOutputState_eIdle,                                         /* Enters on _AspOutput_Create() or after _AspOutput_Disconnect(). */
    B_AspOutputState_eConnected,                                    /* Enters on _AspOutput_Connect*(), after _AspOutput_Stop(), or when _AspOutput_Finish() completes. */
    B_AspOutputState_eStreaming,                                    /* Enters on _AspOutput_Start(). */
    B_AspOutputState_eMax
} B_AspOutputState;

/**
Summary:
B_AspOutput_GetStatus related.

**/
typedef struct B_AspOutputStatus
{
    B_AspOutputState        state;
    int                     aspChannelIndex;
    uint64_t                bytesStreamed;                          /* number of bytes streamed out. */

    bool                    finishRequested;                        /* Gets set when app has called _AspOutput_Finish() and AspOutput is working on finish completion. */
                                                                    /* AspOutput will set finishCompleted flag & issue endOfStreaming callback to reflect this completion.  */

    bool                    finishCompleted;                        /* Gets set previously started finish sequence completes. */
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
                                                                    /* AspOutput state will remain in _eStreaming state. */

} B_AspOutputStatus;

typedef struct B_AspOutputTcpStatus
{
    bool                                    statsValid;
    struct
    {
        unsigned raveCtxDepthInBytes;
        unsigned raveCtxSizeInBytes;
        uint64_t mcpbConsumedInBytes;
        uint64_t mcpbConsumedInTsPkts;
        uint64_t mcpbConsumedInIpPkts;
        unsigned ePktConsumedInBytes;
        unsigned unimacConsumedInUnicastIpPkts;                     /* IP packets sent to the switch (from ASP EPKT). */
        unsigned unimacConsumedInUnicastIpBytes;
        unsigned unimacTxUnicastIpPkts;                             /* IP packets sent to the switch (from ASP EPKT). */
        unsigned unimacTxUnicastIpBytes;
        unsigned nwSwRxFmAspInBytes;
        unsigned nwSwRxFmAspInUnicastIpPkts;
        unsigned nwSwTxToAspInUnicastIpPkts;
        unsigned nwSwTxToP0InUnicastIpPkts;
        unsigned nwSwTxToHostInUnicastIpPkts;
        unsigned nwSwRxP0InUnicastIpPkts;
        unsigned nwSwRxP8InUnicastIpPkts;
        unsigned unimacRxUnicastIpPkts;                             /* IP packets received from the Switch. */
        unsigned nwSwRxP0InDiscards;
        unsigned eDpktRxIpPkts;
        unsigned eDpktPendingPkts;                                  /* Packets which are received by EDPKT but not yet pushed to FW. */
        unsigned mcpbSendWindow;                                    /* Current Send Window: how many bytes of data ASP can send before waiting for TCP ACK. */
        bool     mcpbChEnabled;
        bool     mcpbDescFifoEmpty;                                 /* Set if FW has currently no descriptors programmed for ASP Pipe. */
        unsigned mcpbPendingBufferDepth;                            /* Instantaneous Buffer Depth in bytes: # of bytes committed by FW to MCPB which haven't yet been read by MCPB. */
        bool     mcpbAvPaused;                                      /* Set if AV is paused for this channel. AV is paused when pkts are retransmitted and then cleared. */
        bool     mcpbStalled;                                       /* Set if MCPB gets stalled during streaming, indicated by ASP_MCPB_CORE_ASP_MCPB_DEBUG_14: bit10 */
        struct
        {
            unsigned congestionWindow;
            unsigned receiveWindow;
            unsigned sendWindow;                                    /* minimum of congestion & receive windows. */
            unsigned sendSequenceNumber;
            unsigned rcvdAckNumber;
            uint64_t pktsSent;
            uint64_t pktsRcvd;
            unsigned pktsRetx;                                      /* total retransmitted IP packets. */
            unsigned retxSequenceNumber;
            unsigned pktsDropped;
            unsigned dataPktsDropped;
            unsigned rcvdSequenceNumber;
        } fwStats;
    } stats;
} B_AspOutputTcpStatus;

typedef struct B_AspOutputDisconnectStatus
{
    int *pSocketFd;
} B_AspOutputDisconnectStatus;
#ifdef __cplusplus
}
#endif

#endif
