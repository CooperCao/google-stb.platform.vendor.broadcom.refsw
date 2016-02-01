/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
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

#include "nexus_video_decoder_module.h"
#include "bxvd_pvr.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_video_decoder_trick);

void
NEXUS_VideoDecoder_P_GetTrickState_Common(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderTrickState *pState)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_ASSERT(pState);
    *pState = videoDecoder->trickState;
    return ;
}

NEXUS_Error
NEXUS_VideoDecoder_P_SetTrickState_Avd(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderTrickState *pState)
{
    NEXUS_Error rc;
    BXVD_ChannelHandle dec;
    BXVD_PlaybackRateSettings rate;
    BXVD_FrameRateSettings frameRateOverrideSettings;
    bool sparseMode;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_ASSERT(pState);

    if (!videoDecoder->dec) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    dec = videoDecoder->dec;
#if NEXUS_OTFPVR
    if(videoDecoder->startSettings.codec == NEXUS_VideoCodec_eMpeg2) {
        bool complete;
        rc = NEXUS_VideoDecoder_P_OtfPvr_SetTrickState(videoDecoder, pState, &complete);
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
        if(complete) { return NEXUS_SUCCESS; }
        /* keep going */
    }
#endif /*  NEXUS_OTFPVR */
    if(
        (pState->reorderingMode==NEXUS_VideoDecoderReorderingMode_eChunkForward ||
         pState->reorderingMode==NEXUS_VideoDecoderReorderingMode_eChunkBackward) &&
         pState->decodeMode == NEXUS_VideoDecoderDecodeMode_eI) {
        videoDecoder->errorHandlingOverride = true;
        videoDecoder->errorHandlingMode = NEXUS_VideoDecoderErrorHandling_ePicture;
    } else if(pState->reorderingMode!=NEXUS_VideoDecoderReorderingMode_eNone) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    } else {
        videoDecoder->errorHandlingOverride = false;
    }

    (void)BXVD_GetPlaybackRate(dec, &rate);
    if (pState->rate && pState->tsmEnabled != NEXUS_TsmMode_eSimulated) {
        rate.uiNumerator = pState->rate;
    }
    else {
        rate.uiNumerator = NEXUS_NORMAL_DECODE_RATE;
    }
    rate.uiDenominator = NEXUS_NORMAL_DECODE_RATE;
    rc = BXVD_SetPlaybackRate(dec, rate /* yes, pass by value */);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    (void)BXVD_GetFrameRateOverride(dec, &frameRateOverrideSettings);
    frameRateOverrideSettings.bValid = pState->forceSourceFrameRate != 0;
    frameRateOverrideSettings.bTreatAsSingleElement = frameRateOverrideSettings.bValid;
    if (frameRateOverrideSettings.bValid) {
        frameRateOverrideSettings.stRate.uiNumerator = pState->forceSourceFrameRate;
        frameRateOverrideSettings.stRate.uiDenominator = 1000;
    }
    rc = BXVD_SetFrameRateOverride(dec, &frameRateOverrideSettings);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    sparseMode = (pState->brcmTrickModesEnabled || pState->hostTrickModesEnabled) && (videoDecoder->startSettings.codec == NEXUS_VideoCodec_eH264 || videoDecoder->startSettings.codec == NEXUS_VideoCodec_eH265);
    BDBG_MSG(("SparseMode %d",(unsigned)sparseMode));
    rc = BXVD_PVR_SetHostSparseMode(dec, sparseMode);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}
    rc = BXVD_PVR_EnableReverseFields(dec, pState->reverseFields);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}
    rc = BXVD_PVR_SetGopTrickMode(dec, pState->dqtEnabled);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    /* BTP's are always processed by RAVE and XVD. No setting for this. */

    rc = BXVD_SetDisplayFieldMode(dec, pState->topFieldOnly?BXVD_DisplayFieldType_eTopFieldOnly : BXVD_DisplayFieldType_eBothField);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    if (pState->decodeMode != videoDecoder->trickState.decodeMode) {
        /* A flush is recommended on skip mode transitions, but this must be done by higher level SW so that it can avoid a trick mode transition
        delay due to a full display queue. */
        rc = BXVD_SetSkipPictureModeDecode(dec,
            pState->decodeMode==NEXUS_VideoDecoderDecodeMode_eI?BXVD_SkipMode_eDecode_I_Only:
            (pState->decodeMode==NEXUS_VideoDecoderDecodeMode_eIP?BXVD_SkipMode_eDecode_Ref_Only: /* Ref_Only is preferred for IP trick modes, not _eIP_Only */
            BXVD_SkipMode_eDecode_IPB));
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}
    }

    rc = BXVD_PVR_EnablePause(dec, pState->rate==0);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    if (!pState->maxFrameRepeat) {
        videoDecoder->maxFrameRepeat.pictureDisplayCount = 0;
    }

    BKNI_EnterCriticalSection();
    videoDecoder->trickState = *pState;
    BKNI_LeaveCriticalSection();

    /* the following functions require coordination with other state variables in VideoDecoder */
    rc = NEXUS_VideoDecoder_P_SetDiscardThreshold_Avd(videoDecoder);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    rc = NEXUS_VideoDecoder_P_SetTsm(videoDecoder);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    rc = NEXUS_VideoDecoder_P_SetMute(videoDecoder);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    rc = BXVD_SetErrorHandlingMode(dec, videoDecoder->errorHandlingOverride?videoDecoder->errorHandlingMode:videoDecoder->startSettings.errorHandling);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    /* disable userdata on host trick modes */
    NEXUS_VideoDecoder_P_SetUserdata(videoDecoder);

    return NEXUS_SUCCESS;
