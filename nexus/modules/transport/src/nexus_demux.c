/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#include "nexus_transport_module.h"
#include "bxpt_spid.h"
#include "nexus_playpump_impl.h"

BDBG_MODULE(nexus_demux);
BDBG_FILE_MODULE(nexus_flow_pid_channel);

static void NEXUS_P_HwPidChannel_RemoveSlavePidChannel( NEXUS_P_HwPidChannel *master, NEXUS_P_HwPidChannel *slave );
static void NEXUS_P_HwPidChannel_P_Close(NEXUS_P_HwPidChannel *hwPidChannel); /* this function should only be called from NEXUS_PidChannel_P_Finalizer */
static NEXUS_Error NEXUS_P_HwPidChannel_SetEnabled( NEXUS_P_HwPidChannel *pidChannel, bool enabled );

#if NEXUS_MAX_INPUT_BANDS
static bool NEXUS_P_InputBandIsSupported(NEXUS_InputBand inputBand)
{
    /* BXPT_P_InputBandIsSupported truncates inputBand to 32 bits, so we must guard against 64 bit overrun */
    return inputBand < NEXUS_MAX_INPUT_BANDS && BXPT_P_InputBandIsSupported( inputBand );
}
#endif

void NEXUS_InputBand_GetSettings(NEXUS_InputBand inputBand, NEXUS_InputBandSettings *pSettings)
{
#if NEXUS_MAX_INPUT_BANDS
    /* this code assumes Nexus holds the default state, not the PI or HW */
    if ( !NEXUS_P_InputBandIsSupported( inputBand ) ) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    *pSettings = pTransport->inputBand[inputBand].settings;
#else
    BSTD_UNUSED(inputBand);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#endif
}

