/******************************************************************************
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
 *****************************************************************************/
#ifndef NEXUS_TRANSPORT_MODULE_H__
#define NEXUS_TRANSPORT_MODULE_H__

#include "nexus_base.h"
#include "nexus_platform_features.h"
#include "nexus_transport_thunks.h"
#include "nexus_types.h"
#include "nexus_video_types.h"
#include "nexus_transport_init.h"
#include "priv/nexus_transport_standby_priv.h"
#include "nexus_pid_channel.h"
#include "nexus_pid_channel_scrambling.h"
#include "nexus_timebase.h"
#include "nexus_stc_channel.h"
#include "nexus_message.h"
#include "nexus_message_sam.h"
#include "nexus_remux.h"
#include "nexus_input_band.h"
#include "nexus_parser_band.h"
#include "nexus_packetsub.h"
#include "nexus_playpump.h"
#include "nexus_rave.h"
#include "priv/nexus_transport_priv.h"
#include "priv/nexus_stc_channel_priv.h"
#include "priv/nexus_timebase_priv.h"
#include "priv/nexus_rave_priv.h"
#include "priv/nexus_playpump_priv.h"
#include "priv/nexus_pid_channel_priv.h"
#include "priv/nexus_core.h"
#include "nexus_transport_capabilities.h"
#include "nexus_recpump.h"
#include "nexus_core_utils.h"
#include "priv/nexus_core.h"
#include "nexus_mpod.h"
#include "nexus_transport_wakeup.h"
#include "nexus_vcxo.h"
#include "nexus_dma.h"
#include "nexus_xpt_dma.h"
#if NEXUS_HAS_XPT_DMA
#include "bxpt_dma.h"
#endif

#include "bxpt_capabilities.h"

/* extensions */
#if NEXUS_HAS_ICAM
#include "nexus_icam.h"
#endif
#if NEXUS_HAS_SVP
#include "nexus_svp.h"
#endif
#if NEXUS_HAS_RAVE_EMM
#include "nexus_icamemm.h"
#endif
#if NEXUS_HAS_SIMPLE_PLAYPUMP
#include "nexus_simple_playpump.h"
#endif
#if NEXUS_HAS_NSK2HDX
#include "nexus_nsk2emm.h"
#endif
#if NEXUS_TRANSPORT_EXTRA_STATUS
#include "nexus_transport_extra_status.h"
#endif
#if NEXUS_SW_RAVE_PEEK_EXTENSION
#include "nexus_sw_rave_directv_extension.h"
#endif

#include "blst_queue.h"

#include "bxpt.h"
#if NEXUS_USE_OTT_TRANSPORT
#include "bott_xpt_bxpt_compat.h"
#ifdef BCHP_OTT_XPT_CDB_ITB_REG_START
#include "bchp_cdb_itd_rdb_remap.h"
#endif
#endif

/* BXPT_HAS_DIRECTV_SUPPORT means the HW supports it and is hardcoded in XPT.
B_REFSW_DSS_SUPPORT means the SW supports it and is set by env variable. */
#if B_REFSW_DSS_SUPPORT
#if !BXPT_HAS_DIRECTV_SUPPORT
#error B_REFSW_DSS_SUPPORT not supported on this chip
#endif
#endif
#include "bxpt_pcr.h"
#include "bxpt_pcr_offset.h"
#include "bxpt_rave.h"
#if B_REFSW_DSS_SUPPORT
#include "bxpt_directv.h"
#include "bxpt_directv_pcr.h"
#include "bxpt_directv_pcroffset.h"
#endif
#include "bpcrlib.h"
#ifdef BCHP_XPT_RAVE_REG_START
#include "bchp_xpt_rave.h"
#endif
#if NEXUS_USE_SW_FILTER
#include "nexus_message_swfilter_priv.h"
#endif

#if BXPT_HAS_TSMUX
#include "priv/nexus_tsmux_priv.h"
#endif

#if BXPT_HAS_WAKEUP_PKT_SUPPORT
#include "bchp_xpt_wakeup.h"
#include "bxpt_wakeup.h"
#endif

#if NEXUS_TRANSPORT_EXTENSION_TSIO
#include "nexus_tsio.h"
#endif

#if NEXUS_TRANSPORT_EXTENSION_EPR
#include "nexus_epr.h"
#endif

#include "nexus_tsmf.h"
#include "nexus_parser_band_channelbonding.h"
#include "nexus_gcb_sw_priv.h"

#if NEXUS_TRANSPORT_EXTENSION_ATS
#include "nexus_ats.h"
#endif

#if NEXUS_TRANSPORT_EXTENSION_TBG
#include "nexus_tbg.h"
#endif

#if NEXUS_HAS_ETBG
#include "nexus_etbg.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME transport
#define NEXUS_MODULE_SELF g_NEXUS_Transport_P_State.transportModule

BDBG_OBJECT_ID_DECLARE(NEXUS_Rave);
BDBG_OBJECT_ID_DECLARE(NEXUS_StcChannel);
BDBG_OBJECT_ID_DECLARE(NEXUS_Remux);
BDBG_OBJECT_ID_DECLARE(NEXUS_Message);
BDBG_OBJECT_ID_DECLARE(NEXUS_ParserBand);
BDBG_OBJECT_ID_DECLARE(NEXUS_Timebase);

typedef struct NEXUS_P_HwPidChannel *NEXUS_P_HwPidChannelHandle;
typedef struct NEXUS_P_HwPidChannel NEXUS_P_HwPidChannel;

