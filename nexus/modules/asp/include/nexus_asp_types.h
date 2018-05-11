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
#ifndef NEXUS_ASP_CHANNEL_TYPES_H__
#define NEXUS_ASP_CHANNEL_TYPES_H__

#include "nexus_types.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Declarations of various types used by AspChannel Interface.
**/

typedef struct NEXUS_AspChannel *NEXUS_AspChannelHandle;
typedef struct NEXUS_Asp *NEXUS_AspHandle;

/**
Summary:
NEXUS_AspChannel_Create related settings.
**/
typedef struct NEXUS_AspChannelCreateBufferSettings
{
    unsigned                                size;                   /* Buffer size in bytes. */
    NEXUS_HeapHandle                        heap;                   /* Optional heap handle for buffer allocation. */
    NEXUS_MemoryBlockHandle                 memory;                 /* Optional external allocation of Buffer. Set 'size' above to reflect its size. */
    unsigned                                memoryOffset;           /* offset into 'memory' block. 'size' should be counted from the start of the memoryOffset, not the memory block size. */
} NEXUS_AspChannelCreateBufferSettings;

/**
Summary:
NEXUS_AspChannel_Create settings.
**/
typedef struct NEXUS_AspChannelCreateSettings
{
    NEXUS_AspChannelCreateBufferSettings    writeFifo;              /* FIFO where firmware will read data provided by the host via NEXUS_AspChannel_GetWriteBuffer() and transmit on the network */
                                                                    /* Default size is 4K, caller can increase it if it wants AspChannel to send larger chunk of data to the network. */
                                                                    /* When AspChannel is being used for HTTP, this buffer will be used to build outgoing HTTP Request (for Player usage) & HTTP Response (for Streaming out usage), */
                                                                    /* spaceAvailableCallback will be issued to notify caller about when there is more space available in the buffer. */
                                                                    /* This FIFO memory will need to be host user space accessible (by caller to fill-in data to be sent out) & by ASP Device accessible also. */

    NEXUS_AspChannelCreateBufferSettings    reassembledFifo;        /* FIFO where reassembled frames provided by the host via the NEXUS_AspChannel_GetReassembledWriteBuffer() are written for firmware to continue processing. */
                                                                    /* Default size is 16K, caller can increase it if it expects to feed larger size of reassembled IP Frames to the firmware. */
                                                                    /* This FIFO memory will need to be host user space accessible (by caller to fill-in reassembled frames) & by ASP Device accessible also. */

    NEXUS_AspChannelCreateBufferSettings    reTransmitFifo;         /* FIFO firmware uses to buffer data sent on the network until ACKed by the remote side. */
                                                                    /* Default size is 1MB, caller can increase it if it wants to have larger TCP retransmit queue. */
                                                                    /* This FIFO memory will need to be only ASP Device accessible. */

    NEXUS_AspChannelCreateBufferSettings    receiveFifo;            /* FIFO firmware uses to receive the incoming data from the network. */
                                                                    /* Default size is 1MB, caller can increase it if it wants to have larger receive queue. */
                                                                    /* When AspChannel is being used for HTTP & Streaming hasn't been started, this buffer will buffer up incoming HTTP Response (for Player usage) & HTTP Request (for Streaming out usage), */
                                                                    /* dataReadyCallback will be issued to notify caller about this buffered up data. */
                                                                    /* This FIFO memory will need to be host user space accessible (by caller to read the received payload) & by ASP Device accessible also. */
    NEXUS_AspChannelCreateBufferSettings    miscBuffer;             /* Misc buffer used by FW for sending control packets. Default size is 4K. */
    NEXUS_AspChannelCreateBufferSettings    m2mDmaDescBuffer;       /* Buffer used by ASP FW to allocate XPT's M2M DMA buffer for Stream-in usages. */
} NEXUS_AspChannelCreateSettings;

/**
Declarations of various types used by NEXUS Network module.
**/

#define NEXUS_ETHER_ADDR_LEN                8
/* TODO: make it use the BASP defined size
#define NEXUS_ETHER_ADDR_LEN                BASP_MAX_ENTRY_IN_MAC_ADDR
*/
typedef struct NEXUS_AspEthernetSettings
{
    /* layer 2 state. */
    uint8_t                                 localMacAddress[NEXUS_ETHER_ADDR_LEN];
    uint8_t                                 remoteMacAddress[NEXUS_ETHER_ADDR_LEN];
    unsigned                                etherType;
    unsigned                                vlanTag;
    struct {
        unsigned                            queueNumber;
        unsigned                            ingressBrcmTag;         /* 4-byte tag that overrides network switch's forwarding rules by providing the output switch port # & queue # on that port. */
                                                                    /* ASP HW inserts this tag in the Ethernet header of the output frames to the network switch. */
        unsigned                            egressClassId;          /* Flow identifier that is part of the 4-type Brcm tag inserted by the switch in the frames it forwards to the ASP Port. */
                                                                    /* ASP HW extracts this label from the incoming Brcm tag & uses it to map an incoming frame with an ASP HW Channel. */
    } networkSwitch;
} NEXUS_AspEthernetSettings;

