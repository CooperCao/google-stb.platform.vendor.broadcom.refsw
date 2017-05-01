/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nexus_display_module.h"
#include "priv/nexus_surface_priv.h"
#if NEXUS_HAS_SAGE
#include "priv/nexus_sage_priv.h"
#endif

BDBG_MODULE(nexus_video_window);
BDBG_FILE_MODULE(nexus_flow_video_window);

#define BDBG_MSG_TRACE(X)

static const uint32_t aIreTable[NEXUS_DC_TABLE_COLS - 1] =  {
     64, 100, 152, 239, 327, 414, 502, 590, 677, 765, 852, 940, 956, 972, 988, 1004
};


static const uint32_t alDCTable1[NEXUS_DC_TABLE_ROWS * NEXUS_DC_TABLE_COLS] = {
    /* Used for settop-box */
     64, 64, 100, 190, 382, 550, 658, 735, 799, 826, 874, 916, 940, 940, 940, 940, 940,
     75, 64, 100, 190, 382, 550, 658, 735, 799, 826, 874, 916, 940, 940, 940, 940, 940,
    124, 64,  99, 170, 310, 442, 560, 650, 722, 790, 840, 890, 940, 940, 940, 940, 940,
    198, 64,  97, 160, 260, 406, 511, 601, 680, 753, 820, 880, 940, 940, 940, 940, 940,
    275, 64,  94, 150, 235, 360, 460, 560, 650, 735, 815, 880, 940, 940, 940, 940, 940,
    349, 64,  90, 140, 215, 328, 430, 531, 629, 725, 800, 870, 940, 940, 940, 940, 940,
    425, 64,  85, 130, 200, 305, 418, 515, 624, 718, 795, 870, 940, 940, 940, 940, 940,
    497, 64,  80, 120, 200, 292, 409, 508, 619, 711, 790, 866, 940, 940, 940, 940, 940,
    572, 64,  80, 120, 200, 290, 395, 491, 602, 700, 783, 863, 940, 940, 940, 940, 940,
    652, 64,  80, 120, 200, 290, 388, 481, 581, 684, 773, 860, 940, 940, 940, 940, 940,
    723, 64,  80, 120, 200, 288, 384, 473, 569, 676, 768, 856, 940, 940, 940, 940, 940,
    799, 64,  80, 120, 200, 282, 378, 469, 563, 661, 743, 840, 940, 940, 940, 940, 940,
    875, 64,  80, 120, 200, 277, 375, 465, 558, 659, 742, 835, 940, 940, 940, 940, 940,
    924, 64,  80, 120, 200, 275, 372, 463, 555, 650, 742, 834, 940, 940, 940, 940, 940,
    940, 64,  80, 120, 200, 275, 372, 463, 555, 650, 742, 834, 940, 940, 940, 940, 940
};

#define pVideo (&g_NEXUS_DisplayModule_State)
static NEXUS_Error NEXUS_VideoWindow_P_ApplySplitScreenSettings(NEXUS_VideoWindowHandle window);
static NEXUS_Error NEXUS_VideoWindow_P_RemoveInput(NEXUS_VideoWindowHandle window, NEXUS_VideoInput input, bool skipDisconnect);

static void
NEXUS_VideoWindow_P_LboxCallbackFunc_isr(void *pvParm1, int iParm2, const BVDC_BoxDetectInfo *pBoxDetectInfo)
{
    NEXUS_VideoWindowHandle window = pvParm1;
    bool setevent = false;
    uint32_t  ulOverflowHeight=0;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    BSTD_UNUSED(iParm2);

    /* for now, magnum maps straight to nexus. */
    BDBG_CASSERT(sizeof(*pBoxDetectInfo) == sizeof(window->letterBoxStatus));

    /* Set event for pc auto position if result changed */
    if ((window->letterBoxStatus.whiteBoxLeft  != pBoxDetectInfo->ulWhiteBoxLeft  )||
        (window->letterBoxStatus.whiteBoxTop   != pBoxDetectInfo->ulWhiteBoxTop   )||
        (window->letterBoxStatus.whiteBoxWidth != pBoxDetectInfo->ulWhiteBoxWidth )||
        (window->letterBoxStatus.whiteBoxHeight!= pBoxDetectInfo->ulWhiteBoxHeight))
    {
        /* don't set event until after memcpy */
        /* LB only have 10 bits to indicate the height.*/
        if((int32_t)(pBoxDetectInfo->ulWhiteBoxHeight) < 177)
        {
            ulOverflowHeight = 1024 + (int32_t)(pBoxDetectInfo->ulWhiteBoxHeight);
        }
        else
        {
            ulOverflowHeight = pBoxDetectInfo->ulWhiteBoxHeight;
        }
        if((pBoxDetectInfo->ulWhiteBoxWidth <= 1920) &&
            (ulOverflowHeight <= 1200))
        {
            setevent = true;
        }
    }

    BKNI_Memcpy(&window->letterBoxStatus, pBoxDetectInfo, sizeof(*pBoxDetectInfo));
    NEXUS_IsrCallback_Fire_isr(window->letterBoxDetectionCallback);
    if (setevent) {
        window->letterBoxStatus.whiteBoxHeight = ulOverflowHeight;
        BKNI_SetEvent(window->lb_event);
    }
}

static BERR_Code NEXUS_VideoWindow_P_RecreateWindow(NEXUS_VideoWindowHandle window)
{
    BERR_Code rc;

    if (!window->vdcState.window) {
        /* nothing to recreate */
        return 0;
    }

    if (g_NEXUS_DisplayModule_State.updateMode != NEXUS_DisplayUpdateMode_eAuto) { return BERR_TRACE(NEXUS_NOT_SUPPORTED); }
    NEXUS_VideoWindow_P_DestroyVdcWindow(window);
    rc = NEXUS_VideoWindow_P_CreateVdcWindow(window, &window->cfg);
    if (rc) return BERR_TRACE(rc);
    rc = NEXUS_VideoWindow_P_SetVdcSettings(window, &window->cfg, true);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

BVDC_Mode nexus_p_window_alloc_mtg(NEXUS_VideoWindowHandle window)
{
#if NEXUS_MTG_DISABLED
    return BVDC_Mode_eOff;
#else
    if (window && !g_NEXUS_DisplayModule_State.moduleSettings.memConfig[window->display->index].window[window->index].mtg) {
        return BVDC_Mode_eOff;
    }
    return BVDC_Mode_eAuto;
#endif
}

static bool nexus_p_synclock_capable(NEXUS_VideoInput input, NEXUS_VideoWindowHandle window)
{
#if NEXUS_HAS_VIDEO_DECODER
    if (input && input->type == NEXUS_VideoInputType_eDecoder) {
        BAVC_SourceId sourceId;
        NEXUS_VideoInput_P_Link *link;
        link = input->destination;
        if (link) {
            sourceId = link->id;
        }
        else {
            NEXUS_Module_Lock(pVideo->modules.videoDecoder);
            NEXUS_VideoDecoder_GetSourceId_priv(input->source, &sourceId);
            NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
        }
        if (g_pCoreHandles->boxConfig->stVdc.astSource[sourceId].bMtgCapable && nexus_p_window_alloc_mtg(window) != BVDC_Mode_eOff) {
            /* this input is or will be mtg, so not synclocked */
            return false;
        }
        /* this input and window path is capable of being synclocked */
        return true;
    }
#endif
    return false;
}

static void NEXUS_VideoWindow_P_PredictSyncLock(NEXUS_VideoWindowHandle window)
{
    unsigned j;
    bool foundSyncLockedWindow = false;

    if (!window->status.isSyncLocked && nexus_p_synclock_capable(window->input, window))
    {
        /* if there's another digital window on this display, destroy & recreate it. then this window will become sync-locked. */
        /* if there's another digital window for this source on another display, destroy & recreate it. then this window will become sync-locked. */
        for (j=0;j<NEXUS_NUM_DISPLAYS;j++)
        {
            unsigned i;
            NEXUS_DisplayHandle d = pVideo->displays[j];
            if (!d) continue;
            for (i=0;i<NEXUS_NUM_VIDEO_WINDOWS;i++)
            {
                NEXUS_VideoWindowHandle w = &d->windows[i];
                if (!w->open || w == window) continue; /* skip closed or same window */

                /* if the other window shares the same source or
                another digital source on the same display, then it contends for sync lock. */
                if (w->input == window->input || (d == window->display && nexus_p_synclock_capable(w->input, w)))
                {
                    foundSyncLockedWindow = true;
                    break;
                }
            }
        }

        if (!foundSyncLockedWindow)
        {
            window->status.isSyncLocked = true;
        }
    }
}

struct nexus_synclock_recreate
{
    NEXUS_VideoInput input;
    NEXUS_VideoWindowHandle window[NEXUS_NUM_DISPLAYS];
};

static void NEXUS_VideoWindow_P_DestroyForSyncLock(NEXUS_VideoWindowHandle window, bool preferSyncLock, NEXUS_VideoInput input, struct nexus_synclock_recreate *recreate)
{
    unsigned i;

    BKNI_Memset(recreate, 0, sizeof(*recreate));

    if (!preferSyncLock ||
        window->status.isSyncLocked ||
        window->display->index != 0) return;

    if (!nexus_p_synclock_capable(input, window)) {
        return;
    }

    for (i=0;i<NEXUS_NUM_VIDEO_WINDOWS;i++) {
        NEXUS_VideoWindowHandle w = &window->display->windows[i];
        if (!w->open || w == window) continue; /* skip closed or same window */
        if (w->cfg.preferSyncLock) {
            BDBG_WRN(("two windows have preferSyncLock = true"));
            /* take no action */
            return;
        }
        else if (w->status.isSyncLocked) {
            break;
        }
    }
    if (i<NEXUS_NUM_VIDEO_WINDOWS) {
        /* window[i] is synclocked but does not prefer it. We must destroy windows for this input on all displays,
        then recreate after the window that prefers synclock is created.
        To simplify, we will assume the window->index is the same on all displays */
        unsigned d;
        BDBG_WRN(("NEXUS_VideoWindow_P_DestroyForSyncLock: %d.%d prefers but is not", window->display->index, window->index));
        recreate->input = window->display->windows[i].input;
        for (d=0;d<NEXUS_NUM_DISPLAYS;d++) {
            NEXUS_VideoWindowHandle w;
            if (!pVideo->displays[d] || !pVideo->displays[d]->windows[i].open) continue;
            w = &pVideo->displays[d]->windows[i];
            if (w->input != recreate->input) continue; /* not expected */
            BDBG_WRN(("  destroying %d.%d", d, i));
            recreate->window[d] = w;
            NEXUS_VideoWindow_P_RemoveInput(w, recreate->input, true);
        }
    }

    /* this window should now be sync-locked, the callback will come and verify later */
    window->status.isSyncLocked = true;
}

static void NEXUS_VideoWindow_P_RecreateForSyncLock(struct nexus_synclock_recreate *recreate)
{
    unsigned i;
    if (!recreate->input) return;
    for (i=0;i<NEXUS_NUM_DISPLAYS;i++) {
        if (recreate->window[i]) {
            int rc;
            BDBG_WRN(("  recreating %d.%d as syncslip window", recreate->window[i]->display->index, recreate->window[i]->index));
            rc = NEXUS_VideoWindow_AddInput(recreate->window[i], recreate->input);
            if (rc) BERR_TRACE(rc); /* keep going */
            recreate->window[i]->status.isSyncLocked = false;
        }
    }
}

/* NEXUS_VideoWindow_P_SetCbSetting is called on Open and SetSettings.
 */
static BERR_Code NEXUS_VideoWindow_P_SetCbSetting(NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowSettings *settings)
{
    BVDC_Window_CallbackSettings wcbs;
    BERR_Code rc;

    rc = BVDC_Window_GetCallbackSettings(window->vdcState.window, &wcbs);
    if (rc) return BERR_TRACE(rc);

    wcbs.stMask.bDriftDelay = true;
    wcbs.ulLipSyncTolerance = 1000; /* usec. hardcoded threshold for bDriftDelay. */
    wcbs.stMask.bVsyncDelay = true;
    wcbs.stMask.bSyncLock = true;
    wcbs.stMask.bRectAdjust = true;
    wcbs.stMask.bCrc = (settings->crc.crcQueueSize != 0);
    wcbs.eCrcModule = (BVDC_VnetModule) settings->crc.bvnBlock;
    rc = BVDC_Window_SetCallbackSettings(window->vdcState.window, &wcbs);
    if (rc) return BERR_TRACE(rc);

    if (window->crc.size != settings->crc.crcQueueSize) {
        void *new_ptr = NULL, *old_ptr;

        /* defer the free until after critical section */
        old_ptr = window->crc.queue;
        /* queue size of 1 is treated same as 0 because it can't hold anything */
        if (settings->crc.crcQueueSize > 1) {
            new_ptr = BKNI_Malloc(settings->crc.crcQueueSize * sizeof(NEXUS_VideoWindowCrcData));
            if (!new_ptr) {
                return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            }
        }

        /* must synchronize with ISR, so set state in CS */
        BKNI_EnterCriticalSection();
        window->crc.queue = new_ptr;
        window->crc.size = new_ptr?settings->crc.crcQueueSize:0;
        window->crc.wptr = window->crc.rptr = 0; /* flush */
        BKNI_LeaveCriticalSection();

        if (old_ptr) {
            BKNI_Free(old_ptr);
        }
    }

    return 0;
}

static void NEXUS_VideoWindow_P_UnsetCbSetting(NEXUS_VideoWindowHandle window)
{
    if (window->crc.queue) {
        void *old_ptr = window->crc.queue;
        BKNI_EnterCriticalSection();
        window->crc.queue = NULL;
        window->crc.size = 0;
        BKNI_LeaveCriticalSection();
        BKNI_Free(old_ptr);
    }
}

NEXUS_Error NEXUS_VideoWindow_GetCrcData( NEXUS_VideoWindowHandle window, NEXUS_VideoWindowCrcData *pData, unsigned numEntries, unsigned *pNumEntriesReturned )
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pNumEntriesReturned = 0;
    /* Coverity: 36606, FORWARD_NULL */
    if (pData == NULL)
    {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return 0;
    }
    /* no critical section needed for this type of producer/consumer */
    while (*pNumEntriesReturned < numEntries && window->crc.wptr != window->crc.rptr && window->crc.queue) {
        pData[*pNumEntriesReturned] = window->crc.queue[window->crc.rptr];
        (*pNumEntriesReturned)++;
        if (++window->crc.rptr == window->crc.size) {
            window->crc.rptr = 0;
        }
    }
    return 0;
}

