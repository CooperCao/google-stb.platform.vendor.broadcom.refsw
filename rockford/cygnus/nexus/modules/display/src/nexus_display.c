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

/* This is basically a stub module */

#include "nexus_base.h"
#include "nexus_display_module.h"
#include "priv/nexus_display_priv.h"

BDBG_MODULE(nexus_display);
BDBG_FILE_MODULE(nexus_flow_display);

#define pVideo (&g_NEXUS_DisplayModule_State)
#define B_REFRESH_RATE_10_TO_1000(RATE) (((RATE) == 2397) ? 23976 : (RATE) * 10)


void
NEXUS_Display_GetDefaultSettings(NEXUS_DisplaySettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    /* normally nexus prefers default values which also happen to be zero. but eAuto was added to the end of the list
    so that the beginning of the list can track with VDC */
    pSettings->timingGenerator = NEXUS_DisplayTimingGenerator_eAuto;
    pSettings->displayType = NEXUS_DisplayType_eAuto;
    pSettings->format = NEXUS_VideoFormat_eNtsc;
    pSettings->aspectRatio = NEXUS_DisplayAspectRatio_eAuto; /* This needs to be eAuto. If the user changes the format to 720p et al. but not
                                                                the aspectRatio, they will likely expect the display a/r to change. */
    pSettings->dropFrame = NEXUS_TristateEnable_eNotSet;
    pSettings->timebase = NEXUS_Timebase_eInvalid;
    pSettings->vecIndex = -1;
    pSettings->display3DSettings.overrideOrientation = false;
    pSettings->display3DSettings.orientation = NEXUS_VideoOrientation_e2D;
    pSettings->display3DSettings.sourceBuffer = NEXUS_Display3DSourceBuffer_eDefault;
    return;
}

