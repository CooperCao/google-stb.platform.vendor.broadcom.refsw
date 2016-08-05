/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "nexus_base.h"
#include "nexus_display_module.h"

BDBG_MODULE(NEXUS_picture_ctrl);

/*=======================data structures===================================*/
#define NEXUS_CCB_NUM_GAMMA_VALUE 3
#define NEXUS_CCB_NUM_COLOR_TEMP  5

/*============================private methods====================================*/

/***************************************************************************
Summary:
    Apply the settings set before calling window connected.
Description:
    VDC Window is created in NEXUS_VideoWindow_P_Setup, "setxxxSettings"
    functions will only take effect after video window connect.
    If you called "setxxxSettings" functions before window connected,
    we will save the settings in the VWIN context and set flags to dirty.
    Then in window connecting, we will call this function to apply the
    dirty settings.
****************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_P_ApplySetSettings( NEXUS_VideoWindowHandle window )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PictureCtrlContext  *pContext;
    BVDC_Window_Handle  windowVDC;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;
    pContext = &(window->picContext);

    /* Apply Color settings */
    if (pContext->bCommonSet)
    {
        rc = NEXUS_PictureCtrl_SetCommonSettings(window,
            &pContext->stCommonSettings
            );
        if (rc != NEXUS_SUCCESS) return BERR_TRACE(rc);
    }

    /* Apply Enhanced Color settings */
    if (pContext->bAdvColorSet)
    {
        rc = NEXUS_PictureCtrl_SetAdvColorSettings(window,
            &pContext->stAdvColorSettings
            );
        if (rc != NEXUS_SUCCESS) return BERR_TRACE(rc);
    }

    /* Apply customized contrast stretch settings */
    if (pContext->bCustomContrastSet)
    {
        rc = NEXUS_PictureCtrl_SetCustomContrastStretch(window,
            &pContext->stCustomContrast,
            pContext->customContrastStretchData,
            0
            );
        if (rc != NEXUS_SUCCESS) return BERR_TRACE(rc);
    }

    /* Apply color management settings */
    if (pContext->bCmsSet)
    {
        rc = NEXUS_PictureCtrl_SetCmsSettings(window,
            &pContext->stCmsSettings
            );
        if (rc != NEXUS_SUCCESS) return BERR_TRACE(rc);
    }

    /* Apply dither settings */
    if (pContext->bDitherSet)
    {
        rc = NEXUS_PictureCtrl_SetDitherSettings(window,
            &pContext->stDitherSettings
            );
        if (rc != NEXUS_SUCCESS) return BERR_TRACE(rc);
    }

    return NEXUS_SUCCESS;
}


/*============================public methods====================================*/

/***************************************************************************
Summary:
    Get contrast stretch parameters.
****************************************************************************/
void
NEXUS_PictureCtrl_GetContrastStretch(
    NEXUS_VideoWindowHandle window,
    NEXUS_PictureCtrlContrastStretch *pContrast)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pContrast = window->picContext.stCustomContrast;
}

