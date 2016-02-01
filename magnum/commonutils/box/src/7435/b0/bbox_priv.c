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
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       1,       0,       1,       1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       Invalid, 0      ),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       Invalid, 0      ),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(0,       1,       0,       Invalid, 1      ),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       Invalid, 1      ),  /* disp 5 */
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
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       Invalid, 0      ),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       Invalid, 0      ),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       Invalid, 0      ),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       Invalid, 0      ),  /* disp 5 */
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

							/* MAD-R is not available for C0V1 */
							if (j==1)
							{
								pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
							}
							break;
						case 1:
							pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
							pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
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

BERR_Code BBOX_P_Vdc_SetBoxMode
	( uint32_t               ulBoxId,
	  BBOX_Vdc_Capabilities *pBoxVdc )
{
	BERR_Code eStatus = BERR_SUCCESS;
	uint32_t i;

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

		switch (ulBoxId)
		{
		case 1:
		case 2:
			for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
			{
				pBoxVdc->astDeinterlacerLimits[i].ulWidth = BBOX_VDC_DISREGARD;
				pBoxVdc->astDeinterlacerLimits[i].ulHeight = BBOX_VDC_DISREGARD;
				pBoxVdc->aulDeinterlacerHsclThreshold[i] = BBOX_VDC_DISREGARD;
			}
			pBoxVdc->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
			pBoxVdc->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
			break;
		}
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
