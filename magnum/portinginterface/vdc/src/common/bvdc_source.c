/***************************************************************************
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
 *
 * Module Description:
 *
 ***************************************************************************/
#include "bstd.h"              /* standard types */
#include "bkni.h"              /* memcpy calls */
#include "bvdc.h"              /* Video display */
#include "bvdc_common_priv.h"
#include "bvdc_priv.h"
#include "bvdc_dnr_priv.h"
#include "bvdc_feeder_priv.h"
#include "bvdc_capture_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_hddvi_priv.h"
#include "bchp_int_id_bvnf_intr2_0.h"

BDBG_MODULE(BVDC_SRC);
BDBG_FILE_MODULE(BVDC_MEMC_INDEX_CHECK);
BDBG_FILE_MODULE(BVDC_WIN_BUF);
BDBG_FILE_MODULE(BVDC_SRC_DELTA);



/* How to map mpeg format to BFMT format. */
static const BFMT_VideoFmt s_aeMpegToFmt[] =
{
    BFMT_VideoFmt_eNTSC,
    BFMT_VideoFmt_ePAL_G,
    BFMT_VideoFmt_e480p,
    BFMT_VideoFmt_e576p_50Hz,
    BFMT_VideoFmt_e720p,
    BFMT_VideoFmt_e720p_50Hz,
    BFMT_VideoFmt_e1080i,
    BFMT_VideoFmt_e1080i_50Hz,
    BFMT_VideoFmt_e1080p,
    BFMT_VideoFmt_e1080p_24Hz,
    BFMT_VideoFmt_e1080p_25Hz,
    BFMT_VideoFmt_e1080p_30Hz,
    BFMT_VideoFmt_e1080p_50Hz,
    BFMT_VideoFmt_ePAL_60,
    BFMT_VideoFmt_e720p_24Hz,
    BFMT_VideoFmt_e3840x2160p_24Hz,
    BFMT_VideoFmt_e3840x2160p_25Hz,
    BFMT_VideoFmt_e3840x2160p_30Hz,
    BFMT_VideoFmt_e3840x2160p_50Hz,
    BFMT_VideoFmt_e3840x2160p_60Hz,

    /* */
    BFMT_VideoFmt_e1250i_50Hz,
    BFMT_VideoFmt_e240p_60Hz,
    BFMT_VideoFmt_e288p_50Hz
};

static const uint32_t s_aulFrmRate[] =
{
    0,     /*BAVC_FrameRateCode_eUnknown = 0, */
    2397,  /*BAVC_FrameRateCode_e23_976 = 1,  */
    2400,  /*BAVC_FrameRateCode_e24        ,  */
    2500,  /*BAVC_FrameRateCode_e25        ,  */
    2997,  /*BAVC_FrameRateCode_e29_97     ,  */
    3000,  /*BAVC_FrameRateCode_e30        ,  */
    5000,  /*BAVC_FrameRateCode_e50        ,  */
    5994,  /*BAVC_FrameRateCode_e59_94     ,  */
    6000,   /*BAVC_FrameRateCode_e60          */
    1498,  /*BAVC_FrameRateCode_e14_985    ,*/
    749,   /*BVC_FrameRateCode_e7_493      ,*/
    1000,  /*BAVC_FrameRateCode_e10        ,*/
    1500,  /*BAVC_FrameRateCode_e15        ,*/
    2000,  /*BAVC_FrameRateCode_e20        ,*/
    1250,  /*BAVC_FrameRateCode_e12_5      ,*/
    10000,  /*BAVC_FrameRateCode_e100        ,*/
    11988,  /*BAVC_FrameRateCode_e119_88     ,*/
    12000,  /*BAVC_FrameRateCode_e120        ,*/
    1998,  /*BAVC_FrameRateCode_e19_98     ,*/
    750, /* BAVC_FrameRateCode_e7_5 */
    1200, /* BAVC_FrameRateCode_e12 */
    1198, /* BAVC_FrameRateCode_e11_988 */
     999, /* BAVC_FrameRateCode_e9_99 */
};

#define BVDC_P_MPEG_FMT_DELTA                 (16) /* detal for matching */
#define BVDC_P_MPEG_FMT_COUNT                 (sizeof(s_aeMpegToFmt)/sizeof(BFMT_VideoFmt))
#define BVDC_P_MAX_FRM_RATE_CODE              (sizeof(s_aulFrmRate)/sizeof(uint32_t) -1)

