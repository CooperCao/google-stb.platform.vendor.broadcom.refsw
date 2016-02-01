/******************************************************************************
 * (c) 2003-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/


#ifndef BXPT_PRIV_H__
#define BXPT_PRIV_H__

#include "breg_mem.h"
#include "bint.h"
#include "bmem.h"
#include "bavc.h"
#include "bchp.h"
#include "berr_ids.h"
#include "bxpt.h"
#include "bavc.h"
#include "bxpt_rave.h"
#include "bdbg.h"
#include "bxpt_pcr.h"

BDBG_OBJECT_ID_DECLARE(bxpt_t); /* this is the object identifier to validate an XPT handle */
/* Note: it is not necessary to have a different one for each type of handle because the typechecking will ensure
** that the parameter is of the right type (unless they typecast incorrectly).
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Address delta (in bytes) between elements in a register array. */
#define BP_TABLE_STEP                   ( 4 )
#define RP_TABLE_STEP                   ( 4 )
#define PID_TABLE_STEP                  ( 4 )
#define PID_CTRL1_TABLE_STEP            ( 4 )
#define PID_CTRL2_TABLE_STEP            ( 4 )
#define FILTER_WORD_STEP                ( 32 )
#define GEN_FILT_EN_STEP                ( 4 )
#define PID2BUF_TABLE_STEP              ( 4 )

#define XPT_REG_SIZE_BITS               ( 32 )
#define XPT_REG_SIZE_BYTES              ( 4 )

#define BXPT_P_NUM_PIC_COUNTERS         ( 6 )

#define BXPT_P_MAX_XC_BUFFERS           ( 60 )
#define BXPT_P_DEFAULT_PEAK_RATE        ( 25000000 )


/***************************************************************************
Summary:
Used to define the flags inside a linked list descriptor.
****************************************************************************/
#define TRANS_DESC_INT_FLAG             ( 1ul << 31 )
#define TRANS_DESC_FORCE_RESYNC_FLAG    ( 1ul << 30 )
#define TRANS_DESC_LAST_DESCR_IND       ( 1ul << 0 )

/***************************************************************************
Summary:
Used to maintain a list of which message buffers have been allocated by
porting interface, rather than the caller.
****************************************************************************/

typedef struct
{
    bool IsAllocated;   /* true if PI allocated the memory. */
    uint32_t Address;   /* address of the buffer */
    unsigned OutputMode;    /* Type of filtering this buffer is doing */
}
MessageBufferEntry;

/***************************************************************************
Summary:
Used to maintain a list of which PID channels are allocated and what parser
band and PID are assigned to that channel.
****************************************************************************/

typedef struct
{
    bool IsAllocated;               /* true if allocated. */
    bool IsPidChannelConfigured;    /* has the Pid and Band values been set. */
    bool HasMessageBuffer;          /* true if hardware message buffer is associated with this channel. */
    unsigned int Pid;
    unsigned int Band;

    bool EnableRequested;           /* Has a request to enable this PID channel been recieved? */
#ifdef ENABLE_PLAYBACK_MUX
    bool MuxEnable;
    bool HasDestination;
#endif /*ENABLE_PLAYBACK_MUX*/

    uint32_t MessageBuffercount;
}
PidChannelTableEntry;


/***************************************************************************
Summary:
Used to keep track of which PSI filters have been allocated, and to what
PID.
****************************************************************************/
typedef struct
{
    bool IsAllocated;       /* Is someone using this? */
}
FilterTableEntry;

/***************************************************************************
Summary:
Handle for accessing the record API via a channel. Users should not directly
access the contents of the structure.
****************************************************************************/

