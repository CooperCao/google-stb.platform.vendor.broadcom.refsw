/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#include "nexus_platform_client.h"
#include "nexus_platform.h"
#include "nxclient.h"
#include "nexus_display_dynrng.h"
#include "platform.h"
#include "platform_priv.h"
#include "platform_display_priv.h"
#include "bkni.h"
#include "bdbg.h"

BDBG_MODULE(platform_dynrng_processing);

static void platform_display_p_dynamic_range_processing_capabilities_from_nexus(
    const NEXUS_DynamicRangeProcessingCapabilities * pNexus,
    PlatformDynamicRangeProcessingCapabilities * pCapabilities)
{
    pCapabilities->typesSupported[PlatformDynamicRangeProcessingType_ePlm] = pNexus->typesSupported[NEXUS_DynamicRangeProcessingType_ePlm];
    pCapabilities->typesSupported[PlatformDynamicRangeProcessingType_eDolbyVision] = pNexus->typesSupported[NEXUS_DynamicRangeProcessingType_eDolbyVision];
    pCapabilities->typesSupported[PlatformDynamicRangeProcessingType_eTechnicolorPrime] = pNexus->typesSupported[NEXUS_DynamicRangeProcessingType_eTechnicolorPrime];
}

static PlatformDynamicRangeProcessingMode platform_display_p_dynamic_range_mode_from_nexus(NEXUS_DynamicRangeProcessingMode nexus)
{
    PlatformDynamicRangeProcessingMode mode;
    switch (nexus)
    {
        case NEXUS_DynamicRangeProcessingMode_eOff:
            mode = PlatformDynamicRangeProcessingMode_eOff;
            break;
        default:
        case NEXUS_DynamicRangeProcessingMode_eAuto:
            mode = PlatformDynamicRangeProcessingMode_eAuto;
            break;
    }

    return mode;
}

static NEXUS_DynamicRangeProcessingMode platform_display_p_dynamic_range_mode_to_nexus(PlatformDynamicRangeProcessingMode mode)
{
    NEXUS_DynamicRangeProcessingMode nexus;
    switch (mode)
    {
        case PlatformDynamicRangeProcessingMode_eOff:
            nexus = NEXUS_DynamicRangeProcessingMode_eOff;
            break;
        default:
        case PlatformDynamicRangeProcessingMode_eAuto:
            nexus = NEXUS_DynamicRangeProcessingMode_eAuto;
            break;
    }

    return nexus;
}

static void platform_display_p_dynamic_range_processing_settings_from_nexus(
    const NEXUS_DynamicRangeProcessingSettings * pNexus,
    PlatformDynamicRangeProcessingSettings * pSettings)
{
    pSettings->modes[PlatformDynamicRangeProcessingType_ePlm] = platform_display_p_dynamic_range_mode_from_nexus(pNexus->processingModes[NEXUS_DynamicRangeProcessingType_ePlm]);
    pSettings->modes[PlatformDynamicRangeProcessingType_eDolbyVision] = platform_display_p_dynamic_range_mode_from_nexus(pNexus->processingModes[NEXUS_DynamicRangeProcessingType_eDolbyVision]);
    pSettings->modes[PlatformDynamicRangeProcessingType_eTechnicolorPrime] = platform_display_p_dynamic_range_mode_from_nexus(pNexus->processingModes[NEXUS_DynamicRangeProcessingType_eTechnicolorPrime]);
}

static void platform_display_p_dynamic_range_processing_settings_to_nexus(
    const PlatformDynamicRangeProcessingSettings * pSettings,
    NEXUS_DynamicRangeProcessingSettings * pNexus)
{
    pNexus->processingModes[NEXUS_DynamicRangeProcessingType_ePlm] = platform_display_p_dynamic_range_mode_to_nexus(pSettings->modes[PlatformDynamicRangeProcessingType_ePlm]);
    pNexus->processingModes[NEXUS_DynamicRangeProcessingType_eDolbyVision] = platform_display_p_dynamic_range_mode_to_nexus(pSettings->modes[PlatformDynamicRangeProcessingType_eDolbyVision]);
    pNexus->processingModes[NEXUS_DynamicRangeProcessingType_eTechnicolorPrime] = platform_display_p_dynamic_range_mode_to_nexus(pSettings->modes[PlatformDynamicRangeProcessingType_eTechnicolorPrime]);
}

