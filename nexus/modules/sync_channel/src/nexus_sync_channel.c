/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nexus_sync_channel_module.h"
#include "priv/nexus_core.h"
#include "priv/nexus_syslib_framework.h"
#include "priv/nexus_core_video.h"
#include "priv/nexus_video_input_priv.h"
#include "priv/nexus_video_window_priv.h"
#include "priv/nexus_audio_input_priv.h"
#include "priv/nexus_audio_output_priv.h"
#include "nexus_audio_decoder.h"
#include "priv/nexus_sync_channel_priv.h"
#include "bsynclib.h"

BDBG_MODULE(nexus_sync_channel);

/* module def */
NEXUS_ModuleHandle g_NEXUS_syncChannelModule;
struct {
    NEXUS_SyncChannelModuleSettings settings;
    NEXUS_SYSlib_ContextHandle syslibContext;
    BSYNClib_Handle synclib;
} g_NEXUS_syncChannel;

/****************************************
* Module functions
***************/

NEXUS_Error NEXUS_SyncChannel_P_ConnectVideoInput(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings);
void NEXUS_SyncChannel_P_DisconnectVideoInput(NEXUS_SyncChannelHandle syncChannel);
NEXUS_Error NEXUS_SyncChannel_P_ConnectVideoOutput(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index);
void NEXUS_SyncChannel_P_DisconnectVideoOutput(NEXUS_SyncChannelHandle syncChannel, unsigned int index);
NEXUS_Error NEXUS_SyncChannel_P_ConnectAudioInput(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index);
void NEXUS_SyncChannel_P_DisconnectAudioInput(NEXUS_SyncChannelHandle syncChannel, unsigned int index);
NEXUS_Error NEXUS_SyncChannel_P_ConnectAudioOutput(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index);
void NEXUS_SyncChannel_P_DisconnectAudioOutput(NEXUS_SyncChannelHandle syncChannel, unsigned int index);
static void NEXUS_Synclib_P_VideoInputDelayCallback_isr(void *context, int delay);
NEXUS_Error NEXUS_Synclib_P_SetVideoSink(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index);

void NEXUS_SyncChannelModule_GetDefaultSettings(NEXUS_SyncChannelModuleSettings *pSettings)
{
    BSYNClib_Settings synclibSettings;
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BSYNClib_GetDefaultSettings(&synclibSettings);
    pSettings->video.requireFullScreen = synclibSettings.sVideo.bRequireFullScreen;
    pSettings->video.tsmLockTimeout = synclibSettings.sVideo.uiTsmLockTimeout;
    pSettings->video.unmuteTimeout = synclibSettings.sVideo.uiUnmuteTimeout;
    pSettings->video.unconditionalUnmuteTimeout = synclibSettings.sVideo.uiUnconditionalUnmuteTimeout;
    pSettings->video.rateMismatchDetection.timeout = synclibSettings.sVideo.sRateMismatchDetection.uiTimeout;
    pSettings->video.rateMismatchDetection.acceptableMtbcLower = synclibSettings.sVideo.sRateMismatchDetection.uiAcceptableMtbcLower;
    pSettings->video.rateMismatchDetection.acceptableMtbcUpper = synclibSettings.sVideo.sRateMismatchDetection.uiAcceptableMtbcUpper;
    pSettings->video.rateMismatchDetection.acceptableTtlc = synclibSettings.sVideo.sRateMismatchDetection.uiAcceptableTtlc;
    pSettings->video.rateRematchDetection.timeout = synclibSettings.sVideo.sRateRematchDetection.uiTimeout;
    pSettings->video.rateRematchDetection.acceptableTtlc = synclibSettings.sVideo.sRateRematchDetection.uiAcceptableTtlc;
    pSettings->audio.unmuteTimeout = synclibSettings.sAudio.uiUnmuteTimeout;
    pSettings->audio.receiverDelayCompensation = synclibSettings.sAudio.uiReceiverDelayCompensation;
    pSettings->audio.unconditionalUnmuteTimeout = synclibSettings.sAudio.uiUnconditionalUnmuteTimeout;
}

static void NEXUS_SyncChannelModule_Print(void)
{
#if BDBG_DEBUG_BUILD
    BDBG_LOG(("Sync Channel:"));
    BDBG_LOG((" settings: a:%p v:%p", (void *)g_NEXUS_syncChannel.settings.modules.audio, (void *)g_NEXUS_syncChannel.settings.modules.display));
    BDBG_LOG((" handles: sys:%p sync:%p", (void *)g_NEXUS_syncChannel.syslibContext, (void *)g_NEXUS_syncChannel.synclib));
#endif
}

NEXUS_ModuleHandle NEXUS_SyncChannelModule_Init(const NEXUS_SyncChannelModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_SYSlib_ContextSettings syslibSettings;
    BSYNClib_Settings synclibSettings;
    BERR_Code rc;

    BDBG_ASSERT(!g_NEXUS_syncChannelModule);

    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pSettings->modules.display);
    BDBG_ASSERT(pSettings->modules.audio);

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow;
    moduleSettings.dbgPrint = NEXUS_SyncChannelModule_Print;
    moduleSettings.dbgModules = "nexus_sync_channel";
    g_NEXUS_syncChannelModule = NEXUS_Module_Create("sync_channel", &moduleSettings);
    if (!g_NEXUS_syncChannelModule) {
        return NULL;
    }

    g_NEXUS_syncChannel.settings = *pSettings;

    NEXUS_LockModule();
    NEXUS_SYSlib_GetDefaultContextSettings_priv(&syslibSettings);
    syslibSettings.module = NEXUS_MODULE_SELF;
    NEXUS_SYSlib_CreateContext_priv(&g_NEXUS_syncChannel.syslibContext, &syslibSettings);

    BSYNClib_GetDefaultSettings(&synclibSettings);
    synclibSettings.sVideo.bRequireFullScreen = pSettings->video.requireFullScreen;
    synclibSettings.sVideo.uiTsmLockTimeout = pSettings->video.tsmLockTimeout;
    synclibSettings.sVideo.uiUnmuteTimeout = pSettings->video.unmuteTimeout;
    synclibSettings.sVideo.uiUnconditionalUnmuteTimeout = pSettings->video.unconditionalUnmuteTimeout;
    synclibSettings.sVideo.sRateMismatchDetection.uiTimeout = pSettings->video.rateMismatchDetection.timeout;
    synclibSettings.sVideo.sRateMismatchDetection.uiAcceptableMtbcLower = pSettings->video.rateMismatchDetection.acceptableMtbcLower;
    synclibSettings.sVideo.sRateMismatchDetection.uiAcceptableMtbcUpper = pSettings->video.rateMismatchDetection.acceptableMtbcUpper;
    synclibSettings.sVideo.sRateMismatchDetection.uiAcceptableTtlc = pSettings->video.rateMismatchDetection.acceptableTtlc;
    synclibSettings.sVideo.sRateRematchDetection.uiTimeout = pSettings->video.rateRematchDetection.timeout;
    synclibSettings.sVideo.sRateRematchDetection.uiAcceptableTtlc = pSettings->video.rateRematchDetection.acceptableTtlc;
    synclibSettings.sAudio.uiUnmuteTimeout = pSettings->audio.unmuteTimeout;
    synclibSettings.sAudio.uiReceiverDelayCompensation = pSettings->audio.receiverDelayCompensation;
    synclibSettings.sAudio.uiUnconditionalUnmuteTimeout = pSettings->audio.unconditionalUnmuteTimeout;
    rc = BSYNClib_Open(&synclibSettings, &g_NEXUS_syncChannel.synclib);
    NEXUS_UnlockModule();
    if (rc) {BERR_TRACE(rc); goto error;}

    return g_NEXUS_syncChannelModule;

error:
    NEXUS_SyncChannelModule_Uninit();
    return NULL;
}

void NEXUS_SyncChannelModule_Uninit()
{
    NEXUS_LockModule();
    /* this may destroy timers and events, so it needs to happen before the syslib context shutdown */
    BSYNClib_Close(g_NEXUS_syncChannel.synclib);
    NEXUS_SYSlib_DestroyContext_priv(g_NEXUS_syncChannel.syslibContext);
    NEXUS_UnlockModule();

    NEXUS_Module_Destroy(g_NEXUS_syncChannelModule);
    g_NEXUS_syncChannelModule = NULL;
}

/****************************************
* API functions
***************/

/* Used to collect information to process callbacks from around the system */
struct NEXUS_SyncChannelContext {
    NEXUS_SyncChannelHandle syncChannel;
    unsigned index; /* SYNClib index */
    bool nonRealTime;
    union
    {
        NEXUS_VideoInputSyncSettings videoInput;
        NEXUS_VideoWindowSyncSettings window;
        NEXUS_AudioInputSyncSettings audioInput;
    } settings;
    bool connectionChanged;
};

#define NEXUS_SYNC_CHANNEL_VIDEO_INPUTS  1
#define NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS NEXUS_SYNC_CHANNEL_NUM_VIDEO_OUTPUTS
#define NEXUS_SYNC_CHANNEL_AUDIO_INPUTS  NEXUS_SYNC_CHANNEL_NUM_AUDIO_INPUTS
#define NEXUS_SYNC_CHANNEL_AUDIO_OUTPUTS 4

struct NEXUS_SyncChannel
{
    NEXUS_OBJECT(NEXUS_SyncChannel);
    NEXUS_SyncChannelSettings settings;
    NEXUS_SyncChannelStatus status;
    bool avTrickModeEnabled;
    bool synclibVsourceDelayCallbackEnabled;
    BKNI_EventHandle syncLockEvent;
    NEXUS_EventCallbackHandle syncLockEventCallback;
    struct NEXUS_SyncChannelContext vsourceContext;
    int vsourcePhaseOffset;
    BKNI_EventHandle phaseOffsetAdjustmentEvent;
    NEXUS_EventCallbackHandle phaseOffsetAdjustmentEventCallback;
    BKNI_EventHandle videoInputConnectionChangedEvent;
    NEXUS_EventCallbackHandle videoInputConnectionChangedCallback;
    BKNI_EventHandle audioInputStartEvent;
    NEXUS_EventCallbackHandle audioInputStartEventCallback;
    struct NEXUS_SyncChannelContext vsinkContext[NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS];
    struct NEXUS_SyncChannelContext asourceContext[NEXUS_SYNC_CHANNEL_AUDIO_INPUTS];
    struct NEXUS_SyncChannelContext asinkContext[NEXUS_SYNC_CHANNEL_AUDIO_OUTPUTS];

    struct
    {
        NEXUS_VideoWindowHandle window[NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS];
        NEXUS_AudioOutputHandle audioOutput[NEXUS_SYNC_CHANNEL_AUDIO_OUTPUTS];
    } outputs;

    NEXUS_VideoWindowHandle syncLockedWindow;
    bool syncLockFaked;
    bool displaysAligned;
    bool simpleVideoConnected;
    bool simpleAudioConnected;

    /* syslib info */
    BSYNClib_Channel_Handle synclibChannel;
    bool sarnoffLipsyncOffsetEnabled;
};

static void NEXUS_Synclib_P_ComputePhaseOffsetAdjustment_isr(NEXUS_SyncChannelHandle syncChannel);
static bool NEXUS_SyncChannel_P_AreDisplaysAligned_isr(NEXUS_SyncChannelHandle syncChannel);

static int NEXUS_SyncChannel_P_ComputeMaxDisplayDistance_isr(NEXUS_SyncChannelHandle syncChannel)
{
    int max = -0x7FFFFFFF;
    int min = 0x7FFFFFFF;
    unsigned i = 0;
    NEXUS_VideoWindowSyncStatus status;

    /*
     * Find min display position, find max display position
     * subtract and return
     */

    /* check all other windows for aligned displays */
    for (i = 0; i < NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS; i++)
    {
        if (syncChannel->outputs.window[i])
        {
            NEXUS_VideoWindow_GetSyncStatus_isr(syncChannel->outputs.window[i], &status);
            if (status.phaseDelay < min)
            {
                min = status.phaseDelay;
            }
            if (status.phaseDelay > max)
            {
                max = status.phaseDelay;
            }
        }
    }

    return max - min;
}

static unsigned NEXUS_Synclib_P_ComputeInterDisplayAudioOffset_isr(NEXUS_SyncChannelHandle syncChannel)
{
    unsigned offset = 0;

    NEXUS_OBJECT_ASSERT(NEXUS_SyncChannel, syncChannel);

    /*
     * if sink1 and sink2 are meant to be aligned, and have the same rate
     * but are off by 1 field, and the video source rate does not match the
     * sync-locked display, add the difference between the two displays
     * furthest apart to audio
     */
    if (NEXUS_SyncChannel_P_AreDisplaysAligned_isr(syncChannel))
    {
        int displayDistance = NEXUS_SyncChannel_P_ComputeMaxDisplayDistance_isr(syncChannel);
        if (displayDistance > 0)
        {
            BSYNClib_Convert_isrsafe(((unsigned)displayDistance) / 1000, BSYNClib_Units_eMilliseconds,
                BSYNClib_Units_e45KhzTicks, &offset);
            offset /= 2;
        }
        else
        {
            BDBG_ERR(("Something wrong with display distance calcs: %d", displayDistance));
        }
    }

    return offset;
}

/**********************
*
* SYNClib VideoSource = NEXUS VideoInput, which can be VideoDecoder, AnalogVideoDecoder, etc.
*
***********************/

static BERR_Code
NEXUS_Synclib_P_SetVideoSourceMute(void * pvParm1, int iParm2, unsigned index, bool bMute)
{
    NEXUS_SyncChannelHandle syncChannel = (NEXUS_SyncChannelHandle)pvParm1;
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_VideoInputSyncSettings syncSettings;

    BSTD_UNUSED(iParm2);
    BSTD_UNUSED(index);
    BDBG_OBJECT_ASSERT(syncChannel, NEXUS_SyncChannel);

    if (syncChannel->settings.videoInput)
    {
        BDBG_MSG(("[%p] SetVideoSourceMute(%u): %s", (void *)syncChannel, index, bMute ? "muted" : "unmuted"));
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.display);
        NEXUS_VideoInput_GetSyncSettings_priv(syncChannel->settings.videoInput, &syncSettings);
        syncSettings.mute = bMute;
        rc = NEXUS_VideoInput_SetSyncSettings_priv(syncChannel->settings.videoInput, &syncSettings);
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.display);

        /* update status */
        syncChannel->status.video.source.muted = syncSettings.mute;
    }
    else
    {
        BDBG_MSG(("[%p] Synclib called set video source mute with NULL input", (void *)syncChannel));
    }

    return rc;
}

extern const char * BSYNClib_P_UnitsStrings[];

