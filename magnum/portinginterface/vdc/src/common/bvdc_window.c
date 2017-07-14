/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "bstd.h"                 /* standard types */
#include "bkni.h"                 /* memcpy calls */
#include "bvdc.h"                 /* Video display */
#include "bvdc_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_mcvp_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_gfxfeeder_priv.h"  /* for blend factor validation */
#include "bvdc_csc_priv.h"
#include "bvdc_tnt_priv.h"
#include "bvdc_feeder_priv.h"

BDBG_MODULE(BVDC_WIN);
BDBG_FILE_MODULE(BVDC_WIN_BUF);

/* max clear rect: index by BVDC_P_WindowId */
static const struct
{
    uint32_t ulBoxWinId;               /* BBOX's window ID, See bbox_vdc on how it maps. */
    uint32_t ulMaxMosaicRect;          /* max clear rect of this window */
} s_aWinInfo[] = {
    {0, BVDC_P_CMP_0_V0_CLEAR_RECTS},  /* BVDC_P_WindowId_eComp0_V0 = 0 */
    {1, BVDC_P_CMP_0_V1_CLEAR_RECTS},  /* BVDC_P_WindowId_eComp0_V1 */
    {0, BVDC_P_CMP_1_V0_CLEAR_RECTS},  /* BVDC_P_WindowId_eComp1_V0 */
    {1, BVDC_P_CMP_1_V1_CLEAR_RECTS},  /* BVDC_P_WindowId_eComp1_V1 */
    {0, BVDC_P_CMP_2_V0_CLEAR_RECTS},  /* BVDC_P_WindowId_eComp2_V0 */
    {0, BVDC_P_CMP_3_V0_CLEAR_RECTS},  /* BVDC_P_WindowId_eComp3_V0 */
    {0, BVDC_P_CMP_4_V0_CLEAR_RECTS},  /* BVDC_P_WindowId_eComp4_V0 */
    {0, BVDC_P_CMP_5_V0_CLEAR_RECTS},  /* BVDC_P_WindowId_eComp5_V0 */
    {0, BVDC_P_CMP_6_V0_CLEAR_RECTS},  /* BVDC_P_WindowId_eComp6_V0 */
    {2, 0},                            /* BVDC_P_WindowId_eComp0_G0 */
    {3, 0},                            /* BVDC_P_WindowId_eComp0_G1 */
    {4, 0},                            /* BVDC_P_WindowId_eComp0_G2 */
    {2, 0},                            /* BVDC_P_WindowId_eComp1_G0 */
    {2, 0},                            /* BVDC_P_WindowId_eComp2_G0 */
    {2, 0},                            /* BVDC_P_WindowId_eComp3_G0 */
    {2, 0},                            /* BVDC_P_WindowId_eComp4_G0 */
    {2, 0},                            /* BVDC_P_WindowId_eComp5_G0 */
    {2, 0}                             /* BVDC_P_WindowId_eComp6_G0 */
};


/***************************************************************************
 *
 */
uint32_t BVDC_P_GetBoxWindowId_isrsafe
    ( BVDC_P_WindowId                  eWindowId )
{
    BDBG_CASSERT(BVDC_P_WindowId_eUnknown == sizeof(s_aWinInfo)/sizeof(s_aWinInfo[0]));
    return s_aWinInfo[eWindowId].ulBoxWinId;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetDefaultSettings
    ( BVDC_WindowId                    eWindowId,
      BVDC_Window_Settings            *pDefSettings )
{
    BSTD_UNUSED(eWindowId);

    /* Clear out first */
    BKNI_Memset(pDefSettings, 0, sizeof(BVDC_Window_Settings));

    /* Initialized */
    pDefSettings->hHeap = NULL;
    pDefSettings->hDeinterlacerHeap = NULL;
    pDefSettings->ulMaxMosaicRect = s_aWinInfo[eWindowId].ulMaxMosaicRect;

    /* if this is true, then sync-slipped window may have double-capture-buffer
       instead of multi-buffer operation; this works with VEC locking; */
    pDefSettings->bForceSyncLock = false;

    /* KLUDGE: PR53654, no flash for source format smaller than 720p.  This
     * currently temporary assumption, and will be going thru a public api
     * that allow application to specify the minimun buffer allow that will
     * not flash.
     * Maintain flag for backward compatibility, user specify no flag but
     * did not specify format we default to 720p. */
    #ifndef BVDC_NO_FLASH
    #define BVDC_NO_FLASH (0)
    #endif
    #if (BVDC_NO_FLASH)
    if(!pDefSettings->pMinSrcFmt)
    {
        pDefSettings->pMinSrcFmt = BFMT_GetVideoFormatInfoPtr(BFMT_VideoFmt_e720p);
    }
    #endif

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_Window_ValidateMemconfigSettings
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_Source_Handle               hSource,
      BVDC_P_WindowId                  eWinId )
{
    bool   bSyncSlipInMemconfig, bSrcToBeLocked;
    uint32_t  ulWinIdex;

    if(!BVDC_P_WIN_IS_VIDEO_WINDOW(eWinId))
        return BERR_SUCCESS;

    ulWinIdex = BVDC_P_WIN_IS_V0(eWinId) ? 0 : 1;
    bSyncSlipInMemconfig = hCompositor->hVdc->abSyncSlipInMemconfig[hCompositor->eId][ulWinIdex];

    bSrcToBeLocked = BVDC_P_SRC_IS_MPEG(hSource->eId) &&
       (hSource->ulConnectedWindow == 0) &&
       (!hSource->hSyncLockCompositor) &&
       (!hCompositor->hSyncLockSrc) &&
       (!hCompositor->hSrcToBeLocked);

    BKNI_EnterCriticalSection();
    if(!bSrcToBeLocked && !bSyncSlipInMemconfig)
    {
        BDBG_WRN(("============================================================"));
        BDBG_WRN(("Disp[%d] Win[%d] SyncSlip Mismatch RunTime(%d), Memconfig(%d)",
            hCompositor->eId, eWinId, !bSrcToBeLocked, bSyncSlipInMemconfig));
        BDBG_WRN(("System may run out of memory"));
        BDBG_WRN(("============================================================"));
    }
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_Create
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_Window_Handle              *phWindow,
      BVDC_WindowId                    eId,
      BVDC_Source_Handle               hSource,
      const BVDC_Window_Settings      *pDefSettings )
{
    BVDC_Window_Handle hWindow;
#if (BVDC_P_CHECK_MEMC_INDEX)
    uint32_t    ulMemcIndex = BBOX_VDC_DISREGARD;
    BBOX_Vdc_MemcIndexSettings  *pVdcMemcSettings = NULL;
#endif
    BERR_Code eRet = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_Create);

    BDBG_ASSERT(hCompositor);
    BDBG_ASSERT(phWindow);
    BDBG_ASSERT(hSource);

    /* Check internally if we created. */
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

   /* Make sure the source/compositor is active or created. */
    if(BVDC_P_STATE_IS_INACTIVE(hSource) ||
       BVDC_P_STATE_IS_DESTROY(hSource) ||
       BVDC_P_STATE_IS_INACTIVE(hCompositor) ||
       BVDC_P_STATE_IS_DESTROY(hCompositor))
    {
        BDBG_ERR(("Compositor and/or Source are not active."));
        return BERR_TRACE(BVDC_ERR_WINDOW_NOT_AVAILABLE);
    }

    eRet = BVDC_P_Window_Create(hCompositor, phWindow,hSource->eId, eId);
    if (BERR_SUCCESS != eRet)
    {
        return BERR_TRACE(eRet);
    }

    /* select the corresponding private handle created at BVDC_Open */
    eRet = BVDC_P_Window_GetPrivHandle(hCompositor, eId, hSource->eId, &hWindow, NULL);
    if (BERR_SUCCESS != eRet)
    {
        return BERR_TRACE(eRet);
    }

    /* Error if memory going to be short synclock allocation for MPEG. */
    if(BVDC_P_SRC_IS_MPEG(hSource->eId))
    {
        eRet = BVDC_P_Window_ValidateMemconfigSettings(hCompositor, hSource, hWindow->eId);
        if (BERR_SUCCESS != eRet)
        {
            return BERR_TRACE(eRet);
        }
    }

    /* Check if the window is already created or not. */
    if(!BVDC_P_STATE_IS_INACTIVE(hWindow))
    {
        BDBG_ERR(("Window[%d] state = %d", hWindow->eId, hWindow->eState));
        return BERR_TRACE(BVDC_ERR_WINDOW_ALREADY_CREATED);
    }

    BDBG_MSG(("Creating window%d", hWindow->eId));
    BDBG_ASSERT(BVDC_P_STATE_IS_INACTIVE(hWindow));

    if (BVDC_P_SRC_IS_GFX(hSource->eId))
    {
        BBOX_Vdc_Capabilities *pBoxVdc = &hWindow->hCompositor->hVdc->stBoxConfig.stVdc;

        /* this makes gfx surface be validated in next ApplyChanges */
        hSource->hGfxFeeder->hWindow = hWindow;
        hSource->hGfxFeeder->stCfc.pColorSpaceOut = &(hWindow->hCompositor->stOutColorSpace);

        /* check GFD usage for xcode path*/
        if (BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg) &&
            pBoxVdc->stXcode.ulNumXcodeGfd != BBOX_VDC_DISREGARD)
        {
            hWindow->hCompositor->hVdc->ulXcodeGfd++;
            BDBG_MSG(("Incremented xcode GFD usage count [%d].", hWindow->hCompositor->hVdc->ulXcodeGfd));

            if (hWindow->hCompositor->hVdc->ulXcodeGfd > pBoxVdc->stXcode.ulNumXcodeGfd)
            {
                hWindow->hCompositor->hVdc->ulXcodeGfd--;
                BDBG_ERR(("Exceeded allowed number (%d) of GFD used with transcode.", pBoxVdc->stXcode.ulNumXcodeGfd));
                return BERR_TRACE(BVDC_ERR_RESOURCE_NOT_AVAILABLE);
            }
        }
    }
    else if (BVDC_P_SRC_IS_VFD(hSource->eId))
    {
        BAVC_SourceId  eSrcId;

        /* this VFD should not already been used by some window */
        if (hSource->hVfdFeeder->hWindow)
        {
            BDBG_ERR(("feeder %d is already used by Window[%d]",
                hSource->eId - BAVC_SourceId_eVfd0, hWindow->eId));
            return BERR_TRACE(BVDC_ERR_RESOURCE_NOT_AVAILABLE);
        }

        /* is user provided vfd the right one for this window? */
        eSrcId = BAVC_SourceId_eVfd0 + (hWindow->stResourceRequire.ePlayback - BVDC_P_FeederId_eVfd0);
        if (hSource->eId != eSrcId)
        {
            BDBG_ERR(("feeder %d is not the proper VFD to feed Window[%d]",
                hSource->eId, hWindow->eId));
            return BERR_TRACE(BVDC_ERR_SOURCE_WINDOW_MISMATCH);
        }

        /* this makes gfx surface be validated in next ApplyChanges and
         * marks the vfd as being used */
        hSource->hVfdFeeder->hWindow = hWindow;
    }
    else /* video src, but not vfd */
    {
        BVDC_P_Feeder_Handle  hVfdFeeder;
        BAVC_SourceId  eSrcId;

        /* the assigned playback should not be already in use for gfx surface */
        eSrcId = BAVC_SourceId_eVfd0 + (hWindow->stResourceRequire.ePlayback - BVDC_P_FeederId_eVfd0);
        if (hWindow->hCompositor->hVdc->ahSource[eSrcId])
        {
            hVfdFeeder = hWindow->hCompositor->hVdc->ahSource[eSrcId]->hVfdFeeder;
            if (hVfdFeeder->hWindow)
            {
                BDBG_ERR(("Window[%d]'s playback feeder %d is in use for gfx",
                    hWindow->eId, hWindow->stResourceRequire.ePlayback - BVDC_P_FeederId_eVfd0));
                return BERR_TRACE(BVDC_ERR_RESOURCE_NOT_AVAILABLE);
            }

            /* mark the VFD as being in use */
            hVfdFeeder->hWindow = hWindow;
        }

        if (BVDC_P_SRC_IS_MPEG(hSource->eId) && hSource->hMpegFeeder->bGfxSrc)
        {
            hSource->hMpegFeeder->hWindow = hWindow;
        }
    }

    /* Reinitialize context.  But not make it active until apply. */
    *phWindow = hWindow;

    /* Which heap to use? */
    if(pDefSettings)
    {
        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] create with CAP heap 0x%lx, MAD heap 0x%lx",
            hWindow->eId, (unsigned long)pDefSettings->hHeap,
            (unsigned long)pDefSettings->hDeinterlacerHeap));
        hWindow->hCapHeap = (pDefSettings->hHeap)
            ? pDefSettings->hHeap  : hCompositor->hVdc->hBufferHeap;
        hWindow->hDeinterlacerHeap = (pDefSettings->hDeinterlacerHeap)
            ? pDefSettings->hDeinterlacerHeap : hWindow->hCapHeap;
    }
    else
    {
        hWindow->hCapHeap = hCompositor->hVdc->hBufferHeap;
        hWindow->hDeinterlacerHeap = hWindow->hCapHeap;
    }

    /* Check if video window uses the right heap based on boxmode */
    if(BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId))
    {
        uint32_t  ulWinIdex;
        BBOX_Vdc_Capabilities         *pVdcCap;

        ulWinIdex = BVDC_P_WIN_IS_V0(hWindow->eId) ? 0 : 1;
        pVdcCap = &hCompositor->hVdc->stBoxConfig.stVdc;

        /* Check if window is available */
        if(!pVdcCap->astDisplay[hCompositor->hDisplay->eId].astWindow[ulWinIdex].bAvailable)
        {
            BDBG_ERR(("Window[%d] on display[%d] is not supported in boxmode %d",
                ulWinIdex, hCompositor->hDisplay->eId,
                hCompositor->hVdc->stBoxConfig.stBox.ulBoxId));
            return BERR_TRACE(BVDC_ERR_WINDOW_NOT_AVAILABLE);
        }

#if (BVDC_P_CHECK_MEMC_INDEX)
        pVdcMemcSettings = &hCompositor->hVdc->stBoxConfig.stMemConfig.stVdcMemcIndex;

        /* Check capture heap */
        ulMemcIndex = pVdcMemcSettings->astDisplay[hCompositor->hDisplay->eId].aulVidWinCapMemcIndex[ulWinIdex];
        if(ulMemcIndex != BBOX_MemcIndex_Invalid)
        {
            /* Checking assumes memory is continous, possible problem when there is
             * hole in memory address. Disable the checking till we have correction
             * information from BCHP. */
            eRet = BVDC_P_BufferHeap_CheckHeapMemcIndex(hWindow->hCapHeap,
                ulMemcIndex, hCompositor->hDisplay->eId, ulWinIdex);
            if (BERR_SUCCESS != eRet)
            {
                return BERR_TRACE(eRet);
            }
        }

        /* Check deinterlacer heap */
        ulMemcIndex = pVdcMemcSettings->astDisplay[hCompositor->hDisplay->eId].aulVidWinMadMemcIndex[ulWinIdex];
        if(ulMemcIndex != BBOX_MemcIndex_Invalid)
        {
            eRet = BVDC_P_BufferHeap_CheckHeapMemcIndex(hWindow->hDeinterlacerHeap,
                ulMemcIndex, hCompositor->hDisplay->eId, ulWinIdex);
            if (BERR_SUCCESS != eRet)
            {
                return BERR_TRACE(eRet);
            }
        }
#endif

    }


    /* Check if gfx window uses the right heap based on boxmode */
    if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
    {
        uint32_t  ulWinIdex, ulGfxWinMemcIndex;
        BBOX_Vdc_Capabilities         *pVdcCap;

        ulWinIdex = BVDC_P_WIN_GET_GFX_WINDOW_INDEX(hWindow->eId);
        pVdcCap = &hCompositor->hVdc->stBoxConfig.stVdc;

        /* Check if window is available */
        if(!pVdcCap->astDisplay[hCompositor->hDisplay->eId].astWindow[ulWinIdex].bAvailable)
        {
            BDBG_ERR(("Window[%d] on display[%d] is not supported in boxmode %d",
                ulWinIdex, hCompositor->hDisplay->eId,
                hCompositor->hVdc->stBoxConfig.stBox.ulBoxId));
            return BERR_TRACE(BVDC_ERR_WINDOW_NOT_AVAILABLE);
        }

#if (BVDC_P_CHECK_MEMC_INDEX)
        pVdcMemcSettings = &hCompositor->hVdc->stBoxConfig.stMemConfig.stVdcMemcIndex;
        ulGfxWinMemcIndex = ulWinIdex - BVDC_P_WindowId_eComp0_G0;

        /* Check capture heap */
        ulMemcIndex = pVdcMemcSettings->astDisplay[hCompositor->hDisplay->eId].aulGfdWinMemcIndex[ulGfxWinMemcIndex];
        if(ulMemcIndex != BBOX_MemcIndex_Invalid)
        {
            /* Checking assumes memory is continous, possible problem when there is
             * hole in memory address. Disable the checking till we have correction
             * information from BCHP. */
            eRet = BVDC_P_BufferHeap_CheckHeapMemcIndex(hWindow->hCapHeap,
                ulMemcIndex, hCompositor->hDisplay->eId, ulWinIdex);
            if (BERR_SUCCESS != eRet)
            {
                return BERR_TRACE(eRet);
            }
        }
#else
        BSTD_UNUSED(ulGfxWinMemcIndex);
#endif
    }

    /* Default settings */
    if(pDefSettings)
    {
        bool bBypassDirty;

        hWindow->stSettings = *pDefSettings;
        /* Do not allow override of number of rect this is read-only
         * information. */
        if(hWindow->eId < BVDC_P_WindowId_eUnknown)
        {
            hWindow->stSettings.ulMaxMosaicRect = s_aWinInfo[hWindow->eId].ulMaxMosaicRect;
        }
        else
        {
            BDBG_ASSERT(hWindow->eId < BVDC_P_WindowId_eUnknown);
        }
        hWindow->hCompositor->hDisplay->stNewInfo.bBypassVideoProcess =
            pDefSettings->bBypassVideoProcessings;

        bBypassDirty =
            hWindow->hCompositor->hDisplay->stNewInfo.bBypassVideoProcess ^
            hWindow->hCompositor->hDisplay->stCurInfo.bBypassVideoProcess;
        hWindow->hCompositor->hDisplay->stNewInfo.stDirty.stBits.bHdmiCsc |= bBypassDirty;
#if BVDC_P_SUPPORT_STG
        if(BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg))
            hWindow->hCompositor->hDisplay->stNewInfo.stDirty.stBits.bStgEnable |= bBypassDirty;
