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
#ifndef B_ASP_INPUT_TYPES_H__
#define B_ASP_INPUT_TYPES_H__

#include "nexus_types.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Declarations of various types used by AspInput Interface.
**/

/**
Summary:
B_AspInput_Create related settings.
**/
typedef struct B_AspInputCreateBufferSettings
{
    unsigned                                size;                   /* Buffer size in bytes. */
    NEXUS_HeapHandle                        heap;                   /* Optional heap handle for buffer allocation. */
    NEXUS_MemoryBlockHandle                 memory;                 /* Optional external allocation of Buffer. Set 'size' above to reflect its size. */
    unsigned                                memoryOffset;           /* offset into 'memory' block. 'size' should be counted from the start of the memoryOffset, not the memory block size. */
} B_AspInputCreateBufferSettings;

/**
Summary:
B_AspInput_Create settings.
**/
typedef struct B_AspInputCreateSettings
{
    B_AspInputCreateBufferSettings          writeFifo;              /* FIFO for storing the outgoing HTTP Request provided by the host for transmission over the network */
                                                                    /* Default size is 4K, app can increase it if it wants AspInput to send a larger chunk of data to the network. */
                                                                    /* This FIFO memory must be accessible by host user space (for caller to fill-in data to be sent out) & by ASP Device. */

    B_AspInputCreateBufferSettings          reassembledFifo;        /* FIFO where reassembled frames provided by the host via the B_AspInput_GetReassembledWriteBuffer() are written for firmware to continue processing. */
                                                                    /* Default size is 64K, caller can increase it if it expects to feed larger number of reassembled IP Frames to the firmware. */
                                                                    /* This FIFO memory must be accessible by host user space (by caller to fill-in reassembled frames) & by ASP Device. */

    B_AspInputCreateBufferSettings          reTransmitFifo;         /* FIFO ASP uses to buffer data sent on the network until ACKed by the remote side. */
                                                                    /* Default size is 8KB, caller can increase it if it wants to have larger TCP retransmit queue. */
                                                                    /* This FIFO memory must be accessible by ASP Device only. */

    B_AspInputCreateBufferSettings          receiveFifo;            /* FIFO ASP uses to receive the incoming data from the network. */
                                                                    /* Default size is 1MB, caller can increase it if it wants to have larger TCP receive queue. */
                                                                    /* dataReadyCallback will be issued to notify caller about this buffered up data. */
                                                                    /* This FIFO memory must be accessible by host user space (by caller to read the received payload)) & by ASP Device. */

    B_AspInputCreateBufferSettings          miscBuffer;             /* Misc buffer used by FW for sending control packets. Default size is 4K. */
                                                                    /* Default size is 4KB. */
                                                                    /* This memory must be accessible by ASP Device only. */

    B_AspInputCreateBufferSettings          m2mDmaDescBuffer;       /* Buffer used by ASP FW to allocate XPT's M2M DMA buffer for Stream-in usages. */
                                                                    /* Default size is 16KB. */
                                                                    /* This memory must be accessible by ASP Device only. */
} B_AspInputCreateSettings;

/**
Summary:
B_AspInput_Get/_Set related settings.
**/
typedef struct B_AspInputSettings
{
    NEXUS_CallbackDesc                      endOfStreaming;         /* Invoked when FIN/RST is received, RTO/keepalive timeout happens. */
                                                                    /* Call B_AspInput_GetStatus() to find out the specific event causing this callback. */

    NEXUS_CallbackDesc                      bufferReady;            /* Invoked when there is buffer to be gotten by the app using the B_AspInput_GetBuffer() API. */
                                                                    /* Only applies if B_AspInputStartSettings.feedMode == B_AspInputFeedMode_eHost || */
                                                                    /*              if B_AspInputStartSettings.feedMode == B_AspInputFeedMode_eAutoWithHostDecryption. */

    NEXUS_CallbackDesc                      httpResponseDataReady;  /* Invoked when some data belonging to a HTTP Response is received from network. */
                                                                    /* The app can receive this data using B_AspOuput_GetHttpRepsonseData(). */
} B_AspInputSettings;

/**
Summary:
DRMs supported by AspInput.
**/
typedef enum B_AspInputDrmType
{
    B_AspInputDrmType_eNone,
    B_AspInputDrmType_eDtcpIp,
    B_AspInputDrmType_eMax
} B_AspInputDrmType;