static BERR_Code
NEXUS_Synclib_P_SetVideoSourceDelay(void * pvParm1, int iParm2, unsigned int index, BSYNClib_UnsignedValue * pDelay)
{
    NEXUS_SyncChannelHandle syncChannel = (NEXUS_SyncChannelHandle)pvParm1;
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_VideoInputSyncSettings syncSettings;

    BSTD_UNUSED(iParm2);
    BSTD_UNUSED(index);
    BDBG_OBJECT_ASSERT(syncChannel, NEXUS_SyncChannel);

    if (syncChannel->settings.videoInput)
    {
        if (syncChannel->displaysAligned)
        {
            BDBG_MSG(("[%p] SetVideoSourceDelay(%u): %u %s", (void *)syncChannel, index, pDelay->uiValue + syncChannel->vsourcePhaseOffset, BSYNClib_P_UnitsStrings[pDelay->eUnits]));
        }
        else
        {
            BDBG_MSG(("[%p] SetVideoSourceDelay(%u): %u %s", (void *)syncChannel, index, pDelay->uiValue, BSYNClib_P_UnitsStrings[pDelay->eUnits]));
        }

        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.display); /* TODO: bja - why display? */
        NEXUS_VideoInput_GetSyncSettings_priv(syncChannel->settings.videoInput, &syncSettings);
        BDBG_ASSERT(pDelay->eUnits == BSYNClib_Units_e45KhzTicks);
        if (syncChannel->displaysAligned)
        {
            syncSettings.delay = (unsigned)((int)pDelay->uiValue + syncChannel->vsourcePhaseOffset);
        }
        else
        {
            syncSettings.delay = pDelay->uiValue;
        }
        rc = NEXUS_VideoInput_SetSyncSettings_priv(syncChannel->settings.videoInput, &syncSettings);
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.display);

        /* update status */
        BSYNClib_ConvertSigned_isrsafe(syncSettings.delay, pDelay->eUnits,
            BSYNClib_Units_eMilliseconds, &syncChannel->status.video.source.delay.applied);
    }
    else
    {
        BDBG_MSG(("[%p] Synclib called set video source delay with NULL input", (void *)syncChannel));
    }

    return rc;
}

static BERR_Code
NEXUS_Synclib_P_SetVideoSourceDelayNotification(void * pvParm1, int iParm2, unsigned index, bool bEnable, BSYNClib_UnsignedValue *pThreshold)
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_VideoInputSyncSettings syncSettings;
    NEXUS_SyncChannelHandle syncChannel = (NEXUS_SyncChannelHandle)pvParm1;

    BSTD_UNUSED(iParm2);
    BSTD_UNUSED(index);
    BDBG_OBJECT_ASSERT(syncChannel, NEXUS_SyncChannel);

    /* save this for filtering here instead of in video source code */
    syncChannel->synclibVsourceDelayCallbackEnabled = bEnable;

    if (syncChannel->settings.videoInput)
    {
        NEXUS_VideoInputSyncStatus syncStatus;
        BDBG_MSG(("[%p] SetVideoSourceDelayNotification(%u): %s %u %s", (void *)syncChannel, index, bEnable? "enabled" : "disabled", pThreshold->uiValue, BSYNClib_P_UnitsStrings[pThreshold->eUnits]));
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.display);
        NEXUS_VideoInput_GetSyncSettings_priv(syncChannel->settings.videoInput, &syncSettings);
        syncSettings.delayCallbackThreshold = pThreshold->uiValue;
        rc = NEXUS_VideoInput_SetSyncSettings_priv(syncChannel->settings.videoInput, &syncSettings);
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.display);

        /* update status */
        BSYNClib_ConvertSigned_isrsafe(syncSettings.delayCallbackThreshold, pThreshold->eUnits,
            BSYNClib_Units_eMilliseconds, &syncChannel->status.video.source.delay.notificationThreshold);

        /* if sync has enabled the callback, and the delay is valid, then grab the value */
        if (syncChannel->synclibVsourceDelayCallbackEnabled)
        {
            BKNI_EnterCriticalSection();
            NEXUS_VideoInput_GetSyncStatus_isr(syncChannel->settings.videoInput, &syncStatus);
            if (syncStatus.delayValid)
            {
                NEXUS_Synclib_P_VideoInputDelayCallback_isr(&syncChannel->vsourceContext, syncStatus.delay);
            }
            BKNI_LeaveCriticalSection();
        }
    }
    else
    {
        BDBG_MSG(("[%p] Synclib called set video source delay notification with NULL input", (void *)syncChannel));
    }

    return rc;
}

static bool NEXUS_SyncChannel_P_AreDisplaysAligned_isr(NEXUS_SyncChannelHandle syncChannel)
{
    bool aligned;
    unsigned i;
    NEXUS_VideoWindowSyncStatus status;

    /* check all other windows for aligned displays */
    for (i = 0; i < NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS; i++)
    {
        if (syncChannel->outputs.window[i])
        {
            NEXUS_VideoWindow_GetSyncStatus_isr(syncChannel->outputs.window[i], &status);
            if (status.aligned)
            {
                break;
            }
        }
    }

    if (i < NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS)
    {
        aligned = true;
    }
    else
    {
        aligned = false;
    }

    return aligned;
}

static int NEXUS_SyncChannel_P_FindSyncLockedWindow_isr(NEXUS_SyncChannelHandle syncChannel)
{
    NEXUS_VideoWindowSyncStatus status;

    int syncLockedWindow = -1;
    unsigned i;

    /* check all other windows for aligned displays */
    for (i = 0; i < NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS; i++)
    {
        if (syncChannel->outputs.window[i])
        {
            NEXUS_VideoWindow_GetSyncStatus_isr(syncChannel->outputs.window[i], &status);
            if (status.syncLocked)
            {
                syncLockedWindow = i;
                break;
            }
        }
    }

    return syncLockedWindow;
}

static int NEXUS_SyncChannel_P_GuessPrimaryDisplay_isr(NEXUS_SyncChannelHandle syncChannel)
{
#if NEXUS_NUM_DISPLAYS > 1
    int primaryDisplay = -1;
    int hdmiDisplay = -1;
    int componentDisplay = -1;
    unsigned i;

    /* check all other windows for aligned displays */
    for (i = 0; i < NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS; i++)
    {
        if (syncChannel->outputs.window[i])
        {
            if (NEXUS_VideoWindow_HasOutput_isr(syncChannel->outputs.window[i], NEXUS_VideoOutputType_eHdmi))
            {
                hdmiDisplay = i;
            }
            if (NEXUS_VideoWindow_HasOutput_isr(syncChannel->outputs.window[i], NEXUS_VideoOutputType_eComponent))
            {
                componentDisplay = i;
            }
        }
    }

    if (hdmiDisplay != -1)
    {
        primaryDisplay = hdmiDisplay;
    }
    else if (componentDisplay != -1)
    {
        primaryDisplay = componentDisplay;
    }
    else
    {
        BDBG_WRN(("[%p] Could not guess primary display", (void *)syncChannel));
    }

    return primaryDisplay;
#else
    BSTD_UNUSED(syncChannel);
    return 0;
#endif
}

static unsigned
NEXUS_Synclib_P_ComputeVideoSourceCustomDelay_isr(const NEXUS_SyncChannelHandle syncChannel, const NEXUS_VideoInputSyncStatus * pStatus)
{
    unsigned customDelay = 0;

    /* all delays are in units of ms */
    customDelay = syncChannel->settings.customVideoDelay;
    customDelay += pStatus->customPtsOffset;

    return customDelay;
}

static void
NEXUS_Synclib_P_VideoInputStartCallback_isr(void *context, int param)
{
    BSYNClib_VideoSource_Config vsourceConfig;
    struct NEXUS_SyncChannelContext *syncContext = (struct NEXUS_SyncChannelContext *)context;
    NEXUS_VideoInputSyncStatus status;

    BSTD_UNUSED(param);
    BDBG_OBJECT_ASSERT(syncContext->syncChannel, NEXUS_SyncChannel);

    if (syncContext->syncChannel->settings.videoInput)
    {
        NEXUS_VideoInput_GetSyncStatus_isr(syncContext->syncChannel->settings.videoInput, &status);
        BDBG_MSG(("[%p] VideoInputStart(%u) started=%d digital=%d", (void *)syncContext->syncChannel, syncContext->index, status.started, status.digital));
        BSYNClib_Channel_GetVideoSourceConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsourceConfig);
        vsourceConfig.bStarted = status.started;
        vsourceConfig.bDigital = status.digital;
        vsourceConfig.bNonRealTime = status.nonRealTime;
        vsourceConfig.bLastPictureHeld = status.lastPictureHeld;

        vsourceConfig.sDelay.sCustom.eUnits = BSYNClib_Units_eMilliseconds;
        vsourceConfig.sDelay.sCustom.uiValue = NEXUS_Synclib_P_ComputeVideoSourceCustomDelay_isr(syncContext->syncChannel, &status);

        syncContext->nonRealTime = status.nonRealTime;

        BSYNClib_Channel_SetVideoSourceConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsourceConfig);

        /* status updates */
        syncContext->syncChannel->status.nonRealTime = syncContext->nonRealTime;
        syncContext->syncChannel->status.video.source.started = vsourceConfig.bStarted;
        syncContext->syncChannel->status.video.source.digital = vsourceConfig.bDigital;
        syncContext->syncChannel->status.video.source.lastPictureHeld = vsourceConfig.bLastPictureHeld;
        syncContext->syncChannel->status.video.source.delay.custom = vsourceConfig.sDelay.sCustom.uiValue;
        syncContext->syncChannel->status.video.source.delay.applied = -1;
        syncContext->syncChannel->status.video.source.delay.measured = -1;
        syncContext->syncChannel->status.video.source.delay.phase = -1;
        syncContext->syncChannel->status.video.source.delay.notificationThreshold = -1;
        syncContext->syncChannel->status.video.source.format.height = 0;
        syncContext->syncChannel->status.video.source.format.frameRate = 0;
    }
    else
    {
        BDBG_WRN(("[%p] VideoInputStartStop event received for NULL input", (void *)syncContext->syncChannel));
    }
}

static void
NEXUS_Synclib_P_VideoInputFormatCallback_isr(void *context, int param)
{
    BSYNClib_VideoSource_Config vsourceConfig;
    struct NEXUS_SyncChannelContext *syncContext = (struct NEXUS_SyncChannelContext *)context;
    NEXUS_VideoInputSyncStatus status;

    BSTD_UNUSED(param);
    BDBG_OBJECT_ASSERT(syncContext->syncChannel, NEXUS_SyncChannel);

    if (syncContext->syncChannel->settings.videoInput)
    {
        NEXUS_VideoInput_GetSyncStatus_isr(syncContext->syncChannel->settings.videoInput, &status);
        BDBG_MSG(("[%p] VideoInputFormat(%u) height=%d, interlaced=%d, framerate=%d", (void *)syncContext->syncChannel, syncContext->index, status.height, status.interlaced, status.frameRate));
        BSYNClib_Channel_GetVideoSourceConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsourceConfig);
        vsourceConfig.sFormat.bReceived = true;
        vsourceConfig.sFormat.uiHeight = status.height;
        vsourceConfig.sFormat.bInterlaced = status.interlaced;
        vsourceConfig.sFormat.eFrameRate = status.frameRate;
        BSYNClib_Channel_SetVideoSourceConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsourceConfig);

        /* status updates */
        syncContext->syncChannel->status.video.source.format.height = vsourceConfig.sFormat.uiHeight;
        syncContext->syncChannel->status.video.source.format.interlaced = vsourceConfig.sFormat.bInterlaced;
        syncContext->syncChannel->status.video.source.format.frameRate =
            NEXUS_P_RefreshRate_FromFrameRate_isrsafe(NEXUS_P_FrameRate_FromMagnum_isrsafe(vsourceConfig.sFormat.eFrameRate));
    }
    else
    {
        BDBG_WRN(("[%p] VideoInputFormat event received for NULL input", (void *)syncContext->syncChannel));
    }
}

static void
NEXUS_Synclib_P_VideoInputDelayCallback_isr(void *context, int delay)
{
    BSYNClib_VideoSource_Config vsourceConfig;
    struct NEXUS_SyncChannelContext *syncContext = (struct NEXUS_SyncChannelContext *)context;

    BDBG_OBJECT_ASSERT(syncContext->syncChannel, NEXUS_SyncChannel);
    /* PR49294 added check for callback ignore flag */
    if (syncContext->syncChannel->settings.videoInput)
    {
        /* if callback is enabled by synclib or displays are aligned */
        if (syncContext->syncChannel->synclibVsourceDelayCallbackEnabled || syncContext->syncChannel->displaysAligned)
        {
            BDBG_MSG(("[%p] VideoInputDelay(%u) %d", (void *)syncContext->syncChannel, syncContext->index, delay));
            BSYNClib_Channel_GetVideoSourceConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsourceConfig);
            vsourceConfig.sDelay.bReceived = true;
            vsourceConfig.sDelay.sMeasured.uiValue = delay;
            vsourceConfig.sDelay.sMeasured.eUnits = BSYNClib_Units_e45KhzTicks;
            BSYNClib_Channel_SetVideoSourceConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsourceConfig);

            NEXUS_Synclib_P_ComputePhaseOffsetAdjustment_isr(syncContext->syncChannel);

            /* status updates */
            BSYNClib_ConvertSigned_isrsafe(vsourceConfig.sDelay.sMeasured.uiValue, vsourceConfig.sDelay.sMeasured.eUnits,
                BSYNClib_Units_eMilliseconds, &syncContext->syncChannel->status.video.source.delay.measured);
        }
        else
        {
            BDBG_MSG(("[%p] VideoInputDelay %d received while disabled, ignored", (void *)syncContext->syncChannel, delay));
        }
    }
    else
    {
        BDBG_WRN(("[%p] VideoInputDelay event received for NULL input", (void *)syncContext->syncChannel));
    }
}

static void
NEXUS_Synclib_P_VideoInputConnectionChangedCallback_isr(void *context, int unused)
{
    unsigned int i;
    struct NEXUS_SyncChannelContext *syncContext = (struct NEXUS_SyncChannelContext *)context;

    BDBG_OBJECT_ASSERT(syncContext->syncChannel, NEXUS_SyncChannel);
    BSTD_UNUSED(unused);
    BKNI_SetEvent(syncContext->syncChannel->videoInputConnectionChangedEvent);

    /* mark all sinks attached to this sync channel as changed */
    for (i = 0; i < NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS; i++)
    {
        syncContext->syncChannel->vsinkContext[i].connectionChanged = true;
    }

    /* connection change is imminent, make sure sync channel doesn't use syncLockedWindow */
    syncContext->syncChannel->syncLockedWindow = NULL;
}

/**********************
*
* SYNClib VideoSink = NEXUS VideoWindow
*
***********************/