NEXUS_Error NEXUS_InputBand_SetSettings(NEXUS_InputBand inputBand, const NEXUS_InputBandSettings *pSettings)
{
#if NEXUS_MAX_INPUT_BANDS
    BERR_Code rc;
    BXPT_InputBandConfig  config;

    NEXUS_ASSERT_MODULE(); /* make sure init was called */
    if (!NEXUS_P_InputBandIsSupported( inputBand )) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BXPT_GetInputBandConfig(pTransport->xpt, inputBand, &config);
    config.ClockPolSel = pSettings->clockActiveHigh?BXPT_Polarity_eActiveHigh:BXPT_Polarity_eActiveLow;
    config.LsbFirst = pSettings->dataLsbFirst;
    config.DataPolSel = pSettings->dataActiveHigh?BXPT_Polarity_eActiveHigh:BXPT_Polarity_eActiveLow;
    config.ForceValid = !pSettings->validEnabled;
    config.ValidPolSel = pSettings->validActiveHigh?BXPT_Polarity_eActiveHigh:BXPT_Polarity_eActiveLow;
    config.UseSyncAsValid = pSettings->useSyncAsValid;
    config.SyncPolSel = pSettings->syncActiveHigh?BXPT_Polarity_eActiveHigh:BXPT_Polarity_eActiveLow;
    config.EnableErrorInput = pSettings->errorEnabled;
    config.ErrorPolSel = pSettings->errorActiveHigh?BXPT_Polarity_eActiveHigh:BXPT_Polarity_eActiveLow;
    config.ParallelInputSel = pSettings->parallelInput;
    if (NEXUS_IS_DSS_MODE(pTransport->inputBand[inputBand].transportType)) {
        /* If parser band was configured for DSS, we need to override the input band settings */
        config.IbPktLength = 130;
        config.SyncDetectEn = false;
    }
    else {
        config.IbPktLength = pSettings->packetLength;
        config.SyncDetectEn = pSettings->useInternalSync;
    }
    config.SyncGen.Length = pSettings->parallelInput ? pSettings->packetLength : pSettings->packetLength * 8;
    rc = BXPT_SetInputBandConfig(pTransport->xpt, inputBand, &config);
    if (rc) {return BERR_TRACE(rc);}

    pTransport->inputBand[inputBand].settings = *pSettings;

    return BERR_SUCCESS;
#else
    BSTD_UNUSED(inputBand);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_InputBand_ArmSyncGeneration(NEXUS_InputBand inputBand, unsigned skip)
{
#if NEXUS_MAX_INPUT_BANDS
    BERR_Code rc;
    BXPT_InputBandConfig  config;

    NEXUS_ASSERT_MODULE(); /* make sure init was called */
    if (!NEXUS_P_InputBandIsSupported( inputBand )) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* Since board designers don't dynamically change tuner wiring, we can leave sync gen on */
    BXPT_GetInputBandConfig(pTransport->xpt, inputBand, &config);
    config.SyncGen.Enable = true;
    config.SyncGen.Length = config.ParallelInputSel ? config.IbPktLength : config.IbPktLength * 8;
    rc = BXPT_SetInputBandConfig(pTransport->xpt, inputBand, &config);
    if (rc) {return BERR_TRACE(rc);}

    rc = BXPT_ArmSyncGeneration(pTransport->xpt, inputBand, skip);
    if(rc == BERR_INVALID_PARAMETER)
    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    return BERR_SUCCESS;
#else
    BSTD_UNUSED(inputBand);
    BSTD_UNUSED(skip);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_InputBand_P_SetTransportType(NEXUS_InputBand inputBand, NEXUS_TransportType transportType)
{
#if NEXUS_MAX_INPUT_BANDS
    NEXUS_InputBandSettings settings;
    if (!NEXUS_P_InputBandIsSupported( inputBand )) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    pTransport->inputBand[inputBand].transportType = transportType;
    settings = pTransport->inputBand[inputBand].settings;
    return NEXUS_InputBand_SetSettings(inputBand, &settings);
#else
    BSTD_UNUSED(inputBand);
    BSTD_UNUSED(transportType);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

#ifdef BCHP_XPT_FE_REG_START
#include "bchp_xpt_fe.h"
#endif
NEXUS_Error NEXUS_InputBand_GetStatus(NEXUS_InputBand inputBand, NEXUS_InputBandStatus *pStatus)
{
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    switch (inputBand-NEXUS_InputBand_e0) {

#ifdef BCHP_XPT_FE_IB0_SYNC_COUNT
    case 0: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB0_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB1_SYNC_COUNT
    case 1: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB1_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB2_SYNC_COUNT
    case 2: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB2_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB3_SYNC_COUNT
    case 3: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB3_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB4_SYNC_COUNT
    case 4: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB4_SYNC_COUNT); break;
#endif

/* NOTE: We shouldn't have the same IB defined twice in the same chip, even if it supports both serial
and parallel. The duplicate case statements for 5 below should never fail the build. */
#ifdef BCHP_XPT_FE_IB5_SYNC_COUNT
    case 5: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB5_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB5P_SYNC_COUNT
    case 5: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB5P_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB6_SYNC_COUNT
    case 6: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB6_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB7_SYNC_COUNT
    case 7: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB7_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB8_SYNC_COUNT
    case 8: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB8_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB9_SYNC_COUNT
    case 9: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB9_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB10_SYNC_COUNT
    case 10: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB10_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB11_SYNC_COUNT
    case 11: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB11_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB12_SYNC_COUNT
    case 12: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB12_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB13_SYNC_COUNT
    case 13: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB13_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB14_SYNC_COUNT
    case 14: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB14_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB15_SYNC_COUNT
    case 15: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB15_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB16_SYNC_COUNT
    case 16: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB16_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB17_SYNC_COUNT
    case 17: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB17_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB18_SYNC_COUNT
    case 18: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB18_SYNC_COUNT); break;
#endif

#ifdef BCHP_XPT_FE_IB19_SYNC_COUNT
    case 19: pStatus->syncCount = BREG_Read32(g_pCoreHandles->reg, BCHP_XPT_FE_IB19_SYNC_COUNT); break;
#endif

    default:
    return NEXUS_INVALID_PARAMETER; /* no BERR_TRACE */
    }

    pStatus->overflowErrors = pTransport->overflow.inputBuffer;
    return 0;
}

void NEXUS_PidChannel_GetDefaultSettings(NEXUS_PidChannelSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->pidChannelIndex = NEXUS_PID_CHANNEL_OPEN_ANY;
    pSettings->enabled = true;
    pSettings->continuityCountEnabled = true;
    pSettings->remap.continuityCountEnabled = true;
}


NEXUS_PidChannelHandle NEXUS_PidChannel_P_Create(NEXUS_P_HwPidChannel *hwPidChannel)
{
    NEXUS_PidChannelHandle pidChannel;

    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, hwPidChannel);

    pidChannel = BKNI_Malloc(sizeof(*pidChannel));
    if(pidChannel==NULL) {(void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_malloc;}
    NEXUS_OBJECT_INIT(NEXUS_PidChannel, pidChannel);
    pidChannel->hwPidChannel = hwPidChannel;
    pidChannel->open = true;
    BLST_S_INSERT_HEAD(&hwPidChannel->swPidChannels, pidChannel, link);
    BDBG_MODULE_MSG(nexus_flow_pid_channel, ("create:%p(hw %p)", (void *)pidChannel, (void *)hwPidChannel));

    return pidChannel;
err_malloc:
    return NULL;
}

NEXUS_PidChannelHandle NEXUS_PidChannel_Open(NEXUS_ParserBand parserBand, uint16_t pid,
    const NEXUS_PidChannelSettings *pSettings)
{
    NEXUS_PidChannelHandle handle = NULL;
    NEXUS_ParserBandHandle band = NEXUS_ParserBand_Resolve_priv(parserBand);

    if (band)
    {
        NEXUS_P_HwPidChannel *hwPidChannel = NULL;
        hwPidChannel = NEXUS_P_HwPidChannel_Open(band, NULL, pid, pSettings, false);
        if(hwPidChannel) {
            hwPidChannel->status.parserBand = band->hwIndex;
            handle = NEXUS_PidChannel_P_Create(hwPidChannel);
            if(handle==NULL) {
                if(BLST_S_FIRST(&hwPidChannel->swPidChannels)==NULL) {
                    NEXUS_P_HwPidChannel_P_Close(hwPidChannel);
                }
            }
            else {
                NEXUS_Error rc;
                handle->enabled = pSettings?((pSettings->pidChannelIndex==NEXUS_PID_CHANNEL_OPEN_INJECTION)?false:pSettings->enabled):true;
                rc = NEXUS_P_HwPidChannel_CalcEnabled(hwPidChannel);
                if (rc) {
                    BERR_TRACE(NEXUS_INVALID_PARAMETER);
                    NEXUS_PidChannel_Close(handle);
                    handle = NULL;
                }
            }
        }
    }
    else {
        BDBG_ERR(("unable to resolve %lu", parserBand));
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    return handle;
}

/* find unused pidChannelIndex >= first and < bound */
static NEXUS_Error nexus_p_find_avail_pidch(unsigned first, unsigned bound, unsigned *index)
{
    struct NEXUS_P_HwPidChannel *p;
    unsigned i = first;
    /* pTransport->pidChannels is sorted by p->status.pidChannelIndex */
    for (p=BLST_S_FIRST(&pTransport->pidChannels);p;p=BLST_S_NEXT(p,link)) {
        if (p->status.pidChannelIndex >= bound) {
            break;
        }
        else if (p->status.pidChannelIndex == i) {
            if (++i == bound) {
                return NEXUS_NOT_AVAILABLE;
            }
        }
        else if (p->status.pidChannelIndex > i) {
            break;
        }
    }
    *index = i;
    return NEXUS_SUCCESS;
}

void nexus_p_set_pid_cc(NEXUS_P_HwPidChannel *hwPidChannel, const NEXUS_PidChannelSettings *pSettings)
{
#if BXPT_HAS_FULL_PID_PARSER
    BXPT_PidChannel_CC_Config cfg;
    bool bandContinuityCountEnabled = true;
    NEXUS_Error rc;

    if (hwPidChannel->parserBand)
    {
        bandContinuityCountEnabled = hwPidChannel->parserBand->settings.continuityCountEnabled && !hwPidChannel->parserBand->settings.allPass;
    }
    else if (hwPidChannel->playpump)
    {
        bandContinuityCountEnabled = hwPidChannel->playpump->settings.continuityCountEnabled && !hwPidChannel->playpump->settings.allPass;
    }
    if (NEXUS_GetEnv("cont_count_ignore")) {
        bandContinuityCountEnabled = false;
    }
    if (hwPidChannel->neverEnableContinuityCount) {
        bandContinuityCountEnabled = false;
    }

    BXPT_GetPidChannel_CC_Config(pTransport->xpt, hwPidChannel->status.pidChannelIndex, &cfg);
    cfg.Primary_CC_CheckEnable = pSettings->continuityCountEnabled && bandContinuityCountEnabled;
    cfg.Secondary_CC_CheckEnable = pSettings->remap.continuityCountEnabled && bandContinuityCountEnabled;
    cfg.Generate_CC_Enable = pSettings->generateContinuityCount;
    rc = BXPT_SetPidChannel_CC_Config(pTransport->xpt, hwPidChannel->status.pidChannelIndex, &cfg);
    if (rc) BERR_TRACE(rc); /* should never happen, so don't propagate */
#else
    BSTD_UNUSED(hwPidChannel);
    BSTD_UNUSED(pSettings);
#endif
}

NEXUS_P_HwPidChannel *NEXUS_P_HwPidChannel_Open(NEXUS_ParserBandHandle parserBand, NEXUS_PlaypumpHandle playpump, unsigned combinedPid,
    const NEXUS_PidChannelSettings *pSettings, bool neverEnableContinuityCount)
{
    NEXUS_P_HwPidChannel *duplicatePidChannel = NULL;
    NEXUS_P_HwPidChannel *pidChannel;
    unsigned index; /* hw index */
    BERR_Code rc;
    NEXUS_PidChannelSettings settings;
    uint16_t pid = combinedPid&0xFFFF; /* virtual_pid could also include the sub-stream id */

    NEXUS_ASSERT_MODULE();

    if (!pSettings) {
        NEXUS_PidChannel_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    if (playpump) {
        parserBand = NULL; /* parserBand is invalid if playpump is set. */
    }
    else {
        BDBG_OBJECT_ASSERT(parserBand, NEXUS_ParserBand);
    }

    /* user specifies the HW pid channel */
    if ((int)pSettings->pidChannelIndex >= 0) {
        index = pSettings->pidChannelIndex;
        if (index >= NEXUS_NUM_PID_CHANNELS) {
            BDBG_ERR(("Invalid pid channel index %d", index));
            return NULL;
        }

        /* if already opened */
        for (pidChannel = BLST_S_FIRST(&pTransport->pidChannels); pidChannel; pidChannel = BLST_S_NEXT(pidChannel, link)) {
            if (pidChannel->status.pidChannelIndex == (unsigned)pSettings->pidChannelIndex) {
                /* must be the same pid */
                if (pidChannel->status.pid != pid) {
                    BDBG_ERR(("Cannot open the same pidChannelIndex (%d) with different pids (0x%x and 0x%x)", pSettings->pidChannelIndex, pidChannel->status.pid, pid));
                    return NULL;
                }
                if (pidChannel->settings.pidChannelIndex==NEXUS_PID_CHANNEL_OPEN_INJECTION) {
                    BDBG_ERR(("pidChannelIndex %d already opened with NEXUS_PID_CHANNEL_OPEN_INJECTION", pSettings->pidChannelIndex));
                    return NULL;
                }
                duplicatePidChannel = pidChannel;
                break;
            }
            else if (pidChannel->status.pidChannelIndex > (unsigned)pSettings->pidChannelIndex) {
                break;
            }
        }
    }
    else {
        /* nexus assigns the HW pid channel */
        bool found = false;

        /* first, see if the same HW pid channel is already opened */
        for (pidChannel = BLST_S_FIRST(&pTransport->pidChannels); pidChannel; pidChannel = BLST_S_NEXT(pidChannel, link)) {
#if B_REFSW_DSS_SUPPORT
            /* a different ENABLE_HD_FILTER value allows for a duplicate pid channel */
            if (pidChannel->settings.dssHdFilter != pSettings->dssHdFilter) {
                continue;
            }
#endif

            /* playback */
            if (playpump &&
                pidChannel->status.playback &&
                pidChannel->status.playbackIndex == playpump->index &&
                pidChannel->status.pid == pid)
            {
                break;
            }
            /* live */
            if (!playpump &&
                !pidChannel->status.playback &&
                pidChannel->parserBand == parserBand &&
                (pidChannel->status.pid == pid
                || (parserBand->settings.allPass
                    && parserBand->settings.cableCard == NEXUS_CableCardType_eNone ) ))
            {
                break;
            }
        }
        if (pidChannel &&
            (pidChannel->settings.pidChannelIndex!=NEXUS_PID_CHANNEL_OPEN_INJECTION && pSettings->pidChannelIndex!=NEXUS_PID_CHANNEL_OPEN_INJECTION))
        {
            /* we can/must reuse the HW pid channel */
            duplicatePidChannel = pidChannel;
            index = duplicatePidChannel->status.pidChannelIndex;
            found = true;
        }

        /* we're not already using it, so find an available HW pid channel. */

        /* first, handle allpass special case */
        if (!found &&
            !playpump &&
            parserBand->settings.allPass &&
            parserBand->settings.cableCard == NEXUS_CableCardType_eNone)
        {
            /* allPass is a special mode where HW pid channel must == parser band. */
            /* ignore duplicatePidChannel. this is a test scenario anyway. */
            NEXUS_ParserBand_GetAllPassPidChannelIndex((NEXUS_ParserBand)parserBand, &index);
            found = true;
        }

        /* otherwise, we must search. pid channels are arranged in this pattern of capabilities:
        group 1: live parser band all pass capable, msg filter capable
        group 2: msg filter capable
        group 3: playback parser band all pass capable, msg filter capable
        group 4: msg filter capable
        group 5: not msg filter capable
        if we are searching for ANY, we search in this order: group 4, group 5, group 2, group 1, group 3.
        MESSAGE_CAPABLE is like ANY, but excluding group 5
        NOT_MESSAGE_CAPABLE is group 5 only
        */
        /* Some parts, such as the 7550, don't have message filtering hardware. BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS
           won't be defined for those chips. */
#ifdef BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS
        BDBG_CASSERT(BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS <= NEXUS_NUM_PID_CHANNELS);
#endif
        if (!found) {
            unsigned firstPidChannel;
            unsigned totalPidChannels;
            unsigned firstNonAllPass;
#if NEXUS_NUM_PLAYPUMPS
            firstNonAllPass = BXPT_GET_PLAYBACK_ALLPASS_CHNL(NEXUS_NUM_PLAYPUMPS-1) + 1;
#else
            firstNonAllPass = BXPT_GET_IB_PARSER_ALLPASS_CHNL(NEXUS_NUM_PARSER_BANDS-1) + 1;
#endif
            switch (pSettings->pidChannelIndex) {
            case NEXUS_PID_CHANNEL_OPEN_ANY:
            case NEXUS_PID_CHANNEL_OPEN_INJECTION:
                firstPidChannel = firstNonAllPass;
                totalPidChannels = NEXUS_NUM_PID_CHANNELS;
                break;

#ifdef BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS
            case NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE:
                firstPidChannel = firstNonAllPass;
                totalPidChannels = BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS;
                break;
            case NEXUS_PID_CHANNEL_OPEN_NOT_MESSAGE_CAPABLE:
                /* On some chips, such as the 7435, *all* PID channels are message capable. Handle that case here, otherwise
                the for() loop below will terminate with no channels found. */
                firstPidChannel = BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS == NEXUS_NUM_PID_CHANNELS ?
                    firstNonAllPass : BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS;
                totalPidChannels = NEXUS_NUM_PID_CHANNELS;
                break;
#else
            case NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE:
                BDBG_ERR(("Message filtering not support in hw."));
                BERR_TRACE(NEXUS_NOT_SUPPORTED);
                return NULL;
            case NEXUS_PID_CHANNEL_OPEN_NOT_MESSAGE_CAPABLE:
                firstPidChannel = firstNonAllPass;
                totalPidChannels = NEXUS_NUM_PID_CHANNELS;
                break;
#endif

            default:
                BDBG_ERR(("invalid pidChannelIndex %d", pSettings->pidChannelIndex));
                return NULL;
            }

            if (!nexus_p_find_avail_pidch(firstPidChannel, totalPidChannels, &index)) {
                found = true;
            }
            if (!found && pSettings->pidChannelIndex != NEXUS_PID_CHANNEL_OPEN_NOT_MESSAGE_CAPABLE) {
#if NEXUS_NUM_PLAYPUMPS && NEXUS_NUM_PARSER_BANDS
                /* search group 2 */
                {
                    unsigned first = BXPT_GET_IB_PARSER_ALLPASS_CHNL(NEXUS_NUM_PARSER_BANDS-1) + 1;
                    unsigned bound = BXPT_GET_PLAYBACK_ALLPASS_CHNL(0);
                    if (!nexus_p_find_avail_pidch(first, bound, &index)) {
                        found = true;
                    }
                }
#endif
                if (!found) {
                    /* search group 1 through 3 */
                    if (!nexus_p_find_avail_pidch(0, firstNonAllPass, &index)) {
                        found = true;
                    }
                }
            }
        }
        if (!found) {
            BDBG_ERR(("No more pid channels available: pid %#x, pidChannelIndex %d", pid, pSettings->pidChannelIndex));
            return NULL;
        }
    }

    /* Don't call BXPT_AllocPidChannel because we need the option of selecting the index.
    BXPT_ConfigurePidChannel will reserve it just the same. */

    /* If the parser we want is in allPass mode, make sure the correct PID channel is used. Similar code in
       NEXUS_ParserBand_SetSettings() will cover the possiblity that the parser is configured after the PID channel. */
    if (!playpump && parserBand->settings.allPass && parserBand->settings.cableCard == NEXUS_CableCardType_eNone) {
        unsigned AllPassPidChannel;
        NEXUS_ParserBand_GetAllPassPidChannelIndex((NEXUS_ParserBand)parserBand, &AllPassPidChannel);
        if (index != AllPassPidChannel )
        {
            BDBG_ERR(("Incorrect PID channel used for allPass on parser band %p. See NEXUS_ParserBand_GetAllPassPidChannelIndex()", (void *)parserBand));
            return NULL;
        }
    }

    if (duplicatePidChannel) {
        /* test if new dup pid channel has conflicting settings. can't just memcmp settings because some don't apply.
        ignore requireMessageBuffer and pidChannelIndex. we already know the HW pid channel index matches.
        enabled doesn't have to match. it will be and-ed with others.
        others must match.
        */
        if (pSettings->dssHdFilter != duplicatePidChannel->settings.dssHdFilter ||
            pSettings->continuityCountEnabled != duplicatePidChannel->settings.continuityCountEnabled ||
            pSettings->generateContinuityCount != duplicatePidChannel->settings.generateContinuityCount ||
            pSettings->remap.enabled != duplicatePidChannel->settings.remap.enabled ||
            pSettings->remap.continuityCountEnabled != duplicatePidChannel->settings.remap.continuityCountEnabled ||
            pSettings->remap.pid != duplicatePidChannel->settings.remap.pid)
        {
            BDBG_ERR(("Cannot open duplicate pid channel with different settings"));
            return NULL;
        }
        if (playpump) {
            /* may have disconnected from closed playpump, even though HW pidchannel held open. must reconnect. */
            duplicatePidChannel->playpump = playpump;
        }
        return duplicatePidChannel;
    }

    pidChannel = BKNI_Malloc(sizeof(*pidChannel));
    if (!pidChannel) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_P_HwPidChannel, pidChannel);
    BLST_S_INIT(&pidChannel->swPidChannels);
    pidChannel->settings = *pSettings;
    pidChannel->status.pid = pid;
    pidChannel->status.pidChannelIndex = index;
    pidChannel->combinedPid = combinedPid;
    pidChannel->settingsPrivValid = false;
    pidChannel->neverEnableContinuityCount = neverEnableContinuityCount;

    /* insert in sorted order by pidChannelIndex */
    {
        NEXUS_P_HwPidChannel *p, *prev;
        bool duplicate = false;
        BKNI_EnterCriticalSection();
        for (prev=NULL,p=BLST_S_FIRST(&pTransport->pidChannels);p;prev=p,p=BLST_S_NEXT(p,link)) {
            if (p->status.pidChannelIndex == index) {
                duplicate = true;
                break;
            }
            if (index < p->status.pidChannelIndex) break;
        }
        if (!duplicate) {
            if (prev) {
                BLST_S_INSERT_AFTER(&pTransport->pidChannels, prev, pidChannel, link);
            }
            else {
                BLST_S_INSERT_HEAD(&pTransport->pidChannels, pidChannel, link);
            }
        }
        BKNI_LeaveCriticalSection();
        if (duplicate) {
            BKNI_Free(pidChannel);
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return NULL;
        }
    }

    if (playpump)
    {
        pidChannel->playpump = playpump;
        pidChannel->status.playback = true;
        pidChannel->status.playbackIndex = playpump->index;
        pidChannel->status.parserBand = 0xffff; /* invalid value */
    }
    else
    {
        pidChannel->parserBand = parserBand;
        pidChannel->status.playbackIndex = 0xffff; /* invalid value */
        pidChannel->status.parserBand = parserBand->hwIndex;
        /* must inc parserBand refcnt here. if it fails later, NEXUS_PidChannel_P_Close will decrement */
        parserBand->refcnt++;
        /* status.transportType gets overwritten by Playpump if it packetizes */
        pidChannel->status.transportType = pidChannel->status.originalTransportType = parserBand->settings.transportType;
    }
    BDBG_MODULE_MSG(nexus_flow_pid_channel, ("open %p, hw channel %d, pid %#x, %s %d", (void *)pidChannel, index, pid,
        playpump?"playback":"live",
        playpump?pidChannel->status.playbackIndex:pidChannel->status.parserBand
        ));

    pidChannel->status.remappedPid = pid;
    pidChannel->status.continuityCountErrors = 0;

    {
#if B_REFSW_DSS_SUPPORT
        /* If we're not in DSS mode, HD filtering needs to be disabled BEFORE configuring the PID table */
        rc = BXPT_DirecTv_ConfigHdFiltering(pTransport->xpt, pidChannel->status.pidChannelIndex,
            pSettings->dssHdFilter != NEXUS_PidChannelDssHdFilter_eDisabled,
            ((pSettings->dssHdFilter == NEXUS_PidChannelDssHdFilter_eAux) ? BXPT_HdFilterMode_eAuxOnly : BXPT_HdFilterMode_eNonAuxOnly));
        if (rc) {rc=BERR_TRACE(rc); goto fail1;}
#endif

        /* Configure it */
        if (pSettings->remap.enabled) {
            /* the old pid goes in */
            rc = BXPT_Spid_ConfigureChannel(pTransport->xpt, pidChannel->status.pidChannelIndex, pid, BXPT_Spid_eChannelMode_Remap);
            if (rc) {rc=BERR_TRACE(rc); goto fail1;}

            /* the new pid comes out */
            rc = BXPT_ConfigurePidChannel(pTransport->xpt, pidChannel->status.pidChannelIndex, pSettings->remap.pid, nexus_p_xpt_parser_band(pidChannel));
            if (rc) {rc=BERR_TRACE(rc); goto fail1;}

            pidChannel->status.remappedPid = pSettings->remap.pid; /* this should be the new pid */
        }
        else {
            (void)BXPT_Spid_ConfigureChannel(pTransport->xpt, pidChannel->status.pidChannelIndex, 0, BXPT_Spid_eChannelMode_Disable);

            rc = BXPT_ConfigurePidChannel(pTransport->xpt, pidChannel->status.pidChannelIndex, pid, nexus_p_xpt_parser_band(pidChannel));
            if (rc) {rc=BERR_TRACE(rc); goto fail1;}
        }

        nexus_p_set_pid_cc(pidChannel, pSettings);
    }

    if (!playpump) {
        NEXUS_ParserBand_P_SetEnable(parserBand);
        parserBand->pidChannels++;
        if (parserBand->settings.sourceType == NEXUS_ParserBandSourceType_eMtsif) {
            pTransport->mtsifPidChannelState[index] = NEXUS_MtsifPidChannelState_eChanged;
            BKNI_SetEvent(pTransport->pidChannelEvent);
        }
    }
    return pidChannel;

fail1:
    if (pidChannel) {
        NEXUS_P_HwPidChannel_P_Close(pidChannel);
    }
    return NULL;
}

static void NEXUS_P_HwPidChannel_Disconnect(NEXUS_P_HwPidChannel *hwPidChannel)
{
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, hwPidChannel);
    if (hwPidChannel->playpump) {
        NEXUS_Playpump_P_HwPidChannel_Disconnect(hwPidChannel->playpump, hwPidChannel);
        hwPidChannel->playpump = NULL;
    }
    else if (hwPidChannel->parserBand) {
        hwPidChannel->parserBand->pidChannels--;
        BDBG_ASSERT(hwPidChannel->parserBand->refcnt);
        hwPidChannel->parserBand->refcnt--;
        NEXUS_ParserBand_P_SetEnable(hwPidChannel->parserBand);
        hwPidChannel->parserBand = NULL;
    }
    else if (hwPidChannel->dma) {
        #if NEXUS_HAS_XPT_DMA
        NEXUS_DmaJob_P_HwPidChannel_Disconnect(hwPidChannel->dma, hwPidChannel);
        hwPidChannel->dma = NULL;
        #endif
    }
}

/* this function would close all pidchannels that were tied to a HW pidChannel */
void NEXUS_P_HwPidChannel_CloseAll(NEXUS_P_HwPidChannel *hwPidChannel)
{
    NEXUS_PidChannelHandle pidChannelNext;
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, hwPidChannel);

    /* disconnect from playpump/parserband now in case refcnt keeps a SW pid channel open.
    it is now just a disabled HW pidchannel. */
    NEXUS_P_HwPidChannel_Disconnect(hwPidChannel);

    pidChannelNext=BLST_S_FIRST(&hwPidChannel->swPidChannels);
    while(pidChannelNext) {
        NEXUS_PidChannelHandle pidChannel = pidChannelNext;
        pidChannelNext = BLST_S_NEXT(pidChannel, link);

        if (pidChannel->open) {
            NEXUS_PidChannel_Close(pidChannel);
            /* public API close no longer callable. but refcnt may keep disconnect HW pidchannel open */
        }
    }

    return;
}

