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
#include "bstd.h"
#include "bkni.h"
#include "bvdc_priv.h"
#include "bvdc_mad_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_memconfig_priv.h"


#if (BVDC_P_SUPPORT_MCVP)
#include "bvdc_mcdi_priv.h"
#endif

BDBG_MODULE(BVDC_MEMCONFIG);

/* This is same as hMcdi->ulMosaicMaxChannels:
    MDI_TOP_0_HW_CONFIGURATION.MULTIPLE_CONTEXT  */
#define BVDC_P_DEINTERLACE_MAX_MOSAIC_CHANNEL     (6)

/* Table indicates if display is stg */
static const bool abStg[BVDC_MAX_DISPLAYS] =
{
#if (BCHP_CHIP==7125)  || (BCHP_CHIP==7231)  || \
    (BCHP_CHIP==7344)  || (BCHP_CHIP==7346)  || \
    (BCHP_CHIP==7358)  || (BCHP_CHIP==7360)  || (BCHP_CHIP==7362)  || \
    (BCHP_CHIP==7408)  || \
    (BCHP_CHIP==7420)  || (BCHP_CHIP==7422)  || (BCHP_CHIP==7429)  || \
    (BCHP_CHIP==7468)  || (BCHP_CHIP==7543)  || (BCHP_CHIP==7550)  || \
    (BCHP_CHIP==7552)  || (BCHP_CHIP==7563)  || (BCHP_CHIP==7584)  || \
    (BCHP_CHIP==7228)  || (BCHP_CHIP==75635) || (BCHP_CHIP==73625) || \
    (BCHP_CHIP==75845) || (BCHP_CHIP==74295) || (BCHP_CHIP==11360) || \
    (BCHP_CHIP==7271)  || (BCHP_CHIP==73465) || (BCHP_CHIP==75525) || \
    (BCHP_CHIP==7268)
    /* stg not supported */
    /*  Disp0    Disp1    Disp2    Disp3    disp4   disp5   disp6 */
            0,       0,       0,       0,       0,      0,       0

#elif (BCHP_CHIP==7425) && (BCHP_VER < BCHP_VER_B0)
    /* 1 stg */
    /*  Disp0    Disp1    Disp2    Disp3    disp4   disp5   disp6 */
            0,       0,       0,       1,       0,      0,       0

#elif (BCHP_CHIP==7425) &&(BCHP_VER >= BCHP_VER_B0)
    /* 2 stg */
    /*  Disp0    Disp1    Disp2    Disp3    disp4   disp5   disp6 */
            0,       0,       1,       1,       0,      0,       0

#elif (BCHP_CHIP==7366)
    /* 2 stg */
    /*  Disp0    Disp1    Disp2    Disp3    disp4   disp5   disp6 */
            0,       1,       1,       0,       0,      0,       0

#elif (BCHP_CHIP==7364)|| (BCHP_CHIP==7250) || (BCHP_CHIP==7586)
    /* 2 stg */
    /*  Disp0    Disp1    Disp2    Disp3    disp4   disp5   disp6 */
            0,       1,       0,       0,       0,      0,       0

#elif ((BCHP_CHIP==7439) && (BCHP_VER>=BCHP_VER_B0))
    /* 2 stg: Special case */
    /*  Disp0    Disp1    Disp2    Disp3    disp4   disp5   disp6 */
            0,       0,       1,       1,       0,      0,       0

#elif (BCHP_CHIP==7439) || \
      ((BCHP_CHIP==74371) && (BCHP_VER==BCHP_VER_A0))

    /* 2 stg: Special case */
    /*  Disp0    Disp1    Disp2    Disp3    disp4   disp5   disp6 */
            1,       0,       1,       0,       0,      0,       0

#elif (BCHP_CHIP==7435)
    /* 4 stg */
    /*  Disp0    Disp1    Disp2    Disp3    disp4   disp5   disp6 */
            0,       0,       1,       1,       1,      1,       0

#elif (BCHP_CHIP==7445)
    /* 4 stg */
    /*  Disp0    Disp1    Disp2    Disp3    disp4   disp5   disp6 */
            0,       0,       0,       1,       1,      1,       1

#else
#error "Port reqired for stg table."
#endif
};

/* Table indicates number of video window on a display */
static const uint32_t  aulMaxVideoWindowCount[BVDC_MAX_DISPLAYS] =
{
    BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT,
    BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT,
    BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT,
    BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT,
    BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT,
    BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT,
    BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT
};

/* Table indicates number of clear rects for each video window on a display */
static const uint32_t aaulClearRectCount[BVDC_MAX_DISPLAYS][BVDC_MAX_VIDEO_WINDOWS] =
{
    { BVDC_P_CMP_0_V0_CLEAR_RECTS, BVDC_P_CMP_0_V1_CLEAR_RECTS },
    { BVDC_P_CMP_1_V0_CLEAR_RECTS, BVDC_P_CMP_1_V1_CLEAR_RECTS },
    { BVDC_P_CMP_2_V0_CLEAR_RECTS,                           0 },
    { BVDC_P_CMP_3_V0_CLEAR_RECTS,                           0 },
    { BVDC_P_CMP_4_V0_CLEAR_RECTS,                           0 },
    { BVDC_P_CMP_5_V0_CLEAR_RECTS,                           0 },
    { BVDC_P_CMP_6_V0_CLEAR_RECTS,                           0 }
};

/* Table indicates memc index for each video window on a display */
static const uint32_t aaulMemcIndex[BVDC_MAX_DISPLAYS][BVDC_MAX_VIDEO_WINDOWS] =
{
#if (BCHP_CHIP==11360)
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid},
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid }

