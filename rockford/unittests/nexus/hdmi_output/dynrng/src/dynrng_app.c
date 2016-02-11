/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
******************************************************************************/

#include "nexus_platform.h"
#include "nexus_hdmi_output.h"
#include "nexus_video_decoder.h"
#include "nexus_playback.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_graphics2d.h"
#include "nexus_surface.h"
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include "blst_queue.h"
#include "dynrng_args.h"
#include "dynrng_app.h"
#include "dynrng_app_priv.h"
#include "dynrng_shell.h"
#include "dynrng_osd.h"
#include "dynrng_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int APP_ApplyDrmInfoFrame(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_HdmiOutput_GetSettings(app->hdmi, &hdmiSettings);
    memcpy(&hdmiSettings.dynamicRangeMasteringInfoFrame, &app->drmInfoFrame, sizeof(NEXUS_HdmiDynamicRangeMasteringInfoFrame));
    rc = NEXUS_HdmiOutput_SetSettings(app->hdmi, &hdmiSettings);
    if (rc) { fprintf(stderr, "Error setting DRMInfoFrame\n"); rc = BERR_TRACE(rc); }
    return rc;
}

int APP_ApplyColorSettings(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_HdmiOutput_GetSettings(app->hdmi, &hdmiSettings);
    hdmiSettings.colorSpace = app->args.colorSpace;
    hdmiSettings.colorDepth = app->args.colorDepth;
    if (app->args.colorRange == APP_CMD_eAuto)
    {
        hdmiSettings.overrideColorRange = false;
        hdmiSettings.colorRange = NEXUS_ColorRange_eLimited;
    }
    else
    {
        hdmiSettings.overrideColorRange = true;
        hdmiSettings.colorRange = app->args.colorRange;
    }
    if (app->args.matrixCoefficients == APP_CMD_eAuto)
    {
            hdmiSettings.overrideMatrixCoefficients = false;
            hdmiSettings.matrixCoefficients = NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL;
    }
    else if (app->args.matrixCoefficients == APP_CMD_eInput)
    {
        if ((app->lastStreamInfoValid) &&
            (app->lastStreamInfo.matrixCoefficients != NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL) &&
            (app->lastStreamInfo.matrixCoefficients != NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL))
        {
            hdmiSettings.overrideMatrixCoefficients = true;
            hdmiSettings.matrixCoefficients = NEXUS_MatrixCoefficients_eItu_R_BT_709;
        }
        else
        {
            hdmiSettings.overrideMatrixCoefficients = false;
            hdmiSettings.matrixCoefficients = NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL;
        }
    }
    else
    {
        hdmiSettings.overrideMatrixCoefficients = true;
        hdmiSettings.matrixCoefficients = app->args.matrixCoefficients;
    }
    rc = NEXUS_HdmiOutput_SetSettings(app->hdmi, &hdmiSettings);
    if (rc) { fprintf(stderr, "Error setting colorspace/colordepth\n"); rc = BERR_TRACE(rc); }
    return rc;
}

void APP_HdmiHotplugCallback(void * context, int param)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiOutputStatus status;
    APP_AppHandle app = context;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_GraphicsSettings graphicsCompositorSettings;
    const NEXUS_Rect * fbr;

    BSTD_UNUSED(param);

    rc = NEXUS_HdmiOutput_GetStatus(app->hdmi, &status);
    /* the app can choose to switch to the preferred format, but it's not required. */
    if (rc || !status.connected)
    {
        fprintf(stdout, "No RxDevice Connected\n");
        return;
    }

    rc = NEXUS_HdmiOutput_GetEdidData(app->hdmi, &app->sinkEdid);
    if (!rc)
    {
        app->sinkEdidValid = true;
        APP_PrintEdid(app);
    }
    else { BERR_TRACE(rc); /* continue */}

    NEXUS_Display_GetSettings(app->display, &displaySettings);
    if (!status.videoFormatSupported[displaySettings.format])
    {
        fprintf(stderr, "Current format not supported by attached monitor. Switching to preferred format %d\n",
            status.preferredVideoFormat);
        app->args.format = displaySettings.format = status.preferredVideoFormat;
    }
    rc = NEXUS_Display_SetSettings(app->display, &displaySettings);
    if (rc) { rc = BERR_TRACE(rc); /* continue */ }

    fbr = OSD_GetFrameBufferDimensions(app->osd);

    NEXUS_Display_GetGraphicsSettings(app->display, &graphicsCompositorSettings);
    graphicsCompositorSettings.enabled = true;
    graphicsCompositorSettings.clip.width = fbr->width;
    graphicsCompositorSettings.clip.height = fbr->height;
    rc = NEXUS_Display_SetGraphicsSettings(app->display, &graphicsCompositorSettings);
    if (rc) { rc = BERR_TRACE(rc); /* continue */ }

    rc = NEXUS_Display_SetGraphicsFramebuffer(app->display, OSD_GetFrameBuffer(app->osd));
    if (rc) { rc = BERR_TRACE(rc); /* continue */ }

#if 1
    rc = APP_ApplyColorSettings(app);
    if (rc) { rc = BERR_TRACE(rc); /* continue */ }
#endif

#if 1
    rc = APP_ApplyDrmInfoFrame(app);
    if (rc) { rc = BERR_TRACE(rc); /* continue */ }
#endif

    /* force HDMI updates after a hotplug */
    NEXUS_HdmiOutput_GetSettings(app->hdmi, &hdmiSettings);
    rc = NEXUS_HdmiOutput_SetSettings(app->hdmi, &hdmiSettings);
    if (rc) { rc = BERR_TRACE(rc); /* continue */ }

    rc = APP_UpdateOsd(app);
    if (rc) { rc = BERR_TRACE(rc); /* continue */ }
}

