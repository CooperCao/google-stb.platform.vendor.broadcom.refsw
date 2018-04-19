/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nexus_display_module.h"

BDBG_MODULE(nexus_display_dynrng);

#ifndef NEXUS_DISPLAY_EXTENSION_DYNRNG
#error NEXUS_DISPLAY_EXTENSION_DYNRNG should be defined by dynrng.inc
#endif

extern NEXUS_DisplayModule_State g_NEXUS_DisplayModule_State;
#define pVideo (&g_NEXUS_DisplayModule_State)

static const char * const dynrngProcessingModeStrings[] =
{
     "AUTO",
     "OFF",
     NULL
};

#define GET_PROC_STR(SETTINGS, TYPE) dynrngProcessingModeStrings[(SETTINGS)->processingModes[NEXUS_DynamicRangeProcessingType_e##TYPE]]

static void NEXUS_P_DynamicRangeProcessingSettingsToMagnum(
    const NEXUS_DynamicRangeProcessingSettings * pSettings,
    BVDC_Test_Window_ForceCfcConfig * pCfcConfig
)
{
    /* turns off all disables */
    BKNI_Memset(pCfcConfig, 0, sizeof(*pCfcConfig));

    /* selectively turns on disables */
    switch (pSettings->processingModes[NEXUS_DynamicRangeProcessingType_ePlm])
    {
        case NEXUS_DynamicRangeProcessingMode_eOff:
#if 0
            pCfcConfig->bDisableNl2l = true;
            pCfcConfig->bDisableL2nl = true;
#endif
            pCfcConfig->bDisableLRangeAdj = true;
#if 0
            pCfcConfig->bDisableLmr = true;
            pCfcConfig->bDisableRamLuts = true;
#endif
            break;
        default:
            break;
    }
    switch (pSettings->processingModes[NEXUS_DynamicRangeProcessingType_eDolbyVision])
    {
        case NEXUS_DynamicRangeProcessingMode_eOff:
            pCfcConfig->bDisableDolby = true;
            break;
        default:
            break;
    }
    switch (pSettings->processingModes[NEXUS_DynamicRangeProcessingType_eTechnicolorPrime])
    {
        case NEXUS_DynamicRangeProcessingMode_eOff:
            pCfcConfig->bDisableTch = true;
            break;
        default:
            break;
    }
}

unsigned NEXUS_FindHdmiDisplay(void)
{
    unsigned display = 0;
    unsigned i;

    for (i = 0; i < NEXUS_NUM_DISPLAYS; i++)
    {
        if (pVideo->displays[i] && pVideo->displays[i]->hdmi.outputNotify)
        {
            display = i;
            break;
        }
    }

    return display;
}

static NEXUS_VideoWindowHandle NEXUS_P_GetActiveVideoWindow(NEXUS_DisplayHandle display, unsigned id)
{
    NEXUS_VideoWindowHandle window = NULL;

    if (id < NEXUS_NUM_VIDEO_WINDOWS && display->windows[id].vdcState.window)
    {
        window = &display->windows[id];
    }

    return window;
}

void NEXUS_VideoWindow_GetDynamicRangeProcessingSettings(
    unsigned displayId,
    unsigned windowId,
    NEXUS_DynamicRangeProcessingSettings * pSettings
)
{
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;

    BDBG_ASSERT(pSettings);

    if (displayId >= NEXUS_NUM_DISPLAYS || windowId > 1) {
        BDBG_ERR(("DisplayId %u windowId %u is not supported!", displayId, windowId));
        return;
    }
    display = pVideo->displays[displayId];

    if (display)
    {
        window = NEXUS_P_GetActiveVideoWindow(display, windowId);
        if (window)
        {
            BKNI_Memcpy(pSettings, &window->dynrng.settings, sizeof(*pSettings));
        }
    }
}

NEXUS_Error NEXUS_VideoWindow_SetDynamicRangeProcessingSettings(
    unsigned displayId,
    unsigned windowId,
    const NEXUS_DynamicRangeProcessingSettings * pSettings
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;

    BDBG_ASSERT(pSettings);

    if (displayId >= NEXUS_NUM_DISPLAYS || windowId > 1) {
        BDBG_ERR(("DisplayId %u windowId %u is not supported!", displayId, windowId));
        return NEXUS_NOT_SUPPORTED;
    }
    display = pVideo->displays[displayId];

    if (display)
    {
        window = NEXUS_P_GetActiveVideoWindow(display, windowId);
        if (window)
        {
            NEXUS_P_DynamicRangeProcessingSettingsToMagnum(pSettings, &window->dynrng.cfcConfig);
            rc = BVDC_Test_Window_SetCfcConfig(window->vdcState.window, &window->dynrng.cfcConfig);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
            NEXUS_Display_P_ApplyChanges();
            BKNI_Memcpy(&window->dynrng.settings, pSettings, sizeof(*pSettings));
            BDBG_MSG(("Video window %d PLM: %s; DBV: %s; TCH: %s", windowId,
                GET_PROC_STR(&window->dynrng.settings, Plm),
                GET_PROC_STR(&window->dynrng.settings, DolbyVision),
                GET_PROC_STR(&window->dynrng.settings, TechnicolorPrime)));
        }
    }

end:
    return rc;
}