#elif (BCHP_CHIP==7420)
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid }

#elif (BCHP_CHIP==7422)
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid }

#elif (BCHP_CHIP==7425)
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid }

#elif (BCHP_CHIP==7435)
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid }

#elif (BCHP_CHIP==7445)
    { BBOX_MemcIndex_1, BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_2, BBOX_MemcIndex_2 },
    { BBOX_MemcIndex_1, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_0, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_0, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_0, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_0, BBOX_MemcIndex_Invalid }

#elif (((BCHP_CHIP==7439) && (BCHP_VER==BCHP_VER_A0))  || \
       ((BCHP_CHIP==7366) && (BCHP_VER==BCHP_VER_A0)) || \
       ((BCHP_CHIP==74371) && (BCHP_VER==BCHP_VER_A0)))
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },

#elif ((BCHP_CHIP==7439) && (BCHP_VER >= BCHP_VER_B0))
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },

#elif ((BCHP_CHIP==7366) && (BCHP_VER >= BCHP_VER_B0))
    /* Box mode 1 */
    { BBOX_MemcIndex_1,       BBOX_MemcIndex_1 },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },

#elif ((BCHP_CHIP==7401)  || (BCHP_CHIP==7403)  || (BCHP_CHIP==7408)  || \
       (BCHP_CHIP==7410)  || (BCHP_CHIP==7125)  || (BCHP_CHIP==7468)  || \
       (BCHP_CHIP==7550)  || (BCHP_CHIP==7552)  || (BCHP_CHIP==7358)  || \
       (BCHP_CHIP==7360)  ||  \
       (BCHP_CHIP==7346)  || (BCHP_CHIP==7344)  || (BCHP_CHIP==7231)  || \
       (BCHP_CHIP==7428)  || (BCHP_CHIP==7429)  || (BCHP_CHIP==7584)  || \
       (BCHP_CHIP==7563)  || (BCHP_CHIP==7543)  || (BCHP_CHIP==7362)  || \
       (BCHP_CHIP==7364)  || (BCHP_CHIP==7228)  || (BCHP_CHIP==7250)  || \
       (BCHP_CHIP==75635) || (BCHP_CHIP==7586)  || (BCHP_CHIP==73625) || \
       (BCHP_CHIP==75845) || (BCHP_CHIP==74295) || (BCHP_CHIP==7271)  || \
       (BCHP_CHIP==73465) || (BCHP_CHIP==75525) || (BCHP_CHIP==7268))
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_0,       BBOX_MemcIndex_0 },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },
    { BBOX_MemcIndex_Invalid, BBOX_MemcIndex_Invalid },

#else
#error "Port reqired for memc index table."
#endif
};


/***************************************************************************
 *
 */