#ifndef NEXUS_RAVE_THRESHOLD_UNITS
/* XPT HW has a programmable units for CDB/ITB threshold interrupts. Nexus uses a fixed value. */
#define NEXUS_RAVE_THRESHOLD_UNITS 256
#endif

#if BXPT_NUM_REMULTIPLEXORS
#include "bxpt_remux.h"
#endif

/* SW RAVE is supported by default on all chips */
#ifndef NEXUS_SW_RAVE_SUPPORT
#define NEXUS_SW_RAVE_SUPPORT 1
#endif

/* playpump/recpump with crypto is only supported with MEM_DMA HW, not SHARF_DMA HW.
   we have to use NEXUS_NUM_DMA_CHANNELS, not NEXUS_HAS_DMA, because the latter is
   defined even if the platform only has SHARF_DMA */
#if NEXUS_NUM_DMA_CHANNELS && NEXUS_HAS_SECURITY
#define NEXUS_ENCRYPTED_DVR_WITH_M2M 1
#endif

typedef struct NEXUS_ParserBand * NEXUS_ParserBandHandle;

#if NEXUS_TRANSPORT_CLIENT_CAPTURE_ENABLED
#include <stdio.h>

typedef enum NEXUS_TransportClientType
{
    NEXUS_TransportClientType_eRave,
    NEXUS_TransportClientType_eMessage,
    NEXUS_TransportClientType_eRemux,
    NEXUS_TransportClientType_eMax
} NEXUS_TransportClientType;

typedef struct NEXUS_TransportClientCaptureCreateSettings
{
    NEXUS_PidChannelHandle pidChannel; /* yields parser type and index */

    NEXUS_TransportClientType clientType;
    unsigned clientIndex;
} NEXUS_TransportClientCaptureCreateSettings;

typedef struct NEXUS_TransportClientCaptureDescriptor {
    FILE * file;
    void * read;
    void * base;
    uint32_t validOffsetOffset;
    void * valid;
    void * end;
    unsigned unmoved;
} NEXUS_TransportClientCaptureDescriptor;

#define MAX_XC_CAP_NAME_LEN (1+2+1+2) /* parserType ('I' | 'P') + parserIndex + clientType ('R' | 'M' | 'X') + clientIndex */
typedef struct NEXUS_TransportClientCapture {
    char name[MAX_XC_CAP_NAME_LEN];
    NEXUS_TransportClientCaptureCreateSettings createSettings;
    bool running;
    unsigned no;
    NEXUS_ThreadHandle thread;
    NEXUS_TransportClientCaptureDescriptor xc;
} NEXUS_TransportClientCapture;
#endif

#if NEXUS_TRANSPORT_RS_CAPTURE_ENABLED

#include <stdio.h>

typedef struct NEXUS_TransportRsCaptureCreateSettings
{
    bool isPlayback;
    unsigned parserIndex;
} NEXUS_TransportRsCaptureCreateSettings;

typedef struct NEXUS_TransportRsCaptureDescriptor {
    FILE * file;
    void * read;
    void * base;
    uint32_t validOffsetOffset;
    void * valid;
    void * end;
    void * wrap;
    uint32_t wrapOffsetOffset;
    unsigned unmoved;
} NEXUS_TransportRsCaptureDescriptor;

#define MAX_RS_CAP_NAME_LEN (1+2) /* parserType ('I' | 'P') + parserIndex (2 digits) */

typedef struct NEXUS_TransportRsCapture {
    char name[MAX_RS_CAP_NAME_LEN];
    NEXUS_TransportRsCaptureCreateSettings createSettings;
    bool running;
    unsigned no;
    NEXUS_ThreadHandle thread;
    NEXUS_TransportRsCaptureDescriptor rs;
} NEXUS_TransportRsCapture;
#endif

typedef struct NEXUS_Remux_P_PidChannel {
    BLST_S_ENTRY(NEXUS_Remux_P_PidChannel) link;
    NEXUS_PidChannelHandle  pidChn;
} NEXUS_Remux_P_PidChannel;

struct NEXUS_Remux {
    NEXUS_OBJECT(NEXUS_Remux);
#if BXPT_NUM_REMULTIPLEXORS
    BXPT_Remux_Handle xptRmx ;
#endif
    NEXUS_RemuxSettings settings;
    unsigned index;      /* This channels index. */
    unsigned parserBandCount;
#if defined(NEXUS_NUM_PARSER_BANDS) && defined(NEXUS_NUM_PLAYPUMPS)
#define NEXUS_NUM_REMUX_PARSER_INPUTS (NEXUS_NUM_PARSER_BANDS + NEXUS_NUM_PLAYPUMPS)
#elif defined(NEXUS_NUM_PARSER_BANDS)
#define NEXUS_NUM_REMUX_PARSER_INPUTS (NEXUS_NUM_PARSER_BANDS)
#else
#define NEXUS_NUM_REMUX_PARSER_INPUTS (NEXUS_NUM_PLAYPUMPS)
#endif
    NEXUS_RemuxParserBandwidth remuxParserBandwidth[NEXUS_NUM_REMUX_PARSER_INPUTS];
    unsigned pidChannelCount[NEXUS_NUM_REMUX_PARSER_INPUTS];
    bool started;
    BLST_S_HEAD(NEXUS_Remux_P_PidChannels, NEXUS_Remux_P_PidChannel) pid_list;
};

