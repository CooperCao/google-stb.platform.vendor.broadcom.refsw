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

BDBG_MODULE(nexus_video_decoder_dsp);

BDBG_OBJECT_ID(NEXUS_VideoDecoder_Dsp);

#define BDBG_MSG_TRACE(x)  BDBG_MSG(x)

#define NEXUS_VIDEO_DECODER_P_DSP_DUMMY_DECODER 0   /* Enable this to debug the XDM interface.  Important!  TSM must be disabled or you will spin forever in an interrupt. */

#if NEXUS_VIDEO_DECODER_P_DSP_DUMMY_DECODER
static void *dummyPictureDecoder=NULL;
static const BXDM_Decoder_Interface NEXUS_VideoDecoder_P_Dummy_DecoderInterface;
static void NEXUS_VideoDecoder_P_Dummy_Init(void);
#endif


static void NEXUS_VideoDecoder_P_Close_Dsp( NEXUS_VideoDecoderHandle decoder)
{
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Close_Dsp: %#x", (unsigned)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);
    NEXUS_VideoDecoder_P_Close_Generic(decoder);
    NEXUS_VideoDecoder_P_Xdm_Shutdown(decoder);
    LOCK_AUDIO();
    NEXUS_DspVideoDecoder_Close_priv(decoder->decoder.dsp.decoder);
    UNLOCK_AUDIO();
    BDBG_OBJECT_DESTROY(decoder, NEXUS_VideoDecoder);
    BKNI_Free(decoder);
    return;
}

static NEXUS_Error NEXUS_VideoDecoder_P_SetSettings_Dsp( NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoderSettings *pSettings)
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);

    errCode = NEXUS_VideoDecoder_P_Xdm_ApplySettings(decoder, pSettings, false);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Store settings */
    decoder->settings = *pSettings;

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_VideoDecoder_P_Start_Dsp( NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoderStartSettings *pSettings)
{
    NEXUS_Error  rc;
    NEXUS_StcChannelDecoderConnectionSettings stcChannelConnectionSettings;
    BAVC_VideoCompressionStd videoCmprStd;
    NEXUS_RaveStatus raveStatus;
    bool playback = false;
    unsigned stcChannelIndex;
    NEXUS_DspVideoDecoderStartSettings dspStartSettings;

    BDBG_MSG(("NEXUS_VideoDecoder_P_Start_Dsp:%#x", (unsigned)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);

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

#if !NEXUS_VIDEO_DECODER_P_DSP_DUMMY_DECODER
    LOCK_AUDIO();
    NEXUS_DspVideoDecoder_GetDefaultStartSettings_priv(&dspStartSettings);
    dspStartSettings.raveContext = decoder->rave;
    dspStartSettings.codec = pSettings->codec;
    rc = NEXUS_DspVideoDecoder_Start_priv(decoder->decoder.dsp.decoder, &dspStartSettings);
    UNLOCK_AUDIO();
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
#endif

    rc = NEXUS_VideoDecoder_P_Start_Priv_Generic_Epilogue(decoder, pSettings);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    rc = NEXUS_VideoDecoder_P_Start_Generic_Epilogue(decoder, pSettings);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

static void NEXUS_VideoDecoder_P_Stop_Dsp( NEXUS_VideoDecoderHandle decoder)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Stop_Dsp: %#x", (unsigned)decoder));

    rc = NEXUS_VideoDecoder_P_Stop_Generic_Prologue(decoder);
    if(rc!=NEXUS_SUCCESS) {return;}

    if (decoder->fifoWatchdog.timer) {
        NEXUS_CancelTimer(decoder->fifoWatchdog.timer);
        decoder->fifoWatchdog.timer = NULL;
    }
    if (decoder->s3DTVStatusTimer) {
        NEXUS_CancelTimer(decoder->s3DTVStatusTimer);
        decoder->s3DTVStatusTimer = NULL;
    }

    decoder->started = false;
    decoder->astm.status.started = false;
    /* notify sync */
    if (decoder->sync.settings.startCallback_isr) {
        decoder->sync.status.started = false;
        BKNI_EnterCriticalSection();
        (*decoder->sync.settings.startCallback_isr)(decoder->sync.settings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }
    LOCK_AUDIO();
    NEXUS_DspVideoDecoder_Stop_priv(decoder->decoder.dsp.decoder);
    UNLOCK_AUDIO();
    NEXUS_VideoDecoder_P_Xdm_Stop(decoder);

    LOCK_TRANSPORT();
    NEXUS_Rave_Disable_priv(decoder->rave);
    NEXUS_Rave_Flush_priv(decoder->rave);
    NEXUS_Rave_RemovePidChannel_priv(decoder->rave);
    if(decoder->startSettings.enhancementPidChannel) {
        NEXUS_Rave_Disable_priv(decoder->enhancementRave);
        NEXUS_Rave_Flush_priv(decoder->enhancementRave);
        NEXUS_Rave_RemovePidChannel_priv(decoder->enhancementRave);
    }
    UNLOCK_TRANSPORT();

    NEXUS_VideoDecoder_P_Stop_Generic_Epilogue(decoder);

    return;
}

static void NEXUS_VideoDecoder_P_Flush_Dsp( NEXUS_VideoDecoderHandle decoder)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);

    NEXUS_VideoDecoder_P_Xdm_Stop(decoder);
    LOCK_AUDIO();
    NEXUS_DspVideoDecoder_Flush_priv(decoder->decoder.dsp.decoder);
    UNLOCK_AUDIO();
    rc = NEXUS_VideoDecoder_P_Xdm_Start(decoder);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}

    return;
}