typedef enum NEXUS_AspIpVersion
{
    NEXUS_AspIpVersion_e4,
    NEXUS_AspIpVersion_e6,
    NEXUS_AspIpVersion_eMax
} NEXUS_AspIpVersion;

typedef struct NEXUS_AspIpSettings
{
    NEXUS_AspEthernetSettings               eth;
    NEXUS_AspIpVersion                      ipVersion;
    struct {
        struct {
            unsigned                        localIpAddr;
            unsigned                        remoteIpAddr;
            unsigned                        dscp;
            unsigned                        ecn;
            unsigned                        initialIdentification;
            unsigned                        timeToLive;
        } v4;
        struct {
            unsigned                        localIpAddr[4];
            unsigned                        remoteIpAddr[4];
            unsigned                        trafficClass;
            unsigned                        flowLabel;
            unsigned                        nextHeader;
            unsigned                        hopLimit;
        } v6;
    } ver;
} NEXUS_AspIpSettings;

typedef struct NEXUS_AspTcpSettings
{
    NEXUS_AspIpSettings                     ip;
    unsigned                                localPort;
    unsigned                                remotePort;
    unsigned                                initialSendSequenceNumber;
    unsigned                                initialRecvSequenceNumber;
    unsigned                                currentAckedNumber;
    unsigned                                maxSegmentSize;
    unsigned                                remoteWindowSize;
    bool                                    enableWindowScaling;
    unsigned                                remoteWindowScaleValue;          /* Window scale factor: # of bits to left-shift the received window. */
    unsigned                                localWindowScaleValue;           /* Window scale factor: # of bits to left-shift the send window. */
    bool                                    enableTimeStamps;
    unsigned                                senderTimestamp;                 /* If timestamp option is enabled, this is the value of sender's current/last timestamp value. */
    unsigned                                timestampEchoValue;              /* Optional: TS value from remote peer's last TCP packet. 0 if not available. */
    bool                                    enableSack;
} NEXUS_AspTcpSettings;

typedef struct NEXUS_AspHttpSettings
{
    NEXUS_AspTcpSettings                    tcp;
    struct {
        unsigned                            major;
        unsigned                            minor;
    } version;
    bool                                    enableChunkTransferEncoding;
    unsigned                                chunkSize;
} NEXUS_AspHttpSettings;

typedef struct NEXUS_AspUdpSettings
{
    NEXUS_AspIpSettings                     ip;
    unsigned                                localPort;
    unsigned                                remotePort;
    bool                                    disableChecksum;
} NEXUS_AspUdpSettings;

typedef struct NEXUS_AspRtpSettings
{
    NEXUS_AspUdpSettings                    udp;
    unsigned                                startingSequenceNumber;
    /* TODO: add more RTP related settings. */
} NEXUS_AspRtpSettings;

typedef enum NEXUS_AspStreamingProtocol
{
    NEXUS_AspStreamingProtocol_eTcp,
    NEXUS_AspStreamingProtocol_eHttp,
    NEXUS_AspStreamingProtocol_eUdp,
    NEXUS_AspStreamingProtocol_eRtp,
    NEXUS_AspStreamingProtocol_eMax
} NEXUS_AspStreamingProtocol;

typedef enum NEXUS_AspStreamingMode
{
    NEXUS_AspStreamingMode_eIn,           /* ASP Channel is used for receiving & playing AV data from network. */
    NEXUS_AspStreamingMode_eOut,          /* ASP Channel is used for streaming out AV data to network. */
    NEXUS_AspStreamingMode_eMax
} NEXUS_AspStreamingMode;

/**
Summary:
DRMs supported by AspChannel.
**/
typedef enum NEXUS_AspChannelDrmType
{
    NEXUS_AspChannelDrmType_eNone,
    NEXUS_AspChannelDrmType_eDtcpIp,
    NEXUS_AspChannelDrmType_eMax
} NEXUS_AspChannelDrmType;

typedef struct NEXUS_AspChannelDtcpIpSettings
{
#define NEXUS_ASP_DTCP_IP_EXCH_KEY_IN_BYTES 12
    uint8_t                                 exchKey[NEXUS_ASP_DTCP_IP_EXCH_KEY_IN_BYTES]; /* This key is in clear. ASP FW derives content key using Exchange key, EMI, and Nonce value. */
    unsigned                                exchKeyLabel;
    unsigned                                emiModes;
#define NEXUS_ASP_DTCP_IP_NONCE_IN_BYTES 8
    uint8_t                                 initialNonce[NEXUS_ASP_DTCP_IP_NONCE_IN_BYTES];
    unsigned                                pcpPayloadSize;
} NEXUS_AspChannelDtcpIpSettings;

