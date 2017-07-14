/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#include "nexus_base.h"
#include "nexus_display_module.h"
#include "priv/nexus_display_priv.h"
#if NEXUS_HAS_TRANSPORT
#include "priv/nexus_timebase_priv.h"
#endif

BDBG_MODULE(nexus_display);
BDBG_FILE_MODULE(nexus_flow_display);

#define pVideo (&g_NEXUS_DisplayModule_State)
#define B_REFRESH_RATE_10_TO_1000(RATE) (((RATE) == 2397) ? 23976 : (RATE) * 10)

static NEXUS_Error NEXUS_Display_P_SetVsyncCallback(NEXUS_DisplayHandle display, bool enabled);
static void nexus_display_p_refresh_rate_event(void *context);
static void NEXUS_Display_P_UndriveVideoDecoder( NEXUS_DisplayHandle display );

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
    NEXUS_CallbackDesc_Init(&pSettings->vsyncCallback);
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
        const BFMT_VideoInfo *tmp;
        rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(videoFormat, &formatVdc);
        if (rc) {return BERR_TRACE(rc);}
        tmp = BFMT_GetVideoFormatInfoPtr_isr(formatVdc);
        if (tmp == NULL) {return BERR_TRACE(BERR_INVALID_PARAMETER);}
        *pInfo = *tmp;
    }
    return 0;
}

#if NEXUS_NUM_DISPLAYS > 1
NEXUS_Error NEXUS_Display_P_Align( NEXUS_DisplayHandle display, NEXUS_DisplayHandle target )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BERR_Code mrc = BERR_SUCCESS;
    BVDC_Display_AlignmentSettings sAlignSettings;
    BVDC_Display_Handle hTargetDisplay;

    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);

    BVDC_Display_GetAlignment(display->displayVdc, &hTargetDisplay, &sAlignSettings);

    if (target)
    {
        /* default slower alignment to keep video alive while align; Note if eAuto, bKeepBvnConnected may require bigger RTS to accelerate alignment */
        sAlignSettings.eDirection = BVDC_AlignmentDirection_eSlower;
        sAlignSettings.bKeepBvnConnected = true;
        hTargetDisplay = target->displayVdc;
        BDBG_MSG(("trigger display %p alignment to display %p",  (void *)display->displayVdc, (void *)hTargetDisplay));
    }
    else
    {
        hTargetDisplay = NULL;
        BDBG_MSG(("disable display %p alignment",  (void *)display->displayVdc));
    }

    mrc = BVDC_Display_SetAlignment(display->displayVdc, hTargetDisplay, &sAlignSettings);
    if (mrc!=BERR_SUCCESS) { rc = BERR_TRACE(mrc);goto end;}

end:

    return rc;
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
    BERR_Code rc;
    unsigned i;

    force = force || pVideo->lastUpdateFailed;
#if NEXUS_DBV_SUPPORT
    if (NEXUS_SUCCESS != NEXUS_Display_P_DbvFormatCheck(display, pSettings->format))
    {
        rc = BERR_NOT_SUPPORTED;
        rc = BERR_TRACE(rc);
        return rc;
    }
#endif

    /* NOTE: display->cfg are the old settings. pSettings are the new settings. always apply pSettings, not display->cfg. */

    if (force ||
                (display->cfg.format!=pSettings->format) ||  /* changes in the format */
                (display->cfg.display3DSettings.overrideOrientation != pSettings->display3DSettings.overrideOrientation) ||  /* changes in the overrideOrientation */
                (pSettings->display3DSettings.overrideOrientation && (display->cfg.display3DSettings.orientation != pSettings->display3DSettings.orientation)) /* changes in the orientation */
                ) {
        BFMT_VideoInfo video_format_info;
        NEXUS_Rect newDisplayRect = {0,0,0,0};

        rc = NEXUS_P_Display_GetMagnumVideoFormatInfo_isr(display, pSettings->format, &video_format_info);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_format; }

        newDisplayRect.width = video_format_info.ulDigitalWidth;
        if (pSettings->displayType == NEXUS_DisplayType_eBypass) {
            newDisplayRect.height = video_format_info.ulHeight;
        }
        else {
            newDisplayRect.height = video_format_info.ulDigitalHeight;
        }
        if(pSettings->display3DSettings.overrideOrientation) {
            if(pSettings->display3DSettings.orientation == NEXUS_VideoOrientation_e3D_LeftRight) {
                newDisplayRect.width /= 2;
            } else if(pSettings->display3DSettings.orientation == NEXUS_VideoOrientation_e3D_OverUnder) {
                newDisplayRect.height /= 2;
            }
        }

        /* if the display format dimensions change, we need to disable graphics. the current graphics will not look right. */
        if (!NEXUS_P_Display_RectEqual(&display->displayRect, &newDisplayRect)) {
            display->displayRect = newDisplayRect;
            NEXUS_Display_P_ResetGraphics(display); /* destroys graphics and resets the graphics settings with current displayRect */
        }

        if (pSettings->format != NEXUS_VideoFormat_eCustom2) {
            rc = BVDC_Display_SetVideoFormat(display->displayVdc, video_format_info.eVideoFmt);
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_setformat;}
        }
#if NEXUS_HAS_VBI && defined(MACROVISION_SUPPORT)
        nexus_p_check_macrovision(display, pSettings->format);
#endif
        /* actual display refresh rate should come from the display rate change callback */
        if(display->status.refreshRate == 0) {/* initialize if not updated by display callback */
            display->status.refreshRate = B_REFRESH_RATE_10_TO_1000(video_format_info.ulVertFreq);
        }
        display->formatChanged = true;
        BDBG_MODULE_MSG(nexus_flow_display, ("display %d format %dx%d%c %d.%02dhz (format %d)",
            display->index,
            display->displayRect.width,display->displayRect.height,video_format_info.bInterlaced?'i':'p',
            display->status.refreshRate/1000, display->status.refreshRate%1000,
            display->cfg.format));
    }

    if (force || pSettings->display3DSettings.overrideOrientation != display->cfg.display3DSettings.overrideOrientation ||
        pSettings->display3DSettings.orientation != display->cfg.display3DSettings.orientation)
    {
        rc = BVDC_Display_SetOrientation(
            display->displayVdc,
            NEXUS_P_VideoOrientation_ToMagnum_isrsafe(pSettings->display3DSettings.overrideOrientation ? pSettings->display3DSettings.orientation : NEXUS_VideoOrientation_e2D) /* if overrideOrientation is false, then we need to default back to 2D */
            );
        if(rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_setorientation;}
    }

    if (force || pSettings->display3DSettings.sourceBuffer != display->cfg.display3DSettings.sourceBuffer) {
        rc = BVDC_Display_Set3dSourceBufferSelect(display->displayVdc, (BVDC_3dSourceBufferSelect)pSettings->display3DSettings.sourceBuffer);
        if(rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_setorientation;}
    }

    /* SW7335-794 alignment needs to be done if forced, format changes, or target changes */
    if (force || display->cfg.format!=pSettings->format || display->cfg.alignmentTarget != pSettings->alignmentTarget) {
#if NEXUS_NUM_DISPLAYS > 1
        rc = NEXUS_Display_P_Align(display, pSettings->alignmentTarget);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_setformat;}
#endif
    }

#if NEXUS_HAS_TRANSPORT
    if(force || display->cfg.timebase!=pSettings->timebase) {
        unsigned timebaseIndex;
        rc = NEXUS_Timebase_GetIndex(pSettings->timebase, &timebaseIndex);
        if (rc) { rc = BERR_TRACE(rc); goto err_settimebase; }
        rc = BVDC_Display_SetTimebase(display->displayVdc, BAVC_Timebase_e0 + timebaseIndex);
        if (rc) { rc = BERR_TRACE(rc); goto err_settimebase; }
    }
#endif

    if(force || display->cfg.aspectRatio != pSettings->aspectRatio || display->cfg.format!=pSettings->format ||
        (pSettings->aspectRatio == NEXUS_DisplayAspectRatio_eSar &&
         (display->cfg.sampleAspectRatio.x != pSettings->sampleAspectRatio.x ||
          display->cfg.sampleAspectRatio.y != pSettings->sampleAspectRatio.y))) {
        BFMT_AspectRatio aspectRatioVdc;
        rc = NEXUS_P_DisplayAspectRatio_ToMagnum(pSettings->aspectRatio, pSettings->format, &aspectRatioVdc);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_aspectratio;}
        rc = BVDC_Display_SetAspectRatio(display->displayVdc, aspectRatioVdc);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_setaspectratio;}
        if (pSettings->aspectRatio == NEXUS_DisplayAspectRatio_eSar) {
            rc = BVDC_Display_SetSampleAspectRatio(display->displayVdc, pSettings->sampleAspectRatio.x, pSettings->sampleAspectRatio.y);
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_setaspectratio;}
        }
    }

    if(force || display->cfg.background != pSettings->background) {
        rc = BVDC_Compositor_SetBackgroundColor(display->compositor, B_GET_BITS(pSettings->background, 23, 16), B_GET_BITS(pSettings->background, 15, 8), B_GET_BITS(pSettings->background, 7, 0));
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_background;}
    }

    if(force || display->cfg.dropFrame != pSettings->dropFrame) {
        BVDC_Mode dropFrame;
        rc = NEXUS_P_DisplayTriState_ToMagnum(pSettings->dropFrame, &dropFrame);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_dropframe;}
        rc = BVDC_Display_SetDropFrame(display->displayVdc, dropFrame);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_dropframe;}
    }

    if(force || display->cfg.xvYccEnabled != pSettings->xvYccEnabled) {
        rc = BVDC_Display_SetHdmiXvYcc(display->displayVdc, pSettings->xvYccEnabled);
        if (rc) rc=BERR_TRACE(rc);
    }

    for(i=0;i<sizeof(display->windows)/sizeof(display->windows[0]);i++) {
        NEXUS_VideoWindowHandle window = &display->windows[i];
        if (window->open)
        {
            rc = NEXUS_VideoWindow_P_ConfigMasterFrameRate(window, pSettings, &window->cfg);
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_master;}
            BKNI_EnterCriticalSection();
            NEXUS_VideoWindow_P_UpdatePhaseDelay_isr(window, display->status.refreshRate);
            BKNI_LeaveCriticalSection();
        }
    }

    return BERR_SUCCESS;

