/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

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
#include "bxpt_pcr.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
    #ifdef BCHP_PWR_RESOURCE_XPT_SRAM
    #include "bchp_xpt_fe.h"
    #include "bchp_xpt_msg.h"
    #endif
#endif
#include "bxpt_pwr_mgmt_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Address delta (in bytes) between elements in a register array. */
#define PARSER_REG_STEPSIZE     ( BCHP_XPT_FE_MINI_PID_PARSER1_CTRL1 - BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 )
#define IB_REG_STEPSIZE         ( BCHP_XPT_FE_IB1_CTRL - BCHP_XPT_FE_IB0_CTRL )
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

/* BCHP_PWR_SUPPORT is BCHP_PWR-dynamic PM and standby/resume.
BXPT_POWER_MANAGEMENT is XPT-specific auxiliary PM, such as submodule-clockgating and memory power down */
#if BCHP_PWR_SUPPORT && BXPT_POWER_MANAGEMENT
#define BXPT_P_ENABLE_SUBMODULE_CLOCKGATING 1
#endif
#define PWR_BO_COUNT 0x3fffffff

/***************************************************************************
Summary:
Used to define the flags inside a linked list descriptor.
****************************************************************************/
#define TRANS_DESC_INT_FLAG             ( 1ul << 31 )
#define TRANS_DESC_FORCE_RESYNC_FLAG    ( 1ul << 30 )
#define TRANS_DESC_LAST_DESCR_IND       ( 1ul << 0 )

/***************************************************************************
Summary:
Workaround-defines
****************************************************************************/

/* See SW7439-58, SW7366-48, or SW7445-568. Same issue on different chips */
/* TODO: is it for ALL 7445 revs? */
#if ( (BCHP_CHIP==7445) ||\
      (BCHP_CHIP==7439 && BCHP_VER<BCHP_VER_B0) || (BCHP_CHIP==7415 && BCHP_VER<BCHP_VER_B0) ||\
      (BCHP_CHIP==7366 && BCHP_VER<BCHP_VER_B0) || (BCHP_CHIP==7364 && BCHP_VER<BCHP_VER_B0) || \
      (BCHP_CHIP==74371 && BCHP_VER<BCHP_VER_B0) )
#define BXPT_HAS_HEVD_WORKAROUND   1
#define BXPT_SW7439_115_WORKAROUND 1
#endif

/* SW7445-1658 */
#if ( (BCHP_CHIP==7445 && BCHP_VER<BCHP_VER_E0) || (BCHP_CHIP==7145 && BCHP_VER<BCHP_VER_C0) || \
      (BCHP_CHIP==7439 && BCHP_VER<BCHP_VER_C0) || (BCHP_CHIP==7250 && BCHP_VER<BCHP_VER_B0) || \
      (BCHP_CHIP==3390 && BCHP_VER<BCHP_VER_B0) || (BCHP_CHIP==7364 && BCHP_VER<BCHP_VER_B0) )
#define BXPT_P_HAS_XCBUFF_ENABLE_WORKAROUND 1
#endif

/***************************************************************************
Summary:
Private capability-defines
****************************************************************************/
#define BXPT_P_SAM_CTRL1            1
#define BXPT_P_NUM_SPLICING_QUEUES  3 /* Number of splicing stacks. The stacks are shared between the AV and SCD contexts */

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

#if BXPT_SW7425_1323_WORKAROUND
    bool IsPb;
    bool IsEnabled;
#endif
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

typedef struct BXPT_P_GpcInfo
{
    bool Allocated;
    unsigned Index;
}
BXPT_P_GpcInfo;

typedef struct BXPT_P_PbHandle
{
    BCHP_Handle hChip;              /* [Input] Handle to used chip. */
    BREG_Handle hRegister;          /* [Input] Handle to access regiters. */
    void *vhXpt;                    /* Pointer to parent transport handle. Requires casting. */

    uint8_t ChannelNo;  /* This channels number. */
    uint32_t BaseAddr;  /* Address of the first register in this block. */
    BXPT_PvrDescriptor* LastDescriptor; /* Address of the last descriptor on the linked list. */
    bool Opened;
    bool Running;
    unsigned int SyncMode;
    bool ForceResync;

    uint32_t BitRate;

    BXPT_PidChannel_CC_Config CcConfigBeforeAllPass;
    bool IsParserInAllPass;

#if BXPT_HAS_TSMUX
    unsigned PacingLoadMap;
    unsigned PacingCount;
    bool PacingLoadNeeded;
#endif

    BXPT_Playback_ChannelSettings settings;

    struct {
        BMMA_Block_Handle descBlock;
        void *descPtr;
        BMMA_DeviceOffset descOffset;
    } mma;
}
BXPT_P_PbHandle;