NEXUS_Error NEXUS_P_Display_GetMagnumVideoFormatInfo_isr(NEXUS_DisplayHandle display, NEXUS_VideoFormat videoFormat, BFMT_VideoInfo *pInfo)
{
    BFMT_VideoFmt formatVdc;

    if (videoFormat == NEXUS_VideoFormat_eCustom2) {
        if (display->customFormatInfo) {
            *pInfo = *display->customFormatInfo;
            return 0;
        }
        else {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    else {
        NEXUS_Error rc;
        rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(videoFormat, &formatVdc);
        if (rc) {return BERR_TRACE(rc);}
        rc = BFMT_GetVideoFormatInfo( formatVdc, pInfo );
        if (rc) {return BERR_TRACE(rc);}
    }
    return 0;
}

#if NEXUS_NUM_DISPLAYS > 1
NEXUS_Error NEXUS_Display_P_Align( NEXUS_DisplayHandle display, NEXUS_DisplayHandle target )
{
    BSTD_UNUSED(display);
    BSTD_UNUSED(target);
    return BERR_SUCCESS;
}
#endif

/*
Summary:
Returns bits 'e'..'b' from word 'w',

Example:
    B_GET_BITS(0xDE,7,4)==0x0D
    B_GET_BITS(0xDE,3,0)=0x0E
*/
#define B_GET_BITS(w,e,b)  (((w)>>(b))&(((unsigned)(-1))>>((sizeof(unsigned))*8-(e+1-b))))

static BERR_Code
NEXUS_Display_P_SetSettings(NEXUS_DisplayHandle display, const NEXUS_DisplaySettings *pSettings, bool force)
{
    BSTD_UNUSED(display);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(force);
    return BERR_SUCCESS;
}

#if NEXUS_HAS_VIDEO_ENCODER && NEXUS_NUM_DSP_VIDEO_ENCODERS
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
static void NEXUS_Display_GetCaptureBuffer_isr(NEXUS_DisplayHandle display)
{
    BAVC_EncodePictureBuffer picture;
    NEXUS_Error rc;
    bool drop=false;
    NEXUS_Display_P_Image *pImage;

    if (!display->encoder.window || !display->encoder.window->vdcState.window) {
        return;
    }

    for (;;) {
        BKNI_Memset_isr(&picture, 0, sizeof(BAVC_EncodePictureBuffer));
        BVDC_Display_GetBuffer_isr(display->displayVdc, &picture);
        BDBG_MSG(("GET %p %u", (void *)picture.hLumaBlock, picture.ulPictureId));

        if (picture.hLumaBlock == NULL) {
            if(!BLST_SQ_FIRST(&display->encoder.free)) {
                BDBG_WRN(("%d out of %d Buffers in use", NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS, NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS));
            }
            break;
        }

        BDBG_MSG(("L:%u;C:%u;W:%u;H:%u;P:%u;T:%#x;S:%#x;I:%u;R:%u;X:%u;Y:%u",
            picture.ulLumaOffset,
            picture.ulChromaOffset,
            picture.ulWidth,
            picture.ulHeight,
            picture.ePolarity,
            picture.ulOriginalPTS,
            picture.ulSTCSnapshotLo,
            picture.ulSTCSnapshotHi,
            picture.ulPictureId,
            picture.eFrameRate,
            picture.ulAspectRatioX,
            picture.ulAspectRatioY));

        if (display->encoder.framesEnqueued % display->encoder.dropRate == 0)
        {
            rc = display->encoder.enqueueCb_isr(display->encoder.context, &picture);
            if(rc == BERR_SUCCESS) {
                pImage = BLST_SQ_FIRST(&display->encoder.free);
                BDBG_ASSERT(pImage);
                pImage->hImage = picture.hLumaBlock;
                pImage->picId = picture.ulPictureId;
                BLST_SQ_REMOVE_HEAD(&display->encoder.free, link);
                BLST_SQ_INSERT_TAIL(&display->encoder.queued, pImage, link);
            }
        } else {
            drop = true;
            rc = BERR_SUCCESS;
        }
        if (drop || rc) {
            BDBG_ERR(("RET1 %p %u", (uint32_t)picture.hLumaBlock, picture.ulPictureId));
            BVDC_Display_ReturnBuffer_isr(display->displayVdc, &picture);
        }
        display->encoder.framesEnqueued++;
    }
}

static void NEXUS_Display_ReturnCaptureBuffer_isr(NEXUS_DisplayHandle display)
{
    BAVC_EncodePictureBuffer picture;
    NEXUS_Display_P_Image *pImage;
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (!display->encoder.window || !display->encoder.window->vdcState.window) {
        return;
    }

    /* Query the encoder for all possible buffers that can be returned */
    for(;;) {
        BKNI_Memset(&picture, 0, sizeof(BAVC_EncodePictureBuffer));

        rc = display->encoder.dequeueCb_isr(display->encoder.context, &picture);
        if (rc) { BERR_TRACE(rc); break; }

        if (picture.hLumaBlock == NULL) {
            break;
        }

        for (pImage = BLST_SQ_FIRST(&display->encoder.queued); pImage; pImage= BLST_SQ_NEXT(pImage, link)) {
            if (pImage->hImage == picture.hLumaBlock && pImage->picId == picture.ulPictureId) {
                break;
            }
        }

        if (pImage)
        {
            BDBG_MSG(("RET2 %p %u", (void *)picture.hLumaBlock, picture.ulPictureId));
            BVDC_Display_ReturnBuffer_isr(display->displayVdc, &picture);
            pImage->hImage = NULL;
            BLST_SQ_REMOVE(&display->encoder.queued, pImage , NEXUS_Display_P_Image, link);
            BLST_SQ_INSERT_TAIL(&display->encoder.free, pImage, link);
        }
        else
        {
            break;
        }
    }
}
#else /* DSP NO VIP */
static void NEXUS_Display_GetCaptureBuffer_isr(NEXUS_DisplayHandle display,
    unsigned * pEncPicId,
    unsigned * pDecPicId,
    BAVC_Polarity * pPolarity,
    unsigned * pSourceRate)
{
    BVDC_Test_Window_CapturedImage capture;
    NEXUS_DisplayCapturedImage image;
    NEXUS_Error rc;
    bool drop=false;
    NEXUS_Display_P_Image *pImage;

    if (!display->encoder.window || !display->encoder.window->vdcState.window) {
        return;
    }

    BVDC_Test_Window_GetBuffer_isr(display->encoder.window->vdcState.window, &capture);
    BDBG_MSG(("GET %#x", (uint32_t)capture.hPicBlock));
    if(capture.hPicBlock) {
        if (pEncPicId)
        {
            *pEncPicId = capture.ulEncPicId;
        }
        if (pDecPicId)
        {
            *pDecPicId = capture.ulDecPicId;
        }
        if (pPolarity)
        {
            *pPolarity = capture.eCapturePolarity;
        }
        if (pSourceRate)
        {
            *pSourceRate = capture.ulSourceRate;
        }
        if(display->encoder.framesEnqueued%display->encoder.dropRate == 0) {
            image.hImage = capture.hPicBlock;
            image.offset = capture.ulPicBlockOffset;
            image.width = capture.ulWidth;
            image.height = capture.ulHeight;
            image.encPicId = capture.ulEncPicId;
            image.decPicId = capture.ulDecPicId;
            image.polarity = capture.eCapturePolarity;
            image.origPts = capture.ulOrigPTS;
            image.stallStc = capture.bStallStc;
            image.ignorePicture = capture.bIgnorePicture;
            image.aspectRatioX = capture.ulPxlAspRatio_x;
            image.aspectRatioY = capture.ulPxlAspRatio_y;
            NEXUS_P_FrameRate_FromRefreshRate_isrsafe(display->status.refreshRate , &image.framerate);
            rc = display->encoder.enqueueCb_isr(display->encoder.context, &image);
            if(rc == BERR_SUCCESS) {
                pImage = BLST_SQ_FIRST(&display->encoder.free);
                BDBG_ASSERT(pImage);
                pImage->hImage = capture.hPicBlock;
                pImage->offset = capture.ulPicBlockOffset;
                BLST_SQ_REMOVE_HEAD(&display->encoder.free, link);
                BLST_SQ_INSERT_TAIL(&display->encoder.queued, pImage, link);
            }
        } else {
            drop=true;
            rc = BERR_SUCCESS;
        }
        if(drop || rc) {
            BDBG_MSG(("RET1 %#x", (uint32_t)capture.hPicBlock));
            BVDC_Test_Window_ReturnBuffer_isr(display->encoder.window->vdcState.window, &capture);

        }
        display->encoder.framesEnqueued++;
    }else {
        if(!BLST_SQ_FIRST(&display->encoder.free)) {
            BDBG_WRN(("%d out of %d Buffers in use", NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS, NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS));
        }
    }
}

static void NEXUS_Display_ReturnCaptureBuffer_isr(NEXUS_DisplayHandle display)
{
    BVDC_Test_Window_CapturedImage capture;
    NEXUS_DisplayCapturedImage image;
    NEXUS_Error rc;
    NEXUS_Display_P_Image *pImage;

    if (!display->encoder.window || !display->encoder.window->vdcState.window) {
        return;
    }

    /* Query the encoder for all possible buffers that can be returned */
    for(;;) {
        rc = display->encoder.dequeueCb_isr(display->encoder.context, &image);

        if(image.hImage==NULL) {
            break;
        }
        BDBG_MSG(("RET2 %p", (void *)image.hImage));
        for(pImage = BLST_SQ_FIRST(&display->encoder.queued); pImage;  pImage= BLST_SQ_NEXT(pImage, link)) {
            if(pImage->hImage==image.hImage && pImage->offset == image.offset) {
                break;
            }
        }
        if(pImage) {
            capture.hPicBlock = image.hImage;
            capture.ulPicBlockOffset = image.offset;
            BVDC_Test_Window_ReturnBuffer_isr(display->encoder.window->vdcState.window, &capture);
            pImage->hImage = NULL;
            BLST_SQ_REMOVE(&display->encoder.queued, pImage , NEXUS_Display_P_Image, link);
            BLST_SQ_INSERT_TAIL(&display->encoder.free, pImage, link);
        } else {
            /* This should never happen. We should always find the return buffer in our list if dequeue_isr returns one*/
            /* BDBG_ASSERT(0); */
            BDBG_WRN(("Buffer %p not found in queue", (void *)image.hImage));
        }
    }
}
#endif /* NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT */
#endif /* HAS_DSP_ENCODE */

static void NEXUS_DisplayCb_isr(void *pParam, int iParam, void *pCbData)
{
    NEXUS_DisplayHandle display;
    BAVC_VdcDisplay_Info *pVdcRateInfo;
    BVDC_Display_CallbackData *pCallbackData = pCbData;
    BDBG_ASSERT(NULL != pParam);
    BDBG_ASSERT(NULL != pCbData);
    BSTD_UNUSED(iParam);

    display = pParam;
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);

    pVdcRateInfo = &(pCallbackData->sDisplayInfo);

#if NEXUS_HAS_VIDEO_ENCODER && NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    /* for DSP w/o VIP, do not rely on actual stg picture id change */
    pCallbackData->stMask.bStgPictureId = false;

    if (pCallbackData->stMask.bPerVsync && display->encoder.callbackEnabled) {
        unsigned encPicId = -1;
        unsigned decPicId = -1;
        BAVC_Polarity polarity = BAVC_Polarity_eFrame;
        unsigned sourceRate = 0;

        NEXUS_Display_GetCaptureBuffer_isr(display, &encPicId, &decPicId, &polarity, &sourceRate);
        NEXUS_Display_ReturnCaptureBuffer_isr(display);

        if ((signed)encPicId != -1)
        {
            pCallbackData->ulStgPictureId = encPicId;
            pCallbackData->ulDecodePictureId = decPicId;
            pCallbackData->ePolarity = polarity;
            pCallbackData->sDisplayInfo.ulVertRefreshRate = sourceRate;
            pCallbackData->stMask.bStgPictureId = true;
        }
    }
#endif

#if NEXUS_HAS_VIDEO_ENCODER
    if (display->encodeUserData && pCallbackData->stMask.bStgPictureId)
    {
        BXUDlib_DisplayInterruptHandler_isr(display->hXud, pCallbackData);
    }
#endif

    if (pCallbackData->stMask.bRateChange) {
#if NEXUS_NUM_HDMI_OUTPUTS
        display->hdmi.rateInfo = *pVdcRateInfo;
        display->hdmi.rateInfoValid = true;

        if ( NULL != display->hdmi.rateChangeCb_isr )
        {
            BDBG_MSG(("Propagating rate change to HDMI"));
            display->hdmi.rateChangeCb_isr(display, display->hdmi.pCbParam);
        }
#endif
#if NEXUS_NUM_HDMI_DVO
        display->hdmiDvo.rateInfo = *pVdcRateInfo;
        display->hdmiDvo.rateInfoValid = true;

        if ( NULL != display->hdmiDvo.rateChangeCb_isr )
        {
            BDBG_MSG(("Propagating rate change to HDMI DVO"));
            display->hdmiDvo.rateChangeCb_isr(display, display->hdmiDvo.pCbParam);
        }
#endif
        display->status.refreshRate = B_REFRESH_RATE_10_TO_1000(pVdcRateInfo->ulVertRefreshRate);

        BDBG_MODULE_MSG(nexus_flow_display, ("display %d refresh rate %d.%02dhz", display->index,
            display->status.refreshRate/1000, display->status.refreshRate%1000));
        BKNI_SetEvent(display->refreshRate.event);
    }

    if (pCallbackData->stMask.bCrc && display->crc.queue) {
        NEXUS_DisplayCrcData *pData = &display->crc.queue[display->crc.wptr];
        pData->cmp.luma = pCallbackData->ulCrcLuma;
        pData->cmp.cb = pCallbackData->ulCrcCb;
        pData->cmp.cr = pCallbackData->ulCrcCr;
        if (++display->crc.wptr == display->crc.size) {
            display->crc.wptr = 0;
        }
        if (display->crc.wptr == display->crc.rptr) {
            BDBG_WRN(("Display%d CMP CRC overflow", display->index));
        }
    }

    if (pCallbackData->stMask.bCableDetect) {
        uint32_t i;
        for(i=0; i<BVDC_MAX_DACS; i++) {
            display->dacStatus[i] = pCallbackData->aeDacConnectionState[i];
        }
    }

#if NEXUS_HAS_VIDEO_ENCODER && NEXUS_NUM_DSP_VIDEO_ENCODERS && NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    if (pCallbackData->stMask.bPerVsync && display->encoder.callbackEnabled) {
        NEXUS_Display_GetCaptureBuffer_isr(display);
        NEXUS_Display_ReturnCaptureBuffer_isr(display);
    }
#endif

    if (pCallbackData->stMask.bPerVsync) {
        BTMR_ReadTimer_isr(pVideo->tmr, &display->lastVsyncTime);
    }
}