/* PsF: 30Hz is the max PsF scanout frequency (x100 to follow FMT style); */
#define BVDC_P_PSF_VERT_FREQ                     (30 * BFMT_FREQ_FACTOR)

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetDefaultSettings
    ( BAVC_SourceId                    eSourceId,
      BVDC_Source_Settings            *pDefSettings )
{
    BSTD_UNUSED(eSourceId);

    /* To make sure thing get initialize */
    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));

    /* TODO: double check the default settings */
    pDefSettings->hHeap        = NULL;
    pDefSettings->eDsSrcCompId = BVDC_CompositorId_eCompositor0;

    if (BVDC_P_SRC_IS_GFX(eSourceId) || BVDC_P_SRC_IS_VFD(eSourceId))
    {
        pDefSettings->bGfxSrc = true;
    }

    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_QueryVfd
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_WindowId                    eWindowId,
      BAVC_SourceId                   *peSourceId )
{
    BVDC_Window_Handle  hWindow;
    BERR_Code  eRet;

    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BDBG_ASSERT(NULL!=peSourceId);

    eRet = BVDC_P_Window_GetPrivHandle(
        hCompositor, eWindowId, BAVC_SourceId_eVfd0, &hWindow, NULL);
    if (BERR_SUCCESS != eRet)
    {
        return BERR_TRACE(eRet);
    }

    *peSourceId = BAVC_SourceId_eVfd0 +
        (hWindow->stResourceRequire.ePlayback - BVDC_P_FeederId_eVfd0);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_Create
    ( BVDC_Handle                      hVdc,
      BVDC_Source_Handle              *phSource,
      BAVC_SourceId                    eSourceId,
      const BVDC_Source_Settings      *pDefSettings )
{
    BVDC_P_SourceContext *pSource;
    BVDC_P_DrainContext stTmpDrain;

    BDBG_ENTER(BVDC_Source_Create);
    BDBG_ASSERT(phSource);
    if (eSourceId >= BAVC_SourceId_eMax) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    /* Making sure features is available. */
    if(!hVdc->pFeatures->abAvailSrc[eSourceId])
    {
        BDBG_ERR(("Source[%d] not supported on this chipset.", eSourceId));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if(hVdc->ahSource[eSourceId]!=NULL)
    {
        /* re-activate the source */
        *phSource = hVdc->ahSource[eSourceId];
    }
    else
    {
        /* source has NOT been created yet. */
        BVDC_P_Source_Create(hVdc, &hVdc->ahSource[eSourceId], (BAVC_SourceId)eSourceId, hVdc->hResource,
                    hVdc->pFeatures->ab3dSrc[eSourceId]);
    }

    BKNI_Memset((void*)&stTmpDrain, 0x0, sizeof(BVDC_P_DrainContext));

#ifndef BVDC_FOR_BOOTUPDATER
    /* User asked to disabled these sources in BVDC_Open */
    if((BVDC_P_SRC_IS_HDDVI(eSourceId) && hVdc->stSettings.bDisableHddviInput) ||
       (BVDC_P_SRC_IS_ITU656(eSourceId) && hVdc->stSettings.bDisable656Input))
    {
        BDBG_ERR(("Source[%d] disabled by BVDC_Settings.", eSourceId));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Find an available drain for the source. */
    if(BVDC_P_SRC_NEED_DRAIN(eSourceId))
    {
        if(BVDC_P_Drain_Acquire(&stTmpDrain, hVdc->hResource, eSourceId) != BERR_SUCCESS)
        {
            BDBG_ERR(("No Vnet drain available for Source[%d]", eSourceId));
            BDBG_ERR(("Source[%d] not supported on this chipset.", eSourceId));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

    pSource = hVdc->ahSource[eSourceId];
    BDBG_OBJECT_ASSERT(pSource, BVDC_SRC);
    pSource->stDrain = stTmpDrain;

    /* Check if source is created or not. */
    if(BVDC_P_STATE_IS_ACTIVE(pSource) ||
       BVDC_P_STATE_IS_CREATE(pSource) ||
       BVDC_P_STATE_IS_DESTROY(pSource))
    {
        return BERR_TRACE(BVDC_ERR_SOURCE_ALREADY_CREATED);
    }

#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_RX_CLK0
    if(BVDC_P_SRC_IS_HDDVI(eSourceId))
    {
        pSource->ulHdmiPwrId = BCHP_PWR_RESOURCE_VDC_HDMI_RX_CLK0;

        if(!pSource->ulHdmiPwrAcquire)
        {
            BDBG_MSG(("SRC[%d] Acquires BCHP_PWR_RESOURCE_VDC_HDMI_RX_CLK = 0x%08x",
                pSource->eId, pSource->ulHdmiPwrId));
            BCHP_PWR_AcquireResource(pSource->hVdc->hChip, pSource->ulHdmiPwrId);
            pSource->ulHdmiPwrAcquire++;
        }
    }
#endif

    BDBG_MSG(("Creating source%d", pSource->eId));
    BDBG_ASSERT(BVDC_P_STATE_IS_INACTIVE(pSource));

    /* Reinitialize context.  But not make it active until apply. */
    *phSource = pSource;

    BVDC_P_Source_Init(*phSource, pDefSettings);

    /* Mark as create, awating for apply. */
    pSource->eState = BVDC_P_State_eCreate;

    BDBG_LEAVE(BVDC_Source_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Source_Destroy
    ( BVDC_Source_Handle               hSource )
{
    int i;

    BDBG_ENTER(BVDC_Source_Destroy);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

#if defined(BVDC_GFX_PERSIST)
    hSource->stNewInfo.pfPicCallbackFunc = NULL;
    hSource->stCurInfo.pfPicCallbackFunc = NULL;
    hSource->stNewInfo.pfGenericCallback = NULL;
    hSource->stCurInfo.pfGenericCallback = NULL;
    goto done;
#endif

    /* Check to see if there are any active windows that's still using
     * this source. */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        if(BVDC_P_STATE_IS_ACTIVE(hSource->ahWindow[i]))
        {
            BDBG_ERR(("Window %d is still using this source.", hSource->ahWindow[i]->eId));
            BDBG_ERR(("The window state is %d.", hSource->ahWindow[i]->eState));
            return BERR_TRACE(BERR_LEAKED_RESOURCE);
        }
    }

    /* Release the drain for the source. */
    if(BVDC_P_SRC_NEED_DRAIN(hSource->eId))
    {
        BVDC_P_Drain_Release(&hSource->stDrain, hSource->hVdc->hResource);
    }

    if(BVDC_P_STATE_IS_DESTROY(hSource) ||
       BVDC_P_STATE_IS_INACTIVE(hSource))
    {
        goto done;
    }

    if(BVDC_P_STATE_IS_ACTIVE(hSource) ||
       BVDC_P_STATE_IS_CREATE(hSource))
    {
        hSource->eState = BVDC_P_State_eDestroy;
    }

done:
    BDBG_LEAVE(BVDC_Source_Destroy);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetSize
    ( const BVDC_Source_Handle         hSource,
      uint32_t                        *pulWidth,
      uint32_t                        *pulHeight )
{
    const BFMT_VideoInfo *pFmtInfo;
    uint32_t ulWidth  = 0;
    uint32_t ulHeight = 0;

    BDBG_ENTER(BVDC_Source_GetSize);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* Analog/656 we know the source size, but for mpeg the source size
     * can change dynamically (dynamic source picture change).   Hence
     * we can only return the size of analog or 656 source. */
    if(BVDC_P_SRC_IS_ITU656(hSource->eId))
    {
        pFmtInfo = hSource->stCurInfo.pFmtInfo;
        ulWidth  = pFmtInfo->ulWidth;
        ulHeight = pFmtInfo->ulHeight;
    }
    else if(BVDC_P_SRC_IS_MPEG(hSource->eId) ||
       BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        BKNI_EnterCriticalSection();
        ulWidth  = hSource->stExtVideoFmtInfo.ulWidth;
        ulHeight = hSource->stExtVideoFmtInfo.ulHeight;
        BKNI_LeaveCriticalSection();
    }

    if(pulWidth)
    {
        *pulWidth = ulWidth;
    }

    if(pulHeight)
    {
        *pulHeight = ulHeight;
    }

    BDBG_LEAVE(BVDC_Source_GetSize);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetVideoFormat
    ( BVDC_Source_Handle               hSource,
      BFMT_VideoFmt                    eVideoFmt )
{
    BVDC_P_Source_Info *pNewInfo;

    BDBG_ENTER(BVDC_Source_SetVideoFormat);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    pNewInfo  = &hSource->stNewInfo;

    /* Restricted to only supported format. */
    if(BVDC_P_SRC_IS_ITU656(hSource->eId))
    {
        /* 656 specific validation. */
        if(!BFMT_IS_PAL(eVideoFmt) &&
           !BFMT_IS_NTSC(eVideoFmt))
        {
            return BERR_TRACE(BVDC_ERR_VIDEO_FORMAT_NOT_SUPPORTED);
        }
    }
    else if(!BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* set new info */
    pNewInfo->pFmtInfo = BFMT_GetVideoFormatInfoPtr(eVideoFmt);
    pNewInfo->pVdcFmt  = BVDC_P_GetFormatInfo_isrsafe(eVideoFmt);

    if((hSource->stCurInfo.pFmtInfo->eVideoFmt != eVideoFmt) ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        /* Dirty bit set */
        pNewInfo->stDirty.stBits.bInputFormat = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Source_SetVideoFormat);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetVideoFormat
    ( const BVDC_Source_Handle         hSource,
      BFMT_VideoFmt                   *peVideoFmt )
{
    BDBG_ENTER(BVDC_Source_GetVideoFormat);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* set return value */
    if(peVideoFmt)
    {
        *peVideoFmt = hSource->stCurInfo.pFmtInfo->eVideoFmt;
    }

    BDBG_LEAVE(BVDC_Source_GetVideoFormat);
    return BERR_SUCCESS;
}


/***************************************************************************
 * User call this function to get the interrupt name, so they can create
 * callback with BINT_CreateCallback.
 *
 */
BERR_Code BVDC_Source_GetInterruptName
    ( BVDC_Source_Handle               hSource,
      const BAVC_Polarity              eFieldId,
      BINT_Id                         *pInterruptName )
{
    BDBG_ENTER(BVDC_Source_GetInterruptName);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_MPEG(hSource->eId))
    {
        BDBG_ERR(("Not a MPEG source"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* set return value */
    if(pInterruptName)
    {
        /* Non-mpeg source should not need the interrupt name. */
        *pInterruptName = BRDC_Slot_GetIntId(
            hSource->ahSlot[BVDC_P_NEXT_POLARITY(eFieldId)]);

        BDBG_MSG(("What interrupt MVD should response to next "));
        BDBG_MSG(("eFieldId==%d?  Ans: %d (or BINT_ID=0x%08x)",
            eFieldId, BVDC_P_NEXT_POLARITY(eFieldId), *pInterruptName));
    }

    BDBG_LEAVE(BVDC_Source_GetInterruptName);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetVideoMuteColor
    ( BVDC_Source_Handle               hSource,
      const uint8_t                    ucRed,
      const uint8_t                    ucGreen,
      const uint8_t                    ucBlue )
{
    unsigned int uiColorARGB;
    BVDC_P_Source_Info *pNewInfo;

    BDBG_ENTER(BVDC_Source_SetVideoMuteColor);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    pNewInfo = &hSource->stNewInfo;

    /* set new info */
    pNewInfo->ucRed   = ucRed;
    pNewInfo->ucGreen = ucGreen;
    pNewInfo->ucBlue  = ucBlue;
    uiColorARGB = BPXL_MAKE_PIXEL(BPXL_eA8_R8_G8_B8, 0x00, ucRed, ucGreen, ucBlue);
    BPXL_ConvertPixel_RGBtoYCbCr(BPXL_eA8_Y8_Cb8_Cr8, BPXL_eA8_R8_G8_B8,
        uiColorARGB, (unsigned int*)&pNewInfo->ulMuteColorYCrCb);

    if((hSource->stCurInfo.ucRed   != ucRed) ||
       (hSource->stCurInfo.ucGreen != ucGreen) ||
       (hSource->stCurInfo.ucBlue  != ucBlue) ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        /* Dirty bit set */
        pNewInfo->stDirty.stBits.bUserChanges = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Source_SetVideoMuteColor);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetVideoMuteColor
    ( const BVDC_Source_Handle          hSource,
      uint8_t                          *pucRed,
      uint8_t                          *pucGreen,
      uint8_t                          *pucBlue )
{
    BDBG_ENTER(BVDC_Source_GetVideoMuteColor);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(pucRed)
    {
        *pucRed   = hSource->stCurInfo.ucRed;
    }

    if(pucGreen)
    {
        *pucGreen = hSource->stCurInfo.ucGreen;
    }

    if(pucBlue)
    {
        *pucBlue  = hSource->stCurInfo.ucBlue;
    }

    BDBG_LEAVE(BVDC_Source_GetVideoMuteColor);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetMuteMode
    ( BVDC_Source_Handle               hSource,
      BVDC_MuteMode                    eMuteMode )
{
    BVDC_P_Source_Info *pNewInfo;

    BDBG_ENTER(BVDC_Source_SetMuteMode);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    pNewInfo = &hSource->stNewInfo;

    if(eMuteMode > BVDC_MuteMode_eRepeat)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(((BVDC_P_SRC_IS_VFD(hSource->eId)) ||
        (BVDC_P_SRC_IS_MPEG(hSource->eId) && hSource->hMpegFeeder->bGfxSrc)) &&
       (BVDC_MuteMode_eConst < eMuteMode))
    {
        BDBG_ERR(("MPEG/VFD source %d don't need to SetMuteMode to repeat gfx surface.", hSource->eId));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(BVDC_P_SRC_IS_MPEG(hSource->eId) && !hSource->hMpegFeeder->bGfxSrc &&
       (BVDC_MuteMode_eDisable != eMuteMode))
    {
        BDBG_ERR(("MPEG source %d shall be muted from decoder.", hSource->eId));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* set new value */
    pNewInfo->eMuteMode = eMuteMode;

    if((hSource->stCurInfo.eMuteMode != eMuteMode) ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        /* Dirty bit set */
        pNewInfo->stDirty.stBits.bUserChanges = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Source_SetMuteMode);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetMuteMode
    ( const BVDC_Source_Handle          hSource,
      BVDC_MuteMode                    *peMuteMode )
{
    BDBG_ENTER(BVDC_Source_GetMuteMode);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(peMuteMode)
    {
        *peMuteMode = hSource->stCurInfo.eMuteMode;
    }

    BDBG_LEAVE(BVDC_Source_GetMuteMode);
    return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_OverrideAspectRatio
    ( BVDC_Source_Handle               hSource,
      BFMT_AspectRatio                 eAspectRatio )
{
    BVDC_P_Source_Info *pNewInfo;

    BDBG_ENTER(BVDC_Source_OverrideAspectRatio);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    pNewInfo = &hSource->stNewInfo;

    if(eAspectRatio > BFMT_AspectRatio_e15_9)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* set new value */
    pNewInfo->eAspectRatio  = eAspectRatio;

    /* don't auto-update src aspect ratio accoring to videoFmt in
     * next ApplyChanges, since user override it */
    hSource->bNewUserModeAspRatio = true;

    if((hSource->stCurInfo.eAspectRatio != eAspectRatio) ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        /* Dirty bit set */
        pNewInfo->stDirty.stBits.bAspectRatio = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Source_OverrideAspectRatio);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetAspectRatio
    ( const BVDC_Source_Handle          hSource,
      BFMT_AspectRatio                 *peAspectRatio )
{
    BDBG_ENTER(BVDC_Source_GetAspectRatio);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(peAspectRatio)
    {
        *peAspectRatio = hSource->stCurInfo.eAspectRatio;
    }

    BDBG_LEAVE(BVDC_Source_GetAspectRatio);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetInputPort
    ( BVDC_Source_Handle               hSource,
      uint32_t                         ulInputPort )
{
    BVDC_P_Source_Info *pNewInfo;

    BDBG_ENTER(BVDC_Source_SetInputPort);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    pNewInfo = &hSource->stNewInfo;

    /* set new value */
    pNewInfo->ulInputPort = ulInputPort;

    if((hSource->stCurInfo.ulInputPort != ulInputPort) ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        /* Dirty bit set!  Shared the input format/port  dirty bits */
        pNewInfo->stDirty.stBits.bInputFormat = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Source_SetInputPort);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetInputPort
    ( BVDC_Source_Handle               hSource,
      uint32_t                        *pulInputPort )
{
    BDBG_ENTER(BVDC_Source_GetInputPort);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(pulInputPort)
    {
        *pulInputPort = hSource->stCurInfo.ulInputPort;
    }

    BDBG_LEAVE(BVDC_Source_GetInputPort);

    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetAutoFormat
    ( BVDC_Source_Handle               hSource,
      bool                             bAutoDetect,
      const BFMT_VideoFmt              aeFormats[],
      uint32_t                         ulNumFormats )
{
    BVDC_P_Source_Info *pNewInfo;
    uint32_t ulIndex;

    BDBG_ENTER(BVDC_Source_SetAutoFormat);

    BDBG_ASSERT(hSource);
    BDBG_ASSERT(ulNumFormats < BFMT_VideoFmt_eMaxCount);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        BDBG_ERR(("Source is not HD_DVI"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pNewInfo = &hSource->stNewInfo;

    /* set new value */
    pNewInfo->bAutoDetect = bAutoDetect;
    pNewInfo->ulNumFormats = ulNumFormats;

    if((hSource->stCurInfo.bAutoDetect != bAutoDetect)   ||
       (hSource->stCurInfo.ulNumFormats != ulNumFormats) ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        /* Dirty bit set */
        pNewInfo->stDirty.stBits.bAutoDetectFmt = BVDC_P_DIRTY;
    }

    for(ulIndex = 0; ulIndex < ulNumFormats; ulIndex++)
    {
        pNewInfo->aeFormats[ulIndex] = aeFormats[ulIndex];
        if((hSource->stCurInfo.aeFormats[ulIndex] != aeFormats[ulIndex]) ||
           (hSource->stNewInfo.bErrorLastSetting))
        {
            pNewInfo->stDirty.stBits.bAutoDetectFmt = BVDC_P_DIRTY;
        }
    }

    BDBG_LEAVE(BVDC_Source_SetAutoFormat);

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetDnrConfiguration
    ( BVDC_Source_Handle               hSource,
      const BVDC_Dnr_Settings         *pDnrSettings )
{
#if (!BVDC_P_SUPPORT_DNR)
    BSTD_UNUSED(hSource);
    BSTD_UNUSED(pDnrSettings);
    BDBG_WRN(("DNR is not supported for this chipset"));
#else
    BDBG_ENTER(BVDC_Source_SetDnrConfiguration);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if((pDnrSettings->iBnrLevel <= (-1) * BVDC_QP_ADJUST_STEPS) ||
       (pDnrSettings->iMnrLevel <= (-1) * BVDC_QP_ADJUST_STEPS) ||
       (pDnrSettings->iDcrLevel <= (-1) * BVDC_QP_ADJUST_STEPS))
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* only support mvd/xvd source; */
    if((pDnrSettings->eMnrMode != BVDC_FilterMode_eDisable) ||
       (pDnrSettings->eBnrMode != BVDC_FilterMode_eDisable) ||
       (pDnrSettings->eDcrMode != BVDC_FilterMode_eDisable))
    {
        hSource->stNewInfo.bDnr = true;
    }
    else
    {
        hSource->stNewInfo.bDnr = false;
    }

    /* set new value */
    hSource->stNewInfo.stDnrSettings = *pDnrSettings;
    hSource->stNewInfo.stDnrSettings.iBnrLevel = BVDC_P_MIN(hSource->stNewInfo.stDnrSettings.iBnrLevel, BVDC_P_DNR_H_MAX_RANGE);
    hSource->stNewInfo.stDnrSettings.iMnrLevel = BVDC_P_MIN(hSource->stNewInfo.stDnrSettings.iMnrLevel, BVDC_P_DNR_H_MAX_RANGE);
    hSource->stNewInfo.stDnrSettings.iDcrLevel = BVDC_P_MIN(hSource->stNewInfo.stDnrSettings.iDcrLevel, BVDC_P_DNR_H_MAX_RANGE);
    hSource->stNewInfo.stDirty.stBits.bDnrAdjust = BVDC_P_DIRTY;

    BDBG_LEAVE(BVDC_Source_SetDnrConfiguration);
#endif

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetDnrConfiguration
    ( const BVDC_Source_Handle          hSource,
      BVDC_Dnr_Settings                *pDnrSettings )
{
#if (!BVDC_P_SUPPORT_DNR)
    BSTD_UNUSED(hSource);
    BSTD_UNUSED(pDnrSettings);
    BDBG_WRN(("DNR is not supported for this chipset"));
#else
    BDBG_ENTER(BVDC_Source_GetDnrConfiguration);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* set new value */
    if(pDnrSettings)
    {
        *pDnrSettings = hSource->stCurInfo.stDnrSettings;
    }

    BDBG_LEAVE(BVDC_Source_GetDnrConfiguration);
#endif
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetSplitScreenMode
    ( BVDC_Source_Handle                      hSource,
      const BVDC_Source_SplitScreenSettings  *pSplitScreen )
{
    BVDC_P_Source_Info *pNewInfo;

    BDBG_ENTER(BVDC_Source_SetSplitScreenMode);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    pNewInfo  = &hSource->stNewInfo;

    pNewInfo->stSplitScreenSetting.eDnr = pSplitScreen->eDnr;
    if((hSource->stCurInfo.stSplitScreenSetting.eDnr != pSplitScreen->eDnr) ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        pNewInfo->stDirty.stBits.bDnrAdjust = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Source_SetSplitScreenMode);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetSplitScreenMode
    ( const BVDC_Source_Handle          hSource,
      BVDC_Source_SplitScreenSettings  *pSplitScreen )
{
    BDBG_ENTER(BVDC_Source_GetSplitScreenMode);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(pSplitScreen)
    {
        *pSplitScreen = hSource->stCurInfo.stSplitScreenSetting;
    }

    BDBG_LEAVE(BVDC_Source_GetSplitScreenMode);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetHdDviConfiguration
    ( const BVDC_Source_Handle         hSource,
      BVDC_HdDvi_Settings             *pHdDviSettings )
{
    BDBG_ENTER(BVDC_Source_GetHdDviConfiguration);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        BDBG_ERR(("Source is not HD_DVI"));
        return BERR_TRACE(BVDC_ERR_BAD_SRC_TYPE);
    }

    if(pHdDviSettings)
    {
        *pHdDviSettings = hSource->stCurInfo.stHdDviSetting;
    }

    BDBG_LEAVE(BVDC_Source_GetHdDviConfiguration);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetHdDviConfiguration
    ( BVDC_Source_Handle               hSource,
      const BVDC_HdDvi_Settings       *pHdDviSettings )
{
    BDBG_ENTER(BVDC_Source_SetHdDviConfiguration);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        BDBG_ERR(("Source is not HD_DVI"));
        return BERR_TRACE(BVDC_ERR_BAD_SRC_TYPE);
    }
    else
    {
        BVDC_P_Source_Info *pNewInfo = &hSource->stNewInfo;
        BVDC_P_Source_Info *pCurInfo = &hSource->stCurInfo;
        BVDC_P_Source_DirtyBits *pNewDirty = &pNewInfo->stDirty;
        BVDC_HdDvi_Settings *pCurSetting = &pCurInfo->stHdDviSetting;

        /* Enable or disable DE, data mode */
        pNewInfo->stHdDviSetting = *pHdDviSettings;
        if((pCurSetting->bEnableDe != pHdDviSettings->bEnableDe) ||
           (pCurSetting->eInputDataMode != pHdDviSettings->eInputDataMode) ||
           (pCurSetting->stFmtTolerence.ulWidth  != pHdDviSettings->stFmtTolerence.ulWidth)  ||
           (pCurSetting->stFmtTolerence.ulHeight != pHdDviSettings->stFmtTolerence.ulHeight) ||
           (hSource->stNewInfo.bErrorLastSetting))
        {
            pNewDirty->stBits.bMiscCtrl = BVDC_P_DIRTY;
        }
    }

    BDBG_LEAVE(BVDC_Source_SetHdDviConfiguration);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetHVStart
    ( BVDC_Source_Handle               hSource,
      bool                             bOverrideVdc,
      uint32_t                         ulHStart,
      uint32_t                         ulVStart )
{
    BDBG_ENTER(BVDC_Source_SetHVStart);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        BDBG_ERR(("Source is not HD_DVI"));
        return BERR_TRACE(BVDC_ERR_BAD_SRC_TYPE);
    }
    else
    {
        BVDC_P_Source_Info *pNewInfo = &hSource->stNewInfo;
        BVDC_P_Source_Info *pCurInfo = &hSource->stCurInfo;
        BVDC_P_Source_DirtyBits *pNewDirty = &pNewInfo->stDirty;

        /* set new value */
        /* auto/manual clock adjustment */
        pNewInfo->bHVStartOverride = bOverrideVdc;
        if(pCurInfo->bHVStartOverride != bOverrideVdc)
        {
            pNewDirty->stBits.bManualPos = BVDC_P_DIRTY;
        }

        if(bOverrideVdc)
        {
            pNewInfo->ulHstart = ulHStart;
            pNewInfo->ulVstart = ulVStart;
            if((pCurInfo->ulHstart != ulHStart) ||
               (pCurInfo->ulVstart != ulVStart))
            {
                pNewDirty->stBits.bManualPos = BVDC_P_DIRTY;
            }
        }
    }

    BDBG_LEAVE(BVDC_Source_SetHVStart);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
static BERR_Code BVDC_P_Source_GetHVStart
    ( BVDC_Source_Handle               hSource,
      bool                            *pbOverrideVdc,
      uint32_t                        *pulHStart,
      uint32_t                        *pulVStart,
      bool                             bGetDefault )
{
    BDBG_ENTER(BVDC_P_Source_GetHVStart);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        BDBG_ERR(("Source is not HD_DVI"));
        return BERR_TRACE(BVDC_ERR_BAD_SRC_TYPE);
    }

    if(pbOverrideVdc)
    {
        *pbOverrideVdc = hSource->stCurInfo.bHVStartOverride;
    }

    if((hSource->stCurInfo.bHVStartOverride) && (!bGetDefault))
    {
        if(pulHStart)
        {
            *pulHStart = hSource->stCurInfo.ulHstart;
        }

        if(pulVStart)
        {
            *pulVStart = hSource->stCurInfo.ulVstart;
        }
    }
    else
    {
        if(BVDC_P_SRC_IS_HDDVI(hSource->eId))
        {
            if(pulHStart)
            {
                *pulHStart = hSource->hHdDvi->ulHorzDelay;
            }

            if(pulVStart)
            {
                *pulVStart = hSource->hHdDvi->ulVertDelay;
            }
        }
    }

    BDBG_LEAVE(BVDC_P_Source_GetHVStart);
    return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetHVStart
    ( BVDC_Source_Handle               hSource,
      bool                            *pbOverrideVdc,
      uint32_t                        *pulHStart,
      uint32_t                        *pulVStart )
{
    return BVDC_P_Source_GetHVStart(hSource, pbOverrideVdc, pulHStart, pulVStart,
        false);
}
#endif


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetDefaultHVStart
    ( BVDC_Source_Handle               hSource,
      uint32_t                        *pulHStart,
      uint32_t                        *pulVStart )
{
    return BVDC_P_Source_GetHVStart(hSource, NULL, pulHStart, pulVStart, true);
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetOrientation
    ( BVDC_Source_Handle               hSource,
      bool                             bOverride,
      BFMT_Orientation                 eOrientation )
{
    BDBG_ENTER(BVDC_Source_SetOrientation);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_HDDVI(hSource->eId) && !BVDC_P_SRC_IS_MPEG(hSource->eId) &&
       !BVDC_P_SRC_IS_GFX(hSource->eId))
    {
        BDBG_ERR(("Source is not HD_DVI or MPEG or GFX"));
        return BERR_TRACE(BVDC_ERR_BAD_SRC_TYPE);
    }
    else
    {
        BVDC_P_Source_Info *pNewInfo = &hSource->stNewInfo;
        BVDC_P_Source_Info *pCurInfo = &hSource->stCurInfo;
        BVDC_P_Source_DirtyBits *pNewDirty = &pNewInfo->stDirty;

#if (BVDC_P_DCX_3D_WORKAROUND)
        if(eOrientation != BFMT_Orientation_e2D)
        {
            BDBG_WRN(("==================================================================="));
            BDBG_WRN(("May not have enough bandwidth to support 3D source (orientation %d)",
                eOrientation));
            BDBG_WRN(("==================================================================="));
        }
#endif

        /* set new value */
        pNewInfo->bOrientationOverride = bOverride;
        if(pCurInfo->bOrientationOverride != bOverride)
        {
            pNewDirty->stBits.bOrientation = BVDC_P_DIRTY;
        }

        if(bOverride)
        {
            pNewInfo->eOrientation = eOrientation;
            if(pCurInfo->eOrientation != eOrientation)
            {
                pNewDirty->stBits.bOrientation = BVDC_P_DIRTY;
            }
        }
    }

    BDBG_LEAVE(BVDC_Source_SetOrientation);
    return BERR_SUCCESS;

}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetOrientation
    ( BVDC_Source_Handle               hSource,
      bool                            *pbOverride,
      BFMT_Orientation                *peOrientation )
{
    BDBG_ENTER(BVDC_Source_GetOrientation);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_HDDVI(hSource->eId) && !BVDC_P_SRC_IS_MPEG(hSource->eId) &&
       !BVDC_P_SRC_IS_GFX(hSource->eId))
    {
        BDBG_ERR(("Source is not HD_DVI or MPEG or GFX"));
        return BERR_TRACE(BVDC_ERR_BAD_SRC_TYPE);
    }

    if(pbOverride)
    {
        *pbOverride = hSource->stCurInfo.bOrientationOverride;
    }

    if(peOrientation)
    {
        *peOrientation = hSource->stCurInfo.eOrientation;
    }

    BDBG_LEAVE(BVDC_Source_GetOrientation);
    return BERR_SUCCESS;

}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetColorMatrix
    ( BVDC_Source_Handle               hSource,
      bool                             bOverride,
      const int32_t                    pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                         ulShift )
{
    BDBG_ENTER(BVDC_Source_SetColorMatrix);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        BDBG_ERR(("Source is not HDDVI"));
        return BERR_TRACE(BVDC_ERR_BAD_SRC_TYPE);
    }
    else
    {
        BVDC_P_Source_Info *pNewInfo = &hSource->stNewInfo;
        BVDC_P_Source_DirtyBits *pNewDirty = &pNewInfo->stDirty;

        /* set new value */
        pNewInfo->bUserCsc = bOverride;
        if(bOverride)
        {
            uint32_t ulIndex;
            pNewInfo->ulUserShift = ulShift;
            for(ulIndex = 0; ulIndex < BVDC_CSC_COEFF_COUNT; ulIndex++)
            {
                pNewInfo->pl32_Matrix[ulIndex] = pl32_Matrix[ulIndex];
            }
        }
        pNewDirty->stBits.bColorspace = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Source_SetColorMatrix);
    return BERR_SUCCESS;

}

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetColorMatrix
    ( BVDC_Source_Handle               hSource,
      bool                            *pbOverride,
      int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                        *pulShift )
{
    BDBG_ENTER(BVDC_Source_GetColorMatrix);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(pbOverride)
    {
        *pbOverride = hSource->stCurInfo.bUserCsc;
    }

    if(hSource->stCurInfo.bUserCsc)
    {
        uint32_t ulIndex;
        for(ulIndex = 0; ulIndex < BVDC_CSC_COEFF_COUNT; ulIndex++)
        {
            pl32_Matrix[ulIndex] = hSource->stCurInfo.pl32_Matrix[ulIndex];
        }

        if(pulShift)
        {
            *pulShift = hSource->stCurInfo.ulUserShift;
        }
    }
    else
    {
        if(BVDC_P_SRC_IS_HDDVI(hSource->eId))
        {
            BKNI_EnterCriticalSection();
            BVDC_P_Csc_ToMatrix_isr(pl32_Matrix, &hSource->hHdDvi->stCsc,
                BVDC_P_FIX_POINT_SHIFT);
            BKNI_LeaveCriticalSection();
        }

        if(pulShift)
        {
            *pulShift = BVDC_P_FIX_POINT_SHIFT;
        }
    }

    BDBG_LEAVE(BVDC_Source_GetColorMatrix);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetFrontendColorMatrix
    ( BVDC_Source_Handle               hSource,
      bool                            *pbOverride,
      int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                        *pulShift )
{
    BDBG_ENTER(BVDC_Source_GetFrontendColorMatrix);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(pbOverride)
    {
        *pbOverride = hSource->stCurInfo.bUserFrontendCsc;
    }

    if(hSource->stCurInfo.bUserFrontendCsc)
    {
        uint32_t ulIndex;
        for(ulIndex = 0; ulIndex < BVDC_CSC_COEFF_COUNT; ulIndex++)
        {
            pl32_Matrix[ulIndex] = hSource->stCurInfo.pl32_FrontendMatrix[ulIndex];
        }

        if(pulShift)
        {
            *pulShift = hSource->stCurInfo.ulUserFrontendShift;
        }
    }
    else
    {
        if(pulShift)
        {
            *pulShift = BVDC_P_FIX_POINT_SHIFT;
        }
    }

    BDBG_LEAVE(BVDC_Source_GetFrontendColorMatrix);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetInputStatus
    ( const BVDC_Source_Handle         hSource,
      BVDC_Source_InputStatus         *pInputStatus )
{
    BDBG_ENTER(BVDC_Source_GetInputStatus);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        BDBG_ERR(("Source is not HD_DVI input"));
        return BERR_TRACE(BVDC_ERR_BAD_SRC_TYPE);
    }

    if(pInputStatus)
    {
        /* To make sure thing get initialize */
        BKNI_Memset(pInputStatus, 0, sizeof(*pInputStatus));

        if(BVDC_P_SRC_IS_HDDVI(hSource->eId))
        {
            BVDC_P_HdDvi_GetInputStatus(hSource->hHdDvi, pInputStatus);
        }
    }

    BDBG_LEAVE(BVDC_Source_GetInputStatus);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetPsfMode
    ( BVDC_Source_Handle               hSource,
      bool                             bEnable )
{
    BDBG_ENTER(BVDC_Source_SetPsfMode);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_MPEG(hSource->eId))
    {
        BDBG_ERR(("Source is not MPEG input"));
        return BERR_TRACE(BVDC_ERR_BAD_SRC_TYPE);
    }

    hSource->stNewInfo.bPsfEnable = bEnable;
    if(hSource->stCurInfo.bPsfEnable != bEnable)
    {
        /* Dirty bit set */
        hSource->stNewInfo.stDirty.stBits.bPsfMode = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Source_SetPsfMode);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetPsfMode
    ( BVDC_Source_Handle               hSource,
      bool                            *pbEnable )
{
    BDBG_ENTER(BVDC_Source_GetPsfMode);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!BVDC_P_SRC_IS_MPEG(hSource->eId))
    {
        BDBG_ERR(("Source is not MPEG input"));
        return BERR_TRACE(BVDC_ERR_BAD_SRC_TYPE);
    }

    if(pbEnable)
    {
        *pbEnable = hSource->stCurInfo.bPsfEnable;
    }

    BDBG_LEAVE(BVDC_Source_GetPsfMode);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_ForceFrameCapture
    ( BVDC_Source_Handle               hSource,
      bool                             bForce )
{
    BDBG_ENTER(BVDC_Source_ForceFrameCapture);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    hSource->stNewInfo.bForceFrameCapture = bForce;
    if(hSource->stCurInfo.bForceFrameCapture != bForce)
    {
        /* Dirty bit set */
        hSource->stNewInfo.stDirty.stBits.bPsfMode = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Source_ForceFrameCapture);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Source_InstallPictureCallback
    ( BVDC_Source_Handle               hSource,
      BVDC_Source_PictureCallback_isr  pfSrcCallback,
      void                            *pvParm1,
      int                              iParm2 )
{
    BVDC_P_Source_Info *pNewInfo;

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    pNewInfo = &hSource->stNewInfo;

    if((!BVDC_P_SRC_IS_HDDVI(hSource->eId)) &&
       (!BVDC_P_SRC_IS_GFX(hSource->eId)) &&
       (!BVDC_P_SRC_IS_VFD(hSource->eId)) &&
       (!(BVDC_P_SRC_IS_MPEG(hSource->eId) && hSource->hMpegFeeder->bGfxSrc)))
    {
        BDBG_ERR(("Source is not HD_DVI, VFD, or GFX"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* set new info and dirty bits. */
    pNewInfo->pfPicCallbackFunc = pfSrcCallback;
    pNewInfo->pvParm1 = pvParm1;
    pNewInfo->iParm2 = iParm2;

    if((hSource->stCurInfo.pfPicCallbackFunc != pfSrcCallback) ||
       (hSource->stCurInfo.pvParm1 != pvParm1) ||
       (hSource->stCurInfo.iParm2 != iParm2)   ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        /* Dirty bit set */
        pNewInfo->stDirty.stBits.bPicCallback = BVDC_P_DIRTY;
    }

    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Source_InstallCallback
    ( BVDC_Source_Handle               hSource,
      const BVDC_CallbackFunc_isr      pfCallback,
      void                            *pvParm1,
      int                              iParm2 )
{
    BVDC_P_Source_Info *pNewInfo;

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    pNewInfo = &hSource->stNewInfo;

    /* set new info and dirty bits. */
    pNewInfo->pfGenericCallback = pfCallback;
    pNewInfo->pvGenericParm1 = pvParm1;
    pNewInfo->iGenericParm2 = iParm2;

    if((hSource->stCurInfo.pfGenericCallback != pfCallback) ||
       (hSource->stCurInfo.pvGenericParm1 != pvParm1) ||
       (hSource->stCurInfo.iGenericParm2 != iParm2)   ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        /* Dirty bit set */
        pNewInfo->stDirty.stBits.bGenCallback = BVDC_P_DIRTY;
    }

    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetCallbackSettings
    ( BVDC_Source_Handle                   hSource,
      const BVDC_Source_CallbackSettings  *pSettings )
{
    BDBG_ENTER(BVDC_Source_SetCallbackSettings);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(!pSettings)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hSource->stNewInfo.stCallbackSettings.stMask = pSettings->stMask;
    if((hSource->stCurInfo.stCallbackSettings.stMask.bFmtInfo != pSettings->stMask.bFmtInfo) ||
       (hSource->stCurInfo.stCallbackSettings.stMask.bActive   != pSettings->stMask.bActive) ||
       (hSource->stCurInfo.stCallbackSettings.stMask.bCrcValue!= pSettings->stMask.bCrcValue) ||
       (hSource->stCurInfo.stCallbackSettings.stMask.bFrameRate!= pSettings->stMask.bFrameRate) ||
       (hSource->stCurInfo.stCallbackSettings.stMask.bSrcPending!= pSettings->stMask.bSrcPending) ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        hSource->stNewInfo.stDirty.stBits.bGenCallback = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Source_SetCallbackSettings);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetCallbackSettings
    ( BVDC_Source_Handle                   hSource,
      BVDC_Source_CallbackSettings        *pSettings )
{
    BDBG_ENTER(BVDC_Source_GetCallbackSettings);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if (pSettings)
    {
        *pSettings = hSource->stCurInfo.stCallbackSettings;
    }

    BDBG_LEAVE(BVDC_Source_GetCallbackSettings);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetResumeMode
    ( BVDC_Source_Handle               hSource,
      BVDC_ResumeMode                  eResumeMode )
{
    BVDC_P_Source_Info *pNewInfo;

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    pNewInfo = &hSource->stNewInfo;

    /* set new info and dirty bits. */
    pNewInfo->eResumeMode = eResumeMode;

    if((hSource->stCurInfo.eResumeMode != eResumeMode) ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        /* Dirty bit set */
        pNewInfo->stDirty.stBits.bUserChanges = BVDC_P_DIRTY;
    }

    return BERR_SUCCESS;
}
#endif


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetResumeMode
    ( BVDC_Source_Handle               hSource,
      BVDC_ResumeMode                 *peResumeMode )
{
    BDBG_ENTER(BVDC_Source_GetResumeMode);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(peResumeMode)
    {
        *peResumeMode = hSource->stCurInfo.eResumeMode;
    }

    BDBG_LEAVE(BVDC_Source_GetResumeMode);
    return BERR_SUCCESS;
}
#endif


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_Resume
    ( BVDC_Source_Handle               hSource )
{
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* set new info and dirty bits. */
    hSource->stNewInfo.stDirty.stBits.bResume = BVDC_P_DIRTY;

    return BERR_SUCCESS;
}
#endif


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_ForcePending
    ( BVDC_Source_Handle               hSource )
{
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* set new info and dirty bits. */
    hSource->stNewInfo.bForceSrcPending = true;

    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetCapabilities
    ( const BVDC_Source_Handle         hSource,
      BVDC_Source_Capabilities        *pCapabilities )
{
    BSTD_UNUSED(hSource);

    if(pCapabilities)
    {
        /* To make sure thing get initialize */
        BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));

        pCapabilities->pfIsPxlfmtSupported = BVDC_P_IsPxlfmtSupported;
    }

    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_GetCrcType
    ( const BVDC_Source_Handle         hSource,
      BVDC_Source_CrcType             *peCrcType )
{
    BDBG_ENTER(BVDC_Source_GetCrcType);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(peCrcType)
    {
        *peCrcType = hSource->stCurInfo.eCrcType;
    }

    BDBG_LEAVE(BVDC_Source_GetCrcType);
    return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetCrcType
    ( const BVDC_Source_Handle         hSource,
      BVDC_Source_CrcType              eCrcType )
{
    BVDC_P_Source_Info *pNewInfo;

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    pNewInfo = &hSource->stNewInfo;

    if(!BVDC_P_SRC_IS_MPEG(hSource->eId))
    {
        BDBG_ERR(("Feature is only supported for MPEG source"));
        return BERR_TRACE(BVDC_ERR_FEATURE_NOT_SUPPORTED);
    }

#if (BVDC_P_MFD_SUPPORT_CRC_TYPE)
    /* set new info and dirty bits. */
    pNewInfo->eCrcType = eCrcType;

    if((hSource->stCurInfo.eCrcType != eCrcType) ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        /* Dirty bit set */
        pNewInfo->stDirty.stBits.bUserChanges = BVDC_P_DIRTY;
    }
#else
    BSTD_UNUSED(hSource);
    BSTD_UNUSED(eCrcType);
    BSTD_UNUSED(pNewInfo);
#endif

    return BERR_SUCCESS;
}
#endif

#if (BVDC_P_CHECK_MEMC_INDEX)
static BERR_Code BVDC_P_Source_CheckGfxMemcIndex_isr
    ( BVDC_Source_Handle      hSource,
     const BAVC_Gfx_Picture  *pAvcGfxPic)
{
    BERR_Code   eStatus = BERR_SUCCESS;
    uint32_t    i, ulGfxWinId = 0;
    uint32_t    ulMemcIndex = BBOX_VDC_DISREGARD;
    BBOX_Vdc_MemcIndexSettings  *pVdcMemcSettings = NULL;
    uint64_t    ulSurOffset;

    pVdcMemcSettings = &hSource->hVdc->stBoxConfig.stMemConfig.stVdcMemcIndex;

    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        BVDC_Window_Handle   hWindow;
        BVDC_DisplayId       eDispId;

        /* SKIP: If it's just created or inactive no need to build ruls. */
        if(!hSource->ahWindow[i] ||
            BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
        {
            continue;
        }

        hWindow = hSource->ahWindow[i];
        BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

        eDispId = hWindow->hCompositor->hDisplay->eId;
        ulMemcIndex = pVdcMemcSettings->astDisplay[eDispId].aulGfdWinMemcIndex[ulGfxWinId];
        if(ulMemcIndex == BBOX_MemcIndex_Invalid)
        {
            BDBG_ERR(("Gfx Window[%d] on display[%d] is not support in boxmode %d",
                ulGfxWinId, eDispId, hSource->hVdc->stBoxConfig.stBox.ulBoxId));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        if(pAvcGfxPic->pSurface)
        {
            ulSurOffset = BMMA_GetOffset_isr(pAvcGfxPic->pSurface->hPixels);
            ulSurOffset += pAvcGfxPic->pSurface->ulPixelsOffset;

            if( !BCHP_OffsetOnMemc(hSource->hVdc->hChip, ulSurOffset, ulMemcIndex) )
            {
                BDBG_MODULE_MSG(BVDC_MEMC_INDEX_CHECK,
                    ("Disp[%d]GfxWin[%d] mismatch between surface addr " BDBG_UINT64_FMT " and Box mode MEMC%u",
                    eDispId, ulGfxWinId, BDBG_UINT64_ARG(ulSurOffset), ulMemcIndex));
            }
        }

        if(pAvcGfxPic->pRSurface && (pAvcGfxPic->eInOrientation != BFMT_Orientation_e2D))
        {
            ulSurOffset = BMMA_GetOffset_isr(pAvcGfxPic->pRSurface->hPixels);
            ulSurOffset += pAvcGfxPic->pRSurface->ulPixelsOffset;

            if( !BCHP_OffsetOnMemc(hSource->hVdc->hChip, ulSurOffset, ulMemcIndex) )
            {
                BDBG_MODULE_MSG(BVDC_MEMC_INDEX_CHECK,
                    ("Disp[%d]GfxWin[%d] mismatch between Rsurface addr " BDBG_UINT64_FMT " and Box mode MEMC%u",
                    eDispId, ulGfxWinId, BDBG_UINT64_ARG(ulSurOffset), ulMemcIndex));
            }
        }

    }

    return eStatus;
}
#endif

#ifndef BVDC_FOR_BOOTUPDATER
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetSurface
    ( BVDC_Source_Handle      hSource,
     const BAVC_Gfx_Picture  *pAvcGfxPic)
{
    BERR_Code  eStatus = BERR_SUCCESS;
    BPXL_Format ePxlFmt;

    BDBG_ENTER(BVDC_Source_SetSurface);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_ASSERT(NULL != pAvcGfxPic);

    BKNI_EnterCriticalSection();
    BVDC_P_CHECK_CS_ENTER_VDC(hSource->hVdc);

#if (BVDC_P_CHECK_MEMC_INDEX)
    /* Checking assumes memory is continous, possible problem when there is
     * hole in memory address. Disable the checking till we have correction
     * information from BCHP. */
    /* Check if gfx surface is on the correct memc based on boxmode */
    eStatus = BVDC_P_Source_CheckGfxMemcIndex_isr(hSource, pAvcGfxPic);
    if (BERR_SUCCESS != eStatus)
    {
        return BERR_TRACE(eStatus);
    }
#endif

    if (NULL != hSource->hGfxFeeder && NULL != pAvcGfxPic)
    {
        eStatus = BVDC_P_GfxSurface_SetSurface_isr(
            &hSource->hGfxFeeder->stGfxSurface,
            &hSource->hGfxFeeder->stGfxSurface.stNewSurInfo, pAvcGfxPic, hSource);
    }
    else if (NULL != hSource->hVfdFeeder && NULL != pAvcGfxPic)
    {
        ePxlFmt = pAvcGfxPic->pSurface->eFormat;
        if (!BPXL_IS_YCbCr422_FORMAT(ePxlFmt))
        {
            BDBG_ERR(("Invalid pixel format for VFD"));
            eStatus = BERR_INVALID_PARAMETER;
        }
        else
        {
            eStatus = BVDC_P_GfxSurface_SetSurface_isr(
                &hSource->hVfdFeeder->stGfxSurface,
                &hSource->hVfdFeeder->stGfxSurface.stNewSurInfo, pAvcGfxPic, hSource);
        }
    }
    else if (NULL != hSource->hMpegFeeder && NULL != pAvcGfxPic)
    {
        if(pAvcGfxPic->pSurface) {
            ePxlFmt = pAvcGfxPic->pSurface->eFormat;
            if (!BPXL_IS_YCbCr422_FORMAT(ePxlFmt))
            {
                BDBG_ERR(("Invalid pixel format for MFD"));
                eStatus = BERR_INVALID_PARAMETER;
                goto done;
            }
        } else if(!pAvcGfxPic->pstMfdPic) {
            BDBG_ERR(("Source[%u] set invalid surface", hSource->eId));
            eStatus = BERR_INVALID_PARAMETER;
            goto done;
        } else {
            if(pAvcGfxPic->eInOrientation != BFMT_Orientation_e2D) {
                BDBG_ERR(("Source[%u] doesn't support 3D striped surface", hSource->eId));
                eStatus = BERR_INVALID_PARAMETER;
                goto done;
            }
        }
        eStatus = BVDC_P_GfxSurface_SetSurface_isr(
            &hSource->hMpegFeeder->stGfxSurface,
            &hSource->hMpegFeeder->stGfxSurface.stNewSurInfo, pAvcGfxPic, hSource);
    }
    else
    {
        BDBG_ERR(("Invalid source handle or gfx pic to set gfx surface."));
        eStatus = BERR_INVALID_PARAMETER;
    }

done:
    BVDC_P_CHECK_CS_LEAVE_VDC(hSource->hVdc);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVDC_Source_SetSurface);
    return BERR_TRACE(eStatus);
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetSurface_isr
    ( BVDC_Source_Handle      hSource,
     const BAVC_Gfx_Picture  *pAvcGfxPic)
{
    BPXL_Format ePxlFmt;
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Source_SetSurface_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_ASSERT(NULL != pAvcGfxPic);

#if (BVDC_P_CHECK_MEMC_INDEX)
    /* Checking assumes memory is continous, possible problem when there is
     * hole in memory address. Disable the checking till we have correction
     * information from BCHP. */
    /* Check if gfx surface is on the correct memc based on boxmode */
    eStatus = BVDC_P_Source_CheckGfxMemcIndex_isr(hSource, pAvcGfxPic);
    if (BERR_SUCCESS != eStatus)
    {
        return BERR_TRACE(eStatus);
    }
#endif

    /* possible to be called inside display callback func, so don't assert:
       BDBG_ASSERT(0 == hSource->hVdc->ulInsideCs); */
    hSource->hVdc->ulInsideCs ++;

    if (NULL != hSource->hGfxFeeder && NULL != pAvcGfxPic)
    {
        eStatus = BVDC_P_GfxSurface_SetSurface_isr(
            &hSource->hGfxFeeder->stGfxSurface,
            &hSource->hGfxFeeder->stGfxSurface.stIsrSurInfo, pAvcGfxPic, hSource);
    }
    else if (NULL != hSource->hVfdFeeder && NULL != pAvcGfxPic)
    {
        ePxlFmt = pAvcGfxPic->pSurface->eFormat;
        if (!BPXL_IS_YCbCr422_FORMAT(ePxlFmt))
        {
            BDBG_ERR(("Invalid pixel format for VFD"));
            eStatus = BERR_INVALID_PARAMETER;
        }
        else
        {
            eStatus = BVDC_P_GfxSurface_SetSurface_isr(
                &hSource->hVfdFeeder->stGfxSurface,
                &hSource->hVfdFeeder->stGfxSurface.stIsrSurInfo, pAvcGfxPic, hSource);
        }
    }
    else if (NULL != hSource->hMpegFeeder && NULL != pAvcGfxPic)
    {
        ePxlFmt = pAvcGfxPic->pSurface->eFormat;
        if (!BPXL_IS_YCbCr422_FORMAT(ePxlFmt))
        {
            BDBG_ERR(("Invalid pixel format set for MFD"));
            eStatus = BERR_INVALID_PARAMETER;
        }
        else
        {
            eStatus = BVDC_P_GfxSurface_SetSurface_isr(
                &hSource->hMpegFeeder->stGfxSurface,
                &hSource->hMpegFeeder->stGfxSurface.stIsrSurInfo, pAvcGfxPic, hSource);
        }
    }
    else
    {
        BDBG_ERR(("Invalid source handle or gfx pic to set gfx surface."));
        eStatus = BERR_INVALID_PARAMETER;
    }

    hSource->hVdc->ulInsideCs --;

    BDBG_LEAVE(BVDC_Source_SetSurface_isr);
    return BERR_TRACE(eStatus);
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
*
*/
BERR_Code BVDC_Source_GetSurface
    ( BVDC_Source_Handle         hSource,
     BAVC_Gfx_Picture           *pAvcGfxPic)
{
    BERR_Code  eStatus = BERR_SUCCESS;
    BAVC_Gfx_Picture  *pGfxPic;

    BDBG_ENTER(BVDC_Source_GetSurface);

    if(NULL != pAvcGfxPic)
    {
        BKNI_EnterCriticalSection();
        BVDC_P_CHECK_CS_ENTER_VDC(hSource->hVdc);

        if (NULL != hSource->hGfxFeeder)
        {
            pGfxPic = BVDC_P_GfxSurface_GetSurfaceInHw_isr(
                &hSource->hGfxFeeder->stGfxSurface);
            *pAvcGfxPic = *pGfxPic;
        }
        else if (NULL != hSource->hVfdFeeder)
        {
            pGfxPic = BVDC_P_GfxSurface_GetSurfaceInHw_isr(
                &hSource->hVfdFeeder->stGfxSurface);
            *pAvcGfxPic = *pGfxPic;
        }
        else if (NULL != hSource->hMpegFeeder)
        {
            pGfxPic = BVDC_P_GfxSurface_GetSurfaceInHw_isr(
                &hSource->hMpegFeeder->stGfxSurface);
            *pAvcGfxPic = *pGfxPic;
        }
        else
        {
            BDBG_ERR(("Invalid source handle to get gfx surface."));
            eStatus = BERR_INVALID_PARAMETER;
        }

        BVDC_P_CHECK_CS_LEAVE_VDC(hSource->hVdc);
        BKNI_LeaveCriticalSection();
    }

    BDBG_LEAVE(BVDC_Source_GetSurface);
    return BERR_TRACE(eStatus);
}
#endif

/***************************************************************************
*
*/
BERR_Code BVDC_Source_GetSurface_isr
    ( BVDC_Source_Handle         hSource,
     BAVC_Gfx_Picture           *pAvcGfxPic)
{
    BERR_Code  eStatus = BERR_SUCCESS;
    BAVC_Gfx_Picture  *pGfxPic;

    BDBG_ENTER(BVDC_Source_GetSurface_isr);

    if(NULL != pAvcGfxPic)
    {
        /* possible to be called inside display callback func, so don't assert:
           BDBG_ASSERT(0 == hSource->hVdc->ulInsideCs); */
        hSource->hVdc->ulInsideCs ++;

        if (NULL != hSource->hGfxFeeder)
        {
            pGfxPic = BVDC_P_GfxSurface_GetSurfaceInHw_isr(
                &hSource->hGfxFeeder->stGfxSurface);
            *pAvcGfxPic = *pGfxPic;
        }
        else if (NULL != hSource->hVfdFeeder)
        {
            pGfxPic = BVDC_P_GfxSurface_GetSurfaceInHw_isr(
                &hSource->hVfdFeeder->stGfxSurface);
            *pAvcGfxPic = *pGfxPic;
        }
        else if (NULL != hSource->hMpegFeeder)
        {
            pGfxPic = BVDC_P_GfxSurface_GetSurfaceInHw_isr(
                &hSource->hMpegFeeder->stGfxSurface);
            *pAvcGfxPic = *pGfxPic;
        }
        else
        {
            BDBG_ERR(("Invalid source handle to get gfx surface."));
            eStatus = BERR_INVALID_PARAMETER;
        }

        hSource->hVdc->ulInsideCs--;
    }

    BDBG_LEAVE(BVDC_Source_GetSurface_isr);
    return BERR_TRACE(eStatus);
}

#if (BDBG_DEBUG_BUILD)
/***************************************************************************
 *
 */
static void BVDC_P_Source_PrintPicture_isr
    ( const BVDC_Source_Handle         hSource,
      const BAVC_MVD_Field            *pCurPic,
      const BAVC_MVD_Field            *pNewPic )
{
    uint32_t ulMsgCount;
    uint32_t ulPrintOnDelta;
    uint32_t ulDebugReg = BVDC_P_MPEG_DEBUG_SCRATCH(hSource->eId);

    ulMsgCount = BREG_Read32_isr(hSource->hVdc->hRegister, ulDebugReg);

    /* bit 1: print on mpeg-pic-delta;
     * other 31-bit: message count; */
    ulPrintOnDelta = ulMsgCount & 1;

    /* Check if different from previous field. */
    if(ulPrintOnDelta)
    {
        /* If interlaced make sure the polarity are not repeat. */
        if((false == pNewPic->bMute) &&
           (BAVC_Polarity_eFrame != pNewPic->eSourcePolarity) &&
           (pNewPic->eSourcePolarity == pCurPic->eSourcePolarity))
        {
            BDBG_ERR(("*** XDM sends repeated buffer (%s)!",
                (BAVC_Polarity_eTopField == pNewPic->eSourcePolarity) ? "T" :
                (BAVC_Polarity_eBotField == pNewPic->eSourcePolarity) ? "B" : "F"));
        }
        ulPrintOnDelta = (uint32_t)hSource->bPictureChanged ||
            BVDC_P_FIELD_DIFF(pNewPic, pCurPic, bStreamProgressive) ||
            BVDC_P_FIELD_DIFF(pNewPic, pCurPic, eMpegType) ||
            BVDC_P_FIELD_DIFF(pNewPic, pCurPic, eYCbCrType) ||
            BVDC_P_FIELD_DIFF(pNewPic, pCurPic, eAspectRatio) ||
            BVDC_P_FIELD_DIFF(pNewPic, pCurPic, eFrameRateCode) ||
            BVDC_P_FIELD_DIFF(pNewPic, pCurPic, eBitDepth) ||
            BVDC_P_FIELD_DIFF(pNewPic, pCurPic, stHdrMetadata.eType) ||
            BVDC_P_FIELD_DIFF(pNewPic, pCurPic, eMatrixCoefficients) ||
            BVDC_P_FIELD_DIFF(pNewPic, pCurPic, eChrominanceInterpolationMode);
    }

    if((ulMsgCount >= 2) || (ulPrintOnDelta))
    {
        BAVC_MVD_Field *pPic = (BAVC_MVD_Field *)pNewPic;
        uint32_t ulPicIdx = 0;
        do {
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("------------------------------------------ pic %d", ulPicIdx++));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("src_id                                     : mfd%d", hSource->eId));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->bMute                                : %d", pPic->bMute));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulAdjQp                              : %d", pPic->ulAdjQp));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->bCaptureCrc                          : %d", pPic->bCaptureCrc));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulOrigPTS                            : %d", pPic->ulOrigPTS));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulIdrPicID                           : %d", pPic->ulIdrPicID));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->int32_PicOrderCnt                    : %d", pPic->int32_PicOrderCnt));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eSourcePolarity                      : %d", pPic->eSourcePolarity));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eInterruptPolarity                   : %d", pPic->eInterruptPolarity));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eChrominanceIntMode                  : %d", pPic->eChrominanceInterpolationMode));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eMpegType                            : %d", pPic->eMpegType));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eYCbCrType                           : %d", pPic->eYCbCrType));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eAspectRatio                         : %d", pPic->eAspectRatio));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->uiSampleAspectRatioX                 : %d", pPic->uiSampleAspectRatioX));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->uiSampleAspectRatioY                 : %d", pPic->uiSampleAspectRatioY));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eFrameRateCode                       : %d", pPic->eFrameRateCode));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ePxlFmt                              : %s", BPXL_ConvertFmtToStr(pPic->ePxlFmt)));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eMatrixCoefficients                  : %d", pPic->eMatrixCoefficients));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ePreferredTransferCharacteristics    : %d", pPic->ePreferredTransferCharacteristics));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eTransferCharacteristics             : %d", pPic->eTransferCharacteristics));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->bStreamProgressive                   : %d", pPic->bStreamProgressive));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->bFrameProgressive                    : %d", pPic->bFrameProgressive));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->hLuminanceFrameBufferBlock           : %p", (void *)pPic->hLuminanceFrameBufferBlock));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulLuminanceFrameBufferBlockOffset    : 0x%08x", pPic->ulLuminanceFrameBufferBlockOffset));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->hChrominanceFrameBufferBlock         : %p", (void *)pPic->hChrominanceFrameBufferBlock));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulChrominanceFrameBufferBlockOffset  : 0x%08x", pPic->ulChrominanceFrameBufferBlockOffset));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulRowStride                          : 0x%08x", pPic->ulRowStride));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulLuminanceNMBY                      : %d", pPic->ulLuminanceNMBY));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulChrominanceNMBY                    : %d", pPic->ulChrominanceNMBY));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulLumaRangeRemapping                 : 0x%x", pPic->ulLumaRangeRemapping));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulChromaRangeRemapping               : 0x%x", pPic->ulChromaRangeRemapping));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulDisplayHorizontalSize              : %d", pPic->ulDisplayHorizontalSize));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulDisplayVerticalSize                : %d", pPic->ulDisplayVerticalSize));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulSourceHorizontalSize               : %d", pPic->ulSourceHorizontalSize));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulSourceVerticalSize                 : %d", pPic->ulSourceVerticalSize));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->i32_HorizontalPanScan                : %d", pPic->i32_HorizontalPanScan));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->i32_VerticalPanScan                  : %d", pPic->i32_VerticalPanScan));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulSourceClipLeft                     : %d", pPic->ulSourceClipLeft));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulSourceClipTop                      : %d", pPic->ulSourceClipTop));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->bPictureRepeatFlag                   : %d", pPic->bPictureRepeatFlag));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulChannelId                          : %d", pPic->ulChannelId));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eStripeWidth                         : %d", pPic->eStripeWidth));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->bIgnoreCadenceMatch                  : %d", pPic->bIgnoreCadenceMatch));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->bValidAfd                            : %d", pPic->bValidAfd));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulAfd                                : %d", pPic->ulAfd));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eBarDataType                         : %d", pPic->eBarDataType));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulTopLeftBarValue                    : %d", pPic->ulTopLeftBarValue));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulBotRightBarValue                   : %d", pPic->ulBotRightBarValue));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eBitDepth                            : %u", pPic->eBitDepth));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eChromaBitDepth                      : %u", pPic->eChromaBitDepth));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eBufferFormat                        : %d", pPic->eBufferFormat));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulMaxContentLight                    : %d", pPic->ulMaxContentLight));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulAvgContentLight                    : %d", pPic->ulAvgContentLight));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulMaxDispMasteringLuma               : %d", pPic->ulMaxDispMasteringLuma));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->stHdrMetadata.eType                  : %d", pPic->stHdrMetadata.eType));
            if(BAVC_HdrMetadataType_eUnknown != pPic->stHdrMetadata.eType) {
                BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->stHdrMetadata.pData                  : %p", (void *)pPic->stHdrMetadata.pData));
                BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->stHdrMetadata.ulSize                 : %u", pPic->stHdrMetadata.ulSize));
            }
            if(BAVC_DecodedPictureBuffer_eFieldsPair == pPic->eBufferFormat) {
                BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->hLuminanceBotFieldBlock              : %p", (void *)pPic->hLuminanceBotFieldBufferBlock));
                BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulLuminanceBotFieldBlockOffset       : 0x%08x", pPic->ulLuminanceBotFieldBufferBlockOffset));
                BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->hChrominanceBotFieldBlock            : %p", (void *)pPic->hChrominanceBotFieldBufferBlock));
                BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->ulChrominanceBotFieldBlockOffset     : 0x%08x", pPic->ulChrominanceBotFieldBufferBlockOffset));
            }
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->eOrientation                         : %d", pPic->eOrientation));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->pNext                                : %p", pPic->pNext));
            BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->pEnhanced                            : %p", pPic->pEnhanced));
            if(pPic->pEnhanced)
            {
                BAVC_MVD_Field *pEnhanced = (BAVC_MVD_Field *)pPic->pEnhanced;

                BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->pEnhanced->eOrientation                         : %d",     pEnhanced->eOrientation));
                BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->pEnhanced->hLuminanceFrameBufferBlock           : %p", (void *)pEnhanced->hLuminanceFrameBufferBlock));
                BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->pEnhanced->ulLuminanceFrameBufferBlockOffset    : 0x%08x", pEnhanced->ulLuminanceFrameBufferBlockOffset));
                BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->pEnhanced->hChrominanceFrameBufferBlock         : %p", (void *)pEnhanced->hChrominanceFrameBufferBlock));
                BDBG_MODULE_MSG(BVDC_SRC_DELTA, ("pPic->pEnhanced->ulChrominanceFrameBufferBlockOffset  : 0x%08x", pEnhanced->ulChrominanceFrameBufferBlockOffset));
                BSTD_UNUSED(pEnhanced);
            }
            pPic = pPic->pNext;
            BSTD_UNUSED(ulPicIdx);
        }while (pPic);

        /* keep on printing until we get to zero, and update field. */
        if(ulMsgCount >= 2)
        {
            BREG_Write32_isr(hSource->hVdc->hRegister, ulDebugReg, ulMsgCount - 2);
        }
    }
    return;
}
#endif

