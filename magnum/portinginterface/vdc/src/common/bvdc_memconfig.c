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
    BVDC_Heap_Settings           *pMemcHeapSettings;
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

#if BVDC_P_CMP_CFC_VER >= 3
    for(ulDispIndex = 0; ulDispIndex < BBOX_VDC_HDMI_DISPLAY_COUNT; ulDispIndex++)
    {
        /* CFC memory config */
        if(pMemConfigSettings->hdmiDisplayCfc[ulDispIndex].bUsed)
        {
            BDBG_MSG(("to compute the mem config for hdmiCfc[%d]: MEMC[%d]",
                ulDispIndex, pMemConfigSettings->hdmiDisplayCfc[ulDispIndex].ulMemcIndex));
            pMemConfig->stMemc[pMemConfigSettings->hdmiDisplayCfc[ulDispIndex].ulMemcIndex].ulCfcLutSize += BVDC_P_HDMI_CFC_LUT_SIZE;
            BDBG_MSG(("    VEC_HDMI CFC LUT size = %d", BVDC_P_HDMI_CFC_LUT_SIZE));
        }
    }
#endif

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
            BDBG_MSG(("    VIP size = %d", pMemConfig->stMemc[pDisplay->vip.stCfgSettings.ulMemcId].ulVipSize));
        }
#endif

#if BVDC_P_CMP_CFC_VER >= 3
        /* CFC memory config */
        if(pDisplay->cfc.bUsed)
        {
            BDBG_MSG(("to compute the mem config for display[%d].cfc: CMP_LUT <- MEMC[%d], GFD LUT <- MEMC[%d]",
                ulDispIndex, pDisplay->cfc.ulCmpMemcIndex, pDisplay->cfc.ulGfdMemcIndex));
            pMemConfig->stMemc[pDisplay->cfc.ulCmpMemcIndex].ulCfcLutSize += BVDC_P_CMP_CFC_LUT_SIZE;
            pMemConfig->stMemc[pDisplay->cfc.ulGfdMemcIndex].ulCfcLutSize += BVDC_P_GFD_CFC_LUT_SIZE;
            BDBG_MSG(("    CMP CFC LUT size = %d", BVDC_P_CMP_CFC_LUT_SIZE));
            BDBG_MSG(("    GFD CFC LUT size = %d", BVDC_P_GFD_CFC_LUT_SIZE));
        }
