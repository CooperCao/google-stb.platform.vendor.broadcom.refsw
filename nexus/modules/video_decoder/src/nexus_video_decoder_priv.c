/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 *************************************************************************/
#include "nexus_video_decoder_module.h"
#include "priv/nexus_core.h"
#include "nexus_video_decoder_private.h"
#include "bxvd_pvr.h"
#include "nexus_video_decoder_security.h"
#if NEXUS_HAS_SAGE
#include "nexus_sage.h"
#endif

BDBG_MODULE(nexus_video_decoder_priv);

BTRC_MODULE_DECLARE(ChnChange_DecodeStopVideo);
BTRC_MODULE_DECLARE(ChnChange_Tune);
BTRC_MODULE_DECLARE(ChnChange_TuneLock);
BTRC_MODULE_DECLARE(ChnChange_DecodeStartVideo);
BTRC_MODULE_DECLARE(ChnChange_DecodeStartVideoXVD);
BTRC_MODULE_DECLARE(ChnChange_DecodeFirstVideo);
BTRC_MODULE_DECLARE(ChnChange_DecodeFirstvPTSPassed);
BTRC_MODULE_DECLARE(ChnChange_SyncUnmuteVideo);
BTRC_MODULE_DECLARE(ChnChange_Total_Video);

#if B_HAS_TRC
static const BTRC_Module *video_btrc_modules[] = {
    BTRC_MODULE_HANDLE(ChnChange_DecodeStopVideo),
    BTRC_MODULE_HANDLE(ChnChange_Tune),
    BTRC_MODULE_HANDLE(ChnChange_TuneLock),
    BTRC_MODULE_HANDLE(ChnChange_DecodeStartVideo),
    BTRC_MODULE_HANDLE(ChnChange_DecodeStartVideoXVD),
    BTRC_MODULE_HANDLE(ChnChange_DecodeFirstVideo),
    BTRC_MODULE_HANDLE(ChnChange_DecodeFirstvPTSPassed),
    BTRC_MODULE_HANDLE(ChnChange_SyncUnmuteVideo),
    BTRC_MODULE_HANDLE(ChnChange_Total_Video)
};

#define VIDEO_BTRC_N_MODULES ((sizeof(video_btrc_modules)/sizeof(*video_btrc_modules)))
#endif

static NEXUS_VideoDecoderModuleStatistics g_NEXUS_VideoDecoderModuleStatistics;

void NEXUS_VideoDecoder_P_WatchdogHandler(void *data)
{
    int i;
    NEXUS_Error rc;

    /* must watchdog all HVD cores because of secure HVD FW */
    BSTD_UNUSED(data);

#if NEXUS_HAS_SAGE
    NEXUS_Sage_DisableHvd();
#endif

    for (i=0; i<NEXUS_MAX_XVD_DEVICES; i++) {
        struct NEXUS_VideoDecoderDevice *vDevice = &g_NEXUS_videoDecoderXvdDevices[i];
        unsigned j;
        if (!vDevice->xvd) continue;

        vDevice->numWatchdogs++;

        /* when this is called, the xvd core is reset. now we need to disable and flush each rave context,
        then let xvd process the watchdog (which will bring raptor back up to its previous state),
        then enable rave and let the data flow. */
        LOCK_TRANSPORT();
        for (j=0;j<NEXUS_NUM_XVD_CHANNELS;j++) {
            NEXUS_VideoDecoderHandle videoDecoder = vDevice->channel[j];
            if (videoDecoder && videoDecoder->started) {
                NEXUS_Rave_Disable_priv(videoDecoder->rave);
                NEXUS_Rave_Flush_priv(videoDecoder->rave);
            }
        }
        UNLOCK_TRANSPORT();

        BXVD_ProcessWatchdog(vDevice->xvd);
    }

#if NEXUS_HAS_SAGE
    rc = NEXUS_Sage_EnableHvd();
    if (rc) BERR_TRACE(rc); /* keep going */
#endif

    rc = NEXUS_VideoDecoder_P_BootDecoders();
    if (rc) BERR_TRACE(rc); /* keep going */

    for (i=0; i<NEXUS_MAX_XVD_DEVICES; i++) {
        struct NEXUS_VideoDecoderDevice *vDevice = &g_NEXUS_videoDecoderXvdDevices[i];
        if (!vDevice->xvd) continue;

        rc = BXVD_ProcessWatchdogRestartDecoder(vDevice->xvd);
        if (rc) BERR_TRACE(rc); /* keep going */

        LOCK_TRANSPORT();
        for (i=0;i<NEXUS_NUM_XVD_CHANNELS;i++) {
            NEXUS_VideoDecoderHandle videoDecoder = vDevice->channel[i];
            if (videoDecoder && videoDecoder->started) {
                NEXUS_Rave_Enable_priv(videoDecoder->rave);
            }
        }
        UNLOCK_TRANSPORT();

#if NEXUS_HAS_ASTM
        for (i = 0; i < NEXUS_NUM_XVD_CHANNELS; i++)
        {
            NEXUS_VideoDecoderHandle videoDecoder = vDevice->channel[i];
            if (videoDecoder && videoDecoder->started && videoDecoder->astm.settings.enableAstm)
            {
                NEXUS_Callback astm_watchdog_isr = videoDecoder->astm.settings.watchdog_isr;
                BDBG_MSG(("Video channel %p is notifying ASTM of its watchdog recovery", (void *)videoDecoder));
                if (astm_watchdog_isr)
                {
                    BKNI_EnterCriticalSection();
                    astm_watchdog_isr(videoDecoder->astm.settings.callbackContext, 0);
                    BKNI_LeaveCriticalSection();
                }
            }
        }
#endif
    }
}

void NEXUS_VideoDecoder_P_Watchdog_isr(void *data, int not_used, void *not_used2)
{
    struct NEXUS_VideoDecoderDevice *vDevice = (struct NEXUS_VideoDecoderDevice *)data;
    BSTD_UNUSED(not_used);
    BSTD_UNUSED(not_used2);
    /* convert from isr to event */
    BKNI_SetEvent(vDevice->watchdog_event);
}

#if !BDBG_NO_MSG
static void NEXUS_VideoDecoder_P_DataReady_PrintPicture_isr(NEXUS_VideoDecoderHandle videoDecoder, const BAVC_MFD_Picture * pFieldData)
{
    char chString[8];
    char arString[32];
    static const char *g_arStr[BFMT_AspectRatio_eSAR+1] = { "<unknown a/r>", "SqPix", "4:3", "16:9", "2.21:1", "15:9", "SAR" };
    static const char *g_frameRateStr[BAVC_FrameRateCode_eMax+1] = { "<unknown fr>", "23.976", "24", "25", "29.97", "30", "50", "59.94", "60", "14.985", "7.493", "invalid" };

    if (videoDecoder->mosaicMode) {
        BKNI_Snprintf(chString, sizeof(chString), "ch[%u.%u]", videoDecoder->parentIndex, videoDecoder->index);
    }
    else {
        BKNI_Snprintf(chString, sizeof(chString), "ch[%u]", videoDecoder->parentIndex);
    }
    if (pFieldData->eAspectRatio > BFMT_AspectRatio_eSAR) {
        BKNI_Snprintf(arString, 32, "<bad a/r>");
    }
    else if (pFieldData->eAspectRatio == BFMT_AspectRatio_eSAR) {
        BKNI_Snprintf(arString, 32, "%s%d:%d", g_arStr[pFieldData->eAspectRatio], pFieldData->uiSampleAspectRatioX, pFieldData->uiSampleAspectRatioY);
    }
    else {
        BKNI_Snprintf(arString, 32, "%s", g_arStr[pFieldData->eAspectRatio]);
    }

    BDBG_MSG(("%s %p:%8u %dx%d%c %dx%d %shz %s%s%s",
        chString,
        (void *)pFieldData->hLuminanceFrameBufferBlock, (unsigned)pFieldData->ulLuminanceFrameBufferBlockOffset, /* use to check for unique buffers */
        pFieldData->ulSourceHorizontalSize, pFieldData->ulSourceVerticalSize,
        pFieldData->eSourcePolarity==BAVC_Polarity_eTopField?'t':pFieldData->eSourcePolarity==BAVC_Polarity_eBotField?'b':'p',
        pFieldData->ulDisplayHorizontalSize, pFieldData->ulDisplayVerticalSize,
        g_frameRateStr[pFieldData->eFrameRateCode],
        arString,
        pFieldData->bMute?" muted":"",
        pFieldData->bPictureRepeatFlag?" repeat":""));
}
#endif

static struct {
    /* prevent ISR stack blowout by moving large ISR stack storage in global data */
    NEXUS_VideoDecoderStreamInformation streamInfo;
    BAVC_MFD_Picture modifiedFieldData;
    BXVD_DecodeSettings decodeSettings;
    BXVD_ChannelStatus channelStatus;
} g_NEXUS_videoDecoderIsrStorage;

const BAVC_MFD_Picture *
NEXUS_VideoDecoder_P_DataReady_PreprocessFieldData_isr(NEXUS_VideoDecoderHandle videoDecoder, const BAVC_MFD_Picture *pFieldData)
{
    BAVC_MFD_Picture *pModifiedFieldData = &g_NEXUS_videoDecoderIsrStorage.modifiedFieldData;
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    /* modify values from XVD if necessary */
    if (pFieldData->eAspectRatio == BFMT_AspectRatio_eUnknown && videoDecoder->startSettings.aspectRatio != NEXUS_AspectRatio_eUnknown) {
        if (pFieldData != pModifiedFieldData) {
            *pModifiedFieldData = *pFieldData;
            pFieldData = pModifiedFieldData;
        }
        pModifiedFieldData->eAspectRatio = NEXUS_P_AspectRatio_ToMagnum_isrsafe(videoDecoder->startSettings.aspectRatio);
        pModifiedFieldData->uiSampleAspectRatioX = videoDecoder->startSettings.sampleAspectRatio.x;
        pModifiedFieldData->uiSampleAspectRatioY = videoDecoder->startSettings.sampleAspectRatio.y;
    }
    if (pFieldData->eTransferCharacteristics == BAVC_TransferCharacteristics_eUnknown) {
        if (pFieldData != pModifiedFieldData) {
            *pModifiedFieldData = *pFieldData;
            pFieldData = pModifiedFieldData;
        }
        /* BDBG_CASSERT is in NEXUS_VideoDecoder_P_PictureParams_isr */
        pModifiedFieldData->eTransferCharacteristics = NEXUS_P_TransferCharacteristics_ToMagnum_isrsafe(videoDecoder->startSettings.defaultTransferCharacteristics);
    }
    if (pFieldData->eColorPrimaries == BAVC_ColorPrimaries_eUnknown) {
        if (pFieldData != pModifiedFieldData) {
            *pModifiedFieldData = *pFieldData;
            pFieldData = pModifiedFieldData;
        }
        /* BDBG_CASSERT is in NEXUS_VideoDecoder_P_PictureParams_isr */
        pModifiedFieldData->eColorPrimaries = NEXUS_P_ColorPrimaries_ToMagnum_isrsafe(videoDecoder->startSettings.defaultColorPrimaries);
    }
    if (pFieldData->eMatrixCoefficients == BAVC_MatrixCoefficients_eUnknown) {
        if (pFieldData != pModifiedFieldData) {
            *pModifiedFieldData = *pFieldData;
            pFieldData = pModifiedFieldData;
        }
        /* BDBG_CASSERT is in NEXUS_VideoDecoder_P_PictureParams_isr */
        pModifiedFieldData->eMatrixCoefficients = NEXUS_P_MatrixCoefficients_ToMagnum_isrsafe(videoDecoder->startSettings.defaultMatrixCoefficients);
    }

    if (pFieldData->bPictureRepeatFlag &&
        videoDecoder->trickState.lowLatencyDisplayForTrickModes &&
        (videoDecoder->trickState.rate != NEXUS_NORMAL_DECODE_RATE ||
         !videoDecoder->trickState.tsmEnabled ||
         videoDecoder->trickState.hostTrickModesEnabled ||
         videoDecoder->trickState.brcmTrickModesEnabled ||
         videoDecoder->trickState.stcTrickEnabled))
     {
        if (pFieldData != pModifiedFieldData) {
            *pModifiedFieldData = *pFieldData;
            pFieldData = pModifiedFieldData;
        }
        /* erase repeat flag, which causes picture to be processed by MAD */
        pModifiedFieldData->bPictureRepeatFlag = false;
    }

    return pFieldData;
}

