/***************************************************************************
 *  Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#include "nexus_base.h"
#include "nexus_display_module.h"

BDBG_MODULE(nexus_video_adj);

#define pVideo (&g_NEXUS_DisplayModule_State)

/*============================private methods====================================*/

/***************************************************************************
Apply stored settings after the VDC window is created.

In Nexus, the window handle exists w/ or w/o a source.
In VDC, the window exists only if there's a connected source.
Therefore, Nexus must store the settings. After the VDC window is created, these settings can be applied.
****************************************************************************/
NEXUS_Error
NEXUS_VideoAdj_P_ApplySetSettings( NEXUS_VideoWindowHandle window )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_VideoAdjContext  *pContext;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    pContext = &(window->adjContext);

    /* Apply DNR settings */
    if (pContext->bDnrSet)
    {
        rc = NEXUS_VideoWindow_SetCustomDnrSettings(window,
            &pContext->stDnrSettings, pContext->customDnrData, 0 );
        if (rc != NEXUS_SUCCESS) return BERR_TRACE(rc);
    }

    /* Apply ANR settings */
    if (pContext->bAnrSet)
    {
        rc = NEXUS_VideoWindow_SetCustomAnrSettings(window,
            &pContext->stAnrSettings, pContext->customAnrData, 0 );
        if (rc != NEXUS_SUCCESS) return BERR_TRACE(rc);
    }

    /* Apply MAD settings */
    if (pContext->bMadSet )
    {
        rc = NEXUS_VideoWindow_SetMadSettings(window, &pContext->stMadSettings );
        if (rc != NEXUS_SUCCESS) return BERR_TRACE(rc);
    }

    /* Apply Scaler settings */
    if (pContext->bSclSet)
    {
        rc = NEXUS_VideoWindow_SetScalerSettings(window,
            &pContext->stSclSettings
            );
        if (rc != NEXUS_SUCCESS) return BERR_TRACE(rc);
    }

    if (pContext->bGameModeDelaySet)
    {
        rc = NEXUS_VideoWindow_SetGameModeDelay(window, &pContext->stGameModeDelaySettings );
        if (rc != NEXUS_SUCCESS) return BERR_TRACE(rc);
    }

    if (pContext->bCoefficientIndexSettingsSet)
    {
        rc = NEXUS_VideoWindow_SetCoefficientIndexSettings(window, &pContext->coefficientIndexSettings);
        if (rc != NEXUS_SUCCESS) return BERR_TRACE(rc);
    }

    return NEXUS_SUCCESS;
}


/*============================public methods====================================*/

/***************************************************************************
*
* DNR
*
****************************************************************************/
void NEXUS_VideoWindow_GetDefaultDnrSettings( NEXUS_VideoWindowDnrSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    /* default to best quality */
    pSettings->bnr.mode = NEXUS_VideoWindowFilterMode_eEnable;
    pSettings->bnr.level = BVDC_QP_ADJUST_STEPS;
    pSettings->dcr.mode = NEXUS_VideoWindowFilterMode_eEnable;
    pSettings->dcr.level =  BVDC_QP_ADJUST_STEPS;
    pSettings->mnr.mode = NEXUS_VideoWindowFilterMode_eEnable;
    pSettings->mnr.level = BVDC_QP_ADJUST_STEPS;
}

void
NEXUS_VideoWindow_GetDnrSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowDnrSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->adjContext.stDnrSettings;
}

NEXUS_Error NEXUS_VideoWindow_SetDnrSettings( NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowDnrSettings *pSettings )
{
    return NEXUS_VideoWindow_SetCustomDnrSettings(window, pSettings, NULL, 0);
}

static int nexus_videoadj_p_dnr_alloc(BAVC_SourceId id)
{
    if (id > BAVC_SourceId_eMpegMax) {
        return -1;
    }
    if (!pVideo->dnrAlloc[id]) {
        unsigned i, count = 0;
        for (i=0;i<BAVC_SourceId_eMpegMax+1;i++) {
            if (pVideo->dnrAlloc[i]) count++;
        }
        if (count >= pVideo->vdcCapabilities.ulNumDnr) {
            return -1;
        }
        pVideo->dnrAlloc[id] = true;
    }
    return 0;
}