typedef struct BXPT_P_PbHandle
{
    BDBG_OBJECT(bxpt_t) /* used to check if structure is valid */

    BCHP_Handle hChip;              /* [Input] Handle to used chip. */
    BREG_Handle hRegister;          /* [Input] Handle to access regiters. */
    BMEM_Handle hMemory;            /* [Input] Handle to memory heap to use. */
    void *vhXpt;                    /* Pointer to parent transport handle. Requires casting. */

    uint8_t ChannelNo;      /* This channels number. */
    uint32_t BaseAddr;  /* Address of the first register in this block. */
    uint32_t LastDescriptor;    /* Address of the last descriptor on the linked list. */
    bool Opened;
    bool Running;
    unsigned int SyncMode;
    BINT_CallbackHandle hPbIntCallback; /* Callback Handle to service the ISR */
    bool AlwaysResumeFromLastDescriptor;
    bool ForceResync;

    BXPT_PvrDescriptor *DummyDescriptor;
    uint8_t *DummyBuffer;
    bool ResetPacingOnTsRangeError;
}
BXPT_P_PbHandle;

/***************************************************************************
Summary:
Handle for accessing the remux API via a channel. Users should not directly
access the contents of the structure.
****************************************************************************/

typedef struct BXPT_P_RemuxHandle
{
    BDBG_OBJECT(bxpt_t) /* used to check if structure is valid */

    void *vhXpt;            /* Pointer to parent transport handle. Requires casting. */

    BCHP_Handle hChip;              /* [Input] Handle to used chip. */
    BREG_Handle hRegister;          /* [Input] Handle to access regiters. */
    BMEM_Handle hMemory;            /* [Input] Handle to memory heap to use. */

    bool Opened;
    bool Running;

    uint8_t ChannelNo;      /* This channels number. */
    uint32_t BaseAddr;  /* Address of the first register in this block. */
    bool BufferIsAllocated;
    void *BufferAddress;

#if BXPT_HAS_REMUX_PID_REMAPPING
    bool PidMapEntryAllocated[ BXPT_P_MAX_REMUX_PID_MAPS ];
#endif

}
BXPT_P_RemuxHandle;

/***************************************************************************
Summary:
Handle for accessing the packet substution API via a channel. Users should
not directly access the contents of the structure.
****************************************************************************/

typedef struct BXPT_P_PacketSubHandle
{
    BDBG_OBJECT(bxpt_t) /* used to check if structure is valid */

    BCHP_Handle hChip;              /* [Input] Handle to used chip. */
    BREG_Handle hRegister;          /* [Input] Handle to access regiters. */
    BMEM_Handle hMemory;            /* [Input] Handle to memory heap to use. */
    void *vhXpt;

    uint8_t ChannelNo;      /* This channels number. */
    uint32_t BaseAddr;      /* Address of the first register in this block. */
    uint32_t LastDescriptor;    /* Address of the last descriptor on the linked list. */
    bool Opened;
    bool Running;
}
BXPT_P_PacketSubHandle;

typedef struct BXPT_P_PcrHandle_Impl
{
    BDBG_OBJECT(bxpt_t) /* used to check if structure is valid */

    BCHP_Handle hChip;              /* [Input] Handle to used chip. */
    BREG_Handle hRegister;          /* [Input] Handle to access regiters. */
    void *vhXpt;

    uint8_t ChannelNo;              /* This channels number. */
    uint32_t RegOffset;            /* reg Offset of the given module. */
    bool DMode;
}
BXPT_P_PcrHandle_Impl;

typedef struct BXPT_P_InterruptCallbackArgs
{
    BINT_CallbackFunc   Callback;   /* Callback for this interrupt. */
    void *Parm1;                    /* User arg for the callback. */
    int Parm2;                      /* User arg for the callback. */
}
BXPT_P_InterruptCallbackArgs;

typedef struct ParserConfig
{
    bool SaveShortPsiMsg;
    bool SaveLongPsiMsg;
    bool PsfCrcDis;
    BXPT_PsiMesgModModes PsiMsgMod;
}
ParserConfig;

/***************************************************************************
Summary:
Handles for accessing and controlling the RAVE and it's contexts.
****************************************************************************/

