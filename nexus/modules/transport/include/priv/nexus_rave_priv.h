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
#ifndef NEXUS_RAVE_PRIV_H__
#define NEXUS_RAVE_PRIV_H__

#include "nexus_types.h"
#include "nexus_rave.h"
#include "nexus_pid_channel.h"
#include "bmma.h"
#include "bavc_xpt.h"

#ifdef __cplusplus
extern "C" {
#endif

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Rave);

typedef struct NEXUS_RaveOpenSettings
{
    bool record;
    bool spliceEnabled;
    BAVC_CdbItbConfig config;
    bool supportedCodecs[NEXUS_VideoCodec_eMax]; /* needed for video SW RAVE contexts. unused for audio. */
    NEXUS_HeapHandle heap;  /* heap used for CDB allocation */
} NEXUS_RaveOpenSettings;

void NEXUS_Rave_GetDefaultOpenSettings_priv(
    NEXUS_RaveOpenSettings *pSettings /* [out] */
    );

NEXUS_RaveHandle NEXUS_Rave_Open_priv(  /* attr{destructor=NEXUS_Rave_Close_priv} */
    const NEXUS_RaveOpenSettings *pOpenSettings
    );

void NEXUS_Rave_Close_priv(
    NEXUS_RaveHandle handle
    );

typedef struct NEXUS_RaveSettings
{
    NEXUS_PidChannelHandle pidChannel;
    struct NEXUS_P_HwPidChannel *hwPidChannel; /* either pidChannel or hwPidChannel should be set, latter used internally in the transport module */
    bool bandHold;
    bool continuityCountEnabled;
    bool audioDescriptor;           /* If true, this context will mark audio dsecriptors */
    bool otfPvr;    /* If true, this context is used as input for the OTF PVR */
    bool disableReordering; /* if true, this context would not reorder video frames, where otherwise reordering is required */
    bool numOutputBytesEnabled; /* if true, count numOutputBytes for this RAVE context.
                             this requires a SW poll, so it's not an always-on feature. */
    bool nonRealTime;
    bool includeRepeatedItbStartCodes;
} NEXUS_RaveSettings;

void NEXUS_Rave_GetDefaultSettings_priv(
    NEXUS_RaveSettings *pSettings /* [out] */
    );

NEXUS_Error NEXUS_Rave_ConfigureVideo_priv(
    NEXUS_RaveHandle handle,
    NEXUS_VideoCodec codec,
    const NEXUS_RaveSettings *pSettings
    );

NEXUS_Error NEXUS_Rave_ConfigureAudio_priv(
    NEXUS_RaveHandle handle,
    NEXUS_AudioCodec codec,
    const NEXUS_RaveSettings *pSettings
    );

/* allow all data to pass */
NEXUS_Error NEXUS_Rave_ConfigureAll_priv(
    NEXUS_RaveHandle handle,
    const NEXUS_RaveSettings *pSettings
    );

typedef struct NEXUS_RaveStatus
{
    unsigned index; /* HW index of RAVE */
    int swRaveIndex; /* HW index of SW RAVE. -1 if unused */
    BAVC_XptContextMap xptContextMap; /* register information required for decoders to make connection.
                                         if SW RAVE is enabled, this will be the linked context */
    BMMA_Block_Handle itbBlock;
    BMMA_Block_Handle cdbBlock;
    uint64_t numOutputBytes; /* requires numOutputBytesEnabled = true */
    bool enabled;
    NEXUS_HeapHandle heap; /* heap used for rave allocations */
    bool crrEnabled; /* we have a secure heap and it is actually secure */
} NEXUS_RaveStatus;

NEXUS_Error NEXUS_Rave_GetStatus_priv(
    NEXUS_RaveHandle handle,
    NEXUS_RaveStatus *pStatus /* [out] */
    );

void NEXUS_Rave_Enable_priv(
    NEXUS_RaveHandle handle
    );

void NEXUS_Rave_Disable_priv(
    NEXUS_RaveHandle handle
    );

void NEXUS_Rave_Flush_priv(
    NEXUS_RaveHandle handle
    );

void NEXUS_Rave_SetBandHold(
    NEXUS_RaveHandle rave,
    bool enable
    );

void NEXUS_Rave_AddPidChannel_priv(
    NEXUS_RaveHandle rave,
    NEXUS_PidChannelHandle pidChannel
    );

void NEXUS_Rave_RemovePidChannel_priv(
    NEXUS_RaveHandle handle
    );

void NEXUS_Rave_GetCdbBufferInfo_isr(
    NEXUS_RaveHandle handle,
    unsigned *depth, /* [out] */
    unsigned *size   /* [out] */
    );

void NEXUS_Rave_GetItbBufferInfo(
    NEXUS_RaveHandle rave,
    unsigned *depth, /* [out] */
    unsigned *size   /* [out] */
    );