static BERR_Code
NEXUS_Synclib_P_SetVideoSinkDelay(void * pvParm1, int iParm2, unsigned int index, BSYNClib_UnsignedValue * pDelay)
{
    NEXUS_SyncChannelHandle syncChannel = (NEXUS_SyncChannelHandle)pvParm1;
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_VideoWindowSyncSettings syncSettings;
    NEXUS_VideoWindowSyncStatus syncStatus;
    uint64_t cnvtDelay;

    BDBG_OBJECT_ASSERT(syncChannel, NEXUS_SyncChannel);

    BSTD_UNUSED(iParm2);

    BDBG_MSG(("[%p] SetVideoSinkDelay(%u): %u %s", (void *)syncChannel, index, pDelay->uiValue, BSYNClib_P_UnitsStrings[pDelay->eUnits]));

    if (syncChannel->outputs.window[index]) /* nexus window handle will persist, even when vdc window handle is gone */
    {
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.display);
        NEXUS_VideoWindow_GetSyncSettings_priv(syncChannel->outputs.window[index], &syncSettings);
        /* SW7425-705: only apply multi-window delay eq if source not NRT */
        if (!syncChannel->vsourceContext.nonRealTime)
        {
            unsigned refreshRate;

            /* this should be in 27 MHz units, since we said we prefer that in create */
            BDBG_ASSERT(pDelay->eUnits == BSYNClib_Units_e27MhzTicks);
            cnvtDelay = pDelay->uiValue;

            /* It turns out that refresh rate isn't always set by the time we get
            this callback.  Only format is set, but refresh rate comes from a VDC
            callback which may not have come yet. */
            if (syncChannel->status.video.sinks[index].format.refreshRate)
            {
                refreshRate = syncChannel->status.video.sinks[index].format.refreshRate;
            }
            else
            {
                /* if refresh rate has not yet come, use format, which has to be
                present at this point, otherwise we would not be in this callback */
                BKNI_EnterCriticalSection();
                rc = NEXUS_VideoWindow_GetSyncStatus_isr(syncChannel->outputs.window[index], &syncStatus);
                BKNI_LeaveCriticalSection();
                if (!rc)
                {
                    refreshRate = NEXUS_P_RefreshRate_FromFrameRate_isrsafe(
                        NEXUS_P_FrameRate_FromMagnum_isrsafe(syncStatus.frameRate));
                }
                else
                {
                    refreshRate = 0;
                }
            }

            cnvtDelay *= refreshRate;
            cnvtDelay /= 27000000;
            /* round up */
            if (cnvtDelay % 1000 >= 500)
            {
                cnvtDelay += 1000;
            }
            cnvtDelay /= 1000; /* convert from milliHertz to vsync buffers */
            syncSettings.delay = cnvtDelay;
        }
        else
        {
            BDBG_MSG(("[%p] NRT -> multi-window delay eq set to zero on window %u", (void *)syncChannel, index));
            syncSettings.delay = 0;
        }
        rc = NEXUS_VideoWindow_SetSyncSettings_priv(syncChannel->outputs.window[index], &syncSettings);
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.display);

        /* update status */
        syncChannel->vsourceContext.settings.window.delay = syncSettings.delay;
        if (syncChannel->status.video.sinks[index].format.refreshRate)
        {
            syncChannel->status.video.sinks[index].delay.applied =
                1000 * 1000 * syncSettings.delay / syncChannel->status.video.sinks[index].format.refreshRate;
        }
    }
    else
    {
        BDBG_MSG(("[%p] Synclib called set video sink delay with NULL window %u", (void *)syncChannel, index));
    }

    return rc;
}

static void
NEXUS_Synclib_P_VideoOutputCloseCallback_isr(void *context, int param)
{
    BSYNClib_VideoSink_Config vsinkConfig;
    struct NEXUS_SyncChannelContext *syncContext = (struct NEXUS_SyncChannelContext *)context;

    BSTD_UNUSED(param);
    BDBG_OBJECT_ASSERT(syncContext->syncChannel, NEXUS_SyncChannel);

    BDBG_MSG(("[%p] VideoOutputClose(%u): %p", (void *)syncContext->syncChannel, syncContext->index, (void*)syncContext->syncChannel->outputs.window[syncContext->index]));

    /* don't need to disconnect if closing */
    syncContext->syncChannel->outputs.window[syncContext->index] = NULL;
    syncContext->connectionChanged = false;
    BSYNClib_Channel_GetVideoSinkConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsinkConfig);
    vsinkConfig.bSynchronize = false;
    BSYNClib_Channel_SetVideoSinkConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsinkConfig);
}

static void
NEXUS_Synclib_P_VideoOutputSyncLockChangeHandler_isr(NEXUS_SyncChannelHandle syncChannel, int windex, bool syncLocked, bool delayValid)
{
    BSYNClib_VideoSink_Config vsinkConfig;
    bool syncLockChanged = false;

    BDBG_MSG(("[%p] VideoOutputSyncLockChange(%u) syncLocked=%d", (void *)syncChannel, windex, syncLocked));
    BSYNClib_Channel_GetVideoSinkConfig_isr(syncChannel->synclibChannel, windex, &vsinkConfig);
    if (syncLocked != vsinkConfig.bSyncLocked)
    {
        syncLockChanged = true;
    }
    vsinkConfig.bSyncLocked = syncLocked;
    BSYNClib_Channel_SetVideoSinkConfig_isr(syncChannel->synclibChannel, windex, &vsinkConfig);

    if (syncLockChanged && delayValid)
    {
        NEXUS_Synclib_P_ComputePhaseOffsetAdjustment_isr(syncChannel);
    }
}

static void
NEXUS_Synclib_P_VideoOutputStateChangeHandler_isr(NEXUS_SyncChannelHandle syncChannel, int windex, const NEXUS_VideoWindowSyncStatus * pStatus)
{
    BSYNClib_VideoSink_Config vsinkConfig;

    BDBG_MSG(("[%p] VideoOutputStateChange(%u) visible=%d", (void *)syncChannel, windex, pStatus->visible));
    BSYNClib_Channel_GetVideoSinkConfig_isr(syncChannel->synclibChannel, windex, &vsinkConfig);
    vsinkConfig.bForcedCaptureEnabled = pStatus->forcedCaptureEnabled;
    vsinkConfig.bMasterFrameRateEnabled = pStatus->masterFrameRateEnabled;
    vsinkConfig.bFullScreen = pStatus->fullScreen;
    vsinkConfig.bVisible = pStatus->visible;
    BSYNClib_Channel_SetVideoSinkConfig_isr(syncChannel->synclibChannel, windex, &vsinkConfig);

    /* update status */
    syncChannel->status.video.sinks[windex].forcedCapture = vsinkConfig.bForcedCaptureEnabled;
    syncChannel->status.video.sinks[windex].masterFrameRate = vsinkConfig.bMasterFrameRateEnabled;
    syncChannel->status.video.sinks[windex].fullScreen = vsinkConfig.bFullScreen;
    syncChannel->status.video.sinks[windex].visible = vsinkConfig.bVisible;
}

static void
NEXUS_SyncChannel_P_SetPrecisionLipsync(NEXUS_SyncChannelHandle syncChannel)
{
    BSYNClib_Channel_Config synclibConfig;
    BSYNClib_Channel_GetConfig(syncChannel->synclibChannel, &synclibConfig);
    synclibConfig.bPrecisionLipsyncEnabled =
        syncChannel->settings.enablePrecisionLipsync
        &&
        !syncChannel->avTrickModeEnabled
        &&
        !syncChannel->syncLockFaked
        ;
    BSYNClib_Channel_SetConfig(syncChannel->synclibChannel, &synclibConfig);
}

static void
NEXUS_SyncChannel_P_SyncLockEventHandler(void * pContext)
{
    NEXUS_SyncChannelHandle syncChannel = pContext;

    BDBG_OBJECT_ASSERT(syncChannel, NEXUS_SyncChannel);

    NEXUS_SyncChannel_P_SetPrecisionLipsync(syncChannel);
}

static void
NEXUS_Synclib_P_VideoOutputComputeSyncLock_isr(NEXUS_SyncChannelHandle syncChannel, int windex, const NEXUS_VideoWindowSyncStatus * pStatus)
{
    NEXUS_VideoWindowSyncStatus status;
    int syncLockedWindow;
    int oldSyncLockedWindow;

    status = *pStatus;

    syncLockedWindow = NEXUS_SyncChannel_P_FindSyncLockedWindow_isr(syncChannel);
    oldSyncLockedWindow = syncChannel->status.video.syncLockedWindow;

    if (syncLockedWindow == -1)
    {
        int primaryDisplay;
        BDBG_MSG(("[%p] No sync-locked window found; trying to guess primary display", (void *)syncChannel));
        primaryDisplay = NEXUS_SyncChannel_P_GuessPrimaryDisplay_isr(syncChannel);
        if (primaryDisplay != -1)
        {
            BDBG_MSG(("[%p] Using primary display %d as sync-locked", (void *)syncChannel, primaryDisplay));
            syncLockedWindow = primaryDisplay;
        }
        if (!syncChannel->syncLockFaked)
        {
            syncChannel->syncLockFaked = true;
            BKNI_SetEvent_isr(syncChannel->syncLockEvent);
        }
    }
    else
    {
        if (syncChannel->syncLockFaked)
        {
            syncChannel->syncLockFaked = false;
            BKNI_SetEvent_isr(syncChannel->syncLockEvent);
        }
    }

    if (syncLockedWindow != oldSyncLockedWindow)
    {
        if (oldSyncLockedWindow != -1)
        {
            /* revert old sync-locked window to non-sync-locked status */
            NEXUS_Synclib_P_VideoOutputSyncLockChangeHandler_isr(syncChannel, oldSyncLockedWindow, false, false);
        }

        if (syncLockedWindow != -1)
        {
            if (syncLockedWindow != (int)windex)
            {
                NEXUS_VideoWindow_GetSyncStatus_isr(syncChannel->outputs.window[syncLockedWindow], &status);
            }
            /* set new sync-locked window to sync-locked status */
            syncChannel->syncLockedWindow = syncChannel->outputs.window[syncLockedWindow];
            BDBG_MSG(("sync-locked window: %u", syncLockedWindow));
            syncChannel->status.video.syncLockedWindow = syncLockedWindow;
            NEXUS_Synclib_P_VideoOutputSyncLockChangeHandler_isr(syncChannel, syncLockedWindow, true, status.delayValid);
        }
    }
}

static void
NEXUS_Synclib_P_VideoOutputStateChangeCallback_isr(void *context, int param)
{
    struct NEXUS_SyncChannelContext *syncContext = (struct NEXUS_SyncChannelContext *)context;
    NEXUS_VideoWindowSyncStatus status;

    BSTD_UNUSED(param);
    BDBG_OBJECT_ASSERT(syncContext->syncChannel, NEXUS_SyncChannel);

    /* this output is in the middle of a connection change */
    if (syncContext->connectionChanged)
    {
        /* we'll handle this later when we finish changing the connection */
        BDBG_MSG(("Connection change imminent: output %u skipped state change callback", syncContext->index));
        return;
    }

    /* check if displays are aligned */
    syncContext->syncChannel->displaysAligned = NEXUS_SyncChannel_P_AreDisplaysAligned_isr(syncContext->syncChannel);

    if (syncContext->syncChannel->outputs.window[syncContext->index])
    {
        NEXUS_VideoWindow_GetSyncStatus_isr(syncContext->syncChannel->outputs.window[syncContext->index], &status);
        NEXUS_Synclib_P_VideoOutputStateChangeHandler_isr(syncContext->syncChannel, syncContext->index, &status);
        NEXUS_Synclib_P_VideoOutputComputeSyncLock_isr(syncContext->syncChannel, syncContext->index, &status);
    }
    else
    {
        BDBG_WRN(("[%p] VideoOutputStateChange event received for NULL output %d", (void *)syncContext->syncChannel, syncContext->index));
    }
}

static void
NEXUS_Synclib_P_VideoOutputFormatCallback_isr(void *context, int param)
{
    BSYNClib_VideoSink_Config vsinkConfig;
    struct NEXUS_SyncChannelContext *syncContext = (struct NEXUS_SyncChannelContext *)context;
    NEXUS_VideoWindowSyncStatus status;

    BSTD_UNUSED(param);
    BDBG_OBJECT_ASSERT(syncContext->syncChannel, NEXUS_SyncChannel);

    /* this output is in the middle of a connection change */
    if (syncContext->connectionChanged)
    {
        /* we'll handle this later when we finish changing the connection */
        BDBG_MSG(("Connection change imminent: output %u skipped format callback", syncContext->index));
        return;
    }

    if (syncContext->syncChannel->outputs.window[syncContext->index])
    {
        NEXUS_VideoWindow_GetSyncStatus_isr(syncContext->syncChannel->outputs.window[syncContext->index], &status);
        BDBG_MSG(("[%p] VideoOutputFormat(%u) height=%d, interlaced=%d, framerate=%d, refreshRate=%u", (void *)syncContext->syncChannel, syncContext->index, status.height, status.interlaced, status.frameRate, status.refreshRate));
        BSYNClib_Channel_GetVideoSinkConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsinkConfig);
        vsinkConfig.sFormat.bReceived = true;
        vsinkConfig.sFormat.uiHeight = status.height;
        vsinkConfig.sFormat.bInterlaced = status.interlaced;
        vsinkConfig.sFormat.eFrameRate = status.frameRate;
        BSYNClib_Channel_SetVideoSinkConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsinkConfig);

        NEXUS_Synclib_P_VideoOutputComputeSyncLock_isr(syncContext->syncChannel, syncContext->index, &status);

        /* update status */
        syncContext->syncChannel->status.video.sinks[syncContext->index].format.height = vsinkConfig.sFormat.uiHeight;
        syncContext->syncChannel->status.video.sinks[syncContext->index].format.interlaced = vsinkConfig.sFormat.bInterlaced;
        syncContext->syncChannel->status.video.sinks[syncContext->index].format.refreshRate = status.refreshRate;
        if (status.refreshRate)
        {
            syncContext->syncChannel->status.video.sinks[syncContext->index].delay.applied =
                1000 * 1000 * syncContext->settings.window.delay / ((int)status.refreshRate);
            syncContext->syncChannel->status.video.sinks[syncContext->index].delay.measured =
                1000 * 1000 * status.delay / ((int)status.refreshRate);
        }
    }
    else
    {
        BDBG_WRN(("[%p] VideoOutputFormat event received for NULL output %d", (void *)syncContext->syncChannel, syncContext->index));
    }
}