typedef struct BXPT_P_IndexerHandle
{
    BDBG_OBJECT(bxpt_t) /* used to check if structure is valid */

    BXPT_RaveIdx ChannelType;
    void *hChannel;
    bool Allocated;
    unsigned NumChannels;
    void *vhCtx;
}
BXPT_P_IndexerHandle;

typedef struct StartCodeIndexer
{
    BREG_Handle hReg;
    BMEM_Handle hMem;
    BINT_Handle hInt;
    uint32_t BaseAddr;
    unsigned ChannelNo;
    bool Allocated;
}
StartCodeIndexer;

typedef enum brave_itb_types {
    brave_itb_video_start_code =     0x00,
    brave_itb_base_address =         0x20,
    brave_itb_pts_dts =              0x21,
    brave_itb_pcr_offset =           0x22,
    brave_itb_btp =                  0x23,
    brave_itb_private_hdr =          0x24,
    brave_itb_rts =                  0x25,
    brave_itb_pcr =                  0x26,
    brave_itb_ip_stream_out =        0x30,
    brave_itb_termination =          0x70
} brave_itb_types;

typedef enum eDynamic_splice_btp_marker_commands
{
    brave_itb_splice_pts_marker = 0x0B ,
    brave_itb_splice_start_marker = 0x12,
    brave_itb_splice_stop_marker = 0x13,
    brave_itb_splice_transition_marker = 0x14,
    brave_itb_splice_pcr_offset_marker = 0x15
}
eDynamic_splice_btp_marker_command;

typedef enum eDynamic_splice_state
{
    SoftRave_SpliceState_Copy = 0,
    SoftRave_SpliceState_Discard = 1
}
eDynamic_splice_state;

typedef struct SoftRaveData
{
    size_t ItbSize;
    uint32_t SrcBaseAddr;
    unsigned mode;
    uint32_t last_base_address;
    uint8_t *src_itb_mem, *dest_itb_mem;
    uint32_t last_src_itb_valid, last_dst_itb_valid;
    uint32_t src_itb_base, dest_itb_base;

    uint32_t last_pts_dts;
    uint32_t last_sc;


    bool b_frame_found;
    uint32_t last_dest_valid, flush_cnt;
    uint32_t* P_frame_pts;
    brave_itb_types last_entry_type;

    bool adjust_pts;
    bool insufficient_itb_info;
    bool sequence_hdr_found;
    bool prev_sequence_hdr_found;

    bool discard_till_next_gop;
    bool discarding;
    uint32_t splice_start_PTS;
    uint32_t splice_start_PTS_tolerance;
    uint32_t splice_stop_PTS;
    uint32_t splice_stop_PTS_tolerance;
    uint32_t splice_monitor_PTS;
    uint32_t splice_monitor_PTS_tolerance;
    uint32_t splice_pcr_offset;
    uint32_t splice_pts_offset;
    uint32_t splice_last_pcr_offset;
    bool  SpliceStartPTSFlag;
    bool  SpliceStopPTSFlag;
    bool  SpliceMonitorPTSFlag;
    bool  SpliceModifyPCROffsetFlag;
    bool  SpliceModifyPTSFlag;
    eDynamic_splice_state splice_state;
    uint32_t Splice_Ctx_num;
    void (*SpliceStopPTSCB) ( void *, uint32_t pts);
    void (*SpliceStartPTSCB) ( void *,uint32_t pts);
    void (*SpliceMonitorPTSCB) ( void *, uint32_t pts);
    void * SpliceStopPTSCBParam;
    void * SpliceStartPTSCBParam;
    void * SpliceMonitorPTSCBParam;
    bool InsertStartPts;
    bool InsertStopPts;
    bool StartMarkerInserted;
    bool StopMarkerInserted;

    unsigned SrcContextIndex;
    bool SrcIsHeld;
}
SoftRaveData;

