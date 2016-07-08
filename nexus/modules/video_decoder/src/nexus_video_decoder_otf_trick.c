/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
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
#include "priv/nexus_stc_channel_priv.h"
#include "priv/nexus_pid_channel_priv.h"

BDBG_MODULE(nexus_video_decoder_otf_trick);

void
NEXUS_VideoDecoder_P_OtfPvr_DisableForFlush(NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_ASSERT(videoDecoder->otfPvr.otf);
    BOTF_DisableForFlush(videoDecoder->otfPvr.otf);
    return;
}

void
NEXUS_VideoDecoder_P_OtfPvr_Flush(NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_ASSERT(videoDecoder->otfPvr.otf);
    BOTF_Flush(videoDecoder->otfPvr.otf);
    return;
}

static void
NEXUS_VideoDecoder_P_OtfStep(void *context)
{
    NEXUS_VideoDecoderHandle videoDecoder = context;
    BOTF_StepStatus step;
    BOTF_Status newStatus;
    BERR_Code rc=BERR_SUCCESS;
    BXVD_ChannelStatus channelStatus;
    BOTF_VideoDecoderStatus  videoDecoderStatus;
    BXVD_PTSInfo ptsInfo;

    videoDecoder->otfPvr.timer = NULL;
    BKNI_EnterCriticalSection();
    rc = BXVD_GetPTS_isr(videoDecoder->dec, &ptsInfo);
    BKNI_LeaveCriticalSection();
    if(rc==BERR_SUCCESS) {
        rc = BXVD_GetChannelStatus(videoDecoder->dec, &channelStatus);
    }
    if(rc==BERR_SUCCESS) {
        videoDecoderStatus.pts = ptsInfo.ui32RunningPTS;
        videoDecoderStatus.ptsValid = ptsInfo.ePTSType == BXDM_PictureProvider_PTSType_eCoded || ptsInfo.ePTSType == BXDM_PictureProvider_PTSType_eInterpolatedFromValidPTS;
        videoDecoderStatus.pictureDepth = channelStatus.ulPictureDeliveryCount;
    } else {
        videoDecoderStatus.pts = 0;
        videoDecoderStatus.ptsValid = false;
        videoDecoderStatus.pictureDepth = 0;
    }

    BOTF_PlayStep(videoDecoder->otfPvr.otf, &videoDecoderStatus, BOTF_TASK_SLEEPMS, &step);

    BOTF_GetStatus(videoDecoder->otfPvr.otf, &newStatus);
    if(BKNI_Memcmp(&newStatus, &videoDecoder->otfPvr.status, sizeof(newStatus))!=0) {
        BDBG_MSG(("CDB:%u ITB:%u(%u) OUT:%u", newStatus.inputCdb, newStatus.inputItb, newStatus.unconsumedInputItb, newStatus.outputItb));
        videoDecoder->otfPvr.status = newStatus;
    }
    if(step.stop || step.flush /* || step.eos */) {
        BDBG_WRN(("%s %s %s", step.stop?"STOP":"", step.flush?"FLUSH (too small CDB size)":"", step.eos?"EOS":""));
    }
    if(step.flush) {
        BDBG_ASSERT(videoDecoder->trickState.reorderingMode!=NEXUS_VideoDecoderReorderingMode_eNone);
        NEXUS_VideoDecoder_Flush(videoDecoder);
    }
    videoDecoder->otfPvr.eos = step.eos;
    if(!step.stop) {
        videoDecoder->otfPvr.timer = NEXUS_ScheduleTimer(BOTF_TASK_SLEEPMS, NEXUS_VideoDecoder_P_OtfStep, videoDecoder);
    } else {
        videoDecoder->otfPvr.timer = NULL;
    }
    return;
}

