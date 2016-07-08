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
#include "bbox_priv.h"
#include "bbox_vdc.h"

BDBG_MODULE(BBOX_PRIV);
BDBG_OBJECT_ID(BBOX_BOX_PRIV);

/* Default memc index table for chips without boxmode */
#if (BCHP_CHIP == 7400) || (BCHP_CHIP == 11360)
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 2 */
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
#elif (BCHP_CHIP == 7420)
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       0,       1,       0,       1      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       0,       1,       0,       1      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   2 /* number of MEMC */
};
#elif (BCHP_CHIP == 7422)
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       0      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       1      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       1      ), /* disp 3 */
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
      {
         BBOX_MK_WIN_MEMC_IDX(0,       1,       0,       1,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       1,       0,       1,       0      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       1      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       1      ), /* disp 3 */
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
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       1,       0,       1,       1      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       0      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       0      ), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(0,       1,       0,       1,       1      ), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       1      ), /* disp 5 */
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
#elif (BCHP_CHIP == 7145)
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(1),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       1      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       1,       0,       1,       1      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 0      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 0      ), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 0      ), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   2 /* number of MEMC */
};
#elif (((BCHP_CHIP == 7439) && (BCHP_VER == BCHP_VER_A0))  || \
	   ((BCHP_CHIP == 7366) && (BCHP_VER == BCHP_VER_A0))  || \
	   ((BCHP_CHIP == 74371) && (BCHP_VER == BCHP_VER_A0)) || \
	   ((BCHP_CHIP == 7271)))
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ), /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid), /* disp 6 */
      }
   },
   1 /* number of MEMC */
};
#elif ((BCHP_CHIP==7439) && (BCHP_VER >= BCHP_VER_B0))
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
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
#elif ((BCHP_CHIP==7401)  || (BCHP_CHIP==7403)  || (BCHP_CHIP==7408)  || \
       (BCHP_CHIP==7410)  || (BCHP_CHIP==7125)  || (BCHP_CHIP==7468)  || \
       (BCHP_CHIP==7550)  || (BCHP_CHIP==7552)  || (BCHP_CHIP==7358)  || \
       (BCHP_CHIP==7360)  || (BCHP_CHIP==7340)  || (BCHP_CHIP==7342)  || \
       (BCHP_CHIP==7335)  || (BCHP_CHIP==7336)  || (BCHP_CHIP==7325)  || \
       (BCHP_CHIP==7346)  || (BCHP_CHIP==7344)  || (BCHP_CHIP==7231)  || \
       (BCHP_CHIP==7428)  || (BCHP_CHIP==7429)  || (BCHP_CHIP==7584)  || \
       (BCHP_CHIP==7563)  || (BCHP_CHIP==7543)  || (BCHP_CHIP==7362)  || \
       (BCHP_CHIP==7364)  || (BCHP_CHIP==7228)  || (BCHP_CHIP==7250)  || \
       (BCHP_CHIP==75635) || (BCHP_CHIP==7586)  || (BCHP_CHIP==73625) || \
       (BCHP_CHIP==75845) || (BCHP_CHIP==74295) || (BCHP_CHIP==75525) || \
       (BCHP_CHIP==73465) || (BCHP_CHIP==7268))
static const BBOX_MemConfig stBoxMemConfig =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
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
	}
}

void BBOX_P_Vdc_SetDisplayLimits
	( uint32_t                       ulBoxId,
	  BBOX_Vdc_Display_Capabilities *pBoxVdcDispCap )
{
	uint32_t i, j;

	BSTD_UNUSED(ulBoxId);

	for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
	{
		pBoxVdcDispCap[i].bAvailable = true;
		pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
		pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
		pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
		pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
		pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
		pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;
        if(i == 0)
            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass1;
        else if(i == 1)
            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;
        else
            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;

		for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
		{
			pBoxVdcDispCap[i].astWindow[j].bAvailable = (j<=2) ? true : false;
			pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
			pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
			pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
			pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;
		}
	}
}

void BBOX_P_Vdc_SetDeinterlacerLimits
	( uint32_t                            ulBoxId,
	  BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap )
{
	uint32_t i;

	BSTD_UNUSED(ulBoxId);

	for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
	{
		pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
		pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
		pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;
	}
}

void BBOX_P_Vdc_SetXcodeLimits
	( uint32_t                     ulBoxId,
	  BBOX_Vdc_Xcode_Capabilities *pXcodeCap )
{
	BSTD_UNUSED(ulBoxId);

	pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
	pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
}


BERR_Code BBOX_P_Vdc_SetBoxMode
	( uint32_t               ulBoxId,
	  BBOX_Vdc_Capabilities *pBoxVdc )
{
	BERR_Code eStatus = BERR_SUCCESS;

	BDBG_ASSERT(pBoxVdc);

	if (ulBoxId == 0)
	{
		BBOX_P_Vdc_SetDisplayLimits(ulBoxId, pBoxVdc->astDisplay);
		BBOX_P_Vdc_SetSourceLimits(ulBoxId, pBoxVdc->astSource);
		BBOX_P_Vdc_SetDeinterlacerLimits(ulBoxId, pBoxVdc->astDeinterlacer);
		BBOX_P_Vdc_SetXcodeLimits(ulBoxId, &pBoxVdc->stXcode);

		BDBG_MSG(("Default BOX settings are used."));
	}
	else
	{
		BDBG_ERR(("Box modes are not supported in this chip."));
		eStatus = BERR_INVALID_PARAMETER;
	}

	return eStatus;
}


BERR_Code BBOX_P_GetMemConfig
	( uint32_t                       ulBoxId,
	  BBOX_MemConfig                *pBoxMemConfig )
{
	uint32_t   i, j;
	BERR_Code  eStatus = BERR_SUCCESS;

	BDBG_ASSERT(pBoxMemConfig);
	if (ulBoxId == 0)
	{
		BBOX_MemConfig              stDefMemConfig = stBoxMemConfig;
		BBOX_Vdc_MemcIndexSettings  stDefVdcMemConfig = stDefMemConfig.stVdcMemcIndex;

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
		BDBG_MSG(("Default BOX settings are used."));
	}
	else
	{
		BDBG_ERR(("Box modes are not supported in this chip."));
		eStatus = BERR_INVALID_PARAMETER;
	}

	return eStatus;
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
