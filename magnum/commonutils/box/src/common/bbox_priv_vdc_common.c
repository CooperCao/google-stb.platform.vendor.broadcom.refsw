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

 ******************************************************************************/
#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_priv.h"
#include "bbox_vdc.h"
#include "bbox_vdc_priv.h"
#include "bbox_priv_modes.h"
#include "bchp_common.h"
#include "bchp_memc_arb_0.h"

#ifdef BCHP_MEMC_ARB_1_REG_START
#include "bchp_memc_arb_1.h"
#endif

#ifdef BCHP_MEMC_ARB_2_REG_START
#include "bchp_memc_arb_2.h"
#endif

BDBG_MODULE(BBOX_PRIV_VDC_COMMON);
BDBG_FILE_MODULE(BBOX_SELF_CHECK);
BDBG_FILE_MODULE(BBOX_MEMC);
BDBG_OBJECT_ID(BBOX_BOX_PRIV_VDC_COMMON);


static void BBOX_P_Vdc_SetDefaultSourceCapabilities
    ( BBOX_Vdc_Source_Capabilities        *pSourceCap )
{
    uint32_t i;

    for (i=0; i < BAVC_SourceId_eMax; i++)
    {
        pSourceCap->bAvailable = false;
        pSourceCap->bMtgCapable = false;
        pSourceCap->bCompressed = false;
        pSourceCap->stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
        pSourceCap->stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
        pSourceCap->eColorSpace = BBOX_VDC_DISREGARD;
        pSourceCap->eBpp = BBOX_VDC_DISREGARD;
        pSourceCap++;
    }
}

static void BBOX_P_Vdc_SetDefaultDisplayCapabilities
    ( BBOX_Vdc_Display_Capabilities       *pDisplayCap )
{
    uint32_t i, j;

    for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
    {
        pDisplayCap->bAvailable = false;
        pDisplayCap->eMaxVideoFmt = BBOX_VDC_DISREGARD;
        pDisplayCap->eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
        pDisplayCap->stStgEnc.bAvailable = false;
        pDisplayCap->stStgEnc.ulStgId = BBOX_FTR_INVALID;
        pDisplayCap->stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
        pDisplayCap->stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;
        pDisplayCap->eMosaicModeClass = BBOX_VDC_DISREGARD;

        for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
        {
            pDisplayCap->astWindow[j].bAvailable = false;
            pDisplayCap->astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
            pDisplayCap->astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
            pDisplayCap->astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
            pDisplayCap->astWindow[j].stResource.eCap = BBOX_VDC_DISREGARD;
            pDisplayCap->astWindow[j].stResource.eVfd = BBOX_VDC_DISREGARD;
            pDisplayCap->astWindow[j].stResource.eScl = BBOX_VDC_DISREGARD;
            pDisplayCap->astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;
        }
        pDisplayCap++;
    }
}

static void BBOX_P_Vdc_SetDefaultDeinterlacerCapabilities
    (  BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap )
{
    uint32_t i;

    /* Deinterlacer and transcode defaults */
    for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
    {
        pDeinterlacerCap->stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
        pDeinterlacerCap->stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
        pDeinterlacerCap->ulHsclThreshold = BBOX_VDC_DISREGARD;
        pDeinterlacerCap++;
    }
}

static void BBOX_P_Vdc_SetDefaultXcodeCapabilities
    ( BBOX_Vdc_Xcode_Capabilities  *pXcodeCap )
{
    pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
    pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
}

void BBOX_P_Vdc_SetDefaultCapabilities
    ( BBOX_Vdc_Capabilities         *pBoxVdc )
{
    BDBG_ASSERT(pBoxVdc);

    /* Source defaults */
    BBOX_P_Vdc_SetDefaultSourceCapabilities(&pBoxVdc->astSource[0]);
    /* Display and window defaults */
    BBOX_P_Vdc_SetDefaultDisplayCapabilities(&pBoxVdc->astDisplay[0]);
    /* Deinterlacer limits */
    BBOX_P_Vdc_SetDefaultDeinterlacerCapabilities(&pBoxVdc->astDeinterlacer[0]);
    /* Transcode limts */
    BBOX_P_Vdc_SetDefaultXcodeCapabilities(&pBoxVdc->stXcode);
}

