/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 ******************************************************************************/

#ifndef BSID_PRIV_H__
#define BSID_PRIV_H__

#include "bsid_msg.h"
#include "bsid_dbg.h"
#include "bsid_power.h"
#include "bsid_fw_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSID_P_CLOCK_CONTROL

/*
** Structure Signatures
** These are checked to ensure a GetDefault...() kind of API was used
** to obtain the relevant settings
*/
#define BSID_P_SIGNATURE_BASE             0x53494400   /* "SID" in ASCII */
#define BSID_P_SIGNATURE_OPENSETTINGS     (BSID_P_SIGNATURE_BASE+1)
#define BSID_P_SIGNATURE_OPENCHSETTINGS   (BSID_P_SIGNATURE_BASE+2)
#define BSID_P_SIGNATURE_CHANNELSETTINGS  (BSID_P_SIGNATURE_BASE+3)
#define BSID_P_SIGNATURE_FLUSHSETTINGS    (BSID_P_SIGNATURE_BASE+4)
#define BSID_P_SIGNATURE_STARTDECSETTINGS (BSID_P_SIGNATURE_BASE+5)
#define BSID_P_SIGNATURE_STOPDECSETTINGS  (BSID_P_SIGNATURE_BASE+6)

/*********************************************************************************
/////////////////////// Defines, Typedef, Structs ////////////////////////////////
*********************************************************************************/
#define BSID_SET_CH_STATE(handle, state)  \
        BKNI_EnterCriticalSection(); \
        (handle)->e_ChannelState = BSID_ChannelState_e ## state; \
        BKNI_LeaveCriticalSection()

#define BSID_P_MAX_OUTPUT_BUFFERS                                             10

#define BSID_P_ARC_UART_BAUDRATE                                          115200  /* sid arc uart baud rate */

/* valid metadata indexes are 0 .. OpenChannelSettings->ui32_QueueDepth */
#define BSID_P_INVALID_METADATA_INDEX                                (uint32_t)-1

/* FIXME: a lot of these definitions should be provided by the FW (as part of an API include) */

/******************************************************************************/
/* FIXME: These arc size values should come from FW */
#define BSID_ARC_CODE_SIZE                                            512 * 1024 /* byte */
#define BSID_MBX_MEMORY_SIZE                                                1024 /* byte - needs to be larger than the largest command sent */
#define BSID_DATA_MEMORY_SIZE                                         100 * 1024 /* byte */
#define BSID_INPUT_DMA_MEMORY_SIZE                                          1024 /* byte */

/********************************************************************************/

/* FIXME: How are these size values determined?  Are they FW-specific? */
#define BSID_RAVE_CDB_LENGTH                                      (3*1024*1024/2)
#define BSID_RAVE_ITB_LENGTH                                           (512*1024)

#define BSID_RAVE_CDB_ALIGNMENT                                                 4 /* bits: pow 2 - why does CDB need an alignment? */
#define BSID_RAVE_ITB_ALIGNMENT                                                 4 /* bits: pow 2 - ITB entries are 4 x 32-bit words = 16 bytes */

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
#define BSID_RAVE_CDB_ENDIAN                                                false
#define BSID_RAVE_ITB_ENDIAN                                                 true
#else
#define BSID_RAVE_CDB_ENDIAN                                                 true
#define BSID_RAVE_ITB_ENDIAN                                                false
#endif

/********************************************************************************/
/* Memory allocation alignments */
/* BMEM alignments are in bits (pow 2; i.e. 2 bits = 4 bytes) */
/* BMMA alignments are in bytes */

/* Generic alignments are 4 bytes (32 bits) to ensure MBOX entries are word aligned, etc */
#define BSID_MEM_ALIGN_BYTES                                                    4
#define BSID_MEM_ALIGN_MASK                              (BSID_MEM_ALIGN_BYTES-1)
#define BSID_ARC_CODE_ALIGN_BYTES                                            1024 /* why 1024 bytes?? */
#define BSID_OUTPUT_BUFFER_ALIGNMENT_BYTES                                      4 /* align output to word boundary */
#define BSID_QUEUE_ALIGNMENT_BYTES                                              4 /* align Command headers to word (32-bit) boundary */

