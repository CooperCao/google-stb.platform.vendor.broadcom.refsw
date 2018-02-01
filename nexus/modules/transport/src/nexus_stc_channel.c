/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include "nexus_transport_module.h"
#include "priv/nexus_stc_channel_priv.h"
#include "b_objdb.h"

BDBG_MODULE(nexus_stc_channel);
BDBG_FILE_MODULE(nexus_flow_stc_channel);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

/*#define NEXUS_STC_CHANNEL_DEBUG_DECODER_QUEUE 1*/

static NEXUS_Error NEXUS_StcChannel_P_SetDecoderConnectionSettings(NEXUS_StcChannelDecoderConnectionHandle decoder, const NEXUS_StcChannelDecoderConnectionSettings *pSettings, bool force);
static NEXUS_StcChannelPidChannelEntry * NEXUS_StcChannel_P_FindPidChannelEntry(NEXUS_StcChannelHandle stcChannel, NEXUS_PidChannelHandle pidChannel);
static void NEXUS_StcChannel_P_SetSwPcrOffsetEnabled(NEXUS_StcChannelHandle stcChannel);

#define PID_CHANNEL_INDEX(pidChannel) ((pidChannel)->hwPidChannel->status.pidChannelIndex)
#define PID_CHANNEL_PID(pidChannel) ((pidChannel)->hwPidChannel->status.pid)

void NEXUS_StcChannel_GetDefaultSettings(unsigned index, NEXUS_StcChannelSettings *pSettings)
{
    BSTD_UNUSED(index);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->stcIndex = -1;
    pSettings->timebase = NEXUS_Timebase_eInvalid;
    pSettings->autoConfigTimebase = true;
    pSettings->mode = NEXUS_StcChannelMode_eAuto;
    pSettings->modeSettings.Auto.transportType = NEXUS_TransportType_eTs;
    pSettings->modeSettings.host.transportType = NEXUS_TransportType_eTs;
    /* VideoMaster is default because most people measure PVR start performance by the appearance of video.
    Also, it provides more deterministic behavior than eFirstAvailable.
    The downside is that some audio could be lost, depending on the stream muxing. */
    pSettings->modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eVideoMaster;
    pSettings->modeSettings.pcr.maxPcrError = 0xff;
    pSettings->modeSettings.pcr.offsetThreshold = 8;
    pSettings->modeSettings.pcr.disableJitterAdjustment = false;
    pSettings->modeSettings.pcr.disableTimestampCorrection = false;
    pSettings->modeSettings.Auto.offsetThreshold = 100;
    pSettings->modeSettings.host.offsetThreshold = 100;
    return;
}

static void NEXUS_StcChannel_P_Finalizer(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_StcChannelSettings settings;
    NEXUS_StcChannelPidChannelEntry * e;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

    /* revert to default known state */
    NEXUS_StcChannel_GetDefaultSettings(0, &settings);
    /*
     * 20140611 bandrews
     * we don't want to revert to auto config the default timebase
     * on close if we weren't auto config before close,
     * as the default may be the one the user wanted to keep
     * externally configured (which they signaled by disabling
     * auto config while stc channel was open)
     */
    settings.autoConfigTimebase = stcChannel->settings.autoConfigTimebase;
    rc = NEXUS_StcChannel_SetSettings(stcChannel, &settings);
    if (rc) BERR_TRACE(rc); /* keep going */

    stcChannel->timebase = NULL;

    /* clean up any leftover enabled pid channels */
    while ((e = BLST_Q_FIRST(&stcChannel->pids))) {
        BDBG_WRN(("Unbalanced EnablePidChannel(%u) for pidChannel %u; disabling", e->refcnt, e->pidChannelIndex));
        BXPT_PcrOffset_DisableOffset(stcChannel->pcrOffset, e->pidChannelIndex);
        BLST_Q_REMOVE(&stcChannel->pids, e, link);
        BKNI_Free(e);
    }

    if (stcChannel->pcrlibChannel) {
        BPCRlib_Channel_Destroy(stcChannel->pcrlibChannel);
        stcChannel->pcrlibChannel = NULL;
    }
    if (stcChannel->pcrOffset) {
        (void)BXPT_PcrOffset_Close(stcChannel->pcrOffset);
        stcChannel->pcrOffset = NULL;
    }
    pTransport->stcChannel[stcChannel->index] = NULL;
    BDBG_OBJECT_DESTROY(stcChannel, NEXUS_StcChannel);
    BKNI_Free(stcChannel);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_StcChannel, NEXUS_StcChannel_Close);

BDBG_OBJECT_ID(NEXUS_StcChannelSnapshot);
BDBG_OBJECT_ID(NEXUS_StcChannelDecoderConnection);