static void NEXUS_SyncChannel_P_PhaseOffsetAdjustmentEventHandler(void * context)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SyncChannelHandle syncChannel = context;
    BSYNClib_Source_Status status;
    NEXUS_VideoInputSyncSettings syncSettings;

    BDBG_OBJECT_ASSERT(syncChannel, NEXUS_SyncChannel);

    BSYNClib_Channel_GetVideoSourceStatus(syncChannel->synclibChannel, 0, &status);

    if (syncChannel->settings.videoInput)
    {
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.display); /* lock display because VideoInput is in display module */
        NEXUS_VideoInput_GetSyncSettings_priv(syncChannel->settings.videoInput, &syncSettings);
        BDBG_ASSERT(status.sAppliedDelay.eUnits == BSYNClib_Units_e45KhzTicks);
        if (syncChannel->displaysAligned)
        {
            syncSettings.delay = (unsigned)((int)status.sAppliedDelay.uiValue + syncChannel->vsourcePhaseOffset);
        }
        else
        {
            syncSettings.delay = status.sAppliedDelay.uiValue;
        }
        rc = NEXUS_VideoInput_SetSyncSettings_priv(syncChannel->settings.videoInput, &syncSettings);
        if (rc) {BERR_TRACE(rc); goto unlock; }

unlock:
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.display);
        if (rc) {BERR_TRACE(rc);}

        /* update status */
        BSYNClib_ConvertSigned_isrsafe(syncSettings.delay, BSYNClib_Units_e45KhzTicks,
            BSYNClib_Units_eMilliseconds, &syncChannel->status.video.source.delay.applied);
    }
    else
    {
        BDBG_MSG(("[%p] Phase offset adjustment event handler called with NULL video input", (void *)syncChannel));
    }
}

static const int framePeriods[] =
{
    0,/* BAVC_FrameRateCode_eUnknown */
    1877,/* BAVC_FrameRateCode_e23_976 */
    1875,/* BAVC_FrameRateCode_e24 */
    1800,/* BAVC_FrameRateCode_e25 */
    1502,/* BAVC_FrameRateCode_e29_97 */
    1500,/* BAVC_FrameRateCode_e30 */
    900,/* BAVC_FrameRateCode_e50 */
    751,/* BAVC_FrameRateCode_e59_94 */
    750,/* BAVC_FrameRateCode_e60 */
    3003,/* BAVC_FrameRateCode_e14_985 */
    6006,/* BAVC_FrameRateCode_e7_493 */
    4500,/* BAVC_FrameRateCode_e10 */
    3000,/* BAVC_FrameRateCode_e15 */
    2250,/* BAVC_FrameRateCode_e20 */
    450,/* BAVC_FrameRateCode_e100 */
    375,/* BAVC_FrameRateCode_e119_88 */
    375,/* BAVC_FrameRateCode_e120 */
    0 /* BAVC_FrameRateCode_eMax */
};

#define NEXUS_SYNCLIB_P_VIRTUAL_WINDOW_REGION_NAME(R) \
    (((R) < 0) ? "TOO EARLY" : ((R) > 0) ? "TOO LATE" : "PASS")

static int NEXUS_Synclib_P_VirtualWindowCheck_isr(NEXUS_SyncChannelHandle syncChannel, int pos, int lower, int upper)
{
    int result = 0;

    if (pos < lower)
    {
        result = pos - lower;
    }
    else if (pos > upper)
    {
        result = pos - upper;
    }

    BDBG_ERR(("[%p] %d in [%d, %d]? %s",
        (void *)syncChannel,
        pos,
        lower,
        upper,
        NEXUS_SYNCLIB_P_VIRTUAL_WINDOW_REGION_NAME(result)));

    return result;
}

static int
NEXUS_Synclib_P_ComputePhaseOffsetAdjustmentImpl_isr
(
    NEXUS_SyncChannelHandle syncChannel,
    int slPhase,
    int lastOffset,
    int ssPhase,
    int sourceVsyncPeriod,
    int passWindow,
    int hysteresis
)
{
    int offset = lastOffset;
    int slVirtualWindowPosition = slPhase + offset;
    int ssVirtualWindowPosition = slPhase + offset + ssPhase;
    int earlyVirtualWindowBoundary = -hysteresis;
    int lateVirtualWindowBoundary = passWindow + hysteresis;
    int slRegion;
    int ssRegion;

    /* find where sync lock and sync slipped windows fall with respect to a
    virtual TSM pass window starting at zero frames early and proceeding to
    1 frame late with respect to the natural phase (sans phase offset) of
    the sync-locked and sync-slipped windows to the decoder */
    slRegion = NEXUS_Synclib_P_VirtualWindowCheck_isr(syncChannel,
        slVirtualWindowPosition, earlyVirtualWindowBoundary, lateVirtualWindowBoundary);
    ssRegion = NEXUS_Synclib_P_VirtualWindowCheck_isr(syncChannel,
        ssVirtualWindowPosition, earlyVirtualWindowBoundary, lateVirtualWindowBoundary);

    if (
        offset <= 0  /* keeps us from exceeding +1 VSYNC adjustment */
        &&
        (
            slRegion < 0 /* sync-locked display is too early */
            ||
            ssRegion < 0 /* sync-slipped display is too early */
        )
    )
    {
        /* this will force TSM to repeat a field, i.e. the sync-locked window (TSM) slips from
        the FIRST_VYSNC_SLOT to the SECOND_VSYNC_SLOT, while the sync-slipped
        window not only follows along from the left boundary of the TSM PASS WINDOW,
        but also its multi-buffer also repeats 1-field because the source slips 1-field so that the
        sync-slipped window delay becomes to the left of the rightmost boundary of the
        SECOND_VSYNC_SLOT; now sync-locked window leads the sync-slipped window; */
        offset += sourceVsyncPeriod;
    }
    else if (
        offset >= 0 /* keeps us from exceeding -1 VSYNC adjustment */
        &&
        (
            slRegion > 0 /* sync-locked display is too late */
            ||
            ssRegion > 0 /* sync-slipped display is too late */
        )
    )
    {
        /* this will force TSM to skip a field, i.e. the sync-locked window (TSM) slips from
        the SECOND_VYSNC_SLOT to the FIRST_VSYNC_SLOT, while the sync-slipped
        window not only follows along from the right boundary of the TSM PASS WINDOW,
        but also its multi-buffer will also skip 1 more field because the source slips 1-field
        so that the sync-slipped window delay becomes to the right of the leftmost
        boundary of the FIRST_VSYNC_SLOT;
        from now upon, sync-locked window lags behind the sync-slipped window; */
        offset -= sourceVsyncPeriod;
    }

    return offset;
}

static void
NEXUS_Synclib_P_ComputePhaseOffsetAdjustment_isr(NEXUS_SyncChannelHandle syncChannel)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i = 0;
    NEXUS_VideoWindowSyncStatus syncSlippedStatus;
    NEXUS_VideoWindowSyncStatus syncLockedStatus;
    NEXUS_VideoInputSyncStatus vinStatus;

    BKNI_Memset(&syncLockedStatus, 0, sizeof(NEXUS_VideoWindowSyncStatus));

    if (syncChannel->syncLockedWindow)
    {
        rc = NEXUS_VideoWindow_GetSyncStatus_isr(syncChannel->syncLockedWindow, &syncLockedStatus);
        if (rc) { BERR_TRACE(rc); goto end; }
    }
    else
    {
        goto end;
    }

    if (syncChannel->settings.videoInput)
    {
        rc = NEXUS_VideoInput_GetSyncStatus_isr(syncChannel->settings.videoInput, &vinStatus);
        if (rc) { BERR_TRACE(rc); goto end; }
    }
    else
    {
        goto end;
    }

    BDBG_MSG(("[%p] source %s (%s); sync-locked display: %s", (void *)syncChannel,
        vinStatus.interlaced ? "interlaced" : "progressive",
        vinStatus.delayValid ? "valid" : "invalid",
        syncChannel->syncLockedWindow ? (syncLockedStatus.interlaced ? "interlaced" : " progressive") : "none"));

    for (i = 0; i < NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS; i++)
    {
        if (syncChannel->outputs.window[i] && syncChannel->outputs.window[i] != syncChannel->syncLockedWindow)
        {
            rc = NEXUS_VideoWindow_GetSyncStatus_isr(syncChannel->outputs.window[i], &syncSlippedStatus);
            if (rc) { BERR_TRACE(rc); goto end; }

            /* this window is not sync-locked and display alignment is on */
            if (!syncSlippedStatus.syncLocked && syncChannel->displaysAligned)
            {
                /* if either source or sync-locked-display is progressive ... */
                if ((!vinStatus.interlaced || !syncLockedStatus.interlaced) && vinStatus.delayValid)
                {
                    int ssPhase = (syncSlippedStatus.phaseDelay * 45) / 1000; /* phase delay already stores just the sub-VSYNC delay, within +/- 1 VSYNC */
                    int slPhase = vinStatus.delay;
                    int sourceFramePeriod = framePeriods[vinStatus.frameRate];
                    int slVsyncPeriod = 45 * 1000 * 1000 / syncLockedStatus.refreshRate;
                    int ssVsyncPeriod = 45 * 1000 * 1000 / syncSlippedStatus.refreshRate;
                    int passWindow;
                    int sourceVsyncPeriod = sourceFramePeriod;
                    int hysteresis = 45; /* TODO: pull from adjustment threshold once it is correctly defaulted to JTI threshold */
                    int offset = syncChannel->vsourcePhaseOffset;

                    if (vinStatus.interlaced)
                    {
                        sourceVsyncPeriod /= 2;
                        passWindow = sourceFramePeriod;
                    }
                    else
                    {
                        if (syncLockedStatus.interlaced)
                        {
                            passWindow = slVsyncPeriod * 2;
                        }
                        else
                        {
                            passWindow = ssVsyncPeriod * 2;
                        }
                    }

                    BDBG_MSG(("[%p] slPhase: %u; offset: %d; ssPhase: %d; srcVsync: %d; pass-window: %d; hysteresis: %d",
                        (void *)syncChannel, slPhase, offset, ssPhase, sourceVsyncPeriod, passWindow, hysteresis));

                    syncChannel->vsourcePhaseOffset =
                        NEXUS_Synclib_P_ComputePhaseOffsetAdjustmentImpl_isr(
                            syncChannel, slPhase, offset, ssPhase, sourceVsyncPeriod, passWindow, hysteresis);
                }
                else
                {
                    syncChannel->vsourcePhaseOffset = 0;
                }

                BDBG_MSG(("[%p] new offset: %d", (void *)syncChannel, syncChannel->vsourcePhaseOffset));

                /* force a task-time offset re-apply */
                BKNI_SetEvent(syncChannel->phaseOffsetAdjustmentEvent);
            }
        }
    }

end:
    return;
}

static void
NEXUS_Synclib_P_VideoOutputDelayCallback_isr(void *context, int delay)
{
    BSYNClib_VideoSink_Config vsinkConfig;
    BSYNClib_Sink_Status vsinkStatus;
    struct NEXUS_SyncChannelContext *syncContext = (struct NEXUS_SyncChannelContext *)context;
    NEXUS_SyncChannelHandle syncChannel = syncContext->syncChannel;
    BSYNClib_Units eDelayUnits;

    BDBG_OBJECT_ASSERT(syncChannel, NEXUS_SyncChannel);
    BSTD_UNUSED(delay);

    /* this output is in the middle of a connection change */
    if (syncContext->connectionChanged)
    {
        /* we'll handle this later when we finish changing the connection */
        BDBG_MSG(("Connection change imminent: output %u skipped delay callback: %d", syncContext->index, delay));
        return;
    }

    if (syncChannel->outputs.window[syncContext->index])
    {
        NEXUS_VideoWindowSyncStatus status;
        NEXUS_VideoWindow_GetSyncStatus_isr(syncChannel->outputs.window[syncContext->index], &status);

        if (status.delayValid)
        {
            uint64_t cnvtDelay;

            BDBG_MSG(("[%p] VideoOutputDelay(%u,%p) %d", (void *)syncChannel, syncContext->index, (void *)syncChannel->outputs.window[syncContext->index], status.delay));
            BDBG_MSG(("[%p] VideoOutputPhaseDelay(%u,%p) %d", (void *)syncChannel, syncContext->index, (void *)syncChannel->outputs.window[syncContext->index], status.phaseDelay));

            NEXUS_Synclib_P_ComputePhaseOffsetAdjustment_isr(syncContext->syncChannel);

            eDelayUnits = BSYNClib_Units_e27MhzTicks;
            cnvtDelay = status.delay;
            cnvtDelay *= 27000000;
            if (status.refreshRate)
            {
                cnvtDelay *= 1000; /* refresh rate is in mHz */
                cnvtDelay /= status.refreshRate;
            }
            else
            {
                if (status.interlaced)
                {
                    cnvtDelay /= 2;
                }

                switch (status.frameRate)
                {
                    case BAVC_FrameRateCode_e10:          /* 10 */
                        cnvtDelay /= 10;
                        break;
                    case BAVC_FrameRateCode_e15:          /* 15 */
                        cnvtDelay /= 15;
                        break;
                    case BAVC_FrameRateCode_e19_98:       /* 19.98 */
                    case BAVC_FrameRateCode_e20:          /* 20 */
                        cnvtDelay /= 20;
                        break;
                    case BAVC_FrameRateCode_e23_976:  /* 23.976 */
                    case BAVC_FrameRateCode_e24:          /* 24 */
                        cnvtDelay /= 24;
                        break;

                    case BAVC_FrameRateCode_e25:          /* 25 */
                        cnvtDelay /= 25;
                        break;

                    case BAVC_FrameRateCode_e29_97:       /* 29.97 */
                    case BAVC_FrameRateCode_e30:          /* 30 */
                        cnvtDelay /= 30;
                        break;

                    case BAVC_FrameRateCode_e50:          /* 50 */
                        cnvtDelay /= 50;
                        break;

                    case BAVC_FrameRateCode_e59_94:       /* 59.94 */
                    case BAVC_FrameRateCode_e60:          /* 60 */
                    case BAVC_FrameRateCode_eUnknown: /* Unknown */
                    default:
                        cnvtDelay /= 60;
                        break;
                    case BAVC_FrameRateCode_e100:          /* 100 */
                        cnvtDelay /= 100;
                        break;

                    case BAVC_FrameRateCode_e119_88:       /* 119.88 */
                    case BAVC_FrameRateCode_e120:          /* 120 */
                        cnvtDelay /= 120;
                        break;
                }
            }

            BSYNClib_Channel_GetVideoSinkStatus_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsinkStatus);
            BSYNClib_Channel_GetVideoSinkConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsinkConfig);

            /* only call into synclib if value or units changes, as this will cause a readjustment and reschedule unmute timers */
            if ((vsinkConfig.sDelay.sMeasured.uiValue != (unsigned)cnvtDelay)
               || (vsinkConfig.sDelay.sMeasured.eUnits != eDelayUnits)
               || !vsinkStatus.bDelayReceived)
            {
                vsinkConfig.sDelay.bReceived = true;
                vsinkConfig.sDelay.sMeasured.uiValue = (unsigned)cnvtDelay;
                vsinkConfig.sDelay.sMeasured.eUnits = eDelayUnits;
                BSYNClib_Channel_SetVideoSinkConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &vsinkConfig);
            }

            /* update status */
            BSYNClib_ConvertSigned_isrsafe(syncChannel->vsourcePhaseOffset, BSYNClib_Units_e45KhzTicks,
                BSYNClib_Units_eMilliseconds, &syncChannel->status.video.source.delay.phase);
            syncChannel->status.video.sinks[syncContext->index].delay.phase = status.phaseDelay / 1000;
            if (status.refreshRate)
            {
                syncChannel->status.video.sinks[syncContext->index].delay.measured =
                    1000 * 1000 * status.delay / ((int)status.refreshRate);
            }
        }
    }
    else
    {
        BDBG_WRN(("[%p] VideoOutputDelay event received for NULL output %d", (void *)syncContext->syncChannel, syncContext->index));
    }
}