/***************************************************************************
Summary:
Handle for accessing the remux API via a channel. Users should not directly
access the contents of the structure.
****************************************************************************/

typedef struct BXPT_P_RemuxHandle
{
    void *vhXpt;            /* Pointer to parent transport handle. Requires casting. */

    BCHP_Handle hChip;              /* [Input] Handle to used chip. */
    BREG_Handle hRegister;          /* [Input] Handle to access regiters. */

    bool Opened;
    bool Running;

    uint8_t ChannelNo;      /* This channels number. */
    uint32_t BaseAddr;  /* Address of the first register in this block. */
    bool BufferIsAllocated;
    void *BufferAddress;
}
BXPT_P_RemuxHandle;

/***************************************************************************
Summary:
Handle for accessing the packet substution API via a channel. Users should
not directly access the contents of the structure.
****************************************************************************/

typedef struct BXPT_P_PacketSubHandle
{
    BCHP_Handle hChip;              /* [Input] Handle to used chip. */
    BREG_Handle hRegister;          /* [Input] Handle to access regiters. */
    void *vhXpt;

    uint8_t ChannelNo;      /* This channels number. */
    uint32_t BaseAddr;      /* Address of the first register in this block. */
    void *LastDescriptor_Cached;    /* Address of the last descriptor on the linked list. */
    bool Opened;
    bool Running;

    struct {
        BMMA_Block_Handle block;
        void *ptr;
        BMMA_DeviceOffset offset;
    } mma;
}
BXPT_P_PacketSubHandle;

typedef struct BXPT_P_PcrHandle_Impl
{
    BCHP_Handle hChip;              /* [Input] Handle to used chip. */
    BREG_Handle hRegister;          /* [Input] Handle to access regiters. */
    void *vhXpt;

    uint8_t ChannelNo;              /* This channels number. */
    uint32_t RegOffset;            /* reg Offset of the given module. */
    bool DMode;
#if BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND
    uint8_t FiltB;          /* loop gain. */
    uint8_t FiltC;          /* direct path gain. */
    uint8_t TrackRange;
    int64_t Accum;
    bool InErrState;
    unsigned PcrCount;
    int64_t PcrThreshold;
#endif
    bool pidChnlConfigured;
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
    BXPT_RaveIdx ChannelType;
    void *hChannel;
    bool Allocated;
    unsigned NumChannels;
    void *vhCtx;
    bool SvcMvcMode;
}
BXPT_P_IndexerHandle;

typedef struct StartCodeIndexer
{
    BREG_Handle hReg;
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
    brave_itb_termination =          0x70,
    brave_itb_base_address_40bit =   0x28
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
    BMMA_DeviceOffset src_itb_base, dest_itb_base;

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

    /* SWSTB-3582: The BASE opcode was changed for the 40-bit support in RAVE ITB */
    unsigned BaseOpCode;
}
SoftRaveData;

typedef struct BXPT_P_ContextHandle
{
    BREG_Handle hReg;
    BCHP_Handle hChp;
    BINT_Handle hInt;
    void *vhRave;
    BMMA_Heap_Handle mmaHeap, mmaRHeap;

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

    bool externalCdbAlloc, externalItbAlloc;

    struct {
        BMMA_Block_Handle cdbBlock, itbBlock;
        unsigned cdbBlockOffset, itbBlockOffset;
        void *cdbPtr, *itbPtr;
        BMMA_DeviceOffset cdbOffset, itbOffset;
    } mma;

    /* For streaming IP support only. Don't use in AV or REC processing */
    bool IsMpegTs;      /* True if context is handling MPEG TS, false if DirecTV */

    /* For 7401 only */
    bool VobMode;

    bool IsSoftContext;
    void *SourceContext;
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

#if BXPT_SW7425_1323_WORKAROUND
    bool BandHoldEn;
    unsigned UpperThreshold;
#endif
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
    BINT_Handle hInt;
    uint32_t BaseAddr;
    uint32_t PidTableBaseAddr;
    uint32_t ParseTableBaseAddr;
    unsigned ChannelNo;
    bool Allocated;
}
TpitIndexer;