/***************************************************************************
Summary:
    Set contrast stretch parameters.
****************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetContrastStretch( NEXUS_VideoWindowHandle window, const NEXUS_PictureCtrlContrastStretch *pContrast)
{
    return NEXUS_PictureCtrl_SetCustomContrastStretch(window, pContrast, NULL, 0);
}

NEXUS_Error NEXUS_PictureCtrl_SetCustomContrastStretch( NEXUS_VideoWindowHandle window, const NEXUS_PictureCtrlContrastStretch *pContrast, const int16_t *pTable, unsigned numTableEntries )
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_PictureCtrlContext *pContext;
    BVDC_Window_Handle  windowVDC;
    BVDC_ContrastStretch *pVdcContrastStretchSettings;
    void *freeAfter = NULL;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;
    pContext = &(window->picContext);
    BDBG_ASSERT(pContrast);

    pContext->bCustomContrastSet = true;
    pContext->stCustomContrast = *pContrast;

    if ( !((window->display->index == 0) && (window->windowId == BVDC_WindowId_eVideo0))) {
        /* TODO: if HW doesn't exist, should we really avoid the error? */
        return 0;
    }

    /* NEXUS_PictureCtrl_P_ApplySetSettings might call this function using the already-saved data */
    if (pContext->customContrastStretchData != pTable || numTableEntries != 0) {
        if (pContext->customContrastStretchData) {
            /* we cannot free pContext->customContrastStretchData because VDC still has the memory. free after BVDC_ApplyChanges.
            if freeAfter is set, we must execute the "done:" code to avoid a KNI leak. */
            freeAfter = pContext->customContrastStretchData;
            pContext->customContrastStretchData = NULL;
        }
        if (pTable) {
            /* VDC does not copy the data, so we have to. */
            pContext->customContrastStretchData = BKNI_Malloc(numTableEntries * sizeof(pTable[0]));
            if (!pContext->customContrastStretchData) {
                rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                goto done;
            }
            BKNI_Memcpy(pContext->customContrastStretchData, pTable, numTableEntries * sizeof(pTable[0]));
        }
    }

    if (NULL == windowVDC || g_NEXUS_DisplayModule_State.pqDisabled) {
        rc = NEXUS_SUCCESS;
        goto done;
    }

    pVdcContrastStretchSettings = BKNI_Malloc(sizeof(BVDC_ContrastStretch));
    if (!pVdcContrastStretchSettings) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto done;
    }

    rc = BVDC_Window_GetContrastStretch(windowVDC, pVdcContrastStretchSettings);
    if (rc) {
        BKNI_Free(pVdcContrastStretchSettings);
        rc = BERR_TRACE(rc);
        goto done;
    }

    pVdcContrastStretchSettings->iGain = pContrast->gain;
    pVdcContrastStretchSettings->ulShift = pContrast->gainShift;

    BDBG_CASSERT(sizeof(pVdcContrastStretchSettings->aulDcTable1) == sizeof(window->picContext.stCustomContrast.dcTable1));
    BKNI_Memcpy(&pVdcContrastStretchSettings->aulDcTable1[0], &window->picContext.stCustomContrast.dcTable1[0], sizeof(pVdcContrastStretchSettings->aulDcTable1));
    pVdcContrastStretchSettings->bInterpolateTables = false;
    BDBG_CASSERT(sizeof(pVdcContrastStretchSettings->alIreTable) == sizeof(window->picContext.stCustomContrast.ireTable));
    BKNI_Memcpy(&pVdcContrastStretchSettings->alIreTable[0], &window->picContext.stCustomContrast.ireTable[0], sizeof(pVdcContrastStretchSettings->alIreTable));

    pVdcContrastStretchSettings->pvCustomParams = pContext->customContrastStretchData;
    pVdcContrastStretchSettings->pfCallback = NULL;

    rc = BVDC_Window_SetContrastStretch(windowVDC, pVdcContrastStretchSettings);
    if (rc) {
        BKNI_Free(pVdcContrastStretchSettings);
        rc = BERR_TRACE(rc);
        goto done;
    }
    BKNI_Free(pVdcContrastStretchSettings);

    /* TODO: use pContrast->enabled instead? */
    if(pContrast->gain==0&&pContrast->dynamicContrastBlackGain==0&&pContrast->dynamicContrastWhiteGain==0)
        rc = BVDC_Window_EnableContrastStretch(windowVDC, false);
    else
        rc = BVDC_Window_EnableContrastStretch(windowVDC, true);
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

/***************************************************************************
Summary:
    Get color configurations.
****************************************************************************/
void
NEXUS_PictureCtrl_GetCommonSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_PictureCtrlCommonSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->picContext.stCommonSettings;
}

/***************************************************************************
Summary:
    Set color configurations.
****************************************************************************/
NEXUS_Error
NEXUS_PictureCtrl_SetCommonSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_PictureCtrlCommonSettings *pSettings
    )
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_PictureCtrlContext  *pContext;
    BVDC_Window_Handle  windowVDC;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;
    pContext = &(window->picContext);
    BDBG_ASSERT(pSettings);

    pContext->bCommonSet = true;
    pContext->stCommonSettings = *pSettings;

    if (NULL == windowVDC || g_NEXUS_DisplayModule_State.pqDisabled)
    {
        return NEXUS_SUCCESS;
    }

    rc = BVDC_Window_SetContrast(windowVDC, pSettings->contrast);
    if (rc) return BERR_TRACE(rc);

    rc = BVDC_Window_SetSaturation(windowVDC, pSettings->saturation);
    if (rc) return BERR_TRACE(rc);

    rc = BVDC_Window_SetHue(windowVDC, pSettings->hue);
    if (rc) return BERR_TRACE(rc);

    rc = BVDC_Window_SetBrightness(windowVDC, pSettings->brightness);
    if (rc) return BERR_TRACE(rc);

    rc = BVDC_Window_SetSharpness(windowVDC, pSettings->sharpnessEnable, pSettings->sharpness);
    if (rc && rc != BVDC_ERR_TNT_WINDOW_NOT_SUPPORTED) return BERR_TRACE(rc);

    rc = BVDC_Window_SetColorTemp(windowVDC, pSettings->colorTempEnabled?pSettings->colorTemp:0);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Get enhanced color configurations.