static void NEXUS_SyncChannel_P_VideoInputConnectionChangedEventHandler(void * context)
{
    NEXUS_SyncChannelHandle syncChannel = context;
    BSYNClib_Channel_Config synclibConfig;
    NEXUS_VideoWindowSyncStatus windowStatus;
    unsigned i = 0;

    BDBG_OBJECT_ASSERT(syncChannel, NEXUS_SyncChannel);

	if (syncChannel->settings.videoInput)
	{
	    BDBG_MSG(("[%p] Video input connection changed", (void *)syncChannel));

	    BDBG_MSG(("[%p] Disabling synclib", (void *)syncChannel));
	    BSYNClib_Channel_GetConfig(syncChannel->synclibChannel, &synclibConfig);
	    synclibConfig.bEnabled = false;
	    BSYNClib_Channel_SetConfig(syncChannel->synclibChannel, &synclibConfig);
	    BDBG_MSG(("[%p] Synclib disabled", (void *)syncChannel));

        /* this will also disconnect outputs from nexus side */
	    NEXUS_SyncChannel_P_DisconnectVideoInput(syncChannel);

	    /* this will rediscover and reconnect outputs on the nexus side */
        NEXUS_SyncChannel_P_ConnectVideoInput(syncChannel, &syncChannel->settings);

        /* repopulate config for all new outputs */
	    for (i = 0; i < NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS; i++)
	    {
	        /*
	         * This will disconnect nexus-disconnected windows and
	         * connect nexus-connected windows on the synclib side
	         * We combine these ops here because we don't want to
	         * resync while in between connections.
	         */
            NEXUS_Synclib_P_SetVideoSink(syncChannel, &syncChannel->settings, i);

            /* need to acquire cs here, as display can modify the window handles in cs elsewise */
            BKNI_EnterCriticalSection();
	        if (syncChannel->outputs.window[i])
	        {
	            /* mark sink whose new status has been gathered as finished with the change */
	            /* this must be done prior to retrieving updated state info, otherwise it won't update */
	            syncChannel->vsinkContext[i].connectionChanged = false;

	            /* format and current state are always valid */
	            NEXUS_Synclib_P_VideoOutputStateChangeCallback_isr(&syncChannel->vsinkContext[i], 0);
	            NEXUS_Synclib_P_VideoOutputFormatCallback_isr(&syncChannel->vsinkContext[i], 0);

	            /* delay, we have to check */
	            NEXUS_VideoWindow_GetSyncStatus_isr(syncChannel->outputs.window[i], &windowStatus);
	            if (windowStatus.delayValid)
	            {
	                NEXUS_Synclib_P_VideoOutputDelayCallback_isr(&syncChannel->vsinkContext[i], 0);
	            }
	        }
            BKNI_LeaveCriticalSection();
	    }

	    BDBG_MSG(("[%p] Enabling synclib", (void *)syncChannel));
	    BSYNClib_Channel_GetConfig(syncChannel->synclibChannel, &synclibConfig);
	    synclibConfig.bEnabled = true;
	    BSYNClib_Channel_SetConfig(syncChannel->synclibChannel, &synclibConfig);
	    BDBG_MSG(("[%p] Synclib enabled", (void *)syncChannel));
	}
	else
	{
	    BDBG_MSG(("[%p] Stale video input connection changed event received", (void *)syncChannel));
	}
}

/**********************
*
* SYNClib AudioSource = NEXUS AudioInput, which can be AudioDecoder, analog, etc.
*
***********************/

static BERR_Code
NEXUS_Synclib_P_SetAudioSourceMute(void * pvParm1, int iParm2, unsigned index, bool bMute)
{
    NEXUS_SyncChannelHandle syncChannel = (NEXUS_SyncChannelHandle)pvParm1;
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_AudioInputSyncSettings syncSettings;

    BSTD_UNUSED(iParm2);

    if (syncChannel->settings.audioInput[index])
    {
        BDBG_MSG(("[%p] SetAudioSourceMute(%u): %s", (void *)syncChannel, index, bMute ? "muted" : "unmuted"));
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.audio);
        NEXUS_AudioInput_GetSyncSettings_priv(syncChannel->settings.audioInput[index], &syncSettings);
        syncSettings.mute = bMute;
        rc = NEXUS_AudioInput_SetSyncSettings_priv(syncChannel->settings.audioInput[index], &syncSettings);
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.audio);

        /* update status */
        syncChannel->status.audio.sources[index].muted = syncSettings.mute;
    }
    else
    {
        BDBG_MSG(("[%p] Synclib called set audio source mute with NULL input %d", (void *)syncChannel, index));
    }

    return rc;
}

static BERR_Code
NEXUS_Synclib_P_SetAudioSourceDelay(void * pvParm1, int iParm2, unsigned int index, BSYNClib_UnsignedValue * pDelay)
{
    NEXUS_SyncChannelHandle syncChannel = (NEXUS_SyncChannelHandle)pvParm1;
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_AudioInputSyncSettings syncSettings;
    unsigned delay;

    BSTD_UNUSED(iParm2);

    if (syncChannel->settings.audioInput[index])
    {
        BKNI_EnterCriticalSection();
        delay = pDelay->uiValue;
        /* we only do this for progressive sources because for interlaced
         * sources the variation is too wide to allow a shift in audio away
         * from the HD display center of variation. progressive source rates
         * less than < 27777 result in the HD path not fitting within the
         * +/- 20 ms window, if audio is assumed to add +/- 2 ms.  So we don't
         * do any audio display centering for those rates.
         */
        if (!syncChannel->status.video.source.format.interlaced
            && syncChannel->status.video.source.format.frameRate >= 27777)
        {
            delay += NEXUS_Synclib_P_ComputeInterDisplayAudioOffset_isr(syncChannel);
        }
        BKNI_LeaveCriticalSection();
        BDBG_MSG(("[%p] SetAudioSourceDelay(%u): %u %s", (void *)syncChannel, index, pDelay->uiValue, BSYNClib_P_UnitsStrings[pDelay->eUnits]));
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.audio);
        NEXUS_AudioInput_GetSyncSettings_priv(syncChannel->settings.audioInput[index], &syncSettings);
        BDBG_ASSERT(pDelay->eUnits == BSYNClib_Units_e45KhzTicks);
        syncSettings.delay = delay;
        rc = NEXUS_AudioInput_SetSyncSettings_priv(syncChannel->settings.audioInput[index], &syncSettings);
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.audio);

        /* update status */
        BSYNClib_ConvertSigned_isrsafe(syncSettings.delay, pDelay->eUnits,
            BSYNClib_Units_eMilliseconds, &syncChannel->status.audio.sources[index].delay.applied);
    }
    else
    {
        BDBG_MSG(("[%p] Synclib called set audio source delay with NULL input %u", (void *)syncChannel, index));
    }

    return rc;
}

#define NEXUS_SYNC_CHANNEL_MAX_SUPPORTED_AUDIO_DELAY 144

/* TODO: this was lifted right out of SAD
 * should put in a shared place so it is reused properly */
static bool nexus_p_is_decoder_connected(NEXUS_AudioDecoderHandle audioDecoder)
{
    NEXUS_AudioDecoderConnectorType i;
    for (i=0;i<NEXUS_AudioDecoderConnectorType_eMax;i++) {
        bool connected;
        NEXUS_AudioInputHandle audioInput = NEXUS_AudioDecoder_GetConnector(audioDecoder, i);
        if (audioInput) {
            NEXUS_AudioInput_HasConnectedOutputs(audioInput, &connected);
            if (connected) {
                return true;
            }
        }
    }
    return false;
}


static void
NEXUS_SyncChannel_P_AudioInputStartEventHandler(void * context)
{
    NEXUS_SyncChannelHandle syncChannel = context;
    BSYNClib_AudioSource_Config asourceConfig;
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_SyncChannel, syncChannel);

    /* SW7435-641 check all other audio channels for connections to outputs.
     * if they are not connected, they will not be started, so we need to
     * internally disconnect them from sync now.  Alternatively, if we've
     * disconnected one before, we need to reconnect it now if it has been
     * connected to valid outputs.
     */
    for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_INPUTS; i++)
    {
        bool connected = false;
        if (syncChannel->settings.audioInput[i])
        {
            NEXUS_AudioDecoderHandle decoder = NEXUS_AudioInput_GetDecoderHandle_priv(syncChannel->settings.audioInput[i]);
            if (decoder)
            {
                BDBG_MSG(("[%p] Checking if decoder %p has outputs", (void *)syncChannel, (void *)decoder));
                connected = nexus_p_is_decoder_connected(decoder);
            }
            else
            {
                BDBG_MSG(("[%p] Checking if input %p has outputs", (void *)syncChannel, (void*)syncChannel->settings.audioInput[i]));
                NEXUS_AudioInput_HasConnectedOutputs(syncChannel->settings.audioInput[i], &connected);
            }

            if (!connected)
            {
                BDBG_MSG(("[%p] Audio channel %d has no connected outputs, ignoring", (void *)syncChannel, i));
            }
            else
            {
                BDBG_MSG(("[%p] Audio channel %d has connected outputs", (void *)syncChannel, i));
            }

            BKNI_EnterCriticalSection();
            BSYNClib_Channel_GetAudioSourceConfig_isr(syncChannel->synclibChannel, i, &asourceConfig);
            asourceConfig.bSynchronize = connected;
            BSYNClib_Channel_SetAudioSourceConfig_isr(syncChannel->synclibChannel, i, &asourceConfig);
            BKNI_LeaveCriticalSection();
        }
    }

    NEXUS_SyncChannel_P_SetPrecisionLipsync(syncChannel);
}

static void
NEXUS_Synclib_P_AudioInputStartCallback_isr(void *context, int param)
{
    BSYNClib_AudioSource_Config asourceConfig;
    struct NEXUS_SyncChannelContext *syncContext = (struct NEXUS_SyncChannelContext *)context;
    NEXUS_AudioInputSyncStatus status;

    BSTD_UNUSED(param);

    if (syncContext->syncChannel->settings.audioInput[syncContext->index])
    {
        NEXUS_AudioInput_GetSyncStatus_isr(syncContext->syncChannel->settings.audioInput[syncContext->index], &status);
        BDBG_MSG(("[%p] AudioInputStart(%u) started=%d, digital=%d delay=%u", (void *)syncContext->syncChannel, syncContext->index, status.started, status.digital, status.delay));
        BSYNClib_Channel_GetAudioSourceConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &asourceConfig);
        asourceConfig.bStarted = status.started;
        asourceConfig.bDigital = status.digital;
        if (status.started)
        {
            asourceConfig.sDelay.bReceived = true;
            asourceConfig.sDelay.sMeasured.eUnits = BSYNClib_Units_eMilliseconds;
            if (status.delay > NEXUS_SYNC_CHANNEL_MAX_SUPPORTED_AUDIO_DELAY)
            {
                BDBG_ERR(("[%p] Audio software is returning an invalid decoder delay value: %u", (void *)syncContext->syncChannel, status.delay ));
                status.delay = NEXUS_SYNC_CHANNEL_MAX_SUPPORTED_AUDIO_DELAY;
            }
            asourceConfig.sDelay.sMeasured.uiValue = status.delay;
            /* Sarnoff offset defaults off for NRT */
            if (status.nonRealTime)
            {
                if (NEXUS_GetEnv("sarnoff_lipsync_offset_enabled"))
                {
                    BDBG_MSG(("[%p] NRT: Sarnoff 8 ms lipsync offset enabled", (void *)syncContext->syncChannel));
                    asourceConfig.sDelay.sMeasured.uiValue += 8;
                }
                else
                {
                    BDBG_MSG(("[%p] NRT: Sarnoff 8 ms lipsync offset disabled", (void *)syncContext->syncChannel));
                }
            }
            /* if not NRT, this flag, set at create time, is used instead */
            else if (syncContext->syncChannel->sarnoffLipsyncOffsetEnabled)
            {
                asourceConfig.sDelay.sMeasured.uiValue += 8;
            }
            asourceConfig.bSamplingRateReceived = false;
        }
        else
        {
            asourceConfig.sDelay.bReceived = false;
        }
        BSYNClib_Channel_SetAudioSourceConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &asourceConfig);

        /* PR49294 need to ignore callback in av trick modes */
        syncContext->syncChannel->avTrickModeEnabled = status.dsolaEnabled;

        /* update status */
        syncContext->syncChannel->status.nonRealTime = syncContext->nonRealTime;
        syncContext->syncChannel->status.audio.sources[syncContext->index].samplingRateReceived = asourceConfig.bSamplingRateReceived;
        syncContext->syncChannel->status.audio.sources[syncContext->index].started = asourceConfig.bStarted;
        syncContext->syncChannel->status.audio.sources[syncContext->index].digital = asourceConfig.bDigital;
        BSYNClib_ConvertSigned_isrsafe(asourceConfig.sDelay.sMeasured.uiValue, asourceConfig.sDelay.sMeasured.eUnits,
            BSYNClib_Units_eMilliseconds, &syncContext->syncChannel->status.audio.sources[syncContext->index].delay.measured);
        syncContext->syncChannel->status.audio.sources[syncContext->index].delay.applied = -1;

        /* only call this on start, not stop */
        if (asourceConfig.bStarted)
        {
            BKNI_SetEvent(syncContext->syncChannel->audioInputStartEvent);
        }
    }
    else
    {
        BDBG_WRN(("[%p] AudioInputStartStop event received for NULL input %d", (void *)syncContext->syncChannel, syncContext->index));
    }
}