err_master:
err_background:
err_dropframe:
#if NEXUS_HAS_TRANSPORT
err_settimebase:
#endif
err_aspectratio:
err_setaspectratio:
err_setorientation:
err_setformat:
err_format:
    return rc;
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

        if (picture.hLumaBlock == NULL) {
            if(!BLST_SQ_FIRST(&display->encoder.free)) {
                BDBG_WRN(("%d out of %d Buffers in use", NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS, NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS));
            }
            break;
        }

        BDBG_MSG(("L:%u ; C:%u ; W:%u ; H:%u ; P:%u ; PTS:%#x ; Lo:%#x ; Hi:%u ; ID: %u ; R:%u ; X:%u ; Y:%u",
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

#if((NEXUS_NUM_HDMI_OUTPUTS) && (BAVC_HDMI_CRC_READING_FIX))
    if (display->hdmi.vsync_isr) {
        /* general purpose per-vsync isr. one use is HDMI CRC capture. */
        (display->hdmi.vsync_isr)(display->hdmi.pCbParam);
    }
#endif
}

unsigned NEXUS_Display_GetLastVsyncTime_isr(NEXUS_DisplayHandle display)
{
    return display->lastVsyncTime;
}

/* nexus_p_install_display_cb is called on Open and SetSettings.
it is not evenly paired with nexus_p_uninstall_display_cb */
static BERR_Code nexus_p_install_display_cb(NEXUS_DisplayHandle display)
{
    BVDC_Display_CallbackSettings settings;
    BERR_Code rc;

    BVDC_Display_GetCallbackSettings(display->displayVdc, &settings);
    settings.stMask.bRateChange = 1;
    settings.stMask.bCrc = (display->cfg.crcQueueSize != 0);
    settings.stMask.bPerVsync = 1; /* needed for HDMI crc */
    settings.stMask.bCableDetect = 1;

#if NEXUS_HAS_VIDEO_ENCODER
    if( display->encodeUserData )
    {
        settings.stMask.bStgPictureId = 1;
        BDBG_MSG(("Display %u enables STG display callback.", display->index));
    }
#endif

    rc = BVDC_Display_SetCallbackSettings(display->displayVdc, &settings);
    if (rc) return BERR_TRACE(rc);

    if (display->crc.size != display->cfg.crcQueueSize) {
        void *new_ptr = NULL, *old_ptr;

        /* defer the free until after critical section */
        old_ptr = display->crc.queue;
        /* queue size of 1 is treated same as 0 because it can't hold anything */
        if (display->cfg.crcQueueSize > 1) {
            new_ptr = BKNI_Malloc(display->cfg.crcQueueSize * sizeof(NEXUS_DisplayCrcData));
            if (!new_ptr) {
                return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            }
        }

        /* must synchronize with ISR, so set state in CS */
        BKNI_EnterCriticalSection();
        display->crc.queue = new_ptr;
        display->crc.size = display->cfg.crcQueueSize>1?display->cfg.crcQueueSize:0;
        display->crc.wptr = display->crc.rptr = 0; /* flush */
        BKNI_LeaveCriticalSection();

        if (old_ptr) {
            BKNI_Free(old_ptr);
        }
    }

    rc = BVDC_Display_InstallCallback(display->displayVdc, (BVDC_CallbackFunc_isr)NEXUS_DisplayCb_isr, display, 0);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Display_P_SetVsyncCallback(display, true);

    return 0;
}

static void nexus_p_uninstall_display_cb(NEXUS_DisplayHandle display)
{
    BVDC_Display_CallbackSettings settings;
    BERR_Code rc;

    BVDC_Display_GetCallbackSettings(display->displayVdc, &settings);
    settings.stMask.bRateChange = 0;
    settings.stMask.bCrc = 0;
    rc = BVDC_Display_SetCallbackSettings(display->displayVdc, &settings);
    if (rc) rc = BERR_TRACE(rc);

    rc = BVDC_Display_InstallCallback(display->displayVdc, (BVDC_CallbackFunc_isr)NULL, NULL, 0);
    if (rc) rc = BERR_TRACE(rc);

    if (display->crc.queue) {
        void *old_ptr = display->crc.queue;
        BKNI_EnterCriticalSection();
        display->crc.queue = NULL;
        display->crc.size = 0;
        BKNI_LeaveCriticalSection();
        BKNI_Free(old_ptr);
    }

    NEXUS_Display_P_SetVsyncCallback(display, false);
}

static BERR_Code
NEXUS_Display_P_Open(NEXUS_DisplayHandle display, unsigned displayIndex, const NEXUS_DisplaySettings *pSettings)
{
    BERR_Code rc=BERR_SUCCESS;
    BVDC_DisplayId vdcDisplayId;
    BVDC_CompositorId vdcCmpId = BVDC_CompositorId_eCompositor0 + displayIndex;
    BVDC_Compositor_Settings vdcCmpSettings;
    bool bModifiedSync = (g_NEXUS_DisplayModule_State.moduleSettings.componentOutputSyncType == NEXUS_ComponentOutputSyncType_eAllChannels);
    BDBG_MODULE_MSG(nexus_flow_display, ("open %p, index %d, type %d",
        (void *)display, displayIndex, pSettings->displayType));

    BVDC_Compositor_GetDefaultSettings(vdcCmpId, &vdcCmpSettings);
    /* CFC LUT heap */
    if(NEXUS_MAX_HEAPS != pVideo->moduleSettings.cfc.cmpHeapIndex[displayIndex]) {
        vdcCmpSettings.hCfcHeap = g_pCoreHandles->heap[pVideo->moduleSettings.cfc.cmpHeapIndex[displayIndex]].mma;
    }

    if (pSettings->displayType == NEXUS_DisplayType_eDvo || pSettings->displayType == NEXUS_DisplayType_eLvds) {
        BVDC_Display_Settings vdcDisplayCfg;

        if (displayIndex != 0) {
            BDBG_ERR(("invalid dvo display"));
            goto err_compositor;
        }
        rc = BVDC_Compositor_Create(pVideo->vdc, &display->compositor, vdcCmpId, &vdcCmpSettings);
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

        rc = BVDC_Compositor_Create(pVideo->vdc, &display->compositor, vdcCmpId, &vdcCmpSettings);
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
        rc = BVDC_Compositor_Create(pVideo->vdc, &display->compositor, vdcCmpId, &vdcCmpSettings);
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
#elif NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT /* DSPs with VIP */
        /* encoder timing generator override */
        if (timingGenerator == NEXUS_DisplayTimingGenerator_eAuto || timingGenerator == NEXUS_DisplayTimingGenerator_eEncoder) {
            if (g_pCoreHandles->boxConfig->stVdc.astDisplay[display->index].stStgEnc.bAvailable) {
                display->stgIndex = g_pCoreHandles->boxConfig->stVdc.astDisplay[display->index].stStgEnc.ulStgId;
                vdcDisplayCfg.eMasterTg = BVDC_DisplayTg_eStg0 + display->stgIndex;
            }
        }
#endif
        if( vdcDisplayCfg.eMasterTg >=BVDC_DisplayTg_eStg0 && vdcDisplayCfg.eMasterTg <=BVDC_DisplayTg_eStg5) {
            display->timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
            display->stgSettings.enabled = true;
        }
        else {
            display->timingGenerator = vdcDisplayCfg.eMasterTg;
        }
        BDBG_MSG(("display->stgIndex = %d; display->timingGenerator = %u", display->stgIndex, display->timingGenerator));

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

    if (displayIndex >= NEXUS_NUM_DISPLAYS || \
        ((pVideo->cap.display[displayIndex].numVideoWindows == 0) && (!pVideo->cap.display[displayIndex].graphics.width))) {
        BDBG_ERR(("display[%d] cannot be opened!", displayIndex));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    if(pSettings==NULL) {
        NEXUS_Display_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    if(displayIndex>=sizeof(pVideo->displays)/sizeof(pVideo->displays[0])) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_settings;
    }

    nexus_display_p_init_rdccapture();

#if BVDC_BUF_LOG && NEXUS_BASE_OS_linuxuser
    if (nexus_display_p_init_buflogcapture() != BERR_SUCCESS) {
        goto err_alloc;
    }
#endif

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
    display->refreshRate.handler = NEXUS_RegisterEvent(display->refreshRate.event, nexus_display_p_refresh_rate_event, display);
    if (!display->refreshRate.handler) {rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_registerrefreshrate;}

#if NEXUS_HAS_HDMI_OUTPUT
    BDBG_ASSERT(NULL == display->hdmi.rateChangeCb_isr);
    NEXUS_CallbackHandler_Init(display->hdmi.outputNotifyDisplay,NEXUS_VideoOutput_P_SetHdmiSettings, display );
    display->hdmi.outputNotify = NULL;
#endif

    rc = NEXUS_Display_P_Open(display, displayIndex, pSettings);
    if (rc) {rc = BERR_TRACE(rc); goto err_open;}

    rc = nexus_p_install_display_cb(display);
    if (rc) {rc = BERR_TRACE(rc); goto err_install_display_cb;}

    for(i=0;i<sizeof(display->windows)/sizeof(display->windows[0]);i++) {
        display->windows[i].open = false;
    }

    NEXUS_Display_P_InitGraphics(display);

    rc = NEXUS_Display_P_SetSettings(display, pSettings, true);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_display_set;}

    BDBG_MSG(("First Display VDC ApplyChanges [display=%p]", (void *)display));
    if(pVideo->updateMode != NEXUS_DisplayUpdateMode_eAuto) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);}
    rc = BVDC_ApplyChanges(pVideo->vdc);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_applychanges;}