#define BVDC_P_REFRESH_RATE_MISMATCHES_THRESHOLD   6
/**************************************************************************
 *
 */
static void BVDC_P_Source_ValidateMpegData_isr
    ( BVDC_Source_Handle         hSource,
      BAVC_MVD_Field            *pNewPic,
      const BAVC_MVD_Field      *pCurPic)
{
#if !(BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for legacy MFD that doesn't support fields pair */
    if(pNewPic->eBufferFormat == BAVC_DecodedPictureBuffer_eFieldsPair) {
        /* if forced progressive scanout of interlaced fields pair buffers, downgraded to top-only scanout. */
        if(pNewPic->eSourcePolarity == BAVC_Polarity_eFrame) {
            BDBG_WRN(("This chip doesn't support fields pair buffers in progressive scanout! Downgraded to top-only!"));
            pNewPic->eSourcePolarity = BAVC_Polarity_eTopField;
        }
    }
#endif

    /* SW7425-686 solve XVD and BVN refresh rate mismatch problem */
    if(hSource->hSyncLockCompositor)
    {
        const BFMT_VideoInfo *pFmtInfo;
        uint32_t ulSrcVerq = 0;

        pFmtInfo = hSource->hSyncLockCompositor->hDisplay->stCurInfo.pFmtInfo;
        hSource->ulRefreshRateMismatchCntr = 0;

        if(pNewPic->bMute == false)
        {
            switch(pNewPic->eInterruptRefreshRate)
            {
                case(BFMT_Vert_e7_493Hz):
                    ulSrcVerq = 749; break;
                case(BFMT_Vert_e7_5Hz):
                    ulSrcVerq = 750; break;
                case(BFMT_Vert_e9_99Hz):
                    ulSrcVerq = 999; break;
                case(BFMT_Vert_e10Hz):
                    ulSrcVerq = 1000; break;
                case(BFMT_Vert_e11_988Hz):
                    ulSrcVerq = 1199; break;
                case(BFMT_Vert_e12Hz):
                    ulSrcVerq = 1200; break;
                case(BFMT_Vert_e12_5Hz):
                    ulSrcVerq = 1250; break;
                case(BFMT_Vert_e14_985Hz):
                    ulSrcVerq = 1498; break;
                case(BFMT_Vert_e15Hz):
                    ulSrcVerq = 1500; break;
                case(BFMT_Vert_e19_98Hz):
                    ulSrcVerq = 1998; break;
                case(BFMT_Vert_e20Hz):
                    ulSrcVerq = 2000; break;
                case(BFMT_Vert_e23_976Hz):
                    ulSrcVerq = 2397; break;
                case(BFMT_Vert_e24Hz):
                    ulSrcVerq = 2400; break;
                case(BFMT_Vert_e25Hz):
                    ulSrcVerq = 2500; break;
                case(BFMT_Vert_e29_97Hz):
                    ulSrcVerq = 2997; break;
                case(BFMT_Vert_e30Hz):
                    ulSrcVerq = 3000; break;
                case(BFMT_Vert_e48Hz):
                    ulSrcVerq = 4800; break;
                case(BFMT_Vert_e50Hz):
                    ulSrcVerq = 5000; break;
                case(BFMT_Vert_e59_94Hz):
                    ulSrcVerq = 5994; break;
                case(BFMT_Vert_e60Hz):
                    ulSrcVerq = 6000; break;
                case(BFMT_Vert_e100Hz):
                    ulSrcVerq = 10000; break;
                case(BFMT_Vert_e119_88Hz):
                    ulSrcVerq = 11988; break;
                case(BFMT_Vert_e120Hz):
                    ulSrcVerq = 12000; break;

                case(BFMT_Vert_eInvalid):
                case(BFMT_Vert_eLast):
                default:
                /* @@@ SW7425-832: set SrcVerq to Display fresh rate
                when BFMT_Vert_eInvalid or BFMT_Vert_eLast.
                Need to check with XVD why it is invalid number */
                ulSrcVerq = pFmtInfo->ulVertFreq; break;
            }

            if(!BVDC_P_EQ_DELTA (ulSrcVerq, pFmtInfo->ulVertFreq, 20)) {
                hSource->ulRefreshRateMismatchCntr++;
                if (hSource->ulRefreshRateMismatchCntr >= BVDC_P_REFRESH_RATE_MISMATCHES_THRESHOLD) {
                    BDBG_WRN(("XVD -> BVN refresh rate mismatch! XVD %4d x %4d @ %4d BVN %4d x %4d @ %4d",
                        pNewPic->ulSourceHorizontalSize, pNewPic->ulSourceVerticalSize, pNewPic->eInterruptRefreshRate,
                        pFmtInfo->ulDigitalWidth, pFmtInfo->ulDigitalHeight, pFmtInfo->ulVertFreq));
                }
            }
            else {
                hSource->ulRefreshRateMismatchCntr = 0;
            }
        }
    }

    /* Not gfx surface displayed as mpeg buffer case:
     * if null address or size => force mute */
    /* if requested to capture CRC, don't adjust original source size */
    if((!pNewPic->bCaptureCrc) && (pNewPic->ePxlFmt == BPXL_INVALID) &&
       ((pNewPic->ulSourceHorizontalSize < BVDC_P_SRC_INPUT_H_MIN) ||
        (pNewPic->ulSourceVerticalSize   < BVDC_P_SRC_INPUT_V_MIN) ||
        (!pNewPic->hLuminanceFrameBufferBlock) ||
        (!pNewPic->hChrominanceFrameBufferBlock)))
    {
        pNewPic->ulSourceHorizontalSize  = BVDC_P_SRC_INPUT_H_MIN;
        pNewPic->ulSourceVerticalSize    = BVDC_P_SRC_INPUT_V_MIN;
        pNewPic->ulDisplayHorizontalSize = BVDC_P_SRC_INPUT_H_MIN;
        pNewPic->ulDisplayVerticalSize   = BVDC_P_SRC_INPUT_V_MIN;
        pNewPic->bMute = true;
    }

    /* Avoid invalid size that can lock up BVN when muted */
    if(!pNewPic->bCaptureCrc || pNewPic->bMute)
    {
        /* (1) Source/display size are invalid.  Corrected them here.  Non-zero. */
        if((pNewPic->ulSourceHorizontalSize  < BVDC_P_SRC_INPUT_H_MIN) ||
           (pNewPic->ulSourceVerticalSize    < BVDC_P_SRC_INPUT_V_MIN) ||
           (pNewPic->ulDisplayHorizontalSize < BVDC_P_SRC_INPUT_H_MIN) ||
           (pNewPic->ulDisplayVerticalSize   < BVDC_P_SRC_INPUT_V_MIN))
        {
            pNewPic->ulSourceHorizontalSize  = BVDC_P_SRC_INPUT_H_MIN;
            pNewPic->ulSourceVerticalSize    = BVDC_P_SRC_INPUT_V_MIN;
            pNewPic->ulDisplayHorizontalSize = BVDC_P_SRC_INPUT_H_MIN;
            pNewPic->ulDisplayVerticalSize   = BVDC_P_SRC_INPUT_V_MIN;
        }


#if (BVDC_P_SUPPORT_DTG_RMD) && (!BVDC_P_SUPPORT_DSCL) && (!BVDC_P_SUPPORT_4kx2k_60HZ)
        /* Can only handle 4kx2k@30, always enable PSF for 4kx2k source */
        if((BAVC_Polarity_eFrame == pNewPic->eSourcePolarity) &&
           (pNewPic->ulSourceVerticalSize  > BFMT_1080P_HEIGHT) &&
           (!hSource->hMpegFeeder->bGfxSrc))
        {
            hSource->bEnablePsfBySize = true;
        }
        else
#endif
        {
            hSource->bEnablePsfBySize = false;
        }
        /* 4K source cannot scan out in 100hz! Fall back to PsF in case no MTG. TODO: MTG is favored. */
        if((BAVC_Polarity_eFrame == pNewPic->eSourcePolarity) &&
           (pNewPic->ulSourceVerticalSize  > (BFMT_720P_HEIGHT*2)) &&
           (hSource->ulDispVsyncFreq >= 100*BFMT_FREQ_FACTOR)) {
            hSource->bEnablePsfBySize = true;
        }

        /* (4) If interlaced stream vertical size must be even */
        if(BAVC_Polarity_eFrame != pNewPic->eSourcePolarity)
        {
            /* Jira SW7405-4239: XMD might send odd width for special aspect ratio purpose
               pNewPic->ulSourceHorizontalSize = pNewPic->ulSourceHorizontalSize & ~1; */
            pNewPic->ulSourceVerticalSize   = pNewPic->ulSourceVerticalSize & ~1;
        }
    }

    /* (4) Handle unknown color space.  In the cases of 'unknown', 'forbidden'
     * and 'reserved' values passed from MVD, we display the picture according
     * to its frame size, i.e. if frame size > 720x576, treat it as default
     * HD(BT.709) color; else treat it as NTSC or PAL SD. */
    if((BAVC_MatrixCoefficients_eFCC != pNewPic->eMatrixCoefficients) &&
       (BAVC_MatrixCoefficients_eSmpte_170M != pNewPic->eMatrixCoefficients) &&
       (BAVC_MatrixCoefficients_eSmpte_240M != pNewPic->eMatrixCoefficients) &&
       (BAVC_MatrixCoefficients_eItu_R_BT_709 != pNewPic->eMatrixCoefficients) &&
       (BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG != pNewPic->eMatrixCoefficients) &&
       (BAVC_MatrixCoefficients_eItu_R_BT_2020_CL != pNewPic->eMatrixCoefficients) &&
       (BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL != pNewPic->eMatrixCoefficients))
    {
        const BVDC_Settings *pDefSetting = &hSource->hVdc->stSettings;
        if(pNewPic->ulSourceVerticalSize > BFMT_PAL_HEIGHT)
        {
            pNewPic->eMatrixCoefficients = pDefSetting->eColorMatrix_HD;
        }
        else if(pNewPic->ulSourceVerticalSize > BFMT_480P_HEIGHT)
        {
            pNewPic->eMatrixCoefficients = BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG;
        }
        else
        {
            pNewPic->eMatrixCoefficients = pDefSetting->eColorMatrix_SD;
        }
    }

#ifdef BVDC_MPEG_SOURCE_MAD_DEBUG
    /* Note:
     * 1. if mvd passes frame to vdc, we need to be intelligent here;
     * 2. progressive sequence is true progressive material, so we don't need to
     *    detect and force scanout fields second hand for deinterlacer;
     * 3. interlaced sequence could decode into fields or frames; there is no harm
     *    to force scanout fields for deinterlacer; */
    if(!pNewPic->bCaptureCrc || pNewPic->bMute)
    {
        if((pNewPic->eSourcePolarity == BAVC_Polarity_eFrame) &&
           !pNewPic->bStreamProgressive)
        {
            if((hSource->stCurInfo.bDeinterlace) &&
               (pNewPic->ulSourceVerticalSize   <= BFMT_1080I_HEIGHT)
              )
            {
                BDBG_WRN(("Force scanout field instead of frame"));
                /* force the field data source polarity */
                pNewPic->eSourcePolarity = BVDC_P_NEXT_POLARITY(pCurPic->eSourcePolarity);
            }
        }
    }
#endif

    /* HDR source is scanned as progressive until deinterlacer tells 10-bit capability */
    if(!pNewPic->bMute &&
        BAVC_Polarity_eFrame != pNewPic->eSourcePolarity &&
        BAVC_TransferCharacteristics_eSmpte_ST_2084 == pNewPic->eTransferCharacteristics
        )
    {
        BDBG_MSG(("MFD%d normally should not scan out interlaced for HDR content.", hSource->eId));
        pNewPic->eSourcePolarity = BAVC_Polarity_eFrame;/* override! assume interlaced source must be non-HDR */
    }

     /* BP3 Do NOT Modify Start */
    if(BAVC_HdrMetadataType_eDrpu == pNewPic->stHdrMetadata.eType)
    {
        if(BERR_SUCCESS != BCHP_HasLicensedFeature_isrsafe(hSource->hVdc->hChip,
            BCHP_LicensedFeature_eDolbyVision))
        {
            BDBG_MSG(("Invalid Dolby Vision license content."));
            pNewPic->stHdrMetadata.eType = BAVC_HdrMetadataType_eUnknown;
        }
    }

    if((BAVC_HdrMetadataType_eTch_Cri == pNewPic->stHdrMetadata.eType)||
       (BAVC_HdrMetadataType_eTch_Cvri == pNewPic->stHdrMetadata.eType))
    {
        if(BERR_SUCCESS != BCHP_HasLicensedFeature_isrsafe(hSource->hVdc->hChip,
            BCHP_LicensedFeature_eTchPrime))
        {
            BDBG_MSG(("Invalid Technicolor Prime license content."));
            pNewPic->stHdrMetadata.eType = BAVC_HdrMetadataType_eUnknown;
        }
    }
    /* BP3 Do NOT Modify End */

#if BVDC_P_DBV_SUPPORT
    /* DBV only supports progressive source; TODO: if source is 10-bit, 8-bit
     * deinterlacer should be disabled; */
    if(BAVC_HdrMetadataType_eDrpu == pNewPic->stHdrMetadata.eType)
    {
       pNewPic->eSourcePolarity = BAVC_Polarity_eFrame;
    }
#endif

    /* MVD should not give VDC a bad interrupt field somehing like
     * 5 or -1.  It should giving the interrupt that VDC interrupted them
     * with.   For mute pictures everything about field buffer can be
     * invalid, but not the interrupt field id. */
    BDBG_ASSERT(
        (BAVC_Polarity_eTopField==pNewPic->eInterruptPolarity) ||
        (BAVC_Polarity_eBotField==pNewPic->eInterruptPolarity) ||
        (BAVC_Polarity_eFrame   ==pNewPic->eInterruptPolarity));

    /* Chroma pan-scan/src-clip for H.264 */
    if(pNewPic->eYCbCrType == BAVC_YCbCrType_e4_4_4)
    {
        BDBG_ERR(("eYCbCrType 4:4:4 is not supported by MFD!"));
        BDBG_ASSERT(0);
    }

    /* Check if fields that are to be written to registers fit
     * within their field sizes. */
    if((!hSource->hMpegFeeder->bGfxSrc) &&
        (pNewPic->eSourcePolarity != BAVC_Polarity_eFrame) &&
        (!BVDC_P_SRC_VALIDATE_FIELD(MFD_0_LAC_CNTL, OUTPUT_FIELD_POLARITY,
                                    pNewPic->eSourcePolarity)))
    {
        BDBG_ERR(("eSourcePolarity is invalid"));
        pNewPic->eSourcePolarity = BAVC_Polarity_eTopField;
    }

    /* Muted channel doesn't need to check the following things! */
    if(!pNewPic->bMute)
    {
        if(!BVDC_P_SRC_VALIDATE_FIELD(MFD_0_LAC_CNTL, CHROMA_TYPE,
                                       pNewPic->eYCbCrType - BAVC_YCbCrType_e4_2_0))
        {
            BDBG_ERR(("eYCbCrType is invalid"));
            pNewPic->eYCbCrType = BAVC_YCbCrType_e4_2_0;
        }

        if(!BVDC_P_SRC_VALIDATE_FIELD(MFD_0_LAC_CNTL, CHROMA_INTERPOLATION,
                                       pNewPic->eChrominanceInterpolationMode))
        {
            BDBG_ERR(("eChrominanceInterpolationMode is invalid"));
            pNewPic->eChrominanceInterpolationMode = BAVC_InterpolationMode_eField;
        }

        if((!hSource->hMpegFeeder->bGfxSrc) &&
            !BVDC_P_SRC_VALIDATE_FIELD(MFD_0_LUMA_NMBY, VALUE, pNewPic->ulLuminanceNMBY))
        {
            BDBG_ERR(("ulLuminanceNMBY is invalid: %d", pNewPic->ulLuminanceNMBY));
            pNewPic->ulLuminanceNMBY = BVDC_P_SRC_FIELD_DATA_MASK(MFD_0_LUMA_NMBY, VALUE,
                                                                  pNewPic->ulLuminanceNMBY);
        }

        if((!hSource->hMpegFeeder->bGfxSrc) &&
            !BVDC_P_SRC_VALIDATE_FIELD(MFD_0_CHROMA_NMBY, VALUE, pNewPic->ulChrominanceNMBY))
        {
            BDBG_ERR(("ulChrominanceNMBY is invalid: %d", pNewPic->ulChrominanceNMBY));
            pNewPic->ulChrominanceNMBY = BVDC_P_SRC_FIELD_DATA_MASK(MFD_0_CHROMA_NMBY, VALUE,
                                                                    pNewPic->ulChrominanceNMBY);
        }

        /* Only support BAVC_ChromaLocation_eType0 to BAVC_ChromaLocation_eType3 for now */
        if(pNewPic->eMpegType > BAVC_ChromaLocation_eType3)
        {
            BDBG_WRN(("eMpegType is invalid"));
            pNewPic->eMpegType = BAVC_MpegType_eMpeg2;

        }
    }

    /* Check if there are general changes from previous pictures. */
    if(BVDC_P_FIELD_DIFF(pNewPic, pCurPic, bMute) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, ulSourceHorizontalSize) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, ulSourceVerticalSize) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, ulDisplayHorizontalSize) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, ulDisplayVerticalSize) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, eOrientation) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, uiSampleAspectRatioX) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, uiSampleAspectRatioY) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, eAspectRatio) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, bValidAfd) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, ulAfd) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, eBarDataType) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, ulTopLeftBarValue) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, ulBotRightBarValue) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, bStreamProgressive) ||
       BVDC_P_FIELD_DIFF(pNewPic, pCurPic, bIgnoreCadenceMatch))
    {
        hSource->bPictureChanged = true;

        BDBG_MSG(("bMute changes: %d->%d", pCurPic->bMute, pNewPic->bMute));
        BDBG_MSG(("ulSourceHorizontalSize changes: %d->%d", pCurPic->ulSourceHorizontalSize, pNewPic->ulSourceHorizontalSize));
        BDBG_MSG(("ulSourceVerticalSize changes: %d->%d", pCurPic->ulSourceVerticalSize, pNewPic->ulSourceVerticalSize));
        BDBG_MSG(("ulDisplayHorizontalSize changes: %d->%d", pCurPic->ulDisplayHorizontalSize, pNewPic->ulDisplayHorizontalSize));
        BDBG_MSG(("ulDisplayVerticalSize changes: %d->%d", pCurPic->ulDisplayVerticalSize, pNewPic->ulDisplayVerticalSize));
        BDBG_MSG(("eOrientation changes: %d->%d", pCurPic->eOrientation, pNewPic->eOrientation));
        BDBG_MSG(("uiSampleAspectRatioX changes: %d->%d", pCurPic->uiSampleAspectRatioX, pNewPic->uiSampleAspectRatioX));
        BDBG_MSG(("uiSampleAspectRatioY changes: %d->%d", pCurPic->uiSampleAspectRatioY, pNewPic->uiSampleAspectRatioY));
        BDBG_MSG(("eAspectRatio changes: %d->%d", pCurPic->eAspectRatio, pNewPic->eAspectRatio));
        BDBG_MSG(("bValidAfd changes: %d->%d", pCurPic->bValidAfd, pNewPic->bValidAfd));
        BDBG_MSG(("ulAfd changes: %d->%d", pCurPic->ulAfd, pNewPic->ulAfd));
        BDBG_MSG(("eBarDataType changes: %d->%d", pCurPic->eBarDataType, pNewPic->eBarDataType));
        BDBG_MSG(("ulTopLeftBarValue changes: %d->%d", pCurPic->ulTopLeftBarValue, pNewPic->ulTopLeftBarValue));
        BDBG_MSG(("ulBotRightBarValue changes: %d->%d", pCurPic->ulBotRightBarValue, pNewPic->ulBotRightBarValue));
        BDBG_MSG(("bIgnoreCadenceMatch changes: %d->%d", pCurPic->bIgnoreCadenceMatch, pNewPic->bIgnoreCadenceMatch));
    }