unsigned NEXUS_Display_GetLastVsyncTime_isr(NEXUS_DisplayHandle display)
{
    return display->lastVsyncTime;
}

/* nexus_p_install_display_cb is called on Open and SetSettings.
it is not evenly paired with nexus_p_uninstall_display_cb */
static BERR_Code nexus_p_install_display_cb(NEXUS_DisplayHandle display)
{
    BSTD_UNUSED(display);
    return 0;
}

static void nexus_p_uninstall_display_cb(NEXUS_DisplayHandle display)
{
   BSTD_UNUSED(display);
}

static BERR_Code
NEXUS_Display_P_Open(NEXUS_DisplayHandle display, unsigned displayIndex, const NEXUS_DisplaySettings *pSettings)
{
    BERR_Code rc=BERR_SUCCESS;
    BVDC_DisplayId vdcDisplayId;
    bool bModifiedSync = (g_NEXUS_DisplayModule_State.moduleSettings.componentOutputSyncType == NEXUS_ComponentOutputSyncType_eAllChannels);
    BDBG_MODULE_MSG(nexus_flow_display, ("open %p, index %d, type %d",
        (void *)display, displayIndex, pSettings->displayType));

    if (pSettings->displayType == NEXUS_DisplayType_eDvo || pSettings->displayType == NEXUS_DisplayType_eLvds) {
        BVDC_Display_Settings vdcDisplayCfg;

        if (displayIndex != 0) {
            BDBG_ERR(("invalid dvo display"));
            goto err_compositor;
        }
        rc = BVDC_Compositor_Create(pVideo->vdc, &display->compositor, BVDC_CompositorId_eCompositor0, NULL);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_compositor;}
        vdcDisplayId = BVDC_DisplayId_eDisplay0;
        BVDC_Display_GetDefaultSettings(vdcDisplayId, &vdcDisplayCfg);
        display->timingGenerator = vdcDisplayCfg.eMasterTg = BVDC_DisplayTg_eDviDtg;
        vdcDisplayCfg.bModifiedSync = bModifiedSync;
        rc = BVDC_Display_Create(display->compositor, &display->displayVdc, vdcDisplayId, &vdcDisplayCfg);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_display;}
    }
    else if (pSettings->displayType == NEXUS_DisplayType_eBypass) {
        BVDC_Display_Settings vdcDisplayCfg;

        rc = BVDC_Compositor_Create(pVideo->vdc, &display->compositor, BVDC_CompositorId_eCompositor0 + displayIndex, NULL);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_compositor;}

        BVDC_Display_GetDefaultSettings(BVDC_DisplayId_eDisplay2, &vdcDisplayCfg);
        display->timingGenerator = vdcDisplayCfg.eMasterTg = BVDC_DisplayTg_ePrimIt;
        vdcDisplayCfg.bModifiedSync = bModifiedSync;
        rc = BVDC_Display_Create(display->compositor, &display->displayVdc, BVDC_DisplayId_eDisplay0, &vdcDisplayCfg);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_display;}
    }
    else {
        BVDC_Display_Settings vdcDisplayCfg;
        NEXUS_DisplayTimingGenerator timingGenerator = pSettings->timingGenerator;

        unsigned vecIndex;
        NEXUS_VideoFormatInfo formatInfo;
        if (pSettings->vecIndex == -1) {
            if (displayIndex < 2 && g_NEXUS_DisplayModule_State.moduleSettings.vecSwap) {
                vecIndex = 1-displayIndex;
            }
            else {
                vecIndex = displayIndex;
            }
        }
        else {
            vecIndex = (unsigned)pSettings->vecIndex;
        }
        rc = BVDC_Compositor_Create(pVideo->vdc, &display->compositor, BVDC_CompositorId_eCompositor0+displayIndex, NULL);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_compositor;}

        BVDC_Display_GetDefaultSettings(BVDC_DisplayId_eDisplay0+vecIndex, &vdcDisplayCfg);
        if (timingGenerator < NEXUS_DisplayTimingGenerator_eAuto) {
            BDBG_CASSERT(NEXUS_DisplayTimingGenerator_eEncoder == (NEXUS_DisplayTimingGenerator)BVDC_DisplayTg_eStg);
            vdcDisplayCfg.eMasterTg = (BVDC_DisplayTg)timingGenerator;
        }