#if NEXUS_VBI_SUPPORT
    rc = NEXUS_Display_P_ConnectVbi(display);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_vbi;}
#endif

#if NEXUS_HAS_VIDEO_ENCODER
    BXUDlib_Create(&display->hXud, NULL);
    /* TODO: error checking */
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
    /*Initialize the capture buffer pointer cache */
    {
    NEXUS_Display_P_Image *image;

    BLST_SQ_INIT(&display->encoder.free);
    BLST_SQ_INIT(&display->encoder.queued);
    for(i=0; i<NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS; i++){
        image = BKNI_Malloc(sizeof(*image));
        /* TODO: error checking */
        BLST_SQ_INSERT_TAIL(&display->encoder.free, image, link);
    }
    display->encoder.framesEnqueued = 0;
    }
#endif
#endif
    pVideo->displays[display->index] = display;

    if (displayIndex == 0 && g_NEXUS_DisplayModule_State.requiredOutput) {
        NEXUS_VideoOutput output = g_NEXUS_DisplayModule_State.requiredOutput;
        g_NEXUS_DisplayModule_State.requiredOutput = NULL;
        rc = NEXUS_Display_AddOutput(display, output);
        g_NEXUS_DisplayModule_State.requiredOutput = output;
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_addrequiredoutput;}
    }

    return display;

err_addrequiredoutput:
    NEXUS_Display_Close(display);
    return NULL;
#if NEXUS_VBI_SUPPORT
err_vbi:
#endif
err_applychanges:
err_display_set:
    NEXUS_Display_P_UninitGraphics(display);
    nexus_p_uninstall_display_cb(display);
err_install_display_cb:
    /* TODO: NEXUS_Display_P_Close */
    rc = BVDC_Display_Destroy(display->displayVdc);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    rc = BVDC_Compositor_Destroy(display->compositor);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    rc = BVDC_AbortChanges(pVideo->vdc);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
    rc = BVDC_ApplyChanges(pVideo->vdc); /* an Apply is needed after the Destroy */
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
err_open:
    NEXUS_UnregisterEvent(display->refreshRate.handler);
#if NEXUS_HAS_HDMI_OUTPUT
    NEXUS_CallbackHandler_Shutdown(display->hdmi.outputNotifyDisplay);
#endif
err_registerrefreshrate:
    BKNI_DestroyEvent(display->refreshRate.event);
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
    unsigned i;
    BDBG_MODULE_MSG(nexus_flow_display, ("close %p", (void *)display));
    for(i=0;i<sizeof(display->windows)/sizeof(display->windows[0]);i++) {
        NEXUS_VideoWindowHandle window = &display->windows[i];
        if (window->open) {
            BDBG_WRN(("NEXUS_Display_Close is automatically closing window %p. Application must explicitly close the handle.", (void *)window));
            NEXUS_VideoWindow_Close(window);
        }
    }
    return;
}

static void
NEXUS_Display_P_Finalizer(NEXUS_DisplayHandle display)
{
    BERR_Code rc;
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);

    if (display->index >= sizeof(pVideo->displays)/sizeof(pVideo->displays[0])) {
        /* adding runtime check for static analysis */
        return;
    }

    nexus_display_p_uninit_rdccapture();

#if BVDC_BUF_LOG && NEXUS_BASE_OS_linuxuser
    nexus_display_p_uninit_buflogcapture();
#endif

    /* stop all callbacks from coming into display module */
    nexus_p_uninstall_display_cb(display);
    NEXUS_UnregisterEvent(display->refreshRate.handler);

#if NEXUS_HAS_VIDEO_ENCODER
    BXUDlib_Destroy(display->hXud);
#endif

#if NEXUS_VBI_SUPPORT
    if (display->vbi.settings.vbiSource) {
        NEXUS_DisplayVbiSettings vbiSettings = display->vbi.settings;
        vbiSettings.vbiSource = NULL;
        NEXUS_Display_SetVbiSettings(display, &vbiSettings);
    }
#endif

    /* if we close the last display, then shutdown all cached VideoInput's. this allows implicit close in kernel mode implementations to work. */
    for (i=0;i<sizeof(pVideo->displays)/sizeof(pVideo->displays[0]);i++) {
        if (pVideo->displays[i] && pVideo->displays[i] != display) break;
    }
    if (i == sizeof(pVideo->displays)/sizeof(pVideo->displays[0])) {
        NEXUS_VideoInput_P_Link *inputLink;

        while(NULL!=(inputLink=BLST_S_FIRST(&pVideo->inputs))) {
            BDBG_ASSERT(inputLink->input);
            NEXUS_VideoInput_Shutdown(inputLink->input);
        }
    }

    NEXUS_Display_P_UndriveVideoDecoder(display);

#if NEXUS_VBI_SUPPORT
    NEXUS_Display_P_DisconnectVbi(display);
#endif

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

    rc = BVDC_Display_Destroy(display->displayVdc);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    rc = BVDC_Compositor_Destroy(display->compositor);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    if(pVideo->updateMode != NEXUS_DisplayUpdateMode_eAuto) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);}
    rc = BVDC_ApplyChanges(pVideo->vdc);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    BKNI_DestroyEvent(display->refreshRate.event);
    NEXUS_IsrCallback_Destroy(display->vsyncCallback.isrCallback);

    if (display->customFormatInfo) {
        if (display->customFormatInfo->pCustomInfo) {
            BKNI_Free(display->customFormatInfo->pCustomInfo->pDvoRmTbl1);
            BKNI_Free(display->customFormatInfo->pCustomInfo->pDvoRmTbl0);
            BKNI_Free(display->customFormatInfo->pCustomInfo->pDvoMicrocodeTbl);
            BKNI_Free(display->customFormatInfo->pCustomInfo);
        }
        BKNI_Free(display->customFormatInfo);
        display->customFormatInfo = NULL;
    }
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_CallbackHandler_Shutdown(display->hdmi.outputNotifyDisplay);
#endif
#if NEXUS_HAS_VIDEO_ENCODER && NEXUS_NUM_DSP_VIDEO_ENCODERS
    {
    NEXUS_Display_P_Image *image;
    while(NULL != (image=BLST_SQ_FIRST(&display->encoder.free))){
        BLST_SQ_REMOVE_HEAD(&display->encoder.free, link);
        BKNI_Free(image);
    }
    while(NULL != (image=BLST_SQ_FIRST(&display->encoder.queued))){
        BLST_SQ_REMOVE_HEAD(&display->encoder.queued, link);
        BKNI_Free(image);
    }
    }
#endif

    pVideo->displays[display->index] = NULL;

    NEXUS_OBJECT_DESTROY(NEXUS_Display, display);
    BKNI_Free(display);

    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Display, NEXUS_Display_Close);

NEXUS_Error
NEXUS_Display_AddOutput(NEXUS_DisplayHandle display, NEXUS_VideoOutput output)
{
    BERR_Code rc, cleanup_rc;
    NEXUS_VideoOutput_P_Link *link;

    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);
    BDBG_ASSERT(output->source);
    if (output == g_NEXUS_DisplayModule_State.requiredOutput) {
        return NEXUS_SUCCESS;
    }
    if(output->destination) {
        BDBG_ERR(("This output is already connected to a display."));
        rc = BERR_TRACE(NEXUS_DISPLAY_ADD_OUTPUT_ALREADY_CONNECTED);
        goto err_already_connected;
    }
    link = NEXUS_P_VideoOutput_Link(output);
    if(!link) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); /* unable to create link */
        goto err_link;
    }

    link->display = display;

    rc = nexus_videooutput_p_connect(link);
    if (rc) {rc = BERR_TRACE(rc); goto err_connect;}

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) {rc = BERR_TRACE(rc); goto err_apply;}

    BLST_D_INSERT_HEAD(&display->outputs, link, link);
    return BERR_SUCCESS;

