/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#include "nexus_astm_module.h"
#include "priv/nexus_core.h"
#include "priv/nexus_syslib_framework.h"
#include "priv/nexus_video_decoder_priv.h"
#include "priv/nexus_audio_decoder_priv.h"
#include "priv/nexus_stc_channel_priv.h"
#include "priv/nexus_timebase_priv.h"
#include "bastmlib.h"
#if NEXUS_HAS_ASTM_TEST_SUPPORT
#include "nexus_astm_test_extensions.h"
#endif
#include "blst_queue.h"

BDBG_MODULE(nexus_astm);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */
#define NEXUS_ASTM_P_PRINT_SETTINGS 0 /* set to 1 to enable printing settings on start */
#define NEXUS_ASTM_ALC_SUPPORT 1

static NEXUS_Error NEXUS_Astm_P_Start(NEXUS_AstmHandle astm);
static void NEXUS_Astm_P_Stop(NEXUS_AstmHandle astm);
static void NEXUS_Astm_P_TsmRecoveryAcquisitionTask(void * context);
static void NEXUS_Astm_P_TsmRecoveryTrackingTimeoutHandler(void * context);
static void NEXUS_AstmModule_P_Print(void);

NEXUS_ModuleHandle g_NEXUS_astmModule;

static struct
{
    NEXUS_AstmModuleSettings settings;
    NEXUS_SYSlib_ContextHandle syslibContext;
    BLST_Q_HEAD(nexus_astm_list, NEXUS_Astm) contexts;
} g_astm;

void NEXUS_AstmModule_GetDefaultSettings(NEXUS_AstmModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_ModuleHandle NEXUS_AstmModule_Init(const NEXUS_AstmModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_SYSlib_ContextSettings syslibSettings;

    BDBG_ASSERT(!g_NEXUS_astmModule);

    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pSettings->modules.videoDecoder);
    BDBG_ASSERT(pSettings->modules.audio);
    BDBG_ASSERT(pSettings->modules.transport);

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow;
    moduleSettings.dbgPrint = NEXUS_AstmModule_P_Print;
    moduleSettings.dbgModules = "nexus_astm";
    g_NEXUS_astmModule = NEXUS_Module_Create("astm", &moduleSettings);
    if (!g_NEXUS_astmModule) {
        return NULL;
    }
    NEXUS_LockModule();

    g_astm.settings = *pSettings;

    NEXUS_SYSlib_GetDefaultContextSettings_priv(&syslibSettings);
    syslibSettings.module = NEXUS_MODULE_SELF;
    NEXUS_SYSlib_CreateContext_priv(&g_astm.syslibContext, &syslibSettings);

    NEXUS_UnlockModule();
    return g_NEXUS_astmModule;
}

void NEXUS_AstmModule_Uninit()
{
    NEXUS_LockModule();
    NEXUS_SYSlib_DestroyContext_priv(g_astm.syslibContext);
    g_astm.syslibContext = NULL;
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_astmModule);
    g_NEXUS_astmModule = NULL;
}

/****************************************
* API functions
***************/

struct NEXUS_AstmDecoderStatus
{
    bool started;
    uint32_t pts;
    int32_t ptsStcDiff;
    unsigned int decodedCount;
    struct
    {
        unsigned int address;
        unsigned int size;
    } tsmLog;
};

struct NEXUS_AstmDecoderSettings
{
    int32_t ptsOffset;
    bool tsm;
};

typedef void (*NEXUS_Astm_DecoderStatusAccessor_isr)(NEXUS_AstmHandle astm, void * decoder, struct NEXUS_AstmDecoderStatus * status);
typedef void (*NEXUS_Astm_DecoderStatusAccessor)(NEXUS_AstmHandle astm, void * decoder, struct NEXUS_AstmDecoderStatus * status);
typedef void (*NEXUS_Astm_DecoderSettingsMutator)(NEXUS_AstmHandle astm, void * decoder, const struct NEXUS_AstmDecoderSettings * settings);
typedef void (*NEXUS_Astm_DecoderSettingsAccessor)(NEXUS_AstmHandle astm, void * decoder, struct NEXUS_AstmDecoderSettings * settings);

/* Used to collect information to process callbacks from around the system */
struct NEXUS_AstmContext
{
    NEXUS_AstmHandle astm;
    NEXUS_StcChannelHandle stc;
    bool started;
    void * decoder;
#ifdef BDBG_DEBUG_BUILD
    const char * decoderName;
#endif
    NEXUS_Astm_DecoderStatusAccessor_isr getStatus_isr;
    NEXUS_Astm_DecoderStatusAccessor getStatus;
    NEXUS_Astm_DecoderSettingsAccessor getSettings;
    NEXUS_Astm_DecoderSettingsMutator setSettings;
    BASTMlib_Presenter_Handle presenter;
    BKNI_EventHandle watchdogEvent;
    NEXUS_EventCallbackHandle watchdogEventHandler;
    BKNI_EventHandle lifecycleEvent;
    NEXUS_EventCallbackHandle lifecycleEventHandler;

    unsigned passEventCount;
    unsigned lastDecodedCount;
    NEXUS_TimerHandle tsmRecoveryAcquisitionTimer;
    NEXUS_TimerHandle tsmRecoveryTrackingTimer;
    unsigned ptsStcDiffAdjustmentThreshold;
    unsigned tsmRecoveryAcquisitionPeriod;
    unsigned tsmRecoveryTrackingTimeout;
    BKNI_EventHandle tsmPassEvent;
    NEXUS_EventCallbackHandle tsmPassEventHandler;
    uint32_t maxAllowableFirstPtsStcDiff;
    BKNI_EventHandle firstPtsEvent;
    NEXUS_EventCallbackHandle firstPtsEventHandler;
    bool firstPtsTsm;
    bool firstPtsReceived;
    uint32_t firstPts;
    bool tsmThresholdAdjustment;
    bool manageRateControl;
    NEXUS_ModuleHandle module;
    uint32_t firstPtsArrivalPcr;
    bool firstPtsArrivalPcrValid;
};

#define NEXUS_ASTM_PRESENTERS 3


typedef struct NEXUS_AstmStatus
{
    NEXUS_AstmPresentationRateControl presentationRateControl;
    NEXUS_AstmStcSource stcSource;
    NEXUS_AstmClockCoupling clockCoupling;
    NEXUS_StcChannelTsmMode tsmMode;
} NEXUS_AstmStatus;

struct NEXUS_Astm
{
    NEXUS_OBJECT(NEXUS_Astm);
    BLST_Q_ENTRY(NEXUS_Astm) link;
    NEXUS_AstmSettings settings;
    NEXUS_AstmStatus status;
    NEXUS_TransportType transportType;
    NEXUS_TimebaseHandle timebase;
    struct NEXUS_AstmContext presenterContexts[NEXUS_ASTM_PRESENTERS];

    /* syslib stuff */
    struct
    {
        BASTMlib_Handle handle;
        BASTMlib_Status status;
    } lib;

    bool ready;
    bool started;
    uint32_t lastPcr;
    bool pcrReceived;
    bool usePtsMasterMode;
};

#if 0 /* old recovery scheme */
#define NEXUS_ASTM_DEFAULT_PASS_EVENT_COUNT_THRESHOLD pcfg.uiPassEventCountThreshold
#else /* new TSM recovery scheme */
#define NEXUS_ASTM_DEFAULT_PASS_EVENT_COUNT_THRESHOLD 0
#endif
#define NEXUS_ASTM_DEFAULT_PTS_STC_DIFF_ADJ_THRESHOLD 90 /* 45 KHz ticks -> 2 ms */
#define NEXUS_ASTM_DEFAULT_TSM_RECOVERY_ACQ_PERIOD 200 /* ms */
#define NEXUS_ASTM_DEFAULT_TSM_RECOVERY_TRACKING_TIMEOUT 1000 /* ms */
#define NEXUS_ASTM_DEFAULT_MAX_ALLOWABLE_VIDEO_FIRST_PTS_STC_DIFF 454500 /* 10.1 seconds, want this outside the avc spec */
#define NEXUS_ASTM_DEFAULT_MAX_ALLOWABLE_AUDIO_FIRST_PTS_STC_DIFF 112500 /* 2.5 seconds, < 3 second live audio buffer */
#define NEXUS_ASTM_P_SYNC_LIMIT 5000

#if !(BDBG_NO_LOG && BDBG_NO_MSG)
static void NEXUS_Astm_P_PrintDependencyAstmSettings(NEXUS_AstmHandle astm);
#endif
static void NEXUS_AstmModule_P_Print(void)
{
#if !(BDBG_NO_LOG && BDBG_NO_MSG)
    NEXUS_AstmHandle astm;
    for (astm = BLST_Q_FIRST(&g_astm.contexts); astm; astm = BLST_Q_NEXT(astm, link)) {
        NEXUS_Astm_P_PrintDependencyAstmSettings(astm);
    }
#endif
}

void NEXUS_Astm_GetDefaultSettings(NEXUS_AstmSettings *pSettings)
{
    BASTMlib_Config libcfg;
    BASTMlib_Presenter_Config pcfg;
    unsigned int i = 0;

    BDBG_ASSERT(pSettings);

    BASTMlib_GetDefaultConfig(&libcfg);
    BASTMlib_Presenter_GetDefaultConfig(&pcfg);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->enabled = false;
    pSettings->enableAutomaticLifecycleControl = false; /* PR:50938 */ /* PR:51684 updated to false */
    pSettings->syncLimit = NEXUS_ASTM_P_SYNC_LIMIT;
    pSettings->stcMaster = NULL;
    pSettings->clockCoupling = NEXUS_AstmClockCoupling_eAdaptive;
    pSettings->stcSource = NEXUS_AstmStcSource_eAdaptive;
    pSettings->presentationRateControl = NEXUS_AstmPresentationRateControl_eAdaptive;

    /* clock coupling task default cfg */
    pSettings->clockCouplingConfig.initialAcquisitionTime = libcfg.sClockCoupling.uiInitialAcquisitionTime;
    pSettings->clockCouplingConfig.processingFrequency = libcfg.sClockCoupling.uiProcessingFrequency;
    pSettings->clockCouplingConfig.idealProcessingFrequency = libcfg.sClockCoupling.uiProcessingFrequency;
    pSettings->clockCouplingConfig.settlingTime = libcfg.sClockCoupling.uiSettlingTime;

    pSettings->clockCouplingConfig.minimumTimeBetweenEvents = libcfg.sClockCoupling.uiMinimumTimeBetweenEvents;
    pSettings->clockCouplingConfig.deviationThreshold = libcfg.sClockCoupling.uiDeviationThreshold;
    pSettings->clockCouplingConfig.deviantCountThreshold = libcfg.sClockCoupling.uiDeviantCountThreshold;
    pSettings->clockCouplingConfig.idealCountThreshold = libcfg.sClockCoupling.uiIdealCountThreshold;

    /* presentation task default cfg */
    pSettings->presentationConfig.initialAcquisitionTime = libcfg.sPresentation.uiInitialAcquisitionTime;
    pSettings->presentationConfig.processingFrequency = libcfg.sPresentation.uiProcessingFrequency;
    pSettings->presentationConfig.tsmDisabledWatchdogTimeout = libcfg.sPresentation.uiTsmDisabledWatchdogTimeout;
    pSettings->presentationConfig.settlingTime = libcfg.sPresentation.uiSettlingTime;

    pSettings->videoPresenterConfig.minimumTimeBetweenEvents = pcfg.uiMinimumTimeBetweenEvents;
    pSettings->videoPresenterConfig.failEventCountThreshold = pcfg.uiFailEventCountThreshold;
    pSettings->videoPresenterConfig.passEventCountThreshold = NEXUS_ASTM_DEFAULT_PASS_EVENT_COUNT_THRESHOLD;
    pSettings->videoPresenterConfig.ptsStcDiffAdjustmentThreshold = NEXUS_ASTM_DEFAULT_PTS_STC_DIFF_ADJ_THRESHOLD;
    pSettings->videoPresenterConfig.tsmRecoveryAcquisitionPeriod = NEXUS_ASTM_DEFAULT_TSM_RECOVERY_ACQ_PERIOD;
    pSettings->videoPresenterConfig.tsmRecoveryTrackingTimeout = NEXUS_ASTM_DEFAULT_TSM_RECOVERY_TRACKING_TIMEOUT;
    pSettings->videoPresenterConfig.maxAllowableFirstPtsStcDiff = NEXUS_ASTM_DEFAULT_MAX_ALLOWABLE_VIDEO_FIRST_PTS_STC_DIFF;
    pSettings->videoPresenterConfig.tsmThresholdAdjustment = false; /* default off for video */
    pSettings->videoPresenterConfig.manageRateControl = true;

    for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
    {
        pSettings->audioPresenterConfig[i].failEventCountThreshold = pcfg.uiFailEventCountThreshold;
        pSettings->audioPresenterConfig[i].minimumTimeBetweenEvents = pcfg.uiMinimumTimeBetweenEvents;
        pSettings->audioPresenterConfig[i].passEventCountThreshold = NEXUS_ASTM_DEFAULT_PASS_EVENT_COUNT_THRESHOLD;
        pSettings->audioPresenterConfig[i].ptsStcDiffAdjustmentThreshold = NEXUS_ASTM_DEFAULT_PTS_STC_DIFF_ADJ_THRESHOLD;
        pSettings->audioPresenterConfig[i].tsmRecoveryAcquisitionPeriod = NEXUS_ASTM_DEFAULT_TSM_RECOVERY_ACQ_PERIOD;
        pSettings->audioPresenterConfig[i].tsmRecoveryTrackingTimeout = NEXUS_ASTM_DEFAULT_TSM_RECOVERY_TRACKING_TIMEOUT;
        pSettings->audioPresenterConfig[i].maxAllowableFirstPtsStcDiff = NEXUS_ASTM_DEFAULT_MAX_ALLOWABLE_AUDIO_FIRST_PTS_STC_DIFF;
        pSettings->audioPresenterConfig[i].tsmThresholdAdjustment = true; /* default on for audio */
        pSettings->audioPresenterConfig[i].manageRateControl = true;
    }
}

#if !BDBG_NO_MSG
static const char * const stcSourceStrings[] =
{
    "Adaptive",
    "PCR",
    "PTS",
    NULL
};
#endif

static BASTMlib_ClockCoupling NEXUS_Astm_P_ClockCouplingToMagnum(NEXUS_AstmClockCoupling ncoupling)
{
    BASTMlib_ClockCoupling mcoupling;

    switch (ncoupling)
    {
        case NEXUS_AstmClockCoupling_eInternalClock:
            mcoupling = BASTMlib_ClockCoupling_eInternalClock;
            break;
        case NEXUS_AstmClockCoupling_eInputClock:
        default:
            mcoupling = BASTMlib_ClockCoupling_eInputClock;
            break;
    }

    return mcoupling;
}

static NEXUS_AstmClockCoupling NEXUS_Astm_P_ClockCouplingFromMagnum(BASTMlib_ClockCoupling mcoupling)
{
    NEXUS_AstmClockCoupling ncoupling;

    switch (mcoupling)
    {
        case BASTMlib_ClockCoupling_eInternalClock:
            ncoupling = NEXUS_AstmClockCoupling_eInternalClock;
            break;
        case BASTMlib_ClockCoupling_eInputClock:
        default:
            ncoupling = NEXUS_AstmClockCoupling_eInputClock;
            break;
    }

    return ncoupling;
}

static BASTMlib_StcSource NEXUS_Astm_P_StcSourceToMagnum(NEXUS_AstmStcSource nsource)
{
    BASTMlib_StcSource msource;

    switch (nsource)
    {
        case NEXUS_AstmStcSource_ePts:
            msource = BASTMlib_StcSource_ePresenter;
            break;
        case NEXUS_AstmStcSource_eClockReference:
        default:
            msource = BASTMlib_StcSource_eClockReference;
            break;
    }

    return msource;
}

static NEXUS_AstmStcSource NEXUS_Astm_P_StcSourceFromMagnum(BASTMlib_StcSource msource)
{
    NEXUS_AstmStcSource nsource;

    switch (msource)
    {
        case BASTMlib_StcSource_ePresenter:
            nsource = NEXUS_AstmStcSource_ePts;
            break;
        case BASTMlib_StcSource_eClockReference:
        default:
            nsource = NEXUS_AstmStcSource_eClockReference;
            break;
    }

    return nsource;
}