#if (BCHP_CHIP==7250)
    if((hSource->hVdc->stBoxConfig.stBox.ulBoxId==8) &&
        (hSource->stCurInfo.bMosaicMode && (hSource->ulMosaicCount>1)))
    {
        hSource->bPictureChanged = true;
    }
#endif

    /* I -> P, or P -> I, for non-muted pictures. */
    if((pNewPic->eSourcePolarity | BAVC_Polarity_eBotField) !=
       (pCurPic->eSourcePolarity | BAVC_Polarity_eBotField))
    {
        hSource->bPictureChanged = true;
        hSource->bRasterChanged  = true;
        hSource->stCurInfo.stDirty.stBits.bInputFormat = BVDC_P_DIRTY;
        BDBG_MSG(("mpeg source raster change: old %d -> new %d", pCurPic->eSourcePolarity, pNewPic->eSourcePolarity));
    }

    /* Check if anything wrong with mvd field. */
#if (BDBG_DEBUG_BUILD)
    BVDC_P_Source_PrintPicture_isr(hSource, pCurPic, pNewPic);
#endif

    return;
}

/**************************************************************************
 * Update framerate code for MFD
 *      hSource->eMfdVertRateCode is used in next vsync to configure MTG,
 *      and in callback to upper layer for BXVD_SetMonitorRefreshRate
 */