/* encapsulation of rave error interrupts and counting. a single instance of this struct maps to a single rave context */
struct NEXUS_Rave_P_ErrorCounter {
    BXPT_RaveCx_Handle rave;
    unsigned emuErr, pusiErr, teiErr, ccErr, cdbOverflow, itbOverflow;
    BINT_CallbackHandle emuErr_int, pusiErr_int, teiErr_int, ccErr_int, cdbOverflow_int, itbOverflow_int;
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrEventCallback;
    NEXUS_TimerHandle rave_overflow_int_timer;
};

/* a container object for NEXUS_Rave_P_ErrorCounter, in order to create per-pidchannel links of ErrorCounters */
struct NEXUS_Rave_P_ErrorCounter_Link {
    struct NEXUS_Rave_P_ErrorCounter *counter;
    BLST_S_ENTRY(NEXUS_Rave_P_ErrorCounter_Link) pidchannel_link;
};

#if NEXUS_RAVE_OUTPUT_CAPTURE_ENABLED
#include <stdio.h>

typedef struct NEXUS_RaveCaptureCreateSettings
{
    NEXUS_RaveHandle rave;
} NEXUS_RaveCaptureCreateSettings;

typedef struct NEXUS_RaveCaptureDescriptor {
    FILE * file;
    void * read;
    void * base;
    uint32_t validOffsetOffset;
    void * valid;
    uint32_t wrapOffsetOffset;
    void * wrap;
    unsigned unmoved;
} NEXUS_RaveCaptureDescriptor;

typedef struct NEXUS_RaveCapture {
    int index;
    bool running;
    NEXUS_RaveCaptureCreateSettings createSettings;
    unsigned no;
    NEXUS_ThreadHandle thread;
    NEXUS_RaveCaptureDescriptor cdb;
    NEXUS_RaveCaptureDescriptor itb;
} NEXUS_RaveCapture;
#endif

/* this is the implementation of NEXUS_RaveHandle */
struct NEXUS_Rave {
    NEXUS_OBJECT(NEXUS_Rave);
    BXPT_RaveCx_Handle raveHandle; /* This is the RAVE channel, aka device. */
    NEXUS_RaveSettings settings;
    NEXUS_RaveOpenSettings openSettings;
    BAVC_XptContextMap xptContextMap;
    struct {
        BMMA_Heap_Handle heap;
        BMMA_Block_Handle block;
        void *ptr;
    } cdb, itb;
#if BXPT_P_HAS_AVS_PLUS_WORKAROUND
    struct {
        BXPT_RaveCx_Handle raveHandle;
        BMMA_Block_Handle cdbBlock, itbBlock;
        void *itb_ptr, *cdb_ptr;
    } avsReference;
#endif
    unsigned index; /* HW index */
    unsigned array_index; /* SW index into pTransport->rave[0].context[] */
    bool enabled;
    NEXUS_P_HwPidChannel *pidChannel; /* master */
    NEXUS_TimerHandle timer;
    BSTD_DeviceOffset lastValid; /* CDB_VALID monitoring */
    uint64_t numOutputBytes;
    bool useSecureHeap;
#if NEXUS_SW_RAVE_SUPPORT
    struct {
        BMMA_Heap_Handle heap;
        BMMA_Block_Handle block;
        int index; /* HW index, -1 if unused */
        BXPT_RaveCx_Handle raveHandle;
        BAVC_XptContextMap xptContextMap;
        bool enabled;
        void *extensionData;
    } swRave;
#endif

    struct NEXUS_Rave_P_ErrorCounter raveErrors;

#if NEXUS_RAVE_OUTPUT_CAPTURE_ENABLED
    NEXUS_RaveCapture * cap;
#endif
#if NEXUS_TRANSPORT_CLIENT_CAPTURE_ENABLED
    NEXUS_TransportClientCapture * xcCap;
#endif
    struct {
        const NEXUS_Rave_P_ItbEntry *DataPtr;
    } readItbEvents;

    /* slave pidchannel support */
    BLST_S_ENTRY(NEXUS_Rave) pidchannel_link; /* for NEXUS_PidChannel.raves */
};

int nexus_rave_add_one_pid(NEXUS_RaveHandle rave, NEXUS_P_HwPidChannel *pidChannel);
void nexus_rave_remove_one_pid(NEXUS_RaveHandle rave, NEXUS_P_HwPidChannel *pidChannel);

NEXUS_Error NEXUS_RaveErrorCounter_Init_priv(struct NEXUS_Rave_P_ErrorCounter *r, BXPT_RaveCx_Handle rave);
void NEXUS_RaveErrorCounter_Reset_priv(struct NEXUS_Rave_P_ErrorCounter *r);
void NEXUS_RaveErrorCounter_Uninit_priv(struct NEXUS_Rave_P_ErrorCounter *r);

struct NEXUS_StcChannelDecoderConnection {
    NEXUS_OBJECT(NEXUS_StcChannelDecoderConnection);
    BLST_Q_ENTRY(NEXUS_StcChannelDecoderConnection) link;
    NEXUS_StcChannelHandle parent;
    BPCRlib_StcDecIface pcrlibInterface; /* interface expected by pcrlib */
    bool stcValid;
    NEXUS_StcChannelDecoderConnectionSettings settings;
    NEXUS_StcChannelDecoderConnectionStatus status;
    NEXUS_StcChannelDecoderFifoWatchdogStatus fifoWatchdogStatus;
};