#endif

        /* Get settings for each display */
        for(ulWinIndex = 0; ulWinIndex < BVDC_MAX_VIDEO_WINDOWS; ulWinIndex++)
        {
            uint32_t   ulWinCapBufTotalSize, ulWinMadBufTotalSize;
            BVDC_WinMemConfigSettings  *pWindow;
            BVDC_P_MemConfig_WindowInfo  stWindowInfo;
            BVDC_Heap_Settings  *pCapHeapSetting, *pMadHeapSetting;

            pWindow = &(pDisplay->stWindow[ulWinIndex]);
            BDBG_ASSERT(pWindow);
            pCapHeapSetting = &(pMemConfig->stDisplay[ulDispIndex].stWindow[ulWinIndex].stCapHeapSettings);
            pMadHeapSetting = &(pMemConfig->stDisplay[ulDispIndex].stWindow[ulWinIndex].stMadHeapSettings);
            BDBG_ASSERT(pCapHeapSetting);
            BDBG_ASSERT(pMadHeapSetting);

            /* Skip if window is not used */
            if(!pWindow->bUsed)
            {
                continue;
            }

            BVDC_P_MemConfig_SetBufFormat(&pMemConfigSettings->stHeapSettings,
                pCapHeapSetting);
            BVDC_P_MemConfig_SetBufFormat(&pMemConfigSettings->stHeapSettings,
                pMadHeapSetting);

            /* Get settings for each window */
            BDBG_MSG(("---Buffer Count Per Window---"));
            BVDC_P_MemConfig_GetWindowInfo(pWindow, pDisplay,
                pSystemConfigInfo, ulDispIndex, ulWinIndex,
                &stWindowInfo);
            BDBG_MSG(("Disp[%d]Win[%d] buffer type (4HD, 4HD_Pip, 2HD, 2HD_Pip, HD, HD_Pip, SD, SD_Pip) PicSize",
                ulDispIndex, ulWinIndex));
            BVDC_P_MemConfig_GetWindowBufCnt(pWindow, pSystemConfigInfo,
                &stWindowInfo, ulDispIndex, ulWinIndex);

            BVDC_P_MemConfig_GetWinBufSize(pSystemConfigInfo,
                &stWindowInfo, &ulWinCapBufTotalSize, &ulWinMadBufTotalSize);

            /* Put it in the correct memc */
            BDBG_ASSERT(pWindow->ulMemcIndex < BVDC_MAX_MEMC);

            pHeapSizeInfo = &pSystemConfigInfo->stHeapSizeInfo;

            /* Copy capture buffer count in each window to BVDC_MemConfig */
            pMemConfig->stDisplay[ulDispIndex].stWindow[ulWinIndex].ulCapSize = ulWinCapBufTotalSize;
            pCapHeapSetting->ulBufferCnt_4HD =
                stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD]];
            pCapHeapSetting->ulBufferCnt_2HD =
                stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD]];
            pCapHeapSetting->ulBufferCnt_HD  =
                stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD]];
            pCapHeapSetting->ulBufferCnt_SD  =
                stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD]];
            pCapHeapSetting->ulBufferCnt_4HD_Pip =
                stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD_Pip]];
            pCapHeapSetting->ulBufferCnt_2HD_Pip =
                stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD_Pip]];
            pCapHeapSetting->ulBufferCnt_HD_Pip  =
                stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD_Pip]];
            pCapHeapSetting->ulBufferCnt_SD_Pip  =
                stWindowInfo.aulCapBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD_Pip]];
            BDBG_MSG(("Disp[%d]Win[%d]     CapBuf: %3d %6d %6d %6d %6d %5d %5d %5d  %9d",
                ulDispIndex, ulWinIndex,
                pCapHeapSetting->ulBufferCnt_4HD, pCapHeapSetting->ulBufferCnt_4HD_Pip,
                pCapHeapSetting->ulBufferCnt_2HD, pCapHeapSetting->ulBufferCnt_2HD_Pip,
                pCapHeapSetting->ulBufferCnt_HD,  pCapHeapSetting->ulBufferCnt_HD_Pip,
                pCapHeapSetting->ulBufferCnt_SD,  pCapHeapSetting->ulBufferCnt_SD_Pip,
                pMemConfig->stDisplay[ulDispIndex].stWindow[ulWinIndex].ulCapSize));

            /* Capture buffer */
            pMemcHeapSettings = &(pMemConfig->stMemc[pWindow->ulMemcIndex].stHeapSettings);
            pMemcHeapSettings->ulBufferCnt_4HD     += pCapHeapSetting->ulBufferCnt_4HD;
            pMemcHeapSettings->ulBufferCnt_2HD     += pCapHeapSetting->ulBufferCnt_2HD;
            pMemcHeapSettings->ulBufferCnt_HD      += pCapHeapSetting->ulBufferCnt_HD;
            pMemcHeapSettings->ulBufferCnt_SD      += pCapHeapSetting->ulBufferCnt_SD;
            pMemcHeapSettings->ulBufferCnt_4HD_Pip += pCapHeapSetting->ulBufferCnt_4HD_Pip;
            pMemcHeapSettings->ulBufferCnt_2HD_Pip += pCapHeapSetting->ulBufferCnt_2HD_Pip;
            pMemcHeapSettings->ulBufferCnt_HD_Pip  += pCapHeapSetting->ulBufferCnt_HD_Pip;
            pMemcHeapSettings->ulBufferCnt_SD_Pip  += pCapHeapSetting->ulBufferCnt_SD_Pip;

            pMemConfig->stMemc[pWindow->ulMemcIndex].ulSize += ulWinCapBufTotalSize;

            /* Mad buffer, only if it is configured with a valid MEMC */
            if (pWindow->ulMadMemcIndex >= BBOX_MemcIndex_Invalid)
            {
               continue;
            }

            /* Copy mad buffer count in each window to BVDC_MemConfig */
            pMemConfig->stDisplay[ulDispIndex].stWindow[ulWinIndex].ulMadSize = ulWinMadBufTotalSize;
            pMadHeapSetting->ulBufferCnt_4HD =
                stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD]];
            pMadHeapSetting->ulBufferCnt_2HD =
                stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD]];
            pMadHeapSetting->ulBufferCnt_HD  =
                stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD]];
            pMadHeapSetting->ulBufferCnt_SD  =
                stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD]];
            pMadHeapSetting->ulBufferCnt_4HD_Pip =
                stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD_Pip]];
            pMadHeapSetting->ulBufferCnt_2HD_Pip =
                stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD_Pip]];
            pMadHeapSetting->ulBufferCnt_HD_Pip  =
                stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD_Pip]];
            pMadHeapSetting->ulBufferCnt_SD_Pip  =
                stWindowInfo.aulMadBufCnt[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD_Pip]];
            BDBG_MSG(("Disp[%d]Win[%d]    %s: %3d %6d %6d %6d %6d %5d %5d %5d  %9d",
                ulDispIndex, ulWinIndex,
                stWindowInfo.bMadr ? "MadrBuf" : " MadBuf",
                pMadHeapSetting->ulBufferCnt_4HD, pMadHeapSetting->ulBufferCnt_4HD_Pip,
                pMadHeapSetting->ulBufferCnt_2HD, pMadHeapSetting->ulBufferCnt_2HD_Pip,
                pMadHeapSetting->ulBufferCnt_HD,  pMadHeapSetting->ulBufferCnt_HD_Pip,
                pMadHeapSetting->ulBufferCnt_SD,  pMadHeapSetting->ulBufferCnt_SD_Pip,
                pMemConfig->stDisplay[ulDispIndex].stWindow[ulWinIndex].ulMadSize));

            pMemcHeapSettings = &(pMemConfig->stMemc[pWindow->ulMadMemcIndex].stHeapSettings);
            pMemcHeapSettings->ulBufferCnt_4HD     += pMadHeapSetting->ulBufferCnt_4HD;
            pMemcHeapSettings->ulBufferCnt_2HD     += pMadHeapSetting->ulBufferCnt_2HD;
            pMemcHeapSettings->ulBufferCnt_HD      += pMadHeapSetting->ulBufferCnt_HD;
            pMemcHeapSettings->ulBufferCnt_SD      += pMadHeapSetting->ulBufferCnt_SD;
            pMemcHeapSettings->ulBufferCnt_4HD_Pip += pMadHeapSetting->ulBufferCnt_4HD_Pip;
            pMemcHeapSettings->ulBufferCnt_2HD_Pip += pMadHeapSetting->ulBufferCnt_2HD_Pip;
            pMemcHeapSettings->ulBufferCnt_HD_Pip  += pMadHeapSetting->ulBufferCnt_HD_Pip;
            pMemcHeapSettings->ulBufferCnt_SD_Pip  += pMadHeapSetting->ulBufferCnt_SD_Pip;

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
        BDBG_MSG(("Memc[%d] settings: PicSize = %9d, RulSize = %9d",
            ulMemcIndex,
            pMemConfig->stMemc[ulMemcIndex].ulSize,
            pMemConfig->stMemc[ulMemcIndex].ulRulSize));
#if BVDC_P_SUPPORT_VIP
        BDBG_MSG(("                   Total VIP size = %d",
            pMemConfig->stMemc[ulMemcIndex].ulVipSize));
#endif
#if BVDC_P_CMP_CFC_VER >= 3
        BDBG_MSG(("                   Total CFC LUT size = %d",
            pMemConfig->stMemc[ulMemcIndex].ulCfcLutSize));
#endif
    }

    if(pSystemConfigInfo)
    {
        BKNI_Free((void*)pSystemConfigInfo);
    }
    BDBG_MSG(("------------------------------"));

    return BERR_SUCCESS;
}


/* end of file */