BERR_Code
NEXUS_VideoWindow_P_SetVdcSettings(NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowSettings *settings, bool force)
{
    BERR_Code rc;
    BVDC_Window_Handle windowVdc = window->vdcState.window;
    bool callSync = false;
    NEXUS_DisplayUpdateMode saveUpdateMode;
    bool isFullScreen;

    /* We've about to make a lot of VDC changes. We do not want ApplyChanges called. This will temporarily mask it.
    Be sure to restore it at the end, even in a failure case. */
    saveUpdateMode = g_NEXUS_DisplayModule_State.updateMode;
    g_NEXUS_DisplayModule_State.updateMode = NEXUS_DisplayUpdateMode_eManual;

    BDBG_ASSERT(window->vdcState.window);

    force = force || pVideo->lastUpdateFailed;

    if(force || settings->autoMaster != window->cfg.autoMaster) {
        rc = NEXUS_VideoWindow_P_ConfigMasterFrameRate(window, &window->display->cfg, settings);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_windowCfg;}
        callSync = true;
    }

    if(!g_pCoreHandles->boxConfig->stBox.ulBoxId){
        if(force || settings->forceCapture != window->cfg.forceCapture ||
           settings->forceCapture != window->syncStatus.forcedCaptureEnabled)
        {
            /* Be aware that SyncChannel requires force capture for smooth transitions. */
            rc = BVDC_Window_SetForceCapture(windowVdc, settings->forceCapture);
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_windowCfg;}
            window->syncStatus.forcedCaptureEnabled = settings->forceCapture;
            callSync = true;
        }
    }

    if(force || settings->colorKey.luma.enabled != window->cfg.colorKey.luma.enabled ||
       settings->colorKey.luma.lower != window->cfg.colorKey.luma.lower ||
       settings->colorKey.luma.upper != window->cfg.colorKey.luma.upper ||
       settings->colorKey.luma.mask  != window->cfg.colorKey.luma.mask  ||
       settings->colorKey.cr.enabled != window->cfg.colorKey.cr.enabled ||
       settings->colorKey.cr.lower   != window->cfg.colorKey.cr.lower   ||
       settings->colorKey.cr.upper   != window->cfg.colorKey.cr.upper   ||
       settings->colorKey.cr.mask    != window->cfg.colorKey.cr.mask    ||
       settings->colorKey.cb.enabled != window->cfg.colorKey.cb.enabled ||
       settings->colorKey.cb.lower   != window->cfg.colorKey.cb.lower   ||
       settings->colorKey.cb.upper   != window->cfg.colorKey.cb.upper   ||
       settings->colorKey.cb.mask    != window->cfg.colorKey.cb.mask)
    {
        BVDC_ColorKey_Settings  colorKeySettings;
        colorKeySettings.bLumaKey = settings->colorKey.luma.enabled;
        colorKeySettings.ucLumaKeyLow = settings->colorKey.luma.lower;
        colorKeySettings.ucLumaKeyHigh = settings->colorKey.luma.upper;
        colorKeySettings.ucLumaKeyMask = settings->colorKey.luma.mask;
        colorKeySettings.bChromaRedKey = settings->colorKey.cr.enabled;
        colorKeySettings.ucChromaRedKeyLow = settings->colorKey.cr.lower;
        colorKeySettings.ucChromaRedKeyHigh = settings->colorKey.cr.upper;
        colorKeySettings.ucChromaRedKeyMask = settings->colorKey.cr.mask;
        colorKeySettings.bChromaBlueKey = settings->colorKey.cb.enabled;
        colorKeySettings.ucChromaBlueKeyLow = settings->colorKey.cb.lower;
        colorKeySettings.ucChromaBlueKeyHigh = settings->colorKey.cb.upper;
        colorKeySettings.ucChromaBlueKeyMask = settings->colorKey.cb.mask;
        rc = BVDC_Window_SetColorKeyConfiguration(windowVdc, &colorKeySettings);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_windowCfg;}
    }

    if(force || settings->pixelFormat != window->cfg.pixelFormat || settings->pixelFormat != window->pixelFormat)
    {
        BPXL_Format pixelFormat;
        if (settings->pixelFormat != NEXUS_PixelFormat_eUnknown) {
            NEXUS_PixelFormat nexusFormat = settings->pixelFormat;

            rc = NEXUS_P_PixelFormat_ToMagnum_isrsafe(nexusFormat, &pixelFormat);
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_windowCfg;}

            rc = BVDC_Window_SetPixelFormat(windowVdc, pixelFormat);
            if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowCfg;}
            window->pixelFormat = nexusFormat;
        }
    }

    if(force || settings->visible != window->cfg.visible) {
        rc = BVDC_Window_SetVisibility(windowVdc, settings->visible);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowCfg;}
        window->syncStatus.visible = settings->visible;
        callSync = true;
        window->layoutChanged = true;
    }

    if(force || settings->zorder != window->cfg.zorder) {
        rc = BVDC_Window_SetZOrder(windowVdc, settings->zorder);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowCfg;}
        window->layoutChanged = true;
    }

    if(force || settings->alpha != window->cfg.alpha ||
                settings->sourceBlendFactor != window->cfg.sourceBlendFactor ||
                settings->destBlendFactor != window->cfg.destBlendFactor ||
                settings->constantAlpha != window->cfg.constantAlpha)
    {
        rc = BVDC_Window_SetAlpha(windowVdc, settings->alpha);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowCfg;}

        BDBG_CASSERT(NEXUS_CompositorBlendFactor_eMax-1 == BVDC_BlendFactor_eOneMinusConstantAlpha);
        rc = BVDC_Window_SetBlendFactor( windowVdc, settings->sourceBlendFactor, settings->destBlendFactor, settings->constantAlpha);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_windowCfg;}
    }

    /* internally call SetSettings functions which may have been deferred because the VDC window didn't exist */
    if(force){
        NEXUS_VideoAdj_P_ApplySetSettings(window);
        NEXUS_PictureCtrl_P_ApplySetSettings(window);
        if (window->afdSet) {
            rc = NEXUS_VideoWindow_SetAfdSettings(window, &window->afdSettings);
            if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_windowCfg;}
        }
    }

    if (force ||
        settings->letterBoxDetect != window->cfg.letterBoxDetect ||
        settings->letterBoxAutoCut != window->cfg.letterBoxAutoCut)
    {
        if (settings->letterBoxDetect == true) {
            rc = BVDC_Window_EnableBoxDetect(windowVdc, NEXUS_VideoWindow_P_LboxCallbackFunc_isr, (void *)window, 0, settings->letterBoxAutoCut);
            if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowCfg;}
        }
        else {
            rc = BVDC_Window_DisableBoxDetect(windowVdc);
            if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowCfg;}
        }
    }

    if (force || settings->delay != window->cfg.delay) {
        /* NOTE: BVDC_Window_SetDelayOffset is also set in NEXUS_VideoWindow_SetSyncSettings_priv */
        rc = BVDC_Window_SetDelayOffset(windowVdc, settings->delay + window->syncSettings.delay);
        if (rc) {return BERR_TRACE(rc);}
    }

    if (force || settings->contentMode != window->cfg.contentMode ||
        BKNI_Memcmp(&settings->scaleFactorRounding, &window->cfg.scaleFactorRounding, sizeof(settings->scaleFactorRounding)))
    {
        BVDC_PanScanType panScanType = BVDC_PanScanType_eDisable;
        BVDC_AspectRatioMode aspectRatioMode;

        if (force || settings->contentMode != window->cfg.contentMode) {
            /* if the user changed layout, we need to recompute */
            window->layoutChanged = true;
        }

        rc = BVDC_Window_SetScaleFactorRounding( windowVdc,
            settings->scaleFactorRounding.enabled?settings->scaleFactorRounding.horizontalTolerance:0,
            settings->scaleFactorRounding.enabled?settings->scaleFactorRounding.verticalTolerance:0);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_windowCfg; }

        switch(settings->contentMode) {
        case NEXUS_VideoWindowContentMode_ePanScan:
            panScanType = BVDC_PanScanType_eStream;
            aspectRatioMode = BVDC_AspectRatioMode_eUseAllDestination;
            break;
        case NEXUS_VideoWindowContentMode_ePanScanWithoutCorrection:
            panScanType = BVDC_PanScanType_eStream;
            aspectRatioMode = BVDC_AspectRatioMode_eBypass;
            break;
        case NEXUS_VideoWindowContentMode_eZoom:
            aspectRatioMode = BVDC_AspectRatioMode_eUseAllDestination;
            break;
        case NEXUS_VideoWindowContentMode_eBox:
            aspectRatioMode = BVDC_AspectRatioMode_eUseAllSource;
            break;
        default:
        case NEXUS_VideoWindowContentMode_eFullNonLinear:
        case NEXUS_VideoWindowContentMode_eFull:
            aspectRatioMode = BVDC_AspectRatioMode_eBypass;
            break;
        }

        BDBG_MSG(("window %p pan scan %d aspect ratio %d",(void *)windowVdc,panScanType,aspectRatioMode));
        rc = BVDC_Window_SetPanScanType(windowVdc, panScanType);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_windowCfg; }

        rc = BVDC_Window_SetAspectRatioMode(windowVdc, aspectRatioMode);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_windowCfg; }
    }

    isFullScreen = settings->position.x == 0 &&
                 settings->position.y == 0 &&
                 window->display->displayRect.width == settings->position.width &&
                 window->display->displayRect.height == settings->position.height;

    if (window->syncStatus.fullScreen != isFullScreen)
    {
        window->syncStatus.fullScreen = isFullScreen;
        callSync = true;
    }

    if (force ||
        !NEXUS_P_Display_RectEqual(&window->cfg.position, &settings->position) ||
        settings->contentMode != window->cfg.contentMode)
    {
        unsigned scalerWidth = (isFullScreen && (settings->contentMode==NEXUS_VideoWindowContentMode_eFullNonLinear)) ? settings->position.width/2 : 0;
        rc = BVDC_Window_SetNonLinearScl(windowVdc, scalerWidth, 0);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_windowCfg; }
    }
    if(force || !NEXUS_P_Display_RectEqual(&window->cfg.position, &settings->position) ||
            !NEXUS_P_Display_RectEqual(&window->cfg.clipBase, &settings->clipBase) ||
            !NEXUS_P_Display_RectEqual(&window->cfg.clipRect, &settings->clipRect)
            )
    {
        NEXUS_Rect scalerRect;

        window->layoutChanged = true;

        rc = BVDC_Window_SetDstRect(windowVdc, settings->position.x, settings->position.y, settings->position.width, settings->position.height);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowDimensions;}

        rc = NEXUS_Display_P_GetScalerRect(settings, &scalerRect);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowDimensions;}

        BDBG_MSG(("NEXUS_VideoWindow_P_SetVdcSettings:%#lx scaler output [%u:%u x %u:%u]", (unsigned long)window, scalerRect.x, scalerRect.y, scalerRect.width, scalerRect.height));
        rc = BVDC_Window_SetScalerOutput(windowVdc, scalerRect.x, scalerRect.y, scalerRect.width, scalerRect.height);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowDimensions;}
    }

    if(force ||
       window->cfg.sourceClip.left != settings->sourceClip.left ||
       window->cfg.sourceClip.right != settings->sourceClip.right ||
       window->cfg.sourceClip.top != settings->sourceClip.top ||
       window->cfg.sourceClip.bottom != settings->sourceClip.bottom )
    {
        rc = BVDC_Window_SetSrcClip(windowVdc, settings->sourceClip.left, settings->sourceClip.right, settings->sourceClip.top, settings->sourceClip.bottom);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowDimensions;}
    }

    if (force && window->colorMatrixSet) {
        rc = BVDC_Window_SetColorMatrix(windowVdc, window->colorMatrixOverride, window->colorMatrix.coeffMatrix, window->colorMatrix.shift);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowCfg;}
    }
