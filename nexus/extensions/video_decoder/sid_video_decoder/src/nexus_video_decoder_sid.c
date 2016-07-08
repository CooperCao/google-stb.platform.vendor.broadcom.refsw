/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  .  Except as set forth in an Authorized License, Broadcom grants
 *  no license , right to use, or waiver of any kind with respect to the
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
 *  LICENSORS BE LIABLE FOR  CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR  ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/
#include "nexus_video_decoder_module.h"
#include "nexus_still_decoder_impl.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_video_decoder_sid);

BDBG_OBJECT_ID(NEXUS_VideoDecoder_Sid);

#define BDBG_MSG_TRACE(x)  BDBG_MSG(x)

static void NEXUS_VideoDecoder_P_Flush_Sid( NEXUS_VideoDecoderHandle decoder);

static void NEXUS_VideoDecoder_P_Close_Sid( NEXUS_VideoDecoderHandle decoder)
{
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Close_Sid: %p", (void*)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.sid, NEXUS_VideoDecoder_Sid);
    NEXUS_VideoDecoder_P_Close_Generic(decoder);
    NEXUS_VideoDecoder_P_Xdm_Shutdown(decoder);
    LOCK_PICTURE_DECODER();
    NEXUS_SidVideoDecoder_Close_priv(decoder->decoder.sid.decoder);
    UNLOCK_PICTURE_DECODER();
    BDBG_OBJECT_DESTROY(decoder, NEXUS_VideoDecoder);
    BKNI_Free(decoder);
    return;
}

static NEXUS_Error NEXUS_VideoDecoder_P_SetSettings_Sid( NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoderSettings *pSettings)
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.sid, NEXUS_VideoDecoder_Sid);

    errCode = NEXUS_VideoDecoder_P_Xdm_ApplySettings(decoder, pSettings, false);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Store settings */
    decoder->settings = *pSettings;

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_VideoDecoder_P_Start_Sid( NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoderStartSettings *pSettings)
{
    NEXUS_Error  rc;
    NEXUS_StcChannelDecoderConnectionSettings stcChannelConnectionSettings;
    BAVC_VideoCompressionStd videoCmprStd;
    NEXUS_RaveStatus raveStatus;
    bool playback = false;
    unsigned stcChannelIndex;
    NEXUS_SidVideoDecoderStartSettings sidStartSettings;

    BDBG_MSG(("NEXUS_VideoDecoder_P_Start_Sid:%p", (void*)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.sid, NEXUS_VideoDecoder_Sid);

    if(pSettings->codec != NEXUS_VideoCodec_eMotionJpeg) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    rc = NEXUS_VideoDecoder_P_Start_Generic_Prologue(decoder, pSettings);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    LOCK_TRANSPORT();
    NEXUS_StcChannel_GetDefaultDecoderConnectionSettings_priv(&stcChannelConnectionSettings);
    stcChannelConnectionSettings.type = NEXUS_StcChannelDecoderType_eVideo;
    stcChannelConnectionSettings.priority = decoder->stc.priority;
    stcChannelConnectionSettings.getPts_isr = NEXUS_VideoDecoder_P_GetPtsCallback_Xdm_isr;
    stcChannelConnectionSettings.getCdbLevel_isr = NEXUS_VideoDecoder_P_GetCdbLevelCallback_isr;
    stcChannelConnectionSettings.stcValid_isr = NEXUS_VideoDecoder_P_StcValidCallback_Xdm_isr;
    if (decoder->mosaicMode || decoder->startSettings.nonRealTime) {
        stcChannelConnectionSettings.setPcrOffset_isr = NEXUS_VideoDecoder_P_SetPcrOffset_Xdm_isr;
        stcChannelConnectionSettings.getPcrOffset_isr = NEXUS_VideoDecoder_P_GetPcrOffset_Xdm_isr;
    }
    stcChannelConnectionSettings.pCallbackContext = decoder;
    UNLOCK_TRANSPORT();

    rc = NEXUS_VideoDecoder_P_Start_Generic_Body(decoder, pSettings, false, &videoCmprStd, &raveStatus, &stcChannelConnectionSettings, &playback, &stcChannelIndex);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    rc = NEXUS_VideoDecoder_P_Xdm_Start(decoder);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    LOCK_PICTURE_DECODER();
    NEXUS_SidVideoDecoder_GetDefaultStartSettings_priv(&sidStartSettings);
    sidStartSettings.raveContext = decoder->rave;
    sidStartSettings.codec = pSettings->codec;
    rc = NEXUS_SidVideoDecoder_Start_priv(decoder->decoder.sid.decoder, &sidStartSettings);
    UNLOCK_PICTURE_DECODER();
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    rc = NEXUS_VideoDecoder_P_Start_Priv_Generic_Epilogue(decoder, pSettings);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    rc = NEXUS_VideoDecoder_P_Start_Generic_Epilogue(decoder, pSettings);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

static void NEXUS_VideoDecoder_P_Stop_Sid( NEXUS_VideoDecoderHandle decoder)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.sid, NEXUS_VideoDecoder_Sid);
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Stop_Sid: %p", (void*)decoder));

    NEXUS_VideoDecoder_P_Flush_Sid(decoder);

    rc = NEXUS_VideoDecoder_P_Stop_Generic_Prologue(decoder);
    if(rc!=NEXUS_SUCCESS) {return;}


    NEXUS_VideoDecoder_P_Stop_Priv_Generic_Prologue(decoder);

    LOCK_PICTURE_DECODER();
    NEXUS_SidVideoDecoder_Stop_priv(decoder->decoder.sid.decoder);
    UNLOCK_PICTURE_DECODER();

    NEXUS_VideoDecoder_P_Xdm_Stop(decoder);

    NEXUS_VideoDecoder_P_Stop_Priv_Generic_Epilogue(decoder);
    NEXUS_VideoDecoder_P_Stop_Generic_Epilogue(decoder);
    return;
}