static NEXUS_Error NEXUS_VideoDecoder_P_GetStatus_Dsp( NEXUS_VideoDecoderHandle decoder, NEXUS_VideoDecoderStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);
    NEXUS_VideoDecoder_P_GetStatus_Generic(decoder, pStatus);
    return NEXUS_VideoDecoder_P_GetStatus_Generic_Xdm(decoder, pStatus);
}


NEXUS_VideoDecoderHandle
NEXUS_VideoDecoder_P_Open_Dsp( unsigned index, const NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
    NEXUS_VideoDecoderHandle decoder;
    NEXUS_VideoDecoderOpenMosaicSettings mosaicSettings;
    NEXUS_RaveOpenSettings raveSettings;
    NEXUS_DspVideoDecoderOpenSettings  dspOpenSettings;
    const BXDM_Decoder_Interface *decoderInterface;
    void *decoderContext;
    NEXUS_Error rc;

    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Open_Dsp: %u", index));
    if(index>=NEXUS_NUM_DSP_VIDEO_DECODERS) { rc=BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_args;}
    decoder = BKNI_Malloc(sizeof(*decoder));
    if(!decoder) { rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}

    BKNI_Memset(decoder, 0, sizeof(*decoder));
    BDBG_OBJECT_SET(&decoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);
    decoder->intf = &NEXUS_VideoDecoder_P_Interface_Dsp;

    NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&mosaicSettings);
    if (pOpenSettings) {
        mosaicSettings.openSettings = *pOpenSettings;
    }

    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(&raveSettings);
    UNLOCK_TRANSPORT();

    LOCK_AUDIO();
    NEXUS_DspVideoDecoder_GetRaveSettings_priv(&raveSettings);
    UNLOCK_AUDIO();
    raveSettings.config.Cdb.Length = (3*1024*1024)/2;
    raveSettings.config.Itb.Length = (512*1024);
    raveSettings.supportedCodecs[NEXUS_VideoCodec_eVp6] = true;

    rc = NEXUS_VideoDecoder_P_Init_Generic(decoder, &raveSettings, &mosaicSettings);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    rc = NEXUS_VideoDecoder_P_getUnusedMfd(true, &decoder->decoder.dsp.sourceId, &decoder->decoder.dsp.heapIndex);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    LOCK_AUDIO();
    NEXUS_DspVideoDecoder_GetDefaultOpenSettings_priv(&dspOpenSettings);
    dspOpenSettings.pictureHeap = g_pCoreHandles->heap[decoder->decoder.dsp.heapIndex].mma;
    decoder->decoder.dsp.decoder = NEXUS_DspVideoDecoder_Open_priv(index, &dspOpenSettings);
    UNLOCK_AUDIO();
    if(!decoder->decoder.dsp.decoder) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;}

    LOCK_AUDIO();
    NEXUS_DspVideoDecoder_GetDecoderInterface_priv(decoder->decoder.dsp.decoder, &decoderInterface, &decoderContext);
    UNLOCK_AUDIO();