NEXUS_StcChannelHandle NEXUS_StcChannel_Open(unsigned index, const NEXUS_StcChannelSettings *pSettings)
{
    BXPT_PcrOffset_Defaults pcrOffsetDefaults;
    BPCRlib_ChannelSettings settings;
    BERR_Code rc;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcChannelSettings;
    NEXUS_TimebaseHandle timebase;

    if (!pSettings) {
        NEXUS_StcChannel_GetDefaultSettings(index, &stcChannelSettings);
        pSettings = &stcChannelSettings;
    }

    if (index == NEXUS_ANY_ID) {
        for (index=0;index<BXPT_NUM_PCR_OFFSET_CHANNELS;index++) {
            if (!pTransport->stcChannel[index]) {
                break;
            }
        }
        if (index == BXPT_NUM_PCR_OFFSET_CHANNELS) {
            BDBG_ERR(("no stc channel available"));
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            return NULL;
        }
    } else {
        if (index >= BXPT_NUM_PCR_OFFSET_CHANNELS) {
            BDBG_ERR(("stcChannel %d not available", index));
            (void)BERR_TRACE(NEXUS_NOT_AVAILABLE);
            return NULL;
        }
        if(pTransport->stcChannel[index] != NULL) {
            (void)BERR_TRACE(NEXUS_NOT_AVAILABLE);
            return NULL;
        }
    }


    stcChannel = BKNI_Malloc(sizeof(*stcChannel));
    if (!stcChannel) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(stcChannel, 0, sizeof(*stcChannel));
    NEXUS_OBJECT_SET(NEXUS_StcChannel, stcChannel);
    stcChannel->index = index;
    pTransport->stcChannel[index] = stcChannel;
    stcChannel->stcIndex = (pSettings->stcIndex < 0) ? index : (unsigned)pSettings->stcIndex;
    if (stcChannel->stcIndex >= BXPT_NUM_STCS) {
        BDBG_ERR(("invalid stcIndex %d", stcChannel->stcIndex));
        goto error;
    }
    /* Only NEXUS_ClientMode_eVerified clients can use NEXUS_Timebase_eInvalid (default timebase) and update it.
    For other clients, they can use the use the default timebase as freerun (eAuto or eHost);
    they cannot use the default timebase for live (ePcr). */
    stcChannel->modifyDefaultTimebase = (b_objdb_get_client()->mode < NEXUS_ClientMode_eProtected);

    BXPT_PcrOffset_GetChannelDefSettings(pTransport->xpt, index, &pcrOffsetDefaults);
    pcrOffsetDefaults.UsePcrTimeBase = true;
    /* There could be more StcChannels than Timebases. This gives XPT a meaningful default. */
    timebase = NEXUS_Timebase_Resolve_priv(pSettings->timebase);
    if (timebase)
    {
        pcrOffsetDefaults.WhichPcr = timebase->hwIndex;
    }
    else
    {
        pcrOffsetDefaults.WhichPcr = 0; /* this is just the default, if we are asked to manage the timebase, we will update this later in ApplySettings */
    }
#ifdef BXPT_HAS_MOSAIC_SUPPORT
    pcrOffsetDefaults.WhichStc = stcChannel->stcIndex;
#endif
    rc = BXPT_PcrOffset_Open(pTransport->xpt, index, &pcrOffsetDefaults, &stcChannel->pcrOffset);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

#if NEXUS_HAS_ASTM
    BKNI_Memset(&stcChannel->astm.settings, 0, sizeof(NEXUS_StcChannelAstmSettings));
#endif

    BPCRlib_Channel_GetChannelDefSettings(pTransport->pcrlib, &settings);

    rc = BPCRlib_Channel_Create(pTransport->pcrlib, NULL, &stcChannel->pcrlibChannel, &settings);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    /* on open, need to clear out old settings before applying new ones */
    NEXUS_StcChannel_GetDefaultSettings(index, &stcChannel->settings);

    stcChannel->swPcrOffsetEnabled = true; /* soft, until connected, or host mode */

    rc = NEXUS_StcChannel_SetSettings(stcChannel, pSettings);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    /* default new stc channel to normal rate */
    rc = NEXUS_StcChannel_SetRate(stcChannel, 1, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    BLST_Q_INIT(&stcChannel->decoders);

    stcChannel->stcValid = false;
    stcChannel->nonRealTime = false;
    stcChannel->pairedChannel = NULL;
    stcChannel->lowLatencyDecoder = NULL; /* off by default */

    return stcChannel;

error:
    NEXUS_StcChannel_Close(stcChannel);
    return NULL;
}

void NEXUS_StcChannel_GetSettings(NEXUS_StcChannelHandle stcChannel, NEXUS_StcChannelSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    *pSettings = stcChannel->settings;
}

static NEXUS_TransportType NEXUS_StcChannel_P_GetTransportType(const NEXUS_StcChannelSettings *pSettings)
{
    NEXUS_TransportType transportType = NEXUS_TransportType_eTs; /* default */

    switch (pSettings->mode) {
    case NEXUS_StcChannelMode_ePcr:
        if (!pSettings->modeSettings.pcr.pidChannel) {
            break;
        }
        transportType = pSettings->modeSettings.pcr.pidChannel->hwPidChannel->status.transportType;
        break;
    case NEXUS_StcChannelMode_eAuto:
        transportType = pSettings->modeSettings.Auto.transportType;
        break;
    case NEXUS_StcChannelMode_eHost:
        transportType = pSettings->modeSettings.host.transportType;
        break;
    default:
        break;
    }
    switch (transportType) {
    case NEXUS_TransportType_eAsf:
    case NEXUS_TransportType_eFlv:
    case NEXUS_TransportType_eAvi:
    case NEXUS_TransportType_eMp4:
    case NEXUS_TransportType_eWav:
    case NEXUS_TransportType_eRmff:
    case NEXUS_TransportType_eOgg:
    case NEXUS_TransportType_eFlac:
    case NEXUS_TransportType_eAiff:
    case NEXUS_TransportType_eApe:
    case NEXUS_TransportType_eAmr:
        /* Playpump converts these streams to Mpeg2Pes, so Pcrlib should see it that way. */
        transportType = NEXUS_TransportType_eMpeg2Pes;
        break;
    default:
        break;
    }

    return transportType;
}

#ifdef BXPT_HAS_MOSAIC_SUPPORT
static NEXUS_Error handleIndexChange(NEXUS_StcChannelHandle stcChannel)
{
    uint32_t stc;
    BXPT_PcrOffset_Settings pcr_offset_settings;
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (stcChannel->settings.stcIndex != -1 && stcChannel->settings.stcIndex != (int)stcChannel->stcIndex)
    {
        stcChannel->deferIndexChange = !BLST_Q_EMPTY(&stcChannel->decoders);

        if (!stcChannel->deferIndexChange)
        {
            unsigned oldIndex;
            BXPT_PcrOffset_GetSettings(stcChannel->pcrOffset, &pcr_offset_settings);
            pcr_offset_settings.StcSelect = stcChannel->settings.stcIndex;

            oldIndex = stcChannel->stcIndex;
            /* get current stc value from current index */
            stc = BXPT_PcrOffset_GetStc(stcChannel->pcrOffset);
            /* change index */
            rc = BXPT_PcrOffset_SetSettings(stcChannel->pcrOffset, &pcr_offset_settings);
            if (rc) { BERR_TRACE(rc); goto error; }
            /* set stc */
            rc = BXPT_PcrOffset_SetStc(stcChannel->pcrOffset, stc);
            if (rc) { BERR_TRACE(rc); goto error; }
            /* save the new state */
            stcChannel->stcIndex = stcChannel->settings.stcIndex;

            BDBG_MSG(("STC channel: %p index change from %u to %u completed",
                (void *)stcChannel,
                oldIndex,
                stcChannel->stcIndex));
        }
        else
        {
            BDBG_MSG(("STC channel: %p index change from %u to %u deferred",
                (void *)stcChannel,
                stcChannel->stcIndex,
                stcChannel->settings.stcIndex));
        }
    }

error:
    return rc;
}
#else
static NEXUS_Error handleIndexChange(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (stcChannel->settings.stcIndex != -1 && stcChannel->settings.stcIndex != (int)stcChannel->stcIndex)
    {
        BDBG_ERR(("cannot change stcIndex after Open"));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    return rc;
}
#endif

static void computeTimebaseSettings(
    const NEXUS_StcChannelSettings * pStcSettings,
    NEXUS_StcChannelMode mode,
    NEXUS_TimebaseSettings * pTimebaseSettings /* out */)
{
    BDBG_ASSERT(pStcSettings);
    BDBG_ASSERT(pTimebaseSettings);

    /* always start with default */
    NEXUS_Timebase_GetDefaultSettings(pTimebaseSettings);

    /* I own this timebase, no one else should be modifying it */
    switch (mode)
    {
        case NEXUS_StcChannelMode_ePcr:

            pTimebaseSettings->sourceType = NEXUS_TimebaseSourceType_ePcr;
            pTimebaseSettings->sourceSettings.pcr.pidChannel = pStcSettings->modeSettings.pcr.pidChannel;
            pTimebaseSettings->sourceSettings.pcr.maxPcrError = pStcSettings->modeSettings.pcr.maxPcrError;
            break;

        case NEXUS_StcChannelMode_eAuto:
        case NEXUS_StcChannelMode_eHost:

            pTimebaseSettings->sourceType = NEXUS_TimebaseSourceType_eFreeRun;
            break;

        default:
            break;
    }
}

static NEXUS_Error setDefaultTimebaseConfig(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_TimebaseHandle timebase
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_TimebaseSettings timebaseSettings;

    BDBG_MODULE_MSG(nexus_flow_stc_channel, ("%p applying default config to timebase %u", (void *)stcChannel, timebase->hwIndex));
    NEXUS_Timebase_GetDefaultSettings(&timebaseSettings);
    rc = NEXUS_Timebase_P_SetSettings(timebase, &timebaseSettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

error:
    return rc;
}

static NEXUS_Error setTimebaseConfig(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_TimebaseHandle timebase,
    const NEXUS_StcChannelSettings * pSettings,
    NEXUS_StcChannelMode mode
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_TimebaseSettings timebaseSettings;

    BDBG_MODULE_MSG(nexus_flow_stc_channel, ("%p configuring timebase %u", (void *)stcChannel, timebase->hwIndex));
    computeTimebaseSettings(pSettings, mode, &timebaseSettings);
    rc = NEXUS_Timebase_P_SetSettings(timebase, &timebaseSettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

error:
    return rc;
}

static NEXUS_StcChannelMode computeMode(
    NEXUS_StcChannelHandle stcChannel,
    const NEXUS_StcChannelSettings * pSettings)
{
    NEXUS_StcChannelMode mode;

    mode = pSettings->mode;

#if NEXUS_HAS_ASTM
    if (stcChannel->astm.settings.enabled)
    {
        mode = stcChannel->astm.settings.mode;
        BDBG_MSG(("ASTM is setting the mode for stc%u channel %p to %d", stcChannel->stcIndex, (void *)stcChannel, stcChannel->astm.settings.mode));
    }
#else
    BSTD_UNUSED(stcChannel);
#endif

    return mode;
}

static NEXUS_Error setTimebase(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_TimebaseHandle newTimebase,
    const NEXUS_StcChannelSettings * pSettings,
    NEXUS_StcChannelMode mode
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_TimebaseHandle oldTimebase;
    NEXUS_StcChannelMode oldMode;
    bool wasAutoConfig;
    bool isAutoConfig;
    bool timebaseChange;
    bool autoConfigChange;
    bool modeChange;

    if (!stcChannel->modifyDefaultTimebase && pSettings->timebase == NEXUS_Timebase_eInvalid && mode == NEXUS_StcChannelMode_ePcr && pSettings->autoConfigTimebase) {
        BDBG_ERR(("clients must use timebase from NEXUS_Timebase_Open for NEXUS_StcChannelMode_ePcr"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    oldTimebase = stcChannel->timebase;
    oldMode = computeMode(stcChannel, &stcChannel->settings);
    wasAutoConfig = stcChannel->settings.autoConfigTimebase;
    if (wasAutoConfig && oldMode != NEXUS_StcChannelMode_ePcr && stcChannel->settings.timebase == NEXUS_Timebase_eInvalid) {
        BDBG_MSG(("%p: don't modify default timebase for freerun", (void*)stcChannel));
        wasAutoConfig = false;
    }
    isAutoConfig = pSettings->autoConfigTimebase;
    if (isAutoConfig && mode != NEXUS_StcChannelMode_ePcr && pSettings->timebase == NEXUS_Timebase_eInvalid) {
        BDBG_MSG(("%p: don't modify default timebase for freerun", (void*)stcChannel));
        isAutoConfig = false;
    }

#if NEXUS_HAS_ASTM
    if (stcChannel->astm.settings.enabled)
    {
        wasAutoConfig = false;
        isAutoConfig = false;
    }
#endif

    timebaseChange = oldTimebase != newTimebase;
    autoConfigChange = wasAutoConfig != isAutoConfig;
    modeChange = oldMode != mode;

    /* only apply default to old timebase if things have changed */
    if (oldTimebase && wasAutoConfig && (timebaseChange || autoConfigChange))
    {
        rc = setDefaultTimebaseConfig(stcChannel, oldTimebase);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }

    /* only apply new config to new timebase if things have changed */
    if (newTimebase && isAutoConfig && (timebaseChange || autoConfigChange || modeChange))
    {
        rc = setTimebaseConfig(stcChannel, newTimebase, pSettings, mode);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }

    stcChannel->timebase = newTimebase;

    error:
        return rc;
}

static NEXUS_Error setPcrlibConfig(
    NEXUS_StcChannelHandle stcChannel,
    const NEXUS_StcChannelSettings * pSettings,
    NEXUS_StcChannelMode mode,
    NEXUS_TransportType transportType)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BPCRlib_Config pcrlibConfig;

    BPCRlib_Channel_GetConfig(stcChannel->pcrlibChannel, &pcrlibConfig);

    rc = NEXUS_P_TransportType_ToMagnum_isrsafe(transportType, &pcrlibConfig.stream);
    if (rc) { BERR_TRACE(rc); goto error; }

    switch (mode)
    {
        case NEXUS_StcChannelMode_ePcr:
            pcrlibConfig.playback = false;
            break;

        case NEXUS_StcChannelMode_eHost:
            pcrlibConfig.playback = true;
            pcrlibConfig.tsm_mode = BPCRlib_TsmMode_eOutputMaster;
            break;

        default:
        case NEXUS_StcChannelMode_eAuto:
            pcrlibConfig.playback = true;
            /* TODO: the next 3 are defaults, do we really need to reassert? */
            pcrlibConfig.mode = BPCRlib_Mode_eAutoPts;
            pcrlibConfig.video_pts_offset = (3003*4)/2; /* 120ms */
            pcrlibConfig.audio_pts_offset = (3003*2)/2; /* 60ms */
            /* TODO: end defaults */
            switch (pSettings->modeSettings.Auto.behavior)
            {
                case NEXUS_StcChannelAutoModeBehavior_eVideoMaster:
                    pcrlibConfig.tsm_mode = BPCRlib_TsmMode_eVideoMaster;
                    break;
                case NEXUS_StcChannelAutoModeBehavior_eAudioMaster:
                    pcrlibConfig.tsm_mode = BPCRlib_TsmMode_eAudioMaster;
                    break;
                default:
                case NEXUS_StcChannelAutoModeBehavior_eFirstAvailable:
                    pcrlibConfig.tsm_mode = BPCRlib_TsmMode_eSTCMaster;
                    break;
            }
            break;
    }

#if NEXUS_HAS_ASTM
    if (stcChannel->astm.settings.enabled)
    {
        /* apply tsm mode and sync limit to PCRlib */
        switch (stcChannel->astm.settings.tsmMode)
        {
            case NEXUS_StcChannelTsmMode_eAudioMaster:
                pcrlibConfig.tsm_mode = BPCRlib_TsmMode_eAudioMaster;
                pcrlibConfig.sync_limit = stcChannel->astm.settings.syncLimit;
                break;
            case NEXUS_StcChannelTsmMode_eVideoMaster:
                pcrlibConfig.tsm_mode = BPCRlib_TsmMode_eVideoMaster;
                pcrlibConfig.sync_limit = stcChannel->astm.settings.syncLimit;
                break;
            case NEXUS_StcChannelTsmMode_eOutputMaster:
                pcrlibConfig.tsm_mode = BPCRlib_TsmMode_eOutputMaster;
                pcrlibConfig.sync_limit = 0;
                break;
            default:
            case NEXUS_StcChannelTsmMode_eStcMaster:
                pcrlibConfig.tsm_mode = BPCRlib_TsmMode_eSTCMaster;
                pcrlibConfig.sync_limit = 0;
                break;
        }

        BDBG_MSG(("ASTM is setting the PCRlib tsm mode for stc channel %p to %d", (void *)stcChannel, stcChannel->astm.settings.tsmMode));
    }
#endif

    rc = BPCRlib_Channel_SetConfig(stcChannel->pcrlibChannel, &pcrlibConfig);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

error:
    return rc;
}

static NEXUS_Error setPcrOffsetSettings(
    NEXUS_StcChannelHandle stcChannel,
    const NEXUS_StcChannelSettings * pSettings,
    NEXUS_StcChannelMode mode,
    NEXUS_TransportType transportType)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BXPT_PcrOffset_Settings pcr_offset_settings;
    bool JitterAdjustmentChange = false;

#if B_REFSW_DSS_SUPPORT
    rc = BXPT_DirecTv_PcrOffset_SetPcrMode(stcChannel->pcrOffset, NEXUS_IS_DSS_MODE(transportType)?BXPT_PcrMode_eDirecTv:BXPT_PcrMode_eMpeg);
    if (rc) { rc = BERR_TRACE(rc); goto error; }
#endif

    /* We chose to have PCROFFSET always locked to DPCR. DPCR can freerun or is locked to PCR. */
    rc = BXPT_PcrOffset_FreeRun(stcChannel->pcrOffset, false);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    BXPT_PcrOffset_GetSettings(stcChannel->pcrOffset, &pcr_offset_settings);
    /* default pcr offset settings */
    pcr_offset_settings.OffsetThreshold = 8;
    pcr_offset_settings.MaxPcrError = 0xFF;
    pcr_offset_settings.TimestampDisable = false;
    pcr_offset_settings.CountMode = (NEXUS_IS_DSS_MODE(transportType) ||
        (NEXUS_StcChannel_PcrBits_eLegacy!=pSettings->pcrBits))?BXPT_PcrOffset_StcCountMode_eBinary:BXPT_PcrOffset_StcCountMode_eMod300;
    if (!stcChannel->settings.modeSettings.pcr.disableJitterAdjustment != pcr_offset_settings.EnableJitterAdjustment) {
        pcr_offset_settings.EnableJitterAdjustment = !stcChannel->settings.modeSettings.pcr.disableJitterAdjustment;
        JitterAdjustmentChange = true;
    }

    BDBG_ASSERT(pcr_offset_settings.UsePcrTimeBase == true); /* always use PCR */
    pcr_offset_settings.WhichPcr = stcChannel->timebase->hwIndex;

    switch (mode)
    {
        case NEXUS_StcChannelMode_ePcr:
            {
                NEXUS_P_HwPidChannel *pidChannel;

                pidChannel = pSettings->modeSettings.pcr.pidChannel->hwPidChannel;
                pcr_offset_settings.PidChannelNum = pidChannel->status.pidChannelIndex;
                pcr_offset_settings.UseHostPcrs = false;
                pcr_offset_settings.OffsetThreshold = pSettings->modeSettings.pcr.offsetThreshold;
                pcr_offset_settings.MaxPcrError = pSettings->modeSettings.pcr.maxPcrError;
                pcr_offset_settings.TimestampDisable = pSettings->modeSettings.pcr.disableTimestampCorrection;

                BDBG_MODULE_MSG(nexus_flow_stc_channel, ("%p PCR mode, STC%u, pidChannel %p, offset threshold %d, max pcr error %d, timestamp disable %d, jitter disable %d",
                        (void *)stcChannel, stcChannel->stcIndex, (void *)pidChannel,
                        pcr_offset_settings.OffsetThreshold, pcr_offset_settings.MaxPcrError,
                        pcr_offset_settings.TimestampDisable, pSettings->modeSettings.pcr.disableJitterAdjustment));

            }
            break;

        default:
        case NEXUS_StcChannelMode_eAuto:
        case NEXUS_StcChannelMode_eHost:
            BDBG_MODULE_MSG(nexus_flow_stc_channel, ("%p %s mode, STC%u", (void *)stcChannel, mode==NEXUS_StcChannelMode_eAuto?"auto":"host", stcChannel->stcIndex));
            pcr_offset_settings.PidChannelNum = BXPT_NUM_PID_CHANNELS; /* no PCR pid used */
            pcr_offset_settings.UseHostPcrs = true;

            /* pcrlib will also reassert OFFSET = 0 when it sets STC. */
            rc = BXPT_PcrOffset_SetOffset(stcChannel->pcrOffset, 0);
            if (rc) { rc = BERR_TRACE(rc); goto error; }

            break;
    }

#if BXPT_HAS_TSMUX
    switch (pSettings->pcrBits) {
    default: BERR_TRACE(NEXUS_INVALID_PARAMETER); /* fall through */
    case NEXUS_StcChannel_PcrBits_eLegacy: pcr_offset_settings.BroadcastMode = BXPT_PcrOffset_BroadcastMode_eLegacy; break;
    case NEXUS_StcChannel_PcrBits_eLsb32:  pcr_offset_settings.BroadcastMode = BXPT_PcrOffset_BroadcastMode_eLsb32; break;
    case NEXUS_StcChannel_PcrBits_eMsb32:  pcr_offset_settings.BroadcastMode = BXPT_PcrOffset_BroadcastMode_eMsb32; break;
    case NEXUS_StcChannel_PcrBits_eFull42: pcr_offset_settings.BroadcastMode = BXPT_PcrOffset_BroadcastMode_eFull42; break;
    }
    BDBG_MSG(("STC%u broadcastmode = %u", stcChannel->stcIndex, pcr_offset_settings.BroadcastMode));
#else
    if (pSettings->pcrBits != NEXUS_StcChannel_PcrBits_eLegacy) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }
#endif

    rc = BXPT_PcrOffset_SetSettings(stcChannel->pcrOffset, &pcr_offset_settings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    if (JitterAdjustmentChange) {
        NEXUS_StcChannelPidChannelEntry *e;
        for (e = BLST_Q_FIRST(&stcChannel->pids); e; e = BLST_Q_NEXT(e, link)) {
            BXPT_PcrOffset_ApplyPidChannelSettings(stcChannel->pcrOffset, e->pidChannelIndex);
        }
    }

error:
    return rc;
}

static NEXUS_Error nexus_p_stcchannel_validateSettings(const NEXUS_StcChannelSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    /* if we are requesting pcr mode, ensure pid channel is good */
    if (pSettings->mode == NEXUS_StcChannelMode_ePcr)
    {
        if (NULL == pSettings->modeSettings.pcr.pidChannel)
        {
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error;
        }
        NEXUS_OBJECT_ASSERT(NEXUS_PidChannel, pSettings->modeSettings.pcr.pidChannel);
    }

error:
    return rc;
}

/* do acquire/release on any resource stored in handle->settings */
static void nexus_p_stcchannel_doSettingsAccounting(NEXUS_StcChannelHandle stcChannel, const NEXUS_StcChannelSettings *pSettings)
{
    NEXUS_PidChannelHandle acq;
    NEXUS_PidChannelHandle rel;

    if (pSettings && pSettings->mode == NEXUS_StcChannelMode_ePcr) {
        acq = pSettings->modeSettings.pcr.pidChannel;
    }
    else {
        acq = NULL;
    }
    if (stcChannel->settings.mode == NEXUS_StcChannelMode_ePcr) {
        rel = stcChannel->settings.modeSettings.pcr.pidChannel;
    }
    else {
        rel = NULL;
    }
    if (acq != rel) {
        if (rel) {
            NEXUS_OBJECT_RELEASE(stcChannel, NEXUS_PidChannel, rel);
        }
        if (acq) {
            NEXUS_OBJECT_ACQUIRE(stcChannel, NEXUS_PidChannel, acq);
        }
    }
}

NEXUS_Error NEXUS_StcChannel_SetSettings(NEXUS_StcChannelHandle stcChannel, const NEXUS_StcChannelSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_TransportType transportType;
    NEXUS_StcChannelMode mode;
    NEXUS_TimebaseHandle timebase;

    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

    rc = nexus_p_stcchannel_validateSettings(pSettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    /*
     * This is the release of the previous pcr pid channel from the offset
     * enable pid table, since we unconditionally add the new one to the pid
     * table even if it hasn't changed.  This must occur before nexus_p_stcchannel_doSettingsAccounting
     * so that we don't lose our grip on the old pid channel before we remove
     * it from the context here
     */
    if (stcChannel->settings.mode == NEXUS_StcChannelMode_ePcr)
    {
        NEXUS_StcChannel_DisablePidChannel_priv(stcChannel, stcChannel->settings.modeSettings.pcr.pidChannel);
    }

    nexus_p_stcchannel_doSettingsAccounting(stcChannel, pSettings);

    timebase = NEXUS_Timebase_Resolve_priv(pSettings->timebase);
    if (!timebase) { rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto error; }

    rc = handleIndexChange(stcChannel);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    transportType = NEXUS_StcChannel_P_GetTransportType(pSettings);

    mode = computeMode(stcChannel, pSettings);

    rc = setTimebase(stcChannel, timebase, pSettings, mode);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    rc = setPcrOffsetSettings(stcChannel, pSettings, mode, transportType);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    rc = setPcrlibConfig(stcChannel, pSettings, mode, transportType);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    stcChannel->settings = *pSettings;

    NEXUS_StcChannel_P_SetSwPcrOffsetEnabled(stcChannel);

    /*
     * WARNING: this part must come after settings are copied to the handle,
     * as it uses the mode of the stored settings to disable jitter adjustment
     */
    if (mode == NEXUS_StcChannelMode_ePcr)
    {
        rc = NEXUS_StcChannel_EnablePidChannel_priv(stcChannel, pSettings->modeSettings.pcr.pidChannel);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }

error:
    return rc;
}

NEXUS_Error NEXUS_StcChannel_GetStatus(NEXUS_StcChannelHandle stcChannel, NEXUS_StcChannelStatus * status)
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

    BKNI_Memset(status, 0, sizeof(*status));
#if NEXUS_HAS_ASTM
    if (stcChannel->astm.settings.enabled)
    {
        status->mode = stcChannel->astm.settings.mode;
    }
    else
#endif
    {
        status->mode = stcChannel->settings.mode;
    }
    NEXUS_StcChannel_GetStc(stcChannel, &status->stc);
    status->stcValid = stcChannel->stcValid;

    status->index = stcChannel->index;
    status->stcIndex = stcChannel->stcIndex;
	if(!stcChannel->timebase)
		return BERR_TRACE(NEXUS_INVALID_PARAMETER);
	status->timebaseIndex = stcChannel->timebase->hwIndex;
    return 0;
}

NEXUS_Error NEXUS_StcChannel_Invalidate(NEXUS_StcChannelHandle stcChannel)
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    BDBG_MSG(("%p: stc%u invalid", (void *)stcChannel, stcChannel->stcIndex));
    stcChannel->stcValid = false;
    return BPCRlib_Channel_Invalidate(stcChannel->pcrlibChannel);
}

void NEXUS_StcChannel_GetStc(NEXUS_StcChannelHandle stcChannel, uint32_t *pStc)
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

    if (!stcChannel->swPcrOffsetEnabled) {
        *pStc = BXPT_PcrOffset_GetStc(stcChannel->pcrOffset) + BXPT_PcrOffset_GetOffset(stcChannel->pcrOffset);
    }
    else {
        *pStc = BXPT_PcrOffset_GetStc(stcChannel->pcrOffset) + stcChannel->swPcrOffset;
    }
}

void NEXUS_StcChannel_GetSerialStc_priv( NEXUS_StcChannelHandle stcChannel, uint32_t *pStc )
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    *pStc = BXPT_PcrOffset_GetStc(stcChannel->pcrOffset);
}

void NEXUS_StcChannel_SetPcrOffsetContextAcquireMode_priv(NEXUS_StcChannelHandle stcChannel)
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    BXPT_PcrOffset_Acquire(stcChannel->pcrOffset);
}

static void NEXUS_StcChannel_P_Notify_isr(NEXUS_StcChannelHandle stcChannel, NEXUS_StcChannelDecoderConnectionHandle decoder)
{
    /* this StcChannel is using a SW pcr offset */
    if (decoder->settings.setPcrOffset_isr) {
        (*decoder->settings.setPcrOffset_isr)(decoder->settings.pCallbackContext, stcChannel->swPcrOffset);
    }

    /* tell the all connected decoders that the STC is now valid */
    if (decoder->settings.stcValid_isr) {
        (*decoder->settings.stcValid_isr)(decoder->settings.pCallbackContext);
    }
    decoder->stcValid = true;
}

static void NEXUS_StcChannel_P_SetSwPcrOffset_isr(NEXUS_StcChannelHandle stcChannel, unsigned offset)
{
    NEXUS_StcChannelDecoderConnectionHandle decoder;

    NEXUS_OBJECT_ASSERT(NEXUS_StcChannel, stcChannel);

    /* NRT mode theoretically should have zero TSM error for master; however, the 1/1001 rounding error
     * might cause TSM jittering; add 2 to the SW stc_offset to avoid TSM jitter */
    stcChannel->swPcrOffset = offset;
    BDBG_MSG(("%p HW STC%u=0x%x, SW OFFSET = 0x%08x",
        (void *)stcChannel, stcChannel->stcIndex, BXPT_PcrOffset_GetStc_isr(stcChannel->pcrOffset), stcChannel->swPcrOffset));

    /* do not program the STC. just send the swPcrOffset to all attached decoders */
    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder != NULL; decoder = BLST_Q_NEXT(decoder, link))
    {
        NEXUS_StcChannel_P_Notify_isr(stcChannel, decoder);
    }

    BDBG_MSG(("%p: stc%u valid", (void *)stcChannel, stcChannel->stcIndex));
    stcChannel->stcValid = true;
}