void platform_display_get_video_dynamic_range_processing_capabilities(PlatformDisplayHandle display, unsigned windowId, PlatformDynamicRangeProcessingCapabilities * pCapabilities)
{
    NEXUS_DynamicRangeProcessingCapabilities caps;
    BSTD_UNUSED(display);
    BDBG_ASSERT(pCapabilities);
    NEXUS_VideoWindow_GetDynamicRangeProcessingCapabilities(windowId, &caps);
    platform_display_p_dynamic_range_processing_capabilities_from_nexus(&caps, pCapabilities);
}

void platform_display_get_video_dynamic_range_processing_settings(PlatformDisplayHandle display, unsigned windowId, PlatformDynamicRangeProcessingSettings * pSettings)
{
    NEXUS_DynamicRangeProcessingSettings settings;
    BSTD_UNUSED(display);
    BDBG_ASSERT(pSettings);
    NEXUS_VideoWindow_GetDynamicRangeProcessingSettings(windowId, &settings);
    platform_display_p_dynamic_range_processing_settings_from_nexus(&settings, pSettings);
}

void platform_display_set_video_dynamic_range_processing_settings(PlatformDisplayHandle display, unsigned windowId, const PlatformDynamicRangeProcessingSettings * pSettings)
{
    NEXUS_Error rc =  NEXUS_SUCCESS;
    NEXUS_DynamicRangeProcessingSettings settings;
    BSTD_UNUSED(display);
    BDBG_ASSERT(pSettings);
    NEXUS_VideoWindow_GetDynamicRangeProcessingSettings(windowId, &settings);
    platform_display_p_dynamic_range_processing_settings_to_nexus(pSettings, &settings);
    rc = NEXUS_VideoWindow_SetDynamicRangeProcessingSettings(windowId, &settings);
    if (rc) { rc = BERR_TRACE(rc); }
}

void platform_display_get_graphics_dynamic_range_processing_capabilities(PlatformDisplayHandle display, PlatformDynamicRangeProcessingCapabilities * pCapabilities)
{
    NEXUS_DynamicRangeProcessingCapabilities caps;
    BSTD_UNUSED(display);
    BDBG_ASSERT(pCapabilities);
    NEXUS_Display_GetGraphicsDynamicRangeProcessingCapabilities(&caps);
    platform_display_p_dynamic_range_processing_capabilities_from_nexus(&caps, pCapabilities);
}

void platform_display_get_graphics_dynamic_range_processing_settings(PlatformDisplayHandle display, PlatformDynamicRangeProcessingSettings * pSettings)
{
    NEXUS_DynamicRangeProcessingSettings settings;
    BSTD_UNUSED(display);
    BDBG_ASSERT(pSettings);
    NEXUS_Display_GetGraphicsDynamicRangeProcessingSettings(&settings);
    platform_display_p_dynamic_range_processing_settings_from_nexus(&settings, pSettings);
}

void platform_display_set_graphics_dynamic_range_processing_settings(PlatformDisplayHandle display, const PlatformDynamicRangeProcessingSettings * pSettings)
{
    NEXUS_Error rc =  NEXUS_SUCCESS;
    NEXUS_DynamicRangeProcessingSettings settings;
    BSTD_UNUSED(display);
    BDBG_ASSERT(pSettings);
    NEXUS_Display_GetGraphicsDynamicRangeProcessingSettings(&settings);
    platform_display_p_dynamic_range_processing_settings_to_nexus(pSettings, &settings);
    rc = NEXUS_Display_SetGraphicsDynamicRangeProcessingSettings(&settings);
    if (rc) { rc = BERR_TRACE(rc); }
}
