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
#include "bchp_memc_arb_0.h"
#include "bchp_common.h"


BDBG_MODULE(BBOX_PRIV);
BDBG_OBJECT_ID(BBOX_BOX_PRIV);

extern BBOX_Rts stBoxRts_74371_1stb_box1;

/* Memc Index for box mode 1. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_74371A0_box1 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       Invalid, Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
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
						pBoxVdcSrcCap[i].bAvailable = true;
						pBoxVdcSrcCap[i].bMtgCapable = true;
						break;
					case BAVC_SourceId_eHdDvi0:
						pBoxVdcSrcCap[i].bAvailable = true;
						pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
						pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
						break;
					case BAVC_SourceId_eGfx0:
						pBoxVdcSrcCap[i].bAvailable = true;
						pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
						pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
						pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
						pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
						break;
					case BAVC_SourceId_eGfx1:
						pBoxVdcSrcCap[i].bAvailable = true;
						pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 576;
						pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 720;
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
				pBoxVdcDispCap[i].bAvailable = false;
				pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
				pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
				pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
				pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
				pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
				pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

				for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
				{
					pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
					pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
					pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
					pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
					pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

					switch (i)
					{
						case 0:
							pBoxVdcDispCap[i].bAvailable = true;
							pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p;
							pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_30Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass1;

							if (j==0) /* main */
							{
								pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
								/* MCVP is available  */
								pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
								/* smooth scaling */
								pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
							}
							if (j==1) /* pip */
							{
								pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
								/* size limit */
								pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
								pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
								/* smooth scaling */
								pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
							}
							if (j==2) /* gfx win */
							{
								pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
							}
							break;
						case 1:
							pBoxVdcDispCap[i].bAvailable = true;
							pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
							pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

							if (j==0) /* main win */
							{
								pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
								/* smooth scaling */
								pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
							}
							if (j==1) /* pip */
							{
								pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
								/* size limit */
								pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
								pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
								/* smooth scaling */
								pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
							}
							if (j==2) /* gfx win */
							{
								pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
							}
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
			for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
			{
				pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
				pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
				pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

				switch (i)
				{
					case 1:
						pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
						pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
						break;
				}
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
	if (ulBoxId == 0 || ulBoxId > BBOX_MODES_SUPPORTED)
	{
		BDBG_ERR(("Box mode %d is not supported on this chip.", ulBoxId));
		return (BERR_INVALID_PARAMETER);
	}

	switch (ulBoxId)
	{
		case 1:
			*pBoxMemConfig = stBoxMemConfig_74371A0_box1;
			pBoxMemConfig->ulNumMemc = stBoxRts_74371_1stb_box1.ulNumMemc;
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
				pBoxRts = &stBoxRts_74371_1stb_box1;
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