#if NEXUS_VIDEO_DECODER_P_DSP_DUMMY_DECODER
    NEXUS_VideoDecoder_P_Dummy_Init(decoder->decoder.dsp.heapIndex);
    decoderInterface = &NEXUS_VideoDecoder_P_Dummy_DecoderInterface;
    decoderContext = dummyPictureDecoder;
#endif

    rc = NEXUS_VideoDecoder_P_Xdm_Initialize(decoder, decoderInterface, decoderContext);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    return decoder;

error:
err_alloc:
err_args:
    return NULL;
}

NEXUS_Error NEXUS_VideoDecoderModule_P_Init_Dsp(const NEXUS_VideoDecoderModuleInternalSettings *pSettings)
{
    if(pSettings->audio==NULL) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return BERR_SUCCESS;
}

void NEXUS_VideoDecoderModule_P_Uninit_Dsp(void)
{
    return;
}

static void NEXUS_VideoDecoder_GetDisplayConnection_priv_Dsp( NEXUS_VideoDecoderHandle decoder, NEXUS_VideoDecoderDisplayConnection *pConnection)
{
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_GetDisplayConnection_priv_Dsp: %#x", (unsigned)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);
    NEXUS_ASSERT_MODULE();
    *pConnection = decoder->displayConnection;
    return;
}


static void NEXUS_VideoDecoder_GetSourceId_priv_Dsp( NEXUS_VideoDecoderHandle decoder, BAVC_SourceId *pSource)
{
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_GetSourceId_priv_Dsp: %#x", (unsigned)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);
    NEXUS_ASSERT_MODULE();
    *pSource = decoder->decoder.dsp.sourceId;
    return;
}

static NEXUS_Error NEXUS_VideoDecoder_P_SetTrickState_Dsp( NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoderTrickState *pState)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);

    decoder->trickState = *pState;
    return BERR_SUCCESS;
}

static void NEXUS_VideoDecoder_P_IsCodecSupported_Dsp( NEXUS_VideoDecoderHandle decoder, NEXUS_VideoCodec codec, bool *pSupported )
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);
    BDBG_ASSERT(pSupported);
    BSTD_UNUSED(decoder);

    *pSupported = (codec == NEXUS_VideoCodec_eVp6);
    return;
}

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

static NEXUS_Error NEXUS_VideoDecoder_P_SetPowerState_Dsp(NEXUS_VideoDecoderHandle videoDecoder, bool powerUp)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&videoDecoder->decoder.dsp, NEXUS_VideoDecoder_Dsp);

    if (powerUp && !videoDecoder->decoder.dsp.poweredUp)
    {
#if defined BCHP_PWR_RESOURCE_AVD0_PWR
        BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD0_PWR);
#endif
#if defined BCHP_PWR_RESOURCE_AVD0_CLK
        BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD0_CLK);
#endif
        videoDecoder->decoder.dsp.poweredUp = true;
    }
    else if ( !powerUp && videoDecoder->decoder.dsp.poweredUp )
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
        videoDecoder->decoder.dsp.poweredUp = false;
    }
    return 0;
}

static NEXUS_Error NEXUS_VideoDecoder_SetAstmSettings_priv_Dsp(NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderAstmSettings * pAstmSettings)
{
    /* not supported, but called unconditionally from higher level code. just absorb. */
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pAstmSettings);
    return NEXUS_SUCCESS;
}

#define NOT_SUPPORTED(x) NULL