static void
NEXUS_Synclib_P_AudioInputSampleRateCallback_isr(void *context, int samplingRate)
{
    BSYNClib_AudioSource_Config asourceConfig;
    struct NEXUS_SyncChannelContext *syncContext = (struct NEXUS_SyncChannelContext *)context;

    BSTD_UNUSED(samplingRate); /* SYNClib doesn't need to know the value */
    /* TODO: account for 64 sample DRAM FIFO latency in audio FMM (currently accounted in nexus_audio_decoder delay) */

    if (syncContext->syncChannel->settings.audioInput[syncContext->index])
    {
        BDBG_MSG(("[%p] AudioInputSampleRate(%u)", (void *)syncContext->syncChannel, syncContext->index));
        BSYNClib_Channel_GetAudioSourceConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &asourceConfig);
        asourceConfig.bSamplingRateReceived = true;
        BSYNClib_Channel_SetAudioSourceConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &asourceConfig);

        /* update status */
        syncContext->syncChannel->status.audio.sources[syncContext->index].samplingRateReceived = asourceConfig.bSamplingRateReceived;
    }
    else
    {
        BDBG_WRN(("[%p] AudioInputSampleRate event received for NULL input %d", (void *)syncContext->syncChannel, syncContext->index));
    }
}

/**********************
*
* AudioSink = AudioOutput
*
***********************/

static BERR_Code
NEXUS_Synclib_P_SetAudioSinkDelay(void * pvParm1, int iParm2, unsigned int index, BSYNClib_UnsignedValue * pDelay)
{
    NEXUS_SyncChannelHandle syncChannel = (NEXUS_SyncChannelHandle)pvParm1;
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_AudioOutputSyncSettings syncSettings;

    BDBG_MSG(("[%p] SetAudioSinkDelay(%u): %u %s", (void *)syncChannel, index, pDelay->uiValue, BSYNClib_P_UnitsStrings[pDelay->eUnits]));
    BSTD_UNUSED(iParm2);

    /* TODO: handle analog */

#if 0
    /* TODO: implement independent output delay */
    NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.audio);
    NEXUS_AudioOutput_GetSyncSettings_priv(syncChannel->outputs.audioOutput[index], &syncSettings);
    syncSettings.delay = pDelay->uiValue;
    rc = NEXUS_AudioOutput_SetSyncSettings_priv(syncChannel->outputs.audioOutput[index], &syncSettings);
    NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.audio);
#else
    BSTD_UNUSED(syncChannel);
    BSTD_UNUSED(syncSettings);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pDelay);
#endif
    return rc;
}

#if 0
static void
NEXUS_Synclib_P_AudioOutputStartCallback_isr(void *context, int param)
{
    BSYNClib_AudioSink_Config asinkConfig;
    struct NEXUS_SyncChannelContext *syncContext = (struct NEXUS_SyncChannelContext *)context;
    NEXUS_AudioOutputSyncStatus status;

    BSTD_UNUSED(param);

    if (syncContext->syncChannel->outputs.audioOutput[syncContext->index])
    {
        NEXUS_AudioOutput_GetSyncStatus_isr(syncContext->syncChannel->outputs.audioOutput[syncContext->index], &status);
        BDBG_MSG(("AudioOutputStart(%u) started=%d, compressed=%d", syncContext->index, status.started, status.compressed));
        BSYNClib_Channel_GetAudioSinkConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &asinkConfig);
        /* TODO: remove status.started */
        asinkConfig.bCompressed = status.compressed;
        BSYNClib_Channel_SetAudioSinkConfig_isr(syncContext->syncChannel->synclibChannel, syncContext->index, &asinkConfig);
    }
    else
    {
        BDBG_MSG(("AudioOutputStartStop event received for NULL output %d", syncContext->index));
    }
}
#endif

/**********************
*
* Main functions
*
***********************/

static void NEXUS_SyncChannel_P_GetDefaultStatus(NEXUS_SyncChannelStatus * pStatus)
{
    unsigned i;

    BDBG_ASSERT(pStatus);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_SyncChannelStatus));
    pStatus->displaySync = NEXUS_SyncChannelState_eAcquiring;
    pStatus->audioOutputSync = NEXUS_SyncChannelState_eAcquiring;
    pStatus->avSync = NEXUS_SyncChannelState_eAcquiring;
    pStatus->video.source.delay.applied = -1;
    pStatus->video.source.delay.measured = -1;
    pStatus->video.source.delay.custom = -1;
    pStatus->video.source.delay.notificationThreshold = -1;
    for (i = 0; i < NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS; i++)
    {
        pStatus->video.sinks[i].delay.applied = -1;
        pStatus->video.sinks[i].delay.measured = -1;
        pStatus->video.sinks[i].delay.custom = -1;
        pStatus->video.sinks[i].delay.notificationThreshold = -1;
    }
    pStatus->video.syncLockedWindow = -1;
    for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_INPUTS; i++)
    {
        pStatus->audio.sources[i].delay.applied = -1;
        pStatus->audio.sources[i].delay.measured = -1;
        pStatus->audio.sources[i].delay.custom = -1;
    }
    for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_OUTPUTS; i++)
    {
        pStatus->audio.sinks[i].delay.applied = -1;
        pStatus->audio.sinks[i].delay.measured = -1;
        pStatus->audio.sinks[i].delay.custom = -1;
    }
}

void NEXUS_SyncChannel_GetDefaultSettings(NEXUS_SyncChannelSettings *pSettings)
{
    BSYNClib_Channel_Config chCfg;
    BSYNClib_VideoSource_Config srcCfg;
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BSYNClib_Channel_GetDefaultConfig(&chCfg);
    BSYNClib_Channel_GetDefaultVideoSourceConfig(&srcCfg);
    BSYNClib_Convert_isrsafe(srcCfg.sJitterToleranceImprovementThreshold.uiValue,
        srcCfg.sJitterToleranceImprovementThreshold.eUnits,
        BSYNClib_Units_eMilliseconds,
        &pSettings->adjustmentThreshold);
    pSettings->enablePrecisionLipsync = chCfg.bPrecisionLipsyncEnabled;
    pSettings->enableMuteControl = chCfg.sMuteControl.bEnabled;
    pSettings->allowIncrementalStart = chCfg.sMuteControl.bAllowIncrementalStart;
    pSettings->simultaneousUnmute = chCfg.sMuteControl.bSimultaneousUnmute;
}

NEXUS_SyncChannelHandle NEXUS_SyncChannel_Create(const NEXUS_SyncChannelSettings *pSettings)
{
    BSYNClib_Channel_Settings channelSettings;
    BSYNClib_Channel_Config synclibConfig;
    NEXUS_SyncChannelHandle syncChannel;
    NEXUS_SyncChannelSettings settings;
    BERR_Code rc;

    syncChannel = BKNI_Malloc(sizeof(*syncChannel));
    if (!syncChannel) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_SyncChannel, syncChannel);

    /* off by default due to customers no longer testing with Sarnoff */
    syncChannel->sarnoffLipsyncOffsetEnabled = false;
    /* env var with no API is preferred currently */
    if (NEXUS_GetEnv("sarnoff_lipsync_offset_enabled"))
    {
        BDBG_MSG(("[%p] Sarnoff 8 ms lipsync offset enabled", (void *)syncChannel));
        syncChannel->sarnoffLipsyncOffsetEnabled = true;
    }
    else
    {
        BDBG_MSG(("[%p] Sarnoff 8 ms lipsync offset disabled", (void *)syncChannel));
    }

    BKNI_CreateEvent(&syncChannel->syncLockEvent);
    syncChannel->syncLockEventCallback =
        NEXUS_RegisterEvent(syncChannel->syncLockEvent,
            &NEXUS_SyncChannel_P_SyncLockEventHandler,
            syncChannel);

    BKNI_CreateEvent(&syncChannel->phaseOffsetAdjustmentEvent);
    syncChannel->phaseOffsetAdjustmentEventCallback =
        NEXUS_RegisterEvent(syncChannel->phaseOffsetAdjustmentEvent,
            &NEXUS_SyncChannel_P_PhaseOffsetAdjustmentEventHandler,
            syncChannel);

    BKNI_CreateEvent(&syncChannel->videoInputConnectionChangedEvent);
    syncChannel->videoInputConnectionChangedCallback =
        NEXUS_RegisterEvent(syncChannel->videoInputConnectionChangedEvent,
            &NEXUS_SyncChannel_P_VideoInputConnectionChangedEventHandler,
            syncChannel);
    BKNI_CreateEvent(&syncChannel->audioInputStartEvent);
    syncChannel->audioInputStartEventCallback =
        NEXUS_RegisterEvent(syncChannel->audioInputStartEvent,
            &NEXUS_SyncChannel_P_AudioInputStartEventHandler,
            syncChannel);

    BSYNClib_GetChannelDefaultSettings(&channelSettings);
    NEXUS_SyncChannel_P_GetDefaultStatus(&syncChannel->status);
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannel->settings);

    /* map syslib/framework functions to base */
    channelSettings.cbTimer.pfCreate = NEXUS_SYSlib_P_CreateTimer;
    channelSettings.cbTimer.pfDestroy = NEXUS_SYSlib_P_DestroyTimer;
    channelSettings.cbTimer.pfStart_isr = NEXUS_SYSlib_P_StartTimer_isr;
    channelSettings.cbTimer.pfCancel_isr = NEXUS_SYSlib_P_CancelTimer_isr;
    channelSettings.cbTimer.pvParm1 = g_NEXUS_syncChannel.syslibContext;
    channelSettings.cbTimer.iParm2 = 0; /* unused */
    channelSettings.cbTime.pfGetTime_isr = NEXUS_SYSlib_P_GetTime_isr;
    channelSettings.cbTime.pvParm1 = g_NEXUS_syncChannel.syslibContext;
    channelSettings.cbTime.iParm2 = 0; /* unused */

    /* TODO: move these to SetSettings / Synclib_P_Connect fns */
    /* SYNClib a/v callbacks. They are all task time. You cannot make any change to SYNClib during these callbacks. */
    channelSettings.sVideo.sSource.cbMute.pfSetMute = NEXUS_Synclib_P_SetVideoSourceMute;
    channelSettings.sVideo.sSource.cbMute.pvParm1 = syncChannel;
    channelSettings.sVideo.sSource.cbMute.iParm2 = 0; /* unused */
    channelSettings.sVideo.sSource.cbDelay.pfSetDelay = NEXUS_Synclib_P_SetVideoSourceDelay;
    channelSettings.sVideo.sSource.cbDelay.pfSetDelayNotification = NEXUS_Synclib_P_SetVideoSourceDelayNotification;
    channelSettings.sVideo.sSource.cbDelay.pvParm1 = syncChannel;
    channelSettings.sVideo.sSource.cbDelay.iParm2 = 0; /* unused */
    channelSettings.sVideo.sSink.cbDelay.pfSetDelay = NEXUS_Synclib_P_SetVideoSinkDelay;
    channelSettings.sVideo.sSink.cbDelay.pvParm1 = syncChannel;
    channelSettings.sVideo.sSink.cbDelay.iParm2 = 0; /* unused */
    channelSettings.sVideo.sSink.cbDelay.preferredDelayUnits = BSYNClib_Units_e27MhzTicks;
    channelSettings.sAudio.sSource.cbMute.pfSetMute = NEXUS_Synclib_P_SetAudioSourceMute;
    channelSettings.sAudio.sSource.cbMute.pvParm1 = syncChannel;
    channelSettings.sAudio.sSource.cbMute.iParm2 = 0; /* unused */
    channelSettings.sAudio.sSource.cbDelay.pfSetDelay = NEXUS_Synclib_P_SetAudioSourceDelay;
    channelSettings.sAudio.sSource.cbDelay.preferredDelayUnits = BSYNClib_Units_e45KhzTicks;
    channelSettings.sAudio.sSource.cbDelay.pvParm1 = syncChannel;
    channelSettings.sAudio.sSource.cbDelay.iParm2 = 0; /* unused */
    channelSettings.sAudio.sSink.cbDelay.pfSetDelay = NEXUS_Synclib_P_SetAudioSinkDelay;
    channelSettings.sAudio.sSink.cbDelay.pvParm1 = syncChannel;
    channelSettings.sAudio.sSink.cbDelay.iParm2 = 0; /* unused */

    rc = BSYNClib_CreateChannel(g_NEXUS_syncChannel.synclib, &channelSettings, &syncChannel->synclibChannel);
    if (rc) {BERR_TRACE(rc); goto error;}

    /* do this once at create time */
    BSYNClib_Channel_GetConfig(syncChannel->synclibChannel, &synclibConfig);
    synclibConfig.uiVideoSourceCount = NEXUS_SYNC_CHANNEL_VIDEO_INPUTS;
    synclibConfig.uiVideoSinkCount = NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS;
    synclibConfig.uiAudioSourceCount = NEXUS_SYNC_CHANNEL_AUDIO_INPUTS;
    synclibConfig.uiAudioSinkCount = NEXUS_SYNC_CHANNEL_AUDIO_OUTPUTS;
    BSYNClib_Channel_SetConfig(syncChannel->synclibChannel, &synclibConfig);

    if (pSettings == NULL)
    {
        NEXUS_SyncChannel_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    rc = NEXUS_SyncChannel_SetSettings(syncChannel, pSettings);
    if (rc) {BERR_TRACE(rc); goto error;}

    return syncChannel;
error:
    NEXUS_SyncChannel_Destroy(syncChannel);
    return NULL;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_SyncChannel, NEXUS_SyncChannel_Destroy);

static void NEXUS_SyncChannel_P_Finalizer(NEXUS_SyncChannelHandle syncChannel)
{
    NEXUS_SyncChannelSettings settings;
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_SyncChannel, syncChannel);

    /* disconnect if needed */
    NEXUS_SyncChannel_P_DisconnectVideoInput(syncChannel);
    for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_INPUTS; i++)
    {
        NEXUS_SyncChannel_P_DisconnectAudioInput(syncChannel, i);
    }

    NEXUS_UnregisterEvent(syncChannel->audioInputStartEventCallback);
    syncChannel->audioInputStartEventCallback = NULL;
    BKNI_DestroyEvent(syncChannel->audioInputStartEvent);
    syncChannel->audioInputStartEvent = NULL;

    NEXUS_UnregisterEvent(syncChannel->videoInputConnectionChangedCallback);
    syncChannel->videoInputConnectionChangedCallback = NULL;
    BKNI_DestroyEvent(syncChannel->videoInputConnectionChangedEvent);
    syncChannel->videoInputConnectionChangedEvent = NULL;

    NEXUS_UnregisterEvent(syncChannel->phaseOffsetAdjustmentEventCallback);
    syncChannel->phaseOffsetAdjustmentEventCallback = NULL;
    BKNI_DestroyEvent(syncChannel->phaseOffsetAdjustmentEvent);
    syncChannel->phaseOffsetAdjustmentEvent = NULL;

    NEXUS_UnregisterEvent(syncChannel->syncLockEventCallback);
    syncChannel->syncLockEventCallback = NULL;
    BKNI_DestroyEvent(syncChannel->syncLockEvent);
    syncChannel->syncLockEvent = NULL;

    NEXUS_SyncChannel_GetSettings(syncChannel, &settings);
    settings.videoInput = NULL;
    for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_INPUTS; i++)
    {
        settings.audioInput[i] = NULL;
    }
    NEXUS_SyncChannel_SetSettings(syncChannel, &settings);

    BSYNClib_DestroyChannel(g_NEXUS_syncChannel.synclib, syncChannel->synclibChannel);

    NEXUS_OBJECT_DESTROY(NEXUS_SyncChannel, syncChannel);
    BKNI_Free(syncChannel);
}