static NEXUS_Error NEXUS_StcChannel_P_SetStc_isr(NEXUS_StcChannelHandle stcChannel, NEXUS_StcChannelDecoderConnectionHandle decoder, uint32_t stc, bool propagate)
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    if (!stcChannel->swPcrOffsetEnabled) {
        /* program the Serial STC directly with this STC value. PCR_OFFSET is zero. */
        uint32_t previousStc = BXPT_PcrOffset_GetStc_isr(stcChannel->pcrOffset);
        int32_t diff;
        unsigned n;
        unsigned offsetThreshold=0;


        switch(stcChannel->settings.mode) {
        case NEXUS_StcChannelMode_ePcr:
            offsetThreshold = stcChannel->settings.modeSettings.pcr.offsetThreshold;
            break;

        case NEXUS_StcChannelMode_eMax:
        case NEXUS_StcChannelMode_eAuto:
            offsetThreshold = stcChannel->settings.modeSettings.Auto.offsetThreshold;
            break;

        case NEXUS_StcChannelMode_eHost:
            offsetThreshold = stcChannel->settings.modeSettings.host.offsetThreshold;
            break;
        /* no default so if there is enum added it would cause compiler time warning */
        }

        /* If the serial STC is already very close, there's no point in resetting it. This
        prevents PTS Errors from raptor which tries to follow a tight TSM threshold. */
        diff = previousStc - stc;
        diff = diff>=0 ? diff:-diff;
        if (diff > (int32_t)offsetThreshold) {
            NEXUS_Error rc;
            rc = BXPT_PcrOffset_SetStc_isr(stcChannel->pcrOffset, stc);
            if (rc) return BERR_TRACE(rc);
            rc = BXPT_PcrOffset_SetOffset_isr(stcChannel->pcrOffset, 0);
            if (rc) return BERR_TRACE(rc);
        }

        /* we have to test every StcChannel in the system because we may have a shared stcIndex. */
        for (n=0;n<BXPT_NUM_PCR_OFFSET_CHANNELS;n++) {
            NEXUS_StcChannelHandle otherStc = pTransport->stcChannel[n];
            if (otherStc && otherStc->stcIndex == stcChannel->stcIndex) {
                NEXUS_StcChannelDecoderConnectionHandle otherDecoder;
                /* allow each one to recalc the swPcrOffset. */
                for (otherDecoder = BLST_Q_FIRST(&otherStc->decoders); otherDecoder != NULL; otherDecoder = BLST_Q_NEXT(otherDecoder, link))
                {
                    if (otherStc != stcChannel) {
                        stcChannel->swPcrOffset = previousStc + stcChannel->swPcrOffset - BXPT_PcrOffset_GetStc_isr(stcChannel->pcrOffset);
                    }
                    else {
                        stcChannel->swPcrOffset = 0;
                    }
                    /* if otherStc == stcChannel, should we notify if otherDecoder != decoder? I believe this is redundant with PCRlib and harmless. */
                    NEXUS_StcChannel_P_Notify_isr(otherStc, otherDecoder);
                }
            }
        }
    }
    else {
        /* NRT mode theoretically should have zero TSM error for master; however, the 1/1001 rounding error
         * might cause TSM jittering; add 2 to the SW stc_offset to avoid TSM jitter */
        stcChannel->swPcrOffset = stc - BXPT_PcrOffset_GetStc_isr(stcChannel->pcrOffset) + 2;
        BDBG_MSG(("%p HW STC%u=0x%x, SW OFFSET = 0x%08x", (void *)stcChannel, stcChannel->stcIndex, stc+2-stcChannel->swPcrOffset, stcChannel->swPcrOffset));
        /* do not program the STC. just recalc the swPcrOffset, either for one or all. */
        if (!decoder) {
            for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder != NULL; decoder = BLST_Q_NEXT(decoder, link))
            {
                NEXUS_StcChannel_P_Notify_isr(stcChannel, decoder);
            }
        }
        else {
            NEXUS_StcChannel_P_Notify_isr(stcChannel, decoder);
        }
        if (propagate)
        {
            /* Notify paired channel, if one exists */
            if (stcChannel->pairedChannel)
            {
                NEXUS_StcChannel_P_SetSwPcrOffset_isr(stcChannel->pairedChannel, stcChannel->swPcrOffset);
            }
        }
    }

    BDBG_MSG(("%p: stc%u valid", (void *)stcChannel, stcChannel->stcIndex));
    stcChannel->stcValid = true;

    return 0;
}

NEXUS_Error NEXUS_StcChannel_SetStc(NEXUS_StcChannelHandle stcChannel, uint32_t stc)
{
    NEXUS_Error rc;
    BKNI_EnterCriticalSection();
    rc = NEXUS_StcChannel_P_SetStc_isr(stcChannel, NULL, stc, true);
    BKNI_LeaveCriticalSection();
    return rc;
}

/*********************************************
* private api
******************/

void NEXUS_StcChannel_GetStc_isr(NEXUS_StcChannelHandle stcChannel, uint32_t *pStc)
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    if (!stcChannel->swPcrOffsetEnabled)
    {
        *pStc = BXPT_PcrOffset_GetStc_isr(stcChannel->pcrOffset) + BXPT_PcrOffset_GetOffset_isr(stcChannel->pcrOffset);
    }
    else
    {
        *pStc = BXPT_PcrOffset_GetStc_isr(stcChannel->pcrOffset) + stcChannel->swPcrOffset;
    }
}

/* We have to manage the callbacks here in order to not export pcrlib callback peculiarities outside NEXUS_StcChannel. */

static BERR_Code NEXUS_StcChannel_GetPtsCallbackWrapper_isr(void *pDecoderContext, BAVC_PTSInfo *pPTSInfo)
{
    NEXUS_StcChannelDecoderConnectionHandle decoder = (NEXUS_StcChannelDecoderConnectionHandle)pDecoderContext;
    return (*decoder->settings.getPts_isr)(decoder->settings.pCallbackContext, pPTSInfo);
}

static BERR_Code NEXUS_StcChannel_GetCdbLevelCallbackWrapper_isr(void *pDecoderContext, unsigned *level)
{
    NEXUS_StcChannelDecoderConnectionHandle decoder = (NEXUS_StcChannelDecoderConnectionHandle)pDecoderContext;
    return (*decoder->settings.getCdbLevel_isr)(decoder->settings.pCallbackContext, level);
}

static BERR_Code NEXUS_StcChannel_GetStcCallbackWrapper_isr(void *pTransportContext, void *pDecoderContext,  uint32_t *pStc)
{
    NEXUS_StcChannelHandle stcChannel = (NEXUS_StcChannelHandle)pTransportContext;
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    BSTD_UNUSED(pDecoderContext);
    NEXUS_StcChannel_GetStc_isr(stcChannel, pStc);
    BDBG_MSG(("NEXUS_StcChannel_GetStcCallbackWrapper_isr returning stc%u 0x%08x", stcChannel->stcIndex, *pStc));
    return 0;
}

static BERR_Code NEXUS_StcChannel_SetStcCallbackWrapper_isr(void *pTransportContext, void *pDecoderContext,  bool dss, uint32_t stc)
{
    NEXUS_StcChannelHandle stcChannel = (NEXUS_StcChannelHandle)pTransportContext;

    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    BSTD_UNUSED(dss);
    BSTD_UNUSED(pDecoderContext);

    BDBG_MSG(("NEXUS_StcChannel_SetStcCallbackWrapper_isr stc%u 0x%08x", stcChannel->stcIndex, stc));
    /* 20111117 SW7425-1772 bandrews - notify all decoders during every pcrlib callback by using NULL decoderState */
    return NEXUS_StcChannel_P_SetStc_isr(stcChannel, NULL, stc, true);
}

static BERR_Code NEXUS_StcChannel_UpdateStcCallbackWrapper_isr(void *pTransportContext, bool is_request_stc)
{
    NEXUS_StcChannelHandle stcChannel = (NEXUS_StcChannelHandle)pTransportContext;
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    BDBG_MSG(("NEXUS_StcChannel_UpdateStcCallbackWrapper_isr is_request_stc%u=%d",stcChannel->stcIndex, is_request_stc));
    if (is_request_stc) {
        /* If PCR_OFFSET block has non-zero OFFSET_THRESHOLD, then it needs
        to be forced to regenerate an offset message to RAVE. Otherwise the decoder
        may lose a pcr_offset_valid message. */
        /* SW7335-1336: need to set pcr offset to acquire, to ensure it regenerates the offset */
        BXPT_PcrOffset_Acquire_isr(stcChannel->pcrOffset);
        BXPT_PcrOffset_RegenOffset_isr(stcChannel->pcrOffset);
    }
    return 0;
}

void NEXUS_StcChannel_GetDefaultDecoderConnectionSettings_priv(NEXUS_StcChannelDecoderConnectionSettings *pSettings)
{
    NEXUS_ASSERT_MODULE();
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_StcChannelDecoderConnectionSettings));
    pSettings->type = NEXUS_StcChannelDecoderType_eMax;
}

#if !BDBG_NO_MSG
static const char * const g_stcChannelTypeStr[NEXUS_StcChannelDecoderType_eMax + 1] = {
    "video",
    "audio",
    NULL
};
#endif

static bool connectedDecodersHaveSwOffsetSetter(NEXUS_StcChannelHandle stcChannel)
{
    bool hasSetter = true;
    NEXUS_StcChannelDecoderConnectionHandle decoder;

    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder != NULL; decoder = BLST_Q_NEXT(decoder, link))
    {
        if (!decoder->settings.setPcrOffset_isr)
        {
            hasSetter = false;
            break;
        }
    }

    return hasSetter;
}

static bool hasAudio(NEXUS_StcChannelHandle stcChannel)
{
    bool hasAudio = false;
    NEXUS_StcChannelDecoderConnectionHandle decoder;

    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder != NULL; decoder = BLST_Q_NEXT(decoder, link))
    {
        if (decoder->settings.type == NEXUS_StcChannelDecoderType_eAudio)
        {
            hasAudio = true;
            break;
        }
    }

    return hasAudio;
}

static void NEXUS_StcChannel_P_SetSwPcrOffsetEnabled(NEXUS_StcChannelHandle stcChannel)
{
    /*
     * swPcrOffset is false if there's an audio decoder (because RAP does not
     * support a swPcrOffset) or if there's a video decoder which doesn't
     * provide the setPcrOffset callback. There should be only one audio program
     * (possibly multichannel, but the same PTS domain) active for a set of
     * mosaics. Also, we set swPcrOffset false for Host mode, since the host
     * is in full control over the hardware stc counter, and in PCR mode we
     * expect to use the hardware PCR offset instead of the sw one.
     */
    bool enabled =
        (stcChannel->settings.mode != NEXUS_StcChannelMode_ePcr)
        &&
        (stcChannel->settings.mode != NEXUS_StcChannelMode_eHost);

#if BXPT_HAS_TSMUX
    if (stcChannel->nonRealTime)
    {
        /* must have STC offset setter or not be connected in NRT mode, can't proceed further if not, therefore assert */
        BDBG_ASSERT(connectedDecodersHaveSwOffsetSetter(stcChannel));
    }
    else
#endif
    {
        /*
         * without NRT, only enable sw pcr offset if:
         * - no audio decoders are connected
         * - 1 or more video decoders are connected
         * - all connected videos have sw pcr offset setter defined
         */
        enabled = enabled
            && !hasAudio(stcChannel)
            && !BLST_Q_EMPTY(&stcChannel->decoders)
            && connectedDecodersHaveSwOffsetSetter(stcChannel);
    }
    if (stcChannel->swPcrOffsetEnabled != enabled) {
        if (!enabled) {
            uint32_t stc;
            /* switching from SW -> HW */
            /* only print the message if we are in live mode, otherwise it might
            be confusing */
            if (stcChannel->settings.mode == NEXUS_StcChannelMode_ePcr)
            {
                BDBG_MSG(("%p Using HW PCR OFFSET%u", (void *)stcChannel, stcChannel->stcIndex));
            }
            else if (stcChannel->settings.mode == NEXUS_StcChannelMode_eHost)
            {
                BDBG_MSG(("%p Using raw HW STC counter%u (no offset)", (void *)stcChannel, stcChannel->stcIndex));
            }
            NEXUS_StcChannel_GetStc(stcChannel, &stc); /* get HW STC + swPcrOffset */
            stcChannel->swPcrOffset = 0;
            stcChannel->swPcrOffsetEnabled = enabled;
            if (stcChannel->stcValid) /* don't jam STC unless we had it valid before */
            {
                NEXUS_StcChannel_SetStc(stcChannel, stc);  /* set HW STC */
            }
        }
        else {
            BDBG_MSG(("%p Using SW STC%u OFFSET", (void *)stcChannel, stcChannel->stcIndex));
            /* switching from HW -> SW */
            stcChannel->swPcrOffset = 0;
            stcChannel->swPcrOffsetEnabled = enabled;
        }
    }
}

#define PCRLIB_MAX_AUDIO_COUNT 3
static bool NEXUS_StcChannel_P_IsDecoderManagedByPcrlib_isr(NEXUS_StcChannelHandle stcChannel, NEXUS_StcChannelDecoderConnectionHandle decoder)
{
    bool managed = false;

    if (decoder->settings.type == NEXUS_StcChannelDecoderType_eVideo)
    {
        managed = true;
    }
    else
    {
        NEXUS_StcChannelDecoderConnectionHandle pcrlibDecoder;
        unsigned count = 0;

        for (pcrlibDecoder = BLST_Q_FIRST(&stcChannel->decoders); pcrlibDecoder != NULL; pcrlibDecoder = BLST_Q_NEXT(pcrlibDecoder, link))
        {
            if (pcrlibDecoder == decoder)
            {
                managed = true;
                break;
            }
            if (pcrlibDecoder->settings.type == NEXUS_StcChannelDecoderType_eAudio)
            {
                count++;
                if (count == PCRLIB_MAX_AUDIO_COUNT) /* pcrlib can only manage PCRLIB_MAX_AUDIO_COUNT audios */
                {
                    break;
                }
            }
        }
    }

    return managed;
}

