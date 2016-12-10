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
******************************************************************************/

#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bfmt.h"
#include "bbox_priv.h"
#include "bbox_priv_modes.h"
#include "bbox_vdc.h"
#include "bbox_vdc_priv.h"

BDBG_MODULE(BBOX_PRIV);
BDBG_OBJECT_ID(BBOX_BOX_PRIV);

/* Default memc index table for chips without boxmode */
#if (BCHP_CHIP == 7400) || (BCHP_CHIP == 11360)
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   1 /* number of MEMC */
};
#elif (BCHP_CHIP == 7405)
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   1 /* number of MEMC */
};
#elif (BCHP_CHIP == 7422)
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       0      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   2 /* number of MEMC */
};
#elif (BCHP_CHIP == 7425)
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       1,       0,       1,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       1,       0,       1,       0      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   2 /* number of MEMC */
};
#elif (BCHP_CHIP == 7435)
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       1,       0,       1,       1      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 0      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 0      ), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 1      ), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   2 /* number of MEMC */
};
#elif (BCHP_CHIP == 7445)
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(2),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       2), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(2,       2,       2,       2,       2), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, 2), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, 0), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, 0), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 2), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 2), /* disp 6 */
      }
   },
   3 /* number of MEMC */
};
#elif (((BCHP_CHIP == 7439) && (BCHP_VER == BCHP_VER_A0))  || \
       ((BCHP_CHIP == 7366) && (BCHP_VER == BCHP_VER_A0))  || \
       ((BCHP_CHIP == 74371) && (BCHP_VER == BCHP_VER_A0)) || \
       ((BCHP_CHIP == 7271))||((BCHP_CHIP == 7278)))
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, Invalid, Invalid, Invalid), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, Invalid, Invalid, Invalid), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, Invalid, Invalid, Invalid), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   1 /* number of MEMC */
};
#elif ((BCHP_CHIP==7439) && (BCHP_VER >= BCHP_VER_B0))
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       0,       1,       0,       0      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   2 /* number of MEMC */
};
#elif ((BCHP_CHIP == 7366) && (BCHP_VER >= BCHP_VER_B0))
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       1      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 1      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, 0      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   2 /* number of MEMC */
};
#elif ((BCHP_CHIP==7552)  || (BCHP_CHIP==7358)  || (BCHP_CHIP==7360)  || \
       (BCHP_CHIP==7346)  || (BCHP_CHIP==7344)  || (BCHP_CHIP==7231)  || \
       (BCHP_CHIP==7429)  || (BCHP_CHIP==7584)  || \
       (BCHP_CHIP==7563)  || (BCHP_CHIP==7543)  || (BCHP_CHIP==7362)  || \
       (BCHP_CHIP==7364)  || (BCHP_CHIP==7228)  || (BCHP_CHIP==7250)  || \
       (BCHP_CHIP==75635) || (BCHP_CHIP==7586)  || (BCHP_CHIP==73625) || \
       (BCHP_CHIP==75845) || (BCHP_CHIP==74295) || (BCHP_CHIP==73465) || \
       (BCHP_CHIP==7268))
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   1 /* number of MEMC */
};
#else
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC /* number of MEMC */
};
#endif

BERR_Code BBOX_P_ValidateId
    (uint32_t                ulId)
{
    BERR_Code eStatus = BERR_SUCCESS;
    if (ulId != 0)
    {
        BDBG_ERR(("Box Mode ID %d is not supportedon this chip.", ulId));
        eStatus = BBOX_ID_NOT_SUPPORTED;
    }
    return eStatus;
}

void BBOX_P_Vdc_SetSourceCapabilities
    ( uint32_t                      ulBoxId,
      BBOX_Vdc_Source_Capabilities *pSourceCap )
{
    uint32_t i;

    BSTD_UNUSED(ulBoxId);

    for (i=0; i < BAVC_SourceId_eMax; i++)
    {
        BBOX_P_VDC_SET_LEGACY_SRC_LIMIT(pSourceCap, i);
    }
}

void BBOX_P_Vdc_SetDisplayCapabilities
    ( uint32_t                       ulBoxId,
      BBOX_Vdc_Display_Capabilities *pDisplayCap )
{
    uint32_t i, j;

    BSTD_UNUSED(ulBoxId);

    for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
    {
        if(i == 0)
            BBOX_P_VDC_SET_LEGACY_DISPLAY_LIMIT(pDisplayCap, i, Class1);
        else if(i == 1)
            BBOX_P_VDC_SET_LEGACY_DISPLAY_LIMIT(pDisplayCap, i, Class0);
        else
            BBOX_P_VDC_SET_LEGACY_DISPLAY_LIMIT(pDisplayCap, i, Disregard);


        for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
        {
            BBOX_P_VDC_SET_LEGACY_WINDOW_LIMIT(pDisplayCap, i, j);
        }
    }
}

void BBOX_P_Vdc_SetDeinterlacerCapabilities
    ( uint32_t                            ulBoxId,
      BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap )
{
    uint32_t i;

    BSTD_UNUSED(ulBoxId);

    /* Deinterlacer and transcode defaults */
    for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
    {
        BBOX_P_VDC_SET_LEGACY_DEINTERLACER_LIMIT(pDeinterlacerCap, i);
    }
}

void BBOX_P_Vdc_SetXcodeCapabilities
    ( uint32_t                     ulBoxId,
      BBOX_Vdc_Xcode_Capabilities *pXcodeCap )
{
    BSTD_UNUSED(ulBoxId);

    BBOX_P_VDC_SET_LEGACY_XCODE_LIMIT(pXcodeCap);
}

BERR_Code BBOX_P_GetMemConfig
    ( uint32_t                       ulBoxId,
      BBOX_MemConfig                *pBoxMemConfig )
{
    uint32_t   i, j;
    BERR_Code  eStatus = BERR_SUCCESS;
    BBOX_MemConfig              stDefMemConfig = stBoxMemConfig;
    BBOX_Vdc_MemcIndexSettings  stDefVdcMemConfig = stDefMemConfig.stVdcMemcIndex;

    BDBG_ASSERT(pBoxMemConfig);

    eStatus = BBOX_P_ValidateId(ulBoxId);
    if (eStatus != BERR_SUCCESS) return eStatus;

    pBoxMemConfig->stVdcMemcIndex.ulRdcMemcIndex = stDefVdcMemConfig.ulRdcMemcIndex;

    for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
    {
        for (j=0; j < BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY; j++)
        {
            pBoxMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j]
                = stDefVdcMemConfig.astDisplay[i].aulVidWinCapMemcIndex[j];
            pBoxMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinMadMemcIndex[j]
                = stDefVdcMemConfig.astDisplay[i].aulVidWinMadMemcIndex[j];
        }
        for (j=0; j < BBOX_VDC_GFX_WINDOW_COUNT_PER_DISPLAY; j++)
        {
            pBoxMemConfig->stVdcMemcIndex.astDisplay[i].aulGfdWinMemcIndex[j]
                = stDefVdcMemConfig.astDisplay[i].aulGfdWinMemcIndex[j];
        }
    }
    BDBG_MSG(("Default (box 0) settings are used."));
    return eStatus;
}

BERR_Code BBOX_P_GetRtsConfig
    ( const uint32_t         ulBoxId,
      BBOX_Rts              *pBoxRts )
{
    BERR_Code eStatus = BERR_SUCCESS;

    BSTD_UNUSED(ulBoxId);
    BSTD_UNUSED(pBoxRts);

    return eStatus;
}
/* end of file */
