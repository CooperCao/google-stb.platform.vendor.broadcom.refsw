/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
 ***************************************************************************/

#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_priv_modes.h"
#include "bbox_vdc.h"

#include "bchp_common.h"
#include "bchp_memc_arb_0.h"
#include "bchp_memc_arb_1.h"

BDBG_MODULE(BBOX_PRIV);
BDBG_OBJECT_ID(BBOX_BOX_PRIV);

/* Memc Index for box mode 1. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7435B0_box1 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       1,       Invalid, Invalid, 1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 0      ),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 0      ),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 1      ),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   2
};

/* Memc Index for box mode 2. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7435B0_box2 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       Invalid, Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
    }
   },
   1
};



BERR_Code BBOX_P_ValidateId
    (uint32_t                ulId)
{
    BERR_Code eStatus = BERR_SUCCESS;
    if (ulId == 0 || ulId > BBOX_MODES_SUPPORTED)
    {
        BDBG_ERR(("Box Mode ID %d is not supported.", ulId));
        eStatus = BBOX_ID_NOT_SUPPORTED;
    }
    return eStatus;
}

static void BBOX_P_Vdc_SetSourceLimits
    ( uint32_t                       ulBoxId,
      BBOX_Vdc_Source_Capabilities  *pBoxVdcSrcCap )
{
    uint32_t i;

    BSTD_UNUSED(ulBoxId);

    for (i=0; i < BAVC_SourceId_eMax; i++)
    {
        pBoxVdcSrcCap[i].bAvailable = true;
        pBoxVdcSrcCap[i].bMtgCapable = false;
        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
        pBoxVdcSrcCap[i].eColorSpace = BBOX_VDC_DISREGARD;
        pBoxVdcSrcCap[i].eBpp = BBOX_VDC_DISREGARD;

        switch (i)
        {
#if BCHP_VER <= B0
        case BAVC_SourceId_eGfx5:
        case BAVC_SourceId_eGfx2:
#else
        case BAVC_SourceId_eGfx3:
        case BAVC_SourceId_eGfx2:
#endif
            pBoxVdcSrcCap[i].bAvailable = false;
            break;
        default:
            break;
        }
    }
}

static void BBOX_P_Vdc_SetDisplayLimits
    ( uint32_t                       ulBoxId,
      BBOX_Vdc_Display_Capabilities *pBoxVdcDispCap )
{
    uint32_t i, j;

    switch (ulBoxId)
    {
        case 1:
        case 2:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = true;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
                {
                    pBoxVdcDispCap[i].astWindow[j].bAvailable = ((i<2 && j<=2)||(i>=2 && (j==0 || j==2))) ? true : false;
                    pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                    pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                    pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                    pBoxVdcDispCap[i].astWindow[j].eSclCapBias = ((i>=2) && (j==0 || j==2)) ?
                        BBOX_Vdc_SclCapBias_eAutoDisable : BBOX_VDC_DISREGARD;

                    /* Override the above settings */
                    switch (i)
                    {
                        case 0:
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e1080p;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass1;

                            /* MAD-R is not available for C0V1 */
                            if (j==1)
                            {
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                            }
                            break;
                        case 1:
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;
                            /* Soft (Raaga DSP) transcode */
                            /* MAD-R is not available for C1V0 and C1V1 */
                            pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                            break;
                        case 2:
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p_30Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p_30Hz;
                            pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                            pBoxVdcDispCap[i].stStgEnc.ulStgId = 3;
                            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 1;
                            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
                            break;
                        case 3:
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p_30Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p_30Hz;
                            pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                            pBoxVdcDispCap[i].stStgEnc.ulStgId = 2;
                            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 1;
                            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                            break;
                        case 4:
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p_30Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p_30Hz;
                            pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                            pBoxVdcDispCap[i].stStgEnc.ulStgId = 1;
                            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
                            break;
                        case 5:
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p_30Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p_30Hz;
                            pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                            pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                            break;
                        case 6:
                            pBoxVdcDispCap[i].bAvailable = false;
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                            break;
                    }
                }
            }
            break;
    }
}

static void BBOX_P_Vdc_SetDeinterlacerLimits
    ( uint32_t                            ulBoxId,
      BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap )
{
    uint32_t i;

    switch (ulBoxId)
    {
    case 1:
    case 2:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;
        }
        break;
    }
}

static void BBOX_P_Vdc_SetXcodeLimits
    ( uint32_t                     ulBoxId,
      BBOX_Vdc_Xcode_Capabilities *pXcodeCap )
{
    switch (ulBoxId)
    {
    case 1:
    case 2:
        pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;
    }
}

BERR_Code BBOX_P_Vdc_SetBoxMode
    ( uint32_t               ulBoxId,
      BBOX_Vdc_Capabilities *pBoxVdc )
{
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ASSERT(pBoxVdc);

    if (ulBoxId == 0 || ulBoxId > BBOX_MODES_SUPPORTED)
    {
        BKNI_Memset((void*)pBoxVdc, 0x0, sizeof(BBOX_Vdc_Capabilities));
        BDBG_ERR(("Box mode %d is not supported on this chip.", ulBoxId));
        eStatus = BERR_INVALID_PARAMETER;
    }
    else
    {
        BBOX_P_Vdc_SetDisplayLimits(ulBoxId, pBoxVdc->astDisplay);
        BBOX_P_Vdc_SetSourceLimits(ulBoxId, pBoxVdc->astSource);
        BBOX_P_Vdc_SetDeinterlacerLimits(ulBoxId, pBoxVdc->astDeinterlacer);
        BBOX_P_Vdc_SetXcodeLimits(ulBoxId, &pBoxVdc->stXcode);
    }
    return eStatus;
}

BERR_Code BBOX_P_GetMemConfig
    ( uint32_t                       ulBoxId,
      BBOX_MemConfig                *pBoxMemConfig )
{
    if (ulBoxId != 1000 && (ulBoxId == 0 || ulBoxId > BBOX_MODES_SUPPORTED))
    {
        BDBG_ERR(("Box mode %d is not supported on this chip.", ulBoxId));
        return (BERR_INVALID_PARAMETER);
    }

    switch (ulBoxId)
    {
        case 1:
            *pBoxMemConfig = stBoxMemConfig_7435B0_box1;
            break;
        case 2:
            *pBoxMemConfig = stBoxMemConfig_7435B0_box2;
            break;
    }

    return BERR_SUCCESS;
}

BERR_Code BBOX_P_LoadRts
    ( const BREG_Handle      hReg,
      const uint32_t         ulBoxId )
{
    BERR_Code eStatus = BERR_SUCCESS;

    BSTD_UNUSED(hReg);
    BSTD_UNUSED(ulBoxId);
    return eStatus;
}
/* end of file */
