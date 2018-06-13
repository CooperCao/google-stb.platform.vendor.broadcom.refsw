/********************************************************************************
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
 *
 * File Description:
 *   FW API
 *
 ********************************************************************************/
#ifndef BASP_FW_API_H_
#define BASP_FW_API_H_

#define BASP_MAX_AVFLOWS                  32
#define BASP_MAX_NUM_PIDS                 1 /* if Mux Enable, this is 16. If Mux disable, this is 1 */
#define BASP_MAX_ENTRY_IN_MAC_ADDR        8
#define BASP_MAX_ENTRY_IN_IP_ADDR         4
#define BASP_MAX_ENTRY_PER_SG_TABLE_FEED  4

/* Define ASP FW to Host PI interrupt bits in ASP_ARCSS_HOST_FW2H_L2 */
#define BASP_FW2H_ACK_BIT           0 /* For acking host message */
#define BASP_FW2H_RESP_BIT          1 /* For response to host message */
#define BASP_FW2H_FINNOTIFY_BIT     2 /* For FIN notify message */
#define BASP_FW2H_RSTNOTIFY_BIT     3 /* For RST notify message */
#define BASP_FW2H_FINCOMP_BIT       4 /* For FIN completion message */
#define BASP_FW2H_PAYNOTIFY_BIT     5 /* For payload notify message */
#define BASP_FW2H_FRAMEAVAIL_BIT    6 /* For frame available message */
#define BASP_FW2H_SGFEEDCOMP_BIT    7 /* For generic sg table feed completion message */
#define BASP_FW2H_RTONOTIFY_BIT     8 /* For RTO notify message */

/* Define ASP FW to Host RA interrupt bits in ASP_ARCSS_HOST2_FW2H_L2 */
#define BASP_FW2H2_ACK_BIT          0 /* For acking ra message */
#define BASP_FW2H2_RESP_BIT         1 /* For response to ra message */
#define BASP_FW2H2_COMP_BIT         2 /* For ra completion message */

/* Define ASP FW to SAGE interrupt bits in ASP_ARCSS_SAGE_FW2S_L2 */
#define BASP_FW2S_ACK_BIT           0 /* For acking sage message */
#define BASP_FW2S_RESP_BIT          1 /* For response to sage message */

#define BASP_FW2H_PRINT_BUFFER_SIZE  1024

#define BASP_DEBUG_HOOK_CONGESTION_CONTROL_ENABLE_BIT  0
#define BASP_DEBUG_HOOK_DUPLICATE_ACK_COND_BIT         1
#define BASP_DEBUG_HOOK_KEEP_ALIVE_TIMER_ENABLE_BIT    2
#define BASP_DEBUG_HOOK_PERSIST_TIMER_ENABLE_BIT       3

#define BASP_MAX_FW_TASKS           6 /* Number of fw tasks (RHP, FS, MI, SF, SKP, WD) */

#define BASP_BLOCKOUT_SEND_DEBUG_INFO  30

#define BASP_EPKT_RETX_BUFFER_ALIGNMENT_IN_BYTES  32
#define BASP_EDPKT_HDR_BUFFER_ALIGNMENT_IN_BYTES  32
#define BASP_STATUS_BUFFER_ALIGNMENT_IN_BYTES     32
#define BASP_RECEIVE_BUFFER_ALIGNMENT_IN_BYTES    32
#define BASP_MISC_BUFFER_ALIGNMENT_IN_BYTES       32

#define IPV4_CHECKSUM_ENABLE        1
#define TP_CHECKSUM_ENABLE          2

/* Converts Host reg address to ASP addr space. */
#define BASP_CVT_TO_ASP_REG_ADDR(addr)  ((addr) | 0xf0000000)

/* Enum defines */

/***************************************************************************
Summary:
Enum indicating whether Ack or Response required by Host.
****************************************************************************/
typedef enum BASP_ResponseType
{
    /* Ack: an interrupt is raised by FW once the read pointer is updated after reading a message, no message is send. */
    BASP_ResponseType_eNoneRequired,
    BASP_ResponseType_eAckRequired,
    BASP_ResponseType_eRespRequired,
    BASP_ResponseType_eAckRespRequired,
    BASP_ResponseType_eInvalid = 0x7fffffff
} BASP_ResponseType;

/***************************************************************************
Summary:
Defines the message types.
PI2FW: indicates the message is from Host to ASP FW.
FW2PI: Indicates the message is from ASP FW to Host.
****************************************************************************/

typedef enum BASP_MessageType
{
    /* Host to Asp message, total 13 types */
    BASP_MessageType_ePi2FwInvalid = 0, /* Enum 0 is invalid */
    BASP_MessageType_ePi2FwInit,
    BASP_MessageType_ePi2FwUnInit,
    BASP_MessageType_ePi2FwChannelStartStreamOut,
    BASP_MessageType_ePi2FwChannelStartStreamIn,
    BASP_MessageType_ePi2FwChannelStop,
    BASP_MessageType_ePi2FwChannelAbort,
    BASP_MessageType_ePi2FwChannelAbortWithRst,
    BASP_MessageType_ePi2FwPerformanceGathering,
    BASP_MessageType_ePi2FwGenericSgTableFeed,
    BASP_MessageType_ePi2FwPayloadConsumed,
    BASP_MessageType_ePi2FwFrameConsumed,
    BASP_MessageType_ePi2FwGetDrmConst,

    /* Asp To Host response messages, total 12(responses) + 7 = 19 types */
    BASP_MessageType_eFw2PiInitResp = 32,
    BASP_MessageType_eFw2PiUnInitResp,
    BASP_MessageType_eFw2PiChannelStartStreamOutResp,
    BASP_MessageType_eFw2PiChannelStartStreamInResp,
    BASP_MessageType_eFw2PiChannelStopResp,
    BASP_MessageType_eFw2PiChannelAbortResp,
    BASP_MessageType_eFw2PiChannelAbortWithRstResp,
    BASP_MessageType_eFw2PiPerformanceGatheringResp,
    BASP_MessageType_eFw2PiGenericSgTableFeedResp,
    BASP_MessageType_eFw2PiPayloadConsumedResp,
    BASP_MessageType_eFw2PiFrameConsumedResp,
    BASP_MessageType_eFw2PiGetDrmConstResp,

    /* Message from ASP FW to Host due to events from network / client */
    BASP_MessageType_eFw2PiFinNotify = 64,
    BASP_MessageType_eFw2PiRstNotify,
    BASP_MessageType_eFw2PiFrameAvailable,
    BASP_MessageType_eFw2PiFinComplete,
    BASP_MessageType_eFw2PiPayloadNotify,
    BASP_MessageType_eFw2PiGenericSgTableFeedCompletion,
    BASP_MessageType_eFw2PiRtoNotify, /* FW reaches maximum retx tries */

    /* RA related */
    BASP_MessageType_eRa2FwReassembledPacket = 96, /* enum from 97 to 111 are reserved for new message types */
    BASP_MessageType_eRa2FwReassembledPacketResp = 112, /* enum from 113 to 127 are reserved for new message types */
    BASP_MessageType_eFw2RaReassembledPacketCompletion = 128, /* enum from 129 to 143 are reserved for new message types */

    /* SAGE related */
    BASP_MessageType_eSage2FwConstInfo = 144, /* enum from 145 to 159 are reserved for new message types */
    BASP_MessageType_eFw2SageReqConst  = 160, /* enum from 161 to 175 are reserved for new message types */
    BASP_MessageType_eFw2SageReqConstResp = 176 /* enum from 177 to 191 are reserved for new message types */
} BASP_MessageType;

/***************************************************************************
Summary:
Defines the HTTP Connection Type.
****************************************************************************/
typedef enum BASP_HttpConnectionType
{
    BASP_HttpConnectionType_e10,                  /* HTTP 1.0 */
    BASP_HttpConnectionType_e11WithChunking,      /* HTTP 1.1 with chunking enable */
    BASP_HttpConnectionType_e11WithoutChunking,   /* HTTP 1.1 without chunking enable */
    BASP_HttpConnectionType_eInvalid = 0x7fffffff
} BASP_HttpConnectionType;