BERR_Code BVDC_P_MemConfig_GetDefaultRdcSettings
    ( BVDC_RdcMemConfigSettings          *pRdc )
{
    BDBG_ASSERT(pRdc);

#if (BCHP_CHIP==7445)
    pRdc->ulMemcIndex = BBOX_MemcIndex_2;
#else
    pRdc->ulMemcIndex = BBOX_MemcIndex_0;
#endif

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_P_MemConfig_GetDefaultDisplaySettings
    ( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      uint32_t                            ulDispIndex,
      BVDC_DispMemConfigSettings         *pDisplay )
{
    bool   bStg;

    BDBG_ASSERT(pSystemConfigInfo);
    BDBG_ASSERT(pDisplay);

    if(pSystemConfigInfo->ulNumCmpUsed < pSystemConfigInfo->ulNumCmp)
    {
        pDisplay->bUsed = true;
        pSystemConfigInfo->ulNumCmpUsed++;
    }
    else
    {
        pDisplay->bUsed = false;
    }

    bStg = abStg[ulDispIndex];

#if (BVDC_P_SUPPORT_DTG_RMD) && (!BVDC_P_SUPPORT_DSCL)
    /* BVDC_P_SUPPORT_DTG_RMD only means vec supports 4kx2k,
     * on 7425Bx, BVN do not support 4kx2k, it uses BVDC_P_SUPPORT_DSCL
     * to scale it up on vec.
     * BVN supports 4kx2k means:
     *      BVDC_P_SUPPORT_DTG_RMD & !BVDC_P_SUPPORT_DSCL
     */
    pDisplay->eMaxDisplayFormat =
        (ulDispIndex == 0) ? BFMT_VideoFmt_e4096x2160p_24Hz :
        (bStg ? BFMT_VideoFmt_e1080p : BFMT_VideoFmt_eNTSC);
#elif (BVDC_P_SUPPORT_3D_VIDEO)
    pDisplay->eMaxDisplayFormat =
        (ulDispIndex == 0) ? BFMT_VideoFmt_e3D_1080p_30Hz :
        (bStg ? BFMT_VideoFmt_e1080p : BFMT_VideoFmt_eNTSC);
#elif (BVDC_P_SUPPORT_1080p_60HZ)
    pDisplay->eMaxDisplayFormat =
        ((ulDispIndex == 0) || bStg) ? BFMT_VideoFmt_e1080p : BFMT_VideoFmt_eNTSC;
#else
    pDisplay->eMaxDisplayFormat =
        ((ulDispIndex == 0) || bStg) ? BFMT_VideoFmt_e1080p_30Hz: BFMT_VideoFmt_eNTSC;
#endif

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_P_MemConfig_GetDefaultWindowSettings
    ( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      uint32_t                            ulDispIndex,
      uint32_t                            ulWinIndex,
      BVDC_WinMemConfigSettings          *pWindow )
{
    bool             bStg, bForceCapture, bSclCapSymmetric;
    BVDC_SclCapBias  eSclCapBias;

    BDBG_ASSERT(pSystemConfigInfo);
    BDBG_ASSERT(pWindow);

    pWindow->bUsed = (ulWinIndex < aulMaxVideoWindowCount[ulDispIndex])
        ? true : false;

    if(!pWindow->bUsed)
        return BERR_SUCCESS;

    /* Get MEMC */
    pWindow->ulMemcIndex = aaulMemcIndex[ulDispIndex][ulWinIndex];

    bStg = abStg[ulDispIndex];

#if (BVDC_P_SUPPORT_DTG_RMD) && (!BVDC_P_SUPPORT_DSCL)
    pWindow->eMaxSourceFormat = BFMT_VideoFmt_e4096x2160p_24Hz;
#elif (BVDC_P_SUPPORT_3D_VIDEO)
    pWindow->eMaxSourceFormat =
        (ulDispIndex) ? BFMT_VideoFmt_e1080p : BFMT_VideoFmt_e3D_1080p_30Hz;
#elif (BVDC_P_SUPPORT_1080p_60HZ)
    pWindow->eMaxSourceFormat = BFMT_VideoFmt_e1080p;
#else
    pWindow->eMaxSourceFormat = BFMT_VideoFmt_e1080p_30Hz;
#endif

#if (BVDC_P_SUPPORT_HDDVI || BVDC_P_NUM_656IN_SUPPORT)
    pWindow->bNonMfdSource = bStg ? false : true;
#else
    pWindow->bNonMfdSource = false;
#endif

    pWindow->bMosaicMode = aaulClearRectCount[ulDispIndex][ulWinIndex]
        ? true : false;

#if (BVDC_P_SUPPORT_3D_VIDEO)
    pWindow->b3DMode = ulDispIndex ? false : true;
#else
    pWindow->b3DMode = false;
#endif

    pWindow->bPip = ulWinIndex ? true : false;
    pWindow->b5060Convert = false;
    pWindow->bSlave_24_25_30_Display = (ulDispIndex == 1) ? true : false;

    /* TODO: table for nrt_stg? */
    BVDC_P_Window_Rts_Init(
#if BVDC_P_SUPPORT_XCODE_WIN_CAP
        bStg, false,
#endif
        &bForceCapture, &bSclCapSymmetric, &eSclCapBias, NULL);

    pWindow->bSmoothScaling = (eSclCapBias != BVDC_SclCapBias_eAuto)
        ? true : false;

    pWindow->bPsfMode = bForceCapture;
    pWindow->bSideBySide = bForceCapture;
    pWindow->bBoxDetect = bForceCapture;
    pWindow->bArbitraryCropping = bForceCapture;
    pWindow->bIndependentCropping = bForceCapture;

    if(pWindow->bNonMfdSource)
    {
        pWindow->bSyncSlip = true;
    }
    else
    {
        if(ulDispIndex <= 1)
            pWindow->bSyncSlip = !((ulDispIndex == 0) && (ulWinIndex == 0));
        else
            pWindow->bSyncSlip = !bStg;
    }

    if(pSystemConfigInfo->ulNumMadUsed < pSystemConfigInfo->ulNumMad)
    {
#if (BCHP_CHIP==7445)
        pWindow->eDeinterlacerMode =
            ((ulDispIndex == 0) || bStg)
            ? BVDC_DeinterlacerMode_eBestQuality : BVDC_DeinterlacerMode_eNone;
#else
        pWindow->eDeinterlacerMode =
            ( ((ulDispIndex == 0) || bStg) && (ulWinIndex == 0) )
            ? BVDC_DeinterlacerMode_eBestQuality : BVDC_DeinterlacerMode_eNone;
#endif

        pSystemConfigInfo->ulNumMadUsed +=
            BVDC_P_MEMCONFIG_DEINTERLACER_ON(pWindow->eDeinterlacerMode);
    }
    else
    {
        pWindow->eDeinterlacerMode = BVDC_DeinterlacerMode_eNone;
    }

    pWindow->ulMadMemcIndex =
        BVDC_P_MEMCONFIG_DEINTERLACER_ON(pWindow->eDeinterlacerMode)
        ? pWindow->ulMemcIndex : BBOX_MemcIndex_Invalid;

    pWindow->ulAdditionalBufCnt = 0;
    pWindow->bLipsync = ((ulDispIndex < 2) && (ulWinIndex == 0));

    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_P_MemConfigInfo_Init
    ( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo )
{
    BDBG_ASSERT(pSystemConfigInfo);

    pSystemConfigInfo->ulNumCmp  = BVDC_P_GetNumCmp(&s_VdcFeatures);
    pSystemConfigInfo->ulNumStg  = BVDC_P_SUPPORT_STG;
    pSystemConfigInfo->ulNumMad  = BVDC_P_SUPPORT_MAD + BVDC_P_SUPPORT_MCVP;
    pSystemConfigInfo->ulNumMadr = BVDC_P_SUPPORT_MADR;

    pSystemConfigInfo->ulNumCmpUsed  = 0;
    pSystemConfigInfo->ulNumStgUsed  = 0;
    pSystemConfigInfo->ulNumMadUsed  = 0;
    pSystemConfigInfo->ulNumMadrUsed = 0;

    pSystemConfigInfo->bEnableShare4Lipsync = false;
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_P_MemConfig_GetBufSize
    ( const BVDC_Heap_Settings           *pHeapSettings,
      BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo )
{
    BVDC_P_BufferHeap_SizeInfo  *pHeapSizeInfo;
    BVDC_P_BufferHeapContext     stBufferHeap;

    BDBG_ASSERT(pHeapSettings);
    BDBG_ASSERT(pSystemConfigInfo);

    pHeapSizeInfo = &pSystemConfigInfo->stHeapSizeInfo;

    BVDC_P_BufferHeap_GetHeapOrder(pHeapSettings, pHeapSizeInfo, &stBufferHeap);

    BDBG_MSG(("4HD bufSize    : %d",
        pHeapSizeInfo->aulBufSize[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD]]));
    BDBG_MSG(("4HD_Pip bufSize: %d",
        pHeapSizeInfo->aulBufSize[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD_Pip]]));
    BDBG_MSG(("2HD bufSize    : %d",
        pHeapSizeInfo->aulBufSize[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD]]));
    BDBG_MSG(("2HD_Pip bufSize: %d",
        pHeapSizeInfo->aulBufSize[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD_Pip]]));
    BDBG_MSG(("HD bufSize     : %d",
        pHeapSizeInfo->aulBufSize[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD]]));
    BDBG_MSG(("HD_Pip bufSize : %d",
        pHeapSizeInfo->aulBufSize[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD_Pip]]));
    BDBG_MSG(("SD bufSize     : %d",
        pHeapSizeInfo->aulBufSize[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD]]));
    BDBG_MSG(("SD_Pip bufSize : %d",
        pHeapSizeInfo->aulBufSize[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD_Pip]]));

    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_P_MemConfig_Validate
    ( const BVDC_MemConfigSettings       *pMemConfigSettings,
      BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo )
{
    uint32_t   ulLipsyncCnt = 0;
    uint32_t   ulDispIndex, ulWinIndex;
    uint32_t   ulNumCmp = 0, ulNumMad = 0;
    uint32_t   ulMemcIndex = BBOX_MemcIndex_Invalid;
    const BFMT_VideoInfo  *pSrcFmtInfo, *pDispFmtInfo;

    BDBG_ASSERT(pMemConfigSettings);
    BDBG_ASSERT(pSystemConfigInfo);

    for(ulDispIndex = 0; ulDispIndex < BVDC_MAX_DISPLAYS; ulDispIndex++)
    {
        BVDC_DispMemConfigSettings  *pDisplay;

        pDisplay = (BVDC_DispMemConfigSettings  *)(&pMemConfigSettings->stDisplay[ulDispIndex]);
        BDBG_ASSERT(pDisplay);

        pDispFmtInfo = BFMT_GetVideoFormatInfoPtr(pDisplay->eMaxDisplayFormat);
        BDBG_MSG(("Disp[%d]: Used(%d) %s", ulDispIndex, pDisplay->bUsed,
            pDispFmtInfo->pchFormatStr));

        if(pDisplay->bUsed)
            ulNumCmp++;

        for(ulWinIndex = 0; ulWinIndex < BVDC_MAX_VIDEO_WINDOWS; ulWinIndex++)
        {
            BVDC_WinMemConfigSettings  *pWindow;

            pWindow = &(pDisplay->stWindow[ulWinIndex]);
            BDBG_ASSERT(pWindow);

            pSrcFmtInfo = BFMT_GetVideoFormatInfoPtr(pWindow->eMaxSourceFormat);
            if(pWindow->bUsed)
            {
                if(pWindow->bLipsync)
                {
                    ulLipsyncCnt++;
                    if(ulLipsyncCnt == 1)
                    {
                        ulMemcIndex = pWindow->ulMemcIndex;
                        pSystemConfigInfo->bEnableShare4Lipsync = false;
                    }
                    else if(ulLipsyncCnt == 2)
                    {
                        pSystemConfigInfo->bEnableShare4Lipsync =
                            (ulMemcIndex == pWindow->ulMemcIndex) ? true : false;
                    }
                    else
                    {
                        pSystemConfigInfo->bEnableShare4Lipsync = false;
                    }
                }

                BDBG_MSG(("    Win[%d]: Used(%d) CapMemc[%d] MadMemc[%d] %s",
                    ulWinIndex, pWindow->bUsed, pWindow->ulMemcIndex,
                    pWindow->ulMadMemcIndex, pSrcFmtInfo->pchFormatStr));
                BDBG_MSG(("    Win[%d]: NotMfd Slip Pip Lip Mos Mad Smoo 3D Psf Side Box ACrop ICrop 5060 Slave AddBuf",
                    ulWinIndex));
                BDBG_MSG(("    Win[%d]: %4d %5d %3d %3d %3d %3d %4d %3d %2d %4d %3d %4d %5d %5d %4d %5d",
                    ulWinIndex, pWindow->bNonMfdSource, pWindow->bSyncSlip,
                    pWindow->bPip, pWindow->bLipsync, pWindow->bMosaicMode,
                    pWindow->eDeinterlacerMode, pWindow->bSmoothScaling,
                    pWindow->b3DMode, pWindow->bPsfMode, pWindow->bSideBySide,
                    pWindow->bBoxDetect, pWindow->bArbitraryCropping,
                    pWindow->bIndependentCropping, pWindow->b5060Convert,
                    pWindow->bSlave_24_25_30_Display, pWindow->ulAdditionalBufCnt));

            }
            else
            {
                BDBG_MSG(("    Win[%d]: Used(%d)", ulWinIndex, pWindow->bUsed));
            }

            /* Skip if window is not used */
            if(pWindow->bUsed && BVDC_P_MEMCONFIG_DEINTERLACER_ON(pWindow->eDeinterlacerMode))
                ulNumMad++;
        }
    }

    if(ulNumCmp > pSystemConfigInfo->ulNumCmp)
    {
        BDBG_ERR(("Numer of display %d exceeds total number of available display %d",
            ulNumCmp, pSystemConfigInfo->ulNumCmp));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BSTD_UNUSED(pSrcFmtInfo);
    BSTD_UNUSED(pDispFmtInfo);
    return BERR_SUCCESS;

}


/***************************************************************************
 *
 */
BERR_Code BVDC_P_MemConfig_GetWindowInfo
    ( BVDC_WinMemConfigSettings          *pWindow,
      BVDC_DispMemConfigSettings         *pDisplay,
      BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      uint32_t                            ulDispIndex,
      uint32_t                            ulWinIndex,
      BVDC_P_MemConfig_WindowInfo        *pWinConfigInfo )
{
    uint32_t        ulSrcSize, ulDispSize;
    BFMT_VideoFmt   eSrcFormat, eDispFormat;

#if (!BVDC_P_SUPPORT_MCVP)
    BSTD_UNUSED(pSystemConfigInfo);
    BSTD_UNUSED(ulDispIndex);
    BSTD_UNUSED(ulWinIndex);
#endif

    BDBG_ASSERT(pWindow);
    BDBG_ASSERT(pWinConfigInfo);
    BDBG_ASSERT(pWindow->bUsed);

    BKNI_Memset((void*)(pWinConfigInfo->aulCapBufCnt), 0x0,
        sizeof(uint32_t)*BVDC_P_BufferHeapId_eCount);
    BKNI_Memset((void*)(pWinConfigInfo->aulMadBufCnt), 0x0,
        sizeof(uint32_t)*BVDC_P_BufferHeapId_eCount);

    pWinConfigInfo->bSyncLock = !pWindow->bNonMfdSource && !pWindow->bSyncSlip;
    pWinConfigInfo->bCapture =
        pWindow->bMosaicMode ||
        pWindow->b3DMode ||
        pWindow->bPsfMode ||
        pWindow->bSideBySide ||
        pWindow->bPip ||
        pWindow->bSmoothScaling ||
        pWindow->bBoxDetect ||
        pWindow->bArbitraryCropping ||
        pWindow->bIndependentCropping ||
        pWindow->bLipsync ||
        pWindow->b5060Convert ||
        pWindow->bSlave_24_25_30_Display;
    pWinConfigInfo->bPip = pWindow->bPip;
    pWinConfigInfo->bLipsync = pWindow->bLipsync;
    pWinConfigInfo->b5060Convert = pWindow->b5060Convert;
    pWinConfigInfo->bSlave_24_25_30_Display = pWindow->bSlave_24_25_30_Display;
    pWinConfigInfo->b3d = BFMT_IS_3D_MODE(pDisplay->eMaxDisplayFormat);
    pWinConfigInfo->bMosaicMode = pWindow->bMosaicMode;
    pWinConfigInfo->ulAdditionalBufCnt = pWindow->ulAdditionalBufCnt;
    pWinConfigInfo->eDeinterlacerMode =
        (pWindow->ulMadMemcIndex == BBOX_MemcIndex_Invalid)
        ? BVDC_DeinterlacerMode_eNone : pWindow->eDeinterlacerMode;

#if (BVDC_P_SUPPORT_MCVP)
    if(pSystemConfigInfo->ulNumMad == pSystemConfigInfo->ulNumMadr)
    {
        /* NO Mcvp, all deinterlacers are MADR */
        pWinConfigInfo->bMadr = true;
    }
    else
    {
        if(abStg[ulDispIndex])
        {
            /* STG always has MADR */
            pWinConfigInfo->bMadr = true;
        }
        else
        {
            /* MCVP on main, MADR on PIP */
            pWinConfigInfo->bMadr = (ulWinIndex == 0) ? false : true;
        }
    }
#else
    pWinConfigInfo->bMadr = false;
#endif

    /* Get format */
    eSrcFormat = pWindow->eMaxSourceFormat;
    eDispFormat = pDisplay->eMaxDisplayFormat;

    ulSrcSize = BVDC_P_BufferHeap_GetHeapSize(
        BFMT_GetVideoFormatInfoPtr(eSrcFormat),
        BVDC_P_CAP_PIXEL_FORMAT_8BIT422, false, NULL, NULL, NULL);
    ulDispSize = BVDC_P_BufferHeap_GetHeapSize(
        BFMT_GetVideoFormatInfoPtr(eDispFormat),
        BVDC_P_CAP_PIXEL_FORMAT_8BIT422, false, NULL, NULL, NULL);

    if(pWindow->bSmoothScaling && (ulDispIndex == 0))
    {
        pWinConfigInfo->eFormat = (ulSrcSize > ulDispSize)
            ? eSrcFormat : eDispFormat;
    }
    else
    {
        pWinConfigInfo->eFormat = (ulSrcSize > ulDispSize)
            ? eDispFormat : eSrcFormat;
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_MemConfig_GetWindowCapBufCnt
    ( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      BVDC_P_MemConfig_WindowInfo        *pWinConfigInfo,
      uint32_t                            ulDispIndex,
      uint32_t                            ulWinIndex )
{
    bool                         bFrameCapture = false;
    uint32_t                     ulCapBufCnt, ulBufSize, ulIndex;
    BVDC_P_BufferHeapId          eBufHeapId;
    BVDC_P_Rect                  stCapRect;
    const BFMT_VideoInfo        *pFmtInfo;
    BVDC_P_BufferHeap_SizeInfo  *pHeapSizeInfo;

#if (!BDBG_DEBUG_BUILD)
    BSTD_UNUSED(ulDispIndex);
#endif

    BDBG_ASSERT(pSystemConfigInfo);
    BDBG_ASSERT(pWinConfigInfo);
    BDBG_ASSERT(pWinConfigInfo->bCapture);

    ulCapBufCnt = pWinConfigInfo->bSyncLock ? 2 : 4;

    if(pSystemConfigInfo->bEnableShare4Lipsync)
    {
        if(pWinConfigInfo->bLipsync && (ulDispIndex == 0))
            ulCapBufCnt += 1;
    }
    else
    {
        if(pWinConfigInfo->bLipsync)
        {
            ulCapBufCnt += (ulDispIndex) ? 2 : 1;
        }
    }

    if(pWinConfigInfo->b5060Convert && !pWinConfigInfo->bSyncLock)
    {
#if BVDC_ENABLE_50HZ_60HZ_FRAME_CAPTURE
        bFrameCapture = true;
#endif
        ulCapBufCnt += 1;
    }

    if(pWinConfigInfo->bSlave_24_25_30_Display  && !pWinConfigInfo->bSyncLock)
    {
        bFrameCapture = true;
        ulCapBufCnt -= 1;
    }

    ulCapBufCnt += pWinConfigInfo->ulAdditionalBufCnt;

    stCapRect.lLeft = stCapRect.lLeft_R = stCapRect.lTop = 0;
    pFmtInfo = BFMT_GetVideoFormatInfoPtr(pWinConfigInfo->eFormat);
    if(pWinConfigInfo->bPip)
    {
        stCapRect.ulWidth = pFmtInfo->ulWidth / 2;
        stCapRect.ulHeight = pFmtInfo->ulHeight / 2;
    }
    else
    {
        stCapRect.ulWidth = pFmtInfo->ulWidth;
        stCapRect.ulHeight = pFmtInfo->ulHeight;
    }

    /* TODO: 10bit Capture? */
    pHeapSizeInfo = &pSystemConfigInfo->stHeapSizeInfo;
    BVDC_P_Window_GetBufSize_isr(ulWinIndex, &stCapRect,
        pFmtInfo->bInterlaced && !bFrameCapture, pWinConfigInfo->bMosaicMode,
        pWinConfigInfo->b3d, false, BVDC_P_CAP_PIXEL_FORMAT_8BIT422,
        NULL, BVDC_P_BufHeapType_eCapture, &ulBufSize, BAVC_VideoBitDepth_e8Bit);
    BVDC_P_BufferHeap_GetHeapIdBySize(pHeapSizeInfo, ulBufSize, &eBufHeapId);

    ulIndex = pHeapSizeInfo->aulIndex[eBufHeapId];
    pWinConfigInfo->aulCapBufCnt[ulIndex] = ulCapBufCnt;
    BDBG_MSG(("Disp[%d]Win[%d]     CapBuf: %3d %6d %6d %6d %6d %5d %5d %5d",
        ulDispIndex, ulWinIndex,
        pWinConfigInfo->aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD]],
        pWinConfigInfo->aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD_Pip]],
        pWinConfigInfo->aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD]],
        pWinConfigInfo->aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD_Pip]],
        pWinConfigInfo->aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD]],
        pWinConfigInfo->aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD_Pip]],
        pWinConfigInfo->aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD]],
        pWinConfigInfo->aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD_Pip]]));

    return BERR_SUCCESS;

}