****************************************************************************/
void
NEXUS_PictureCtrl_GetAdvColorSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_PictureCtrlAdvColorSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->picContext.stAdvColorSettings;
}

#define NEXUS_P_ATTENUATION_LIMIT(a) (((a)<NEXUS_PICTURE_ATTENUATION_BASE) ? (a) : NEXUS_PICTURE_ATTENUATION_BASE)

/***************************************************************************
Summary:
    Set enhanced color configurations.
****************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetAdvColorSettings( NEXUS_VideoWindowHandle window, const NEXUS_PictureCtrlAdvColorSettings *pSettings )
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_PictureCtrlContext  *pContext;
    BVDC_Window_Handle  windowVDC;
    BVDC_BlueStretch stBlueStretchSettings;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;
    pContext = &(window->picContext);
    BDBG_ASSERT(pSettings);

    /* Pending PR49461, Check for 1st window of primary display */
    if ( (window->display->index == 0) && (window->windowId == BVDC_WindowId_eVideo0))
    {
        pContext->bAdvColorSet = true;
        pContext->stAdvColorSettings = *pSettings;

        if (NULL == windowVDC || g_NEXUS_DisplayModule_State.pqDisabled)
        {
            return NEXUS_SUCCESS;
        }

        if (pSettings->attenuationRbgEnabled) {
            int32_t lAttenuationR, lAttenuationG, lAttenuationB;
            lAttenuationR = NEXUS_P_ATTENUATION_LIMIT(pSettings->attenuationR);
            lAttenuationG = NEXUS_P_ATTENUATION_LIMIT(pSettings->attenuationG);
            lAttenuationB = NEXUS_P_ATTENUATION_LIMIT(pSettings->attenuationB);

            rc = BVDC_Window_SetAttenuationRGB(windowVDC,
                lAttenuationR,
                lAttenuationG,
                lAttenuationB,
                pSettings->offsetR,
                pSettings->offsetG,
                pSettings->offsetB);
            if (rc) return BERR_TRACE(rc);
        }

        /* load CAB table */
        rc = BVDC_Window_SetAutoFlesh(windowVDC,
            pSettings->fleshTone);
        if (rc) return BERR_TRACE(rc);

        rc = BVDC_Window_SetGreenBoost(windowVDC,
            pSettings->greenBoost);
        if (rc) return BERR_TRACE(rc);

        rc = BVDC_Window_SetBlueBoost(windowVDC,
            pSettings->blueBoost);
        if (rc) return BERR_TRACE(rc);

        stBlueStretchSettings.ulBlueStretchOffset = pSettings->blueStretchSettings.blueStretchOffset;
        stBlueStretchSettings.ulBlueStretchSlope = pSettings->blueStretchSettings.blueStretchSlope;
        rc = BVDC_Window_SetBlueStretch(windowVDC,
                &stBlueStretchSettings);
        if (rc) return BERR_TRACE(rc);


        rc = NEXUS_Display_P_ApplyChanges();
        if (rc) return BERR_TRACE(rc);

    }

    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Get CMS control Settings.
****************************************************************************/
void
NEXUS_PictureCtrl_GetCmsSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_PictureCtrlCmsSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->picContext.stCmsSettings;
}