err_apply:
    nexus_videooutput_p_disconnect(link);
err_connect:
    link->display = NULL;
    output->destination = NULL; /* unlink */
    cleanup_rc = BVDC_AbortChanges(pVideo->vdc);
    if(cleanup_rc!=BERR_SUCCESS) { cleanup_rc = BERR_TRACE(cleanup_rc); }
err_link:
    if (link) {NEXUS_VideoOutput_P_DestroyLink(link);}
err_already_connected:
    return rc;
}

NEXUS_Error
NEXUS_Display_RemoveOutput( NEXUS_DisplayHandle display, NEXUS_VideoOutput output)
{
    BERR_Code rc;
    NEXUS_VideoOutput_P_Link *link;

    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);
    BDBG_ASSERT(output->source);
    if (output == g_NEXUS_DisplayModule_State.requiredOutput) {
        BDBG_WRN(("NEXUS_Display_RemoveOutput(%p,%p) skipped because of requiredOutput setting", (void*)display, (void*)output));
        return NEXUS_SUCCESS;
    }
    if(output->destination == NULL) {
        rc = BERR_TRACE(NEXUS_DISPLAY_REMOVE_OUTPUT_NOT_CONNECTED);
        goto err_already_disconnected;
    }
    link = output->destination;
    BDBG_OBJECT_ASSERT(link, NEXUS_VideoOutput_P_Link);
    if(link->display!=display) {
        BDBG_ERR(("This output is not connected to this display."));
        rc = BERR_TRACE(NEXUS_DISPLAY_REMOVE_OUTPUT_WRONG_CONNECTION);
        goto err_already_disconnected;
    }
    BLST_D_REMOVE(&display->outputs, link, link); /* remove it from list right the way */
    link->display = NULL;
    /* ignore failure on disconnect. We must clean up state anyway. VDC may fail on BKNI_WaitForEvent if a signal is pending. */
    (void)link->iface.disconnect(output->source, display);
    output->destination = NULL;

    /* deallocate the link too */
    NEXUS_VideoOutput_P_DestroyLink(link);

    /* even if VDC fails, we have removed from the list, so we must succeed */
    (void)NEXUS_Display_P_ApplyChanges();

    return BERR_SUCCESS;

err_already_disconnected:
    return rc;
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

#if NEXUS_DISPLAY_OPEN_REQUIRES_HDMI_OUTPUT
NEXUS_Error NEXUS_DisplayModule_AddRequiredOutput_priv(NEXUS_VideoOutput output)
{
    NEXUS_ASSERT_MODULE();
    if (!g_NEXUS_DisplayModule_State.requiredOutput) {
        g_NEXUS_DisplayModule_State.requiredOutput = output;
        g_NEXUS_DisplayModule_State.requiredOutputSystem = true;
        return 0;
    }
    else {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
}

void NEXUS_DisplayModule_RemoveRequiredOutput_priv(NEXUS_VideoOutput output)
{
    NEXUS_ASSERT_MODULE();
    if (output == g_NEXUS_DisplayModule_State.requiredOutput) {
        g_NEXUS_DisplayModule_State.requiredOutput = NULL;
        g_NEXUS_DisplayModule_State.requiredOutputSystem = false;
    }
    else {
        BERR_TRACE(NEXUS_UNKNOWN);
    }
}
#endif

static bool
NEXUS_P_Display_RectEqual3d(const NEXUS_Rect *winRect, const NEXUS_Rect *dispRect, NEXUS_VideoOrientation orientation)
{
    BDBG_ASSERT(winRect);
    BDBG_ASSERT(dispRect);

    if (orientation==NEXUS_VideoOrientation_e3D_LeftRight) {
        return winRect->width == dispRect->width/2 && winRect->height == dispRect->height;
    }
    else if (orientation==NEXUS_VideoOrientation_e3D_OverUnder) {
        return winRect->width == dispRect->width && winRect->height == dispRect->height/2;
    }

    return false;
}

void nexus_display_p_disconnect_outputs(NEXUS_DisplayHandle display, const NEXUS_DisplaySettings *pSettings, NEXUS_VideoFormat hdmiOutputFormat)
{
    NEXUS_VideoOutput_P_Link *output;
    for(output=BLST_D_FIRST(&display->outputs);output;output=BLST_D_NEXT(output, link)) {
        if (nexus_p_bypass_video_output_connect(output, pSettings, hdmiOutputFormat)) {
            nexus_videooutput_p_disconnect(output);
        }
    }
}

void nexus_display_p_connect_outputs(NEXUS_DisplayHandle display)
{
    NEXUS_VideoOutput_P_Link *output;
    for(output=BLST_D_FIRST(&display->outputs);output;output=BLST_D_NEXT(output, link)) {
        (void)nexus_videooutput_p_connect(output);
    }
}

bool NEXUS_Display_P_HasOutput_isr(NEXUS_DisplayHandle display, NEXUS_VideoOutputType type)
{
    bool hasHdmi = false;
    NEXUS_VideoOutput_P_Link *output;

    for(output=BLST_D_FIRST(&display->outputs);output;output=BLST_D_NEXT(output, link)) {
        if (output->output->type == type)
        {
            hasHdmi = true;
            break;
        }
    }

    return hasHdmi;
}

void
NEXUS_Display_GetSettings(NEXUS_DisplayHandle display, NEXUS_DisplaySettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    BDBG_ASSERT(pSettings);
    *pSettings = display->cfg;
    return;
}

static void nexus_p_max_window_size(NEXUS_DisplayHandle display, unsigned window, unsigned *pWidth, unsigned *pHeight)
{
    *pWidth = display->displayRect.width;
    *pHeight = display->displayRect.height;
    if (g_pCoreHandles->boxConfig->stBox.ulBoxId) {
        const BBOX_Vdc_WindowSizeLimits *pSizeLimits =
           &(g_pCoreHandles->boxConfig->stVdc.astDisplay[display->index].astWindow[window].stSizeLimits);
        if(BBOX_VDC_DISREGARD != pSizeLimits->ulHeightFraction) {
            *pWidth = *pWidth/pSizeLimits->ulWidthFraction;
            *pHeight = *pHeight/pSizeLimits->ulHeightFraction;
        }
    }
}

NEXUS_Error
NEXUS_Display_SetSettings(NEXUS_DisplayHandle display, const NEXUS_DisplaySettings *pSettings)
{
    BERR_Code rc;
    NEXUS_VideoOutput_P_Link *output;
    NEXUS_Rect prevDisplayRect;
    NEXUS_DisplaySettings previousSettings;
    bool formatChanged = false;

    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    BDBG_ASSERT(pSettings);

    prevDisplayRect = display->displayRect;
    if(pSettings->format != display->cfg.format) {
        formatChanged = true;
#if NEXUS_VBI_SUPPORT
        NEXUS_Display_P_DisableVbi(display);
#endif
        nexus_display_p_disconnect_outputs(display, pSettings, display->hdmi.outputFormat);
    }
    rc = NEXUS_Display_P_SetSettings(display, pSettings, false);
    if(rc!=BERR_SUCCESS) { goto err_setsettings; }

    if ((pSettings->format!=display->cfg.format) ||
        (pSettings->display3DSettings.overrideOrientation != display->cfg.display3DSettings.overrideOrientation) ||
        (pSettings->display3DSettings.orientation != display->cfg.display3DSettings.orientation))
    {
        unsigned i;
        for(i=0;i<sizeof(display->windows)/sizeof(display->windows[0]);i++) {
            NEXUS_VideoWindowHandle window = &display->windows[i];
            unsigned maxWidth, maxHeight;
            /* resize window */
            if (!window->open) continue;
            nexus_p_max_window_size(display, i, &maxWidth, &maxHeight);
            if (NEXUS_P_Display_RectEqual(&window->cfg.position, &prevDisplayRect) || /* preserve fullscreen */
                (window->cfg.position.width > maxWidth) || (window->cfg.position.height > maxHeight) || /* don't exceed box mode limit */
                (window->cfg.position.x + window->cfg.position.width > display->displayRect.width) || /* window will exceed bounds of new display */
                (window->cfg.position.y + window->cfg.position.height > display->displayRect.height+2) || /* The +2 on height is needed to account for NTSC as 482 or 480 */
                NEXUS_P_Display_RectEqual3d(&window->cfg.position, &prevDisplayRect,
                    pSettings->display3DSettings.overrideOrientation?display->cfg.display3DSettings.orientation:NEXUS_VideoOrientation_e2D)) /* preserve fullscreen for halfres 3D formats on 40nm BVN */
            {
                /* the simple solution is to put the window at 0,0,maxWidth,maxHeight for this display.
                likely the app will want to reposition the window if the display size changes. */
                window->cfg.position.x = 0;
                window->cfg.position.y = 0;
                window->cfg.position.width = maxWidth;
                window->cfg.position.height = maxHeight;

                if(window->vdcState.window) {
                    rc = NEXUS_VideoWindow_P_SetVdcSettings(window, &window->cfg, true);
                    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowsettings;}
                }
#if NEXUS_NUM_MOSAIC_DECODES
                if (BLST_S_FIRST(&window->mosaic.children)) {
                    NEXUS_VideoWindowHandle mosaicChild;
                    /* mosaics cannot exceed parent window. the preceding code assumes app will reposition window, so
                    just park mosaic size and position to safe spot until that is done. */
                    for (mosaicChild = BLST_S_FIRST(&window->mosaic.children);mosaicChild;mosaicChild = BLST_S_NEXT(mosaicChild, mosaic.link)) {
                        mosaicChild->cfg.position.x = 0;
                        mosaicChild->cfg.position.y = 0;
                        mosaicChild->cfg.position.width = 100;
                        mosaicChild->cfg.position.height = 100;
                    }
                    NEXUS_VideoWindow_P_ApplyMosaic(window);
                }
#endif
            }
            if(window->input) {
                NEXUS_Display_P_VideoInputDisplayUpdate(NULL, window, pSettings);
            }
        }
    }

    {
        BFMT_AspectRatio beforeAR, afterAR;
        rc = NEXUS_P_DisplayAspectRatio_ToMagnum(display->cfg.aspectRatio, display->cfg.format, &beforeAR);
        if (!rc) {
            rc = NEXUS_P_DisplayAspectRatio_ToMagnum(pSettings->aspectRatio, pSettings->format, &afterAR);
        }

        if ( !rc)
        {
            bool formatChange, aspectRatioChange, _3dOrientationChange ;

            formatChange = pSettings->format != display->cfg.format ;
            aspectRatioChange = beforeAR != afterAR;
            _3dOrientationChange =
                (pSettings->display3DSettings.overrideOrientation != display->cfg.display3DSettings.overrideOrientation)
             || (pSettings->display3DSettings.orientation != display->cfg.display3DSettings.orientation) ;

            if (formatChange || aspectRatioChange || _3dOrientationChange)
            {
                for( output=BLST_D_FIRST(&display->outputs); NULL != output; output=BLST_D_NEXT(output, link) )
                {
                     if( output->iface.formatChange )
                     {
                         /* Update format to outputs that require knowledge (e.g. HDMI) */
                         rc = output->iface.formatChange(output->output->source, display, pSettings->format, pSettings->aspectRatio, _3dOrientationChange);
                         if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);}
                     }
                }
            }
        }
    }

    /* keep a copy of the current settings */
    previousSettings = display->cfg ;
    display->cfg = *pSettings;

    rc = nexus_p_install_display_cb(display);
    if (rc) {rc = BERR_TRACE(rc); goto err_install_display_cb;}