static void NEXUS_VideoDecoder_P_Flush_Sid( NEXUS_VideoDecoderHandle decoder)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.sid, NEXUS_VideoDecoder_Sid);

    NEXUS_VideoDecoder_P_Xdm_Stop(decoder);
    LOCK_PICTURE_DECODER();
    NEXUS_SidVideoDecoder_Flush_priv(decoder->decoder.sid.decoder);
    UNLOCK_PICTURE_DECODER();
    rc = NEXUS_VideoDecoder_P_Xdm_Start(decoder);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}

    return;
}

static NEXUS_Error NEXUS_VideoDecoder_P_GetStatus_Sid( NEXUS_VideoDecoderHandle decoder, NEXUS_VideoDecoderStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.sid, NEXUS_VideoDecoder_Sid);
    NEXUS_VideoDecoder_P_GetStatus_Generic(decoder, pStatus);
    return NEXUS_VideoDecoder_P_GetStatus_Generic_Xdm(decoder, pStatus);
}

NEXUS_VideoDecoderHandle
NEXUS_VideoDecoder_P_Open_Sid( unsigned index, const NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
    NEXUS_VideoDecoderHandle decoder;
    NEXUS_VideoDecoderOpenMosaicSettings mosaicSettings;
    NEXUS_RaveOpenSettings raveSettings;
    NEXUS_SidVideoDecoderOpenSettings  sidOpenSettings;
    const BXDM_Decoder_Interface *decoderInterface;
    void *decoderContext;
    NEXUS_Error rc;

    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Open_Sid: %u", index));
    if(index>=NEXUS_NUM_SID_VIDEO_DECODERS) { rc=BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_args;}
    decoder = BKNI_Malloc(sizeof(*decoder));
    if(!decoder) { rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}

    BKNI_Memset(decoder, 0, sizeof(*decoder));
    BDBG_OBJECT_SET(&decoder->decoder.sid, NEXUS_VideoDecoder_Sid);
    decoder->intf = &NEXUS_VideoDecoder_P_Interface_Sid;
    NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&mosaicSettings);
    if (pOpenSettings) {
        mosaicSettings.openSettings = *pOpenSettings;
    }

    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(&raveSettings);
    raveSettings.supportedCodecs[NEXUS_VideoCodec_eMotionJpeg] = true;
    UNLOCK_TRANSPORT();

    LOCK_PICTURE_DECODER();
    NEXUS_SidVideoDecoder_GetRaveSettings_priv(&raveSettings);
    UNLOCK_PICTURE_DECODER();

    rc = NEXUS_VideoDecoder_P_Init_Generic(decoder, &raveSettings, &mosaicSettings);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    rc = NEXUS_VideoDecoder_P_getUnusedMfd(false, &decoder->decoder.sid.sourceId, &decoder->decoder.sid.heapIndex);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    LOCK_PICTURE_DECODER();
    NEXUS_SidVideoDecoder_GetDefaultOpenSettings_priv(&sidOpenSettings);
    sidOpenSettings.pictureHeap = g_pCoreHandles->heap[decoder->decoder.sid.heapIndex].nexus;
    decoder->decoder.sid.decoder = NEXUS_SidVideoDecoder_Open_priv(index, &sidOpenSettings);
    UNLOCK_PICTURE_DECODER();
    if(!decoder->decoder.sid.decoder) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;}

    LOCK_PICTURE_DECODER();
    NEXUS_SidVideoDecoder_GetDecoderInterface_priv(decoder->decoder.sid.decoder, &decoderInterface, &decoderContext);
    UNLOCK_PICTURE_DECODER();

    rc = NEXUS_VideoDecoder_P_Xdm_Initialize(decoder, decoderInterface, decoderContext);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    return decoder;