#if !NEXUS_NUM_DSP_VIDEO_ENCODERS
        /* encoder timing generator override */
        if (timingGenerator == NEXUS_DisplayTimingGenerator_eAuto || timingGenerator == NEXUS_DisplayTimingGenerator_eEncoder) {
#if BCHP_CHIP == 7425
/* no BBOX */
            if (display->index == 2 || display->index == 3) {
                display->stgIndex = 3 - display->index;
                vdcDisplayCfg.eMasterTg = BVDC_DisplayTg_eStg0 + display->stgIndex;
            }
#elif BCHP_CHIP == 7145 && BCHP_VER == BCHP_VER_A0
/* no BBOX */
            if (display->index == 2 || display->index == 3 || display->index == 4 || display->index == 5) {
                display->stgIndex = 5 - display->index;
                vdcDisplayCfg.eMasterTg = BVDC_DisplayTg_eStg0 + display->stgIndex;
            }
#elif (BCHP_CHIP == 7439  && BCHP_VER == BCHP_VER_A0) || \
      (BCHP_CHIP == 74371 && BCHP_VER == BCHP_VER_A0)
/* no BBOX */
            if (display->index == 2) {
                display->stgIndex = 2 - display->index;
                vdcDisplayCfg.eMasterTg = BVDC_DisplayTg_eStg0 + display->stgIndex;
            }
#else
            if (g_pCoreHandles->boxConfig->stVdc.astDisplay[display->index].stStgEnc.bAvailable) {
                display->stgIndex = g_pCoreHandles->boxConfig->stVdc.astDisplay[display->index].stStgEnc.ulStgId;
                vdcDisplayCfg.eMasterTg = BVDC_DisplayTg_eStg0 + display->stgIndex;
                timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
            }
#endif
        }

        if(timingGenerator == NEXUS_DisplayTimingGenerator_eAuto)
        {
            if(pVideo->vdcCapabilities.ulNumCmp < pVideo->vdcCapabilities.ulNumStg + pVideo->vdcCapabilities.ulNumAtg)
            {
                /* there is overlap usage between analog and stg TG */
                if(vecIndex < pVideo->vdcCapabilities.ulNumAtg)
                {
                    vdcDisplayCfg.eMasterTg = vecIndex % NEXUS_DisplayTimingGenerator_eTertiaryInput;
                }
            }
            else if(vecIndex < pVideo->vdcCapabilities.ulNumCmp - pVideo->vdcCapabilities.ulNumStg)
            {
                /* only touch non-STG displays */
                if(pVideo->vdcCapabilities.ulNumCmp - pVideo->vdcCapabilities.ulNumStg == pVideo->vdcCapabilities.ulNumAtg)
                {
                    /* enough IT => default analog master */
                    vdcDisplayCfg.eMasterTg = vecIndex % NEXUS_DisplayTimingGenerator_eTertiaryInput;
                }
                else if(pVideo->vdcCapabilities.ulNumAtg + pVideo->vdcCapabilities.ulNum656Output >= pVideo->vdcCapabilities.ulNumCmp - pVideo->vdcCapabilities.ulNumStg)
                {
                    /* The last display gets 656 master otherwise analog master */
                    if(vecIndex >= pVideo->vdcCapabilities.ulNumAtg)
                        vdcDisplayCfg.eMasterTg = NEXUS_DisplayTimingGenerator_e656Output;
                    else
                        vdcDisplayCfg.eMasterTg = vecIndex % NEXUS_DisplayTimingGenerator_eTertiaryInput;
                }
                else
                {
                    /* the first display gets HDMI master */
                    if(vecIndex < pVideo->vdcCapabilities.ulNumCmp - pVideo->vdcCapabilities.ulNumStg - pVideo->vdcCapabilities.ulNumAtg - pVideo->vdcCapabilities.ulNum656Output)
                        vdcDisplayCfg.eMasterTg = NEXUS_DisplayTimingGenerator_eHdmiDvo;
                    else if(vecIndex >= pVideo->vdcCapabilities.ulNumAtg + pVideo->vdcCapabilities.ulNumHdmiOutput)
                        vdcDisplayCfg.eMasterTg = NEXUS_DisplayTimingGenerator_e656Output;
                    else
                        vdcDisplayCfg.eMasterTg = vecIndex % NEXUS_DisplayTimingGenerator_eTertiaryInput;
                }
            }
        }

        if( vdcDisplayCfg.eMasterTg >=BVDC_DisplayTg_eStg0 && vdcDisplayCfg.eMasterTg <=BVDC_DisplayTg_eStg5)
            display->timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
        else
            display->timingGenerator = vdcDisplayCfg.eMasterTg;