typedef struct B_AspInputDtcpIpSettings
{
#define B_ASP_DTCP_IP_EXCH_KEY_IN_BYTES 12
    uint8_t                                 exchKey[B_ASP_DTCP_IP_EXCH_KEY_IN_BYTES]; /* This key is in clear. ASP FW derives content key using Exchange key, EMI, and Nonce value. */
    unsigned                                exchKeyLabel;
    unsigned                                emiModes;
#define B_ASP_DTCP_IP_NONCE_IN_BYTES 8
    uint8_t                                 initialNonce[B_ASP_DTCP_IP_NONCE_IN_BYTES];
    unsigned                                pcpPayloadSize;
} B_AspInputDtcpIpSettings;

/**
Summary:
B_AspInput_Connect* related settings.
**/
typedef struct B_AspInputConnectHttpSettings
{
    NEXUS_TransportType                     transportType;
    NEXUS_PlaypumpHandle                    hPlaypump;
    NEXUS_DmaHandle                         hDma;
} B_AspInputConnectHttpSettings;

/**
Summary:
B_AspInput_Start related settings.
**/
typedef enum B_AspInputFeedMode
{
    B_AspInputFeedMode_eAuto,                       /* Incoming data is automatically fed to transport.  */
    B_AspInputFeedMode_eHost,                       /* Incoming data is consumed by the host. It gets a buffer of received data using B_AspInput_GetBuffer() API, */
                                                    /* processes it, & then releases it using the B_AspInput_BufferComplete() API. */
    B_AspInputFeedMode_eAutoWithHostDecryption,     /* Incoming data is decrypted by host.  It gets a buffer of received data using  B_AspInput_GetBuffer() API, */
                                                    /* then decrypts it, & then submits it back by using the B_AspInput_BufferComplete() API. */
    B_AspInputFeedMode_eMax
} B_AspInputFeedMode;

typedef struct B_AspInputStartSettings
{
    B_AspInputFeedMode                      feedMode;
} B_AspInputStartSettings;


typedef enum B_AspInputState
{
    B_AspInputState_eIdle,                          /* Enters on _AspInput_Create() or after _AspInput_Disconnect(). */
    B_AspInputState_eConnected,                     /* Enters on _AspInput_Connect*(), after _AspInput_Stop(), or when _AspInput_Finish() completes. */
    B_AspInputState_eStreaming,                     /* Enters on _AspInput_Start(). */
    B_AspInputState_eMax
} B_AspInputState;

/**
Summary:
B_AspInput_GetStatus related.

**/
typedef struct B_AspInputStatus
{
    int                     aspChannelIndex;
    B_AspInputState         state;

    bool                    remoteDoneSending;      /* When network peer has closed its connection by sending a TCP FIN. */
                                                    /* AspInput will issue endOfStreaming callback.  */
                                                    /* AspInput state will remain in _eStreaming state until app calls AspInput_Stop API. */

    bool                    connectionReset;        /* When network peer has sent a TCP RST. */
                                                    /* AspInput will issue endOfStreaming callback.  */
                                                    /* AspInput state will remain in _eStreaming state until app calls AspInput_Stop API. */

    bool                    networkTimeout;         /* When ASP exceeds the transmission retries and can't send anymore data (HTTP Request or keepalive probes, if enabled) to remote. */
                                                    /* AspInput will issue endOfStreaming callback.  */
                                                    /* AspInput state will remain in _eStreaming state until app calls AspInput_Stop API. */

} B_AspInputStatus;

typedef struct B_AspInputTcpStatus
{
    bool                                    statsValid;
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
} B_AspInputTcpStatus;

typedef struct B_AspInputHttpStatus
{
    B_AspInputTcpStatus     tcp;
    unsigned                httpRequestsSent;
    unsigned                httpResponsesReceived;
} B_AspInputHttpStatus;

typedef struct B_AspInputDisconnectStatus
{
    struct
    {
        unsigned  finalSendSequenceNumber;
        unsigned  finalRecvSequenceNumber;
    } tcpState;
} B_AspInputDisconnectStatus;
#ifdef __cplusplus
}
#endif

#endif  /* B_ASP_INPUT_TYPES_H__ */