error:
err_alloc:
err_args:
    return NULL;
}

NEXUS_Error NEXUS_VideoDecoderModule_P_Init_Sid(const NEXUS_VideoDecoderModuleInternalSettings *pSettings)
{
    if(pSettings->pictureDecoder==NULL) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return BERR_SUCCESS;
}

void NEXUS_VideoDecoderModule_P_Uninit_Sid(void)
{
    return;
}

static void NEXUS_VideoDecoder_GetDisplayConnection_priv_Sid( NEXUS_VideoDecoderHandle decoder, NEXUS_VideoDecoderDisplayConnection *pConnection)
{
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_GetDisplayConnection_priv_Sid: %p", (void*)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.sid, NEXUS_VideoDecoder_Sid);
    NEXUS_ASSERT_MODULE();
    *pConnection = decoder->displayConnection;
    return;
}


static void NEXUS_VideoDecoder_GetSourceId_priv_Sid( NEXUS_VideoDecoderHandle decoder, BAVC_SourceId *pSource)
{
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_GetSourceId_priv_Sid: %p", (void*)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.sid, NEXUS_VideoDecoder_Sid);
    NEXUS_ASSERT_MODULE();
    *pSource = decoder->decoder.sid.sourceId;
    return;
}

static NEXUS_Error NEXUS_VideoDecoder_P_SetTrickState_Sid( NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoderTrickState *pState)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.sid, NEXUS_VideoDecoder_Sid);

    decoder->trickState = *pState;
    return BERR_SUCCESS;
}

static void NEXUS_VideoDecoder_P_IsCodecSupported_Sid( NEXUS_VideoDecoderHandle decoder, NEXUS_VideoCodec codec, bool *pSupported )
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.sid, NEXUS_VideoDecoder_Sid);
    BDBG_ASSERT(pSupported);
    BSTD_UNUSED(decoder);

    *pSupported = (codec == NEXUS_VideoCodec_eMotionJpeg);
    return;
}

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

static NEXUS_Error NEXUS_VideoDecoder_P_SetPowerState_Sid(NEXUS_VideoDecoderHandle videoDecoder, bool powerUp)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&videoDecoder->decoder.sid, NEXUS_VideoDecoder_Sid);

    if (powerUp && !videoDecoder->decoder.sid.poweredUp)
    {
#if defined BCHP_PWR_RESOURCE_AVD0_PWR
        BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD0_PWR);
#endif
#if defined BCHP_PWR_RESOURCE_AVD0_CLK
        BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD0_CLK);
