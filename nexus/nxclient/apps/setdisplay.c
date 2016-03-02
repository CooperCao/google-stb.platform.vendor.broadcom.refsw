/******************************************************************************
 *    (c)2010-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "namevalue.h"
#include "nxapps_cmdline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(setdisplay);

static void print_usage(void)
{
    char formatlist[1024] = "";
    unsigned n = 0;
    unsigned i;
#if NEXUS_HAS_HDMI_OUTPUT
    {
    NEXUS_HdmiOutputHandle hdmiOutput;
    hdmiOutput = NEXUS_HdmiOutput_Open(NEXUS_ALIAS_ID + 0, NULL);
    if (hdmiOutput) {
        NEXUS_HdmiOutputStatus status;
        if (!NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status) && status.connected) {
            for (i=0;i<NEXUS_VideoFormat_eMax;i++) {
                if (status.videoFormatSupported[i]) {
                    const char *s = lookup_name(g_videoFormatStrs, i);
                    if (s) {
                        n += snprintf(&formatlist[n], sizeof(formatlist)-n, "%s%s", n?",":"", s);
                        if (n >= sizeof(formatlist)) break;
                    }
                }
            }
        }
        NEXUS_HdmiOutput_Close(hdmiOutput);
    }
    }
#endif
    if (!formatlist[0]) {
        for (i=0;g_videoFormatStrs[i].name;i++) {
            n += snprintf(&formatlist[n], sizeof(formatlist)-n, "%s%s", n?",":"", g_videoFormatStrs[i].name);
            if (n >= sizeof(formatlist)) break;
        }
    }
    n += snprintf(&formatlist[n], sizeof(formatlist)-n, "\n");

    printf(
        "Usage: setdisplay OPTIONS\n"
        "  -format %s",
        formatlist
        );
    printf(
        "  -component {on|off}\n"
        "  -composite {on|off}\n"
        "  -rfm {3|4}\n"
        "  -hdmi {on|off}\n"
        "  -3d {on|lr|ou|off}        enable stereoscopic (3D) video\n"
        "  -color CONTRAST,SATURATION,HUE,BRIGHTNESS   graphics color. values range between -32767 and 32768.\n"
        "  -backgroundColor          ARGB888 hex value for graphics background\n"
        );
    print_list_option("ar",g_displayAspectRatioStrs);
    printf(
        "  -sar X,Y                  Set sample aspect ratio with X:Y dimensions\n"
        "  -sd                       Apply following -format, -ar, -sar and -backgroundColor options to SD display\n"
        "  -wait                     Wait for status changes\n"
        "  -status                   Print extended status\n"
        );
    print_list_option("colorSpace",g_colorSpaceStrs);
    printf(
        "  -colorDepth {0|8|10}      0 is auto\n"
        );
    print_list_option("macrovision",g_macrovisionStrs);
    print_list_option("eotf",g_videoEotfStrs);
    printf(
        "  -cll MAX_CLL         max content light level, units 1 cd/m^2, -1 means from input\n"
        "  -fal MAX_FAL         max frame average light level, units 1 cd/m^2, -1 means from input\n"
    );
    printf(
        "  -mdcv.red X,Y        red chromaticity coordinate, range 0 to 50000, maps to 0.0 to 1.0, -1 means from input\n"
        "  -mdcv.green X,Y      green chromaticity coordinate, range 0 to 50000, maps to 0.0 to 1.0, -1 means from input\n"
        "  -mdcv.blue X,Y       blue chromaticity coordinate, range 0 to 50000, maps to 0.0 to 1.0, -1 means from input\n"
        "  -mdcv.white X,Y      chromaticity white point, range 0 to 50000, maps to 0.0 to 1.0, -1 means from input\n"
    );
    printf(
        "  -mdcv.luma.max MAX   units 1 cd / m^2, -1 means from input\n"
        "  -mdcv.luma.min MIN   units 0.0001 cd / m^2, -1 means from input\n"
    );
}

static bool drm_configured(const NEXUS_HdmiDynamicRangeMasteringInfoFrame *pInfo)
{
    return (int)pInfo->metadata.typeSettings.type1.contentLightLevel.max != -1 ||
        (int)pInfo->metadata.typeSettings.type1.contentLightLevel.maxFrameAverage != -1 ||
        pInfo->metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.x != -1 ||
        pInfo->metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.y != -1 ||
        pInfo->metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.x != -1 ||
        pInfo->metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.y != -1 ||
        pInfo->metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.x != -1 ||
        pInfo->metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.y != -1 ||
        pInfo->metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.x != -1 ||
        pInfo->metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.y != -1 ||
        (int)pInfo->metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.max != -1 ||
        (int)pInfo->metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.min != -1;
}

static void print_settings(const char *name, const NxClient_DisplaySettings *pSettings, const NxClient_PictureQualitySettings *pqSettings)
{
    char buf[256];
    unsigned n;
    printf("%s display settings:\n", name);

    n = 0;
    n += snprintf(&buf[n], sizeof(buf)-n, "  format: %s", lookup_name(g_videoFormatStrs, pSettings->format));
    switch (pSettings->display3DSettings.orientation) {
    case NEXUS_VideoOrientation_e3D_LeftRight:
        n += snprintf(&buf[n], sizeof(buf)-n, " (lr)");
        break;
    case NEXUS_VideoOrientation_e3D_OverUnder:
        n += snprintf(&buf[n], sizeof(buf)-n, " (ou)");
        break;
    default: break;
    }
    printf("%s\n", buf);

    n = 0;
    n += snprintf(&buf[n], sizeof(buf)-n, "  outputs:");
    if (pSettings->componentPreferences.enabled) {
        n += snprintf(&buf[n], sizeof(buf)-n, " component");
    }
    if (pSettings->compositePreferences.enabled) {
        n += snprintf(&buf[n], sizeof(buf)-n, " composite");
    }
    if (pSettings->hdmiPreferences.enabled) {
        n += snprintf(&buf[n], sizeof(buf)-n, " hdmi(%d bit,%s",
            pSettings->hdmiPreferences.colorDepth,
            lookup_name(g_colorSpaceStrs, pSettings->hdmiPreferences.colorSpace));
        if (drm_configured(&pSettings->hdmiPreferences.drmInfoFrame)) {
            n += snprintf(&buf[n], sizeof(buf)-n, ",DRM{%s,mdcv{rgbw=(%d,%d),(%d,%d),(%d,%d),(%d,%d),luma=(%d,%d)},cll={%d,%d}}",
            lookup_name(g_videoEotfStrs, pSettings->hdmiPreferences.drmInfoFrame.eotf),
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.contentLightLevel.max,
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.contentLightLevel.maxFrameAverage,
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.x,
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.y,
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.x,
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.y,
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.x,
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.y,
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.x,
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.y,
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.max,
            pSettings->hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.min);
        }
        n += snprintf(&buf[n], sizeof(buf)-n, ")");
    }
    printf("%s\n", buf);

    if (pqSettings) {
        printf(
            "  color %d,%d,%d,%d\n"
            "  backgroundColor %#x\n",
            pqSettings->graphicsColor.contrast,
            pqSettings->graphicsColor.saturation,
            pqSettings->graphicsColor.hue,
            pqSettings->graphicsColor.brightness,
            pSettings->backgroundColor);
    }
}

static const char *g_hdcpState[NEXUS_HdmiOutputHdcpState_eMax] = {
    "Unpowered",
    "Unauthenticated",
    "WaitForValidVideo",
    "InitializedAuthentication",
    "WaitForReceiverAuthentication",
    "ReceiverR0Ready",
    "R0LinkFailure",
    "ReceiverAuthenticated",
    "WaitForRepeaterReady",
    "CheckForRepeaterReady",
    "RepeaterReady",
    "LinkAuthenticated",
    "EncryptionEnabled",
    "RepeaterAuthenticationFailure",
    "RiLinkIntegrityFailure",
    "PjLinkIntegrityFailure"};

static void print_status(void)
{
    int rc;
    NxClient_DisplayStatus status;
    rc = NxClient_GetDisplayStatus(&status);
    if (!rc) {
        printf(
            "display status\n"
            "  framebuffers=%d\n",
            status.framebuffer.number
            );
        printf("%d of %d transcode displays used\n",
            status.transcodeDisplays.used,
            status.transcodeDisplays.total);
#if NEXUS_HAS_HDMI_OUTPUT
        printf("HdmiOutput: connected? %c; preferred format %s, hdcp %s\n",
            status.hdmi.status.connected?'y':'n',
            lookup_name(g_videoFormatStrs, status.hdmi.status.preferredVideoFormat),
            g_hdcpState[status.hdmi.hdcp.hdcpState]);
#endif
    }
#if NEXUS_HAS_HDMI_OUTPUT
    {
    NEXUS_HdmiOutputHandle hdmiOutput;
    hdmiOutput = NEXUS_HdmiOutput_Open(NEXUS_ALIAS_ID + 0, NULL);
    if (hdmiOutput) {
        NEXUS_HdmiOutputBasicEdidData edid;
        rc = NEXUS_HdmiOutput_GetBasicEdidData(hdmiOutput, &edid);
        printf(
            "Basic EDID:\n"
            "  VendorID: %02x%02x\n"
            "  ProductID: %02x%02x\n"
            "  SerialNum: %02x%02x%02x%02x\n",
            edid.vendorID[0],edid.vendorID[1],
            edid.productID[0],edid.productID[1],
            edid.serialNum[0],edid.serialNum[1],edid.serialNum[2],edid.serialNum[3]
            );
        NEXUS_HdmiOutput_Close(hdmiOutput);
    }
    }
#endif
}

void nxclient_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    switch (param) {
    case 0:
        BDBG_WRN(("hdmiOutputHotplug callback"));
        print_status();
        break;
    case 2:
        BDBG_WRN(("hdmiOutputHdcpChanged callback"));
        print_status();
        break;
    case 1:
        {
        NxClient_DisplaySettings settings;
        BDBG_WRN(("displaySettingsChanged callback"));
        NxClient_GetDisplaySettings(&settings);
        print_settings("change", &settings, NULL);
        }
        break;
    }
}

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_DisplaySettings displaySettings;
    NxClient_PictureQualitySettings pqSettings;
    bool change = false;
    int curarg = 1;
    int rc;
    bool wait = false;
    bool sd = false;
    NEXUS_DisplayMacrovisionType macrovision = NEXUS_DisplayMacrovisionType_eMax;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
    
    NxClient_GetDisplaySettings(&displaySettings);
    NxClient_GetPictureQualitySettings(&pqSettings);
    
    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-sd")) {
            sd = true;
        }
        else if ((!strcmp(argv[curarg], "-format") || !strcmp(argv[curarg], "-display_format")) && argc>curarg+1) {
            NEXUS_VideoFormat format = lookup(g_videoFormatStrs, argv[++curarg]);
            change = true;
            if (sd) {
                displaySettings.slaveDisplay[0].format = format;
            }
            else {
                displaySettings.format = format;
            }
        }
        else if (!strcmp(argv[curarg], "-colorSpace") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.colorSpace = lookup(g_colorSpaceStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-colorDepth") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.colorDepth = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-component") && argc>curarg+1) {
            change = true;
            displaySettings.componentPreferences.enabled = parse_boolean(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-composite") && argc>curarg+1) {
            change = true;
            displaySettings.compositePreferences.enabled = parse_boolean(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-rfm") && argc>curarg+1) {
            change = true;
            displaySettings.compositePreferences.rfmChannel = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-hdmi") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.enabled = parse_boolean(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-3d") && argc>curarg+1) {
            curarg++;
            change = true;
            if (strcmp(argv[curarg], "on")==0 || strcmp(argv[curarg], "lr")==0) {
                displaySettings.display3DSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
            }
            else if(strcmp(argv[curarg], "ou")==0) {
                displaySettings.display3DSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
            }
            else { /* off */
                displaySettings.display3DSettings.orientation = NEXUS_VideoOrientation_e2D;
            }
        }
        else if (!strcmp(argv[curarg], "-status")) {
            print_status();
        }
        else if (!strcmp(argv[curarg], "-color") && argc>curarg+1) {
            int contrast, saturation, hue, brightness;
            if (sscanf(argv[++curarg], "%d,%d,%d,%d", &contrast,&saturation,&hue,&brightness) == 4) {
                pqSettings.graphicsColor.contrast = contrast;
                pqSettings.graphicsColor.saturation = saturation;
                pqSettings.graphicsColor.hue = hue;
                pqSettings.graphicsColor.brightness = brightness;
                change = true;
            }
        }
        else if (!strcmp(argv[curarg], "-backgroundColor") && argc>curarg+1) {
            unsigned backgroundColor = strtoul(argv[++curarg], NULL, 16);
            if (sd) {
                displaySettings.slaveDisplay[0].backgroundColor = backgroundColor;
            }
            else {
                displaySettings.backgroundColor = backgroundColor;
            }
            change = true;
        }
        else if (!strcmp(argv[curarg], "-ar") && argc>curarg+1) {
            NEXUS_DisplayAspectRatio aspectRatio = lookup(g_displayAspectRatioStrs, argv[++curarg]);
            if (sd) {
                displaySettings.slaveDisplay[0].aspectRatio = aspectRatio;
            }
            else {
                displaySettings.aspectRatio = aspectRatio;
            }
            change = true;
        }
        else if (!strcmp(argv[curarg], "-sar") && argc>curarg+1) {
            int x,y;
            if (sscanf(argv[++curarg], "%d,%d", &x, &y) == 2) {
                if (sd) {
                    displaySettings.slaveDisplay[0].aspectRatio = NEXUS_DisplayAspectRatio_eSar;
                    displaySettings.slaveDisplay[0].sampleAspectRatio.x = x;
                    displaySettings.slaveDisplay[0].sampleAspectRatio.y = y;
                }
                else {
                    displaySettings.aspectRatio = NEXUS_DisplayAspectRatio_eSar;
                    displaySettings.sampleAspectRatio.x = x;
                    displaySettings.sampleAspectRatio.y = y;
                }
                change = true;
            }
        }
        else if (!strcmp(argv[curarg], "-wait")) {
            wait = true;
        }
        else if (!strcmp(argv[curarg], "-macrovision") && curarg+1<argc) {
            macrovision = lookup(g_macrovisionStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-eotf") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.drmInfoFrame.eotf = lookup(g_videoEotfStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-mdcv.red") && argc>curarg+1) {
            int x,y;
            if (sscanf(argv[++curarg], "%d,%d", &x, &y) == 2) {
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.x = x;
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.y = y;
                change = true;
            }
        }
        else if (!strcmp(argv[curarg], "-mdcv.green") && argc>curarg+1) {
            int x,y;
            if (sscanf(argv[++curarg], "%d,%d", &x, &y) == 2) {
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.x = x;
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.y = y;
                change = true;
            }
        }
        else if (!strcmp(argv[curarg], "-mdcv.blue") && argc>curarg+1) {
            int x,y;
            if (sscanf(argv[++curarg], "%d,%d", &x, &y) == 2) {
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.x = x;
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.y = y;
                change = true;
            }
        }
        else if (!strcmp(argv[curarg], "-mdcv.white") && argc>curarg+1) {
            int x,y;
            if (sscanf(argv[++curarg], "%d,%d", &x, &y) == 2) {
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.x = x;
                displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.y = y;
                change = true;
            }
        }
        else if (!strcmp(argv[curarg], "-mdcv.luma.max") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.max = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-mdcv.luma.min") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.min = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-cll") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.contentLightLevel.max = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-fal") && argc>curarg+1) {
            change = true;
            displaySettings.hdmiPreferences.drmInfoFrame.metadata.typeSettings.type1.contentLightLevel.maxFrameAverage = atoi(argv[++curarg]);
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (macrovision != NEXUS_DisplayMacrovisionType_eMax) {
        NxClient_Display_SetMacrovision(macrovision, NULL);
    }

    if (wait) {
        NxClient_CallbackThreadSettings settings;
        NxClient_GetDefaultCallbackThreadSettings(&settings);
        settings.hdmiOutputHotplug.callback = nxclient_callback;
        settings.hdmiOutputHotplug.param = 0;
        settings.hdmiOutputHdcpChanged.callback = nxclient_callback;
        settings.hdmiOutputHdcpChanged.param = 2;
        settings.displaySettingsChanged.callback = nxclient_callback;
        settings.displaySettingsChanged.param = 1;
        rc = NxClient_StartCallbackThread(&settings);
        BDBG_ASSERT(!rc);
    }

    if (change) {
        rc = NxClient_SetDisplaySettings(&displaySettings);
        if (rc) BERR_TRACE(rc);
        rc = NxClient_SetPictureQualitySettings(&pqSettings);
        if (rc) BERR_TRACE(rc);
        print_settings("new", &displaySettings, &pqSettings);
    }
    else {
        print_settings("current", &displaySettings, &pqSettings);
    }
    
    if (wait) {
        BKNI_Sleep(30 * 1000);
        NxClient_StopCallbackThread();
    }

    NxClient_Uninit();
    return 0;
}