const NEXUS_VideoDecoder_P_Interface NEXUS_VideoDecoder_P_Interface_Dsp = {
    NEXUS_VideoDecoder_P_Close_Dsp,
    NEXUS_VideoDecoder_P_GetOpenSettings_Common,
    NEXUS_VideoDecoder_P_GetSettings_Common,
    NEXUS_VideoDecoder_P_SetSettings_Dsp,
    NEXUS_VideoDecoder_P_Start_Dsp,
    NEXUS_VideoDecoder_P_Stop_Dsp,
    NEXUS_VideoDecoder_P_Flush_Dsp,
    NEXUS_VideoDecoder_P_GetStatus_Dsp,
    NEXUS_VideoDecoder_P_GetConnector_Common,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetStreamInformation_Dsp),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_SetStartPts_Dsp),
    NEXUS_VideoDecoder_P_IsCodecSupported_Dsp,
    NEXUS_VideoDecoder_P_SetPowerState_Dsp,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_Reset_Dsp),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetExtendedStatus_Dsp),
    NEXUS_VideoDecoder_P_GetExtendedSettings_NotImplemented,
    NEXUS_VideoDecoder_P_SetExtendedSettings_NotImplemented,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_CreateStripedSurface_Dsp),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_DestroyStripedSurface_Dsp),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_CreateStripedMosaicSurfaces_Dsp),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetMostRecentPts_Dsp),
    NEXUS_VideoDecoder_P_GetTrickState_Common,
    NEXUS_VideoDecoder_P_SetTrickState_Dsp,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_FrameAdvance_Dsp),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetNextPts_Dsp),
    NEXUS_VideoDecoder_P_GetPlaybackSettings_Common,
    NEXUS_VideoDecoder_P_SetPlaybackSettings_Common,
    NOT_SUPPORTED(NEXUS_StillDecoder_P_Open_Dsp),

#if NEXUS_HAS_ASTM
    NEXUS_VideoDecoder_GetAstmSettings_priv_Common,
    NEXUS_VideoDecoder_SetAstmSettings_priv_Dsp,
    NEXUS_VideoDecoder_GetAstmStatus_Common_isr,
#endif

    NEXUS_VideoDecoder_GetDisplayConnection_priv_Dsp,
    NEXUS_VideoDecoder_SetDisplayConnection_priv_Xdm,
    NEXUS_VideoDecoder_GetSourceId_priv_Dsp,
    NEXUS_VideoDecoder_GetHeap_priv_Common,
    NEXUS_VideoDecoder_GetSyncSettings_priv_Common,
    NEXUS_VideoDecoder_SetSyncSettings_priv_Xdm,
    NEXUS_VideoDecoder_GetSyncStatus_Common_isr,
    NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Xdm,
    NOT_SUPPORTED(NEXUS_VideoDecoder_GetDecodedFrames_Dsp),
    NOT_SUPPORTED(NEXUS_VideoDecoder_ReturnDecodedFrames_Dsp)
};

#if NEXUS_VIDEO_DECODER_P_DSP_DUMMY_DECODER
#include "blst_squeue.h"
#include "priv/nexus_core.h"

typedef struct NEXUS_VideoDecoder_P_FrameElement_Dummy {
    BXDM_Picture picture; /* must be the first member */
    BLST_SQ_ENTRY(NEXUS_VideoDecoder_P_FrameElement_Dummy) link;
} NEXUS_VideoDecoder_P_FrameElement_Dummy;
BLST_SQ_HEAD(NEXUS_VideoDecoder_P_FrameQueue_Head_Dummy, NEXUS_VideoDecoder_P_FrameElement_Dummy);

#define NEXUS_VIDEO_DECODER_P_DUMMYDECODE_N_FRAMES   4

typedef struct dummyState {
    struct NEXUS_VideoDecoder_P_FrameQueue_Head_Dummy ready;
    struct NEXUS_VideoDecoder_P_FrameQueue_Head_Dummy free; /* frames that could be used by the decoder */
    struct NEXUS_VideoDecoder_P_FrameQueue_Head_Dummy display; /* frames used in the display */
    struct NEXUS_VideoDecoder_P_FrameQueue_Head_Dummy recycled; /* frames recycled by the display */
    NEXUS_VideoDecoder_P_FrameElement_Dummy frames[NEXUS_VIDEO_DECODER_P_DUMMYDECODE_N_FRAMES];
    void *frameMemory;
} dummyState;