static void BVDC_P_Source_UpdateMfdVertRateCode_isr
    ( BVDC_Source_Handle         hSource,
      BAVC_MVD_Field            *pXvdPic )
{
    uint32_t            ulCurVertFreq;

#if (!BVDC_P_SUPPORT_MTG)
    BSTD_UNUSED(pXvdPic);
#endif

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    ulCurVertFreq = hSource->ulVertFreq;

    if (hSource->hSyncLockCompositor)
    {
        /* BVDC_P_Source_RefreshRateCode_FromRefreshRate_isrsafe is not efficient */
        if (ulCurVertFreq != hSource->hSyncLockCompositor->hDisplay->stCurInfo.ulVertFreq)
        {
            hSource->eMfdVertRateCode = BVDC_P_Source_RefreshRateCode_FromRefreshRate_isrsafe(
                hSource->hSyncLockCompositor->hDisplay->stCurInfo.ulVertFreq);
        }
    }
#if BVDC_P_SUPPORT_MTG
    else if (hSource->bMtgSrc && hSource->bUsedMadAtWriter)
    {
        BAVC_FrameRateCode eMtgVertRateCode;
        eMtgVertRateCode =
            BVDC_P_Source_MtgRefreshRate_FromFrameRateCode_isrsafe(hSource, pXvdPic->eFrameRateCode);
        /* in case content does not provide valid eFrameRateCode */
        if (BAVC_FrameRateCode_eUnknown != eMtgVertRateCode)
        {
            hSource->eMfdVertRateCode = eMtgVertRateCode;
        }
        else if (NULL != hSource->hMfdVertDrivingWin)
        {
            uint32_t ulVertFrq = BVDC_P_ROUND_OFF(hSource->hMfdVertDrivingWin->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
                                                  BFMT_FREQ_FACTOR/2, BFMT_FREQ_FACTOR);
            hSource->eMfdVertRateCode =
                ((ulVertFrq == 25) || (ulVertFrq == 50))? BAVC_FrameRateCode_e50 :
                (hSource->hMfdVertDrivingWin->hCompositor->bFullRate)? BAVC_FrameRateCode_e60 : BAVC_FrameRateCode_e59_94;
        }
    }
#endif
    else if (NULL != hSource->hMfdVertDrivingWin)
    {
        /* Note: Refer to SW7445-2809, when MAD is not used, and when src and disp are both 24 hz progressive,
         * if MTG is running at higher rate, XDM needs to repeat, and VDC needs to drop, but VDC could drop
         * the wrong one and then repeat --> motion jitter. So we now make MTG run at display frequency until
         * we use bRepeat bit from XDM in multi-bufering */
#if BVDC_P_SUPPORT_MTG
        if (hSource->bMtgSrc)
        {
            uint32_t ulVertFrq = BVDC_P_ROUND_OFF(hSource->hMfdVertDrivingWin->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
                                                  BFMT_FREQ_FACTOR/2, BFMT_FREQ_FACTOR);
            hSource->eMfdVertRateCode =
                ((ulVertFrq == 25) || (ulVertFrq == 50))?     BAVC_FrameRateCode_e50 :
                (hSource->hMfdVertDrivingWin->hCompositor->bFullRate)? BAVC_FrameRateCode_e60 : BAVC_FrameRateCode_e59_94;

            /* don't support 4K100 scanout: only use high MTG rate within MFD throughput limit */
            if(ulVertFrq >= 100 && pXvdPic->ulSourceVerticalSize <= (BFMT_720P_HEIGHT * 2)) {
                hSource->eMfdVertRateCode =
                    (ulVertFrq == 100)? BAVC_FrameRateCode_e100 :
                    (hSource->hMfdVertDrivingWin->hCompositor->bFullRate)? BAVC_FrameRateCode_e120 : BAVC_FrameRateCode_e119_88;
            }
        }
        else
#endif
        if (ulCurVertFreq != hSource->hMfdVertDrivingWin->hCompositor->hDisplay->stCurInfo.ulVertFreq)
        {
            hSource->eMfdVertRateCode = BVDC_P_Source_RefreshRateCode_FromRefreshRate_isrsafe(
                hSource->hMfdVertDrivingWin->hCompositor->hDisplay->stCurInfo.ulVertFreq);
        }
    }
    else
    {
        /* else no window is connected to the src yet */
        hSource->eMfdVertRateCode = hSource->eDefMfdVertRateCode;
    }

    hSource->ulVertFreq = BVDC_P_Source_RefreshRate_FromFrameRateCode_isrsafe(hSource->eMfdVertRateCode);
}

/***************************************************************************
 * Application calls this function at every interrupt to set source info.
 *
 * Called to pass infomation from MVD to VDC.  For non-mpeg source
 * pNewPic will be NULL.   The data is use to build for the next
 * display vsync (or possibly queue up).
 * pvSourceHandle is (BVDC_Source_Handle).
 *
 */
void BVDC_Source_MpegDataReady_isr
    ( void                            *pvSourceHandle,
      int                              iParm2,
      void                            *pvMvdField )
{
    uint32_t i;
    BVDC_Source_Handle hSource = (BVDC_Source_Handle)pvSourceHandle;
    BVDC_Window_Handle hWindow;
    BRDC_Slot_Handle hSlot;/* master slot */
    BRDC_List_Handle hList;
    BAVC_MVD_Field *pCurPic;
    BAVC_MVD_Field *pNewPic;
    uint32_t ulOldEntries;
    BVDC_P_Source_DirtyBits *pCurDirty, *pOldDirty;
    bool bBuildBoth = false;
    uint32_t ulPictureIdx = 0;
    uint32_t ulSubRulEntries = 0;
    BRDC_Slot_Handle hSlotSlave = NULL;
    BRDC_List_Handle hListSlave = NULL;
    BVDC_P_ListInfo stMasterList, stList;
    BRDC_Trigger eTrigger = 0;
    uint32_t ulMosaicCount = 0;
    uint32_t ulConnectedWindow = 0; /* number windows for this source */
    bool bNoScanout = false; /* for PsF */
    BVDC_P_Source_Info *pNewInfo = &hSource->stNewInfo;
    BVDC_P_Source_Info *pCurInfo = &hSource->stCurInfo;
    BAVC_Polarity       ePrePolarity;
    BAVC_MVD_Field   *pXvdPic, *pVdcPic;
    bool   bChannelSet[BAVC_MOSAIC_MAX];

    BDBG_ENTER(BVDC_Source_MpegDataReady_isr);
    BSTD_UNUSED(iParm2);

    /* Casting to get parms */
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hSource->hVdc, BVDC_VDC);
    pCurDirty = &hSource->stCurInfo.stDirty;

    /* Make sure the BKNI enter/leave critical section works. */
    BVDC_P_CHECK_CS_ENTER_VDC(hSource->hVdc);

    /* We should not even get the Gfx or analog source here. */
    BDBG_ASSERT(BVDC_P_SRC_IS_MPEG(hSource->eId));

    pXvdPic = (BAVC_MVD_Field*)pvMvdField;

    /* Update source user info before using pCurInfo */
    BVDC_P_Source_UpdateSrcState_isr(hSource);
    BVDC_P_Source_UpdateMfdVertRateCode_isr(hSource, pXvdPic);

    if(hSource->stCurInfo.bMosaicMode)
    {
        /* Copy link list based on zorder */
        for(i = 0; i < hSource->ulMosaicCount; i++)
        {
            bChannelSet[i] = false;
            hSource->stNewPic[i].bMute = true;
            hSource->stNewPic[i].ulChannelId = i;
            hSource->stNewPic[i].eInterruptPolarity = pXvdPic->eInterruptPolarity;
        }
    }

    pVdcPic = pNewPic = &(hSource->stNewPic[0]);
    pCurPic = &hSource->stPrevMvdField;

    while(pXvdPic)
    {
        /* re-order pic list for z-order */
        uint32_t  ulIndex=0;
        bool bNewInterlaced, bCurInterlaced;

        if(hSource->stCurInfo.bMosaicMode)
        {
            ulIndex = hSource->aulMosaicZOrderIndex[pXvdPic->ulChannelId];
            pVdcPic = &(hSource->stNewPic[ulIndex]);
        }
        bNewInterlaced = pXvdPic->eSourcePolarity != BAVC_Polarity_eFrame;
        bCurInterlaced = pVdcPic->eSourcePolarity != BAVC_Polarity_eFrame;

        /* When input vertical size is odd, we round up to even lines for MFD
             * scan out, then use scaler to cut the last line. Scaler needs to
             * drain the last line at the end of picture. This drian may cause
             * scaler input completes after its output. This will cause scaler
             * enable error in mosaic mode */
        pXvdPic->ulSourceVerticalSize  = pXvdPic->ulSourceVerticalSize & ~1;
        pXvdPic->ulDisplayVerticalSize = pXvdPic->ulDisplayVerticalSize & ~1;

        /* If we receiving 1088i stream we will assume it contains the
         * non-active video in the last 8 lines.  We'll drop them.  PR10698.  Or
         * any value that might break the VDC (restricting to HW limit, not
         * necessary rts limit). */
        if((!pXvdPic->bCaptureCrc)
           && ((1088 == pXvdPic->ulSourceVerticalSize)
#ifdef BCHP_MFD_0_DISP_VSIZE_VALUE_MASK
           || (!BVDC_P_SRC_VALIDATE_FIELD(MFD_0_DISP_VSIZE, VALUE, pXvdPic->ulSourceVerticalSize))
#endif
#ifdef BCHP_MFD_0_DISP_HSIZE_VALUE_MASK
           || (!BVDC_P_SRC_VALIDATE_FIELD(MFD_0_DISP_HSIZE, VALUE, pXvdPic->ulSourceHorizontalSize))
#endif
#ifdef BCHP_MFD_0_PICTURE0_DISP_VERT_WINDOW_START_MASK
           || (!BVDC_P_SRC_VALIDATE_FIELD(MFD_0_PICTURE0_DISP_VERT_WINDOW, START, pXvdPic->ulSourceVerticalSize))
#endif
          ))
        {
            pXvdPic->ulSourceVerticalSize    =
                BVDC_P_MIN(pXvdPic->ulSourceVerticalSize,    BFMT_1080I_HEIGHT);
            pXvdPic->ulSourceHorizontalSize  =
                BVDC_P_MIN(pXvdPic->ulSourceHorizontalSize,  BFMT_1080I_WIDTH);
        }
#if BVDC_P_SUPPORT_MOSAIC_MODE
        if ((pVdcPic->ulSourceHorizontalSize !=pXvdPic->ulSourceHorizontalSize) ||
            ((pVdcPic->ulSourceVerticalSize >> bCurInterlaced)!=
            (pXvdPic->ulSourceVerticalSize >> bNewInterlaced)))
        {
            if (pVdcPic->bMute && pXvdPic->bMute)
            {
                bool bHmin, bVmin;

                bHmin = (BVDC_P_MAX(pVdcPic->ulSourceHorizontalSize, pXvdPic->ulSourceHorizontalSize) > BVDC_P_SRC_INPUT_H_MIN);
                bVmin = (BVDC_P_MAX(pVdcPic->ulSourceVerticalSize,   pXvdPic->ulSourceVerticalSize)  > BVDC_P_SRC_INPUT_V_MIN);
                if(bHmin ||bVmin)
                    pCurDirty->stBits.bMosaicIntra = BVDC_P_DIRTY;
            }
            else
                pCurDirty->stBits.bMosaicIntra = BVDC_P_DIRTY;
        }
#else
        BSTD_UNUSED(bNewInterlaced);
        BSTD_UNUSED(bCurInterlaced);
#endif
        bChannelSet[pXvdPic->ulChannelId] = true;
        *pVdcPic = *pXvdPic;
        if(pXvdPic->bMute)
        {
            pVdcPic->eOrientation = BFMT_Orientation_e2D;
            /* When bMute is set, all values except eInterruptPolarity,
             * bIgnorePicture, and bStallStc are invalid.
             * align size to be min 64x64*/
            pVdcPic->ulSourceHorizontalSize = BVDC_P_MAX(pVdcPic->ulSourceHorizontalSize, BVDC_P_SRC_INPUT_H_MIN);
            pVdcPic->ulSourceVerticalSize   = BVDC_P_MAX(pVdcPic->ulSourceVerticalSize,   BVDC_P_SRC_INPUT_V_MIN);
        }

        if(hSource->stCurInfo.bMosaicMode)
            pXvdPic = pXvdPic->pNext;
        else
            pXvdPic = NULL;
    }

    if(hSource->stCurInfo.bMosaicMode)
    {
        for(i = 0; i < hSource->ulMosaicCount; i++)
        {
            if(!bChannelSet[i])
            {
                hSource->stNewPic[hSource->aulMosaicZOrderIndex[i]].ulChannelId = i;
                bChannelSet[i] = true;
            }
        }

        hSource->ulPixelCount = 0;
        for(i = 0; i < hSource->ulMosaicCount; i++)
        {
            hSource->ulPixelCount += ((hSource->stNewPic[i].ulSourceHorizontalSize *
                hSource->stNewPic[i].ulSourceVerticalSize) / BFMT_FREQ_FACTOR) * hSource->ulVertFreq;

            if(i > 0)
            {
                hSource->stNewPic[i-1].pNext = &(hSource->stNewPic[i]);
            }
            hSource->aulChannelId[i] = hSource->stNewPic[i].ulChannelId;
        }
        hSource->stNewPic[i].pNext = NULL;
    }
    else
    {
        hSource->ulPixelCount = ((pNewPic->ulSourceHorizontalSize *
             pNewPic->ulSourceVerticalSize) / BFMT_FREQ_FACTOR) * hSource->ulVertFreq;
    }

    hSource->ulMosaicFirstUnmuteRectIndex = 0;
    hSource->bMosaicFirstUnmuteRectIndexSet = false;

