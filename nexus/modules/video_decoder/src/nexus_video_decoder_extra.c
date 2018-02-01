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
 **************************************************************************/

#include "nexus_video_decoder_module.h"
#include "priv/nexus_core.h"
#include "priv/nexus_rave_priv.h"
#include "nexus_surface.h"
#include "bgrc.h"
#include "bgrc_packet.h"

BDBG_MODULE(nexus_video_decoder_extra);

NEXUS_Error NEXUS_VideoDecoder_P_GetExtendedStatus_Avd( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderExtendedStatus *pStatus )
{
    NEXUS_Error rc;
    NEXUS_RaveStatus raveStatus;
    BXVD_RevisionInfo info;
    NEXUS_PidChannelStatus pidChanStatus;
    NEXUS_StcChannelStatus stcChanStatus;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    pStatus->interruptPolarity = NEXUS_P_PicturePolarity_FromMagnum_isrsafe(videoDecoder->last_field.eInterruptPolarity);
    pStatus->sourcePolarity = NEXUS_P_PicturePolarity_FromMagnum_isrsafe(videoDecoder->last_field.eSourcePolarity);

    pStatus->isMpeg1 = videoDecoder->last_field.eMpegType == BAVC_MpegType_eMpeg1;
    pStatus->isYCbCr422 = videoDecoder->last_field.eYCbCrType == BAVC_YCbCrType_e4_2_2;
    pStatus->fieldChrominanceInterpolationMode = videoDecoder->last_field.eChrominanceInterpolationMode == BAVC_InterpolationMode_eField;
    pStatus->adjQp = videoDecoder->last_field.ulAdjQp;
    pStatus->pictureRepeatFlag = videoDecoder->last_field.bPictureRepeatFlag;
    pStatus->luminanceNMBY = videoDecoder->last_field.ulLuminanceNMBY;
    pStatus->chrominanceNMBY = videoDecoder->last_field.ulChrominanceNMBY;
    pStatus->stripeWidth = NEXUS_P_StripeWidth_FromMagnum_isrsafe(videoDecoder->last_field.eStripeWidth);
    pStatus->captureCrc = videoDecoder->last_field.bCaptureCrc;
    pStatus->idrPicID = videoDecoder->last_field.ulIdrPicID;
    pStatus->picOrderCnt = videoDecoder->last_field.int32_PicOrderCnt;
    pStatus->sourceClipTop = videoDecoder->last_field.ulSourceClipTop;
    pStatus->sourceClipLeft = videoDecoder->last_field.ulSourceClipLeft;
    pStatus->lumaRangeRemapping = videoDecoder->last_field.ulLumaRangeRemapping;
    pStatus->chromaRangeRemapping = videoDecoder->last_field.ulChromaRangeRemapping;
    pStatus->dataReadyCount = videoDecoder->dataReadyCount;
    pStatus->lastPictureFlag = videoDecoder->last_field_flag;

    pStatus->rave.mainIndex = -1;
    pStatus->rave.mainSwRaveIndex = -1;
    pStatus->rave.enhancementIndex = -1;
    pStatus->rave.enhancementSwRaveIndex = -1;
    LOCK_TRANSPORT();
    if (videoDecoder->rave) {
        rc = NEXUS_Rave_GetStatus_priv(videoDecoder->rave, &raveStatus);
        if (!rc) {
            pStatus->rave.mainIndex = raveStatus.index;
            pStatus->rave.mainSwRaveIndex = raveStatus.swRaveIndex;
        }
    }
    if (videoDecoder->enhancementRave) {
        rc = NEXUS_Rave_GetStatus_priv(videoDecoder->enhancementRave, &raveStatus);
        if (!rc) {
            pStatus->rave.enhancementIndex = raveStatus.index;
            pStatus->rave.enhancementSwRaveIndex = raveStatus.swRaveIndex;
        }
    }
    UNLOCK_TRANSPORT();

    rc = BXVD_GetRevision(videoDecoder->device->xvd, &info);
    if (rc) return BERR_TRACE(rc);
    pStatus->version.firmware = info.ulDecoderFwRev;

	if(videoDecoder->startSettings.pidChannel)
	{
        rc =  NEXUS_PidChannel_GetStatus(videoDecoder->startSettings.pidChannel, &pidChanStatus);
        if (rc) return BERR_TRACE(rc);
        pStatus->pidChannelIndex = (int) pidChanStatus.pidChannelIndex;
	}
    else
        pStatus->pidChannelIndex = -1; /* No channel, decoder is stopped. */
    if(videoDecoder->startSettings.stcChannel)
    {
        rc =  NEXUS_StcChannel_GetStatus(videoDecoder->startSettings.stcChannel, &stcChanStatus);
        if (rc) return BERR_TRACE(rc);
        pStatus->stcChannelIndex = (int) stcChanStatus.index;
    }
    else
        pStatus->stcChannelIndex = -1;

    return 0;
}