#if NEXUS_VBI_SUPPORT
    rc = NEXUS_Display_P_EnableVbi(display, pSettings->format);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_vbi; }
#endif

    /* PR:54526 must get sync notification after window has been configured */
    if (formatChanged)
    {
        unsigned int i;
        for(i=0;i<sizeof(display->windows)/sizeof(display->windows[0]);i++) {
            NEXUS_VideoWindowHandle window = &display->windows[i];
            if (!window->open) continue;
            if (window->syncSettings.formatCallback_isr) {
                BKNI_EnterCriticalSection();
                (*window->syncSettings.formatCallback_isr)(window->syncSettings.callbackContext, 0);
                BKNI_LeaveCriticalSection();
            }
        }
        nexus_display_p_connect_outputs(display);
    }

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) {rc = BERR_TRACE(rc);goto err_applychanges;}
#if NEXUS_VBI_SUPPORT
err_vbi: /* ignore VBI errors */
#endif
    return BERR_SUCCESS;

err_applychanges:
err_install_display_cb:
    /* restore previous settings */
    display->cfg = previousSettings ;

    /* fall through to unwind VDC changes */

err_setsettings:
err_windowsettings:
    {
        BERR_Code rc = BVDC_AbortChanges(pVideo->vdc);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    }
    return rc;
}

NEXUS_Error
NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode updateMode)
{
    return NEXUS_DisplayModule_SetUpdateMode_priv(updateMode, NULL);
}

NEXUS_Error NEXUS_DisplayModule_SetUpdateMode_priv( NEXUS_DisplayUpdateMode updateMode, NEXUS_DisplayUpdateMode *pPrevUpdateMode )
{
    NEXUS_Error rc;

    NEXUS_ASSERT_MODULE();
    if (pPrevUpdateMode) {
        *pPrevUpdateMode = pVideo->updateMode;
    }
    switch(updateMode) {
    case NEXUS_DisplayUpdateMode_eManual:
        pVideo->updateMode = updateMode;
        break;
   case NEXUS_DisplayUpdateMode_eAuto:
        pVideo->updateMode = updateMode;
        /* keep going */
   case NEXUS_DisplayUpdateMode_eNow:
        rc = BVDC_ApplyChanges(pVideo->vdc);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_applychanges; }
        pVideo->lastUpdateFailed = false;
    }
    return NEXUS_SUCCESS;

err_applychanges:
    pVideo->lastUpdateFailed = true;
    {
        BERR_Code rc = BVDC_AbortChanges(pVideo->vdc);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    }
    return rc;
}

#if NEXUS_HAS_VIDEO_DECODER
static void
NEXUS_Display_P_DecoderDataReady_isr(void *context, const BAVC_MFD_Picture *picture)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(picture);
    return;
}
#endif

#if NEXUS_HAS_VIDEO_ENCODER
NEXUS_Error
NEXUS_DisplayModule_SetUserDataEncodeMode_priv(NEXUS_DisplayHandle display, bool encodeUserData, BXUDlib_Settings *userDataEncodeSettings, NEXUS_VideoWindowHandle udWindow)
{
    NEXUS_Error rc = BERR_SUCCESS;
    unsigned i;

    BXUDlib_Settings *pXudSettings = (BXUDlib_Settings *)userDataEncodeSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    display->encodeUserData = encodeUserData;
    if(encodeUserData) {
        display->userDataEncodeSettings = *pXudSettings;
        BXUDlib_SetSettings(display->hXud, &display->userDataEncodeSettings);
    }
    if (udWindow && udWindow->open && udWindow->input)
    {
        display->xudSource = encodeUserData? udWindow->input:NULL;
        NEXUS_Display_P_VideoInputDisplayUpdate(NULL, udWindow, &display->cfg);
    }
    else
    {
        for(i=0;i<sizeof(display->windows)/sizeof(display->windows[0]);i++) {
            NEXUS_VideoWindowHandle window = &display->windows[i];
            if (!window->open) continue;
            if(window->input) {/* first match takes it for now */
                display->xudSource = encodeUserData? window->input:NULL;
                NEXUS_Display_P_VideoInputDisplayUpdate(NULL, window, &display->cfg);
                break;
            }
        }
    }

    rc = nexus_p_install_display_cb(display);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_installcb; }
    rc = NEXUS_Display_P_ApplyChanges();
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_applychanges; }

err_installcb:
err_applychanges:

    return rc;
}

NEXUS_Error
NEXUS_DisplayModule_SetBarDataEncodeMode_priv(NEXUS_DisplayHandle display, bool encodeBarData)
{
    NEXUS_Error rc = BERR_SUCCESS;

    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    if(!encodeBarData) {
        /* It's a switch for now. TODO: add option to override display bar data type and values; */
        rc = BVDC_Display_SetBarData(display->displayVdc, BVDC_Mode_eOff, BAVC_BarDataType_eInvalid, 0, 0);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_applychanges; }
        rc = NEXUS_Display_P_ApplyChanges();
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_applychanges; }
    }/* by default, display will send bar data */

err_applychanges:
    return rc;
}
#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
NEXUS_Error
NEXUS_DisplayModule_SetStgResolutionRamp_priv(
    NEXUS_DisplayHandle display,
    unsigned rampFrameCount
    )
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    rc = BVDC_Display_RampStgResolution(display->displayVdc, rampFrameCount);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_ramp; }
    rc = NEXUS_Display_P_ApplyChanges();
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_applychanges; }
err_ramp:
err_applychanges:
    return rc;
}
#endif
#endif