/***************************************************************************
Summary:
Defines the 3-bit Parser Stream Type value in ASP_MCPB_CHx_SP_PARSER_CTRL
****************************************************************************/
typedef enum BASP_ParserStreamType
{
    BASP_ParserStreamType_eMpeg    = 0,  /* MPEG mode, default */
    BASP_ParserStreamType_eDss     = 1,  /* DSS mode */
    BASP_ParserStreamType_eBlock   = 6,  /* Block mode for frame pass-through */
    BASP_ParserStreamType_eInvalid = 7   /* Undefined */
} BASP_ParserStreamType;

/***************************************************************************
Summary:
Defines the type of transport stream time-stamp config
****************************************************************************/
typedef enum BASP_TsTimeStampConfig
{
    BASP_TsTimeStampConfig_eNone    = 0,  /* Disable all ts timestamps, default */
    BASP_TsTimeStampConfig_eInput   = 1,  /* Input ts timestamp enabled */
    BASP_TsTimeStampConfig_eOutput  = 2,  /* Output ts timestamp enabled */
    BASP_TsTimeStampConfig_eBoth    = 3   /* Both input/output timestamps enabled */
} BASP_TsTimeStampConfig;

/***************************************************************************
Summary:
Message response status.
****************************************************************************/
typedef enum BASP_ResponseStatus
{
    BASP_ResponseStatus_eSuccess,
    BASP_ResponseStatus_eFailed
} BASP_ResponseStatus;

/***************************************************************************
Summary:
This enum describes the time stamp type information .
Note: This enum must be kept in-sync with nexus_types.h:NEXUS_TransportTimestampType
****************************************************************************/
typedef enum BASP_TransportTimeStampType
{
    /* No timestamp is prepended to the transport packets. */
    BASP_TransportTimeStampType_eNone,

    /* 30 bit timestamp, mod-300 value, 2 user bits. no parity bits. A 4-byte 27MHz timestamp is prepended to every transport packet.Used for MPEG2TS streams (188 byte packets).The 27Mhz timestamp is divided by 300 to convert to a 90KHz timestamp before use because MPEG2TS PTS/PCR units are 90Khz. */
    BASP_TransportTimeStampType_e302uMod300,
    BASP_TransportTimeStampType_eMod300 = BASP_TransportTimeStampType_e302uMod300, /* alias */

    /* 30 bit timestamp, binary value, 2 user bits. no parity bits. A 4-byte 27MHz timestamp is prepended to every transport packet.Used for DSS streams (130 byte packets).The 27Mz timestamp is used unmodified because DSS PTS/PCR units are 27Mhz. */
    BASP_TransportTimeStampType_e302uBinary,
    BASP_TransportTimeStampType_eBinary = BASP_TransportTimeStampType_e302uBinary, /* alias */

    /* 32 bit timestamp, mod-300 value, no user bits. no parity bits. */
    BASP_TransportTimeStampType_e32Mod300,

    /* 32 bit timestamp, binary value, no user bits. no parity bits. */
    BASP_TransportTimeStampType_e32Binary,

    /* e28_4P_Binary is not supported */
    /* 28 bit timestamp, 4 bit parity, mod-300 value */
    BASP_TransportTimeStampType_e284pMod300,
    BASP_TransportTimeStampType_e284pBinary,

    BASP_TransportTimeStampType_e28Mod300,
    BASP_TransportTimeStampType_e28Binary,

    BASP_TransportTimeStampType_eMax
} BASP_TransportTimeStampType;

/***************************************************************************
Summary:
FW task mailbox status.
****************************************************************************/
/* Rhp task progress status. Max 24 bit value */
typedef enum BASP_MailboxRhpStatus
{
    BASP_MailboxRhpStatus_eUnInit                             = 0x100000,
    BASP_MailboxRhpStatus_eInit,                             /* 0x100001 */
    BASP_MailboxRhpStatus_eWaitAspInit,                      /* 0x100002 */
    BASP_MailboxRhpStatus_eProcessHeaders,                   /* 0x100003 */
    BASP_MailboxRhpStatus_eInspectAck,                       /* 0x100004 */
    BASP_MailboxRhpStatus_eInspectFinRst,                    /* 0x100005 */
    BASP_MailboxRhpStatus_eInspectPayload,                   /* 0x100006 */
    BASP_MailboxRhpStatus_eProcessByPass,                    /* 0x100007 */
    BASP_MailboxRhpStatus_eProcessStreamOutChannelStop,      /* 0x100008 */
    BASP_MailboxRhpStatus_eProcessStreamInChannelStop,       /* 0x100009 */
    BASP_MailboxRhpStatus_eSendAck,                          /* 0x10000a */
    BASP_MailboxRhpStatus_eStreamInDMAIdle,                  /* 0x10000b */
    BASP_MailboxRhpStatus_eStreamInDMAFull,                  /* 0x10000c */
    BASP_MailboxRhpStatus_eStreamInDMAWriteDesc,             /* 0x10000d */
    BASP_MailboxRhpStatus_eStreamInPcpHeaderBtp,             /* 0x10000e */
    BASP_MailboxRhpStatus_eStreamInWriteBtpDesc,             /* 0x10000f */
    BASP_MailboxRhpStatus_eStreamInDramPcpBtpFull,           /* 0x100010 */
    BASP_MailboxRhpStatus_eStreamInClearLastDesc,            /* 0x100011 */
    BASP_MailboxRhpStatus_eStreamInChunkUnWrap,              /* 0x100012 */
    BASP_MailboxRhpStatus_eStreamInPcpHeaderUnWrap,          /* 0x100013 */
    BASP_MailboxRhpStatus_eStreamInRAFifoRefillIdle,         /* 0x100014 */
    BASP_MailboxRhpStatus_eStreamInRAFifoRefillPend,         /* 0x100015 */

    /* Add more enums below */

    BASP_MailboxRhpStatus_eTaskSelfYield,
    BASP_MailboxRhpStatus_eMax
} BASP_MailboxRhpStatus;

/* MI task progress status. Max 24 bit value */
typedef enum BASP_MailboxMiStatus
{
    BASP_MailboxMiStatus_eTaskCreate                = 0x300000,
    BASP_MailboxMiStatus_eTaskRun,
    BASP_MailboxMiStatus_eMsgFetch,
    BASP_MailboxMiStatus_eMsgProc,
    BASP_MailboxMiStatus_eMsgWrite,
    BASP_MailboxMiStatus_eTaskYield,
    BASP_MailboxMiStatus_eMax
} BASP_MailboxMiStatus;

/* FS task progress status. Max 24 bit value */
typedef enum BASP_MailboxFsStatus
{
    BASP_MailboxFsStatus_eUnInit                     = 0x200000,
    BASP_MailboxFsStatus_eInit,                     /* 0x200001 */
    BASP_MailboxFsStatus_eWaitAspInit,              /* 0x200002 */
    BASP_MailboxFsStatus_eProcessChannelClose,      /* 0x200003 */
    BASP_MailboxFsStatus_eProcessHttpRequest,       /* 0x200004 */
    BASP_MailboxFsStatus_eProcessRetransmission,    /* 0x200005 */
    BASP_MailboxFsStatus_eProcessFramePassthrough,  /* 0x200006 */
    /* Add more enums below */

    BASP_MailboxFsStatus_eTaskSelfYield,
    BASP_MailboxFsStatus_eMax
} BASP_MailboxFsStatus;