/* NOTE: The Image Header obtained from a GetStreamInfo command is stored in
   the data queue, so each data queue entry must be at least large enough to
   store the image header */
#define BSID_DATA_QUEUE_NUM_ELEM         (sizeof(BSID_ImageHeader)+(sizeof(uint32_t)-1))/sizeof(uint32_t)

/********************************************************************************/
typedef struct BSID_LinearBuffer {
    uint32_t  ui32_Size;
    BMMA_Block_Handle  hBlock;
    /* NOTE: This is really the device offset, not an address
       The lower 32-bits are used as the physical address by the FW */
    BMMA_DeviceOffset  ui32_PhysAddr;
    void               *pv_CachedAddr;
} BSID_LinearBuffer;

/********************************************************************************/
typedef struct BSID_FwHwConfig {
    bool                bSelfTest;
    uint16_t            ui16_JPEGHorizAndVerFilt;
    uint8_t             ui8_AlphaValue;
    bool                b_EndianessSwap;
    BSID_MemoryMode     eMemoryMode;   /* FIXME: This used? Seems to only support "Unified" */
    BSID_LinearBuffer   sMemory;
    BSID_LinearBuffer   sCodeMemory;
    struct {
        BSID_LinearBuffer sCmd;
        BSID_LinearBuffer sRsp;
    } sMbxMemory;
    BSID_LinearBuffer   sInpDmaMemory;
    BSID_LinearBuffer   sDataMemory;
} BSID_FwHwConfig;

typedef struct BSID_MailboxInfo {
    BMMA_Block_Handle  hCmdMbxBlock;
    BMMA_Block_Handle  hRspMbxBlock;
    void     *pv_CmdQCachedAddr;
    uint32_t  ui32_CmdQPhysAddr; /* These are 32-bit HW/FW physical addresses */
    void     *pv_RspQCachedAddr;
    uint32_t  ui32_RspQPhysAddr; /* These are 32-bit HW/FW physical addresses */
    BKNI_EventHandle hMailboxEvent;   /* mailbox event */
} BSID_MailboxInfo;

/* FIXME: DataMap does not seem to be used - was this for support of "scatter"
   data map type?  There is an equivalent structure called ST_DataMap in sidapi.h
   in the FW that defines the segmented data map */
/********************************************************************************/
typedef struct BSID_DataMapHeader {
    uint32_t ui32_InpBufferAddr;
    uint32_t ui32_InpBufferSize;
    uint32_t ui32_NumSegments;
} BSID_DataMapHeader;

/********************************************************************************/
typedef struct BSID_DataMapSegment {
    uint32_t ui32_SegmentOffset;
    uint32_t ui32_SegmentSize;
} BSID_DataMapSegment;

/********************************************************************************/
typedef struct BSID_DataMap {
    BSID_DataMapHeader   sDataMapHeader;
    BSID_DataMapSegment  sFirstSegment;
} BSID_DataMap;

/********************************************************************************/
typedef struct BSID_P_Context {
    BDBG_OBJECT(BSID_P_Context)
    uint32_t            ui32_SidInstance;     /* instance number. */
    BCHP_Handle         hChp;                 /* handle to chip module. */
    BREG_Handle         hReg;                 /* handle to register module. */
    BMMA_Heap_Handle    hMma;                 /* handle to bmma memory heap */
    BINT_Handle         hInt;                 /* handle to interrupt module. */
    BSID_FwHwConfig     sFwHwConfig;          /* setup for SID ARC firmware and Hardware */
    BSID_MailboxInfo    sMailbox;             /* mailbox related registers/variables. */
    BINT_CallbackHandle hServiceIsr;          /* isr callback associated with sid irq */
    BINT_CallbackHandle hWatchdogIsr;         /* isr for ARC watchdog */
    BSID_BootInfo       sBootInfo;
    BERR_Code           (*pExternalBootCallback)(void *, const BSID_BootInfo *);
    void                *pExternalBootCallbackData;
    BSID_WatchdogCallbackFunc fWatchdogCallback_isr;
    BSID_WatchdogCallbackData *pWatchdogCallbackData;
    BSID_ChannelHandle  ahChannel[BSID_MAX_CHANNELS];
    bool                bStandby;
    bool                bArcReLoad;           /* first boot does code and data ARC boot, and FW verification.  After that it only does data load to ARC */
    bool                bWatchdogOccurred;
    BSID_P_PowerResource PowerResources[BSID_P_ResourceType_eLast];
    const BIMG_Interface *pImgInterface;
    const void           *pImgContext;
} BSID_P_Context;