void
NEXUS_VideoDecoder_P_DataReady_Generic_Prologue_isr(NEXUS_VideoDecoderHandle videoDecoder, const BAVC_MFD_Picture *pFieldData)
{
    bool source_changed = false;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    /* Verify that we've received a good picture from XVD.
    If bad data could be caused by a bad stream, we should not ASSERT. We need to survive that.
    Do not ASSERT if the enum is likely to grow and there is no inherent eMax. No one will know to modify this code and it could die unexpectedly.
    If bad data could only come from bad FW or bad SW, we should ASSERT and get it fixed. */
    BDBG_ASSERT(pFieldData->eFrameRateCode < BAVC_FrameRateCode_eMax); /* has an eMax */
    BDBG_ASSERT(pFieldData->eInterruptPolarity <= BAVC_Polarity_eFrame); /* unlikely to expand */

    if (videoDecoder->dataReadyCount == 0 && pFieldData->bMute) {
        /* the first muted frames after a start are not decoded pictures. data is not ready.
        therefore, we should not count them, nor report the BAVC_MFD_Picture data as status, nor compare with previous BAVC_MFD_Picture data.
        the DataReady_isr still needs to go to Display, but it's essentially a vsync interrupt with no data. */
    }
    else {
#if B_HAS_TRC
        /* here we are unmuted and we have valid data ready, so stop ch chg meas. */
        if (!videoDecoder->validOutputPic) /* this flag is checked before update, so we can catch the edge */
        {
            if (BTRC_MODULE_HANDLE(ChnChange_DecodeStopVideo)->stats[0].count) {
                BTRC_TRACE(ChnChange_Total_Video, STOP);
                BKNI_SetEvent(videoDecoder->channelChangeReportEvent);
            }
        }
#endif
        videoDecoder->validOutputPic = true;

        videoDecoder->dataReadyCount++;

        /* before storing BAVC_MVD_Field, check for significant changes, but don't set event until end of function */
        if (pFieldData->eAspectRatio != videoDecoder->last_field.eAspectRatio ||
            (pFieldData->eAspectRatio == BFMT_AspectRatio_eSAR && (
                pFieldData->uiSampleAspectRatioX != videoDecoder->last_field.uiSampleAspectRatioX ||
                pFieldData->uiSampleAspectRatioY != videoDecoder->last_field.uiSampleAspectRatioY)) ||
            pFieldData->eFrameRateCode != videoDecoder->last_field.eFrameRateCode ||
            pFieldData->bStreamProgressive != videoDecoder->last_field.bStreamProgressive ||
            pFieldData->ulSourceHorizontalSize != videoDecoder->last_field.ulSourceHorizontalSize ||
            pFieldData->ulSourceVerticalSize != videoDecoder->last_field.ulSourceVerticalSize ||
            (pFieldData->bLast && !videoDecoder->last_field_flag) )
        {
            source_changed = true;
        }
        if ( !(videoDecoder->started && videoDecoder->startSettings.appDisplayManagement) ) {
            videoDecoder->last_field = *pFieldData;
        }

        if ( pFieldData->bLast ) {
            videoDecoder->last_field_flag = true;
        }

        {
            NEXUS_VideoDecoderDynamicRangeMetadataType dynamicMetadataType = NEXUS_VideoDecoderDynamicRangeMetadataType_eNone;
            switch (pFieldData->stHdrMetadata.eType)
            {
                case BAVC_HdrMetadataType_eDrpu:
                    dynamicMetadataType = NEXUS_VideoDecoderDynamicRangeMetadataType_eDolbyVision;
                    break;
                case BAVC_HdrMetadataType_eTch_Cri:
                case BAVC_HdrMetadataType_eTch_Cvri:
                    dynamicMetadataType = NEXUS_VideoDecoderDynamicRangeMetadataType_eTechnicolorPrime;
                    break;
                default:
                    break;
            }
            if (videoDecoder->dynamicMetadataType != dynamicMetadataType) {
                videoDecoder->dynamicMetadataType = dynamicMetadataType;
                NEXUS_IsrCallback_Fire_isr(videoDecoder->streamChangedCallback);
                NEXUS_IsrCallback_Fire_isr(videoDecoder->private.streamChangedCallback);
            }
        }
    }

    if (pFieldData->ulAdjQp > 400) {
        BDBG_WRN(("Too large ulAdjQp: %u", (unsigned)pFieldData->ulAdjQp));
    }

#if !BDBG_NO_MSG
    NEXUS_VideoDecoder_P_DataReady_PrintPicture_isr(videoDecoder, pFieldData);
#endif

#if NEXUS_VIDEO_DECODER_EXTENSION_PREPROCESS_PICTURE
    NEXUS_VideoDecoder_P_PreprocessPicture_isr(videoDecoder, pFieldData);
#endif

    if (source_changed) {
        NEXUS_IsrCallback_Fire_isr(videoDecoder->sourceChangedCallback);
    }
}

#define NEXUS_VIDEO_DECODER_P_FIFO_EMPTY_COUNT 2


void
NEXUS_VideoDecoder_P_FlushFifoEmpty(NEXUS_VideoDecoderHandle videoDecoder)
{
    BKNI_EnterCriticalSection();
    videoDecoder->fifoEmpty.count = 0;
    videoDecoder->fifoEmpty.emptyCount = 0;
    videoDecoder->fifoEmpty.noLongerEmptyCount = 0;
    videoDecoder->fifoEmpty.lastCdbReadPointer = 0;
    videoDecoder->fifoEmpty.lastCdbValidPointer = 0;
    BKNI_LeaveCriticalSection();
}

static void
NEXUS_VideoDecoder_P_UpdateFifoEmpty_isr(NEXUS_VideoDecoderHandle videoDecoder)
{
    uint64_t cdbValidPointer, cdbReadPointer;

    NEXUS_Rave_GetCdbPointers_isr(videoDecoder->rave, &cdbValidPointer, &cdbReadPointer);

    if (videoDecoder->firstPtsReady &&
        (cdbValidPointer == videoDecoder->fifoEmpty.lastCdbValidPointer) &&
        (videoDecoder->pictureDeliveryCount == 0)) {
        if (videoDecoder->fifoEmpty.count < NEXUS_VIDEO_DECODER_P_FIFO_EMPTY_COUNT) {
            videoDecoder->fifoEmpty.count++;
        }
        else if (videoDecoder->fifoEmpty.count == NEXUS_VIDEO_DECODER_P_FIFO_EMPTY_COUNT) {
            videoDecoder->fifoEmpty.emptyCount++;
            /* fire it once when we reach NEXUS_VIDEO_DECODER_P_FIFO_EMPTY_COUNT */
            NEXUS_IsrCallback_Fire_isr(videoDecoder->fifoEmpty.callback);
            videoDecoder->fifoEmpty.count++; /* set to NEXUS_VIDEO_DECODER_P_FIFO_EMPTY_COUNT+1 and leave it there */
            /* we can't just keep on incrementing. a box could be left in paused state for days and eventually the count would wrap. */
        }
    }
    else {
        videoDecoder->fifoEmpty.count = 0;
        if (videoDecoder->pictureDeliveryCount > 0
            && (videoDecoder->fifoEmpty.emptyCount > videoDecoder->fifoEmpty.noLongerEmptyCount)
            && videoDecoder->settings.stateChanged.enabled.fifoNoLongerEmpty)
        {
            videoDecoder->fifoEmpty.noLongerEmptyCount++;
            NEXUS_IsrCallback_Fire_isr(videoDecoder->stateChangedCallback);
        }
    }
    videoDecoder->fifoEmpty.lastCdbValidPointer = cdbValidPointer;
    videoDecoder->fifoEmpty.lastCdbReadPointer = cdbReadPointer;
}

void
NEXUS_VideoDecoder_P_DataReady_Generic_Epilogue_isr(NEXUS_VideoDecoderHandle videoDecoder, const BAVC_MFD_Picture *pFieldData, unsigned pictureDeliveryCount)
{
    /* this callback must occur *only* for the parent decoder in mosaic mode */
    if (videoDecoder->displayConnection.dataReadyCallback_isr) {
        (*videoDecoder->displayConnection.dataReadyCallback_isr)(videoDecoder->displayConnection.callbackContext, pFieldData);
    }

    videoDecoder->pictureDeliveryCount = pictureDeliveryCount;

    /* this call only makes sense for the parent decoder in mosaic mode */
    NEXUS_VideoDecoder_P_UpdateFifoEmpty_isr(videoDecoder);

    /* fake first pts passed in non-TSM mode */
    if (!videoDecoder->firstPtsPassed
        && (videoDecoder->pictureDeliveryCount > 0)
        && videoDecoder->started
        && !videoDecoder->startSettings.stcChannel)
    {
        NEXUS_VideoDecoder_P_FirstPtsPassed_isr(videoDecoder, 0, NULL);
    }

	/* Don't fire this callback in app-dm mode.  It's fired elsewhere. */
    if ( !(videoDecoder->started && videoDecoder->startSettings.appDisplayManagement) ) {
		/* TODO: do we want one of these for each mosaic decoder? */
        NEXUS_IsrCallback_Fire_isr(videoDecoder->dataReadyCallback);
    }
    return;
}

static NEXUS_VideoDecoderHandle nexus_p_find_mosaic_channel_isr(struct NEXUS_VideoDecoderDevice *device, unsigned mosaicIndex)
{
    unsigned i;
    NEXUS_VideoDecoderHandle videoDecoder;
    for (i=0;i<NEXUS_NUM_XVD_CHANNELS;i++) {
        videoDecoder = device->channel[i];
        if (videoDecoder && videoDecoder->dec && videoDecoder->mosaicMode && videoDecoder->mosaicIndex == mosaicIndex) return videoDecoder;
    }
    if (device->slaveLinkedDevice) {
        BDBG_ASSERT(!device->slaveLinkedDevice->slaveLinkedDevice); /* assert one level of recursion */
        videoDecoder = nexus_p_find_mosaic_channel_isr(device->slaveLinkedDevice, mosaicIndex);
        if (videoDecoder && videoDecoder->linkedDevice == device) return videoDecoder;
    }

    return NULL;
}

void
NEXUS_VideoDecoder_P_DataReady_isr(void *data, int unused, void *field)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;
    const BAVC_MFD_Picture *pFieldData = (BAVC_MFD_Picture*)field;
    BXVD_ChannelStatus *xvdChannelStatus = &g_NEXUS_videoDecoderIsrStorage.channelStatus;
    const BAVC_MFD_Picture *pNext;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BSTD_UNUSED(unused);

    BXVD_GetChannelStatus_isr(videoDecoder->dec, xvdChannelStatus);
    /* BDBG_ERR(("%u %u %u", pFieldData->eMatrixCoefficients, pFieldData->eColorPrimaries, pFieldData->eTransferCharacteristics)); */

    /* this preprocessing of the field data only applies to the parent/decoupled decoder */
    pFieldData = NEXUS_VideoDecoder_P_DataReady_PreprocessFieldData_isr(videoDecoder, pFieldData);

    /* keep single decode prologue separate from mosaic */
    if (!pFieldData->pNext) {
        NEXUS_VideoDecoder_P_DataReady_Generic_Prologue_isr(videoDecoder, pFieldData);
    }
    else {
        /* do generic prologue for each mosaic decoder.
        The pFieldData linked list maps to channel[], but the matrix is sparse. */
        for (pNext = pFieldData; pNext; pNext = pNext->pNext) {
            NEXUS_VideoDecoderHandle v;
            /* pNext->ulChannelId is uiVDCRectangleNum, which is assigned to videoDecoder->mosaicIndex at start time */
            v = nexus_p_find_mosaic_channel_isr(videoDecoder->device, pNext->ulChannelId);
            if (v) {
                NEXUS_VideoDecoder_P_DataReady_Generic_Prologue_isr(v, pNext);
            }
        }
    }

    /* there are parts of the following call which are required to happen only for the parent decoder in mosaic mode */
    NEXUS_VideoDecoder_P_DataReady_Generic_Epilogue_isr(videoDecoder, pFieldData, xvdChannelStatus->ulPictureDeliveryCount);
    return;
}