BERR_Code BBOX_P_Vdc_SetCapabilities
    ( uint32_t                      ulBoxId,
      BBOX_Vdc_Capabilities        *pBoxVdc )
{
    BERR_Code eStatus = BERR_SUCCESS;

    eStatus = BBOX_P_ValidateId(ulBoxId);

    BBOX_P_Vdc_SetSourceCapabilities(ulBoxId, &pBoxVdc->astSource[0]);

    BBOX_P_Vdc_SetDisplayCapabilities(ulBoxId, &pBoxVdc->astDisplay[0]);

    BBOX_P_Vdc_SetDeinterlacerCapabilities(ulBoxId, &pBoxVdc->astDeinterlacer[0]);

    BBOX_P_Vdc_SetXcodeCapabilities(ulBoxId, &pBoxVdc->stXcode);

    return eStatus;
}

BERR_Code BBOX_P_Vdc_SetBoxMode
    ( uint32_t               ulBoxId,
      BBOX_Vdc_Capabilities *pBoxVdc )
{
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ASSERT(pBoxVdc);

    BDBG_CASSERT((BAVC_Colorspace)BBOX_Vdc_Colorspace_eFuture == BAVC_Colorspace_eFuture);

    /* Set defaults. */
    BBOX_P_Vdc_SetDefaultCapabilities(pBoxVdc);

    /* Set box specific limits */
    eStatus = BBOX_P_Vdc_SetCapabilities(ulBoxId, pBoxVdc);

    return eStatus;
}
BERR_Code BBOX_P_Vdc_SetSourceLimits
    ( BBOX_Vdc_Source_Capabilities *pSourceCap,
      BAVC_SourceId                 eSourceId,
      uint32_t                      ulMtg,
      uint32_t                      ulWidth,
      uint32_t                      ulHeight,
      BBOX_Vdc_Colorspace           eColorSpace,
      BBOX_Vdc_Bpp                  eBpp,
      bool                          bCompressed )
{
    BERR_Code err = BERR_SUCCESS;

    if (eSourceId > BAVC_SourceId_eMax)
    {
        return BERR_INVALID_PARAMETER;
        BDBG_ERR(("Unknown source"));
    }
    else
    {
        pSourceCap += eSourceId;
    }

    if (BBOX_P_SRC_IS_MPEG(eSourceId))
    {
        pSourceCap->bAvailable = true;
        pSourceCap->bMtgCapable = (ulMtg == BBOX_P_VDC_MTG_ENABLE) ? true : false;
        pSourceCap->bCompressed = false;
        pSourceCap->eBpp = eBpp;
        pSourceCap->stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
        pSourceCap->stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
        pSourceCap->eColorSpace = BBOX_VDC_DISREGARD;
    }
    else if (BBOX_P_SRC_IS_HDDVI(eSourceId))
    {
        pSourceCap->bAvailable = true;
        pSourceCap->stSizeLimits.ulHeight = ulHeight;
        pSourceCap->stSizeLimits.ulWidth = ulWidth;
        pSourceCap->bMtgCapable = false;
        pSourceCap->bCompressed = false;
        pSourceCap->eBpp = BBOX_VDC_DISREGARD;
        pSourceCap->eColorSpace = BBOX_VDC_DISREGARD;
    }
    else if (BBOX_P_SRC_IS_GFX(eSourceId))
    {
        pSourceCap->bAvailable = true;
        pSourceCap->stSizeLimits.ulHeight = ulHeight;
        pSourceCap->stSizeLimits.ulWidth = ulWidth;
        pSourceCap->eColorSpace = eColorSpace;
        pSourceCap->eBpp = eBpp;
        pSourceCap->bCompressed = bCompressed;
        pSourceCap->bMtgCapable = false;
    }
    else if (BBOX_P_SRC_IS_VDEC(eSourceId) ||
             BBOX_P_SRC_IS_656IN(eSourceId) ||
             BBOX_P_SRC_IS_DS0(eSourceId) ||
             BBOX_P_SRC_IS_VFD(eSourceId))
    {
        pSourceCap->bAvailable = true;
        pSourceCap->eBpp = eBpp;
        pSourceCap->stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
        pSourceCap->stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
        pSourceCap->eColorSpace = BBOX_VDC_DISREGARD;
        pSourceCap->bMtgCapable = false;
        pSourceCap->bCompressed = false;
    }
    return err;
}

