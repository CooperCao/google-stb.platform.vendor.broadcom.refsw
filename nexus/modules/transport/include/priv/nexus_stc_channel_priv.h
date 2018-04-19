/***************************************************************************
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
*
* API Description:
*  Internal timebase APIs.  Primarily used by audio and video decoders.
*
***************************************************************************/
#ifndef NEXUS_STCCHANNEL_PRIV_H__
#define NEXUS_STCCHANNEL_PRIV_H__

#include "nexus_types.h"
#include "nexus_stc_channel.h"
#include "nexus_timebase_priv.h"
#include "bavc.h"
#include "bavc_xpt.h"

#ifdef __cplusplus
extern "C" {
#endif

#if NEXUS_HAS_ASTM
typedef enum NEXUS_StcChannelTsmMode
{
    NEXUS_StcChannelTsmMode_eStcMaster = 0, /* STC is seeded with either the video or audio PTS, depending on which decoder makes an STC request first */
    NEXUS_StcChannelTsmMode_eVideoMaster, /* STC is seeded with the video PTS */
    NEXUS_StcChannelTsmMode_eAudioMaster, /* STC is seeded with the audio PTS */
    NEXUS_StcChannelTsmMode_eOutputMaster, /* No tsm is performed.  Output clock pulls data through decoder.  Also called VSYNC mode for video. */
    NEXUS_StcChannelTsmMode_eMax
} NEXUS_StcChannelTsmMode;

typedef struct NEXUS_StcChannelAstmSettings
{
    bool enabled;
    NEXUS_StcChannelMode mode;
    NEXUS_StcChannelTsmMode tsmMode;
    unsigned int syncLimit;
} NEXUS_StcChannelAstmSettings;

typedef struct NEXUS_StcChannelAstmStatus
{
    NEXUS_TimebaseHandle timebase;
} NEXUS_StcChannelAstmStatus;
#endif /* NEXUS_HAS_ASTM */

typedef enum NEXUS_StcChannelDecoderType
{
    NEXUS_StcChannelDecoderType_eVideo,    /* Video decoder */
    NEXUS_StcChannelDecoderType_eAudio,    /* Audio decoder */
    NEXUS_StcChannelDecoderType_eMax
} NEXUS_StcChannelDecoderType;

typedef struct NEXUS_StcChannelDecoderConnection * NEXUS_StcChannelDecoderConnectionHandle;

typedef struct NEXUS_StcChannelDecoderConnectionStatus
{
    bool flush;
    bool zeroFill;
} NEXUS_StcChannelDecoderConnectionStatus;

typedef struct NEXUS_StcChannelDecoderFifoWatchdogStatus
{
    bool isHung;
    bool frameSyncUnlocked;
    unsigned percentFull;
    bool tsmWait;
} NEXUS_StcChannelDecoderFifoWatchdogStatus;

typedef BERR_Code (*NEXUS_StcChannel_GetPtsCallback)(void *pContext, BAVC_PTSInfo *pPTSInfo);
typedef BERR_Code (*NEXUS_StcChannel_GetCdbLevelCallback)(void *pContext, unsigned *pCdbLevel);
typedef BERR_Code (*NEXUS_StcChannel_StcValidCallback)(void *pContext);
typedef BERR_Code (*NEXUS_StcChannel_SetPcrOffset)(void *pContext, uint32_t pcrOffset);
typedef BERR_Code (*NEXUS_StcChannel_GetPcrOffset)(void *pContext, uint32_t *pPcrOffset);

typedef struct NEXUS_StcChannelDecoderConnectionSettings
{
    NEXUS_StcChannelDecoderType           type;               /* Required. Specifies decoder type. */
    unsigned                              priority;           /* Reserved for future use. Currently, priority is determined by connection order. */
    NEXUS_StcChannel_GetPtsCallback       getPts_isr;         /* Required */
    NEXUS_StcChannel_GetCdbLevelCallback  getCdbLevel_isr;    /* Required */
    NEXUS_StcChannel_StcValidCallback     stcValid_isr;       /* The STC has been marked valid. Required if decoder invalidates the STC during playback mode. */
    NEXUS_StcChannel_SetPcrOffset         setPcrOffset_isr;   /* Optional. If set, then don't set the Serial STC but call this function with the needed PCR_OFFSET. */
    NEXUS_StcChannel_GetPcrOffset         getPcrOffset_isr;   /* Optional. If set, then call this function to get the offset to add w/ the Serial STC. */
    void                                 *pCallbackContext;        /* Context for all callbacks. */
    BKNI_EventHandle                      statusUpdateEvent;  /* Required. Allows STC channel to notify decoders of updates in status */
    bool                                  lowLatencyEnabled;  /* Optional. Can only apply to one decoder */
} NEXUS_StcChannelDecoderConnectionSettings;

void NEXUS_StcChannel_GetDefaultDecoderConnectionSettings_priv(
    NEXUS_StcChannelDecoderConnectionSettings *pSettings /* [out] */
);

/*
When a decoder (audio or video) receives a StcChannel, it must call NEXUS_StcChannel_ConnectDecoder_priv and provide
the isr callbacks and event handles. These are needed by StcChannel to perform basic lipsync. This is usually called when starting decode.
Call NEXUS_StcChannel_DisconnectDecoder_priv when stopping decode.
*/
NEXUS_StcChannelDecoderConnectionHandle NEXUS_StcChannel_ConnectDecoder_priv(
    NEXUS_StcChannelHandle stcChannel,
    const NEXUS_StcChannelDecoderConnectionSettings * pSettings
);

/*
Disconnect callbacks set up by NEXUS_StcChannel_ConnectDecoder_priv, usually called when stopping decode.
*/
void NEXUS_StcChannel_DisconnectDecoder_priv(
    NEXUS_StcChannelDecoderConnectionHandle connection
    );

/*
Enable Output to a specified PID Channel
*/
BERR_Code NEXUS_StcChannel_EnablePidChannel_priv(
    NEXUS_StcChannelHandle handle,
    NEXUS_PidChannelHandle pidChannel
    );

/*
Disable Output to a specified PID Channel
*/
void NEXUS_StcChannel_DisablePidChannel_priv(
    NEXUS_StcChannelHandle handle,
    NEXUS_PidChannelHandle pidChannel
    );

/*
Get the index of the STC block (not PCR_OFFSET context) used by this StcChannel
*/
void NEXUS_StcChannel_GetIndex_priv(
    NEXUS_StcChannelHandle handle,
    unsigned *pIndex /* [out] */
    );

/*
When a decoder receives a "RequestStc" interrupt (also called "FirstPts" interrupt), it must
call this StcChannel function.
*/
BERR_Code NEXUS_StcChannel_RequestStc_isr(
    NEXUS_StcChannelDecoderConnectionHandle decoder,
    const struct BAVC_PTSInfo *pPts
    );

/*
When a decoder receives a "PtsError" interrupt , it must
call this StcChannel function.
*/
BERR_Code NEXUS_StcChannel_PtsError_isr(
    NEXUS_StcChannelDecoderConnectionHandle decoder,
    const struct BAVC_PTSInfo *pPts     /* last PTS from the stream, in 45KHz units */
    );

/**
Get the current STC for this StcChannel
**/
void NEXUS_StcChannel_GetStc_isr(
    NEXUS_StcChannelHandle stcChannel,
    uint32_t *pStc /* [out] */
    );

#if NEXUS_HAS_ASTM
void NEXUS_StcChannel_GetAstmSettings_priv(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_StcChannelAstmSettings * pAstmSettings  /* [out] */
    );

NEXUS_Error NEXUS_StcChannel_SetAstmSettings_priv(
    NEXUS_StcChannelHandle stcChannel,
    const NEXUS_StcChannelAstmSettings * pAstmSettings
    );

NEXUS_Error NEXUS_StcChannel_GetAstmStatus_priv(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_StcChannelAstmStatus * pAstmStatus /* [out] */
    );
#endif /* NEXUS_HAS_ASTM */

/*
Watchdog processing.
Each decoder must report whether it believes it is hung.
This is needed to avoid a TSM deadlock.
One flavor of TSM deadlock is when one decoder's fifo fills while waiting for a PTS to mature, causing band hold. Then, other decoder's fifo drains because of the band hold. Decode is stuck.
*/

void NEXUS_StcChannel_GetDefaultDecoderFifoWatchdogStatus_priv(
    NEXUS_StcChannelDecoderFifoWatchdogStatus *pStatus
    );

void NEXUS_StcChannel_ReportDecoderHang_priv(
    NEXUS_StcChannelDecoderConnectionHandle decoder,
    const NEXUS_StcChannelDecoderFifoWatchdogStatus *pStatus
    );

void NEXUS_StcChannel_ReportDecoderFlush_priv(
    NEXUS_StcChannelDecoderConnectionHandle decoder
    );

void NEXUS_StcChannel_ReportDecoderZeroFill_priv(
    NEXUS_StcChannelDecoderConnectionHandle decoder
    );

void NEXUS_StcChannel_GetDecoderConnectionStatus_priv(
    NEXUS_StcChannelDecoderConnectionHandle decoder,
    NEXUS_StcChannelDecoderConnectionStatus * pStatus
    );

/**
Summary:
Get the current Serial STC value (PCR_OFFSET STC without the offset).
**/
void NEXUS_StcChannel_GetSerialStc_priv(
    NEXUS_StcChannelHandle handle,
    uint32_t *pStc                                /* [out] Current Stc */
    );

/**
Summary:
Set pcr offset context into acquire mode for pcr_offset entry generation
**/
void NEXUS_StcChannel_SetPcrOffsetContextAcquireMode_priv(
    NEXUS_StcChannelHandle stcChannel
    );

/*
Summary:
Type of event that coult increment the STC
*/
typedef enum NEXUS_StcChannelTriggerMode {
    NEXUS_StcChannelTriggerMode_eTimebase,            /* Increment STC by timebase */
    NEXUS_StcChannelTriggerMode_eExternalTrig,        /* Increment by external trigger source. */
    NEXUS_StcChannelTriggerMode_eSoftIncrement,       /* Increment when STC_INC_TRIG register is written */
    NEXUS_StcChannelTriggerMode_eMax
} NEXUS_StcChannelTriggerMode;


/*
Summary:
Configuration of non-realtime mode of STC channel
*/
typedef struct NEXUS_StcChannelNonRealtimeSettings {
    NEXUS_StcChannelTriggerMode triggerMode; /* Event that will increment the STC */
    uint64_t stcIncrement; /* used when triggerMode == NEXUS_StcChannel_TriggerMode_eExternalTrig */
    unsigned externalTrigger;/* Identifies which external trigger is used when TriggerMode == _eExternalTrig */
} NEXUS_StcChannelNonRealtimeSettings;

void NEXUS_StcChannel_GetDefaultNonRealtimeSettings_priv (
    NEXUS_StcChannelNonRealtimeSettings *pSettings
        );

NEXUS_Error NEXUS_StcChannel_SetNonRealtimeConfig_priv(
    NEXUS_StcChannelHandle handle,
    NEXUS_StcChannelNonRealtimeSettings *pSettings,
    bool reset
    );

/**
Summary:
Get address of the soft increment register.
**/
void NEXUS_StcChannel_GetSoftIncrementRegOffset_priv(
    NEXUS_StcChannelHandle handle,
    BAVC_Xpt_StcSoftIncRegisters *regMap
    );

typedef struct NEXUS_StcChannelSnapshot * NEXUS_StcChannelSnapshotHandle;

typedef enum NEXUS_StcChannelSnapshotMode
{
    NEXUS_StcChannelSnapshotMode_eLegacy,
    NEXUS_StcChannelSnapshotMode_eLsb32,
    NEXUS_StcChannelSnapshotMode_eMsb32,
    NEXUS_StcChannelSnapshotMode_eMax
} NEXUS_StcChannelSnapshotMode;

typedef struct NEXUS_StcChannelSnapshotSettings
{
    unsigned triggerIndex;
    NEXUS_StcChannelSnapshotMode mode;
} NEXUS_StcChannelSnapshotSettings;

typedef struct NEXUS_StcChannelSnapshotStatus
{
    uint32_t stcLoAddr;
    uint32_t stcHiAddr;
    unsigned index;
} NEXUS_StcChannelSnapshotStatus;

NEXUS_StcChannelSnapshotHandle NEXUS_StcChannel_OpenSnapshot_priv(NEXUS_StcChannelHandle stcChannel);
void NEXUS_StcChannel_CloseSnapshot_priv(NEXUS_StcChannelSnapshotHandle snapshot);
void NEXUS_StcChannel_GetSnapshotSettings_priv(NEXUS_StcChannelSnapshotHandle snapshot, NEXUS_StcChannelSnapshotSettings * pSettings);
NEXUS_Error NEXUS_StcChannel_SetSnapshotSettings_priv(NEXUS_StcChannelSnapshotHandle snapshot, const NEXUS_StcChannelSnapshotSettings * pSettings);
NEXUS_Error NEXUS_StcChannel_GetSnapshotStatus_priv(NEXUS_StcChannelSnapshotHandle snapshot, NEXUS_StcChannelSnapshotStatus * pStatus);

void NEXUS_StcChannel_GetRate_priv(NEXUS_StcChannelHandle stcChannel, unsigned * increment, unsigned * prescale);
bool NEXUS_StcChannel_IsFrozen_priv(NEXUS_StcChannelHandle stcChannel);

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_StcChannel);

#ifdef __cplusplus
}
#endif

#endif