#endif
        videoDecoder->decoder.sid.poweredUp = true;
    }
    else if ( !powerUp && videoDecoder->decoder.sid.poweredUp )
    {
        /* if started, we can automatically stop. */
        if (videoDecoder->started) {
            NEXUS_VideoDecoder_Stop(videoDecoder);
        }

        /* if we're connected, we must fail. destroying a connected XVD channel will cause VDC window timeouts.
        the VDC window must be destroyed using NEXUS_VideoWindow_RemoveInput. */
        if (videoDecoder->displayConnection.dataReadyCallback_isr) {
            BDBG_ERR(("You must call NEXUS_VideoWindow_RemoveInput before NEXUS_VideoDecoder_SetPowerState(false)."));
            return BERR_TRACE(NEXUS_UNKNOWN);
        }

#if defined BCHP_PWR_RESOURCE_AVD0_CLK
        BCHP_PWR_ReleaseResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD0_CLK);
#endif
#if defined BCHP_PWR_RESOURCE_AVD0_PWR
        BCHP_PWR_ReleaseResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD0_PWR);
#endif
        videoDecoder->decoder.sid.poweredUp = false;
    }
    return 0;
}

#if NEXUS_HAS_ASTM
static NEXUS_Error NEXUS_VideoDecoder_SetAstmSettings_priv_Sid(NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderAstmSettings * pAstmSettings)
{
    /* not supported, but called unconditionally from higher level code. just absorb. */
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pAstmSettings);
    return NEXUS_SUCCESS;
}
#endif

#define NOT_SUPPORTED(x) NULL

const NEXUS_VideoDecoder_P_Interface NEXUS_VideoDecoder_P_Interface_Sid = {
    NEXUS_VideoDecoder_P_Close_Sid,
    NEXUS_VideoDecoder_P_GetOpenSettings_Common,
    NEXUS_VideoDecoder_P_GetSettings_Common,
    NEXUS_VideoDecoder_P_SetSettings_Sid,
    NEXUS_VideoDecoder_P_Start_Sid,
    NEXUS_VideoDecoder_P_Stop_Sid,
    NEXUS_VideoDecoder_P_Flush_Sid,
    NEXUS_VideoDecoder_P_GetStatus_Sid,
    NEXUS_VideoDecoder_P_GetConnector_Common,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetStreamInformation_Sid),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_SetStartPts_Sid),
    NEXUS_VideoDecoder_P_IsCodecSupported_Sid,
    NEXUS_VideoDecoder_P_SetPowerState_Sid,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_Reset_Sid),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetExtendedStatus_Sid),
    NEXUS_VideoDecoder_P_GetExtendedSettings_NotImplemented,
    NEXUS_VideoDecoder_P_SetExtendedSettings_NotImplemented,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_CreateStripedSurface_Sid),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_DestroyStripedSurface_Sid),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_CreateStripedMosaicSurfaces_Sid),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_DestroyStripedMosaicSurfaces_Sid),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetMostRecentPts_Sid),
    NEXUS_VideoDecoder_P_GetTrickState_Common,
    NEXUS_VideoDecoder_P_SetTrickState_Sid,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_FrameAdvance_Sid),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetNextPts_Sid),
    NEXUS_VideoDecoder_P_GetPlaybackSettings_Common,
    NEXUS_VideoDecoder_P_SetPlaybackSettings_Common,
    NOT_SUPPORTED(NEXUS_StillDecoder_P_Open_Sid),

#if NEXUS_HAS_ASTM
    NEXUS_VideoDecoder_GetAstmSettings_priv_Common,
    NEXUS_VideoDecoder_SetAstmSettings_priv_Sid,
    NEXUS_VideoDecoder_GetAstmStatus_Common_isr,
#endif

    NEXUS_VideoDecoder_GetDisplayConnection_priv_Sid,
    NEXUS_VideoDecoder_SetDisplayConnection_priv_Xdm,
    NEXUS_VideoDecoder_GetSourceId_priv_Sid,
    NEXUS_VideoDecoder_GetHeap_priv_Common,

#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_VideoDecoder_GetSyncSettings_priv_Common,
    NEXUS_VideoDecoder_SetSyncSettings_priv_Xdm,
    NEXUS_VideoDecoder_GetSyncStatus_Common_isr,
#endif

    NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Xdm,
    NOT_SUPPORTED(NEXUS_VideoDecoder_GetDecodedFrames_Sid),
    NOT_SUPPORTED(NEXUS_VideoDecoder_ReturnDecodedFrames_Sid)
};