void NEXUS_VideoDecoder_P_GetExtendedSettings_Avd( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderExtendedSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    *pSettings = videoDecoder->extendedSettings;
}

NEXUS_Error NEXUS_VideoDecoder_P_SetExtendedSettings_Avd( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderExtendedSettings *pSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bool setUserdata = false;
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    if (pSettings->s3DTVStatusEnabled != videoDecoder->extendedSettings.s3DTVStatusEnabled) {
        setUserdata = true;
    }
    videoDecoder->extendedSettings = *pSettings;
    NEXUS_IsrCallback_Set(videoDecoder->dataReadyCallback, &pSettings->dataReadyCallback);
    NEXUS_IsrCallback_Set(videoDecoder->s3DTVChangedCallback, &pSettings->s3DTVStatusChanged);

    rc = NEXUS_VideoDecoder_P_SetCrcFifoSize(videoDecoder, false);
    if (rc) return BERR_TRACE(rc);

    if (setUserdata) {
        rc = NEXUS_VideoDecoder_P_SetUserdata(videoDecoder);
        if (rc) return BERR_TRACE(rc);
    }

    return 0;
}

NEXUS_Error NEXUS_VideoDecoder_Get3DTVStatus( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoder3DTVStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    *pStatus = videoDecoder->s3DTVStatus;
    return 0;
}