void NEXUS_SyncChannel_GetSettings(NEXUS_SyncChannelHandle syncChannel, NEXUS_SyncChannelSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(syncChannel, NEXUS_SyncChannel);
    *pSettings = syncChannel->settings;
}

static NEXUS_Error NEXUS_Synclib_P_SetVideoSource(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index)
{
    NEXUS_VideoInputSyncStatus status;
    BSYNClib_VideoSource_Config vsourceConfig;

    BKNI_Memset(&status, 0, sizeof(NEXUS_VideoInputSyncStatus));

    BKNI_EnterCriticalSection();
    if (pSettings->videoInput)
    {
        NEXUS_VideoInput_GetSyncStatus_isr(pSettings->videoInput, &status);
    }
    BSYNClib_Channel_GetVideoSourceConfig_isr(syncChannel->synclibChannel, index, &vsourceConfig);
    vsourceConfig.bSynchronize = (pSettings->videoInput != NULL) || (syncChannel->simpleVideoConnected && index == 0);
    vsourceConfig.sJitterToleranceImprovementThreshold.uiValue = pSettings->adjustmentThreshold;
    /* status will be zero'd by memset above if no input is available */
    vsourceConfig.sDelay.sCustom.uiValue = NEXUS_Synclib_P_ComputeVideoSourceCustomDelay_isr(syncChannel, &status);
    BSYNClib_Channel_SetVideoSourceConfig_isr(syncChannel->synclibChannel, index, &vsourceConfig);
    BKNI_LeaveCriticalSection();
    BDBG_MSG(("[%p] vsrc[%d] = %d%c", (void *)syncChannel, index, vsourceConfig.bSynchronize,
        (pSettings->videoInput == NULL) && (syncChannel->simpleVideoConnected && index == 0) ? 's' : ' '));

    /* update status */
    syncChannel->status.video.source.synchronized = vsourceConfig.bSynchronize;
    syncChannel->status.video.source.delay.custom = vsourceConfig.sDelay.sCustom.uiValue;

    return 0;
}

NEXUS_Error NEXUS_Synclib_P_SetVideoSink(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index)
{
    BSYNClib_VideoSink_Config vsinkConfig;

    BSTD_UNUSED(pSettings); /* for now */

    BKNI_EnterCriticalSection();
    BSYNClib_Channel_GetVideoSinkConfig_isr(syncChannel->synclibChannel, index, &vsinkConfig);
    vsinkConfig.bSynchronize = (syncChannel->outputs.window[index] != NULL);
    BSYNClib_Channel_SetVideoSinkConfig_isr(syncChannel->synclibChannel, index, &vsinkConfig);
    BKNI_LeaveCriticalSection();
    BDBG_MSG(("[%p] vsink[%d] = %d", (void *)syncChannel, index, vsinkConfig.bSynchronize));

    /* update status */
    syncChannel->status.video.sinks[index].synchronized = vsinkConfig.bSynchronize;

    return 0;
}

static NEXUS_Error NEXUS_SyncChannel_P_ApplyAudioConnectState(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_AudioInputSyncStatus status;

    if (pSettings->audioInput[index])
    {
        BKNI_EnterCriticalSection();
        NEXUS_Synclib_P_AudioInputStartCallback_isr(&syncChannel->asourceContext[index], 0);
        rc = NEXUS_AudioInput_GetSyncStatus_isr(pSettings->audioInput[index], &status);
        if (!rc)
        {
            if (status.started)
            {
                /* TODO: this is assumed to have happened, since we are already started
                 * it would be better to know for sure */
                NEXUS_Synclib_P_AudioInputSampleRateCallback_isr(&syncChannel->asourceContext[index], 0);
            }
        }
        BKNI_LeaveCriticalSection();
    }

    return rc;
}

static NEXUS_Error NEXUS_Synclib_P_SetAudioSource(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BSYNClib_AudioSource_Config asourceConfig;

    BKNI_EnterCriticalSection();
    BSYNClib_Channel_GetAudioSourceConfig_isr(syncChannel->synclibChannel, index, &asourceConfig);
    asourceConfig.bSynchronize = (pSettings->audioInput[index] != NULL) || (syncChannel->simpleAudioConnected && index == 0);
    asourceConfig.sDelay.sCustom.uiValue = pSettings->customAudioDelay[index];
    BSYNClib_Channel_SetAudioSourceConfig_isr(syncChannel->synclibChannel, index, &asourceConfig);
    BKNI_LeaveCriticalSection();
    BDBG_MSG(("[%p] asrc[%d] = %d%c", (void *)syncChannel, index, asourceConfig.bSynchronize,
        (pSettings->audioInput[index] == NULL) && (syncChannel->simpleAudioConnected && index == 0) ? 's' : ' '));

    /* update status */
    syncChannel->status.audio.sources[index].synchronized = asourceConfig.bSynchronize;
    syncChannel->status.audio.sources[index].delay.custom = asourceConfig.sDelay.sCustom.uiValue;

#if 0
    if (!asourceConfig.bSynchronize) continue;
#endif

    return rc;
}

static NEXUS_Error NEXUS_Synclib_P_SetAudioSink(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index)
{
    BSYNClib_AudioSink_Config asinkConfig;

    BSTD_UNUSED(pSettings);

    BKNI_EnterCriticalSection();
    BSYNClib_Channel_GetAudioSinkConfig_isr(syncChannel->synclibChannel, index, &asinkConfig);
#if 0
    asinkConfig.bSynchronize = (syncChannel->outputs.audioOutput[i] != NULL);
#else
    asinkConfig.bSynchronize = index == 0;
#endif
    asinkConfig.sDelay.bReceived = true;
    asinkConfig.sDelay.sMeasured.uiValue = 0; /* hardcoded */
    asinkConfig.sDelay.sMeasured.eUnits = BSYNClib_Units_eMilliseconds;
    asinkConfig.bCompressed = false; /* TODO */
    BSYNClib_Channel_SetAudioSinkConfig_isr(syncChannel->synclibChannel, index, &asinkConfig);
    BKNI_LeaveCriticalSection();
    BDBG_MSG(("[%p] asink[%d] = %d", (void *)syncChannel, index, asinkConfig.bSynchronize));

    /* update status */
    syncChannel->status.audio.sinks[index].synchronized = asinkConfig.bSynchronize;
    syncChannel->status.audio.sinks[index].compressed = asinkConfig.bCompressed;

#if 0 /* really necessary? */
    if (!asinkConfig.bSynchronize) continue;
#endif
    return 0;
}

#define NEXUS_SYNC_CHANNEL_VIDEO_START_DELAY 150 /* ms */

NEXUS_Error NEXUS_SyncChannel_P_ConnectVideoInput(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings)
{
    unsigned int i = 0;
    unsigned num;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_VideoInputSyncSettings syncSettings;

    if (pSettings->videoInput) /* check new settings for input */
    {
        BDBG_MSG(("[%p] Connecting video input: %p", (void *)syncChannel, (void *)pSettings->videoInput));

        /* TODO: move context set up to create */
        /* set up context */
        syncChannel->vsourceContext.syncChannel = syncChannel;
        syncChannel->vsourceContext.index = 0;

        /* set up callbacks */
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.display);
        NEXUS_VideoInput_GetSyncSettings_priv(pSettings->videoInput, &syncSettings);
        BSYNClib_Convert_isrsafe(
            pSettings->adjustmentThreshold,
            BSYNClib_Units_eMilliseconds,
            BSYNClib_Units_e45KhzTicks,
            &syncSettings.delayCallbackThreshold);
        syncSettings.delayCallback_isr = NEXUS_Synclib_P_VideoInputDelayCallback_isr;
        syncSettings.startCallback_isr = NEXUS_Synclib_P_VideoInputStartCallback_isr;
        syncSettings.formatCallback_isr = NEXUS_Synclib_P_VideoInputFormatCallback_isr;
        syncSettings.connectionChangedCallback_isr = NEXUS_Synclib_P_VideoInputConnectionChangedCallback_isr;
        syncSettings.callbackContext = &syncChannel->vsourceContext;
        syncSettings.mute = pSettings->enableMuteControl;
        BSYNClib_Convert_isrsafe(NEXUS_SYNC_CHANNEL_VIDEO_START_DELAY, BSYNClib_Units_eMilliseconds,
            BSYNClib_Units_e45KhzTicks, &syncSettings.startDelay);
        syncSettings.delay = syncSettings.startDelay;
        rc = NEXUS_VideoInput_SetSyncSettings_priv(pSettings->videoInput, &syncSettings);
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.display);

        /* clear outputs on each input connection */
        BKNI_Memset(syncChannel->outputs.window, 0, sizeof(syncChannel->outputs.window));
        syncChannel->syncLockedWindow = NULL;

        /* discover outputs */
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.display);
        NEXUS_Display_P_GetWindows_priv(pSettings->videoInput, syncChannel->outputs.window, NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS, &num);
        /* TODO: refcnt each window */
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.display);

        /* connect all discovered video outputs for this input */
        for (i = 0; i < num; i++)
        {
#if 0 /* synclib connections are all done in create right now */
            rc = NEXUS_Synclib_P_ConnectVideoSink(syncChannel, pSettings, i);
            if (rc) goto end;
#endif
            rc = NEXUS_SyncChannel_P_ConnectVideoOutput(syncChannel, pSettings, i);
            if (rc) goto end;
        }
    }

end:

    return rc;
}

void NEXUS_SyncChannel_P_DisconnectVideoInput(NEXUS_SyncChannelHandle syncChannel)
{
    unsigned int i = 0;
    NEXUS_VideoInputSyncSettings syncSettings;

    if (syncChannel->settings.videoInput) /* check current settings for input */
    {
        BDBG_MSG(("[%p] Disconnecting video input: %p", (void *)syncChannel, (void *)syncChannel->settings.videoInput));

        /* need to disconnect all outputs first */
        for (i = 0; i < NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS; i++)
        {
            NEXUS_SyncChannel_P_DisconnectVideoOutput(syncChannel, i);
#if 0 /* synclib connections are all done in create right now */
            NEXUS_Synclib_P_DisconnectVideoSink(syncChannel, pSettings, i);
#endif
        }

        /* tear down callbacks; reset mute */
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.display);
        NEXUS_VideoInput_GetSyncSettings_priv(syncChannel->settings.videoInput, &syncSettings);
        syncSettings.delayCallback_isr  = NULL;
        syncSettings.startCallback_isr  = NULL;
        syncSettings.connectionChangedCallback_isr  = NULL;
        syncSettings.formatCallback_isr = NULL;
        syncSettings.callbackContext = NULL;
        syncSettings.mute = false;
        NEXUS_VideoInput_SetSyncSettings_priv(syncChannel->settings.videoInput, &syncSettings);
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.display);
    }
}

NEXUS_Error NEXUS_SyncChannel_P_ConnectVideoOutput(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_VideoWindowSyncSettings syncSettings;

    BSTD_UNUSED(pSettings); /* for now */

    /* need to lock before we check the handle, as display can kill it if we don't have the lock */
    NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.display);
    if (syncChannel->outputs.window[index]) /* check current outputs */
    {
        BDBG_MSG(("[%p] Connecting video output: %p", (void *)syncChannel, (void *)syncChannel->outputs.window[index]));

        /* TODO: move context set up to create */
        /* set up context */
        syncChannel->vsinkContext[index].syncChannel = syncChannel;
        syncChannel->vsinkContext[index].index = index;

        /* set up callbacks */
        NEXUS_VideoWindow_GetSyncSettings_priv(syncChannel->outputs.window[index], &syncSettings);
        syncSettings.delayCallback_isr = NEXUS_Synclib_P_VideoOutputDelayCallback_isr;
        syncSettings.stateChangeCallback_isr = NEXUS_Synclib_P_VideoOutputStateChangeCallback_isr;
        syncSettings.formatCallback_isr = NEXUS_Synclib_P_VideoOutputFormatCallback_isr;
        syncSettings.closeCallback_isr = NEXUS_Synclib_P_VideoOutputCloseCallback_isr;
        syncSettings.callbackContext = &syncChannel->vsinkContext[index];
        rc = NEXUS_VideoWindow_SetSyncSettings_priv(syncChannel->outputs.window[index], &syncSettings);
    }
    NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.display);

    return rc;
}

void NEXUS_SyncChannel_P_DisconnectVideoOutput(NEXUS_SyncChannelHandle syncChannel, unsigned int index)
{
    NEXUS_VideoWindowSyncSettings syncSettings;

    /* need to lock before we check the handle, as display can kill it if we don't have the lock */
    NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.display);
    if (syncChannel->outputs.window[index]) /* check current outputs */
    {
        BDBG_MSG(("[%p] Disconnecting video output %u: %p", (void *)syncChannel, index, (void *)syncChannel->outputs.window[index]));

        /* tear down callbacks */
        NEXUS_VideoWindow_GetSyncSettings_priv(syncChannel->outputs.window[index], &syncSettings);
        syncSettings.delayCallback_isr = NULL;
        syncSettings.stateChangeCallback_isr = NULL;
        syncSettings.formatCallback_isr = NULL;
        syncSettings.closeCallback_isr = NULL;
        syncSettings.callbackContext = NULL;
        (void)NEXUS_VideoWindow_SetSyncSettings_priv(syncChannel->outputs.window[index], &syncSettings);
    }
    NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.display);
}

NEXUS_Error NEXUS_SyncChannel_P_ConnectAudioInput(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index)
{
    unsigned int i = 0;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_AudioInputSyncSettings syncSettings;

    if (pSettings->audioInput[index])
    {
        BDBG_MSG(("[%p] Connecting audio input: %p", (void *)syncChannel, (void *)pSettings->audioInput[index]));

        /* TODO: move context set up to create */
        /* set up context */
        syncChannel->asourceContext[index].syncChannel = syncChannel;
        syncChannel->asourceContext[index].index = index;

        /* set up callbacks */
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.audio);
        NEXUS_AudioInput_GetSyncSettings_priv(pSettings->audioInput[index], &syncSettings);
        syncSettings.startCallback_isr = NEXUS_Synclib_P_AudioInputStartCallback_isr;
        syncSettings.sampleRateCallback_isr = NEXUS_Synclib_P_AudioInputSampleRateCallback_isr;
        syncSettings.callbackContext = &syncChannel->asourceContext[index];
        syncSettings.mute = pSettings->enableMuteControl;
        rc = NEXUS_AudioInput_SetSyncSettings_priv(pSettings->audioInput[index], &syncSettings);
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.audio);

        /* clear outputs */
        BKNI_Memset(syncChannel->outputs.audioOutput, 0, sizeof(syncChannel->outputs.audioOutput));