/*
 * This function reorders audio connections to pcrlib.
 * As such it needs to have the settings already saved into each connector before
 * calling it.
 */
static void NEXUS_StcChannel_P_SetPcrlibAudioCallbacks(NEXUS_StcChannelHandle stcChannel, BPCRlib_Config *config)
{
    NEXUS_StcChannelDecoderConnectionHandle decoder;
    void ** contexts[PCRLIB_MAX_AUDIO_COUNT];
    const BPCRlib_StcDecIface ** interfaces[PCRLIB_MAX_AUDIO_COUNT];
    unsigned i;

    contexts[0] = &config->audio;
    interfaces[0] = &config->audio_iface;
    contexts[1] = &config->secondary_audio;
    interfaces[1] = &config->secondary_audio_iface;
    contexts[2] = &config->tertiary_audio;
    interfaces[2] = &config->tertiary_audio_iface;

    for (i = 0; i < PCRLIB_MAX_AUDIO_COUNT; i++)
    {
        *(contexts[i]) = NULL;
        *(interfaces[i]) = NULL;
    }

    i = 0;
    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); (decoder != NULL) && (i < PCRLIB_MAX_AUDIO_COUNT); decoder = BLST_Q_NEXT(decoder, link))
    {
        if (decoder->settings.type == NEXUS_StcChannelDecoderType_eAudio)
        {
            *(contexts[i]) = decoder;
            *(interfaces[i]) = &decoder->pcrlibInterface;
            i++;
        }
    }
}

#if NEXUS_STC_CHANNEL_DEBUG_DECODER_QUEUE
static void NEXUS_StcChannel_P_PrintDecoderPriorityQueue(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_StcChannelDecoderConnectionHandle ql;

    BDBG_MSG(("stc channel %p priority queue {", stcChannel));
    for (ql = BLST_Q_FIRST(&stcChannel->decoders); ql != NULL; ql = BLST_Q_NEXT(ql, link))
    {
        BDBG_MSG(("%s %p", g_stcChannelTypeStr[ql->settings.type], ql->settings.pCallbackContext));
    }
    BDBG_MSG(("}"));
}
#endif

/* call only after settings have been saved to decoder connector, otherwise priority will be wrong */
static void NEXUS_StcChannel_P_DecoderPriorityQueueInsert(NEXUS_StcChannelHandle stcChannel, NEXUS_StcChannelDecoderConnectionHandle decoder)
{
    NEXUS_StcChannelDecoderConnectionHandle ql;

    for (ql = BLST_Q_FIRST(&stcChannel->decoders); ql != NULL; ql = BLST_Q_NEXT(ql, link))
    {
        if (ql->settings.priority > decoder->settings.priority)
        {
            BLST_Q_INSERT_BEFORE(&stcChannel->decoders, ql, decoder, link);
            break;
        }
    }

    if (!ql)
    {
        BLST_Q_INSERT_TAIL(&stcChannel->decoders, decoder, link);
    }

#if NEXUS_STC_CHANNEL_DEBUG_DECODER_QUEUE
    NEXUS_StcChannel_P_PrintDecoderPriorityQueue(stcChannel);
#endif
}

static void NEXUS_StcChannel_P_DecoderPriorityQueueRemove(NEXUS_StcChannelHandle stcChannel, NEXUS_StcChannelDecoderConnectionHandle decoder)
{
    NEXUS_StcChannelDecoderConnectionHandle ql;

    for (ql = BLST_Q_FIRST(&stcChannel->decoders); ql != NULL; ql = BLST_Q_NEXT(ql, link))
    {
        if (ql == decoder)
        {
            BLST_Q_REMOVE(&stcChannel->decoders, ql, link);
            break;
        }
    }

#if NEXUS_STC_CHANNEL_DEBUG_DECODER_QUEUE
    NEXUS_StcChannel_P_PrintDecoderPriorityQueue(stcChannel);
#endif
}

static void initPcrlibDecoderInterface(BPCRlib_StcDecIface * pPcrlibInterface)
{
    BKNI_Memset(pPcrlibInterface, 0, sizeof(BPCRlib_StcDecIface));
    pPcrlibInterface->getPts = NEXUS_StcChannel_GetPtsCallbackWrapper_isr;
    pPcrlibInterface->getCdbLevel = NEXUS_StcChannel_GetCdbLevelCallbackWrapper_isr;
    pPcrlibInterface->getStc = NEXUS_StcChannel_GetStcCallbackWrapper_isr;
    pPcrlibInterface->setStc = NEXUS_StcChannel_SetStcCallbackWrapper_isr;
    pPcrlibInterface->updateStc = NEXUS_StcChannel_UpdateStcCallbackWrapper_isr;
    pPcrlibInterface->useAuxTrp = true;
}

/*
 * TODO: these are settings that should be applied during decoder connect, since
 * SetSettings always checks changes and defaults may not get applied.  There
 * must be a better way.
 */
static NEXUS_Error setPcrlibOnDecoderConnect(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BPCRlib_Config pcrlibConfig;

    BPCRlib_Channel_GetConfig(stcChannel->pcrlibChannel, &pcrlibConfig);
    pcrlibConfig.aux_transport = stcChannel;
    /* TODO: playback is set both on decoder connect and on StcChannel_SetSettings call, consolidate */
    pcrlibConfig.playback = (stcChannel->settings.mode != NEXUS_StcChannelMode_ePcr);
#if NEXUS_HAS_ASTM
    if (stcChannel->astm.settings.enabled)
    {
        BDBG_MSG(("ASTM is %s PCRlib playback mode for stc channel %p", (stcChannel->astm.settings.mode != NEXUS_StcChannelMode_ePcr)?"enabling":"disabling", (void *)stcChannel));
        pcrlibConfig.playback = (stcChannel->astm.settings.mode != NEXUS_StcChannelMode_ePcr);
    }
#endif
    pcrlibConfig.pcr_offset = 0;

    /* TODO: stream is set both on decoder connect and on StcChannel_SetSettings call, consolidate */
    rc = NEXUS_P_TransportType_ToMagnum_isrsafe(NEXUS_StcChannel_P_GetTransportType(&stcChannel->settings), &pcrlibConfig.stream);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = BPCRlib_Channel_SetConfig(stcChannel->pcrlibChannel, &pcrlibConfig);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

/* requires settings to be saved to decoder connector already */
static NEXUS_Error connectPcrlibDecoder(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_StcChannelDecoderConnectionHandle decoder)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BPCRlib_Config pcrlibConfig;

    BPCRlib_Channel_GetConfig(stcChannel->pcrlibChannel, &pcrlibConfig);

    switch (decoder->settings.type) {
    case NEXUS_StcChannelDecoderType_eVideo:
        pcrlibConfig.video = decoder;
        pcrlibConfig.video_iface = &decoder->pcrlibInterface;
        break;
    case NEXUS_StcChannelDecoderType_eAudio:
        NEXUS_StcChannel_P_SetPcrlibAudioCallbacks(stcChannel, &pcrlibConfig);
        break;
    default:
        BDBG_ERR(("invalid decoder type"));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    rc = BPCRlib_Channel_SetConfig(stcChannel->pcrlibChannel, &pcrlibConfig);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

/*
 * requires settings to reflect decoder type and priority you want to remove
 * not the new settings you may be trying to add
 */
static void disconnectPcrlibDecoder(NEXUS_StcChannelHandle stcChannel, NEXUS_StcChannelDecoderConnectionHandle decoder)
{
    BPCRlib_Config pcrlibConfig;
    BERR_Code rc = BERR_SUCCESS;

    BPCRlib_Channel_GetConfig(stcChannel->pcrlibChannel, &pcrlibConfig);

    switch (decoder->settings.type) {
    case NEXUS_StcChannelDecoderType_eVideo:
        pcrlibConfig.video = NULL;
        pcrlibConfig.video_iface = NULL;
        break;
    case NEXUS_StcChannelDecoderType_eAudio:
        NEXUS_StcChannel_P_SetPcrlibAudioCallbacks(stcChannel, &pcrlibConfig);
        break;
    default:
        return;
    }

    rc = BPCRlib_Channel_SetConfig(stcChannel->pcrlibChannel, &pcrlibConfig);
    if (rc) { rc = BERR_TRACE(rc); }
}

NEXUS_StcChannelDecoderConnectionHandle NEXUS_StcChannel_ConnectDecoder_priv(
    NEXUS_StcChannelHandle stcChannel,
    const NEXUS_StcChannelDecoderConnectionSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelDecoderConnectionHandle decoder = NULL;

    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    NEXUS_ASSERT_MODULE();

    decoder = (NEXUS_StcChannelDecoderConnectionHandle)BKNI_Malloc(sizeof(struct NEXUS_StcChannelDecoderConnection));
    if (!decoder) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }

    BKNI_Memset(decoder, 0, sizeof(struct NEXUS_StcChannelDecoderConnection));
    BDBG_OBJECT_SET(decoder, NEXUS_StcChannelDecoderConnection);
    decoder->settings.type = NEXUS_StcChannelDecoderType_eMax;

    decoder->parent = stcChannel;

    initPcrlibDecoderInterface(&decoder->pcrlibInterface);

    rc = setPcrlibOnDecoderConnect(stcChannel);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    rc = NEXUS_StcChannel_P_SetDecoderConnectionSettings(decoder, pSettings, true);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    return decoder;

error:
    if (decoder)
    {
        NEXUS_StcChannel_DisconnectDecoder_priv(decoder);
    }
    return NULL;
}

void NEXUS_StcChannel_DisconnectDecoder_priv(NEXUS_StcChannelDecoderConnectionHandle decoder)
{
    NEXUS_ASSERT_MODULE();

    if (decoder)
    {
        NEXUS_StcChannelHandle stcChannel;

        BDBG_OBJECT_ASSERT(decoder, NEXUS_StcChannelDecoderConnection);

        stcChannel = decoder->parent;
        BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

        NEXUS_StcChannel_P_DecoderPriorityQueueRemove(stcChannel, decoder);

        /* audio reorder relies on priority queue state */
        disconnectPcrlibDecoder(stcChannel, decoder);

        if (decoder->settings.type == NEXUS_StcChannelDecoderType_eAudio)
        {
            /* depends on lack of audio connections */
            NEXUS_StcChannel_P_SetSwPcrOffsetEnabled(stcChannel);
        }

        BKNI_Free(decoder);

        /* we may be able to apply an STC counter index change now */
        handleIndexChange(stcChannel);

        /* if we have no more connected decoders, even in playback mode,
         * invalidate the PCR OFFSET so if we start up again in PCR mode we don't
         * get a valid offset sent to the decoders without any PCRs
         * having arrived (or in the case of high threshold, something
         * that is close to the right timebase, but not quite close enough)
         * it is not necessary to set the value of the offset, invalidating it
         * is enough
         */
        if (BLST_Q_EMPTY(&stcChannel->decoders))
        {
            BXPT_PcrOffset_ForceInvalid(stcChannel->pcrOffset);
        }
    }
}

static NEXUS_StcChannelDecoderConnectionHandle getVideoConnector(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_StcChannelDecoderConnectionHandle video = NULL;
    NEXUS_StcChannelDecoderConnectionHandle decoder;

    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder; decoder = BLST_Q_NEXT(decoder, link))
    {
        if (decoder->settings.type == NEXUS_StcChannelDecoderType_eVideo)
        {
            video = decoder;
            break;
        }
    }

    return video;
}

static NEXUS_StcChannelDecoderConnectionHandle getAudioConnector(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_StcChannelDecoderConnectionHandle audio = NULL;
    NEXUS_StcChannelDecoderConnectionHandle decoder;

    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder; decoder = BLST_Q_NEXT(decoder, link))
    {
        if (decoder->settings.type == NEXUS_StcChannelDecoderType_eAudio)
        {
                audio = decoder;
                break;
        }
    }

    return audio;
}

static NEXUS_StcChannelDecoderConnectionHandle findLowLatencyDecoder(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_StcChannelDecoderConnectionHandle llDecoder = NULL;
    NEXUS_StcChannelDecoderConnectionHandle decoder;
    unsigned priority = 0xffffffff;

    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder; decoder = BLST_Q_NEXT(decoder, link))
    {
        if (decoder->settings.lowLatencyEnabled && decoder->settings.priority < priority)
        {
            llDecoder = decoder;
            priority = decoder->settings.priority;
        }
    }

    return llDecoder;
}

static NEXUS_Error NEXUS_StcChannel_P_RegenStcRequest(NEXUS_StcChannelDecoderConnectionHandle decoder)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelHandle stcChannel;
    BAVC_PTSInfo pts;

    if (decoder)
    {
        stcChannel = decoder->parent;
        BDBG_MSG(("Regenerating an STC request on behalf of %s for stc%u channel %p",
            g_stcChannelTypeStr[decoder->settings.type], stcChannel->stcIndex, (void *)stcChannel));
        BKNI_EnterCriticalSection();
        NEXUS_StcChannel_GetPtsCallbackWrapper_isr(decoder, &pts);
        rc = NEXUS_StcChannel_RequestStc_isr(decoder, &pts);
        BKNI_LeaveCriticalSection();
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }

error:
    return rc;
}

static NEXUS_Error setLowLatencyDecoder(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelDecoderConnectionHandle oldDecoder = stcChannel->lowLatencyDecoder;
    NEXUS_StcChannelDecoderConnectionHandle newDecoder = NULL;

    newDecoder = findLowLatencyDecoder(stcChannel);

    /* switching into low latency mode or switching which decoder is low latency master does nothing until a request comes in */
    stcChannel->lowLatencyDecoder = newDecoder;

    /* however, if we are switching out of low latency mode... */
    if (oldDecoder && !newDecoder)
    {
        NEXUS_StcChannelDecoderConnectionHandle video;
        NEXUS_StcChannelDecoderConnectionHandle p1audio;

        switch (stcChannel->settings.mode)
        {
            case NEXUS_StcChannelMode_ePcr:
                /* regen PCR offset */
                BKNI_EnterCriticalSection();
                NEXUS_StcChannel_UpdateStcCallbackWrapper_isr(stcChannel, true);
                BKNI_LeaveCriticalSection();
                break;
            case NEXUS_StcChannelMode_eAuto:
                switch (stcChannel->settings.modeSettings.Auto.behavior)
                {
                    case NEXUS_StcChannelAutoModeBehavior_eFirstAvailable:
                        video = getVideoConnector(stcChannel);
                        p1audio = getAudioConnector(stcChannel);
                        /* audio, then video, to mimic normal streams */
                        rc = NEXUS_StcChannel_P_RegenStcRequest(p1audio);
                        if (rc) goto error;
                        rc = NEXUS_StcChannel_P_RegenStcRequest(video);
                        break;
                    case NEXUS_StcChannelAutoModeBehavior_eVideoMaster:
                        if (oldDecoder->settings.type == NEXUS_StcChannelDecoderType_eAudio)
                        {
                            video = getVideoConnector(stcChannel);
                            if (video)
                            {
                                rc = NEXUS_StcChannel_P_RegenStcRequest(video);
                            }
                        }
                        break;
                    case NEXUS_StcChannelAutoModeBehavior_eAudioMaster:
                        if (oldDecoder->settings.type == NEXUS_StcChannelDecoderType_eVideo)
                        {
                            p1audio = getAudioConnector(stcChannel);
                            if (p1audio)
                            {
                                rc = NEXUS_StcChannel_P_RegenStcRequest(p1audio);
                            }
                        }
                        break;
                    default:
                        /* do nothing for output master */
                        break;
                }
                break;
            default:
                /* do nothing special for host mode */
                break;
        }
    }

error:
    return rc;
}

