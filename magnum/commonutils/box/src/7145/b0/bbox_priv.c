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
#include "bchp_memc_arb_0.h"
#include "bchp_memc_arb_1.h"
#include "bchp_common.h"


BDBG_MODULE(BBOX_PRIV);
BDBG_OBJECT_ID(BBOX_BOX_PRIV);

/* Available Box Mode RTS */
extern BBOX_Rts stBoxRts_3385_1u3t_box1;

/* Memc Index for box mode 1. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7145B0_box1 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
          BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ),  /* disp 0 */
          BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 1 */
          BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
          BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ),  /* disp 3 */
          BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ),  /* disp 4 */
          BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ),  /* disp 5 */
          BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
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

	switch (ulBoxId)
	{
		case 1:
			for (i=0; i < BAVC_SourceId_eMax; i++)
			{
				pBoxVdcSrcCap[i].bAvailable = false;
				pBoxVdcSrcCap[i].bMtgCapable = false;
				pBoxVdcSrcCap[i].stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
				pBoxVdcSrcCap[i].stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
				pBoxVdcSrcCap[i].eColorSpace = BBOX_VDC_DISREGARD;
				pBoxVdcSrcCap[i].eBpp = BBOX_VDC_DISREGARD;
				switch (i)
				{
					case BAVC_SourceId_eMpeg0:
					case BAVC_SourceId_eMpeg1:
					case BAVC_SourceId_eMpeg2:
					case BAVC_SourceId_eMpeg3:
						pBoxVdcSrcCap[i].bAvailable = true;
						break;
					case BAVC_SourceId_eGfx0:
						pBoxVdcSrcCap[i].bAvailable = true;
						pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
						pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
						pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
						pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
						break;
					case BAVC_SourceId_eGfx3:
					case BAVC_SourceId_eGfx4:
					case BAVC_SourceId_eGfx5:
						pBoxVdcSrcCap[i].bAvailable = true;
						pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 720;
						pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1280;
						pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
						pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
						break;
				}
			}
			break;
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
			for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
			{
				pBoxVdcDispCap[i].bAvailable = (i<6) ? true : false;
				pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
				pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
				pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
				pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
				pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
				pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

				/* Display-STG-Encoder mapping */
				switch(i)
				{
#if 0
					case 2:
						pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
						pBoxVdcDispCap[i].stStgEnc.ulStgId = 3;
						pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 1;
						pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
						break;
#endif
					case 3:
						pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
						pBoxVdcDispCap[i].stStgEnc.ulStgId = 2;
						pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 1;
						pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
						break;
					case 4:
						pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
						pBoxVdcDispCap[i].stStgEnc.ulStgId = 1;
						pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
						pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
						break;
					case 5:
						pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
						pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
						pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
						pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
						break;
				}


				for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
				{
					pBoxVdcDispCap[i].astWindow[j].bAvailable = false;;
					pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
					pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
					pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
					pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

					switch (i)
					{
						case 0:
							pBoxVdcDispCap[i].bAvailable = true;
							pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p;
							pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e1080p;

							if (j==0)
							{
								pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
							}
							/* GFX window is available */
							if (j==2)
							{
								pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
							}
							break;
						case 3:
						case 4:
						case 5:
							pBoxVdcDispCap[i].bAvailable = true;
							pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p_30Hz;
							pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p_30Hz;

							if (j==0)
							{
								pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
							}
							/* GFX window is available  */
							if (j==2)
							{
								pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
							}

							pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;
							/* MAD-R is available, ie BBOX_FTR_HD_MR1, BBOX_FTR_HD_MR2, BBOX_FTR_HD_MR3 */
							pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
							break;
						default:
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
			/* Deinterlacer mapping for 7145B0:
			   Deinterlacer 0 is MCVP for C0V0 window,
			   Deinterlacer 1 is MADR for C3V0 window,
			   Deinterlacer 2 is MADR for C4V0 window,
			   Deinterlacer 3 is MADR for C5V0 window,
			 */

			for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
			{
				pBoxVdc->astDeinterlacerLimits[i].ulWidth = BBOX_VDC_DISREGARD;
				pBoxVdc->astDeinterlacerLimits[i].ulHeight = BBOX_VDC_DISREGARD;
				pBoxVdc->aulDeinterlacerHsclThreshold[i] = BBOX_VDC_DISREGARD;

				switch (i)
				{
					case 2:
					case 3:
					case 4:
						pBoxVdc->astDeinterlacerLimits[i].ulWidth = 1920;
						pBoxVdc->astDeinterlacerLimits[i].ulHeight = 1080;
						pBoxVdc->aulDeinterlacerHsclThreshold[i] = 1280;
						break;
				}
			}
			pBoxVdc->ulNumXcodeCapVfd = 1;
			pBoxVdc->ulNumXcodeGfd = 2;
			break;
		}
	}

	return eStatus;
}


BERR_Code BBOX_P_GetMemConfig
	( uint32_t                       ulBoxId,
	  BBOX_MemConfig                *pBoxMemConfig )
{
	if (ulBoxId == 0 || ulBoxId > BBOX_MODES_SUPPORTED)
	{
		BDBG_ERR(("Box mode %d is not supported on this chip.", ulBoxId));
		return (BERR_INVALID_PARAMETER);
	}

	switch (ulBoxId)
	{
		case 1:
			*pBoxMemConfig = stBoxMemConfig_7145B0_box1;
			pBoxMemConfig->ulNumMemc = stBoxRts_3385_1u3t_box1.ulNumMemc;
			break;
	}

	return BERR_SUCCESS;
}

BERR_Code BBOX_P_LoadRts
	( const BREG_Handle      hReg,
	  const uint32_t         ulBoxId )
{
	BERR_Code eStatus = BERR_SUCCESS;

	if (ulBoxId == 0 || ulBoxId > BBOX_MODES_SUPPORTED)
	{
		BDBG_ERR(("Box mode %d is not supported on this chip.", ulBoxId));
		eStatus = BERR_INVALID_PARAMETER;
	}
	else
	{
		uint32_t i, j;
		BBOX_Rts *pBoxRts = NULL;

		switch (ulBoxId)
		{
			case 1:
				pBoxRts = &stBoxRts_3385_1u3t_box1;
				break;
		}

		/* verify box ID */
		if (pBoxRts->ulBoxId != ulBoxId)
		{
			BDBG_ERR(("Mismatched box ID between device tree/env var and RTS file."));
			eStatus = BBOX_ID_AND_RTS_MISMATCH;
			goto done;
		}

		for (i=0;i<pBoxRts->ulNumMemc;i++)
		{
		uint32_t ulMemcBaseAddr = 0x0;
			BDBG_ASSERT(pBoxRts->paulMemc[i][0]);

			if (i==0)
			{
#ifdef BCHP_MEMC_ARB_0_REG_START
				ulMemcBaseAddr = BCHP_MEMC_ARB_0_CLIENT_INFO_0;
#else
				BDBG_ERR(("There is no MEMC0. Verify the number of MEMC defined in RTS file."));
				eStatus = BBOX_INCORRECT_MEMC_COUNT;
				goto done;
#endif
			}
			else if (i==1)
			{
#ifdef BCHP_MEMC_ARB_1_REG_START
				ulMemcBaseAddr = BCHP_MEMC_ARB_1_CLIENT_INFO_0;

#else
				BDBG_ERR(("There is no MEMC1. Verify the number of MEMC defined in RTS file."));
				eStatus = BBOX_INCORRECT_MEMC_COUNT;
				goto done;
#endif
			}

			BDBG_ASSERT(ulMemcBaseAddr);

			for (j=0;j<pBoxRts->ulNumMemcEntries;j++)
			{
				BREG_Write32(hReg, ulMemcBaseAddr+(j*4), pBoxRts->paulMemc[i][j]);
				BDBG_MSG(("memc[%d][%d] = 0x%x", i, j, pBoxRts->paulMemc[i][j]));
			 }

		}

		for (i=0;i<pBoxRts->ulNumPfriClients;i++)
		{
			BREG_Write32(hReg, pBoxRts->pastPfriClient[i].ulAddr, pBoxRts->pastPfriClient[i].ulData);
			BDBG_MSG(("PFRI[%d] = 0x%x : 0x%x\n", i, pBoxRts->pastPfriClient[i].ulAddr, pBoxRts->pastPfriClient[i].ulData));
		}
	}

done:
	return eStatus;
}
/* end of file */