NEXUS_Error NEXUS_Display_DriveVideoDecoder( NEXUS_DisplayHandle display )
{
#if NEXUS_HAS_VIDEO_DECODER
    BERR_Code rc;
    NEXUS_VideoDecoderDisplayConnection decoderConnect;

    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);

    /* similar to NEXUS_VideoInput_P_Connect, but uses BVDC_Display_GetInterruptName for interrupts.
    also lacks protection against duplicate connections, etc. app must be written right. */
    if (!pVideo->modules.videoDecoder) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    NEXUS_Module_Lock(pVideo->modules.videoDecoder);
    NEXUS_VideoDecoder_GetDefaultDisplayConnection_priv(&decoderConnect);
    NEXUS_Module_Unlock(pVideo->modules.videoDecoder);

    rc = BVDC_Display_GetInterruptName(display->displayVdc, BAVC_Polarity_eTopField, &decoderConnect.top.intId);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_intr_name;}
    rc = BVDC_Display_GetInterruptName(display->displayVdc, BAVC_Polarity_eBotField, &decoderConnect.bottom.intId);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_intr_name;}
    rc = BVDC_Display_GetInterruptName(display->displayVdc, BAVC_Polarity_eFrame, &decoderConnect.frame.intId);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_intr_name;}
    decoderConnect.callbackContext = NULL;
    decoderConnect.dataReadyCallback_isr = NEXUS_Display_P_DecoderDataReady_isr;

    NEXUS_Module_Lock(pVideo->modules.videoDecoder);
    rc = NEXUS_VideoDecoder_SetDefaultDisplayConnection_priv(&decoderConnect);
    NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_connect;}

    pVideo->displayDrivingVideoDecoder = display;
    NEXUS_Display_P_VideoInputDisplayUpdate(NULL, NULL, &display->cfg);

    return 0;

err_connect:
err_intr_name:
    return rc;
#else
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

static void NEXUS_Display_P_UndriveVideoDecoder( NEXUS_DisplayHandle display )
{
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoDecoderDisplayConnection decoderConnect;
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    if (pVideo->displayDrivingVideoDecoder != display) return;
    NEXUS_Module_Lock(pVideo->modules.videoDecoder);
    NEXUS_VideoDecoder_GetDefaultDisplayConnection_priv(&decoderConnect);
    (void)NEXUS_VideoDecoder_SetDefaultDisplayConnection_priv(&decoderConnect);
    NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
    pVideo->displayDrivingVideoDecoder = NULL;
#else
    BSTD_UNUSED(display);
#endif
}

static void NEXUS_Display_P_VecInterrupt_isr(void *context, int param)
{
    BSTD_UNUSED(param);
    NEXUS_IsrCallback_Fire_isr(((NEXUS_DisplayHandle)context)->vsyncCallback.isrCallback);
}

static NEXUS_Error NEXUS_Display_P_SetVsyncCallback(NEXUS_DisplayHandle display, bool enabled)
{
    const NEXUS_CallbackDesc *pDesc;
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    /* NEXUS_Display_SetVsyncCallback takes predecence over Settings.vsyncCallback */
    pDesc = display->vsyncCallback.desc.callback?&display->vsyncCallback.desc:&display->cfg.vsyncCallback;
    NEXUS_IsrCallback_Set(display->vsyncCallback.isrCallback, pDesc);

    if (!enabled || !pDesc->callback) {
        unsigned i;
        for (i=0;i<3;i++) {
            if (display->vsyncCallback.intCallback[i]) {
                BINT_DestroyCallback(display->vsyncCallback.intCallback[i]);
                display->vsyncCallback.intCallback[i] = NULL;
            }
        }
    }
    else {
        unsigned i;
        for (i=0;i<3;i++) {
            if (!display->vsyncCallback.intCallback[i]) {
                BERR_Code rc;
                BINT_Id intId;
                BAVC_Polarity pol = i==0?BAVC_Polarity_eTopField:i==1?BAVC_Polarity_eBotField:BAVC_Polarity_eFrame;

                rc = BVDC_Display_GetInterruptName(display->displayVdc, pol, &intId);
                if (rc) return BERR_TRACE(rc);

                rc = BINT_CreateCallback(&display->vsyncCallback.intCallback[i], g_pCoreHandles->bint, intId, NEXUS_Display_P_VecInterrupt_isr, display, 0);
                if (rc) return BERR_TRACE(rc);
                rc = BINT_EnableCallback(display->vsyncCallback.intCallback[i]);
                if (rc) return BERR_TRACE(rc);
            }
        }
    }

    return 0;
}

NEXUS_Error NEXUS_Display_SetVsyncCallback(NEXUS_DisplayHandle display, const NEXUS_CallbackDesc *pDesc )
{
    if (pDesc) {
        display->vsyncCallback.desc = *pDesc;
    }
    else {
        display->vsyncCallback.desc.callback = NULL;
    }
    return NEXUS_Display_P_SetVsyncCallback(display, true);
}

#include "bfmt_custom.h"

void NEXUS_Display_GetDefaultCustomFormatSettings(NEXUS_DisplayCustomFormatSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error NEXUS_Display_SetCustomFormatSettings( NEXUS_DisplayHandle display, NEXUS_VideoFormat format, const NEXUS_DisplayCustomFormatSettings *pSettings )
{
    BERR_Code rc;
    BFMT_AspectRatio magnumAspectRatio;
    BFMT_VideoFmt magnumVideoFormat;
    NEXUS_DisplaySettings settings = display->cfg;
    NEXUS_VideoFormatInfo nexusVideoFormatInfo;

    if (format != NEXUS_VideoFormat_eCustom2) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    if(pVideo->updateMode != NEXUS_DisplayUpdateMode_eAuto) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);}

    if (!pSettings) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    /* allocate memory. VDC expects the custom BFMT_VideoInfo to persist outside of VDC. */
    if (!display->customFormatInfo) {
        /* allocate the data structure and all its linked substructures */
        display->customFormatInfo = BKNI_Malloc(sizeof(BFMT_VideoInfo));
        display->customFormatInfo->pCustomInfo = NULL; /* no custom info */
        /* this memory is freed in NEXUS_Display_Close */
    }

    rc = NEXUS_P_DisplayAspectRatio_ToMagnum(pSettings->aspectRatio, format, &magnumAspectRatio);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(format, &magnumVideoFormat);
    if (rc) return BERR_TRACE(rc);

    display->customFormatInfo->eVideoFmt = magnumVideoFormat;
    display->customFormatInfo->ulWidth = pSettings->width;
    display->customFormatInfo->ulHeight = pSettings->height;
    display->customFormatInfo->ulDigitalWidth = pSettings->width;
    display->customFormatInfo->ulDigitalHeight = pSettings->height;
    display->customFormatInfo->ulScanWidth = pSettings->width;
    display->customFormatInfo->ulScanHeight = pSettings->height;
    display->customFormatInfo->ulVertFreq = pSettings->refreshRate/10; /* convert from 1/1000 to 1/100 */
    /* pixel freq is in 1 /BFMT_FREQ_FACTOR MHz; NOTE: be aware of 32-bit math overflow! */
    display->customFormatInfo->ulPxlFreq = pSettings->width * pSettings->height / 10 * (pSettings->refreshRate/10)/100000;
    display->customFormatInfo->bInterlaced = pSettings->interlaced;
    display->customFormatInfo->eAspectRatio = magnumAspectRatio;
    display->customFormatInfo->pchFormatStr = "custom_format";
    /* all other members are "don't care" */

    rc = BVDC_Display_SetCustomVideoFormat(display->displayVdc, display->customFormatInfo);
    if (rc) return BERR_TRACE(rc);

    if (pSettings->aspectRatio == NEXUS_DisplayAspectRatio_eSar) {
        rc = BVDC_Display_SetSampleAspectRatio(display->displayVdc, pSettings->sampleAspectRatio.x, pSettings->sampleAspectRatio.y);
        if (rc) return BERR_TRACE(rc);
    }

    /* change this setting, as if SetSettings was called. */

    BKNI_Memset(&nexusVideoFormatInfo, 0, sizeof(nexusVideoFormatInfo));
    nexusVideoFormatInfo.width = display->customFormatInfo->ulWidth;
    nexusVideoFormatInfo.height = display->customFormatInfo->ulHeight;
    nexusVideoFormatInfo.digitalWidth = display->customFormatInfo->ulDigitalWidth;
    nexusVideoFormatInfo.digitalHeight = display->customFormatInfo->ulDigitalHeight;
    nexusVideoFormatInfo.scanWidth = display->customFormatInfo->ulScanWidth;
    nexusVideoFormatInfo.scanHeight = display->customFormatInfo->ulScanHeight;
    nexusVideoFormatInfo.topActive = display->customFormatInfo->ulTopActive;
#if NEXUS_VBI_SUPPORT
    nexusVideoFormatInfo.topMaxVbiPassThru = display->customFormatInfo->ulTopMaxVbiPassThru;
    nexusVideoFormatInfo.bottomMaxVbiPassThru = display->customFormatInfo->ulBotMaxVbiPassThru;