void APP_StreamChangedCallback(void * context, int param)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    APP_AppHandle app = context;
    NEXUS_VideoDecoderStreamInformation streamInfo;
    bool streamInfoChanged = false;

    BSTD_UNUSED(param);

    rc = NEXUS_VideoDecoder_GetStreamInformation(app->videoDecoder, &streamInfo);
    if (!rc)
    {
        if (!app->lastStreamInfoValid || (app->lastStreamInfo.matrixCoefficients != streamInfo.matrixCoefficients))
        {
            fprintf(stdout, "Stream matrixCoefficients changed %s -> %s\n",
                    !app->lastStreamInfoValid ? "unspecified" :
                    APP_GetMatrixCoefficientsName(app->lastStreamInfo.matrixCoefficients),
                    APP_GetMatrixCoefficientsName(streamInfo.matrixCoefficients));
            streamInfoChanged = true;
        }
        if (!app->lastStreamInfoValid || (app->lastStreamInfo.eotf != streamInfo.eotf))
        {
            fprintf(stdout, "Stream EOTF changed %s -> %s\n",
                app->lastStreamInfoValid ?
                    APP_GetEotfName(APP_NexusEotfToApp(app->lastStreamInfo.eotf))
                    : "unspecified",
                APP_GetEotfName(APP_NexusEotfToApp(streamInfo.eotf)));
            if (app->eotf == APP_Eotf_eInput)
            {
                app->drmInfoFrame.eotf = streamInfo.eotf;
            }
            streamInfoChanged = true;
        }
        if (!app->lastStreamInfoValid || BKNI_Memcmp(&app->lastStreamInfo.contentLightLevel, &streamInfo.contentLightLevel, sizeof(NEXUS_ContentLightLevel)))
        {
            NEXUS_HdmiDynamicRangeMasteringStaticMetadata metadata;
            fprintf(stdout, "Stream content light level changed (%u, %u) -> (%u, %u)\n",
                app->lastStreamInfo.contentLightLevel.max, app->lastStreamInfo.contentLightLevel.maxFrameAverage,
                streamInfo.contentLightLevel.max, streamInfo.contentLightLevel.maxFrameAverage);
            SMD_GetInputMetadata(app->smd, &metadata);
            BKNI_Memcpy(&metadata.typeSettings.type1.contentLightLevel, &streamInfo.contentLightLevel, sizeof(NEXUS_ContentLightLevel));
            rc = SMD_SetInputMetadata(app->smd, &metadata);
            if (rc) { BERR_TRACE(rc); }
            streamInfoChanged = true;
        }
        if (!app->lastStreamInfoValid || BKNI_Memcmp(&app->lastStreamInfo.masteringDisplayColorVolume, &streamInfo.masteringDisplayColorVolume, sizeof(NEXUS_MasteringDisplayColorVolume)))
        {
            NEXUS_HdmiDynamicRangeMasteringStaticMetadata metadata;
            fprintf(stdout, "Stream mastering display color volume changed: ");
            fprintf(stdout, "G(%u, %u) B(%u, %u) R(%u, %u) W(%u, %u) L(%u, %u) -> G(%u, %u) B(%u, %u) R(%u, %u) W(%u, %u) L(%u, %u)\n",
                app->lastStreamInfo.masteringDisplayColorVolume.greenPrimary.x, app->lastStreamInfo.masteringDisplayColorVolume.greenPrimary.y,
                app->lastStreamInfo.masteringDisplayColorVolume.bluePrimary.x, app->lastStreamInfo.masteringDisplayColorVolume.bluePrimary.y,
                app->lastStreamInfo.masteringDisplayColorVolume.redPrimary.x, app->lastStreamInfo.masteringDisplayColorVolume.redPrimary.y,
                app->lastStreamInfo.masteringDisplayColorVolume.whitePoint.x, app->lastStreamInfo.masteringDisplayColorVolume.whitePoint.y,
                app->lastStreamInfo.masteringDisplayColorVolume.luminance.max, app->lastStreamInfo.masteringDisplayColorVolume.luminance.min,
                streamInfo.masteringDisplayColorVolume.greenPrimary.x, streamInfo.masteringDisplayColorVolume.greenPrimary.y,
                streamInfo.masteringDisplayColorVolume.bluePrimary.x, streamInfo.masteringDisplayColorVolume.bluePrimary.y,
                streamInfo.masteringDisplayColorVolume.redPrimary.x, streamInfo.masteringDisplayColorVolume.redPrimary.y,
                streamInfo.masteringDisplayColorVolume.whitePoint.x, streamInfo.masteringDisplayColorVolume.whitePoint.y,
                streamInfo.masteringDisplayColorVolume.luminance.max, streamInfo.masteringDisplayColorVolume.luminance.min);
            SMD_GetInputMetadata(app->smd, &metadata);
            BKNI_Memcpy(&metadata.typeSettings.type1.masteringDisplayColorVolume, &streamInfo.masteringDisplayColorVolume, sizeof(NEXUS_MasteringDisplayColorVolume));
            rc = SMD_SetInputMetadata(app->smd, &metadata);
            if (rc) { BERR_TRACE(rc); }
            streamInfoChanged = true;
        }

        if (streamInfoChanged && (SMD_GetSmdSource(app->smd) == SMD_SmdSource_eInput))
        {
            rc = SMD_GetMetadata(app->smd, &app->drmInfoFrame.metadata);
            if (rc) { BERR_TRACE(rc); }

            rc = APP_ApplyDrmInfoFrame(app);
            if (rc) { BERR_TRACE(rc); }
        }

        app->lastStreamInfo = streamInfo;
        app->lastStreamInfoValid = true;

        if (streamInfoChanged)
        {
            rc = APP_ApplyColorSettings(app);
            if (rc) { rc = BERR_TRACE(rc); }
        }

        rc = APP_UpdateOsd(app);
        if (rc) { BERR_TRACE(rc); }
    }
}