static BERR_Code NEXUS_VideoDecoder_P_Dummy_GetPictureCount_isr(void *context, uint32_t *pictureCount)
{
    const NEXUS_VideoDecoder_P_FrameElement_Dummy *element;
    dummyState *decoder = context;
    unsigned count;

    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Dummy_GetPictureCount_isr>:%#x", (unsigned)decoder));
    for(count=0,element=BLST_SQ_FIRST(&decoder->ready);element!=NULL;element=BLST_SQ_NEXT(element,link)) {
        count++;
    }
    *pictureCount = count;
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Dummy_GetPictureCount_isr<:%#x %u", (unsigned)decoder, *pictureCount));
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_VideoDecoder_P_Dummy_PeekAtPicture_isr(void *context, uint32_t index, const BXDM_Picture **pUnifiedPicture)
{
    dummyState *decoder = context;
    const NEXUS_VideoDecoder_P_FrameElement_Dummy *frame;
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Dummy_PeekAtPicture_isr:%#x %u", (unsigned)decoder, index));
    for(frame=BLST_SQ_FIRST(&decoder->ready);frame && index>0;index--) {
        frame = BLST_SQ_NEXT(frame, link);
    }
    if(frame==NULL) {
        *pUnifiedPicture = NULL;
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    *pUnifiedPicture = &frame->picture;
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_VideoDecoder_P_Dummy_GetNextPicture_isr(void *context, const BXDM_Picture **pUnifiedPicture)
{
    dummyState *decoder = context;
    NEXUS_VideoDecoder_P_FrameElement_Dummy *frame, *sentframe;

    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Dummy_GetNextPicture_isr>:%#x", (unsigned)decoder));
    while(NULL!=(frame=BLST_SQ_FIRST(&decoder->recycled))) {
        BLST_SQ_REMOVE_HEAD(&decoder->recycled, link);
        BLST_SQ_INSERT_TAIL(&decoder->free, frame, link);
    }
    frame=BLST_SQ_FIRST(&decoder->ready);
    if(frame==NULL) {
        *pUnifiedPicture = NULL;
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BLST_SQ_REMOVE_HEAD(&decoder->ready, link);
    BLST_SQ_INSERT_TAIL(&decoder->display, frame, link);
    *pUnifiedPicture = &frame->picture;
    sentframe = frame;
    /* add pictures back into the ready queue */
    while(NULL!=(frame=BLST_SQ_FIRST(&decoder->free))) {
        BLST_SQ_REMOVE_HEAD(&decoder->free, link);
        BLST_SQ_INSERT_TAIL(&decoder->ready, frame, link);
    }
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Dummy_GetNextPicture_isr<:%#x %#x %#x[%u]", (unsigned)decoder, (unsigned)*pUnifiedPicture, (*pUnifiedPicture)->stBufferInfo.pLumaBufferVirtualAddress, sentframe - decoder->frames));
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_VideoDecoder_P_Dummy_ReleasePicture_isr(void *context, const BXDM_Picture *pUnifiedPicture, const BXDM_Decoder_ReleasePictureInfo *pReleasePictureInfo)
{
    dummyState *decoder = context;
    NEXUS_VideoDecoder_P_FrameElement_Dummy *frame = (void *)pUnifiedPicture;
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Dummy_ReleasePicture_isr>:%#x %#x %#x", (unsigned)decoder, (unsigned)pUnifiedPicture, (unsigned)pReleasePictureInfo));
    BSTD_UNUSED(pReleasePictureInfo);
    BLST_SQ_REMOVE(&decoder->display, frame, NEXUS_VideoDecoder_P_FrameElement_Dummy, link);
    BLST_SQ_INSERT_TAIL(&decoder->recycled, frame, link);
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Dummy_ReleasePicture_isr<:%#x %#x %#x", (unsigned)decoder, (unsigned)pUnifiedPicture, (unsigned)pReleasePictureInfo));
    return BERR_SUCCESS;
}

static const BXDM_Decoder_Interface NEXUS_VideoDecoder_P_Dummy_DecoderInterface = {
    NEXUS_VideoDecoder_P_Dummy_GetPictureCount_isr,
    NEXUS_VideoDecoder_P_Dummy_PeekAtPicture_isr,
    NEXUS_VideoDecoder_P_Dummy_GetNextPicture_isr,
    NEXUS_VideoDecoder_P_Dummy_ReleasePicture_isr,
    NULL,
    NULL,
    NULL
};

#define NEXUS_VIDEO_DECODER_P_DUMMY_DEFAULT_WIDTH    352
#define NEXUS_VIDEO_DECODER_P_DUMMY_DEFAULT_HEIGHT   240


#define STRIPE_WIDTH_64     (64)
#define STRIPE_WIDTH_128    (128)

struct NEXUS_VideoDecoder_P_BufferInfo {
    unsigned lumaSize;
    unsigned chromaSize;
    uint32_t                 ulLuminanceNMBY;
    uint32_t                 ulChrominanceNMBY;
    uint32_t                 ulStripeWidth;
    unsigned long ulNumOfStripes, ulTotalByteWidth, ulTotalLumaHeight, ulTotalChromaHeight;
};

static void NEXUS_VideoDecoder_P_GetBufferInformation(unsigned width, unsigned height, struct NEXUS_VideoDecoder_P_BufferInfo *info)
{
    /* from ../../rockford/unittests/portinginterface/vdc/static_buffer.c */
    unsigned long ulNumOfStripes, ulTotalByteWidth, ulTotalLumaHeight, ulTotalChromaHeight;
    unsigned long             ulWidth = width;
    unsigned long             ulHeight = height;
    uint32_t                  ulStripeWidth = 64;
    unsigned long ulBankHeight, ulBankRemainder, ulBankAdd;
    unsigned long ulLumaBufSize, ulChromaBufSize;
    uint32_t                 ulLuminanceNMBY;
    uint32_t                 ulChrominanceNMBY;
    bool                      bChroma422 = false;

#if (BCHP_CHIP == 7425) || (BCHP_CHIP == 7422) || \
    (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) || (BCHP_CHIP == 7231)
    {
        uint32_t ulDataWidth;
        BCHP_GetFeature(g_pCoreHandles->chp, BCHP_Feature_eMemCtrl0DramWidthCount, &ulDataWidth);
        if(ulDataWidth == 16)
            ulStripeWidth = STRIPE_WIDTH_64;
        else
            ulStripeWidth = STRIPE_WIDTH_128;
    }
#endif

    {
        /* Determine number of vertical stripes for both luma and chroma
           buffers. ulStripeWidth is the width of the stripe in bytes */
        ulNumOfStripes = (ulWidth + (ulStripeWidth-1)) / ulStripeWidth;
        ulTotalByteWidth = ulStripeWidth * ulNumOfStripes;

        /* Determine the height of the luma stripe. Height is broken down
           into banks of 16 pixels high. */
        ulBankHeight = (ulHeight + 15)/16;
#if (BCHP_CHIP == 7405) || (BCHP_CHIP == 7325) || (BCHP_CHIP == 7335) || \
    (BCHP_CHIP == 7336) || (BCHP_CHIP == 3548) || (BCHP_CHIP == 3556)
        if(ulStripeWidth == STRIPE_WIDTH_128)
        {
            /* NOTE: NMBY must be 16 * N + 8; */
            ulBankRemainder = ulBankHeight & 15;
            /* 0 + 4; 1 + 3; 2 + 2; 3 + 1; 4 + 0; 5 + 7; 6 + 6; 7 + 5*/
            ulBankAdd = (8 - ulBankRemainder) & 15;
        }
        else
        {
            /* NOTE: NMBY must be 8 * N + 4; */
            ulBankRemainder = ulBankHeight & 7;
            /* 0 + 4; 1 + 3; 2 + 2; 3 + 1; 4 + 0; 5 + 7; 6 + 6; 7 + 5*/
            ulBankAdd = (4 - ulBankRemainder) & 7;
        }
#elif (BCHP_CHIP == 7420) || (BCHP_CHIP == 7425) || (BCHP_CHIP == 7422) || \
      (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) || (BCHP_CHIP == 7231)
    /* NOTE: NMBY must be 8 * N + 4; */
    ulBankRemainder = ulBankHeight & 7;
    /* 0 + 4; 1 + 3; 2 + 2; 3 + 1; 4 + 0; 5 + 7; 6 + 6; 7 + 5*/
    ulBankAdd = (4 - ulBankRemainder) & 7;
#else
        /* NOTE: NMBY must be 4 * N + 2; */
        ulBankRemainder = ulBankHeight & 3;
        /* 0 + 2; 1 + 1; 2 + 0; 3 + 3; */
        ulBankAdd = (2 - ulBankRemainder) & 3;
#endif
        ulTotalLumaHeight = (ulBankHeight + ulBankAdd) * 16;
        ulLuminanceNMBY = (ulBankHeight + ulBankAdd);

        /* Determine the height of the chroma stript. When format is
           4:2:0 then chroma is based on 1/2 pixel height. When format is
           4:2:2 then chroma is based on pixel height */
        ulBankHeight = ((ulHeight >> (1 - bChroma422)) + 15) / 16;
#if (BCHP_CHIP == 7405) || (BCHP_CHIP == 7325) || (BCHP_CHIP == 7335) || \
    (BCHP_CHIP == 7336) || (BCHP_CHIP == 3548) || (BCHP_CHIP == 3556)
        if(ulStripeWidth == STRIPE_WIDTH_128)
        {
            ulBankRemainder = ulBankHeight & 15;
            ulBankAdd = (8 - ulBankRemainder) & 15;
        }
        else
        {
            ulBankRemainder = ulBankHeight & 7;
            ulBankAdd = (4 - ulBankRemainder) & 7;
        }
#elif (BCHP_CHIP == 7420) || (BCHP_CHIP == 7425) || (BCHP_CHIP == 7422) || \
      (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) || (BCHP_CHIP == 7231)
        ulBankRemainder = ulBankHeight & 7;
        ulBankAdd = (4 - ulBankRemainder) & 7;
#else
        ulBankRemainder = ulBankHeight & 3;
        ulBankAdd = (2 - ulBankRemainder) & 3;
#endif
        ulTotalChromaHeight = (ulBankHeight + ulBankAdd) * 16;
        ulChrominanceNMBY = (ulBankHeight + ulBankAdd);

        /* Calculate size of luma and chroma buffers in bytes */
        ulLumaBufSize   = ulTotalByteWidth*ulTotalLumaHeight;
        ulChromaBufSize = ulTotalByteWidth*ulTotalChromaHeight;
    }

    info->lumaSize = ulLumaBufSize;
    info->chromaSize = ulChromaBufSize;
    info->ulLuminanceNMBY = ulLuminanceNMBY;
    info->ulChrominanceNMBY = ulChrominanceNMBY;
    info->ulStripeWidth = ulStripeWidth;
    info->ulNumOfStripes = ulNumOfStripes;
    info->ulTotalByteWidth = ulTotalByteWidth;
    info->ulTotalLumaHeight = ulTotalLumaHeight;
    info->ulTotalChromaHeight = ulTotalChromaHeight;

    return;
}

static void NEXUS_VideoDecoder_P_FrameInit_Dummy(NEXUS_VideoDecoder_P_FrameElement_Dummy *frame, void *buffer, unsigned heapIndex)
{
    struct NEXUS_VideoDecoder_P_BufferInfo bufferInfo;
    size_t luma_size, chroma_size;
    size_t width = NEXUS_VIDEO_DECODER_P_DUMMY_DEFAULT_WIDTH;
    size_t height = NEXUS_VIDEO_DECODER_P_DUMMY_DEFAULT_HEIGHT;

    NEXUS_VideoDecoder_P_GetBufferInformation(width, height, &bufferInfo);
    luma_size = bufferInfo.lumaSize;
    chroma_size = bufferInfo.chromaSize;

    frame->picture.stBufferInfo.pLumaBufferVirtualAddress = buffer;
    frame->picture.stBufferInfo.pChromaBufferVirtualAddress = (uint8_t *)buffer + luma_size;
    frame->picture.stBufferInfo.hHeap = g_pCoreHandles->heap[heapIndex].nexus;
    frame->picture.stBufferInfo.stSource.uiWidth = width;
    frame->picture.stBufferInfo.stSource.uiHeight = height;
    frame->picture.stBufferInfo.stSource.bValid = true;
    frame->picture.stBufferInfo.stDisplay = frame->picture.stBufferInfo.stSource;
#if 0
    frame->picture.stBufferInfo.eStripeWidth = BAVC_StripeWidth_e64Byte;
#else
    frame->picture.stBufferInfo.eStripeWidth = BAVC_StripeWidth_e128Byte;
#endif
    frame->picture.stBufferInfo.uiLumaStripeHeight = bufferInfo.ulLuminanceNMBY << 4;
    frame->picture.stBufferInfo.uiChromaStripeHeight = bufferInfo.ulChrominanceNMBY << 4;
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eFrame].bValid = true;
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eFrame].eMpegType = BAVC_MpegType_eMpeg2;
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eTopField].bValid = true;
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eTopField].eMpegType= BAVC_MpegType_eMpeg2;
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eBotField].bValid = true;
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eBotField].eMpegType= BAVC_MpegType_eMpeg2;
    frame->picture.stBufferInfo.eYCbCrType = BAVC_YCbCrType_e4_2_0;
    frame->picture.stBufferInfo.ePulldown = BXDM_Picture_PullDown_eFrameX1;
    frame->picture.stBufferInfo.eSourceFormat = BXDM_Picture_SourceFormat_eProgressive;
    frame->picture.stBufferInfo.bValid = true;
    frame->picture.stFrameRate.eType = BXDM_Picture_FrameRateType_eFixed;
    frame->picture.stFrameRate.stRate.uiNumerator = 25000;
    frame->picture.stFrameRate.stRate.uiDenominator = 1000;
    frame->picture.stFrameRate.bValid = true;

    return;
}