/***************************************************************************
 *
 */
static BERR_Code BVDC_P_MemConfig_GetWindowDeinterlacerBufCnt
    ( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      BVDC_P_MemConfig_WindowInfo        *pWinConfigInfo,
      uint32_t                            ulDispIndex,
      uint32_t                            ulWinIndex )
{
    uint32_t                      ulPixelBufCnt = 0, ulQmBufCnt = 0;
    uint32_t                      ulIndex, ulBufSize;
    uint32_t                      aulMadBufCnt[BVDC_P_BufferHeapId_eCount];
    BVDC_P_Rect                   stMadBufRect;
    BVDC_P_BufferHeapId           eBufHeapId;
    BPXL_Format                   ePxlFormat;
    BVDC_MadGameMode              eMadGameMode;
    BVDC_P_BufferHeap_SizeInfo   *pHeapSizeInfo;
    BVDC_P_Compression_Settings   stCompression;

#if (BVDC_P_SUPPORT_MAD)
    BSTD_UNUSED(pWinConfigInfo);
    BSTD_UNUSED(eMadGameMode);
#endif

#if (!BDBG_DEBUG_BUILD)
    BSTD_UNUSED(ulDispIndex);
#endif

    BDBG_ASSERT(pSystemConfigInfo);
    BDBG_ASSERT(pWinConfigInfo);

    BKNI_Memset((void*)aulMadBufCnt, 0x0,
        sizeof(uint32_t)*BVDC_P_BufferHeapId_eCount);

    stMadBufRect.lLeft = stMadBufRect.lLeft_R = stMadBufRect.lTop = 0;
    pHeapSizeInfo = &pSystemConfigInfo->stHeapSizeInfo;

    BVDC_P_Window_Compression_Init(false, false, NULL, &stCompression, BVDC_P_Mvp_Dcxs);

    BVDC_P_Mad_Init_Default(NULL, &ePxlFormat,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

#if (BVDC_P_SUPPORT_MAD)
    /* MAD */
    ulPixelBufCnt = BVDC_P_MAD_PIXEL_SD_BUFFER_COUNT;
    ulQmBufCnt    = BVDC_P_MAD_QM_BUFFER_COUNT;

#if (BVDC_P_SUPPORT_MAD_SRC_1080I )
    /* HD MAD: 6 HD buffers */
    stMadBufRect.ulWidth = BFMT_1080I_WIDTH;
    stMadBufRect.ulHeight = BFMT_1080I_HEIGHT;
#else
    /* SD MAD: 6 SD buffers */
    stMadBufRect.ulWidth = BFMT_PAL_WIDTH;
    stMadBufRect.ulHeight = BFMT_PAL_HEIGHT;
#endif

#elif (BVDC_P_SUPPORT_MCVP)
    /*MCVP */
    /* TODO: Clean up for mosaic. Box mode? */
#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
    if(pWinConfigInfo->bMosaicMode)
    {
        stMadBufRect.ulWidth = BFMT_PAL_WIDTH;
        stMadBufRect.ulHeight = BFMT_PAL_HEIGHT;
        /* can not share buffers between channels, alloc for worse case */
        stCompression.bEnable = false;
    }
    else
#endif
    {
        stMadBufRect.ulWidth = BFMT_1080I_WIDTH;
        stMadBufRect.ulHeight = BFMT_1080I_HEIGHT;
    }

    if(pWinConfigInfo->eDeinterlacerMode == BVDC_DeinterlacerMode_eLowestLatency)
        eMadGameMode = BVDC_MadGameMode_eMinField_ForceSpatial;
    else
        eMadGameMode = BVDC_MadGameMode_eOff;


    ulPixelBufCnt =BVDC_P_Mcdi_GetPixBufCnt_isr(pWinConfigInfo->bMadr, eMadGameMode);
    ulQmBufCnt = pWinConfigInfo->bMadr ?
        BVDC_P_MAD_QM_BUFFER_COUNT : BVDC_P_MCDI_QM_BUFFER_COUNT;

    /* no QM buffer needed for 1/0 field buffer force spatial */
    ulQmBufCnt = BVDC_P_MAD_SPATIAL(eMadGameMode)? 0:ulQmBufCnt;
#endif

    /* Get Pixel buffers */
    BVDC_P_Window_GetBufSize_isr(ulWinIndex, &stMadBufRect, true,
        false, false, false, ePxlFormat, &stCompression,
        BVDC_P_BufHeapType_eMad_Pixel,  &ulBufSize, BAVC_VideoBitDepth_e8Bit);
#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
    if(pWinConfigInfo->bMosaicMode)
    {
        /* can not share buffers between channels */
        ulBufSize *= BVDC_P_DEINTERLACE_MAX_MOSAIC_CHANNEL;
    }
#endif
    BVDC_P_BufferHeap_GetHeapIdBySize(pHeapSizeInfo, ulBufSize, &eBufHeapId);
    ulIndex = pHeapSizeInfo->aulIndex[eBufHeapId];
    aulMadBufCnt[ulIndex] += ulPixelBufCnt;

    /* Get QM buffers */
    BVDC_P_Window_GetBufSize_isr(ulWinIndex, &stMadBufRect, true,
        false, false, false, ePxlFormat, &stCompression,
        BVDC_P_BufHeapType_eMad_QM, &ulBufSize, BAVC_VideoBitDepth_e8Bit);
#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
    if(pWinConfigInfo->bMosaicMode)
    {
        /* can not share buffers between channels */
        ulBufSize *= BVDC_P_DEINTERLACE_MAX_MOSAIC_CHANNEL;
    }
#endif
    BVDC_P_BufferHeap_GetHeapIdBySize(pHeapSizeInfo, ulBufSize, &eBufHeapId);
    ulIndex = pHeapSizeInfo->aulIndex[eBufHeapId];
    aulMadBufCnt[ulIndex] += ulQmBufCnt;

    BDBG_MSG(("Disp[%d]Win[%d]    %s: %3d %6d %6d %6d %6d %5d %5d %5d",
        ulDispIndex, ulWinIndex,
        pWinConfigInfo->bMadr ? "MadrBuf" : " MadBuf",
        aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD]],
        aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD_Pip]],
        aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD]],
        aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD_Pip]],
        aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD]],
        aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD_Pip]],
        aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD]],
        aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD_Pip]]));

    for(ulIndex = 0; ulIndex < BVDC_P_BufferHeapId_eCount; ulIndex++)
    {
        pWinConfigInfo->aulMadBufCnt[ulIndex] = aulMadBufCnt[ulIndex];
    }

    return BERR_SUCCESS;

}