void APP_PrintEdid(APP_AppHandle app)
{
    unsigned i;

    fprintf(stdout, "# Sink EDID\n");
    fprintf(stdout, "valid = %s\n", app->sinkEdidValid ? "true" : "false");
    if (app->sinkEdidValid)
    {
        if (app->sinkEdid.hdrdb.valid)
        {
            fprintf(stdout, "# EOTF Support\n");
            for (i = 0; i < NEXUS_VideoEotf_eMax; i++)
            {
                fprintf(stdout, "%s = %s\n", APP_GetEotfName(APP_NexusEotfToApp(i)),
                    app->sinkEdid.hdrdb.eotfSupported[i] ? "supported" : "not supported");
            }
        }
        else
        {
            fprintf(stdout, "No HDRDB; SDR support only\n");
        }
    }
    else
    {
        fprintf(stdout, "EOTF support unknown\n");
    }
}

void APP_PrintStreamInfo(APP_AppHandle app)
{
    fprintf(stdout, "# Stream Info\n");
    fprintf(stdout, "valid = %s\n", app->lastStreamInfoValid ? "true" : "false");
    fprintf(stdout, "eotf = %s\n", APP_GetEotfName(APP_NexusEotfToApp(app->lastStreamInfo.eotf)));
}

void APP_PrintDrmInfoFrame(APP_AppHandle app)
{
    fprintf(stdout, "# DRM Info Frame\n");
    fprintf(stdout, "eotf = %s\n", APP_GetEotfName(APP_NexusEotfToApp(app->drmInfoFrame.eotf)));
    fprintf(stdout, "metadata-source = %s\n", SMD_GetSmdSourceName(SMD_GetSmdSource(app->smd)));
    SMD_PrintMetadata("stb", &app->drmInfoFrame.metadata);
}

void APP_PrintNexus(APP_AppHandle app)
{
    fprintf(stdout, "# NEXUS\n");
    fprintf(stdout, "This function is not yet implemented\n");
    BSTD_UNUSED(app);
    /* TODO */
}

void APP_Print(APP_AppHandle app)
{
    ARGS_Print(&app->args);
    APP_PrintStreamInfo(app);
    APP_PrintDrmInfoFrame(app);
    APP_PrintEdid(app);
    APP_PrintNexus(app);
    OSD_Print(app->osd);
    SMD_Print(app->smd);
    SHELL_Print(app->shell);
}

OSD_Eotf APP_NexusEotfToOsd(NEXUS_VideoEotf nxEotf)
{
    OSD_Eotf eotf = OSD_Eotf_eUnknown;

    switch (nxEotf)
    {
        case NEXUS_VideoEotf_eSdr:
            eotf = OSD_Eotf_eSdr;
            break;
        case NEXUS_VideoEotf_eHdr:
            eotf = OSD_Eotf_eHdr;
            break;
        case NEXUS_VideoEotf_eFuture:
            eotf = OSD_Eotf_eArib;
            break;
        case NEXUS_VideoEotf_eSmpteSt2084:
            eotf = OSD_Eotf_eSmpte;
            break;
        default:
            eotf = OSD_Eotf_eUnknown;
            break;
    }

    return eotf;
}

int APP_UpdateOsd(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    OSD_OsdModel model;

    OSD_GetModel(app->osd, &model);
    if (app->lastStreamInfoValid)
    {
        model.input = APP_NexusEotfToOsd(app->lastStreamInfo.eotf);
    }
    else
    {
        model.input = OSD_Eotf_eUnknown;
    }
    model.output = APP_NexusEotfToOsd(app->drmInfoFrame.eotf);
    if (app->sinkEdidValid)
    {
        if (app->sinkEdid.hdrdb.valid)
        {
            if (app->sinkEdid.hdrdb.eotfSupported[app->drmInfoFrame.eotf])
            {
                model.tv = OSD_EotfSupport_eYes;
            }
            else
            {
                model.tv = OSD_EotfSupport_eNo;
            }
        }
        else
        {
            /* if we have valid edid with no hdrdb, this means SDR support only */
            if (app->drmInfoFrame.eotf == NEXUS_VideoEotf_eSdr)
            {
                model.tv = OSD_EotfSupport_eYes;
            }
            else
            {
                model.tv = OSD_EotfSupport_eNo;
            }
        }
    }
    else
    {
        model.tv = OSD_EotfSupport_eUnknown;
    }
    rc = OSD_SetModel(app->osd, &model);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

int APP_DoOsd(void * context, char * args)
{
    APP_AppHandle app = context;
    NEXUS_Error rc = NEXUS_SUCCESS;
    OSD_OsdSettings settings;
    OSD_OsdMode mode;
    const char * modeStr;
    const char * timeoutStr;

    OSD_GetSettings(app->osd, &settings);
    if (args)
    {
        modeStr = strtok(args, " \t\n");
        mode = OSD_ParseOsdMode(modeStr);
        if (mode != OSD_OsdMode_eMax)
        {
            settings.mode = mode;
            if (mode == OSD_OsdMode_eTimer)
            {
                timeoutStr = strtok(NULL, " \t\n");
                if (timeoutStr)
                {
                    settings.timeout = atoi(timeoutStr);
                    if (!settings.timeout)
                    {
                        settings.mode = OSD_OsdMode_eOff;
                    }
                }
            }
        }
        else
        {
            fprintf(stderr, "Unrecognized argument '%s' ignored\n", args);
            goto end;
        }
    }
    else
    {
        switch (settings.mode)
        {
            case OSD_OsdMode_eTimer:
                /* no change */
                break;
            case OSD_OsdMode_eOn:
                /* toggle osd state */
                settings.mode = OSD_OsdMode_eOff;
                break;
            case OSD_OsdMode_eOff:
                /* toggle osd state */
                settings.mode = OSD_OsdMode_eOn;
                break;
            default:
                rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
                goto end;
                break;
        }
    }

    rc = OSD_SetSettings(app->osd, &settings);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

int APP_SetupOsdCommand(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    command = SHELL_CreateCommand(app->shell,
        "osd",
        "draws icons for stream, stb, and TV status on lower left corner",
        "[on|off|timer [timeout]]",
        &APP_DoOsd,
        app);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = SHELL_AddCommandAlias(command, "o");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "on", false, "enable OSD");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "off", false, "disable OSD");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "timer", false, "enable OSD for x seconds, then disable it. X is queried at the time of command issue.");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