void NEXUS_VideoWindow_GetTargetPeakBrightness(
    unsigned displayId,
    unsigned windowId,
    int *psHdrPeak,
    int *psSdrPeak
)
{
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;

    if (displayId >= NEXUS_NUM_DISPLAYS || windowId > 1) {
        BDBG_ERR(("DisplayId %u windowId %u is not supported!", displayId, windowId));
        return;
    }
    display = pVideo->displays[displayId];

    if (display)
    {
        window = NEXUS_P_GetActiveVideoWindow(display, windowId);
        if (window)
        {
            BVDC_Test_Window_GetTargetPeakBrightness(window->vdcState.window, (int16_t *)psHdrPeak, (int16_t *)psSdrPeak);
        }
    }
}

void NEXUS_VideoWindow_SetTargetPeakBrightness(
    unsigned displayId,
    unsigned windowId,
    int  sHdrPeak,
    int  sSdrPeak
)
{
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;

    if (displayId >= NEXUS_NUM_DISPLAYS || windowId > 1) {
        BDBG_ERR(("DisplayId %u windowId %u is not supported!", displayId, windowId));
        return;
    }
    display = pVideo->displays[displayId];

    if (display)
    {
        window = NEXUS_P_GetActiveVideoWindow(display, windowId);
        if (window)
        {
            BVDC_Test_Window_SetTargetPeakBrightness(window->vdcState.window, sHdrPeak, sSdrPeak);
            NEXUS_Display_P_ApplyChanges();
        }
    }
}

void NEXUS_Display_GetGraphicsDynamicRangeProcessingSettings(
    unsigned displayId,
    NEXUS_DynamicRangeProcessingSettings * pSettings
)
{
    NEXUS_DisplayHandle display;

    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    if (displayId >= NEXUS_NUM_DISPLAYS) {
        BDBG_ERR(("DisplayId %u is not supported!", displayId));
        return;
    }
    display = pVideo->displays[displayId];

    if (display)
    {
        BKNI_Memcpy(pSettings, &display->graphics.dynrng.settings, sizeof(*pSettings));
    }
}

NEXUS_Error NEXUS_Display_SetGraphicsDynamicRangeProcessingSettings(
    unsigned displayId,
    const NEXUS_DynamicRangeProcessingSettings * pSettings
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DisplayHandle display;

    BDBG_ASSERT(pSettings);

    if (displayId >= NEXUS_NUM_DISPLAYS) {
        BDBG_ERR(("DisplayId %u is not supported!", displayId));
        return NEXUS_NOT_SUPPORTED;
    }
    display = pVideo->displays[displayId];

    if (display && display->graphics.windowVdc)
    {
        NEXUS_P_DynamicRangeProcessingSettingsToMagnum(pSettings, &display->graphics.dynrng.cfcConfig);
        rc = BVDC_Test_Window_SetCfcConfig(display->graphics.windowVdc, &display->graphics.dynrng.cfcConfig);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
        NEXUS_Display_P_ApplyChanges();
        BKNI_Memcpy(&display->graphics.dynrng.settings, pSettings, sizeof(*pSettings));
        BDBG_MSG(("Graphics PLM: %s; DBV: %s; TCH: %s",
            GET_PROC_STR(&display->graphics.dynrng.settings, Plm),
            GET_PROC_STR(&display->graphics.dynrng.settings, DolbyVision),
            GET_PROC_STR(&display->graphics.dynrng.settings, TechnicolorPrime)));
    }

end:
    return rc;
}

void NEXUS_Display_GetGraphicsLuminanceBounds(
    unsigned displayId,
    NEXUS_GraphicsLuminanceBounds * pBounds
)
{
    NEXUS_DisplayHandle display;

    BDBG_ASSERT(pBounds);
    BKNI_Memset(pBounds, 0, sizeof(*pBounds));

    if (displayId >= NEXUS_NUM_DISPLAYS) {
        BDBG_ERR(("DisplayId %u is not supported!", displayId));
        return;
    }
    display = pVideo->displays[displayId];

    if (display)
    {
        BKNI_Memcpy(pBounds, &display->graphics.dynrng.luminanceBounds, sizeof(*pBounds));
    }
}

NEXUS_Error NEXUS_Display_SetGraphicsLuminanceBounds(
    unsigned displayId,
    const NEXUS_GraphicsLuminanceBounds * pBounds
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DisplayHandle display;

    BDBG_ASSERT(pBounds);

    if (displayId >= NEXUS_NUM_DISPLAYS) {
        BDBG_ERR(("DisplayId %u is not supported!", displayId));
        return NEXUS_NOT_SUPPORTED;
    }
    display = pVideo->displays[displayId];

    if (display && display->graphics.source)
    {
        rc = BVDC_Source_SetGfxLuminance(display->graphics.source, pBounds->min, pBounds->max);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
        NEXUS_Display_P_ApplyChanges();
        BKNI_Memcpy(&display->graphics.dynrng.luminanceBounds, pBounds, sizeof(*pBounds));
        BDBG_MSG(("Graphics luminance bounds: (%u, %u)", display->graphics.dynrng.luminanceBounds.min, display->graphics.dynrng.luminanceBounds.max));
    }

end:
    return rc;
}
