/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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

#include "bstd.h"                 /* standard types */
#include "bkni.h"                 /* memcpy calls */
#include "bvdc.h"
#include "bvdc_memconfig_priv.h"
#if BVDC_P_SUPPORT_VIP
#include "bvdc_displayvip_priv.h"
#endif

BDBG_MODULE(BVDC_MEMCONFIG);

/***************************************************************************
 *
 */
void BVDC_GetDefaultMemConfigSettings
	( BVDC_MemConfigSettings             *pMemConfigSettings )
{
	uint32_t    ulDispIndex, ulWinIndex;
	BVDC_P_MemConfig_SystemInfo   *pSystemConfigInfo;

	if(!pMemConfigSettings)
		return;

	BKNI_Memset((void*)pMemConfigSettings, 0x0, sizeof(BVDC_MemConfigSettings));

	pSystemConfigInfo = (BVDC_P_MemConfig_SystemInfo*)
		(BKNI_Malloc(sizeof(BVDC_P_MemConfig_SystemInfo)));
	if(!pSystemConfigInfo)
	{
		return;
	}
	BKNI_Memset((void*)pSystemConfigInfo, 0x0, sizeof(BVDC_P_MemConfig_SystemInfo));

	/* Collect information */
	BVDC_P_MemConfigInfo_Init(pSystemConfigInfo);

	/* Get default buffer heap settings */
	pMemConfigSettings->stHeapSettings = s_stDefaultSettings.stHeapSettings;
	BVDC_P_MemConfig_GetBufSize(&pMemConfigSettings->stHeapSettings,
		pSystemConfigInfo);

	BVDC_P_MemConfig_GetDefaultRdcSettings(&pMemConfigSettings->stRdc);

	for(ulDispIndex = 0; ulDispIndex < BVDC_MAX_DISPLAYS; ulDispIndex++)
	{
		BVDC_DispMemConfigSettings  *pDisplay;
		const BFMT_VideoInfo  *pDispFmtInfo;

		pDisplay = (BVDC_DispMemConfigSettings *)(&pMemConfigSettings->stDisplay[ulDispIndex]);
		BDBG_ASSERT(pDisplay);

		BVDC_P_MemConfig_GetDefaultDisplaySettings(pSystemConfigInfo,
			ulDispIndex, pDisplay);

		pDispFmtInfo = BFMT_GetVideoFormatInfoPtr(pDisplay->eMaxDisplayFormat);
		BDBG_MSG(("Disp[%d] DEF: Used(%d) %s", ulDispIndex, pDisplay->bUsed,
			pDispFmtInfo->pchFormatStr));
		BSTD_UNUSED(pDispFmtInfo);

		/* Skip if display is not used */
		if(!pDisplay->bUsed)
		{
			continue;
		}

		/* VIP default not support */
		pDisplay->vip.bUsed = 0;
		pDisplay->vip.stCfgSettings.ulMaxWidth  = 720;
		pDisplay->vip.stCfgSettings.ulMaxHeight = 480;
		pDisplay->vip.stCfgSettings.bSupportInterlaced = false;

		/* Get settings for each display */
		for(ulWinIndex = 0; ulWinIndex < BVDC_MAX_VIDEO_WINDOWS; ulWinIndex++)
		{
			BVDC_WinMemConfigSettings  *pWindow;
			const BFMT_VideoInfo  *pSrcFmtInfo;

			pWindow = &(pDisplay->stWindow[ulWinIndex]);
			BDBG_ASSERT(pWindow);

			BVDC_P_MemConfig_GetDefaultWindowSettings(pSystemConfigInfo,
				ulDispIndex, ulWinIndex, pWindow);
			pSrcFmtInfo = BFMT_GetVideoFormatInfoPtr(pWindow->eMaxSourceFormat);
			BDBG_MSG(("    Win[%d] DEF: Used(%d) CapMemc[%d] MadMemc[%d] %s",
				ulWinIndex, pWindow->bUsed, pWindow->ulMemcIndex,
				pWindow->ulMadMemcIndex, pSrcFmtInfo->pchFormatStr));
			BDBG_MSG(("    Win[%d] DEF: NotMfd Slip Pip Lip Mos Mad Smoo 3D Psf Side Box ACrop ICrop 5060 Slave AddBuf",
				ulWinIndex));
			BDBG_MSG(("    Win[%d] DEF: %4d %5d %3d %3d %3d %3d %4d %3d %2d %4d %3d %4d %5d %5d %4d %5d",
				ulWinIndex, pWindow->bNonMfdSource, pWindow->bSyncSlip,
				pWindow->bPip, pWindow->bLipsync, pWindow->bMosaicMode,
				pWindow->eDeinterlacerMode, pWindow->bSmoothScaling,
				pWindow->b3DMode, pWindow->bPsfMode, pWindow->bSideBySide,
				pWindow->bBoxDetect, pWindow->bArbitraryCropping,
				pWindow->bIndependentCropping, pWindow->b5060Convert,
				pWindow->bSlave_24_25_30_Display, pWindow->ulAdditionalBufCnt));

			BSTD_UNUSED(pSrcFmtInfo);
		}
	}

	if(pSystemConfigInfo)
		BKNI_Free((void*)pSystemConfigInfo);

}