static const UTILS_StringIntMapEntry statusRequestAliases[] =
{
    { "args", APP_StatusRequest_eArgs },
    { "nexus", APP_StatusRequest_eNexus },
    { "stream", APP_StatusRequest_eStreamInfo },
    { "eotf", APP_StatusRequest_eEotf },
    { "drm", APP_StatusRequest_eDrmInfoFrame },
    { "edid", APP_StatusRequest_eEdid },
    { "osd", APP_StatusRequest_eOsd },
    { "smd", APP_StatusRequest_eSmd },
    { "shell", APP_StatusRequest_eShell },
    { NULL, APP_StatusRequest_eMax }
};

int APP_DoStatus(void * context, char * args)
{
    APP_AppHandle app = context;
    NEXUS_Error rc = NEXUS_SUCCESS;
    const char * reqStr;
    APP_StatusRequest req;

    if (args)
    {
        for (reqStr = strtok(args, " \t\n"); reqStr; reqStr = strtok(NULL, " \t\n"))
        {
            req = (APP_StatusRequest)UTILS_ParseTableAlias(statusRequestAliases, reqStr);

            switch (req)
            {
                case APP_StatusRequest_eArgs:
                    ARGS_Print(&app->args);
                    break;
                case APP_StatusRequest_eNexus:
                    APP_PrintNexus(app);
                    break;
                case APP_StatusRequest_eStreamInfo:
                    APP_PrintStreamInfo(app);
                    break;
                case APP_StatusRequest_eDrmInfoFrame:
                    APP_PrintDrmInfoFrame(app);
                    break;
                case APP_StatusRequest_eEdid:
                    APP_PrintEdid(app);
                    break;
                case APP_StatusRequest_eOsd:
                    OSD_Print(app->osd);
                    break;
                case APP_StatusRequest_eSmd:
                    SMD_Print(app->smd);
                    break;
                case APP_StatusRequest_eShell:
                    SHELL_Print(app->shell);
                    break;
                default:
                    fprintf(stderr, "Unrecognized argument '%s' ignored\n", reqStr);
                    break;
            }
        }
    }
    else
    {
        APP_PrintEotf(app);
    }

    return rc;
}

int APP_SetupStatusCommand(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    command = SHELL_CreateCommand(app->shell,
        "status",
        "prints status of stream, stb, and TV on the console",
        "[args|nexus|stream|eotf|drm|edid|osd|smd|shell]",
        &APP_DoStatus,
        app);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = SHELL_AddCommandAlias(command, "st");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandAlias(command, "s");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "args", false, "prints arguments supplied to the program");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "nexus", false, "prints nexus status");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "stream", false, "prints stream info");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "eotf", false, "prints eotf status");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "drm", false, "prints DRMInfoFrame status");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "edid", false, "prints sink EDID status");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "osd", false, "prints OSD status");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "smd", false, "prints static metadata status");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "shell", false, "prints shell status");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

static const UTILS_StringIntMapEntry trickModeAliases[] =
{
    { "pause", APP_TrickMode_ePause },
    { "p", APP_TrickMode_ePause },
    { "resume", APP_TrickMode_eResume },
    { "r", APP_TrickMode_eResume },
    { NULL, APP_TrickMode_eMax }
};

static const UTILS_StringIntMapEntry trickModeNames[] =
{
    { "Paused", APP_TrickMode_ePause },
    { "Playing", APP_TrickMode_eResume },
    { NULL, APP_TrickMode_eMax }
};

const char * APP_GetTrickModeName(APP_TrickMode mode)
{
    return UTILS_GetTableName(trickModeNames, mode);
}

void APP_PrintTrickMode(APP_AppHandle app)
{
    const char * trickModeName = APP_GetTrickModeName(app->trickMode);
    fprintf(stdout, "# Trick Mode\n");
    fprintf(stdout, "playback = %s\n", trickModeName);
}

int APP_DoTrick(void * context, char * args)
{
    APP_AppHandle app = context;
    NEXUS_Error rc = NEXUS_SUCCESS;
    const char * modeStr;
    APP_TrickMode mode;

    if (args)
    {
        for (modeStr = strtok(args, " \t\n"); modeStr; modeStr = strtok(NULL, " \t\n"))
        {
            mode = (APP_TrickMode)UTILS_ParseTableAlias(trickModeAliases, modeStr);
            switch (mode)
            {
                case APP_TrickMode_ePause:
                    rc = NEXUS_Playback_Pause(app->playback);
                    if (rc) { rc = BERR_TRACE(rc); goto end; }
                    break;
                case APP_TrickMode_eResume:
                    rc = NEXUS_Playback_Play(app->playback);
                    if (rc) { rc = BERR_TRACE(rc); goto end; }
                    break;
                default:
                    fprintf(stderr, "Unrecognized argument '%s' ignored\n", modeStr);
                    break;
            }
        }

        app->trickMode = mode;
    }
    else
    {
        APP_PrintTrickMode(app);
    }

end:
    return rc;
}