static NEXUS_Error
NEXUS_VideoDecoder_P_OtfPvr_MapSettings(const NEXUS_VideoDecoderTrickState *pState, BOTF_Config  *config)
{
    BKNI_Memset(config, 0, sizeof(*config));
    switch(pState->reorderingMode) {
    default:
    /* case NEXUS_VideoDecoderReorderingMode_eNone:  */
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* we should be in the OTF mode */
    case NEXUS_VideoDecoderReorderingMode_eSequential:
        config->FeederMode = BOTF_FeederMode_eSequential;
        break;
    case NEXUS_VideoDecoderReorderingMode_eGop:
        config->FeederMode = BOTF_FeederMode_eGop;
        break;
    case NEXUS_VideoDecoderReorderingMode_eInterleaved:
        config->FeederMode = BOTF_FeederMode_eInterleaved;
        break;
    case NEXUS_VideoDecoderReorderingMode_eChunkForward:
        config->FeederMode = BOTF_FeederMode_eChunkForward;
        break;
    case NEXUS_VideoDecoderReorderingMode_eChunkBackward:
        config->FeederMode = BOTF_FeederMode_eChunkBackward;
        break;
    }
    switch(pState->decodeMode) {
    case NEXUS_VideoDecoderDecodeMode_eAll:
        config->FrameFilter = BOTF_FrameFilter_eIPBFrames;
        break;
    case NEXUS_VideoDecoderDecodeMode_eIP:
        config->FrameFilter = BOTF_FrameFilter_eIPFrames;
        break;
    case NEXUS_VideoDecoderDecodeMode_eI:
        config->FrameFilter = BOTF_FrameFilter_eIFrames;
        break;
    default: return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    config->IsRewind = pState->reverseFields;
    BDBG_CASSERT(NEXUS_NORMAL_DECODE_RATE>=BOTF_PLAY_RATE_MODULO);
    if(pState->rate >= NEXUS_NORMAL_DECODE_RATE) {
        config->FrameRate = pState->rate / (NEXUS_NORMAL_DECODE_RATE/BOTF_PLAY_RATE_MODULO);
    } else {
        config->FrameRate = BOTF_PLAY_RATE_MODULO;
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_VideoDecoder_P_OtfPvr_Activate(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_RaveStatus *raveStatus, BXVD_DecodeSettings *xvdCfg)
{
    BOTF_Params Params;
    BERR_Code rc = BERR_SUCCESS;

    BOTF_GetDefaultParams(&Params);

    Params.hBReg = g_pCoreHandles->reg;
    Params.InputParserCDBSize = videoDecoder->cdbLength;
    Params.InputParserITBSize = videoDecoder->itbLength;
    Params.OutputParserITBSize = 256*1024;
    Params.OutputParserITBAlign = 8;
    Params.inputContext = raveStatus->xptContextMap;
    {
        NEXUS_RaveOpenSettings rave2_cfg;
        NEXUS_RaveStatus raveStatus2;
        LOCK_TRANSPORT();
        NEXUS_Rave_GetDefaultOpenSettings_priv(&rave2_cfg);
        rave2_cfg.config.Cdb.Length = 8192; /* XPT now requires a minimum CDB size */
        rave2_cfg.config.Itb.Length = 0;
        videoDecoder->otfPvr.rave = NEXUS_Rave_Open_priv(&rave2_cfg);
        if(videoDecoder->otfPvr.rave) {
            rc = NEXUS_Rave_GetStatus_priv(videoDecoder->otfPvr.rave, &raveStatus2);
        }
        UNLOCK_TRANSPORT();
        if (rc) {return BERR_TRACE(rc);}
        if(!videoDecoder->otfPvr.rave) {return BERR_TRACE(NEXUS_NOT_SUPPORTED);}
        Params.outputContext = raveStatus2.xptContextMap;
        Params.hBMem = NEXUS_Heap_GetMemHandle(raveStatus2.heap); /* OTF PVR needs heap for RAVE */ 
        if (!NEXUS_P_CpuAccessibleHeap(raveStatus2.heap)) {rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_otf;} /* and it must be CPU-accessible. */
        videoDecoder->otfPvr.xptContextMap = raveStatus2.xptContextMap;
        xvdCfg->pContextMap = &videoDecoder->otfPvr.xptContextMap;
    }
    Params.bUsePtsAsTag = false;
    rc = BOTF_Open(&Params, &videoDecoder->otfPvr.otf);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto err_otf;}


    return NEXUS_SUCCESS;

err_otf:
    if (videoDecoder->otfPvr.rave) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Close_priv(videoDecoder->otfPvr.rave);
        UNLOCK_TRANSPORT();
        videoDecoder->otfPvr.rave = NULL;
    }
    return rc;
}

static NEXUS_Error
NEXUS_VideoDecoder_P_OtfPvr_SetDecoderSettings(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderTrickState *pState)
{
    BERR_Code rc;

    BXVD_ChannelHandle dec;
    dec = videoDecoder->dec;

    rc = BXVD_SetVideoDisplayMode(videoDecoder->dec, BXVD_DisplayMode_eVSYNCMode);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    rc = BXVD_PVR_SetSlowMotionRate(dec, pState->rate?NEXUS_NORMAL_DECODE_RATE/pState->rate:1);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    rc = BXVD_PVR_EnablePause(dec, pState->rate==0);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    rc = BXVD_SetDisplayFieldMode(dec, pState->topFieldOnly?BXVD_DisplayFieldType_eTopFieldOnly : BXVD_DisplayFieldType_eBothField);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    rc = BXVD_PVR_EnableReverseFields(dec, pState->reverseFields);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_settings;}

    return NEXUS_SUCCESS;

err_settings:
    return rc;
}