#if NEXUS_NUM_MOSAIC_DECODES
    if (BLST_S_FIRST(&window->mosaic.children)) {
        /* if a mosaic parent, never allow direct clearRect settings */
        if (settings->clearRect.enabled) {
            rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto err_windowCfg;
        }
        /* else, skip mosaic functions here, even if force == true. They do not apply. */
    }else if (force || window->cfg.clearRect.enabled != settings->clearRect.enabled ||
        window->cfg.clearRect.position.x != settings->clearRect.position.x ||
        window->cfg.clearRect.position.y != settings->clearRect.position.y ||
        window->cfg.clearRect.position.width != settings->clearRect.position.width ||
        window->cfg.clearRect.position.height != settings->clearRect.position.height ||
        window->cfg.clearRect.color != settings->clearRect.color)
    {
        BVDC_MosaicConfiguration mosaicCfg;
        BKNI_Memset(&mosaicCfg, 0, sizeof(mosaicCfg));
        if (!settings->clearRect.enabled) {
            /* do the minimum if disabling */
            BVDC_Window_SetMosaicConfiguration(windowVdc, false, &mosaicCfg);
        }
        else {
            BVDC_Rect clearRect;
            bool mosaicMode;
            mosaicCfg.bVideoInMosaics = false;
            mosaicCfg.bClearRectByMaskColor = true;
            mosaicCfg.ulClearRectAlpha = (settings->clearRect.color >> 24) & 0xff;
            mosaicCfg.ulMaskColorRed = (settings->clearRect.color >> 16) & 0xff;
            mosaicCfg.ulMaskColorGreen = (settings->clearRect.color >> 8) & 0xff;
            mosaicCfg.ulMaskColorBlue = (settings->clearRect.color >> 0) & 0xff;
            BVDC_Window_SetMosaicConfiguration(windowVdc, settings->clearRect.enabled, &mosaicCfg);

            clearRect.lLeft = settings->clearRect.position.x;
            clearRect.lTop = settings->clearRect.position.y;
            clearRect.ulWidth = settings->clearRect.position.width;
            clearRect.ulHeight = settings->clearRect.position.height;
            BVDC_Window_SetMosaicDstRects(windowVdc, 1, &clearRect);

            mosaicMode = settings->clearRect.enabled;
            BVDC_Window_SetMosaicRectsVisibility(windowVdc, 1, &mosaicMode);
        }
    }
#endif

    if (force ||
        window->cfg.window3DSettings.rightViewOffset != settings->window3DSettings.rightViewOffset) {
        rc = BVDC_Window_SetDstRightRect(windowVdc, settings->window3DSettings.rightViewOffset);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_windowCfg;}
    }

    if (force ||
        window->cfg.crc.crcQueueSize != settings->crc.crcQueueSize ||
        window->cfg.crc.bvnBlock != settings->crc.bvnBlock)
    {
        rc = NEXUS_VideoWindow_P_SetCbSetting(window, settings);
        if (rc) {rc = BERR_TRACE(rc); goto err_windowCfg;}
    }

    if (force ||
        settings->userCaptureBufferCount != window->cfg.userCaptureBufferCount)
    {
        if (window->cfg.userCaptureBufferCount == NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS)
        {
            BDBG_ERR(("User capture buffer count already set to %d for Raaga encoder",
            NEXUS_DISPLAY_ENCODER_MAX_PICTURE_BUFFERS));
            rc = BERR_INVALID_PARAMETER;
            goto err_windowCfg;
        }
        else
        {
            rc = BVDC_Window_SetUserCaptureBufferCount(windowVdc, settings->userCaptureBufferCount);
            if (rc)
            {
                rc = BERR_TRACE(rc);
                goto err_windowCfg;
            }
            else
            {
                window->cfg.userCaptureBufferCount = settings->userCaptureBufferCount;
            }
        }
    }

    if (callSync && window->syncSettings.stateChangeCallback_isr) {
        BKNI_EnterCriticalSection();
        (*window->syncSettings.stateChangeCallback_isr)(window->syncSettings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }

    g_NEXUS_DisplayModule_State.updateMode = saveUpdateMode;
    return BERR_SUCCESS;

err_windowCfg:
    g_NEXUS_DisplayModule_State.updateMode = saveUpdateMode;
    return rc;

err_windowDimensions:
    BDBG_ERR(("Invalid window %d dimensions: position(%d,%d,%d,%d), clipRect(%d,%d,%d,%d), clipBase(%d,%d,%d,%d)",
        window->index,
        settings->position.x,settings->position.y,settings->position.width,settings->position.height,
        settings->clipRect.x,settings->clipRect.y,settings->clipRect.width,settings->clipRect.height,
        settings->clipBase.x,settings->clipBase.y,settings->clipBase.width,settings->clipBase.height));
    g_NEXUS_DisplayModule_State.updateMode = saveUpdateMode;
    return rc;
}

#define PHASE_TOLERANCE 50

void NEXUS_VideoWindow_P_UpdatePhaseDelay_isr(NEXUS_VideoWindowHandle window, unsigned refreshRate)
{
    window->status.phaseDelay = 0; /* set to zero here, if refresh rate is ready, this will get updated to real value */
    if (refreshRate)
    {
        int vsyncDelayInUs;
        int vsyncDelay;

        vsyncDelay = window->status.delay;
        vsyncDelayInUs = vsyncDelay * (1000000*1000/refreshRate);
        if (window->phaseDelay >= 0)
        {
            window->status.phaseDelay = window->phaseDelay - vsyncDelayInUs; /* subtract out 'delay' portion */
        }
    }
}

static void NEXUS_VideoWindow_P_SyncLockEventHandler(void *data)
{
    NEXUS_VideoWindowHandle window = data;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoWindow, window);
    BDBG_ASSERT(window->display);
    if (window->input)
    {
        NEXUS_Display_P_VideoInputDisplayUpdate(NULL, window, &window->display->cfg);
    }
}

static void NEXUS_VideoWindow_P_Callback_isr(void *data, int iParm2, void * pvVdcData)
{
    NEXUS_VideoWindowHandle window = (NEXUS_VideoWindowHandle)data;
    const BVDC_Window_CallbackData * pCbData = (const BVDC_Window_CallbackData *)pvVdcData;
    const BVDC_Window_CallbackMask * pMask = &pCbData->stMask;

    BSTD_UNUSED(iParm2);

    /* record the status for GetStatus calls */
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);

    if (!window->open) return;

    if (pMask->bVsyncDelay)
    {
        window->status.delay = pCbData->ulVsyncDelay;
        window->syncStatus.delayValid = true;
    }

    if (pMask->bDriftDelay)
    {
        window->phaseDelay = pCbData->ulDriftDelay; /* total delay in microseconds */
        NEXUS_VideoWindow_P_UpdatePhaseDelay_isr(window, window->display->status.refreshRate);
    }

    if (pMask->bRectAdjust)
    {
        if (window->status.outputRect.x != pCbData->stOutputRect.lLeft ||
            window->status.outputRect.y != pCbData->stOutputRect.lTop ||
            window->status.outputRect.width != pCbData->stOutputRect.ulWidth ||
            window->status.outputRect.height != pCbData->stOutputRect.ulHeight)
        {
            window->status.outputRect.x = pCbData->stOutputRect.lLeft;
            window->status.outputRect.y = pCbData->stOutputRect.lTop;
            window->status.outputRect.width = pCbData->stOutputRect.ulWidth;
            window->status.outputRect.height = pCbData->stOutputRect.ulHeight;
            NEXUS_IsrCallback_Fire_isr(window->outputRectChangedCallback);
        }
    }

    if (pMask->bSyncLock)
    {
        if (window->status.isSyncLocked != pCbData->bSyncLock)
        {
            BDBG_WRN(("Predictive sync lock status mismatch: %u.%u = %u", window->display->index, window->index, pCbData->bSyncLock));
            window->status.isSyncLocked = pCbData->bSyncLock;
        }
        window->syncStatus.syncLocked = pCbData->bSyncLock;

        BKNI_SetEvent(window->syncLockEvent);

        if (window->syncSettings.stateChangeCallback_isr)
        {
            (*window->syncSettings.stateChangeCallback_isr)(window->syncSettings.callbackContext, 0);
        }
    }

    if ((pMask->bVsyncDelay || pMask->bDriftDelay) && window->syncSettings.delayCallback_isr) {
        (*window->syncSettings.delayCallback_isr)(window->syncSettings.callbackContext, window->status.delay);
    }

    if (pMask->bCrc && window->crc.queue) {
        NEXUS_VideoWindowCrcData *pData = &window->crc.queue[window->crc.wptr];
        pData->bvn.luma = pCbData->ulCrcLuma;
        pData->bvn.chroma = pCbData->ulCrcChroma;
        if (++window->crc.wptr == window->crc.size) {
            window->crc.wptr = 0;
        }
        if (window->crc.wptr == window->crc.rptr) {
            BDBG_WRN(("Window%d Vnet CRC overflow", window->index));
        }
    }
}

bool NEXUS_VideoWindow_IsSmoothScaling_isrsafe(NEXUS_VideoWindowHandle window)
{
    return g_pCoreHandles->boxConfig->stBox.ulBoxId &&
        (BBOX_Vdc_SclCapBias_eAutoDisable == g_pCoreHandles->boxConfig->stVdc.astDisplay[window->display->index].astWindow[window->index].eSclCapBias ||
         BBOX_Vdc_SclCapBias_eSclBeforeCap == g_pCoreHandles->boxConfig->stVdc.astDisplay[window->display->index].astWindow[window->index].eSclCapBias ||
         BBOX_Vdc_SclCapBias_eAutoDisable1080p == g_pCoreHandles->boxConfig->stVdc.astDisplay[window->display->index].astWindow[window->index].eSclCapBias);
}

#if BVDC_BUF_LOG && NEXUS_BASE_OS_linuxuser
typedef enum NEXUS_VideoBufLog_Trigger
{
    NEXUS_VideoBufLog_Trigger_eReset,
    NEXUS_VideoBufLog_Trigger_eManual,
    NEXUS_VideoBufLog_Trigger_eAutomatic,
    NEXUS_VideoBufLog_Trigger_eAutomaticReduced
} NEXUS_VideoBufLog_Trigger;

static void
NEXUS_VideoWindow_P_MultiBufLogEventHandler(void *data)
{
    BSTD_UNUSED(data);
    NEXUS_Display_P_BufLogCapture();
}

static void
NEXUS_VideoWindow_P_MultiBufLogCallBack_isr( void *pParm1, int iParm2, void *pData )
{
    NEXUS_VideoWindowHandle window = (NEXUS_VideoWindowHandle)pParm1;

    BSTD_UNUSED(iParm2);
    BSTD_UNUSED(pData);

    BKNI_SetEvent(window->bufLogEvent);
}

NEXUS_Error
NEXUS_VideoWindow_P_EnableMultiBufLog( NEXUS_VideoWindowHandle window, NEXUS_VideoBufLog_Trigger trigger, bool enable )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);

    if (enable)
    {
        BVDC_SetBufLogStateAndDumpTrigger((BVDC_BufLogState)trigger,
            NEXUS_VideoWindow_P_MultiBufLogCallBack_isr, (void *)window, 0);
    }
    else
    {
        BVDC_SetBufLogStateAndDumpTrigger((BVDC_BufLogState)NEXUS_VideoBufLog_Trigger_eReset,
            NULL, NULL, 0);
    }

    BVDC_Window_EnableBufLog(window->vdcState.window, enable);

    return rc;
}
#endif /* BVDC_BUF_LOG && NEXUS_BASE_OS_linuxuser */