/**
Summary:
NEXUS_AspChannel_Start related settings.
**/
typedef struct NEXUS_AspChannelStartSettings
{
    NEXUS_AspStreamingProtocol              protocol;
    bool                                    connectionLost;
    struct {
        NEXUS_AspTcpSettings                tcp;
        NEXUS_AspHttpSettings               http;
        NEXUS_AspUdpSettings                udp;
        NEXUS_AspRtpSettings                rtp;
    } protocolSettings;

    NEXUS_AspStreamingMode                  mode;
    struct {
        struct {
            NEXUS_PlaypumpHandle            playpump;
            NEXUS_DmaHandle                 dma;
        } streamIn;

        struct {
            NEXUS_RecpumpHandle             recpump;
        } streamOut;
    } modeSettings;

    bool                                    autoStartStreaming;             /* If true, streaming will begin automatically on Start.  Otherwise call
                                                                               NEXUS_AspChannel_StartStreaming() */
    struct {
        NEXUS_TransportType                 transportType;
        unsigned                            maxBitRate;
    } mediaInfoSettings;

    NEXUS_AspChannelDrmType                 drmType;
} NEXUS_AspChannelStartSettings;


/**
Summary:
Settings for stopping AspChannel.
**/
typedef enum NEXUS_AspChannelStopMode
{
    NEXUS_AspChannelStopMode_eAbort,                                        /* Default: Stop as soon as possible.
                                                                               Any data pending for TCP ACKnowlegement will be dropped.
                                                                               TCP Connection will be closed with a RST. */
    NEXUS_AspChannelStopMode_eNormal,                                       /* Normal Stop where ASP ensures that all pending TCP ACKs are received from the remote.
                                                                               TCP Connection will be closed with a FIN. */
    NEXUS_AspChannelStopMode_eMax
} NEXUS_AspChannelStopMode;

typedef struct NEXUS_AspChannelStopSettings
{
    NEXUS_AspChannelStopMode mode;
} NEXUS_AspChannelStopSettings;

/**
Summary:
NEXUS_AspChannel_Get/_Set related settings.
**/
typedef struct NEXUS_AspChannelSettings
{
    NEXUS_CallbackDesc                      dataReady;                      /* invoked when new data is available from the network. */

    NEXUS_CallbackDesc                      overflow;                       /* invoked when data from network had to be dropped due to receive buffer overflow. */

    NEXUS_CallbackDesc                      spaceAvailable;                 /* invoked when new data can be written to network. */

    NEXUS_CallbackDesc                      reassembledSpaceAvailable;      /* invoked when new data can be written to the reassebly FIFO. */

    NEXUS_CallbackDesc                      stateChanged;                   /* invoked when AspChannel State changes. Call NEXUS_AspChannel_GetStatus() to get the new state. */

    NEXUS_CallbackDesc                      noMoreDataFromRemote;           /* invoked when remote peer has indicated that it has no more data to send (by sending TCP FIN). */
} NEXUS_AspChannelSettings;

typedef enum NEXUS_AspChannelState
{
    NEXUS_AspChannelState_eIdle,                                            /* enters on _AspChannel_Create() or
                                                                               after _AspChannel_Stop() when FW has has stopped streaming & sent back the Resp for StopStreaming msg. */
    NEXUS_AspChannelState_eStarted,                                         /* enters on _AspChannel_Start() when autoStartStreaming == false. */
    NEXUS_AspChannelState_eWaitingForStreamingStart,                        /* enters on _AspChannel_StartStreamer(), waits for FW to start streaming & send back a resp msg. */
    NEXUS_AspChannelState_eStartedStreaming,                                /* enters when FW has sent the Resp to StartStreaming msg & has started Streaming. */
    NEXUS_AspChannelState_eRcvdEndOfStream,                                 /* enters when FIN msg is received from network for StreamingIn Mode. */
    NEXUS_AspChannelState_eNetworkError,                                    /* enters when RST msg is received from network. */
    NEXUS_AspChannelState_eWaitingForStreamingStop,                         /* enters on _AspChannel_StopStreaming() or _AspChannel_Stop(), waiting for FW to finish stopping & send back a resp. */
    NEXUS_AspChannelState_eWaitingForAbort,                                 /* enters on _AspChannel_Abort(), waiting for FW to finish aborting & send back a resp. */
    NEXUS_AspChannelState_eStoppedStreaming,                                /* enters when FW has has stopped streaming & sent back the Resp for StopStreaming msg. */
    NEXUS_AspChannelState_eMax
} NEXUS_AspChannelState;

/**
Summary:
NEXUS_AspChannel_GetStatus related.
**/
typedef struct NEXUS_AspChannelStatus
{
    int                                     aspChannelIndex;
    NEXUS_AspChannelState                   state;
    bool                                    statsValid;
    struct
    {
        unsigned                            finalSendSequenceNumber;
        unsigned                            finalRecvSequenceNumber;
    } tcpState;
    struct
    {
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
    } stats;

} NEXUS_AspChannelStatus;

#ifdef __cplusplus
}
#endif

#endif