void nexus_videoadj_p_dnr_dealloc(BAVC_SourceId id)
{
    if (id > BAVC_SourceId_eMpegMax) {
        return;
    }
    pVideo->dnrAlloc[id] = false;
}

NEXUS_Error NEXUS_VideoWindow_SetCustomDnrSettings( NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowDnrSettings *pSettings,
    const uint8_t *pData, unsigned numEntries )
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_VideoAdjContext *pContext;
    BVDC_Dnr_Settings  DnrSettings;
    BVDC_Window_Handle  windowVDC;
    NEXUS_VideoInput_P_Link *pLink;
    void *freeAfter = NULL;
    bool enable;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;
    pContext = &(window->adjContext);
    BDBG_ASSERT(pSettings);

    pContext->bDnrSet = true;
    pContext->stDnrSettings = *pSettings;

    /* NEXUS_VideoAdj_P_ApplySetSettings might call this function using the already-saved data */
    if (pContext->customDnrData != pData || numEntries != 0) {
        if (pContext->customDnrData) {
            /* we cannot free pContext->customDnrData because VDC still has the memory. free after BVDC_ApplyChanges.
            if freeAfter is set, we must execute the "done:" code to avoid a KNI leak. */
            freeAfter = pContext->customDnrData;
            pContext->customDnrData = NULL;
        }
        if (pData) {
            /* VDC does not copy the data, so we have to. */
            pContext->customDnrData = BKNI_Malloc(numEntries * sizeof(pData[0]));
            if (!pContext->customDnrData) {
                rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                goto done;
            }
            BKNI_Memcpy(pContext->customDnrData, pData, numEntries * sizeof(pData[0]));
        }
    }

    if (NULL == windowVDC || g_NEXUS_DisplayModule_State.pqDisabled) /* Window not connected */
    {
        rc = NEXUS_SUCCESS;
        goto done;
    }

    pLink = NEXUS_VideoInput_P_Get(window->input);
    if (!pLink) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto done;
    }

    if (pLink->id > BAVC_SourceId_eMpegMax) {
        /* no DNR for non-digital sources */
        rc = NEXUS_SUCCESS;
        goto done;
    }

    /* There may be fewer DNR blocks than MFD's, but VDC fails too late for Nexus to absorb the failure. So we do a
    simple allocation scheme to fail gracefully now. */
    enable = pSettings->bnr.mode != NEXUS_VideoWindowFilterMode_eDisable ||
             pSettings->dcr.mode != NEXUS_VideoWindowFilterMode_eDisable ||
             pSettings->mnr.mode != NEXUS_VideoWindowFilterMode_eDisable;
    if (enable) {
        if (nexus_videoadj_p_dnr_alloc(pLink->id)) {
            BDBG_WRN(("Can't enable DNR for MFD%d path. Only %d DNR block(s) available.", pLink->id, pVideo->vdcCapabilities.ulNumDnr));
            rc = NEXUS_SUCCESS; /* but don't fail */
            goto done;
        }
    }
    else {
        nexus_videoadj_p_dnr_dealloc(pLink->id);
    }

    BVDC_Source_GetDnrConfiguration(pLink->sourceVdc, &DnrSettings);
    DnrSettings.eBnrMode = pSettings->bnr.mode;
    DnrSettings.eDcrMode = pSettings->dcr.mode;
    DnrSettings.eMnrMode = pSettings->mnr.mode;
    DnrSettings.iBnrLevel = pSettings->bnr.level;
    DnrSettings.iMnrLevel = pSettings->mnr.level;
    DnrSettings.iDcrLevel = pSettings->dcr.level;
    DnrSettings.pvUserInfo = pContext->customDnrData;
    DnrSettings.ulQp = pSettings->qp;
    rc = BVDC_Source_SetDnrConfiguration(pLink->sourceVdc, &DnrSettings);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto err_setdnr;
    }

    if (freeAfter) {
        /* This NEXUS_Display_P_ApplyChanges must take effect if freeAfter is set. */
        if(g_NEXUS_DisplayModule_State.updateMode != NEXUS_DisplayUpdateMode_eAuto) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);} /* fall through */

        rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
        if (rc) {
            rc = BERR_TRACE(rc);
            goto err_setdnr;
        }
    }
    else {
        rc = NEXUS_Display_P_ApplyChanges();
        if (rc) {
            rc = BERR_TRACE(rc);
            goto err_setdnr;
        }
    }