void
NEXUS_VideoDecoder_P_RequestStc_isr(void *data, int unused, void *pts_info)
{
    BERR_Code rc;
    BAVC_PTSInfo pts;
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;

    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BTRC_TRACE(ChnChange_DecodeFirstVideo, STOP);

    /* if force vsync, no first vPTS passed will occur */
    if (!NEXUS_GetEnv("force_vsync"))
    {
        BTRC_TRACE(ChnChange_DecodeFirstvPTSPassed, START);
    }

    pts.ui32CurrentPTS = *(uint32_t*)pts_info;
    pts.ePTSType = BAVC_PTSType_eCoded;
    BDBG_MSG(("video:%d Request STC %#x", (unsigned)videoDecoder->mfdIndex, pts.ui32CurrentPTS));

    if (videoDecoder->trickState.tsmEnabled == NEXUS_TsmMode_eSimulated) {
        (void)NEXUS_VideoDecoder_P_SetSimulatedStc_isr(videoDecoder, pts.ui32CurrentPTS);
    }
    else {
        if (videoDecoder->startSettings.stcChannel && videoDecoder->stc.connector && videoDecoder->started) {
           rc = NEXUS_StcChannel_RequestStc_isr(videoDecoder->stc.connector, &pts);
           if (rc) {rc=BERR_TRACE(rc);} /* keep going */
        }
    }
    if (videoDecoder->startPts.waitForStart && videoDecoder->startSettings.pauseAtStartPts) {
        rc = BXVD_PVR_EnablePause_isr(videoDecoder->dec, true);
        if (rc) {rc=BERR_TRACE(rc);} /* keep going */
        videoDecoder->startPts.waitForStart = false;
    }

#if NEXUS_HAS_ASTM
    if (videoDecoder->astm.settings.enableAstm && videoDecoder->astm.settings.enableTsm)
    {
        videoDecoder->astm.status.pts = pts.ui32CurrentPTS;

        if (videoDecoder->astm.settings.firstPts_isr)
        {
            videoDecoder->astm.settings.firstPts_isr(videoDecoder->astm.settings.callbackContext, 0);
        }
    }
#endif

    NEXUS_IsrCallback_Fire_isr(videoDecoder->firstPtsCallback);
    NEXUS_IsrCallback_Fire_isr(videoDecoder->playback.firstPtsCallback);
}

void
NEXUS_VideoDecoder_P_PtsError_isr(void *data, int unused, void *pts_info)
{
    BERR_Code rc;
    BAVC_PTSInfo pts;
    BXVD_PTSInfo * xvdPts = pts_info;
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;

    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    pts.ui32CurrentPTS = xvdPts->ui32RunningPTS;
    pts.uiDecodedFrameCount = xvdPts->uiPicturesDecodedCount;
    pts.uiDroppedFrameCount = xvdPts->uiDisplayManagerDroppedCount;
    pts.ePTSType = xvdPts->ePTSType;
    BDBG_MSG(("video:%d PTS error %#x", (unsigned)videoDecoder->mfdIndex, pts.ui32CurrentPTS));

    if (videoDecoder->trickState.tsmEnabled == NEXUS_TsmMode_eSimulated) {
        (void)NEXUS_VideoDecoder_P_SetSimulatedStc_isr(videoDecoder, pts.ui32CurrentPTS);
    }
    else {
        if (videoDecoder->trickState.stcTrickEnabled && videoDecoder->startSettings.stcChannel) {
            uint32_t stc;
            /* in STC trick mode, PTS might lag the STC because of decoder drop algorithm. don't reset STC in this case. */
            NEXUS_StcChannel_GetStc_isr(videoDecoder->startSettings.stcChannel, &stc);
            if (stc > pts.ui32CurrentPTS &&
                (NEXUS_IS_DSS_MODE(videoDecoder->transportType) ? (stc - pts.ui32CurrentPTS < 27000000 * 8) : (stc - pts.ui32CurrentPTS < 45000 * 8)) ) {
                return;
            }
        }

        if (videoDecoder->startSettings.stcChannel && videoDecoder->stc.connector && videoDecoder->started) {
            rc = NEXUS_StcChannel_PtsError_isr(videoDecoder->stc.connector, &pts);
            if (rc) {rc=BERR_TRACE(rc);} /* keep going */
        }
    }

#if NEXUS_HAS_ASTM
    if (videoDecoder->astm.settings.enableAstm && videoDecoder->astm.settings.enableTsm)
    {
        videoDecoder->astm.status.pts = pts.ui32CurrentPTS;

        if (videoDecoder->astm.settings.tsmFail_isr)
        {
            videoDecoder->astm.settings.tsmFail_isr(videoDecoder->astm.settings.callbackContext, 0);
        }
    }
#endif

    videoDecoder->pts_error_cnt++;

    NEXUS_IsrCallback_Fire_isr(videoDecoder->ptsErrorCallback);
}

void
NEXUS_VideoDecoder_P_TsmPass_isr(void *data, int unused, void *pts_info)
{
    BAVC_PTSInfo pts;
    BXVD_PTSInfo * xvdPts = pts_info;
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;

    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    pts.ui32CurrentPTS = xvdPts->ui32RunningPTS;
    pts.uiDecodedFrameCount = xvdPts->uiPicturesDecodedCount;
    pts.uiDroppedFrameCount = xvdPts->uiDisplayManagerDroppedCount;
    pts.ePTSType = xvdPts->ePTSType;
    BDBG_MSG(("video:%d TSM pass %#x", (unsigned)videoDecoder->mfdIndex, pts.ui32CurrentPTS));

#if NEXUS_HAS_ASTM
    if (videoDecoder->astm.settings.enableAstm && !videoDecoder->astm.settings.enableTsm)
    {
        videoDecoder->astm.status.pts = pts.ui32CurrentPTS;

        if (videoDecoder->astm.settings.tsmPass_isr)
        {
            videoDecoder->astm.settings.tsmPass_isr(videoDecoder->astm.settings.callbackContext, 0);
        }
    }
#endif
}

void
NEXUS_VideoDecoder_P_PtsStcOffset_isr(void *data, int unused, void *pOffset)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;

    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    videoDecoder->ptsStcDifference = (int32_t)*(uint32_t*)pOffset; /* in PTS units */
#if NEXUS_HAS_SYNC_CHANNEL
    videoDecoder->sync.status.delay = videoDecoder->ptsStcDifference;

    /* PR:48453 bandrews - convert to 45 KHz units for sync channel */
    switch (videoDecoder->transportType)
    {
        case NEXUS_TransportType_eDssEs:
        case NEXUS_TransportType_eDssPes:
            videoDecoder->sync.status.delay /= 600;
            break;
        default:
            break;
    }

    videoDecoder->sync.status.delayValid = true;

    if (videoDecoder->sync.settings.delayCallback_isr)
    {
        (*videoDecoder->sync.settings.delayCallback_isr)(videoDecoder->sync.settings.callbackContext, videoDecoder->sync.status.delay);
    }
#endif
}

void
NEXUS_VideoDecoder_P_DecodeError_isr(void *data, int unused, void *unused2)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;
    BSTD_UNUSED(unused);
    BSTD_UNUSED(unused2);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_IsrCallback_Fire_isr(videoDecoder->decodeErrorCallback);
}

void
NEXUS_VideoDecoder_P_FnrtChunkDone_isr(void *data, int unused, void *unused2)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;
    BSTD_UNUSED(unused);
    BSTD_UNUSED(unused2);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_IsrCallback_Fire_isr(videoDecoder->fnrtChunkDoneCallback);
}

void
NEXUS_VideoDecoder_P_EndOfGOP_isr(void *data, int unused, void *param)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;
    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    videoDecoder->dqt.status = *(const BXVD_DQTStatus *)param;
    videoDecoder->dqt.set = true;
}