typedef struct BXPT_P_ContextHandle
{
    BDBG_OBJECT(bxpt_t) /* used to check if structure is valid */
    BCHP_Handle hChp;
    BREG_Handle hReg;
    BMEM_Handle hMem;
    BINT_Handle hInt;
    BMEM_Handle hRHeap;
    void *vhRave;

    unsigned Type;
    uint32_t BaseAddr;
    bool Allocated;
    unsigned Index;
    bool CdbReset;
    bool ItbReset;
    bool CdbReadReset;
    bool ItbReadReset;

    bool HaveSpliceQueue;
    unsigned SpliceQueueIdx;

    BAVC_StreamType InputFormat;
    BAVC_ItbEsType ItbFormat;
    StartCodeIndexer *hAvScd;
    int PicCounter;     /* Offset into table of Pic Counter registers */
    bool Transcoding;
    uint32_t allocatedCdbBufferSize; /* Total CDB buffer allocated by AllocContext */
    uint32_t usedCdbBufferSize;      /* actual CDB buffer used */
    void *CdbBufferAddr;             /* save the cdb allocated bufffer adddress */
    void *ItbBufferAddr;             /* save the itb allocated buffer address allocated */
    bool externalCdbAlloc;
    bool externalItbAlloc;

    /* For streaming IP support only. Don't use in AV or REC processing */
    bool IsMpegTs;      /* True if context is handling MPEG TS, false if DirecTV */

    /* For 7401 only */
    bool VobMode;

    bool IsSoftContext;
    SoftRaveData SoftRave;

    StartCodeIndexer *hVctScd;
    void *VctNeighbor;

    /* PR57627 :
    ** SC_OR_MODE is used to select the way scramble control bits are reported.
    ** 0 = Disable OR-ing of current and previous scramble control bits (Default).
    ** 1 = Enable OR-ing of current and previous scramble control bits. This is to
    ** support streams which have mixture of scrambled and unscrambled packets within
    ** the same PID. In such case, these PIDs will be treated as scramble PIDs.
    ** By default this is disabled.
    */
    bool ScOrMode;
}
BXPT_P_ContextHandle;

typedef struct CtxIntData
{
    BLST_S_ENTRY( CtxIntData ) Link;

    BINT_CallbackFunc   Callback;   /* Callback for this interrupt. */
    void *Parm1;                    /* User arg for the callback. */
    int Parm2;                      /* User arg for the callback. */
    uint32_t EnableRegAddr;
    uint32_t StatusRegAddr;
}
CtxIntData;

typedef struct TpitIndexer
{
    BREG_Handle hReg;
    BMEM_Handle hMem;
    BINT_Handle hInt;
    uint32_t BaseAddr;
    uint32_t PidTableBaseAddr;
    uint32_t ParseTableBaseAddr;
    unsigned ChannelNo;
    bool Allocated;
}
TpitIndexer;

#define BXPT_P_MAX_PIC_COUNTER  ( 5 )

typedef struct BXPT_P_RaveHandle
{
    BDBG_OBJECT(bxpt_t) /* used to check if structure is valid */

    BCHP_Handle hChip;
    BREG_Handle hReg;
    BMEM_Handle hMem;
    BINT_Handle hInt;
    void *lvXpt;
    unsigned ChannelNo;

    BXPT_P_ContextHandle ContextTbl[ BXPT_P_MAX_RAVE_CONTEXTS ];
    bool SpliceQueueAllocated[ BXPT_P_NUM_SPLICING_QUEUES ];
    StartCodeIndexer ScdTable[ BXPT_P_MAX_SCD ];
    TpitIndexer TpitTable[ BXPT_P_MAX_TPIT ];
    BXPT_P_IndexerHandle IndexerHandles[ BXPT_P_MAX_SCD + BXPT_P_MAX_TPIT ];
    bool PicCounterUsed[ BXPT_P_MAX_PIC_COUNTER ];
    RaveChannelOpenCB chanOpenCB;
}
BXPT_P_RaveHandle;

typedef struct RaveChannel
{
    void *Handle;
    bool Allocated;
}
RaveChannel;

