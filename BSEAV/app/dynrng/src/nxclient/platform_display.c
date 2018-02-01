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
 *****************************************************************************/
#include "nexus_platform_client.h"
#include "nexus_platform.h"
#include "nxclient.h"
#include "namevalue.h"
#include "nexus_video_types.h"
#include "nexus_hdmi_output_extra.h"
#include "nxapps_cmdline.h"
#include "platform.h"
#include "platform_priv.h"
#include "platform_display_priv.h"
#include "bkni.h"
#include "bdbg.h"

BDBG_MODULE(platform_display);

#define ENABLE_ASPECT_RATIO 0

PlatformDisplayHandle platform_display_open(PlatformHandle platform)
{
    PlatformDisplayHandle display;
    display = BKNI_Malloc(sizeof(*display));
    BDBG_ASSERT(display);
    BKNI_Memset(display, 0, sizeof(*display));
    platform->display = display;
    display->platform = platform;
    display->hdmi.alias = NEXUS_HdmiOutput_Open(NEXUS_ALIAS_ID + 0, NULL);
    return display;
}

void platform_display_close(PlatformDisplayHandle display)
{
    if (!display) return;
    NEXUS_HdmiOutput_Close(display->hdmi.alias);
    display->platform->display = NULL;
    BKNI_Free(display);
}

void platform_display_get_picture_info(PlatformDisplayHandle display, PlatformPictureInfo * pInfo)
{
    BDBG_ASSERT(display);
    BDBG_ASSERT(pInfo);
    NxClient_GetDisplaySettings(&display->nxSettings);
    display->info.depth = display->nxSettings.hdmiPreferences.colorDepth;
    display->info.dynrng = platform_p_output_dynamic_range_from_nexus(display->nxSettings.hdmiPreferences.drmInfoFrame.eotf,
            display->nxSettings.hdmiPreferences.dolbyVision.outputMode);
    display->info.gamut = platform_p_colorimetry_from_nexus(display->nxSettings.hdmiPreferences.matrixCoefficients);
    display->info.space = platform_p_color_space_from_nexus(display->nxSettings.hdmiPreferences.colorSpace);
    display->info.sampling = platform_p_color_sampling_from_nexus(display->nxSettings.hdmiPreferences.colorSpace);
    display->info.format.dropFrame = display->nxSettings.dropFrame == NEXUS_TristateEnable_eEnable;
    platform_p_picture_format_from_nexus(display->nxSettings.format, &display->info.format);
    platform_p_aspect_ratio_from_nexus(&display->info.ar, display->nxSettings.aspectRatio, display->nxSettings.sampleAspectRatio.x, display->nxSettings.sampleAspectRatio.y);
    BKNI_Memcpy(pInfo, &display->info, sizeof(*pInfo));
}

void platform_display_print_hdmi_drm_settings(PlatformDisplayHandle display, const char *name)
{
    const NxClient_DisplaySettings *pSettings;
    const NEXUS_MasteringDisplayColorVolume * pMdcv;
    const NEXUS_ContentLightLevel * pCll;
    char buf[256];
    unsigned n;

    BDBG_ASSERT(display);

    BDBG_MSG(("%s dynrng settings:", name));

    pSettings = &display->nxSettings;
    pMdcv = &pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume;
    pCll = &pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.contentLightLevel;

    n = 0;
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, "dolby=%s", lookup_name(g_dolbyVisionModeStrs, pSettings->hdmiPreferences.dolbyVision.outputMode));
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, " eotf=%s", lookup_name(g_videoEotfStrs, pSettings->hdmiPreferences.drmInfoFrame.eotf));
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, " matrixCoeffs=%s", lookup_name(g_matrixCoeffStrs, pSettings->hdmiPreferences.matrixCoefficients));
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, " cll={");
    PRINT_PARAM(pCll->max);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, ",");
    PRINT_PARAM(pCll->maxFrameAverage);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, "}");
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, ",mdcv={rgbw=(");
    PRINT_PARAM(pMdcv->redPrimary.x);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, ",");
    PRINT_PARAM(pMdcv->redPrimary.y);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, "),(");
    PRINT_PARAM(pMdcv->greenPrimary.x);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, ",");
    PRINT_PARAM(pMdcv->greenPrimary.y);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, "),(");
    PRINT_PARAM(pMdcv->bluePrimary.x);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, ",");
    PRINT_PARAM(pMdcv->bluePrimary.y);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, "),(");
    PRINT_PARAM(pMdcv->whitePoint.x);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, ",");
    PRINT_PARAM(pMdcv->whitePoint.y);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, "),luma=(");
    PRINT_PARAM(pMdcv->luminance.max);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, ",");
    PRINT_PARAM(pMdcv->luminance.min);
    n += BKNI_Snprintf(&buf[n], sizeof(buf)-n, ")}");
    BDBG_MSG(("%s", buf));
}

void platform_display_p_load_status(PlatformDisplayHandle display)
{
    BDBG_ASSERT(display);
    NEXUS_HdmiOutput_GetStatus(display->hdmi.alias, &display->hdmi.status);
    NEXUS_HdmiOutput_GetExtraStatus(display->hdmi.alias, &display->hdmi.extraStatus);
}

bool platform_display_hdmi_is_connected(PlatformDisplayHandle display)
{
    platform_display_p_load_status(display);
    return display->hdmi.status.connected;
}