bool NEXUS_Rave_FindPts_priv(
    NEXUS_RaveHandle rave,
    uint32_t pts);

bool NEXUS_Rave_FindVideoStartCode_priv(
    NEXUS_RaveHandle rave,
    uint8_t startCode
    );

void NEXUS_Rave_GetAudioFrameCount_priv(
    NEXUS_RaveHandle rave,
    unsigned *pFrameCount     /* [out] */
    );

bool NEXUS_Rave_IsConsumableVideoElementAvailable_priv(NEXUS_RaveHandle rave);

bool NEXUS_Rave_CompareVideoStartCodeCount_priv(
    NEXUS_RaveHandle rave,
    unsigned threshold
    );

NEXUS_Error NEXUS_Rave_SetCdbThreshold_priv(
    NEXUS_RaveHandle rave,
    unsigned cdbDepth       /* CDB threshold in bytes (0 is default) */
    );

/**
Summary:
This function is used to determine if data is entering the CDB.
If the VALID and READ pointers become static over a period of time, then
either data has stopped flowing or the buffer is full and the RAVE band hold
is set.
**/
void NEXUS_Rave_GetCdbPointers_isr(
    NEXUS_RaveHandle rave,
    uint64_t *validPointer, /* [out] */
    uint64_t *readPointer /* [out] */
    );

/**
Summary:
Added to support NEXUS_VideoDecoder_GetMostRecentPts and NEXUS_VideoDecoder_GetFifoStatus
**/
NEXUS_Error NEXUS_Rave_GetPtsRange_priv(
    NEXUS_RaveHandle rave,
    uint32_t *pMostRecentPts,
    uint32_t *pLeastRecentPts
    );

NEXUS_Error NEXUS_Rave_P_UseSecureHeap(
    NEXUS_HeapHandle heap, /* user heap. can be null. */
    bool useSecureHeap,    /* backward compat. used if heap is null, or verified if heap is not null. */
    bool *pUseSecureHeap   /* out param if rave will use secure heap */
    );

#define NEXUS_RAVE_P_ITB_SIZE   16

typedef struct NEXUS_Rave_P_ItbEntry {
    uint32_t word[NEXUS_RAVE_P_ITB_SIZE/sizeof(uint32_t)];
} NEXUS_Rave_P_ItbEntry;

typedef enum NEXUS_Rave_P_ItbType {
    NEXUS_Rave_P_ItbType_eExtractedData =   0x00,
    NEXUS_Rave_P_ItbType_eBaseAddress =     0x20,
    NEXUS_Rave_P_ItbType_ePtsDts =          0x21,
    NEXUS_Rave_P_ItbType_ePcrOffset =       0x22,
    NEXUS_Rave_P_ItbType_eBtp =             0x23,
    NEXUS_Rave_P_ItbType_ePrivateHdr =      0x24,
    NEXUS_Rave_P_ItbType_eRts =             0x25,
    NEXUS_Rave_P_ItbType_ePcr =             0x26,
    NEXUS_Rave_P_ItbType_eIpStreamOut =     0x30,
    NEXUS_Rave_P_ItbType_eTermination =     0x70
} NEXUS_Rave_P_ItbType;

struct BXPT_Rave_ContextPtrs;

NEXUS_Error NEXUS_Rave_ScanItb_priv(NEXUS_RaveHandle rave, bool (*one_itb)(void *, const NEXUS_Rave_P_ItbEntry *), void *context);
NEXUS_Error NEXUS_Rave_CheckBuffer_priv(NEXUS_RaveHandle rave, struct BXPT_Rave_ContextPtrs *pCtxPtrs);
NEXUS_Error NEXUS_Rave_UpdateReadOffset_priv(NEXUS_RaveHandle rave, size_t CdbByteCount, size_t ItbByteCount);

typedef enum NEXUS_Rave_SpliceType {
    NEXUS_Rave_SpliceType_eDisabled,
    NEXUS_Rave_SpliceType_eStopPts,
    NEXUS_Rave_SpliceType_eStartPts,
    NEXUS_Rave_SpliceType_eStopLive
} NEXUS_Rave_SpliceType;

typedef struct NEXUS_Rave_SpliceSettings {
    NEXUS_Rave_SpliceType type;
    uint32_t pts;
    uint32_t ptsThreshold;
    void (*splicePoint)(void *, uint32_t pts);
    void *context;
} NEXUS_Rave_SpliceSettings;

NEXUS_Error NEXUS_Rave_SetSplicePoint_priv(
    NEXUS_RaveHandle rave,
    const NEXUS_Rave_SpliceSettings * pSettings
    );
#ifdef __cplusplus
}
#endif

#endif