void
NEXUS_VideoDecoder_P_PictureParams_isr(void *data, int unused, void *info_)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;
    BXVD_PictureParameterInfo *info = (BXVD_PictureParameterInfo *)info_;
    NEXUS_VideoDecoderStreamInformation *streamInfo = &g_NEXUS_videoDecoderIsrStorage.streamInfo;
    BXVD_ChannelStatus status;
    BERR_Code rc;
    BAVC_PTSInfo apts;
    BXVD_PTSInfo xpts;

    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    videoDecoder->pictureParameterInfo.pictureCoding = info->uiPictureCodingType;

    BKNI_Memset(streamInfo, 0, sizeof(*streamInfo));
    streamInfo->valid = true;
    streamInfo->sourceHorizontalSize = info->ulSourceHorizontalSize;
    streamInfo->sourceVerticalSize = info->ulSourceVerticalSize;
    if (streamInfo->sourceHorizontalSize > g_NEXUS_VideoDecoderModuleStatistics.maxDecodedWidth) {
        g_NEXUS_VideoDecoderModuleStatistics.maxDecodedWidth = streamInfo->sourceHorizontalSize;
    }
    if (streamInfo->sourceVerticalSize > g_NEXUS_VideoDecoderModuleStatistics.maxDecodedHeight) {
        g_NEXUS_VideoDecoderModuleStatistics.maxDecodedHeight = streamInfo->sourceVerticalSize;
    }
    streamInfo->codedSourceHorizontalSize = info->uiCodedSourceWidth;
    streamInfo->codedSourceVerticalSize = info->uiCodedSourceHeight;
    streamInfo->displayHorizontalSize = info->ulDisplayHorizontalSize;
    streamInfo->displayVerticalSize = info->ulDisplayVerticalSize;
    streamInfo->aspectRatio = NEXUS_P_AspectRatio_FromMagnum_isrsafe(info->eAspectRatio);
    streamInfo->sampleAspectRatioX = info->uiSampleAspectRatioX;
    streamInfo->sampleAspectRatioY = info->uiSampleAspectRatioY;
    streamInfo->colorDepth = info->eBitDepth;
    streamInfo->protocolLevel = (NEXUS_VideoProtocolLevel)info->uiLevel;
    streamInfo->protocolProfile = (NEXUS_VideoProtocolProfile)info->uiProfile;

    /* pick 4 values to help assure Nexus and Magnum enums stay in sync */
    BDBG_CASSERT(NEXUS_MatrixCoefficients_eSmpte_240M == (NEXUS_MatrixCoefficients)BAVC_MatrixCoefficients_eSmpte_240M);
    BDBG_CASSERT(NEXUS_TransferCharacteristics_eIec_61966_2_4 == (NEXUS_TransferCharacteristics)BAVC_TransferCharacteristics_eIec_61966_2_4);
    BDBG_CASSERT(NEXUS_ColorPrimaries_eGenericFilm == (NEXUS_ColorPrimaries)BAVC_ColorPrimaries_eGenericFilm);
    BDBG_CASSERT(NEXUS_VideoFrameRate_e60 == (NEXUS_VideoFrameRate)BAVC_FrameRateCode_e60);

    /* These enums are straight mappings from Magnum */
    streamInfo->frameRate = info->eFrameRateCode;
    streamInfo->colorPrimaries = NEXUS_P_ColorPrimaries_FromMagnum_isrsafe(info->eColorPrimaries);
    streamInfo->transferCharacteristics = NEXUS_P_TransferCharacteristics_FromMagnum_isrsafe(info->eTransferCharacteristics);
    streamInfo->matrixCoefficients = NEXUS_P_MatrixCoefficients_FromMagnum_isrsafe(info->eMatrixCoefficients);

    /* dynrng stuff */
    /* grab preferred xfer chars */
    streamInfo->preferredTransferCharacteristics = NEXUS_P_TransferCharacteristics_FromMagnum_isrsafe(info->ePreferredTransferCharacteristics);
    /* convert xfer chars + preferred xfer chars to EOTF */
    streamInfo->eotf = NEXUS_P_TransferCharacteristicsToEotf_isrsafe(streamInfo->transferCharacteristics, streamInfo->preferredTransferCharacteristics);
    /* convert CLL */
    NEXUS_P_ContentLightLevel_FromMagnum_isrsafe(&streamInfo->contentLightLevel, info->ulMaxContentLight, info->ulAvgContentLight);
    /* convert MDCV */
    NEXUS_P_MasteringDisplayColorVolume_FromMagnum_isrsafe(&streamInfo->masteringDisplayColorVolume,
        info->stDisplayPrimaries, &info->stWhitePoint, info->ulMaxDispMasteringLuma, info->ulMinDispMasteringLuma);

    /* other stuff */
    streamInfo->streamProgressive = info->bStreamProgressive; /* don't use the bStreamProgressive_7411 flavor. it could lock onto a false value. */
    streamInfo->frameProgressive = info->bFrameProgressive;
    streamInfo->horizontalPanScan = info->i32_HorizontalPanScan;
    streamInfo->verticalPanScan = info->i32_VerticalPanScan;
    streamInfo->lowDelayFlag = info->uiLowDelayFlag;
    streamInfo->fixedFrameRateFlag = info->uiFixedFrameRateFlag;
    if (info->bValidAfd && info->uiAfd != videoDecoder->userdata.status.afdValue) {
        videoDecoder->userdata.status.afdValue = info->uiAfd;
        NEXUS_IsrCallback_Fire_isr(videoDecoder->afdChangedCallback);
    }

    if (BKNI_Memcmp(streamInfo, &videoDecoder->streamInfo, sizeof(*streamInfo))) {
        videoDecoder->streamInfo = *streamInfo;
        NEXUS_IsrCallback_Fire_isr(videoDecoder->streamChangedCallback);
        NEXUS_IsrCallback_Fire_isr(videoDecoder->private.streamChangedCallback);

#if NEXUS_HAS_SYNC_CHANNEL
        /* notify sync., but only if things have changed */
        if (videoDecoder->sync.status.height != streamInfo->sourceVerticalSize
            ||
            videoDecoder->sync.status.interlaced != !streamInfo->streamProgressive
            ||
            videoDecoder->sync.status.frameRate != info->eFrameRateCode)
        {
            videoDecoder->sync.status.height = streamInfo->sourceVerticalSize;
            videoDecoder->sync.status.interlaced = !streamInfo->streamProgressive;
            videoDecoder->sync.status.frameRate = info->eFrameRateCode;
            if (videoDecoder->sync.settings.formatCallback_isr) {
                (*videoDecoder->sync.settings.formatCallback_isr)(videoDecoder->sync.settings.callbackContext, 0);
            }
        }
#endif
    }

    /* 20100519 bandrews - to fix SW7325-706 */
    BXVD_GetChannelStatus_isr(videoDecoder->dec, &status);

    if (status.uiDecoderInputOverflow > videoDecoder->overflowCount)
    {
        videoDecoder->overflowCount = status.uiDecoderInputOverflow;

        /* will this pts be valid after overflow? prob not, but in live it
        doesn't matter and pb shouldn't overflow */
        BXVD_GetPTS_isr(videoDecoder->dec, &xpts);

        apts.ePTSType = xpts.ePTSType;
        apts.ui32CurrentPTS = xpts.ui32RunningPTS;

        BXVD_GetDecodeSettings_isrsafe(videoDecoder->dec,&g_NEXUS_videoDecoderIsrStorage.decodeSettings);
        if (videoDecoder->startSettings.stcChannel && !g_NEXUS_videoDecoderIsrStorage.decodeSettings.bPlayback && videoDecoder->stc.connector) {
            BDBG_WRN(("video:%d Overflow -> Request STC %#x", (unsigned)videoDecoder->mfdIndex, apts.ui32CurrentPTS));
            rc = NEXUS_StcChannel_RequestStc_isr(videoDecoder->stc.connector, &apts);
            if (rc) {rc=BERR_TRACE(rc);} /* keep going */
        }

        /* notify app that this has occurred */
        if (videoDecoder->settings.stateChanged.enabled.fifoOverflow)
        {
            NEXUS_IsrCallback_Fire_isr(videoDecoder->stateChangedCallback);
        }
    }
}

void
NEXUS_VideoDecoder_P_FirstPtsReady_isr(void *data, int unused, void *pts_info)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;

    BSTD_UNUSED(unused);
    BSTD_UNUSED(pts_info);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    videoDecoder->firstPtsReady = true;
}

void
NEXUS_VideoDecoder_P_FirstPtsPassed_isr(void *data, int unused, void *pts_info)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;

    BSTD_UNUSED(unused);
    BSTD_UNUSED(pts_info);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    BTRC_TRACE(ChnChange_DecodeFirstvPTSPassed, STOP);

    /* if sync_disabled, or sync already unmuted, then no additional sync mute
    time will occur */
    if (!NEXUS_GetEnv("sync_disabled")
#if NEXUS_HAS_SYNC_CHANNEL
    && videoDecoder->sync.settings.mute
#endif
    )
    {
        BTRC_TRACE(ChnChange_SyncUnmuteVideo, START);
    }

    videoDecoder->firstPtsPassed = true;
    NEXUS_IsrCallback_Fire_isr(videoDecoder->firstPtsPassedCallback);
    NEXUS_IsrCallback_Fire_isr(videoDecoder->playback.firstPtsPassedCallback);
}

static void NEXUS_VideoDecoder_P_3DTVTimeout(void* context)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)context;
    NEXUS_VideoDecoderStatus status;
    videoDecoder->s3DTVStatusTimer = NULL;

    NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
    if (videoDecoder->s3DTVStatusPts==status.pts) { /* if the PTS hasn't moved, then the decoder must have been starved. don't fire the callback */
        NEXUS_VideoDecoder_P_3DTVTimer(videoDecoder);
        return;
    }
    else {
        BKNI_Memset(&videoDecoder->s3DTVStatus.codecData, 0, sizeof(videoDecoder->s3DTVStatus.codecData));
        videoDecoder->s3DTVStatus.format = NEXUS_VideoDecoder3DTVFormat_eNone;

        BDBG_WRN(("3DTV message timeout"));

        if (videoDecoder->settings.customSourceOrientation==false) {
            BXVD_3DSetting xvd3dSetting;
            BXVD_Get3D(videoDecoder->dec, &xvd3dSetting);
            xvd3dSetting.bOverrideOrientation = false;
            xvd3dSetting.eOrientation = BXDM_PictureProvider_Orientation_e2D;
            BXVD_Set3D(videoDecoder->dec, &xvd3dSetting);
        }

        BKNI_EnterCriticalSection();
        NEXUS_IsrCallback_Fire_isr(videoDecoder->s3DTVChangedCallback);
        BKNI_LeaveCriticalSection();
    }
}

void NEXUS_VideoDecoder_P_3DTVTimer(void* context)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)context;
    NEXUS_VideoDecoderStatus status;
    if (videoDecoder->s3DTVStatusTimer) {
        NEXUS_CancelTimer(videoDecoder->s3DTVStatusTimer);
        videoDecoder->s3DTVStatusTimer = NULL;
    }

    if (videoDecoder->extendedSettings.s3DTVStatusTimeout) { /* Timeout==0 means no timer */
        videoDecoder->s3DTVStatusTimer = NEXUS_ScheduleTimer(videoDecoder->extendedSettings.s3DTVStatusTimeout,
            NEXUS_VideoDecoder_P_3DTVTimeout, videoDecoder);
    }

    NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
    videoDecoder->s3DTVStatusPts = status.pts;
}


void
NEXUS_VideoDecoder_P_Jp3dSignal_isr(NEXUS_VideoDecoderHandle videoDecoder, uint16_t type)
{
    BKNI_SetEvent(videoDecoder->s3DTVStatusEvent); /* this calls NEXUS_VideoDecoder_P_3DTVTimer() */
    /*BDBG_WRN(("JP3D message"));*/

    /* has the type changed? */
    if (type != videoDecoder->s3DTVStatus.codecData.mpeg2.jp3d.stereoVideoFormatSignalingType) {
        BXVD_Orientation xvdFormat = BXVD_Orientation_e2D;
        BXVD_3DSetting xvdSetting;
        /* videoDecoder->s3DTVStatus is cleared in NEXUS_VideoDecoder_P_Start_Generic_Body() */
        videoDecoder->s3DTVStatus.codec = NEXUS_VideoCodec_eMpeg2;

        switch (type) {
            case 0x3:
                videoDecoder->s3DTVStatus.format = NEXUS_VideoDecoder3DTVFormat_eSideBySideHalf;
                xvdFormat = BXVD_Orientation_eLeftRight;
                break;
            case 0x4:
                videoDecoder->s3DTVStatus.format = NEXUS_VideoDecoder3DTVFormat_eTopAndBottomHalf;
                xvdFormat = BXVD_Orientation_eOverUnder;
                break;
            case 0x8: /* 2D */
                videoDecoder->s3DTVStatus.format = NEXUS_VideoDecoder3DTVFormat_eNone;
                break;
        }
        videoDecoder->s3DTVStatus.codecData.mpeg2.jp3d.stereoVideoFormatSignalingType = type;
        videoDecoder->s3DTVStatus.codecData.mpeg2.valid = true;

        if (videoDecoder->settings.customSourceOrientation==false) { /* if app specifies custom orientation, then honor that and don't override */
            /* for mpeg2, the 3d info parsing is done outside of XVD, so we need to inform XVD what the orientation is */
            BXVD_Get3D_isr(videoDecoder->dec, &xvdSetting);
            if (xvdSetting.eOrientation!=xvdFormat) {
                NEXUS_Error rc;
                xvdSetting.bOverrideOrientation = true;
                xvdSetting.eOrientation = xvdFormat;
                xvdSetting.bSetNextPointer = false;
                rc = BXVD_Set3D_isr(videoDecoder->dec, &xvdSetting);
                if (rc!=BERR_SUCCESS) {
                    rc = BERR_TRACE(rc);
                }
                videoDecoder->settings.sourceOrientation = (NEXUS_VideoDecoderSourceOrientation)xvdFormat;
            }
        }

        NEXUS_IsrCallback_Fire_isr(videoDecoder->s3DTVChangedCallback);
    }
}