static void NEXUS_P_HwPidChannel_P_Finalizer(NEXUS_P_HwPidChannel *pidChannel)
{
    unsigned index; /* hw index */

    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, pidChannel);
    BDBG_MODULE_MSG(nexus_flow_pid_channel, ("close %p", (void *)pidChannel));

    BDBG_ASSERT(BLST_S_FIRST(&pidChannel->swPidChannels)==NULL); /* when this function called all SW pidChannels should be already deleted */
    NEXUS_P_HwPidChannel_Disconnect(pidChannel);

#if BXPT_HAS_RAVE_SCRAMBLING_CONTROL
    NEXUS_P_HwPidChannel_StopScramblingCheck(pidChannel);
#endif

    if (pidChannel->settings.remap.enabled) {
        (void)BXPT_Spid_ConfigureChannel(pTransport->xpt, pidChannel->status.pidChannelIndex, 0, BXPT_Spid_eChannelMode_Disable);
    }

    index = pidChannel->status.pidChannelIndex;
    if (index < NEXUS_NUM_PID_CHANNELS) {
        BXPT_DisablePidChannel(pTransport->xpt, index);
        BXPT_FreePidChannel(pTransport->xpt, index);

        BKNI_EnterCriticalSection();
        BLST_S_REMOVE(&pTransport->pidChannels, pidChannel, NEXUS_P_HwPidChannel, link);
        BKNI_LeaveCriticalSection();

        if (pTransport->mtsifPidChannelState[index] == NEXUS_MtsifPidChannelState_eChanged) {
            pTransport->mtsifPidChannelState[index] = NEXUS_MtsifPidChannelState_eClosed;
            BKNI_SetEvent(pTransport->pidChannelEvent);
        }
    }

    NEXUS_OBJECT_DESTROY(NEXUS_P_HwPidChannel, pidChannel);
    BKNI_Free(pidChannel);
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_P_HwPidChannel, NEXUS_P_HwPidChannel_P_Close);

