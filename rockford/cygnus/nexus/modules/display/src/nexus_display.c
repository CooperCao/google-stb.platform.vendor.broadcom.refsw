/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

 ******************************************************************************/

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


unsigned NEXUS_Display_GetLastVsyncTime_isr(NEXUS_DisplayHandle display)
{
    return display->lastVsyncTime;
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