void
NEXUS_VideoDecoder_P_PictureExtensionData_isr(void *data, int unused, void *pData)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;
    BXDM_Picture_ExtensionInfo *extInfo = (BXDM_Picture_ExtensionInfo *)pData;
    BXDM_Picture_Extension_SEIFramePacking *seiMessage = NULL;
    unsigned newMsg = 0;
    unsigned i;

    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_CASSERT(sizeof(videoDecoder->s3DTVStatus.codecData.avc.data) == sizeof(*extInfo->astExtensionData[0].data.stSEIFramePacking.pstSeiData));

    if (!videoDecoder->extendedSettings.s3DTVStatusEnabled) {
        return;
    }

    for (i=0; i<extInfo->uiCount; i++) {
        if (extInfo->astExtensionData[i].eType == BXDM_Picture_ExtensionType_eSEIMsg_FramePacking) {
            seiMessage = extInfo->astExtensionData[i].data.stSEIFramePacking.pstSeiData;
            /* SEI messages are valid for a duration defined by the repetition period.
               newMsg != 0 indicates that either the message has changed or the previous duration expired */
            newMsg = extInfo->astExtensionData[i].data.stSEIFramePacking.uiFlags & BXDM_PICTURE_SEIMSG_FRAMEPACK_NEW_MESSAGE_FLAG;
            break;
        }
    }

    if (seiMessage) {
        BKNI_SetEvent(videoDecoder->s3DTVStatusEvent); /* this calls NEXUS_VideoDecoder_P_3DTVTimer() */

#if 0
        BDBG_MSG(("new %u, flags %u, id %u, type %u, ciType %u, pos %u:%u:%u:%u, res %u, rep %u",
            newMsg, seiMessage->uiFlags, seiMessage->uiFramePackingArrangementId, seiMessage->uiFramePackingArrangementType, seiMessage->uiContentInterpretationType,
            seiMessage->uiFrame0GridPositionX, seiMessage->uiFrame0GridPositionY, seiMessage->uiFrame1GridPositionX, seiMessage->uiFrame1GridPositionY,
            seiMessage->uiFramePackingArrangementReservedByte, seiMessage->uiFramePackingArrangementRepetitionPeriod));
#endif

        if (newMsg) {
            /* don't use BKNI_Memcmp */
            if (videoDecoder->s3DTVStatus.codecData.avc.data.flags == seiMessage->uiFlags &&
                videoDecoder->s3DTVStatus.codecData.avc.data.framePackingArrangementId == seiMessage->uiFramePackingArrangementId &&
                videoDecoder->s3DTVStatus.codecData.avc.data.framePackingArrangementType == seiMessage->uiFramePackingArrangementType &&
                videoDecoder->s3DTVStatus.codecData.avc.data.contentInterpretationType == seiMessage->uiContentInterpretationType &&
                videoDecoder->s3DTVStatus.codecData.avc.data.frame0GridPositionX == seiMessage->uiFrame0GridPositionX &&
                videoDecoder->s3DTVStatus.codecData.avc.data.frame0GridPositionY == seiMessage->uiFrame0GridPositionY &&
                videoDecoder->s3DTVStatus.codecData.avc.data.frame1GridPositionX == seiMessage->uiFrame1GridPositionX &&
                videoDecoder->s3DTVStatus.codecData.avc.data.frame1GridPositionY == seiMessage->uiFrame1GridPositionY &&
                videoDecoder->s3DTVStatus.codecData.avc.data.framePackingArrangementReservedByte == seiMessage->uiFramePackingArrangementReservedByte &&
                videoDecoder->s3DTVStatus.codecData.avc.data.framePackingArrangementRepetitionPeriod == seiMessage->uiFramePackingArrangementRepetitionPeriod)
            {
                return; /* same message */
            }
        }
        else {
            return;
        }

        /* videoDecoder->s3DTVStatus is cleared in NEXUS_VideoDecoder_P_Start_Generic_Body() */
        videoDecoder->s3DTVStatus.codec = NEXUS_VideoCodec_eH264;
        videoDecoder->s3DTVStatus.format = NEXUS_VideoDecoder3DTVFormat_eNone;
        switch (seiMessage->uiFramePackingArrangementType) {
            case 3:
                videoDecoder->s3DTVStatus.format = NEXUS_VideoDecoder3DTVFormat_eSideBySideHalf;
                break;
            case 4:
                videoDecoder->s3DTVStatus.format = NEXUS_VideoDecoder3DTVFormat_eTopAndBottomHalf;
                break;
        }
        BKNI_Memcpy(&videoDecoder->s3DTVStatus.codecData.avc.data, seiMessage, sizeof(*seiMessage));
        videoDecoder->s3DTVStatus.codecData.avc.valid = true;
        NEXUS_IsrCallback_Fire_isr(videoDecoder->s3DTVChangedCallback);
    }
}

BERR_Code NEXUS_VideoDecoder_P_GetPtsCallback_isr(void *pContext, BAVC_PTSInfo *pPTSInfo)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)pContext;
    BXVD_PTSInfo ptsInfo;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BXVD_GetPTS_isr(videoDecoder->dec, &ptsInfo);

    /* map data structures */
    pPTSInfo->ui32CurrentPTS = ptsInfo.ui32RunningPTS;
    pPTSInfo->ePTSType = ptsInfo.ePTSType;
    return 0;
}

BERR_Code NEXUS_VideoDecoder_P_GetCdbLevelCallback_isr(void *pContext, unsigned *pCdbLevel)
{
    unsigned depth, size;
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)pContext;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_Rave_GetCdbBufferInfo_isr(videoDecoder->rave, &depth, &size);
    *pCdbLevel = depth;
    return 0;
}

BERR_Code NEXUS_VideoDecoder_P_StcValidCallback_isr(void *pContext)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)pContext;
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    return BXVD_SetSTCInvalidFlag_isr(videoDecoder->dec, false);
}

BERR_Code NEXUS_VideoDecoder_P_SetPcrOffset_isr(void *pContext, uint32_t pcrOffset)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)pContext;
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    return BXVD_SetSwPcrOffset_isr(videoDecoder->dec, pcrOffset);
}

BERR_Code NEXUS_VideoDecoder_P_GetPcrOffset_isr(void *pContext, uint32_t *pPcrOffset)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)pContext;
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    return BXVD_GetSwPcrOffset_isr(videoDecoder->dec, pPcrOffset);
}

/***********
* Private API
***********/

void NEXUS_VideoDecoder_GetDefaultDisplayConnection_priv(NEXUS_VideoDecoderDisplayConnection *connection)
{
    BKNI_Memset(connection, 0, sizeof(*connection));
}

NEXUS_Error NEXUS_VideoDecoder_SetDefaultDisplayConnection_priv(const NEXUS_VideoDecoderDisplayConnection *connection)
{
    unsigned i, j;

    /* NEXUS_Display_DriveVideoDecoder connection. Apply to all AVD's that don't have BVN paths. */
    for (i=0;i<NEXUS_MAX_XVD_DEVICES;i++) {
        struct NEXUS_VideoDecoderDevice *device = &g_NEXUS_videoDecoderXvdDevices[i];
        device->defaultConnection = *connection;
        if (device->xvd && !device->defaultConnection.top.intId) {
            /* if clearing defaultConnection, must stop video-as-graphics decoders */
            for (j=0;j<NEXUS_NUM_XVD_CHANNELS;j++) {
                NEXUS_VideoDecoderHandle videoDecoder = device->channel[j];
                if (videoDecoder && videoDecoder->videoAsGraphics) {
                    NEXUS_VideoDecoder_SetPowerState(videoDecoder, false);
                }
            }
        }
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_VideoDecoder_P_SetXvdDisplayInterrupt(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderDisplayConnection *connection, BXVD_DisplayInterrupt interrupt)
{
    BERR_Code rc;

    BXVD_DeviceVdcInterruptSettings VDCDevIntrSettings;

    BXVD_GetVdcDeviceInterruptDefaultSettings(videoDecoder->device->xvd, &VDCDevIntrSettings);

    VDCDevIntrSettings.VDCIntId_Topfield = connection->top.intId;
    VDCDevIntrSettings.VDCIntId_Botfield = connection->bottom.intId;
    if (videoDecoder->displayConnection.callbackContext) {
        /* callbackContext is set for routing through BVN */
        VDCDevIntrSettings.VDCIntId_Frame = connection->frame.intId;
        VDCDevIntrSettings.uiFlags &= ~BXVD_DeviceVdcIntrSettingsFlags_UseFieldAsFrame;
    }
    else {
        /* NULL callbackContext means "video as graphics" */
        VDCDevIntrSettings.VDCIntId_Frame = 0;
        VDCDevIntrSettings.uiFlags |= BXVD_DeviceVdcIntrSettingsFlags_UseFieldAsFrame;
    }
    if (videoDecoder->linkedDevice && connection->dataReadyCallback_isr) {
        VDCDevIntrSettings.uiFlags |= BXVD_DeviceVdcIntrSettingsFlags_Linked;
        VDCDevIntrSettings.hAppXdmDih = videoDecoder->linkedDevice->hXdmDih[interrupt];
    }
    else {
        VDCDevIntrSettings.hAppXdmDih = connection->dataReadyCallback_isr?videoDecoder->device->hXdmDih[interrupt]:NULL;
    }
    VDCDevIntrSettings.eDisplayInterrupt = interrupt;
    BDBG_MSG(("connect AVD%d XDM%d interrupts: %#x, %#x, %#x", videoDecoder->device->index, videoDecoder->xdmIndex,
        connection->top.intId, connection->bottom.intId, connection->frame.intId));

    rc = BXVD_RegisterVdcDeviceInterrupt(videoDecoder->device->xvd, &VDCDevIntrSettings);
    if (rc) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

/**
These functions are called by Display when connecting a NEXUS_VideoInput to a NEXUS_VideoWindow.
Or by Start for video-as-graphics.
**/
NEXUS_Error NEXUS_VideoDecoder_SetDisplayConnection_priv_Avd(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderDisplayConnection *connection)
{
    BERR_Code rc;
    unsigned i;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    if (!connection) {
        videoDecoder->displayConnection = videoDecoder->device->defaultConnection;
        videoDecoder->videoAsGraphics = true;
        connection = &videoDecoder->displayConnection;
    }
    else {
	    videoDecoder->displayConnection = *connection;
        videoDecoder->videoAsGraphics = false;
    }

    if (videoDecoder->dec && !connection->dataReadyCallback_isr) {
        if (!videoDecoder->settings.manualPowerState) {
            rc = NEXUS_VideoDecoder_SetPowerState(videoDecoder, false);
            if (rc) return BERR_TRACE(rc);
        }
    }
    else if (connection->dataReadyCallback_isr) {
        /* even in manualPowerState, it's ok to automatically power up here. */
        rc = NEXUS_VideoDecoder_SetPowerState(videoDecoder, true);
        if (rc) return BERR_TRACE(rc);
    }

    /* If any of the decoder channels is already running or
       any of the channels are still conencted to a window
       then do not unregister VDC device interrupt */
    if(!connection->dataReadyCallback_isr) {
        for(i=0; i<NEXUS_NUM_XVD_CHANNELS; i++){
            NEXUS_VideoDecoderHandle v = videoDecoder->device->channel[i];
            if (!v) continue;
            if(v->started ||
               v->displayConnection.top.intId ||
               v->displayConnection.bottom.intId ||
               v->displayConnection.frame.intId)
            return 0;
        }
    }

    rc = NEXUS_VideoDecoder_P_SetXvdDisplayInterrupt(videoDecoder, connection, videoDecoder->xdmIndex + BXVD_DisplayInterrupt_eZero);
    if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }

    NEXUS_VideoDecoder_P_SetUserdata(videoDecoder);

    return 0;
}

void NEXUS_VideoDecoder_GetDisplayConnection_priv_Avd(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderDisplayConnection *pConnection)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_ASSERT_MODULE();
    *pConnection = videoDecoder->displayConnection;
    if (videoDecoder->linkedDevice) {
        unsigned i;
        /* find any open channel and use its MFD */
        for (i=0;i<NEXUS_NUM_XVD_CHANNELS;i++) {
            if (videoDecoder->linkedDevice->channel[i]) {
                pConnection->parentIndex = videoDecoder->linkedDevice->channel[i]->mfdIndex;
                break;
            }
        }
        if (i == NEXUS_NUM_XVD_CHANNELS) {
            BDBG_ERR(("invalid linked decoder: at least one channel on master device must be open"));
            pConnection->parentIndex = videoDecoder->mfdIndex; /* return something */
        }
    }
    else {
        pConnection->parentIndex = videoDecoder->mfdIndex;
    }
    pConnection->secureVideo = nexus_p_use_picbuf_type(videoDecoder);
}

unsigned NEXUS_VideoDecoder_GetMosaicIndex_isrsafe(NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    return videoDecoder->mosaicIndex;
}

void NEXUS_VideoDecoder_GetSourceId_priv_Avd(NEXUS_VideoDecoderHandle videoDecoder, BAVC_SourceId *pSource)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_ASSERT_MODULE();
    *pSource = BAVC_SourceId_eMpeg0 + videoDecoder->mfdIndex;
}