#endif
    nexusVideoFormatInfo.verticalFreq = display->customFormatInfo->ulVertFreq;
    nexusVideoFormatInfo.interlaced = display->customFormatInfo->bInterlaced;
    nexusVideoFormatInfo.aspectRatio = display->customFormatInfo->eAspectRatio;
    nexusVideoFormatInfo.pixelFreq = display->customFormatInfo->ulPxlFreq;
    NEXUS_P_VideoFormat_SetInfo(format, &nexusVideoFormatInfo);

    /* don't set the new format display rect here; otherwise, fullscreen video window won't be resized later; */
    display->cfg.format = NEXUS_VideoFormat_eNtsc; /* force a change -> eCustom2 */
    settings.format = format;
    settings.aspectRatio = pSettings->aspectRatio;
    settings.sampleAspectRatio.x = pSettings->sampleAspectRatio.x;
    settings.sampleAspectRatio.y = pSettings->sampleAspectRatio.y;
    return NEXUS_Display_SetSettings(display, &settings);
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
#if NEXUS_POWER_MANAGEMENT
    unsigned i;
    BERR_Code rc=0;
    NEXUS_VideoInput_P_Link *input_link;

    BSTD_UNUSED(pSettings);

    if(!enabled) {
        BTMR_TimerSettings sTmrSettings;
        BTMR_GetDefaultTimerSettings(&sTmrSettings);
        sTmrSettings.type = BTMR_Type_eSharedFreeRun;
        rc = BTMR_CreateTimer(g_pCoreHandles->tmr, &pVideo->tmr, &sTmrSettings);
        if (rc) {rc = BERR_TRACE(rc); goto err;}
        rc = BRDC_Resume(pVideo->rdc);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
        rc = BVDC_Resume(pVideo->vdc);
        if (rc) { rc = BERR_TRACE(rc); goto err; }

#if NEXUS_VBI_SUPPORT
        rc = BVBI_Resume(pVideo->vbi);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
#endif
    }

    for (i=0;i<sizeof(pVideo->displays)/sizeof(pVideo->displays[0]);i++) {
        NEXUS_DisplayHandle display = pVideo->displays[i];
        NEXUS_VideoOutput_P_Link *link;
        unsigned j;

        if(!display) {
            continue;
        }

        if(!enabled) {
            /* Open BVDC_Display and BVDC_Compositor */
            rc = NEXUS_Display_P_Open(display, display->index, &display->cfg);
            if (rc) {rc = BERR_TRACE(rc); goto err_apply;}

            rc = nexus_p_install_display_cb(display);
            if (rc) {rc = BERR_TRACE(rc); goto err_apply;}

            rc = NEXUS_Display_P_SetSettings(display, &display->cfg, true);
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_apply;}

#if NEXUS_VBI_SUPPORT
            rc = NEXUS_Display_P_ConnectVbi(display);
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_apply;}
#endif

            /* Connect Outputs */
            for(link=BLST_D_FIRST(&display->outputs);link!=NULL;link=BLST_D_NEXT(link, link)) {
                rc = nexus_videooutput_p_connect(link);
                if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_apply;}
            }

            NEXUS_Display_SetGraphicsSettings(display, &display->graphics.cfg);

            /* Connect Inputs */
            for(j=0; j<sizeof(display->windows)/sizeof(display->windows[0]); j++) {
                NEXUS_VideoWindowHandle window = &display->windows[j];
                if (!window->open) continue;
                if(window->standby) {
                    rc = NEXUS_VideoWindow_AddInput(window, window->standby);
                    if (rc) { rc = BERR_TRACE(rc); goto err_apply; }
                    window->standby = NULL;
                }
            }

            rc = NEXUS_Display_P_ApplyChanges();
            if(rc) { rc = BERR_TRACE(rc); goto err_apply; }

        } else {
            /* Disconnect Inputs */
            for(j=0; j<sizeof(display->windows)/sizeof(display->windows[0]); j++) {
                NEXUS_VideoWindowHandle window = &display->windows[j];
                if (!window->open) continue;
                if(window->input) {
                    window->standby = window->input;
                    rc = NEXUS_VideoWindow_RemoveInput(window, window->input);
                    if (rc) { rc = BERR_TRACE(rc); goto err_apply; }
                }
            }

            /* Dont uninit graphics. Just destroy source and window */
            NEXUS_Display_P_DestroyGraphicsSource(display);

            /* Disconnect Outputs */
            for(link=BLST_D_FIRST(&display->outputs);link!=NULL;link=BLST_D_NEXT(link, link)) {
                nexus_videooutput_p_disconnect(link);
            }

#if NEXUS_VBI_SUPPORT
            NEXUS_Display_P_DisconnectVbi(display);
#endif

            nexus_p_uninstall_display_cb(display);

            rc = NEXUS_Display_P_ApplyChanges();
            if(rc) { rc = BERR_TRACE(rc); goto err_apply; }

            /* Close BVDC_Display and BVDC_Compositor */
            rc = BVDC_Display_Destroy(display->displayVdc);
            if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc); goto err_apply;}
            rc = BVDC_Compositor_Destroy(display->compositor);
            if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc); goto err_apply;}

            rc = NEXUS_Display_P_ApplyChanges();
            if(rc) { rc = BERR_TRACE(rc); goto err_apply; }
        }
    }

    /*Shutdown all inputs */
    if(enabled) {
        while(1) {
            input_link = BLST_S_FIRST(&pVideo->inputs);
            if(!input_link)
            break;
            NEXUS_VideoInput_Shutdown(input_link->input);
        }
    }

    rc = NEXUS_Display_P_ApplyChanges();
    if(rc) { rc = BERR_TRACE(rc); goto err_apply; }

    if(enabled) {
#if NEXUS_VBI_SUPPORT
        rc = BVBI_Standby(pVideo->vbi, NULL);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
#endif
        rc = BVDC_Standby(pVideo->vdc, NULL);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
        rc = BRDC_Standby(pVideo->rdc, NULL);
        if (rc) { rc = BERR_TRACE(rc); goto err; }
        BTMR_DestroyTimer(pVideo->tmr);
    }

    return BERR_SUCCESS;

err_apply:
    /* SW7231-349, Coverity: 35291 */
    rc = BVDC_AbortChanges(pVideo->vdc);
    rc = BERR_TRACE(rc);
err:
    return rc;
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
#endif
    return NEXUS_SUCCESS;
}

void NEXUS_Display_GetStgSettings( NEXUS_DisplayHandle display, NEXUS_DisplayStgSettings *pSettings )
{
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    *pSettings = display->stgSettings;
}

NEXUS_Error NEXUS_Display_SetStgSettings( NEXUS_DisplayHandle display, const NEXUS_DisplayStgSettings *pSettings )
{
    BVDC_Display_StgSettings vdcSettings;
    BERR_Code rc;
    bool enabled;

    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);

    BVDC_Display_GetStgConfiguration(display->displayVdc, &enabled, &vdcSettings);
    vdcSettings.bNonRealTime = pSettings->nonRealTime;
    if (display->timingGenerator == NEXUS_DisplayTimingGenerator_eEncoder) {
        enabled = true;
    }
    else {
        enabled = pSettings->enabled;
    }
    rc = BVDC_Display_SetStgConfiguration(display->displayVdc, enabled, &vdcSettings);
    if (rc) return BERR_TRACE(rc);

    display->stgSettings = *pSettings;

    return 0;
}

NEXUS_Error NEXUS_Display_GetStatus( NEXUS_DisplayHandle display, NEXUS_DisplayStatus *pStatus )
{
    uint32_t numWindows, numGraphics;
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    *pStatus = display->status;
    (void)BVDC_Compositor_GetMaxWindowCount(display->compositor, &numWindows, &numGraphics);
    pStatus->numWindows = numWindows;
    pStatus->graphicsSupported = (numGraphics != 0);
    pStatus->timingGenerator = display->timingGenerator;
    return 0;
}

static void nexus_display_p_refresh_rate_event(void *context)
{
    NEXUS_DisplayHandle display = context;
    unsigned i;
    for(i=0;i<sizeof(display->windows)/sizeof(display->windows[0]);i++) {
        NEXUS_VideoWindowHandle window = &display->windows[i];
        if (window->open)
        {
            BKNI_EnterCriticalSection();
            NEXUS_VideoWindow_P_UpdatePhaseDelay_isr(window, display->status.refreshRate);
            if (window->syncSettings.formatCallback_isr) {
                (*window->syncSettings.formatCallback_isr)(window->syncSettings.callbackContext, 0);
            }
            BKNI_LeaveCriticalSection();
            if(window->input) {
                NEXUS_Display_P_VideoInputDisplayUpdate(NULL, window, &display->cfg);
            }
        }
    }

    NEXUS_Display_P_VideoInputDisplayUpdate(NULL, NULL, &display->cfg);
}

#if NEXUS_HAS_VIDEO_ENCODER && NEXUS_NUM_DSP_VIDEO_ENCODERS
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
unsigned NEXUS_Display_GetStgIndex_priv(NEXUS_DisplayHandle display)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    return display->stgIndex;
}
#endif