/***************************************************************************
Summary:
    Set CMS control Settings.
****************************************************************************/
NEXUS_Error
NEXUS_PictureCtrl_SetCmsSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_PictureCtrlCmsSettings *pSettings
    )
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_PictureCtrlContext  *pContext;
    BVDC_Window_Handle  windowVDC;
    BVDC_ColorBar hueGain, saturationGain;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;
    pContext = &(window->picContext);
    BDBG_ASSERT(pSettings);

    pContext->bCmsSet = true;
    pContext->stCmsSettings = *pSettings;

    if (NULL == windowVDC || g_NEXUS_DisplayModule_State.pqDisabled)
    {
        return NEXUS_SUCCESS;
    }

    hueGain.lRed = pSettings->hueGain.red;
    hueGain.lGreen = pSettings->hueGain.green;
    hueGain.lBlue = pSettings->hueGain.blue;
    hueGain.lCyan = pSettings->hueGain.cyan;
    hueGain.lMagenta = pSettings->hueGain.magenta;
    hueGain.lYellow = pSettings->hueGain.yellow;
    saturationGain.lRed = pSettings->saturationGain.red;
    saturationGain.lGreen = pSettings->saturationGain.green;
    saturationGain.lBlue = pSettings->saturationGain.blue;
    saturationGain.lCyan = pSettings->saturationGain.cyan;
    saturationGain.lMagenta = pSettings->saturationGain.magenta;
    saturationGain.lYellow = pSettings->saturationGain.yellow;

    rc = BVDC_Window_SetCmsControl(windowVDC,
        &saturationGain,
        &hueGain);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        return NEXUS_UNKNOWN;
    }

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    return rc;
}

/***************************************************************************
Summary:
    Get current window dither configurations.
****************************************************************************/
void
NEXUS_PictureCtrl_GetDitherSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_PictureCtrlDitherSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    *pSettings = window->picContext.stDitherSettings;
}