#if (BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP)
    if((pNewPic->eOrientation == BFMT_Orientation_e3D_LeftRight) &&
        pNewPic->pEnhanced)
    {
        BAVC_MVD_Field   *pEnhanced;

        pEnhanced = (BAVC_MVD_Field *)pNewPic->pEnhanced;
        if(pEnhanced->eOrientation == BFMT_Orientation_eLeftRight_Enhanced)
        {
            /* Set eOrientation to BFMT_Orientation_eLeftRight_Enhanced
             * to distinguish from regular BFMT_Orientation_e3D_LeftRight */
            pNewPic->eOrientation = BFMT_Orientation_eLeftRight_Enhanced;
        }
    }
#endif

    /* bOrientationOverride only valid when original
     * orientation from XVD is 2D */
    if(pNewPic->eOrientation == BFMT_Orientation_e2D)
    {
        if(pCurInfo->bOrientationOverride)
        {
            if(pCurInfo->eOrientation == BFMT_Orientation_e3D_OverUnder)
            {
                pNewPic->ulSourceVerticalSize  = pNewPic->ulSourceVerticalSize / 2;
                pNewPic->ulDisplayVerticalSize = pNewPic->ulDisplayVerticalSize / 2;
            }
            else if(pCurInfo->eOrientation == BFMT_Orientation_e3D_LeftRight)
            {
                pNewPic->ulSourceHorizontalSize  = pNewPic->ulSourceHorizontalSize / 2;
                pNewPic->ulDisplayHorizontalSize = pNewPic->ulDisplayHorizontalSize / 2;
            }
            pNewPic->eOrientation = pCurInfo->eOrientation;
        }
    }
    else if(pNewPic->eOrientation == BFMT_Orientation_e3D_OverUnder)
    {
        pNewPic->ulSourceVerticalSize  = pNewPic->ulSourceVerticalSize / 2;
        pNewPic->ulDisplayVerticalSize = pNewPic->ulDisplayVerticalSize / 2;
    }
    else if(pNewPic->eOrientation == BFMT_Orientation_e3D_LeftRight)
    {
        pNewPic->ulSourceHorizontalSize  = pNewPic->ulSourceHorizontalSize / 2;
        pNewPic->ulDisplayHorizontalSize = pNewPic->ulDisplayHorizontalSize / 2;
    }

    /* H & V size alignment */
    /* Jira SW7405-4239: XMD might send odd width for special aspect ratio purpose
      pNewPic->ulSourceHorizontalSize = pNewPic->ulSourceHorizontalSize & ~1; */
    pNewPic->ulSourceVerticalSize   = pNewPic->ulSourceVerticalSize & ~1;

    /* PsF: check if need to scanout; also store the scanout state for the
     *      next isr's scanout decision;
     * bForceFrameCapture:  share the same mechanism as PsF */
    if((hSource->stCurInfo.bPsfEnable || hSource->stCurInfo.bForceFrameCapture ||
        hSource->bEnablePsfBySize) &&
       (!(hSource->bMtgSrc && !hSource->stCurInfo.bMosaicMode)))
    {
        bool bPsfSrc;

#if (!BVDC_P_SUPPORT_4kx2k_60HZ)
        /* Always enable PSF for 4kx2k source */
        bPsfSrc =
            (((BFMT_1080P_WIDTH     < pNewPic->ulSourceHorizontalSize) ||
              (BFMT_1080P_HEIGHT     < pNewPic->ulSourceVerticalSize)) &&
             (BAVC_Polarity_eFrame == pNewPic->eSourcePolarity)) ||
            (hSource->stCurInfo.bForceFrameCapture);
#else
        bPsfSrc = hSource->stCurInfo.bForceFrameCapture ||
            (((BFMT_720P_HEIGHT*2) < pNewPic->ulSourceVerticalSize) &&
             (hSource->ulDispVsyncFreq >= 100*BFMT_FREQ_FACTOR));
#endif

        /* if user just turns on PsF, but decoder is in pause mode(bRepeat=true),
           we still need to scan out for the first time;
           if 1080p source repeats, don't scanout;
           if last 1080p frame was scaned out and not in 1080p pass-thru (<=30Hz),
           don't scanout for this time; */
        /* TODO: for interlaced src, if we have bandwidth, maybe we should process
           all non- repeat fields (so that MAD can work corretcly) and capture all frames,
           but only give half to raaga?
         */
        bNoScanout = (hSource->bPsfScanout) || /* --> drop this src frame/field if last was captured */
            (!(pCurDirty->stBits.bPsfMode) &&
             pNewPic->bPictureRepeatFlag &&   /* --> drop this src frame/field if it is marked as a "repeat" */
              bPsfSrc);

        /* to keep src/win state machine moving, don't drop too many frames; */
        if(bNoScanout)
        {
            if(++hSource->ulDropFramesCnt > 3)
            {
                bNoScanout = false;
                hSource->ulDropFramesCnt = 0;
                BDBG_MSG(("Mfd%d drops 4 1080p frames in a roll; Resume scanout!", hSource->eId));
            }
        }
        else
        {
            hSource->ulDropFramesCnt = 0;
        }

        /* 1080p PsF scanout RUL must chop the RUL size to protect itself! */
        hSource->bPsfScanout =
            !bNoScanout &&
             bPsfSrc  &&
            (hSource->ulDispVsyncFreq > BVDC_P_PSF_VERT_FREQ);

        /* PsF: the source vertical freq is the frame rate (for frame capture) */
        if(hSource->bPsfScanout)
        {
            uint32_t ulPsfVertFreq;

            switch(pNewPic->eFrameRateCode)
            {
            case BAVC_FrameRateCode_e23_976:
                ulPsfVertFreq = 2397;
                break;
            case BAVC_FrameRateCode_e24:
                ulPsfVertFreq = 24 * BFMT_FREQ_FACTOR;
                break;
            case BAVC_FrameRateCode_e25:
                ulPsfVertFreq = 25 * BFMT_FREQ_FACTOR;
                break;
            case BAVC_FrameRateCode_e29_97:
                ulPsfVertFreq = 2997;
                break;
            case BAVC_FrameRateCode_e30:
                ulPsfVertFreq = 30 * BFMT_FREQ_FACTOR;
                break;
            default:
                /* "Forced PSF" allows all type of mpeg/gfx src (format and frequency),
                 * SCL will convert intrelaced fields into frames, and at least
                 * drop half of the src fields / frames */
                BDBG_MSG(("ulSourceVerticalSize = %d", pNewPic->ulSourceVerticalSize));
                BDBG_MSG(("eSourcePolarity      = %d", pNewPic->eSourcePolarity));
                BDBG_MSG(("eFrameRateCode       = %d", pNewPic->eFrameRateCode));
                BDBG_MSG(("Ignore stream frame rate. Use PsF %d Hz",
                    BVDC_P_PSF_VERT_FREQ/BFMT_FREQ_FACTOR));
                ulPsfVertFreq = BVDC_P_PSF_VERT_FREQ;
            }

            if(hSource->ulVertFreq != ulPsfVertFreq)
            {
                /* overwrite previously set ulVertFreq in UpdateMfdVerRate_code */
                hSource->ulVertFreq = ulPsfVertFreq;
                hSource->bRasterChanged = true;
                /* change eMfdVertRateCode accordingly to be consistent with ulVertFreq */
                hSource->eMfdVertRateCode = BVDC_P_Source_RefreshRateCode_FromRefreshRate_isrsafe(hSource->ulVertFreq);
            }
        }
    }
    /* PsF: if user disables PsF, skip one new scanout to avoid MFD hang; */
    else if(hSource->bPsfScanout)
    {
        hSource->bPsfScanout = false;
        bNoScanout = true;
        pCurDirty->stBits.bPsfMode  = BVDC_P_CLEAN;
        BDBG_MSG(("MFD[%d] disables PsF! Skip a new scanout to avoid MFD hang.", hSource->eId));
    }

    /* to avoid display/gfx flickering caused by the nop src RUL due to the
       field swap on the artificial triggers, we need to
       build RUL for the field swapped slot; */
    if(hSource->bPrevFieldSwap != hSource->bFieldSwap)
    {
        BDBG_MSG(("MFD[%d] bFieldSwap[%d->%d]!", hSource->eId,
            hSource->bPrevFieldSwap, hSource->bFieldSwap));

        /* field swap: this is the next real interrupt to come */
        pNewPic->eInterruptPolarity =
            BVDC_P_NEXT_POLARITY(pNewPic->eInterruptPolarity);

        /* update prev flag */
        hSource->bPrevFieldSwap = hSource->bFieldSwap;
    }

    /* validate the input data structure; */
    BVDC_P_Source_ValidateMpegData_isr(hSource, pNewPic, pCurPic);

    if(hSource->ulRefreshRateMismatchCntr && (pNewPic->eSourcePolarity != BAVC_Polarity_eFrame))
    {
        ePrePolarity = pCurPic->eSourcePolarity;

        pNewPic->eSourcePolarity = (ePrePolarity == BAVC_Polarity_eTopField) ?
            BAVC_Polarity_eBotField : BAVC_Polarity_eTopField;
    }

    /* Update the format information for MPEG */
    if(hSource->bPictureChanged)
    {
        const BFMT_VideoInfo *pNewFmtInfo;
        const BFMT_VideoInfo *pCurFmtInfo;
        uint32_t ulNewPicFrmRate;
        bool bNewPicInterlaced;
        uint32_t ulSourceHorizontalSize, ulSourceVerticalSize;

        ulSourceHorizontalSize = pNewPic->ulSourceHorizontalSize;
        ulSourceVerticalSize   = pNewPic->ulSourceVerticalSize;

        if(pNewPic->eOrientation == BFMT_Orientation_e3D_LeftRight)
        {
            ulSourceHorizontalSize  = ulSourceHorizontalSize * 2;
        }
        else if(pNewPic->eOrientation == BFMT_Orientation_e3D_OverUnder)
        {
            ulSourceVerticalSize  = ulSourceVerticalSize * 2;
        }

        ulNewPicFrmRate = s_aulFrmRate[BVDC_P_MIN(BVDC_P_MAX_FRM_RATE_CODE, pNewPic->eFrameRateCode)];
        bNewPicInterlaced = (BAVC_Polarity_eFrame != pNewPic->eSourcePolarity);

        /* retain last rate if framerate is unknown. */
        if(BAVC_FrameRateCode_eUnknown != pNewPic->eFrameRateCode)
        {
            hSource->ulStreamVertFreq = ulNewPicFrmRate << (!pNewPic->bStreamProgressive);
        }

        pCurFmtInfo = (BFMT_VideoFmt_eMaxCount == hSource->stExtVideoFmtInfo.eVideoFmt)?
            &(hSource->stExtVideoFmtInfo) : pCurInfo->pFmtInfo;

        if((!BVDC_P_EQ_DELTA(ulSourceHorizontalSize, pCurFmtInfo->ulDigitalWidth, BVDC_P_MPEG_FMT_DELTA)) ||
           (!BVDC_P_EQ_DELTA(ulSourceVerticalSize, pCurFmtInfo->ulDigitalHeight, BVDC_P_MPEG_FMT_DELTA)) ||
           (!BVDC_P_EQ_DELTA(ulNewPicFrmRate, pCurFmtInfo->ulVertFreq >> pCurFmtInfo->bInterlaced, BVDC_P_MPEG_FMT_DELTA)) ||
           (bNewPicInterlaced != pCurFmtInfo->bInterlaced))
        {
            /* src video format changed. try to match an enumerated video format */
            for(i = 0; i < BVDC_P_MPEG_FMT_COUNT; i++)
            {
                pNewFmtInfo = BFMT_GetVideoFormatInfoPtr_isrsafe(s_aeMpegToFmt[i]);
                if((BVDC_P_EQ_DELTA(ulSourceHorizontalSize, pNewFmtInfo->ulDigitalWidth, BVDC_P_MPEG_FMT_DELTA)) &&
                   (BVDC_P_EQ_DELTA(ulSourceVerticalSize, pNewFmtInfo->ulDigitalHeight, BVDC_P_MPEG_FMT_DELTA)) &&
                   (BVDC_P_EQ_DELTA(ulNewPicFrmRate, pNewFmtInfo->ulVertFreq >> pNewFmtInfo->bInterlaced, BVDC_P_MPEG_FMT_DELTA)) &&
                   (bNewPicInterlaced == pNewFmtInfo->bInterlaced))
                {
                    break;
                }
            }

            if(BVDC_P_MPEG_FMT_COUNT == i)
            {
                BFMT_VideoFmt  eVFmtCode;

                /* non-enumerated mpeg video format, will return stExtVideoFmtInfo as callback */
                hSource->stExtVideoFmtInfo.eVideoFmt = BFMT_VideoFmt_eMaxCount;
                hSource->stExtVideoFmtInfo.ulDigitalWidth = ulSourceHorizontalSize;
                hSource->stExtVideoFmtInfo.ulDigitalHeight = ulSourceVerticalSize;
                hSource->stExtVideoFmtInfo.ulVertFreq = ulNewPicFrmRate << bNewPicInterlaced;
                hSource->stExtVideoFmtInfo.bInterlaced = bNewPicInterlaced;

                /* approximate video Fmt code for VDC internal use only */
                if (bNewPicInterlaced)
                {
                    eVFmtCode = (((ulSourceHorizontalSize <= BFMT_NTSC_WIDTH) &&
                                  (ulSourceVerticalSize   <= BFMT_NTSC_HEIGHT)) ?
                                 BFMT_VideoFmt_eNTSC :
                                 ((ulSourceHorizontalSize <= BFMT_PAL_WIDTH) &&
                                  (ulSourceVerticalSize   <= BFMT_PAL_HEIGHT)) ?
                                 BFMT_VideoFmt_ePAL_G :
                                 BFMT_VideoFmt_e1080i);
                }
                else
                {
                    eVFmtCode = (((ulSourceHorizontalSize <= BFMT_480P_WIDTH) &&
                                  (ulSourceVerticalSize   <= BFMT_480P_HEIGHT)) ?
                                 BFMT_VideoFmt_e480p :
                                 ((ulSourceHorizontalSize <= BFMT_576P_WIDTH) &&
                                  (ulSourceVerticalSize   <= BFMT_576P_HEIGHT)) ?
                                 BFMT_VideoFmt_e576p_50Hz :
                                 ((ulSourceHorizontalSize <= BFMT_720P_WIDTH) &&
                                  (ulSourceVerticalSize   <= BFMT_720P_HEIGHT)) ?
                                 BFMT_VideoFmt_e720p :
                                 ((ulSourceHorizontalSize <= BFMT_1080P_WIDTH) &&
                                  (ulSourceVerticalSize   <= BFMT_1080P_HEIGHT)) ?
                                 BFMT_VideoFmt_e1080p :
                                 BFMT_VideoFmt_e3840x2160p_24Hz);
                }
                pNewFmtInfo = BFMT_GetVideoFormatInfoPtr_isrsafe(eVFmtCode);

                BDBG_MSG(("MPEG[%d] changes to non-enumerated video fmt: w %d, h %d, Frmrate %d/100, bInterlaced %d ",
                    hSource->eId, ulSourceHorizontalSize, ulSourceVerticalSize,
                    ulNewPicFrmRate, bNewPicInterlaced? 1 : 0));
            }
            else
            {
                /* this is used later as deciding which formatInfo to send with callback */
                hSource->stExtVideoFmtInfo.eVideoFmt = pNewFmtInfo->eVideoFmt;
            }

            /* this is an enumerated video format */
            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("MPEG[%d] Format change %s -> %s", hSource->eId,
                pCurInfo->pFmtInfo->pchFormatStr, pNewFmtInfo->pchFormatStr));

#if (BVDC_P_DCX_3D_WORKAROUND)
            if(BFMT_IS_3D_MODE(pNewFmtInfo->eVideoFmt))
            {
                BDBG_WRN(("============================================================"));
                BDBG_WRN(("May not have enough bandwidth to support 3D Format %s",
                    pNewFmtInfo->pchFormatStr));
                BDBG_WRN(("============================================================"));
            }
#endif

            /* for BVDC_Source_GetSize support: need original size  */
            hSource->stExtVideoFmtInfo.ulWidth = ulSourceHorizontalSize;
            hSource->stExtVideoFmtInfo.ulHeight = ulSourceVerticalSize;

            /* Start the new format: this is only used for VDC internally for certain decision,
             * might not have every thing accurate */
            pCurInfo->pFmtInfo      = pNewInfo->pFmtInfo = pNewFmtInfo;

            /* Get vdc base fmt information */
            pCurInfo->pVdcFmt       = pNewInfo->pVdcFmt  =
                    BVDC_P_GetFormatInfo_isrsafe(pNewFmtInfo->eVideoFmt);

            /* Format changes, so set dirty to rebuild RUL. */
            pCurDirty->stBits.bInputFormat = BVDC_P_DIRTY;

            /* inform next ApplyChanges to copy activated isr setting into new info */
            hSource->stIsrInfo.stActivated.stBits.bInputFormat = BVDC_P_DIRTY;
        }
    }

    /* the eNextFieldId tells interrupt cadence */
    hSource->eNextFieldId   = pNewPic->eInterruptPolarity;
    hSource->eNextFieldIntP = pNewPic->eInterruptPolarity;

    /* STG format switch between i/p */
#if BVDC_P_SUPPORT_STG
    if(hSource->hSyncLockCompositor &&
       BVDC_P_DISPLAY_USED_STG(hSource->hSyncLockCompositor->hDisplay->eMasterTg))
    {
        /* i->p format switch */
        if((BAVC_Polarity_eFrame != pNewPic->eInterruptPolarity) &&
           (!hSource->hSyncLockCompositor->hDisplay->stCurInfo.pFmtInfo->bInterlaced)
#if (BVDC_P_SUPPORT_IT_VER >= 2)
           && (2 != hSource->hSyncLockCompositor->hDisplay->stCurInfo.ulTriggerModuloCnt)
#endif
        )
        {
            hSource->eNextFieldId = BAVC_Polarity_eFrame;
            BDBG_MSG(("MFD[%d] display format switch from I->P", hSource->eId));
        }
        /* p->i format switch */
        else if((BAVC_Polarity_eFrame == pNewPic->eInterruptPolarity) &&
           (hSource->hSyncLockCompositor->hDisplay->stCurInfo.pFmtInfo->bInterlaced))
        {
            hSource->eNextFieldId = BAVC_Polarity_eTopField;
            BDBG_MSG(("MFD[%d] display format switch from P->I", hSource->eId));
#if BVDC_P_STG_NRT_CADENCE_WORKAROUND /* for STG hw that cannot repeat trigger polarity */
            hSource->bToggleCadence = false; /* reset the need to toggle stg cadence for workaround */
#endif
        }
    }