static void NEXUS_PidChannel_P_Release(NEXUS_PidChannelHandle pidChannel)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_PidChannel, pidChannel, Close);
    /* CloseAll should not call public Close again */
    pidChannel->open = false;

    /* If this pidchannel is assigned to the bypassKeyslot, and if this is the last public Close,
    then we have refcnt leak. Nexus will automatically sweep when the client exits, but this is
    a hint for fixing an app that doesn't call NEXUS_SetPidChannelBypassKeyslot(NEXUS_BypassKeySlot_eG2GR)
    properly and is not exiting. */
    if (pidChannel->hwPidChannel->bypassKeyslotPidChannel) {
        NEXUS_PidChannelHandle p;
        for (p = BLST_S_FIRST(&pidChannel->hwPidChannel->swPidChannels);p;p = BLST_S_NEXT(p, link)) {
            if (p->open) break;
        }
        if (!p) {
            BDBG_WRN(("NEXUS_SetPidChannelBypassKeyslot PidChannel %p leak. Will only cleanup on client exit.", (void*)pidChannel));
        }
    }
}

static void NEXUS_PidChannel_P_Finalizer(NEXUS_PidChannelHandle pidChannel)
{
    NEXUS_P_HwPidChannel *hwPidChannel;
    bool wasEnabled;
    NEXUS_OBJECT_ASSERT(NEXUS_PidChannel, pidChannel);
    BDBG_MODULE_MSG(nexus_flow_pid_channel, ("destroy %p", (void *)pidChannel));
    wasEnabled = pidChannel->enabled;
    hwPidChannel = pidChannel->hwPidChannel;
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, hwPidChannel);
    BLST_S_REMOVE(&hwPidChannel->swPidChannels, pidChannel, NEXUS_PidChannel, link);
    NEXUS_OBJECT_DESTROY(NEXUS_PidChannel, pidChannel);
    BKNI_Free(pidChannel);
    if(BLST_S_FIRST(&hwPidChannel->swPidChannels)==NULL) { /* all swPidChannels were closed */
        NEXUS_P_HwPidChannel_P_Close(hwPidChannel);
    }
    else if (wasEnabled) {
        (void)NEXUS_P_HwPidChannel_CalcEnabled(hwPidChannel);
    }
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_PidChannel, NEXUS_PidChannel_Close);