BERR_Code
NEXUS_VideoWindow_P_CreateVdcWindow(NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowSettings *cfg)
{
    BERR_Code rc;
    NEXUS_DisplayHandle display;
    BVDC_Window_Settings windowCfg;
    NEXUS_VideoInput_P_Link *link;
    BVDC_WindowId windowId;
    unsigned windowHeapIndex = NEXUS_MAX_HEAPS;

    BDBG_OBJECT_ASSERT(window->input, NEXUS_VideoInput);
    link = window->input->destination;
    BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);
    BDBG_ASSERT(link->sourceVdc);
    BDBG_OBJECT_ASSERT(window->display, NEXUS_Display);
    display = window->display;
    BDBG_ASSERT(cfg);

    BDBG_MSG((">%s(%d) window: %ux%u display=%p window=%p", link->id>=BAVC_SourceId_eHdDvi0?"avc/hdmi": link->id==BAVC_SourceId_eVdec0?"analog": link->id==BAVC_SourceId_e656In0?"656": "other(mpeg?)", (int)link->id, cfg->position.width, cfg->position.height, (void *)display, (void *)window));

    rc = BVDC_Window_GetDefaultSettings(window->windowId, &windowCfg);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_window;}

    /*
     * This is for the cases where the application is setting the per window heap using Set Window Settings API
     */
    if (cfg->heap && cfg->heap != pVideo->heap) {
        window->vdcHeap = windowCfg.hHeap = NEXUS_Display_P_CreateHeap(cfg->heap);
        window->vdcDeinterlacerHeap = window->vdcHeap;
    }
    else
    {
        unsigned deinterlacerHeapIndex;

        if (link->secureVideo) {
            windowHeapIndex = pVideo->moduleSettings.secure.videoWindowHeapIndex[window->display->index][window->index];
        }
        else {
            windowHeapIndex = pVideo->moduleSettings.videoWindowHeapIndex[window->display->index][window->index];
        }
        BDBG_MSG(("window zorder %d display index %d windowheapindex %d display main heap %d",
            window->index,window->display->index,windowHeapIndex,pVideo->moduleSettings.primaryDisplayHeapIndex ));
        if (windowHeapIndex >= NEXUS_MAX_HEAPS) {
            /* for non-memconfig platforms, or invalid param */
            windowHeapIndex = 0;
        }
        else if (!g_pCoreHandles->heap[windowHeapIndex].nexus) {
            BDBG_ERR(("no heap[%d] for display %d, window %d", windowHeapIndex, window->display->index, window->index));
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto err_window;
        }
        if(pVideo->moduleSettings.primaryDisplayHeapIndex != windowHeapIndex)
        {
           window->vdcHeap= windowCfg.hHeap = NEXUS_Display_P_CreateHeap(g_pCoreHandles->heap[windowHeapIndex].nexus);
        }

        if (link->secureVideo) {
            deinterlacerHeapIndex = pVideo->moduleSettings.secure.deinterlacerHeapIndex[window->display->index][window->index];
        }
        else {
            deinterlacerHeapIndex = pVideo->moduleSettings.deinterlacerHeapIndex[window->display->index][window->index];
        }
        /* this should only occur to the newer chips using platform memconfig which removed primary display heap */
        if (deinterlacerHeapIndex < NEXUS_MAX_HEAPS && deinterlacerHeapIndex != windowHeapIndex) {
            window->vdcDeinterlacerHeap = windowCfg.hDeinterlacerHeap = NEXUS_Display_P_CreateHeap(g_pCoreHandles->heap[deinterlacerHeapIndex].nexus);
        }
    }

    /* BOXMODE: boxmode driven, pre-alloc fullscreen capture memory for smooth
     * scaling.*/
    windowCfg.bAllocFullScreen = NEXUS_VideoWindow_IsSmoothScaling_isrsafe(window)?true:cfg->allocateFullScreen;

    if (cfg->minimumSourceFormat != NEXUS_VideoFormat_eUnknown) {
        BFMT_VideoFmt fmt;
        rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(cfg->minimumSourceFormat, &fmt);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_window;}
        windowCfg.pMinSrcFmt = BFMT_GetVideoFormatInfoPtr(fmt);
    }
    if (cfg->minimumDisplayFormat != NEXUS_VideoFormat_eUnknown) {
        BFMT_VideoFmt fmt;
        rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(cfg->minimumDisplayFormat, &fmt);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_window;}
        windowCfg.pMinDspFmt = BFMT_GetVideoFormatInfoPtr(fmt);
    }

    if(g_NEXUS_DisplayModule_State.moduleSettings.memConfig[window->display->index].window[window->windowId].deinterlacer == NEXUS_DeinterlacerMode_eBestQuality)
    {
        /* This causes MAD to allocate 5 fields. This allows apps to be written using NEXUS_VideoWindowGameMode_e5Fields_ForceSpatial instead of having
        to dynamically learn how many buffers are in use in order to use game mode. */
        windowCfg.bDeinterlacerAllocFull  = true;
    }
    else
        windowCfg.bDeinterlacerAllocFull  = false;

    windowCfg.bBypassVideoProcessings = window->bypassVideoProcessing;

    windowId = (display->index < 2) ? window->windowId : BVDC_WindowId_eAuto;
#if NEXUS_HAS_SAGE
    if (link->secureVideo)
    {
        BVDC_Window_GetCores(windowId, g_pCoreHandles->box, link->sourceVdc, window->display->compositor, &window->sage.coreList);
        rc = NEXUS_Sage_AddSecureCores(&window->sage.coreList);
        if (rc) {rc = BERR_TRACE(rc); goto err_window;}
    }
#endif

    rc = BVDC_Window_Create( display->compositor, &window->vdcState.window,
                             windowId,
                             link->sourceVdc, &windowCfg);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_createwindow;}
    BDBG_ASSERT(window->vdcState.window);

    /* will call NEXUS_VideoWindow_P_SetCbSetting in NEXUS_VideoWindow_P_SetVdcSettings */
    rc = BVDC_Window_InstallCallback(window->vdcState.window, NEXUS_VideoWindow_P_Callback_isr, window, 0);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto err_postcreate;
    }

    NEXUS_VideoWindow_P_PredictSyncLock(window);

    /* Do not apply any Nexus settings to the window here. See NEXUS_VideoWindow_P_SetVdcSettings for that code. */

#if BVDC_BUF_LOG && NEXUS_BASE_OS_linuxuser
    rc = NEXUS_VideoWindow_P_EnableMultiBufLog(window, NEXUS_VideoBufLog_Trigger_eAutomatic, true);
    if (rc != NEXUS_SUCCESS) goto err_postcreate;
#endif

    BDBG_MSG(("<window:%p", (void *)window->vdcState.window));
    return BERR_SUCCESS;

err_postcreate:
    BVDC_Window_Destroy(window->vdcState.window);
    window->vdcState.window = NULL;
err_createwindow:
#if NEXUS_HAS_SAGE
    if (link->secureVideo)
    {
        NEXUS_Sage_RemoveSecureCores(&window->sage.coreList);
    }
#endif
err_window:
    return rc;
}


void
NEXUS_VideoWindow_P_DestroyVdcWindow(NEXUS_VideoWindowHandle window)
{
    BERR_Code rc;
    NEXUS_VideoInput_P_Link *link;

    BDBG_ASSERT(window->vdcState.window);

#if BVDC_BUF_LOG && NEXUS_BASE_OS_linuxuser
    rc = NEXUS_VideoWindow_P_EnableMultiBufLog(window, NEXUS_VideoBufLog_Trigger_eReset, false);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); }
#endif

    rc = BVDC_Window_Destroy(window->vdcState.window);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); }
    window->vdcState.window = NULL;

    window->phaseDelay = -1;
    window->syncStatus.delayValid = false;

    /* may need to throw SetMasterFrameRate to other window */
    (void)NEXUS_VideoWindow_P_ConfigMasterFrameRate(window, &window->display->cfg, &window->cfg);

    if(g_NEXUS_DisplayModule_State.updateMode != NEXUS_DisplayUpdateMode_eAuto) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);}
    rc = BVDC_ApplyChanges(pVideo->vdc);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); }

    if (window->status.isSyncLocked) {
        unsigned i;
        window->status.isSyncLocked = false;
        /* attempt to throw isSyncLocked = true to another window on this display */
        for (i=0;i<NEXUS_NUM_VIDEO_WINDOWS;i++) {
            NEXUS_VideoWindowHandle w = &window->display->windows[i];
            if (w != window && w->open && nexus_p_synclock_capable(w->input, w)) {
                w->status.isSyncLocked = true;
                BDBG_WRN(("throw synclock to window %d.%d", w->display->index, w->index));
                break;
            }
        }
        /* attempt to throw isSyncLocked = true to window for same input on another display */
        if (i == NEXUS_NUM_VIDEO_WINDOWS) {
            for (i=0;i<NEXUS_NUM_DISPLAYS;i++) {
                NEXUS_VideoWindowHandle w;
                if (!pVideo->displays[i] || i == window->display->index) continue;
                /* assume window has same index on both displays */
                w = &pVideo->displays[i]->windows[window->index];
                if (w->open && w->input == window->input) {
                    w->status.isSyncLocked = true;
                    /* this is normal on shutdown, so don't print WRN */
                    BDBG_MSG(("throw synclock to window %d.%d", w->display->index, w->index));
                    break;
                }
            }
        }
    }

    if (window->vdcDeinterlacerHeap) {
        if (window->vdcDeinterlacerHeap != window->vdcHeap) {
            NEXUS_Display_P_DestroyHeap(window->vdcDeinterlacerHeap);
        }
        window->vdcDeinterlacerHeap = NULL;
    }
    if (window->vdcHeap) {
        NEXUS_Display_P_DestroyHeap(window->vdcHeap);
        window->vdcHeap = NULL;
    }

#if NEXUS_HAS_SAGE
    BDBG_OBJECT_ASSERT(window->input, NEXUS_VideoInput);
    link = window->input->destination;
    if (link->secureVideo)
    {
        NEXUS_Sage_RemoveSecureCores(&window->sage.coreList);
    }
#else
    BSTD_UNUSED(link);
#endif

    return;
}

void NEXUS_VideoWindow_GetSettings_priv( NEXUS_VideoWindowHandle handle, NEXUS_VideoWindowSettings *pSettings )
{
    NEXUS_ASSERT_MODULE();
    /* call _impl version */
    NEXUS_VideoWindow_GetSettings(handle, pSettings);
}

void
NEXUS_VideoWindow_GetSettings(NEXUS_VideoWindowHandle window, NEXUS_VideoWindowSettings *settings)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    BDBG_ASSERT(settings);
    *settings = window->cfg;
    return;
}

NEXUS_Error
NEXUS_VideoWindow_AddInput(NEXUS_VideoWindowHandle window, NEXUS_VideoInput input)
{
    NEXUS_Error rc;
    NEXUS_VideoInput_P_Link *link;
    struct nexus_synclock_recreate recreate;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);

    NEXUS_OBJECT_ASSERT(NEXUS_VideoInput, input);

    BDBG_MODULE_MSG(nexus_flow_video_window, ("%p add input %p, source %p", (void *)window, (void *)input, (void *)input->source));
    /* verify there is no existing connection for this window */
    if (window->input!=NULL) {
        BDBG_ERR(("Input already in use!"));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_sanity_check;
    }

    /* first, see if we need to recreate for desired sync lock */
    NEXUS_VideoWindow_P_DestroyForSyncLock(window, window->cfg.preferSyncLock,  input, &recreate);

#if NEXUS_NUM_MOSAIC_DECODES
    if (window->mosaic.parent) {
        rc = NEXUS_VideoWindow_P_AddMosaicInput(window, input);
        if (rc) {rc = BERR_TRACE(rc); goto err_add_mosaic;}
        /* mosaics can now be connected as a normal input/input, but do not create the VDC window */
    }
#endif

    link = NEXUS_VideoInput_P_GetForWindow(input, window);
    if (!link) {
        /* The only case where NEXUS_VideoInput_P_Get can fail is if this NEXUS_VideoInput is
        connected to another module. As of now, that case doesn't exist, but this check would be needed. */
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_link;
    }

    /* we must connect before calling setup. this allows inputs to create/destroy their VDC source
    on connect/disconnect instead of requiring them to always be created. */
    if (link->ref_cnt == 0) {
        rc = NEXUS_VideoInput_P_Connect(input);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_connect;}
    }
    link->ref_cnt++;
    window->input = input;
    BDBG_ASSERT(input->destination == link);

#if NEXUS_NUM_MOSAIC_DECODES
    if (!window->mosaic.parent)
#endif
    {
        rc = NEXUS_VideoWindow_P_CreateVdcWindow(window, &window->cfg);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_setup;}
        BDBG_ASSERT(window->vdcState.window);

        /* now that the window is created, we can apply settings */
        rc = NEXUS_VideoWindow_P_SetVdcSettings(window, &window->cfg, true);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_setsettings;}

        rc = NEXUS_Display_P_ApplyChanges();
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_applychanges;}
    }

    NEXUS_VideoWindow_P_RecreateForSyncLock(&recreate);

    /* update display settings on the input side */
    BDBG_ASSERT(window->display);