#endif

    /* MosaicMode: the frame rate tracking channel will set it later; */
    if(hSource->stCurInfo.bMosaicMode)
    {
        hSource->eFrameRateCode = BAVC_FrameRateCode_eUnknown;
        hSource->eMatrixCoefficients = BAVC_MatrixCoefficients_eUnknown;
    }
    else
    {
        hSource->eFrameRateCode = pNewPic->eFrameRateCode;
        hSource->eMatrixCoefficients = pNewPic->eMatrixCoefficients;
    }

    if((hSource->stCurInfo.pfGenericCallback) &&
       (hSource->bCaptureCrc      ||
        pCurDirty->stBits.bInputFormat   ||
        hSource->bDeferSrcPendingCb ||
        (pCurDirty->stBits.bAddWin && hSource->stCurInfo.eResumeMode) ||
        (pCurPic->eFrameRateCode != pNewPic->eFrameRateCode) ||
        BVDC_P_CB_IS_DIRTY_isr(&hSource->stCurInfo.stCallbackSettings.stMask) ||
        (pCurPic->bMute && !pNewPic->bMute)))
    {
        BVDC_Source_CallbackData *pCbData = &hSource->stSourceCbData;
        BVDC_Source_CallbackMask *pCbMask = &pCbData->stMask;
        const BVDC_Source_CallbackSettings *pCbSettings;

        /* Clear dirty bits */
        BVDC_P_CB_CLEAN_ALL_DIRTY(pCbMask);

        pCbSettings = &pCurInfo->stCallbackSettings;

        /* Issue src pending call back when shutdown BVN completed. */
        if(hSource->bDeferSrcPendingCb)
        {
            BVDC_P_Source_CheckAndIssueCallback_isr(hSource, pCbMask);
        }

        /* Make sure the callback happen at least once, on first
         * installation of callback to report the current status. */
        if(pCurDirty->stBits.bGenCallback)
        {
            pCbMask->bActive     = BVDC_P_DIRTY;
            pCbMask->bFmtInfo    = BVDC_P_DIRTY;
            hSource->bDeferSrcPendingCb = true;
        }
        else if((pCurDirty->stBits.bInputFormat) ||
           (pCurDirty->stBits.bAddWin && hSource->stCurInfo.eResumeMode) ||
           (pCurPic->eFrameRateCode != pNewPic->eFrameRateCode) ||
           (pCurPic->bMute && !pNewPic->bMute))
        {
            /* defer it until all windows are shutdown!
             * Note, if input format changes, the next vsync will show window
             * shutdown dirty bits;
             * Only when all its windows shutdown dirty bits are cleared;
             * is it safe to callback source pending; */
            hSource->bDeferSrcPendingCb = hSource->stCurInfo.eResumeMode;
            pCbMask->bFmtInfo       = pCurDirty->stBits.bInputFormat;
        }

        /* 7401 & beyond MFD supports field-accurate CRC checking on demand.
         * Note: when a picture is required to capture CRC, its RUL is executed one
         *       vsync later; and its CRC computation is completed at end of that
         *       picture, i.e. another vsync later! So we use the stored key flag
         *       to callback the captured CRC; */
        if((hSource->bCaptureCrc || pCbSettings->stMask.bCrcValue)
#if BVDC_P_SUPPORT_STG /* SW7445-2544: don't callback CRC for ignore picture in NRT mode */
        && !(hSource->hSyncLockCompositor &&
            (BVDC_P_DISPLAY_USED_STG(hSource->hSyncLockCompositor->hDisplay->eMasterTg)) &&
            (hSource->hSyncLockCompositor->hDisplay->stCurInfo.bStgNonRealTime) &&
            /* since this is executed before the cmp updates the ignore flag in vec build isr */
            (hSource->hSyncLockCompositor->bCrcToIgnore))
#endif
        )
        {
            pCbMask->bCrcValue = true;

            /* update CRC readings */
            BVDC_P_Feeder_GetCrc_isr(hSource->hMpegFeeder, hSource->hVdc->hRegister, pCbData);
        }

        if (hSource->stCurInfo.stCallbackSettings.stMask.bFrameRate &&
            ((pCbData->eFrameRateCode != hSource->eMfdVertRateCode) ||
             (pCbData->bMtgSrc != (hSource->bMtgSrc && !hSource->stCurInfo.bMosaicMode))))
        {
            pCbMask->bFrameRate  = BVDC_P_DIRTY;
            pCbData->eFrameRateCode = hSource->eMfdVertRateCode;
            pCbData->ulVertRefreshRate = hSource->ulVertFreq;
            BDBG_ASSERT(hSource->ulVertFreq == BVDC_P_Source_RefreshRate_FromFrameRateCode_isrsafe(hSource->eMfdVertRateCode));
            pCbData->bMtgSrc = (hSource->bMtgSrc && !hSource->stCurInfo.bMosaicMode);
        }

        /* callback only if something changed */
        if(BVDC_P_CB_IS_DIRTY_isr(pCbMask))
        {
            /* Update Callback data */
            pCbData->bActive  = !pNewPic->bMute;
            pCbData->pFmtInfo = ((BFMT_VideoFmt_eMaxCount == hSource->stExtVideoFmtInfo.eVideoFmt)?
                                 &(hSource->stExtVideoFmtInfo) : hSource->stCurInfo.pFmtInfo);

            hSource->stCurInfo.pfGenericCallback(
                hSource->stCurInfo.pvGenericParm1,
                hSource->stCurInfo.iGenericParm2, (void*)pCbData);
            BDBG_MSG(("MPEG[%d] callBack: VideoFmtCode %d origPts = %#x, lumaCrc = %#x",
                      hSource->eId, pCbData->pFmtInfo->eVideoFmt, pNewPic->ulOrigPTS, pCbData->ulLumaCrc));
        }
    }

    /* if the compositor is in slip2lock transition, clean up syncslip display RUL
       to avoid sync-slipped VFD overwrite error in case source/display _isr are
       called in reverse order; */
    if(hSource->hSyncLockCompositor && hSource->hSyncLockCompositor->ulSlip2Lock)
    {
        BDBG_OBJECT_ASSERT(hSource->hSyncLockCompositor, BVDC_CMP);
        BDBG_OBJECT_ASSERT(hSource->hSyncLockCompositor->hDisplay, BVDC_DSP);

        /* Check if we're doing frame.  If we're doing frame we're use a topfield
         * slot to trigger the frame slot in source isr for sycn lock. */
        BVDC_P_CMP_NEXT_RUL(hSource->hSyncLockCompositor, BAVC_Polarity_eTopField);
        hList = BVDC_P_CMP_GET_LIST(hSource->hSyncLockCompositor, BAVC_Polarity_eTopField);

        /* Reset the RUL entry count and build synclock display RUL! */
        BRDC_Slot_UpdateLastRulStatus_isr(hSource->hSyncLockCompositor->ahSlot[BAVC_Polarity_eTopField], hList, true);/* top slot */
        BRDC_Slot_UpdateLastRulStatus_isr(hSource->hSyncLockCompositor->ahSlot[BAVC_Polarity_eBotField], hList, true);/* bot slot */
        BRDC_List_SetNumEntries_isr(hList, 0);
        BVDC_P_ReadListInfo_isr(&stList, hList);

        BVDC_P_Compositor_BuildSyncLockRul_isr(hSource->hSyncLockCompositor, &stList, hSource->eNextFieldIntP);

        /* Updated lists count and assign the RUL to both t/b display slots */
        BVDC_P_WriteListInfo_isr(&stList, hList);
        BRDC_Slots_SetList_isr(hSource->hSyncLockCompositor->ahSlot, hList, BVDC_P_CMP_MAX_SLOT_COUNT); /* two slots per display */

        /* clear the flag */
        hSource->hSyncLockCompositor->ulSlip2Lock = 0;
        BDBG_MSG(("Src[%d] intP%d clears ulSlip2Lock", hSource->eId, hSource->eNextFieldIntP));
    }

    /* Get the approriate slot/list for building RUL. */
    BVDC_P_SRC_NEXT_RUL(hSource, hSource->eNextFieldId);
    hSlot = BVDC_P_SRC_GET_SLOT(hSource, hSource->eNextFieldId);
    hList = BVDC_P_SRC_GET_LIST(hSource, hSource->eNextFieldId);

    BRDC_Slot_UpdateLastRulStatus_isr(hSlot, hList, true);

    /* Reset the RUL entry count and build RUL for backend! */
    BRDC_List_SetNumEntries_isr(hList, 0);

#if BVDC_P_SUPPORT_MTG
    if (hSource->bMtgSrc)
    {
        /* reset mtg if needed */
        BVDC_P_Feeder_Mtg_MpegDataReady_isr(hSource->hMpegFeeder, hList, pNewPic);
    }