static NEXUS_Error NEXUS_StcChannel_P_SetDecoderConnectionSettings(NEXUS_StcChannelDecoderConnectionHandle decoder, const NEXUS_StcChannelDecoderConnectionSettings *pSettings, bool force)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (decoder && pSettings)
    {
        bool priorityChanged = false;
        bool typeChanged = false;
        bool setPcrOffsetChanged = false;
        bool lowLatencyChanged = false;
        bool pcrlibReconnectRequired = false;
        NEXUS_StcChannelHandle stcChannel;

        BDBG_OBJECT_ASSERT(decoder, NEXUS_StcChannelDecoderConnection);

        stcChannel = decoder->parent;

        BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

        if (pSettings->priority != decoder->settings.priority)
        {
            priorityChanged = true;
        }

        if (pSettings->type != decoder->settings.type)
        {
            typeChanged = true;
        }

        if (pSettings->setPcrOffset_isr != decoder->settings.setPcrOffset_isr)
        {
            setPcrOffsetChanged = true;
        }

        if (pSettings->lowLatencyEnabled != decoder->settings.lowLatencyEnabled)
        {
            lowLatencyChanged = true;
        }

        pcrlibReconnectRequired = (priorityChanged && (decoder->settings.type == NEXUS_StcChannelDecoderType_eAudio))
                ||
                typeChanged
                ||
                force;

        if (pcrlibReconnectRequired)
        {
            /* disconnect with old settings */
            disconnectPcrlibDecoder(stcChannel, decoder);
        }

        /* copy settings */
        decoder->settings = *pSettings;

        if (priorityChanged || force)
        {
            NEXUS_StcChannel_P_DecoderPriorityQueueRemove(stcChannel, decoder);
            NEXUS_StcChannel_P_DecoderPriorityQueueInsert(stcChannel, decoder);
        }

        if (pcrlibReconnectRequired)
        {
            /* connect with new settings */
            rc = connectPcrlibDecoder(stcChannel, decoder);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
        }

        if (setPcrOffsetChanged || force)
        {
            NEXUS_StcChannel_P_SetSwPcrOffsetEnabled(stcChannel);
        }

        if (lowLatencyChanged || force)
        {
            rc = setLowLatencyDecoder(stcChannel);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
        }
    }

end:
    return rc;
}

NEXUS_Error NEXUS_StcChannel_RequestStc_isr(NEXUS_StcChannelDecoderConnectionHandle decoder, const struct BAVC_PTSInfo *pPts)
{
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_StcChannelDecoderConnection);
    stcChannel = decoder->parent;
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

    if (stcChannel->settings.mode == NEXUS_StcChannelMode_eHost)
    {
        /* in eHost mode, STC request interrupts do not set the STC,
        but we must ensure decoders think the STC is valid when it is
        set before they start */
        if (stcChannel->stcValid)
        {
            NEXUS_StcChannel_P_Notify_isr(stcChannel, decoder);
        }

        return 0;
    }

    /* handle unmanaged decoders first */
    if (!NEXUS_StcChannel_P_IsDecoderManagedByPcrlib_isr(stcChannel, decoder))
    {
        BDBG_MSG(("Unmanaged %s decoder STC request", g_stcChannelTypeStr[decoder->settings.type]));
        if (stcChannel->stcValid)
        {
            NEXUS_StcChannel_P_Notify_isr(stcChannel, decoder);
        }
        return 0;
    }

    if (!stcChannel->lowLatencyDecoder || (stcChannel->lowLatencyDecoder == decoder))
    {
        switch (decoder->settings.type) {
        case NEXUS_StcChannelDecoderType_eVideo:
            rc = BPCRlib_Channel_VideoRequestStc_isr(pTransport->pcrlib, decoder, pPts);
            break;
        case NEXUS_StcChannelDecoderType_eAudio:
            rc = BPCRlib_Channel_AudioRequestStc_isr(pTransport->pcrlib, decoder, pPts->ui32CurrentPTS);
            break;
        default:
            BDBG_ERR(("invalid decoder type"));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            break;
        }
    }
    else
    {
        if (stcChannel->stcValid)
        {
            NEXUS_StcChannel_P_Notify_isr(stcChannel, decoder);
        }
    }

    return rc;
}

NEXUS_Error NEXUS_StcChannel_PtsError_isr(NEXUS_StcChannelDecoderConnectionHandle decoder, const struct BAVC_PTSInfo *pPts)
{
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t stc;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_StcChannelDecoderConnection);
    stcChannel = decoder->parent;
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

    NEXUS_StcChannel_GetStc_isr(stcChannel, &stc);

    if (stcChannel->settings.mode == NEXUS_StcChannelMode_eHost
        || stcChannel->lowLatencyDecoder)
    {
        /* in eHost mode or low latency mode, PTS interrupts do not set the STC */
        return 0;
    }

    /* handle unmanaged decoders first */
    if (!NEXUS_StcChannel_P_IsDecoderManagedByPcrlib_isr(stcChannel, decoder))
    {
/*        BDBG_MSG(("Unmanaged %s decoder PTS error", g_stcChannelTypeStr[decoder->settings.type]));*/
        return 0;
    }

    switch (decoder->settings.type) {
    case NEXUS_StcChannelDecoderType_eVideo:
        rc = BPCRlib_Channel_VideoPtsError_isr(pTransport->pcrlib, decoder, pPts, stc);
        break;
    case NEXUS_StcChannelDecoderType_eAudio:
        rc = BPCRlib_Channel_AudioPtsError_isr(pTransport->pcrlib, decoder, pPts, stc);
        break;
    default:
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    }
    return rc;
}

static NEXUS_StcChannelPidChannelEntry * NEXUS_StcChannel_P_FindPidChannelEntry(NEXUS_StcChannelHandle stcChannel, NEXUS_PidChannelHandle pidChannel)
{
    NEXUS_StcChannelPidChannelEntry * scpce = NULL;
    NEXUS_StcChannelPidChannelEntry * e;

    if (!pidChannel) goto end;

    for (e = BLST_Q_FIRST(&stcChannel->pids); e; e = BLST_Q_NEXT(e, link))
    {
        BDBG_ASSERT(e->refcnt);
        if (e->pidChannelIndex == PID_CHANNEL_INDEX(pidChannel))
        {
            scpce = e;
            break;
        }
    }

end:
    return scpce;
}

static bool NEXUS_StcChannel_P_AddPidChannel(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_PidChannelHandle pidChannel
)
{
    bool enable = false;
    NEXUS_StcChannelPidChannelEntry * e;
    e = NEXUS_StcChannel_P_FindPidChannelEntry(stcChannel, pidChannel);
    if (!e)
    {
        e = BKNI_Malloc(sizeof(*e));
        if (!e) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto end; }
        BKNI_Memset(e, 0, sizeof(*e));
        BDBG_ASSERT(pidChannel);
        e->pidChannelIndex = PID_CHANNEL_INDEX(pidChannel);
        BLST_Q_INSERT_TAIL(&stcChannel->pids, e, link);
        enable = true;
        BDBG_MSG_TRACE(("Added pid refcnt entry for pidch: %u (%u)", PID_CHANNEL_INDEX(pidChannel), PID_CHANNEL_PID(pidChannel)));
    }
    e->refcnt++;
    BDBG_MSG_TRACE(("add pidch: %u (%u) refcnt: %u", e->pidChannelIndex, PID_CHANNEL_PID(pidChannel), e->refcnt));

end:
    return enable;
}

static bool NEXUS_StcChannel_P_RemovePidChannel(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_PidChannelHandle pidChannel
)
{
    bool disable = false;
    NEXUS_StcChannelPidChannelEntry * e;

    e = NEXUS_StcChannel_P_FindPidChannelEntry(stcChannel, pidChannel);
    if (e)
    {
        BDBG_ASSERT(e->refcnt);
        e->refcnt--;
        BDBG_MSG_TRACE(("remove pidch: %u (%u) refcnt: %u", e->pidChannelIndex, PID_CHANNEL_PID(pidChannel), e->refcnt));
        if (!e->refcnt)
        {
            BLST_Q_REMOVE(&stcChannel->pids, e, link);
            BKNI_Free(e);
            disable = true;
            BDBG_MSG_TRACE(("Removed pid refcnt entry for pidch: %u (%u)", PID_CHANNEL_INDEX(pidChannel), PID_CHANNEL_PID(pidChannel)));
        }
    }

    return disable;
}

/*
Enable Output to a specified PID Channel
*/
BERR_Code NEXUS_StcChannel_EnablePidChannel_priv(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_PidChannelHandle pidChannel
    )
{
#ifdef BXPT_HAS_MOSAIC_SUPPORT
    BERR_Code rc;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(pidChannel, NEXUS_PidChannel);
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

    if (NEXUS_StcChannel_P_AddPidChannel(stcChannel, pidChannel))
    {
        rc = BXPT_PcrOffset_EnableOffset(stcChannel->pcrOffset, PID_CHANNEL_INDEX(pidChannel));
        if (rc) { (void)NEXUS_StcChannel_P_RemovePidChannel(stcChannel, pidChannel); return BERR_TRACE(rc);}
    }
#else
    BSTD_UNUSED(stcChannel);
    BSTD_UNUSED(pidChannel);
    BSTD_UNUSED(NEXUS_StcChannel_P_AddPidChannel);
#endif
    return BERR_SUCCESS;
}

/*
Disable Output to a specified PID Channel
*/
void NEXUS_StcChannel_DisablePidChannel_priv(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_PidChannelHandle pidChannel
    )
{
#ifdef BXPT_HAS_MOSAIC_SUPPORT
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    BDBG_OBJECT_ASSERT(pidChannel, NEXUS_PidChannel);

    if (NEXUS_StcChannel_P_RemovePidChannel(stcChannel, pidChannel))
    {
        BXPT_PcrOffset_DisableOffset(stcChannel->pcrOffset, PID_CHANNEL_INDEX(pidChannel));
    }
#else
    BSTD_UNUSED(stcChannel);
    BSTD_UNUSED(pidChannel);
    BSTD_UNUSED(NEXUS_StcChannel_P_RemovePidChannel);
#endif
}

void NEXUS_StcChannel_GetIndex_priv( NEXUS_StcChannelHandle stcChannel, unsigned *pIndex )
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    NEXUS_ASSERT_MODULE();
    *pIndex = stcChannel->stcIndex;
}

NEXUS_Error NEXUS_StcChannel_Freeze( NEXUS_StcChannelHandle stcChannel, bool frozen )
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    stcChannel->frozen = frozen;
    return BXPT_PcrOffset_FreezeStc( stcChannel->pcrOffset, frozen);
}

bool NEXUS_StcChannel_IsFrozen_priv( NEXUS_StcChannelHandle stcChannel)
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    return stcChannel->frozen;
}

void NEXUS_StcChannel_GetRate_priv(NEXUS_StcChannelHandle stcChannel, unsigned * increment, unsigned * prescale)
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    *increment = stcChannel->increment;
    *prescale = stcChannel->prescale;
}

NEXUS_Error NEXUS_StcChannel_SetRate( NEXUS_StcChannelHandle stcChannel, unsigned Increment, unsigned Prescale )
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

    /* STC_CTRL only has 8 bit resolution. So it cannot take values more than 256, so scale down. */
    while (Increment > 255 || Prescale > 255) {
        Increment /= 2;
        Prescale /= 2;
    }
    stcChannel->isStcTrick = (Increment != Prescale + 1);
    stcChannel->increment = Increment;
    stcChannel->prescale = Prescale;

    BDBG_MSG(("BXPT_PcrOffset_ChangeStcRate %d %d", Increment, Prescale));
    return BXPT_PcrOffset_ChangeStcRate( stcChannel->pcrOffset, Increment, Prescale );
}

static bool NEXUS_StcChannel_P_PerformAvWindowStallDetection(NEXUS_StcChannelHandle thisChannel, bool * wait)
{
    bool stalled = false;
#if BXPT_HAS_TSMUX
    bool thisCounterMoved, thatCounterMoved;
    NEXUS_StcChannelHandle thatChannel;
    NEXUS_StcChannelHandle stallingChannel;
    uint64_t thisCurrentCounterValue;
    uint64_t thatCurrentCounterValue;
    uint64_t thisIncrement;
    uint64_t thatIncrement;
    uint64_t diff;
    uint64_t avWindowTicks;
    uint64_t thisConverter;
    uint64_t thatConverter;
    BXPT_PcrOffset_Settings theseSettings;
    BXPT_PcrOffset_Settings thoseSettings;
    BXPT_PcrOffset_NRTConfig thisNrtConfig;
    BXPT_PcrOffset_NRTConfig thatNrtConfig;

    BDBG_MSG_TRACE(("stcChannel%u %p: Performing av window stall detection...", thisChannel->stcIndex, thisChannel));

    thisCounterMoved = false;
    thatCounterMoved = false;

    thatChannel = thisChannel->pairedChannel;

    BXPT_PcrOffset_GetSettings(thisChannel->pcrOffset, &theseSettings);
    BXPT_PcrOffset_GetSettings(thatChannel->pcrOffset, &thoseSettings);

    BXPT_PcrOffset_GetNRTConfig(thisChannel->pcrOffset, &thisNrtConfig);
    BXPT_PcrOffset_GetNRTConfig(thatChannel->pcrOffset, &thatNrtConfig);

    if (theseSettings.CountMode != BXPT_PcrOffset_StcCountMode_eBinary)
    {
        thisConverter = 600; /* 45 KHz to 27 MHz conversion */
        thisIncrement = (thisNrtConfig.StcIncrement >> 9) * 300 + (thisNrtConfig.StcIncrement & 0x00000000000001FF);
    }
    else
    {
        thisConverter = 1;
        thisIncrement = thisNrtConfig.StcIncrement;
    }

    if (thoseSettings.CountMode != BXPT_PcrOffset_StcCountMode_eBinary)
    {
        thatConverter = 600; /* 45 KHz to 27 MHz conversion */
        thatIncrement = (thatNrtConfig.StcIncrement >> 9) * 300 + (thatNrtConfig.StcIncrement & 0x00000000000001FF);
    }
    else
    {
        thatConverter = 1;
        thatIncrement = thatNrtConfig.StcIncrement;
    }

    BDBG_MSG_TRACE(("stcChannel%u %p: thisConverter = %llu; thatConverter = %llu", thisChannel->stcIndex, thisChannel, thisConverter, thatConverter));

    /* convert everything to 27 MHz */

    thisCurrentCounterValue = BXPT_PcrOffset_GetStc(thisChannel->pcrOffset) * thisConverter;
    thatCurrentCounterValue = BXPT_PcrOffset_GetStc(thatChannel->pcrOffset) * thatConverter;

    BDBG_MSG_TRACE(("stcChannel%u %p: thisIncrement = %llu; thatIncrement = %llu", thisChannel->stcIndex, thisChannel, thisIncrement, thatIncrement));
    BDBG_MSG_TRACE(("stcChannel%u %p: thisCCV = %llu; thatCCV = %llu", thisChannel->stcIndex, thisChannel, thisCurrentCounterValue, thatCurrentCounterValue));
    BDBG_MSG_TRACE(("stcChannel%u %p: this->lCCVal = %s; that->lCCVal = %s",
        thisChannel->stcIndex, thisChannel,
        thisChannel->lastCounterValueValid ? "true" : "false",
        thatChannel->lastCounterValueValid ? "true" : "false"));

    if (thisChannel->lastCounterValueValid && thatChannel->lastCounterValueValid)
    {
        thisCounterMoved = thisCurrentCounterValue != thisChannel->lastCounterValue;
        thatCounterMoved = thatCurrentCounterValue != thatChannel->lastCounterValue;

        BDBG_MSG_TRACE(("stcChannel%u %p: thisCMov = %s; this->lCV = %llu; thatCMov = %s; that->lCV = %llu",
            thisChannel->stcIndex, thisChannel,
            thisCounterMoved ? "true" : "false",
            thisChannel->lastCounterValue,
            thatCounterMoved ? "true" : "false",
            thatChannel->lastCounterValue));

        if (!thisCounterMoved && !thatCounterMoved)
        {
            /* both STC counters are stalled */

            BDBG_MSG_TRACE(("stcChannel%u %p: thisAVW = %u; thatAVW = %u", thisChannel->stcIndex, thisChannel, thisNrtConfig.AvWindow, thatNrtConfig.AvWindow));

            if (thisNrtConfig.AvWindow == thatNrtConfig.AvWindow)
            {
                /* AV window milliseconds to 27 MHz clock tick conversion */
                avWindowTicks = (uint64_t)thisNrtConfig.AvWindow * 27000;

                BDBG_MSG_TRACE(("stcChannel%u %p: AVWTicks = %llu", thisChannel->stcIndex, thisChannel, avWindowTicks));

                /* check if (this STC + 1 increment - that STC) >= AV window or vice versa */

                if (thisCurrentCounterValue >= thatCurrentCounterValue)
                {
                    diff = thisCurrentCounterValue + thisIncrement - thatCurrentCounterValue;
                    stallingChannel = thatChannel;
                }
                else
                {
                    diff = thatCurrentCounterValue + thatIncrement - thisCurrentCounterValue;
                    stallingChannel = thisChannel;
                }

                BDBG_MSG_TRACE(("stcChannel%u %p: diff = %llu; stallingCh = %p", thisChannel->stcIndex, thisChannel, diff, stallingChannel));

                if (diff >= avWindowTicks)
                {
                    BDBG_MSG_TRACE(("stcChannel%u %p: this->lSSC = %p", thisChannel->stcIndex, thisChannel, thisChannel->lastStallingStcChannel));

                    /* this condition means that both decoders are stalled and the STCs are stalled because of av window reached */
                    if (!thisChannel->lastStallingStcChannel || thisChannel->lastStallingStcChannel == stallingChannel)
                    {
                        /* either this is our first stall, or the last stall was caused by the same channel */
                        thisChannel->lastStallingStcChannel = stallingChannel;
                        thatChannel->lastStallingStcChannel = stallingChannel;
                        /*
                         * we used to say we are stalled if any channel is stalled
                         * but now we only say we are stalled if we are not the
                         * stalling channel
                         */
                        stalled = stallingChannel != thisChannel;
                        BDBG_MSG(("stcChannel%u %p: AV window stall detected.  Stalling STC channel is %p", thisChannel->stcIndex, (void *)thisChannel, (void *)stallingChannel));
                    }
                    else /* (thisChannel->lastStallingStcChannel != stallingChannel) */
                    {
                        /* we've switched stalling channels, this is likely a deadlock, not a gap */
                        BDBG_MSG(("stcChannel%u %p: Stalling STC channel switch detected (%p -> %p), likely deadlock", thisChannel->stcIndex, (void *)thisChannel, (void *)thisChannel->lastStallingStcChannel, (void *)stallingChannel));
                        thisChannel->lastStallingStcChannel = NULL; /* reset for next detection cycle */
                        thatChannel->lastStallingStcChannel = NULL;
                        stalled = false;
                    }
                }
                else
                {
                    BDBG_MSG_TRACE(("stcChannel%u %p: diff < AV window; no stall", thisChannel->stcIndex, (void *)thisChannel));
                }
            }
            else
            {
                BDBG_ERR(("stcChannel%u %p: Paired STC channels with different AV windows", thisChannel->stcIndex, (void *)thisChannel));
                goto end;
            }
        }
        else if (thisCounterMoved && thatCounterMoved)
        {
            BDBG_MSG_TRACE(("stcChannel%u %p: Both paired channel STC counters moved, resetting stall detection", thisChannel->stcIndex, thisChannel));
            /* if both counters move, we are not stalled any more, need to reset */
            /* this is not true if only one counter moves, like from attemped ZF during true deadlock */
            thisChannel->lastStallingStcChannel = NULL;
            thatChannel->lastStallingStcChannel = NULL;
        }
        else
        {
            BDBG_MSG_TRACE(("stcChannel%u %p: %s counter moved; no stall", thisChannel->stcIndex, thisChannel, thisCounterMoved ? "this" : "that"));
        }
    }
    else
    {
        *wait = true;
    }

    thisChannel->lastCounterValue = thisCurrentCounterValue;
    thisChannel->lastCounterValueValid = true;
    thatChannel->lastCounterValue = thatCurrentCounterValue;
    thatChannel->lastCounterValueValid = true;

end:
#else
    BSTD_UNUSED(thisChannel);
    *wait = false;
#endif
    return stalled;
}