done:
    if (freeAfter) {
        BKNI_Free(freeAfter);
    }
    return rc;

err_setdnr:
    nexus_videoadj_p_dnr_dealloc(pLink->id);
    if (freeAfter) {
        BKNI_Free(freeAfter);
    }
    return rc;
}   /* NEXUS_VideoWindow_SetDnrSettings() */

/***************************************************************************
Summary:
    Get current MAD configurations.
****************************************************************************/
void
NEXUS_VideoWindow_GetMadSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowMadSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->adjContext.stMadSettings;
}

/***************************************************************************
Summary:
    Set MAD configurations.
****************************************************************************/
NEXUS_Error
NEXUS_VideoWindow_SetMadSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowMadSettings *pSettings
    )
{
    bool deinterlace;
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_VideoAdjContext  *pContext;
    BVDC_Window_Handle  windowVDC;
    BVDC_Deinterlace_Settings madSettings;
    BVDC_Deinterlace_Reverse22Settings Reverse22Setting;
    BVDC_Deinterlace_Reverse32Settings Reverse32Setting;
    BVDC_Deinterlace_ChromaSettings ChromaSettings;
    BVDC_Deinterlace_MotionSettings MotionSettings;
    BVDC_422To444UpSampler upSampler;
    BVDC_444To422DnSampler downSampler;
    BVDC_Deinterlace_LowAngleSettings lowAngles;
    NEXUS_DisplayMemConfig memConfig;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;
    pContext = &(window->adjContext);
    BDBG_ASSERT(pSettings);

    pContext->bMadSet = true;
    pContext->stMadSettings = *pSettings;

    if (NULL == windowVDC || g_NEXUS_DisplayModule_State.pqDisabled)
    {
        return NEXUS_SUCCESS;
    }

    deinterlace = pSettings->deinterlace;
    memConfig = g_NEXUS_DisplayModule_State.moduleSettings.memConfig[window->display->index];

    /* must hook up substructures before getting a default from VDC.
    there is a risk here that a new substructure is added and not defaults, therefore we memset. */
    BKNI_Memset(&madSettings, 0, sizeof(madSettings));
    madSettings.pReverse22Settings = &Reverse22Setting;
    madSettings.pReverse32Settings = &Reverse32Setting;
    madSettings.pChromaSettings = &ChromaSettings;
    madSettings.pMotionSettings = &MotionSettings;
    madSettings.pUpSampler = &upSampler;
    madSettings.pDnSampler = &downSampler;
    madSettings.pLowAngles = &lowAngles;
    BVDC_Window_GetDeinterlaceDefaultConfiguration(&madSettings);

    madSettings.bReverse32Pulldown = pSettings->enable32Pulldown;
    madSettings.bReverse22Pulldown = pSettings->enable22Pulldown;
    BDBG_CASSERT(NEXUS_VideoWindowGameMode_e3Fields_ForceSpatial == (NEXUS_VideoWindowGameMode)BVDC_MadGameMode_e3Fields_ForceSpatial);
    if(memConfig.window[window->windowId].deinterlacer == NEXUS_DeinterlacerMode_eBestQuality)
        madSettings.eGameMode = pSettings->gameMode;
    else if(memConfig.window[window->windowId].deinterlacer == NEXUS_DeinterlacerMode_eLowestLatency)
        madSettings.eGameMode = BVDC_MadGameMode_eMinField_ForceSpatial;
    else if(memConfig.window[window->windowId].deinterlacer == NEXUS_DeinterlacerMode_eNone)
        deinterlace = false;
    madSettings.bShrinkWidth = pSettings->shrinkWidth;
    madSettings.ePqEnhancement = pSettings->pqEnhancement;

    if (pSettings->pixelFormat != NEXUS_PixelFormat_eUnknown) {
        rc = NEXUS_P_PixelFormat_ToMagnum_isrsafe(pSettings->pixelFormat, &madSettings.ePxlFormat);
        if (rc) return BERR_TRACE(rc);
    }

    rc = BVDC_Window_SetDeinterlaceConfiguration(windowVDC, deinterlace, &madSettings);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}   /* NEXUS_VideoWindow_SetMadSettings() */