/* SF task progress status. Max 24 bit value */
typedef enum BASP_MailboxSfStatus
{
    BASP_MailboxSfStatus_eUnInit                     = 0x400000,
    BASP_MailboxSfStatus_eInit,                     /* 0x400001 */
    BASP_MailboxSfStatus_eWaitAspInit,              /* 0x400002 */
    BASP_MailboxSfStatus_eProcessChannelStartInfo,  /* 0x400003 */
    BASP_MailboxSfStatus_eProcessSkpMessage,        /* 0x400004 */
    BASP_MailboxSfStatus_eDataStreamerStart,        /* 0x400005 */
    BASP_MailboxSfStatus_eDsProcessChannelClose,    /* 0x400006 */
    BASP_MailboxSfStatus_eDsProcessRavePointers,    /* 0x400007 */
    BASP_MailboxSfStatus_eDsProcessLastItb,         /* 0x400008 */
    BASP_MailboxSfStatus_eDsSendPacketOut,          /* 0x400009 */
    BASP_MailboxSfStatus_eDsInitiateChannelClose,   /* 0x40000A */
    BASP_MailboxSfStatus_eDsUpdateChannelCloseState,/* 0x40000B */
    BASP_MailboxSfStatus_eDsSelfYield,              /* 0x40000C */
    /* Add more enums below */

    BASP_MailboxSfStatus_eTaskSelfYield,
    BASP_MailboxSfStatus_eMax
} BASP_MailboxSfStatus;

/* SKP task progress status. Max 24 bit value */
typedef enum BASP_MailboxSkpStatus
{
    BASP_MailboxSkpStatus_eUnInit                    = 0x500000,
    BASP_MailboxSkpStatus_eInit,                    /* 0x500001 */
    BASP_MailboxSkpStatus_eWaitAspInit,             /* 0x500002 */
    BASP_MailboxSkpStatus_eNewChannelRequest,       /* 0x500003 */
    BASP_MailboxSkpStatus_eCurrChannelRequest,      /* 0x500004 */
    BASP_MailboxSkpStatus_eKeyGenSuccess,           /* 0x500005 */
    BASP_MailboxSkpStatus_eKeyGenFail,              /* 0x500006 */
    BASP_MailboxSkpStatus_eKeyGenQueueFull,         /* 0x500007 */
    /* Add more enums below */

    BASP_MailboxSkpStatus_eTaskSelfYield,
    BASP_MailboxSkpStatus_eMax
} BASP_MailboxSkpStatus;

/* WDT task progress status. Max 24 bit value */
typedef enum BASP_MailboxWdtStatus
{
    BASP_MailboxWdtStatus_eUnInit                    = 0x600000,
    BASP_MailboxWdtStatus_eMax
} BASP_MailboxWdtStatus;

/***************************************************************************
Summary:
This indicates the ASP PID information
****************************************************************************/
typedef struct BASP_PidInfo
{
    uint32_t ui32ProgramType;
    uint32_t ui32PidChannel; /* is  Parser band index */
    uint32_t ui32PidValue;
} BASP_PidInfo;

/***************************************************************************
Summary:
This indicates the ASP PID List and number of ASP PIDS
****************************************************************************/
typedef struct BASP_Pids
{
    uint32_t ui32NumASPPids;
    /* TODO: If file source is directly feeding the AV payloads via ASP FW messages, we may need to add AV Pid info */
    /* TODO: include videoPid, audioPid, pcrPid, pmtPid, audioPids */
    /* Usage is not clearly known at this point of time(11/11/2014). Looks like it may be useful at the time of asp based Mux development. For current usages we will set this value for allpass Pid cahnnel and no pid value.*/
    BASP_PidInfo aPidList[BASP_MAX_NUM_PIDS];
} BASP_Pids;

/***************************************************************************
Summary:
Structure that holds the linear Dram buffer address and size.
****************************************************************************/
typedef struct BASP_DramBufferInfo
{
    uint32_t ui32BaseAddrLo;
    uint32_t ui32BaseAddrHi;
    uint32_t ui32Size;
} BASP_DramBufferInfo;

/***************************************************************************
Summary:
Describes the common message header.
****************************************************************************/
typedef struct BASP_MessageHeader
{
    /* Describes the message type. */
    BASP_MessageType  MessageType;

    /* This is unique for every message. */
    uint32_t ui32MessageCounter;

    /* This describes the message response type. SUCCESS or Fail */
    BASP_ResponseType ResponseType;

    /* This is the channel number, for init message this field has no meaning/useless */
    uint32_t ui32ChannelIndex;

    /* reserved for future use */
    uint32_t ui32Reserved1;
    uint32_t ui32Reserved2;
    uint32_t ui32Reserved3;
    uint32_t ui32Reserved4;
} BASP_MessageHeader;

/***************************************************************************
Summary:
ASP initialization details.
****************************************************************************/
typedef struct BASP_InitMessage
{
    /* Max number of channels supported */
    uint32_t ui32NumMaxChannels;

    /* Base Address (in ASP space) of XPT MEMDMA MCPB registers (use BASP_CVT_TO_ASP_REG_ADDR macro) */
    uint32_t ui32MemDmaMcpbBaseAddressLo; /* Low 32 bits */
    uint32_t ui32MemDmaMcpbBaseAddressHi; /* High 32 bits */

    /* Base Address (in ASP space) of XPT MCPB registers (use BASP_CVT_TO_ASP_REG_ADDR macro) */
    uint32_t ui32XptMcpbBaseAddressLo;   /* Low 32 bits */
    uint32_t ui32XptMcpbBaseAddressHi;   /* High 32 bits  */

    /* Stream In Descriptor Type - Bryan */
    uint32_t ui32McpbStreamInDescType;  /* 0: 8  Word Descriptor
                                           1: 4  Word Descriptor
                                           2: 12 Word Descriptor
                                           3: Reserved
                                          (Default is 2)
                                        */

    /* This specifies the common memory required for asp */
    /* Common Header buffer: for receiving IP/TCP/UDP headers from e-depacketizer */
    BASP_DramBufferInfo EdPktHeaderBuffer;

    /* common  status  buffer : this can be used to check the overall asp status at any instant */
    /* This can be used to determine asp channel status at any instant. This will also include network stats. */
    /* TODO: RM: make sure this statusBuffer is contigous for all channels! This way FW can write all status in one DRAM transcation for all channels */
    /* This needs more discussion with Flaviu */
    BASP_DramBufferInfo StatusBuffer;

    /* Debug buffer: this can be used to collect debug messages */
    BASP_DramBufferInfo DebugBuffer;

   /* Any hardware initialization related information need to be passed at the time of initialization */
    BASP_DramBufferInfo HwInitInfo;
} BASP_InitMessage;

/***************************************************************************
Summary:
ASP un-initialization details.
****************************************************************************/
typedef struct BASP_UnInitMessage
{
    /* Dummy: since there is no payload for this message. */
    uint32_t ui32Unused;
} BASP_UnInitMessage;

/*
* CRC: generation happens by the unimac and not by the E-PKT h/w.
* X-Packet header always have timestamps + 188 byte payload
* IPv4 ID value:  we will need to see if we can get this Id value from Linux at the time of connx offload,
* otherwise, pass a random 16bit value to PI. It needs to be different for us to handle the case where
* fragments can get re-ordered.
*
* Need to capture TCP packet and study the timestamp value: does sender need to include tscer echo value
* if it has not received any current data from the receiver?
*
*/

/***************************************************************************
Summary:
This structure contains per-channel DTCP-IP info.
****************************************************************************/
typedef struct BASP_DtcpIpInfoFromHost
{
    uint32_t ui32ExchangeKeys[3];    /* Obtained after source device after Full/Restricted authentication */
    uint32_t ui32C_A2;               /* C_A2 and C_A are 2 bits identifying the cipher algorithm */
    uint32_t ui32ExchangeKeyLabel;   /* Non-zero label means encrypted content */
    uint32_t ui32EmiModes;           /* 4 bit value (Encryption mode indicator) for indicating CCI(copy-control information) */
    uint32_t ui32Nc[2];              /* Nonce for content channel (64 bit) */
    uint32_t ui32ASPKeySlot;
} BASP_DtcpIpInfoFromHost;