NEXUS_Error NEXUS_VideoDecoder_P_GetStripedSurfaceCreateSettings( NEXUS_VideoDecoderHandle videoDecoder, const BAVC_MFD_Picture *pPicture, NEXUS_StripedSurfaceCreateSettings *pCreateSettings )
{
    bool bFieldPair;

    NEXUS_StripedSurface_GetDefaultCreateSettings(pCreateSettings);

    if( pPicture->bMute )
        return NEXUS_NOT_AVAILABLE;

    if( (!pPicture->ulSourceHorizontalSize) || (!pPicture->ulSourceVerticalSize) )
        return NEXUS_NOT_AVAILABLE;

    BKNI_EnterCriticalSection();
    if (videoDecoder->last_field.ulDecodePictureId == videoDecoder->last_getStripedSurfaceSerialNumber) {
        BKNI_LeaveCriticalSection();
        return NEXUS_NOT_AVAILABLE;
    }
    videoDecoder->last_getStripedSurfaceSerialNumber = videoDecoder->last_field.ulDecodePictureId;
    BKNI_LeaveCriticalSection();

    bFieldPair = (BAVC_DecodedPictureBuffer_eFieldsPair==pPicture->eBufferFormat && NULL!=pPicture->hLuminanceBotFieldBufferBlock /*&& pPicture->bStreamProgressive*/);

    pCreateSettings->imageWidth = pPicture->ulSourceHorizontalSize;
    if (bFieldPair && videoDecoder->startSettings.codec != NEXUS_VideoCodec_eH265) {
        pCreateSettings->imageHeight = pPicture->ulSourceVerticalSize * 2;
    }
    else {
        pCreateSettings->imageHeight = pPicture->ulSourceVerticalSize;
    }

    pCreateSettings->lumaBuffer = NEXUS_VideoDecoder_P_MemoryBlockFromMma(videoDecoder, pPicture->hLuminanceFrameBufferBlock);
    pCreateSettings->chromaBuffer = NEXUS_VideoDecoder_P_MemoryBlockFromMma(videoDecoder, pPicture->hChrominanceFrameBufferBlock);
    if (bFieldPair) {
        pCreateSettings->bottomFieldLumaBuffer = NEXUS_VideoDecoder_P_MemoryBlockFromMma(videoDecoder, pPicture->hLuminanceBotFieldBufferBlock);
        pCreateSettings->bottomFieldChromaBuffer = NEXUS_VideoDecoder_P_MemoryBlockFromMma(videoDecoder, pPicture->hChrominanceBotFieldBufferBlock);
    } else {
        pCreateSettings->bottomFieldLumaBuffer = NULL;
        pCreateSettings->bottomFieldChromaBuffer = NULL;
    }
    if(pCreateSettings->lumaBuffer==NULL || pCreateSettings->chromaBuffer==NULL) {
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        if(pCreateSettings->lumaBuffer) {
            NEXUS_MemoryBlock_Free(pCreateSettings->lumaBuffer);
        }
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    pCreateSettings->lumaBufferOffset = pPicture->ulLuminanceFrameBufferBlockOffset;
    pCreateSettings->chromaBufferOffset = pPicture->ulChrominanceFrameBufferBlockOffset;
    if (bFieldPair) {
        if(pCreateSettings->bottomFieldLumaBuffer==NULL || pCreateSettings->bottomFieldChromaBuffer==NULL) {
            (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            if(pCreateSettings->chromaBuffer) {
                NEXUS_MemoryBlock_Free(pCreateSettings->chromaBuffer);
            }
            if(pCreateSettings->bottomFieldLumaBuffer) {
                NEXUS_MemoryBlock_Free(pCreateSettings->bottomFieldLumaBuffer);
            }
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        pCreateSettings->bottomFieldLumaBufferOffset = pPicture->ulLuminanceBotFieldBufferBlockOffset;
        pCreateSettings->bottomFieldChromaBufferOffset = pPicture->ulChrominanceBotFieldBufferBlockOffset;
        pCreateSettings->bufferType = NEXUS_VideoBufferType_eFieldPair;
    } else {
        switch (pPicture->eSourcePolarity) {
        default:
        case BAVC_Polarity_eFrame: pCreateSettings->bufferType = NEXUS_VideoBufferType_eFrame; break;
        case BAVC_Polarity_eTopField: pCreateSettings->bufferType = NEXUS_VideoBufferType_eTopField; break;
        case BAVC_Polarity_eBotField: pCreateSettings->bufferType = NEXUS_VideoBufferType_eBotField; break;
        }
    }
    BDBG_CASSERT(BXVD_BufferType_eMax == (BXVD_BufferType)NEXUS_VideoBufferType_eMax);
    pCreateSettings->pitch = NEXUS_P_StripeWidth_FromMagnum_isrsafe(pPicture->eStripeWidth);
    pCreateSettings->stripedWidth = pCreateSettings->pitch;
    pCreateSettings->lumaStripedHeight = pPicture->ulLuminanceNMBY * 16;
    pCreateSettings->chromaStripedHeight = pPicture->ulChrominanceNMBY * 16;

    if (BAVC_YCbCrType_e4_2_0 == pPicture->eYCbCrType)
    {
        if (BAVC_VideoBitDepth_e10Bit == pPicture->eBitDepth)
        {
            pCreateSettings->lumaPixelFormat = BM2MC_PACKET_PixelFormat_eY10;
            pCreateSettings->chromaPixelFormat = BM2MC_PACKET_PixelFormat_eCb10_Cr10;
        }
        else
        {
            pCreateSettings->lumaPixelFormat = BM2MC_PACKET_PixelFormat_eY8;
            pCreateSettings->chromaPixelFormat = BM2MC_PACKET_PixelFormat_eCb8_Cr8;
        }
    }
    else
    {
        BM2MC_PACKET_PixelFormat m2mcPxlFmt;
        BGRC_Packet_ConvertPixelFormat(&m2mcPxlFmt, pPicture->ePxlFmt);
        pCreateSettings->lumaPixelFormat = m2mcPxlFmt;
    }

    pCreateSettings->matrixCoefficients = NEXUS_P_MatrixCoefficients_FromMagnum_isrsafe(pPicture->eMatrixCoefficients);

    return NEXUS_SUCCESS;
}

static NEXUS_StripedSurfaceHandle NEXUS_VideoDecoder_CreateStripedSurface_priv( NEXUS_VideoDecoderHandle videoDecoder, const BAVC_MFD_Picture *pPicture )
{
    NEXUS_StripedSurfaceCreateSettings createSettings;
    NEXUS_Error errCode;

    errCode = NEXUS_VideoDecoder_P_GetStripedSurfaceCreateSettings(videoDecoder, pPicture, &createSettings);
    if ( errCode )
    {
        return NULL;
    }

    return NEXUS_StripedSurface_Create(&createSettings);
}

NEXUS_StripedSurfaceHandle NEXUS_VideoDecoder_P_CreateStripedSurface_Avd( NEXUS_VideoDecoderHandle videoDecoder )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    return NEXUS_VideoDecoder_CreateStripedSurface_priv( videoDecoder, &videoDecoder->last_field );
}

NEXUS_Error NEXUS_VideoDecoder_P_CreateStripedMosaicSurfaces_Avd( NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_StripedSurfaceHandle *pStripedSurfaces, unsigned surfaceMax, unsigned int *pSurfaceCount )
{
    NEXUS_VideoDecoderHandle v;
    unsigned channelIdx, i;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    /* Coverity: 25911 */
    if (pStripedSurfaces == NULL) {
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    *pSurfaceCount = 0;
    channelIdx = 0, i = 0;

    while ((i<surfaceMax) && (channelIdx<NEXUS_NUM_XVD_CHANNELS)) {
        v = videoDecoder->device->channel[channelIdx];
        if (v && v->dec && v->mosaicMode) {
            /* some stripedSurfaces may be NULL, while others may not be. there is no guarantee that
               all mosaic surfaces will have the same bPictureRepeatFlag at the same time */
            pStripedSurfaces[i++] = NEXUS_VideoDecoder_CreateStripedSurface_priv(v, &v->last_field);
        }
        channelIdx++;
    }

    *pSurfaceCount = i;
    return 0;
}

void NEXUS_VideoDecoder_P_DestroyStripedSurface_Avd( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_StripedSurfaceHandle stripedSurface )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_StripedSurface_Destroy(stripedSurface);
}

NEXUS_Error NEXUS_VideoDecoder_P_GetMostRecentPts_Avd( NEXUS_VideoDecoderHandle videoDecoder, uint32_t *pPts )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    LOCK_TRANSPORT();
    rc = NEXUS_Rave_GetPtsRange_priv(videoDecoder->rave, pPts, NULL);
    UNLOCK_TRANSPORT();
    return rc; /* don't use BERR_TRACE. failure can be normal. */
}

void NEXUS_VideoDecoder_P_GetExtendedSettings_NotImplemented( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderExtendedSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BKNI_Memcpy(pSettings, &videoDecoder->extendedSettings, sizeof(*pSettings));
    return;
}

NEXUS_Error NEXUS_VideoDecoder_P_SetExtendedSettings_NotImplemented( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderExtendedSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_ASSERT(pSettings);
    if(BKNI_Memcmp(pSettings, &videoDecoder->extendedSettings, sizeof(*pSettings))!=0) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return NEXUS_SUCCESS;
}