#if 0
            /* discover outputs */
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.audio);
        num = 0;
        if (pSettings->audioInput[0]) {
            NEXUS_Audio_P_GetOutputs_priv(pSettings->audioInput[0], syncChannel->outputs.audioOutput, NEXUS_SYNC_CHANNEL_AUDIO_OUTPUTS, &num);
        }
        if (pSettings->audioInput[1] && num < NEXUS_SYNC_CHANNEL_AUDIO_OUTPUTS) {
            NEXUS_Audio_P_GetOutputs_priv(pSettings->audioInput[1], &syncChannel->outputs.audioOutput[num], NEXUS_SYNC_CHANNEL_AUDIO_OUTPUTS-num, &num);
            /* TODO: pack list. unlikely, but there could be dups because of mixing. */
        }
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.audio);
#else
            /* discover outputs */
        syncChannel->outputs.audioOutput[0] = (NEXUS_AudioOutput)1;
#endif

        /* connect all discovered audio outputs for this input */
        for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_OUTPUTS; i++)
        {
#if 0 /* synclib connections are all done in create right now */
            rc = NEXUS_Synclib_P_ConnectAudioSink(syncChannel, pSettings, i);
            if (rc) goto end;
#endif

            rc = NEXUS_SyncChannel_P_ConnectAudioOutput(syncChannel, pSettings, i);
            if (rc) goto end;
        }
    }

end:

    return rc;
}

void NEXUS_SyncChannel_P_DisconnectAudioInput(NEXUS_SyncChannelHandle syncChannel, unsigned int index)
{
    unsigned int i = 0;
    NEXUS_AudioInputSyncSettings syncSettings;

    if (syncChannel->settings.audioInput[index])
    {
        BDBG_MSG(("[%p] Disconnecting audio input: %p", (void *)syncChannel, (void *)syncChannel->settings.audioInput[index]));

        /* need to disconnect all outputs first */
        for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_OUTPUTS; i++)
        {
            NEXUS_SyncChannel_P_DisconnectAudioOutput(syncChannel, i);
#if 0 /* synclib connections are all done in create right now */
            NEXUS_Synclib_P_DisconnectAudioSink(syncChannel, i);
#endif
        }

        /* tear down callbacks, and reset mute and delay */
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.audio);
        NEXUS_AudioInput_GetSyncSettings_priv(syncChannel->settings.audioInput[index], &syncSettings);
        syncSettings.startCallback_isr      = NULL;
        syncSettings.sampleRateCallback_isr = NULL;
        syncSettings.callbackContext        = NULL;
        syncSettings.mute = false;
/* 20100727 bandrews - resetting the delay here causes us to be off by 1 frame */
/*        syncSettings.delay = 0;*/
        NEXUS_AudioInput_SetSyncSettings_priv(syncChannel->settings.audioInput[index], &syncSettings);
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.audio);
    }
}

NEXUS_Error NEXUS_SyncChannel_P_ConnectAudioOutput(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings, unsigned int index)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
/* TODO */
#if 0
    NEXUS_AudioOutputSyncSettings syncSettings;

    BDBG_MSG(("Connecting audio output: %p", syncChannel->outputs.audioOutput[index]));

    if (syncChannel->outputs.audioOutput[index])
    {
        /* TODO: move context set up to create */
        syncChannel->asinkContext[index].syncChannel = syncChannel;
        syncChannel->asinkContext[index].index = index;

        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.audio);
        NEXUS_AudioOutput_GetSyncSettings_priv(syncChannel->outputs.audioOutput[index], &syncSettings);
            syncSettings.startCallback_isr = NEXUS_Synclib_P_AudioOutputStartCallback_isr;
        syncSettings.callbackContext = &syncChannel->asinkContext[index];
        rc = NEXUS_AudioOutput_SetSyncSettings_priv(syncChannel->outputs.audioOutput[index], &syncSettings);
            NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.audio);
        }
#else
    BSTD_UNUSED(syncChannel);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(index);
    if (syncChannel->outputs.audioOutput[index])
    {
        BDBG_MSG(("[%p] Connecting audio output: %p", (void *)syncChannel, (void *)syncChannel->outputs.audioOutput[index]));
    }
#endif

    return rc;
}

void NEXUS_SyncChannel_P_DisconnectAudioOutput(NEXUS_SyncChannelHandle syncChannel, unsigned int index)
{
/* TODO */
#if 0
    NEXUS_AudioOutputSyncSettings syncSettings;

    BDBG_MSG(("Disconnecting audio output: %p", syncChannel->outputs.audioOutput[index]));

    if (syncChannel->outputs.audioOutput[index])
    {
        NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.audio);
        NEXUS_AudioOutput_GetSyncSettings_priv(syncChannel->outputs.audioOutput[index], &syncSettings);
        syncSettings.startCallback_isr = NULL;
        syncSettings.callbackContext = NULL;
        NEXUS_AudioOutput_SetSyncSettings_priv(syncChannel->outputs.audioOutput[index], &syncSettings);
        NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.audio);
    }
#else
    BSTD_UNUSED(syncChannel);
    BSTD_UNUSED(index);
    if (syncChannel->outputs.audioOutput[index])
    {
        BDBG_MSG(("[%p] Disconnecting audio output: %p", (void *)syncChannel, (void *)syncChannel->outputs.audioOutput[index]));
    }
#endif
}

#if NEXUS_SYNC_CHANNEL_DEBUG_AUDIO_CONNECTIONS
void printAttachedAudios(NEXUS_SyncChannelHandle syncChannel)
{
    unsigned i;

    for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_INPUTS; i++)
    {
        BDBG_MSG(("[%p] audioInput[%d] = %p", syncChannel, i, syncChannel->settings.audioInput[i]));
    }
}
#endif

NEXUS_Error NEXUS_SyncChannel_SetSettings(NEXUS_SyncChannelHandle syncChannel, const NEXUS_SyncChannelSettings *pSettings)
{
    BSYNClib_Channel_Config synclibConfig;
    unsigned i;
    NEXUS_Error rc;
    bool setPrecisionSync = false;

    BDBG_OBJECT_ASSERT(syncChannel, NEXUS_SyncChannel);

    if (pSettings == NULL)
    {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BDBG_MSG(("[%p] Disabling synclib", (void *)syncChannel));
    /* disable */
    BSYNClib_Channel_GetConfig(syncChannel->synclibChannel, &synclibConfig);
    synclibConfig.bEnabled = false;
    BSYNClib_Channel_SetConfig(syncChannel->synclibChannel, &synclibConfig);
    BDBG_MSG(("[%p] Synclib disabled", (void *)syncChannel));

    if ((syncChannel->settings.videoInput && !pSettings->videoInput) /* disconnect old */
        || (syncChannel->settings.videoInput && pSettings->videoInput
        && (syncChannel->settings.videoInput != pSettings->videoInput))) /* change */
    {
        /* disconnect video input */
        NEXUS_SyncChannel_P_DisconnectVideoInput(syncChannel);
    }

    if ((!syncChannel->settings.videoInput && pSettings->videoInput) /* connect new */
        || (syncChannel->settings.videoInput && pSettings->videoInput
        && (syncChannel->settings.videoInput != pSettings->videoInput))) /* change */
    {
        /* connect video input */
        rc = NEXUS_SyncChannel_P_ConnectVideoInput(syncChannel, pSettings);
        if (rc) goto end;
    }

    /* TODO: more external settings coming */
    rc = NEXUS_Synclib_P_SetVideoSource(syncChannel, pSettings, 0);
    if (rc) goto end;

    for (i = 0; i < NEXUS_SYNC_CHANNEL_VIDEO_OUTPUTS; i++)
    {
        rc = NEXUS_Synclib_P_SetVideoSink(syncChannel, pSettings, i);
        if (rc) goto end;
    }

    /* process all disconnections for all audio channels first
    because some outputs may be disconnected from one input
    and connected to another */
    for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_INPUTS; i++)
    {
        if ((syncChannel->settings.audioInput[i] && !pSettings->audioInput[i]) /* disconnect old */
            || (syncChannel->settings.audioInput[i] && pSettings->audioInput[i]
            && (syncChannel->settings.audioInput[i] != pSettings->audioInput[i]))) /* change */
        {
            /* disconnect audio input i */
            NEXUS_SyncChannel_P_DisconnectAudioInput(syncChannel, i);
        }
    }

    /* process all connections for all audio channels second */
    for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_INPUTS; i++)
    {
        if ((!syncChannel->settings.audioInput[i] && pSettings->audioInput[i]) /* connect new */
            || (syncChannel->settings.audioInput[i] && pSettings->audioInput[i]
            && (syncChannel->settings.audioInput[i] != pSettings->audioInput[i]))) /* change */
        {
            /* connect audio input i */
            rc = NEXUS_SyncChannel_P_ConnectAudioInput(syncChannel, pSettings, i);
            if (rc) goto end;
        }
    }

    /* TODO: more external settings coming */
    for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_INPUTS; i++)
    {
        rc = NEXUS_Synclib_P_SetAudioSource(syncChannel, pSettings, i);
        if (rc) goto end;
    }

    for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_OUTPUTS; i++)
    {
        rc = NEXUS_Synclib_P_SetAudioSink(syncChannel, pSettings, i);
        if (rc) goto end;
    }

    /* PR49294 20081125 bandrews - added precision lipsync flag */
    if (pSettings->enablePrecisionLipsync != syncChannel->settings.enablePrecisionLipsync)
    {
        setPrecisionSync = true;
    }
    if ((pSettings->enableMuteControl != syncChannel->settings.enableMuteControl)
        || (pSettings->simultaneousUnmute != syncChannel->settings.simultaneousUnmute)
        || (pSettings->allowIncrementalStart != syncChannel->settings.allowIncrementalStart))
    {
        BDBG_MSG(("[%p] mute control: %s %s %s", (void *)syncChannel,
            pSettings->enableMuteControl ? "enabled" : "disabled",
            pSettings->simultaneousUnmute ? "simultaneous" : "staggered",
            pSettings->allowIncrementalStart ? "incremental" : "conjoined"));
        BSYNClib_Channel_GetConfig(syncChannel->synclibChannel, &synclibConfig);
        synclibConfig.sMuteControl.bEnabled = pSettings->enableMuteControl;
        synclibConfig.sMuteControl.bSimultaneousUnmute = pSettings->simultaneousUnmute;
        synclibConfig.sMuteControl.bAllowIncrementalStart = pSettings->allowIncrementalStart;
        BSYNClib_Channel_SetConfig(syncChannel->synclibChannel, &synclibConfig);
    }

    /* if changing mute control, need to tell decoders */
    if (pSettings->enableMuteControl != syncChannel->settings.enableMuteControl)
    {
        NEXUS_VideoInputSyncSettings videoSettings;
        NEXUS_AudioInputSyncSettings audioSettings;

        if (pSettings->videoInput)
        {
            NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.display);
            NEXUS_VideoInput_GetSyncSettings_priv(pSettings->videoInput, &videoSettings);
            videoSettings.mute = pSettings->enableMuteControl;
            rc = NEXUS_VideoInput_SetSyncSettings_priv(pSettings->videoInput, &videoSettings);
            NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.display);
        }

        for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_INPUTS; i++)
        {
            if (pSettings->audioInput[i])
            {
                NEXUS_Module_Lock(g_NEXUS_syncChannel.settings.modules.audio);
                NEXUS_AudioInput_GetSyncSettings_priv(pSettings->audioInput[i], &audioSettings);
                audioSettings.mute = pSettings->enableMuteControl;
                rc = NEXUS_AudioInput_SetSyncSettings_priv(pSettings->audioInput[i], &audioSettings);
                NEXUS_Module_Unlock(g_NEXUS_syncChannel.settings.modules.audio);
            }
        }
    }

    /* copy settings to internal */
    syncChannel->settings = *pSettings;

#if NEXUS_SYNC_CHANNEL_DEBUG_AUDIO_CONNECTIONS
    printAttachedAudios(syncChannel);
#endif

    if (setPrecisionSync)
    {
        NEXUS_SyncChannel_P_SetPrecisionLipsync(syncChannel);
    }

    /* apply connection settings */
    for (i = 0; i < NEXUS_SYNC_CHANNEL_AUDIO_INPUTS; i++)
    {
        rc = NEXUS_SyncChannel_P_ApplyAudioConnectState(syncChannel, pSettings, i);
        if (rc) goto end;
    }

    BDBG_MSG(("[%p] Enabling synclib", (void *)syncChannel));
    /* enable */
    BSYNClib_Channel_GetConfig(syncChannel->synclibChannel, &synclibConfig);
    synclibConfig.bEnabled = true;
    BSYNClib_Channel_SetConfig(syncChannel->synclibChannel, &synclibConfig);
    BDBG_MSG(("[%p] Synclib enabled", (void *)syncChannel));

end:

    return 0;
}

void NEXUS_SyncChannel_SimpleVideoConnected_priv(NEXUS_SyncChannelHandle syncChannel)
{
    if (!syncChannel->simpleVideoConnected)
    {
        BDBG_MSG(("[%p] SimpleVideoConnected", (void*)syncChannel));
        syncChannel->simpleVideoConnected = true;
        NEXUS_Synclib_P_SetVideoSource(syncChannel, &syncChannel->settings, 0);
    }
}

void NEXUS_SyncChannel_SimpleVideoDisconnected_priv(NEXUS_SyncChannelHandle syncChannel)
{
    if (syncChannel->simpleVideoConnected)
    {
        BDBG_MSG(("[%p] SimpleVideoDisconnected", (void*)syncChannel));
        syncChannel->simpleVideoConnected = false;
        NEXUS_Synclib_P_SetVideoSource(syncChannel, &syncChannel->settings, 0);
    }
}

void NEXUS_SyncChannel_SimpleAudioConnected_priv(NEXUS_SyncChannelHandle syncChannel)
{
    BDBG_MSG(("[%p] SimpleAudioConnected", (void*)syncChannel));
    syncChannel->simpleAudioConnected = true;
    NEXUS_Synclib_P_SetAudioSource(syncChannel, &syncChannel->settings, 0);
}

void NEXUS_SyncChannel_SimpleAudioDisconnected_priv(NEXUS_SyncChannelHandle syncChannel)
{
    BDBG_MSG(("[%p] SimpleAudioDisconnected", (void*)syncChannel));
    syncChannel->simpleAudioConnected = false;
    NEXUS_Synclib_P_SetAudioSource(syncChannel, &syncChannel->settings, 0);
}