BERR_Code BBOX_P_Vdc_SetDisplayLimits
    ( BBOX_Vdc_Display_Capabilities *pDisplayCap,
      BBOX_Vdc_DisplayId             eDisplayId,
      BFMT_VideoFmt                  eMaxVideoFmt,
      BFMT_VideoFmt                  eMaxHdmiDisplayFmt,
      uint32_t                       ulStgId,
      uint32_t                       ulEncoderCoreId,
      uint32_t                       ulEncoderChannel,
      BBOX_Vdc_MosaicModeClass       eMosaicClass )
{
    BERR_Code err = BERR_SUCCESS;

    pDisplayCap += eDisplayId;
    pDisplayCap->bAvailable = true;
    pDisplayCap->eMaxVideoFmt = eMaxVideoFmt;
    pDisplayCap->eMaxHdmiDisplayFmt = eMaxHdmiDisplayFmt;
    pDisplayCap->stStgEnc.bAvailable = (ulStgId == BBOX_FTR_INVALID) ? false : true;
    pDisplayCap->stStgEnc.ulStgId = ulStgId;
    pDisplayCap->stStgEnc.ulEncoderCoreId = ulEncoderCoreId;
    pDisplayCap->stStgEnc.ulEncoderChannel = ulEncoderChannel;
    pDisplayCap->eMosaicModeClass = eMosaicClass;

    return err;
}

BERR_Code BBOX_P_Vdc_SetWindowLimits
    ( BBOX_Vdc_Display_Capabilities *pDisplayCap,
      BBOX_Vdc_DisplayId             eDisplayId,
      BBOX_Vdc_WindowId              eWinId,
      uint32_t                       ulMad,
      BBOX_Vdc_Resource_Capture      eCap,
      BBOX_Vdc_Resource_Feeder       eVfd,
      BBOX_Vdc_Resource_Scaler       eScl,
      uint32_t                       ulWinWidthFraction,
      uint32_t                       ulWinHeightFraction,
      BBOX_Vdc_SclCapBias            eSclCapBias )
{
    BERR_Code err = BERR_SUCCESS;

    pDisplayCap += eDisplayId;
    pDisplayCap->astWindow[eWinId].bAvailable = (eWinId <= BBOX_Vdc_Window_eGfx0) ? true : false;
    pDisplayCap->astWindow[eWinId].stResource.ulMad = ulMad;
    pDisplayCap->astWindow[eWinId].stResource.eCap = eCap;
    pDisplayCap->astWindow[eWinId].stResource.eVfd = eVfd;
    pDisplayCap->astWindow[eWinId].stResource.eScl = eScl;
    pDisplayCap->astWindow[eWinId].eSclCapBias = eSclCapBias;
    pDisplayCap->astWindow[eWinId].stSizeLimits.ulHeightFraction = ulWinHeightFraction;
    pDisplayCap->astWindow[eWinId].stSizeLimits.ulWidthFraction = ulWinWidthFraction;
    return err;
}

BERR_Code BBOX_P_Vdc_SetDeinterlacerLimits
    ( BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap,
      BBOX_Vdc_DeinterlacerId             eId,
      uint32_t                            ulWidth,
      uint32_t                            ulHeight,
      uint32_t                            ulHsclThreshold )
{
    BERR_Code err = BERR_SUCCESS;

    pDeinterlacerCap += eId;

    if (eId != BBOX_Vdc_Deinterlacer_eInvalid)
    {
        pDeinterlacerCap->stPictureLimits.ulWidth = ulWidth;
        pDeinterlacerCap->stPictureLimits.ulHeight = ulHeight;
        pDeinterlacerCap->ulHsclThreshold = ulHsclThreshold;
    }
    return err;
}