static BASTMlib_PresentationRateControl NEXUS_Astm_P_PresentationRateControlToMagnum(NEXUS_AstmPresentationRateControl ncontrol)
{
    BASTMlib_PresentationRateControl mcontrol;

    switch (ncontrol)
    {
        case NEXUS_AstmPresentationRateControl_eOutputClock:
            mcontrol = BASTMlib_PresentationRateControl_eOutputClock;
            break;
        case NEXUS_AstmPresentationRateControl_eTimeStamp:
        default:
            mcontrol = BASTMlib_PresentationRateControl_eTimeStamp;
            break;
    }

    return mcontrol;
}

static NEXUS_AstmPresentationRateControl NEXUS_Astm_P_PresentationRateControlFromMagnum(BASTMlib_PresentationRateControl mcontrol)
{
    NEXUS_AstmPresentationRateControl ncontrol;

    switch (mcontrol)
    {
        case BASTMlib_PresentationRateControl_eOutputClock:
            ncontrol = NEXUS_AstmPresentationRateControl_eOutputClock;
            break;
        case BASTMlib_PresentationRateControl_eTimeStamp:
        default:
            ncontrol = NEXUS_AstmPresentationRateControl_eTimeStamp;
            break;
    }

    return ncontrol;
}

static NEXUS_Error NEXUS_Astm_P_SetDependencyAstmBehavior(NEXUS_AstmHandle astm, bool enabled)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_VideoDecoderAstmSettings videoDecoderAstmSettings;
    NEXUS_AudioDecoderAstmSettings audioDecoderAstmSettings;
    NEXUS_StcChannelAstmSettings stcChannelAstmSettings;
    NEXUS_TimebaseAstmSettings timebaseAstmSettings;
    unsigned int i;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    BDBG_MSG(("%p: Setting dependency ASTM behavior to %s", (void *)astm, enabled ? "enabled" : "disabled"));
    if (astm->settings.videoDecoder)
    {
        NEXUS_Module_Lock(g_astm.settings.modules.videoDecoder);
        NEXUS_VideoDecoder_GetAstmSettings_priv(astm->settings.videoDecoder, &videoDecoderAstmSettings);
        videoDecoderAstmSettings.enableAstm = enabled;
        rc = NEXUS_VideoDecoder_SetAstmSettings_priv(astm->settings.videoDecoder, &videoDecoderAstmSettings);
        NEXUS_Module_Unlock(g_astm.settings.modules.videoDecoder);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
    }

    NEXUS_Module_Lock(g_astm.settings.modules.audio);
    for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
    {
        if (astm->settings.audioDecoder[i])
        {
            NEXUS_AudioDecoder_GetAstmSettings_priv(astm->settings.audioDecoder[i], &audioDecoderAstmSettings);
            audioDecoderAstmSettings.enableAstm = enabled;
            rc = NEXUS_AudioDecoder_SetAstmSettings_priv(astm->settings.audioDecoder[i], &audioDecoderAstmSettings);
        }
        if (rc) break;
    }
    NEXUS_Module_Unlock(g_astm.settings.modules.audio);
    if (rc) { rc = BERR_TRACE(rc); goto err; }

    /* lock xpt for stc channel and timebase */
    NEXUS_Module_Lock(g_astm.settings.modules.transport);

    if (astm->settings.stcChannel)
    {
        NEXUS_StcChannel_GetAstmSettings_priv(astm->settings.stcChannel, &stcChannelAstmSettings);
        stcChannelAstmSettings.enabled = enabled;
        rc = NEXUS_StcChannel_SetAstmSettings_priv(astm->settings.stcChannel, &stcChannelAstmSettings);
        if (rc) { rc = BERR_TRACE(rc); goto xpt_unlock; }
    }

    if (astm->timebase)
    {
        NEXUS_Timebase_GetAstmSettings_priv(astm->timebase, &timebaseAstmSettings);
        timebaseAstmSettings.enabled = enabled;
        rc = NEXUS_Timebase_SetAstmSettings_priv(astm->timebase, &timebaseAstmSettings);
        if (rc) { rc = BERR_TRACE(rc); goto xpt_unlock; }
    }

xpt_unlock:
    NEXUS_Module_Unlock(g_astm.settings.modules.transport);

end:
    return rc;

err:
    /* TODO: unroll partial changes */
    goto end;
}

static void NEXUS_Astm_P_ResolveAdaptiveModes(NEXUS_AstmHandle astm, NEXUS_AstmClockCoupling * clockCoupling, NEXUS_AstmStcSource * stcSource, NEXUS_AstmPresentationRateControl * presentationRateControl)
{
    unsigned int i;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    /* if we are adaptive and one of the presenters has already started, we
    can check the STC channel for whether we should do live or pb */
    for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
    {
        if (astm->presenterContexts[i].started)
        {
            NEXUS_StcChannelSettings stcChannelSettings;

            if (astm->settings.stcChannel)
            {
                NEXUS_StcChannel_GetSettings(astm->settings.stcChannel, &stcChannelSettings);

                if (*stcSource == NEXUS_AstmStcSource_eAdaptive)
                {
                    if (stcChannelSettings.mode == NEXUS_StcChannelMode_ePcr)
                    {
                        BDBG_MSG(("%p: STC channel user mode was PCR; setting stc source to PCR.", (void *)astm));
                        *stcSource = NEXUS_AstmStcSource_eClockReference;
                    }
                    else
                    {
                        BDBG_MSG(("%p: STC channel user mode was Auto/Host; setting stc source to PTS.", (void *)astm));
                        *stcSource = NEXUS_AstmStcSource_ePts;
                    }
                }

                if (*clockCoupling == NEXUS_AstmClockCoupling_eAdaptive)
                {
                    if (stcChannelSettings.mode == NEXUS_StcChannelMode_ePcr)
                    {
                        BDBG_MSG(("%p: STC channel user mode was PCR; setting clock coupling to input clock.", (void *)astm));
                        *clockCoupling = NEXUS_AstmClockCoupling_eInputClock;
                    }
                    else
                    {
                        BDBG_MSG(("%p: STC channel user mode was Auto/Host; setting clock coupling to internal clock.", (void *)astm));
                        *clockCoupling = NEXUS_AstmClockCoupling_eInternalClock;
                    }
                }

                break;
            }
            else
            {
                BDBG_MSG(("%p: ResolveAdaptiveModes called while STC channel not set; setting clock coupling to internal clock and stc source to PTS", (void *)astm));
                *stcSource = NEXUS_AstmStcSource_ePts;
                *clockCoupling = NEXUS_AstmClockCoupling_eInternalClock;
            }
        }
    }

    /*
     * if no presenters are running, even if we have an STC channel handle,
     * we need to default to no-PCR mode because we don't listen to stc channel
     * settings updates, and stc channel mode could change between stop/start
     * of any presenters, but not while any presenters are running.
     */
    if (*stcSource == NEXUS_AstmStcSource_eAdaptive)
    {
        *stcSource = NEXUS_AstmStcSource_ePts;
    }

    if (*clockCoupling == NEXUS_AstmClockCoupling_eAdaptive)
    {
        *clockCoupling = NEXUS_AstmClockCoupling_eInternalClock;
    }

    if (*presentationRateControl == NEXUS_AstmPresentationRateControl_eAdaptive)
    {
        *presentationRateControl = NEXUS_AstmPresentationRateControl_eTimeStamp;
    }
}

static NEXUS_Error NEXUS_Astm_P_ApplyStcSource(NEXUS_AstmHandle astm, NEXUS_AstmStcSource stcSource, void * stcMaster, unsigned syncLimit)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelAstmSettings stcChannelAstmSettings;
    NEXUS_VideoDecoderAstmSettings videoDecoderAstmSettings;
    NEXUS_AudioDecoderAstmSettings audioDecoderAstmSettings;
    unsigned int i;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (stcSource == NEXUS_AstmStcSource_eAdaptive)
    {
        BDBG_ERR(("%p: Cannot apply adaptive STC source", (void *)astm));
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }

    if (stcSource == NEXUS_AstmStcSource_eClockReference)
    {
        if (astm->settings.videoDecoder)
        {
            NEXUS_Module_Lock(g_astm.settings.modules.videoDecoder);
            NEXUS_VideoDecoder_GetAstmSettings_priv(astm->settings.videoDecoder, &videoDecoderAstmSettings);
            videoDecoderAstmSettings.enablePlayback = false;
            videoDecoderAstmSettings.syncLimit = 0;
            rc = NEXUS_VideoDecoder_SetAstmSettings_priv(astm->settings.videoDecoder, &videoDecoderAstmSettings);
            NEXUS_Module_Unlock(g_astm.settings.modules.videoDecoder);
            if (rc) { rc = BERR_TRACE(rc); goto err; }
        }
        else
        {
            BDBG_MSG(("%d:%p: video decoder was NULL", __LINE__, (void *)astm));
        }

        NEXUS_Module_Lock(g_astm.settings.modules.audio);
        for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
        {
            if (astm->settings.audioDecoder[i])
            {
                NEXUS_AudioDecoder_GetAstmSettings_priv(astm->settings.audioDecoder[i], &audioDecoderAstmSettings);
                audioDecoderAstmSettings.enablePlayback = false;
                audioDecoderAstmSettings.syncLimit = 0;
                rc = NEXUS_AudioDecoder_SetAstmSettings_priv(astm->settings.audioDecoder[i], &audioDecoderAstmSettings);
            }
            else
            {
                BDBG_MSG(("%d:%p: audio decoder %d was NULL", __LINE__, (void *)astm, i));
            }
            if (rc) break;
        }
        NEXUS_Module_Unlock(g_astm.settings.modules.audio);
        if (rc) { rc = BERR_TRACE(rc); goto err; }

        if (astm->settings.stcChannel)
        {
            NEXUS_Module_Lock(g_astm.settings.modules.transport);
            NEXUS_StcChannel_GetAstmSettings_priv(astm->settings.stcChannel, &stcChannelAstmSettings);
            stcChannelAstmSettings.mode = NEXUS_StcChannelMode_ePcr;
            stcChannelAstmSettings.syncLimit = 0;
            rc = NEXUS_StcChannel_SetAstmSettings_priv(astm->settings.stcChannel, &stcChannelAstmSettings);
            NEXUS_Module_Unlock(g_astm.settings.modules.transport);
            if (rc) { rc = BERR_TRACE(rc); goto err; }
        }
        else
        {
            BDBG_MSG(("%p: Attempted to apply stc source state to a NULL stc channel", (void *)astm));
        }
    }
    else if (stcSource == NEXUS_AstmStcSource_ePts)
    {
        if (astm->settings.videoDecoder)
        {
            NEXUS_Module_Lock(g_astm.settings.modules.videoDecoder);
            NEXUS_VideoDecoder_GetAstmSettings_priv(astm->settings.videoDecoder, &videoDecoderAstmSettings);
            if (stcMaster == astm->settings.videoDecoder)
            {
                BDBG_MSG(("%p: Stc master: video", (void *)astm));
                videoDecoderAstmSettings.syncLimit = syncLimit;
            }
            else
            {
                videoDecoderAstmSettings.syncLimit = 0;
            }

            videoDecoderAstmSettings.enablePlayback = true;

            rc = NEXUS_VideoDecoder_SetAstmSettings_priv(astm->settings.videoDecoder, &videoDecoderAstmSettings);
            NEXUS_Module_Unlock(g_astm.settings.modules.videoDecoder);
            if (rc) { rc = BERR_TRACE(rc); goto err; }
        }
        else
        {
            BDBG_MSG(("%d:%p: video decoder was NULL", __LINE__, (void *)astm));
        }

        NEXUS_Module_Lock(g_astm.settings.modules.audio);
        for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
        {
            if (astm->settings.audioDecoder[i])
            {
                NEXUS_AudioDecoder_GetAstmSettings_priv(astm->settings.audioDecoder[i], &audioDecoderAstmSettings);
                if (stcMaster == astm->settings.audioDecoder[i])
                {
                    BDBG_MSG(("%p: Stc master: audio %d", (void *)astm, i));
                    audioDecoderAstmSettings.syncLimit = syncLimit;
                }
                else
                {
                    audioDecoderAstmSettings.syncLimit = 0;
                }

                audioDecoderAstmSettings.enablePlayback = true;

                rc = NEXUS_AudioDecoder_SetAstmSettings_priv(astm->settings.audioDecoder[i], &audioDecoderAstmSettings);
                if (rc) break;
            }
            else
            {
                BDBG_MSG(("%d:%p: audio decoder %d was NULL", __LINE__, (void *)astm, i));
            }
        }
        NEXUS_Module_Unlock(g_astm.settings.modules.audio);
        if (rc) { rc = BERR_TRACE(rc); goto err; }

        if (astm->settings.stcChannel)
        {
            NEXUS_Module_Lock(g_astm.settings.modules.transport);
            NEXUS_StcChannel_GetAstmSettings_priv(astm->settings.stcChannel, &stcChannelAstmSettings);
            stcChannelAstmSettings.mode = NEXUS_StcChannelMode_eAuto;
            stcChannelAstmSettings.syncLimit = syncLimit;
            rc = NEXUS_StcChannel_SetAstmSettings_priv(astm->settings.stcChannel, &stcChannelAstmSettings);
            NEXUS_Module_Unlock(g_astm.settings.modules.transport);
            if (rc) { rc = BERR_TRACE(rc); goto err; }
        }
        else
        {
            BDBG_MSG(("%p: Attempted to apply stc source state to a NULL stc channel", (void *)astm));
        }
    }

    if (astm->status.stcSource != stcSource)
    {
        BDBG_MSG(("%p: stc source: %s", (void *)astm, stcSourceStrings[stcSource]));
        astm->status.stcSource = stcSource;
    }

end:
    return rc;

err:
    /* TODO: rollback partial changes */
    goto end;
}


#if !(BDBG_NO_LOG && BDBG_NO_MSG)
static const char * const clockCouplingStrings[] =
{
    "Adaptive",
    "Input clock",
    "Internal clock",
    NULL
};
#endif

static NEXUS_TimebaseClockCoupling NEXUS_Astm_P_ClockCouplingToTimebase(NEXUS_AstmClockCoupling ncoupling)
{
    NEXUS_TimebaseClockCoupling tcoupling = NEXUS_TimebaseClockCoupling_eMax;

    switch (ncoupling)
    {
        case NEXUS_AstmClockCoupling_eInputClock:
            tcoupling = NEXUS_TimebaseClockCoupling_eInputClock;
            break;
        case NEXUS_AstmClockCoupling_eInternalClock:
            tcoupling = NEXUS_TimebaseClockCoupling_eInternalClock;
            break;
        default:
            break;
    }

    return tcoupling;
}

static NEXUS_Error NEXUS_Astm_P_ApplyClockCoupling(NEXUS_AstmHandle astm, NEXUS_AstmClockCoupling clockCoupling)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_TimebaseAstmSettings timebaseAstmSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (astm->timebase)
    {
        if (clockCoupling == NEXUS_AstmClockCoupling_eAdaptive)
        {
            BDBG_ERR(("Cannot apply adaptive clock coupling"));
            rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto err;
        }

        NEXUS_Module_Lock(g_astm.settings.modules.transport);
        NEXUS_Timebase_GetAstmSettings_priv(astm->timebase, &timebaseAstmSettings);
        timebaseAstmSettings.clockCoupling = NEXUS_Astm_P_ClockCouplingToTimebase(clockCoupling);
        rc = NEXUS_Timebase_SetAstmSettings_priv(astm->timebase, &timebaseAstmSettings);
        NEXUS_Module_Unlock(g_astm.settings.modules.transport);
        if (rc) {rc = BERR_TRACE(rc); goto err; }
    }
    else
    {
        BDBG_MSG(("%p: Attempted to apply clock coupling state to an invalid timebase", (void *)astm));
    }

    if (astm->status.clockCoupling != clockCoupling)
    {
        BDBG_MSG(("%p: clock coupling: %s", (void *)astm, clockCouplingStrings[clockCoupling]));
        astm->status.clockCoupling = clockCoupling;
    }

end:
    return rc;

err:
    /* TODO: rollback partial changes */
    goto end;
}

#if !BDBG_NO_MSG
static const char * const presentationRateControlStrings[] =
{
    "Adaptive",
    "Time stamp",
    "Output clock",
    NULL
};
#endif