/***************************************************************************
Summary:
This structure carries connection specific details.
This is used for connection migration for TCP.
****************************************************************************/
typedef struct BASP_ConnectionControlBlock
{
    uint8_t aui8DestMacAddr[BASP_MAX_ENTRY_IN_MAC_ADDR];
    uint8_t aui8SrcMacAddr[BASP_MAX_ENTRY_IN_MAC_ADDR];

    /* This is a variable field we do't want FW to hardcode this. This field defines the protocol at layer 3 level. For AV streaming purposes this ip 0x806 */
    uint32_t ui32EtherType;

    /* brcm Tag */
    uint32_t ui32IngressBrcmTag;
    uint32_t ui32EgressClassId;

    uint32_t ui32IpVersion;    /** TODO: Change it to a enum **/

    /************** ipv4 specific *************/
    /* TODO: Check the names with Sanjeev . */
    uint32_t ui32Dscp;
    uint32_t ui32Ecn;
    /*****************************************/

    /*TODO: this is the unique identification number that we will provide at the time of channel start. Need to check how to generate this.
    IPv4 ID value will need to see if we can get this Id value from Linux at the time of connx offload, otherwise pass a random 16bit value to PI.
    This is mainly used for fragmented packet re-assembly.*/
    uint32_t ui32IpIdSel;

    uint32_t ui32TimeToLive;

    uint32_t ui32ProtocolType; /** TODO: Change it to a enum **/

    /************* ipv6 specific *************/
    uint32_t ui32TrafficClass;
    uint32_t ui32FlowLabel;
    uint32_t ui32NextHeader;
    uint32_t ui32HopLimit;
    /*****************************************/

    uint32_t ai32SrcIpAddr[BASP_MAX_ENTRY_IN_IP_ADDR];
    uint32_t ai32DestIpAddr[BASP_MAX_ENTRY_IN_IP_ADDR];

    uint32_t ui32SrcPort;
    uint32_t ui32DestPort;

    /* Initial sequence number of asp. */
    uint32_t ui32InitialSendSeqNumber;

    /* Initial expected sequence number from remote side for stream-in. */
    uint32_t ui32InitialReceivedSeqNumber;

    /* Initial ack sequence number of asp. */
    uint32_t ui32CurrentAckedNumber;

    /* Initial window size of remote. */
    uint32_t ui32RemoteWindowSize;

    /* For FW to calculate actual window size to send.
     * ACK Window Size = Current Value of EDPKT RX Buffer depth - WindowAdvConst
     * */
    uint32_t ui32WindowAdvConst;

    /* Window scale value of asp, set to 0 to disable window scaling. */
    uint32_t ui32LocalWindowScaleValue;

    /* Window scale value of remote, set to 0 to disable window scaling. */
    uint32_t ui32RemoteWindowScaleValue;

    uint32_t ui32SackEnable;

    uint32_t ui32TimeStampEnable;
    /* Need to check that the initial value for TSECRneed to be specified by host. */
    /* This is local timestamp value from Host to FW */
    uint32_t ui32LocalTimeStampValue; /* TODO: we will check if host can get this value, else send 0 */

    uint32_t ui32MaxSegmentSize;

    /* Keep-alive parameters, valid only when ui32KeepAliveTimerEnable = 1 */
    uint32_t ui32KaTimeout;  /* 1st keep-alive timeout in milliseconds, default: 7200000 */
    uint32_t ui32KaInterval; /* Subsequent KA interval in milliseconds, default: 75000 */
    uint32_t ui32KaMaxProbes;  /* Max No. KA probes, default: 9 */

    /* Retx parameters, valid only when ui32RetransmissionEnable = 1 */
    uint32_t ui32RetxTimeout;  /* 1st retx timeout in milliseconds, default: 1000 */
    uint32_t ui32RetxMaxRetries; /* Max No. retx tries, default: 13 */

    /* IPV4 and TCP checksum enable for EDPKT */
    uint32_t ui32CheckSumEnable;         /* b'00 for all disable
                                            b'10 for TCP checksum enable
                                            b'01 for IPV4 checksum enable
                                            b'11 for all enable
                                          */
} BASP_ConnectionControlBlock;

/***************************************************************************
Summary:
This structure defines the MCPB streamout configuration.
****************************************************************************/
typedef struct BASP_McpbStreamOutConfig
{
    /* This is for ASP PID List and Number of ASP PIDs */
    BASP_Pids ASPPids;

    /* This is used to support different kind of pacing. */
    uint32_t ui32PacingType;

    /* Transport stream time-stamp related configurations */
    uint32_t ui32TsTimestampEnable; /* 0: All TS timestamp disabled (default);
                                       1: Input TS timestamp enabled;
                                       2: Output TS timestamp enabled;
                                       3: Both input/output timestamps enabled.
                                    */

    /* This describes the input timestamp type information */
    uint32_t ui32SrcTimestampType;

    /* TODO: Set 1 if you want to restamp the incoming timestamps or if you are recording and want to add timestamps. Anand will check with Rajesh and get back. */
    uint32_t ui32ForceRestamping;

    /* Some chips can pace playback using a PCR pid. Set timestamp.pcrPacingPid to a non-zero value and set timestamp.pacing = 1. */
    uint32_t ui32PcrPacingPid;

    /* Set 1 to disable timestamp parity check. */
    uint32_t ui32ParityCheckDisable;

     /* In bps: Per Rajesh, this bitrate will be used for per packet blockout calculations to avoid bursty stream out behavior */
    /* Default value can be 35Mbs, it may be fixed per customer box usage */
    uint32_t ui32AvgStreamBitrate;

    /* timebase for contolling the pacing clock */
    uint32_t ui32TimebaseIndex;

    /* Packet Length */
    uint32_t ui32PacketLen;

    uint32_t ui32ParserAllPassControl;
    uint32_t ui32ParserStreamType;

    /* Control whether to insert the BTP packet at packet boundary or at byte boundary:
     * 0 - PKT based mode (default)
     * 1 - BYTE based mode */
    uint32_t ui32BtpCtrl3PktOrByteBasedMode;

    /* TMEU Bound Late */
    uint32_t ui32McpbTmeuErrorBoundLate;

} BASP_McpbStreamOutConfig;

/***************************************************************************
Summary:
Channel Start Stream-In message .
****************************************************************************/
typedef struct BASP_ChannelStartStreamInMessage
{
    /* Punting feature Enable/disable - Bryan */
    uint32_t ui32AspBypassEnabled;        /* 0: Punting disabled
                                             1: Punting enabled
                                          */

    uint32_t ui32SendRstPktOnRetransTimeOutEnable;
                                          /* 0: FW sending RST packet disabled
                                             1: FW sending RST packet enabled
                                             (Default is disabled)
                                          */
    uint32_t ui32KeepAliveTimerEnable; /* 0: Keep alive timer Disable
                                              1: Keep alive timer Enable (default)
                                            */

    uint32_t ui32DrmEnabled;              /* 0: DRM Disable
                                             1: DRM Enable
                                         */
    uint32_t ui32PcpPayloadSize;

    /* Receive Buffer Info
       This is the payload buffer that EDPKT puts to FW
    */
    BASP_DramBufferInfo ReceivePayloadBuffer;

    /* Ethernet Header Buffer Info. */
    BASP_DramBufferInfo EthernetHeaderBuffer;

    /* This is the connection control block that is created from the linux TCB structure,with this connection will be offloaded to asp */
    BASP_ConnectionControlBlock ConnectionControlBlock;

    /* This is to be sent by Host to Sage and FW and Sage will attach this label to the messages sent to FW. FW will need to match the label received from Host and Sage.  */
    uint32_t ui32StreamLabel;

    /* This is MemDmaMcpbDramDescriptor buffer info only needs to be added in StreamInCfg.Fw use it for McpbFirstDRAMDescAddr , this Dram buffer should be 16 byte alligned. */
    BASP_DramBufferInfo MemDmaMcpbDramDescriptorBuffer;

    /* This is used for configuring the MCPB Channel Number for XPT MCPB RDB or MEMDMA MCPB RDB */
    uint32_t ui32PlaybackChannelNumber;

    uint32_t ui32PidChannel;

    /* In bps: Per Rajesh, this bitrate will be used for per packet blockout calculations to avoid bursty stream out behavior */
    /* Default value can be 35Mbs, it may be fixed per customer box usage */
    uint32_t ui32AvgStreamBitrate;

    BASP_DramBufferInfo HttpRequestBuffer; /* For HTTP Request buffer */

    /* EPKT ReTransmission buffer */
    BASP_DramBufferInfo ReTransmissionBuffer;

    /*  SwitchConfig */
    uint32_t ui32SwitchSlotsPerEthernetPacket;
    uint32_t ui32SwitchQueueNumber;

    /* DTCP-IP info */
    uint32_t ui32ExchangeKeys[3];
    uint32_t ui32ASPKeySlot;

} BASP_ChannelStartStreamInMessage;