bool platform_display_p_is_dolby_vision_supported(PlatformDisplayHandle display)
{
    platform_display_p_load_status(display);
    return display->hdmi.extraStatus.dolbyVision.supported;
}

void platform_display_print_hdmi_status(PlatformDisplayHandle display)
{
    BDBG_ASSERT(display);

    platform_display_p_load_status(display);

#if NEXUS_HAS_HDMI_OUTPUT
    BDBG_MSG(("HdmiOutput: connected? %c (eotf=%s) %s",
        display->hdmi.status.connected ? 'y' : 'n',
        lookup_name(g_videoEotfStrs, display->hdmi.status.eotf),
        display->hdmi.extraStatus.dolbyVision.supported ? "dbv support" : ""));
    NEXUS_HdmiOutput_DisplayRxEdid(display->hdmi.alias);
#endif
}

void platform_display_set_picture_info(PlatformDisplayHandle display, const PlatformPictureInfo * pInfo)
{
    int rc = 0;

    BDBG_ASSERT(display);
    BDBG_ASSERT(pInfo);

    NxClient_GetDisplaySettings(&display->nxSettings);
    platform_p_output_dynamic_range_to_nexus(pInfo->dynrng, &display->nxSettings.hdmiPreferences.drmInfoFrame.eotf,
            &display->nxSettings.hdmiPreferences.dolbyVision.outputMode);
    /* NOTE: now that VDC handles determining which metadata to send based on input, we don't need to do it anymore */
    display->nxSettings.format = platform_p_picture_format_to_nexus(&pInfo->format);
#if ENABLE_ASPECT_RATIO
    display->nxSettings.aspectRatio = platform_p_aspect_ratio_to_nexus(&pInfo->ar, &display->nxSettings.sampleAspectRatio.x, &display->nxSettings.sampleAspectRatio.y);
#endif
    display->nxSettings.dropFrame = pInfo->format.dropFrame ? NEXUS_TristateEnable_eEnable : NEXUS_TristateEnable_eDisable;
    display->nxSettings.hdmiPreferences.matrixCoefficients = platform_p_colorimetry_to_nexus(pInfo->gamut);
    display->nxSettings.hdmiPreferences.colorSpace = platform_p_color_space_and_sampling_to_nexus(pInfo->space, pInfo->sampling);
    display->nxSettings.hdmiPreferences.colorDepth = pInfo->depth;
    rc = NxClient_SetDisplaySettings(&display->nxSettings);
    if (rc) BERR_TRACE(rc);
#if !BDBG_NO_MSG
    {
#define BUF_LEN 128
        char buf[BUF_LEN];
        platform_print_picture_info("new display info", pInfo, buf, BUF_LEN);
        BDBG_MSG((buf));
    }
#endif
    platform_display_print_hdmi_drm_settings(display, "new");
}

void platform_display_set_gfx_luminance(PlatformDisplayHandle display, unsigned min, unsigned max)
{
    char buf[10];
    BSTD_UNUSED(display);
    BKNI_Snprintf(buf, 10, "%u", min);
    NEXUS_SetEnv("dbv.gfx.luma.min", buf);
    BKNI_Snprintf(buf, 10, "%u", max);
    NEXUS_SetEnv("dbv.gfx.luma.max", buf);
}

void platform_display_set_rendering_priority(PlatformDisplayHandle display, PlatformRenderingPriority renderingPriority)
{
    int rc = 0;
    BDBG_ASSERT(display);
    NxClient_GetDisplaySettings(&display->nxSettings);
    display->nxSettings.hdmiPreferences.dolbyVision.priorityMode = platform_p_rendering_priority_to_nexus(renderingPriority);
    rc = NxClient_SetDisplaySettings(&display->nxSettings);
    if (rc) BERR_TRACE(rc);
}

void platform_display_get_picture_quality(PlatformDisplayHandle display, PlatformPictureCtrlSettings * pInfo)
{
    BDBG_ASSERT(display);
    BDBG_ASSERT(pInfo);
    NxClient_GetPictureQualitySettings(&display->nxPQSettings);
    display->pictureCtrlSettings.brightness = display->nxPQSettings.graphicsColor.brightness;
    display->pictureCtrlSettings.contrast   = display->nxPQSettings.graphicsColor.contrast;
    display->pictureCtrlSettings.hue        = display->nxPQSettings.graphicsColor.hue;
    display->pictureCtrlSettings.saturation = display->nxPQSettings.graphicsColor.saturation;
    BKNI_Memcpy(pInfo, &display->pictureCtrlSettings, sizeof(*pInfo));
}

void platform_display_set_picture_quality(PlatformDisplayHandle display, const PlatformPictureCtrlSettings * pInfo)
{
    int rc = 0;

    BDBG_ASSERT(display);
    BDBG_ASSERT(pInfo);

    NxClient_GetPictureQualitySettings(&display->nxPQSettings);
    display->nxPQSettings.graphicsColor.brightness = pInfo->brightness;
    display->nxPQSettings.graphicsColor.contrast   = pInfo->contrast;
    display->nxPQSettings.graphicsColor.hue        = pInfo->hue;
    display->nxPQSettings.graphicsColor.saturation = pInfo->saturation;
    rc = NxClient_SetPictureQualitySettings(&display->nxPQSettings);
    if (rc) BERR_TRACE(rc);
}