/**
This function is only called from the Display to get the Display heap.
If openSettings.heap is NULL, we should return NULL, then Display will get its own default heap.
**/
void NEXUS_VideoDecoder_GetHeap_priv_Common( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_HeapHandle *pHeap )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_ASSERT_MODULE();
    *pHeap = NULL; /* Don't use AVD picture heap for Display */
    return;
}

#if NEXUS_HAS_SYNC_CHANNEL
void NEXUS_VideoDecoder_GetSyncSettings_priv_Common(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoInputSyncSettings *pSyncSettings)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_ASSERT_MODULE();
    *pSyncSettings = videoDecoder->sync.settings;
}

NEXUS_Error NEXUS_VideoDecoder_SetSyncSettings_priv_Avd(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoInputSyncSettings *pSyncSettings)
{
    BERR_Code rc;
    unsigned delayCallbackThreshold;
    bool oldMuteStatus;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_ASSERT_MODULE();

    oldMuteStatus = videoDecoder->sync.mute;
    videoDecoder->sync.settings = *pSyncSettings;

    delayCallbackThreshold = videoDecoder->sync.settings.delayCallbackThreshold;

    /* PR:48453 bandrews - convert 45 KHz to appropriate decoder units */
    switch (videoDecoder->transportType)
    {
        case NEXUS_TransportType_eDssEs:
        case NEXUS_TransportType_eDssPes:
            delayCallbackThreshold *= 600;
            break;
        default:
            break;
    }
    if (videoDecoder->dec) {
        rc = BXVD_SetPtsStcDiffThreshold(videoDecoder->dec, delayCallbackThreshold);
        if (rc) {return BERR_TRACE(rc);}
    }

    videoDecoder->sync.startDelay = videoDecoder->sync.settings.startDelay;

    if (!videoDecoder->started)
    {
        /* if sync sets mute before video is started, it is to modify start behavior */
        videoDecoder->sync.startMuted = videoDecoder->sync.settings.mute;
        videoDecoder->sync.delay = videoDecoder->sync.settings.delay;
    }
    else
    {
        videoDecoder->sync.delay = videoDecoder->sync.settings.delay;
        rc = NEXUS_VideoDecoder_P_SetPtsOffset(videoDecoder);
        if (rc) {return BERR_TRACE(rc);}
    }

    videoDecoder->sync.mute = videoDecoder->sync.settings.mute;

    if ((oldMuteStatus) && (!videoDecoder->sync.mute))
    {
        /* apply unmute edge */

        rc = NEXUS_VideoDecoder_P_SetMute(videoDecoder);
        if (rc) {return BERR_TRACE(rc);}

#if B_HAS_TRC
        /* don't count sync unmute if it occurs before first vpts passed */
        if (videoDecoder->firstPtsPassed)
        {
            BTRC_TRACE(ChnChange_SyncUnmuteVideo, STOP);
        }
#endif
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_VideoDecoder_GetSyncStatus_Common_isr(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoInputSyncStatus *pSyncStatus)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    *pSyncStatus = videoDecoder->sync.status;
    return 0;
}
#endif /* NEXUS_HAS_SYNC_CHANNEL */

NEXUS_Error NEXUS_VideoDecoder_P_SetPtsOffset(NEXUS_VideoDecoderHandle videoDecoder)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BKNI_EnterCriticalSection();
    rc = NEXUS_VideoDecoder_P_SetPtsOffset_isr(videoDecoder);
    BKNI_LeaveCriticalSection();

    return rc;
}

NEXUS_Error NEXUS_VideoDecoder_P_SetPtsOffset_isr(NEXUS_VideoDecoderHandle videoDecoder)
{
    int syncPtsOffset;
    int astmPtsOffset;
    int lowLatencyPtsOffset;

    syncPtsOffset = 0;
    astmPtsOffset = 0;
    lowLatencyPtsOffset = 0;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (!videoDecoder->dec) {
        return 0;
    }

    /* PR:48453 bandrews - convert 45 KHz to appropriate decoder units */
#if NEXUS_HAS_SYNC_CHANNEL
    syncPtsOffset = videoDecoder->sync.delay;
#endif
#if NEXUS_HAS_ASTM
    astmPtsOffset = videoDecoder->astm.settings.ptsOffset;
#endif
    if (videoDecoder->extendedSettings.lowLatencySettings.mode != NEXUS_VideoDecoderLowLatencyMode_eOff)
    {
        lowLatencyPtsOffset = videoDecoder->lowLatency.ptsOffset;
    }
    switch (videoDecoder->transportType)
    {
        case NEXUS_TransportType_eDssEs:
        case NEXUS_TransportType_eDssPes:
            syncPtsOffset *= 600;
            astmPtsOffset *= 600;
            lowLatencyPtsOffset *= 600;
            break;
        default:
            break;
    }

    return BXVD_SetDisplayOffset_isr(videoDecoder->dec,
        syncPtsOffset
        + astmPtsOffset
        + lowLatencyPtsOffset
        + videoDecoder->settings.ptsOffset
        + (signed)videoDecoder->primerPtsOffset
        + (signed)videoDecoder->additionalPtsOffset);
}

NEXUS_Error NEXUS_VideoDecoder_P_SetFreeze(NEXUS_VideoDecoderHandle videoDecoder)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (!videoDecoder->dec) {
        return 0;
    }

    /* user freeze is highest precedence */
    if (videoDecoder->settings.freeze)
    {
        BDBG_MSG(("Freezing video per user request"));
        rc = BXVD_EnableVideoFreeze(videoDecoder->dec);
        if (rc) return BERR_TRACE(rc);
    }
/* 20100326 bandrews - freeze doesn't do normal TSM in the bg while displaying the frozen frame, so can't use it */
#if 0
    else if (videoDecoder->syncSettings.mute)
    {
        /* only sync freeze if ch chg mode says so */
        switch (videoDecoder->settings.channelChangeMode)
        {
            case NEXUS_VideoDecoder_ChannelChangeMode_eMuteUntilFirstPicture:
            case NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture:
                /* the above two modes are here because the user expects a freeze after the first
                picture is shown anyway (while TSM for the second picture matures).  XVD team says
                if both FREEZE and "first picture preview" are enabled, "first picture preview" will
                take precendence...and then freeze will occur on the first picture */
            case NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock:
                BDBG_MSG(("Freezing video per sync request"));
                rc = BXVD_EnableVideoFreeze(videoDecoder->dec);
                if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
                break;
            default:
                break;
        }
    }
#endif
    else /* unfreeze */
    {
        BDBG_MSG(("Unfreezing video"));
        rc = BXVD_DisableVideoFreeze(videoDecoder->dec);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    }

    return rc;
}

NEXUS_Error NEXUS_VideoDecoder_P_SetMute(NEXUS_VideoDecoderHandle videoDecoder)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (!videoDecoder->dec) {
        return 0;
    }

    /* user mute is highest precedence */
    if (videoDecoder->settings.mute)
    {
        BDBG_MSG(("Muting video per user request"));
        rc = BXVD_EnableMute(videoDecoder->dec, videoDecoder->settings.mute);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    }
#if NEXUS_HAS_SYNC_CHANNEL
    else if (videoDecoder->sync.mute
        /* if preroll is nonzero, we want to unmute now */
        && !(videoDecoder->started && videoDecoder->startSettings.prerollRate))
    {
        NEXUS_VideoDecoderTrickState normal;

        /* unmute if trick mode and sync mute activated, doesn't matter if started or not */
        NEXUS_VideoDecoder_GetNormalPlay(&normal);
        if (BKNI_Memcmp(&normal, &videoDecoder->trickState, sizeof(NEXUS_VideoDecoderTrickState)))
        {
            /* this overrides mute control of sync */
            videoDecoder->sync.mute = false;
            rc = BXVD_EnableMute(videoDecoder->dec, false);
            if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
        }
        else
        {
            /* SW7420-2314: if decoder was already started, don't mute */
            if (!videoDecoder->started)
            {
                /* only sync mute if ch chg mode says so */
                switch (videoDecoder->settings.channelChangeMode)
                {
                    case NEXUS_VideoDecoder_ChannelChangeMode_eMute:
                        BDBG_MSG(("Muting video per sync request"));
                        rc = BXVD_EnableMute(videoDecoder->dec, true);
                        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
                        break;
                    default:
                        break;
                }
            }
        }
    }
#endif
    else /* unmute */
    {
        BDBG_MSG(("Unmuting video"));
        rc = BXVD_EnableMute(videoDecoder->dec, false);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    }

    return rc;
}

NEXUS_Error NEXUS_VideoDecoder_P_SetTsm(NEXUS_VideoDecoderHandle videoDecoder)
{
    BXVD_ClockOverride clockOverride;
    BXVD_DisplayMode displayMode;
    bool tsm;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (!videoDecoder->dec) {
        return 0;
    }

    /* Lots of things can turn off TSM */
    tsm = videoDecoder->trickState.tsmEnabled != NEXUS_TsmMode_eDisabled &&
          !NEXUS_GetEnv("force_vsync") &&
          videoDecoder->transportType != NEXUS_TransportType_eEs;

    /* HW TSM requires STC channel. SW TSM does not. */
    if (videoDecoder->trickState.tsmEnabled == NEXUS_TsmMode_eEnabled) {
        tsm = tsm && videoDecoder->startSettings.stcChannel;
    }

#if NEXUS_HAS_ASTM
    /* ASTM can turn off TSM */
    if (videoDecoder->astm.settings.enableAstm)
    {
        BDBG_MSG(("ASTM is %s TSM for video channel %p", videoDecoder->astm.settings.enableTsm ? "enabling" : "disabling", (void *)videoDecoder));
        tsm = tsm && videoDecoder->astm.settings.enableTsm;
    }
#endif

    if (videoDecoder->startSettings.crcMode == NEXUS_VideoDecoderCrcMode_eMfd) {
        tsm = false;
    }

    (void)BXVD_GetClockOverride(videoDecoder->dec, &clockOverride);
    clockOverride.bLoadSwStc = false;

    if (tsm) {
        if (videoDecoder->trickState.tsmEnabled == NEXUS_TsmMode_eSimulated) {
            int rate;
            unsigned displayFrameRate; /* units of 1/100 second */

            displayMode = BXVD_DisplayMode_eTSMMode;
            clockOverride.bEnableClockOverride = true;

            /* SW STC TSM - compute per-vsync STC delta based on trick rate, source frame rate, TS type */
            displayFrameRate = videoDecoder->displayInformation.refreshRate; /* applied every vsync, so it must be field or frame. */
            rate = NEXUS_IS_DSS_MODE(videoDecoder->transportType) ? 27000000 : 45000;
            rate = rate * 100 / (displayFrameRate?displayFrameRate:3000);
            rate = rate * videoDecoder->trickState.rate / (int)NEXUS_NORMAL_DECODE_RATE;
            clockOverride.iStcDelta = rate;

            /* we only enable simulated STC and set rate here. the stc value will be loaded
            in NEXUS_VideoDecoder_P_SetSimulatedStc_isr in response to requeststc/ptserror isr.
            we also reenable the request interrupt so we can that interrupt as fast as possible. */

            rc = BXVD_EnableInterrupt(videoDecoder->dec, BXVD_Interrupt_eRequestSTC, true);
            if (rc) return BERR_TRACE(rc);
        }
        else {
            /* HW STC TSM */
            displayMode = BXVD_DisplayMode_eTSMMode;
            clockOverride.bEnableClockOverride = false;
        }
    }
    else {
        displayMode = BXVD_DisplayMode_eVSYNCMode;
        clockOverride.bEnableClockOverride = false;
    }

    rc = BXVD_SetVideoDisplayMode(videoDecoder->dec, displayMode);
    if (rc) return BERR_TRACE(rc);

    BDBG_MSG(("BXVD_SetClockOverride %d %d %d %d", displayMode, clockOverride.bEnableClockOverride, videoDecoder->trickState.rate, clockOverride.iStcDelta));
    rc = BXVD_SetClockOverride(videoDecoder->dec, &clockOverride);
    if (rc) return BERR_TRACE(rc);

    videoDecoder->tsm = tsm;

    return 0;

}

