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
#include "bstd.h"              /* standard types */
#include "bvdc.h"              /* Video display */
#include "bvdc_source_priv.h"
#include "bvdc_gfxfeeder_priv.h"
#include "bvdc_feeder_priv.h"

BDBG_MODULE(BVDC);

#if !B_REFSW_MINIMAL
/***************************************************************************
*
*/
BERR_Code BVDC_Source_SetChromaExpansion
    ( BVDC_Source_Handle    hSource,
      BVDC_ChromaExpansion  eChromaExpansion )
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Source_SetChromaExpansion);

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);

    if( eChromaExpansion <= BVDC_ChromaExpansion_eLinearInterpolate )
    {
        hSource->hGfxFeeder->stNewCfgInfo.stDirty.stBits.bChromaExpan = 1;
        hSource->hGfxFeeder->stNewCfgInfo.eChromaExpansion = eChromaExpansion;
    }
    else
    {
        BDBG_ERR(("Bad eChromaExpansion."));
        eStatus = BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_LEAVE(BVDC_Source_SetChromaExpansion);
    return eStatus;
}
#endif


/***************************************************************************
*
*/
BERR_Code BVDC_Source_EnableColorKey
    ( BVDC_Source_Handle  hSource,
      uint32_t            ulMinAC2C1C0,
      uint32_t            ulMaxAC2C1C0,
      uint32_t            ulMaskAC2C1C0,
      uint8_t             ucKeyedAlpha )
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Source_EnableColorKey);

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);

    hSource->hGfxFeeder->stNewCfgInfo.stDirty.stBits.bKey = 1;
    hSource->hGfxFeeder->stNewCfgInfo.stFlags.bEnableKey = 1;
    hSource->hGfxFeeder->stNewCfgInfo.ulKeyMinAMNO = ulMinAC2C1C0;
    hSource->hGfxFeeder->stNewCfgInfo.ulKeyMaxAMNO = ulMaxAC2C1C0;
    hSource->hGfxFeeder->stNewCfgInfo.ulKeyMaskAMNO = ulMaskAC2C1C0;
    hSource->hGfxFeeder->stNewCfgInfo.ucKeyedAlpha = ucKeyedAlpha;

    BDBG_LEAVE(BVDC_Source_EnableColorKey);
    return eStatus;
}


/***************************************************************************
*
*/
BERR_Code BVDC_Source_DisableColorKey
    ( BVDC_Source_Handle  hSource )
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Source_DisableColorKey);

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);

    hSource->hGfxFeeder->stNewCfgInfo.stFlags.bEnableKey = 0;

    BDBG_LEAVE(BVDC_Source_DisableColorKey);
    return eStatus;
}


/***************************************************************************
*
*/
BERR_Code BVDC_Source_SetScaleCoeffs
    ( BVDC_Source_Handle               hSource,
      BVDC_FilterCoeffs                eHorzCoeffs,
      BVDC_FilterCoeffs                eVertCoeffs )
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Source_SetScaleCoeffs);

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);
    if((eHorzCoeffs <= BVDC_FilterCoeffs_eSharp) &&
       (eVertCoeffs <= BVDC_FilterCoeffs_eSharp))
    {
        hSource->hGfxFeeder->stNewCfgInfo.stDirty.stBits.bScaleCoeffs = 1;
        hSource->hGfxFeeder->stNewCfgInfo.eHorzScaleCoeffs = eHorzCoeffs;
        hSource->hGfxFeeder->stNewCfgInfo.eVertScaleCoeffs = eVertCoeffs;
    }
    else
    {
        BDBG_ERR(("Bad eCoeffs."));
        eStatus = BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_LEAVE(BVDC_Source_SetScaleCoeffs);
    return eStatus;
}

#define  GFD_MAX_NUM_GAMMA_T_ENTR     256

#if !B_REFSW_MINIMAL
/***************************************************************************
*
*/
BERR_Code BVDC_Source_SetConstantColor
    ( BVDC_Source_Handle               hSource,
      uint8_t                          ucRed,
      uint8_t                          ucGreen,
      uint8_t                          ucBlue )
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Source_SetConstantColor);

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);

    hSource->hGfxFeeder->stNewCfgInfo.stDirty.stBits.bConstantColor = 1;
    hSource->hGfxFeeder->stNewCfgInfo.ulConstantColor =
        (ucRed << 16) | (ucGreen << 8) | (ucBlue);

    BDBG_LEAVE(BVDC_Source_SetConstantColor);
    return eStatus;
}
#endif

/***************************************************************************
*
*/
BERR_Code BVDC_Source_SetSdrGfxToHdrApproximationAdjust
    ( BVDC_Source_Handle               hSource,
      const BVDC_Source_SdrGfxToHdrApproximationAdjust *pSdrGfxToHdrApproxAdj)
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Source_SetSdrGfxToHdrApproximationAdjust);

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);
    BDBG_ASSERT(NULL!=pSdrGfxToHdrApproxAdj);

    hSource->hGfxFeeder->stNewCfgInfo.stDirty.stBits.bSdrGfx2HdrAdj = 1;
    hSource->hGfxFeeder->stNewCfgInfo.stSdrGfx2HdrAdj = *pSdrGfxToHdrApproxAdj;

    BDBG_LEAVE(BVDC_Source_SetSdrGfxToHdrApproximationAdjust);
    return eStatus;
}

/* End of File */
