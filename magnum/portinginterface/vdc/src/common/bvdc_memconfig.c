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
    ( const BBOX_Config                  *pBoxConfig,
      BVDC_MemConfigSettings             *pMemConfigSettings )
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

#if BVDC_P_CMP_CFC_VER >= 3
    /* HDMI CFC */
    for(ulDispIndex = 0; ulDispIndex < BBOX_VDC_HDMI_DISPLAY_COUNT; ulDispIndex++)
    {
        pMemConfigSettings->hdmiDisplayCfc[ulDispIndex].bUsed = true;
    }
#endif

    for(ulDispIndex = 0; ulDispIndex < BVDC_MAX_DISPLAYS; ulDispIndex++)
    {
        BVDC_DispMemConfigSettings  *pDisplay;

        pDisplay = (BVDC_DispMemConfigSettings *)(&pMemConfigSettings->stDisplay[ulDispIndex]);
        BDBG_ASSERT(pDisplay);

        BVDC_P_MemConfig_GetDefaultDisplaySettings(pSystemConfigInfo,
            ulDispIndex, pDisplay);

        /* Skip if display is not used */
        if(!pDisplay->bUsed)
        {
            continue;
        }

       /* VIP default not support */
        pDisplay->vip.bUsed = 0;
        pDisplay->vip.stCfgSettings.ulMaxWidth  = 720;
        pDisplay->vip.stCfgSettings.ulMaxHeight = 480;
#if BVDC_P_SUPPORT_VIP
        pDisplay->vip.stCfgSettings.bSupportInterlaced = true;
        pDisplay->vip.stCfgSettings.bSupportDecimatedLuma = true;
        pDisplay->vip.stCfgSettings.bSupportBframes = true;
        pDisplay->vip.stCfgSettings.bSupportItfp = true;
#else
        pDisplay->vip.stCfgSettings.bSupportInterlaced = false;
#endif
        /* Get settings for each display */
        for(ulWinIndex = 0; ulWinIndex < BVDC_MAX_VIDEO_WINDOWS; ulWinIndex++)
        {
            BVDC_WinMemConfigSettings  *pWindow;

            /* Only displays 0 and 1 have 2 video windows */
            if ((ulDispIndex >= (uint32_t)BVDC_DisplayId_eDisplay2) && (ulWinIndex > 0))
                continue;

            pWindow = &(pDisplay->stWindow[ulWinIndex]);
            BDBG_ASSERT(pWindow);

            BVDC_P_MemConfig_GetDefaultWindowSettings(pSystemConfigInfo,
                ulDispIndex, ulWinIndex, pWindow);
        }
    }

    /* Allocate any remaining available deinterlacers to any path that has a FTR_SD resource */
    for(ulDispIndex = 0; ulDispIndex < BVDC_MAX_DISPLAYS; ulDispIndex++)
    {
        BVDC_DispMemConfigSettings  *pDisplay;

        pDisplay = (BVDC_DispMemConfigSettings *)(&pMemConfigSettings->stDisplay[ulDispIndex]);
        BDBG_ASSERT(pDisplay);

        if (pSystemConfigInfo->ulNumMadUsed < pSystemConfigInfo->ulNumMad)
        {
            for(ulWinIndex = 0; ulWinIndex < BVDC_MAX_VIDEO_WINDOWS; ulWinIndex++)
            {
                BVDC_WinMemConfigSettings  *pWindow;

                /* Only displays 0 and 1 have 2 video windows */
            if ((ulDispIndex >= (uint32_t)BVDC_DisplayId_eDisplay2) && (ulWinIndex > 0))
                    continue;

                pWindow = &(pDisplay->stWindow[ulWinIndex]);
                BDBG_ASSERT(pWindow);

                BVDC_P_MemConfig_GetDefaultDeinterlacerSettings(pSystemConfigInfo,
                    ulDispIndex, ulWinIndex, pWindow, true);
            }
        }
    }

    if(pBoxConfig)
    {
        BVDC_P_Memconfig_UpdateSettingByBoxmode(pBoxConfig, NULL, pMemConfigSettings);
    }

    /* Dump out default settings */
    for(ulDispIndex = 0; ulDispIndex < BVDC_MAX_DISPLAYS; ulDispIndex++)
    {
        BVDC_DispMemConfigSettings  *pDisplay;
        const BFMT_VideoInfo  *pDispFmtInfo;

        pDisplay = (BVDC_DispMemConfigSettings *)(&pMemConfigSettings->stDisplay[ulDispIndex]);
        BDBG_ASSERT(pDisplay);

        if(!pDisplay->bUsed)
            continue;

#if BVDC_P_CMP_CFC_VER >= 3
        pDisplay->cfc.bUsed = true;
#endif

        pDispFmtInfo = BFMT_GetVideoFormatInfoPtr(pDisplay->eMaxDisplayFormat);
        BDBG_MSG(("Disp[%d] DEF: Used(%d) %s", ulDispIndex, pDisplay->bUsed,
            pDispFmtInfo->pchFormatStr));

        for(ulWinIndex = 0; ulWinIndex < BVDC_MAX_VIDEO_WINDOWS; ulWinIndex++)
        {
            BVDC_WinMemConfigSettings  *pWindow;
            const BFMT_VideoInfo  *pSrcFmtInfo;

            /* Only displays 0 and 1 have 2 video windows */
            if ((ulDispIndex >= (uint32_t)BVDC_DisplayId_eDisplay2) && (ulWinIndex > 0))
                continue;

            pWindow = &(pDisplay->stWindow[ulWinIndex]);
            BDBG_ASSERT(pWindow);

            pSrcFmtInfo = BFMT_GetVideoFormatInfoPtr(pWindow->eMaxSourceFormat);
            BDBG_MSG(("    Win[%d] DEF: Used(%d) CapMemc[%d] MadMemc[%d] %s",
                ulWinIndex, pWindow->bUsed, pWindow->ulMemcIndex,
                pWindow->ulMadMemcIndex, pSrcFmtInfo->pchFormatStr));
            BDBG_MSG(("    Win[%d] DEF: NotMfd Slip Pip Lip Mos Mad Bias 3D Psf Side Box ACrop ICrop 5060 Slave AddBuf",
                ulWinIndex));
            BDBG_MSG(("    Win[%d] DEF: %4d %5d %3d %3d %3d %3d %4d %3d %2d %4d %3d %4d %5d %5d %4d %5d",
                ulWinIndex, pWindow->bNonMfdSource, pWindow->bSyncSlip,
                pWindow->bPip, pWindow->bLipsync, pWindow->bMosaicMode,
                pWindow->eDeinterlacerMode, pWindow->eSclCapBias,
                pWindow->b3DMode, pWindow->bPsfMode, pWindow->bSideBySide,
                pWindow->bBoxDetect, pWindow->bArbitraryCropping,
                pWindow->bIndependentCropping, pWindow->b5060Convert,
                pWindow->bSlave_24_25_30_Display, pWindow->ulAdditionalBufCnt));
        }
    }

    if(pSystemConfigInfo)
        BKNI_Free((void*)pSystemConfigInfo);

}