#if NEXUS_HAS_VIDEO_ENCODER
    if(window->display->encodeUserData) {
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
        if(window->display->encoder.window == window)
#endif
        window->display->xudSource = window->input;
    }
#endif
    NEXUS_Display_P_VideoInputDisplayUpdate(NULL, window, &window->display->cfg);

#if NEXUS_NUM_MOSAIC_DECODES
    if (window->mosaic.parent) {
        rc = NEXUS_VideoWindow_P_ApplyMosaic(window->mosaic.parent);
        if (rc) {rc = BERR_TRACE(rc);} /* fall through */
    }
#endif
    NEXUS_OBJECT_ACQUIRE(window, NEXUS_VideoInput, input);

    return NEXUS_SUCCESS;

err_applychanges:
err_setsettings:
    NEXUS_VideoWindow_P_DestroyVdcWindow(window);
err_setup:
    BDBG_ASSERT(link->ref_cnt>0);
    if(--link->ref_cnt==0) {
        NEXUS_VideoInput_P_Disconnect(window->input);
    }
    window->input = NULL;
err_connect:
    /* reverse NEXUS_VideoInput_P_Get */
    NEXUS_VideoInput_Shutdown(input);
err_link:
#if NEXUS_NUM_MOSAIC_DECODES
    if (window->mosaic.parent) {
        NEXUS_VideoWindow_P_RemoveMosaicInput(window, input);
    }
err_add_mosaic:
#endif
    NEXUS_VideoWindow_P_RecreateForSyncLock(&recreate);
err_sanity_check:
    return rc;
}

NEXUS_Error
NEXUS_VideoWindow_RemoveInput(NEXUS_VideoWindowHandle window, NEXUS_VideoInput input)
{
    return NEXUS_VideoWindow_P_RemoveInput(window, input, false);
}

static NEXUS_Error
NEXUS_VideoWindow_P_RemoveInput(NEXUS_VideoWindowHandle window, NEXUS_VideoInput input, bool skipDisconnect)
{
    NEXUS_VideoInput_P_Link *link;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    BDBG_OBJECT_ASSERT(input, NEXUS_VideoInput);

    if(window->input!=input) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    BDBG_MODULE_MSG(nexus_flow_video_window, ("%p remove input %p", (void *)window, (void *)input));

    link = NEXUS_VideoInput_P_Get(window->input);
    if (!link) {
        BDBG_ERR(("NEXUS_VideoWindow_RemoveInput:%p invalid input %p", (void *)window, (void *)window->input));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

#if NEXUS_NUM_MOSAIC_DECODES
    if (window->mosaic.parent) {
        NEXUS_VideoWindow_P_RemoveMosaicInput(window, input);
    }
    else
#endif
    {
        NEXUS_VideoWindow_P_DestroyVdcWindow(window);
    }

    BDBG_ASSERT(link->ref_cnt>0);
    if (--link->ref_cnt==0) {
        if (!skipDisconnect) {
            NEXUS_VideoInput_P_Disconnect(window->input);
        }

#if NEXUS_NUM_MOSAIC_DECODES
        /* because each mosaic's link copies the VDC source of the parent, we must shutdown the mosaic on
        the last disconnect to clear the copy. */
        if (window->mosaic.parent) {
            NEXUS_VideoInput_Shutdown(input);
        }
#endif
    }

    window->input = NULL;

    NEXUS_OBJECT_RELEASE(window, NEXUS_VideoInput, input);

    return NEXUS_SUCCESS;
}

void
NEXUS_VideoWindow_RemoveAllInputs(NEXUS_VideoWindowHandle window)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);

    if(window->input) {
        rc = NEXUS_VideoWindow_RemoveInput(window, window->input);
        if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
    }
    return;
}

NEXUS_Error
NEXUS_VideoWindow_SetSettings(NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowSettings *settings)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    BDBG_OBJECT_ASSERT(window->display, NEXUS_Display);
    BDBG_ASSERT(settings);

    if (window->vdcState.window)
    {
        NEXUS_VideoWindowSettings oldSettings;
        struct nexus_synclock_recreate recreate;

        NEXUS_VideoWindow_P_DestroyForSyncLock(window, settings->preferSyncLock, window->input, &recreate);
        NEXUS_VideoWindow_P_RecreateForSyncLock(&recreate);

        rc = NEXUS_VideoWindow_P_SetVdcSettings(window, settings, false);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_setsettings;}

        /* We must save the settings here for VDB to read them in ApplyChanges.  If ApplyChanges fails, we will roll back. */
        oldSettings = window->cfg;
        window->cfg = *settings;

        rc = NEXUS_Display_P_ApplyChanges();
        if (rc!=BERR_SUCCESS)
        {
            rc = BERR_TRACE(rc);
            window->cfg = oldSettings;
            goto err_applychanges;
        }
    }
    else
    {
        /* Just store the settings */
        window->cfg = *settings;
    }

    if (window->letterBoxDetectionCallback) {
        NEXUS_IsrCallback_Set(window->letterBoxDetectionCallback, &settings->letterBoxDetectionChange);
    }
    if (window->outputRectChangedCallback) {
        NEXUS_IsrCallback_Set(window->outputRectChangedCallback, &settings->outputRectChanged);
    }

#if NEXUS_NUM_MOSAIC_DECODES
    if (window->mosaic.parent) {
        NEXUS_VideoWindow_P_ApplyMosaic(window->mosaic.parent);
    }
#endif
    return rc;
err_applychanges:
err_setsettings:
    {
        BERR_Code rc = BVDC_AbortChanges(pVideo->vdc);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    }
    return rc;
}

NEXUS_SurfaceHandle NEXUS_VideoWindow_CaptureVideoBuffer( NEXUS_VideoWindowHandle window )
{
    BVDC_Window_Handle  windowVDC;
    BVDC_Window_CapturedImage captureBuffer;
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_SurfaceHandle surface;
    unsigned tries = 5;

    BDBG_ENTER(NEXUS_VideoWindow_CaptureVideoBuffer);

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;

    if (NULL == windowVDC) /* window not connected */
    {
        BDBG_ERR(("Window not connnected"));
        BDBG_LEAVE(NEXUS_VideoWindow_CaptureVideoBuffer);
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }

    if (window->captureBuffer) {
        BDBG_ERR(("must return currently captured surface before capturing another"));
        return NULL;
    }

    /* set capture buffers used, should apply changes to take effect */
    if (window->cfg.userCaptureBufferCount == 0)
    {
        rc = BVDC_Window_SetUserCaptureBufferCount(windowVDC, 1);
        if (rc) {rc=BERR_TRACE(rc);return NULL;}
        rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
        if (rc) {rc=BERR_TRACE(rc);return NULL;}
    }

    if(g_NEXUS_DisplayModule_State.updateMode != NEXUS_DisplayUpdateMode_eAuto) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);}

    /* now that we've allocated a user capture buffer inside VDC, we must succeed or goto error_free_buffer to free that buffer. */

    /* Get the video buffer from VDC.
       Note that BVDC_Window_GetBuffer will set captureBuffer.hCaptureBuffer even if rc is non-zero.
       This is atypical behavior but must be accounted for. */
    while (tries--) {
        rc = BVDC_Window_GetBuffer(windowVDC, &captureBuffer);
        if (rc == BVDC_ERR_NO_AVAIL_CAPTURE_BUFFER) {
            /* Only a buffer at certain postion can be returned and it must have valid picture captured on it. Without this, tearing occurs.
            This tries just a few times with a slight delay. */
            BKNI_Delay(10 * 1000);
        }
        else {
            break;
        }
    }
    if ((rc != BERR_SUCCESS) || (captureBuffer.captureBuffer.hPixels == NULL))
    {
        rc = BERR_TRACE(rc);
        goto error_free_buffer;
    }

    /* store for later return to VDC. a new nexus API is needed to return it to the user. */
    window->captureImage = captureBuffer;

    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.surface);
    surface = NEXUS_Surface_CreateFromPixelPlane_priv(&captureBuffer.captureBuffer);
    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.surface);
    if(!surface) {
        BDBG_ERR(("unknown surface. was this a video capture buffer?"));
        goto error_free_buffer;
    }
    NEXUS_OBJECT_REGISTER(NEXUS_Surface, surface, Create);

    /* Save the buffer in the window private context */
    window->captureBuffer = surface;

    BDBG_LEAVE(NEXUS_VideoWindow_CaptureVideoBuffer);

    return surface;

error_free_buffer:
    if (captureBuffer.captureBuffer.hPixels != NULL) {
        rc = BVDC_Window_ReturnBuffer(windowVDC, &captureBuffer);
        if (rc) {rc = BERR_TRACE(rc);} /* fall through */
        window->captureImage.captureBuffer.hPixels = NULL;
        rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
        if (rc) {rc = BERR_TRACE(rc);} /* fall through */
    }

    if (window->cfg.userCaptureBufferCount == 0)
    {
        rc = BVDC_Window_SetUserCaptureBufferCount(windowVDC, 0);
        if (rc) {rc = BERR_TRACE(rc);} /* fall through */
        rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
        if (rc) {rc = BERR_TRACE(rc);} /* fall through */
    }

    return NULL;

}   /* NEXUS_VideoWindow_CaptureVideoBuffer() */

NEXUS_Error NEXUS_VideoWindow_ReleaseVideoBuffer( NEXUS_VideoWindowHandle window, NEXUS_SurfaceHandle surface )
{
    BVDC_Window_Handle  windowVDC;
    BERR_Code rc = BERR_SUCCESS;
    const BPXL_Plane *magnumSurface;

    BDBG_ENTER(NEXUS_VideoWindow_ReleaseVideoBuffer);

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;

    if(!surface || surface != window->captureBuffer) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (NULL == windowVDC) /* window not connected */
    {
        BDBG_ERR(("Window not connnected"));
        BDBG_LEAVE(NEXUS_VideoWindow_ReleaseVideoBuffer);
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.surface);
    magnumSurface = NEXUS_Surface_GetPixelPlane_priv(surface);
    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.surface);
    NEXUS_OBJECT_UNREGISTER(NEXUS_Surface, surface, Destroy);
    NEXUS_Surface_Destroy(surface);
    BDBG_ASSERT(magnumSurface);
    window->captureBuffer = NULL;

    {
        rc = BVDC_Window_ReturnBuffer(windowVDC, &window->captureImage);
        if (rc) return BERR_TRACE(rc);
        window->captureImage.captureBuffer.hPixels = NULL;
        window->captureImage.rCaptureBuffer.hPixels = NULL;
    }
    if(g_NEXUS_DisplayModule_State.updateMode != NEXUS_DisplayUpdateMode_eAuto) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);}
    rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
    if (rc) return BERR_TRACE(rc);

    if (window->cfg.userCaptureBufferCount == 0)
    {
        rc = BVDC_Window_SetUserCaptureBufferCount(windowVDC, 0);
        if (rc) return BERR_TRACE(rc);
        rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
        if (rc) return BERR_TRACE(rc);
    }

    return NEXUS_SUCCESS;
}

void
NEXUS_VideoWindow_GetSplitScreenSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowSplitScreenSettings *pSettings)
{
    *pSettings = window->splitScreenSettings;
}

NEXUS_Error
NEXUS_VideoWindow_SetSplitScreenSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowSplitScreenSettings *pSettings
    )
{
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    BDBG_ASSERT(pSettings);

    /* save for later */
    if (!window->vdcState.window) {
        window->splitScreenSettings = *pSettings;
        return NEXUS_SUCCESS;
    }

    window->splitScreenSettings = *pSettings;
    rc = NEXUS_VideoWindow_P_ApplySplitScreenSettings(window);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    return 0;
}