#else /* DSPs with VIP */
        /* encoder timing generator override */
        if (timingGenerator == NEXUS_DisplayTimingGenerator_eAuto || timingGenerator == NEXUS_DisplayTimingGenerator_eEncoder) {
            if (g_pCoreHandles->boxConfig->stVdc.astDisplay[display->index].stStgEnc.bAvailable) {
                display->stgIndex = g_pCoreHandles->boxConfig->stVdc.astDisplay[display->index].stStgEnc.ulStgId;
                vdcDisplayCfg.eMasterTg = BVDC_DisplayTg_eStg0 + display->stgIndex;
            }
        }

        if( vdcDisplayCfg.eMasterTg >=BVDC_DisplayTg_eStg0 && vdcDisplayCfg.eMasterTg <=BVDC_DisplayTg_eStg5)
            display->timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
        else
            display->timingGenerator = vdcDisplayCfg.eMasterTg;

        BDBG_MSG(("display->stgIndex = %d; display->timingGenerator = %u", display->stgIndex, display->timingGenerator));
#endif
        NEXUS_VideoFormat_GetInfo(pSettings->format, &formatInfo);
        if (formatInfo.isFullRes3d && timingGenerator!=NEXUS_DisplayTimingGenerator_eHdmiDvo) {
            BDBG_WRN(("3D output display format selected without HDMI master mode (NEXUS_DisplayTimingGenerator_eHdmiDvo)"));
        }
        vdcDisplayCfg.bModifiedSync = bModifiedSync;
        rc = BVDC_Display_Create(display->compositor, &display->displayVdc, BVDC_DisplayId_eDisplay0+vecIndex, &vdcDisplayCfg);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_display;}
    }

    /* eAuto is a nexus construct. It must be resolved before being passed to VDC. Then, any nexus code must test the resolved value. */
    BDBG_ASSERT(display->timingGenerator < NEXUS_DisplayTimingGenerator_eAuto);

    return rc;