static NEXUS_Error
NEXUS_VideoDecoder_P_OtfPvr_Start(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderTrickState *pState)
{
    NEXUS_Error rc;
    BOTF_Config  config;

    BDBG_ASSERT(videoDecoder->otfPvr.otf);

    BDBG_MSG(("NEXUS_VideoDecoder_P_OtfPvr_Start: %p ", (void *)videoDecoder));
    rc = NEXUS_VideoDecoder_P_OtfPvr_MapSettings(pState, &config);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto err_settings;}
    rc = BOTF_ConfigSet(videoDecoder->otfPvr.otf, &config);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto err_settings;}

    BDBG_ASSERT(videoDecoder->otfPvr.otf);
    rc = NEXUS_VideoDecoder_P_OtfPvr_SetDecoderSettings(videoDecoder, pState);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    rc = BOTF_Start(videoDecoder->otfPvr.otf);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc); }
    videoDecoder->otfPvr.eos = false;

    BOTF_GetStatus(videoDecoder->otfPvr.otf, &videoDecoder->otfPvr.status);

    return NEXUS_SUCCESS;

err_settings:
    return rc;
}

void
NEXUS_VideoDecoder_P_OtfPvr_Stop(NEXUS_VideoDecoderHandle videoDecoder)
{
    BOTF_Stop(videoDecoder->otfPvr.otf);

    if(videoDecoder->otfPvr.timer) {
        NEXUS_CancelTimer(videoDecoder->otfPvr.timer);
        videoDecoder->otfPvr.timer = NULL;
    }
    BOTF_Close(videoDecoder->otfPvr.otf);
    videoDecoder->otfPvr.otf = NULL;

    LOCK_TRANSPORT();
    NEXUS_Rave_Close_priv(videoDecoder->otfPvr.rave);
    UNLOCK_TRANSPORT();
    videoDecoder->otfPvr.rave = false;

    videoDecoder->trickState.reorderingMode=NEXUS_VideoDecoderReorderingMode_eNone;
    return;
}