static NEXUS_Error NEXUS_VideoWindow_P_ApplySplitScreenSettings(NEXUS_VideoWindowHandle window)
{
    BERR_Code rc;
    BVDC_Window_Handle windowVDC;
    NEXUS_VideoInput_P_Link *pLink;
    BVDC_Window_SplitScreenSettings winSettings;
    BVDC_Source_SplitScreenSettings srcSettings;
    NEXUS_VideoWindowSplitScreenSettings *pSettings = &window->splitScreenSettings;
    int i =0,j=0;

    BDBG_ENTER(NEXUS_VideoWindow_SetSplitScreenSettings);

    windowVDC = window->vdcState.window;
    BDBG_ASSERT(windowVDC);
    pLink = NEXUS_VideoInput_P_Get(window->input);

    /* Check for 1st window of primary display (PEP_WINDOW restriction in BVDC_Window_SetSplitScreenMode) */
    if ((window->display->index == 0) && (window->windowId == BVDC_WindowId_eVideo0))
    {
        (void)BVDC_Window_GetSplitScreenMode(windowVDC, &winSettings);
        winSettings.eHue = pSettings->hue;
        winSettings.eContrast = pSettings->contrast;
        winSettings.eBrightness = pSettings->brightness;
        winSettings.eColorTemp = pSettings->colorTemp;

        winSettings.eAutoFlesh = pSettings->autoFlesh;
        winSettings.eSharpness = pSettings->sharpness;
        winSettings.eBlueBoost = pSettings->blueBoost;
        winSettings.eGreenBoost = pSettings->greenBoost;
        winSettings.eBlueStretch = pSettings->blueStretch;
        winSettings.eCms = pSettings->cms;
        winSettings.eContrastStretch = pSettings->contrastStretch;
        winSettings.eAnr = pSettings->anr;
        winSettings.eDeJagging = pSettings->dejagging;
        winSettings.eDeRinging = pSettings->deringing;
        rc = BVDC_Window_SetSplitScreenMode(windowVDC, &winSettings);
        if (rc) return BERR_TRACE(rc);
    }

    if (pLink && pLink->sourceVdc) {
        (void)BVDC_Source_GetSplitScreenMode(pLink->sourceVdc, &srcSettings);

        srcSettings.eDnr = BVDC_SplitScreenMode_eDisable;

        for (i=0;i<NEXUS_NUM_DISPLAYS && srcSettings.eDnr==BVDC_SplitScreenMode_eDisable;i++) {
            if (!pVideo->displays[i]) continue;
            for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++) {
                NEXUS_VideoWindowHandle window = &pVideo->displays[i]->windows[j];
                if (window->open && window->input == pLink->input) {
                    if (window->splitScreenSettings.dnr) {
                        srcSettings.eDnr = window->splitScreenSettings.dnr;
                        BDBG_MSG(("display %d window %d DNR set to %d",i, j, srcSettings.eDnr));
                        break;
                    }
                }
            }
        }

        rc = BVDC_Source_SetSplitScreenMode(pLink->sourceVdc, &srcSettings);
        if (rc) return BERR_TRACE(rc);
    }
    /* don't ApplyChanges here */
    return NEXUS_SUCCESS;
}

NEXUS_VideoWindowHandle
NEXUS_VideoWindow_Open(NEXUS_DisplayHandle display, unsigned windowIndex)
{
    BERR_Code rc;
    NEXUS_VideoWindowHandle window;
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    if (windowIndex >= pVideo->cap.display[display->index].numVideoWindows) {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }
    window = &display->windows[windowIndex];

    if (window->open) {
        BDBG_ERR(("window already open"));
        return NULL;
    }

    NEXUS_VideoWindow_P_InitState(window, windowIndex, windowIndex, display);
    window->letterBoxDetectionCallback = NEXUS_IsrCallback_Create(window, NULL);
    window->outputRectChangedCallback = NEXUS_IsrCallback_Create(window, NULL);
    BDBG_CASSERT(BVDC_WindowId_eVideo0 == 0);
    BDBG_CASSERT(BVDC_WindowId_eVideo1 == 1);
    window->windowId = windowIndex + BVDC_WindowId_eVideo0;
    window->phaseDelay = -1; /* invalid */

    if (BKNI_CreateEvent(&window->syncLockEvent)) {
        BDBG_ERR(("window sync lock event fault"));
        goto error;
    }

    window->syncLockCallback = NEXUS_RegisterEvent(window->syncLockEvent, &NEXUS_VideoWindow_P_SyncLockEventHandler, window);

    if (BKNI_CreateEvent(&window->lb_event)) {
        BDBG_ERR(("window lb event fault"));
        goto error;
    }

#if BVDC_BUF_LOG && NEXUS_BASE_OS_linuxuser
    if (BKNI_CreateEvent(&window->bufLogEvent)) {
        BDBG_ERR(("window buffer log event fault"));
        goto error;
    }

    window->bufLogCallback = NEXUS_RegisterEvent(window->bufLogEvent, &NEXUS_VideoWindow_P_MultiBufLogEventHandler, window);
#endif

#if NEXUS_NUM_MOSAIC_DECODES
    BLST_S_INIT(&window->mosaic.children);
#endif

    NEXUS_OBJECT_REGISTER(NEXUS_VideoWindow, window, Create);
    BDBG_MODULE_MSG(nexus_flow_video_window, ("open %p, display %d, window %d", (void *)window, window->display->index, window->index));
    return window;

error:
    NEXUS_VideoWindow_Close(window);
    return NULL;
}

static void
NEXUS_VideoWindow_P_Release(NEXUS_VideoWindowHandle window)
{
    if (!window->open) {
        BDBG_ERR(("invalid window"));
    }

    NEXUS_OBJECT_UNREGISTER(NEXUS_VideoWindow, window, Destroy);
    BDBG_MODULE_MSG(nexus_flow_video_window, ("close %p", (void *)window));
    NEXUS_VideoWindow_RemoveAllInputs(window);
#if NEXUS_NUM_MOSAIC_DECODES
    if (window->mosaic.parent==NULL) {
        NEXUS_VideoWindowHandle mosaicChild;
        while ((mosaicChild = BLST_S_FIRST(&window->mosaic.children))) {
            BDBG_WRN(("NEXUS_VideoWindow_Close is automatically closing mosaic window %p", (void *)mosaicChild));
            NEXUS_VideoWindow_Close(mosaicChild);
        }
    }
#endif
    return;
}

static void
NEXUS_VideoWindow_P_Finalizer(NEXUS_VideoWindowHandle window)
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoWindow, window);

#if NEXUS_HAS_SYNC_CHANNEL
    if (window->syncSettings.closeCallback_isr)
    {
        BKNI_EnterCriticalSection();
        window->syncSettings.closeCallback_isr(window->syncSettings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }
#endif

    NEXUS_VideoWindow_P_UnsetCbSetting(window);

    if (window->adjContext.customDnrData) {
        BKNI_Free(window->adjContext.customDnrData);
    }
    if (window->adjContext.customAnrData) {
        BKNI_Free(window->adjContext.customAnrData);
    }
    if (window->picContext.customContrastStretchData) {
        BKNI_Free(window->picContext.customContrastStretchData);
    }

#if NEXUS_NUM_MOSAIC_DECODES
    if (window->mosaic.parent) {
        BKNI_EnterCriticalSection();
        BLST_S_REMOVE(&window->mosaic.parent->mosaic.children, window, NEXUS_VideoWindow, mosaic.link);
        BKNI_LeaveCriticalSection();
        BDBG_OBJECT_DESTROY(window, NEXUS_VideoWindow);
        BKNI_Free(window);
        return;
    }
#endif
    NEXUS_IsrCallback_Destroy(window->letterBoxDetectionCallback);
    NEXUS_IsrCallback_Destroy(window->outputRectChangedCallback);
    BKNI_DestroyEvent(window->lb_event);
    if (window->syncLockCallback)
    {
        NEXUS_UnregisterEvent(window->syncLockCallback);
        window->syncLockCallback = NULL;
    }
    if (window->syncLockEvent)
    {
        BKNI_DestroyEvent(window->syncLockEvent);
        window->syncLockEvent = NULL;
    }

#if BVDC_BUF_LOG && NEXUS_BASE_OS_linuxuser
    if (window->bufLogCallback)
    {
        NEXUS_UnregisterEvent(window->bufLogCallback);
        window->bufLogCallback = NULL;
    }
    if (window->bufLogEvent)
    {
        BKNI_DestroyEvent(window->bufLogEvent);
        window->bufLogEvent = NULL;
    }
#endif

    NEXUS_OBJECT_DESTROY(NEXUS_VideoWindow, window);
    window->open = false;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_VideoWindow, NEXUS_VideoWindow_Close);

void NEXUS_VideoWindow_GetSyncSettings_priv( NEXUS_VideoWindowHandle window, NEXUS_VideoWindowSyncSettings *pSyncSettings )
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    NEXUS_ASSERT_MODULE();
    *pSyncSettings = window->syncSettings;
}

NEXUS_Error NEXUS_VideoWindow_SetSyncSettings_priv( NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowSyncSettings *pSyncSettings )
{
    BERR_Code rc;
    BVDC_Window_Handle windowVdc;
    bool hookingUp = false;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    NEXUS_ASSERT_MODULE();

    hookingUp = (window->syncSettings.stateChangeCallback_isr == NULL &&
        pSyncSettings->stateChangeCallback_isr != NULL);

    window->syncSettings = *pSyncSettings;

    if (!window->mosaic.parent)
    {
        windowVdc = window->vdcState.window;
    }
    else
    {
        windowVdc = window->mosaic.parent->vdcState.window;
    }
    if (windowVdc)
    {
        rc = BVDC_Window_SetDelayOffset(windowVdc, window->cfg.delay + window->syncSettings.delay);
        if (rc) {return BERR_TRACE(rc);}

        rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
        if (rc) {return BERR_TRACE(rc);}

        /* fire some callbacks to set SyncChannel's initial state */
        if (hookingUp) {
            BKNI_EnterCriticalSection();
            if (window->syncSettings.stateChangeCallback_isr) {
                (*window->syncSettings.stateChangeCallback_isr)(window->syncSettings.callbackContext, 0);
            }
            if (window->syncSettings.formatCallback_isr) {
                (*window->syncSettings.formatCallback_isr)(window->syncSettings.callbackContext, 0);
            }
            if (window->syncSettings.delayCallback_isr) {
                (*window->syncSettings.delayCallback_isr)(window->syncSettings.callbackContext, window->status.delay);
            }
            BKNI_LeaveCriticalSection();
        }
    }

    return 0;
}

bool NEXUS_VideoWindow_HasOutput_isr(NEXUS_VideoWindowHandle window, NEXUS_VideoOutputType type)
{
    return NEXUS_Display_P_HasOutput_isr(window->display, type);
}

NEXUS_Error NEXUS_VideoWindow_GetSyncStatus_isr( NEXUS_VideoWindowHandle window, NEXUS_VideoWindowSyncStatus *pSyncStatus )
{
    BFMT_VideoInfo video_format_info;
    unsigned long ulFrameRate;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);

    /* translate display status on demand */
    rc = NEXUS_P_Display_GetMagnumVideoFormatInfo_isr(window->display, window->display->cfg.format, &video_format_info);
    if (rc) {return BERR_TRACE(rc);}
    /* master frame rate sync status is set in ConfigMasterFrameRate */
    window->syncStatus.height = video_format_info.ulDigitalHeight;
    window->syncStatus.interlaced = video_format_info.bInterlaced;
    window->syncStatus.delay = window->status.delay;
    window->syncStatus.phaseDelay = window->status.phaseDelay;
    window->syncStatus.syncLocked = window->status.isSyncLocked;
    window->syncStatus.aligned = window->display->cfg.alignmentTarget && window->display->cfg.alignmentTarget != window->display;
    window->syncStatus.refreshRate = window->display->status.refreshRate;
    ulFrameRate = video_format_info.bInterlaced ? window->syncStatus.refreshRate / 2 : window->syncStatus.refreshRate;
    switch (ulFrameRate) {
    case 12500: window->syncStatus.frameRate = BAVC_FrameRateCode_e12_5; break;
    case 19980: window->syncStatus.frameRate = BAVC_FrameRateCode_e19_98; break;
    case 20000: window->syncStatus.frameRate = BAVC_FrameRateCode_e20; break;
    case 23976: window->syncStatus.frameRate = BAVC_FrameRateCode_e23_976; break;
    case 24000: window->syncStatus.frameRate = BAVC_FrameRateCode_e24; break;
    case 25000: window->syncStatus.frameRate = BAVC_FrameRateCode_e25; break;
    case 29970: window->syncStatus.frameRate = BAVC_FrameRateCode_e29_97; break;
    case 30000: window->syncStatus.frameRate = BAVC_FrameRateCode_e30; break;
    case 50000: window->syncStatus.frameRate = BAVC_FrameRateCode_e50; break;
    case 59940: window->syncStatus.frameRate = BAVC_FrameRateCode_e59_94; break;
    case 60000: window->syncStatus.frameRate = BAVC_FrameRateCode_e60; break;
    case 100000: window->syncStatus.frameRate = BAVC_FrameRateCode_e100; break;
    case 119880: window->syncStatus.frameRate = BAVC_FrameRateCode_e119_88; break;
    case 120000: window->syncStatus.frameRate = BAVC_FrameRateCode_e120; break;
    default: window->syncStatus.frameRate = BAVC_FrameRateCode_eUnknown; break;
    }

    *pSyncStatus = window->syncStatus;

    return 0;
}