/***************************************************************************
 *
 */
BERR_Code BVDC_P_MemConfig_GetWindowBufCnt
    ( BVDC_WinMemConfigSettings          *pWindow,
      BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      BVDC_P_MemConfig_WindowInfo        *pWinConfigInfo,
      uint32_t                            ulDispIndex,
      uint32_t                            ulWinIndex )
{
    uint32_t    i;

    BSTD_UNUSED(pWindow);
    BDBG_ASSERT(pSystemConfigInfo);
    BDBG_ASSERT(pWinConfigInfo);

    /* Get capture buffer settings */
    if(pWinConfigInfo->bCapture)
    {
        BVDC_P_MemConfig_GetWindowCapBufCnt(pSystemConfigInfo,
            pWinConfigInfo, ulDispIndex, ulWinIndex);
    }

    /* Get deinterlacer buffer settings */
    if(BVDC_P_MEMCONFIG_DEINTERLACER_ON(pWindow->eDeinterlacerMode) &&
       BVDC_P_MEMCONFIG_DEINTERLACER_ON(pWinConfigInfo->eDeinterlacerMode))
    {
        BVDC_P_MemConfig_GetWindowDeinterlacerBufCnt(pSystemConfigInfo,
            pWinConfigInfo, ulDispIndex, ulWinIndex);
    }

    if(pWinConfigInfo->b3d)
    {
        for(i = 0; i < BVDC_P_BufferHeapId_eCount; i++)
        {
            pWinConfigInfo->aulCapBufCnt[i] *= 2;
            pWinConfigInfo->aulMadBufCnt[i] *= 2;
        }
    }

    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_P_MemConfig_GetWinBufSize
    ( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      BVDC_P_MemConfig_WindowInfo        *pWinConfigInfo,
      uint32_t                           *pulCapSize,
      uint32_t                           *pulMadSize )
{
    uint32_t   i, ulTotalCapSize = 0, ulTotalMadSize = 0;
    BVDC_P_BufferHeap_SizeInfo  *pHeapSizeInfo;

    BDBG_ASSERT(pSystemConfigInfo);
    BDBG_ASSERT(pWinConfigInfo);

    pHeapSizeInfo = &pSystemConfigInfo->stHeapSizeInfo;
    for(i = 0; i < BVDC_P_BufferHeapId_eCount; i++)
    {
        ulTotalCapSize += pHeapSizeInfo->aulBufSize[i] * pWinConfigInfo->aulCapBufCnt[i];
        ulTotalMadSize += pHeapSizeInfo->aulBufSize[i] * pWinConfigInfo->aulMadBufCnt[i];
    }

#if BVDC_P_SUPPORT_MOSAIC_MODE
    /* drain buffer for capture */
    ulTotalCapSize += BVDC_P_ALIGN_UP(16*4, 4);
#endif

    if(pulCapSize)
        *pulCapSize = ulTotalCapSize;

    if(pulMadSize)
        *pulMadSize = ulTotalMadSize;

    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_P_MemConfig_SetBufFormat
    ( const BVDC_Heap_Settings                 *pHeapSettingsIn,
      BVDC_Heap_Settings                 *pHeapSettingsOut )
{
    BDBG_ASSERT(pHeapSettingsIn);

    if(pHeapSettingsOut)
    {
        pHeapSettingsOut->eBufferFormat_4HD = pHeapSettingsIn->eBufferFormat_4HD;
        pHeapSettingsOut->eBufferFormat_2HD = pHeapSettingsIn->eBufferFormat_2HD;
        pHeapSettingsOut->eBufferFormat_HD  = pHeapSettingsIn->eBufferFormat_HD;
        pHeapSettingsOut->eBufferFormat_SD  = pHeapSettingsIn->eBufferFormat_SD;

        pHeapSettingsOut->ePixelFormat_4HD  = pHeapSettingsIn->ePixelFormat_4HD;
        pHeapSettingsOut->ePixelFormat_2HD  = pHeapSettingsIn->ePixelFormat_2HD;
        pHeapSettingsOut->ePixelFormat_HD   = pHeapSettingsIn->ePixelFormat_HD;
        pHeapSettingsOut->ePixelFormat_SD   = pHeapSettingsIn->ePixelFormat_SD;
    }

    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_P_MemConfig_GetRulSize
    ( uint32_t                           *pulRulSize )
{
    uint32_t  i = 0, j = 0;
    uint32_t  ulTotalRulSize = 0;
    uint32_t  ulMasterRulSize, ulSlaveRulSize, ulBoRulSize;
    uint32_t  ulSlotUsed, ulRulAlign;

    ulRulAlign = 32;
    ulMasterRulSize = sizeof(uint32_t) * BVDC_P_MAX_ENTRY_PER_RUL;
    ulMasterRulSize = BVDC_P_ALIGN_UP(ulMasterRulSize, ulRulAlign);
    ulSlaveRulSize = sizeof(uint32_t) * BVDC_P_MAX_ENTRY_PER_MPEG_RUL;
    ulSlaveRulSize = BVDC_P_ALIGN_UP(ulSlaveRulSize, ulRulAlign);
    ulBoRulSize = sizeof(uint32_t) * 0x4000;
    ulBoRulSize = BVDC_P_ALIGN_UP(ulBoRulSize, ulRulAlign);

    /* RUL for CMP*/
    for(i = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT; i++)
    {
        if(s_VdcFeatures.abAvailCmp[i])
        {
            for(j = 0; j < BVDC_P_CMP_MAX_LIST_COUNT; j++)
            {
                ulTotalRulSize += ulMasterRulSize;
            }
        }
    }

    /* RUL for source */
    for(i  = 0; i < BVDC_P_MAX_SOURCE_COUNT; i++)
    {
        if(s_VdcFeatures.abAvailSrc[i] && BVDC_P_SRC_IS_VIDEO(i) &&
           !BVDC_P_SRC_IS_VFD(i))
        {
            ulSlotUsed = BVDC_P_SRC_IS_MPEG(i)
                ? (BVDC_P_SRC_MAX_SLOT_COUNT) : (BVDC_P_SRC_MAX_SLOT_COUNT - 1);

            for(j = 0; j < (ulSlotUsed * BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT); j++)
            {
                ulTotalRulSize += ulMasterRulSize;
            }

            if(BVDC_P_SRC_IS_MPEG(i) && (i < BRDC_MAX_COMBO_TRIGGER_COUNT))
            {
                /* Only need top/bottom slave slots and lists */
                for(j = 0; j < 2 * BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT; j++)
                {
                    ulTotalRulSize += ulSlaveRulSize;
                }
            }
        }
    }

    /* RUL for blockout */
    ulTotalRulSize += ulBoRulSize;

    if(pulRulSize)
        *pulRulSize = ulTotalRulSize;

    return BERR_SUCCESS;
}


/* end of file */