/***************************************************************************
Summary:
    Get scaler configurations.
****************************************************************************/
void
NEXUS_VideoWindow_GetScalerSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowScalerSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->adjContext.stSclSettings;
}

/***************************************************************************
Summary:
    Set scaler configurations.
****************************************************************************/
NEXUS_Error
NEXUS_VideoWindow_SetScalerSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowScalerSettings *pSettings
    )
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_VideoAdjContext  *pContext;
    BVDC_Window_Handle  windowVDC;
    BVDC_Scaler_Settings scalerSettings;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;
    pContext = &(window->adjContext);
    BDBG_ASSERT(pSettings);

    pContext->bSclSet = true;
    pContext->stSclSettings = *pSettings;
    if (pSettings->nonLinearScaling == false) {
        /* clear the setting to be zero after turning off non-linear. */
        pContext->stSclSettings.nonLinearSourceWidth = 0;
        pContext->stSclSettings.nonLinearScalerOutWidth = 0;
    }

    if (NULL == windowVDC || g_NEXUS_DisplayModule_State.pqDisabled) /* window not connected */
    {
        return NEXUS_SUCCESS;
    }

    /* set scaler mode */
    if (pSettings->nonLinearScaling == true)
    {
        /* TODO - do we need to add in a restriction here to force the
        * nonLinearSourceWidth and nonLinearScalerOutWidth to be larger than one
        * quarter of the source/display width? */
        BDBG_MSG(("Turn on non-linear, nonLinearSourceWidth=%d, nonLinearScalerOutWidth=%d",
            pSettings->nonLinearSourceWidth,
            pSettings->nonLinearScalerOutWidth));

        rc = BVDC_Window_SetNonLinearScl(windowVDC,
            pSettings->nonLinearSourceWidth,
            pSettings->nonLinearScalerOutWidth);
        if (rc) return BERR_TRACE(rc);
    }
    else
    {
        /* set both source/scl output width to turn off non-linear. If non-linear
        * is turned off, the source/scl output width val in SCL settings will be
        * ignored. */
        BDBG_MSG(("Turn off non-linear"));

        rc = BVDC_Window_SetNonLinearScl(windowVDC, 0, 0);
        if (rc) return BERR_TRACE(rc);
    }

    if (g_pCoreHandles->boxConfig->stBox.ulBoxId) {
        if (pSettings->bandwidthEquationParams.bias != NEXUS_ScalerCaptureBias_eAuto) {
            BDBG_WRN(("NEXUS_ScalerCaptureBias not supported for box mode systems. Ignoring %d setting.", pSettings->bandwidthEquationParams.bias));
        }
    }
    else {
        BDBG_CASSERT(BVDC_SclCapBias_eSclAfterCap == (BVDC_SclCapBias)NEXUS_ScalerCaptureBias_eScalerAfterCapture);
        rc = BVDC_Window_SetBandwidthEquationParams(windowVDC, pSettings->bandwidthEquationParams.delta, pSettings->bandwidthEquationParams.bias);
        if (rc) return BERR_TRACE(rc);
    }

    rc = BVDC_Window_GetScalerConfiguration(windowVDC, &scalerSettings);
    if (rc) return BERR_TRACE(rc);
    scalerSettings.bSclVertDejagging = pSettings->verticalDejagging;
    scalerSettings.bSclHorzLumaDeringing = pSettings->horizontalLumaDeringing;
    scalerSettings.bSclVertLumaDeringing = pSettings->verticalLumaDeringing;
    scalerSettings.bSclHorzChromaDeringing = pSettings->horizontalChromaDeringing;
    scalerSettings.bSclVertChromaDeringing = pSettings->verticalChromaDeringing;
    rc = BVDC_Window_SetScalerConfiguration(windowVDC, &scalerSettings);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);
    return NEXUS_SUCCESS;
}