typedef struct BXPT_P_PcrOffset_Impl
{
    BDBG_OBJECT(bxpt_t) /* used to check if structure is valid */

    BCHP_Handle hChip;
    BREG_Handle hReg;
    BMEM_Handle hMem;
    unsigned int ChannelNo;
    uint32_t BaseAddr;
    void *lvXpt;
    uint32_t CurrentTimeBase;
    unsigned PidChannelNum;
    unsigned WhichStc;
    bool UseHostPcrs;
}
BXPT_P_PcrOffset_Impl;

typedef struct PcrOffsetData
{
    void *Handle;
    bool Allocated;
}
PcrOffsetData;

/***************************************************************************
Summary:
The handle for the transport module. Users should not directly access the
contents.
****************************************************************************/
typedef struct BXPT_P_TransportData
{
    BDBG_OBJECT(bxpt_t) /* used to check if structure is valid */

    BCHP_Handle hChip;              /* [Input] Handle to used chip. */
    BREG_Handle hRegister;          /* [Input] Handle to access regiters. */
    BMEM_Handle hMemory;            /* [Input] Handle to memory heap to use. */
    BINT_Handle hInt;               /* [Input] Handle to interrupt interface to use. */

    /* Handle for secure memory allocations. */
    BMEM_Handle hRHeap;
    BMMA_Heap_Handle mmaHeap, mmaRHeap; /* hMemory = mmaHeap and hRHeap = mmaRHeap. for compatibility with new API */

    BINT_CallbackHandle hMsgCb;     /* Callback handle for message interrupts */
    BINT_CallbackHandle hMsgOverflowCb; /* Callback handle for message overflow interrupts */

    unsigned int MaxPlaybacks;          /* Number of playback blocks we support. */
    unsigned int MaxPidChannels;        /* Number of PID channels we support. */
    unsigned int MaxPidParsers;         /* Number of PID parsers we support. */
    unsigned int MaxInputBands;         /* Max number of input bands we support. */
    unsigned int MaxFilterBanks;        /* Max number of filter banks */
    unsigned int MaxFiltersPerBank;     /* Number of filters in each bank. */
    unsigned int MaxPcrs;               /* Number of PCR blocks */
    unsigned int MaxPacketSubs;         /* Max number of packet substitution channels. */
    unsigned int MaxTpitPids;           /* Max number of PIDs handled by each TPIT parser */
    unsigned int MaxRaveContexts;       /* Max number of RAVE contexts, AV plus record */
    unsigned int MaxRaveChannels;       /* Max number of RAVE channels (or instances of the RAVE core in XPT). */

#ifdef ENABLE_PLAYBACK_MUX
    unsigned int NumPlaybackMuxes;      /* Number of playback blocks to be used for muxing. */
#endif /* ENABLE_PLAYBACK_MUX */

#if BXPT_HAS_IB_PID_PARSERS
    ParserConfig IbParserTable[ BXPT_P_MAX_PID_PARSERS ];
    bool InputParserContCountIgnore[ BXPT_P_MAX_PID_PARSERS ];
#endif

    ParserConfig PbParserTable[ BXPT_P_MAX_PLAYBACKS ];

#if BXPT_HAS_MESG_BUFFERS
    struct {
        BMMA_Block_Handle block;
        void *ptr;
        BMMA_DeviceOffset offset;
    } messageMma[BXPT_P_MAX_MESG_BUFFERS];

    bool PidChannelParserConfigOverride[BXPT_P_MAX_MESG_BUFFERS]; /* if set the true then PSI setting for this pid
                                                                     channel can different from the the PSI settings
                                                                     for the PARSER to which this pid channel belongs */

    MessageBufferEntry MessageBufferTable[ BXPT_P_MAX_MESG_BUFFERS ];   /* Table of PI allocated buffers */
    bool MesgBufferIsInitialized[ BXPT_P_MAX_MESG_BUFFERS ];            /* true if buffer has been configured. */

    FilterTableEntry FilterTable[ BXPT_P_MAX_FILTER_BANKS ][ BXPT_P_FILTER_TABLE_SIZE ];

    /* Pointers to the interrupt handlers. */
    BXPT_P_InterruptCallbackArgs MesgIntrCallbacks[ BXPT_P_MAX_MESG_BUFFERS ];
    BXPT_P_InterruptCallbackArgs OverflowIntrCallbacks[ BXPT_P_MAX_MESG_BUFFERS ];
#endif

#if BXPT_HAS_REMUX
    BXPT_P_RemuxHandle RemuxHandles[ BXPT_P_MAX_REMULTIPLEXORS ];
#endif

#if BXPT_HAS_PACKETSUB
    BXPT_P_PacketSubHandle PacketSubHandles[ BXPT_P_MAX_PACKETSUBS ];
#endif

    PidChannelTableEntry PidChannelTable[ BXPT_P_MAX_PID_CHANNELS ];    /* Table of PI allocated PID channels. */

    BXPT_P_PbHandle PlaybackHandles[ BXPT_P_MAX_PLAYBACKS ];

#if BXPT_HAS_DPCRS
    BXPT_P_PcrHandle_Impl  *PcrHandles[ BXPT_P_MAX_PCRS ];
#endif

    RaveChannel RaveChannels[ BXPT_P_MAX_RAVE_CHANNELS ];
    PcrOffsetData PcrOffsets[ BXPT_P_MAX_PCR_OFFSET_CHANNELS ];

    bool IsLittleEndian;
    bool Pid2BuffMappingOn;
    bool MesgDataOnRPipe;

    BMEM_Handle hPbHeap;

#if BXPT_HAS_RSBUF
    unsigned long RsBufBO[ BXPT_P_MAX_PID_PARSERS ];
#endif
#if BXPT_HAS_XCBUF
    unsigned long XcBufBO[ BXPT_P_MAX_XC_BUFFERS ];
#endif
    bool bStandby; /* true if in standby */

    unsigned DpcrRefCount;  /* Number of DPCR channels that have been openned */

#if BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL
    BXPT_PCR_JitterTimestampMode JitterTimestamp[ BXPT_P_MAX_PCRS ];
    BXPT_PCR_JitterCorrection PbJitterDisable[ BXPT_P_MAX_PCRS ];
    BXPT_PCR_JitterCorrection LiveJitterDisable[ BXPT_P_MAX_PCRS ];
#endif

#if BXPT_HAS_FIXED_RSBUF_CONFIG || BXPT_HAS_FIXED_XCBUF_CONFIG
    #define MAX_NUM_RSBUFF (BXPT_NUM_PID_PARSERS*2 + BXPT_NUM_PLAYBACKS*1)
    #define MAX_NUM_XCBUFF (BXPT_NUM_PID_PARSERS*4 + BXPT_NUM_PLAYBACKS*4)
#else
    #define MAX_NUM_RSBUFF (BXPT_P_MAX_PID_PARSERS*2 + BXPT_P_MAX_PLAYBACKS*1)
    #define MAX_NUM_XCBUFF (BXPT_P_MAX_PID_PARSERS*4 + BXPT_P_MAX_PLAYBACKS*4)
#endif
    struct {
        BMMA_Block_Handle block;
        BMMA_DeviceOffset offset;
    } rsbuff[MAX_NUM_RSBUFF];
    struct {
        BMMA_Block_Handle block;
        BMMA_DeviceOffset offset;
    } xcbuff[MAX_NUM_XCBUFF];

}
BXPT_P_TransportData;