/***************************************************************************
 *
 */
BERR_Code BVDC_GetMemoryConfiguration
	( const BVDC_MemConfigSettings       *pMemConfigSettings,
	  BVDC_MemConfig                     *pMemConfig )
{
	uint32_t                      ulDispIndex, ulWinIndex;
	uint32_t                      ulRulSize, ulMemcIndex;
	BERR_Code                     err;
	BVDC_Heap_Settings           *pHeapSettings;
	BVDC_P_BufferHeap_SizeInfo   *pHeapSizeInfo;
	BVDC_P_MemConfig_SystemInfo  *pSystemConfigInfo;

	BDBG_ASSERT(pMemConfigSettings);
	BDBG_ASSERT(pMemConfig);

	BKNI_Memset((void*)pMemConfig, 0x0, sizeof(BVDC_MemConfig));

	pSystemConfigInfo = (BVDC_P_MemConfig_SystemInfo*)
		(BKNI_Malloc(sizeof(BVDC_P_MemConfig_SystemInfo)));
	if(!pSystemConfigInfo)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}
	BKNI_Memset((void*)pSystemConfigInfo, 0x0, sizeof(BVDC_P_MemConfig_SystemInfo));

	/* Collect information */
	BVDC_P_MemConfigInfo_Init(pSystemConfigInfo);

	/* Get buffer heap size */
	BVDC_P_MemConfig_GetBufSize(&pMemConfigSettings->stHeapSettings,
		pSystemConfigInfo);

	/* Validate input settings */
	err = BVDC_P_MemConfig_Validate(pMemConfigSettings, pSystemConfigInfo);
	if(err != BERR_SUCCESS)
		return BERR_TRACE(err);

	for(ulDispIndex = 0; ulDispIndex < BVDC_MAX_DISPLAYS; ulDispIndex++)
	{
		BVDC_DispMemConfigSettings  *pDisplay;

		pDisplay = (BVDC_DispMemConfigSettings  *)(&pMemConfigSettings->stDisplay[ulDispIndex]);
		BDBG_ASSERT(pDisplay);

		/* Skip if display is not used */
		if(!pDisplay->bUsed)
		{
			continue;
		}
#if BVDC_P_SUPPORT_VIP
		/* VIP memory config */
		if(pDisplay->vip.bUsed)
		{
			BVDC_P_VipMemSettings stVipMemSettings;
			BVDC_P_VipMemConfig stVipMemConfig;

			if(NULL == pDisplay->vip.pstMemoryInfo) {
				BDBG_ERR(("Please specify the pstMemoryInfo for display[%d].vip", ulDispIndex));
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			}
			BDBG_MSG(("to compute the mem config for display[%d].vip: MEMC[%d]", ulDispIndex, pDisplay->vip.stCfgSettings.ulMemcId));
			stVipMemSettings.DcxvEnable = false; /* TODO: bring up Dcxv */
			stVipMemSettings.DramStripeWidth = pDisplay->vip.pstMemoryInfo->memc[pDisplay->vip.stCfgSettings.ulMemcId].ulStripeWidth;
			stVipMemSettings.PageSize = pDisplay->vip.pstMemoryInfo->memc[pDisplay->vip.stCfgSettings.ulMemcId].ulPageSize;
			stVipMemSettings.X = pDisplay->vip.pstMemoryInfo->memc[pDisplay->vip.stCfgSettings.ulMemcId].ulMbMultiplier;
			stVipMemSettings.Y = pDisplay->vip.pstMemoryInfo->memc[pDisplay->vip.stCfgSettings.ulMemcId].ulMbRemainder;
			stVipMemSettings.bInterlaced = pDisplay->vip.stCfgSettings.bSupportInterlaced;
			stVipMemSettings.MaxPictureWidthInPels = pDisplay->vip.stCfgSettings.ulMaxWidth;
			stVipMemSettings.MaxPictureHeightInPels = pDisplay->vip.stCfgSettings.ulMaxHeight;
			pMemConfig->stMemc[pDisplay->vip.stCfgSettings.ulMemcId].ulVipSize += BVDC_P_MemConfig_GetVipBufSizes(&stVipMemSettings, &stVipMemConfig);
		}
#endif
		/* Get settings for each display */
		for(ulWinIndex = 0; ulWinIndex < BVDC_MAX_VIDEO_WINDOWS; ulWinIndex++)
		{
			uint32_t   ulWinCapBufTotalSize, ulWinMadBufTotalSize;
			BVDC_WinMemConfigSettings  *pWindow;
			BVDC_P_MemConfig_WindowInfo  stWindowInfo;

			pWindow = &(pDisplay->stWindow[ulWinIndex]);
			BDBG_ASSERT(pWindow);

			/* Skip if window is not used */
			if(!pWindow->bUsed)
			{
				continue;
			}

			/* Get settings for each window */
			BDBG_MSG(("---Buffer Count Per Window---"));
			BVDC_P_MemConfig_GetWindowInfo(pWindow, pDisplay,
				pSystemConfigInfo, ulDispIndex, ulWinIndex,
				&stWindowInfo);
			BDBG_MSG(("Disp[%d]Win[%d] buffer type (4HD, 4HD_Pip, 2HD, 2HD_Pip, HD, HD_Pip, SD, SD_Pip)", ulDispIndex, ulWinIndex));
			BVDC_P_MemConfig_GetWindowBufCnt(pWindow, pSystemConfigInfo,
				&stWindowInfo, ulDispIndex, ulWinIndex);

			BVDC_P_MemConfig_GetWinBufSize(pSystemConfigInfo,
				&stWindowInfo, &ulWinCapBufTotalSize, &ulWinMadBufTotalSize);

			/* Put it in the correct memc */
			BDBG_ASSERT(pWindow->ulMemcIndex < BVDC_MAX_MEMC);

			pHeapSizeInfo = &pSystemConfigInfo->stHeapSizeInfo;
			/* Capture buffer */
			pHeapSettings = &(pMemConfig->stMemc[pWindow->ulMemcIndex].stHeapSettings);
			pHeapSettings->ulBufferCnt_4HD +=
				stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD]];
			pHeapSettings->ulBufferCnt_2HD +=
				stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD]];
			pHeapSettings->ulBufferCnt_HD  +=
				stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD]];
			pHeapSettings->ulBufferCnt_SD  +=
				stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD]];
			pHeapSettings->ulBufferCnt_4HD_Pip +=
				stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD_Pip]];
			pHeapSettings->ulBufferCnt_2HD_Pip +=
				stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD_Pip]];
			pHeapSettings->ulBufferCnt_HD_Pip  +=
				stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD_Pip]];
			pHeapSettings->ulBufferCnt_SD_Pip  +=
				stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD_Pip]];

			pMemConfig->stMemc[pWindow->ulMemcIndex].ulSize += ulWinCapBufTotalSize;

			/* Mad buffer */
			pHeapSettings = &(pMemConfig->stMemc[pWindow->ulMadMemcIndex].stHeapSettings);
			pHeapSettings->ulBufferCnt_4HD +=
				stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD]];
			pHeapSettings->ulBufferCnt_2HD +=
				stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD]];
			pHeapSettings->ulBufferCnt_HD  +=
				stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD]];
			pHeapSettings->ulBufferCnt_SD  +=
				stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD]];
			pHeapSettings->ulBufferCnt_4HD_Pip +=
				stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD_Pip]];
			pHeapSettings->ulBufferCnt_2HD_Pip +=
				stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD_Pip]];
			pHeapSettings->ulBufferCnt_HD_Pip  +=
				stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD_Pip]];
			pHeapSettings->ulBufferCnt_SD_Pip  +=
				stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD_Pip]];

			pMemConfig->stMemc[pWindow->ulMadMemcIndex].ulSize += ulWinMadBufTotalSize;

		}
	}

	/* Get memory size for RUL */
	BVDC_P_MemConfig_GetRulSize(&ulRulSize);

	for(ulMemcIndex = 0; ulMemcIndex < BVDC_MAX_MEMC; ulMemcIndex++)
	{
		BVDC_P_MemConfig_SetBufFormat(&pMemConfigSettings->stHeapSettings,
			&pMemConfig->stMemc[ulMemcIndex].stHeapSettings);

		if(ulMemcIndex == pMemConfigSettings->stRdc.ulMemcIndex)
		{
			pMemConfig->stMemc[ulMemcIndex].ulRulSize += ulRulSize;
		}
	}

	BDBG_MSG(("---Buffer Count Per Memory Controller---"));
	for(ulMemcIndex = 0; ulMemcIndex < BVDC_MAX_MEMC; ulMemcIndex++)
	{
		BDBG_MSG(("Memc[%d]     Total Buffer: %3d %6d %6d %6d %6d %5d %5d %5d", ulMemcIndex,
			pMemConfig->stMemc[ulMemcIndex].stHeapSettings.ulBufferCnt_4HD,
			pMemConfig->stMemc[ulMemcIndex].stHeapSettings.ulBufferCnt_4HD_Pip,
			pMemConfig->stMemc[ulMemcIndex].stHeapSettings.ulBufferCnt_2HD,
			pMemConfig->stMemc[ulMemcIndex].stHeapSettings.ulBufferCnt_2HD_Pip,
			pMemConfig->stMemc[ulMemcIndex].stHeapSettings.ulBufferCnt_HD,
			pMemConfig->stMemc[ulMemcIndex].stHeapSettings.ulBufferCnt_HD_Pip,
			pMemConfig->stMemc[ulMemcIndex].stHeapSettings.ulBufferCnt_SD,
			pMemConfig->stMemc[ulMemcIndex].stHeapSettings.ulBufferCnt_SD_Pip));
	}

	BDBG_MSG(("---Total Memory (bytes) Per Memory Controller---"));
	for(ulMemcIndex = 0; ulMemcIndex < BVDC_MAX_MEMC; ulMemcIndex++)
	{
		BDBG_MSG(("Memc[%d] settings: PicSize = %9d, RulSize = %9d, Total: %9d",
			ulMemcIndex,
			pMemConfig->stMemc[ulMemcIndex].ulSize,
			pMemConfig->stMemc[ulMemcIndex].ulRulSize,
			pMemConfig->stMemc[ulMemcIndex].ulSize +
			pMemConfig->stMemc[ulMemcIndex].ulRulSize));
	}

	if(pSystemConfigInfo)
	{
		BKNI_Free((void*)pSystemConfigInfo);
	}
	BDBG_MSG(("------------------------------"));

	return BERR_SUCCESS;
}


/* end of file */