/***************************************************************************
*
* ANR
*
****************************************************************************/
void NEXUS_VideoWindow_GetDefaultAnrSettings( NEXUS_VideoWindowAnrSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->pixelFormat = NEXUS_PixelFormat_eUnknown; /* eUnknown allows VDB to set the optimal format */
    /* default to best quality */
    pSettings->anr.mode = NEXUS_VideoWindowFilterMode_eDisable;
    pSettings->anr.level = BVDC_QP_ADJUST_STEPS;
}

void
NEXUS_VideoWindow_GetAnrSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowAnrSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->adjContext.stAnrSettings;
}

NEXUS_Error NEXUS_VideoWindow_SetAnrSettings( NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowAnrSettings *pSettings )
{
    return NEXUS_VideoWindow_SetCustomAnrSettings(window, pSettings, NULL, 0);
}

NEXUS_Error NEXUS_VideoWindow_SetCustomAnrSettings( NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowAnrSettings *pSettings,
    const uint8_t *pData, unsigned numEntries )
{
    NEXUS_VideoAdjContext  *pContext;
    BVDC_Window_Handle  windowVDC;
    NEXUS_VideoInput_P_Link *pLink;
    BERR_Code rc = NEXUS_SUCCESS;
    BVDC_Anr_Settings anrCfg;
    void *freeAfter = NULL;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;
    pContext = &(window->adjContext);
    BDBG_ASSERT(pSettings);

    pContext->bAnrSet = true;
    pContext->stAnrSettings = *pSettings;

    /* NEXUS_VideoAdj_P_ApplySetSettings might call this function using the already-saved data */
    if (pContext->customAnrData != pData || numEntries != 0) {
        if (pContext->customAnrData) {
            /* we cannot free pContext->customAnrData because VDC still has the memory. free after BVDC_ApplyChanges.
            if freeAfter is set, we must execute the "done:" code to avoid a KNI leak. */
            freeAfter = pContext->customAnrData;
            pContext->customAnrData = NULL;
        }
        if (pData) {
            /* VDC does not copy the data, so we have to. */
            pContext->customAnrData = BKNI_Malloc(numEntries * sizeof(pData[0]));
            if (!pContext->customAnrData) {
                rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                goto done;
            }
            BKNI_Memcpy(pContext->customAnrData, pData, numEntries * sizeof(pData[0]));
        }
    }

    if (NULL == windowVDC || g_NEXUS_DisplayModule_State.pqDisabled) /* window not connected */
    {
        rc = NEXUS_SUCCESS;
        goto done;
    }

    /* for now, only main window gets ANR */
    if (window->index != 0 || window->display->index != 0) {
        rc = NEXUS_SUCCESS;
        goto done;
    }

    pLink = NEXUS_VideoInput_P_Get(window->input);
    if (!pLink) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto done;
    }

    BVDC_Window_GetAnrConfiguration(windowVDC, &anrCfg);
    anrCfg.eMode = pSettings->anr.mode;
    anrCfg.iSnDbAdjust = pSettings->anr.level;
    anrCfg.pvUserInfo = pContext->customAnrData;

    if (pSettings->pixelFormat != NEXUS_PixelFormat_eUnknown) {
        rc = NEXUS_P_PixelFormat_ToMagnum_isrsafe(pSettings->pixelFormat, &anrCfg.ePxlFormat);
        if (rc) {
            rc = BERR_TRACE(rc);
            goto done;
        }
    }

    rc = BVDC_Window_SetAnrConfiguration(windowVDC, &anrCfg);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto done;
    }

    if (freeAfter) {
        /* This NEXUS_Display_P_ApplyChanges must take effect if freeAfter is set. */
        if(g_NEXUS_DisplayModule_State.updateMode != NEXUS_DisplayUpdateMode_eAuto) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);} /* fall through */

        rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
        if (rc) {
            rc = BERR_TRACE(rc);
            goto done;
        }
    }
    else {
        rc = NEXUS_Display_P_ApplyChanges();
        if (rc) {
            rc = BERR_TRACE(rc);
            goto done;
        }
    }