struct NEXUS_StcChannelSnapshot
{
    NEXUS_OBJECT(NEXUS_StcChannelSnapshot);
    BLST_Q_ENTRY(NEXUS_StcChannelSnapshot) link;
    NEXUS_StcChannelHandle parent;
#if BXPT_NUM_STC_SNAPSHOTS > 0
    BXPT_PcrOffset_StcSnapshot magSnapshot;
#endif
    NEXUS_StcChannelSnapshotSettings settings;
    NEXUS_StcChannelSnapshotStatus status;
};

typedef struct NEXUS_StcChannelPidChannelEntry
{
    BLST_Q_ENTRY(NEXUS_StcChannelPidChannelEntry) link;
    unsigned pidChannelIndex;
    unsigned refcnt;
} NEXUS_StcChannelPidChannelEntry;
typedef BLST_Q_HEAD(NEXUS_StcChannelPidChannelList, NEXUS_StcChannelPidChannelEntry) NEXUS_StcChannelPidChannelList;
typedef BLST_Q_HEAD(NEXUS_StcChannelSnapshotList, NEXUS_StcChannelSnapshot) NEXUS_StcChannelSnapshotList;
typedef BLST_Q_HEAD(NEXUS_StcChannelDecoderConnectionQueue, NEXUS_StcChannelDecoderConnection) NEXUS_StcChannelDecoderConnectionQueue;
/* this is the implementation of NEXUS_StcChannelHandle */
struct NEXUS_StcChannel {
    NEXUS_OBJECT(NEXUS_StcChannel);
    unsigned index;
    bool isStcTrick;
    bool frozen;
    unsigned increment;
    unsigned prescale;
    bool stcValid;
    bool nonRealTime;
    BXPT_PcrOffset_Handle pcrOffset;
    unsigned stcIndex; /* STC HW index */
    bool deferIndexChange;
    NEXUS_StcChannelSettings settings;
    NEXUS_TimebaseHandle timebase;
    bool modifyDefaultTimebase;
    BPCRlib_Channel pcrlibChannel;
    NEXUS_StcChannelDecoderConnectionQueue decoders;
    bool swPcrOffsetEnabled;
    unsigned swPcrOffset; /* set if swPcrOffsetEnabled = true */
#if NEXUS_HAS_ASTM
    struct
    {
        NEXUS_StcChannelAstmSettings settings;
    } astm;
#endif
    NEXUS_StcChannelDecoderConnectionHandle lowLatencyDecoder;
    NEXUS_StcChannelHandle pairedChannel;
    NEXUS_StcChannelHandle lastStallingStcChannel;
    bool lastCounterValueValid;
    uint64_t lastCounterValue;
    NEXUS_StcChannelSnapshotList snapshots;
    NEXUS_StcChannelPidChannelList pids;
};

/* this is the implementation of NEXUS_PidChannelHandle */
struct NEXUS_PidChannel {
    NEXUS_OBJECT(NEXUS_PidChannel);
    BLST_S_ENTRY(NEXUS_PidChannel) link; /* link to link multiple pid-channels to same NEXUS_P_HwPidChannel */
    NEXUS_P_HwPidChannel *hwPidChannel; /* pointer to HW pid_channel */
    bool open; /* if true, public Close not called */
    bool enabled; /* NEXUS_PidChannelSettings.enabled is aggregated */
};


/* flags for pidchannel destinations */
#define NEXUS_PIDCHANNEL_P_DESTINATION_RAVE    (1 << 0)
#define NEXUS_PIDCHANNEL_P_DESTINATION_MESSAGE (1 << 1)
#define NEXUS_PIDCHANNEL_P_DESTINATION_REMUX0  (1 << 2)
#define NEXUS_PIDCHANNEL_P_DESTINATION_REMUX1  (1 << 3)

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_P_HwPidChannel);

/* this is the 'master' pid-channel - one instance per HW pid channel */
struct NEXUS_P_HwPidChannel {
    NEXUS_OBJECT(NEXUS_P_HwPidChannel);
    BLST_S_HEAD(nexus_sw_pidchannel_list, NEXUS_PidChannel) swPidChannels; /* list of SW PidChannels that are using this HW PidChannel */
    BLST_S_ENTRY(NEXUS_P_HwPidChannel) link;
    NEXUS_ParserBandHandle parserBand;
    NEXUS_PidChannelStatus status;
    unsigned combinedPid;
    NEXUS_PidChannelSettings settings;
    NEXUS_PlaypumpHandle playpump; /* set if this is a playpump pidchannel */
    NEXUS_DmaJobHandle dma;
    struct {
        NEXUS_RaveHandle rave;
        NEXUS_PidChannelScramblingSettings settings;
    } scramblingCheck;

    BLST_S_HEAD(nexus_pidchannel_rave_counters, NEXUS_Rave_P_ErrorCounter_Link) raveCounters;

    /* slave pidchannel support */
    NEXUS_P_HwPidChannel *master; /* set to master if a slave */
    BLST_S_HEAD(nexus_pidchannel_slaves, NEXUS_P_HwPidChannel) slaves; /* if master, list of slaves */
    BLST_S_HEAD(nexus_pidchannel_raves, NEXUS_Rave) raves; /* raves links to this master. never set for slaves. */
    BLST_S_ENTRY(NEXUS_P_HwPidChannel) slave_link; /* for NEXUS_PidChannel.slaves */

