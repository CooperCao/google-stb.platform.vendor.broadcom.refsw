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

static void NEXUS_P_DynamicRangeProcessingCapabilitiesFromMagnum(
    NEXUS_DynamicRangeProcessingCapabilities * pNexus,
    const BVDC_Window_Capabilities * pMagnum)
{
    BKNI_Memset(pNexus, 0, sizeof(*pNexus));
    pNexus->typesSupported[NEXUS_DynamicRangeProcessingType_ePlm] = pMagnum->bConvHdr10;
    pNexus->typesSupported[NEXUS_DynamicRangeProcessingType_eDolbyVision] = pMagnum->bDolby;
    pNexus->typesSupported[NEXUS_DynamicRangeProcessingType_eTechnicolorPrime] = pMagnum->bTchInput;
}

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

static NEXUS_DisplayHandle NEXUS_P_FindHdmiDisplay(void)
{
    NEXUS_DisplayHandle display = NULL;
    unsigned i;

    for (i = 0; i < NEXUS_NUM_DISPLAYS; i++)
    {
        if (pVideo->displays[i] && pVideo->displays[i]->hdmi.outputNotify)
        {
            display = pVideo->displays[i];
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

void NEXUS_VideoWindow_GetDynamicRangeProcessingCapabilities(
    unsigned windowId,
    NEXUS_DynamicRangeProcessingCapabilities * pCapabilities
)
{
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    BVDC_Window_Capabilities vdcCaps;

    BDBG_ASSERT(pCapabilities);
    BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));

    display = NEXUS_P_FindHdmiDisplay();

    if (display)
    {
        window = NEXUS_P_GetActiveVideoWindow(display, windowId);
        if (window)
        {
            BVDC_Window_GetCapabilities(window->vdcState.window, &vdcCaps);
            NEXUS_P_DynamicRangeProcessingCapabilitiesFromMagnum(pCapabilities, &vdcCaps);
        }
    }
}

void NEXUS_VideoWindow_GetDynamicRangeProcessingSettings(
    unsigned windowId,
    NEXUS_DynamicRangeProcessingSettings * pSettings
)
{
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;

    BDBG_ASSERT(pSettings);

    display = NEXUS_P_FindHdmiDisplay();

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
    unsigned windowId,
    const NEXUS_DynamicRangeProcessingSettings * pSettings
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;

    BDBG_ASSERT(pSettings);

    display = NEXUS_P_FindHdmiDisplay();

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
    unsigned windowId,
    int *psHdrPeak,
    int *psSdrPeak
)
{
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;

    display = NEXUS_P_FindHdmiDisplay();

    if (display)
    {
        window = NEXUS_P_GetActiveVideoWindow(display, windowId);
        if (window)
        {
            *psHdrPeak = window->dynrng.hdrPeak;
            *psSdrPeak = window->dynrng.sdrPeak;
        }
    }
}

void NEXUS_VideoWindow_SetTargetPeakBrightness(
    unsigned windowId,
    int  sHdrPeak,
    int  sSdrPeak
)
{
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;

    display = NEXUS_P_FindHdmiDisplay();

    if (display)
    {
        window = NEXUS_P_GetActiveVideoWindow(display, windowId);
        if (window)
        {
            BVDC_Test_Window_SetTargetPeakBrightness(window->vdcState.window, sHdrPeak, sSdrPeak);
            NEXUS_Display_P_ApplyChanges();
            window->dynrng.hdrPeak = sHdrPeak;
            window->dynrng.sdrPeak = sSdrPeak;
        }
    }
}

void NEXUS_Display_GetGraphicsDynamicRangeProcessingCapabilities(
    NEXUS_DynamicRangeProcessingCapabilities * pCapabilities
)
{
    NEXUS_DisplayHandle display;
    BVDC_Window_Capabilities vdcCaps;

    BDBG_ASSERT(pCapabilities);
    BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));

    display = NEXUS_P_FindHdmiDisplay();

    if (display && display->graphics.windowVdc)
    {
        BVDC_Window_GetCapabilities(display->graphics.windowVdc, &vdcCaps);
        NEXUS_P_DynamicRangeProcessingCapabilitiesFromMagnum(pCapabilities, &vdcCaps);
    }
}

void NEXUS_Display_GetGraphicsDynamicRangeProcessingSettings(
    NEXUS_DynamicRangeProcessingSettings * pSettings
)
{
    NEXUS_DisplayHandle display;

    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    display = NEXUS_P_FindHdmiDisplay();

    if (display)
    {
        BKNI_Memcpy(pSettings, &display->graphics.dynrng.settings, sizeof(*pSettings));
    }
}

NEXUS_Error NEXUS_Display_SetGraphicsDynamicRangeProcessingSettings(
    const NEXUS_DynamicRangeProcessingSettings * pSettings
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DisplayHandle display;

    BDBG_ASSERT(pSettings);

    display = NEXUS_P_FindHdmiDisplay();

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