BERR_Code BBOX_P_Vdc_SetXcodeLimits
    ( BBOX_Vdc_Xcode_Capabilities *pXcodeCap,
      uint32_t                     ulNumXcodeCapVfd,
      uint32_t                     ulNumXcodeGfd )
{
    BERR_Code err = BERR_SUCCESS;

    pXcodeCap->ulNumXcodeCapVfd = ulNumXcodeCapVfd;
    pXcodeCap->ulNumXcodeGfd = ulNumXcodeGfd;
    return err;
}

BERR_Code BBOX_P_Vdc_ResetSourceLimits
    ( BBOX_Vdc_Source_Capabilities *pSourceCap,
      BAVC_SourceId                 eSourceId )
{
    BERR_Code err = BERR_SUCCESS;

    if (eSourceId > BAVC_SourceId_eMax)
    {
        return BERR_INVALID_PARAMETER;
        BDBG_ERR(("Unknown source"));
    }
    else
    {
        pSourceCap += eSourceId;
    }

    pSourceCap->bAvailable = false;
    pSourceCap->bMtgCapable = false;
    pSourceCap->bCompressed = false;
    pSourceCap->stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
    pSourceCap->stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
    pSourceCap->eColorSpace = BBOX_VDC_DISREGARD;

    return err;
}

BERR_Code BBOX_P_Vdc_ResetDisplayLimits
    ( BBOX_Vdc_Display_Capabilities *pDisplayCap,
      BBOX_Vdc_DisplayId             eDisplayId )
{
    BERR_Code err = BERR_SUCCESS;

    pDisplayCap += eDisplayId;
    pDisplayCap->bAvailable = false;
    pDisplayCap->eMaxVideoFmt = BBOX_VDC_DISREGARD;
    pDisplayCap->eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
    pDisplayCap->stStgEnc.bAvailable = false;
    pDisplayCap->stStgEnc.ulStgId = BBOX_FTR_INVALID;
    pDisplayCap->stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
    pDisplayCap->stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;
    pDisplayCap->eMosaicModeClass = BBOX_VDC_DISREGARD;

    return err;
}

BERR_Code BBOX_P_Vdc_ResetWindowLimits
    ( BBOX_Vdc_Display_Capabilities *pDisplayCap,
      BBOX_Vdc_DisplayId             eDisplayId,
      BBOX_Vdc_WindowId              eWinId )
{
    BERR_Code err = BERR_SUCCESS;

    pDisplayCap += eDisplayId;
    pDisplayCap->astWindow[eWinId].bAvailable = false;
    pDisplayCap->astWindow[eWinId].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
    pDisplayCap->astWindow[eWinId].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
    pDisplayCap->astWindow[eWinId].stResource.ulMad = BBOX_FTR_INVALID;
    pDisplayCap->astWindow[eWinId].stResource.eCap = BBOX_VDC_DISREGARD;
    pDisplayCap->astWindow[eWinId].stResource.eVfd = BBOX_VDC_DISREGARD;
    pDisplayCap->astWindow[eWinId].stResource.eScl = BBOX_VDC_DISREGARD;
    pDisplayCap->astWindow[eWinId].eSclCapBias = BBOX_VDC_DISREGARD;
    return err;
}

BERR_Code BBOX_P_Vdc_ResetDeinterlacerLimits
    ( BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap,
      BBOX_Vdc_DeinterlacerId             eId )
{
    BERR_Code err = BERR_SUCCESS;

    pDeinterlacerCap += eId;

    pDeinterlacerCap->stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
    pDeinterlacerCap->stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
    pDeinterlacerCap->ulHsclThreshold = BBOX_VDC_DISREGARD;

    return err;
}