    unsigned destinations; /* current destinations. see NEXUS_PIDCHANNEL_P_DESTINATION_* */
    bool settingsPrivValid;
    NEXUS_Playpump_OpenPidChannelSettings_priv settingsPriv;
    NEXUS_PidChannelHandle bypassKeyslotPidChannel; /* if set, security cleanup required and refcnt bumped on this pid channel until it's done */
    bool neverEnableContinuityCount;
};

#ifndef NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES
    #ifndef BXPT_NUM_MESG_BUFFERS
        /* On devices where BXPT_NUM_MESG_BUFFERS wasn't defined, the hw only supports 128 buffers/handles. */
        #define NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES 128
    #elif (BXPT_NUM_MESG_BUFFERS == 0)
    /* Software only message filtering */
        #define NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES 128
    #else
        /* On other devices, the PID2BUF table size is defined in bxpt_capabilities.h */
        #define NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES BXPT_NUM_MESG_BUFFERS
    #endif
#endif

/* this is the impl of NEXUS_MessageHandle */
#ifndef NEXUS_USE_SW_FILTER /* SW message filter uses separate impl maintained in nexus_message_swfilter.c */
struct NEXUS_Message {
    NEXUS_OBJECT(NEXUS_Message);
    NEXUS_MessageSettings settings;
    NEXUS_MessageStartSettings startSettings;
    NEXUS_MessageStatus status;
    unsigned FilterNum;
    unsigned BankNum;
    int MesgBufferNum; /* -1 means the buffer num has not been assigned */
    unsigned PidChannelNum;
    BXPT_PsiMessageSettings psiMessageSettings;

    NEXUS_MemoryBlockHandle mmaBlock;
    void *buffer; /* actual buffer in use. cached addr. */
    unsigned bufferSize;

    NEXUS_MemoryBlockHandle mmaAllocatedBlock;
    void *allocatedBuffer; /* buffer allocated by Open if bufferSize is set. can be overridden by Start buffer. */

    unsigned lastGetBufferLength; /* last length returned by NEXUS_Message_GetBuffer. this could be from the main
                                     buffer or wrappedMessage. */

    struct {
        NEXUS_MemoryBlockHandle mmaBlock;
        void *buffer; /* size is settings.maxContiguousMessageSize */
        unsigned length; /* size of message in the buffer */
        unsigned amountConsumed;
    } wrappedMessage;

    struct {
        const void *buffer; /* point to data last retrieved from XPT PI */
        unsigned length; /* length of data last retrieved from XPT PI */
        unsigned amountConsumed; /* total amount consumed from the buffer/length last reported by XPT PI */
    } getBufferState;

#define NEXUS_MESSAGE_P_MAX_ADDITIONAL_FILTERS 32
    struct {
        bool used;
        unsigned filterNum;
    } additionalFilters[NEXUS_MESSAGE_P_MAX_ADDITIONAL_FILTERS];

    bool started;

#if NEXUS_MESSAGE_USE_CHECK_TIMER
/* if you stop getting dataReady callbacks, it could be because you aren't calling ReadComplete. this was a fix after
nexus_message_pid2buf.c was released for a while, so I'm adding this debug option. */
    unsigned noReadCompleteCount;
    NEXUS_TimerHandle checkTimer;
#endif

    NEXUS_IsrCallbackHandle dataReady, overflow, psiLengthError, crcError, pesLengthError, pesStartCodeError;
    struct {
        uint8_t *buffer;
        unsigned size, wptr, rptr;
        NEXUS_TimerHandle timer;
    } copy;
};
#endif /* NEXUS_USE_SW_FILTER */

/**
NEXUS_MAX_INPUT_BANDS comes from RDB. It defines the range of valid
input bands; not all within the range may be valid.
**/
#if defined BCHP_XPT_FE_IB20_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 21
#elif defined BCHP_XPT_FE_IB19_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 20
#elif defined BCHP_XPT_FE_IB18_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 19
#elif defined BCHP_XPT_FE_IB17_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 18
#elif defined BCHP_XPT_FE_IB16_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 17
#elif defined BCHP_XPT_FE_IB15_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 16
#elif defined BCHP_XPT_FE_IB14_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 15
#elif defined BCHP_XPT_FE_IB13_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 14
#elif defined BCHP_XPT_FE_IB12_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 13
#elif defined BCHP_XPT_FE_IB11_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 12
#elif defined BCHP_XPT_FE_IB10_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 11
#elif defined BCHP_XPT_FE_IB9_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 10
#elif defined BCHP_XPT_FE_IB8_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 9
#elif defined BCHP_XPT_FE_IB7_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 8
#elif defined BCHP_XPT_FE_IB6_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 7
#elif defined BCHP_XPT_FE_IB5_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 6
#elif defined BCHP_XPT_FE_IB4_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 5
#elif defined BCHP_XPT_FE_IB3_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 4
#elif defined BCHP_XPT_FE_IB2_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 3
#elif defined BCHP_XPT_FE_IB1_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 2
#elif defined BCHP_XPT_FE_IB0_SYNC_COUNT
#define NEXUS_MAX_INPUT_BANDS 1
#else
#define NEXUS_MAX_INPUT_BANDS 0
#endif