err_display:
    BVDC_Compositor_Destroy(display->compositor);
err_compositor:
    {
        BERR_Code erc = BVDC_AbortChanges(pVideo->vdc);
        if (erc!=BERR_SUCCESS) {rc=BERR_TRACE(erc);}
    }
    return rc;
}


NEXUS_DisplayHandle
NEXUS_Display_Open(unsigned displayIndex,const NEXUS_DisplaySettings *pSettings)
{
    NEXUS_DisplayHandle  display;
    BERR_Code rc;
    unsigned i;
    NEXUS_DisplaySettings defaultSettings;
    NEXUS_DisplayTimingGenerator timingGenerator;

    if (displayIndex >= NEXUS_NUM_DISPLAYS || pVideo->cap.display[displayIndex].numVideoWindows == 0) {
        BDBG_ERR(("display[%d] cannot be opened!", displayIndex));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    if(pSettings==NULL) {
        NEXUS_Display_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    timingGenerator = pSettings->timingGenerator;

    if(displayIndex>=sizeof(pVideo->displays)/sizeof(pVideo->displays[0])) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_settings;
    }

    display = BKNI_Malloc(sizeof(*display));
    if(!display) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    NEXUS_OBJECT_INIT(NEXUS_Display, display);
    display->cfg = *pSettings;
    display->index = displayIndex;
    BLST_D_INIT(&display->outputs);
    display->vsyncCallback.isrCallback = NEXUS_IsrCallback_Create(display, NULL);
    if (!display->vsyncCallback.isrCallback) {rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_createisrcallback;}
    rc = BKNI_CreateEvent(&display->refreshRate.event);
    if (rc) {rc = BERR_TRACE(rc); goto err_createrefreshrateevent;}

    if (timingGenerator == NEXUS_DisplayTimingGenerator_eEncoder) {
        display->stgSettings.enabled = true;
    }

    for(i=0;i<sizeof(display->windows)/sizeof(display->windows[0]);i++) {
        display->windows[i].open = false;
    }

    NEXUS_Display_P_InitGraphics(display);

    pVideo->displays[display->index] = display;

    return display;

err_createrefreshrateevent:
    NEXUS_IsrCallback_Destroy(display->vsyncCallback.isrCallback);
err_createisrcallback:
    NEXUS_OBJECT_DESTROY(NEXUS_Display, display);
    BKNI_Free(display);
err_alloc:
err_settings:
    return NULL;
}

static void
NEXUS_Display_P_Release(NEXUS_DisplayHandle display)
{
  BSTD_UNUSED(display);
  return;
}

static void
NEXUS_Display_P_Finalizer(NEXUS_DisplayHandle display)
{
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);

    if (display->index >= sizeof(pVideo->displays)/sizeof(pVideo->displays[0])) {
        /* adding runtime check for static analysis */
        return;
    }

    /* stop all callbacks from coming into display module */
    display->cfg.vsyncCallback.callback = NULL; /* force the INT callbacks to be destroyed */

    /* if we close the last display, then shutdown all cached VideoInput's. this allows implicit close in kernel mode implementations to work. */
    for (i=0;i<sizeof(pVideo->displays)/sizeof(pVideo->displays[0]);i++) {
        if (pVideo->displays[i] && pVideo->displays[i] != display) break;
    }

    NEXUS_Display_P_UninitGraphics(display);

    /* must remove all outputs after destroying video windows and graphics because
    of possible required output. temporarily clear requiredOutput because we want
    to remove it here, but not wipe out the user settings. */
    {
    NEXUS_VideoOutput saveRequiredOutput = g_NEXUS_DisplayModule_State.requiredOutput;
    g_NEXUS_DisplayModule_State.requiredOutput = NULL;
    NEXUS_Display_RemoveAllOutputs(display);
    g_NEXUS_DisplayModule_State.requiredOutput = saveRequiredOutput;
    }

    BKNI_DestroyEvent(display->refreshRate.event);
    NEXUS_IsrCallback_Destroy(display->vsyncCallback.isrCallback);

    pVideo->displays[display->index] = NULL;

    NEXUS_OBJECT_DESTROY(NEXUS_Display, display);
    BKNI_Free(display);

    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Display, NEXUS_Display_Close);