static void NEXUS_Astm_P_CancelPendingTimers(NEXUS_AstmHandle astm)
{
    unsigned int i;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
    {
        if (astm->presenterContexts[i].tsmRecoveryAcquisitionTimer)
        {
            NEXUS_CancelTimer(astm->presenterContexts[i].tsmRecoveryAcquisitionTimer);
            astm->presenterContexts[i].tsmRecoveryAcquisitionTimer = NULL;
        }
        if (astm->presenterContexts[i].tsmRecoveryTrackingTimer)
        {
            NEXUS_CancelTimer(astm->presenterContexts[i].tsmRecoveryTrackingTimer);
            astm->presenterContexts[i].tsmRecoveryTrackingTimer = NULL;
        }
    }
}

static NEXUS_Error NEXUS_Astm_P_ApplyPresentationRateControl(NEXUS_AstmHandle astm, NEXUS_AstmPresentationRateControl presentationRateControl)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_VideoDecoderAstmSettings videoDecoderAstmSettings;
    NEXUS_AudioDecoderAstmSettings audioDecoderAstmSettings;
    unsigned int i;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (presentationRateControl == NEXUS_AstmPresentationRateControl_eAdaptive)
    {
        BDBG_ERR(("Cannot apply adaptive presentation rate control"));
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }

    /* switching TSM modes means clearing/restarting TSM recovery tasks */
    NEXUS_Astm_P_CancelPendingTimers(astm);

    if (presentationRateControl == NEXUS_AstmPresentationRateControl_eTimeStamp)
    {
        if (astm->settings.videoDecoder && astm->settings.videoPresenterConfig.manageRateControl)
        {
            NEXUS_Module_Lock(g_astm.settings.modules.videoDecoder);
            NEXUS_VideoDecoder_GetAstmSettings_priv(astm->settings.videoDecoder, &videoDecoderAstmSettings);
            videoDecoderAstmSettings.enableTsm = true
                && astm->presenterContexts[0].firstPtsTsm; /* don't enable TSM if we've disabled it for buffering reasons */
            videoDecoderAstmSettings.ptsOffset = 0;
            rc = NEXUS_VideoDecoder_SetAstmSettings_priv(astm->settings.videoDecoder, &videoDecoderAstmSettings);
            NEXUS_Module_Unlock(g_astm.settings.modules.videoDecoder);
            if (rc) { rc = BERR_TRACE(rc); goto err; }
        }

        NEXUS_Module_Lock(g_astm.settings.modules.audio);
        for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
        {
            if (astm->settings.audioDecoder[i] && astm->settings.audioPresenterConfig[i].manageRateControl)
            {
                NEXUS_AudioDecoder_GetAstmSettings_priv(astm->settings.audioDecoder[i], &audioDecoderAstmSettings);
                audioDecoderAstmSettings.enableTsm = true
                    && astm->presenterContexts[i + 1].firstPtsTsm; /* don't enable TSM if we've disabled it for buffering reasons */;
                audioDecoderAstmSettings.ptsOffset = 0;
                rc = NEXUS_AudioDecoder_SetAstmSettings_priv(astm->settings.audioDecoder[i], &audioDecoderAstmSettings);
                if (rc) break;
            }
        }
        NEXUS_Module_Unlock(g_astm.settings.modules.audio);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
    }
    else if (presentationRateControl == NEXUS_AstmPresentationRateControl_eOutputClock)
    {
        if (astm->settings.videoDecoder && astm->settings.videoPresenterConfig.manageRateControl)
        {
            NEXUS_Module_Lock(g_astm.settings.modules.videoDecoder);
            NEXUS_VideoDecoder_GetAstmSettings_priv(astm->settings.videoDecoder, &videoDecoderAstmSettings);
            videoDecoderAstmSettings.enableTsm = false;
            rc = NEXUS_VideoDecoder_SetAstmSettings_priv(astm->settings.videoDecoder, &videoDecoderAstmSettings);
            NEXUS_Module_Unlock(g_astm.settings.modules.videoDecoder);
            if (rc) { rc = BERR_TRACE(rc); goto err; }

            /* start timer tasks for non-TSM mode recovery */
            BDBG_MSG(("%p: Starting TSM recovery task for presenter %s", (void *)astm, astm->presenterContexts[0].decoderName));
            astm->presenterContexts[0].tsmRecoveryAcquisitionTimer =
                NEXUS_ScheduleTimer(astm->settings.videoPresenterConfig.tsmRecoveryAcquisitionPeriod,
                    &NEXUS_Astm_P_TsmRecoveryAcquisitionTask, &astm->presenterContexts[0]);
        }

        NEXUS_Module_Lock(g_astm.settings.modules.audio);
        for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
        {
            if (astm->settings.audioDecoder[i] && astm->settings.audioPresenterConfig[i].manageRateControl)
            {
                NEXUS_AudioDecoder_GetAstmSettings_priv(astm->settings.audioDecoder[i], &audioDecoderAstmSettings);
                audioDecoderAstmSettings.enableTsm = false;
                rc = NEXUS_AudioDecoder_SetAstmSettings_priv(astm->settings.audioDecoder[i], &audioDecoderAstmSettings);
                if (rc) break;

                /* start timer tasks for non-TSM mode recovery */
                BDBG_MSG(("%p: Starting TSM recovery task for presenter %s", (void *)astm, astm->presenterContexts[i + 1].decoderName));
                astm->presenterContexts[i + 1].tsmRecoveryAcquisitionTimer =
                    NEXUS_ScheduleTimer(astm->settings.audioPresenterConfig[i].tsmRecoveryAcquisitionPeriod,
                        &NEXUS_Astm_P_TsmRecoveryAcquisitionTask, &astm->presenterContexts[i + 1]);
            }
        }
        NEXUS_Module_Unlock(g_astm.settings.modules.audio);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
    }

    if (astm->status.presentationRateControl != presentationRateControl)
    {
        BDBG_MSG(("%p: presentation rate control: %s", (void *)astm, presentationRateControlStrings[presentationRateControl]));
        astm->status.presentationRateControl = presentationRateControl;
    }

end:
    return rc;

err:
    /* TODO: rollback partial changes */
    goto end;
}

#if !(BDBG_NO_LOG && BDBG_NO_MSG)
static const char * const tsmModeStrings[] =
{
    "STC master",
    "video master",
    "audio master",
    "output master",
    NULL
};
#endif

static NEXUS_StcChannelTsmMode NEXUS_Astm_P_ComputeStcChannelTsmMode(
    NEXUS_AstmHandle astm,
    NEXUS_AstmStcSource stcSource,
    void * stcMaster,
    NEXUS_AstmPresentationRateControl presentationRateControl
)
{
    NEXUS_StcChannelTsmMode tsmMode = NEXUS_StcChannelTsmMode_eMax;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if ((presentationRateControl == NEXUS_AstmPresentationRateControl_eTimeStamp
        && stcSource == NEXUS_AstmStcSource_eClockReference))
    {
        tsmMode = NEXUS_StcChannelTsmMode_eStcMaster;
    }
    else if (presentationRateControl == NEXUS_AstmPresentationRateControl_eTimeStamp
        && stcSource == NEXUS_AstmStcSource_ePts)
    {
        if (stcMaster == astm->settings.videoDecoder)
        {
            tsmMode = NEXUS_StcChannelTsmMode_eVideoMaster;
        }
        else if (stcMaster == NULL)
        {
            tsmMode = NEXUS_StcChannelTsmMode_eStcMaster; /* FCFS PTS in playback mode */
        }
        else
        {
            unsigned int i;

            for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
            {
                if (stcMaster == astm->settings.audioDecoder[i])
                {
                    tsmMode = NEXUS_StcChannelTsmMode_eAudioMaster;
                    break;
                }
            }
        }
    }
    else if (presentationRateControl == NEXUS_AstmPresentationRateControl_eOutputClock)
    {
        unsigned int i;
        bool manageVideoRateControl = true;
        bool manageAudioRateControl = true;

        manageVideoRateControl = astm->settings.videoPresenterConfig.manageRateControl;

        for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
        {
            /* if at least 1 audio channel does not allow rate control management, this will be false */
            manageAudioRateControl = manageAudioRateControl &&
                astm->settings.audioPresenterConfig[i].manageRateControl;
        }

        if (manageVideoRateControl && manageAudioRateControl)
        {
            /* video and all audio channels allow rate control management, so output master mode */
            tsmMode = NEXUS_StcChannelTsmMode_eOutputMaster;
        }
        else if (manageVideoRateControl && !manageAudioRateControl)
        {
            /* at least one audio does not allow rate control management, but video allows it, so audio master mode */
            tsmMode = NEXUS_StcChannelTsmMode_eAudioMaster;
        }
        else if (!manageVideoRateControl && manageAudioRateControl)
        {
            /* video does not allow rate control management, but audio allows it, so video master mode */
            tsmMode = NEXUS_StcChannelTsmMode_eVideoMaster;
        }
        else
        {
            /* none of the presenters allow rate control management, leave as invalid Max setting */
            BDBG_MSG(("%p: No presenters allow presentation rate control management", (void *)astm));
        }
    }

    return tsmMode;
}

static NEXUS_Error NEXUS_Astm_P_ApplyStcChannelTsmMode(
    NEXUS_AstmHandle astm,
    NEXUS_StcChannelTsmMode tsmMode
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelAstmSettings stcChannelAstmSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (astm->settings.stcChannel)
    {
        NEXUS_Module_Lock(g_astm.settings.modules.transport);
        NEXUS_StcChannel_GetAstmSettings_priv(astm->settings.stcChannel, &stcChannelAstmSettings);
        stcChannelAstmSettings.tsmMode = tsmMode;
        rc = NEXUS_StcChannel_SetAstmSettings_priv(astm->settings.stcChannel, &stcChannelAstmSettings);
        NEXUS_Module_Unlock(g_astm.settings.modules.transport);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
    }
    else
    {
        BDBG_MSG(("%p: Attempted to apply tsm mode to a NULL stc channel", (void *)astm));
    }

    if (astm->status.tsmMode != tsmMode)
    {
        BDBG_MSG(("%p: tsm mode: %s", (void *)astm, tsmModeStrings[tsmMode]));
        astm->status.tsmMode = tsmMode;
    }

end:
    return rc;

err:
    /* TODO: unroll partial changes */
    goto end;
}

#if !(BDBG_NO_LOG && BDBG_NO_MSG)

static void NEXUS_Astm_P_PrintTimebaseAstmSettings(NEXUS_AstmHandle astm)
{
    NEXUS_TimebaseAstmSettings settings;
    NEXUS_Module_Lock(g_astm.settings.modules.transport);
    NEXUS_Timebase_GetAstmSettings_priv(astm->timebase, &settings);
    NEXUS_Module_Unlock(g_astm.settings.modules.transport);

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    BDBG_LOG(("timebase: astm = %s; clock coupling = %s; pcrReceived = %p; context = %p",
        settings.enabled ? "enabled" : "disabled",
        clockCouplingStrings[settings.clockCoupling],
        (void *)(unsigned long)settings.pcrReceived_isr,
        settings.callbackContext));
}

static const char * const modeStrings[] =
{
    "PCR",
    "Auto",
    "Host",
    NULL
};

static void NEXUS_Astm_P_PrintStcChannelAstmSettings(NEXUS_AstmHandle astm)
{
    NEXUS_StcChannelAstmSettings settings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    NEXUS_Module_Lock(g_astm.settings.modules.transport);
    NEXUS_StcChannel_GetAstmSettings_priv(astm->settings.stcChannel, &settings);
    NEXUS_Module_Unlock(g_astm.settings.modules.transport);

    BDBG_LOG(("stc: astm = %s; mode = %s; tsm mode = %s; sync limit = %u",
        settings.enabled ? "enabled" : "disabled",
        modeStrings[settings.mode],
        tsmModeStrings[settings.tsmMode],
        settings.syncLimit));
}

static void NEXUS_Astm_P_PrintVideoDecoderAstmSettings(NEXUS_AstmHandle astm)
{
    NEXUS_VideoDecoderAstmSettings settings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    NEXUS_Module_Lock(g_astm.settings.modules.videoDecoder);
    NEXUS_VideoDecoder_GetAstmSettings_priv(astm->settings.videoDecoder, &settings);
    NEXUS_Module_Unlock(g_astm.settings.modules.videoDecoder);

    BDBG_LOG(("video settings: astm = %s; sync limit = %u; tsm = %s; playback = %s",
        settings.enableAstm ? "enabled" : "disabled",
        settings.syncLimit,
        settings.enableTsm ? "enabled" : "disabled",
        settings.enablePlayback ? "enabled" : "disabled"));
    BDBG_LOG(("video interface: tsmPass = %p; tsmFail = %p; tsmLog = %p; lifecycle = %p; watchdog = %p; context = %p",
        (void *)(unsigned long)settings.tsmPass_isr,
        (void *)(unsigned long)settings.tsmFail_isr,
        (void *)(unsigned long)settings.tsmLog_isr,
        (void *)(unsigned long)settings.lifecycle_isr,
        (void *)(unsigned long)settings.watchdog_isr,
        settings.callbackContext));
}

static void NEXUS_Astm_P_PrintAudioDecoderAstmSettings(NEXUS_AstmHandle astm, unsigned int index)
{
    NEXUS_AudioDecoderAstmSettings settings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    NEXUS_Module_Lock(g_astm.settings.modules.audio);
    NEXUS_AudioDecoder_GetAstmSettings_priv(astm->settings.audioDecoder[index], &settings);
    NEXUS_Module_Unlock(g_astm.settings.modules.audio);

    BDBG_LOG(("audio %u settings: astm = %s; sync limit = %u; tsm = %s; playback = %s",
        index,
        settings.enableAstm ? "enabled" : "disabled",
        settings.syncLimit,
        settings.enableTsm ? "enabled" : "disabled",
        settings.enablePlayback ? "enabled" : "disabled"));
    BDBG_LOG(("audio %u interface: tsmPass = %p; tsmFail = %p; tsmLog = %p; lifecycle = %p; watchdog = %p; context = %p",
        index,
        (void *)(unsigned long)settings.tsmPass_isr,
        (void *)(unsigned long)settings.tsmFail_isr,
        (void *)(unsigned long)settings.tsmLog_isr,
        (void *)(unsigned long)settings.lifecycle_isr,
        (void *)(unsigned long)settings.watchdog_isr,
        settings.callbackContext));
}

static void NEXUS_Astm_P_PrintDependencyAstmSettings(NEXUS_AstmHandle astm)
{
    unsigned int i;
    bool audio = false;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    BDBG_LOG(("ASTM dependency settings: instance %p", (void *)astm));

    if (astm->timebase)
    {
        NEXUS_Astm_P_PrintTimebaseAstmSettings(astm);
    }
    else
    {
        BDBG_LOG(("timebase: no timebase configured for ASTM"));
    }
    if (astm->settings.stcChannel)
    {
        NEXUS_Astm_P_PrintStcChannelAstmSettings(astm);
    }
    else
    {
        BDBG_LOG(("stc: no STC channel configured for ASTM"));
    }

    if (astm->settings.videoDecoder)
    {
        NEXUS_Astm_P_PrintVideoDecoderAstmSettings(astm);
    }
    else
    {
        BDBG_LOG(("video: no video decoder configured for ASTM"));
    }

    for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
    {
        if (astm->settings.audioDecoder[i])
        {
            NEXUS_Astm_P_PrintAudioDecoderAstmSettings(astm, i);
            audio = true;
        }
    }
    if (!audio)
    {
        BDBG_LOG(("audio: no audio decoders configured for ASTM"));
    }
}

#else

#define NEXUS_Astm_P_PrintTimebaseAstmSettings(astm)
#define NEXUS_Astm_P_PrintStcChannelAstmSettings(astm)
#define NEXUS_Astm_P_PrintVideoDecoderAstmSettings(astm)
#define NEXUS_Astm_P_PrintAudioDecoderAstmSettings(astm, i)
#define NEXUS_Astm_P_PrintDependencyAstmSettings(astm)

#endif

static void * NEXUS_Astm_P_ResolveStcMasterFromMagnum(NEXUS_AstmHandle astm, void * magnumMaster)
{
    void * stcMaster = NULL;

    if (magnumMaster == astm->presenterContexts[0].presenter)
    {
        stcMaster = astm->settings.videoDecoder;
    }
    else
    {
        unsigned int i;

        for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
        {
            if (magnumMaster == astm->presenterContexts[i + 1].presenter)
            {
                stcMaster = astm->settings.audioDecoder[i];
            }
        }
    }

    return stcMaster;
}