struct NEXUS_ParserBand
{
    NEXUS_OBJECT(NEXUS_ParserBand);
    NEXUS_ParserBand enumBand; /* enum variant */
    unsigned hwIndex;
    NEXUS_ParserBandSettings settings;
    int refcnt;
    int pidChannels;
    bool enabled;
    BINT_CallbackHandle ccErrorInt;
    NEXUS_IsrCallbackHandle ccErrorCallback;
    unsigned ccErrorCount;
    BINT_CallbackHandle teiErrorInt;
    NEXUS_IsrCallbackHandle teiErrorCallback;
    unsigned teiErrorCount;
    BINT_CallbackHandle lengthErrorInt;
    NEXUS_IsrCallbackHandle lengthErrorCallback;
    unsigned lengthErrorCount;
    bool mpodBand; /* true if used as virtual band for mpod */
    void *gcbSwHandle;

#if NEXUS_TRANSPORT_RS_CAPTURE_ENABLED
    NEXUS_TransportRsCapture *rsCap;
#endif
};

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_ParserBand);

struct NEXUS_Timebase
{
    NEXUS_OBJECT(NEXUS_Timebase);
    NEXUS_Timebase enumTimebase; /* enum variant */
    unsigned hwIndex;
    BXPT_PCR_Handle pcr;
    bool isDss;
    bool acquired;
    NEXUS_TimebaseSettings settings;
    NEXUS_TimebaseStatus status;
    BINT_CallbackHandle intMonitorCallback;
    NEXUS_IsrCallbackHandle monitorCallback;
    BINT_CallbackHandle intPcrErrorCallback;
    NEXUS_IsrCallbackHandle pcrErrorCallback;
    NEXUS_VideoFrameRate hdDviFrameRate;
    NEXUS_VideoFrameRate vdecFrameRate;
#if NEXUS_HAS_ASTM
    struct
    {
        bool permitted;
        NEXUS_TimebaseAstmSettings settings;
        bool sourceTypeSaved;
        NEXUS_TimebaseSourceType savedSourceType;
    } astm;
#endif /* NEXUS_HAS_ASTM */
    unsigned onePcrErrCount, twoPcrErrCount, phaseSaturationEventCount;
    BINT_CallbackHandle intOnePcrErr;
    BINT_CallbackHandle intTwoPcrErr;
    BINT_CallbackHandle intPhaseSaturation;
};

typedef enum NEXUS_XptDataInterrupt {
    NEXUS_XptDataInterrupt_ePsiLengthError,
    NEXUS_XptDataInterrupt_eCrcError,
    NEXUS_XptDataInterrupt_ePesLengthError,
    NEXUS_XptDataInterrupt_ePesStartCodeError,
    NEXUS_XptDataInterrupt_eMax
} NEXUS_XptDataInterrupt;

struct NEXUS_Transport_P_State {
    NEXUS_ModuleHandle transportModule;
    BXPT_Handle xpt;
    BPCRlib_Handle pcrlib;
    unsigned powerCount;

#if NEXUS_MAX_INPUT_BANDS
    struct {
        NEXUS_InputBandSettings settings;
        NEXUS_TransportType transportType;
        bool enabled;
    } inputBand[NEXUS_MAX_INPUT_BANDS];
#endif
#if NEXUS_NUM_PARSER_BANDS
    NEXUS_ParserBandHandle parserBand[NEXUS_NUM_PARSER_BANDS];
#endif
#if NEXUS_NUM_PLAYPUMPS
    struct {
        NEXUS_PlaypumpHandle playpump; /* dynamically allocated by NEXUS_Playpump_Open */
    } playpump[NEXUS_NUM_PLAYPUMPS];
#endif
#ifndef BXPT_NUM_RAVE_CONTEXTS
#define BXPT_NUM_RAVE_CONTEXTS BXPT_P_MAX_RAVE_CONTEXTS
#endif
    NEXUS_RecpumpHandle recpump[BXPT_NUM_RAVE_CONTEXTS];

    BLST_S_HEAD(NEXUS_Transport_P_PidChannels, NEXUS_P_HwPidChannel) pidChannels;

#if BXPT_NUM_PCRS
    NEXUS_TimebaseHandle timebase[BXPT_NUM_PCRS];
#endif

#if BXPT_NUM_PCR_OFFSET_CHANNELS
    NEXUS_StcChannelHandle stcChannel[BXPT_NUM_PCR_OFFSET_CHANNELS];
#endif

    struct {
        BXPT_Rave_Handle channel;
        struct NEXUS_Rave *context[BXPT_NUM_RAVE_CONTEXTS];
    } rave[BXPT_NUM_RAVE_CHANNELS];

    struct {
        NEXUS_MessageHandle handle[NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES];
        BINT_CallbackHandle xptDataInterrupt[NEXUS_XptDataInterrupt_eMax];
#if !NEXUS_USE_SW_FILTER
        unsigned bank_refcnt[BXPT_NUM_FILTER_BANKS];
#endif
    } message;
#if BXPT_HAS_WAKEUP_PKT_SUPPORT
    struct {
        BINT_CallbackHandle intPacketFoundCallback;
        NEXUS_TransportWakeupSettings *settings;
    } wakeup;
#endif

    struct {
        bool postResumePending;
#if NEXUS_NUM_PARSER_BANDS
        bool parserEnabled[NEXUS_NUM_PARSER_BANDS];
#endif
    } standby;

#if NEXUS_HAS_XPT_DMA
    struct {
        BXPT_Dma_Handle dma;
        BLST_S_HEAD(NEXUS_DmaContexts, NEXUS_Dma) dmaHandles;
    } dmaChannel[NEXUS_NUM_DMA_CHANNELS];