typedef struct NEXUS_StcChannelDeadlockDetectionInfo
{
    bool allHung;
    bool oneEmpty;
    bool oneFull;
    bool tsmWaiting;
} NEXUS_StcChannelDeadlockDetectionInfo;

static void NEXUS_StcChannel_P_InitDeadlockDetectionInfo(NEXUS_StcChannelDeadlockDetectionInfo * pInfo)
{
    BKNI_Memset(pInfo, 0, sizeof(NEXUS_StcChannelDeadlockDetectionInfo));
    pInfo->allHung = true;
}

static void NEXUS_StcChannel_P_PerformDeadlockDetection(NEXUS_StcChannelHandle stcChannel, NEXUS_StcChannelDeadlockDetectionInfo * pInfo)
{
    NEXUS_StcChannelDecoderConnectionHandle decoder;

    NEXUS_StcChannel_P_InitDeadlockDetectionInfo(pInfo);

    BDBG_MSG_TRACE(("stcChannel%u %p: Performing deadlock detection...", stcChannel->stcIndex, stcChannel));

    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder; decoder = BLST_Q_NEXT(decoder, link))
    {
        if (decoder->fifoWatchdogStatus.tsmWait)
        {
            pInfo->tsmWaiting = true;
        }
        if (decoder->fifoWatchdogStatus.percentFull > 95) {
            pInfo->oneFull = true;
        }
        if (decoder->fifoWatchdogStatus.percentFull < 5) {
            pInfo->oneEmpty = true;
        }
        if (!decoder->fifoWatchdogStatus.isHung) {
            pInfo->allHung = false;
        }
        else {
            BDBG_MSG(("stcChannel%u %p, %s fifo is static (%d%% full)", stcChannel->stcIndex, (void *)stcChannel, g_stcChannelTypeStr[decoder->settings.type], decoder->fifoWatchdogStatus.percentFull));
        }
    }
}

static NEXUS_StcChannelDecoderConnectionHandle NEXUS_StcChannel_P_FindUnlockedDecoder(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_StcChannelDecoderConnectionHandle unlocked = NULL;
    NEXUS_StcChannelDecoderConnectionHandle decoder;

    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder; decoder = BLST_Q_NEXT(decoder, link))
    {
        /* check connected decoders for frame sync unlock state */
        if (decoder->fifoWatchdogStatus.frameSyncUnlocked)
        {
            unlocked = decoder;
            break;
        }
    }

    return unlocked;
}

void NEXUS_StcChannel_GetDefaultDecoderFifoWatchdogStatus_priv(NEXUS_StcChannelDecoderFifoWatchdogStatus *pStatus)
{
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
}

static void NEXUS_StcChannel_P_NotifyStatusUpdate(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_StcChannelDecoderConnectionHandle decoder;

    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder; decoder = BLST_Q_NEXT(decoder, link))
    {
        if (decoder->settings.statusUpdateEvent)
        {
            BKNI_SetEvent(decoder->settings.statusUpdateEvent);
        }
    }
}

static void NEXUS_StcChannel_P_RequestConnectedZeroFill(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_StcChannelDecoderConnectionHandle decoder;

    /* zero fill all decoders connected to this STC channel */
    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder; decoder = BLST_Q_NEXT(decoder, link))
    {
        decoder->status.zeroFill = true;
    }
}

static void NEXUS_StcChannel_P_ReportPeerGap(
    NEXUS_StcChannelHandle stcChannel
    )
{
    if (stcChannel->pairedChannel)
    {
        BDBG_MSG(("stcChannel%u %p: Gap reported by peer stcChannel%u %p", stcChannel->stcIndex, (void *)stcChannel, stcChannel->pairedChannel->stcIndex, (void *)stcChannel->pairedChannel));

        if (stcChannel == stcChannel->lastStallingStcChannel)
        {
            NEXUS_StcChannel_P_RequestConnectedZeroFill(stcChannel);
            NEXUS_StcChannel_P_NotifyStatusUpdate(stcChannel);
        }
    }
}

static void NEXUS_StcChannel_P_RequestConnectedFlush(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_StcChannelDecoderConnectionHandle decoder;

    /* flush request goes to all connected decoders */
    for (decoder = BLST_Q_FIRST(&stcChannel->decoders); decoder; decoder = BLST_Q_NEXT(decoder, link))
    {
        /* the only way this gets set back to false is for an actual flush to occur, or a disconnect */
        decoder->status.flush = true;
    }
}

static void NEXUS_StcChannel_P_ReportPeerFlush(
    NEXUS_StcChannelHandle stcChannel
    )
{
    if (stcChannel->pairedChannel)
    {
        BDBG_MSG(("stcChannel%u %p: Flush reported by peer stcChannel%u %p", stcChannel->stcIndex, (void *)stcChannel, stcChannel->pairedChannel->stcIndex, (void *)stcChannel->pairedChannel));

        NEXUS_StcChannel_P_RequestConnectedFlush(stcChannel);
        NEXUS_StcChannel_P_NotifyStatusUpdate(stcChannel);
    }
}

void NEXUS_StcChannel_ReportDecoderHang_priv(NEXUS_StcChannelDecoderConnectionHandle decoder, const NEXUS_StcChannelDecoderFifoWatchdogStatus * pStatus)
{
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelDeadlockDetectionInfo overallDdInfo;
    NEXUS_StcChannelDeadlockDetectionInfo thisDdInfo;
    bool gap;
    bool stalled;
    bool wait;
    bool flush;
    bool zeroFill;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_StcChannelDecoderConnection);
    stcChannel = decoder->parent;
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    NEXUS_ASSERT_MODULE();

    decoder->status.flush = flush = false;
    decoder->status.zeroFill = zeroFill = false;
    gap = false;
    stalled = false;
    wait = false;

    /* this logic only applies to auto mode */
    if (stcChannel->settings.mode != NEXUS_StcChannelMode_eAuto) {
        return;
    }

    /* if we're in an stc trick mode, ignore any reports of hangs */
    /* TODO: consider only limiting for very slow stc trick. */
    if (stcChannel->isStcTrick) {
        return;
    }

    /* copy passed in status report */
    decoder->fifoWatchdogStatus = *pStatus;
    if (!decoder->stcValid) {
        decoder->fifoWatchdogStatus.tsmWait = true;
    }

    BDBG_MSG_TRACE(("%u:FIFO Watchdog Report:%s: buf %u%% %s%s%s",
        stcChannel->stcIndex,
        g_stcChannelTypeStr[decoder->settings.type],
        decoder->fifoWatchdogStatus.percentFull,
        decoder->fifoWatchdogStatus.isHung ? "hung ": "",
        decoder->fifoWatchdogStatus.tsmWait ? "waiting " : "",
        decoder->fifoWatchdogStatus.frameSyncUnlocked ? "unlocked" : ""));

    NEXUS_StcChannel_P_InitDeadlockDetectionInfo(&overallDdInfo);

    /* see if all connected decoders are hung.
       we know there must be at least one connected, otherwise we would not receive this call */
    NEXUS_StcChannel_P_PerformDeadlockDetection(stcChannel, &thisDdInfo);

    if (stcChannel->pairedChannel)
    {
        NEXUS_StcChannelHandle thatChannel = stcChannel->pairedChannel;
        NEXUS_StcChannelDeadlockDetectionInfo thatDdInfo;

        NEXUS_StcChannel_P_PerformDeadlockDetection(thatChannel, &thatDdInfo);

        overallDdInfo.allHung = thisDdInfo.allHung && thatDdInfo.allHung;
        overallDdInfo.oneEmpty = thisDdInfo.oneEmpty || thatDdInfo.oneEmpty;
        overallDdInfo.oneFull = thisDdInfo.oneFull || thatDdInfo.oneFull;
        overallDdInfo.tsmWaiting = thisDdInfo.tsmWaiting || thatDdInfo.tsmWaiting;

        /* check for av window stalls, which only happen with paired STC channels in NRT mode */
        if (stcChannel->nonRealTime && overallDdInfo.allHung)
        {
            BDBG_MSG(("stcChannel%u %p: All decoders are hung", stcChannel->stcIndex, (void *)stcChannel));
            stalled = NEXUS_StcChannel_P_PerformAvWindowStallDetection(stcChannel, &wait);
            if (stalled)
            {
                BDBG_MSG(("stcChannel%u %p: av window stall detected", stcChannel->stcIndex, (void *)stcChannel));
            }
        }

        /* A gap is detected when *this* decoder is not in TSM wait state,
         * it is stalled by a *different* decoder's av window,
         * and that decoder has underflowed / is empty.
         */
        gap = !thisDdInfo.tsmWaiting
            &&
            stalled
            &&
            thatDdInfo.oneEmpty;

        switch (stcChannel->settings.underflowHandling)
        {
            case NEXUS_StcChannelUnderflowHandling_eAllowProducerPause:
                /*
                 * if producer pause is allowed, then we only declare gap
                 * when this decoder is full.  The reason is that if we are
                 * full and the other decoder is empty, we are deadlocked, so
                 * we must do something.
                 */
                gap = gap && thisDdInfo.oneFull;
                break;

            default:
            case NEXUS_StcChannelUnderflowHandling_eDefault:
                /* no further restrictions on gap detection */
                break;
        }
    }
    else
    {
        overallDdInfo = thisDdInfo;
    }

    if (gap)
    {
        BDBG_MSG(("stcChannel%u %p: Gap detected", stcChannel->stcIndex, (void *)stcChannel));
        /* the only decoder(s) that should ZF are the ones whose STC caused the stall */
        if (stcChannel->pairedChannel)
        {
            NEXUS_StcChannel_P_ReportPeerGap(stcChannel->pairedChannel);
        }
    }
    else
    {
        if (!wait)
        {
            /* deadlock occurs when all decoders are hung, one is full and TSM waiting
                (which means no more data can enter the system) and the others
                are either empty or stalled */
            if
            (
                overallDdInfo.allHung
                &&
                overallDdInfo.oneFull
                &&
                overallDdInfo.tsmWaiting
                &&
                (
                    overallDdInfo.oneEmpty
                    ||
                    stalled
                )
            )
            {
                NEXUS_StcChannelDecoderConnectionHandle unlocked = NEXUS_StcChannel_P_FindUnlockedDecoder(stcChannel);
                BDBG_WRN(("stcChannel%u %p: All decoders are stuck, one fifo is full and one is empty or is av window stalled", stcChannel->stcIndex, (void *)stcChannel));
                if (NEXUS_GetEnv("check_lock") && unlocked && stcChannel->nonRealTime)
                {
                    /* if we find unlocked decoder in NRT mode,
                     * set zero fill of unlocked decoder,
                     * do nothing to this one for now */
                    BDBG_WRN(("stcChannel%u %p: A decoder is frame-sync unlocked. Zero-filling the unlocked decoder.", stcChannel->stcIndex, (void *)stcChannel));
                    zeroFill = true;
                    decoder = unlocked; /* override to notify the unlocked decoder, not this one */
                }
                else
                {
                    BDBG_WRN(("stcChannel%u %p: Flushing all decoder FIFOs to recover from likely TSM deadlock.", stcChannel->stcIndex, (void *)stcChannel));
                    flush = true;
                }
            }
            else
            {
                BDBG_MSG_TRACE(("stcChannel%u %p: Neither gap nor deadlock detected", stcChannel->stcIndex, stcChannel));
            }
        }
        else
        {
            BDBG_MSG_TRACE(("stcChannel%u %p: Waiting one cycle for gap detection", stcChannel->stcIndex, stcChannel));
        }
    }

    if (zeroFill)
    {
        NEXUS_StcChannel_P_RequestConnectedZeroFill(stcChannel);
    }

    if (flush)
    {
        NEXUS_StcChannel_P_RequestConnectedFlush(stcChannel);
        if (stcChannel->pairedChannel)
        {
            NEXUS_StcChannel_P_ReportPeerFlush(stcChannel->pairedChannel);
        }
    }

    if (zeroFill || flush)
    {
        NEXUS_StcChannel_P_NotifyStatusUpdate(stcChannel);
    }
}

void NEXUS_StcChannel_GetDecoderConnectionStatus_priv(
    NEXUS_StcChannelDecoderConnectionHandle decoder,
    NEXUS_StcChannelDecoderConnectionStatus * pStatus
    )
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_StcChannelDecoderConnection);
    BDBG_ASSERT(pStatus);
    NEXUS_ASSERT_MODULE();
    *pStatus = decoder->status;
}

void NEXUS_StcChannel_ReportDecoderFlush_priv(NEXUS_StcChannelDecoderConnectionHandle decoder)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_StcChannelDecoderConnection);
    NEXUS_ASSERT_MODULE();
    decoder->status.flush = false;
    decoder->stcValid = false;
}

void NEXUS_StcChannel_ReportDecoderZeroFill_priv(NEXUS_StcChannelDecoderConnectionHandle decoder)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_StcChannelDecoderConnection);
    NEXUS_ASSERT_MODULE();
    decoder->status.zeroFill = false;
}