/********************************************************************************/

typedef struct BSID_DataQueueEntry {
   /* NOTE: currently, this is only used for storing the decoded stream info header */
   uint32_t     entry[BSID_DATA_QUEUE_NUM_ELEM];
} BSID_DataQueueEntry;

typedef struct BSID_ChannelQueue {
    BSID_LinearBuffer sBuf;
    BSID_CommandQueueHeader  sHdr;
} BSID_ChannelQueue;

typedef enum BSID_ChannelChangeOutputMode
{
  BSID_ChannelChangeOutputMode_eMute,
  BSID_ChannelChangeOutputMode_eLastFramePreviousChannel,
  BSID_ChannelChangeOutputMode_eLast
} BSID_ChannelChangeOutputMode;

/* FIXME: This must be common to FW also */
typedef struct BSID_RaveStatusReport {

    /* start and end of cdb buffer: don't change during playback */
    uint32_t CDB_Base;
    uint32_t CDB_End;

    /* used by pi to update fw */
    struct from_pi {
        uint32_t CDB_Read;
        uint32_t CDB_Valid;
        uint32_t CDB_Wrap;
    } from_pi;

    /* used by fw to indicate where the last decode was executed at */
    struct from_fw {
        uint32_t CDB_Read;
        uint32_t CDB_Wrap;
    } from_fw;

} BSID_RaveStatusReport;


typedef struct BSID_PlaybackQueueState {
    BSID_OutputBufferState ui32_OutputState;
    BXDM_Picture           *ps_UnifiedPicture;
} BSID_PlaybackQueueState;

typedef struct BSID_PlaybackQueue {
    BSID_LinearBuffer        sWriteBuf; /* Host->Arc data: Arc can't change these data */
    BSID_LinearBuffer        sReadBuf;  /* Arc->Host data: Host can't change these data */
    BSID_LinearBuffer        sStateBuf; /* Host owned only state buffer */
    uint32_t                 ui32_HostDecodeReadIndex;
    uint32_t                 ui32_DisplayReadIndex;
    uint32_t                 ui32_DecodeQueueFullnessCount;
    uint32_t                 ui32_DisplayQueueFullnessCount;
} BSID_PlaybackQueue;

/* metadata entry to store the virtual addresses associated with
   the locations stream info is written by the firmware */
typedef struct BSID_P_MetadataEntry
{
   void *pvStreamInfoFWAddr;  /* where the fw writes the stream info
                                 (virtual address corresponding to the physical
                                 address sent in the GetStreamInfo command)
                                 => this will be a location in the DataQueue */
   void *pStreamInfoDestAddr; /* where the application wants the stream info to go */
   bool bInUse;               /* indicates whether this entry is in use or free */
} BSID_P_MetadataEntry;