    BLST_S_HEAD(NEXUS_Transport_P_DmaPidChannels, NEXUS_PidChannel) dmaPidChannels;
    unsigned hwDmaPidChannelRefCnt[BXPT_DMA_NUM_PID_CHANNELS];
#endif

    NEXUS_TransportModuleInternalSettings moduleSettings;
    NEXUS_TransportModuleSettings settings;

    BKNI_EventHandle pidChannelEvent;
    NEXUS_MtsifPidChannelState mtsifPidChannelState[NEXUS_NUM_PID_CHANNELS];

    struct {
        /* nexus only counts ibp-input errors for now (pbp-input errors are ignored) */
        struct {
            unsigned mpodIbp[32], ibp[32];
        } rsbuff;

        struct {
            unsigned ibp2rave[32], ibp2msg[32], ibp2rmx0[32], ibp2rmx1[32];
        } xcbuff;
        unsigned inputBuffer;
    } overflow;

#if BXPT_NUM_MTSIF
    unsigned mtsifLengthErrors[BXPT_NUM_MTSIF];
    unsigned mtsifBandDetect[BXPT_NUM_MTSIF];
    unsigned mtsifStatus[BXPT_NUM_MTSIF];
#endif
};

/* global module handle & data */
extern struct NEXUS_Transport_P_State g_NEXUS_Transport_P_State;
#define pTransport (&g_NEXUS_Transport_P_State)

typedef struct NEXUS_Tsmf_P_State
{
    BLST_S_HEAD(NEXUS_TsmfContexts, NEXUS_Tsmf) handles;
} NEXUS_Tsmf_P_State;

extern NEXUS_Tsmf_P_State g_NEXUS_Tsmf_P_State;

#ifdef NEXUS_HAS_ETBG
typedef struct NEXUS_Etbg_P_State
{
   BLST_S_HEAD(NEXUS_EtbgGroups, NEXUS_Etbg) handles;
   unsigned numGroups;
   unsigned groupMap;
} NEXUS_Etbg_P_State;

extern NEXUS_Etbg_P_State g_NEXUS_Etbg_P_State;
#endif

#if NEXUS_TRANSPORT_EXTENSION_TBG
struct NEXUS_Tbg_P_State {
    uint8_t markerTag;
    struct {
        bool changed;
        NEXUS_ParserBand primaryBand;
        unsigned unmappedPidChannel;
    } band[NEXUS_NUM_PARSER_BANDS];
};
extern struct NEXUS_Tbg_P_State g_NEXUS_Tbg_P_State;
#endif

void NEXUS_Playpump_P_ConsumerStarted(NEXUS_PlaypumpHandle p);
void NEXUS_Playpump_P_GetOpenSettings(NEXUS_PlaypumpHandle p, NEXUS_PlaypumpOpenSettings *pOpenSettings);

NEXUS_Error NEXUS_Timebase_P_StartMonitor(NEXUS_TimebaseHandle timebase);
void NEXUS_Timebase_P_StopMonitor(NEXUS_TimebaseHandle timebase);
NEXUS_Error NEXUS_Timebase_P_SetSettings(NEXUS_TimebaseHandle timebase, const NEXUS_TimebaseSettings *pSettings);
void NEXUS_Timebase_P_GetSettings(NEXUS_TimebaseHandle timebase, NEXUS_TimebaseSettings *pSettings);

void NEXUS_ParserBand_P_UninitAll(void);
void NEXUS_ParserBand_P_SetEnable(NEXUS_ParserBandHandle parserBand);
void NEXUS_ParserBand_P_ResetOverflowCounts(NEXUS_ParserBandHandle parserBand);
void NEXUS_ParserBand_P_CountCcErrors_isr(void);
void NEXUS_ParserBand_P_GetSettings(NEXUS_ParserBandHandle band, NEXUS_ParserBandSettings *pSettings);
NEXUS_Error NEXUS_ParserBand_P_SetSettings(NEXUS_ParserBandHandle band, const NEXUS_ParserBandSettings *pSettings);
NEXUS_ParserBandHandle NEXUS_ParserBand_Resolve_priv(NEXUS_ParserBand band);
void NEXUS_ParserBand_P_GetDefaultSettings(unsigned index, NEXUS_ParserBandSettings * pSettings);

NEXUS_Error NEXUS_InputBand_P_SetTransportType(NEXUS_InputBand inputBand, NEXUS_TransportType transportType);

void NEXUS_P_HwPidChannel_StopScramblingCheck(NEXUS_P_HwPidChannel *pidChannel);
NEXUS_P_HwPidChannel *NEXUS_P_HwPidChannel_Open(NEXUS_ParserBandHandle parserBand, NEXUS_PlaypumpHandle playpump, unsigned pid,
    const NEXUS_PidChannelSettings *pSettings, bool bandContinuityCountEnabled);
NEXUS_Error NEXUS_P_HwPidChannel_CalcEnabled( NEXUS_P_HwPidChannel *pidChannel);
void NEXUS_P_HwPidChannel_CloseAll(NEXUS_P_HwPidChannel *hwPidChannel); /* this function would close all SW pid channels, and then close hwPidChannel */
NEXUS_Error NEXUS_Playpump_P_HwPidChannel_Disconnect(NEXUS_PlaypumpHandle p, NEXUS_P_HwPidChannel *pidChannel);
NEXUS_Error NEXUS_DmaJob_P_HwPidChannel_Disconnect(NEXUS_DmaJobHandle handle, NEXUS_P_HwPidChannel *pidChannel);
NEXUS_Error NEXUS_DmaJob_P_Wait(NEXUS_DmaJobHandle handle);
NEXUS_Error NEXUS_P_HwPidChannel_GetStatus(NEXUS_P_HwPidChannel *pidChannel, NEXUS_PidChannelStatus *pStatus);
unsigned nexus_p_xpt_parser_band(NEXUS_P_HwPidChannel *hwPidChannel);
void nexus_p_set_pid_cc(NEXUS_P_HwPidChannel *hwPidChannel, const NEXUS_PidChannelSettings *pSettings);