static void * NEXUS_Astm_P_ResolveStcMasterToMagnum(NEXUS_AstmHandle astm, const NEXUS_AstmSettings * pSettings)
{
    void * stcMaster = NULL;

    if (pSettings->stcMaster == pSettings->videoDecoder)
    {
        stcMaster = astm->presenterContexts[0].presenter;
    }
    else
    {
        unsigned int i;

        for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
        {
            if (pSettings->stcMaster == pSettings->audioDecoder[i])
            {
                stcMaster = astm->presenterContexts[i + 1].presenter;
                break;
            }
        }
    }

    return stcMaster;
}

static BERR_Code NEXUS_Astmlib_P_PresentationStateChange(void * pvParm1, int iParm2)
{
    NEXUS_AstmHandle astm = (NEXUS_AstmHandle)pvParm1;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_AstmStcSource stcSource;
    NEXUS_AstmPresentationRateControl presentationRateControl;
    NEXUS_StcChannelTsmMode tsmMode;
    void * stcMaster;

    BSTD_UNUSED(iParm2);

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    BASTMlib_GetStatus(astm->lib.handle, &astm->lib.status);

    stcSource = NEXUS_Astm_P_StcSourceFromMagnum(astm->lib.status.eStcSource);
    presentationRateControl = NEXUS_Astm_P_PresentationRateControlFromMagnum(astm->lib.status.ePresentationRateControl);
    stcMaster = NEXUS_Astm_P_ResolveStcMasterFromMagnum(astm, astm->lib.status.hStcMaster);

    /* this will enable/disable TSM in the decoders.
    we want to disable it first, but enable it last */
    if (astm->settings.presentationRateControl == NEXUS_AstmPresentationRateControl_eAdaptive
        && presentationRateControl == NEXUS_AstmPresentationRateControl_eOutputClock)
    {
        rc = NEXUS_Astm_P_ApplyPresentationRateControl(astm, presentationRateControl);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
    }

    /* changing pcrlib tsm mode must happen before we switch between Pcr and Auto,
    so we handle any interrupts accordingly */
    /* if either stc source or presentation rate control are adaptive, it affects
    the stc channel tsm mode */
    if (astm->settings.stcSource == NEXUS_AstmStcSource_eAdaptive
        || astm->settings.presentationRateControl == NEXUS_AstmPresentationRateControl_eAdaptive)
    {
        tsmMode = NEXUS_Astm_P_ComputeStcChannelTsmMode(astm, stcSource,
            stcMaster, presentationRateControl);
        if (tsmMode != NEXUS_StcChannelTsmMode_eMax)
        {
            rc = NEXUS_Astm_P_ApplyStcChannelTsmMode(astm, tsmMode);
            if (rc) { rc = BERR_TRACE(rc); goto err; }
        }
    }

    /* this will enable/disable playback in the decoders and switch pcrlib between PCR and Auto
    this must occur after we've picked which pcrlib mode we want, otherwise Auto may be
    handled incorrectly */
    /* only apply the settings for the ones that are adaptive */
    if (astm->settings.stcSource == NEXUS_AstmStcSource_eAdaptive)
    {
        rc = NEXUS_Astm_P_ApplyStcSource(astm, stcSource, stcMaster, astm->settings.syncLimit);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
    }

    /* this will enable/disable TSM in the decoders.
    we want to disable it first, but enable it last */
    if (astm->settings.presentationRateControl == NEXUS_AstmPresentationRateControl_eAdaptive
        && presentationRateControl == NEXUS_AstmPresentationRateControl_eTimeStamp)
    {
        rc = NEXUS_Astm_P_ApplyPresentationRateControl(astm, presentationRateControl);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
    }

end:
    return rc;

err:
    /* TODO: anything to unroll? */
    goto end;
}

static BERR_Code NEXUS_Astmlib_P_ClockCouplingStateChange(void * pvParm1, int iParm2)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_AstmHandle astm = (NEXUS_AstmHandle)pvParm1;
    NEXUS_AstmClockCoupling clockCoupling;

    BSTD_UNUSED(iParm2);

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    BASTMlib_GetStatus(astm->lib.handle, &astm->lib.status);

    clockCoupling = NEXUS_Astm_P_ClockCouplingFromMagnum(astm->lib.status.eClockCoupling);

    if (astm->settings.clockCoupling == NEXUS_AstmClockCoupling_eAdaptive)
    {
        rc = NEXUS_Astm_P_ApplyClockCoupling(astm, clockCoupling);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
    }

end:
    return rc;

err:
    /* TODO: anything to unroll? */
    goto end;
}

static void NEXUS_Astm_P_GetAudioSettings(NEXUS_AstmHandle astm, void * decoder, struct NEXUS_AstmDecoderSettings * settings)
{
    NEXUS_AudioDecoderAstmSettings audioSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (decoder)
    {
        if (settings)
        {
            NEXUS_AudioDecoder_GetAstmSettings_priv((NEXUS_AudioDecoderHandle)decoder, &audioSettings);
            settings->ptsOffset = audioSettings.ptsOffset;
            settings->tsm = audioSettings.enableTsm;
        }
        else
        {
            BDBG_MSG(("%p: GetAudioSettings NULL settings", (void *)astm));
        }
    }
    else
    {
        BDBG_MSG(("%p: GetAudioSettings NULL decoder handle", (void *)astm));
    }
}

static void NEXUS_Astm_P_SetAudioSettings(NEXUS_AstmHandle astm, void * decoder, const struct NEXUS_AstmDecoderSettings * settings)
{
    NEXUS_AudioDecoderAstmSettings audioSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (decoder)
    {
        if (settings)
        {
            NEXUS_Error rc = NEXUS_SUCCESS;
            /* SW7335-1330: Coverity: 35222, UNINIT */
            /* 20111207 bandrews - can't init with memset, need to preserve callbacks and such from previous settings */
            NEXUS_AudioDecoder_GetAstmSettings_priv((NEXUS_AudioDecoderHandle)decoder, &audioSettings);
            if (audioSettings.ptsOffset != settings->ptsOffset)
            {
                BDBG_MSG(("%p: audio %p ptsOffset = %ld", (void *)astm, (void *)decoder, (long)settings->ptsOffset));
            }
            audioSettings.ptsOffset = settings->ptsOffset;
            audioSettings.enableTsm = settings->tsm;
            rc = NEXUS_AudioDecoder_SetAstmSettings_priv((NEXUS_AudioDecoderHandle)decoder, &audioSettings);
            if (rc) { rc = BERR_TRACE(rc); }
        }
        else
        {
            BDBG_MSG(("%p: SetAudioSettings NULL settings", (void *)astm));
        }
    }
    else
    {
        BDBG_MSG(("%p: SetAudioSettings NULL decoder handle", (void *)astm));
    }
}

static void NEXUS_Astm_P_GetAudioStatus_isr(NEXUS_AstmHandle astm, void * decoder, struct NEXUS_AstmDecoderStatus * status)
{
    NEXUS_AudioDecoderAstmStatus audioStatus;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (decoder)
    {
        NEXUS_AudioDecoder_GetAstmStatus_isr((NEXUS_AudioDecoderHandle)decoder, &audioStatus);
        if (status)
        {
            status->started = audioStatus.started;
            status->pts = audioStatus.pts;
            status->ptsStcDiff = -audioStatus.ptsStcDiff; /* stc - pts is reported */
            status->tsmLog.address = audioStatus.tsmLog.address;
            status->tsmLog.size = audioStatus.tsmLog.size;
        }
    }
    else
    {
        BDBG_MSG(("%p: GetAudioStatus_isr NULL decoder handle", (void *)astm));
    }
}

static void NEXUS_Astm_P_GetAudioStatus(NEXUS_AstmHandle astm, void * decoder, struct NEXUS_AstmDecoderStatus * status)
{
    NEXUS_AudioDecoderStatus audioStatus;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (decoder)
    {
        NEXUS_Error rc = NEXUS_SUCCESS;
        BKNI_EnterCriticalSection();
        NEXUS_Astm_P_GetAudioStatus_isr(astm, decoder, status);
        BKNI_LeaveCriticalSection();
        rc = NEXUS_AudioDecoder_GetStatus((NEXUS_AudioDecoderHandle)decoder, &audioStatus);
        if (rc) { rc = BERR_TRACE(rc); }

        if (status && audioStatus.framesDecoded)
        {
            status->decodedCount = audioStatus.framesDecoded;
        }
    }
    else
    {
        BDBG_MSG(("%p: GetAudioStatus NULL decoder handle", (void *)astm));
    }
}

static void NEXUS_Astm_P_GetVideoSettings(NEXUS_AstmHandle astm, void * decoder, struct NEXUS_AstmDecoderSettings * settings)
{
    NEXUS_VideoDecoderAstmSettings videoSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (decoder)
    {
        if (settings)
        {
            NEXUS_VideoDecoder_GetAstmSettings_priv((NEXUS_VideoDecoderHandle)decoder, &videoSettings);
            settings->ptsOffset = videoSettings.ptsOffset;
            settings->tsm = videoSettings.enableTsm;
        }
        else
        {
            BDBG_MSG(("%p: GetVideoSettings NULL settings", (void *)astm));
        }
    }
    else
    {
        BDBG_MSG(("%p: GetVideoSettings NULL decoder handle", (void *)astm));
    }
}

static void NEXUS_Astm_P_SetVideoSettings(NEXUS_AstmHandle astm, void * decoder, const struct NEXUS_AstmDecoderSettings * settings)
{
    NEXUS_VideoDecoderAstmSettings videoSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (decoder)
    {
        if (settings)
        {
            NEXUS_Error rc = NEXUS_SUCCESS;
            NEXUS_VideoDecoder_GetAstmSettings_priv((NEXUS_VideoDecoderHandle)decoder, &videoSettings);
            if (videoSettings.ptsOffset != settings->ptsOffset)
            {
                BDBG_MSG(("%p: video %p ptsOffset = %d", (void *)astm, decoder, (int)settings->ptsOffset));
            }
            videoSettings.ptsOffset = settings->ptsOffset;
            videoSettings.enableTsm = settings->tsm;
            rc = NEXUS_VideoDecoder_SetAstmSettings_priv((NEXUS_VideoDecoderHandle)decoder, &videoSettings);
            if (rc) { rc = BERR_TRACE(rc); }
        }
        else
        {
            BDBG_MSG(("%p: SetVideoSettings NULL settings", (void *)astm));
        }
    }
    else
    {
        BDBG_MSG(("%p: SetVideoSettings NULL decoder handle", (void *)astm));
    }
}

static void NEXUS_Astm_P_GetVideoStatus_isr(NEXUS_AstmHandle astm, void * decoder, struct NEXUS_AstmDecoderStatus * status)
{
    NEXUS_VideoDecoderAstmStatus videoStatus;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (decoder)
    {
        NEXUS_VideoDecoder_GetAstmStatus_isr((NEXUS_VideoDecoderHandle)decoder, &videoStatus);
        if (status)
        {
            status->started = videoStatus.started;
            status->pts = videoStatus.pts;
            status->tsmLog.address = videoStatus.tsmLog.address;
            status->tsmLog.size = videoStatus.tsmLog.size;
        }
    }
    else
    {
        BDBG_MSG(("%p: GetVideoStatus_isr NULL decoder handle", (void *)astm));
    }
}

static void NEXUS_Astm_P_GetVideoStatus(NEXUS_AstmHandle astm, void * decoder, struct NEXUS_AstmDecoderStatus * status)
{
    NEXUS_VideoDecoderStatus videoStatus;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (decoder)
    {
        NEXUS_Error rc = NEXUS_SUCCESS;
        BKNI_EnterCriticalSection();
        NEXUS_Astm_P_GetVideoStatus_isr(astm, decoder, status);
        BKNI_LeaveCriticalSection();
        rc = NEXUS_VideoDecoder_GetStatus((NEXUS_VideoDecoderHandle)decoder, &videoStatus);
        if (rc) { rc = BERR_TRACE(rc); }

        if (status)
        {
            status->decodedCount = videoStatus.numDecoded;
            status->ptsStcDiff = videoStatus.ptsStcDifference;
        }
    }
    else
    {
        BDBG_MSG(("%p: GetVideoStatus NULL decoder handle", (void *)astm));
    }
}

static void NEXUS_Astm_P_TsmCallback_isr(struct NEXUS_AstmContext * astmContext, bool pass)
{
    struct NEXUS_AstmDecoderStatus decoderStatus;
    BASTMlib_Presenter_Event event;

    BKNI_Memset(&decoderStatus, 0, sizeof(struct NEXUS_AstmDecoderStatus));
    BKNI_Memset(&event, 0, sizeof(BASTMlib_Presenter_Event));

    if (astmContext)
    {
        NEXUS_OBJECT_ASSERT(NEXUS_Astm, astmContext->astm);

        if (astmContext->astm->ready && astmContext->astm->started)
        {
            if (astmContext->getStatus_isr)
            {
                astmContext->getStatus_isr(astmContext->astm, astmContext->decoder, &decoderStatus);
                event.uiPts = decoderStatus.pts;
            }
            else
            {
                BDBG_MSG(("%p: %s TSM callback NULL getStatus_isr function pointer", (void *)astmContext->astm, astmContext->decoderName));
            }

            if (astmContext->stc)
            {
                NEXUS_StcChannel_GetStc_isr(astmContext->stc, &event.uiStc);
            }
            else
            {
                BDBG_MSG(("%p: %s TSM callback NULL STC channel handle", (void *)astmContext->astm, astmContext->decoderName));
            }

            event.bPass = pass;

            BDBG_MSG_TRACE(("%p: %s TSM %s callback pts=%#x stc=%#x", (void *)astmContext->astm, astmContext->decoderName, pass ? "PASS" : "FAIL", event.uiPts, event.uiStc));

            if (astmContext->presenter)
            {
                BASTMlib_Presenter_EventHandler_isr(astmContext->presenter, &event);
            }
            else
            {
                BDBG_MSG(("%p: %s TSM callback NULL presenter handle", (void *)astmContext->astm, astmContext->decoderName));
            }
        }
        else
        {
            BDBG_MSG_TRACE(("%p: %s TSM callback (ASTM not ready)", astmContext->astm, astmContext->decoderName));
        }
    }
    else
    {
        BDBG_MSG(("%d: TSM callback NULL context", __LINE__));
    }
}

static void NEXUS_Astm_P_DecoderFirstPtsEventHandler(void * context)
{
    NEXUS_AstmHandle astm;
    struct NEXUS_AstmContext * astmContext = (struct NEXUS_AstmContext * )context;
    NEXUS_StcChannelSettings stcChannelSettings;
    struct NEXUS_AstmDecoderSettings settings;

    if (astmContext)
    {
        /* this event only happens when the first pts/pcr diff is outside the allowable range */
        astm = astmContext->astm;

        if (!astm)
        {
            goto spurious;
        }

        NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

        if (!astm->settings.stcChannel)
        {
            goto spurious;
        }

        NEXUS_StcChannel_GetSettings(astm->settings.stcChannel, &stcChannelSettings);

        if (stcChannelSettings.mode == NEXUS_StcChannelMode_ePcr)
        {
            if (astm->usePtsMasterMode)
            {
                NEXUS_StcChannelTsmMode tsmMode;
                BDBG_MSG(("%p: First PTS event occurred in live mode, switching to PTS master mode", (void *)astm));
                NEXUS_Astm_P_ApplyStcSource(astm, NEXUS_AstmStcSource_ePts,
                    astm->settings.stcMaster, astm->settings.syncLimit);
                tsmMode = NEXUS_Astm_P_ComputeStcChannelTsmMode(astm,
                    NEXUS_AstmStcSource_ePts, astm->settings.stcMaster,
                    NEXUS_AstmPresentationRateControl_eTimeStamp);
                NEXUS_Astm_P_ApplyStcChannelTsmMode(astm, tsmMode);
            }
            else
            {
                /* TODO: should manageRateControl apply to this? */
                BDBG_MSG(("%p: First PTS event occurred in live mode, disabling TSM on offending decoder", (void *)astm));
                if (astmContext->getSettings && astmContext->setSettings)
                {
                    astmContext->firstPtsTsm = false; /* save for later */
                    NEXUS_Module_Lock(astmContext->module);
                    astmContext->getSettings(astm, astmContext->decoder, &settings);
                    settings.tsm = false;
                    settings.ptsOffset = 0;
                    astmContext->setSettings(astm, astmContext->decoder, &settings);
                    NEXUS_Module_Unlock(astmContext->module);
                }
            }
        }
        else
        {
            BDBG_MSG(("%p: First PTS event occurred in playback mode, ignored", (void *)astm));
        }
    }
    else
    {
        BDBG_MSG(("%d: decoder first pts event handler NULL context", __LINE__));
    }

end:
    return;

spurious:
    BDBG_MSG(("%p spurious decoder first PTS event", (void*)astmContext));
    goto end;
}