done:
    if (freeAfter) {
        BKNI_Free(freeAfter);
    }
    return rc;
}

void NEXUS_VideoWindow_GetCoefficientIndexSettings( NEXUS_VideoWindowHandle window, NEXUS_VideoWindowCoefficientIndexSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->adjContext.coefficientIndexSettings;
}

NEXUS_Error NEXUS_VideoWindow_SetCoefficientIndexSettings( NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowCoefficientIndexSettings *pSettings )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    window->adjContext.coefficientIndexSettings = *pSettings;
    window->adjContext.bCoefficientIndexSettingsSet = true;

    if (NULL == window->vdcState.window || g_NEXUS_DisplayModule_State.pqDisabled) {
        /* they will get applied when the window is created */
        return BERR_SUCCESS;
    }

    BDBG_CASSERT(sizeof(NEXUS_VideoWindowCoefficientIndexSettings) == sizeof(BVDC_CoefficientIndex));

    rc = BVDC_Window_SetCoefficientIndex(window->vdcState.window, (const BVDC_CoefficientIndex *)pSettings);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    return rc;
}

NEXUS_Error NEXUS_VideoWindow_SetGameModeDelay( NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowGameModeDelay *pSettings )
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_VideoAdjContext  *pContext;
    BVDC_Window_Handle  windowVDC;
    BVDC_Window_GameModeSettings GameModeDelaySettings;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;
    pContext = &(window->adjContext);
    BDBG_ASSERT(pSettings);

    pContext->bGameModeDelaySet = true;
    pContext->stGameModeDelaySettings = *pSettings;

    if (NULL == windowVDC || g_NEXUS_DisplayModule_State.pqDisabled) {
        /* they will get applied when the window is created */
        return BERR_SUCCESS;
    }

    /* get a default to prevent uninitialized memory when new params are added */
    (void)BVDC_Window_GetGameModeDelay(windowVDC, &GameModeDelaySettings);

    GameModeDelaySettings.bEnable = pSettings->enable;
    GameModeDelaySettings.bForceCoarseTrack = pSettings->forceCoarseTrack;
    GameModeDelaySettings.ulBufferDelayTarget = pSettings->bufferDelayTarget;
    GameModeDelaySettings.ulBufferDelayTolerance = pSettings->bufferDelayTolerance;

    rc = BVDC_Window_SetGameModeDelay(windowVDC, &GameModeDelaySettings);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    return rc;
}

void NEXUS_VideoWindow_GetGameModeDelay( NEXUS_VideoWindowHandle window, NEXUS_VideoWindowGameModeDelay *pSettings)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->adjContext.stGameModeDelaySettings;
}

void NEXUS_VideoWindow_GetDefaultMadSettings(NEXUS_VideoWindowMadSettings *pSettings )
{
    BVDC_Deinterlace_Settings madSettings;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BKNI_Memset(&madSettings, 0, sizeof(madSettings));
    BVDC_Window_GetDeinterlaceDefaultConfiguration(&madSettings);
    pSettings->deinterlace = true;
    pSettings->enable32Pulldown = madSettings.bReverse32Pulldown;
    pSettings->enable22Pulldown = madSettings.bReverse22Pulldown;
    pSettings->pixelFormat = NEXUS_PixelFormat_eUnknown; /* eUnknown allows VDB to set the optimal format */
    pSettings->pqEnhancement = NEXUS_MadPqEnhancement_eOff; /* eOff means MAD stays on for 480i->480i and 1080i->1080i which prevents audio glitch because of lipsync adjustment. */
}

void NEXUS_VideoWindow_GetDefaultScalerSettings( NEXUS_VideoWindowScalerSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    /* copied from BVDC_P_BW_DEFAULT_DELTA. only applies to older silicon.
    and, unless you call Set, it won't be used. */
    pSettings->bandwidthEquationParams.delta = 10;
    pSettings->verticalDejagging = true;
    pSettings->horizontalLumaDeringing = true;
    pSettings->verticalLumaDeringing = true;
    pSettings->horizontalChromaDeringing = true;
    pSettings->verticalChromaDeringing = true;
}