/***************************************************************************
Summary:
Channel Start Stream-Out message .
****************************************************************************/
typedef struct BASP_ChannelStartStreamOutMessage
{
    /* Synthetic Packet Enable/disable - Bryan */
    uint32_t ui32AspBypassEnabled;        /* 0: Synthetic Packet disabled
                                             1: Synthetic Packet enabled
                                          */

    uint32_t ui32SendHttpResponsePktEnable; /* 0: FW sending HTTP Response disabled
                                               1: FW sending HTTP Response enabled
                                               (Default is disabled for Version 1.0
                                            */
    uint32_t ui32SendRstPktOnRetransTimeOutEnable;
                                         /* 0: FW sending RST packet disabled
                                            1: FW sending RST packet enabled
                                            (Default is disabled)
                                         */

    /* Mux Enable/Disable - Bryan */
    uint32_t ui32MuxStatusEnabled;       /* 0: Mux Disable
                                            1: Mux Enable
                                         */

    /* Bit[0]: 0 - Congestion Flow Disable, 1 - Enable
     * Bit[1]: Duplicate Ack Condition:
     *         0 - same ack number only, 1 - same ack number + same receiver window size.
     */
    uint32_t ui32CongestionFlowControlEnable;

    uint32_t ui32RetransmissionEnable; /* 0: Retransmission Disable
                                          1: Retransmission Enable (default)
                                        */

    uint32_t ui32KeepAliveTimerEnable; /* 0: Keep alive timer Disable
                                          1: Keep alive timer Enable (default)
                                        */

    uint32_t ui32PersistTimerEnable; /* 0: Persist timer Disable
                                        1: Persist timer Enable (default)
                                      */

    uint32_t ui32DrmEnabled;              /* 0: DRM Disable
                                             1: DRM Enable
                                         */
    uint32_t ui32PcpPayloadSize;         /* if Drm is disabled, host will set this field as 0, FW can ignore */

    uint32_t ui32HttpType;             /* 0: HTTP 1.0
                                                      1: HTTP 1.1 With Chunking
                                                      2: HTTP 1.1 Without Chunking
                                                   */

    uint32_t ui32FullChunkHeaderSize;          /* Chunk HEader Size + Trailer */
    uint32_t ui32ChunkSize;                    /* Size of chunk payload */

    /* Receive Buffer Info
       This is the payload buffer that EDPKT puts to FW
    */
    BASP_DramBufferInfo ReceivePayloadBuffer;

    /* Ethernet Header Buffer Info. */
    BASP_DramBufferInfo EthernetHeaderBuffer;

    /* Buffer for sending http response if ui32SendHttpResponsePktEnable is set */
    BASP_DramBufferInfo HttpResponseBuffer;

    /* ReTransmission buffer */
    BASP_DramBufferInfo ReTransmissionBuffer;

    /* This is the connection control block that is created from the linux TCB structure,with this connection will be offloaded to asp */
    BASP_ConnectionControlBlock ConnectionControlBlock;

    /* This is to be sent by Host to Sage and FW and Sage will attach this label to the messages sent to FW. FW will need to match the label received from Host and Sage.  */
    uint32_t ui32StreamLabel;

    /* Base Address (in ASP space) of RAVE context registers (use BASP_CVT_TO_ASP_REG_ADDR macro) */
    uint32_t ui32RaveContextBaseAddressLo; /* Low 32 bits */
    uint32_t ui32RaveContextBaseAddressHi; /* High 32 bits */

    BASP_McpbStreamOutConfig McpbStreamOutConfig;

    /* Switch config info */
    uint32_t ui32SwitchSlotsPerEthernetPacket;
    uint32_t ui32SwitchQueueNumber;

    BASP_DtcpIpInfoFromHost DtcpIpInfo;

} BASP_ChannelStartStreamOutMessage;

/***************************************************************************
Summary:
Message for PI to inform FW when DTCP is ready.
****************************************************************************/
typedef struct BASP_GetDrmConstMessage
{
    uint32_t ui32Unused;
} BASP_GetDrmConstMessage;

/***************************************************************************
Summary:
Stop message.
****************************************************************************/
typedef struct BASP_ChannelStopMessage
{
    uint32_t ui32Unused;
} BASP_ChannelStopMessage;

/***************************************************************************
Summary:
Local initiated abort message.
****************************************************************************/
typedef struct BASP_ChannelAbortMessage
{
    uint32_t ui32Unused;
} BASP_ChannelAbortMessage;

/***************************************************************************
Summary:
Local initiated abort message.
****************************************************************************/
typedef struct BASP_ChannelAbortWithRstMessage
{
    uint32_t ui32Unused;
} BASP_ChannelAbortWithRstMessage;

/***************************************************************************
Summary:
Message for reassembled packet.
****************************************************************************/
typedef struct BASP_ReassembledPktMessage
{
    BASP_DramBufferInfo ReassembledPacketBuffer;
} BASP_ReassembledPktMessage;

/***************************************************************************
Summary:
Message for reassembled packet response.
****************************************************************************/
typedef struct BASP_ReassembledPktResponse
{
    uint32_t ResponseStatus;
} BASP_ReassembledPktResponse;

/***************************************************************************
Summary:
Message for reassembled packet completion.
****************************************************************************/
typedef struct BASP_ReassembledPktCompletion
{
    uint32_t ui32PA2AddrLo; /* PA2 address from MEMDMA MCPB */
    uint32_t ui32PA2AddrHi;
} BASP_ReassembledPktCompletion;

/***************************************************************************
Summary:
Message for Performance Gathering Message from host.
****************************************************************************/
typedef struct BASP_PerformanceGatheringMessage
{
    uint32_t ui32Unused;
} BASP_PerformanceGatheringMessage;

/***************************************************************************
Summary:
Message for Generic SG Table Feed Message
This is meant for either Synthetic Packet support (in Stream Out)
or RA packet support (in Stream In).
****************************************************************************/
typedef struct BASP_GenericSGTableFeedMessage
{
    uint32_t ui32NumEntries;                  /* Number of Entries: max 16 */
    uint32_t ui32TypeOfData;                  /* 0 = EPKT Data (Synthetic Packet)
                                                 1 = RA DATA   (Stream-in Mode)
                                              */
    uint32_t ui32SofEofInfo;                 /* This is a 2-bit info.
                                               Bit 0: SOF = 0 or 1
                                               Bit 1: EOF = 0 or 1
                                               If Header and Payload are stored in contiguous memory,
                                               SOF and EOF are both set.
                                               If Header and Payload are stored in different parts of memory,
                                               SOF = 1, EOF = 0 indicates Header Address location
                                               SOF = 0, EOF = 0 indicates Payload Addr location, but there are more payload data of an E-frame to be sent
                                               SOF = 0 ,EOF = 1 indicates Payload Addr location, the payload data size is able to form an E-frame
                                             */
    BASP_DramBufferInfo aFeedBuffer[BASP_MAX_ENTRY_PER_SG_TABLE_FEED];
    uint32_t ui32LastEframeStatus;            /* Last Eframe sent
                                                 0 = Not the last EFrame
                                                 1 = Last Eframe
                                              */
} BASP_GenericSGTableFeedMessage;