NEXUS_Error NEXUS_Display_SetEncoderCallback_priv(NEXUS_DisplayHandle display, NEXUS_VideoWindowHandle window, NEXUS_DisplayEncoderSettings *pSettings)
{
    BERR_Code rc = NEXUS_SUCCESS;
    unsigned encodeRate;
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    BAVC_EncodePictureBuffer picture;
    BVDC_Display_StgSettings vdcSettings;
    bool stgEnabled;
#else
    BVDC_Test_Window_CapturedImage capture;
#endif
    NEXUS_Display_P_Image *pImage;
    NEXUS_VideoWindowHandle encodeWindow = window?window:&display->windows[0];

#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    if (!encodeWindow->vdcState.window) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
#endif

    if(pSettings && pSettings->enqueueCb_isr && pSettings->dequeueCb_isr) {
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
        unsigned memcIndex;
        unsigned heapIndex = pVideo->moduleSettings.videoWindowHeapIndex[display->index][encodeWindow->index];
        rc = NEXUS_Core_HeapMemcIndex_isrsafe(heapIndex, &memcIndex);
        if (rc || memcIndex != 0) {
            BDBG_ERR(("DSP Encode requires capture buffers to be on MEMC0: heap[%d], MEMC%d", heapIndex, memcIndex));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
#endif

#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
        BVDC_Display_GetStgConfiguration(display->displayVdc, &stgEnabled, &vdcSettings);
        vdcSettings.ulStcSnapshotLoAddr = pSettings->stcSnapshotLoAddr;
        vdcSettings.ulStcSnapshotHiAddr = pSettings->stcSnapshotHiAddr;
        vdcSettings.vip.hHeap = pSettings->vip.hHeap;
        vdcSettings.vip.stMemSettings.ulMemcId = pSettings->vip.stMemSettings.ulMemcId;
        vdcSettings.vip.stMemSettings.ulMaxHeight = pSettings->vip.stMemSettings.ulMaxHeight;
        vdcSettings.vip.stMemSettings.ulMaxWidth = pSettings->vip.stMemSettings.ulMaxWidth;
        vdcSettings.vip.stMemSettings.bSupportInterlaced = pSettings->vip.stMemSettings.bSupportInterlaced;
        stgEnabled = true;
        BDBG_MSG(("VIP heap set to %p", (void *)(vdcSettings.vip.hHeap)));
        rc = BVDC_Display_SetStgConfiguration(display->displayVdc, stgEnabled, &vdcSettings);
        if (rc) {
            return BERR_TRACE(rc);
        }
#endif

        encodeRate = NEXUS_P_RefreshRate_FromFrameRate_isrsafe(pSettings->encodeRate);
        BKNI_EnterCriticalSection();
        display->encoder.enqueueCb_isr = pSettings->enqueueCb_isr;
        display->encoder.dequeueCb_isr = pSettings->dequeueCb_isr;
        display->encoder.context = pSettings->context;
        display->encoder.framesEnqueued = 0;
        display->encoder.window = encodeWindow;
#if 0
        display->encoder.dropRate = ((display->status.refreshRate/1000)+1) / (encodeRate/1000);
#else
        display->encoder.dropRate = 1; /* No drop ins nexus. DSP firmware will handle frame drops */
#endif
        for(pImage = BLST_SQ_FIRST(&display->encoder.free); pImage;  pImage= BLST_SQ_NEXT(pImage, link)) {
            pImage->hImage = NULL;
        }
        BKNI_LeaveCriticalSection();
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
        /* force frame capture before starting encoder and capbuf callback to avoid field capture artifact! */
        rc = NEXUS_VideoInput_P_ForceFrameCapture(display->encoder.window->input, true);
        if (rc) return BERR_TRACE(rc);
        rc = BVDC_ApplyChanges(pVideo->vdc);
        if (rc) return BERR_TRACE(rc);
        rc = BVDC_Window_SetUserCaptureBufferCount(display->encoder.window->vdcState.window, NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS);
        if (rc) {
            return BERR_TRACE(rc);
        } else  {
            display->encoder.window->cfg.userCaptureBufferCount = NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS;
        }
        /* only use artificial vsync for legacy soft encoder */
        rc = BVDC_Display_SetArtificialVsync(display->displayVdc, true, pSettings->extIntAddress, 1<<pSettings->extIntBitNum);
        if (rc) return BERR_TRACE(rc);
#endif
        rc = BVDC_ApplyChanges(pVideo->vdc);
        if (rc) return BERR_TRACE(rc);
    } else {
        /* Reclaim all buffers that encoder might be holding. Note: soft encoder should have been stopped up to this point. */
        BKNI_EnterCriticalSection();
        while(NULL != (pImage=BLST_SQ_FIRST(&display->encoder.queued))){
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
            picture.hLumaBlock = pImage->hImage;
            picture.ulPictureId = pImage->picId;
            BVDC_Display_ReturnBuffer_isr(display->displayVdc, &picture);
#else
            capture.hPicBlock = pImage->hImage;
            capture.ulPicBlockOffset = pImage->offset;
            BVDC_Test_Window_ReturnBuffer_isr(display->encoder.window->vdcState.window, &capture);
#endif
            pImage->hImage = NULL;
            BLST_SQ_REMOVE_HEAD(&display->encoder.queued, link);
            BLST_SQ_INSERT_TAIL(&display->encoder.free, pImage, link);
        }
        display->encoder.framesEnqueued = 0;
        display->encoder.enqueueCb_isr = NULL;
        display->encoder.dequeueCb_isr = NULL;
        display->encoder.context = NULL;
        BKNI_LeaveCriticalSection();
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
        BVDC_Display_GetStgConfiguration(display->displayVdc, &stgEnabled, &vdcSettings);
        vdcSettings.vip.hHeap = NULL;
        stgEnabled = true; /* always leave STG enabled to have hw trigger and interrupt */
        BDBG_MSG(("VIP heap set to %p", (void *)(vdcSettings.vip.hHeap)));
        rc = BVDC_Display_SetStgConfiguration(display->displayVdc, stgEnabled, &vdcSettings);
        if (rc) { return BERR_TRACE(rc); }
#else
        /* only use artificial vsync for legacy soft encoder */
        rc = BVDC_Display_SetArtificialVsync(display->displayVdc, false, 0, 0);
        if (rc) {rc = BERR_TRACE(rc);}
        rc = BVDC_ApplyChanges(pVideo->vdc);
        if (rc) {rc = BERR_TRACE(rc);}
        /* need to guard on cleanup, since we might have lost input already */
        if (display->encoder.window->input)
        {
            rc = NEXUS_VideoInput_P_ForceFrameCapture(display->encoder.window->input, false);
            if (rc) return BERR_TRACE(rc);
        }
        rc = BVDC_Window_SetUserCaptureBufferCount(display->encoder.window->vdcState.window, 0);
        if (rc) {
            return BERR_TRACE(rc);
        } else {
            display->encoder.window->cfg.userCaptureBufferCount = 0;
        }
#endif
        rc = BVDC_ApplyChanges(pVideo->vdc);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    }

    return rc;
}

NEXUS_Error NEXUS_Display_EnableEncoderCallback_priv(NEXUS_DisplayHandle display)
{
    BERR_Code rc = NEXUS_SUCCESS;
    BVDC_Display_CallbackSettings settings;
    unsigned i;

    /* fail if any window is VideoImageInput, which is not supported for DSP encode */
    for(i=0;i<sizeof(display->windows)/sizeof(display->windows[0]);i++) {
        NEXUS_VideoWindowHandle window = &display->windows[i];
        if (window->open && window->input && window->input->type == NEXUS_VideoInputType_eImage) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }

    BVDC_Display_GetCallbackSettings(display->displayVdc, &settings);
    settings.stMask.bPerVsync = 1;
    rc = BVDC_Display_SetCallbackSettings(display->displayVdc, &settings);
    if (rc) return BERR_TRACE(rc);
    rc = BVDC_ApplyChanges(pVideo->vdc);
    if (rc) return BERR_TRACE(rc);

    BKNI_EnterCriticalSection();
    display->encoder.callbackEnabled = true;
    BKNI_LeaveCriticalSection();

    return rc;
}

NEXUS_Error NEXUS_Display_DisableEncoderCallback_priv(NEXUS_DisplayHandle display)
{
    BERR_Code rc = NEXUS_SUCCESS;
    BVDC_Display_CallbackSettings settings;

    BKNI_EnterCriticalSection();
    display->encoder.callbackEnabled = false;
    BKNI_LeaveCriticalSection();

    BVDC_Display_GetCallbackSettings(display->displayVdc, &settings);
    settings.stMask.bPerVsync = 0;
    rc = BVDC_Display_SetCallbackSettings(display->displayVdc, &settings);
    if (rc) return BERR_TRACE(rc);
    rc = BVDC_ApplyChanges(pVideo->vdc);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    return rc;
}
#endif /* NEXUS_HAS_VIDEO_ENCODER && NEXUS_NUM_DSP_VIDEO_ENCODERS */

NEXUS_Error NEXUS_Display_GetMaxMosaicCoverage( unsigned displayIndex, unsigned numMosaics, NEXUS_DisplayMaxMosaicCoverage *pCoverage )
{
    int rc;
    uint32_t coverage;
    if (displayIndex != 0) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    BKNI_Memset(pCoverage, 0, sizeof(*pCoverage));
    rc = BVDC_GetMaxMosaicCoverage(pVideo->vdc, displayIndex, numMosaics, &coverage);
    if (rc) return BERR_TRACE(rc);
    pCoverage->maxCoverage = coverage;
    return NEXUS_SUCCESS;
}

void NEXUS_Display_GetIndex_driver( NEXUS_DisplayHandle display, unsigned *pDisplayIndex )
{
    *pDisplayIndex = display->index;
}