NEXUS_Error NEXUS_VideoDecoder_P_SetDiscardThreshold_Avd(NEXUS_VideoDecoderHandle videoDecoder)
{
    NEXUS_Error rc;
    unsigned discardThreshold;
    unsigned units = 45000; /* PTS units for 1 second. XVD always expects it in 45KHz units, even for DSS */

    unsigned veryLateThreshold = videoDecoder->defaultVeryLateThreshold; /* take XVD's default */

    if (videoDecoder->trickState.tsmEnabled == NEXUS_TsmMode_eSimulated) {
        /* expand the normal play threshold by a factor of 'rate' */
        int std_threshold = videoDecoder->settings.discardThreshold ? videoDecoder->settings.discardThreshold : videoDecoder->defaultDiscardThreshold;
        unsigned numerator = videoDecoder->trickState.rate > 0 ? videoDecoder->trickState.rate : -videoDecoder->trickState.rate;
        int target_threshold = units / NEXUS_NORMAL_DECODE_RATE * numerator;
        /* for AVC, the normal play threshold is 10 seconds. this scales up to too high a value.
        so, scale up a fixed 1 second threshold using the trick rate, but only use it if greater than normal play. */
        discardThreshold = (target_threshold > std_threshold) ? target_threshold : std_threshold;
        veryLateThreshold = discardThreshold;
    }
    /* Allow the user to override */
    else if (videoDecoder->settings.discardThreshold) {
        discardThreshold = videoDecoder->settings.discardThreshold;
    }
    /* For host paced trick modes, we need a discard threshold >= the time between I frames. */
    else if (videoDecoder->trickState.tsmEnabled == NEXUS_TsmMode_eEnabled && (videoDecoder->trickState.hostTrickModesEnabled || videoDecoder->trickState.brcmTrickModesEnabled)) {
        discardThreshold = 30 * units;
    }
    else {
        discardThreshold = videoDecoder->defaultDiscardThreshold;
    }

    BDBG_MSG(("set discard threshold %d", discardThreshold));
    /* BXVD_SetDiscardThreshold sets the "early" discard threshold. That is, if a picture arrives in the DM which is earlier than
    this threshold, the DM will drop it. If you set this threshold too large, then the DM could get stuck on PTS discontinuities. */
    rc = BXVD_SetDiscardThreshold(videoDecoder->dec, discardThreshold);
    if (rc) return BERR_TRACE(rc);

    BXVD_SetVeryLateThreshold(videoDecoder->dec, veryLateThreshold);
    if (rc) return BERR_TRACE(rc);

    return 0;
}

NEXUS_Error NEXUS_VideoDecoder_P_SetUserdata(NEXUS_VideoDecoderHandle videoDecoder)
{
    if (!videoDecoder->userdata.handle) {
        return 0;
    }

    BKNI_EnterCriticalSection();
    videoDecoder->userdata.vbiDataCallback_isr = videoDecoder->displayConnection.vbiDataCallback_isr;
    videoDecoder->userdata.vbiVideoInput = &videoDecoder->input;
    BKNI_LeaveCriticalSection();

    BDBG_MSG(("NEXUS_VideoDecoder_P_SetUserdata %p: started=%d userDataEnabled=%d vbiDataCallback_isr=%p", (void *)videoDecoder, videoDecoder->started,
        videoDecoder->settings.userDataEnabled, (void *)(unsigned long)videoDecoder->userdata.vbiDataCallback_isr));

    return BXVD_Userdata_Enable(videoDecoder->userdata.handle,
        videoDecoder->started &&
        (videoDecoder->settings.userDataEnabled || videoDecoder->extendedSettings.s3DTVStatusEnabled ||
         videoDecoder->userdata.vbiDataCallback_isr || videoDecoder->displayConnection.userDataCallback_isr));
}

void
NEXUS_VideoDecoder_P_ApplyDisplayInformation_Common( NEXUS_VideoDecoderHandle videoDecoder)
{
    BERR_Code rc;

    if(videoDecoder->startSettings.stcChannel) {
        bool b1001Slip;
        unsigned vsyncRate;

        NEXUS_StcChannelNonRealtimeSettings stcNonRealtimeSettings;
        NEXUS_VideoDecoder_P_GetVsyncRate_isrsafe(videoDecoder->displayInformation.refreshRate, &vsyncRate, &b1001Slip);
        LOCK_TRANSPORT();
        NEXUS_StcChannel_GetDefaultNonRealtimeSettings_priv( &stcNonRealtimeSettings);
        stcNonRealtimeSettings.externalTrigger = 0;
        if(videoDecoder->startSettings.nonRealTime) {
            stcNonRealtimeSettings.triggerMode = NEXUS_StcChannelTriggerMode_eExternalTrig;
            stcNonRealtimeSettings.externalTrigger = videoDecoder->displayInformation.stgIndex;
            if(videoDecoder->displayInformation.refreshRate) {
                stcNonRealtimeSettings.stcIncrement = ((uint64_t)27000000 * (b1001Slip?1001:1000) / 10) / vsyncRate; /* convert from 1/100Hz to 27MHz ticks */
            } else {
                stcNonRealtimeSettings.stcIncrement = 0;
            }
        } else {
            stcNonRealtimeSettings.triggerMode = NEXUS_StcChannelTriggerMode_eTimebase;
        }
        /* reset NRT STC only when starting */
        rc = NEXUS_StcChannel_SetNonRealtimeConfig_priv(videoDecoder->startSettings.stcChannel, &stcNonRealtimeSettings,
            !videoDecoder->started);
        UNLOCK_TRANSPORT();
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); /* keep going */}
    }
    return;
}

void
NEXUS_VideoDecoder_P_ApplyDisplayInformation( NEXUS_VideoDecoderHandle videoDecoder)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (videoDecoder->dec) {
        uint32_t refreshRate;
        switch(videoDecoder->displayInformation.refreshRate) {
        case 749:
           refreshRate = BXVD_MONITOR_REFRESH_RATE_7_493Hz; break;
        case 750:
           refreshRate = BXVD_MONITOR_REFRESH_RATE_7_5Hz; break;
        case 999:
           refreshRate = BXVD_MONITOR_REFRESH_RATE_9_99Hz; break;
        case 1000:
           refreshRate = BXVD_MONITOR_REFRESH_RATE_10Hz; break;
        case 1198:
        case 1199: /* real value is 11.988 */
           refreshRate = BXVD_MONITOR_REFRESH_RATE_11_988Hz; break;
        case 1200:
           refreshRate = BXVD_MONITOR_REFRESH_RATE_12Hz; break;
        case 1250:
           refreshRate = BXVD_MONITOR_REFRESH_RATE_12_5Hz; break;
        case 1498:
        case 1499:
           refreshRate = BXVD_MONITOR_REFRESH_RATE_14_985Hz; break;
        case 1500:
           refreshRate = BXVD_MONITOR_REFRESH_RATE_15Hz; break;
        case 1998:
           refreshRate = BXVD_MONITOR_REFRESH_RATE_19_98Hz; break;
        case 2000:
           refreshRate = BXVD_MONITOR_REFRESH_RATE_20Hz; break;
        case 2398:
        case 2397: /* real value is 23.976 */
            refreshRate = BXVD_MONITOR_REFRESH_RATE_23_976Hz; break;
        case 2400:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_24Hz; break;
        case 2500:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_25Hz; break;
        case 2997:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_29_97Hz; break;
        case 3000:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_30Hz; break;
        case 4800:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_48Hz; break;
        case 5000:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_50Hz; break;
        case 5994:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_59_94Hz; break;
        case 6000:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_60Hz; break;
        case 10000:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_100Hz; break;
        case 11988:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_119_88Hz; break;
        case 12000:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_120Hz; break;
        default:
            refreshRate = BXVD_MONITOR_REFRESH_RATE_60Hz; break;
        }
        BDBG_MSG(("ch[%d] BXVD_SetMonitorRefreshRate %d.%02d", videoDecoder->parentIndex,
            videoDecoder->displayInformation.refreshRate/100,
            videoDecoder->displayInformation.refreshRate%100));
        rc = BXVD_SetMonitorRefreshRate(videoDecoder->dec, refreshRate);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    }
    NEXUS_VideoDecoder_P_ApplyDisplayInformation_Common(videoDecoder);

    if (videoDecoder->trickState.tsmEnabled == NEXUS_TsmMode_eSimulated) {
        (void)NEXUS_VideoDecoder_P_SetTsm(videoDecoder);
    }
    return;
}

void
NEXUS_VideoDecoder_UpdateDefaultDisplayInformation_priv(const NEXUS_VideoDecoder_DisplayInformation *pDisplayInformation)
{
    unsigned i, j;
    NEXUS_ASSERT_MODULE();
    /* apply to all decoders have don't have BVN paths. */
    for (i=0;i<NEXUS_MAX_XVD_DEVICES;i++) {
        struct NEXUS_VideoDecoderDevice *device = &g_NEXUS_videoDecoderXvdDevices[i];
        if (!device->xvd) continue;
        for (j=0;j<NEXUS_NUM_XVD_CHANNELS;j++) {
            NEXUS_VideoDecoderHandle videoDecoder = device->channel[j];
            if (videoDecoder && videoDecoder->videoAsGraphics) {
                NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Avd(videoDecoder, pDisplayInformation);
            }
        }
    }
}

void
NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Common( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoder_DisplayInformation *pDisplayInformation)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (pDisplayInformation->refreshRate) {
        BDBG_MSG(("NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Common: %#lx  refreshRate:%u", (unsigned long)videoDecoder, pDisplayInformation->refreshRate));
        videoDecoder->displayInformation.refreshRate = pDisplayInformation->refreshRate;
    }
    /* always update stg index */
    videoDecoder->displayInformation.stgIndex = pDisplayInformation->stgIndex;
    return;
}

void
NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Avd( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoder_DisplayInformation *pDisplayInformation)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Common( videoDecoder, pDisplayInformation);
    NEXUS_VideoDecoder_P_ApplyDisplayInformation(videoDecoder);
    return;
}