/* create new instance of [SW] NEXUS_PidChannel from existing HW PidChannel , this will link two together */
NEXUS_PidChannelHandle NEXUS_PidChannel_P_Create(NEXUS_P_HwPidChannel *hwPidChannel);
bool NEXUS_PidChannel_P_IsDataPresent(NEXUS_PidChannelHandle pidChannel);

void NEXUS_Message_P_FireInterrupt_isr(NEXUS_MessageHandle msg, unsigned pidChannelIndex, NEXUS_XptDataInterrupt xptDataInterrupt);
bool NEXUS_Message_P_HasCallback(NEXUS_MessageHandle msg, NEXUS_XptDataInterrupt xptDataInterrupt);

void NEXUS_Transport_P_SetInterrupts(void);

void NEXUS_Vcxo_Init(void);
void NEXUS_Vcxo_Uninit(void);

/* calculate rave index from CDB_READ register address */
#define NEXUS_RAVE_INDEX(CDB_READ)    (((CDB_READ) - BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR) / (BCHP_XPT_RAVE_CX1_AV_CDB_READ_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR))

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_PacketSub);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Message);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Remux);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Mpod);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_MpodInput);

#if NEXUS_TRANSPORT_EXTENSION_TSIO
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_TsioCard);
#endif

#if NEXUS_TRANSPORT_EXTENSION_EPR
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Epr);
#endif

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Tsmf);

#if NEXUS_TRANSPORT_EXTENSION_TBG
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Tbg);
#endif

#if NEXUS_HAS_XPT_DMA
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Dma);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_DmaJob);
#endif

#if NEXUS_HAS_ETBG
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Etbg);
#endif

#if NEXUS_RAVE_OUTPUT_CAPTURE_ENABLED
void NEXUS_RaveCapture_GetDefaultCreateSettings(NEXUS_RaveCaptureCreateSettings * pCreateSettings);
NEXUS_RaveCapture * NEXUS_RaveCapture_Create(const NEXUS_RaveCaptureCreateSettings * pCreateSettings);
void NEXUS_RaveCapture_Destroy(NEXUS_RaveCapture *cap);
void NEXUS_RaveCapture_Open(NEXUS_RaveCapture *cap);
void NEXUS_RaveCapture_Close(NEXUS_RaveCapture *cap);
void NEXUS_RaveCapture_Flush(NEXUS_RaveCapture *cap);
NEXUS_Error NEXUS_RaveCapture_Start(NEXUS_RaveCapture *cap);
void NEXUS_RaveCapture_Stop(NEXUS_RaveCapture *cap);
#endif

#if NEXUS_TRANSPORT_CLIENT_CAPTURE_ENABLED
void NEXUS_TransportClientCapture_GetDefaultCreateSettings(NEXUS_TransportClientCaptureCreateSettings * pCreateSettings);
NEXUS_TransportClientCapture * NEXUS_TransportClientCapture_Create(const NEXUS_TransportClientCaptureCreateSettings * pCreateSettings);
void NEXUS_TransportClientCapture_Destroy(NEXUS_TransportClientCapture *cap);
void NEXUS_TransportClientCapture_Open(NEXUS_TransportClientCapture *cap);
void NEXUS_TransportClientCapture_Close(NEXUS_TransportClientCapture *cap);
void NEXUS_TransportClientCapture_Flush(NEXUS_TransportClientCapture *cap);
NEXUS_Error NEXUS_TransportClientCapture_Start(NEXUS_TransportClientCapture *cap);
void NEXUS_TransportClientCapture_Stop(NEXUS_TransportClientCapture *cap);
#endif

#if NEXUS_TRANSPORT_RS_CAPTURE_ENABLED
void NEXUS_TransportRsCapture_GetDefaultCreateSettings(NEXUS_TransportRsCaptureCreateSettings * pCreateSettings);
NEXUS_TransportRsCapture * NEXUS_TransportRsCapture_Create(const NEXUS_TransportRsCaptureCreateSettings * pCreateSettings);
void NEXUS_TransportRsCapture_Destroy(NEXUS_TransportRsCapture *cap);
void NEXUS_TransportRsCapture_Open(NEXUS_TransportRsCapture *cap);
void NEXUS_TransportRsCapture_Close(NEXUS_TransportRsCapture *cap);
void NEXUS_TransportRsCapture_Flush(NEXUS_TransportRsCapture *cap);
NEXUS_Error NEXUS_TransportRsCapture_Start(NEXUS_TransportRsCapture *cap);
void NEXUS_TransportRsCapture_Stop(NEXUS_TransportRsCapture *cap);
#endif

#if NEXUS_GISB_BLOCKER_ENABLED
void NEXUS_P_ConfigGisbBlocker(uint32_t start, uint32_t end, bool on);
#endif

#ifdef __cplusplus
}
#endif

#endif