#endif
    }
    else
    {
        BVDC_Window_GetDefaultSettings(hWindow->eId, &hWindow->stSettings);
    }

#if BVDC_P_SUPPORT_MOSAIC_MODE
    if((pDefSettings) && (pDefSettings->hHeap ))
    {
        /* Allocate drain buffer from window heap */
        if(hWindow->stSettings.ulMaxMosaicRect)
        {
            hWindow->ullNullBufOffset = pDefSettings->hHeap->ullMosaicDrainOffset;
        }
        else
        {
            hWindow->hMosaicMmaBlock = NULL;
            hWindow->ullNullBufOffset = 0;
        }
    }
    else
    {
        /* Use the drain buffer from VDC main heap. Set pvNullBufAddr to NULL
         * so it will not be free in BVDC_Window_Destroy */
        hWindow->hMosaicMmaBlock = NULL;
        hWindow->ullNullBufOffset = hCompositor->hVdc->ullVdcNullBufOffset;
    }
#endif

    /* Inititial window fields */
    BVDC_P_Window_Init(*phWindow, hSource);

    /* Keep count of new number of windows that connected to source! */
    hSource->stNewInfo.ulWindows++;
    hSource->stNewInfo.stDirty.stBits.bWindowNum = BVDC_P_DIRTY;

    /* Mark as create, awating for apply. */
    hWindow->eState = BVDC_P_State_eCreate;

    BDBG_MSG(("Window[%d] is BVDC_P_State_eCreate", hWindow->eId));

    BDBG_LEAVE(BVDC_Window_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_Destroy
    ( BVDC_Window_Handle               hWindow )
{
    BERR_Code eRet = BERR_SUCCESS;
    BVDC_Source_Handle  hSource;

    BDBG_ENTER(BVDC_Window_Destroy);

#if defined(BVDC_GFX_PERSIST)
    hWindow->stNewInfo.pfGenCallback = NULL;
    hWindow->stCurInfo.pfGenCallback = NULL;
    hWindow->stNewInfo.BoxDetectCallBack = NULL;
    hWindow->stCurInfo.BoxDetectCallBack = NULL;
    hWindow->stNewInfo.stContrastStretch.pfCallback = NULL;
    hWindow->stCurInfo.stContrastStretch.pfCallback = NULL;
    goto done;
#endif

    /* Return if trying to free a NULL handle. */
    if(!hWindow)
    {
        goto done;
    }

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    if(BVDC_P_STATE_IS_DESTROY(hWindow) ||
       BVDC_P_STATE_IS_INACTIVE(hWindow))
    {
        goto done;
    }

    BVDC_P_Window_DisableBoxDetect(hWindow);

    hSource = hWindow->stCurInfo.hSource;

    BKNI_EnterCriticalSection();
    if (BVDC_P_STATE_IS_CREATE(hWindow))
    {
        hWindow->eState = BVDC_P_State_eInactive;
        BDBG_MSG(("Window[%d] is BVDC_P_State_eInactive", hWindow->eId));

        hSource = hWindow->stNewInfo.hSource;

        /* Keep count of new number of windows that connected to source! */
        hSource->stNewInfo.ulWindows--;
        hSource->stNewInfo.stDirty.stBits.bWindowNum = BVDC_P_DIRTY;
    }
    else if (BVDC_P_STATE_IS_ACTIVE(hWindow))
    {
        /* check if user-captured buffers, if any, were returned */
        if (BVDC_P_Window_CheckForUnReturnedUserCapturedBuffer_isr(hWindow))
        {
            eRet = BVDC_ERR_USER_STILL_HAS_CAPTURE_BUFFER;
        }

        /* Keep count of new number of windows that connected to source! */
        hSource->stNewInfo.ulWindows--;
        hSource->stNewInfo.stDirty.stBits.bWindowNum = BVDC_P_DIRTY;
        hWindow->eState = BVDC_P_State_eDestroy;
        BDBG_MSG(("Window[%d] is BVDC_P_State_eDestroy", hWindow->eId));
    }

    if (BVDC_P_SRC_IS_GFX(hSource->eId))
    {
        BBOX_Vdc_Capabilities *pBoxVdc = &hWindow->hCompositor->hVdc->stBoxConfig.stVdc;

        /* stop validating this gfd */
        hSource->hGfxFeeder->hWindow = NULL;

        if (BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg) &&
            pBoxVdc->stXcode.ulNumXcodeGfd != BBOX_VDC_DISREGARD)
        {
            hWindow->hCompositor->hVdc->ulXcodeGfd--;
            BDBG_MSG(("Decremented xcode GFD usage count [%d].", hWindow->hCompositor->hVdc->ulXcodeGfd));
        }

    }
    else if (BVDC_P_SRC_IS_VFD(hSource->eId))
    {
        /* mark the vfd as NOT being in use and stop validating this vfd */
        hSource->hVfdFeeder->hWindow = NULL;
    }
    else if (BVDC_P_SRC_IS_VIDEO(hSource->eId))
    {
        BAVC_SourceId  eSrcId;

        /* mark the vfd as NOT being in use */
        eSrcId = BAVC_SourceId_eVfd0 + (hWindow->stResourceRequire.ePlayback - BVDC_P_FeederId_eVfd0);
        if (hWindow->hCompositor->hVdc->ahSource[eSrcId])
        {
            hWindow->hCompositor->hVdc->ahSource[eSrcId]->hVfdFeeder->hWindow = NULL;
        }

        if (BVDC_P_SRC_IS_MPEG(hSource->eId))
        {
            /* mark the mfd as NOT being in use and stop validating this mfd */
            hSource->hMpegFeeder->hWindow = NULL;
        }
    }
    BKNI_LeaveCriticalSection();

done:
    BDBG_LEAVE(BVDC_Window_Destroy);
    return eRet;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetSrcClip
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulLeft,
      uint32_t                         ulRight,
      uint32_t                         ulTop,
      uint32_t                         ulBottom )
{
    BDBG_ENTER(BVDC_Window_SetSrcClip);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* 4:2:2 BVB round down the pixel position and width to even number */
    if((BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId) &&
       ((ulLeft & 1) || (ulRight & 1))))
    {
        BDBG_MSG(("left=%d, right=%u; to round down to even pixels:", ulLeft, ulRight));
        ulLeft  = BVDC_P_ALIGN_DN(ulLeft, 2);
        ulRight = BVDC_P_ALIGN_DN(ulRight, 2);
        BDBG_MSG(("left=%d, right=%u now!", ulLeft, ulRight));
    }

    /* set new value */
    hWindow->stNewInfo.stSrcClip.ulLeft   = ulLeft;
    hWindow->stNewInfo.stSrcClip.ulRight  = ulRight;
    hWindow->stNewInfo.stSrcClip.ulTop    = ulTop;
    hWindow->stNewInfo.stSrcClip.ulBottom = ulBottom;

    BDBG_LEAVE(BVDC_Window_SetSrcClip);
    return BERR_SUCCESS;
}



#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetSrcRightClip
    ( BVDC_Window_Handle               hWindow,
      int32_t                          lLeftDelta )
{
    BERR_Code eRet = BERR_SUCCESS;

#if (!BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP)
    BSTD_UNUSED(lLeftDelta);
#endif

    BDBG_ENTER(BVDC_Window_SetSrcRightClip);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

#if (!BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP)
    BDBG_ERR(("Independent source clipping not support on this chip!"));
    eRet = BVDC_ERR_3D_INDEP_SRC_CLIP_NOT_SUPPORTED;
#else
    /* 4:2:2 BVB round down the pixel position and width to even number */
    if((BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId) &&
       (lLeftDelta & 1)))
    {
        BDBG_MSG(("LeftDelta=%d; to round down to even pixels:", lLeftDelta));
        lLeftDelta  = BVDC_P_ALIGN_DN(lLeftDelta, 2);
        BDBG_MSG(("LeftDelta=%d!", lLeftDelta));
    }

    /* set new value */
    hWindow->stNewInfo.stSrcClip.lLeftDelta_R = lLeftDelta;