int APP_SetupTrickCommand(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    command = SHELL_CreateCommand(app->shell,
        "trick",
        "modifies the trick state of the playback channel feeding the stream",
        "[pause|resume]",
        &APP_DoTrick,
        app);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = SHELL_AddCommandAlias(command, "tr");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandAlias(command, "t");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "pause", false, "pauses the playback of the stream");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "resume", false, "resumes the playback of the stream");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

static const UTILS_StringIntMapEntry mdcvRequestAliases[] =
{
    { "file", APP_MdcvRequest_eFile },
    { "f", APP_MdcvRequest_eFile },
    { "input", APP_MdcvRequest_eInput },
    { "in", APP_MdcvRequest_eInput },
    { "i", APP_MdcvRequest_eInput },
    { "bt2020", APP_MdcvRequest_eBt2020 },
    { "2020", APP_MdcvRequest_eBt2020 },
    { "bt709", APP_MdcvRequest_eBt709 },
    { "709", APP_MdcvRequest_eBt709 },
    { "primaries", APP_MdcvRequest_ePrimaries },
    { "prim", APP_MdcvRequest_ePrimaries },
    { "p", APP_MdcvRequest_ePrimaries },
    { "red", APP_MdcvRequest_eRed },
    { "r", APP_MdcvRequest_eRed },
    { "green", APP_MdcvRequest_eGreen },
    { "g", APP_MdcvRequest_eGreen },
    { "blue", APP_MdcvRequest_eBlue },
    { "b", APP_MdcvRequest_eBlue },
    { "white", APP_MdcvRequest_eWhite },
    { "w", APP_MdcvRequest_eWhite },
    { "max", APP_MdcvRequest_eMaxLuma },
    { "lx", APP_MdcvRequest_eMaxLuma },
    { "min", APP_MdcvRequest_eMinLuma },
    { "lm", APP_MdcvRequest_eMinLuma },
    { NULL, APP_TrickMode_eMax }
};

void APP_PrintMdcv(APP_AppHandle app)
{
    BSTD_UNUSED(app);
    fprintf(stdout, "# MDCV\n");
}

int APP_ApplyUserMetadata(APP_AppHandle app, const NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata)
{
    int rc = 0;

    rc = SMD_SetUserMetadata(app->smd, pMetadata);
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    if (SMD_GetSmdSource(app->smd) == SMD_SmdSource_eUser)
    {
        rc = SMD_GetMetadata(app->smd, &app->drmInfoFrame.metadata);
        if (rc) { BERR_TRACE(rc); goto end; }

        rc = APP_ApplyDrmInfoFrame(app);
        if (rc) { BERR_TRACE(rc); goto end; }
    }

end:
    return rc;
}

int APP_CopyToUserMetadata(APP_AppHandle app, const NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pFrom)
{
    int rc = 0;
    NEXUS_HdmiDynamicRangeMasteringStaticMetadata to;

    SMD_GetUserMetadata(app->smd, &to);
    if (memcmp(&to.typeSettings.type1.masteringDisplayColorVolume, &pFrom->typeSettings.type1.masteringDisplayColorVolume, sizeof(NEXUS_MasteringDisplayColorVolume)))
    {
        memcpy(&to.typeSettings.type1.masteringDisplayColorVolume, &pFrom->typeSettings.type1.masteringDisplayColorVolume, sizeof(NEXUS_MasteringDisplayColorVolume));
        rc = APP_ApplyUserMetadata(app, &to);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
    }

end:
    return rc;
}