void NEXUS_PidChannel_CloseAll(NEXUS_ParserBand parserBand)
{
    NEXUS_ParserBandHandle handle;
    NEXUS_P_HwPidChannel *pidChannel;

    handle = NEXUS_ParserBand_Resolve_priv(parserBand);
    if (!handle) return;

    for (pidChannel = BLST_S_FIRST(&pTransport->pidChannels); pidChannel;) {
        NEXUS_P_HwPidChannel *next = BLST_S_NEXT(pidChannel, link);
        if (pidChannel->parserBand == handle) {
            NEXUS_P_HwPidChannel_CloseAll(pidChannel);
        }
        pidChannel = next;
    }
}

NEXUS_Error NEXUS_P_HwPidChannel_GetStatus(NEXUS_P_HwPidChannel *pidChannel, NEXUS_PidChannelStatus *pStatus)
{
    struct NEXUS_Rave_P_ErrorCounter_Link *raveLink;
    struct NEXUS_Rave_P_ErrorCounter *counter;
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, pidChannel);
    /* no need to check pidChannel->refcnt. if zero, we should have already asserted */
    *pStatus = pidChannel->status;

    /* sum errors on all rave contexts connected to this pid channel */
    BKNI_Memset(&pStatus->raveStatus, 0, sizeof(pStatus->raveStatus));

    for (raveLink = BLST_S_FIRST(&pidChannel->raveCounters); raveLink; raveLink = BLST_S_NEXT(raveLink, pidchannel_link)) {
        counter = raveLink->counter;
        #if 0
        BDBG_MSG(("pidChannel(%u) rave status: %u %u %u %u %u %u", pidChannel->status.pidChannelIndex,
            counter->ccErr, counter->emuErr, counter->pusiErr, counter->teiErr, counter->cdbOverflow, counter->itbOverflow));
        #endif

        BKNI_EnterCriticalSection();
        pStatus->raveStatus.continuityCountErrors += counter->ccErr;
        pStatus->raveStatus.emulationByteRemovalErrors = counter->emuErr;
        pStatus->raveStatus.pusiErrors += counter->pusiErr;
        pStatus->raveStatus.teiErrors += counter->teiErr;
        pStatus->raveStatus.cdbOverflowErrors += counter->cdbOverflow;
        pStatus->raveStatus.itbOverflowErrors += counter->itbOverflow;
        BKNI_LeaveCriticalSection();
    }

    /* sum xcbuff overflows */
    BKNI_Memset(&pStatus->xcBufferStatus, 0, sizeof(pStatus->xcBufferStatus));
    if (pidChannel->parserBand && pidChannel->parserBand->hwIndex<32) {
        unsigned ibp = pidChannel->parserBand->hwIndex;
        if (pidChannel->destinations & NEXUS_PIDCHANNEL_P_DESTINATION_RAVE) {
            pStatus->xcBufferStatus.overflowErrors += pTransport->overflow.xcbuff.ibp2rave[ibp];
        }
        if (pidChannel->destinations & NEXUS_PIDCHANNEL_P_DESTINATION_MESSAGE) {
            pStatus->xcBufferStatus.overflowErrors += pTransport->overflow.xcbuff.ibp2msg[ibp];
        }
        if (pidChannel->destinations & NEXUS_PIDCHANNEL_P_DESTINATION_REMUX0) {
            pStatus->xcBufferStatus.overflowErrors += pTransport->overflow.xcbuff.ibp2rmx0[ibp];
        }
        if (pidChannel->destinations & NEXUS_PIDCHANNEL_P_DESTINATION_REMUX1) {
            pStatus->xcBufferStatus.overflowErrors += pTransport->overflow.xcbuff.ibp2rmx1[ibp];
        }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_PidChannel_GetStatus(NEXUS_PidChannelHandle pidChannel, NEXUS_PidChannelStatus *pStatus)
{
    NEXUS_OBJECT_ASSERT(NEXUS_PidChannel, pidChannel);
    return NEXUS_P_HwPidChannel_GetStatus(pidChannel->hwPidChannel, pStatus);
}

static void NEXUS_P_HwPidChannel_ConsumerStarted(NEXUS_P_HwPidChannel *pidChannel)
{
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, pidChannel);
    if (pidChannel->status.playback) {
        NEXUS_PlaypumpHandle playpump;

        BDBG_MSG(("NEXUS_PidChannel_ConsumerStarted on playback %d", pidChannel->status.playbackIndex));
#if NEXUS_NUM_PLAYPUMPS
        if (pidChannel->status.playbackIndex < NEXUS_NUM_PLAYPUMPS) {
            playpump = pTransport->playpump[pidChannel->status.playbackIndex].playpump;
        }
        else
#endif
        {
            playpump = NULL;
        }
        if (playpump) {
            NEXUS_Playpump_P_ConsumerStarted(playpump);
        } else {
           BDBG_ERR(("NEXUS_PidChannel_ConsumerStarted: %#lx invalid state", (unsigned long)pidChannel));
        }
    }
}