static void NEXUS_VideoDecoder_P_Dummy_Init(unsigned heapIndex)
{
    struct NEXUS_VideoDecoder_P_BufferInfo bufferInfo;
    dummyState *decoder;
    size_t frame_size;
    unsigned i;
    void *pCached;
    BMEM_Heap_Handle heap = g_pCoreHandles->heap[heapIndex];
    int rc;

    decoder = BKNI_Malloc(sizeof(*decoder));
    BDBG_ASSERT(decoder);
    BKNI_Memset(decoder, 0, sizeof(*decoder));

    NEXUS_VideoDecoder_P_GetBufferInformation(NEXUS_VIDEO_DECODER_P_DUMMY_DEFAULT_WIDTH, NEXUS_VIDEO_DECODER_P_DUMMY_DEFAULT_HEIGHT, &bufferInfo);
    frame_size = bufferInfo.lumaSize + bufferInfo.chromaSize;

    decoder->frameMemory = BMEM_Alloc(heap, frame_size*(NEXUS_VIDEO_DECODER_P_DUMMYDECODE_N_FRAMES));
    BDBG_ASSERT(decoder->frameMemory);
    rc = BMEM_ConvertAddressToCached(g_pCoreHandles->heap[heapIndex], decoder->frameMemory, &pCached);
    if (!rc) {
        for ( i = 0; i < NEXUS_VIDEO_DECODER_P_DUMMYDECODE_N_FRAMES; i++ )
        {
            uint8_t *pFrame = pCached;
            pFrame += frame_size*i;
            BKNI_Memset(pFrame, 0xff/(NEXUS_VIDEO_DECODER_P_DUMMYDECODE_N_FRAMES-i), bufferInfo.lumaSize);
            BKNI_Memset(pFrame + bufferInfo.lumaSize, 0x80, bufferInfo.chromaSize);
        }
        BMEM_FlushCache(g_pCoreHandles->heap[heapIndex], pCached, frame_size*(NEXUS_VIDEO_DECODER_P_DUMMYDECODE_N_FRAMES));
    }
    BLST_SQ_INIT(&decoder->display);
    BLST_SQ_INIT(&decoder->recycled);
    BLST_SQ_INIT(&decoder->free);
    BLST_SQ_INIT(&decoder->ready);
    for(i=0;i<NEXUS_VIDEO_DECODER_P_DUMMYDECODE_N_FRAMES;i++) {
        NEXUS_VideoDecoder_P_FrameInit_Dummy(&decoder->frames[i], (uint8_t *)decoder->frameMemory + frame_size*i, heapIndex);
        BLST_SQ_INSERT_TAIL(&decoder->ready, &decoder->frames[i], link);
    }
    dummyPictureDecoder=decoder;
    return;
}


#endif /* NEXUS_VIDEO_DECODER_P_DSP_DUMMY_DECODER */