static bool NEXUS_Astm_P_TimestampDiffCheck_isr(
    NEXUS_AstmHandle astm,
    uint32_t firstTimestamp,
    uint32_t secondTimestamp,
    uint32_t positiveThreshold,
    uint32_t negativeThreshold)
{
    bool fail = false;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    BDBG_MSG(("%p: first timestamp: %#x; second timestamp: %#x; allowable diff range: (-%u, %u)", (void *)astm,
        firstTimestamp, secondTimestamp, negativeThreshold, positiveThreshold));

    if (firstTimestamp > secondTimestamp)
    {
        if (firstTimestamp - secondTimestamp > positiveThreshold)
        {
            fail = true;
        }
    }
    else
    {
        if (secondTimestamp - firstTimestamp > negativeThreshold)
        {
            fail = true;
        }
    }

    return fail;
}

static bool NEXUS_Astm_P_CheckFirstPtsReceived_isr(NEXUS_AstmHandle astm)
{
    unsigned i;
    bool allFirstPtsReceived = true;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
    {
        struct NEXUS_AstmContext * astmContext = &astm->presenterContexts[i];
        allFirstPtsReceived = allFirstPtsReceived
            && (astmContext->firstPtsReceived || !astmContext->maxAllowableFirstPtsStcDiff);
    }

    return allFirstPtsReceived;
}

static void NEXUS_Astm_P_PropagatePcr_isr(NEXUS_AstmHandle astm)
{
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
    {
        struct NEXUS_AstmContext * astmContext = &astm->presenterContexts[i];
        if (!astmContext->firstPtsArrivalPcrValid && astmContext->firstPtsReceived)
        {
            astmContext->firstPtsArrivalPcr = astm->lastPcr;
            astmContext->firstPtsArrivalPcrValid = true;
        }
    }
}

#define MAX_PCR_SPACING_45K 100 * 45 /* 100 ms per MPEG spec */

static bool NEXUS_Astm_P_CheckCompatibleTimestamps_isr(NEXUS_AstmHandle astm)
{
    unsigned i;
    bool compatiblePts = true;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
    {
        struct NEXUS_AstmContext * astmContext = &astm->presenterContexts[i];
        if (astmContext->maxAllowableFirstPtsStcDiff)
        {
            compatiblePts = compatiblePts
                && !NEXUS_Astm_P_TimestampDiffCheck_isr(astm,
                        astmContext->firstPts, astmContext->firstPtsArrivalPcr, astmContext->maxAllowableFirstPtsStcDiff, 2 * MAX_PCR_SPACING_45K); /* safety factor of 2 used in case callbacks are delayed */
        }
    }

    return compatiblePts;
}

static bool NEXUS_Astm_P_CheckMasterSuitability_isr(NEXUS_AstmHandle astm, uint32_t master)
{
    unsigned i;
    bool compatiblePts = true;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
    {
        struct NEXUS_AstmContext * astmContext = &astm->presenterContexts[i];
        if (astmContext->maxAllowableFirstPtsStcDiff)
        {
            compatiblePts = compatiblePts
                && !NEXUS_Astm_P_TimestampDiffCheck_isr(astm,
                        astmContext->firstPts, master, astmContext->maxAllowableFirstPtsStcDiff, 0);
        }
    }

    return compatiblePts;
}

static uint32_t NEXUS_Astm_P_FindMasterTimestamp_isr(NEXUS_AstmHandle astm)
{
    unsigned i;
    uint32_t master = 0;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (astm->settings.stcMaster)
    {
        /* just take the one who is the master */
        for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
        {
            struct NEXUS_AstmContext * astmContext = &astm->presenterContexts[i];
            if (astmContext->decoder == astm->settings.stcMaster)
            {
                master = astmContext->firstPts;
                break;
            }
        }
    }
    else
    {
        /* get the min */
        /* TODO: does not handle wraparound */
        master = 0xFFFFFFFF;
        for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
        {
            struct NEXUS_AstmContext * astmContext = &astm->presenterContexts[i];
            if (master < astmContext->firstPts)
            {
                master = astmContext->firstPts;
            }
        }
    }

    return master;
}

static void NEXUS_Astm_P_SetFirstPtsEvents_isr(NEXUS_AstmHandle astm)
{
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    /* we set the event for all decoders because some of them may need to vsync themselves
    if we didn't want to do pts master mode, and setting pts master mode N times won't hurt */
    for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
    {
        struct NEXUS_AstmContext * astmContext = &astm->presenterContexts[i];
        BKNI_SetEvent(astmContext->firstPtsEvent);
    }
}

static void NEXUS_Astm_P_HandleTimestampCompatibility_isr(NEXUS_AstmHandle astm)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    /* check pcr and all contexts for timestamp receipt */
    if (astm->pcrReceived && NEXUS_Astm_P_CheckFirstPtsReceived_isr(astm))
    {
        BDBG_MSG(("%p: PCR and all first PTSs received; checking compatibility", (void *)astm));

        /* if any decoder pts is incompatible with its arrival PCR */
        if (!NEXUS_Astm_P_CheckCompatibleTimestamps_isr(astm))
        {
            uint32_t master = NEXUS_Astm_P_FindMasterTimestamp_isr(astm);
            BDBG_MSG(("%p: PCR/PTS incompatibility detected", (void *)astm));
            astm->usePtsMasterMode = NEXUS_Astm_P_CheckMasterSuitability_isr(astm, master);
            NEXUS_Astm_P_SetFirstPtsEvents_isr(astm);
        }
    }
}

static void NEXUS_Astm_P_DecoderFirstPtsCallback_isr(void * context, int param)
{
    NEXUS_AstmHandle astm = NULL;
    struct NEXUS_AstmContext * astmContext = (struct NEXUS_AstmContext * )context;
    struct NEXUS_AstmDecoderStatus status;

    BSTD_UNUSED(param);

    if (astmContext)
    {
        astm = astmContext->astm;

        NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

        BDBG_MSG(("%d:%p: %s decoder first PTS callback", __LINE__, (void *)astm, astmContext->decoderName));

        if (astmContext->getStatus_isr)
        {
            /* save data for this context */
            astmContext->getStatus_isr(astm, astmContext->decoder, &status);
            astmContext->firstPts = status.pts;
            astmContext->firstPtsReceived = true;
            if (astm->pcrReceived)
            {
                /* if we already saw a PCR, copy it for this decoder */
                astmContext->firstPtsArrivalPcr = astm->lastPcr; /* this will copy the most recent PCR */
                astmContext->firstPtsArrivalPcrValid = true;
            }

            NEXUS_Astm_P_HandleTimestampCompatibility_isr(astm);
        }
        else
        {
            BDBG_MSG(("%d:%p: decoder first PTS callback NULL getStatus_isr", __LINE__, (void *)astm));
        }
    }
    else
    {
        BDBG_MSG(("%d: decoder first PTS callback NULL context", __LINE__));
    }
}

static void NEXUS_Astm_P_DecoderTsmPassEventHandler(void * context)
{
    struct NEXUS_AstmContext * astmContext = (struct NEXUS_AstmContext * )context;

    if (astmContext)
    {
        if (!astmContext->astm)
        {
            goto spurious;
        }

        NEXUS_OBJECT_ASSERT(NEXUS_Astm, astmContext->astm);

        /* if acq timer exists (it should) cancel it */
        if (astmContext->tsmRecoveryAcquisitionTimer)
        {
            NEXUS_CancelTimer(astmContext->tsmRecoveryAcquisitionTimer);
            astmContext->tsmRecoveryAcquisitionTimer = NULL;
        }

        if (!astmContext->tsmRecoveryTrackingTimer)
        {
            struct NEXUS_AstmDecoderStatus status;

            if (astmContext->getStatus)
            {
                astmContext->getStatus(astmContext->astm, astmContext->decoder, &status);
                astmContext->lastDecodedCount = status.decodedCount;
            }

            /* schedule track timer */
            astmContext->tsmRecoveryTrackingTimer =
                NEXUS_ScheduleTimer(astmContext->tsmRecoveryTrackingTimeout,
                    &NEXUS_Astm_P_TsmRecoveryTrackingTimeoutHandler, astmContext);
        }
    }
    else
    {
        BDBG_MSG(("%d: decoder TSM pass event handler NULL context", __LINE__));
    }

end:
    return;

spurious:
    BDBG_MSG(("%p spurious decoder TSM pass event", (void*)astmContext));
    goto end;
}

static void NEXUS_Astm_P_DecoderTsmPassCallback_isr(void * context, int param)
{
    struct NEXUS_AstmContext * astmContext = (struct NEXUS_AstmContext * )context;

    BSTD_UNUSED(param);

    if (astmContext)
    {
        NEXUS_OBJECT_ASSERT(NEXUS_Astm, astmContext->astm);

        BDBG_MSG(("%d:%p: %s decoder TSM pass callback", __LINE__, (void *)astmContext->astm, astmContext->decoderName));
        /* count the pass event */
        astmContext->passEventCount++;

        BKNI_SetEvent(astmContext->tsmPassEvent);
    }
    else
    {
        BDBG_MSG(("%d: decoder TSM pass callback NULL context", __LINE__));
    }
}

static void NEXUS_Astm_P_DecoderTsmFailCallback_isr(void * context, int param)
{
    struct NEXUS_AstmContext * astmContext = (struct NEXUS_AstmContext * )context;

    BSTD_UNUSED(param);

    NEXUS_Astm_P_TsmCallback_isr(astmContext, false);
}

static void NEXUS_Astm_P_DecoderTsmLogCallback_isr(void * context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    /* TODO */
}

static void NEXUS_Astm_P_DecoderWatchdogCallback_isr(void * context, int param)
{
    struct NEXUS_AstmContext * astmContext = (struct NEXUS_AstmContext * )context;

    BSTD_UNUSED(param);

    if (astmContext)
    {
        NEXUS_OBJECT_ASSERT(NEXUS_Astm, astmContext->astm);

        BDBG_ERR(("%p: %s watchdog callback", (void *)astmContext->astm, astmContext->decoderName));

        BKNI_SetEvent(astmContext->watchdogEvent);
    }
    else
    {
        BDBG_MSG(("%d: decoder watchdog callback NULL context", __LINE__));
    }
}

static void NEXUS_Astm_P_DecoderWatchdogEventHandler(void * context)
{
    struct NEXUS_AstmContext * astmContext = (struct NEXUS_AstmContext * )context;

    if (astmContext)
    {
        if (!astmContext->astm)
        {
            goto spurious;
        }

        NEXUS_OBJECT_ASSERT(NEXUS_Astm, astmContext->astm);

        /* if started, restart */
        if (astmContext->astm->started)
        {
            BDBG_ERR(("%p: Auto-restarting", (void *)astmContext->astm));
            NEXUS_Astm_P_Stop(astmContext->astm);
            NEXUS_Astm_P_Start(astmContext->astm);
        }
    }
    else
    {
        BDBG_MSG(("%d: decoder watchdog event handler NULL context", __LINE__));
    }

end:
    return;

spurious:
    BDBG_MSG(("%p spurious decoder watchdog event", (void*)astmContext));
    goto end;
}

static void NEXUS_Astm_P_DecoderLifecycleCallback_isr(void * context, int param)
{
    struct NEXUS_AstmDecoderStatus decoderStatus;
    struct NEXUS_AstmContext * astmContext = (struct NEXUS_AstmContext * )context;

    BSTD_UNUSED(param);

    BKNI_Memset(&decoderStatus, 0, sizeof(struct NEXUS_AstmDecoderStatus));

    if (astmContext)
    {
        NEXUS_OBJECT_ASSERT(NEXUS_Astm, astmContext->astm);

        if (astmContext->getStatus_isr)
        {
            astmContext->getStatus_isr(astmContext->astm, astmContext->decoder, &decoderStatus);
            astmContext->started = decoderStatus.started;
            astmContext->firstPtsReceived = false;
            astmContext->firstPtsArrivalPcrValid = false;
            astmContext->firstPtsTsm = true;

            BDBG_MSG(("%p: %s lifecycle callback: %s", (void *)astmContext->astm,
                astmContext->decoderName, astmContext->started ? "started" : "stopped"));

            BKNI_SetEvent(astmContext->lifecycleEvent);
        }
        else
        {
            BDBG_MSG(("%p: %s lifecycle callback NULL getStatus_isr function pointer", (void *)astmContext->astm, astmContext->decoderName));
        }
    }
    else
    {
        BDBG_MSG(("%d: decoder lifecycle callback NULL context", __LINE__));
    }
}

static void NEXUS_Astm_P_SetConfigFromStcChannelMode(NEXUS_AstmHandle astm)
{
    BASTMlib_Config astmlibConfig;
    NEXUS_StcChannelSettings stcChannelSettings;

    NEXUS_StcChannel_GetSettings(astm->settings.stcChannel, &stcChannelSettings);

    if (stcChannelSettings.mode == NEXUS_StcChannelMode_ePcr)
    {
        /* assumes pcr's should be available from xpt hw */
        BDBG_MSG(("%p: STC channel in PCR mode, assuming PCRs available", (void *)astm));
        BASTMlib_GetConfig(astm->lib.handle, &astmlibConfig);
        astmlibConfig.sPresentation.ePreferredStcSource = BASTMlib_StcSource_eClockReference;
        astmlibConfig.sClockCoupling.ePreferredClockCoupling = BASTMlib_ClockCoupling_eInputClock;
        BASTMlib_SetConfig(astm->lib.handle, &astmlibConfig);
    }
    else
    {
        /* assumes pcr's should not be available from xpt hw */
        BDBG_MSG(("%p: STC channel in auto/host mode, assuming PCRs unavailable", (void *)astm));
        BASTMlib_GetConfig(astm->lib.handle, &astmlibConfig);
        astmlibConfig.sPresentation.ePreferredStcSource = BASTMlib_StcSource_ePresenter;
        astmlibConfig.sClockCoupling.ePreferredClockCoupling = BASTMlib_ClockCoupling_eInternalClock;
        BASTMlib_SetConfig(astm->lib.handle, &astmlibConfig);
    }
    /* TODO: need to reapply current ASTM settings to decoder on restart */

#if NEXUS_ASTM_P_PRINT_SETTINGS
    NEXUS_Astm_P_PrintDependencyAstmSettings(astmContext->astm);
#endif
}

static void NEXUS_Astm_P_DecoderLifecycleEventHandler(void * context)
{
    struct NEXUS_AstmContext * astmContext = (struct NEXUS_AstmContext * )context;

    if (astmContext)
    {
        if (!astmContext->astm)
        {
            goto spurious;
        }

        NEXUS_OBJECT_ASSERT(NEXUS_Astm, astmContext->astm);

        if (!astmContext->astm->settings.stcChannel)
        {
            goto spurious;
        }

        /* if a decoder is being started, stc channel state should be valid
        for this session */
        /* PR:49215 playback support */
        if (astmContext->started)
        {
            NEXUS_Astm_P_SetConfigFromStcChannelMode(astmContext->astm);
        }

        /* PR50938 add ability to disable ALC */
        if (astmContext->astm->settings.enableAutomaticLifecycleControl)
        {
            if (astmContext->started)
            {
                /* if one presenter is started, start ASTM */
                if (!astmContext->astm->started)
                {
                    BDBG_MSG(("%p: Auto-starting", (void *)astmContext->astm));
                    NEXUS_Astm_P_Start(astmContext->astm);
                }
            }
            else /* this presenter was just stopped, check others */
            {
                unsigned int i = 0;
                for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
                {
                    if (astmContext->astm->presenterContexts[i].started)
                    {
                        break;
                    }
                }

                /* all presenters are stopped */
                if (i == NEXUS_ASTM_PRESENTERS)
                {
                    BDBG_MSG(("%p: Auto-stopping", (void *)astmContext->astm));
                    NEXUS_Astm_P_Stop(astmContext->astm);
                }
            }
        }
    }
    else
    {
        BDBG_MSG(("%d: decoder lifecycle event handler NULL context", __LINE__));
    }

end:
    return;

spurious:
    BDBG_MSG(("%p spurious decoder lifecycle event", (void*)astmContext));
    goto end;
}