err_settings:
    return rc;
}

NEXUS_Error
NEXUS_VideoDecoder_P_FrameAdvance_Avd(NEXUS_VideoDecoderHandle videoDecoder)
{
    NEXUS_Error  rc;
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    if (!videoDecoder->dec) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* for interlaced content not in topFieldOnly mode, BXVD_PVR_FrameAdvanceMode_eFrame_by_Frame will advance two fields. */
    rc = BXVD_PVR_FrameAdvance(videoDecoder->dec,
        videoDecoder->trickState.fieldByFieldAdvance ? BXVD_PVR_FrameAdvanceMode_eField_by_Field : BXVD_PVR_FrameAdvanceMode_eFrame_by_Frame);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_VideoDecoder_P_GetNextPts_Avd(NEXUS_VideoDecoderHandle videoDecoder, uint32_t *pNextPts)
{
    BERR_Code rc;
    BXVD_PTSInfo nextPtsInfo;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    if (!videoDecoder->dec) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    rc = BXVD_GetNextPTS_isr(videoDecoder->dec, &nextPtsInfo);

    if ((nextPtsInfo.ePTSType==(BXDM_PictureProvider_PTSType)BXVD_PTSType_eInterpolatedFromInvalidPTS) || (rc!=BERR_SUCCESS)) {
        BDBG_MSG(("NEXUS_VideoDecoder_GetNextPts: Next PTS is invalid"));
        *pNextPts = 0; /* If the next PTS is invalid, we return 0 */
        return NEXUS_UNKNOWN;
    }
    else {
        *pNextPts = nextPtsInfo.ui32EffectivePTS;
        BDBG_MSG(("NEXUS_VideoDecoder_GetNextPts: Next PTS=%08X", *pNextPts));
        return NEXUS_SUCCESS;
    }
}

void
NEXUS_VideoDecoder_GetNormalPlay(NEXUS_VideoDecoderTrickState *pState)
{
    BKNI_Memset(pState, 0, sizeof(*pState));
    pState->rate = NEXUS_NORMAL_DECODE_RATE;
    pState->decodeMode = NEXUS_VideoDecoderDecodeMode_eAll;
    pState->tsmEnabled = NEXUS_TsmMode_eEnabled;
    pState->reorderingMode = NEXUS_VideoDecoderReorderingMode_eNone;
    pState->lowLatencyDisplayForTrickModes = true;
}

void
NEXUS_VideoDecoder_P_Trick_Reset_Generic(NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_VideoDecoder_GetNormalPlay(&videoDecoder->trickState);
    return;
}

NEXUS_Error NEXUS_VideoDecoder_InvalidateSimulatedStc( NEXUS_VideoDecoderHandle videoDecoder )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (videoDecoder->trickState.tsmEnabled != NEXUS_TsmMode_eSimulated) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    return BXVD_SetSTCInvalidFlag(videoDecoder->dec, true); /* true means it is invalid */
}

NEXUS_Error NEXUS_VideoDecoder_P_SetSimulatedStc_isr(NEXUS_VideoDecoderHandle videoDecoder, uint32_t pts)
{
    BXVD_ClockOverride clockOverride;
    NEXUS_Error rc;

    BDBG_ASSERT(videoDecoder->trickState.tsmEnabled == NEXUS_TsmMode_eSimulated);

    (void)BXVD_GetClockOverride_isr(videoDecoder->dec, &clockOverride);
    clockOverride.uiStcValue = pts;
    clockOverride.bLoadSwStc = true;
    rc = BXVD_SetClockOverride_isr(videoDecoder->dec, &clockOverride);
    if (rc) return BERR_TRACE(rc);
    BDBG_MSG(("set simulated stc %08x (%d %d)", pts, clockOverride.bEnableClockOverride, clockOverride.iStcDelta));

    (void)BXVD_SetSTCInvalidFlag_isr(videoDecoder->dec, false); /* false means it is valid */

    return 0;
}

NEXUS_Error NEXUS_VideoDecoder_P_SetUnderflowMode_isr(NEXUS_VideoDecoderHandle videoDecoder, bool underflowMode)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_WRN(("Set video underflow mode %s", underflowMode ? "on" : "off"));
    rc = BXVD_PVR_SetIgnoreNRTUnderflow_isr(videoDecoder->dec, underflowMode);

    return rc;
}