void NEXUS_VideoWindow_GetColorMatrix( NEXUS_VideoWindowHandle window, NEXUS_ColorMatrix *pSettings)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->colorMatrix;
}

NEXUS_Error NEXUS_VideoWindow_SetColorMatrix( NEXUS_VideoWindowHandle window, const NEXUS_ColorMatrix *pSettings )
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);

    if (pSettings) {
        window->colorMatrix = *pSettings;
    }
    window->colorMatrixSet = true;
    window->colorMatrixOverride = pSettings != NULL;
    if (window->vdcState.window) {
        BERR_Code rc;
        rc = BVDC_Window_SetColorMatrix(window->vdcState.window, window->colorMatrixOverride, window->colorMatrix.coeffMatrix, window->colorMatrix.shift);
        if (rc) return BERR_TRACE(rc);

        rc = NEXUS_Display_P_ApplyChanges();
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

NEXUS_Error NEXUS_VideoWindow_GetStatus( NEXUS_VideoWindowHandle window, NEXUS_VideoWindowStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);

    /* window->status is incrementally updated from various isr callbacks from VDC, so we
    start with this copy */
    *pStatus = window->status;

    return 0;
}


NEXUS_Error NEXUS_VideoWindow_GetLetterBoxStatus( NEXUS_VideoWindowHandle window, NEXUS_VideoWindowLetterBoxStatus *pStatus )
{
    *pStatus = window->letterBoxStatus;
    return 0;
}

void NEXUS_GetDefaultCalculateVideoWindowPositionSettings( NEXUS_CalculateVideoWindowPositionSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->manualAspectRatioCorrection = NEXUS_VideoWindowContentMode_eFull;
    pSettings->sourceAspectRatio = NEXUS_AspectRatio_e4x3;
    pSettings->viewport.width = 1920;
    pSettings->viewport.height = 1080;
    pSettings->displayAspectRatio = NEXUS_DisplayAspectRatio_eAuto;
}

NEXUS_Error NEXUS_CalculateVideoWindowPosition( const NEXUS_CalculateVideoWindowPositionSettings *pPosition,
    const NEXUS_VideoWindowSettings *pInputWindowSettings, NEXUS_VideoWindowSettings *pWindowSettings)
{
    unsigned clip;
    NEXUS_Rect viewport = pPosition->viewport; /* may need to modify for manual a/r correction */
    unsigned verticalClipping = pPosition->verticalClipping; /* may need to modify for manual a/r correction */
    unsigned horizontalClipping = pPosition->horizontalClipping; /* may need to modify for manual a/r correction */
    unsigned displayAr, sourceAr; /* aspect ratio in 1/100th's units (e.g. 4:3 = 4/3 = 133 */
    unsigned aspectNumerator, aspectDenominator;

    *pWindowSettings = *pInputWindowSettings;

    if (!pPosition->displayWidth || !pPosition->displayHeight) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* store AR as width/height in 1/100ths.
    This has the advantage of already being in units of a horizontal percentage for clipping-based zoom. */
    switch (pPosition->displayAspectRatio) {
    case NEXUS_DisplayAspectRatio_eAuto:
        displayAr = pPosition->displayWidth * 100 / pPosition->displayHeight;
        if (!displayAr) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    case NEXUS_DisplayAspectRatio_e4x3:
        displayAr = 4 * 100 / 3;
        break;
    case NEXUS_DisplayAspectRatio_e16x9:
        displayAr = 16 * 100 / 9;
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    switch (pPosition->sourceAspectRatio) {
    case NEXUS_AspectRatio_e4x3:
        sourceAr = 400 / 3;
        aspectNumerator = 4;
        aspectDenominator = 3;
        break;
    case NEXUS_AspectRatio_e16x9:
        sourceAr = 1600 / 9;
        aspectNumerator = 16;
        aspectDenominator = 9;
        break;
    case NEXUS_AspectRatio_eSquarePixel:
        /* square pixel is the same as SAR 1:1 */
        if (pPosition->sourceHeight) {
            sourceAr = pPosition->sourceWidth * 100 / pPosition->sourceHeight;
            aspectNumerator = pPosition->sourceWidth;
            aspectDenominator = pPosition->sourceHeight;
        }
        else {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        break;
    case NEXUS_AspectRatio_e221x1:
        sourceAr = 221;
        aspectNumerator = 221;
        aspectDenominator = 100;
        break;
    case NEXUS_AspectRatio_e15x9:
        sourceAr = 1500 / 9;
        aspectNumerator = 15;
        aspectDenominator = 9;
        break;
    case NEXUS_AspectRatio_eSar:
        if (pPosition->sampleAspectRatio.x && pPosition->sampleAspectRatio.y && pPosition->sourceWidth && pPosition->sourceHeight) {
            sourceAr = pPosition->sourceWidth * 100 * pPosition->sampleAspectRatio.x / pPosition->sampleAspectRatio.y / pPosition->sourceHeight;
            aspectNumerator = pPosition->sourceWidth * pPosition->sampleAspectRatio.x;
            aspectDenominator = pPosition->sourceHeight * pPosition->sampleAspectRatio.y;
        }
        else {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        break;
    default:
        sourceAr = 0;
        aspectNumerator = 0;
        aspectDenominator = 1;
        break;
    }

    /**
    Manual aspect ratio correction involves either changes in the viewport or in clipping.
    **/
    switch (pPosition->manualAspectRatioCorrection) {
    case NEXUS_VideoWindowContentMode_eBox:
        if (!sourceAr) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }

        /* if display and source ARs are equal, no letter/pillar boxing required */
        if(displayAr != sourceAr)
        {
            if (displayAr > sourceAr) {
                /* pillar box */
                unsigned w = viewport.height * aspectNumerator / aspectDenominator;
                viewport.x += (viewport.width - w) / 2;
                viewport.width = w;
            }
            else {
                /* letter box */
                unsigned h = viewport.width * aspectDenominator / aspectNumerator;
                viewport.y += (viewport.height - h) / 2;
                viewport.height = h;
            }
        }
        pWindowSettings->contentMode = NEXUS_VideoWindowContentMode_eFull;
        break;
    case NEXUS_VideoWindowContentMode_eZoom:
        if (!sourceAr) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }

        /* if display and source ARs are equal, no adjustment required */
        if(displayAr != sourceAr)
        {
            if (displayAr > sourceAr) {
                /* vertical clipping - convert to height/width, then do the 1/100ths based math */
                verticalClipping += (100 * 100 / sourceAr) - (100 * 100 / displayAr);
            }
            else {
                /* horizontal clipping - units of sourceAr & displayAr are ready for direct math */
                horizontalClipping += sourceAr - displayAr;
            }
        }
        pWindowSettings->contentMode = NEXUS_VideoWindowContentMode_eFull;
        break;
    case NEXUS_VideoWindowContentMode_eFull:
        pWindowSettings->contentMode = NEXUS_VideoWindowContentMode_eFull;
        break;
    case NEXUS_VideoWindowContentMode_eFullNonLinear:
    case NEXUS_VideoWindowContentMode_ePanScan:
    case NEXUS_VideoWindowContentMode_ePanScanWithoutCorrection:
        /* only auto a/r correction supported for these modes */
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    default:
        /* invalid value */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }


    /* use display coordinates as clipBase */
    pWindowSettings->clipBase.x = 0; /* unused */
    pWindowSettings->clipBase.y = 0; /* unused */
    pWindowSettings->clipBase.width = pPosition->displayWidth;
    pWindowSettings->clipBase.height = pPosition->displayHeight;

    /* start off with no clipping */
    pWindowSettings->clipRect = pWindowSettings->clipBase;

    /* apply user's clipping */
    clip = pWindowSettings->clipBase.width * horizontalClipping / 10000;
    pWindowSettings->clipRect.x += clip/2;
    pWindowSettings->clipRect.width -= clip;
    clip = pWindowSettings->clipBase.height * verticalClipping / 10000;
    pWindowSettings->clipRect.y += clip/2;
    pWindowSettings->clipRect.height -= clip;

    /* apply h/v position while maintaining the viewport and not changing scaling.
    let x/y go negative for now. */
    pWindowSettings->clipRect.x -= pPosition->horizontalPosition;
    pWindowSettings->clipRect.y -= pPosition->verticalPosition;

    /* convert viewport to window position, making adjustments to clipping as needed */
    pWindowSettings->position = viewport;

    /* check if offscreen */
    if (viewport.x >= (int)pPosition->displayWidth ||
        viewport.x + viewport.width <= 0 ||
        viewport.y >= (int)pPosition->displayHeight ||
        viewport.y + viewport.height <= 0)
    {
        pWindowSettings->position = pWindowSettings->clipRect = pWindowSettings->clipBase;
        pWindowSettings->position.width = pWindowSettings->position.height = 0;
    }
    else {
        if (viewport.x < 0) {
            int temp;
            pWindowSettings->position.x = 0;
            pWindowSettings->position.width += viewport.x;

            temp = (-viewport.x) * pWindowSettings->clipBase.width / viewport.width;
            pWindowSettings->clipRect.x += temp;
            pWindowSettings->clipRect.width -= temp;
        }
        if (viewport.y < 0) {
            int temp;
            pWindowSettings->position.y = 0;
            pWindowSettings->position.height += viewport.y;

            temp = (-viewport.y) * pWindowSettings->clipBase.height / viewport.height;
            pWindowSettings->clipRect.y += temp;
            pWindowSettings->clipRect.height -= temp;
        }
        if (viewport.x + viewport.width > (int)pPosition->displayWidth) {
            pWindowSettings->position.width -= viewport.x + viewport.width - pPosition->displayWidth;
            pWindowSettings->clipRect.width -= (viewport.width - pWindowSettings->position.width) * pWindowSettings->clipBase.width / viewport.width;
        }
        if (viewport.y + viewport.height > (int)pPosition->displayHeight) {
            pWindowSettings->position.height -= viewport.y + viewport.height - pPosition->displayHeight;
            pWindowSettings->clipRect.height -= (viewport.height - pWindowSettings->position.height) * pWindowSettings->clipBase.height / viewport.height;
        }
    }

    BDBG_MSG_TRACE(("NEXUS_CalculateVideoWindowPosition %d,%d,%d,%d; %d,%d,%d,%d; %d,%d,%d,%d",
        pWindowSettings->position.x, pWindowSettings->position.y, pWindowSettings->position.width, pWindowSettings->position.height,
        pWindowSettings->clipRect.x, pWindowSettings->clipRect.y, pWindowSettings->clipRect.width, pWindowSettings->clipRect.height,
        pWindowSettings->clipBase.x, pWindowSettings->clipBase.y, pWindowSettings->clipBase.width, pWindowSettings->clipBase.height));
    /* verify that our math is right */
    BDBG_ASSERT(pWindowSettings->position.x >= 0);
    BDBG_ASSERT(pWindowSettings->position.y >= 0);
    BDBG_ASSERT(pWindowSettings->position.width <= pPosition->displayWidth);
    BDBG_ASSERT(pWindowSettings->position.height <= pPosition->displayHeight);

    return 0;
}

bool NEXUS_VideoAdj_P_DefaultMadEnabled_priv(NEXUS_VideoWindowHandle window)
{
    if (!g_pCoreHandles->boxConfig->stBox.ulBoxId) {
        /* if no box mode, only default single MAD for main window on main display */
        return (window->index == 0 && window->display->index == 0);
    }
    else {
        BDBG_ASSERT(window->display->index < BBOX_VDC_DISPLAY_COUNT);
        BDBG_ASSERT(window->index < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY);
        /* default on if BBOX and Memconfig allow MAD */
        return (g_pCoreHandles->boxConfig->stVdc.astDisplay[window->display->index].astWindow[window->index].stResource.ulMad != BBOX_Vdc_Resource_eInvalid) &&
               (g_NEXUS_DisplayModule_State.moduleSettings.memConfig[window->display->index].window[window->index].deinterlacer != NEXUS_DeinterlacerMode_eNone);
    }
}


void NEXUS_VideoWindow_GetDefaultMinDisplayFormat_isrsafe(NEXUS_VideoWindowHandle window, NEXUS_VideoFormat *pMinDisplayFormat)
{
    if (NEXUS_VideoWindow_IsSmoothScaling_isrsafe(window)) {
        *pMinDisplayFormat = g_NEXUS_DisplayModule_State.moduleSettings.memConfig[window->display->index].maxFormat;
    }
    else {
        *pMinDisplayFormat = NEXUS_VideoFormat_eUnknown;
    }
}

void NEXUS_VideoWindow_P_InitState(NEXUS_VideoWindowHandle window, unsigned parentIndex, unsigned index, NEXUS_DisplayHandle display)
{
    NEXUS_OBJECT_INIT(NEXUS_VideoWindow, window);
    window->open = true;
    window->display = display;
    window->index = parentIndex; /* have explicit window->index instead of VDC's windowId for dereferencing arrays, etc. */
    NEXUS_VideoWindow_GetDefaultMadSettings(&window->adjContext.stMadSettings);
    window->adjContext.stMadSettings.deinterlace = NEXUS_VideoAdj_P_DefaultMadEnabled_priv(window);
    if (window->adjContext.stMadSettings.deinterlace) {
        /* VDC defaults off, so don't set unless enabled */
        window->adjContext.bMadSet = true;
    }
    NEXUS_VideoWindow_GetDefaultDnrSettings(&window->adjContext.stDnrSettings);
    window->adjContext.bDnrSet = true;
    NEXUS_VideoWindow_GetDefaultAnrSettings(&window->adjContext.stAnrSettings);
    if (index == 0 && window->display->index == 0) {
        window->adjContext.bAnrSet = true;
    }
    NEXUS_VideoWindow_GetDefaultScalerSettings(&window->adjContext.stSclSettings);
    /* window->adjContext.bSclSet defaults false, but VDC defaults these features on. */
#ifdef NEXUS_DYNAMIC_BACKLIGHT_SCALE_FACTOR
    window->picContext.stCustomContrast.dynamicBacklightScaleFactor = NEXUS_DYNAMIC_BACKLIGHT_SCALE_FACTOR; /* full scaling */
#endif
    window->cfg.autoMaster = (index == 0);
    window->cfg.alpha = 0xFF;
    window->cfg.contentMode =  NEXUS_VideoWindowContentMode_eFull;
    window->cfg.sourceBlendFactor = NEXUS_CompositorBlendFactor_eSourceAlpha;
    window->cfg.destBlendFactor = NEXUS_CompositorBlendFactor_eInverseSourceAlpha;
    window->cfg.constantAlpha = 0xFF;
    window->cfg.visible = true;
    window->cfg.zorder = index;
    window->cfg.scaleFactorRounding.enabled = true;
    window->cfg.scaleFactorRounding.horizontalTolerance = 3; /* only scale if >3%. for instance, 704 will not be scaled to 720. */
    window->cfg.scaleFactorRounding.verticalTolerance = 3; /* only scale if >3%. for instance, 480 will not be scaled to 482. */
    window->cfg.colorKey.luma.enabled = false;
    window->cfg.colorKey.luma.lower = 0;
    window->cfg.colorKey.luma.upper = 0xff;
    window->cfg.colorKey.luma.mask = 0;
    window->cfg.colorKey.cr.enabled = false;
    window->cfg.colorKey.cr.lower = 0;
    window->cfg.colorKey.cr.upper = 0xff;
    window->cfg.colorKey.cr.mask = 0;
    window->cfg.colorKey.cb.enabled = false;
    window->cfg.colorKey.cb.lower = 0;
    window->cfg.colorKey.cb.upper = 0xff;
    window->cfg.colorKey.cb.mask = 0;
    NEXUS_CallbackDesc_Init(&window->cfg.letterBoxDetectionChange);
    NEXUS_CallbackDesc_Init(&window->cfg.outputRectChanged);
    NEXUS_VideoWindow_GetDefaultMinDisplayFormat_isrsafe(window, &window->cfg.minimumSourceFormat);
    NEXUS_VideoWindow_GetDefaultMinDisplayFormat_isrsafe(window, &window->cfg.minimumDisplayFormat);
    BKNI_Memcpy(&window->picContext.stCustomContrast.dcTable1[0], &alDCTable1[0], sizeof(window->picContext.stCustomContrast.dcTable1));
    BKNI_Memcpy(&window->picContext.stCustomContrast.dcTable2[0], &alDCTable1[0], sizeof(window->picContext.stCustomContrast.dcTable2));
    BKNI_Memcpy(&window->picContext.stCustomContrast.ireTable[0], &aIreTable[0], sizeof(window->picContext.stCustomContrast.ireTable));

    /* We default forceCapture on for set-tops VEC displays because it's usually needed for HD/SD VEC displays.
    We don't default forceCapture on for encoder displays because it can't be used in some modes.
    Ignore for chips with boxmode. */
    if(!g_pCoreHandles->boxConfig->stBox.ulBoxId){
        window->cfg.forceCapture = (NEXUS_DisplayTimingGenerator_eEncoder != display->timingGenerator);
    }

#if NEXUS_NUM_MOSAIC_DECODES
    window->mosaic.mosaicSettings.backendMosaic.clipRect.width = 1920;
    window->mosaic.mosaicSettings.backendMosaic.clipRect.width = 1080;
    window->mosaic.mosaicSettings.backendMosaic.clipBase = window->mosaic.mosaicSettings.backendMosaic.clipRect;
#endif

    window->cfg.position = display->displayRect;
    window->cfg.clipBase = display->displayRect;
    if (display->cfg.displayType == NEXUS_DisplayType_eBypass) {
        window->cfg.position.height = 482;
    }
    /* coverity[dead_error_condition] */
    if(index>0) {
        window->cfg.position.x = display->displayRect.width/2;
        window->cfg.position.y = 0;
        window->cfg.position.width /=2;
        window->cfg.position.height /=2;
    }
}

void NEXUS_VideoWindow_GetAfdSettings( NEXUS_VideoWindowHandle window, NEXUS_VideoWindowAfdSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->afdSettings;
}

NEXUS_Error NEXUS_VideoWindow_SetAfdSettings( NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowAfdSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);

    window->afdSettings = *pSettings;
    window->afdSet = true;

    if (window->vdcState.window) {
        BERR_Code rc;

        BDBG_CASSERT(NEXUS_AfdMode_eUser == (NEXUS_AfdMode)BVDC_AfdMode_eUser);
        BDBG_CASSERT(NEXUS_AfdClip_eOptionalLevel2 == (NEXUS_AfdClip)BVDC_AfdClip_eOptionalLevel2);
        BDBG_CASSERT(sizeof(NEXUS_VideoWindowAfdSettings) == sizeof(BVDC_AfdSettings));
        rc = BVDC_Window_SetAfdSettings(window->vdcState.window, (const BVDC_AfdSettings *)(const NEXUS_VideoWindowAfdSettings *)&window->afdSettings);
        if (rc) return BERR_TRACE(rc);

        rc = NEXUS_Display_P_ApplyChanges();
        if (rc) return BERR_TRACE(rc);
    }

    return 0;
}