NEXUS_Error
NEXUS_Display_AddOutput(NEXUS_DisplayHandle display, NEXUS_VideoOutput output)
{
  BSTD_UNUSED(display);
  BSTD_UNUSED(output);
  return BERR_SUCCESS;
}

NEXUS_Error
NEXUS_Display_RemoveOutput( NEXUS_DisplayHandle display, NEXUS_VideoOutput output)
{
  BSTD_UNUSED(display);
  BSTD_UNUSED(output);
  return BERR_SUCCESS;
}

void
NEXUS_Display_RemoveAllOutputs(NEXUS_DisplayHandle display)
{
    NEXUS_VideoOutput_P_Link *link;
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    /* can't assume list will be empty because of AddRequiredOutput_priv or RemoveOutput failure */
    for (link = BLST_D_FIRST(&display->outputs); link; ) {
        NEXUS_VideoOutput_P_Link *next = BLST_D_NEXT(link, link);
        BDBG_ASSERT(link->display == display);
        (void)NEXUS_Display_RemoveOutput( display, link->output);
        link = next;
    }
    return;
}

NEXUS_Error NEXUS_DisplayModule_AddRequiredOutput_priv(NEXUS_VideoOutput output)
{
  BSTD_UNUSED(output);
  return BERR_SUCCESS;
}