int APP_DoMdcv(void * context, char * args)
{
    APP_AppHandle app = context;
    NEXUS_Error rc = NEXUS_SUCCESS;
    const char * mdcvStr;
    APP_MdcvRequest mdcv;
    NEXUS_HdmiDynamicRangeMasteringStaticMetadata metadata;

    if (args)
    {
        mdcvStr = strtok(args, " \t\n");
        if (mdcvStr)
        {
            mdcv = (APP_MdcvRequest)UTILS_ParseTableAlias(mdcvRequestAliases, mdcvStr);
            if (mdcv < APP_MdcvRequest_ePrimaries)
            {
                switch (mdcv)
                {
                    case APP_MdcvRequest_eFile:
                        SMD_GetFileMetadata(app->smd, &metadata);
                        break;
                    case APP_MdcvRequest_eInput:
                        SMD_GetInputMetadata(app->smd, &metadata);
                        break;
                    case APP_MdcvRequest_eBt2020:
                        SMD_GetBt2020Metadata(app->smd, &metadata);
                        break;
                    case APP_MdcvRequest_eBt709:
                        SMD_GetBt709Metadata(app->smd, &metadata);
                        break;
                    default:
                        break;
                }
                rc = APP_CopyToUserMetadata(app, &metadata);
                if (rc) { rc = BERR_TRACE(rc); goto end; }
            }
            else
            {
                static const char * colorNames[] =
                {
                    "red",
                    "green",
                    "blue",
                    "white",
                    NULL
                };

                SMD_GetUserMetadata(app->smd, &metadata);

                if (mdcv >= APP_MdcvRequest_eRed && mdcv <= APP_MdcvRequest_eWhite)
                {
                    const char * cName = NULL;
                    NEXUS_Point * pOldPoint = NULL;
                    NEXUS_Point newPoint = { 0, 0 };

                    switch (mdcv)
                    {
                        case APP_MdcvRequest_eRed:
                            cName = colorNames[0];
                            pOldPoint = &metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary;
                            break;
                        case APP_MdcvRequest_eGreen:
                            cName = colorNames[1];
                            pOldPoint = &metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary;
                            break;
                        case APP_MdcvRequest_eBlue:
                            cName = colorNames[2];
                            pOldPoint = &metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary;
                            break;
                        case APP_MdcvRequest_eWhite:
                            cName = colorNames[3];
                            pOldPoint = &metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint;
                            break;
                        default:
                            break;
                    }

                    /* parse new x, y */
                    mdcvStr = strtok(NULL, " \t\n");
                    if (mdcvStr)
                    {
                        newPoint.x = atoi(mdcvStr);
                        mdcvStr = strtok(NULL, " \t\n");
                        if (mdcvStr)
                        {
                            newPoint.y = atoi(mdcvStr);
                        }
                        else
                        {
                            if (pOldPoint)
                            {
                                newPoint.y = pOldPoint->y;
                            }
                            else
                            {
                                newPoint.y = 0;
                            }
                        }
                        if (pOldPoint && memcmp(&newPoint, pOldPoint, sizeof(NEXUS_Point)))
                        {
                            memcpy(pOldPoint, &newPoint, sizeof(NEXUS_Point));
                            rc = APP_ApplyUserMetadata(app, &metadata);
                            if (rc) { rc = BERR_TRACE(rc); goto end; }
                        }
                    }

                    /* print new or old point info */
                    if (cName && pOldPoint)
                    {
                        fprintf(stdout, "%s = ( %u, %u )\n", cName, pOldPoint->x, pOldPoint->y);
                    }
                }
                else if (mdcv == APP_MdcvRequest_eMaxLuma || mdcv == APP_MdcvRequest_eMinLuma)
                {
                    static const char * valNames[] =
                    {
                        "max",
                        "min",
                        NULL
                    };
                    const char * vName = NULL;
                    unsigned newValue = 0;
                    unsigned * pOldValue = NULL;

                    switch (mdcv)
                    {
                        case APP_MdcvRequest_eMaxLuma:
                            vName = valNames[0];
                            pOldValue = &metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.max;
                            break;
                        case APP_MdcvRequest_eMinLuma:
                            vName = valNames[1];
                            pOldValue = &metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.min;
                            break;
                        default:
                            break;
                    }

                    /* parse new value */
                    mdcvStr = strtok(NULL, " \t\n");
                    if (mdcvStr)
                    {
                        newValue = atoi(mdcvStr);
                        if (pOldValue && newValue != *pOldValue)
                        {
                            *pOldValue = newValue;
                            rc = APP_ApplyUserMetadata(app, &metadata);
                            if (rc) { rc = BERR_TRACE(rc); goto end; }
                        }
                    }

                    /* print new or old value */
                    if (vName && pOldValue)
                    {
                        fprintf(stdout, "%s luma = %u\n", vName, *pOldValue);
                    }
                }
                else
                {
                    unsigned i = 0;
                    NEXUS_Point * oldPrimaries[3] = { NULL, NULL, NULL };
                    NEXUS_Point newPrimaries[3];

                    oldPrimaries[0] = &metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary;
                    oldPrimaries[1] = &metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary;
                    oldPrimaries[2] = &metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary;
                    memset(&newPrimaries, 0, 3 * sizeof(NEXUS_Point));

                    /* parse all 3 primaries */
                    for (i = 0; i < 3; i++)
                    {
                        /* parse new x, y */
                        mdcvStr = strtok(NULL, " \t\n");
                        if (mdcvStr)
                        {
                            newPrimaries[i].x = atoi(mdcvStr);
                            mdcvStr = strtok(NULL, " \t\n");
                            if (mdcvStr)
                            {
                                newPrimaries[i].y = atoi(mdcvStr);
                            }
                            else
                            {
                                if (oldPrimaries[i])
                                {
                                    newPrimaries[i].y = oldPrimaries[i]->y;
                                }
                                else
                                {
                                    newPrimaries[i].y = 0;
                                }
                            }
                        }
                    }

                    /* compare new and old and apply if different */
                    if (memcmp(&newPrimaries[0], oldPrimaries[0], sizeof(NEXUS_Point)) ||
                        memcmp(&newPrimaries[1], oldPrimaries[1], sizeof(NEXUS_Point)) ||
                        memcmp(&newPrimaries[2], oldPrimaries[2], sizeof(NEXUS_Point)))
                    {
                        for (i = 0; i < 3; i++)
                        {
                            memcpy(oldPrimaries[i], &newPrimaries[i], sizeof(NEXUS_Point));
                        }
                        rc = APP_ApplyUserMetadata(app, &metadata);
                        if (rc) { rc = BERR_TRACE(rc); goto end; }
                    }

                    /* print new or old point info */
                    for (i = 0; i < 3; i++)
                    {
                        fprintf(stdout, "%s = ( %u, %u ) ", colorNames[i], oldPrimaries[i]->x, oldPrimaries[i]->y);
                    }
                    fprintf(stdout, "\n");
                }
            }
        }
    }
    else
    {
        APP_PrintMdcv(app);
    }

end:
    return rc;
}

int APP_SetupMdcvCommand(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    command = SHELL_CreateCommand(app->shell,
        "mdcv",
        "modifies the user version of the mastering display color volume static metadata",
        "[file|input|primaries|red|green|blue|white|max|min|bt2020|bt709]",
        &APP_DoMdcv,
        app);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = SHELL_AddCommandAlias(command, "md");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandAlias(command, "m");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "file", false, "sets the chromaticity coordinates and white point to BT2020 values");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "input", false, "sets the chromaticity coordinates and white point to BT2020 values");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "bt2020", false, "sets the chromaticity coordinates and white point to BT2020 values");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "b709", false, "sets the chromaticity coordinates and white point to BT709 values");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "primaries", false, "sets the chromaticity coordinates for the color primaries in order of GBR");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "red", false, "sets the red chromaticity coordinate");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "green", false, "sets the green chromaticity coordinate");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "blue", false, "sets the blue chromaticity coordinate");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "white", false, "sets the white point");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "max", false, "sets the max mastering display luma");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "min", false, "sets the min mastering display luma");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