static void NEXUS_Astm_P_TimebasePcrReceivedCallback_isr(void * context, int param)
{
    NEXUS_AstmHandle astm = (NEXUS_AstmHandle)context;
    NEXUS_TimebaseStatus status;
    BASTMlib_ClockReference_Event sEvent;

    BSTD_UNUSED(param);
    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    BKNI_Memset(&sEvent, 0, sizeof(BASTMlib_ClockReference_Event));

    if (astm->ready && astm->started)
    {
        NEXUS_Timebase_GetStatus_priv_isr(astm->timebase, &status);
        sEvent.uiClockReference = status.lastValue;

        if (astm->lib.status.eClockCoupling == BASTMlib_ClockCoupling_eInputClock)
        {
            /* if input clock, then error is between stc and pcr */
            sEvent.uiStc = status.lastValue - status.lastError; /* error saturates
                at 10 bits (instead of 32), but for our purposes this is okay */
        }
        else
        {
            /* if internal, then error is between stc and xtal, so need to just
            read STC.  This will be sensitive to high latency environments, but
            the worst case in high latency situation is that we measure bad
            PCR/STC jitter and stay in the state we are already in */
            NEXUS_StcChannel_GetStc_isr(astm->settings.stcChannel, &sEvent.uiStc);
        }

        BDBG_MSG_TRACE(("%p: TimebasePcrReceived pcr=%#x stc=%#x", astm, sEvent.uiClockReference, sEvent.uiStc));

        BASTMlib_ClockReferenceEventHandler_isr(astm->lib.handle, &sEvent);

        astm->lastPcr = status.lastValue;

        if (!astm->pcrReceived) /* just check the first PCR */
        {
            astm->pcrReceived = true;

            /* propagate the first PCR to any decoder contexts that had FirstPTS before the PCR arrived */
            NEXUS_Astm_P_PropagatePcr_isr(astm);

            NEXUS_Astm_P_HandleTimestampCompatibility_isr(astm);
        }
    }
    else
    {
        BDBG_MSG_TRACE(("%p: TimebasePcrReceived (ASTM not ready)", astm));
    }
}

static void NEXUS_Astm_P_TsmRecoveryTrackingTimeoutHandler(void * context)
{
    struct NEXUS_AstmContext * astmContext = context;
    struct NEXUS_AstmDecoderStatus status;

    if (astmContext)
    {
        if (!astmContext->astm)
        {
            goto spurious;
        }

        NEXUS_OBJECT_ASSERT(NEXUS_Astm, astmContext->astm);

        astmContext->tsmRecoveryTrackingTimer = NULL;

        if (astmContext->getStatus)
        {
            astmContext->getStatus(astmContext->astm, astmContext->decoder, &status);

            /* TODO: may need a threshold here */
            BDBG_MSG(("%d:%p: tsm pass count (%u); decoded count (%u)", __LINE__, (void *)astmContext->astm,
                astmContext->passEventCount, status.decodedCount - astmContext->lastDecodedCount));
            if (status.decodedCount && (status.decodedCount <= astmContext->passEventCount + astmContext->lastDecodedCount))
            {
                BKNI_EnterCriticalSection();
                NEXUS_Astm_P_TsmCallback_isr(astmContext, true);
                BKNI_LeaveCriticalSection();
            }
            else
            {
                BDBG_MSG(("%d:%p: reverting to acquisition stage", __LINE__, (void *)astmContext->astm));

                /* revert to acquisition */
                astmContext->tsmRecoveryAcquisitionTimer =
                    NEXUS_ScheduleTimer(astmContext->tsmRecoveryAcquisitionPeriod,
                        &NEXUS_Astm_P_TsmRecoveryAcquisitionTask, astmContext);
            }

            astmContext->lastDecodedCount = status.decodedCount;
            astmContext->passEventCount = 0;
        }
        else
        {
            BDBG_MSG(("%d:%p: decoder status accessor was NULL", __LINE__, (void *)astmContext->astm));
        }
    }
    else
    {
        BDBG_MSG(("%d: tsm recovery tracking timeout handler NULL context", __LINE__));
    }

end:
    return;

spurious:
    BDBG_MSG(("%p spurious TSM recovery tracking timeout", (void*)astmContext));
    goto end;
}

static void NEXUS_Astm_P_TsmRecoveryAcquisitionTask(void * context)
{
    struct NEXUS_AstmContext * astmContext = context;
    struct NEXUS_AstmDecoderSettings settings;
    struct NEXUS_AstmDecoderStatus status;

    if (astmContext)
    {
        if (!astmContext->astm)
        {
            goto spurious;
        }

        NEXUS_OBJECT_ASSERT(NEXUS_Astm, astmContext->astm);

        astmContext->tsmRecoveryAcquisitionTimer = NULL;

        /* if we've disabled TSM for first PTS reasons, don't bother coming back */
        if (astmContext->firstPtsTsm)
        {
            if (astmContext->getStatus)
            {
                astmContext->getStatus(astmContext->astm, astmContext->decoder, &status);
                astmContext->lastDecodedCount = status.decodedCount;

                if (astmContext->getSettings && astmContext->setSettings)
                {
                    BDBG_MSG(("%d:%p: %s pts/stc diff = %d", __LINE__, (void *)astmContext->astm,
                        astmContext->decoderName, (int)status.ptsStcDiff));

                    if (status.ptsStcDiff > (int32_t)astmContext->ptsStcDiffAdjustmentThreshold
                        || status.ptsStcDiff < -(int32_t)astmContext->ptsStcDiffAdjustmentThreshold)
                    {
                        NEXUS_Module_Lock(astmContext->module);
                        astmContext->getSettings(astmContext->astm, astmContext->decoder, &settings);
                        settings.ptsOffset += -status.ptsStcDiff;
                        astmContext->setSettings(astmContext->astm, astmContext->decoder, &settings);
                        NEXUS_Module_Unlock(astmContext->module);
                    }
                    else
                    {
                        BDBG_MSG(("%d:%p: %s pts/stc diff within tolerance", __LINE__, (void *)astmContext->astm, astmContext->decoderName));
                    }
                }
                else
                {
                    BDBG_MSG(("%d:%p: %s decoder settings mutator was NULL", __LINE__, (void *)astmContext->astm, astmContext->decoderName));
                }
            }
            else
            {
                BDBG_MSG(("%d:%p: %s decoder status accessor was NULL", __LINE__, (void *)astmContext->astm, astmContext->decoderName));
            }

            /* schedule the next recovery task acq instant */
            astmContext->tsmRecoveryAcquisitionTimer =
                NEXUS_ScheduleTimer(astmContext->tsmRecoveryAcquisitionPeriod,
                    &NEXUS_Astm_P_TsmRecoveryAcquisitionTask, astmContext);
        }
    }
    else
    {
        BDBG_MSG(("%d: tsm recovery acquisition timeout handler NULL context", __LINE__));
    }

end:
    return;

spurious:
    BDBG_MSG(("%p spurious TSM recovery acquisition timeout", (void*)astmContext));
    goto end;
}

static void NEXUS_Astm_P_PcrDataPresenceDetector(void * context, bool * dataPresent)
{
    NEXUS_AstmHandle astm = context;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    *dataPresent = false;

    NEXUS_Module_Lock(g_astm.settings.modules.transport);
    if (astm->timebase)
    {
        *dataPresent = NEXUS_Timebase_IsDataPresent_priv(astm->timebase);
    }
    NEXUS_Module_Unlock(g_astm.settings.modules.transport);
}

/**********************
*
* Main functions
*
***********************/

NEXUS_AstmHandle NEXUS_Astm_Create(const NEXUS_AstmSettings *pSettings)
{
    BASTMlib_Settings settings;
    NEXUS_AstmHandle astm;
    BERR_Code rc;
    unsigned int i = 0;
    BASTMlib_Presenter_Settings presenterSettings;

    astm = BKNI_Malloc(sizeof(*astm));
    if (!astm) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_Astm, astm);
    NEXUS_Astm_GetDefaultSettings(&astm->settings);
    astm->timebase = NULL;

    astm->status.clockCoupling = NEXUS_AstmClockCoupling_eMax;
    astm->status.stcSource = NEXUS_AstmStcSource_eMax;
    astm->status.presentationRateControl = NEXUS_AstmPresentationRateControl_eMax;
    astm->status.tsmMode = NEXUS_StcChannelTsmMode_eMax;

    BKNI_EnterCriticalSection();
    astm->ready = false;
    BKNI_LeaveCriticalSection();

    BASTMlib_GetDefaultSettings(&settings);

    /* map syslib/framework functions to base */
    settings.cbTimer.pfCreate = NEXUS_SYSlib_P_CreateTimer;
    settings.cbTimer.pfDestroy = NEXUS_SYSlib_P_DestroyTimer;
    settings.cbTimer.pfStart_isr = NEXUS_SYSlib_P_StartTimer_isr;
    settings.cbTimer.pfCancel_isr = NEXUS_SYSlib_P_CancelTimer_isr;
    settings.cbTimer.pvParm1 = g_astm.syslibContext;
    settings.cbTimer.iParm2 = 0; /* unused */

    /* ASTMlib state change callbacks. They are all task time. You cannot make any change to ASTMlib during these callbacks. */
    settings.sClockCoupling.cbStateChange.pfDo = &NEXUS_Astmlib_P_ClockCouplingStateChange;
    settings.sClockCoupling.cbStateChange.pvParm1 = astm;
    settings.sClockCoupling.cbStateChange.iParm2 = 0; /* unused */
    settings.sPresentation.cbStateChange.pfDo = &NEXUS_Astmlib_P_PresentationStateChange;
    settings.sPresentation.cbStateChange.pvParm1 = astm;
    settings.sPresentation.cbStateChange.iParm2 = 0; /* unused */

    rc = BASTMlib_Create(&astm->lib.handle, &settings);
    if (rc) {BERR_TRACE(rc); goto error;}

    BASTMlib_Presenter_GetDefaultSettings(&presenterSettings);

    for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
    {
        BASTMlib_Presenter_Create(astm->lib.handle, &astm->presenterContexts[i].presenter, &presenterSettings);
        BKNI_CreateEvent(&astm->presenterContexts[i].watchdogEvent);
        BKNI_CreateEvent(&astm->presenterContexts[i].lifecycleEvent);
        BKNI_CreateEvent(&astm->presenterContexts[i].tsmPassEvent);
        BKNI_CreateEvent(&astm->presenterContexts[i].firstPtsEvent);
    }

    BASTMlib_GetStatus(astm->lib.handle, &astm->lib.status);

    rc = NEXUS_Astm_SetSettings(astm, pSettings);
    if (rc) {BERR_TRACE(rc); goto error;}

    BKNI_EnterCriticalSection();
    astm->ready = true;
    BKNI_LeaveCriticalSection();

    BLST_Q_INSERT_TAIL(&g_astm.contexts, astm, link);

    return astm;

error:
    NEXUS_Astm_Destroy(astm);
    return NULL;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Astm, NEXUS_Astm_Destroy);

static void NEXUS_Astm_P_Finalizer(NEXUS_AstmHandle astm)
{
    unsigned int i;
    NEXUS_AstmSettings settings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    NEXUS_Astm_GetSettings(astm, &settings);
    settings.videoDecoder = NULL;
    settings.stcChannel = NULL;
    settings.stcMaster = NULL;
    for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
    {
        settings.audioDecoder[i] = NULL;
    }
    NEXUS_Astm_SetSettings(astm, &settings);

    /* destroy presenters */
    for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
    {
        if (astm->presenterContexts[i].firstPtsEvent)
        {
            BKNI_DestroyEvent(astm->presenterContexts[i].firstPtsEvent);
        }
        if (astm->presenterContexts[i].tsmPassEvent)
        {
            BKNI_DestroyEvent(astm->presenterContexts[i].tsmPassEvent);
        }
        if (astm->presenterContexts[i].watchdogEvent)
        {
            BKNI_DestroyEvent(astm->presenterContexts[i].watchdogEvent);
        }
        if (astm->presenterContexts[i].lifecycleEvent)
        {
            BKNI_DestroyEvent(astm->presenterContexts[i].lifecycleEvent);
        }
        if (astm->presenterContexts[i].presenter)
        {
            BASTMlib_Presenter_Destroy(astm->presenterContexts[i].presenter);
        }
    }

    BASTMlib_Destroy(astm->lib.handle);

    BLST_Q_REMOVE(&g_astm.contexts, astm, link);
    NEXUS_OBJECT_DESTROY(NEXUS_Astm, astm);
    BKNI_Free(astm);
}

void NEXUS_Astm_GetSettings(NEXUS_AstmHandle astm, NEXUS_AstmSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);
    *pSettings = astm->settings;
}

#ifdef BDBG_DEBUG_BUILD
static const char * const decoderNameStrings[] =
{
    "Video",
    "Audio 0",
    "Audio 1",
    NULL
};
#endif

static void NEXUS_Astm_P_DisconnectVideoDecoder(NEXUS_AstmHandle astm)
{
    NEXUS_VideoDecoderAstmSettings astmSettings;
#ifdef BDBG_DEBUG_BUILD
    BASTMlib_Presenter_Settings presenterSettings;
#endif
    struct NEXUS_AstmContext * presenterContext;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    presenterContext = &astm->presenterContexts[0];

    NEXUS_UnregisterEvent(presenterContext->lifecycleEventHandler);
    NEXUS_UnregisterEvent(presenterContext->watchdogEventHandler);
    NEXUS_UnregisterEvent(presenterContext->tsmPassEventHandler);
    NEXUS_UnregisterEvent(presenterContext->firstPtsEventHandler);

    NEXUS_Module_Lock(g_astm.settings.modules.videoDecoder);
    NEXUS_VideoDecoder_GetAstmSettings_priv(astm->settings.videoDecoder, &astmSettings);
    astmSettings.firstPts_isr = NULL;
    astmSettings.tsmPass_isr = NULL;
    astmSettings.tsmFail_isr = NULL;
    astmSettings.tsmLog_isr  = NULL;
    astmSettings.watchdog_isr  = NULL;
    astmSettings.lifecycle_isr  = NULL;
    astmSettings.callbackContext = NULL;
    astmSettings.enableAstm = false;
    astmSettings.ptsOffset = 0;
    (void)NEXUS_VideoDecoder_SetAstmSettings_priv(astm->settings.videoDecoder, &astmSettings);
    NEXUS_Module_Unlock(g_astm.settings.modules.videoDecoder);

    presenterContext->decoder = NULL;
#ifdef BDBG_DEBUG_BUILD
    presenterContext->decoderName = NULL;
#endif
    presenterContext->astm = NULL;
    presenterContext->getStatus_isr = NULL;
    presenterContext->getStatus = NULL;
    presenterContext->setSettings = NULL;
    presenterContext->getSettings = NULL;

    BASTMlib_RemovePresenter(astm->lib.handle, presenterContext->presenter);

#ifdef BDBG_DEBUG_BUILD
    BASTMlib_Presenter_GetSettings(presenterContext->presenter, &presenterSettings);
    presenterSettings.pcName = NULL;
    BASTMlib_Presenter_SetSettings(presenterContext->presenter, &presenterSettings);
#endif

    BDBG_MSG(("%p: video decoder %p disconnected", (void *)astm, (void *)astm->settings.videoDecoder));

    astm->settings.videoDecoder = NULL;
}