typedef struct BXPT_P_RaveHandle
{
    BCHP_Handle hChip;
    BREG_Handle hReg;
    BINT_Handle hInt;
    void *lvXpt;
    unsigned ChannelNo;
    BMMA_Heap_Handle mmaHeap, mmaRHeap;

    BXPT_P_ContextHandle *ContextTbl[ BXPT_NUM_RAVE_CONTEXTS ];
    bool SpliceQueueAllocated[ BXPT_P_NUM_SPLICING_QUEUES ];
    StartCodeIndexer ScdTable[ BXPT_NUM_SCD ];
    TpitIndexer TpitTable[ BXPT_NUM_TPIT ];
    BXPT_P_IndexerHandle IndexerHandles[ BXPT_NUM_SCD + BXPT_NUM_TPIT ];
    RaveChannelOpenCB chanOpenCB;

#if BXPT_SW7425_1323_WORKAROUND
    signed PidChanToContextMap[ BXPT_NUM_PID_CHANNELS ];
    unsigned WatermarkGranularity;
    bool DoWorkaround;
#endif

}
BXPT_P_RaveHandle;

typedef struct RaveChannel
{
    void *Handle;
    bool Allocated;
}
RaveChannel;

typedef struct BXPT_P_StcSnapshot
{
    BREG_Handle hReg;
    bool Allocated;
    unsigned Index;
    unsigned WhichStc;
}
BXPT_P_StcSnapshot;