void APP_PrintCll(APP_AppHandle app)
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadata metadata;
    SMD_GetUserMetadata(app->smd, &metadata);
    fprintf(stdout, "# CLL\n");
    fprintf(stdout, "cll = %u\n", metadata.typeSettings.type1.contentLightLevel.max);
}

int APP_DoCll(void * context, char * args)
{
    APP_AppHandle app = context;
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned cll;

    if (args)
    {
        NEXUS_HdmiDynamicRangeMasteringStaticMetadata metadata;
        SMD_GetUserMetadata(app->smd, &metadata);
        cll = atoi(args);
        if (cll != metadata.typeSettings.type1.contentLightLevel.max)
        {
            metadata.typeSettings.type1.contentLightLevel.max = cll;
            rc = APP_ApplyUserMetadata(app, &metadata);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
        }
    }
    else
    {
        APP_PrintCll(app);
    }

end:
    return rc;
}

int APP_SetupCllCommand(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    command = SHELL_CreateCommand(app->shell,
        "cll",
        "modifies the user version of the content light level static metadata",
        "###",
        &APP_DoCll,
        app);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = SHELL_AddCommandAlias(command, "l");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "###", false, "sets the cll max value to the supplied number");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

void APP_PrintFal(APP_AppHandle app)
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadata metadata;
    SMD_GetUserMetadata(app->smd, &metadata);
    fprintf(stdout, "# FAL\n");
    fprintf(stdout, "fal = %u\n", metadata.typeSettings.type1.contentLightLevel.maxFrameAverage);
}

int APP_DoFal(void * context, char * args)
{
    APP_AppHandle app = context;
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned fal;

    if (args)
    {
        NEXUS_HdmiDynamicRangeMasteringStaticMetadata metadata;
        SMD_GetUserMetadata(app->smd, &metadata);
        fal = atoi(args);
        if (fal != metadata.typeSettings.type1.contentLightLevel.maxFrameAverage)
        {
            metadata.typeSettings.type1.contentLightLevel.maxFrameAverage = fal;
            rc = APP_ApplyUserMetadata(app, &metadata);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
        }
    }
    else
    {
        APP_PrintFal(app);
    }

end:
    return rc;
}

int APP_SetupFalCommand(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    command = SHELL_CreateCommand(app->shell,
        "fal",
        "modifies the user version of the max frame average light level static metadata",
        "###",
        &APP_DoFal,
        app);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = SHELL_AddCommandAlias(command, "f");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "###", false, "sets the max frame average light level value to the supplied number");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