void NEXUS_VideoDecoder_P_ChannelChangeReport(void *context)
{
#if B_HAS_TRC
#define REPORT_FREQUENCY 10

    const unsigned chn = 0;
    unsigned i = 0;
#if (defined B_PERF_BMIPS3300) || (defined B_PERF_BMIPS4380)
    const unsigned COUNTER = 3; /* Cycles */
    const unsigned CPU_CLOCK_RATE = 405; /* MHz */
#elif (defined B_PERF_LINUX)
    const unsigned COUNTER = 0; /* Time, us */
    const unsigned CPU_CLOCK_RATE = 1;
#else
    BSTD_UNUSED(context);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return;
#endif

    if (BTRC_MODULE_HANDLE(ChnChange_Total_Video)->stats[0].count % REPORT_FREQUENCY == 0) {
        if (NEXUS_GetEnv("force_vsync")) {BKNI_Printf("Channel change measurements with force_vsync=y\n");}
        if (NEXUS_GetEnv("sync_disabled")) {BKNI_Printf("Channel change measurements with sync_disabled=y\n");}
        BKNI_Printf("%-32s %6s %12s %12s %12s\n", "Name", "Cnt", "Avg(us)", "Max(us)", "Min(us)");

        for (i=0; i < VIDEO_BTRC_N_MODULES; i++) {
            const struct BTRC_P_Stats *stats = &video_btrc_modules[i]->stats[chn];

            if (stats->count) {
                const unsigned avg = (unsigned)(((((uint64_t)stats->total_hi.perf.data[COUNTER])<<((sizeof(unsigned))*8)) + stats->total.perf.data[COUNTER])/stats->count / CPU_CLOCK_RATE);
                const unsigned max = stats->max.perf.data[COUNTER] / CPU_CLOCK_RATE;
                const unsigned min = stats->min.perf.data[COUNTER] / CPU_CLOCK_RATE;
                BKNI_Printf("%-32s %6u %12u %12u %12u\n", video_btrc_modules[i]->name, stats->count, avg, max, min);
            }
        }
    }
#endif
    BSTD_UNUSED(context);
    return;
}

#if NEXUS_HAS_ASTM
void NEXUS_VideoDecoder_GetAstmSettings_priv_Common(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderAstmSettings * pAstmSettings)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_ASSERT_MODULE();

    *pAstmSettings = videoDecoder->astm.settings;
}

NEXUS_Error NEXUS_VideoDecoder_SetAstmSettings_priv_Avd(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderAstmSettings *pAstmSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_ASSERT_MODULE();

    BKNI_EnterCriticalSection();
    if (videoDecoder->astm.settings.enableAstm != pAstmSettings->enableAstm)
    {
        rc = BXVD_EnableInterrupt_isr(videoDecoder->dec, BXVD_Interrupt_eTSMPassInASTMMode, pAstmSettings->enableAstm);
        if (rc) {rc=BERR_TRACE(rc); /* continue */ }
    }

    /* copy settings as-is, this way ASTM will always get what it set */
    videoDecoder->astm.settings = *pAstmSettings;
    BKNI_LeaveCriticalSection();

    /* if ASTM is internally permitted, apply settings */
    rc = NEXUS_VideoDecoder_P_SetTsm(videoDecoder);

    rc = NEXUS_VideoDecoder_P_SetPtsOffset(videoDecoder);

    /* PR:49215 use HW PCR offset depends on playback flag */
    if (videoDecoder->dec)
    {
        rc = BXVD_SetHwPcrOffsetEnable(videoDecoder->dec, !videoDecoder->astm.settings.enablePlayback);
    }

    return rc;
}

NEXUS_Error NEXUS_VideoDecoder_GetAstmStatus_Common_isr(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderAstmStatus *pAstmStatus)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    *pAstmStatus = videoDecoder->astm.status;

    return NEXUS_SUCCESS;
}

#endif /* NEXUS_HAS_ASTM */

static BXDM_DisplayInterruptHandler_Handle nexus_p_alloc_dih_by_mfd(unsigned mfdIndex, unsigned *pAvdIndex, unsigned *pXdmIndex)
{
    unsigned i, avdIndex, xdmIndex;
    BXDM_DisplayInterruptHandler_Handle dih;

    *pXdmIndex = *pAvdIndex = 0;
    for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
        if (g_NEXUS_videoDecoderModuleSettings.mfdMapping[i] == mfdIndex) {
            avdIndex = g_NEXUS_videoDecoderModuleSettings.avdMapping[i];
            break;
        }
    }
    if (i == NEXUS_MAX_VIDEO_DECODERS) {BERR_TRACE(-1); return NULL;}

    for (xdmIndex = 0; xdmIndex < BXVD_DisplayInterrupt_eMax; xdmIndex++) {
        if (!g_NEXUS_videoDecoderXvdDevices[avdIndex].xdmInUse[xdmIndex]) break;
    }
    if (xdmIndex == BXVD_DisplayInterrupt_eMax) {BERR_TRACE(-1); return NULL;}
    dih = g_NEXUS_videoDecoderXvdDevices[avdIndex].hXdmDih[xdmIndex];
    if (!dih) {BERR_TRACE(-1); return NULL;}
    g_NEXUS_videoDecoderXvdDevices[avdIndex].xdmInUse[xdmIndex] = true;
    *pAvdIndex = avdIndex;
    *pXdmIndex = xdmIndex;
    return dih;
}

static void nexus_p_free_dih(BXDM_DisplayInterruptHandler_Handle dih)
{
    unsigned i, xdmIndex;
    for (i=0;i<NEXUS_MAX_XVD_DEVICES;i++) {
        for (xdmIndex = 0; xdmIndex < BXVD_DisplayInterrupt_eMax; xdmIndex++) {
            if (g_NEXUS_videoDecoderXvdDevices[i].hXdmDih[xdmIndex] == dih) {
                g_NEXUS_videoDecoderXvdDevices[i].xdmInUse[xdmIndex] = false;
                break;
            }
        }
    }
}

NEXUS_Error NEXUS_VideoDecoderModule_InstallXdmProvider_priv(
    unsigned mfdIndex,
    BXDM_PictureProvider_Handle provider,
    BXDM_DisplayInterruptHandler_PictureDataReady_isr callback_isr,
    void *pContext,
    BINT_Id topField,
    BINT_Id bottomField,
    BINT_Id frame,
    BXDM_DisplayInterruptHandler_Handle *dih
    )
{
    BERR_Code rc;
    BXDM_DisplayInterruptHandler_AddPictureProviderInterface_Settings addPictureProviderSettings;
    BXVD_DeviceVdcInterruptSettings VDCDevIntrSettings;
    unsigned avdIndex, xdmIndex;

    NEXUS_ASSERT_MODULE();

    *dih = nexus_p_alloc_dih_by_mfd(mfdIndex, &avdIndex, &xdmIndex);
    if (!*dih) return BERR_TRACE(-1);

    BXDM_DIH_AddPictureProviderInterface_GetDefaultSettings(&addPictureProviderSettings);
    rc = BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt(*dih, callback_isr, pContext, 0);
    if(rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc); goto err_install;}
    rc = BXDM_DisplayInterruptHandler_AddPictureProviderInterface(*dih, BXDM_PictureProvider_GetPicture_isr, provider, &addPictureProviderSettings);
    if(rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc); goto err_addpic;}

    BXVD_GetVdcDeviceInterruptDefaultSettings(g_NEXUS_videoDecoderXvdDevices[avdIndex].xvd, &VDCDevIntrSettings);

    VDCDevIntrSettings.VDCIntId_Topfield = topField;
    VDCDevIntrSettings.VDCIntId_Botfield = bottomField;
    VDCDevIntrSettings.VDCIntId_Frame = frame;
    VDCDevIntrSettings.uiFlags &= ~BXVD_DeviceVdcIntrSettingsFlags_UseFieldAsFrame;
    VDCDevIntrSettings.eDisplayInterrupt = xdmIndex;
    VDCDevIntrSettings.hAppXdmDih = *dih;

    rc = BXVD_RegisterVdcDeviceInterrupt(g_NEXUS_videoDecoderXvdDevices[avdIndex].xvd, &VDCDevIntrSettings);
    if (rc) {rc = BERR_TRACE(rc); goto err_register;}

    return NEXUS_SUCCESS;

err_register:
    BXDM_DisplayInterruptHandler_RemovePictureProviderInterface(*dih, BXDM_PictureProvider_GetPicture_isr, provider);
err_addpic:
    BXDM_DisplayInterruptHandler_UnInstallCallback_PictureDataReadyInterrupt(*dih);
err_install:
    nexus_p_free_dih(*dih);
    return rc;
}

void NEXUS_VideoDecoderModule_RemoveXdmProvider_priv(BXDM_DisplayInterruptHandler_Handle dih, BXDM_PictureProvider_Handle provider)
{
    NEXUS_ASSERT_MODULE();
    BXDM_DisplayInterruptHandler_UnInstallCallback_PictureDataReadyInterrupt(dih);
    BXDM_DisplayInterruptHandler_RemovePictureProviderInterface(dih, BXDM_PictureProvider_GetPicture_isr, provider);
    nexus_p_free_dih(dih);
}

void NEXUS_VideoDecoder_P_GetVsyncRate_isrsafe(unsigned refreshRate, unsigned *vsyncRate, bool *b1001Slip)
{
    switch (refreshRate) {
    case 2398:
    case 2397: /* real value is 23.976 */
        *b1001Slip = true;
        *vsyncRate = 2400;
        break;
    case 2400:
        *b1001Slip = false;
        *vsyncRate = 2400;
        break;
    case 2500:
        *b1001Slip = false;
        *vsyncRate = 2500;
        break;
    case 2997:
        *b1001Slip = true;
        *vsyncRate = 3000;
        break;
    case 3000:
        *b1001Slip = false;
        *vsyncRate = 3000;
        break;
    case 4800:
        *b1001Slip = false;
        *vsyncRate = 4800;
        break;
    case 5000:
        *b1001Slip = false;
        *vsyncRate = 5000;
        break;
    case 5994:
        *b1001Slip = true;
        *vsyncRate = 6000;
        break;
    case 1250:
        *b1001Slip = false;
        *vsyncRate = 1250;
        break;
    case 1498:
    case 1499:
        *b1001Slip = true;
        *vsyncRate = 1500;
        break;
    case 1500:
        *b1001Slip = false;
        *vsyncRate = 1500;
        break;
    case 2000:
        *b1001Slip = false;
        *vsyncRate = 2000;
        break;
    default:
    case 6000:
        *b1001Slip = false;
        *vsyncRate = refreshRate;
        break;
    }
}

BXDM_PictureProvider_MonitorRefreshRate NEXUS_VideoDecoder_P_GetXdmMonitorRefreshRate_isrsafe(unsigned nexusRefreshRate)
{
    BXDM_PictureProvider_MonitorRefreshRate refreshRate;
    switch(nexusRefreshRate) {
    case 749:
       refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e7_493Hz; break;
    case 750:
       refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e7_5Hz; break;
    case 999:
       refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e9_99Hz; break;
    case 100:
       refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e10Hz; break;
    case 1198:
    case 1199:
       refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e11_988Hz; break;
    case 1200:
       refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e12Hz; break;
    case 1250:
       refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz; break;
    case 1498:
    case 1499:
       refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz; break;
    case 1500:
       refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e15Hz; break;
    case 1998:
       refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e19_98Hz; break;
    case 2000:
       refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e20Hz; break;
    case 2398:
    case 2397: /* real value is 23.976 */
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e23_976Hz; break;
    case 2400:
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e24Hz; break;
    case 2500:
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e25Hz; break;
    case 2997:
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e29_97Hz; break;
    case 3000:
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e30Hz; break;
    case 4800:
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e48Hz; break;
    case 5000:
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e50Hz; break;
    default:
    case 5994:
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e59_94Hz; break;
    case 6000:
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e60Hz; break;
    case 10000:
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e100Hz; break;
    case 11988:
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e119_88Hz; break;
    case 12000:
        refreshRate = BXDM_PictureProvider_MonitorRefreshRate_e120Hz; break;
    }
    return refreshRate;
}

NEXUS_Timebase NEXUS_VideoDecoder_P_GetTimebase_isrsafe(NEXUS_VideoDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    return handle->timebase;
}

void NEXUS_VideoDecoderModule_GetStatistics( NEXUS_VideoDecoderModuleStatistics *pStats )
{
    BKNI_EnterCriticalSection();
    *pStats = g_NEXUS_VideoDecoderModuleStatistics;
    BKNI_LeaveCriticalSection();
}