/********************************************************************************/
typedef struct BSID_P_Channel {
    BDBG_OBJECT(BSID_P_Channel)
    /* generic */
    BSID_Handle          hSid;
    BSID_DecodeMode      eDecodeMode;
    uint32_t             ui32_ChannelNum; /* FIXME: what's the diff between these two? */
    /* => channel number maps to a specific supported channel 0 ..  MAX_NUM_CHANNELS-1 */
    /* channel ID is a unique ID for the channel used by FW? What's it used for?
       So multiple IDs can map to a single channel num when we support multi-channel virtualization? */
    uint32_t             ui32_ChannelID;
    uint32_t             ui32_QueueTrueDepth;
    uint32_t             ui32_SequenceNumber;
    BSID_OpenChannelSettings sChSettings;
    BSID_ChannelQueue    sReqQueue;  /* operations from mips to arc */
    BSID_ChannelQueue    sRelQueue;  /* results from arc to mips */
    BSID_ChannelQueue    sDataQueue; /* large data exchange between mips and arc (stream info, decoded image) */
    bool                 b_FlushPending;
    bool                 bAbortInitiated;
    bool                 bStarted;            /* indicates if the channel has been started */
    BKNI_EventHandle     hSyncEvent;
    BKNI_EventHandle     hAbortedEvent;

    /* motion channel only related */
    BSID_ChannelType     e_ChannelType;
    BSID_ChannelState    e_ChannelState;
    BMMA_Heap_Handle     hChMmaHeap;
    BAVC_XptContextMap   sContextMap;
    BMMA_Block_Handle    hCdbBlock;
    void                *pv_CachedCdbAddr;
    BMMA_Block_Handle    hItbBlock;
    void                *pv_CachedItbAddr;
    BSID_LinearBuffer    a_OutputBuffer[BSID_P_MAX_OUTPUT_BUFFERS];
    BSID_LinearBuffer    sRaveReport;
    BSID_PlaybackQueue   sPlaybackQueue;
    BXDM_Picture         a_DisplayBuffer[BSID_P_MAX_OUTPUT_BUFFERS];
    unsigned int         last_ITB_Read;
    BSID_ChannelChangeOutputMode e_ChannelChangeOutputMode;
    BSID_P_MetadataEntry *pMetadata;    /* This has ui32_QueueTrueDepth entries in the array */
#ifdef BSID_P_DEBUG_SAVE_BUFFER
    BSID_P_DebugSaveData stDebugSaveData;
#endif
#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
    BSID_P_TracePlayback stDebugPlaybackTrace;
#endif
} BSID_P_Channel;

/*********************************************************************************
////////////////////// Function prototypes declaration ///////////////////////////
*********************************************************************************/
BERR_Code BSID_P_ResetChannel(BSID_ChannelHandle hSidCh);
void BSID_P_ResetDmaInfo(BSID_Handle hSid);
BERR_Code BSID_P_SetFwHwDefault(BSID_Handle hSid, BSID_Settings ps_DefSettings);
void  BSID_P_ResetFwHwDefault(BSID_Handle hSid);
BERR_Code BSID_P_CreateChannelMemory(BSID_ChannelHandle hSidCh);
BERR_Code BSID_P_ResetChannelMemory(BSID_ChannelHandle hSidCh);
void BSID_P_DestroyChannelMemory(BSID_ChannelHandle hSidCh);
BERR_Code BSID_P_MotionDecode(BSID_ChannelHandle hSidCh, const BSID_DecodeMotion *ps_MotionSettings);
BERR_Code BSID_P_SuspendChannels(BSID_Handle hSid);
BERR_Code BSID_P_ResumeChannel(BSID_ChannelHandle hSidCh);
BERR_Code BSID_P_ResumeActiveChannels(BSID_Handle hSid);
void BSID_P_AbortDecode(BSID_ChannelHandle hSidCh);
bool BSID_P_IsStillOperationAllowed(BSID_Handle hSid);
bool BSID_P_IsMotionOperationAllowed(BSID_Handle hSid);
bool BSID_P_AnyChannelAvailable(BSID_Handle hSid, uint32_t *idleChannel);
void BSID_P_Watchdog_isr(void *pContext, int iParam);
bool BSID_P_IsChannelQueueFull(BSID_ChannelHandle hSidCh);

/*********************************************************************************
//////////////////////////////////////////////////////////////////////////////////
*********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BSID_PRIV_H__ */

/* end of file */