#endif /* #if BVDC_P_SUPPORT_MTG */

    /* PsF: prefix a NOP RUL at head of PIP source RUL for robustness purpose;
            sync-slipped mpg source isr will only build capture side RUL; if 1080p
            PsF scanout is on, robustness RUL will simply chop the RUL size to the
            NOP minmum; */
    if(hSource->bPsfScanout && !hSource->hSyncLockCompositor)
    {
        BVDC_P_BuildNoOpsRul_isr(hList);
        BVDC_P_ReadListInfo_isr(&stMasterList, hList);
        stMasterList.ulPsfMark = (uint32_t)(stMasterList.pulCurrent - stMasterList.pulStart);
        stList.ulPsfMark = stMasterList.ulPsfMark;
    }
    else
    {
        BVDC_P_ReadListInfo_isr(&stMasterList, hList);
    }
    stMasterList.bMasterList = true;
    stList.bMasterList = true;

    /* MosaicMode: picture list
     * ------------------------
     * 1. each picture represents a channel (with channel id) of mosaic list;
     * 2. picture list is sorted in ascending order by DM; shall we validate?
     * 3. existing channel needs to be captured into he correct location, according to
     *    the channel id (equivalent to rectangle id), into the cap buffer;
     * 3. missing channel (closed channel) simply results in no scanout/capture; but we
     *    need to use ClearRect to clear its corresponding mosaic rectangle at CMP;
     * 4. missing channel also means ulPictureIdx != ulChannelId;
     *    ulPictureIdx = 0 means to build master RUL, while ulPictureIdx > 0 means to
     *    build slave RUL; we can also assume ulPictureIdx <= ulChannelId;
     * 5. what if the first picture has ulChannelId != 0, or > ulMosaicCount?
     * 6. need to make sure ulChannelId < BAVC_MOSAIC_MAX;
     * 7. if the last picture ulChannelId < ulMosaicCount, ClearRect for the rest;
     */
    do
    {
        bool  bBuiltCanvasCtrlWithWin = false;

    #define BDC_P_LIST_SEL(master, slave, idx)   ((0 == (idx))? (master) : (slave))

        /* get the old num of entries */
        BRDC_List_GetNumEntries_isr(BDC_P_LIST_SEL(hList, hListSlave, ulPictureIdx), &ulOldEntries);

        /* validate the next input data structure; */
        if(ulPictureIdx)
        {
            BVDC_P_Source_ValidateMpegData_isr(hSource, pNewPic, pCurPic);
        }

#if BVDC_P_SUPPORT_MOSAIC_MODE
        /* make sure channel id is valid */
        BDBG_ASSERT(pNewPic->ulChannelId < BAVC_MOSAIC_MAX);

        if(!hSource->bMosaicFirstUnmuteRectIndexSet)
        {
            if(!pNewPic->bMute)
            {
                hSource->ulMosaicFirstUnmuteRectIndex = ulPictureIdx;
                hSource->bMosaicFirstUnmuteRectIndexSet = true;
            }
        }
#endif

        /* update windows that connected to this source, including user info,
         * destroy related state and disconnecting from source */
        for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
        {
            /* SKIP: If it's just created or inactive no need to build ruls. */
            if(!hSource->ahWindow[i] ||
                BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
                BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
            {
                continue;
            }

            hWindow = hSource->ahWindow[i];
            BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

            /* Non_MosaicMode window skips; */
            if(ulPictureIdx && !hWindow->stCurInfo.bMosaicMode)
            {
                continue;
            }

            BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
            BDBG_OBJECT_ASSERT(hWindow->hCompositor->hDisplay, BVDC_DSP);

            /* Vec format switch, executed the last force trigger from sync-lock
             * display's slot.  It's in the middle of format change, no need to
             * serve this interrupt. Clean up all source slots to be safe. */
            if((BVDC_P_ItState_eActive != hWindow->hCompositor->hDisplay->eItState) &&
               ((hWindow->hCompositor->hSyncLockSrc == hSource) ||
                (hWindow->hCompositor->hForceTrigPipSrc == hSource)))
            {
                BVDC_P_Source_CleanupSlots_isr(hSource);
                BDBG_MSG(("Synclocked display switches format. Clean up MFD[%d] slots!",
                    hSource->eId));
                goto MpegDataReady_isr_Done;
            }

            /* MosaicMode: don't adjust DstRect; only adj sub-rects SrcCnt and SclOut; */
            if((0 == ulPictureIdx)
#if BVDC_P_SUPPORT_MOSAIC_MODE
                ||
                (hWindow->stCurInfo.bMosaicMode &&
                 (hSource->bPictureChanged ||
                  BVDC_P_FIELD_DIFF(&hWindow->stCurInfo.astMosaicRect[pNewPic->ulChannelId],
                                   &hWindow->stCurInfo.astMosaicRect[pCurPic->ulChannelId], ulWidth) ||
                  BVDC_P_FIELD_DIFF(&hWindow->stCurInfo.astMosaicRect[pNewPic->ulChannelId],
                                   &hWindow->stCurInfo.astMosaicRect[pCurPic->ulChannelId], ulHeight)))
#endif
               )
            {
                uint32_t ulRectIdx;

                /* MosaicMode: if the 1st picture's channel id exceeds mosaic count, we still
                   need to adjust the rectangle for scanout; capture can drain it; so take
                   mosaic rect 0; */
                {
                    ulRectIdx = pNewPic->ulChannelId;
                }

                /* in mosaic mode, even if bPictureChanged==false, we still need to
                 * do initMuteRect or AdjustRectangles, because mosaic pic size might
                 * be diff from prev mosaic pic */
                if(pNewPic->bMute)
                {
                    /* If source is muted, VDC will ignore rectangle clipping */
                    if(hSource->bPictureChanged || hSource->stCurInfo.bMosaicMode ||
                       pCurDirty->stBits.bAddWin)
                    {
                        BVDC_P_Window_InitMuteRec_isr(hSource->ahWindow[i],
                            hWindow->hCompositor->stCurInfo.pFmtInfo, pNewPic, ulRectIdx);
                    }
                }
                else
                {
                    BVDC_P_Window_AdjustRectangles_isr(hSource->ahWindow[i], pNewPic, NULL,
                        ulRectIdx);
                }
            }
        }

        if(!pNewPic->bCaptureCrc || pNewPic->bMute)
        {
            /* Get the source display rectangle. Combine the user pan-scan info
             * from all the window that uses this source */
            if((0 == ulPictureIdx) || pCurDirty->stBits.bRecAdjust)
            {
                BVDC_P_Source_GetScanOutRect_isr(hSource, pNewPic, NULL, &hSource->stScanOut);
            }

            BVDC_P_Feeder_SetMpegInfo_isr(hSource->hMpegFeeder,
                hSource->stCurInfo.ulMuteColorYCrCb, pNewPic, &hSource->stScanOut);
        }
        else
        {
            BVDC_P_Rect stScanOut;

            /* if requested to capture CRC, feed the full size picture;
             * Note, we don't care the display quality in this case; */
            stScanOut.lLeft = 0;
            stScanOut.lLeft_R = 0;
            stScanOut.lTop = 0;
            stScanOut.ulWidth  = pNewPic->ulSourceHorizontalSize & ~1;
            stScanOut.ulHeight = pNewPic->ulSourceVerticalSize & ~1;

            /* Get the source display rectangle. Combine the user pan-scan info
             * from all the window that uses this source;
             * Note, this is to avoid crashing BVN since some of downstream modules
             * programming depend on this rectangle; */
            BVDC_P_Source_GetScanOutRect_isr(hSource, pNewPic, NULL, &hSource->stScanOut);
            BVDC_P_Feeder_SetMpegInfo_isr(hSource->hMpegFeeder,
                hSource->stCurInfo.ulMuteColorYCrCb, pNewPic, &stScanOut);
        }

        /* Now update base next picture to be display, for all windows
         * connected to this source. */
        for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
        {
            /* SKIP: If it's just created or inactive no need to build ruls. */
            if(!hSource->ahWindow[i] ||
                BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
                BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
            {
                continue;
            }
            hWindow = hSource->ahWindow[i];
            BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

#if BVDC_P_SUPPORT_MOSAIC_MODE

            /* MosaicMode: count number of mosaic windows for combo trigger;
               note that one of simul windows enables mosaic mode would require
               combo trigger for the whole picture list; un-captured rects are
               drained by CAP and need trigger to proceed to the next mosaic picture; */
            if(hSource->stCurInfo.bMosaicMode && (ulPictureIdx == 0))
            {
                /* get the maxim mosaic count among windows to config proper trigger; */
                ulMosaicCount = BVDC_P_MAX(ulMosaicCount, hWindow->stCurInfo.ulMosaicCount);

                /* increment count to set combo trigger */
                hSource->aeCombTriggerList[ulConnectedWindow++] = hWindow->stCurResource.hCapture->eTrig;

                if(ulMosaicCount == hWindow->stCurInfo.ulMosaicCount)
                {
                    eTrigger = hWindow->stCurResource.hCapture->eTrig;
                }
            }
#endif
            /* MosaicMode: need to pass the sub-window index to build Capture RUL; note here we only care master
                RUL's exec status; */
            /* PsF: no window write state change while no scanout; */
            if(!bNoScanout)
            {
                BVDC_P_Window_Writer_isr(hSource->ahWindow[i], pNewPic, NULL,
                    hSource->eNextFieldId, &stMasterList, ulPictureIdx);
            }

            if((NULL == hSource->hSyncLockCompositor) && (0 == ulPictureIdx))
            {
#if BVDC_P_SUPPORT_MTG
                if (BVDC_P_MVP_USED_MAD_AT_WRITER(hSource->ahWindow[i]->stVnetMode, hSource->ahWindow[i]->stMvpMode))
                {
                    hSource->bUsedMadAtWriter = true;
                }
#endif
                /* if mtg is not used, and not syncLocked, MFD follows the 1st connected win
                 * TODO: which win is trigering this src slot? */
                if (hSource->hMfdVertDrivingWin == NULL)
                {
                    hSource->hMfdVertDrivingWin = hSource->ahWindow[i];
                }
            }

            /* note: mfd rul is built together with other modules in the vnet in window */
        }

        if(pCurDirty->stBits.bRecAdjust || hSource->bWait4ReconfigVnet)
        {
            /* Gather window information after vnetmode is determined */
            BVDC_P_Source_GetWindowVnetmodeInfo_isr(hSource);
        }

        /* If this source is sync-lock with a compositor/vec we'll need to
         * build RUL for that VEC, and CMP.
         * Note, we need to handle field swap for captured sync-locked src to be displayed
         * correctly. */
        /* MosaicMode: only build playback side RUL once for master RUL! */
        if(hSource->hSyncLockCompositor && (0 == ulPictureIdx))
        {
            /* FIELDSWAP 4): swap again with reader to display correct polarity; */
            BVDC_P_ReadListInfo_isr(&stList, hList);
            BVDC_P_Compositor_BuildSyncSlipRul_isr(hSource->hSyncLockCompositor,
                &stList, (hSource->bFieldSwap) ?
                BVDC_P_NEXT_POLARITY(hSource->eNextFieldId) : hSource->eNextFieldId,
                false /* bBuildCanvasCtrl */);
            BVDC_P_WriteListInfo_isr(&stList, hList);

            /* PsF: mark the initial chop location; */
            if(hSource->bPsfScanout)
            {
                stList.ulPsfMark = (uint32_t)(stList.pulCurrent - stList.pulStart);
            }
        }

        /* Read plist master ? slave ? */
        BVDC_P_ReadListInfo_isr(&stList, BDC_P_LIST_SEL(hList, hListSlave, ulPictureIdx));

        /* For each window using this SOURCE, If the window is sync-lock to this
         * source build both the Writer & Reader modules.  If it's slip that means
         * somebody else already building the Reader we're only need to build the
         * writer. */
        /* MosaicMode: simul-windows use combined capture triggers; */
        for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
        {
            /* SKIP: If it's just created or inactive no need to build ruls. */
            if(!hSource->ahWindow[i] ||
               BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
               BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
            {
                continue;
            }

            hWindow = hSource->ahWindow[i];
            BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);


            /* build both slots RUL's if interlaced src format and window state or mode is in
             * the transition to workaround non-alternating trigger pattern;
             * during vnet changes, build double RUL's to commit change
             * and get rid of change's RUL footprint afterwards; */
            bBuildBoth |= hWindow->stCurInfo.stDirty.stBits.bShutdown ||
                hWindow->stCurInfo.stDirty.stBits.bReConfigVnet;

            /* if window is already shutdown, don't need to build the shutdown RUL again
             * to avoid killing the next established window path in case of variable
             * isr latency doesn't promptly remove it from the slot next time; */
            if(BVDC_P_State_eShutDown != hWindow->stCurInfo.eWriterState)
            {
                /* in case we just got the sync-lock and the new locked cmp will has
                 * slip to lock transition, then we know that the new synclock cmp
                 * will build the window reader RUL in the next vsync to retain the
                 * consistent timing based on the same vec, so we only need
                 * to build writer side here; */
                if(hWindow->bSyncLockSrc)
                {
                    /* MosaicMode: playback side only build RUL once!! */
                    BVDC_P_Window_BuildRul_isr(hWindow, &stList, hSource->eNextFieldId,
                        !bNoScanout,        /* writer*/
                        /* MosaicMode: only need to program reader once; */
                        (0 == ulPictureIdx) /* reader */,
                        (0 == ulPictureIdx) /* CanvasCtrl */ );
                    bBuiltCanvasCtrlWithWin = (0 == ulPictureIdx);
                }
                else
                {
                #if BVDC_P_SUPPORT_STG
                    /* sync slaved cmp enabled if not in eShutdown state: to keep bvb flow for NRT mode */
                    if(hWindow->hCompositor->bSyncSlave) {
                        /* NOTE: if syncslave window is not eShutdown, build sync slave display RUL in this source slot; */
                        BVDC_P_Compositor_BuildSyncSlipRul_isr(hWindow->hCompositor,
                            &stList, (hSource->bFieldSwap) ?
                            BVDC_P_NEXT_POLARITY(hSource->eNextFieldId) : hSource->eNextFieldId,
                            false /* bBuildCanvasCtrl */);
                    }
                #endif

                    BVDC_P_Window_BuildRul_isr(hWindow, &stList, hSource->eNextFieldId,
                        !bNoScanout,  /* writer*/
                        false         /* reader */,
                        false         /* CanvasCtrl */ );
                    hWindow->bSlipBuiltWriter = true;

                #if BVDC_P_SUPPORT_STG
                    /* sync slaved cmp enabled if not in eShutdown state: to keep bvb flow for NRT mode */
                    if(hWindow->hCompositor->bSyncSlave) {
                        BVDC_P_Compositor_BuildConvasCtrlRul_isr(hWindow->hCompositor, &stList);
                    }
                #endif
                }
            }
            #if BVDC_P_SUPPORT_STG
            else if(hWindow->hCompositor->bSyncSlave) {
                /* if sync slave window is eShutdown, build sync slaved display RUL in its own display slot to keep it going in NRT mode! */
                BRDC_List_Handle hSlaveCmpList;
                BVDC_P_ListInfo stSlaveCmpList;
                uint32_t* pulSlaveRulMarker;

                BVDC_P_CMP_NEXT_RUL(hWindow->hCompositor, BAVC_Polarity_eTopField);
                hSlaveCmpList = BVDC_P_CMP_GET_LIST(hWindow->hCompositor, BAVC_Polarity_eTopField);

                /* Reset the RUL entry count and build synclock display RUL! */
                BRDC_Slot_UpdateLastRulStatus_isr(hWindow->hCompositor->ahSlot[BAVC_Polarity_eTopField], hSlaveCmpList, true);/* top slot */
                BRDC_Slot_UpdateLastRulStatus_isr(hWindow->hCompositor->ahSlot[BAVC_Polarity_eBotField], hSlaveCmpList, true);/* bot slot */
                BRDC_List_SetNumEntries_isr(hSlaveCmpList, 0);
                BVDC_P_ReadListInfo_isr(&stSlaveCmpList, hSlaveCmpList);

                /* if the stg display is busy, skip re-enabling it! NOTE there may be case that the sync slaved display trigger
                 * didn't fire yet while this shutdown source isr replaced its slot with the sync-slaved cmp RUL; however the
                 * sync slaved cmp had been enabled currently when this source isr fired. That'd cause BVN error/hang. The
                 * solution is to make this transitional RUL watch for the sync slave STG's  status: if busy, skip the rest RUL! */
                *stSlaveCmpList.pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
                #if BCHP_VIDEO_ENC_STG_0_DEBUG_STATUS
                *stSlaveCmpList.pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_STG_0_DEBUG_STATUS + hWindow->hCompositor->hDisplay->ulStgRegOffset);
                *stSlaveCmpList.pulCurrent++ = BRDC_OP_VAR_AND_IMM_TO_VAR(BRDC_Variable_0, BRDC_Variable_1);
                *stSlaveCmpList.pulCurrent++ = BCHP_MASK(VIDEO_ENC_STG_0_DEBUG_STATUS, OUT_PIC_IN_PROGRESS_FLAG);
                #else /* TODO: without STG eop state(7425/35), we have to resort to CMP eop state, which might be a few pixels earlier than actual STG eop! */
                *stSlaveCmpList.pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_CANVAS_CTRL + hWindow->hCompositor->ulRegOffset);
                *stSlaveCmpList.pulCurrent++ = BRDC_OP_VAR_AND_IMM_TO_VAR(BRDC_Variable_0, BRDC_Variable_1);
                *stSlaveCmpList.pulCurrent++ = BCHP_MASK(CMP_0_CANVAS_CTRL, ENABLE);
                #endif
                *stSlaveCmpList.pulCurrent++ = BRDC_OP_COND_SKIP(BRDC_Variable_1);
                pulSlaveRulMarker = stSlaveCmpList.pulCurrent++;/* place holder */

                BVDC_P_Compositor_BuildSyncSlipRul_isr(hWindow->hCompositor,
                    &stSlaveCmpList, (hSource->bFieldSwap) ?
                    BVDC_P_NEXT_POLARITY(hSource->eNextFieldId) : hSource->eNextFieldId,
                    true /* bBuildCanvasCtrl */);

                /* fill the place holder for conditional skip command */
                *pulSlaveRulMarker = stSlaveCmpList.pulCurrent - pulSlaveRulMarker - 1;

                /* Updated lists count and assign the RUL to both t/b display slots */
                BVDC_P_WriteListInfo_isr(&stSlaveCmpList, hSlaveCmpList);
                BRDC_Slots_SetList_isr(hWindow->hCompositor->ahSlot, hSlaveCmpList, BVDC_P_CMP_MAX_SLOT_COUNT); /* two slots per display */
                BDBG_MSG(("MFD[%u] built sync slave CMP[%u] RUL when WIN[%u] shutdown.", hSource->eId, hWindow->hCompositor->eId, hWindow->eId));
            }
            #endif
        }

        /* build compositor convas ctrl after syncLock window RUL is built */
        if(hSource->hSyncLockCompositor && (0 == ulPictureIdx) && !bBuiltCanvasCtrlWithWin)
        {
            BVDC_P_Compositor_BuildConvasCtrlRul_isr(hSource->hSyncLockCompositor, &stList);
        }


        /* save the previous picture's CRC key flag, to be used on next vsync;
         * update current pictures. */
        if(!pCurPic->bMute)
        {
            hSource->bCaptureCrc  = pCurPic->bCaptureCrc;
            hSource->stSourceCbData.ulIdrPicId   = pCurPic->ulIdrPicID;
            hSource->stSourceCbData.lPicOrderCnt = pCurPic->int32_PicOrderCnt;
            hSource->stSourceCbData.eSourcePolarity = pCurPic->eSourcePolarity;
        }
        else
        {
            /* if muted, don't capture CRC! */
            hSource->bCaptureCrc  = false;
        }

        /* context swap:
         * Note this may not be optimal but can avoid unnecessary bug and confusion later! */
        *pCurPic = *pNewPic;

        /* reset flag */
        hSource->bPictureChanged = false;
        hSource->bRasterChanged = false;

#if BVDC_P_SUPPORT_MOSAIC_MODE
        /* if combo trigger for the slave slot's trigger */
        if((ulConnectedWindow > 1) &&
           (ulMosaicCount > 1 ) && (ulPictureIdx == 0) &&
           BVDC_P_SRC_VALID_COMBO_TRIGGER(hSource->eCombTrigger))
        {
            BDBG_ASSERT(ulConnectedWindow == hSource->ulConnectedWindow);
            eTrigger = hSource->eCombTrigger;
            BRDC_SetComboTriggers_isr(hSource->hVdc->hRdc, hSource->eCombTrigger,
                BRDC_ComboTrigMode_eAllTriggers, hSource->aeCombTriggerList, ulConnectedWindow,
                &stList.pulCurrent);
        }
#else
        BSTD_UNUSED(ulConnectedWindow);
#endif
        /* Update pList */
        BVDC_P_WriteListInfo_isr(&stList, BDC_P_LIST_SEL(hList, hListSlave, ulPictureIdx));

        /* increment picture index for the next round;
           if input pictures are more than the mosaic count, drop it; */
        if(++ulPictureIdx >= ulMosaicCount)
        {
            break;
        }

        /* Exhaust the picture list; if input pictures are less than mosaic count,
           ignore the extra mosaics;
           Note: window shutdown process shall not loop since window state machine
           relies on the RUL execution status;  */
        if(!(pNewPic = pNewPic->pNext) || bBuildBoth)
        {
            break;
        }

        /* if channel id is larger than mosaic count, ignore the rest of picture list */
        if(pNewPic->ulChannelId >= ulMosaicCount)
        {
            break;
        }

        /* Only mosaic mode can go beyond this point; do it once; */
        if(ulPictureIdx == 1)
        {
            /* MosaicMode: get next slave slot and list */
            BVDC_P_SRC_GET_NEXT_SLAVE_RUL_IDX(hSource);
            hSlotSlave = hSource->hSlotSlave;

            /* Get the current slave list pointed by ulSlaveRulIdx */
            hListSlave = hSource->ahListSlave[hSource->ulSlaveRulIdx];

            BDBG_ASSERT(hSlotSlave);
            BDBG_ASSERT(hListSlave);

            /* Update the status of last executed RUL.
             * Only need to check the master RUL exec status; */
            BRDC_Slot_UpdateLastRulStatus_isr(hSlotSlave, hListSlave, false);
            BRDC_List_SetNumEntries_isr(hListSlave, 0);

            /* clear the master flag to avoid building capture timestamp rul */
            stList.bMasterList = false;
        }

        /* config slave slot for the mosaics captures; */
        if(ulPictureIdx == 1)
        {
            /* use capture trigger to drive mosaic slave-RULs; also a place holder of the
             * count for the next slave RUL! */
            BRDC_Slot_RulConfigSlaveTrigger_isr(hSlot, hSlotSlave, hList,
                eTrigger, true);

            /* link to the slave RUL's header */
            BRDC_List_RulSetSlaveListHead_isr(hList, hSlotSlave, hListSlave);
        }
        else
        {
            uint32_t ulScrap;

            /* place holder to config count of next slave RUL */
            BRDC_Slot_RulConfigSlaveTrigger_isr(hSlotSlave, hSlotSlave, hListSlave,
                eTrigger, true);

            /* link to the next slave RUL */
            ulScrap = BRDC_Slot_RulSetNextAddr_isr(hSlotSlave, hListSlave);

            /* calculate ulSubRulEntries */
            BRDC_List_GetNumEntries_isr(hListSlave, &ulSubRulEntries);
            ulSubRulEntries -= ulOldEntries + ulScrap;

            /* update config count with current slave RUL's size into the previous RUL's
             * place holder; */
            BRDC_Slot_RulConfigCount_isr((ulPictureIdx == 2) ? hSlot : hSlotSlave, ulSubRulEntries);
        }
    }   while(true);

    /* Clear old dirty bits. */
    /* TODO: should we move this up, at least for vnet updating? */
    pOldDirty = &hSource->astOldDirty[hSource->eNextFieldId];

    if((BVDC_P_IS_DIRTY(pOldDirty)) &&
       (!stMasterList.bLastExecuted))
    {
        BVDC_P_OR_ALL_DIRTY(pCurDirty, pOldDirty);
    }

    /* Copy cur dirty bits to "old" and clear cur dirty bits. */
    *pOldDirty = *pCurDirty;
    BVDC_P_CLEAN_ALL_DIRTY(pCurDirty);

    /* SWAP: Program the hw, to setup the nEntries for the slot with this new RUL. */
    /* MosaicMode: don't overwrite the first slot config of the RUL chain */
    if(ulPictureIdx >= 2)
    {
        /* chained sub-RULs tail: disable itself */
        BRDC_Slot_RulConfigSlaveListTail_isr(hSlotSlave, hListSlave);

        /* calculate ulSubRulEntries */
        BRDC_List_GetNumEntries_isr(hListSlave, &ulSubRulEntries);
        ulSubRulEntries -= ulOldEntries;

        /* update config count with current slave RUL's size into the previous RUL's
         * place holder; */
        BRDC_Slot_RulConfigCount_isr((ulPictureIdx == 2)? hSlot : hSlotSlave, ulSubRulEntries);

        /* flush slave RULs */
        BRDC_Slot_FlushCachedList_isr(hSlotSlave, hListSlave);
    }

    /* Put on t/b/f slots always */
    /* PsF: build RUL to chop its own size! */
    if(hSource->bPsfScanout)
    {
        for(i = 0; i < BVDC_P_SRC_MAX_SLOT_COUNT; i++)
        {
            BRDC_Slot_RulConfigRulSize_isr(hSource->ahSlot[i], hList, stList.ulPsfMark);
        }
    }
    BRDC_Slots_SetList_isr(hSource->ahSlot, hList, BVDC_P_SRC_MAX_SLOT_COUNT);

#if BVDC_P_SUPPORT_STG
    /* NRT STG host arm after the new RUL is installed to HW */
    if(hSource->hSyncLockCompositor &&
       (BVDC_P_DISPLAY_USED_STG(hSource->hSyncLockCompositor->hDisplay->eMasterTg)) &&
       (hSource->hSyncLockCompositor->hDisplay->stCurInfo.bStgNonRealTime ||
       (hSource->hSyncLockCompositor->hDisplay->stStgChan.bModeSwitch)))
    {
        uint32_t ulStgTriggerToBeArmed =
            hSource->hSyncLockCompositor->hDisplay->ulStgTriggerToBeArmed;

        if(hSource->hSyncLockCompositor->hDisplay->stCurInfo.bStgNonRealTime) {
            /* swap immediate trigger polarity for ignore picture to REPEAT_POLARITY: Note sync-locked display RUL check this
               scratch register to decide which source slot (t/b) to trigger on the fly */
            BRDC_WriteScratch_isrsafe(hSource->hVdc->hRegister, hSource->ulScratchPolReg, hSource->hSyncLockCompositor->bStgIgnorePicture);
            BDBG_MSG(("Src[%u] isr wrote scratch[%u], pol[%u][%u]", hSource->eId,hSource->hSyncLockCompositor->bStgIgnorePicture, ((BAVC_MVD_Field*)pvMvdField)->eInterruptPolarity, pNewPic->eInterruptPolarity));
        } else if(hSource->hSyncLockCompositor->hDisplay->stStgChan.bModeSwitch) {
            BRDC_WriteScratch_isrsafe(hSource->hVdc->hRegister, hSource->ulScratchPolReg, 0);/* reset scratch reg when switched to rt mode */
#if BVDC_P_STG_NRT_CADENCE_WORKAROUND /* for STG hw that cannot repeat trigger polarity */
            hSource->bToggleCadence = false; /* reset the toggle stg cadence for workaround */
#endif
        }

        if(ulStgTriggerToBeArmed)
        {
            /* make sure display isr done before set host arm*/
            BREG_Write32_isr(hSource->hVdc->hRegister, BCHP_VIDEO_ENC_STG_0_HOST_ARM + hSource->hSyncLockCompositor->hDisplay->ulStgRegOffset, 1);
            BDBG_MSG(("Src[%u] isr wrote host arm, pol[%u][%u]", hSource->eId, ((BAVC_MVD_Field*)pvMvdField)->eInterruptPolarity, pNewPic->eInterruptPolarity));
            hSource->hSyncLockCompositor->hDisplay->ulStgTriggerToBeArmed = 0;
            hSource->hSyncLockCompositor->hDisplay->stStgChan.bModeSwitch = false;/* clear it if src isr is later than disp isr */
#if BVDC_P_STG_NRT_CADENCE_WORKAROUND /* for STG hw that cannot repeat trigger polarity */
            hSource->bToggleCadence ^= hSource->hSyncLockCompositor->bStgIgnorePicture; /* every ignore picture needs to toggle stg cadence for workaround */
            BDBG_MSG(("Src[%u] bToggleCad = %u", hSource->eId, hSource->bToggleCadence));
#endif
        }
        else
            /* write bit 1 to indicate src isr done with programming */
            hSource->hSyncLockCompositor->hDisplay->ulStgTriggerToBeArmed = 2;
            BDBG_MSG(("Src[%u] to host arm", hSource->eId));
    }
#endif

MpegDataReady_isr_Done:
    /* Set apply done for all slip windows that already built writer */
    if(hSource->hSyncLockCompositor)
    {
        for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
        {
            /* SKIP: If it's just created or inactive no need to build ruls. */
            if(!hSource->hSyncLockCompositor->ahWindow[i] ||
               BVDC_P_STATE_IS_CREATE(hSource->hSyncLockCompositor->ahWindow[i]) ||
               BVDC_P_STATE_IS_INACTIVE(hSource->hSyncLockCompositor->ahWindow[i]))
            {
                continue;
            }

            hWindow = hSource->hSyncLockCompositor->ahWindow[i];
            if(!hWindow->bSyncLockSrc || hSource->hSyncLockCompositor->ulSlip2Lock)
            {
                if(hWindow->bSlipBuiltWriter)
                {
                    if(hWindow->bSetAppliedEventPending &&
                       (BVDC_P_IS_CLEAN(&hWindow->stCurInfo.stDirty) ||
                        ((hWindow->stCurInfo.stDirty.stBits.bSrcPending ||
                          hWindow->stCurInfo.stDirty.stBits.bBufferPending) &&
                         !hWindow->stCurInfo.stDirty.stBits.bShutdown)))
                    {
                        hWindow->bSetAppliedEventPending = false;
                        BDBG_MSG(("Slip Window[%d] set apply done event", hWindow->eId));
                        BKNI_SetEvent_isr(hWindow->hAppliedDoneEvent);
                    }
                    hWindow->bSlipBuiltWriter = false;
                }
            }
        }
    }

    /* apply done event early for sync locked windows and windows with
       identical timing (has source driven by its compositor.  Fixes
       ApplyChanges taking more than one vsync to complete.
       Usually handled in BVDC_P_Window_UpdateState_isr() */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        /* SKIP: If it's just created or inactive no need to build ruls. */
        if(!hSource->ahWindow[i] ||
            BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
        {
            continue;
        }

        hWindow = hSource->ahWindow[i];
        BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

        if(((hWindow->hCompositor->hSyncLockSrc == hSource) ||
            (hWindow->hCompositor->hForceTrigPipSrc == hSource)) &&
            (hWindow->bSetAppliedEventPending) &&
            ((BVDC_P_IS_CLEAN(&hWindow->stCurInfo.stDirty)) ||
            ((hWindow->stCurInfo.stDirty.stBits.bSrcPending ||
              hWindow->stCurInfo.stDirty.stBits.bBufferPending) &&
            !hWindow->stCurInfo.stDirty.stBits.bShutdown)))
        {
            hWindow->bSetAppliedEventPending = false;
            BDBG_MSG(("Window[%d] set apply done event", hWindow->eId));
            hWindow->bSlipBuiltWriter = false;
            BKNI_SetEvent_isr(hWindow->hAppliedDoneEvent);
        }
    }

    BVDC_P_CHECK_CS_LEAVE_VDC(hSource->hVdc);
    BDBG_LEAVE(BVDC_Source_MpegDataReady_isr);
    return;
}

/* End of File */