/***************************************************************************
Summary:
Message for PI to FW on HTTP REsponse completion whereby PI will
inform FW of the start address offset for beginning of AV data.
****************************************************************************/
typedef struct BASP_PayloadConsumedMessage
{
    uint32_t ui32RequireMoreData; /* This flag indicates to FW that ASP PI requires FW to send more packets to it as HTTP Response could have span across few E-pkts.
                                     0: Require No more data. (FW does not need to send more packets to host)
                                     1: Require more data. (FW has to send another message containing new packets that FW received to Host).
                                  */
    uint32_t ui32HttpType;             /* 0: HTTP 1.0
                                                      1: HTTP 1.1 With Chunking
                                                      2: HTTP 1.1 Without Chunking
                                                   */

    uint32_t ui32FullChunkHeaderSize;          /* Chunk HEader Size + Trailer */
    uint32_t ui32ChunkSize;                    /* Size of chunk payload */

    uint32_t ui32DrmEnabled;              /* 0: DRM Disable
                                             1: DRM Enable
                                         */
    uint32_t ui32PcpPayloadSize;         /* if Drm is disabled, host will set this field as 0, FW can ignore */

    uint32_t ui32NumberofBytesToSkip; /* This field indicates to FW to skip the number of bytes from the start of the address. The skipped address will be the
                                         start of real AV data payload
                                      */

    /* PCP Header required information */
    uint32_t ui32PcpHeader0;         /*[07:06]: Packet_Type [00 for PCP]
                                      *[05:04]: C_A2/C_A
                                      *[03:00]: E-EMI
                                     */
    uint32_t ui32ExchangeKeyLabel;   /* Non-zero label means encrypted content */
    uint32_t ui32Nc[2];              /* Nonce for content channel (64 bit) */
} BASP_PayloadConsumedMessage;

/***************************************************************************
Summary:
Message for PI to FW on Stream In Mode Punting function. This message
is sent to FW after ASP PI has consumed the data sent to it.
****************************************************************************/
typedef struct BASP_FrameConsumedMessage
{
    BASP_DramBufferInfo FrameConsumedInfo;
} BASP_FrameConsumedMessage;

/***************************************************************************
Summary:
FinNotify message from FW to PI/app, when asp receives FIN from remote host.
****************************************************************************/
typedef struct BASP_FinNotifyMessage
{
    /* Sequence number of the FIN Packet */
    uint32_t ui32SequenceNumber;
    /* Size of the receive Window in the FIN packet */
    uint32_t ui32WindowSize;
} BASP_FinNotifyMessage;

/***************************************************************************
Summary:
RstNotify message from FW to PI/app, when asp receives RST from remote host.
****************************************************************************/
typedef struct BASP_RstNotifyMessage
{
    uint32_t ui32Unused;
} BASP_RstNotifyMessage;

/***************************************************************************
Summary:
RTO message from FW to PI/app, when asp has tried the maximum times of retx.
****************************************************************************/
typedef struct BASP_RtoNotifyMessage
{
    uint32_t ui32NumRetxMade; /* Number of Retransmission carried out and still no reply */
    uint32_t ui32CurrRtoTime; /* The exponential back-off RTO time in milliseconds */
} BASP_RtoNotifyMessage;

/***************************************************************************
Summary:
PacketAvailable message from FW to PI/app, when asp( Server ) received data  from
remote client, Ex: another get request from client .
****************************************************************************/
typedef struct BASP_FrameAvailableMessage
{
    BASP_DramBufferInfo FrameAvailableInfo;
} BASP_FrameAvailableMessage;

/***************************************************************************
Summary:
Channel Fin complete message. This is just like stop response message,
but this is actually not a response this is a FW generated message in  response to the
Remote Client Initiated Connection Termination. Since in this case FW stops the channel
by itself. and once it is done it sends the BASP_P_ChannelFinComplete message.
****************************************************************************/
typedef struct BASP_ChannelFinCompleteMessage
{
   /* last sequence number seen by firmware */
    uint32_t ui32CurrentSeqNumber;

    /* last Acked sequence number seen by firmware */
    uint32_t ui32CurrentAckedNumber;

} BASP_ChannelFinCompleteMessage;

/***************************************************************************
Summary:
HTTP Response for Stream In Message from FW to PI. FW informs PI on
the location of the HTTP Response
****************************************************************************/
typedef struct BASP_PayloadNotify
{
    BASP_DramBufferInfo HttpResponseAddress;
} BASP_PayloadNotify;

/***************************************************************************
Summary:
HTTP Response for Stream In Message from FW to PI. FW informs PI on
the location of the HTTP Response
****************************************************************************/
typedef struct BASP_MaxRetransRtoNotify
{
    uint32_t MaxNumberOfRetransDone;    /* Number of Retransmission carried out and still no reply */
    uint32_t RetransTimeOut;            /* The exponential backoff RTO time in milliseconds */
} BASP_MaxRetransRtoNotify;

/***************************************************************************
Summary:
Init completion response.
****************************************************************************/
typedef struct BASP_InitResponse
{
    uint32_t ResponseStatus;
    uint32_t aspFwPlatformVersion;
    uint32_t aspFwVersion;
} BASP_InitResponse;

/***************************************************************************
Summary:
UnInit completion response.
****************************************************************************/
typedef struct BASP_UnInitResponse
{
    uint32_t ResponseStatus;
} BASP_UnInitResponse;

/***************************************************************************
Summary:
Channel start Stream-Out completion response.
****************************************************************************/
typedef struct BASP_ChannelStartStreamOutResponse
{
    uint32_t ResponseStatus;
} BASP_ChannelStartStreamOutResponse;

/***************************************************************************
Summary:
Channel start Stream-In completion response.
****************************************************************************/
typedef struct BASP_ChannelStartStreamInResponse
{
    uint32_t ResponseStatus;
} BASP_ChannelStartStreamInResponse;

/***************************************************************************
Summary:
Generic SG Table Feed Response.
****************************************************************************/
typedef struct BASP_GenericSGTableFeedResponse
{
    uint32_t ResponseStatus;
} BASP_GenericSGTableFeedResponse;

/***************************************************************************
Summary:
Payload Consumed Response.
****************************************************************************/
typedef struct BASP_PayloadConsumedResponse
{
    uint32_t ResponseStatus;
} BASP_PayloadConsumedResponse;

/***************************************************************************
Summary:
Packet Consumed Response.
****************************************************************************/
typedef struct BASP_FrameConsumedResponse
{
    uint32_t ResponseStatus;
} BASP_FrameConsumedResponse;

/***************************************************************************
Summary:
Channel stop completion response.
****************************************************************************/
typedef struct BASP_ChannelStopResponse
{
    uint32_t ResponseStatus;

    /* last sequence number seen by firmware */
    uint32_t ui32CurrentSeqNumber;

    /* last Acked sequence number seen by firmware */
    uint32_t ui32CurrentAckedNumber;

} BASP_ChannelStopResponse;

/***************************************************************************
Summary:
Local initiated Channel RST message completion response.
****************************************************************************/
typedef struct BASP_ChannelAbortResponse
{
    uint32_t ResponseStatus;

    /* last sequence number seen by firmware */
    uint32_t ui32CurrentSeqNumber;

    /* last Acked sequence number seen by firmware */
    uint32_t ui32CurrentAckedNumber;

} BASP_ChannelAbortResponse;

/***************************************************************************
Summary:
Local initiated Channel RST message completion response with RST.
****************************************************************************/
typedef struct BASP_ChannelAbortWithRstResponse
{
    uint32_t ResponseStatus;

    /* last sequence number seen by firmware */
    uint32_t ui32CurrentSeqNumber;

    /* last Acked sequence number seen by firmware */
    uint32_t ui32CurrentAckedNumber;

} BASP_ChannelAbortWithRstResponse;

/***************************************************************************
Summary:
Performance Gathering Response from FW to PI. The list may change
or grow depending on information required. But we will settle for
these items now.
****************************************************************************/
typedef struct BASP_PerformanceGatheringResponse
{
   uint32_t ResponseStatus;
} BASP_PerformanceGatheringResponse;