typedef struct BXPT_P_PcrOffset_Impl
{
    BCHP_Handle hChip;
    BREG_Handle hReg;
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

#ifdef BXPT_P_HAS_224B_SLOT_SIZE
   #define BXPT_P_MINIMUM_BUF_SIZE            (4 * 224)
#else
   #define BXPT_P_MINIMUM_BUF_SIZE            (256)
#endif

typedef struct BXPT_P_TransportData
{
    BCHP_Handle hChip;              /* [Input] Handle to used chip. */
    BREG_Handle hRegister;          /* [Input] Handle to access regiters. */
    BINT_Handle hInt;               /* [Input] Handle to interrupt interface to use. */
    BMMA_Heap_Handle mmaHeap, mmaRHeap;

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

    #define MAX_NUM_RSBUFF (BXPT_NUM_PID_PARSERS*2 + BXPT_NUM_PLAYBACKS*1)
    #define MAX_NUM_XCBUFF (BXPT_NUM_PID_PARSERS*4 + BXPT_NUM_PLAYBACKS*4)

    struct {
        BMMA_Block_Handle block;
        BMMA_DeviceOffset offset;
    } rsbuff[MAX_NUM_RSBUFF];
    struct {
        BMMA_Block_Handle block;
        BMMA_DeviceOffset offset;
    } xcbuff[MAX_NUM_XCBUFF];

    struct {
        BMMA_Block_Handle block;
        BMMA_DeviceOffset offset;
    } sharedRsXcBuff;

#if BXPT_NUM_TSIO
    struct {
        BMMA_Block_Handle block;
        BMMA_DeviceOffset offset;
    } tsioRsbuff[ BXPT_NUM_PID_PARSERS ];
#endif

#ifdef ENABLE_PLAYBACK_MUX
    unsigned int NumPlaybackMuxes;      /* Number of playback blocks to be used for muxing. */
#endif /* ENABLE_PLAYBACK_MUX */

#if BXPT_HAS_IB_PID_PARSERS
    ParserConfig IbParserTable[ BXPT_NUM_PID_PARSERS ];
    bool IsParserInAllPass[ BXPT_NUM_PID_PARSERS ];
#endif

    ParserConfig PbParserTable[ BXPT_NUM_PLAYBACKS ];

#if BXPT_HAS_MESG_BUFFERS
    struct {
        BMMA_Block_Handle block;
        void *ptr;
        BMMA_DeviceOffset offset;
    } messageMma[BXPT_NUM_MESG_BUFFERS];

    bool PidChannelParserConfigOverride[BXPT_NUM_MESG_BUFFERS]; /* if set the true then PSI setting for this pid
                                                                     channel can different from the the PSI settings
                                                                     for the PARSER to which this pid channel belongs */

    MessageBufferEntry MessageBufferTable[ BXPT_NUM_MESG_BUFFERS ];   /* Table of PI allocated buffers */
    bool MesgBufferIsInitialized[ BXPT_NUM_MESG_BUFFERS ];            /* true if buffer has been configured. */

    FilterTableEntry FilterTable[ BXPT_NUM_FILTER_BANKS ][ BXPT_P_FILTER_TABLE_SIZE ];

    /* Pointers to the interrupt handlers. */
#if BXPT_HAS_MESG_L2
    BINT_CallbackHandle MesgIntrCallbacks[ BXPT_NUM_MESG_BUFFERS ];
    BINT_CallbackHandle OverflowIntrCallbacks[ BXPT_NUM_MESG_BUFFERS ];
#else
    BXPT_P_InterruptCallbackArgs MesgIntrCallbacks[ BXPT_NUM_MESG_BUFFERS ];
    BXPT_P_InterruptCallbackArgs OverflowIntrCallbacks[ BXPT_NUM_MESG_BUFFERS ];
#endif

#endif

#if BXPT_HAS_REMUX
    BXPT_P_RemuxHandle RemuxHandles[ BXPT_NUM_REMULTIPLEXORS ];
#endif

#if BXPT_HAS_PACKETSUB
    BXPT_P_PacketSubHandle PacketSubHandles[ BXPT_NUM_PACKETSUBS ];
#endif

    PidChannelTableEntry PidChannelTable[ BXPT_NUM_PID_CHANNELS ];    /* Table of PI allocated PID channels. */

    BXPT_P_PbHandle PlaybackHandles[ BXPT_NUM_PLAYBACKS ];

#if BXPT_HAS_DPCRS
    BXPT_P_PcrHandle_Impl  *PcrHandles[ BXPT_NUM_PCRS ];
#endif

    RaveChannel RaveChannels[ BXPT_NUM_RAVE_CHANNELS ];
    PcrOffsetData PcrOffsets[ BXPT_NUM_PCR_OFFSET_CHANNELS ];

    bool IsLittleEndian;
    bool Pid2BuffMappingOn;
    bool MesgDataOnRPipe;

    BXPT_PidChannel_CC_Config CcConfigBeforeAllPass[ BXPT_NUM_PID_CHANNELS ];

#if BXPT_HAS_PARSER_REMAPPING
    BXPT_ParserBandMapping BandMap;
#endif

#if BXPT_HAS_MEMDMA
    void *dmaChannels[ BXPT_NUM_PLAYBACKS ];
#endif

    /* TODO: consolidate power-related variables into a struct */
    bool bStandby; /* true if in standby */
    bool bS3Standby;
    bool WakeupEnabled; /* Wakeup packet support enabled */
    uint32_t pmuClockCtrl;
    struct {
        unsigned refcnt[BXPT_P_Submodule_eMax];
    } power;

#ifdef BCHP_PWR_RESOURCE_XPT
    struct BXPT_Backup sramBackup;
    struct BXPT_Backup regBackup;
    void *vhRave;
    bool WakeupArmed;
    uint32_t *mcpbBackup;
#endif

    unsigned DpcrRefCount;  /* Number of DPCR channels that have been openned */

    BXPT_PCR_JitterTimestampMode JitterTimestamp[ BXPT_NUM_PCRS ];
    BXPT_PCR_JitterCorrection PbJitterDisable[ BXPT_NUM_PCRS ];
    BXPT_PCR_JitterCorrection LiveJitterDisable[ BXPT_NUM_PCRS ];

#if BXPT_SW7425_1323_WORKAROUND
    bool DoWorkaround;
#endif

    BXPT_P_GpcInfo PacingCounters[ BXPT_NUM_PACING_COUNTERS ];

#if BXPT_NUM_STC_SNAPSHOTS
    BXPT_P_StcSnapshot StcSnapshots[ BXPT_NUM_STC_SNAPSHOTS ];
#endif

#if BXPT_NUM_TSIO
    bool tsioOpened[ BXPT_NUM_TSIO ];
#endif

#if BXPT_NUM_TBG
    unsigned numTbg; /* total number of TBG channels opened */
    void *tbg[BXPT_NUM_TBG];
    BINT_CallbackHandle rsbuffOverflowIrq;
#endif
}
BXPT_P_TransportData;

BERR_Code BXPT_Dma_P_Init(BXPT_Handle hXpt);

uint32_t BXPT_P_GetParserCtrlRegAddr(
    BXPT_Handle hXpt,               /* [in] Handle for the transport to access. */
    unsigned ParserNum,
    unsigned Reg0
    );

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

/* Enable PID2BUFF support. On by default in this sw base. */
void SetPid2BuffMap(BXPT_Handle hXpt);

bool BXPT_P_InputBandIsSupported(
    unsigned ib
    );

void BXPT_Playback_P_EnableInterrupts(
        BXPT_Handle hXpt
        );
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_PRIV_H__ */

/* end of file */