static NEXUS_Error NEXUS_Astm_P_ConnectVideoDecoder(NEXUS_AstmHandle astm, const NEXUS_AstmSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BERR_Code mrc = BERR_SUCCESS;
    NEXUS_VideoDecoderAstmSettings astmSettings;
#ifdef BDBG_DEBUG_BUILD
    BASTMlib_Presenter_Settings presenterSettings;
#endif
    struct NEXUS_AstmContext * presenterContext;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    presenterContext = &astm->presenterContexts[0];

    presenterContext->lifecycleEventHandler = NEXUS_RegisterEvent(presenterContext->lifecycleEvent, &NEXUS_Astm_P_DecoderLifecycleEventHandler, presenterContext);
    presenterContext->watchdogEventHandler = NEXUS_RegisterEvent(presenterContext->watchdogEvent, &NEXUS_Astm_P_DecoderWatchdogEventHandler, presenterContext);
    presenterContext->tsmPassEventHandler = NEXUS_RegisterEvent(presenterContext->tsmPassEvent, &NEXUS_Astm_P_DecoderTsmPassEventHandler, presenterContext);
    presenterContext->firstPtsEventHandler = NEXUS_RegisterEvent(presenterContext->firstPtsEvent, &NEXUS_Astm_P_DecoderFirstPtsEventHandler, presenterContext);

#ifdef BDBG_DEBUG_BUILD
    BASTMlib_Presenter_GetSettings(presenterContext->presenter, &presenterSettings);
    presenterContext->decoderName = presenterSettings.pcName = decoderNameStrings[0];
    BASTMlib_Presenter_SetSettings(presenterContext->presenter, &presenterSettings);
#endif

    mrc = BASTMlib_AddPresenter(astm->lib.handle, presenterContext->presenter);
    if (mrc) {rc = NEXUS_UNKNOWN; goto err;}

    presenterContext->decoder = pSettings->videoDecoder;
    presenterContext->astm = astm;
    presenterContext->getStatus_isr = &NEXUS_Astm_P_GetVideoStatus_isr;
    presenterContext->getStatus = &NEXUS_Astm_P_GetVideoStatus;
    presenterContext->getSettings = &NEXUS_Astm_P_GetVideoSettings;
    presenterContext->setSettings = &NEXUS_Astm_P_SetVideoSettings;
    presenterContext->passEventCount = 0;
    presenterContext->ptsStcDiffAdjustmentThreshold = pSettings->videoPresenterConfig.ptsStcDiffAdjustmentThreshold;
    presenterContext->tsmRecoveryAcquisitionTimer = NULL;
    presenterContext->tsmRecoveryAcquisitionPeriod = pSettings->videoPresenterConfig.tsmRecoveryAcquisitionPeriod;
    presenterContext->tsmRecoveryTrackingTimer = NULL;
    presenterContext->tsmRecoveryTrackingTimeout = pSettings->videoPresenterConfig.tsmRecoveryTrackingTimeout;
    presenterContext->maxAllowableFirstPtsStcDiff = pSettings->videoPresenterConfig.maxAllowableFirstPtsStcDiff;
    presenterContext->tsmThresholdAdjustment = pSettings->videoPresenterConfig.tsmThresholdAdjustment;
    presenterContext->manageRateControl = pSettings->videoPresenterConfig.manageRateControl;
    presenterContext->firstPtsTsm = true;
    presenterContext->firstPtsReceived = false;
    presenterContext->module = g_astm.settings.modules.videoDecoder;

    NEXUS_Module_Lock(g_astm.settings.modules.videoDecoder);
    NEXUS_VideoDecoder_GetAstmSettings_priv(pSettings->videoDecoder, &astmSettings);
    astmSettings.enableAstm = false;
    astmSettings.enableTsm = true;
    astmSettings.enablePlayback = false;
    astmSettings.syncLimit = 0;
    astmSettings.ptsOffset = 0;
    astmSettings.firstPts_isr = &NEXUS_Astm_P_DecoderFirstPtsCallback_isr;
    astmSettings.tsmPass_isr = &NEXUS_Astm_P_DecoderTsmPassCallback_isr;
    astmSettings.tsmFail_isr = &NEXUS_Astm_P_DecoderTsmFailCallback_isr;
    astmSettings.tsmLog_isr = &NEXUS_Astm_P_DecoderTsmLogCallback_isr;
    astmSettings.watchdog_isr = &NEXUS_Astm_P_DecoderWatchdogCallback_isr;
    astmSettings.lifecycle_isr = &NEXUS_Astm_P_DecoderLifecycleCallback_isr;
    astmSettings.callbackContext = presenterContext;
    rc = NEXUS_VideoDecoder_SetAstmSettings_priv(pSettings->videoDecoder, &astmSettings);
    NEXUS_Module_Unlock(g_astm.settings.modules.videoDecoder);
    if (rc) goto err;

    astm->settings.videoDecoder = pSettings->videoDecoder;

    BDBG_MSG(("%p: video decoder %p connected", (void *)astm, (void *)astm->settings.videoDecoder));

err:
    return rc;
}

static NEXUS_Error NEXUS_Astm_P_DisconnectAudioDecoder(NEXUS_AstmHandle astm, unsigned int index)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_AudioDecoderAstmSettings astmSettings;
#ifdef BDBG_DEBUG_BUILD
    BASTMlib_Presenter_Settings presenterSettings;
#endif
    unsigned int presenterIndex = index + 1;
    struct NEXUS_AstmContext * presenterContext;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (index > 1)
    {
        rc = NEXUS_INVALID_PARAMETER;
        goto err;
    }

    presenterContext = &astm->presenterContexts[presenterIndex];

    NEXUS_UnregisterEvent(presenterContext->lifecycleEventHandler);
    NEXUS_UnregisterEvent(presenterContext->watchdogEventHandler);
    NEXUS_UnregisterEvent(presenterContext->tsmPassEventHandler);
    NEXUS_UnregisterEvent(presenterContext->firstPtsEventHandler);

    NEXUS_Module_Lock(g_astm.settings.modules.audio);
    NEXUS_AudioDecoder_GetAstmSettings_priv(astm->settings.audioDecoder[index], &astmSettings);
    astmSettings.firstPts_isr = NULL;
    astmSettings.tsmPass_isr = NULL;
    astmSettings.tsmFail_isr = NULL;
    astmSettings.tsmLog_isr  = NULL;
    astmSettings.watchdog_isr  = NULL;
    astmSettings.lifecycle_isr  = NULL;
    astmSettings.callbackContext = NULL;
    astmSettings.enableAstm = false;
    astmSettings.ptsOffset = 0;
    (void)NEXUS_AudioDecoder_SetAstmSettings_priv(astm->settings.audioDecoder[index], &astmSettings);
    NEXUS_Module_Unlock(g_astm.settings.modules.audio);

    presenterContext->decoder = NULL;
    presenterContext->astm = NULL;
    presenterContext->getStatus_isr = NULL;
    presenterContext->getStatus = NULL;
    presenterContext->getSettings = NULL;
    presenterContext->setSettings = NULL;

    BASTMlib_RemovePresenter(astm->lib.handle, presenterContext->presenter);

#ifdef BDBG_DEBUG_BUILD
    BASTMlib_Presenter_GetSettings(presenterContext->presenter, &presenterSettings);
    presenterSettings.pcName = NULL;
    BASTMlib_Presenter_SetSettings(presenterContext->presenter, &presenterSettings);
#endif

    BDBG_MSG(("%p: audio decoder %p disconnected from index %u", (void *)astm, (void *)astm->settings.audioDecoder[index], index));

    astm->settings.audioDecoder[index] = NULL;

err:
    return rc;
}

static NEXUS_Error NEXUS_Astm_P_ConnectAudioDecoder(NEXUS_AstmHandle astm, unsigned int index, const NEXUS_AstmSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BERR_Code mrc = BERR_SUCCESS;
    NEXUS_AudioDecoderAstmSettings astmSettings;
#ifdef BDBG_DEBUG_BUILD
    BASTMlib_Presenter_Settings presenterSettings;
#endif
    unsigned int presenterIndex = index + 1;
    struct NEXUS_AstmContext * presenterContext;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (index > 1)
    {
        rc = NEXUS_INVALID_PARAMETER;
        goto err;
    }

    presenterContext = &astm->presenterContexts[presenterIndex];

    presenterContext->lifecycleEventHandler = NEXUS_RegisterEvent(presenterContext->lifecycleEvent, &NEXUS_Astm_P_DecoderLifecycleEventHandler, presenterContext);
    presenterContext->watchdogEventHandler = NEXUS_RegisterEvent(presenterContext->watchdogEvent, &NEXUS_Astm_P_DecoderWatchdogEventHandler, presenterContext);
    presenterContext->tsmPassEventHandler = NEXUS_RegisterEvent(presenterContext->tsmPassEvent, &NEXUS_Astm_P_DecoderTsmPassEventHandler, presenterContext);
    presenterContext->firstPtsEventHandler = NEXUS_RegisterEvent(presenterContext->firstPtsEvent, &NEXUS_Astm_P_DecoderFirstPtsEventHandler, presenterContext);

#ifdef BDBG_DEBUG_BUILD
    BASTMlib_Presenter_GetSettings(presenterContext->presenter, &presenterSettings);
    presenterContext->decoderName = presenterSettings.pcName = decoderNameStrings[presenterIndex];
    BASTMlib_Presenter_SetSettings(presenterContext->presenter, &presenterSettings);
#endif

    mrc = BASTMlib_AddPresenter(astm->lib.handle, presenterContext->presenter);
    if (mrc) {rc = NEXUS_UNKNOWN; goto err;}

    presenterContext->decoder = pSettings->audioDecoder[index];
    presenterContext->astm = astm;
    presenterContext->getStatus_isr = &NEXUS_Astm_P_GetAudioStatus_isr;
    presenterContext->getStatus = &NEXUS_Astm_P_GetAudioStatus;
    presenterContext->getSettings = &NEXUS_Astm_P_GetAudioSettings;
    presenterContext->setSettings = &NEXUS_Astm_P_SetAudioSettings;
    presenterContext->passEventCount = 0;
    presenterContext->ptsStcDiffAdjustmentThreshold = pSettings->audioPresenterConfig[index].ptsStcDiffAdjustmentThreshold;
    presenterContext->tsmRecoveryAcquisitionTimer = NULL;
    presenterContext->tsmRecoveryAcquisitionPeriod = pSettings->audioPresenterConfig[index].tsmRecoveryAcquisitionPeriod;
    presenterContext->tsmRecoveryTrackingTimer = NULL;
    presenterContext->tsmRecoveryTrackingTimeout = pSettings->audioPresenterConfig[index].tsmRecoveryTrackingTimeout;
    presenterContext->maxAllowableFirstPtsStcDiff = pSettings->audioPresenterConfig[index].maxAllowableFirstPtsStcDiff;
    presenterContext->firstPtsTsm = true;
    presenterContext->firstPtsReceived = false;
    presenterContext->tsmThresholdAdjustment = pSettings->audioPresenterConfig[index].tsmThresholdAdjustment;
    presenterContext->manageRateControl = pSettings->audioPresenterConfig[index].manageRateControl;
    presenterContext->module = g_astm.settings.modules.audio;

    NEXUS_Module_Lock(g_astm.settings.modules.audio);
    NEXUS_AudioDecoder_GetAstmSettings_priv(pSettings->audioDecoder[index], &astmSettings);
    astmSettings.enableAstm = false;
    astmSettings.enableTsm = true;
    astmSettings.enablePlayback = false;
    astmSettings.syncLimit = 0;
    astmSettings.ptsOffset = 0;
    astmSettings.firstPts_isr = &NEXUS_Astm_P_DecoderFirstPtsCallback_isr;
    astmSettings.tsmPass_isr = &NEXUS_Astm_P_DecoderTsmPassCallback_isr;
    astmSettings.tsmFail_isr = &NEXUS_Astm_P_DecoderTsmFailCallback_isr;
    astmSettings.tsmLog_isr = &NEXUS_Astm_P_DecoderTsmLogCallback_isr;
    astmSettings.watchdog_isr = &NEXUS_Astm_P_DecoderWatchdogCallback_isr;
    astmSettings.lifecycle_isr = &NEXUS_Astm_P_DecoderLifecycleCallback_isr;
    astmSettings.callbackContext = presenterContext;
    rc = NEXUS_AudioDecoder_SetAstmSettings_priv(pSettings->audioDecoder[index], &astmSettings);
    NEXUS_Module_Unlock(g_astm.settings.modules.audio);
    if (rc) goto err;

    astm->settings.audioDecoder[index] = pSettings->audioDecoder[index];

    BDBG_MSG(("%p: audio decoder %p connected at index %u", (void *)astm, (void *)astm->settings.audioDecoder[index], index));

err:
    return rc;
}

static void NEXUS_Astm_P_DisconnectTimebase(NEXUS_AstmHandle astm)
{
    NEXUS_TimebaseAstmSettings astmSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    NEXUS_Module_Lock(g_astm.settings.modules.transport);
    NEXUS_Timebase_GetAstmSettings_priv(astm->timebase, &astmSettings);
    astmSettings.enabled = false;
    astmSettings.pcrReceived_isr = NULL;
    (void)NEXUS_Timebase_SetAstmSettings_priv(astm->timebase, &astmSettings);
    NEXUS_Module_Unlock(g_astm.settings.modules.transport);

    BDBG_MSG(("%p: timebase %p disconnected", (void *)astm, (void *)astm->timebase));

    astm->timebase = NULL;
}

static NEXUS_Error NEXUS_Astm_P_ConnectTimebase(NEXUS_AstmHandle astm, const NEXUS_AstmSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelSettings stcChannelSettings;
    NEXUS_TimebaseAstmSettings astmSettings;
    NEXUS_StcChannelAstmStatus stcChannelAstmStatus;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    NEXUS_StcChannel_GetSettings(pSettings->stcChannel, &stcChannelSettings);

    NEXUS_Module_Lock(g_astm.settings.modules.transport);
    NEXUS_StcChannel_GetAstmStatus_priv(pSettings->stcChannel, &stcChannelAstmStatus);
    NEXUS_Module_Unlock(g_astm.settings.modules.transport);

    astm->timebase = stcChannelAstmStatus.timebase;

    switch (stcChannelSettings.mode)
    {
        case NEXUS_StcChannelMode_ePcr:
            if (stcChannelSettings.modeSettings.pcr.pidChannel)
            {
                NEXUS_PidChannelStatus pidChannelStatus;
                NEXUS_PidChannel_GetStatus(stcChannelSettings.modeSettings.pcr.pidChannel, &pidChannelStatus);
                astm->transportType = pidChannelStatus.transportType;
            }
            break;
        case NEXUS_StcChannelMode_eAuto:
            astm->transportType = stcChannelSettings.modeSettings.Auto.transportType;
            break;
        case NEXUS_StcChannelMode_eHost:
            astm->transportType = stcChannelSettings.modeSettings.host.transportType;
            break;
        default:
            BDBG_WRN(("%p: ASTM transport type not set; default to MPEG2 transport stream", (void *)astm));
            astm->transportType = NEXUS_TransportType_eTs;
            break;
    }

    NEXUS_Module_Lock(g_astm.settings.modules.transport);
    NEXUS_Timebase_GetAstmSettings_priv(astm->timebase, &astmSettings);
    astmSettings.enabled = false;
    astmSettings.clockCoupling = NEXUS_TimebaseClockCoupling_eInputClock;
    astmSettings.pcrReceived_isr = NEXUS_Astm_P_TimebasePcrReceivedCallback_isr;
    astmSettings.callbackContext = astm;
    rc = NEXUS_Timebase_SetAstmSettings_priv(astm->timebase, &astmSettings);
    NEXUS_Module_Unlock(g_astm.settings.modules.transport);
    if (rc) goto err;

    BDBG_MSG(("%p: timebase %p connected", (void *)astm, (void *)astm->timebase));

err:
    return rc;
}

static void NEXUS_Astm_P_DisconnectStcChannel(NEXUS_AstmHandle astm)
{
    unsigned int i = 0;
    NEXUS_StcChannelAstmSettings astmSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    NEXUS_Module_Lock(g_astm.settings.modules.transport);
    NEXUS_StcChannel_GetAstmSettings_priv(astm->settings.stcChannel, &astmSettings);
    astmSettings.enabled = false;
    (void)NEXUS_StcChannel_SetAstmSettings_priv(astm->settings.stcChannel, &astmSettings);
    NEXUS_Module_Unlock(g_astm.settings.modules.transport);

    /* 20081204 bandrews - apply STC channel to all video and audio decoder
    presenter contexts, in case they were set at a different time from the STC channel */
    for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
    {
        astm->presenterContexts[i].stc = NULL;
    }

    BDBG_MSG(("%p: Stc channel %p disconnected", (void *)astm, (void *)astm->settings.stcChannel));

    astm->settings.stcChannel = NULL;
}

static NEXUS_Error NEXUS_Astm_P_ConnectStcChannel(NEXUS_AstmHandle astm, const NEXUS_AstmSettings * pSettings)
{
    unsigned int i = 0;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelAstmSettings astmSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    NEXUS_Module_Lock(g_astm.settings.modules.transport);
    NEXUS_StcChannel_GetAstmSettings_priv(pSettings->stcChannel, &astmSettings);
    astmSettings.enabled = false;
    astmSettings.mode = NEXUS_StcChannelMode_ePcr;
    astmSettings.tsmMode = NEXUS_StcChannelTsmMode_eStcMaster;
    astmSettings.syncLimit = 0;
    rc = NEXUS_StcChannel_SetAstmSettings_priv(pSettings->stcChannel, &astmSettings);
    NEXUS_Module_Unlock(g_astm.settings.modules.transport);
    if (rc) goto err;

    astm->settings.stcChannel = pSettings->stcChannel;

    /* 20081204 bandrews - apply STC channel to all video and audio decoder
    presenter contexts, in case they were set at a different time from the STC channel */
    for (i = 0; i < NEXUS_ASTM_PRESENTERS; i++)
    {
        astm->presenterContexts[i].stc = pSettings->stcChannel;
    }

    BDBG_MSG(("%p: Stc channel %p connected", (void *)astm, (void *)pSettings->stcChannel));

    NEXUS_Astm_P_SetConfigFromStcChannelMode(astm);

err:
    return rc;
}