void NEXUS_DisplayModule_RemoveRequiredOutput_priv(NEXUS_VideoOutput output)
{
   BSTD_UNUSED(output);
   return;
}

void nexus_display_p_disconnect_outputs(NEXUS_DisplayHandle display, const NEXUS_DisplaySettings *pSettings, NEXUS_VideoFormat hdmiOutputFormat)
{
   BSTD_UNUSED(display);
   BSTD_UNUSED(pSettings);
   BSTD_UNUSED(hdmiOutputFormat);
   return;
}

void nexus_display_p_connect_outputs(NEXUS_DisplayHandle display)
{
  BSTD_UNUSED(display);
  return;
}


void
NEXUS_Display_GetSettings(NEXUS_DisplayHandle display, NEXUS_DisplaySettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    BDBG_ASSERT(pSettings);
    *pSettings = display->cfg;
    return;
}

NEXUS_Error
NEXUS_Display_SetSettings(NEXUS_DisplayHandle display, const NEXUS_DisplaySettings *pSettings)
{
  BSTD_UNUSED(display);
  BSTD_UNUSED(pSettings);
  return BERR_SUCCESS;
}

NEXUS_Error
NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode updateMode)
{
    return NEXUS_DisplayModule_SetUpdateMode_priv(updateMode, NULL);
}

NEXUS_Error NEXUS_DisplayModule_SetUpdateMode_priv( NEXUS_DisplayUpdateMode updateMode, NEXUS_DisplayUpdateMode *pPrevUpdateMode )
{
   BSTD_UNUSED(updateMode);
   BSTD_UNUSED(pPrevUpdateMode);
   return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Display_DriveVideoDecoder( NEXUS_DisplayHandle display )
{
  BSTD_UNUSED(display);
  return BERR_SUCCESS;
}

#include "bfmt_custom.h"

void NEXUS_Display_GetDefaultCustomFormatSettings(NEXUS_DisplayCustomFormatSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error NEXUS_Display_SetCustomFormatSettings( NEXUS_DisplayHandle display, NEXUS_VideoFormat format, const NEXUS_DisplayCustomFormatSettings *pSettings )
{
  BSTD_UNUSED(display);
  BSTD_UNUSED(format);
  BSTD_UNUSED(pSettings);
  return BERR_SUCCESS;
}

NEXUS_Error NEXUS_DisplayModule_SetAutomaticPictureQuality(void)
{
    /* no-op. we use to turn on de-interlacing, but that is now done with default values. */
    return 0;
}

NEXUS_Error NEXUS_Display_GetCrcData( NEXUS_DisplayHandle display, NEXUS_DisplayCrcData *pData, unsigned numEntries, unsigned *pNumEntriesReturned )
{
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    *pNumEntriesReturned = 0;
    /* Coverity: 36606, FORWARD_NULL */
    if (pData == NULL)
    {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return 0;
    }
    /* no critical section needed for this type of producer/consumer */
    while (*pNumEntriesReturned < numEntries && display->crc.wptr != display->crc.rptr && display->crc.queue) {
        pData[*pNumEntriesReturned] = display->crc.queue[display->crc.rptr];
        (*pNumEntriesReturned)++;
        if (++display->crc.rptr == display->crc.size) {
            display->crc.rptr = 0;
        }
    }
    return 0;
}

NEXUS_Error NEXUS_DisplayModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
}

void NEXUS_Display_GetStgSettings( NEXUS_DisplayHandle display, NEXUS_DisplayStgSettings *pSettings )
{
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    *pSettings = display->stgSettings;
}

NEXUS_Error NEXUS_Display_SetStgSettings( NEXUS_DisplayHandle display, const NEXUS_DisplayStgSettings *pSettings )
{
  BSTD_UNUSED(display);
  BSTD_UNUSED(pSettings);
  return 0;
}

NEXUS_Error NEXUS_Display_GetStatus( NEXUS_DisplayHandle display, NEXUS_DisplayStatus *pStatus )
{
   BSTD_UNUSED(display);
   BSTD_UNUSED(pStatus);
   return 0;
}

NEXUS_Error NEXUS_Display_SetVsyncCallback(NEXUS_DisplayHandle display, const NEXUS_CallbackDesc *pDesc )
{
  BSTD_UNUSED(display);
  BSTD_UNUSED(pDesc);
  return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Display_GetMaxMosaicCoverage( unsigned displayIndex, unsigned numMosaics, NEXUS_DisplayMaxMosaicCoverage *pCoverage )
{
   BSTD_UNUSED(displayIndex);
   BSTD_UNUSED(numMosaics);
   BSTD_UNUSED(pCoverage);
   return BERR_SUCCESS;
}