/*****************************************************************************
Summary:
This is a response message for GenericSGTableFeed message from host.
In the Feed message, host will have sent X number of entries to FW.
After FW has consumed all the entries and generated descriptors to MCPB,
FW sends this response message pointing the last address of the data
fed it has consumed.
******************************************************************************/
typedef struct BASP_GenericSGTableCompletionResponse
{
    uint32_t ResponseStatus;
    uint32_t ui32AddrLo;      /* The last entry address that ASP FW has consumed from the number of entries of addresses that host passed to it.*/
    uint32_t ui32AddrHi;      /* High address*/
} BASP_GenericSGTableCompletionResponse;

/***************************************************************************
Summary:
Response message of BASP_P_GetDrmConstMessage.
****************************************************************************/
typedef struct BASP_GetDrmConstMessageResponse
{
    uint32_t ResponseStatus;
} BASP_GetDrmConstMessageResponse;

/***************************************************************************
Summary:
Host to FW messages.
****************************************************************************/
typedef struct BASP_Pi2Fw_Message
{
    BASP_MessageHeader MessageHeader;
    union
    {
        BASP_InitMessage                       Init;
        BASP_UnInitMessage                     UnInit;
        BASP_ChannelStartStreamOutMessage      ChannelStartStreamOut;
        BASP_ChannelStartStreamInMessage       ChannelStartStreamIn;
        BASP_ChannelStopMessage                ChannelStop;
        BASP_ChannelAbortWithRstMessage        ChannelAbortWithRst;
        BASP_ChannelAbortMessage               ChannelAbort;
        BASP_PerformanceGatheringMessage       PerformanceGathering;
        BASP_GenericSGTableFeedMessage         GenericSGTableFeed;
        BASP_PayloadConsumedMessage            PayloadConsumed;
        BASP_FrameConsumedMessage              FrameConsumed;
        BASP_GetDrmConstMessage                GetDrmConst;
    } MessagePayload;
} BASP_Pi2Fw_Message;

/***************************************************************************
Summary:
FW to Host messages.
****************************************************************************/
typedef struct BASP_Fw2Pi_Message
{
    BASP_MessageHeader MessageHeader;
    /* We can split the FW2PI Message and response in two separate buffer provided we have two separate queues from FW2PI */
    union
    {
        union
        {
            BASP_FinNotifyMessage                    FinNotify;
            BASP_RstNotifyMessage                    RstNotify;
            BASP_RtoNotifyMessage                    RtoNotify;
            BASP_FrameAvailableMessage               FrameAvailable;
            BASP_PayloadNotify                       PayloadNotify;
        } MessagePayload;

        union
        {
            BASP_InitResponse                       InitResponse;
            BASP_UnInitResponse                     UnInitResponse;
            BASP_ChannelStartStreamOutResponse      StartStreamOutResponse;
            BASP_ChannelStartStreamInResponse       StartStreamInResponse;
            BASP_ChannelStopResponse                StopResponse;
            BASP_ChannelAbortWithRstResponse        AbortWithRstResponse;
            BASP_ChannelAbortResponse               AbortResponse;
            BASP_PerformanceGatheringResponse       PerformanceGatherResponse;
            BASP_GenericSGTableFeedResponse         GenericSGTableFeedResponse;
            BASP_PayloadConsumedResponse            PayloadConsumedResponse;
            BASP_FrameConsumedResponse              PacketConsumedResponse;
            BASP_GenericSGTableCompletionResponse   GenericSGTableCompletionResponse;
            BASP_GetDrmConstMessageResponse         GetDrmConstMessageResponse;
        } ResponsePayload;
    } Message;
} BASP_Fw2Pi_Message;

/***************************************************************************
Summary:
RA to FW messages.
****************************************************************************/
typedef struct BASP_Ra2Fw_Message
{
    BASP_MessageHeader MessageHeader;
    union
    {
        BASP_ReassembledPktMessage ReassembledPkt;
    } MessagePayload;
} BASP_Ra2Fw_Message;

/***************************************************************************
Summary:
FW to RA messages.
****************************************************************************/
typedef struct BASP_Fw2Ra_Message
{
    BASP_MessageHeader MessageHeader;

    /* We can split the FW2PI Message and response in two separate buffer provided we have two separate queues from FW2PI */
    union
    {
        union
        {
            BASP_ReassembledPktCompletion ReassembledPktCompletion;
        } MessagePayload;

        union
        {
            BASP_ReassembledPktResponse ReassembledPktResponse;
        } ResponsePayload;
    } Message;
} BASP_Fw2Ra_Message;

/***************************************************************************
Summary:
SAGE to FW messages.
****************************************************************************/
typedef struct BASP_SageConstInfo
{
    uint32_t ui32StreamLabel;    /* Compare this number with the number from Host */
    uint32_t ui32IVc[2];         /* Initialization vector constant for AES-128 */
    uint32_t ui32ConstKeyCa0[3]; /* universal secret constants from DTLA */
    uint32_t ui32ConstKeyCb1[3]; /* universal secret constants from DTLA */
    uint32_t ui32ConstKeyCb0[3]; /* universal secret constants from DTLA */
    uint32_t ui32ConstKeyCc1[3]; /* universal secret constants from DTLA */
    uint32_t ui32ConstKeyCc0[3]; /* universal secret constants from DTLA */
    uint32_t ui32ConstKeyCd0[3]; /* universal secret constants from DTLA */
} BASP_SageConstInfo;

typedef struct BASP_Fw2Sage_ReqConst
{
    BASP_MessageHeader MessageHeader;
} BASP_Fw2Sage_ReqConst;

typedef struct BASP_Sage2Fw_ConstInfo
{
    BASP_MessageHeader MessageHeader;
    union
    {
        BASP_SageConstInfo SageToFWConstInfo;
    } MessagePayload;
} BASP_Sage2Fw_ConstInfo;

typedef struct BASP_Fw2Sage_ReqConstResponse
{
    BASP_MessageHeader MessageHeader;
    uint32_t ResponseStatus;
} BASP_Fw2Sage_ReqConstResponse;

typedef struct BASP_FwPerformanceInfo
{
    uint32_t ui32ChNum;
    uint32_t ui32NumChannelsAlive;       /* Number of MCPB channels enabled */
    uint32_t ui32NumBytesSentPerChannelLo; /* Number of bytes sent in this channel */
    uint32_t ui32NumBytesSentPerChannelHi; /* Number of bytes sent in this channel */
    uint32_t ui32UnimacErrorCodes;       /* Unimac error codes */
    uint32_t ui32XonXoffStatus;          /* Xon/Xoff Status */
    uint32_t ui32AnyBuffersOverflow;     /* 0: No buffer overflow
                                       1: One of the buffers overflow
                                    */
    uint32_t ui32TransmitWindowSize;     /* Window Size for Tx Part */
    uint32_t ui32EthernetPacketCounts;   /* Inflight packets */
    uint32_t ui32TotalNumEPKTSent;       /* In the channel */
    uint32_t ui32SwitchPauseStatus;      /* How to get this information? */
    uint32_t ui32EpktStatus;             /* Refer to Arch Specs */
    uint32_t ui32EdpktStatus;            /* Refer to Arch Specs */
    uint32_t ui32WindowPause;
    uint32_t ui32DMACommittedbytes;      /* Read from MCPB register */
    uint32_t ui32BufferDepth;            /* Which buffer */
    uint32_t ui32HwChannelProcessing;    /* EPKT/EDPKT Arch spec section8.2.4 */
    uint32_t ui32TimeStampValLo;         /* TimeStamp Counter from EPKT and EDPKT (64bits) */
    uint32_t ui32TimeStampValHi;         /* TimeStamp Counter from EPKT and EDPKT (64bits) */
    uint32_t ui32TimeStampEcho;          /* TimeStamp Echo value per channel */
    uint32_t ui32RoundTripTime;          /* Round Trip Time calculated */

} BASP_FwPerformanceInfo;

/* Mailboxes for each task */
typedef uint32_t volatile mailbox_t;