/**
set this window's "SetMasterFrameRate" setting.
if true, we may need to set another window false. if false, we may need to set another window true.
this function must be called from all places in nexus where these conditions may change.
**/
NEXUS_Error NEXUS_VideoWindow_P_ConfigMasterFrameRate(NEXUS_VideoWindowHandle window, const NEXUS_DisplaySettings *pDisplaySettings, const NEXUS_VideoWindowSettings *pWindowsettings)
{
    BERR_Code rc;
    bool masterFrameRate;

    /* pDisplaySettings->frameRateMaster takes precedence */
    masterFrameRate = (pDisplaySettings->frameRateMaster ? pDisplaySettings->frameRateMaster == window->input : pWindowsettings->autoMaster);

    /* if vdc window destroyed, we can throw SetMasterFrameRate to the other window as well */
    masterFrameRate = masterFrameRate && window->vdcState.window;

#if 0
    /* TODO: masterFrameRate could be forced false if this window is not sync-locked. but that info is currently not available immediately after window creation. */
    if (masterFrameRate) {
        BVDC_Window_Status vdcStatus;
        rc = BVDC_Window_GetStatus(window->vdcState.window, &vdcStatus);
        if (rc) return BERR_TRACE(rc);
        masterFrameRate = masterFrameRate && vdcStatus.bSyncLock;
        BDBG_MSG_TRACE(("synclock %d", vdcStatus.bSyncLock));
    }
#endif

    BDBG_MSG_TRACE(("setting SetMasterFrameRate(%d, %d -> %d) based on %d %d %d", window->index, window->vdcState.masterFrameRate, masterFrameRate,
        pDisplaySettings->frameRateMaster, pWindowsettings->autoMaster, window->vdcState.window));

    if (masterFrameRate != window->vdcState.masterFrameRate) {
#if NEXUS_NUM_VIDEO_WINDOWS > 1
        NEXUS_VideoWindowHandle otherWindow = NULL;

        /* only one window per display can have SetMasterFrameRate(true). main overrides pip. */
        otherWindow = &window->display->windows[1 - window->index];
        if (!otherWindow->open) otherWindow = NULL;
#endif

        if (masterFrameRate) {
            /* find any conflict and turn off SetMasterFrameRate on another window */
#if NEXUS_NUM_VIDEO_WINDOWS > 1
            if (otherWindow && otherWindow->vdcState.window && otherWindow->vdcState.masterFrameRate) {
                if (otherWindow->index == 1) {
                    rc = BVDC_Window_SetMasterFrameRate(otherWindow->vdcState.window, false);
                    if (rc) return BERR_TRACE(rc);
                    otherWindow->vdcState.masterFrameRate = false;
                }
                else {
                    /* don't allow pip to override main */
                    masterFrameRate = false;
                }
            }
#endif

            rc = BVDC_Window_SetMasterFrameRate(window->vdcState.window, masterFrameRate);
            if (rc) return BERR_TRACE(rc);
        }
        else {
            if (window->vdcState.window) {
                rc = BVDC_Window_SetMasterFrameRate(window->vdcState.window, false);
                if (rc) return BERR_TRACE(rc);
            }

            /* throw SetMasterFrameRate to other window if it wants it. it's ok if it's sync-slip. */
#if NEXUS_NUM_VIDEO_WINDOWS > 1
            if (otherWindow && otherWindow->vdcState.window && otherWindow->cfg.autoMaster && !otherWindow->vdcState.masterFrameRate) {
                rc = BVDC_Window_SetMasterFrameRate(otherWindow->vdcState.window, true);
                if (rc) return BERR_TRACE(rc);
                otherWindow->vdcState.masterFrameRate = true;
            }
#endif
        }
        window->vdcState.masterFrameRate = masterFrameRate;
    }

    window->syncStatus.masterFrameRateEnabled = masterFrameRate && (pDisplaySettings->dropFrame == NEXUS_TristateEnable_eNotSet);

    return 0;
}

NEXUS_Error NEXUS_VideoWindow_SetSettings_priv( NEXUS_VideoWindowHandle handle, const NEXUS_VideoWindowSettings *pSettings )
{
    NEXUS_ASSERT_MODULE();
    /* call _impl version */
    return NEXUS_VideoWindow_SetSettings(handle, pSettings);
}

#if NEXUS_HAS_VIDEO_ENCODER
void NEXUS_DisplayModule_ClearDisplay_priv(NEXUS_DisplayHandle display)
{
    unsigned i;
    NEXUS_ASSERT_MODULE();
    BDBG_WRN(("ClearDisplay:%p", (void *)display));
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    for(i=0;i<sizeof(display->windows)/sizeof(display->windows[0]);i++) {
        BERR_Code rc;
        NEXUS_VideoWindowHandle window = &display->windows[i];
        if (!window->open) {
            continue;
        }
        rc = NEXUS_VideoWindow_P_RecreateWindow(window);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); /* keep on going */ }
    }
    return;
}
#endif /* NEXUS_HAS_VIDEO_ENCODER  */

NEXUS_Error NEXUS_VideoWindow_BypassVideoProcessing_priv( NEXUS_DisplayHandle display, NEXUS_VideoWindowHandle window, bool bypassVideoProcessing )
{
    if (!window) {
        window = &display->windows[0];
    }
    if (!window->open) return 0;
    if (window->bypassVideoProcessing != bypassVideoProcessing) {
        window->bypassVideoProcessing = bypassVideoProcessing;
        if(window->vdcState.window) {
            BERR_Code rc;
            rc = NEXUS_VideoWindow_P_RecreateWindow(window);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); /* keep on going */ }
            rc = NEXUS_Display_P_ApplyChanges();
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); /* keep on going */ }
            return rc;
        }
    }
    return 0;
}

#if NEXUS_NUM_DSP_VIDEO_ENCODERS
NEXUS_Error NEXUS_Display_P_GetWindowMemc_isrsafe(unsigned displayIndex, unsigned windowIndex, unsigned *pMemcIndex)
{
    /* if box modes not everywhere, must use our own settings */
    if (displayIndex < NEXUS_MAX_DISPLAYS && windowIndex < NEXUS_NUM_VIDEO_WINDOWS) {
        if (!NEXUS_Core_HeapMemcIndex_isrsafe(pVideo->moduleSettings.videoWindowHeapIndex[displayIndex][windowIndex], pMemcIndex)) {
            return NEXUS_SUCCESS;
        }
    }
    return NEXUS_NOT_AVAILABLE; /* no BERR_TRACE */
}
#endif