void BXPT_P_ResetTransport(
    BREG_Handle hReg
    );

void BXPT_P_Interrupt_MsgVector_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int L1Shift         /* [in] Dummy arg. Not used by this interface. */
    );

void BXPT_P_Interrupt_MsgOverflowVector_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int L1Shift         /* [in] Dummy arg. Not used by this interface. */
    );

void BXPT_P_Interrupt_MsgSw_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum            /* [in] Message Buffer */
    );

BERR_Code BXPT_P_GetGroupSelect( unsigned int Bank, unsigned int *GenGrpSel );

#if BXPT_HAS_PID_CHANNEL_PES_FILTERING

typedef enum
{
    BXPT_Spid_eChannelFilterMode_Disable = 0,           /* Disable filter mode */
    BXPT_Spid_eChannelFilterMode_StreamId = 8,          /* PB packetizer filters data on stream id */
    BXPT_Spid_eChannelFilterMode_StreamIdRange = 10,    /* PB packetizer filters on hi to lo stream id range */
    BXPT_Spid_eChannelFilterMode_StreamIdExtension = 12,/* PB packetizer filters on stream id and stream id extension */
    BXPT_Spid_eChannelFilterMode_SubStreamId = 14       /* PB packetizer filters on substream id */

}BXPT_Spid_eChannelFilterMode;