#if BVDC_P_SUPPORT_VIP
static BERR_Code BVDC_P_Memconfig_GetVipSize
    ( const BVDC_MemConfigSettings       *pMemConfigSettings,
      BVDC_MemConfig                     *pMemConfig )
{
    uint32_t   ulDispIndex;

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

        /* VIP memory config */
        if(pDisplay->vip.bUsed)
        {
            BVDC_P_VipMemSettings stVipMemSettings;
            BVDC_P_VipMemConfig stVipMemConfig;

            if(NULL == pDisplay->vip.pstMemoryInfo) {
                BDBG_ERR(("Please specify the pstMemoryInfo for display[%d].vip", ulDispIndex));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            /* compute the memory allocation; TODO: there might be runtime switch of DCXV for some workaround;
               debug capture probably requires DCXV off; */
            stVipMemSettings.DcxvEnable = BVDC_P_VIP_DCX_ON; /* compile option */
            stVipMemSettings.DramStripeWidth = pDisplay->vip.pstMemoryInfo->memc[pDisplay->vip.stCfgSettings.ulMemcId].ulStripeWidth;
            stVipMemSettings.PageSize = pDisplay->vip.pstMemoryInfo->memc[pDisplay->vip.stCfgSettings.ulMemcId].ulPageSize;
            stVipMemSettings.X = pDisplay->vip.pstMemoryInfo->memc[pDisplay->vip.stCfgSettings.ulMemcId].ulMbMultiplier;
            stVipMemSettings.Y = pDisplay->vip.pstMemoryInfo->memc[pDisplay->vip.stCfgSettings.ulMemcId].ulMbRemainder;
            stVipMemSettings.bInterlaced = pDisplay->vip.stCfgSettings.bSupportInterlaced;
            stVipMemSettings.bDecimatedLuma= pDisplay->vip.stCfgSettings.bSupportDecimatedLuma;
            stVipMemSettings.bBframes = pDisplay->vip.stCfgSettings.bSupportBframes;
            stVipMemSettings.MaxPictureWidthInPels = pDisplay->vip.stCfgSettings.ulMaxWidth;
            stVipMemSettings.MaxPictureHeightInPels = pDisplay->vip.stCfgSettings.ulMaxHeight;
            pMemConfig->stMemc[pDisplay->vip.stCfgSettings.ulMemcId].ulVipSize += BVDC_P_MemConfig_GetVipBufSizes(&stVipMemSettings, &stVipMemConfig);
            BDBG_MSG(("Disp[%d] VIP size: %d, Memc[%d]", ulDispIndex,
                pMemConfig->stMemc[pDisplay->vip.stCfgSettings.ulMemcId].ulVipSize,
                pDisplay->vip.stCfgSettings.ulMemcId));
        }
    }

    return BERR_SUCCESS;
}
#endif


