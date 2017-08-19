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
#include "namevalue.h"
#include "nexus_video_types.h"
#include "nexus_hdmi_output_extra.h"
#include "nexus_core_utils.h"
#include "nxapps_cmdline.h"
#include "platform.h"
#include "platform_priv.h"
#include "platform_display_priv.h"
#include "bkni.h"
#include "bdbg.h"

BDBG_MODULE(platform_display);

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

const PlatformPictureInfo * platform_display_get_picture_info(PlatformDisplayHandle display)
{
    NEXUS_VideoFormatInfo info;
    BDBG_ASSERT(display);
    NxClient_GetDisplaySettings(&display->nxSettings);
    display->info.depth = display->nxSettings.hdmiPreferences.colorDepth;
    display->info.dynrng = platform_p_output_dynamic_range_from_nexus(display->nxSettings.hdmiPreferences.drmInfoFrame.eotf,
            display->nxSettings.hdmiPreferences.dolbyVision.outputMode);
    display->info.gamut = platform_p_colorimetry_from_nexus(display->nxSettings.hdmiPreferences.matrixCoefficients);
    display->info.space = platform_p_color_space_from_nexus(display->nxSettings.hdmiPreferences.colorSpace);
    NEXUS_VideoFormat_GetInfo(display->nxSettings.format, &info);
    display->info.format.width = info.digitalWidth;
    display->info.format.height = info.digitalHeight;
    display->info.format.interlaced = info.interlaced;
    display->info.format.rate = info.verticalFreq;
    return &display->info;
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

static const NEXUS_HdmiDynamicRangeMasteringStaticMetadata SMD_ZERO =
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
    { /* typeSettings */
        { /* Type1 */
            { /* MasteringDisplayColorVolume */
                { 0, 0 }, /* redPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* greenPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* bluePrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* whitePoint (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* displayLuminance (max, min) units of 1 cd / m2 and 0.0001 cd / m2 respectively */
            },
            { /* ContentLightLevel */
                0, /* maxContentLightLevel units of 1 cd/m2 */
                0 /* maxFrameAverageLightLevel units of 1 cd/m2 */
            }
        }
    }
};

static const NEXUS_HdmiDynamicRangeMasteringStaticMetadata SMD_BT709 =
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
    { /* typeSettings */
        { /* Type1 */
            { /* MasteringDisplayColorVolume */
                { SMD_TO_SMPTE_ST2086(0.64), SMD_TO_SMPTE_ST2086(0.33) }, /* redPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.30), SMD_TO_SMPTE_ST2086(0.60) }, /* greenPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.15), SMD_TO_SMPTE_ST2086(0.06) }, /* bluePrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.3127), SMD_TO_SMPTE_ST2086(0.3290) }, /* whitePoint (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* displayMasteringLuminance (max, min) units of 1 cd / m2 and 0.0001 cd / m2 respectively */
            },
            { /* ContentLightLevel */
                0, /* maxContentLightLevel units of 1 cd/m2 */
                0 /* maxFrameAverageLightLevel units of 1 cd/m2 */
            }
        }
    }
};

static const NEXUS_HdmiDynamicRangeMasteringStaticMetadata SMD_BT2020 =
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
    { /* typeSettings */
        { /* Type1 */
            { /* MasteringDisplayColorVolume */
                { SMD_TO_SMPTE_ST2086(0.708), SMD_TO_SMPTE_ST2086(0.292) }, /* redPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.170), SMD_TO_SMPTE_ST2086(0.797) }, /* greenPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.131), SMD_TO_SMPTE_ST2086(0.046) }, /* bluePrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.3127), SMD_TO_SMPTE_ST2086(0.3290) }, /* whitePoint (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* displayMasteringLuminance (max, min) units of 1 cd / m2 and 0.0001 cd / m2 respectively */
            },
            { /* ContentLightLevel */
                0, /* maxContentLightLevel units of 1 cd/m2 */
                0 /* maxFrameAverageLightLevel units of 1 cd/m2 */
            }
        }
    }
};

void platform_display_p_compute_hdmi_drm_metadata(NEXUS_HdmiDynamicRangeMasteringInfoFrame * pInfoFrame)
{
    BDBG_ASSERT(pInfoFrame);

    if (pInfoFrame->eotf == NEXUS_VideoEotf_eSdr)
    {
        BKNI_Memcpy(&pInfoFrame->metadata, &SMD_ZERO, sizeof(pInfoFrame->metadata));
    }
    else if (pInfoFrame->eotf == NEXUS_VideoEotf_eHdr10)
    {
        BKNI_Memcpy(&pInfoFrame->metadata, &SMD_BT2020, sizeof(pInfoFrame->metadata));
    }
    else if (pInfoFrame->eotf == NEXUS_VideoEotf_eHlg)
    {
        BKNI_Memcpy(&pInfoFrame->metadata, &SMD_ZERO, sizeof(pInfoFrame->metadata));
    }
    else if (pInfoFrame->eotf == NEXUS_VideoEotf_eInvalid)
    {
        BKNI_Memcpy(&pInfoFrame->metadata, &SMD_ZERO, sizeof(pInfoFrame->metadata));
    }
    else
    {
        BDBG_ERR(("Unrecognized eotf: %u", pInfoFrame->eotf));
    }
}

void platform_display_set_dynamic_range(PlatformDisplayHandle display, PlatformDynamicRange dynrng)
{
    int rc = 0;

    BDBG_ASSERT(display);

    NxClient_GetDisplaySettings(&display->nxSettings);
    platform_p_output_dynamic_range_to_nexus(dynrng, &display->nxSettings.hdmiPreferences.drmInfoFrame.eotf,
            &display->nxSettings.hdmiPreferences.dolbyVision.outputMode);
    platform_display_p_compute_hdmi_drm_metadata(&display->nxSettings.hdmiPreferences.drmInfoFrame);
    rc = NxClient_SetDisplaySettings(&display->nxSettings);
    if (rc) BERR_TRACE(rc);
    platform_display_print_hdmi_drm_settings(display, "new");
}

void platform_display_set_colorimetry(PlatformDisplayHandle display, PlatformColorimetry colorimetry)
{
    int rc = 0;

    BDBG_ASSERT(display);

    NxClient_GetDisplaySettings(&display->nxSettings);
    display->nxSettings.hdmiPreferences.matrixCoefficients = platform_p_colorimetry_to_nexus(colorimetry);
    rc = NxClient_SetDisplaySettings(&display->nxSettings);
    if (rc) BERR_TRACE(rc);
    platform_display_print_hdmi_drm_settings(display, "new");
}