typedef struct BXPT_Spid_eChannelFilter
{
    BXPT_Spid_eChannelFilterMode Mode;

    union
    {
        unsigned char StreamId;
        struct
        {
            unsigned char Hi;
            unsigned char Lo;
        }StreamIdRange;

        struct
        {
            unsigned char Id;
            unsigned char Extension;
        }StreamIdAndExtension;

        struct
        {
            unsigned char Id;
            unsigned char SubStreamId;
        }StreamIdAndSubStreamId;
    }FilterConfig;

}BXPT_Spid_eChannelFilter;

/***************************************************************************
Set the configuration for the given secondary PID channel stream filter
based on various stream id combinations. Used during DVD playback mode.
****************************************************************************/
BERR_Code BXPT_Spid_P_ConfigureChannelFilter(BXPT_Handle hXpt,unsigned int ChannelNum,BXPT_Spid_eChannelFilter Filter);

#endif

#if BXPT_HAS_PID2BUF_MAPPING

    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ARRAY_BASE                           BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_MASK              BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_SHIFT             BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_GEN_OFFSET_MASK                       BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_GEN_OFFSET_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_SPECIAL_TYPE_MASK                     BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_SPECIAL_TYPE_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_SKIP_BYTE2_MASK                       BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_SKIP_BYTE2_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_GEN_GRP_SEL_MASK                      BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_GEN_GRP_SEL_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_FILTER_MODE_MASK                      BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_FILTER_MODE_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_GEN_OFFSET_SHIFT                      BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_GEN_OFFSET_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_SPECIAL_TYPE_SHIFT                    BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_SPECIAL_TYPE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_SKIP_BYTE2_SHIFT                      BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_SKIP_BYTE2_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_GEN_GRP_SEL_SHIFT                     BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_GEN_GRP_SEL_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_FILTER_MODE_SHIFT                     BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_FILTER_MODE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ERR_CK_MODE_MASK                      BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ERR_CK_MODE_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ERR_CK_DIS_MASK                       BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ERR_CK_DIS_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SPECIAL_SEL_MASK                      BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SPECIAL_SEL_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ALIGN_MODE_MASK                       BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ALIGN_MODE_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MASK                 BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SPECIAL_SEL_SHIFT                     BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SPECIAL_SEL_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ALIGN_MODE_SHIFT                      BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ALIGN_MODE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_FIRST              BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_FIRST
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_SHIFT                BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ERR_CK_MODE_SHIFT                     BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ERR_CK_MODE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ERR_CK_DIS_SHIFT                      BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ERR_CK_DIS_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_NO_OUTPUT            BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_NO_OUTPUT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_ARRAY_BASE                            BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_ARRAY_BASE
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_SPECIAL_TYPE_MASK                     BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_SPECIAL_TYPE_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_FILTER_MODE_MASK                      BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_FILTER_MODE_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_SPECIAL_TYPE_SHIFT                    BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_SPECIAL_TYPE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_FILTER_MODE_SHIFT                     BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_FILTER_MODE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SPECIAL_SEL_MASK                      BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SPECIAL_SEL_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ALIGN_MODE_MASK                       BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ALIGN_MODE_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DIRECTV_PES_HD_FILT_MODE_MASK         BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DIRECTV_PES_HD_FILT_MODE_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MASK                 BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ALIGN_MODE_SHIFT                      BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ALIGN_MODE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DIRECTV_PES_HD_FILT_MODE_SHIFT        BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DIRECTV_PES_HD_FILT_MODE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_SHIFT                BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_RAW              BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_RAW

    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_PES      BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_PES
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_TS       BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_TS
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_PAYLOAD  BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_PAYLOAD
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_PSI      BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_PSI
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_PES      BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_PES
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_PSI      BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DATA_OUTPUT_MODE_MPEG_PSI
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_MASK         BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_MASK          BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_PSF_CRC_DIS_MASK               BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_PSF_CRC_DIS_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_MSG_MOD_MODE_MASK              BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_MSG_MOD_MODE_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_SHIFT        BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_SHIFT         BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_PSF_CRC_DIS_SHIFT              BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_PSF_CRC_DIS_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_MSG_MOD_MODE_SHIFT             BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_MSG_MOD_MODE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_PSF_CRC_DIS_MASK               BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_PSF_CRC_DIS_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_PSF_CRC_DIS_SHIFT              BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_PSF_CRC_DIS_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_MASK          BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_SHIFT         BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_MASK         BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_SHIFT        BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_MSG_MOD_MODE_MASK              BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_MSG_MOD_MODE_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_MSG_MOD_MODE_SHIFT             BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_MSG_MOD_MODE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_MASK         BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_MASK          BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_PSF_CRC_DIS_MASK               BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_PSF_CRC_DIS_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_MSG_MOD_MODE_MASK              BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_MSG_MOD_MODE_MASK
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_SHIFT        BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_SHORT_PSI_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_SHIFT         BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_SAVE_LONG_PSI_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_PSF_CRC_DIS_SHIFT              BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_PSF_CRC_DIS_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_MSG_MOD_MODE_SHIFT             BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_MSG_MOD_MODE_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_GEN_GRP_SEL_G8_B0              BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_GEN_GRP_SEL_G8_B0
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_GEN_GRP_SEL_G8_B8              BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_GEN_GRP_SEL_G8_B8
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_GEN_GRP_SEL_G4_B0              BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_GEN_GRP_SEL_G4_B0
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_GEN_GRP_SEL_G4_B4              BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_GEN_GRP_SEL_G4_B4
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_GEN_GRP_SEL_G4_B8              BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_GEN_GRP_SEL_G4_B8
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_GEN_GRP_SEL_G4_B12             BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_GEN_GRP_SEL_G4_B12
    #define BCHP_XPT_MSG_PID_ERR_00_31                                    BCHP_XPT_MSG_BUF_ERR_00_31

    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_PHY_EN_MASK            BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_PHY_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_NET_EN_MASK            BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_NET_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_M40_EN_MASK            BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_M40_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_M24_EN_MASK            BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_M24_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_M16_EN_MASK            BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_M16_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_ID_REJECT_EN_MASK      BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_ID_REJECT_EN_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_SPECIAL_NOT_MASK       BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_SPECIAL_NOT_MASK
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_PHY_EN_SHIFT           BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_PHY_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_NET_EN_SHIFT           BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_NET_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_M40_EN_SHIFT           BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_M40_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_M24_EN_SHIFT           BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_M24_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_M16_EN_SHIFT           BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_M16_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_ID_REJECT_EN_SHIFT     BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_ID_REJECT_EN_SHIFT
    #define BCHP_XPT_MSG_PID_CTRL2_TABLE_i_SPECIAL_NOT_SHIFT      BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_SPECIAL_NOT_SHIFT

    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_NONE BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_NONE
    #define BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_ALL  BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_ALL


#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_PRIV_H__ */

/* end of file */