int APP_SetupShell(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    rc = APP_SetupEotfCommand(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = APP_SetupMatrixCoefficientsCommand(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = APP_SetupGfxSdr2HdrCommand(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = APP_SetupSmdCommand(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = APP_SetupStatusCommand(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = APP_SetupOsdCommand(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = APP_SetupTrickCommand(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = APP_SetupMdcvCommand(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = APP_SetupCllCommand(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    rc = APP_SetupFalCommand(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

void APP_Destroy(APP_AppHandle app)
{
    BSTD_UNUSED(app);
}

APP_AppHandle APP_Create(const ARGS_Args * args)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_VideoDecoderOpenSettings videoOpenSettings;
    NEXUS_VideoDecoderSettings videoSettings;
    NEXUS_AudioDecoderOpenSettings audioOpenSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_GraphicsSettings graphicsCompositorSettings;
    NEXUS_HdmiOutputSettings hdmiSettings;
    APP_AppHandle app = NULL;
    OSD_OsdSettings osdSettings;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.transportModuleSettings.maxDataRate.playback[0] = 125000000;
    NEXUS_Platform_Init(&platformSettings);

    app = BKNI_Malloc(sizeof(struct APP_App));
    if (!app) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }

    BKNI_Memset(app, 0, sizeof(struct APP_App));
    app->lastStreamInfo.eotf = NEXUS_VideoEotf_eMax;
    BKNI_Memcpy(&app->args, args, sizeof(ARGS_Args));

    NEXUS_Platform_GetConfiguration(&platformConfig);

    app->hdmi = platformConfig.outputs.hdmi[0];
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    app->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    if (!app->playpump) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    app->playback = NEXUS_Playback_Create();
    if (!app->playback) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    app->stc = NEXUS_StcChannel_Open(NEXUS_ANY_ID, NULL);

    NEXUS_Playback_GetSettings(app->playback, &playbackSettings);
    playbackSettings.playpump = app->playpump;
    playbackSettings.playpumpSettings.transportType = app->args.transportType;
    playbackSettings.stcChannel = app->stc;
    rc = NEXUS_Playback_SetSettings(app->playback, &playbackSettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    app->input = NEXUS_FilePlay_OpenPosix(app->args.inputFilename, app->args.inputFilename);
    if (!app->input) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoOpenSettings);
    videoOpenSettings.fifoSize = 8 * 1024 * 1024;
    app->videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    if (!app->videoDecoder) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);
    audioOpenSettings.fifoSize = 1 * 1024 * 1024;
    app->audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    if (!app->audioDecoder) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); /* ignore */ }

    if (app->audioDecoder)
    {
        rc = NEXUS_AudioOutput_AddInput(
            NEXUS_HdmiOutput_GetAudioConnector(app->hdmi),
            NEXUS_AudioDecoder_GetConnector(app->audioDecoder, NEXUS_AudioConnectorType_eStereo));
        if (rc) { rc = BERR_TRACE(rc); /* ignore */ }
    }

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = app->args.format;
    app->display = NEXUS_Display_Open(0, &displaySettings);
    if (!app->display) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    NEXUS_Display_AddOutput(app->display, NEXUS_HdmiOutput_GetVideoConnector(app->hdmi));

    NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = APP_HdmiHotplugCallback;
    hdmiSettings.hotplugCallback.context = app;
    rc = NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
    if (rc) { rc = BERR_TRACE(rc); /* continue */ }

    app->smd = SMD_Create();
    if (!app->smd) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    if (app->args.smdFilename)
    {
        SMD_SetFilePath(app->smd, app->args.smdFilename);
        rc = SMD_SetSmdSource(app->smd, SMD_SmdSource_eFile);
        if (rc)
        {
            rc = BERR_TRACE(rc);
            rc = SMD_SetSmdSource(app->smd, SMD_SmdSource_eInput);
            if (rc) { rc = BERR_TRACE(rc); /* continue */ }
        }
    }
    else
    {
        rc = SMD_SetSmdSource(app->smd, SMD_SmdSource_eInput);
        if (rc) { rc = BERR_TRACE(rc); /* continue */ }
    }

    rc = SMD_GetMetadata(app->smd, &app->drmInfoFrame.metadata);
    if (rc) { rc = BERR_TRACE(rc); /* continue */ }

    rc = APP_SetEotf(app, (APP_Eotf) app->args.eotf);
    if (rc) { rc = BERR_TRACE(rc); /* continue */ }

    app->window = NEXUS_VideoWindow_Open(app->display, 0);
    if (!app->window) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    /* OSD is 1080p */
    NEXUS_VideoFormat_GetInfo(NEXUS_VideoFormat_e1080p, &videoFormatInfo);

    app->osd = OSD_Create(videoFormatInfo.width, videoFormatInfo.height);
    if (!app->osd) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    OSD_GetSettings(app->osd, &osdSettings);
    osdSettings.mode = app->args.osdMode;
    rc = OSD_SetSettings(app->osd, &osdSettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    NEXUS_Display_GetGraphicsSettings(app->display, &graphicsCompositorSettings);
    /* graphicsCompositorSettings.position will default to the display size */
    graphicsCompositorSettings.clip.width = videoFormatInfo.width;
    graphicsCompositorSettings.clip.height = videoFormatInfo.height;
    graphicsCompositorSettings.sdrToHdr.y = app->args.gfxSdr2Hdr.y;
    graphicsCompositorSettings.sdrToHdr.cb = app->args.gfxSdr2Hdr.cb;
    graphicsCompositorSettings.sdrToHdr.cr = app->args.gfxSdr2Hdr.cr;
    rc = NEXUS_Display_SetGraphicsSettings(app->display, &graphicsCompositorSettings);
    if (rc) { rc = BERR_TRACE(rc); /* continue */ }

    NEXUS_VideoDecoder_GetDefaultStartSettings(&app->videoStartSettings);
    app->videoStartSettings.codec = app->args.videoCodec;
    app->videoStartSettings.defaultTransferCharacteristics = NEXUS_TransferCharacteristics_eUnknown;
    app->videoStartSettings.stcChannel = app->stc;
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = app->args.videoCodec; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = app->videoDecoder;
    app->videoStartSettings.pidChannel = NEXUS_Playback_OpenPidChannel(app->playback, app->args.videoPid, &playbackPidSettings);
    if (!app->videoStartSettings.pidChannel) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }
    app->videoStartSettings.frameRate = NEXUS_VideoFrameRate_e30;

    NEXUS_VideoDecoder_GetSettings(app->videoDecoder, &videoSettings);
    videoSettings.streamChanged.callback = &APP_StreamChangedCallback;
    videoSettings.streamChanged.context = app;
    videoSettings.maxWidth = 3840;
    videoSettings.maxHeight = 2160;
    rc = NEXUS_VideoDecoder_SetSettings(app->videoDecoder, &videoSettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    rc = NEXUS_VideoWindow_AddInput(app->window, NEXUS_VideoDecoder_GetConnector(app->videoDecoder));
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    if (app->audioDecoder && app->args.audioPid && app->args.audioCodec != NEXUS_AudioCodec_eMax)
    {
        NEXUS_AudioDecoder_GetDefaultStartSettings(&app->audioStartSettings);
        app->audioStartSettings.codec = app->args.audioCodec;
        app->audioStartSettings.stcChannel = app->stc;
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.primary = app->audioDecoder;
        app->audioStartSettings.pidChannel = NEXUS_Playback_OpenPidChannel(app->playback, app->args.audioPid, &playbackPidSettings);
        if (!app->audioStartSettings.pidChannel) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }
    }

    app->shell = SHELL_Create();
    if (!app->shell) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    rc = APP_SetupShell(app);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    APP_HdmiHotplugCallback(app, 0);

    rc = NEXUS_VideoDecoder_Start(app->videoDecoder, &app->videoStartSettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    if (app->audioDecoder && app->args.audioPid && app->args.audioCodec != NEXUS_AudioCodec_eMax)
    {
        rc = NEXUS_AudioDecoder_Start(app->audioDecoder, &app->audioStartSettings);
        if (rc) { rc = BERR_TRACE(rc); /* ignore */ }
    }

    rc = NEXUS_Playback_Start(app->playback, app->input, NULL);
    if (rc) { rc = BERR_TRACE(rc); goto error; }


    return app;

error:
    if (app)
    {
        APP_Destroy(app);
    }
    return NULL;
}

int APP_Run(APP_AppHandle app)
{
    return SHELL_Run(app->shell);
}