/* DO NOT change the order here, it is important it remains in the same location */
/* Each task to initialize this in the respective init function */
    /* Bit 31:24 = channel number
     * Bit 23:00 = Task state value
     */
typedef struct BASP_Mailboxes
{
    mailbox_t ui32GlobalStatus; /* RHP init and use by all. */
    mailbox_t ui32RhpStatus;    /* Receive header processing task to init and use this. */
    mailbox_t ui32MiStatus;     /* Manager interaction task to init and use this. */
    mailbox_t ui32FsStatus;     /* Frame sender task to init and use this. */
    mailbox_t ui32SfStatus;     /* Stream feeder task to init and use this. */
    mailbox_t ui32SkpStatus;    /* Sender key prepare task to init and use this. */
    mailbox_t ui32WdtStatus;    /* Watch dog task to init and use this. A non-zero value indicate WDT is initialized and represent number to times this task runs */
} BASP_Mailboxes;

/* Channel state for all tasks */
typedef enum ChState_e
{
    CHAN_STATE_INVALID         = 0, /* ASP has not been initialized yet */
    CHAN_STATE_INIT_STOP       = 1, /* ASP init is done and channel is inactive */
    CHAN_STATE_HTTP_RESP_SEND  = 2, /* Channel enabled for stream-out, to send http response */
    CHAN_STATE_START_STREAMOUT = 3, /* Channel enabled for stream-out, start streaming */
    CHAN_STATE_PASS_THROUGH    = 4, /* Frame pass-through mode */
    CHAN_STATE_HTTP_INIT       = 5, /* Channel enabled for stream-in, to send http request */
    CHAN_STATE_START_STREAMIN  = 6, /* Channel enabled for stream-in, start streaming */
    CHAN_STATE_MAX
} ChState_e;

/* RHP States defined for State Machine */
typedef enum RHPSMState_e
{
    RHP_IDLE = 0,          /* RHP at IDLE */
    RHP_SLOW_START,
    RHP_CONGESTION_AVOIDANCE,
    RHP_FAST_RETX,
    RHP_FAST_RECOVERY,
    RHP_KA,
    RHP_PERSIST,
    RHP_RST,
    RHP_FIN,
    RHP_CH_STOP,           /* Channel Stop State */
    RHP_MAX
} RHPSMState_e;

/* This is the per-channel debug info structure that FW periodically sends to
 * debug buffer given in asp init message.
 */
typedef struct BASP_FwChannelInfo
{
    /* Channel number */
    uint32_t ui32ChNum;

    /* Channel state, defined as in ChState_e. */
    ChState_e ui32ChannelState;

    /* Channel mode:
     *   Bit[12]: TCP option SACK enable;
         Bit[11]: TCP option timestamp enable;
         Bit[10:8]: Parser Stream Type (MPEG:0/DSS:1/BLOCK:6);
         Bit[7]: Congestion control enable;
         Bit[6]: MUX enable;
         Bit[5]: Bypass enable;
         Bit[4:2]: DRM type;
         Bit[1:0]: HTTP connection type;
     */
    uint32_t ui32ChannelMode;

    /* Current message counter */
    uint32_t ui32MessageCounter;

    /* Latest message type received from host */
    uint32_t ui32ReceivedMessageType;

    /* Latest message type sent to host */
    uint32_t ui32SentMessageType;

    /* The timestamp when this channel info is gathered.
     * We used the upper 32-bit of ASP_EPKT_CORE_TX_TS_48 timer as this timestamp.
     * Each tick of this timestamp equals to 4ms under current setting. */
    uint32_t ui32TimeStamp;

    /* Info from ASP Tx path */

    /* Current congestion window, calculated from RFC 5681 */
    uint32_t ui32CongestionWindow;

    /* Current ASP window */
    uint32_t ui32SendWindow;

    /* Current send sequence number from ASP_EPKT_CORE_CHxx_CH_SEQ_NUM. */
    uint32_t ui32SequenceNum;

    /* Current Retx sequence number from ASP_EPKT_CORE_CHxx_CH_RETX_SEQ_NUM. */
    uint32_t ui32RetxSequenceNum;

    /* Current max sequence number from ASP_MCPB_CHx_ESCD_TCPIP_MAX_SEQUENCE_NUMBER. */
    uint32_t ui32MaxSequenceNum;

    /* Current send ack sequence number. */
    uint32_t ui32AckSequenceNumber;

    /* Number of total data bytes sent, calculated using ASP_MCPB_CHx_DCPM_LOCAL_PACKET_COUNTER */
    /*uint64_t ui32NumOfMcpbBytesSent;*/

    /* Number of IP packet sent (estimated) */
    uint32_t ui32NumOfIpPktsSent;

    /* Number of duplicated retx made on current retx sequence number */
    uint32_t ui32NumOfDuplCurrRetx;

    /* Number of total retx made */
    uint32_t ui32NumOfTotalRetx;

    /* ASP_UMAC_GTUC - Transmit Unicast Packet Counter */
    uint32_t ui32NumOfUnicastPktSent;

    /* Info from ASP Rx path */

    /* Number of total packets received */
    uint32_t ui32NumReceivedPkts;

    /* Number of total packets dropped */
    uint32_t ui32NumPktDropped;

    /* Number of data packets received */
    uint32_t ui32NumReceivedDataPkts;

    /* Number of data packets dropped */
    uint32_t ui32NumDataPktDropped;

    /* Current received sequence number. */
    uint32_t ui32ReceivedSequenceNumber;

    /* Current received ack sequence number. */
    uint32_t ui32ReceivedAckSequenceNumber;

    /* Current received window size. */
    uint32_t ui32ReceivedWindowSize;

    /* Latest timestamp value received */
    uint32_t ui32ReceivedTimestamp;

    /* ASP_UMAC_GRUC - Receive Unicast Packet Counter */
    uint32_t ui32NumOfUnicastPktReceived;

    /* Number of DRAM descriptor sent to MCPB */
    uint32_t ui32NumOfDescriptorSent;

    /* Number of bytes fed to descriptors */
    uint32_t ui32NumBytesFedToDescriptors;

    /* collect fw stats for number of spans used*/
    uint32_t ui32NumSpansUsed;

    /* collect fw stats for span full info
     * 0 = Not full
     * 1 = full
    */
    uint32_t ui32IsSpanFull;

    /* un-scaled sent window size (for stream-in mainly)*/
    uint32_t ui32UnScaledSentWinSize;

    /* Number of retransmitted packets received */
    uint32_t ui32NumRetransPktReceived;

    /* Number of retransmitted bytes received */
    uint32_t ui32NumRetransBytesReceived;

    /* Number of Retrans Pkt sent due to 3dup */
    uint32_t ui32NumRetransPktDueTo3Dup;

    /* Number of Retrans Pkt sent due to RTO */
    uint32_t ui32NumRetransPktDueToRto;

    /* Number of Retrans Pkt sent due to Sack */
    uint32_t ui32NumRetransPktDueToSack;

    /* Number of Retrans Pkt sent at Fast Retx State */
    uint32_t ui32NumRetransPktAtFastRetx;

    /* Number of Retrans Pkt sent at Fast Recovery State */
    uint32_t ui32NumRetransPktAtFastRecov;

    /* Updated PA1 Address */
    uint32_t ui32UpdatedPa1Addr;

} BASP_FwChannelInfo;

typedef struct BASP_FwStatusInfo
{
    /* The per-channel info structure has to come first to simplify the address calculation
     * for an arbitery channel no.
     */
    BASP_FwChannelInfo stChannelInfo[BASP_MAX_AVFLOWS];
    BASP_Mailboxes stTaskStatus;
} BASP_FwStatusInfo;

typedef struct Fw2HostPrintBuffer_t
{
    uint32_t ui32ReadPtr;
    uint32_t ui32WritePtr;
    uint8_t ui8CharBuffer[BASP_FW2H_PRINT_BUFFER_SIZE];
} Fw2HostPrintBuffer_t;

#endif /* ifndef BASP_FW_API_H_*/