NEXUS_Error NEXUS_StcChannel_VerifyPidChannel( NEXUS_StcChannelHandle stcChannel , NEXUS_PidChannelHandle pidChannel )
{
    NEXUS_TransportType pidChannelTransportType, stcTransportType = NEXUS_TransportType_eUnknown;
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(pidChannel, NEXUS_PidChannel);
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

    pidChannelTransportType = pidChannel->hwPidChannel->status.originalTransportType;

    switch ( stcChannel->settings.mode) {
        case NEXUS_StcChannelMode_eHost:
            stcTransportType = stcChannel->settings.modeSettings.host.transportType;
            break;
        case NEXUS_StcChannelMode_eAuto:
            stcTransportType = stcChannel->settings.modeSettings.Auto.transportType;
            break;
        case NEXUS_StcChannelMode_ePcr:
            if (!stcChannel->settings.modeSettings.pcr.pidChannel) {
                break;
            }
            stcTransportType = stcChannel->settings.modeSettings.pcr.pidChannel->hwPidChannel->status.transportType;
            break;
        default:  /* not used */
            break;
    }

    if ( NEXUS_IS_DSS_MODE(stcTransportType) !=  NEXUS_IS_DSS_MODE(pidChannelTransportType) ) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    return rc;
}

#if NEXUS_HAS_ASTM
void NEXUS_StcChannel_GetAstmSettings_priv(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_StcChannelAstmSettings * pAstmSettings  /* [out] */
)
{
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    NEXUS_ASSERT_MODULE();
    *pAstmSettings = stcChannel->astm.settings;
}

NEXUS_Error NEXUS_StcChannel_SetAstmSettings_priv(
    NEXUS_StcChannelHandle stcChannel,
    const NEXUS_StcChannelAstmSettings * pAstmSettings
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bool regenVideoStcRequest = false;
    bool regenAudioStcRequest = false;

    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

    NEXUS_ASSERT_MODULE();

    if
        (
            (pAstmSettings->enabled)
            &&
            (pAstmSettings->mode == NEXUS_StcChannelMode_eAuto)
            &&
            (
                (
                    (stcChannel->astm.settings.tsmMode == NEXUS_StcChannelTsmMode_eOutputMaster)
                    &&
                    (pAstmSettings->tsmMode != NEXUS_StcChannelTsmMode_eOutputMaster)
                )
                ||
                (stcChannel->astm.settings.mode == NEXUS_StcChannelMode_ePcr)
            )
        )
    {
        if (pAstmSettings->tsmMode == NEXUS_StcChannelTsmMode_eVideoMaster)
        {
            regenVideoStcRequest = true;
        }
        else if (pAstmSettings->tsmMode == NEXUS_StcChannelTsmMode_eAudioMaster)
        {
            regenAudioStcRequest = true;
        }
        /* CSP359468: STC master mode must also generate a request, so we do
        both (just like in real STC master mode) */
        else if (pAstmSettings->tsmMode == NEXUS_StcChannelTsmMode_eStcMaster)
        {
            regenVideoStcRequest = true;
            regenAudioStcRequest = true;
        }
    }

    /* always copy settings, so ASTM can get what it sets */
    stcChannel->astm.settings = *pAstmSettings;

    rc = NEXUS_StcChannel_SetSettings(stcChannel, &stcChannel->settings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    if (regenVideoStcRequest || regenAudioStcRequest)
    {
        BDBG_MSG(("ASTM is forcing an STC request regeneration"));
    }

    /* CSP359468: changed order of processing requests to match many real streams (audio request is
    first for STC master mode) */

    if (regenAudioStcRequest)
    {
        NEXUS_StcChannelDecoderConnectionHandle audio = getAudioConnector(stcChannel);
        if (audio)
        {
            rc = NEXUS_StcChannel_P_RegenStcRequest(audio);
            if (rc) { rc = BERR_TRACE(rc); goto error; }
        }
    }

    if (regenVideoStcRequest)
    {
        NEXUS_StcChannelDecoderConnectionHandle video = getVideoConnector(stcChannel);
        if (video)
        {
            rc = NEXUS_StcChannel_P_RegenStcRequest(video);
            if (rc) { rc = BERR_TRACE(rc); goto error; }
        }
    }

error:
    return rc;
}

NEXUS_Error NEXUS_StcChannel_GetAstmStatus_priv(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_StcChannelAstmStatus * pAstmStatus
)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);

    pAstmStatus->timebase = stcChannel->timebase;
    return 0;
}

#endif /* NEXUS_HAS_ASTM */

void NEXUS_StcChannel_GetDefaultPairSettings(NEXUS_StcChannelPairSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->window = 100;
    pSettings->connected = false;
    return;
}

#define STC_PHASE_MISMATCH_AV_WINDOW 1 /* ms */
#define STC_PHASE_MISMATCH_STC_INC 10 /* ms */
#ifndef INT32_MAX
#define INT32_MAX 0x7fffffff
#endif

#if BXPT_HAS_TSMUX
static NEXUS_Error stc_phase_mismatch_av_window_test(NEXUS_StcChannelHandle testStc, NEXUS_StcChannelHandle refStc)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t testValue;
    uint32_t refValue;
    uint32_t oldTestValue;
    uint32_t oldRefValue;

    BDBG_MSG_TRACE(("STC%u: checking av window increment constraint", testStc->stcIndex));
    oldTestValue = BXPT_PcrOffset_GetStc(testStc->pcrOffset);
    oldRefValue = BXPT_PcrOffset_GetStc(refStc->pcrOffset);
    BDBG_MSG_TRACE(("STC%u: setting stc delta to 10x av_window width = %u", testStc->stcIndex, STC_PHASE_MISMATCH_AV_WINDOW * 450));
    rc = BXPT_PcrOffset_SetStc(testStc->pcrOffset, (uint32_t)INT32_MAX + STC_PHASE_MISMATCH_AV_WINDOW * 225);
    if (rc) { BERR_TRACE(rc); goto error; }
    rc = BXPT_PcrOffset_SetStc(refStc->pcrOffset, (uint32_t)INT32_MAX - STC_PHASE_MISMATCH_AV_WINDOW * 225);
    if (rc) { BERR_TRACE(rc); goto error; }
    testValue = BXPT_PcrOffset_GetStc(testStc->pcrOffset);
    refValue = BXPT_PcrOffset_GetStc(refStc->pcrOffset);
    BDBG_MSG_TRACE(("STC%u: stc delta = %u", testStc->stcIndex, testValue - refValue));
    BDBG_MSG_TRACE(("STC%u: triggering test increment", testStc->stcIndex));
    BXPT_PcrOffset_TriggerStcIncrement(testStc->pcrOffset);
    testValue = BXPT_PcrOffset_GetStc(testStc->pcrOffset);
    BDBG_MSG_TRACE(("STC%u: stc delta = %u", testStc->stcIndex, testValue - refValue));
    BDBG_MSG_TRACE(("STC%u: resetting stcs", testStc->stcIndex));
    rc = BXPT_PcrOffset_SetStc(testStc->pcrOffset, oldTestValue);
    if (rc) { BERR_TRACE(rc); goto error; }
    rc = BXPT_PcrOffset_SetStc(refStc->pcrOffset, oldRefValue);
    if (rc) { BERR_TRACE(rc); goto error; }
    rc = ((testValue - refValue) == (STC_PHASE_MISMATCH_AV_WINDOW * 450)) ? NEXUS_SUCCESS : NEXUS_NOT_INITIALIZED;

error:
    return rc;
}

static NEXUS_Error stc_phase_mismatch_set_config(NEXUS_StcChannelHandle stc)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BXPT_PcrOffset_NRTConfig testNrtConfig;
    BXPT_PcrOffset_Settings settings;

    /* use MOD300 count mode for workaround */
    BXPT_PcrOffset_GetSettings(stc->pcrOffset, &settings);
    settings.CountMode = BXPT_PcrOffset_StcCountMode_eMod300;
    rc = BXPT_PcrOffset_SetSettings(stc->pcrOffset, &settings);
    if (rc) { BERR_TRACE(rc); goto error; }

    BXPT_PcrOffset_GetNRTConfig(stc->pcrOffset, &testNrtConfig);
    testNrtConfig.EnableAvWindowComparison = true;
    testNrtConfig.AvWindow = STC_PHASE_MISMATCH_AV_WINDOW;
    testNrtConfig.StcIncrement = (STC_PHASE_MISMATCH_STC_INC * 45) << 10;
    testNrtConfig.TriggerMode = BXPT_PcrOffset_StcTriggerMode_eSoftIncrement;
    rc = BXPT_PcrOffset_SetNRTConfig(stc->pcrOffset, &testNrtConfig);
    if (rc) { BERR_TRACE(rc); goto error; }

error:
    return rc;
}

static NEXUS_Error stc_phase_mismatch_av_window_disable(NEXUS_StcChannelHandle testStc)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BXPT_PcrOffset_NRTConfig nrtConfig;
    BXPT_PcrOffset_GetNRTConfig(testStc->pcrOffset, &nrtConfig);
    nrtConfig.EnableAvWindowComparison = false;
    rc = BXPT_PcrOffset_SetNRTConfig(testStc->pcrOffset, &nrtConfig);
    return rc;
}

static NEXUS_Error stc_phase_mismatch_av_window_enable(NEXUS_StcChannelHandle testStc)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BXPT_PcrOffset_NRTConfig nrtConfig;
    BXPT_PcrOffset_GetNRTConfig(testStc->pcrOffset, &nrtConfig);
    nrtConfig.EnableAvWindowComparison = true;
    rc = BXPT_PcrOffset_SetNRTConfig(testStc->pcrOffset, &nrtConfig);
    return rc;
}

static NEXUS_Error stc_phase_mismatch_force_rollover_nrt(NEXUS_StcChannelHandle stc)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t value;
    uint32_t oldValue;

    BDBG_MSG_TRACE(("STC%u: forcing rollover", stc->stcIndex));
    oldValue = BXPT_PcrOffset_GetStc(stc->pcrOffset);
    BDBG_MSG_TRACE(("STC%u: value = %u (0x%08x)", stc->stcIndex, oldValue, oldValue));
    BDBG_MSG_TRACE(("STC%u: disabling av window", stc->stcIndex));
    rc = stc_phase_mismatch_av_window_disable(stc);
    if (rc) { BERR_TRACE(rc); goto error; }
    BDBG_MSG_TRACE(("STC%u: setting value to 0xffffffff", stc->stcIndex));
    rc = BXPT_PcrOffset_SetStc(stc->pcrOffset, 0xffffffff);
    if (rc) { BERR_TRACE(rc); goto error; }
    value = BXPT_PcrOffset_GetStc(stc->pcrOffset);
    BDBG_MSG_TRACE(("STC%u: value = %u (0x%08x)", stc->stcIndex, value, value));
    BDBG_MSG_TRACE(("STC%u: triggering rollover increment", stc->stcIndex));
    BXPT_PcrOffset_TriggerStcIncrement(stc->pcrOffset);
    value = BXPT_PcrOffset_GetStc(stc->pcrOffset);
    BDBG_MSG_TRACE(("STC%u: value = %u (0x%08x)", stc->stcIndex, value, value));
    BDBG_MSG_TRACE(("STC%u: re-enabling av window", stc->stcIndex));
    rc = stc_phase_mismatch_av_window_enable(stc);
    if (rc) { BERR_TRACE(rc); goto error; }
    rc = BXPT_PcrOffset_SetStc(stc->pcrOffset, oldValue);
    if (rc) { BERR_TRACE(rc); goto error; }
    BSTD_UNUSED(value);
error:
    return rc;
}

/*
 * STC phase mismatch workaround
 *
 * The phase mismatch appears when bit 43 of one STC does not match bit 43
 * of the other STC used in a pair.  To test this, we will set the
 * av_window to 1 ms wide, and the stc_inc to 10 ms.  If we start with
 * both stcs in the middle of the uint32_t range, separated by 10 av_window
 * lengths, there's no chance that the test itself will cause a rollover, which
 * would toggle bit 43 on the incremented stc.  Given the four possible settings
 * for bit 43 between the two stcs, the expected results are as follows:
 *
 * stc0_43  stc1_43  stc0_45K_pre_inc  stc1_45K    stc0_45K_post_inc
 * 0        0        0x800000e0        0x7fffff1e  0x800000e0
 * 0        1        0x800000e0        0x7fffff1e  0x800002a2
 * 1        0        0x800000e0        0x7fffff1e  0x800000e0
 * 1        1        0x800000e0        0x7fffff1e  0x800000e0
 *
 * stc0_43  stc1_43  stc1_45K_pre_inc  stc0_45K    stc1_45K_post_inc
 * 0        0        0x800000e0        0x7fffff1e  0x800000e0
 * 0        1        0x800000e0        0x7fffff1e  0x800000e0
 * 1        0        0x800000e0        0x7fffff1e  0x800002a2
 * 1        1        0x800000e0        0x7fffff1e  0x800000e0
 *
 * Therefore, the test should be to put both stcs at +/- 5 av_windows around
 * 0x7ffffff and try to increment one of them (which results in a reverse lookup
 * in the tables above).  The test algo is thus:
 *
 * av_window := 1 ms
 * stc_inc := 10 ms
 * stc0 := 0x7fffffff + 5 * av_window * 45
 * stc1 := 0x7fffffff - 5 * av_window * 45
 * if (increment_succeeds(stc0)) then
 *     // av window test failed
 *     // stc1_43 is 1 and stc0_43 is 0 (phase mismatched)
 *     force_rollover(stc1);
 * else
 *     // not enough info -> test stc1
 *     stc1 := 0x7fffffff + 5 * av_window * 45
 *     stc0 := 0x7fffffff - 5 * av_window * 45
 *     if (increment_succeeds(stc1)) then
 *         // av window test failed
 *         // stc0_43 is 1 and stc1_43 is 0 (phase mismatched)
 *         force_rollover(stc0);
 *     else
 *         // av window test passed
 *         // (stc0_43 == stc1_43) is true (phase matched)
 * return
 */