/***************************************************************************
Summary:
    Set Dither configurations.
****************************************************************************/
NEXUS_Error
NEXUS_PictureCtrl_SetDitherSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_PictureCtrlDitherSettings *pSettings
    )
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_PictureCtrlContext  *pContext;
    BVDC_Window_Handle  windowVDC;
    BVDC_DitherSettings ditherSettings;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;
    pContext = &(window->picContext);
    BDBG_ASSERT(pSettings);

    pContext->bDitherSet = true;
    pContext->stDitherSettings = *pSettings;

    if (NULL == windowVDC || g_NEXUS_DisplayModule_State.pqDisabled)
    {
        return NEXUS_SUCCESS;
    }

    ditherSettings.bReduceSmooth = pSettings->reduceSmooth;
    ditherSettings.bSmoothEnable = pSettings->smoothEnable;
    ditherSettings.ulSmoothLimit = pSettings->smoothLimit;
    rc = BVDC_Window_SetDitherConfiguration(windowVDC,
        &ditherSettings);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        return NEXUS_UNKNOWN;
    }

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    This function loads the CAB table.
**************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_LoadCabTable( NEXUS_VideoWindowHandle window, const uint32_t *pTable, unsigned numTableEntries, unsigned offset )
{
#if NEXUS_HAS_PEP
    BVDC_Window_Handle  windowVDC;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;

    if (NULL == windowVDC)
    {
        BDBG_ERR(("VideoWindow must have VideoInput connected to load CAB table"));
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    if (g_NEXUS_DisplayModule_State.pqDisabled) {
        return 0;
    }

    if (pTable && numTableEntries > 0)
    {
        rc = BVDC_Window_LoadCabTable(windowVDC, pTable, offset, numTableEntries);
        if (rc) return BERR_TRACE(rc);
    }
    else
    {
        rc = BVDC_Window_LoadCabTable(windowVDC, NULL, 0, 0);
        if (rc) return BERR_TRACE(rc);
    }

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(window);
    BSTD_UNUSED(pTable);
    BSTD_UNUSED(numTableEntries);
    BSTD_UNUSED(offset);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

/***************************************************************************
Summary:
    This function loads the LAB table.
****************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_LoadLabTable( NEXUS_VideoWindowHandle window, const uint32_t *pTable, unsigned numTableEntries, unsigned offset )
{
#if NEXUS_HAS_PEP
    BVDC_Window_Handle  windowVDC;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    windowVDC = window->vdcState.window;

    if (NULL == windowVDC)
    {
        BDBG_ERR(("VideoWindow must have VideoInput connected to load LAB table"));
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    if (g_NEXUS_DisplayModule_State.pqDisabled) {
        return 0;
    }

    if (pTable && numTableEntries > 0)
    {
        rc = BVDC_Window_LoadLabTable(windowVDC, pTable, offset, numTableEntries);
        if (rc) return BERR_TRACE(rc);
    }
    else
    {
        rc = BVDC_Window_LoadLabTable(windowVDC, NULL, 0, 0);
        if (rc) return BERR_TRACE(rc);
    }

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(window);
    BSTD_UNUSED(pTable);
    BSTD_UNUSED(numTableEntries);
    BSTD_UNUSED(offset);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error
NEXUS_PictureCtrl_ConfigDisplayLumaStatistics(
    NEXUS_DisplayHandle display,
    const NEXUS_ClipRect *pRect)
{
    BSTD_UNUSED(display);
    BSTD_UNUSED(pRect);
    BDBG_MSG(("Function no longer supported"));
    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_PictureCtrl_GetDisplayLumaStatistics(
    NEXUS_DisplayHandle display,
    NEXUS_LumaStatistics *pLumaStat)
{
    BSTD_UNUSED(display);
    BSTD_UNUSED(pLumaStat);
    BDBG_MSG(("Function no longer supported"));
    return NEXUS_SUCCESS;
}


/***************************************************************************
Summary:
    Set luma average calculation region.
**************************************************************************/
NEXUS_Error
NEXUS_PictureCtrl_ConfigWindowLumaStatistics(
    NEXUS_VideoWindowHandle window,
    const NEXUS_ClipRect *pRect)
{
    BSTD_UNUSED(window);
    BSTD_UNUSED(pRect);
    BDBG_MSG(("Function no longer supported"));
    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Get luma average value of main video surface.
**************************************************************************/
NEXUS_Error
NEXUS_PictureCtrl_GetWindowLumaStatistics(
    NEXUS_VideoWindowHandle window,
    NEXUS_LumaStatistics *pLumaStat)
{
    BSTD_UNUSED(window);
    BSTD_UNUSED(pLumaStat);
    BDBG_MSG(("Function no longer supported"));
    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_PictureCtrl_SetColorCorrectionTable( NEXUS_DisplayHandle display, const NEXUS_PictureControlColorCorrectionSettings *pSettings )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(display, NEXUS_Display);

    if (g_NEXUS_DisplayModule_State.pqDisabled) {
        return 0;
    }

    if (pSettings==NULL || !pSettings->enabled)
    {
        rc = BVDC_Display_EnableColorCorrection(display->displayVdc, false);
        if (rc) return BERR_TRACE(rc);
    }
    else
    {
        rc = BVDC_Display_EnableColorCorrection(display->displayVdc, true);
        if (rc) return BERR_TRACE(rc);

        if((pSettings->gammaId < NEXUS_CCB_NUM_GAMMA_VALUE) && (pSettings->colorId < NEXUS_CCB_NUM_COLOR_TEMP))
        {
            rc = BVDC_Display_SetColorCorrectionTable(display->displayVdc, pSettings->gammaId, pSettings->colorId);
            if (rc) return BERR_TRACE(rc);
        }
        else
        {
            BDBG_ERR(("Invalid CCB table IDs"));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_PictureCtrl_SetCustomColorCorrectionTable( NEXUS_DisplayHandle display, const uint32_t *pTable, unsigned numTableEntries )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BDBG_ASSERT(pTable);
    BSTD_UNUSED(numTableEntries);

    if (g_NEXUS_DisplayModule_State.pqDisabled) {
        return 0;
    }

    rc = BVDC_Display_EnableColorCorrection(display->displayVdc, true);
    if (rc) return BERR_TRACE(rc);

    rc = BVDC_Display_LoadColorCorrectionTable(display->displayVdc, pTable);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);
    return NEXUS_SUCCESS;
}

void NEXUS_PictureCtrl_GetColorClipSettings( NEXUS_DisplayHandle display, NEXUS_PictureCtrlColorClipSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    *pSettings = display->colorClipSettings;
}

NEXUS_Error NEXUS_PictureCtrl_SetColorClipSettings( NEXUS_DisplayHandle display, const NEXUS_PictureCtrlColorClipSettings *pSettings )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BDBG_CASSERT(sizeof(BVDC_ColorClipSettings) == sizeof(NEXUS_PictureCtrlColorClipSettings));
    BDBG_CASSERT(BVDC_ColorClipMode_Both == NEXUS_ColorClipMode_eMax-1);

    if (g_NEXUS_DisplayModule_State.pqDisabled) {
        return 0;
    }

    rc = BVDC_Compositor_SetColorClip(display->compositor, (BVDC_ColorClipSettings*)pSettings);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    display->colorClipSettings = *pSettings;

    return NEXUS_SUCCESS;
}