void NEXUS_PidChannel_ConsumerStarted(NEXUS_PidChannelHandle pidChannel)
{
    NEXUS_OBJECT_ASSERT(NEXUS_PidChannel, pidChannel);
    NEXUS_P_HwPidChannel_ConsumerStarted(pidChannel->hwPidChannel);
}

/**
TODO: consider adding pTransport->rave[].splicePidChannel list to keep track. for now, this is a seldomly used API, so we'll let XPT keep track.
 also consider adding pidChannel->ravelist to avoid the look up. for now, it's acceptable.
**/
static NEXUS_Error NEXUS_P_HwPidChannel_AddSplicePidChannel( NEXUS_P_HwPidChannel *pidChannel, NEXUS_P_HwPidChannel *splicePidChannel)
{
    NEXUS_Error rc;
    NEXUS_RaveHandle rave;
    unsigned i,j;
    bool found = false;

    for (j=0;j<BXPT_NUM_RAVE_CHANNELS;j++) {
        for (i=0;i<BXPT_NUM_RAVE_CONTEXTS;i++) {
            rave = pTransport->rave[j].context[i];
            if (rave && rave->pidChannel && rave->pidChannel->status.pidChannelIndex == pidChannel->status.pidChannelIndex && rave->raveHandle) {
                rc = BXPT_Rave_PushPidChannel(rave->raveHandle, pidChannel->status.pidChannelIndex, splicePidChannel->status.pidChannelIndex);
                if (rc) return BERR_TRACE(rc);
                found = true;
            }
        }
    }

    if (!found) {
        BDBG_ERR(("Unable to look up RAVE context. The NEXUS_PidChannel must be currently decoding to allow pid splicing."));
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_PidChannel_AddSplicePidChannel( NEXUS_PidChannelHandle pidChannel, NEXUS_PidChannelHandle splicePidChannel )
{
    return NEXUS_P_HwPidChannel_AddSplicePidChannel(pidChannel->hwPidChannel, splicePidChannel->hwPidChannel);
}


static NEXUS_Error NEXUS_P_HwPidChannel_RemoveSplicePidChannel( NEXUS_P_HwPidChannel *pidChannel, NEXUS_P_HwPidChannel *splicePidChannel )
{
    NEXUS_Error rc;
    NEXUS_RaveHandle rave;
    unsigned i,j;
    bool found = false;

    for (j=0;j<BXPT_NUM_RAVE_CHANNELS;j++) {
        for (i=0;i<BXPT_NUM_RAVE_CONTEXTS;i++) {
            rave = pTransport->rave[j].context[i];
            if (rave && rave->pidChannel && rave->pidChannel->status.pidChannelIndex == pidChannel->status.pidChannelIndex && rave->raveHandle) {
                rc = BXPT_Rave_RemovePidChannel(rave->raveHandle, splicePidChannel->status.pidChannelIndex);
                if (rc) return BERR_TRACE(rc);
                found = true;
            }
        }
    }

    if (!found) {
        BDBG_ERR(("Unable to look up RAVE context. The NEXUS_PidChannel must be currently decoding to allow pid splicing."));
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_PidChannel_RemoveSplicePidChannel( NEXUS_PidChannelHandle pidChannel, NEXUS_PidChannelHandle splicePidChannel )
{
    return NEXUS_P_HwPidChannel_RemoveSplicePidChannel(pidChannel->hwPidChannel, splicePidChannel->hwPidChannel);
}

static void NEXUS_P_HwPidChannel_RemoveAllSplicePidChannels( NEXUS_P_HwPidChannel *pidChannel )
{
    NEXUS_Error rc;
    NEXUS_RaveHandle rave;
    unsigned i,j;
    bool found = false;

    for (j=0;j<BXPT_NUM_RAVE_CHANNELS;j++) {
        for (i=0;i<BXPT_NUM_RAVE_CONTEXTS;i++) {
            rave = pTransport->rave[j].context[i];
            if (rave && rave->pidChannel && rave->pidChannel->status.pidChannelIndex == pidChannel->status.pidChannelIndex && rave->raveHandle) {
                rc = BXPT_Rave_ClearQueue(rave->raveHandle);
                if (rc) rc = BERR_TRACE(rc);
                found = true;
            }
        }
    }

    if (!found) {
        BDBG_ERR(("Unable to look up RAVE context. The NEXUS_PidChannel must be currently decoding to allow pid splicing."));
        rc = BERR_TRACE(NEXUS_UNKNOWN);
    }
}

void NEXUS_PidChannel_RemoveAllSplicePidChannels( NEXUS_PidChannelHandle pidChannel )
{
    NEXUS_P_HwPidChannel_RemoveAllSplicePidChannels( pidChannel->hwPidChannel);
}

NEXUS_Error NEXUS_PidChannel_SetEnabled( NEXUS_PidChannelHandle pidChannel, bool enabled )
{
    if ((pidChannel->hwPidChannel->settings.pidChannelIndex==NEXUS_PID_CHANNEL_OPEN_INJECTION) && enabled) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (pidChannel->enabled != enabled) {
        pidChannel->enabled = enabled;
        return NEXUS_P_HwPidChannel_CalcEnabled(pidChannel->hwPidChannel);
    }
    else {
        return NEXUS_SUCCESS;
    }
}

NEXUS_Error NEXUS_P_HwPidChannel_CalcEnabled( NEXUS_P_HwPidChannel *pidChannel )
{
    NEXUS_PidChannelHandle p;
    bool enabled;

    p = BLST_S_FIRST(&pidChannel->swPidChannels);
    enabled = (p != NULL);
    for (;p && enabled;p=BLST_S_NEXT(p,link)) {
        if (!p->enabled) enabled = false;
    }
    if (pidChannel->settings.pidChannelIndex == NEXUS_PID_CHANNEL_OPEN_INJECTION) {
        BDBG_ASSERT(!enabled);
    }
    return NEXUS_P_HwPidChannel_SetEnabled(pidChannel, enabled);
}

static NEXUS_Error NEXUS_P_HwPidChannel_SetEnabled(NEXUS_P_HwPidChannel *pidChannel, bool enabled)
{
    if ( pidChannel->status.enabled != enabled )
    {
        BERR_Code errCode;

        if ( enabled )
        {
            errCode = BXPT_EnablePidChannel(pTransport->xpt, pidChannel->status.pidChannelIndex);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
        else
        {
            errCode = BXPT_DisablePidChannel(pTransport->xpt, pidChannel->status.pidChannelIndex);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
        pidChannel->status.enabled = enabled;
    }

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_P_HwPidChannel_SetRemapSettings( NEXUS_P_HwPidChannel *pidChannel, const NEXUS_PidChannelRemapSettings *pSettings)
{
    /* SW7344-192, Coverity: 35340 */
    BERR_Code rc = NEXUS_SUCCESS;
    bool wasEnabled = false;

    NEXUS_ASSERT_MODULE();
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, pidChannel);
    BDBG_ASSERT(pSettings);

    if (pidChannel->status.enabled) {
        rc = NEXUS_P_HwPidChannel_SetEnabled(pidChannel, false);
        if (rc) {rc=BERR_TRACE(rc); goto fail;}
        wasEnabled = true;
    }

    if (pSettings->enabled) {
        /* the old pid goes in */
        rc = BXPT_Spid_ConfigureChannel(pTransport->xpt, pidChannel->status.pidChannelIndex, pidChannel->status.pid, BXPT_Spid_eChannelMode_Remap);
        if (rc) {rc=BERR_TRACE(rc); goto fail;}

        /* the new pid comes out */
        rc = BXPT_ConfigurePidChannel(pTransport->xpt, pidChannel->status.pidChannelIndex, pSettings->pid, pidChannel->playpump?BXPT_PB_PARSER(pidChannel->playpump->index):pidChannel->parserBand->hwIndex);
        if (rc) {rc=BERR_TRACE(rc); goto fail;}

        pidChannel->status.remappedPid = pSettings->pid; /* this should be the new pid */

        {
            NEXUS_PidChannelSettings settings = pidChannel->settings;
            settings.remap = *pSettings;
            nexus_p_set_pid_cc(pidChannel, &settings);
        }

        pidChannel->settings.remap = *pSettings;

    }
    else {
        if (pidChannel->settings.remap.pid) {
            (void)BXPT_Spid_ConfigureChannel(pTransport->xpt, pidChannel->status.pidChannelIndex, 0, BXPT_Spid_eChannelMode_Disable);
        }
    }

fail:
    if (wasEnabled) {
        /* even on failure condition, we want to reenable */
        (void)NEXUS_P_HwPidChannel_CalcEnabled(pidChannel);
    }
    return rc;
}

NEXUS_Error NEXUS_PidChannel_SetRemapSettings( NEXUS_PidChannelHandle pidChannel, const NEXUS_PidChannelRemapSettings *pSettings)
{
    return NEXUS_P_HwPidChannel_SetRemapSettings(pidChannel->hwPidChannel, pSettings);
}

static NEXUS_Error NEXUS_P_HwPidChannel_ResetStatus( NEXUS_P_HwPidChannel *pidChannel )
{
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, pidChannel);
    pidChannel->status.continuityCountErrors = 0;
    if (pidChannel->parserBand != NULL)
        NEXUS_ParserBand_P_ResetOverflowCounts(pidChannel->parserBand);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_PidChannel_ResetStatus( NEXUS_PidChannelHandle pidChannel )
{
    return NEXUS_P_HwPidChannel_ResetStatus(pidChannel->hwPidChannel);
}

void NEXUS_PidChannel_GetDefaultSlaveSettings( NEXUS_PidChannelSlaveSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static NEXUS_Error NEXUS_P_HwPidChannel_AddSlavePidChannel( NEXUS_P_HwPidChannel *master, NEXUS_P_HwPidChannel *slave, const NEXUS_PidChannelSlaveSettings *pSettings )
{
    int rc;
    NEXUS_RaveHandle rave;

    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, master);
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, slave);
    BSTD_UNUSED(pSettings);

    if (slave->master) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (master->master) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BLST_S_INSERT_HEAD(&master->slaves, slave, slave_link);
    slave->master = master;

    for (rave = BLST_S_FIRST(&master->raves); rave; rave = BLST_S_NEXT(rave, pidchannel_link)) {
        rc = nexus_rave_add_one_pid(rave, slave);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }
    return 0;

error:
    NEXUS_P_HwPidChannel_RemoveSlavePidChannel(master, slave);
    return rc;
}

NEXUS_Error NEXUS_PidChannel_AddSlavePidChannel( NEXUS_PidChannelHandle master, NEXUS_PidChannelHandle slave, const NEXUS_PidChannelSlaveSettings *pSettings )
{
    return NEXUS_P_HwPidChannel_AddSlavePidChannel(master->hwPidChannel, slave->hwPidChannel, pSettings);
}

static void NEXUS_P_HwPidChannel_RemoveSlavePidChannel( NEXUS_P_HwPidChannel *master, NEXUS_P_HwPidChannel *slave )
{
    NEXUS_RaveHandle rave;

    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, master);
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, slave);

    if (slave->master != master) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }

    BLST_S_REMOVE(&master->slaves, slave, NEXUS_P_HwPidChannel, slave_link);
    slave->master = NULL;

    for (rave = BLST_S_FIRST(&master->raves); rave; rave = BLST_S_NEXT(rave, pidchannel_link)) {
        nexus_rave_remove_one_pid(rave, slave);
    }
}

void NEXUS_PidChannel_RemoveSlavePidChannel( NEXUS_PidChannelHandle master, NEXUS_PidChannelHandle slave )
{
    NEXUS_P_HwPidChannel_RemoveSlavePidChannel(master->hwPidChannel, slave->hwPidChannel);
}

static bool NEXUS_P_HwPidChannel_P_IsDataPresent(NEXUS_P_HwPidChannelHandle hwPidChannel)
{
    bool dataPresent = true;

#if BXPT_HAS_RSBUF || BXPT_HAS_FIXED_RSBUF_CONFIG
    BXPT_IsDataPresent(pTransport->xpt,
        hwPidChannel->status.pidChannelIndex,
        &dataPresent);
#else
    BSTD_UNUSED(hwPidChannel);
#endif

    return dataPresent;
}

bool NEXUS_PidChannel_P_IsDataPresent(NEXUS_PidChannelHandle pidChannel)
{
    return NEXUS_P_HwPidChannel_P_IsDataPresent(pidChannel->hwPidChannel);
}

void NEXUS_PidChannel_GetSettings(
    NEXUS_PidChannelHandle pidChannel,
    NEXUS_PidChannelSettings *pSettings
    )
{
    NEXUS_OBJECT_ASSERT( NEXUS_PidChannel, pidChannel );
    BDBG_ASSERT( pSettings );
    *pSettings = pidChannel->hwPidChannel->settings;
    pSettings->enabled = pidChannel->enabled;
}

NEXUS_Error NEXUS_PidChannel_SetSettings(
    NEXUS_PidChannelHandle pidChannel,
    const NEXUS_PidChannelSettings *pSettings
    )
{
    BERR_Code rc = NEXUS_SUCCESS;

    NEXUS_OBJECT_ASSERT( NEXUS_PidChannel, pidChannel );
    BDBG_ASSERT( pSettings );

    if( pSettings->pidChannelIndex != pidChannel->hwPidChannel->settings.pidChannelIndex )
    {
        BDBG_ERR(( "pidChannelIndex doesn't match" ));
        rc = BERR_TRACE( NEXUS_INVALID_PARAMETER );
        goto done;
    }
    if ((pidChannel->hwPidChannel->settings.pidChannelIndex==NEXUS_PID_CHANNEL_OPEN_INJECTION) && pSettings->enabled) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto done;
    }

#if B_REFSW_DSS_SUPPORT
    if( pSettings->dssHdFilter != pidChannel->hwPidChannel->settings.dssHdFilter )
    {
        rc = BXPT_DirecTv_ConfigHdFiltering(pTransport->xpt, pidChannel->hwPidChannel->status.pidChannelIndex,
            pSettings->dssHdFilter != NEXUS_PidChannelDssHdFilter_eDisabled,
            ((pSettings->dssHdFilter == NEXUS_PidChannelDssHdFilter_eAux) ? BXPT_HdFilterMode_eAuxOnly : BXPT_HdFilterMode_eNonAuxOnly));
        if (rc) {rc=BERR_TRACE(rc); goto done;}
    }
#endif

    if( pSettings->enabled != pidChannel->enabled )
    {
        pidChannel->enabled = pSettings->enabled;
        rc = NEXUS_P_HwPidChannel_CalcEnabled(pidChannel->hwPidChannel);
        if (rc) {rc=BERR_TRACE(rc); goto done;}
    }

    if( pSettings->remap.enabled != pidChannel->hwPidChannel->settings.remap.enabled
    || pSettings->remap.pid != pidChannel->hwPidChannel->settings.remap.pid
    || pSettings->remap.continuityCountEnabled != pidChannel->hwPidChannel->settings.remap.continuityCountEnabled
    )
    {
        rc = NEXUS_P_HwPidChannel_SetRemapSettings( pidChannel->hwPidChannel, &pSettings->remap );
        if (rc) {rc=BERR_TRACE(rc); goto done;}
    }

    if( pSettings->continuityCountEnabled != pidChannel->hwPidChannel->settings.continuityCountEnabled
    || pSettings->generateContinuityCount != pidChannel->hwPidChannel->settings.generateContinuityCount
    )
    {
        nexus_p_set_pid_cc(pidChannel->hwPidChannel, pSettings);

        pidChannel->hwPidChannel->settings.continuityCountEnabled = pSettings->continuityCountEnabled;
        pidChannel->hwPidChannel->settings.generateContinuityCount = pSettings->generateContinuityCount;
    }

    if( pSettings->pesFiltering.low != pidChannel->hwPidChannel->settings.pesFiltering.low
    || pSettings->pesFiltering.high != pidChannel->hwPidChannel->settings.pesFiltering.high
    )
    {
        /* Changing these fields requires determining the mapping of the PID channel to the RAVE contexts.
        That looks non-trivial, so postpone this until it's really needed.
        */
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    done:
    return rc;
}

/* Store bypassKeyslotSet in hwPidChannel, but allow security module to work with NEXUS_PidChannelHandle.
By doing ACQUIRE/RELEASE on NEXUS_PidChannelHandle, we keep the hwPidChannel open until cleanup.
Always do ACQUIRE/RELEASE on the NEXUS_PidChannelHandle that initially set it. */
NEXUS_PidChannelHandle NEXUS_PidChannel_GetBypassKeyslotCleanup_priv(void)
{
    NEXUS_P_HwPidChannel *hwPidChannel;
    NEXUS_ASSERT_MODULE();
    /* Consider a dedicated list for handles with pidChannel->bypassKeyslotSet. Wait for greater use. */
    for (hwPidChannel = BLST_S_FIRST(&pTransport->pidChannels); hwPidChannel; hwPidChannel = BLST_S_NEXT(hwPidChannel, link)) {
        NEXUS_PidChannelHandle pidChannel;
        if (!hwPidChannel->bypassKeyslotPidChannel) continue;
        for (pidChannel = BLST_S_FIRST(&hwPidChannel->swPidChannels); pidChannel; pidChannel = BLST_S_NEXT(pidChannel, link)) {
            if (pidChannel->open) break;
        }
        /* if none open, cleanup */
        if (!pidChannel) {
            return hwPidChannel->bypassKeyslotPidChannel;
        }
    }
    return NULL;
}

void NEXUS_PidChannel_SetBypassKeyslot_priv(NEXUS_PidChannelHandle pidChannel, bool set)
{
    NEXUS_ASSERT_MODULE();
    BDBG_MSG(("NEXUS_PidChannel_SetBypassKeyslot_priv(%p,%u) %p", (void *)pidChannel, set, (void *)pidChannel->hwPidChannel->bypassKeyslotPidChannel));
    if (pidChannel->hwPidChannel->bypassKeyslotPidChannel) {
        if (set) return;
        /* always to ACQUIRE/RELEASE on one sw pid channel */
        pidChannel = pidChannel->hwPidChannel->bypassKeyslotPidChannel;
        pidChannel->hwPidChannel->bypassKeyslotPidChannel = NULL;
    }
    else {
        if (!set) return;
        pidChannel->hwPidChannel->bypassKeyslotPidChannel = pidChannel;
    }
    if (set) {
        NEXUS_OBJECT_ACQUIRE(pidChannel, NEXUS_PidChannel, pidChannel);
    }
    else {
        NEXUS_OBJECT_RELEASE(pidChannel, NEXUS_PidChannel, pidChannel);
    }
}

unsigned NEXUS_PidChannel_GetIndex_isrsafe(NEXUS_PidChannelHandle pidChannel)
{
    /* immutable, therefore isrsafe */
    NEXUS_OBJECT_ASSERT(NEXUS_PidChannel, pidChannel);
    return pidChannel->hwPidChannel->status.pidChannelIndex;
}

NEXUS_Error NEXUS_PidChannel_ChangePid( NEXUS_PidChannelHandle pidChannel, unsigned pid )
{
    NEXUS_P_HwPidChannel *hwPidChannel;
    int rc;
    NEXUS_OBJECT_ASSERT(NEXUS_PidChannel, pidChannel);
    hwPidChannel = pidChannel->hwPidChannel;
    rc = BXPT_ConfigurePidChannel(pTransport->xpt, hwPidChannel->status.pidChannelIndex, pid, nexus_p_xpt_parser_band(hwPidChannel));
    if (rc) return BERR_TRACE(rc);
    hwPidChannel->status.pid = pid;

    if (hwPidChannel->parserBand && hwPidChannel->parserBand->settings.sourceType == NEXUS_ParserBandSourceType_eMtsif) {
        pTransport->mtsifPidChannelState[hwPidChannel->status.pidChannelIndex] = NEXUS_MtsifPidChannelState_eChanged;
        BKNI_SetEvent(pTransport->pidChannelEvent);
    }
    return NEXUS_SUCCESS;
}

unsigned nexus_p_xpt_parser_band(NEXUS_P_HwPidChannel *hwPidChannel)
{
    return hwPidChannel->playpump?BXPT_PB_PARSER(hwPidChannel->playpump->index):hwPidChannel->parserBand->hwIndex;
}
