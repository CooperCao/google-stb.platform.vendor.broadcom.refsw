/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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

BERR_Code BBOX_P_Vdc_SetBoxMode
	( uint32_t               ulBoxId,
	  BBOX_Vdc_Capabilities *pBoxVdc )
{
	BERR_Code eStatus = BERR_SUCCESS;
	uint32_t i;

	BDBG_ASSERT(pBoxVdc);

	if (ulBoxId == 0)
	{
		BBOX_P_Vdc_SetDisplayLimits(ulBoxId, pBoxVdc->astDisplay);
		BBOX_P_Vdc_SetSourceLimits(ulBoxId, pBoxVdc->astSource);

		for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
		{
			pBoxVdc->astDeinterlacerLimits[i].ulWidth = BBOX_VDC_DISREGARD;
			pBoxVdc->astDeinterlacerLimits[i].ulHeight = BBOX_VDC_DISREGARD;
			pBoxVdc->aulDeinterlacerHsclThreshold[i] = BBOX_VDC_DISREGARD;
		}

		pBoxVdc->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
		pBoxVdc->ulNumXcodeGfd = BBOX_VDC_DISREGARD;

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