static NEXUS_Error stc_phase_mismatch_workaround(NEXUS_StcChannelHandle stc1, NEXUS_StcChannelHandle stc2)
{
    static const int TRIES = 2;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    BXPT_PcrOffset_NRTConfig stc1SavedNrtConfig;
    BXPT_PcrOffset_NRTConfig stc2SavedNrtConfig;
    BXPT_PcrOffset_Settings stc1Settings, stc2Settings;
    int i;

    /* store the programmed config */
    BXPT_PcrOffset_GetNRTConfig(stc1->pcrOffset, &stc1SavedNrtConfig);
    BXPT_PcrOffset_GetNRTConfig(stc2->pcrOffset, &stc2SavedNrtConfig);
    BXPT_PcrOffset_GetSettings(stc1->pcrOffset, &stc1Settings);
    BXPT_PcrOffset_GetSettings(stc2->pcrOffset, &stc2Settings);

    rc = stc_phase_mismatch_set_config(stc1);
    if (rc) { BERR_TRACE(rc); goto error; }

    rc = stc_phase_mismatch_set_config(stc2);
    if (rc) { BERR_TRACE(rc); goto error; }

    for (i = 0; i < TRIES; i++)
    {
        rc = stc_phase_mismatch_av_window_test(stc1, stc2);
        if (rc != NEXUS_SUCCESS)
        {
            BDBG_MSG_TRACE(("STC%u: phase mismatch detected; forcing rollover", stc2->stcIndex, i));
            rc = stc_phase_mismatch_force_rollover_nrt(stc2);
            if (rc) { BERR_TRACE(rc); goto error; }
        }
        else
        {
            break;
        }
    }

    /* reset back to programmed config */
    rc = BXPT_PcrOffset_SetSettings(stc1->pcrOffset, &stc1Settings);
    if (rc) { BERR_TRACE(rc); goto error; }

    rc = BXPT_PcrOffset_SetSettings(stc2->pcrOffset, &stc2Settings);
    if (rc) { BERR_TRACE(rc); goto error; }

    rc = BXPT_PcrOffset_SetNRTConfig(stc1->pcrOffset, &stc1SavedNrtConfig);
    if (rc) { BERR_TRACE(rc); goto error; }

    rc = BXPT_PcrOffset_SetNRTConfig(stc2->pcrOffset, &stc2SavedNrtConfig);
    if (rc) { BERR_TRACE(rc); goto error; }

    /* what was result of testing loop? */
    if (i == 0)
    {
        BDBG_MSG_TRACE(("STC%u: phase mismatch not detected", stc2->stcIndex));
        rc = NEXUS_UNKNOWN; /* do not trace, this is normal */
    }
    else if (i < TRIES)
    {
        BDBG_MSG_TRACE(("STC%u: phase mismatch fixed", stc2->stcIndex));
        rc = NEXUS_SUCCESS;
    }
    else
    {
        BDBG_ERR(("STC%u: phase mismatch forced rollover didn't work; bailing", stc2->stcIndex));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

error:
    return rc;
}
#endif

NEXUS_Error NEXUS_StcChannel_SetPairSettings( NEXUS_StcChannelHandle stcChannel1, NEXUS_StcChannelHandle stcChannel2, const NEXUS_StcChannelPairSettings *pSettings)
{
#if BXPT_HAS_TSMUX
    BERR_Code rc;
    BXPT_PcrOffset_NRTConfig nrtConfig1, nrtConfig2;
    BXPT_PcrOffset_Settings pcrOffsetSettings1, pcrOffsetSettings2;
    BPCRlib_Config pcrlibConfig;

    BDBG_OBJECT_ASSERT(stcChannel1, NEXUS_StcChannel);
    BDBG_OBJECT_ASSERT(stcChannel2, NEXUS_StcChannel);
    BDBG_ASSERT(pSettings);

    if (pSettings->connected)
    {
        BDBG_MSG(("channel%u %p is paired with channel%u %p", stcChannel1->stcIndex, (void *)stcChannel1, stcChannel2->stcIndex, (void *)stcChannel2));
    }
    else
    {
        BDBG_MSG(("channel%u %p is unpaired from channel%u %p", stcChannel1->stcIndex, (void *)stcChannel1, stcChannel2->stcIndex, (void *)stcChannel2));
    }
    stcChannel1->pairedChannel = pSettings->connected ? stcChannel2 : NULL;
    stcChannel2->pairedChannel = pSettings->connected ? stcChannel1 : NULL;

    BXPT_PcrOffset_GetNRTConfig( stcChannel1->pcrOffset, &nrtConfig1 );
    BXPT_PcrOffset_GetNRTConfig( stcChannel2->pcrOffset, &nrtConfig2 );

    rc = BXPT_PcrOffset_GetSettings( stcChannel1->pcrOffset, &pcrOffsetSettings1 );
    if(rc!=BERR_SUCCESS){rc=BERR_TRACE(rc);goto error;}

    rc = BXPT_PcrOffset_GetSettings( stcChannel2->pcrOffset, &pcrOffsetSettings2 );
    if(rc!=BERR_SUCCESS){rc=BERR_TRACE(rc);goto error;}

    nrtConfig1.PairedStc = pcrOffsetSettings2.StcSelect;
    nrtConfig2.PairedStc = pcrOffsetSettings1.StcSelect;

    nrtConfig1.EnableAvWindowComparison = pSettings->connected;
    nrtConfig2.EnableAvWindowComparison = pSettings->connected;

    nrtConfig1.AvWindow = pSettings->window;
    nrtConfig2.AvWindow = pSettings->window;

    rc = BXPT_PcrOffset_SetNRTConfig(stcChannel1->pcrOffset, &nrtConfig1);
    if(rc!=BERR_SUCCESS){rc=BERR_TRACE(rc);goto error;}

    rc = BXPT_PcrOffset_SetNRTConfig(stcChannel2->pcrOffset, &nrtConfig2);
    if(rc!=BERR_SUCCESS){rc=BERR_TRACE(rc);goto error;}

    BPCRlib_Channel_GetConfig(stcChannel1->pcrlibChannel, &pcrlibConfig);
    pcrlibConfig.paired = pSettings->connected;
    rc = BPCRlib_Channel_SetConfig(stcChannel1->pcrlibChannel, &pcrlibConfig);
    if(rc!=BERR_SUCCESS){rc=BERR_TRACE(rc);goto error;}

    BPCRlib_Channel_GetConfig(stcChannel2->pcrlibChannel, &pcrlibConfig);
    pcrlibConfig.paired = pSettings->connected;
    rc = BPCRlib_Channel_SetConfig(stcChannel2->pcrlibChannel, &pcrlibConfig);
    if(rc!=BERR_SUCCESS){rc=BERR_TRACE(rc);goto error;}

    if (pSettings->connected)
    {
        rc = stc_phase_mismatch_workaround(stcChannel1, stcChannel2);
        if (rc == NEXUS_UNKNOWN)
        {
            rc = stc_phase_mismatch_workaround(stcChannel2, stcChannel1);
            if (rc != NEXUS_SUCCESS && rc != NEXUS_UNKNOWN) { BERR_TRACE(rc); goto error; }
        }
        else if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    }

    return NEXUS_SUCCESS;
error:
    return rc;

#else /* BXPT_HAS_TSMUX */
    BSTD_UNUSED(stcChannel1);
    BSTD_UNUSED(stcChannel2);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

void NEXUS_StcChannel_GetDefaultNonRealtimeSettings_priv (NEXUS_StcChannelNonRealtimeSettings *pSettings)
{
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->triggerMode = NEXUS_StcChannelTriggerMode_eTimebase;
    return;
}

NEXUS_Error NEXUS_StcChannel_SetNonRealtimeConfig_priv(NEXUS_StcChannelHandle handle, NEXUS_StcChannelNonRealtimeSettings *pSettings, bool reset)
{
#if BXPT_HAS_TSMUX
    BERR_Code rc;
    BXPT_PcrOffset_NRTConfig nrtConfig;
    BPCRlib_Config pcrlibConfig;
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(pSettings);
    BDBG_OBJECT_ASSERT(handle, NEXUS_StcChannel);
    BDBG_CASSERT(NEXUS_StcChannelTriggerMode_eTimebase  ==  (unsigned)BXPT_PcrOffset_StcTriggerMode_eTimebase);
    BDBG_CASSERT(NEXUS_StcChannelTriggerMode_eExternalTrig ==  (unsigned)BXPT_PcrOffset_StcTriggerMode_eExternalTrig);
    BDBG_CASSERT(NEXUS_StcChannelTriggerMode_eSoftIncrement == (unsigned)BXPT_PcrOffset_StcTriggerMode_eSoftIncrement);
    BXPT_PcrOffset_GetNRTConfig( handle->pcrOffset, &nrtConfig );
    handle->nonRealTime = (pSettings->triggerMode != NEXUS_StcChannelTriggerMode_eTimebase);
    BDBG_MSG(("%p STC%u channel configured in %s mode, extTrig: %u", (void *)handle,
        handle->stcIndex, handle->nonRealTime ? "non-realtime" : "realtime", pSettings->externalTrigger));
    nrtConfig.TriggerMode = pSettings->triggerMode;
#if B_REFSW_DSS_SUPPORT
    if(NEXUS_IS_DSS_MODE(NEXUS_StcChannel_P_GetTransportType(&handle->settings)))
    {
        nrtConfig.StcIncrement = pSettings->stcIncrement; /* binary format for DSS */
    }
    else
#endif
    {/* binary to MOD300 conversion for MPEG2TS */
        nrtConfig.StcIncrement = ((pSettings->stcIncrement/300)<<9) + (pSettings->stcIncrement % 300);
    }
    nrtConfig.ExternalTriggerNum = pSettings->externalTrigger;

    rc = BXPT_PcrOffset_SetNRTConfig(handle->pcrOffset, &nrtConfig);
    if(rc!=BERR_SUCCESS){rc=BERR_TRACE(rc);goto error;}

    if (handle->nonRealTime && reset)
    {
        rc = BXPT_PcrOffset_SetStc(handle->pcrOffset, 0);
        if(rc!=BERR_SUCCESS){rc=BERR_TRACE(rc);goto error;}
    }

    BPCRlib_Channel_GetConfig(handle->pcrlibChannel, &pcrlibConfig);
    pcrlibConfig.non_real_time = handle->nonRealTime;
    rc = BPCRlib_Channel_SetConfig(handle->pcrlibChannel, &pcrlibConfig);
    if(rc!=BERR_SUCCESS){rc=BERR_TRACE(rc);goto error;}

    /* re-set this as it depends on NRT config */
    NEXUS_StcChannel_P_SetSwPcrOffsetEnabled(handle);

    return NEXUS_SUCCESS;
error:
    return rc;
#else /* BXPT_HAS_TSMUX */
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(pSettings);
    BSTD_UNUSED(handle);
    BSTD_UNUSED(reset);
    if(pSettings->triggerMode==NEXUS_StcChannelTriggerMode_eTimebase) {
        return NEXUS_SUCCESS;
    }
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif

}

void NEXUS_StcChannel_GetSoftIncrementRegOffset_priv(NEXUS_StcChannelHandle handle, BAVC_Xpt_StcSoftIncRegisters *regMap)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(handle, NEXUS_StcChannel);
    BDBG_ASSERT(regMap);
#if BXPT_HAS_TSMUX
    BXPT_PcrOffset_GetSoftIncrementRegisterOffsets(handle->pcrOffset, regMap);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(regMap);
    (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
    return;
}

#if BXPT_NUM_STC_SNAPSHOTS > 0
static void NEXUS_StcChannel_P_GetDefaultSnapshotSettings(NEXUS_StcChannelSnapshotSettings * pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_StcChannelSnapshotSettings));
    /* trigger index and mode match hardware and PI defaults of zero and legacy respectively */
}

NEXUS_StcChannelSnapshotHandle NEXUS_StcChannel_OpenSnapshot_priv(NEXUS_StcChannelHandle stcChannel)
{
    NEXUS_StcChannelSnapshotHandle snapshot = NULL;
    BXPT_PcrOffset_StcSnapshot magSnapshot = NULL;

    NEXUS_OBJECT_ASSERT(NEXUS_StcChannel, stcChannel);

    magSnapshot = BXPT_PcrOffset_AllocStcSnapshot(pTransport->xpt, stcChannel->stcIndex);
    if (!magSnapshot)
    {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto end;
    }
    snapshot = BKNI_Malloc(sizeof(struct NEXUS_StcChannelSnapshot));
    if (!snapshot)
    {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }
    BKNI_Memset(snapshot, 0, sizeof(struct NEXUS_StcChannelSnapshot));
    BDBG_OBJECT_SET(snapshot, NEXUS_StcChannelSnapshot);
    snapshot->magSnapshot = magSnapshot;
    snapshot->parent = stcChannel;
    BLST_Q_INSERT_TAIL(&stcChannel->snapshots, snapshot, link);
    NEXUS_StcChannel_P_GetDefaultSnapshotSettings(&snapshot->settings);
    BDBG_MSG(("opened snapshot %p on STC%u", (void*)snapshot, stcChannel->stcIndex));

end:
    return snapshot;
}

void NEXUS_StcChannel_CloseSnapshot_priv(NEXUS_StcChannelSnapshotHandle snapshot)
{
    BDBG_OBJECT_ASSERT(snapshot, NEXUS_StcChannelSnapshot);

    BDBG_MSG(("closing snapshot %p on STC%u", (void*)snapshot, snapshot->parent->stcIndex));
    BXPT_PcrOffset_FreeStcSnapshot(snapshot->magSnapshot);
    BLST_Q_REMOVE(&snapshot->parent->snapshots, snapshot, link);
    snapshot->parent = NULL;
    BDBG_OBJECT_DESTROY(snapshot, NEXUS_StcChannelSnapshot);
    BKNI_Free(snapshot);
}

void NEXUS_StcChannel_GetSnapshotSettings_priv(NEXUS_StcChannelSnapshotHandle snapshot, NEXUS_StcChannelSnapshotSettings * pSettings)
{
    BDBG_OBJECT_ASSERT(snapshot, NEXUS_StcChannelSnapshot);
    BDBG_ASSERT(pSettings);
    BKNI_Memcpy(pSettings, &snapshot->settings, sizeof(NEXUS_StcChannelSnapshotSettings));
}

NEXUS_Error NEXUS_StcChannel_SetSnapshotSettings_priv(NEXUS_StcChannelSnapshotHandle snapshot, const NEXUS_StcChannelSnapshotSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BXPT_PcrOffset_StcSnapshotSettings magSettings;
    NEXUS_StcChannelSnapshotSettings defaultSettings;

    BDBG_OBJECT_ASSERT(snapshot, NEXUS_StcChannelSnapshot);

    if (!pSettings)
    {
        NEXUS_StcChannel_P_GetDefaultSnapshotSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    BDBG_MSG(("snapshot %p: triggerIndex = %u; mode = %u", (void*)snapshot, pSettings->triggerIndex, pSettings->mode));
    BXPT_PcrOffset_GetStcSnapshotConfig(snapshot->magSnapshot, &magSettings);
    magSettings.ExternalTriggerNum = pSettings->triggerIndex;
    switch (pSettings->mode)
    {
        case NEXUS_StcChannelSnapshotMode_eLegacy:
            magSettings.Mode = BXPT_PcrOffset_StcSnapshotMode_eLegacy;
            break;
        case NEXUS_StcChannelSnapshotMode_eLsb32:
            magSettings.Mode = BXPT_PcrOffset_StcSnapshotMode_eLsb32;
            break;
        case NEXUS_StcChannelSnapshotMode_eMsb32:
            magSettings.Mode = BXPT_PcrOffset_StcSnapshotMode_eMsb32;
            break;
        default:
            break;
    }
    rc = BXPT_PcrOffset_SetStcSnapshotConfig(snapshot->magSnapshot, &magSettings);
    if (rc) { BERR_TRACE(rc); goto end; }

    BKNI_Memcpy(&snapshot->settings, pSettings, sizeof(NEXUS_StcChannelSnapshotSettings));

end:
    return rc;
}

NEXUS_Error NEXUS_StcChannel_GetSnapshotStatus_priv(NEXUS_StcChannelSnapshotHandle snapshot, NEXUS_StcChannelSnapshotStatus * pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BXPT_PcrOffset_StcSnapshotRegisters regMap;

    BDBG_OBJECT_ASSERT(snapshot, NEXUS_StcChannelSnapshot);
    BDBG_ASSERT(pStatus);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_StcChannelSnapshotStatus));

    BXPT_PcrOffset_GetStcSnapshotRegisterOffsets(snapshot->magSnapshot, &regMap);
    pStatus->stcLoAddr = regMap.StcLo;
    pStatus->stcHiAddr = regMap.StcHi;

    BDBG_MSG(("snapshot %p: stcLoAddr = %#x; stcHiAddr = %#x", (void*)snapshot, pStatus->stcLoAddr, pStatus->stcHiAddr));
    return rc;
}
#else
NEXUS_StcChannelSnapshotHandle NEXUS_StcChannel_OpenSnapshot_priv(NEXUS_StcChannelHandle stcChannel)
{
    BDBG_ERR(("NEXUS_StcChannel_OpenSnapshot_priv: not supported"));
    BSTD_UNUSED(stcChannel);
    return NULL;
}

void NEXUS_StcChannel_CloseSnapshot_priv(NEXUS_StcChannelSnapshotHandle snapshot)
{
    BDBG_ERR(("NEXUS_StcChannel_CloseSnapshot_priv: not supported"));
    BSTD_UNUSED(snapshot);
}

void NEXUS_StcChannel_GetSnapshotSettings_priv(NEXUS_StcChannelSnapshotHandle snapshot, NEXUS_StcChannelSnapshotSettings * pSettings)
{
    BDBG_ERR(("NEXUS_StcChannel_GetSnapshotSettings_priv: not supported"));
    BSTD_UNUSED(snapshot);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_StcChannel_SetSnapshotSettings_priv(NEXUS_StcChannelSnapshotHandle snapshot, const NEXUS_StcChannelSnapshotSettings * pSettings)
{
    BDBG_ERR(("NEXUS_StcChannel_SetSnapshotSettings_priv: not supported"));
    BSTD_UNUSED(snapshot);
    BSTD_UNUSED(pSettings);
    return NEXUS_NOT_SUPPORTED;
}

NEXUS_Error NEXUS_StcChannel_GetSnapshotStatus_priv(NEXUS_StcChannelSnapshotHandle snapshot, NEXUS_StcChannelSnapshotStatus * pStatus)
{
    BDBG_ERR(("NEXUS_StcChannel_GetSnapshotStatus_priv: not supported"));
    BSTD_UNUSED(snapshot);
    BSTD_UNUSED(pStatus);
    return NEXUS_NOT_SUPPORTED;
}
#endif