NEXUS_Error
NEXUS_VideoDecoder_P_OtfPvr_SetTrickState(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderTrickState *pState, bool *complete)
{
    NEXUS_Error rc;
    *complete = true;
    if(pState->reorderingMode!=NEXUS_VideoDecoderReorderingMode_eNone) {
        if(videoDecoder->startSettings.codec != NEXUS_VideoCodec_eMpeg2) {return BERR_TRACE(NEXUS_NOT_SUPPORTED);}
        if(pState->brcmTrickModesEnabled) {return BERR_TRACE(NEXUS_NOT_SUPPORTED);}
        if(pState->dqtEnabled) {return BERR_TRACE(NEXUS_NOT_SUPPORTED);}
        if(pState->tsmEnabled) {return BERR_TRACE(NEXUS_NOT_SUPPORTED);}
        if(pState->stcTrickEnabled) {return BERR_TRACE(NEXUS_NOT_SUPPORTED);}

        videoDecoder->otfPvr.wasActive = true;
        if(!videoDecoder->otfPvr.active)  {
            /* enable OTF PVR */
            rc = NEXUS_VideoDecoder_P_SetChannelChangeMode(videoDecoder, NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture);
            if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
            NEXUS_VideoDecoder_P_Stop_priv(videoDecoder);
            BDBG_MSG(("%s: %p enabling OTF PVR", "NEXUS_VideoDecoder_OtfPvr_SetSettings", (void *)videoDecoder));
            rc = NEXUS_VideoDecoder_P_Start_priv(videoDecoder, &videoDecoder->startSettings, true);
            if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

            rc = NEXUS_VideoDecoder_P_OtfPvr_Start(videoDecoder, pState);
            if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

            videoDecoder->trickState = *pState;
            videoDecoder->otfPvr.active = true;
            NEXUS_VideoDecoder_P_OtfStep(videoDecoder); /* run OTF PVR step it would also kick start timer */
            return NEXUS_SUCCESS;
        } else {
            if(pState->reorderingMode != videoDecoder->trickState.reorderingMode) { /* need to restart OTF PVR */
                BDBG_MSG(("%s: %p restarting OtfPvr", "NEXUS_VideoDecoder_OtfPvr_SetSettings", (void *)videoDecoder));
                NEXUS_VideoDecoder_Flush(videoDecoder);
                BOTF_Stop(videoDecoder->otfPvr.otf);
                rc = NEXUS_VideoDecoder_P_OtfPvr_Start(videoDecoder, pState);
                if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
            } else {
                BOTF_Config  config;
                rc = NEXUS_VideoDecoder_P_OtfPvr_MapSettings(pState, &config);
                if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto err_settings;}
                rc = BOTF_ConfigSet(videoDecoder->otfPvr.otf, &config);
                if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto err_settings;}
            }
            rc = NEXUS_VideoDecoder_P_OtfPvr_SetDecoderSettings(videoDecoder, pState);
            if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
            videoDecoder->trickState = *pState;
            return NEXUS_SUCCESS;
        }
    } else if(videoDecoder->otfPvr.active) {
        BDBG_MSG(("%s: %p disabling OTF PVR", "NEXUS_VideoDecoder_OtfPvr_SetSettings", (void *)videoDecoder));

        videoDecoder->otfPvr.active = false;
        rc = NEXUS_VideoDecoder_P_SetChannelChangeMode(videoDecoder, NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}

        NEXUS_VideoDecoder_P_Stop_priv(videoDecoder);

        NEXUS_VideoDecoder_P_OtfPvr_Stop(videoDecoder);

        rc  = NEXUS_VideoDecoder_P_Start_priv(videoDecoder, &videoDecoder->startSettings, false);
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
        /* and proceed further */
    }
    *complete = false;
    return NEXUS_SUCCESS;

err_settings:
    return rc;
}


void
NEXUS_VideoDecoder_P_OtfPvr_UpdateStatus(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderStatus *pStatus)
{
    if(videoDecoder->otfPvr.eos) {
        /* clear FIFO depth on EOF */
        pStatus->fifoDepth = 0;
    }
    return;
}