typedef struct NEXUS_AstmChanges
{
    bool enabled;
    bool clockCoupling;
    bool stcSource;
    bool presentationRateControl;
    bool tsmMode;
    bool video;
    bool audio;
    bool stc;
} NEXUS_AstmChanges;

static NEXUS_Error NEXUS_Astm_P_ApplyChanges(NEXUS_AstmHandle astm, const NEXUS_AstmChanges * pChanges)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BASTMlib_Config astmlibConfig;

    /* apply settings */
    if (astm->settings.enabled)
    {
        if (pChanges->enabled || pChanges->clockCoupling)
        {
            BDBG_MSG(("SetSettings"));
            rc = NEXUS_Astm_P_ApplyClockCoupling(astm, astm->status.clockCoupling);
            if (rc) goto err;
            BDBG_MSG(("Setting astmlib config PCC"));
            BASTMlib_GetConfig(astm->lib.handle, &astmlibConfig);
            astmlibConfig.sClockCoupling.ePreferredClockCoupling = NEXUS_Astm_P_ClockCouplingToMagnum(astm->status.clockCoupling);
            BASTMlib_SetConfig(astm->lib.handle, &astmlibConfig);
        }

        if (pChanges->enabled || pChanges->stcSource)
        {
            rc = NEXUS_Astm_P_ApplyStcSource(astm, astm->status.stcSource, astm->settings.stcMaster, astm->settings.syncLimit);
            if (rc) goto err;
            BDBG_MSG(("Setting astmlib config PSS"));
            BASTMlib_GetConfig(astm->lib.handle, &astmlibConfig);
            astmlibConfig.sPresentation.ePreferredStcSource = NEXUS_Astm_P_StcSourceToMagnum(astm->status.stcSource);
            BASTMlib_SetConfig(astm->lib.handle, &astmlibConfig);
        }

        if (pChanges->enabled || pChanges->presentationRateControl)
        {
            rc = NEXUS_Astm_P_ApplyPresentationRateControl(astm, astm->status.presentationRateControl);
            if (rc) goto err;
            BDBG_MSG(("Setting astmlib config PPRC"));
            BASTMlib_GetConfig(astm->lib.handle, &astmlibConfig);
            astmlibConfig.sPresentation.ePreferredPresentationRateControl = NEXUS_Astm_P_PresentationRateControlToMagnum(astm->status.presentationRateControl);
            BASTMlib_SetConfig(astm->lib.handle, &astmlibConfig);
        }

        if (pChanges->enabled || pChanges->tsmMode)
        {
            rc = NEXUS_Astm_P_ApplyStcChannelTsmMode(astm, astm->status.tsmMode);
            if (rc) goto err;
        }
    }

err:
    return rc;
}

static void NEXUS_Astm_P_ComputeStatus(NEXUS_AstmHandle astm, const NEXUS_AstmSettings * pSettings, NEXUS_AstmStatus * pStatus)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    pStatus->clockCoupling = pSettings->clockCoupling;
    pStatus->stcSource = pSettings->stcSource;
    pStatus->presentationRateControl = pSettings->presentationRateControl;
    NEXUS_Astm_P_ResolveAdaptiveModes(astm, &pStatus->clockCoupling, &pStatus->stcSource, &pStatus->presentationRateControl);
    pStatus->tsmMode = NEXUS_Astm_P_ComputeStcChannelTsmMode(astm,
        pStatus->stcSource,
        pSettings->stcMaster,
        pStatus->presentationRateControl);
}

static void NEXUS_Astm_P_ComputeChanges(NEXUS_AstmHandle astm, const NEXUS_AstmSettings * pSettings, const NEXUS_AstmStatus * pStatus, NEXUS_AstmChanges * pChanges)
{
    if (pSettings->enabled != astm->settings.enabled)
    {
        pChanges->enabled = true;
    }

    /* determine changes */
    if (astm->timebase && (pStatus->clockCoupling != astm->status.clockCoupling))
    {
        pChanges->clockCoupling = true;
    }

    if
    (
        pSettings->stcChannel
        &&
        (
            pChanges->stc
            ||
            pChanges->video
            ||
            pChanges->audio
            ||
            pStatus->stcSource != astm->status.stcSource
            ||
            pSettings->stcMaster != astm->settings.stcMaster
            ||
            pSettings->syncLimit != astm->settings.syncLimit
        )
    )
    {
        pChanges->stcSource = true;
    }

    if
    (
        pChanges->video
        ||
        pChanges->audio
        ||
        pStatus->presentationRateControl != astm->status.presentationRateControl
    )
    {
        pChanges->presentationRateControl = true;
    }

    if
    (
        pSettings->stcChannel
        &&
        (
            pChanges->stc
            ||
            pStatus->tsmMode != astm->status.tsmMode
        )
    )
    {
        pChanges->tsmMode = true;
    }
}

static NEXUS_Error NEXUS_Astm_P_ApplySettings(NEXUS_AstmHandle astm, const NEXUS_AstmSettings * pSettings, const NEXUS_AstmChanges * pChanges)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_AstmStatus status;
    NEXUS_AstmChanges changes;

    BKNI_Memset(&changes, 0, sizeof(NEXUS_AstmChanges));

    if (pChanges)
    {
        changes = *pChanges;
    }

    if (!pSettings)
    {
        pSettings = &astm->settings;
    }

    /* compute new status based on resolution of modes in settings */
    NEXUS_Astm_P_ComputeStatus(astm, pSettings, &status);

    BDBG_MSG(("pSettings->clockCoupling: %s; newStatus.clockCoupling = %s",
        clockCouplingStrings[pSettings->clockCoupling],
        clockCouplingStrings[status.clockCoupling]));

    /* determine changes */
    NEXUS_Astm_P_ComputeChanges(astm, pSettings, &status, &changes);

    /* copy settings */
    if (pSettings != &astm->settings)
    {
        astm->settings = *pSettings;
    }

    /* copy status */
    astm->status = status;

    /* do it, do it now */
    rc = NEXUS_Astm_P_ApplyChanges(astm, &changes);
    if (rc) { rc = BERR_TRACE(rc); }

    return rc;
}

NEXUS_Error NEXUS_Astm_SetSettings(NEXUS_AstmHandle astm, const NEXUS_AstmSettings *pSettings)
{
    BASTMlib_Config astmlibConfig;
    BASTMlib_Presenter_Config astmPresenterConfig;
    unsigned int i;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_AstmSettings defaultSettings;
    NEXUS_AstmChanges changes;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (!pSettings)
    {
        NEXUS_Astm_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    BKNI_EnterCriticalSection();
    astm->ready = false;
    BKNI_LeaveCriticalSection();

    BKNI_Memset(&changes, 0, sizeof(NEXUS_AstmChanges));

    if (pSettings->stcChannel != astm->settings.stcChannel)
    {
        changes.stc = true;

        /* disco timebase first, relies on STC channel, may change in future with mutable timebases */
        if (astm->timebase)
        {
            NEXUS_Astm_P_DisconnectTimebase(astm);
        }

        if (astm->settings.stcChannel)
        {
            NEXUS_Astm_P_DisconnectStcChannel(astm);
        }

        if (pSettings->stcChannel)
        {
            NEXUS_Astm_P_ConnectStcChannel(astm, pSettings);

            /* need STC channel to connect timebase, this may change in future with mutable timebases */
            NEXUS_Astm_P_ConnectTimebase(astm, pSettings);
        }
    }

    if (pSettings->videoDecoder != astm->settings.videoDecoder)
    {
        changes.video = true;

        if (astm->settings.videoDecoder)
        {
            NEXUS_Astm_P_DisconnectVideoDecoder(astm);
        }

        if (pSettings->videoDecoder)
        {
            NEXUS_Astm_P_ConnectVideoDecoder(astm, pSettings);
        }
    }

    for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
    {
        if (pSettings->audioDecoder[i] != astm->settings.audioDecoder[i])
        {
            changes.audio = true;

            if (astm->settings.audioDecoder[i])
            {
                NEXUS_Astm_P_DisconnectAudioDecoder(astm, i);
            }

            if (pSettings->audioDecoder[i])
            {
                NEXUS_Astm_P_ConnectAudioDecoder(astm, i, pSettings);
            }
        }
    }

    /* ASTMlib config */
    {
        BASTMlib_GetConfig(astm->lib.handle, &astmlibConfig);

        if (astm->transportType == NEXUS_TransportType_eDssEs
            || astm->transportType == NEXUS_TransportType_eDssPes)
        {
            astmlibConfig.eStcRate = BASTMlib_ClockRate_e27Mhz;
            astmlibConfig.sClockCoupling.eClockReferenceDomain = BASTMlib_ClockRate_e27Mhz;

        }
        else
        {
            astmlibConfig.eStcRate = BASTMlib_ClockRate_e45Khz;
            astmlibConfig.sClockCoupling.eClockReferenceDomain = BASTMlib_ClockRate_e45Khz;
        }

        astmlibConfig.sPresentation.hPreferredPresenter = NEXUS_Astm_P_ResolveStcMasterToMagnum(astm, pSettings);

        astmlibConfig.bEnabled = pSettings->enabled;
        astmlibConfig.sPresentation.uiInitialAcquisitionTime = pSettings->presentationConfig.initialAcquisitionTime;
        astmlibConfig.sPresentation.uiProcessingFrequency = pSettings->presentationConfig.processingFrequency;
        astmlibConfig.sPresentation.uiTsmDisabledWatchdogTimeout = pSettings->presentationConfig.tsmDisabledWatchdogTimeout;
        astmlibConfig.sPresentation.uiSettlingTime = pSettings->presentationConfig.settlingTime;
        astmlibConfig.sPresentation.ePreferredPresentationRateControl =
            NEXUS_Astm_P_PresentationRateControlToMagnum(pSettings->presentationConfig.preferredPresentationRateControl);
        astmlibConfig.sClockCoupling.uiMinimumTimeBetweenEvents = pSettings->clockCouplingConfig.minimumTimeBetweenEvents;
        astmlibConfig.sClockCoupling.uiDeviationThreshold = pSettings->clockCouplingConfig.deviationThreshold;
        astmlibConfig.sClockCoupling.uiDeviantCountThreshold = pSettings->clockCouplingConfig.deviantCountThreshold;
        astmlibConfig.sClockCoupling.uiIdealCountThreshold = pSettings->clockCouplingConfig.idealCountThreshold;
        astmlibConfig.sClockCoupling.uiInitialAcquisitionTime = pSettings->clockCouplingConfig.initialAcquisitionTime;
        astmlibConfig.sClockCoupling.uiProcessingFrequency = pSettings->clockCouplingConfig.processingFrequency;
        astmlibConfig.sClockCoupling.uiIdealProcessingFrequency = pSettings->clockCouplingConfig.idealProcessingFrequency;
        astmlibConfig.sClockCoupling.uiSettlingTime = pSettings->clockCouplingConfig.settlingTime;
        astmlibConfig.sClockCoupling.pfDetectDataPresent = &NEXUS_Astm_P_PcrDataPresenceDetector;
        astmlibConfig.sClockCoupling.pvDataPresentContext = astm;

        BASTMlib_SetConfig(astm->lib.handle, &astmlibConfig);
    }

    if (astm->settings.videoDecoder)
    {
        BASTMlib_Presenter_GetConfig(astm->presenterContexts[0].presenter, &astmPresenterConfig );
        astmPresenterConfig.uiMinimumTimeBetweenEvents = pSettings->videoPresenterConfig.minimumTimeBetweenEvents;
        astmPresenterConfig.uiPassEventCountThreshold = pSettings->videoPresenterConfig.passEventCountThreshold;
        astmPresenterConfig.uiFailEventCountThreshold = pSettings->videoPresenterConfig.failEventCountThreshold;
        if (astm->transportType == NEXUS_TransportType_eDssEs || astm->transportType == NEXUS_TransportType_eDssPes)
        {
            astmPresenterConfig.ePtsDomain = BASTMlib_ClockRate_e27Mhz;
        }
        else
        {
            astmPresenterConfig.ePtsDomain = BASTMlib_ClockRate_e45Khz;
        }
        BASTMlib_Presenter_SetConfig(astm->presenterContexts[0].presenter, &astmPresenterConfig );
    }

    for (i = 0; i < NEXUS_ASTM_AUDIO_DECODERS; i++)
    {
        if (astm->settings.audioDecoder[i])
        {
            BASTMlib_Presenter_GetConfig(astm->presenterContexts[i + 1].presenter, &astmPresenterConfig);
            astmPresenterConfig.uiMinimumTimeBetweenEvents = pSettings->audioPresenterConfig[i].minimumTimeBetweenEvents;
            astmPresenterConfig.uiPassEventCountThreshold = pSettings->audioPresenterConfig[i].passEventCountThreshold;
            astmPresenterConfig.uiFailEventCountThreshold = pSettings->audioPresenterConfig[i].failEventCountThreshold;
            if (astm->transportType == NEXUS_TransportType_eDssEs || astm->transportType == NEXUS_TransportType_eDssPes)
            {
                astmPresenterConfig.ePtsDomain = BASTMlib_ClockRate_e27Mhz;
            }
            else
            {
                astmPresenterConfig.ePtsDomain = BASTMlib_ClockRate_e45Khz;
            }
            BASTMlib_Presenter_SetConfig(astm->presenterContexts[i + 1].presenter, &astmPresenterConfig);
        }
    }

    BASTMlib_GetStatus(astm->lib.handle, &astm->lib.status);

    rc = NEXUS_Astm_P_ApplySettings(astm, pSettings, &changes);
    if (rc) { rc = BERR_TRACE(rc); goto err; }

    BDBG_MSG(("%p: ALC: %s", (void *)astm, astm->settings.enableAutomaticLifecycleControl ? "enabled" : "disabled"));

end:

    BKNI_EnterCriticalSection();
    astm->ready = true;
    BKNI_LeaveCriticalSection();

    return rc;

err:
    /* TODO: unroll partial changes */
    goto end;
}

static NEXUS_Error NEXUS_Astm_P_Start(NEXUS_AstmHandle astm)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (!NEXUS_GetEnv("astm_disabled"))
    {
        BDBG_MSG(("%p: ASTM starting", (void *)astm));
        rc = (NEXUS_Error)BASTMlib_Start(astm->lib.handle);
        if (rc) goto err;

        /* need to re-resolve adaptive modes and reapply on start */
        rc = NEXUS_Astm_P_ApplySettings(astm, NULL, NULL);
        if (rc) { rc = BERR_TRACE(rc); goto err; }

        /* we really only enable dependency behavior here */
        rc = NEXUS_Astm_P_SetDependencyAstmBehavior(astm, astm->settings.enabled);
        if (rc) goto err;
    }

    astm->started = true;

err:

    return rc;
}

static void NEXUS_Astm_P_Stop(NEXUS_AstmHandle astm)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (!NEXUS_GetEnv("astm_disabled"))
    {
        BDBG_MSG(("%p: ASTM stopping", (void *)astm));
        BASTMlib_Stop(astm->lib.handle);

        NEXUS_Astm_P_SetDependencyAstmBehavior(astm, false);
    }

    /* cancel any pending nexus timers */
    NEXUS_Astm_P_CancelPendingTimers(astm);

    astm->pcrReceived = false;
    astm->started = false;
}

NEXUS_Error NEXUS_Astm_Start(NEXUS_AstmHandle astm)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (!astm->settings.enableAutomaticLifecycleControl)
    {
        if (!astm->started)
        {
            rc = NEXUS_Astm_P_Start(astm);
        }
        else
        {
            BDBG_MSG(("%p: ASTM already started", (void *)astm));
        }
    }
    else
    {
        BDBG_WRN(("%p: You must disable automatic lifecycle control before manually starting ASTM.", (void *)astm));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    return rc;
}

void NEXUS_Astm_Stop(NEXUS_AstmHandle astm)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Astm, astm);

    if (!astm->settings.enableAutomaticLifecycleControl)
    {
        if (astm->started)
        {
            NEXUS_Astm_P_Stop(astm);
        }
        else
        {
            BDBG_MSG(("%p: ASTM not started", (void *)astm));
        }
    }
    else
    {
        BDBG_WRN(("%p: You must disable automatic lifecycle control before manually stopping ASTM.", (void *)astm));
    }
}