#endif

    BDBG_LEAVE(BVDC_Window_SetSrcRightClip);
    return BERR_TRACE(eRet);
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetSrcRightClip
    ( const BVDC_Window_Handle         hWindow,
      int32_t                         *plLeftDelta )
{
    BDBG_ENTER(BVDC_Window_GetSrcRightClip);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(plLeftDelta)
    {
        *plLeftDelta = hWindow->stCurInfo.stSrcClip.lLeftDelta_R;
    }

    BDBG_LEAVE(BVDC_Window_GetSrcRightClip);
    return BERR_SUCCESS;
}
#endif


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetSrcClip
    ( const BVDC_Window_Handle         hWindow,
      uint32_t                        *pulLeft,
      uint32_t                        *pulRight,
      uint32_t                        *pulTop,
      uint32_t                        *pulBottom )
{
    BDBG_ENTER(BVDC_Window_GetSrcClip);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* get value */
    if(pulLeft)
    {
        *pulLeft = hWindow->stCurInfo.stSrcClip.ulLeft;
    }

    if(pulRight)
    {
        *pulRight = hWindow->stCurInfo.stSrcClip.ulRight;
    }

    if(pulTop)
    {
        *pulTop = hWindow->stCurInfo.stSrcClip.ulTop;
    }

    if(pulBottom)
    {
        *pulBottom = hWindow->stCurInfo.stSrcClip.ulBottom;
    }

    BDBG_LEAVE(BVDC_Window_GetSrcClip);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetDstRect
    ( BVDC_Window_Handle               hWindow,
      int32_t                          lLeft,
      int32_t                          lTop,
      uint32_t                         ulWidth,
      uint32_t                         ulHeight )
{
    BDBG_ENTER(BVDC_Window_SetDstRect);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Dst Rect cannot be smaller than 16x10. */
    if((ulWidth  < BVDC_P_WIN_DST_OUTPUT_H_MIN) ||
       (ulHeight < BVDC_P_WIN_DST_OUTPUT_V_MIN))
    {
        ulWidth  = (ulWidth < BVDC_P_WIN_DST_OUTPUT_H_MIN)?
            BVDC_P_WIN_DST_OUTPUT_H_MIN : ulWidth;
        ulHeight  = (ulHeight < BVDC_P_WIN_DST_OUTPUT_V_MIN)?
            BVDC_P_WIN_DST_OUTPUT_V_MIN : ulHeight;

        BDBG_MSG(("win[%d] align up DstRect to %d x %d",
            hWindow->eId, ulWidth, ulHeight));
    }

    /* set new value */
    hWindow->stNewInfo.stDstRect.lLeft    = lLeft;
    hWindow->stNewInfo.stDstRect.lTop     = lTop;
    hWindow->stNewInfo.stDstRect.ulWidth  = ulWidth;
    hWindow->stNewInfo.stDstRect.ulHeight = ulHeight;

    BDBG_LEAVE(BVDC_Window_SetDstRect);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetDstRect
    ( const BVDC_Window_Handle         hWindow,
      int32_t                         *plLeftScreen,
      int32_t                         *plTopScreen,
      uint32_t                        *pulWidth,
      uint32_t                        *pulHeight )
{
    BDBG_ENTER(BVDC_Window_GetDstRect);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* get value */
    if(plLeftScreen)
    {
        *plLeftScreen = hWindow->stCurInfo.stDstRect.lLeft;
    }

    if(plTopScreen)
    {
        *plTopScreen = hWindow->stCurInfo.stDstRect.lTop;
    }

    if(pulWidth)
    {
        *pulWidth = hWindow->stCurInfo.stDstRect.ulWidth;
    }

    if(pulHeight)
    {
        *pulHeight = hWindow->stCurInfo.stDstRect.ulHeight;
    }

    BDBG_LEAVE(BVDC_Window_GetDstRect);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetScalerOutput
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulLeft,
      uint32_t                         ulTop,
      uint32_t                         ulWidth,
      uint32_t                         ulHeight )
{
    BDBG_ENTER(BVDC_Window_SetScalerOutput);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Preliminary checking that does not depend on until applychanges. */
    if((ulLeft > ulWidth) ||
       (ulTop  > ulHeight))
    {
        BDBG_ERR(("Top, Left must be inside rectangle dimension"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* 4:2:2 BVB round up the width to even number */
    if(BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId) && (ulWidth & 1))
    {
        BDBG_MSG(("width=%u; to round up to even pixels:", ulWidth));
        ulWidth = BVDC_P_ALIGN_UP(ulWidth, 2);
        BDBG_MSG((" width=%u now!", ulWidth));
    }

    if((ulWidth < BVDC_P_WIN_DST_OUTPUT_H_MIN) ||(ulHeight < BVDC_P_WIN_DST_OUTPUT_V_MIN))
    {
        ulWidth  = (ulWidth < BVDC_P_WIN_DST_OUTPUT_H_MIN)?
            BVDC_P_WIN_DST_OUTPUT_H_MIN : ulWidth;
        ulHeight  = (ulHeight < BVDC_P_WIN_DST_OUTPUT_V_MIN)?
            BVDC_P_WIN_DST_OUTPUT_V_MIN : ulHeight;

        BDBG_MSG(("win[%d] align up ScalerOutput %d x %d",
            hWindow->eId, ulWidth, ulHeight));
    }

    /* set new value */
    hWindow->stNewInfo.stScalerOutput.lLeft    = ulLeft;
    hWindow->stNewInfo.stScalerOutput.lTop     = ulTop;
    hWindow->stNewInfo.stScalerOutput.ulWidth  = ulWidth;
    hWindow->stNewInfo.stScalerOutput.ulHeight = ulHeight;

    BDBG_LEAVE(BVDC_Window_SetScalerOutput);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetScalerOutput
    ( const BVDC_Window_Handle         hWindow,
      uint32_t                        *pulLeft,
      uint32_t                        *pulTop,
      uint32_t                        *pulWidth,
      uint32_t                        *pulHeight )
{
    BDBG_ENTER(BVDC_Window_GetScalerOutput);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* get value */
    if(pulLeft)
    {
        *pulLeft = hWindow->stCurInfo.stScalerOutput.lLeft;
    }

    if(pulTop)
    {
        *pulTop = hWindow->stCurInfo.stScalerOutput.lTop;
    }

    if(pulWidth)
    {
        *pulWidth = hWindow->stCurInfo.stScalerOutput.ulWidth;
    }

    if(pulHeight)
    {
        *pulHeight = hWindow->stCurInfo.stScalerOutput.ulHeight;
    }


    BDBG_LEAVE(BVDC_Window_GetScalerOutput);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetAlpha
    ( BVDC_Window_Handle               hWindow,
      uint8_t                          ucAlpha )
{
    BDBG_ENTER(BVDC_Window_SetAlpha);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    hWindow->stNewInfo.ucAlpha = ucAlpha;

    BDBG_LEAVE(BVDC_Window_SetAlpha);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetAlpha
    ( const BVDC_Window_Handle         hWindow,
      uint8_t                         *pucAlpha )
{
    BDBG_ENTER(BVDC_Window_GetAlpha);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    if(pucAlpha)
    {
        *pucAlpha = hWindow->stCurInfo.ucAlpha;
    }

    BDBG_LEAVE(BVDC_Window_GetAlpha);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetBlendFactor
    ( BVDC_Window_Handle               hWindow,
      BVDC_BlendFactor                 eSrcBlendFactor,
      BVDC_BlendFactor                 eDstBlendFactor,
      uint8_t                          ucConstantAlpha )
{
    BERR_Code  eResult;

    BDBG_ENTER(BVDC_Window_SetBlendFactor);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* For gfx window need to restrict blending factor. */
    if( BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId) )
    {
        eResult = BVDC_P_GfxFeeder_ValidateBlend( eSrcBlendFactor,
                                                  eDstBlendFactor,
                                                  ucConstantAlpha );
        if ( BERR_SUCCESS != eResult )
        {
            return BERR_TRACE(eResult);
        }
    }

    /* set new value */
    hWindow->stNewInfo.ucConstantAlpha   = ucConstantAlpha;
    hWindow->stNewInfo.eFrontBlendFactor = eSrcBlendFactor;
    hWindow->stNewInfo.eBackBlendFactor  = eDstBlendFactor;

    BDBG_LEAVE(BVDC_Window_SetBlendFactor);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetBlendFactor
    ( const BVDC_Window_Handle         hWindow,
      uint8_t                         *pucConstantAlpha,
      BVDC_BlendFactor                *peSrcBlendFactor,
      BVDC_BlendFactor                *peDstBlendFactor )
{
    BDBG_ENTER(BVDC_Window_GetBlendFactor);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    if(pucConstantAlpha)
    {
        *pucConstantAlpha = hWindow->stCurInfo.ucConstantAlpha;
    }

    if(peSrcBlendFactor)
    {
        *peSrcBlendFactor = hWindow->stCurInfo.eFrontBlendFactor;
    }

    if(peDstBlendFactor)
    {
        *peDstBlendFactor = hWindow->stCurInfo.eBackBlendFactor;
    }

    BDBG_LEAVE(BVDC_Window_GetBlendFactor);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetZOrder
    ( BVDC_Window_Handle               hWindow,
      uint8_t                          ucZOrder )
{
    BDBG_ENTER(BVDC_Window_SetZOrder);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    hWindow->stNewInfo.ucZOrder = ucZOrder;

    BDBG_LEAVE(BVDC_Window_SetZOrder);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetZOrder
    ( const BVDC_Window_Handle         hWindow,
      uint8_t                         *pucZOrder )
{
    BDBG_ENTER(BVDC_Window_GetZOrder);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    if(pucZOrder)
    {
        *pucZOrder = hWindow->stCurInfo.ucZOrder;
    }

    BDBG_LEAVE(BVDC_Window_GetZOrder);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetVisibility
    ( BVDC_Window_Handle               hWindow,
      bool                             bVisible )
{
    BDBG_ENTER(BVDC_Window_SetVisibility);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

#if defined(BVDC_GFX_PERSIST)
    if (!bVisible)
    {
        return BERR_SUCCESS;
    }
#endif

    /* set new value */
    hWindow->stNewInfo.bVisible = bVisible;

    BDBG_LEAVE(BVDC_Window_SetVisibility);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetVisibility
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbVisible )
{
    BDBG_ENTER(BVDC_Window_GetVisibility);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    if(pbVisible)
    {
        *pbVisible = hWindow->stCurInfo.bVisible;
    }

    BDBG_LEAVE(BVDC_Window_GetVisibility);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetDeinterlaceConfiguration
    ( BVDC_Window_Handle               hWindow,
      bool                             bDeinterlace,
      const BVDC_Deinterlace_Settings *pMadSettings )
{
    BVDC_P_Deinterlace_Settings *pNewMad;
    const BVDC_P_Deinterlace_Settings *pCurMad;
    BERR_Code    eResult;

    BDBG_ENTER(BVDC_Window_SetDeinterlaceConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    pNewMad = &hWindow->stNewInfo.stMadSettings;
    pCurMad = &hWindow->stCurInfo.stMadSettings;

    /* set new value */
    hWindow->stNewInfo.bDeinterlace = bDeinterlace;

    if(bDeinterlace && pMadSettings)
    {
        eResult = BVDC_P_Window_SetMcvp_DeinterlaceConfiguration(hWindow, bDeinterlace, pMadSettings);
        if(eResult != BERR_SUCCESS)
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        pNewMad->eGameMode = pMadSettings->eGameMode;
        pNewMad->ePqEnhancement = pMadSettings->ePqEnhancement;
        pNewMad->bShrinkWidth = pMadSettings->bShrinkWidth;
        pNewMad->bReverse32Pulldown = pMadSettings->bReverse32Pulldown;
        pNewMad->bReverse22Pulldown = pMadSettings->bReverse22Pulldown;

        if(pMadSettings->pChromaSettings)
        {
            pNewMad->stChromaSettings = *(pMadSettings->pChromaSettings);
            hWindow->stNewInfo.bChromaCustom = true;
        }
        else
        {
            hWindow->stNewInfo.bChromaCustom = false;
        }

        if(pMadSettings->pMotionSettings)
        {
            pNewMad->stMotionSettings= *(pMadSettings->pMotionSettings);
            hWindow->stNewInfo.bMotionCustom = true;
        }
        else
        {
            hWindow->stNewInfo.bMotionCustom = false;
        }

        if(pMadSettings->pReverse32Settings)
        {
            pNewMad->stReverse32Settings = *(pMadSettings->pReverse32Settings);
            hWindow->stNewInfo.bRev32Custom = true;
        }
        else
        {
            hWindow->stNewInfo.bRev32Custom= false;
        }

        if(pMadSettings->pReverse22Settings)
        {
            pNewMad->stReverse22Settings = *(pMadSettings->pReverse22Settings);
            hWindow->stNewInfo.bRev22Custom = true;
        }
        else
        {
            hWindow->stNewInfo.bRev22Custom = false;
        }

        /* Use the customized Up/Down sampler filter settings */
        if(pMadSettings->pUpSampler)
        {
            pNewMad->stUpSampler = *(pMadSettings->pUpSampler);
        }
        else
        {
            BVDC_P_Mvp_Init_Custom(&pNewMad->stUpSampler, NULL, NULL);
        }
        if(pMadSettings->pDnSampler)
        {
            pNewMad->stDnSampler = *(pMadSettings->pDnSampler);
        }
        else
        {
            BVDC_P_Mvp_Init_Custom(NULL, &pNewMad->stDnSampler, NULL);
        }

        /* low angles setting */
        if(pMadSettings->pLowAngles)
        {
            pNewMad->stLowAngles = *(pMadSettings->pLowAngles);
        }
        else
        {
            BVDC_P_Mvp_Init_Custom(NULL, NULL, &pNewMad->stLowAngles);
        }
    }

    /* On/Off toggle and game mode change will be handled separately */
    if( (bDeinterlace != hWindow->stCurInfo.bDeinterlace) ||
      (bDeinterlace && pMadSettings &&
      (pNewMad->eGameMode != pCurMad->eGameMode ||
       pNewMad->ePixelFmt != pCurMad->ePixelFmt ||
       pNewMad->ePqEnhancement != pCurMad->ePqEnhancement||
       pNewMad->bShrinkWidth != pCurMad->bShrinkWidth ||
       pNewMad->bReverse22Pulldown != pCurMad->bReverse22Pulldown ||
       pNewMad->bReverse32Pulldown != pCurMad->bReverse32Pulldown ||
       pNewMad->stChromaSettings.bChromaField420EdgeDetMode != pCurMad->stChromaSettings.bChromaField420EdgeDetMode ||
       pNewMad->stChromaSettings.bChromaField420InitPhase != pCurMad->stChromaSettings.bChromaField420InitPhase ||
       pNewMad->stChromaSettings.eChroma422InverseTelecineMode!= pCurMad->stChromaSettings.eChroma422InverseTelecineMode ||
       pNewMad->stChromaSettings.eChroma422MotionAdaptiveMode != pCurMad->stChromaSettings.eChroma422MotionAdaptiveMode ||
       pNewMad->stChromaSettings.eChromaField420InvMethod != pCurMad->stChromaSettings.eChromaField420InvMethod ||
       pNewMad->stChromaSettings.eChroma420MotionMode != pCurMad->stChromaSettings.eChroma420MotionMode ||
       pNewMad->stChromaSettings.eChroma422MotionMode != pCurMad->stChromaSettings.eChroma422MotionMode ||
       pNewMad->stChromaSettings.eChroma420MotionAdaptiveMode != pCurMad->stChromaSettings.eChroma420MotionAdaptiveMode ||
       pNewMad->stChromaSettings.bMS_3548 != pCurMad->stChromaSettings.bMS_3548 ||
       pNewMad->stChromaSettings.bMT_3548 != pCurMad->stChromaSettings.bMT_3548 ||
       pNewMad->stChromaSettings.ulMaxXChroma != pCurMad->stChromaSettings.ulMaxXChroma ||
       pNewMad->stMotionSettings.bEnableQmK != pCurMad->stMotionSettings.bEnableQmK ||
       pNewMad->stMotionSettings.bEnableQmL != pCurMad->stMotionSettings.bEnableQmL ||
       pNewMad->stMotionSettings.bEnableQmM != pCurMad->stMotionSettings.bEnableQmM ||
       pNewMad->stMotionSettings.eSmMode != pCurMad->stMotionSettings.eSmMode ||
       pNewMad->stMotionSettings.eTmMode != pCurMad->stMotionSettings.eTmMode ||
       pNewMad->stReverse22Settings.ulBwvLuma22Threshold != pCurMad->stReverse22Settings.ulBwvLuma22Threshold ||
       pNewMad->stReverse22Settings.ulEnterLockLevel != pCurMad->stReverse22Settings.ulEnterLockLevel ||
       pNewMad->stReverse22Settings.ulLockSatLevel != pCurMad->stReverse22Settings.ulLockSatLevel ||
       pNewMad->stReverse22Settings.ulNonMatchMatchRatio != pCurMad->stReverse22Settings.ulNonMatchMatchRatio ||
       pNewMad->stReverse32Settings.ulBadEditLevel != pCurMad->stReverse32Settings.ulBadEditLevel ||
       pNewMad->stReverse32Settings.ulBwvLuma32Threshold != pCurMad->stReverse32Settings.ulBwvLuma32Threshold ||
       pNewMad->stReverse32Settings.ulEnterLockLevel != pCurMad->stReverse32Settings.ulEnterLockLevel ||
       pNewMad->stReverse32Settings.ulLockSatLevel != pCurMad->stReverse32Settings.ulLockSatLevel ||
       pNewMad->stReverse32Settings.ulPhaseMatchThreshold != pCurMad->stReverse32Settings.ulPhaseMatchThreshold ||
       pNewMad->stReverse32Settings.ulRepfPxlCorrectLevel!= pCurMad->stReverse32Settings.ulRepfPxlCorrectLevel ||
       pNewMad->stReverse32Settings.ulRepfVetoLevel != pCurMad->stReverse32Settings.ulRepfVetoLevel ||
       hWindow->stNewInfo.bChromaCustom != hWindow->stCurInfo.bChromaCustom ||
       hWindow->stNewInfo.bMotionCustom != hWindow->stCurInfo.bMotionCustom ||
       hWindow->stNewInfo.bRev32Custom != hWindow->stCurInfo.bRev32Custom ||
       hWindow->stNewInfo.bRev22Custom != hWindow->stCurInfo.bRev22Custom ||
       pNewMad->stUpSampler.eFilterType != pCurMad->stUpSampler.eFilterType ||
       pNewMad->stUpSampler.eRingRemoval != pCurMad->stUpSampler.eRingRemoval ||
       pNewMad->stUpSampler.bUnbiasedRound != pCurMad->stUpSampler.bUnbiasedRound ||
       pNewMad->stDnSampler.eFilterType != pCurMad->stDnSampler.eFilterType ||
       pNewMad->stDnSampler.eRingRemoval != pCurMad->stDnSampler.eRingRemoval ||
       pNewMad->stLowAngles.ulLaControlDirRatio != pCurMad->stLowAngles.ulLaControlDirRatio ||
       pNewMad->stLowAngles.ulLaControlRangeLimitScale != pCurMad->stLowAngles.ulLaControlRangeLimitScale ||
       pNewMad->stLowAngles.ulLaMinNorthStrength != pCurMad->stLowAngles.ulLaMinNorthStrength)) )
    {
        hWindow->stNewInfo.stDirty.stBits.bDeinterlace = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Window_SetDeinterlaceConfiguration);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetDeinterlaceConfiguration
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbDeinterlace,
      BVDC_Deinterlace_Settings       *pMadSettings )
{
    const BVDC_P_Deinterlace_Settings *pCurMadSettings;

    BDBG_ENTER(BVDC_Window_GetDeinterlaceConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Cur Info */
    pCurMadSettings = &hWindow->stCurInfo.stMadSettings;

    /* set new value */
    if(pbDeinterlace)
    {
        *pbDeinterlace = hWindow->stCurInfo.bDeinterlace;
    }

    if(pMadSettings)
    {
        pMadSettings->eGameMode = pCurMadSettings->eGameMode;
        pMadSettings->ePxlFormat = pCurMadSettings->ePixelFmt;
        pMadSettings->ePqEnhancement = pCurMadSettings->ePqEnhancement;
        pMadSettings->bShrinkWidth       = pCurMadSettings->bShrinkWidth;
        pMadSettings->bReverse22Pulldown = pCurMadSettings->bReverse22Pulldown;
        pMadSettings->bReverse32Pulldown = pCurMadSettings->bReverse32Pulldown;

        /* Customized structure are NULL */
        pMadSettings->pChromaSettings    = NULL;
        pMadSettings->pMotionSettings    = NULL;
        pMadSettings->pReverse32Settings = NULL;
        pMadSettings->pReverse22Settings = NULL;
        pMadSettings->pUpSampler         = NULL;
        pMadSettings->pDnSampler         = NULL;
        pMadSettings->pLowAngles         = NULL;
    }

    BDBG_LEAVE(BVDC_Window_GetDeinterlaceConfiguration);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetDeinterlaceDefaultConfiguration
    ( const BVDC_Window_Handle         hWindow,
      BVDC_Deinterlace_Settings       *pMadSettings )
{
    BDBG_ENTER(BVDC_Window_GetDeinterlaceDefaultConfiguration);
    /* sanity check input parameter as much as we can */
    BDBG_ASSERT(pMadSettings);
    BSTD_UNUSED(hWindow);

    /* Get static default */
    BVDC_P_Mvp_Init_Default(
        &pMadSettings->eGameMode,
        &pMadSettings->ePxlFormat,
        &pMadSettings->ePqEnhancement,
        &pMadSettings->bShrinkWidth,
        &pMadSettings->bReverse32Pulldown,
        &pMadSettings->bReverse22Pulldown,
         pMadSettings->pChromaSettings,
         pMadSettings->pMotionSettings);

    BVDC_P_Mvp_Init_Custom(
        pMadSettings->pUpSampler,
        pMadSettings->pDnSampler,
        pMadSettings->pLowAngles);

    BDBG_LEAVE(BVDC_Window_GetDeinterlaceDefaultConfiguration);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetAfdSettings
    ( BVDC_Window_Handle               hWindow,
      const BVDC_AfdSettings          *pAfdSettings )
{
    BDBG_ENTER(BVDC_Window_SetAfdSettings);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    hWindow->stNewInfo.stAfdSettings = *pAfdSettings;

    BDBG_LEAVE(BVDC_Window_SetAfdSettings);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetAfdSettings
    ( const BVDC_Window_Handle         hWindow,
      BVDC_AfdSettings                *pAfdSettings )
{
    BDBG_ENTER(BVDC_Window_GetAfdSettings);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    if(pAfdSettings)
    {
        *pAfdSettings = hWindow->stCurInfo.stAfdSettings;
    }

    BDBG_LEAVE(BVDC_Window_GetAfdSettings);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetPanScanType
    ( BVDC_Window_Handle               hWindow,
      BVDC_PanScanType                 ePanScanType )
{
    BDBG_ENTER(BVDC_Window_SetPanScanType);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    hWindow->stNewInfo.ePanScanType = ePanScanType;

    BDBG_LEAVE(BVDC_Window_SetPanScanType);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetPanScanType
    ( const BVDC_Window_Handle         hWindow,
      BVDC_PanScanType                *pePanScanType )
{
    BDBG_ENTER(BVDC_Window_GetPanScanType);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    if(pePanScanType)
    {
        *pePanScanType = hWindow->stCurInfo.ePanScanType;
    }

    BDBG_LEAVE(BVDC_Window_GetPanScanType);
    return BERR_SUCCESS;
}
#endif


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetUserPanScan
    ( BVDC_Window_Handle               hWindow,
      int32_t                          lHorzPanScan,
      int32_t                          lVertPanScan )
{
    BDBG_ENTER(BVDC_Window_SetUserPanScan);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    hWindow->stNewInfo.lUserHorzPanScan = lHorzPanScan;
    hWindow->stNewInfo.lUserVertPanScan = lVertPanScan;

    BDBG_LEAVE(BVDC_Window_SetUserPanScan);
    return BERR_SUCCESS;
}
#endif


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetUserPanScan
    ( const BVDC_Window_Handle         hWindow,
      int32_t                         *plHorzPanScan,
      int32_t                         *plVertPanScan )
{
    BDBG_ENTER(BVDC_Window_GetUserPanScan);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(plHorzPanScan)
    {
        *plHorzPanScan = hWindow->stCurInfo.lUserHorzPanScan;
    }

    if(plVertPanScan)
    {
        *plVertPanScan = hWindow->stCurInfo.lUserVertPanScan;
    }

    BDBG_LEAVE(BVDC_Window_GetUserPanScan);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_EnableBoxDetect
    ( BVDC_Window_Handle                hWindow,
      BVDC_Window_BoxDetectCallback_isr pfBoxDetectCb,
      void  *                           pvParm1,
      int                               iParm2,
      bool                              bAutoCutBlack )
{
    BERR_Code eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_EnableBoxDetect);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

#if (BVDC_P_SUPPORT_BOX_DETECT)
    BDBG_MSG(("LBox Detect might not work with extreme source clipping due to bandwidth issue"));
    eResult = BVDC_P_Window_EnableBoxDetect(hWindow, pfBoxDetectCb,
        pvParm1, iParm2, bAutoCutBlack );
#else
    BDBG_MSG(("LBox Detect hardware not available."));
    BSTD_UNUSED(pfBoxDetectCb);
    BSTD_UNUSED(pvParm1);
    BSTD_UNUSED(iParm2);
    BSTD_UNUSED(bAutoCutBlack);
#endif

    BDBG_LEAVE(BVDC_Window_EnableBoxDetect);
    return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_DisableBoxDetect
    ( BVDC_Window_Handle                hWindow )
{
    BERR_Code eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_DisableBoxDetect);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

#if (BVDC_P_SUPPORT_BOX_DETECT)
    eResult = BVDC_P_Window_DisableBoxDetect( hWindow );
#endif

    BDBG_LEAVE(BVDC_Window_DisableBoxDetect);
    return BERR_TRACE(eResult);
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetAspectRatioMode
    ( BVDC_Window_Handle               hWindow,
      BVDC_AspectRatioMode             eAspectRatioMode )
{
    BDBG_ENTER(BVDC_Window_SetAspectRatioMode);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    hWindow->stNewInfo.eAspectRatioMode = eAspectRatioMode;

    BDBG_LEAVE(BVDC_Window_SetAspectRatioMode);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetAspectRatioMode
    ( const BVDC_Window_Handle         hWindow,
      BVDC_AspectRatioMode            *peAspectRatioMode )
{
    BDBG_ENTER(BVDC_Window_GetAspectRatioMode);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(peAspectRatioMode)
    {
        *peAspectRatioMode = hWindow->stCurInfo.eAspectRatioMode;
    }

    BDBG_LEAVE(BVDC_Window_GetAspectRatioMode);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetNonLinearScl(
      BVDC_Window_Handle               hWindow,
      uint32_t                         ulNonLinearSrcWidth,
      uint32_t                         ulNonLinearSclOutWidth)
{
    BDBG_ENTER(BVDC_Window_SetNonLinearScl);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    hWindow->stNewInfo.ulNonlinearSrcWidth = ulNonLinearSrcWidth;
    hWindow->stNewInfo.ulNonlinearSclOutWidth = ulNonLinearSclOutWidth;

    BDBG_LEAVE(BVDC_Window_SetNonLinearScl);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetNonLinearScl
    ( const BVDC_Window_Handle         hWindow,
      uint32_t                        *pulNonLinearSrcWidth,
      uint32_t                        *pulNonLinearSclOutWidth)
{
    BDBG_ENTER(BVDC_Window_GetHorizScale);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pulNonLinearSrcWidth)
    {
        *pulNonLinearSrcWidth = hWindow->stCurInfo.ulNonlinearSrcWidth;
    }
    if(pulNonLinearSclOutWidth)
    {
        *pulNonLinearSclOutWidth = hWindow->stCurInfo.ulNonlinearSclOutWidth;
    }

    BDBG_LEAVE(BVDC_Window_GetHorizScale);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetScaleFactorRounding
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulHrzTolerance,
      uint32_t                         ulVrtTolerance )
{
    BDBG_ENTER(BVDC_Window_SetScaleFactorRounding);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    hWindow->stNewInfo.ulHrzSclFctRndToler = ulHrzTolerance;
    hWindow->stNewInfo.ulVrtSclFctRndToler = ulVrtTolerance;

    BDBG_LEAVE(BVDC_Window_SetScaleFactorRounding);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetScaleFactorRounding
    ( BVDC_Window_Handle               hWindow,
      uint32_t                        *pulHrzTolerance,
      uint32_t                        *pulVrtTolerance )
{
    BDBG_ENTER(BVDC_Window_GetScaleFactorRounding);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pulHrzTolerance)
    {
        *pulHrzTolerance = hWindow->stCurInfo.ulHrzSclFctRndToler;
    }
    if(pulVrtTolerance)
    {
        *pulVrtTolerance = hWindow->stCurInfo.ulVrtSclFctRndToler;
    }

    BDBG_LEAVE(BVDC_Window_GetScaleFactorRounding);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetMasterFrameRate
    ( BVDC_Window_Handle               hWindow,
      bool                             bUseSrcFrameRate )
{
    BDBG_ENTER(BVDC_Window_SetMasterFrameRate);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    hWindow->stNewInfo.bUseSrcFrameRate = bUseSrcFrameRate;

    BDBG_LEAVE(BVDC_Window_SetMasterFrameRate);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetMasterFrameRate
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbUseSrcFrameRate )
{
    BDBG_ENTER(BVDC_Window_GetMasterFrameRate);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pbUseSrcFrameRate)
    {
        *pbUseSrcFrameRate = hWindow->stCurInfo.bUseSrcFrameRate;
    }

    BDBG_LEAVE(BVDC_Window_GetMasterFrameRate);
    return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetRgbMatching
    ( BVDC_Window_Handle               hWindow,
      bool                             bRgbMatching )
{
    BDBG_ENTER(BVDC_Window_SetRgbMatching);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    hWindow->stNewInfo.bCscRgbMatching = bRgbMatching;

    BDBG_LEAVE(BVDC_Window_SetRgbMatching);
    return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetRgbMatching
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbRgbMatching )
{
    BDBG_ENTER(BVDC_Window_GetRgbMatching);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pbRgbMatching)
    {
        *pbRgbMatching = hWindow->stCurInfo.bCscRgbMatching;
    }

    BDBG_LEAVE(BVDC_Window_GetRgbMatching);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetContrast
    ( BVDC_Window_Handle               hWindow,
      int16_t                          sContrast )
{
    BDBG_ENTER(BVDC_Window_SetContrast);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hWindow->stNewInfo.sContrast = sContrast;

    BDBG_LEAVE(BVDC_Window_SetContrast);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetContrast
    ( BVDC_Window_Handle               hWindow,
      int16_t                         *psContrast )
{
    BDBG_ENTER(BVDC_Window_GetContrast);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(psContrast)
    {
        *psContrast = hWindow->stCurInfo.sContrast;
    }

    BDBG_LEAVE(BVDC_Window_GetContrast);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetSaturation
    ( BVDC_Window_Handle               hWindow,
      int16_t                          sSaturation )
{
    BDBG_ENTER(BVDC_Window_SetSaturation);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hWindow->stNewInfo.sSaturation = sSaturation;

    BDBG_LEAVE(BVDC_Window_SetSaturation);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetSaturation
    ( BVDC_Window_Handle               hWindow,
      int16_t                         *psSaturation )
{
    BDBG_ENTER(BVDC_Window_GetSaturation);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(psSaturation)
    {
        *psSaturation = hWindow->stCurInfo.sSaturation;
    }

    BDBG_LEAVE(BVDC_Window_GetSaturation);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetHue
    ( BVDC_Window_Handle               hWindow,
      int16_t                          sHue )
{
    BDBG_ENTER(BVDC_Window_SetHue);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hWindow->stNewInfo.sHue = sHue;

    BDBG_LEAVE(BVDC_Window_SetHue);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetHue
    ( BVDC_Window_Handle               hWindow,
      int16_t                         *psHue )
{
    BDBG_ENTER(BVDC_Window_GetHue);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(psHue)
    {
        *psHue = hWindow->stCurInfo.sHue;
    }

    BDBG_LEAVE(BVDC_Window_GetHue);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetBrightness
    ( BVDC_Window_Handle               hWindow,
      int16_t                          sBrightness )
{
    BDBG_ENTER(BVDC_Window_SetBrightness);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hWindow->stNewInfo.sBrightness = sBrightness;

    BDBG_LEAVE(BVDC_Window_SetBrightness);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetBrightness
    ( BVDC_Window_Handle               hWindow,
      int16_t                         *psBrightness )
{
    BDBG_ENTER(BVDC_Window_GetBrightness);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(psBrightness)
    {
        *psBrightness = hWindow->stCurInfo.sBrightness;
    }

    BDBG_LEAVE(BVDC_Window_GetBrightness);
    return BERR_SUCCESS;
}
#endif

#if (BVDC_P_SUPPORT_TNT_VER >= 6)            /* TNT-R HW base */
#if !B_REFSW_MINIMAL
BERR_Code BVDC_Window_SetSharpnessConfig
    ( BVDC_Window_Handle               hWindow,
      const BVDC_SharpnessSettings    *pSharpnessSettings )
{
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(pSharpnessSettings);
    return BERR_TRACE(BVDC_ERR_TNT_WINDOW_NOT_SUPPORTED);
}
#endif


#if !B_REFSW_MINIMAL
BERR_Code BVDC_Window_GetSharpnessConfig
    ( BVDC_Window_Handle               hWindow,
      BVDC_SharpnessSettings          *pSharpnessSettings )
{
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(pSharpnessSettings);
    return BERR_TRACE(BVDC_ERR_TNT_WINDOW_NOT_SUPPORTED);
}
#endif
#elif (BVDC_P_SUPPORT_TNT_VER == 5)            /* TNT2 HW base */
#if !B_REFSW_MINIMAL
BERR_Code BVDC_Window_SetSharpnessConfig
    ( BVDC_Window_Handle               hWindow,
      const BVDC_SharpnessSettings    *pSharpnessSettings )

{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_SetSharpnessConfig);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Only support main display's main window */
    if((hWindow->eId != BVDC_P_WindowId_eComp0_V0) &&
        (NULL !=pSharpnessSettings))
    {
        return BERR_TRACE(BVDC_ERR_TNT_WINDOW_NOT_SUPPORTED);
    }

    if(pSharpnessSettings != NULL)
    {
        err = BVDC_P_Tnt_ValidateSharpnessSettings(pSharpnessSettings);
        if (err != BERR_SUCCESS)
        {
            return err;
        }
        else
        {
            /* set new value */
            hWindow->stNewInfo.bUserSharpnessConfig = true;
            hWindow->stNewInfo.stSharpnessConfig = *pSharpnessSettings;

            BVDC_P_Tnt_StoreSharpnessSettings(hWindow, pSharpnessSettings);
        }
    }
    else
    {
        hWindow->stNewInfo.bUserSharpnessConfig = false;
    }

    hWindow->stNewInfo.stDirty.stBits.bTntAdjust = BVDC_P_DIRTY;
    hWindow->stNewInfo.sSharpness = 0;

    BDBG_LEAVE(BVDC_Window_SetSharpnessConfig);
    return err;

}
#endif


#if !B_REFSW_MINIMAL
BERR_Code BVDC_Window_GetSharpnessConfig
    ( BVDC_Window_Handle               hWindow,
      BVDC_SharpnessSettings          *pSharpnessSettings )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_GetSharpnessConfig);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(pSharpnessSettings);

    /* Only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_TNT_WINDOW_NOT_SUPPORTED);
    }

    if(pSharpnessSettings)
    {
        *pSharpnessSettings = hWindow->stCurInfo.stSharpnessConfig;
    }

    BDBG_LEAVE(BVDC_Window_GetSharpnessConfig);
    return err;

}
#endif

#else

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetSharpnessConfig
    ( BVDC_Window_Handle               hWindow,
      const BVDC_SharpnessSettings    *pSharpnessSettings )
{
    BDBG_ENTER(BVDC_Window_SetSharpnessConfig);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Only support main display's main window */
    if((hWindow->eId != BVDC_P_WindowId_eComp0_V0) &&
        (NULL != pSharpnessSettings))
    {
        return BERR_TRACE(BVDC_ERR_TNT_WINDOW_NOT_SUPPORTED);
    }

    if(pSharpnessSettings != NULL)
    {
        /* set new value */
        hWindow->stNewInfo.bUserSharpnessConfig = true;
        hWindow->stNewInfo.stSharpnessConfig = *pSharpnessSettings;
    }
    else
    {
        hWindow->stNewInfo.bUserSharpnessConfig = false;
    }

    hWindow->stNewInfo.stDirty.stBits.bTntAdjust = BVDC_P_DIRTY;
    hWindow->stNewInfo.sSharpness = 0;

    BDBG_LEAVE(BVDC_Window_SetSharpnessConfig);
    return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
 BERR_Code BVDC_Window_GetSharpnessConfig
    ( BVDC_Window_Handle               hWindow,
      BVDC_SharpnessSettings          *pSharpnessSettings )
{
    BDBG_ENTER(BVDC_Window_GetSharpnessConfig);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pSharpnessSettings)
    {
        *pSharpnessSettings = hWindow->stCurInfo.stSharpnessConfig;
        if(!hWindow->stCurInfo.bUserSharpnessConfig)
        {
            BKNI_EnterCriticalSection();
            pSharpnessSettings->ulLumaCtrlGain = hWindow->stCurResource.hPep->ulLumaChromaGain;
            pSharpnessSettings->ulChromaCtrlGain = hWindow->stCurResource.hPep->ulLumaChromaGain;
            BKNI_LeaveCriticalSection();
        }
    }

    BDBG_LEAVE(BVDC_Window_GetSharpnessConfig);
    return BERR_SUCCESS;
}
#endif
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetSharpness
    ( BVDC_Window_Handle               hWindow,
      bool                             bSharpnessEnable,
      int16_t                          sSharpness )
{
    BDBG_ENTER(BVDC_Window_SetSharpness);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Only support main display's main window */
    if((hWindow->eId != BVDC_P_WindowId_eComp0_V0) &&
        ( true == bSharpnessEnable))
    {
        return BERR_TRACE(BVDC_ERR_TNT_WINDOW_NOT_SUPPORTED);
    }

    hWindow->stNewInfo.bSharpnessEnable = bSharpnessEnable;
    hWindow->stNewInfo.sSharpness = sSharpness;

#if BVDC_P_SUPPORT_TNTD
    if(hWindow->stNewInfo.bSharpnessEnable != hWindow->stCurInfo.bSharpnessEnable)
    {
        hWindow->stNewInfo.stDirty.stBits.bTntd = BVDC_P_DIRTY;
    }
#endif

    BDBG_LEAVE(BVDC_Window_SetSharpness);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetSharpness
    ( BVDC_Window_Handle               hWindow,
      bool                            *pbSharpnessEnable,
      int16_t                         *psSharpness )
{
    BDBG_ENTER(BVDC_Window_GetSharpness);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(psSharpness)
    {
        *psSharpness = hWindow->stCurInfo.sSharpness;
    }

    if(pbSharpnessEnable)
    {
        *pbSharpnessEnable = hWindow->stCurInfo.bSharpnessEnable;
    }

    BDBG_LEAVE(BVDC_Window_GetSharpness);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetColorTemp
    (BVDC_Window_Handle hWindow,
     int16_t sColorTemp)
{
    int32_t lAttenuationR;
    int32_t lAttenuationG;
    int32_t lAttenuationB;
    BVDC_P_Csc3x4 stCscCoeffs;

    BDBG_ENTER(BVDC_Window_SetColorTemp);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hWindow->stNewInfo.sColorTemp = sColorTemp;

    BVDC_P_Cfc_ColorTempToAttenuationRGB(sColorTemp,
        &lAttenuationR, &lAttenuationG, &lAttenuationB, &stCscCoeffs);

    hWindow->stNewInfo.lAttenuationR = lAttenuationR;
    hWindow->stNewInfo.lAttenuationG = lAttenuationG;
    hWindow->stNewInfo.lAttenuationB = lAttenuationB;
    hWindow->stNewInfo.lOffsetR = 0;
    hWindow->stNewInfo.lOffsetG = 0;
    hWindow->stNewInfo.lOffsetB = 0;

    BDBG_LEAVE(BVDC_Window_SetColorTemp);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetColorTemp
    ( BVDC_Window_Handle               hWindow,
      int16_t                         *psColorTemp )
{
    BDBG_ENTER(BVDC_Window_GetColorTemp);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(psColorTemp)
    {
        *psColorTemp= hWindow->stCurInfo.sColorTemp;
    }

    BDBG_LEAVE(BVDC_Window_GetColorTemp);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetAttenuationRGB
    (BVDC_Window_Handle hWindow,
      int32_t   nAttentuationR,
      int32_t   nAttentuationG,
      int32_t   nAttentuationB,
      int32_t   nOffsetR,
      int32_t   nOffsetG,
      int32_t   nOffsetB)
{
    BDBG_ENTER(BVDC_Window_SetAttenuationRGB);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hWindow->stNewInfo.lAttenuationR = BVDC_P_CFC_ITOFIX(nAttentuationR);
    hWindow->stNewInfo.lAttenuationG = BVDC_P_CFC_ITOFIX(nAttentuationG);
    hWindow->stNewInfo.lAttenuationB = BVDC_P_CFC_ITOFIX(nAttentuationB);
    hWindow->stNewInfo.lOffsetR      = BVDC_P_CFC_ITOFIX(nOffsetR);
    hWindow->stNewInfo.lOffsetG      = BVDC_P_CFC_ITOFIX(nOffsetG);
    hWindow->stNewInfo.lOffsetB      = BVDC_P_CFC_ITOFIX(nOffsetB);

    BDBG_LEAVE(BVDC_Window_SetAttenuationRGB);

    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetAttenuationRGB
    ( BVDC_Window_Handle               hWindow,
      int32_t                         *plAttenuationR,
      int32_t                         *plAttenuationG,
      int32_t                         *plAttenuationB,
      int32_t                         *plOffsetR,
      int32_t                         *plOffsetG,
      int32_t                         *plOffsetB )
{
    int32_t ulShiftBits;

    BDBG_ENTER(BVDC_Window_GetAttenuationRGB);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    ulShiftBits = BVDC_P_CSC_SW_CX_F_BITS;

    (*plAttenuationR) = hWindow->stCurInfo.lAttenuationR >> ulShiftBits;
    (*plAttenuationG) = hWindow->stCurInfo.lAttenuationG >> ulShiftBits;
    (*plAttenuationB) = hWindow->stCurInfo.lAttenuationB >> ulShiftBits;
    (*plOffsetR)      = hWindow->stCurInfo.lOffsetR      >> ulShiftBits;
    (*plOffsetG)      = hWindow->stCurInfo.lOffsetG      >> ulShiftBits;
    (*plOffsetB)      = hWindow->stCurInfo.lOffsetB      >> ulShiftBits;

    BDBG_LEAVE(BVDC_Window_GetAttenuationRGB);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetAutoFlesh
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulFleshtone )
{
    BDBG_ENTER(BVDC_Window_SetAutoFlesh);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }
    if(ulFleshtone > BVDC_P_PEP_MAX_CAB_SETTING_GRANUALITY)
    {
        return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
    }

    hWindow->stNewInfo.ulFleshtone = ulFleshtone;

    BDBG_LEAVE(BVDC_Window_SetAutoFlesh);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetAutoFlesh
    ( const BVDC_Window_Handle          hWindow,
      uint32_t                         *pulFleshtone )
{
    BDBG_ENTER(BVDC_Window_GetAutoFlesh);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pulFleshtone)
    {
        *pulFleshtone = hWindow->stCurInfo.ulFleshtone;
    }

    BDBG_LEAVE(BVDC_Window_GetAutoFlesh);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetGreenBoost
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulGreenBoost )
{
    BDBG_ENTER(BVDC_Window_SetGreenBoost);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }
    if(ulGreenBoost > BVDC_P_PEP_MAX_CAB_SETTING_GRANUALITY)
    {
        return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
    }

    hWindow->stNewInfo.ulGreenBoost = ulGreenBoost;

    BDBG_LEAVE(BVDC_Window_SetGreenBoost);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetGreenBoost
    ( const BVDC_Window_Handle          hWindow,
      uint32_t                         *pulGreenBoost )
{
    BDBG_ENTER(BVDC_Window_GetGreenBoost);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pulGreenBoost)
    {
        *pulGreenBoost= hWindow->stCurInfo.ulGreenBoost;
    }

    BDBG_LEAVE(BVDC_Window_GetGreenBoost);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetBlueBoost
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulBlueBoost )
{
    BDBG_ENTER(BVDC_Window_SetBlueBoost);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }
    if(ulBlueBoost > BVDC_P_PEP_MAX_CAB_SETTING_GRANUALITY)
    {
        return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
    }

    hWindow->stNewInfo.ulBlueBoost = ulBlueBoost;

    BDBG_LEAVE(BVDC_Window_SetBlueBoost);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetBlueBoost
    ( const BVDC_Window_Handle          hWindow,
      uint32_t                         *pulBlueBoost )
{
    BDBG_ENTER(BVDC_Window_GetBlueBoost);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pulBlueBoost)
    {
        *pulBlueBoost = hWindow->stCurInfo.ulBlueBoost;
    }

    BDBG_LEAVE(BVDC_Window_GetBlueBoost);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetCmsControl
    ( const BVDC_Window_Handle          hWindow,
      const BVDC_ColorBar              *pSaturationGain,
      const BVDC_ColorBar              *pHueGain )
{
    BDBG_ENTER(BVDC_Window_SetCmsControl);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }
    if(pSaturationGain &&
       !BVDC_P_PEP_CMS_SAT_WITHIN_RANGE(pSaturationGain))
    {
        return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
    }
    if(pHueGain &&
       !BVDC_P_PEP_CMS_HUE_WITHIN_RANGE(pHueGain))
    {
        return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
    }

    if(pSaturationGain)
    {
        hWindow->stNewInfo.stSatGain = *pSaturationGain;
    }
    if(pHueGain)
    {
        hWindow->stNewInfo.stHueGain = *pHueGain;
    }

    BDBG_LEAVE(BVDC_Window_SetCmsControl);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetCmsControl
    ( const BVDC_Window_Handle          hWindow,
      BVDC_ColorBar                    *pSaturationGain,
      BVDC_ColorBar                    *pHueGain )
{
    BDBG_ENTER(BVDC_Window_GetCmsControl);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pSaturationGain)
    {
        *pSaturationGain = hWindow->stCurInfo.stSatGain;
    }

    if(pHueGain)
    {
        *pHueGain = hWindow->stCurInfo.stHueGain;
    }

    BDBG_LEAVE(BVDC_Window_GetCmsControl);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_EnableContrastStretch
    ( BVDC_Window_Handle               hWindow,
      bool                             bEnable )
{
    BDBG_ENTER(BVDC_Window_EnableContrastStretch);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }

    hWindow->stNewInfo.bContrastStretch   = bEnable;

    BDBG_LEAVE(BVDC_Window_SetContrastStretch);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetContrastStretch
    ( BVDC_Window_Handle               hWindow,
      const BVDC_ContrastStretch      *pContrastStretch )
{
    BDBG_ENTER(BVDC_Window_SetContrastStretch);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }

    /*
    * Validating user data:
    * - All values in the table should be 0-1023.
    * - The first column should start at 64 or less, and finish at 940 or more,
    *   and should always be increasing (avoids the zero problem we have found here).
    * - The values along each row (excluding the first column) should always be increasing.
    * - The values in the IRE table should always be increasing, with increments
    *   of at least 16 between each entry.
    */
    if(pContrastStretch && pContrastStretch->aulDcTable1[0] != 0)
    {
        uint32_t i, j = 0;
        for(i = 0; i < BVDC_DC_TABLE_ROWS * BVDC_DC_TABLE_COLS; i++)
        {
            if(pContrastStretch->aulDcTable1[i] > BVDC_P_PEP_MAX_LUMA_VALUE ||
                (pContrastStretch->bInterpolateTables &&
                pContrastStretch->aulDcTable2[i] > BVDC_P_PEP_MAX_LUMA_VALUE))
            {
                BDBG_MSG(("No value in the table should be greater than %d", BVDC_P_PEP_MAX_LUMA_VALUE));
                BDBG_MSG(("aulDcTable1[%d] = %d, bInterpolateTables = %d, aulDcTable2[%d] = %d",
                    i, pContrastStretch->aulDcTable1[i], pContrastStretch->bInterpolateTables,
                    i, pContrastStretch->aulDcTable2[i]));
                return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
            }

            if(j == 1 &&
                (pContrastStretch->aulDcTable1[i] > BVDC_P_PEP_BLACK_LUMA_VALUE ||
                (pContrastStretch->bInterpolateTables &&
                pContrastStretch->aulDcTable2[i] > BVDC_P_PEP_BLACK_LUMA_VALUE)))
            {
                BDBG_MSG(("The first point on each curve should be %d or less (black level)", BVDC_P_PEP_BLACK_LUMA_VALUE));
                BDBG_MSG(("aulDcTable1[%d] = %d, bInterpolateTables = %d, aulDcTable2[%d] = %d",
                    i, pContrastStretch->aulDcTable1[i], pContrastStretch->bInterpolateTables,
                    i, pContrastStretch->aulDcTable2[i]));
                return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
            }

            if(j > 0 && j < (BVDC_DC_TABLE_COLS - 1) &&
                (i < (BVDC_DC_TABLE_ROWS * BVDC_DC_TABLE_COLS - 1)) &&
                (pContrastStretch->aulDcTable1[i] > pContrastStretch->aulDcTable1[i + 1] ||
                (pContrastStretch->bInterpolateTables &&
                pContrastStretch->aulDcTable2[i] > pContrastStretch->aulDcTable2[i + 1])))
            {
                BDBG_MSG(("Each point on a curve should be greater than or equal to the previous point on the curve (monotonically increasing along the curve)"));
                BDBG_MSG(("aulDcTable1[%d] = %d, aulDcTable1[%d] = %d, bInterpolateTables = %d, aulDcTable2[%d] = %d, aulDcTable2[%d] = %d",
                    i, pContrastStretch->aulDcTable1[i], i+1, pContrastStretch->aulDcTable1[i+1],
                    pContrastStretch->bInterpolateTables,
                    i, pContrastStretch->aulDcTable2[i], i+1, pContrastStretch->aulDcTable2[i + 1]));
                return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
            }

            j++;
            if(j == BVDC_DC_TABLE_COLS)
            {
                if(pContrastStretch->aulDcTable1[i] < BVDC_P_PEP_WHITE_LUMA_VALUE ||
                    (pContrastStretch->bInterpolateTables &&
                    pContrastStretch->aulDcTable2[i] < BVDC_P_PEP_WHITE_LUMA_VALUE))
                {
                    BDBG_MSG(("The last point on each curve should be %d or more (white level)", BVDC_P_PEP_WHITE_LUMA_VALUE));
                    BDBG_MSG(("aulDcTable1[%d] = %d, bInterpolateTables = %d, aulDcTable2[%d] = %d",
                        i, pContrastStretch->aulDcTable1[i], pContrastStretch->bInterpolateTables,
                        i, pContrastStretch->aulDcTable2[i]));
                    return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
                }
                j = 0;
            }
        }

        for(i = 1; i < BVDC_DC_TABLE_COLS - 1; i++)
        {
            if((pContrastStretch->alIreTable[i] - pContrastStretch->alIreTable[i - 1]) < 16)
            {
                BDBG_MSG(("Steps between values in the IRE table must be at least 16"));
                BDBG_MSG(("alIreTable[%d] = %d, alIreTable[%d] = %d",
                    i, pContrastStretch->alIreTable[i], i - 1, pContrastStretch->alIreTable[i - 1]));
                return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
            }
        }
    }

    if(pContrastStretch)
    {
        hWindow->stNewInfo.stContrastStretch = *pContrastStretch;
    }

    hWindow->stNewInfo.stDirty.stBits.bLabAdjust = BVDC_P_DIRTY;

    BDBG_LEAVE(BVDC_Window_SetContrastStretch);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetContrastStretch
    ( const BVDC_Window_Handle          hWindow,
      BVDC_ContrastStretch             *pContrastStretch )
{
    BDBG_ENTER(BVDC_Window_GetContrastStretch);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pContrastStretch)
    {
        *pContrastStretch = hWindow->stCurInfo.stContrastStretch;
    }

    BDBG_LEAVE(BVDC_Window_GetContrastStretch);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetBlueStretch
    ( BVDC_Window_Handle               hWindow,
      const BVDC_BlueStretch          *pBlueStretch )
{
    BDBG_ENTER(BVDC_Window_SetBlueStretch);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }

    hWindow->stNewInfo.bBlueStretch = false;
    BSTD_UNUSED(pBlueStretch);

    BDBG_LEAVE(BVDC_Window_SetBlueStretch);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetBlueStretch
    ( const BVDC_Window_Handle          hWindow,
      BVDC_BlueStretch                 *pBlueStretch )
{
    BDBG_ENTER(BVDC_Window_GetBlueStretch);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pBlueStretch)
    {
        *pBlueStretch = hWindow->stCurInfo.stBlueStretch;
    }

    BDBG_LEAVE(BVDC_Window_GetBlueStretch);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetSplitScreenMode
    ( BVDC_Window_Handle                      hWindow,
      const BVDC_Window_SplitScreenSettings  *pSplitScreen )
{
    BVDC_Window_SplitScreenSettings *pSetting;

    BDBG_ENTER(BVDC_Window_SetSplitScreenMode);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    pSetting = &hWindow->stNewInfo.stSplitScreenSetting;

    if((pSplitScreen->eHue             != BVDC_SplitScreenMode_eDisable ||
        pSplitScreen->eContrast        != BVDC_SplitScreenMode_eDisable ||
        pSplitScreen->eBrightness      != BVDC_SplitScreenMode_eDisable ||
        pSplitScreen->eColorTemp       != BVDC_SplitScreenMode_eDisable) &&
       (hWindow->eId != BVDC_P_WindowId_eComp0_V0))
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }

    if((pSplitScreen->eAutoFlesh       != BVDC_SplitScreenMode_eDisable ||
        pSplitScreen->eSharpness       != BVDC_SplitScreenMode_eDisable ||
        pSplitScreen->eBlueBoost       != BVDC_SplitScreenMode_eDisable ||
        pSplitScreen->eGreenBoost      != BVDC_SplitScreenMode_eDisable ||
        pSplitScreen->eCms             != BVDC_SplitScreenMode_eDisable ||
        pSplitScreen->eContrastStretch != BVDC_SplitScreenMode_eDisable ||
        pSplitScreen->eBlueStretch     != BVDC_SplitScreenMode_eDisable) &&
       (hWindow->eId != BVDC_P_WindowId_eComp0_V0))
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }

    /* Since Auto Flesh, Green Boost and Blue Boost are controlled */
    /* by CAB, their setting can't be different */
    if((pSplitScreen->eAutoFlesh != pSplitScreen->eBlueBoost)  ||
       (pSplitScreen->eAutoFlesh != pSplitScreen->eGreenBoost) ||
       (pSplitScreen->eBlueBoost != pSplitScreen->eGreenBoost))
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* User can select which features of the display (hue, brightness, contrast,  */
    /* or colortemp) to apply to the split screen demo mode but they have */
    /* to be applied to the same side of the screen */
    if(pSplitScreen->eHue != BVDC_SplitScreenMode_eDisable &&
       ((pSplitScreen->eBrightness != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eBrightness != pSplitScreen->eHue) ||
        (pSplitScreen->eContrast != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eContrast != pSplitScreen->eHue) ||
        (pSplitScreen->eColorTemp != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eColorTemp != pSplitScreen->eHue)))
    {
        BDBG_ERR(("Hue=%d, Brightness=%d, Contrast=%d, ColorTemp=%d",
            pSplitScreen->eHue, pSplitScreen->eBrightness, pSplitScreen->eContrast, pSplitScreen->eColorTemp));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if(pSplitScreen->eBrightness != BVDC_SplitScreenMode_eDisable &&
       ((pSplitScreen->eHue != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eHue != pSplitScreen->eBrightness) ||
        (pSplitScreen->eContrast != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eContrast != pSplitScreen->eBrightness) ||
        (pSplitScreen->eColorTemp != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eColorTemp != pSplitScreen->eBrightness)))
    {
        BDBG_ERR(("Brightness=%d, Hue=%d, Contrast=%d, ColorTemp=%d",
            pSplitScreen->eBrightness, pSplitScreen->eHue, pSplitScreen->eContrast, pSplitScreen->eColorTemp));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if(pSplitScreen->eContrast != BVDC_SplitScreenMode_eDisable &&
       ((pSplitScreen->eHue != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eHue != pSplitScreen->eContrast) ||
        (pSplitScreen->eBrightness != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eBrightness != pSplitScreen->eContrast) ||
        (pSplitScreen->eColorTemp != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eColorTemp != pSplitScreen->eContrast)))
    {
        BDBG_ERR(("Contrast=%d, Hue=%d, Brightness=%d, ColorTemp=%d",
            pSplitScreen->eContrast, pSplitScreen->eHue, pSplitScreen->eBrightness, pSplitScreen->eColorTemp));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if(pSplitScreen->eColorTemp != BVDC_SplitScreenMode_eDisable &&
       ((pSplitScreen->eHue != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eHue != pSplitScreen->eColorTemp) ||
        (pSplitScreen->eBrightness != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eBrightness != pSplitScreen->eColorTemp) ||
        (pSplitScreen->eContrast != BVDC_SplitScreenMode_eDisable &&
         pSplitScreen->eContrast != pSplitScreen->eColorTemp)))
    {
        BDBG_ERR(("ColorTemp=%d, Hue=%d, Brightness=%d, Contrast=%d",
            pSplitScreen->eColorTemp, pSplitScreen->eContrast, pSplitScreen->eHue, pSplitScreen->eBrightness));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Indicate require copy of new bits */
    if((pSetting->eDeJagging != pSplitScreen->eDeJagging) ||
       (pSetting->eDeRinging != pSplitScreen->eDeRinging))
    {
        hWindow->stNewInfo.stDirty.stBits.bMiscCtrl = BVDC_P_DIRTY;
    }

    *pSetting = *pSplitScreen;

    BDBG_LEAVE(BVDC_Window_SetSplitScreenMode);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetSplitScreenMode
    ( const BVDC_Window_Handle          hWindow,
      BVDC_Window_SplitScreenSettings  *pSplitScreen )
{
    BDBG_ENTER(BVDC_Window_GetSplitScreenMode);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pSplitScreen)
    {
        *pSplitScreen = hWindow->stCurInfo.stSplitScreenSetting;
    }

    BDBG_LEAVE(BVDC_Window_GetSplitScreenMode);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetForceCapture
    ( BVDC_Window_Handle               hWindow,
      bool                             bForceCapture )
{
    BDBG_ENTER(BVDC_Window_SetForceCapture);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hWindow->stNewInfo.bForceCapture = bForceCapture;

    BDBG_LEAVE(BVDC_Window_SetForceCapture);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetForceCapture
    ( const BVDC_Window_Handle          hWindow,
      bool                             *pbForceCapture )
{
    BDBG_ENTER(BVDC_Window_GetForceCapture);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pbForceCapture)
    {
        *pbForceCapture = hWindow->stCurInfo.bForceCapture;
    }

    BDBG_LEAVE(BVDC_Window_GetForceCapture);
    return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetMaxDelay
    ( const BVDC_Window_Handle         hWindow,
      unsigned int                    *puiMaxVsyncDelay )
{
    BDBG_ENTER(BVDC_Window_GetMaxDelay);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Base on if this window has deinterlacer, and/or capture. */
    if(puiMaxVsyncDelay)
    {
        *puiMaxVsyncDelay  = BVDC_P_LIP_SYNC_VEC_DELAY;
        *puiMaxVsyncDelay += (hWindow->stResourceRequire.bRequirePlayback)
            ? BVDC_P_LIP_SYNC_CAP_PLK_SLIP_DELAY : 0;
        *puiMaxVsyncDelay += (hWindow->stResourceFeature.ulMad != BVDC_P_Able_eInvalid)
            ? BVDC_P_LIP_SYNC_DEINTERLACED_DELAY : 0;
    }

    BDBG_LEAVE(BVDC_Window_GetMaxDelay);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetDelayOffset
    ( BVDC_Window_Handle               hWindow,
      unsigned int                     uiVsyncDelayOffset )
{
    BDBG_ENTER(BVDC_Window_SetDelayOffset);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hWindow->stNewInfo.uiVsyncDelayOffset = uiVsyncDelayOffset;

    BDBG_LEAVE(BVDC_Window_SetDelayOffset);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetDelayOffset
    ( const BVDC_Window_Handle         hWindow,
      unsigned int                    *puiVsyncDelayOffset )
{
    BDBG_ENTER(BVDC_Window_GetDelayOffset);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if( puiVsyncDelayOffset )
    {
        *puiVsyncDelayOffset = hWindow->stCurInfo.uiVsyncDelayOffset;
    }

    BDBG_LEAVE(BVDC_Window_GetDelayOffset);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetMosaicConfiguration
    ( BVDC_Window_Handle               hWindow,
      bool                             bEnable,
      const BVDC_MosaicConfiguration  *pSettings )
{
#if BVDC_P_SUPPORT_MOSAIC_MODE
    BDBG_ENTER(BVDC_Window_SetMosaicConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* preliminary error checking */
    if(bEnable && (!pSettings || !hWindow->bClearRectSupport ||
       hWindow->stNewInfo.hSource->hVdc->stSettings.bDisableMosaic ||
       !BVDC_P_SRC_VALID_COMBO_TRIGGER(hWindow->stNewInfo.hSource->eCombTrigger)))
    {
        if(!pSettings)
        {
            BDBG_ERR(("Please provide Moasic configuration settings!"));
        }
        if(!hWindow->bClearRectSupport)
        {
            BDBG_ERR(("This window[%d] does not support Mosaic Mode!", hWindow->eId));
        }
        if(hWindow->stNewInfo.hSource->hVdc->stSettings.bDisableMosaic)
        {
            BDBG_ERR(("This window[%d] mosaic disabled by BVDC_Settings.", hWindow->eId));
        }
        if(!BVDC_P_SRC_VALID_COMBO_TRIGGER(hWindow->stNewInfo.hSource->eCombTrigger))
        {
            BDBG_ERR(("This window[%d] mosaic disabled with no valid combo trigger for source[%d].",
                hWindow->eId, hWindow->stNewInfo.hSource->eId));
        }
        return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
    }

    hWindow->stNewInfo.bClearRect = bEnable;
    if(bEnable)
    {
        uint32_t ulARGB;

        /* 8-bit settings for now */
        if((pSettings->ulClearRectAlpha > 255) ||
           (pSettings->bClearRectByMaskColor &&
            (pSettings->ulMaskColorBlue  |
             pSettings->ulMaskColorGreen |
             pSettings->ulMaskColorRed) > 255))
        {
            return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
        }

        if(!BVDC_P_SRC_IS_MPEG(hWindow->stNewInfo.hSource->eId) && pSettings->bVideoInMosaics)
        {
            BDBG_ERR(("Video in Mosaic Mode must be MPEG window!"));
            return(BVDC_ERR_INVALID_MOSAIC_MODE);
        }

        hWindow->stNewInfo.bMosaicMode   = pSettings->bVideoInMosaics;
        hWindow->stNewInfo.bClearRectByMaskColor = pSettings->bClearRectByMaskColor;
        hWindow->stNewInfo.ulClearRectAlpha = pSettings->ulClearRectAlpha;
        ulARGB = BPXL_MAKE_PIXEL(BPXL_eA8_R8_G8_B8,
            0x00, pSettings->ulMaskColorRed, pSettings->ulMaskColorGreen,
            pSettings->ulMaskColorBlue);
        BPXL_ConvertPixel_RGBtoYCbCr(BPXL_eA8_Y8_Cb8_Cr8, BPXL_eA8_R8_G8_B8,
            ulARGB, (unsigned int*)&hWindow->stNewInfo.ulMaskColorYCrCb);
    }
    else
    {
        hWindow->stNewInfo.bMosaicMode = false;
    }
    BDBG_LEAVE(BVDC_Window_SetMosaicConfiguration);
#else
    BDBG_ERR(("This chipset doesn't support Mosaic Mode!"));
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(bEnable);
    BSTD_UNUSED(pSettings);
#endif
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetMosaicConfiguration
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbEnable,
      BVDC_MosaicConfiguration        *pSettings )
{
#if BVDC_P_SUPPORT_MOSAIC_MODE
    BDBG_ENTER(BVDC_Window_GetMosaicConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    if(pbEnable)
    {
        *pbEnable = hWindow->stCurInfo.bClearRect;
    }

    if(pSettings)
    {
        unsigned int uiARGB;

        /* To make sure thing get initialize */
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));

        pSettings->bVideoInMosaics = hWindow->stCurInfo.bMosaicMode;
        pSettings->bClearRectByMaskColor = hWindow->stCurInfo.bClearRectByMaskColor;
        pSettings->ulClearRectAlpha = hWindow->stCurInfo.ulClearRectAlpha;
        BPXL_ConvertPixel_YCbCrtoRGB(BPXL_eA8_R8_G8_B8, BPXL_eA8_Y8_Cb8_Cr8,
            hWindow->stNewInfo.ulMaskColorYCrCb, 0, 0, &uiARGB);
        pSettings->ulMaskColorRed   = BPXL_GET_COMPONENT(BPXL_eA8_R8_G8_B8, uiARGB, 2);
        pSettings->ulMaskColorGreen = BPXL_GET_COMPONENT(BPXL_eA8_R8_G8_B8, uiARGB, 1);
        pSettings->ulMaskColorBlue  = BPXL_GET_COMPONENT(BPXL_eA8_R8_G8_B8, uiARGB, 0);
    }

    BDBG_LEAVE(BVDC_Window_GetMosaicConfiguration);
#else
    BDBG_ERR(("This chipset doesn't support Mosaic Mode!"));
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(pbEnable);
    BSTD_UNUSED(pSettings);
#endif
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetMosaicRectsVisibility
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulRectsCount,
      const bool                       abRectsVisible[] )
{
#if BVDC_P_SUPPORT_MOSAIC_MODE
    BDBG_ENTER(BVDC_Window_SetMosaicRectsVisibility);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* preliminary error checking */
    if((ulRectsCount > hWindow->stSettings.ulMaxMosaicRect) ||
       (ulRectsCount && !abRectsVisible))
    {
        return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
    }
    if(!hWindow->bClearRectSupport)
    {
        BDBG_ERR(("This window[%d] does not support Mosaic Mode!", hWindow->eId));
        return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
    }
    hWindow->stNewInfo.ulMosaicCount = ulRectsCount;

    if(ulRectsCount)
    {
        BKNI_Memcpy((void*)hWindow->stNewInfo.abMosaicVisible, (void*)abRectsVisible,
            sizeof(bool) * ulRectsCount);
    }
    BDBG_LEAVE(BVDC_Window_SetMosaicRectsVisibility);
#else
    BDBG_ERR(("This chipset doesn't support Mosaic Mode!"));
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(ulRectsCount);
    BSTD_UNUSED(abRectsVisible);
#endif
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetMosaicDstRects
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulMosaicCount,
      const BVDC_Rect                  astRect[] )
{
#if BVDC_P_SUPPORT_MOSAIC_MODE
    uint32_t   i;

    BDBG_ENTER(BVDC_Window_SetMosaicDstRects);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* preliminary error checking */
    if((ulMosaicCount > hWindow->stSettings.ulMaxMosaicRect) ||
       (ulMosaicCount && !astRect))
    {
        return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
    }
    if(!hWindow->bClearRectSupport)
    {
        BDBG_ERR(("This window[%d] does not support Mosaic Mode!", hWindow->eId));
        return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
    }

    hWindow->stNewInfo.ulMosaicCount = ulMosaicCount;
    for(i = 0; i < ulMosaicCount; i++)
    {
        hWindow->stNewInfo.astMosaicRect[i].lLeft    = astRect[i].lLeft;
        hWindow->stNewInfo.astMosaicRect[i].lLeft_R  = astRect[i].lLeft;
        hWindow->stNewInfo.astMosaicRect[i].lTop     = astRect[i].lTop;
        hWindow->stNewInfo.astMosaicRect[i].ulWidth  = astRect[i].ulWidth;
        hWindow->stNewInfo.astMosaicRect[i].ulHeight = astRect[i].ulHeight;
    }

    BDBG_LEAVE(BVDC_Window_SetMosaicDstRects);
#else
    BDBG_ERR(("This chipset doesn't support Mosaic Mode!"));
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(ulMosaicCount);
    BSTD_UNUSED(astRect);
#endif
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetMosaicRectsZOrder
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulRectsCount,
      const uint8_t                    aucZOrder[] )
{
#if BVDC_P_SUPPORT_MOSAIC_MODE
    BDBG_ENTER(BVDC_Window_SetMosaicRectsZOrder);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* preliminary error checking */
    if((ulRectsCount > hWindow->stSettings.ulMaxMosaicRect) ||
       (ulRectsCount && !aucZOrder))
    {
        return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
    }
    if(!hWindow->bClearRectSupport)
    {
        BDBG_ERR(("This window[%d] does not support Mosaic Mode!", hWindow->eId));
        return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
    }
    hWindow->stNewInfo.ulMosaicCount = ulRectsCount;

    if(ulRectsCount)
    {
        BKNI_Memcpy((void*)hWindow->stNewInfo.aucMosaicZOrder, (void*)aucZOrder,
            sizeof(uint8_t) * ulRectsCount);
    }

    BDBG_LEAVE(BVDC_Window_SetMosaicRectsZOrder);

#else
    BDBG_ERR(("This chipset doesn't support Mosaic Mode!"));
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(ulRectsCount);
    BSTD_UNUSED(aucZOrder);
#endif
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetMosaicTrack
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulChannelId )
{
#if BVDC_P_SUPPORT_MOSAIC_MODE
    BDBG_ENTER(BVDC_Window_SetMosaicTrack);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* preliminary error checking */
    if(ulChannelId >= hWindow->stSettings.ulMaxMosaicRect)
    {
        return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
    }
    if(!hWindow->bClearRectSupport)
    {
        BDBG_ERR(("This window[%d] does not support Mosaic Mode!", hWindow->eId));
        return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
    }

    hWindow->stNewInfo.ulMosaicTrackChannelId = ulChannelId;

    BDBG_LEAVE(BVDC_Window_SetMosaicTrack);
#else
    BDBG_ERR(("This chipset doesn't support Mosaic Mode!"));
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(ulChannelId);
#endif
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetMosaicTrack
    ( BVDC_Window_Handle               hWindow,
      uint32_t                        *pulChannelId )
{
#if BVDC_P_SUPPORT_MOSAIC_MODE
    BDBG_ENTER(BVDC_Window_GetMosaicTrack);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(!hWindow->bClearRectSupport)
    {
        BDBG_ERR(("This window[%d] does not support Mosaic Mode!", hWindow->eId));
        return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
    }

    if(pulChannelId)
    {
        *pulChannelId = hWindow->stCurInfo.ulMosaicTrackChannelId;
    }

    BDBG_LEAVE(BVDC_Window_GetMosaicTrack);
#else
    BDBG_ERR(("This chipset doesn't support Mosaic Mode!"));
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(pulChannelId);
#endif
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetCallbackSettings
    ( BVDC_Window_Handle                  hWindow,
      const BVDC_Window_CallbackSettings *pSettings )
{
    BDBG_ENTER(BVDC_Window_SetCallbackSettings);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(!pSettings)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }


    if ((pSettings->stMask.bCrc || hWindow->stNewInfo.stCbSettings.stMask.bCrc) &&
        (pSettings->stMask.bCrc != hWindow->stNewInfo.stCbSettings.stMask.bCrc ||
         pSettings->eCrcModule != hWindow->stNewInfo.stCbSettings.eCrcModule))
    {
        if (pSettings->stMask.bCrc &&
            (pSettings->eCrcModule >= BVDC_VnetModule_eCap))
        {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        hWindow->stNewInfo.stDirty.stBits.bVnetCrc = BVDC_P_DIRTY;

        /* inform writer_isr to redecide vnet mode */
        hWindow->stNewInfo.stDirty.stBits.bReDetVnet = BVDC_P_DIRTY;
    }

    hWindow->stNewInfo.stCbSettings = *pSettings;
    hWindow->stNewInfo.stDirty.stBits.bMiscCtrl = BVDC_P_DIRTY;

    BDBG_LEAVE(BVDC_Window_SetCallbackSettings);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetCallbackSettings
        ( BVDC_Window_Handle                  hWindow,
          BVDC_Window_CallbackSettings       *pSettings )
{
    BDBG_ENTER(BVDC_Window_GetCallbackSettings);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pSettings)
    {
        *pSettings = hWindow->stCurInfo.stCbSettings;
    }

    BDBG_LEAVE(BVDC_Window_GetCallbackSettings);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_InstallCallback
    ( BVDC_Window_Handle               hWindow,
      const BVDC_CallbackFunc_isr      pfCallback,
      void                            *pvParm1,
      int                              iParm2 )
{
    BDBG_ENTER(BVDC_Window_InstallCallback);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Store the new infos */
    hWindow->stNewInfo.pfGenCallback      = pfCallback;
    hWindow->stNewInfo.pvGenCallbackParm1 = pvParm1;
    hWindow->stNewInfo.iGenCallbackParm2  = iParm2;

    BDBG_LEAVE(BVDC_Window_InstallCallback);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetPixelFormat
    ( BVDC_Window_Handle         hWindow,
      BPXL_Format                ePixelFormat)
{
    BDBG_ENTER(BVDC_Window_SetPixelFormat);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(!BVDC_P_VALID_PIXEL_FORMAT(ePixelFormat))
    {
        BDBG_ERR(("Pixel format %s is not supported by Video feeders",
            BPXL_ConvertFmtToStr(ePixelFormat)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hWindow->stNewInfo.ePixelFormat = ePixelFormat;

    if(ePixelFormat != hWindow->stCurInfo.ePixelFormat)
    {
        hWindow->stNewInfo.stDirty.stBits.bMiscCtrl = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Window_SetPixelFormat);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetUserCaptureBufferCount
    ( BVDC_Window_Handle        hWindow,
      unsigned int              uiCaptureBufferCount )
{
    BDBG_ENTER(BVDC_Window_SetUserCaptureBufferCount);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if (uiCaptureBufferCount > BVDC_P_MAX_USER_CAPTURE_BUFFER_COUNT)
    {
        return(BERR_TRACE(BVDC_ERR_CAPTURE_BUFFERS_MORE_THAN_MAX));
    }

    hWindow->stNewInfo.uiCaptureBufCnt = uiCaptureBufferCount;

    /* set dirty bit */
    hWindow->stNewInfo.stDirty.stBits.bUserCaptureBuffer = BVDC_P_DIRTY;

    BDBG_LEAVE(BVDC_Window_SetUserCaptureBufferCount);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetBuffer
    ( BVDC_Window_Handle         hWindow,
      BVDC_Window_CapturedImage *pCapturedImage )
{
    BERR_Code eRet = BERR_SUCCESS;
    BVDC_P_Window_CapturedPicture stCaptPic;

    BDBG_ENTER(BVDC_Window_GetBuffer);
    BDBG_ASSERT(pCapturedImage);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    BKNI_EnterCriticalSection();
    eRet = BVDC_P_Window_CapturePicture_isr(hWindow, &stCaptPic);
    BKNI_LeaveCriticalSection();

    if (eRet == BERR_SUCCESS)
    {
        if (stCaptPic.hPicBlock)
        {
            BPXL_Plane_Init(&pCapturedImage->captureBuffer, stCaptPic.ulWidth, stCaptPic.ulHeight, stCaptPic.ePxlFmt);
            pCapturedImage->captureBuffer.ulPitch = stCaptPic.ulPitch;
            pCapturedImage->captureBuffer.hPixels = stCaptPic.hPicBlock;
            pCapturedImage->captureBuffer.ulPixelsOffset = stCaptPic.ulPicBlockOffset;
            pCapturedImage->eCapturePolarity = stCaptPic.ePolarity;
            /* Store surface in picture node */
            stCaptPic.pPicture->pSurface = &pCapturedImage->captureBuffer;

            if(stCaptPic.hPicBlock_R)
            {
                BPXL_Plane_Init(&pCapturedImage->rCaptureBuffer, stCaptPic.ulWidth, stCaptPic.ulHeight, stCaptPic.ePxlFmt);
                pCapturedImage->rCaptureBuffer.ulPitch = stCaptPic.ulPitch;
                pCapturedImage->rCaptureBuffer.hPixels = stCaptPic.hPicBlock_R;
                pCapturedImage->rCaptureBuffer.ulPixelsOffset = stCaptPic.ulPicBlockOffset_R;

                /* Store right surface in picture node */
                stCaptPic.pPicture->pSurface_R = &pCapturedImage->rCaptureBuffer;
                pCapturedImage->eOutOrientation = stCaptPic.eDispOrientation;
            }
            else
            {
                pCapturedImage->rCaptureBuffer.hPixels = NULL;
            }
        }
        else
        {
            pCapturedImage->captureBuffer.hPixels = NULL;
            pCapturedImage->eCapturePolarity = BAVC_Polarity_eTopField;
            BDBG_ERR(("No captured picture"));
            return BERR_TRACE(eRet);
        }
    }
    else
    {
        pCapturedImage->captureBuffer.hPixels = NULL;
        pCapturedImage->rCaptureBuffer.hPixels = NULL;
    }

    BDBG_LEAVE(BVDC_Window_GetBuffer);
    return BERR_TRACE(eRet);
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_ReturnBuffer
    ( BVDC_Window_Handle               hWindow,
      BVDC_Window_CapturedImage       *pCapturedImage )
{
    BERR_Code eRet = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_ReturnBuffer);
    BDBG_ASSERT(pCapturedImage);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    BKNI_EnterCriticalSection();
    eRet = BVDC_P_Window_ReleasePicture_isr(hWindow, &pCapturedImage->captureBuffer);
    BKNI_LeaveCriticalSection();

    if (eRet != BERR_SUCCESS)
        return BERR_TRACE(eRet);
    else
    {
        BDBG_MSG(("returning buffer"));
        /* Destroy surface */
        pCapturedImage->captureBuffer.hPixels = NULL;
    }

    if(pCapturedImage->rCaptureBuffer.hPixels)
    {
        BKNI_EnterCriticalSection();
        eRet = BVDC_P_Window_ReleasePicture_isr(hWindow, &pCapturedImage->rCaptureBuffer);
        BKNI_LeaveCriticalSection();

        if (eRet != BERR_SUCCESS)
            return BERR_TRACE(eRet);
        else
        {
            BDBG_MSG(("returning right"));
            /* Destroy surface */
            pCapturedImage->rCaptureBuffer.hPixels = NULL;
        }
    }

    BDBG_LEAVE(BVDC_Window_ReturnBuffer);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetColorKeyConfiguration
    ( BVDC_Window_Handle               hWindow,
      const BVDC_ColorKey_Settings    *pColorKeySettings )
{
    BERR_Code             err = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_SetColorKeyConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(pColorKeySettings);

    if( BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId) )
    {
#if  defined(BCHP_CMP_0_V0_RECT_COLOR) && \
    !defined(BCHP_CMP_0_V0_RECT_TOP_CTRL_RECT_KEY_VALUE_MASK)
        if(hWindow->stNewInfo.bMosaicMode)
        {
            err = BERR_TRACE(BERR_NOT_SUPPORTED);
            BDBG_ERR(("Color key not supported for window[%d] in mosaic mode",
                hWindow->eId));
        }
        else
#endif
        {
            /* set new values */
            hWindow->stNewInfo.stColorKey = *pColorKeySettings;
        }
    }
    else
    {
        err = BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ERR(("Color key not supported for window[%d]", hWindow->eId));
    }

    BDBG_LEAVE(BVDC_Window_SetColorKeyConfiguration);
    return err;
}

#if !B_REFSW_MINIMAL
BERR_Code BVDC_Window_GetColorKeyConfiguration
    ( BVDC_Window_Handle               hWindow,
      BVDC_ColorKey_Settings          *pColorKeySettings )
{
    BDBG_ENTER(BVDC_Window_GetColorKeyConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pColorKeySettings)
    {
        *pColorKeySettings = hWindow->stNewInfo.stColorKey;
    }

    BDBG_LEAVE(BVDC_Window_GetColorKeyConfiguration);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetColorMatrix
    ( BVDC_Window_Handle               hWindow,
      bool                             bOverride,
      const int32_t                    pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                         ulShift )
{
    BDBG_ENTER(BVDC_Window_SetColorMatrix);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if (bOverride)
    {
        BVDC_P_Window_SetColorMatrix(hWindow, pl32_Matrix, hWindow->stNewInfo.pl32_Matrix);
    }

    hWindow->stNewInfo.bUserCsc = bOverride;
    hWindow->stNewInfo.ulUserShift = ulShift;

    /* set dirty bit */
    hWindow->stNewInfo.stDirty.stBits.bCscAdjust = BVDC_P_DIRTY;

    BDBG_LEAVE(BVDC_Window_SetColorMatrix);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetColorMatrix
    ( BVDC_Window_Handle               hWindow,
      bool                            *pbOverride,
      int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                        *pulShift )
{
    BDBG_ENTER(BVDC_Window_GetColorMatrix);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    BVDC_P_Window_GetColorMatrix(hWindow, hWindow->stNewInfo.bUserCsc,
                                 hWindow->stCurInfo.pl32_Matrix, pl32_Matrix);

    *pbOverride = hWindow->stNewInfo.bUserCsc;
    *pulShift =  (*pbOverride)? hWindow->stNewInfo.ulUserShift : BVDC_P_FIX_POINT_SHIFT;

    BDBG_LEAVE(BVDC_Window_GetColorMatrix);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_LoadCabTable
    ( BVDC_Window_Handle               hWindow,
      const uint32_t                  *pulCabTable,
      uint32_t                         ulOffset,
      uint32_t                         ulSize )
{
    BDBG_ENTER(BVDC_Window_LoadCabTable);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }

    /* Check for valid offset and size */
    if((pulCabTable != NULL) && ((ulOffset + ulSize) > BVDC_P_CAB_TABLE_SIZE))
    {
        return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
    }

    if(pulCabTable != NULL)
    {
        uint32_t id;

        /* set new value */
        hWindow->stNewInfo.bUserCabEnable = true;

        /* copy pulCabTable to internal table */
        for(id = 0; id < ulSize; id++)
        {
            *(hWindow->stNewInfo.pulCabTable + (id + ulOffset)) =
                *(pulCabTable + id);
            BDBG_MSG(("User CabTable[%d]= 0x%x", id + ulOffset, *(hWindow->stNewInfo.pulCabTable + (id + ulOffset))));
        }
    }
    else
    {
        hWindow->stNewInfo.bUserCabEnable = false;
    }

    BDBG_LEAVE(BVDC_Window_LoadCabTable);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_LoadLabTableCustomized
    ( BVDC_Window_Handle               hWindow,
      const uint32_t                  *pulLumaTable,
      const uint32_t                  *pulCbTable,
      const uint32_t                  *pulCrTable,
      uint32_t                         ulOffset,
      uint32_t                         ulSize )
{
    uint32_t id;

    BDBG_ENTER(BVDC_Window_LoadLabTableCustomized);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }

    /* Check for valid offset and size */
    if((pulLumaTable != NULL || pulCbTable != NULL || pulCrTable != NULL) &&
       ((ulOffset + ulSize) > BVDC_P_LAB_TABLE_SIZE))
    {
        return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
    }

    /* Check if user provides both Cb and Cr tables or not */
    if((pulCbTable != NULL && pulCrTable == NULL) ||
       (pulCbTable == NULL && pulCrTable != NULL))
    {
        BDBG_ERR(("Need to provide both Cb and Cr tables"));
        return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
    }

    hWindow->stNewInfo.bUserLabLuma = (pulLumaTable != NULL) ? true : false;
    hWindow->stNewInfo.bUserLabCbCr = (pulCbTable != NULL) ? true : false;

    /* copy to internal tables */
    for(id = 0; id < ulSize; id++)
    {
        if(pulLumaTable != NULL)
        {
            *(hWindow->stNewInfo.pulLabLumaTbl + (id + ulOffset)) =
                *(pulLumaTable + id);
            BDBG_MSG(("User LumTable[%d]= 0x%x", id + ulOffset, *(hWindow->stNewInfo.pulLabLumaTbl + (id + ulOffset))));
        }
        if(pulCbTable != NULL && pulCrTable != NULL)
        {
            *(hWindow->stNewInfo.pulLabCbTbl + (id + ulOffset)) =
                *(pulCbTable + id);
            *(hWindow->stNewInfo.pulLabCrTbl + (id + ulOffset)) =
                *(pulCrTable + id);
            BDBG_MSG(("User CbTable[%d]= 0x%x, CrTable[%d] = 0x%x",
                id + ulOffset, *(hWindow->stNewInfo.pulLabCbTbl + (id + ulOffset)),
                id + ulOffset, *(hWindow->stNewInfo.pulLabCrTbl + (id + ulOffset))));
        }
    }

    BDBG_LEAVE(BVDC_Window_LoadLabTableCustomized);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_LoadLabTable
    ( BVDC_Window_Handle               hWindow,
      const uint32_t                  *pulLabTable,
      uint32_t                         ulOffset,
      uint32_t                         ulSize )
{
    BDBG_ENTER(BVDC_Window_LoadLabTable);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

#if (BVDC_P_SUPPORT_PEP)
    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }

    /* Check for valid offset and size */
    if((pulLabTable != NULL) && ((ulOffset + ulSize) > BVDC_P_LAB_TABLE_SIZE))
    {
        return BERR_TRACE(BVDC_ERR_PEP_INVALID_PARAMETER);
    }

    if(pulLabTable != NULL)
    {
        uint32_t id;
        uint32_t *pulLumaTable;
        uint32_t *pulCbTable;
        uint32_t *pulCrTable;

        pulLumaTable = (uint32_t *)(BKNI_Malloc(BVDC_P_LAB_TABLE_SIZE * sizeof(uint32_t)));
        pulCbTable = (uint32_t *)(BKNI_Malloc(BVDC_P_LAB_TABLE_SIZE * sizeof(uint32_t)));
        pulCrTable = (uint32_t *)(BKNI_Malloc(BVDC_P_LAB_TABLE_SIZE * sizeof(uint32_t)));

        /* break out the data into luma, Cb and Cr tables */
        for(id = 0; id < ulSize; id++)
        {
#if (BVDC_P_SUPPORT_PEP_VER_5>BVDC_P_SUPPORT_PEP_VER)
            *(pulLumaTable + (id + ulOffset)) =
                BCHP_GET_FIELD_DATA(*(pulLabTable + id), PEP_CMP_0_V0_LAB_LUT_DATA_i, LUMA_DATA);
            *(pulCbTable + (id + ulOffset)) =
                BCHP_GET_FIELD_DATA(*(pulLabTable + id), PEP_CMP_0_V0_LAB_LUT_DATA_i, CB_OFFSET);
            *(pulCrTable + (id + ulOffset)) =
                BCHP_GET_FIELD_DATA(*(pulLabTable + id), PEP_CMP_0_V0_LAB_LUT_DATA_i, CR_OFFSET);
            BDBG_MSG(("User Luma[%d]= 0x%x, Cb = 0x%x, Cr = 0x%x",
                id + ulOffset, pulLumaTable[id+ulOffset],
                pulCbTable[id+ulOffset], pulCrTable[id+ulOffset]));
#endif
        }
        /* Calling the new function to load Lab table */
        BVDC_Window_LoadLabTableCustomized(hWindow, pulLumaTable, pulCbTable, pulCrTable, ulOffset, ulSize);

        BKNI_Free((uint32_t *)pulLumaTable);
        BKNI_Free((uint32_t *)pulCbTable);
        BKNI_Free((uint32_t *)pulCrTable);
    }
    else
    {
        BVDC_Window_LoadLabTableCustomized(hWindow, NULL, NULL, NULL, ulOffset, ulSize);
    }
#else
    BSTD_UNUSED(pulLabTable);
    BSTD_UNUSED(ulOffset);
    BSTD_UNUSED(ulSize);
#endif

    BDBG_LEAVE(BVDC_Window_LoadLabTable);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetCoefficientIndex
    ( BVDC_Window_Handle               hWindow,
      const BVDC_CoefficientIndex     *pCtIndex )
{
    BDBG_ENTER(BVDC_Window_SetCoefficientIndex);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pCtIndex)
    {
        hWindow->stNewInfo.stCtIndex = *pCtIndex;

        /* Flag change */
        if((pCtIndex->ulSclHorzLuma    != hWindow->stCurInfo.stCtIndex.ulSclHorzLuma   ) ||
           (pCtIndex->ulSclVertLuma    != hWindow->stCurInfo.stCtIndex.ulSclVertLuma   ) ||
           (!BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId) &&
            ((pCtIndex->ulSclHorzChroma  != hWindow->stCurInfo.stCtIndex.ulSclHorzChroma ) ||
             (pCtIndex->ulSclVertChroma  != hWindow->stCurInfo.stCtIndex.ulSclVertChroma ) ||
             (pCtIndex->ulMadHorzLuma    != hWindow->stCurInfo.stCtIndex.ulMadHorzLuma   ) ||
             (pCtIndex->ulMadHorzChroma  != hWindow->stCurInfo.stCtIndex.ulMadHorzChroma ) ||
             (pCtIndex->ulHsclHorzLuma   != hWindow->stCurInfo.stCtIndex.ulHsclHorzLuma  ) ||
             (pCtIndex->ulHsclHorzChroma != hWindow->stCurInfo.stCtIndex.ulHsclHorzChroma))))
        {
            hWindow->stNewInfo.stDirty.stBits.bCtIndex = BVDC_P_DIRTY;
        }
    }

    BDBG_LEAVE(BVDC_Window_SetCoefficientIndex);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetGameModeDelay
    ( BVDC_Window_Handle               hWindow,
      const BVDC_Window_GameModeSettings    *pstGameModeDelay )
{
    #define BVDC_P_GAME_MODE_MIN_BUF_DELAY     (1000) /* in usecs */

    BDBG_ENTER(BVDC_Window_SetGameModeDelay);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(!pstGameModeDelay)
    {
        hWindow->stNewInfo.stGameDelaySetting.bEnable = false;
    }
    else
    {
        if(pstGameModeDelay->bEnable)
        {
            if(pstGameModeDelay->ulBufferDelayTarget < pstGameModeDelay->ulBufferDelayTolerance)
            {
                BDBG_ERR(("Game mode delay target[%d] < tolerance[%d]!",
                    pstGameModeDelay->ulBufferDelayTarget, pstGameModeDelay->ulBufferDelayTolerance));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            if(pstGameModeDelay->ulBufferDelayTarget < BVDC_P_GAME_MODE_MIN_BUF_DELAY)
            {
                BDBG_ERR(("Game mode delay target[%d] < %d usecs, too small!",
                    pstGameModeDelay->ulBufferDelayTarget, BVDC_P_GAME_MODE_MIN_BUF_DELAY));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            if(pstGameModeDelay->ulBufferDelayTarget >=
                (hWindow->hBuffer->ulActiveBufCnt) * 1000000 * BFMT_FREQ_FACTOR / hWindow->stCurInfo.hSource->ulVertFreq)
            {
                BDBG_ERR(("Game mode delay target[%d usec] > %d fields, too large!",
                    pstGameModeDelay->ulBufferDelayTarget,
                    hWindow->hBuffer->ulActiveBufCnt-1));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }
        hWindow->stNewInfo.stGameDelaySetting = *pstGameModeDelay;
    }

    BDBG_LEAVE(BVDC_Window_SetGameModeDelay);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetGameModeDelay
    ( BVDC_Window_Handle               hWindow,
      BVDC_Window_GameModeSettings    *pstGameModeDelay )
{
    BDBG_ENTER(BVDC_Window_GetGameModeDelay);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(!pstGameModeDelay)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    *pstGameModeDelay = hWindow->stCurInfo.stGameDelaySetting;

    BDBG_LEAVE(BVDC_Window_GetGameModeDelay);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetDitherConfiguration
    ( BVDC_Window_Handle               hWindow,
      const BVDC_DitherSettings       *pDither )
{
    BDBG_ENTER(BVDC_Window_SetDitherConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }

    /* set new values */
    hWindow->stNewInfo.stDither = *pDither;

    /* set dirty bit */
    if((pDither->bReduceSmooth != hWindow->stCurInfo.stDither.bReduceSmooth) ||
       (pDither->bSmoothEnable != hWindow->stCurInfo.stDither.bSmoothEnable) ||
       (pDither->ulSmoothLimit != hWindow->stCurInfo.stDither.ulSmoothLimit))
    {
        hWindow->stNewInfo.stDirty.stBits.bDitAdjust = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Window_SetDitherConfiguration);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetDitherConfiguration
    ( const BVDC_Window_Handle         hWindow,
      BVDC_DitherSettings             *pDither )
{
    BDBG_ENTER(BVDC_Window_GetDitherConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* only support main display's main window */
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return BERR_TRACE(BVDC_ERR_PEP_WINDOW_NOT_SUPPORT);
    }

    /* set new values */
    if(pDither)
    {
        *pDither = hWindow->stNewInfo.stDither;
    }

    BDBG_LEAVE(BVDC_Window_GetDitherConfiguration);
    return BERR_SUCCESS;

}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetAnrConfiguration
    ( BVDC_Window_Handle               hWindow,
      const BVDC_Anr_Settings         *pAnrSettings )
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_SetAnrConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

#if (BVDC_P_SUPPORT_MANR)
    /* only support main display's main window */
    if(!pAnrSettings )
    {
        eStatus = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        BVDC_P_Window_Info *pNewInfo = &hWindow->stNewInfo;
        const BVDC_P_Window_Info *pCurInfo = &hWindow->stCurInfo;

        if(pAnrSettings->eMode)
        {
            if(!BPXL_IS_YCbCr422_FORMAT(pAnrSettings->ePxlFormat) &&
               !BPXL_IS_YCbCr422_10BIT_FORMAT(pAnrSettings->ePxlFormat) &&
               !BPXL_IS_YCbCr422_10BIT_PACKED_FORMAT(pAnrSettings->ePxlFormat))
            {
                BDBG_ERR(("This chip only supports 8-bit or 10-bit 4:2:2 ANR pixel format!"));
                return BERR_TRACE(BVDC_ERR_MAD_NOT_SUPPORTED);
            }
        }

        /* configuration */
        pNewInfo->bAnr = (BVDC_FilterMode_eEnable == pAnrSettings->eMode);
        pNewInfo->stAnrSettings = *pAnrSettings;

        /* Dirty bit set */
        if((pNewInfo->stAnrSettings.eMode       != pCurInfo->stAnrSettings.eMode) ||
           (pNewInfo->stAnrSettings.iSnDbAdjust != pCurInfo->stAnrSettings.iSnDbAdjust) ||
           (pNewInfo->stAnrSettings.pvUserInfo  != pCurInfo->stAnrSettings.pvUserInfo) ||
           (pNewInfo->stAnrSettings.ePxlFormat  != pCurInfo->stAnrSettings.ePxlFormat))
        {
            pNewInfo->stDirty.stBits.bAnrAdjust = BVDC_P_DIRTY;
        }
    }
#else
    BSTD_UNUSED(pAnrSettings);
    BDBG_MSG(("ANR is not supported for this chipset"));
#endif

    BDBG_LEAVE(BVDC_Window_SetAnrConfiguration);
    return eStatus;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetAnrConfiguration
    ( BVDC_Window_Handle               hWindow,
      BVDC_Anr_Settings               *pAnrSettings )
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_GetAnrConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

#if (BVDC_P_SUPPORT_MANR)
    if(pAnrSettings)
    {
        *pAnrSettings = hWindow->stCurInfo.stAnrSettings;
    }
    else
    {
        eStatus = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#else
    BSTD_UNUSED(pAnrSettings);
    BDBG_MSG(("ANR is not supported for this chipset"));
#endif
    BDBG_LEAVE(BVDC_Window_GetAnrConfiguration);
    return eStatus;
}


/***************************************************************************
 *
 */

BERR_Code BVDC_Window_SetBandwidthEquationParams
    ( BVDC_Window_Handle               hWindow,
      const uint32_t                   ulDelta,
      const BVDC_SclCapBias            eSclCapBias)
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_SetBandwidthEquationParams);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hWindow->stNewInfo.eSclCapBias = eSclCapBias;
    hWindow->stNewInfo.ulBandwidthDelta = ulDelta;

    BDBG_LEAVE(BVDC_Window_SetBandwidthEquationParams);
    return eStatus;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */

BERR_Code BVDC_Window_GetBandwidthEquationParams
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         *pulDelta,
      BVDC_SclCapBias                  *peSclCapBias)
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_GetBandwidthEquationParams);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    *pulDelta = hWindow->stCurInfo.ulBandwidthDelta;
    *peSclCapBias = hWindow->stCurInfo.eSclCapBias;

    BDBG_LEAVE(BVDC_Window_GetBandwidthEquationParams);
    return eStatus;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetScalerConfiguration
    ( BVDC_Window_Handle               hWindow,
      const BVDC_Scaler_Settings      *pScalerSettings )
{
    BDBG_ENTER(BVDC_Window_SetScalerConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(pScalerSettings)
    {
        const BVDC_Scaler_Settings *pCurSclSettings = &hWindow->stCurInfo.stSclSettings;
        hWindow->stNewInfo.stSclSettings = *pScalerSettings;

        /* Flag change */
        if((pScalerSettings->bSclVertDejagging       != pCurSclSettings->bSclVertDejagging      ) ||
           (pScalerSettings->bSclHorzLumaDeringing   != pCurSclSettings->bSclHorzLumaDeringing  ) ||
           (pScalerSettings->bSclVertLumaDeringing   != pCurSclSettings->bSclVertLumaDeringing  ) ||
           (pScalerSettings->bSclHorzChromaDeringing != pCurSclSettings->bSclHorzChromaDeringing) ||
           (pScalerSettings->bSclVertChromaDeringing != pCurSclSettings->bSclVertChromaDeringing) ||
           (pScalerSettings->bSclVertPhaseIgnore     != pCurSclSettings->bSclVertPhaseIgnore    ) ||
           (pScalerSettings->ulSclDejaggingCore      != pCurSclSettings->ulSclDejaggingCore     ) ||
           (pScalerSettings->ulSclDejaggingGain      != pCurSclSettings->ulSclDejaggingGain     ) ||
           (pScalerSettings->ulSclDejaggingHorz      != pCurSclSettings->ulSclDejaggingHorz     ))
        {
            hWindow->stNewInfo.stDirty.stBits.bMiscCtrl = BVDC_P_DIRTY;
        }
    }

    BDBG_LEAVE(BVDC_Window_SetScalerConfiguration);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetScalerConfiguration
    ( BVDC_Window_Handle               hWindow,
      BVDC_Scaler_Settings            *pScalerSettings )
{
    BDBG_ENTER(BVDC_Window_GetScalerConfiguration);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    if(pScalerSettings)
    {
        *pScalerSettings = hWindow->stCurInfo.stSclSettings;
    }

    BDBG_LEAVE(BVDC_Window_GetScalerConfiguration);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetStatus
    ( const BVDC_Window_Handle         hWindow,
      BVDC_Window_Status              *pWindowStatus )
{
    BDBG_ENTER(BVDC_Window_GetStatus);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    if(pWindowStatus)
    {
        pWindowStatus->bSyncLock = hWindow->bSyncLockSrc;
    }

    BDBG_LEAVE(BVDC_Window_GetStatus);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_SetDstRightRect
    ( const BVDC_Window_Handle         hWindow,
      int32_t                          lRWinXOffsetDelta)
{
    BDBG_ENTER(BVDC_Window_SetDstRightRect);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    hWindow->stNewInfo.lRWinXOffsetDelta  = lRWinXOffsetDelta;
    hWindow->stNewInfo.stDirty.stBits.b3D = BVDC_P_DIRTY;

    BDBG_LEAVE(BVDC_Window_SetDstRightRect);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetDstRightRect
    ( const BVDC_Window_Handle         hWindow,
      int32_t                         *plRWinXOffsetDelta )
{
    BDBG_ENTER(BVDC_Window_GetDstRightRect);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* set new value */
    if(plRWinXOffsetDelta)
    {
        *plRWinXOffsetDelta = hWindow->stCurInfo.lRWinXOffsetDelta;
    }

    BDBG_LEAVE(BVDC_Window_GetDstRightRect);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_GetCapabilities
    ( BVDC_Window_Handle               hWindow,
      BVDC_Window_Capabilities        *pCapabilities )
{
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if (pCapabilities)
    {
        BVDC_Compositor_Handle hCompositor = hWindow->hCompositor;
        int iWinInCmp = hWindow->eId - BVDC_P_CMP_GET_V0ID(hCompositor);
        BVDC_P_Cfc_Capability stCapability;

        stCapability.ulInts = hCompositor->stCfcCapability[iWinInCmp].ulInts;
        stCapability.stBits.bBlackBoxNLConv = stCapability.stBits.bNL2L && !stCapability.stBits.bMb;

        pCapabilities->bConvColorimetry = stCapability.stBits.bBlackBoxNLConv || stCapability.stBits.bMb;
        pCapabilities->bConvHdr10 = stCapability.stBits.bLRngAdj || stCapability.stBits.bLMR;
        pCapabilities->bConvHlg = stCapability.stBits.bLMR;
        pCapabilities->bTchInput = stCapability.stBits.bTpToneMapping;
        pCapabilities->bDolby = stCapability.stBits.bDbvToneMapping || stCapability.stBits.bDbvCmp;
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
void BVDC_Window_GetCores
    ( BVDC_WindowId                    eWinId,
      BBOX_Handle                      hBox,
      BVDC_Source_Handle               hSrc,
      BVDC_Compositor_Handle           hCmp,
      BAVC_CoreList                   *pCoreList
    )
{
    uint32_t ulBoxWinId;
    BVDC_P_WindowId eWindowId;
    const BBOX_Vdc_Capabilities *pBoxVdcCap;
    const BBOX_Vdc_Display_Capabilities *pBoxVdcDispCap;

    BSTD_UNUSED(hBox);
    BDBG_ASSERT(eWinId <= BVDC_WindowId_eAuto);
    BDBG_OBJECT_ASSERT(hSrc, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hCmp, BVDC_CMP);

    BKNI_Memset(pCoreList, 0, sizeof(*pCoreList));

    pBoxVdcCap     = &hCmp->hVdc->stBoxConfig.stVdc;
    pBoxVdcDispCap = &pBoxVdcCap->astDisplay[hCmp->eId];

    if(BERR_SUCCESS == BVDC_P_Window_GetPrivHandle(hCmp, eWinId, hSrc->eId, NULL, &eWindowId))
    {
        ulBoxWinId = BVDC_P_GetBoxWindowId_isrsafe(eWindowId);
    }
    else
    {
        BDBG_ERR(("win[%d] is not available with src[%d] and cmp[%d].",
            eWinId, hSrc->eId, hCmp->eId));
        return;
    }

    /* Get window cores */
    if (pBoxVdcDispCap->bAvailable && pBoxVdcDispCap->astWindow[ulBoxWinId].bAvailable)
    {
        const BVDC_P_ResourceFeature *pResourceFeature;
        const BVDC_P_ResourceRequire *pResourceRequire;
        const BBOX_Vdc_ResourceFeature *pBoxVdcResource;

        /* Get VDC private video window ID */
        if (ulBoxWinId <= BVDC_WindowId_eVideo1)
        {
            /* Also add compositor ID */
            pCoreList->aeCores[BAVC_CoreId_eCMP_0 + hCmp->eId] = true;

            pResourceFeature = BVDC_P_Window_GetResourceFeature_isrsafe(eWindowId);
            pResourceRequire = BVDC_P_Window_GetResourceRequire_isrsafe(eWindowId);
            pBoxVdcResource = &pBoxVdcDispCap->astWindow[ulBoxWinId].stResource;

            /* Get deinterlacer */
            if (pBoxVdcResource->ulMad != BBOX_FTR_INVALID)
            {
                uint32_t ulResId;
                BVDC_P_ResourceType eType = BVDC_P_ResourceType_eInvalid;

                uint32_t ulMad = (pBoxVdcResource->ulMad == BBOX_VDC_DISREGARD)
                    ? pResourceFeature->ulMad : pBoxVdcResource->ulMad;

                eType = BVDC_P_ResourceType_eMcvp;
                BVDC_P_Resource_GetResourceId(eType, ulMad, &ulResId);

                if (ulResId != BVDC_P_HW_ID_INVALID)
                {
#if (BVDC_P_SUPPORT_MCVP > BVDC_P_SUPPORT_MADR)
                    /* Has MCDI */
                    pCoreList->aeCores[BAVC_CoreId_eMVP_0 + ulResId] = true;
#else
                    /* MADR only */
                    pCoreList->aeCores[BAVC_CoreId_eMAD_0 + ulResId] = true;
#endif
                }
            }

            /* Get CAP, VFD, SCL */
            if (pResourceRequire->bRequireCapture)
            {
                BVDC_P_CaptureId eCapture;

                BDBG_ASSERT(pBoxVdcResource->eCap != BBOX_Vdc_Resource_Capture_eUnknown);

                eCapture = (pBoxVdcResource->eCap == BBOX_VDC_DISREGARD) ?
                    pResourceRequire->eCapture : (BVDC_P_CaptureId)pBoxVdcResource->eCap;

                pCoreList->aeCores[BAVC_CoreId_eCAP_0 + (eCapture - BVDC_P_CaptureId_eCap0)] = true;
            }

            if (pResourceRequire->bRequirePlayback)
            {
                BVDC_P_FeederId ePlayback;

                BDBG_ASSERT(pBoxVdcResource->eVfd != BBOX_Vdc_Resource_Feeder_eUnknown);

                ePlayback = (pBoxVdcResource->eVfd == BBOX_VDC_DISREGARD) ?
                    pResourceRequire->ePlayback : (BVDC_P_FeederId)pBoxVdcResource->eVfd;

                pCoreList->aeCores[BAVC_CoreId_eVFD_0 + (ePlayback - BVDC_P_FeederId_eVfd0)] = true;
            }

            if (pResourceRequire->bRequireScaler)
            {
                BVDC_P_ScalerId eScaler;

                BDBG_ASSERT(pBoxVdcResource->eScl != BBOX_Vdc_Resource_Scaler_eUnknown);

                eScaler = (pBoxVdcResource->eScl == BBOX_VDC_DISREGARD) ?
                    pResourceRequire->eScaler : (BVDC_P_ScalerId)pBoxVdcResource->eScl;

                pCoreList->aeCores[BAVC_CoreId_eSCL_0 + (eScaler - BVDC_P_ScalerId_eScl0)] = true;
            }
            else
            {
                uint32_t ulResId = BVDC_P_HW_ID_INVALID;
                BVDC_P_Resource_GetResourceId(BVDC_P_ResourceType_eScl, pResourceFeature->ulScl, &ulResId);
                if (ulResId != BVDC_P_HW_ID_INVALID)
                {
                    BVDC_P_ScalerId eScaler;

                    BDBG_ASSERT(pBoxVdcResource->eScl != BBOX_Vdc_Resource_Scaler_eUnknown);

                    eScaler = (pBoxVdcResource->eScl == BBOX_VDC_DISREGARD) ?
                        ulResId : (BVDC_P_ScalerId)pBoxVdcResource->eScl;

                    pCoreList->aeCores[BAVC_CoreId_eSCL_0 + (eScaler - BVDC_P_ScalerId_eScl0)] = true;
                }
            }
        }
        else /* Graphics window */
        {
            BDBG_ASSERT(BVDC_P_SRC_IS_GFX(hSrc->eId));
            pCoreList->aeCores[hCmp->eId + BAVC_CoreId_eGFD_0] = true;
        }
    }
    else
    {
        BDBG_ERR(("cmp[%d]win[%d]  is not available for boxmode[%d].",
           hCmp->eId, ulBoxWinId, hCmp->hVdc->stBoxConfig.stBox.ulBoxId));
        return;
    }

    /* Get source core */
    if (pBoxVdcCap->astSource[hSrc->eId].bAvailable)
    {
        if (BVDC_P_SRC_IS_MPEG(hSrc->eId))
        {
            pCoreList->aeCores[hSrc->eId + BAVC_CoreId_eMFD_0] = true;
        }
    }
    else
    {
        BDBG_ERR(("src[%d] is not available for boxmode[%d].",
            hSrc->eId, hCmp->hVdc->stBoxConfig.stBox.ulBoxId));
        return;
    }

    return;
}

/* End of File */