BERR_Code BBOX_P_Vdc_ResetXcodeLimits
    ( BBOX_Vdc_Xcode_Capabilities *pXcodeCap )
{
    BERR_Code err = BERR_SUCCESS;

    pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
    pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;

    return err;
}

BERR_Code BBOX_P_Vdc_SelfCheck
    ( const BBOX_MemConfig          *pMemConfig,
      const BBOX_Vdc_Capabilities   *pVdcCap )
{
    BERR_Code eStatus = BERR_SUCCESS;
    uint32_t i, j;

    /* Check each entry in MEMC table against appropriate entries in BBOX_Vdc_Capabilities */
    for (i=0; i<BBOX_VDC_DISPLAY_COUNT; i++)
    {
        /* Check HDMI CFC memc index entries for CMP 0 and 1 only */
        if (i==0 || i==1)
        {
            if (pMemConfig->stVdcMemcIndex.aulHdmiDisplayCfcMemcIndex[i] != BBOX_MemcIndex_Invalid &&
                pVdcCap->astDisplay[i].bAvailable == false)
            {
                BDBG_ERR(("HDMI CFC allocation in MEMC %d doesn't correspond to display %d entry.",
                    pMemConfig->stVdcMemcIndex.aulHdmiDisplayCfcMemcIndex[i], i));
                eStatus = BERR_INVALID_PARAMETER;
            }
            if (pMemConfig->stVdcMemcIndex.astDisplay[i].ulCmpCfcMemcIndex != BBOX_MemcIndex_Invalid &&
                pVdcCap->astDisplay[i].bAvailable == false)
            {
                BDBG_ERR(("CMP CFC allocation in MEMC %d doesn't correspond to display %d entry.",
                    pMemConfig->stVdcMemcIndex.astDisplay[i].ulCmpCfcMemcIndex, i));
                eStatus = BERR_INVALID_PARAMETER;
            }
        }

        for (j=0; j<BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY; j++)
        {
            BBOX_Vdc_MosaicModeClass eMosaic = pVdcCap->astDisplay[i].eMosaicModeClass;
            BFMT_VideoFmt eMaxFmt = pVdcCap->astDisplay[i].eMaxVideoFmt;

            BDBG_MODULE_MSG(BBOX_SELF_CHECK, ("Display %d: Win%d: %x, Mad%d: %x", i, j,
                pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j], j,
                pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinMadMemcIndex[j]));

            if (((pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j] != BBOX_MemcIndex_Invalid) &&
                (pVdcCap->astDisplay[i].astWindow[j].bAvailable == false)) ||
                ((pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j] == BBOX_MemcIndex_Invalid) &&
                (pVdcCap->astDisplay[i].astWindow[j].bAvailable == true)))
            {
                BDBG_ERR(("Memconfig Table display %d window %d entry [%x] doesn't correspond to VDC BOX config entry [%s].",
                    i, j, pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j],
                    pVdcCap->astDisplay[i].astWindow[j].bAvailable ? "true" : "false"));
                eStatus = BERR_INVALID_PARAMETER;
            }

            if (((pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinMadMemcIndex[j] != BBOX_MemcIndex_Invalid) &&
                (pVdcCap->astDisplay[i].astWindow[j].stResource.ulMad == BBOX_FTR_INVALID)) ||
                ((pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinMadMemcIndex[j] == BBOX_MemcIndex_Invalid) &&
                (pVdcCap->astDisplay[i].astWindow[j].stResource.ulMad != BBOX_FTR_INVALID)))
            {
                BDBG_ERR(("Memconfig Table display %d deinterlacer %d entry [%x] doesn't correspond to VDC BOX config entry [%x].",
                    i, j, pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinMadMemcIndex[j],
                    pVdcCap->astDisplay[i].astWindow[j].stResource.ulMad));
                eStatus = BERR_INVALID_PARAMETER;
            }

            if (((pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j] != BBOX_MemcIndex_Invalid) &&
                (pVdcCap->astDisplay[i].astWindow[j].stResource.eCap == BBOX_Vdc_Resource_Capture_eUnknown)) ||
                ((pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j] == BBOX_MemcIndex_Invalid) &&
                (pVdcCap->astDisplay[i].astWindow[j].stResource.eCap != BBOX_Vdc_Resource_Capture_eUnknown &&
                 pVdcCap->astDisplay[i].astWindow[j].stResource.eCap != BBOX_VDC_DISREGARD)))
            {
                BDBG_ERR(("Memconfig Table display %d capture %d entry [%x] doesn't correspond to VDC BOX config entry [%x].",
                    i, j, pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinMadMemcIndex[j],
                    pVdcCap->astDisplay[i].astWindow[j].stResource.eCap));
                eStatus = BERR_INVALID_PARAMETER;
            }

            if (((pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j] != BBOX_MemcIndex_Invalid) &&
                (pVdcCap->astDisplay[i].astWindow[j].stResource.eVfd == BBOX_Vdc_Resource_Feeder_eUnknown)) ||
                ((pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j] == BBOX_MemcIndex_Invalid) &&
                (pVdcCap->astDisplay[i].astWindow[j].stResource.eVfd != BBOX_Vdc_Resource_Feeder_eUnknown &&
                 pVdcCap->astDisplay[i].astWindow[j].stResource.eVfd != BBOX_VDC_DISREGARD)))
            {
                BDBG_ERR(("Memconfig Table display %d feeder %d entry [%x] doesn't correspond to VDC BOX config entry [%x].",
                    i, j, pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinMadMemcIndex[j],
                    pVdcCap->astDisplay[i].astWindow[j].stResource.eVfd));
                eStatus = BERR_INVALID_PARAMETER;
            }

            /* check if CAP/VFD/SCL used are correctly paired */
            if (pVdcCap->astDisplay[i].astWindow[j].stResource.eCap != BBOX_Vdc_Resource_Capture_eUnknown &&
                pVdcCap->astDisplay[i].astWindow[j].stResource.eCap != BBOX_VDC_DISREGARD &&
                pVdcCap->astDisplay[i].astWindow[j].stResource.eVfd != BBOX_Vdc_Resource_Feeder_eUnknown &&
                pVdcCap->astDisplay[i].astWindow[j].stResource.eVfd != BBOX_VDC_DISREGARD &&
                pVdcCap->astDisplay[i].astWindow[j].stResource.eScl != BBOX_Vdc_Resource_Scaler_eUnknown &&
                pVdcCap->astDisplay[i].astWindow[j].stResource.eScl != BBOX_VDC_DISREGARD)
            {
                if ((uint8_t)pVdcCap->astDisplay[i].astWindow[j].stResource.eCap !=
                    (uint8_t)(pVdcCap->astDisplay[i].astWindow[j].stResource.eVfd - BBOX_Vdc_Resource_Feeder_eVfd0) &&
                    (uint8_t)pVdcCap->astDisplay[i].astWindow[j].stResource.eCap != (uint8_t)pVdcCap->astDisplay[i].astWindow[j].stResource.eScl)
                {
                    BDBG_ERR(("CAP, VFD, SCL resources are incorrectly paired, CAP%d, VFD%d, SCL%d.",
                        (uint8_t)pVdcCap->astDisplay[i].astWindow[j].stResource.eCap,
                        (uint8_t)pVdcCap->astDisplay[i].astWindow[j].stResource.eVfd,
                        (uint8_t)pVdcCap->astDisplay[i].astWindow[j].stResource.eScl));
                    eStatus = BERR_INVALID_PARAMETER;
                }
            }

            if (eMaxFmt < BFMT_VideoFmt_eMaxCount && eMosaic < BBOX_Vdc_MosaicModeClass_eDisregard &&
                (!(eMosaic >= BBOX_Vdc_MosaicModeClass_eClass2 && BFMT_IS_UHD(eMaxFmt)) &&
                (!(eMosaic == BBOX_Vdc_MosaicModeClass_eClass1 &&
                  ((eMaxFmt >= BFMT_VideoFmt_e1080i_50Hz && eMaxFmt <= BFMT_VideoFmt_e1080p_120Hz) ||
                   (eMaxFmt >= BFMT_VideoFmt_e1080i      && eMaxFmt <= BFMT_VideoFmt_e1080p) ||
                   (eMaxFmt >= BFMT_VideoFmt_e720p       && eMaxFmt <= BFMT_VideoFmt_e720p_24Hz_3DOU_AS) ||
                   (eMaxFmt >= BFMT_VideoFmt_e720p_24Hz  && eMaxFmt <= BFMT_VideoFmt_e720p_50Hz) ||
                   (eMaxFmt == BFMT_VideoFmt_e480p) || (eMaxFmt == BFMT_VideoFmt_e576p_50Hz))))) &&
                (!(eMosaic == BBOX_Vdc_MosaicModeClass_eClass0 &&
                   eMaxFmt <= BFMT_VideoFmt_eSECAM_H)))
            {
                BFMT_VideoInfo videoFmtInfo;
                BFMT_GetVideoFormatInfo(eMaxFmt, &videoFmtInfo);

                BDBG_ERR(("Mosaic class %d is not compatible to VDC BOX display %d format %s.",
                           pVdcCap->astDisplay[i].eMosaicModeClass,
                           i, videoFmtInfo.pchFormatStr));
                eStatus = BERR_INVALID_PARAMETER;
            }

        }

        for (j=0; j<BBOX_VDC_GFX_WINDOW_COUNT_PER_DISPLAY; j++)
        {
            BDBG_MODULE_MSG(BBOX_SELF_CHECK, ("Display %d: Gfd%d: %x, VDC Gfd%d: %d", i, j,
                pMemConfig->stVdcMemcIndex.astDisplay[i].aulGfdWinMemcIndex[j],
                j+BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY,
                pVdcCap->astDisplay[i].astWindow[j+BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY].bAvailable));

            if (((pMemConfig->stVdcMemcIndex.astDisplay[i].aulGfdWinMemcIndex[j] != BBOX_MemcIndex_Invalid) &&
                (pVdcCap->astDisplay[i].astWindow[j+BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY].bAvailable == false)) ||
                ((pMemConfig->stVdcMemcIndex.astDisplay[i].aulGfdWinMemcIndex[j] == BBOX_MemcIndex_Invalid) &&
                (pVdcCap->astDisplay[i].astWindow[j+BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY].bAvailable == true)))
            {
                BDBG_ERR(("Memconfig Table display %d gfx window %d entry [%x] doesn't correspond to VDC BOX config entry [%s].",
                    i, j, pMemConfig->stVdcMemcIndex.astDisplay[i].aulGfdWinMemcIndex[j],
                    pVdcCap->astDisplay[i].astWindow[j+BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY].bAvailable ? "true" : "false"));
                eStatus = BERR_INVALID_PARAMETER;
            }

            if ((pVdcCap->astDisplay[i].astWindow[j+BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY].bAvailable == true) &&
                (pVdcCap->astDisplay[i].astWindow[j+BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY].eSclCapBias != BBOX_Vdc_SclCapBias_eDisregard))
            {
                BDBG_ERR(("Scl-Cap bias is enabled on graphics window for display %d.", i));
                eStatus = BERR_INVALID_PARAMETER;
            }

            if (j==0) /* first GFX window only */
            {
                if (pMemConfig->stVdcMemcIndex.astDisplay[i].ulGfdCfcMemcIndex != BBOX_MemcIndex_Invalid &&
                    pVdcCap->astDisplay[i].astWindow[j+BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY].bAvailable == false)
                {
                    BDBG_ERR(("GFD CFC allocation in MEMC %d doesn't correspond to display %d gfx window %d entry.",
                        pMemConfig->stVdcMemcIndex.astDisplay[i].ulGfdCfcMemcIndex, i, j));
                    eStatus = BERR_INVALID_PARAMETER;
                }
            }
        }
    }

    return eStatus;
}

/* end of file */