#if BVDC_P_CMP_CFC_VER >= 3
static void BVDC_P_Memconfig_GetCfcSize
    ( const BVDC_MemConfigSettings       *pMemConfigSettings,
      BVDC_MemConfig                     *pMemConfig )
{
    uint32_t   ulDispIndex;
    uint32_t   ulHdmiCfcLutSize, ulCmpCfcLutSize, ulGfdCfcLutSize;

    BDBG_MSG(("---CFC LUT Size Per Display---"));
    BDBG_MSG(("            VEC_HDMI          CMP            GFD"));
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

        ulHdmiCfcLutSize = ulCmpCfcLutSize = ulGfdCfcLutSize = 0;

        if((ulDispIndex < BBOX_VDC_HDMI_DISPLAY_COUNT) && pMemConfigSettings->hdmiDisplayCfc[ulDispIndex].bUsed)
        {
            ulHdmiCfcLutSize = BVDC_P_ALIGN_UP(BVDC_P_HDMI_CFC_LUT_SIZE, sizeof(uint32_t))*BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT;
            pMemConfig->stMemc[pMemConfigSettings->hdmiDisplayCfc[ulDispIndex].ulMemcIndex].ulCfcLutSize += ulHdmiCfcLutSize;/* double-buffer LUT */
        }

        /* CFC memory config */
        if(pDisplay->cfc.bUsed)
        {
            ulCmpCfcLutSize = BVDC_P_ALIGN_UP(BVDC_P_CMP_CFC_LUT_SIZE, sizeof(uint32_t))*BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT;
            pMemConfig->stMemc[pDisplay->cfc.ulCmpMemcIndex].ulCfcLutSize += ulCmpCfcLutSize;
            ulGfdCfcLutSize = BVDC_P_ALIGN_UP(BVDC_P_GFD_CFC_LUT_SIZE, sizeof(uint32_t))*BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT;
            pMemConfig->stMemc[pDisplay->cfc.ulGfdMemcIndex].ulCfcLutSize += ulGfdCfcLutSize;
        }

        if(ulHdmiCfcLutSize || ulCmpCfcLutSize || ulGfdCfcLutSize)
        {
            BDBG_MSG(("Disp[%d]: %6d(Memc_%d) %6d(Memc_%d) %6d(Memc_%d)", ulDispIndex,
                ulHdmiCfcLutSize, pMemConfigSettings->hdmiDisplayCfc[ulDispIndex].ulMemcIndex,
                ulCmpCfcLutSize, pDisplay->cfc.ulCmpMemcIndex,
                ulGfdCfcLutSize, pDisplay->cfc.ulGfdMemcIndex));
        }
    }

}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_GetMemoryConfiguration
    ( const BBOX_Config                  *pBoxConfig,
      const BVDC_MemConfigSettings       *pMemConfigSettings,
      BVDC_MemConfig                     *pMemConfig )
{
    uint32_t                      ulDispIndex, ulWinIndex;
    uint32_t                      ulRulSize, ulMemcIndex;
    BERR_Code                     err;
    BVDC_Heap_Settings           *pMemcHeapSettings;
    BVDC_P_BufferHeap_SizeInfo   *pHeapSizeInfo;
    BVDC_P_MemConfig_SystemInfo  *pSystemConfigInfo;
    BVDC_MemConfigSettings        *pBoxMemConfigSettings;

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

    pBoxMemConfigSettings = (BVDC_MemConfigSettings*)
        (BKNI_Malloc(sizeof(BVDC_MemConfigSettings)));
    if(!pBoxMemConfigSettings)
    {
        if(pSystemConfigInfo)
        {
            BKNI_Free((void*)pSystemConfigInfo);
        }
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    *pBoxMemConfigSettings = *pMemConfigSettings;

    /* Collect information */
    BVDC_P_MemConfigInfo_Init(pSystemConfigInfo);

    /* Get buffer heap size */
    BVDC_P_MemConfig_GetBufSize(&pBoxMemConfigSettings->stHeapSettings,
        pSystemConfigInfo);

    /* Validate input settings */
    err = BVDC_P_MemConfig_Validate(pBoxMemConfigSettings, pSystemConfigInfo);
    if(err != BERR_SUCCESS)
    {
        if(pSystemConfigInfo)
        {
            BKNI_Free((void*)pSystemConfigInfo);
        }
        if(pBoxMemConfigSettings)
        {
            BKNI_Free((void*)pBoxMemConfigSettings);
        }
        return BERR_TRACE(err);
    }

    if(pBoxConfig)
    {
        BVDC_P_Memconfig_UpdateSettingByBoxmode(pBoxConfig, pMemConfigSettings, pBoxMemConfigSettings);
    }

    /* Dump out user + boxmode settings */
    for(ulDispIndex = 0; ulDispIndex < BVDC_MAX_DISPLAYS; ulDispIndex++)
    {
        BVDC_DispMemConfigSettings  *pDisplay;
        const BFMT_VideoInfo  *pSrcFmtInfo, *pDispFmtInfo;

        pDisplay = (BVDC_DispMemConfigSettings  *)(&pBoxMemConfigSettings->stDisplay[ulDispIndex]);
        BDBG_ASSERT(pDisplay);

        if(!pDisplay->bUsed)
            continue;

        pDispFmtInfo = BFMT_GetVideoFormatInfoPtr(pDisplay->eMaxDisplayFormat);
        BDBG_MSG(("Disp[%d]: Used(%d) %s", ulDispIndex, pDisplay->bUsed,
            pDispFmtInfo->pchFormatStr));

        for(ulWinIndex = 0; ulWinIndex < BVDC_MAX_VIDEO_WINDOWS; ulWinIndex++)
        {
            BVDC_WinMemConfigSettings  *pWindow;

            pWindow = &(pDisplay->stWindow[ulWinIndex]);
            BDBG_ASSERT(pWindow);

            pSrcFmtInfo = BFMT_GetVideoFormatInfoPtr(pWindow->eMaxSourceFormat);
            if(pWindow->bUsed)
            {
                BDBG_MSG(("    Win[%d]: Used(%d) CapMemc[%d] MadMemc[%d] %s",
                    ulWinIndex, pWindow->bUsed, pWindow->ulMemcIndex,
                    pWindow->ulMadMemcIndex, pSrcFmtInfo->pchFormatStr));
                BDBG_MSG(("    Win[%d]: NotMfd Slip Pip Lip Mos Mad Bias 3D Psf Side Box ACrop ICrop 5060 Slave AddBuf",
                    ulWinIndex));
                BDBG_MSG(("    Win[%d]: %4d %5d %3d %3d %3d %3d %4d %3d %2d %4d %3d %4d %5d %5d %4d %5d",
                    ulWinIndex, pWindow->bNonMfdSource, pWindow->bSyncSlip,
                    pWindow->bPip, pWindow->bLipsync, pWindow->bMosaicMode,
                    pWindow->eDeinterlacerMode, pWindow->eSclCapBias,
                    pWindow->b3DMode, pWindow->bPsfMode, pWindow->bSideBySide,
                    pWindow->bBoxDetect, pWindow->bArbitraryCropping,
                    pWindow->bIndependentCropping, pWindow->b5060Convert,
                    pWindow->bSlave_24_25_30_Display, pWindow->ulAdditionalBufCnt));
            }
            else
            {
                BDBG_MSG(("    Win[%d]: Used(%d)", ulWinIndex, pWindow->bUsed));
            }
        }
    }

#if BVDC_P_SUPPORT_VIP
    err = BVDC_P_Memconfig_GetVipSize(pBoxMemConfigSettings, pMemConfig);
    if(err != BERR_SUCCESS)
    {
        if(pSystemConfigInfo)
        {
            BKNI_Free((void*)pSystemConfigInfo);
        }
        if(pBoxMemConfigSettings)
        {
            BKNI_Free((void*)pBoxMemConfigSettings);
        }
        return BERR_TRACE(err);
    }
#endif

#if BVDC_P_CMP_CFC_VER >= 3
    BVDC_P_Memconfig_GetCfcSize(pBoxMemConfigSettings, pMemConfig);
#endif

    for(ulDispIndex = 0; ulDispIndex < BVDC_MAX_DISPLAYS; ulDispIndex++)
    {
        BVDC_DispMemConfigSettings  *pDisplay;

        pDisplay = (BVDC_DispMemConfigSettings  *)(&pBoxMemConfigSettings->stDisplay[ulDispIndex]);
        BDBG_ASSERT(pDisplay);

        /* Skip if display is not used */
        if(!pDisplay->bUsed)
        {
            continue;
        }

        /* Get settings for each display */
        for(ulWinIndex = 0; ulWinIndex < BVDC_MAX_VIDEO_WINDOWS; ulWinIndex++)
        {
            uint32_t   ulWinCapBufTotalSize, ulWinMadBufTotalSize;
            BVDC_WinMemConfigSettings  *pWindow;
            BVDC_P_MemConfig_WindowInfo  stWindowInfo;
            BVDC_Heap_Settings  *pCapHeapSetting, *pMadHeapSetting;
            BBOX_Vdc_WindowClass   eWinClass = BBOX_Vdc_WindowClass_eLegacy;

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

            BVDC_P_MemConfig_SetBufFormat(&pBoxMemConfigSettings->stHeapSettings,
                pCapHeapSetting);
            BVDC_P_MemConfig_SetBufFormat(&pBoxMemConfigSettings->stHeapSettings,
                pMadHeapSetting);

            /* Get settings for each window */
            BDBG_MSG(("---Buffer Count Per Window---"));

            if(pBoxConfig)
            {
                eWinClass = pBoxConfig->stVdc.astDisplay[ulDispIndex].astWindow[ulWinIndex].eClass;
            }

            BVDC_P_MemConfig_GetWindowInfo(pWindow, pDisplay,
                pSystemConfigInfo, ulDispIndex, ulWinIndex, eWinClass,
                &stWindowInfo);
            BDBG_MSG(("Disp[%d]Win[%d] buffer type (4HD, 4HD_Pip, 2HD, 2HD_Pip, HD, HD_Pip, SD, SD_Pip) PicSize",
                ulDispIndex, ulWinIndex));
            BVDC_P_MemConfig_GetWindowBufCnt(pWindow, pSystemConfigInfo,
                &stWindowInfo, ulDispIndex, ulWinIndex);

            BVDC_P_MemConfig_GetWinBufSize(pSystemConfigInfo,
                &stWindowInfo, &ulWinCapBufTotalSize, &ulWinMadBufTotalSize);

            /* Put it in the correct memc */
            if (pWindow->ulMemcIndex < BVDC_MAX_MEMC)
            {
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
            }

            BDBG_MSG(("Disp[%d]Win[%d]     CapBuf: %3d %6d %6d %6d %6d %5d %5d %5d  %9d",
                ulDispIndex, ulWinIndex,
                pCapHeapSetting->ulBufferCnt_4HD, pCapHeapSetting->ulBufferCnt_4HD_Pip,
                pCapHeapSetting->ulBufferCnt_2HD, pCapHeapSetting->ulBufferCnt_2HD_Pip,
                pCapHeapSetting->ulBufferCnt_HD,  pCapHeapSetting->ulBufferCnt_HD_Pip,
                pCapHeapSetting->ulBufferCnt_SD,  pCapHeapSetting->ulBufferCnt_SD_Pip,
                pMemConfig->stDisplay[ulDispIndex].stWindow[ulWinIndex].ulCapSize));

            /* Mad buffer, only if it is configured with a valid MEMC */
            if (pWindow->ulMadMemcIndex < BBOX_MemcIndex_Invalid)
            {
                pHeapSizeInfo = &pSystemConfigInfo->stHeapSizeInfo;

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

            BDBG_MSG(("Disp[%d]Win[%d]    %s: %3d %6d %6d %6d %6d %5d %5d %5d  %9d",
                ulDispIndex, ulWinIndex,
                stWindowInfo.bMadr ? "MadrBuf" : " MadBuf",
                pMadHeapSetting->ulBufferCnt_4HD, pMadHeapSetting->ulBufferCnt_4HD_Pip,
                pMadHeapSetting->ulBufferCnt_2HD, pMadHeapSetting->ulBufferCnt_2HD_Pip,
                pMadHeapSetting->ulBufferCnt_HD,  pMadHeapSetting->ulBufferCnt_HD_Pip,
                pMadHeapSetting->ulBufferCnt_SD,  pMadHeapSetting->ulBufferCnt_SD_Pip,
                pMemConfig->stDisplay[ulDispIndex].stWindow[ulWinIndex].ulMadSize));


        }
    }

    /* Get memory size for RUL */
    BVDC_P_MemConfig_GetRulSize(&ulRulSize);

    for(ulMemcIndex = 0; ulMemcIndex < BVDC_MAX_MEMC; ulMemcIndex++)
    {
        BVDC_P_MemConfig_SetBufFormat(&pBoxMemConfigSettings->stHeapSettings,
            &pMemConfig->stMemc[ulMemcIndex].stHeapSettings);

        if(ulMemcIndex == pBoxMemConfigSettings->stRdc.ulMemcIndex)
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
    BDBG_MSG(("          PicSize       RUL         VIP    CFC_LUT "));
    for(ulMemcIndex = 0; ulMemcIndex < BVDC_MAX_MEMC; ulMemcIndex++)
    {
        BDBG_MSG(("Memc[%d]: %9d %9d %9d %9d",
            ulMemcIndex,
            pMemConfig->stMemc[ulMemcIndex].ulSize,
            pMemConfig->stMemc[ulMemcIndex].ulRulSize,
            pMemConfig->stMemc[ulMemcIndex].ulVipSize,
            pMemConfig->stMemc[ulMemcIndex].ulCfcLutSize));
    }

    if(pSystemConfigInfo)
    {
        BKNI_Free((void*)pSystemConfigInfo);
    }

    if(pBoxMemConfigSettings)
    {
        BKNI_Free((void*)pBoxMemConfigSettings);
    }
    BDBG_MSG(("------------------------------"));

    return BERR_SUCCESS;
}


/* end of file */
