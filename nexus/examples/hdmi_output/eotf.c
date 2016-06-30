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
/* Nexus example app: single playback 4k hevc video decode routed to HDMI with HDR signaling */

#include "nexus_platform.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#include "nexus_video_decoder.h"
#include "nexus_playback.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(eotf);

typedef struct Args
{
    unsigned videoPid;
    const char * inputFilename;
    NEXUS_VideoCodec videoCodec;
    NEXUS_TransportType transportType;
} Args;

typedef struct App
{
    Args args;
    NEXUS_FilePlayHandle input;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoStartSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_HdmiOutputHandle hdmi;
    NEXUS_HdmiOutputEdidData sinkEdid;
    NEXUS_VideoDecoderStreamInformation lastStreamInfo;
    bool lastStreamInfoValid;
    NEXUS_SurfaceHandle framebuffer;
    BKNI_MutexHandle lock;
} App;

static const char * const eotfStrings[NEXUS_VideoEotf_eMax + 1] =
{
    "SDR (GAMMA)",
    "HDR (GAMMA)",
    "SMPTE 2084 (PQ)",
    "Future (BBC/NHK)",
    "unspecified"
};

static void printEdid(App * app)
{
    if (app->sinkEdid.hdrdb.valid)
    {
        unsigned i;
        for (i = 0; i < NEXUS_VideoEotf_eMax; i++)
        {
            BDBG_MSG(("SINK EDID EOTF %s %s", eotfStrings[i], app->sinkEdid.hdrdb.eotfSupported[i] ? "supported" : "not supported"));
        }
    }
    else
    {
        BDBG_MSG(("SINK EDID has no HDR support"));
    }
}

static void setHdmiOut(App *app, NEXUS_VideoDecoderStreamInformation *streamInfo, bool force)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiOutputSettings hdmiSettings;
    bool apply = false;

    NEXUS_HdmiOutput_GetSettings(app->hdmi, &hdmiSettings);

    hdmiSettings.dynamicRangeMasteringInfoFrame.metadata.type = NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1;
    if (force || !app->lastStreamInfoValid || app->lastStreamInfo.eotf != streamInfo->eotf)
    {
        fprintf(stdout, "input EOTF changed %s -> %s\n", eotfStrings[app->lastStreamInfo.eotf], eotfStrings[streamInfo->eotf]);
        if (!app->sinkEdid.hdrdb.valid)
        {
            fprintf(stdout, "TV does not support HDR, no DRMInfoFrame sent. Display HDR video as SDR\n");
            hdmiSettings.dynamicRangeMasteringInfoFrame.eotf = NEXUS_VideoEotf_eSdr;
        }
        else
        {
            hdmiSettings.dynamicRangeMasteringInfoFrame.eotf = streamInfo->eotf;
        }
        apply = true;
    }
    if (force || !app->lastStreamInfoValid || BKNI_Memcmp(&app->lastStreamInfo.contentLightLevel, &streamInfo->contentLightLevel, sizeof(streamInfo->contentLightLevel)))
    {
        fprintf(stdout, "input content light level changed: (%u, %u) -> (%u, %u)\n",
                app->lastStreamInfo.contentLightLevel.max,
                app->lastStreamInfo.contentLightLevel.maxFrameAverage,
                streamInfo->contentLightLevel.max,
                streamInfo->contentLightLevel.maxFrameAverage);
        BKNI_Memcpy(&hdmiSettings.dynamicRangeMasteringInfoFrame.metadata.typeSettings.type1.contentLightLevel,
                    &streamInfo->contentLightLevel,
                    sizeof(streamInfo->contentLightLevel));
        apply = true;
    }
    if (force || !app->lastStreamInfoValid || BKNI_Memcmp(&app->lastStreamInfo.masteringDisplayColorVolume, &streamInfo->masteringDisplayColorVolume, sizeof(streamInfo->masteringDisplayColorVolume)))
    {
        fprintf(stdout, "input mastering display color volume changed:\n");
        fprintf(stdout, "\tred (%u, %u) -> (%u, %u)\n",
                app->lastStreamInfo.masteringDisplayColorVolume.redPrimary.x,
                app->lastStreamInfo.masteringDisplayColorVolume.redPrimary.y,
                streamInfo->masteringDisplayColorVolume.redPrimary.x,
                streamInfo->masteringDisplayColorVolume.redPrimary.y);
        fprintf(stdout, "\tgreen (%u, %u) -> (%u, %u)\n",
                app->lastStreamInfo.masteringDisplayColorVolume.greenPrimary.x,
                app->lastStreamInfo.masteringDisplayColorVolume.greenPrimary.y,
                streamInfo->masteringDisplayColorVolume.greenPrimary.x,
                streamInfo->masteringDisplayColorVolume.greenPrimary.y);
        fprintf(stdout, "\tblue (%u, %u) -> (%u, %u)\n",
                app->lastStreamInfo.masteringDisplayColorVolume.bluePrimary.x,
                app->lastStreamInfo.masteringDisplayColorVolume.bluePrimary.y,
                streamInfo->masteringDisplayColorVolume.bluePrimary.x,
                streamInfo->masteringDisplayColorVolume.bluePrimary.y);
        fprintf(stdout, "\twhite (%u, %u) -> (%u, %u)\n",
                app->lastStreamInfo.masteringDisplayColorVolume.whitePoint.x,
                app->lastStreamInfo.masteringDisplayColorVolume.whitePoint.y,
                streamInfo->masteringDisplayColorVolume.whitePoint.x,
                streamInfo->masteringDisplayColorVolume.whitePoint.y);
        fprintf(stdout, "\tluma (%u, %u) -> (%u, %u)\n",
                app->lastStreamInfo.masteringDisplayColorVolume.luminance.max,
                app->lastStreamInfo.masteringDisplayColorVolume.luminance.min,
                streamInfo->masteringDisplayColorVolume.luminance.max,
                streamInfo->masteringDisplayColorVolume.luminance.min);
        BKNI_Memcpy(&hdmiSettings.dynamicRangeMasteringInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume,
                    &streamInfo->masteringDisplayColorVolume,
                    sizeof(streamInfo->masteringDisplayColorVolume));
        apply = true;
    }
    if (apply)
    {
        rc = NEXUS_HdmiOutput_SetSettings(app->hdmi, &hdmiSettings);
        BDBG_ASSERT(!rc);
    }
}

static void streamChangedCallback(void * context, int param)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    App * app = context;
    NEXUS_VideoDecoderStreamInformation streamInfo;

    BSTD_UNUSED(param);

    BKNI_AcquireMutex(app->lock);
    rc = NEXUS_VideoDecoder_GetStreamInformation(app->videoDecoder, &streamInfo);
    if (!rc)
    {
        setHdmiOut(app, &streamInfo, false);
        app->lastStreamInfo = streamInfo;
        app->lastStreamInfoValid = true;
    }
    BKNI_ReleaseMutex(app->lock);
}

static void setGraphics(App *app)
{
    int i, j;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_SurfaceMemory mem;
    NEXUS_GraphicsSettings graphicsSettings;

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.width = 720;
    surfaceCreateSettings.height = 480;
    surfaceCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    app->framebuffer = NEXUS_Surface_Create(&surfaceCreateSettings);
    NEXUS_Surface_GetMemory(app->framebuffer, &mem);

    for (i=0;i<surfaceCreateSettings.height;i++) {
        for (j=0;j<surfaceCreateSettings.width;j++) {
            if (j>=20 && j<700) {
                int c = (((j-20) / 68) * 255) / 9;
                if (i>=400 && i<410) {
                    ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = (0xFF000000 | (c << 16));
                } else if (i>=410 && i<420) {
                    ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = (0xFF000000 | (c << 8));
                } else if (i>=420 && i<430) {
                    ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = (0xFF000000 | (c << 0));
                } else if (i>=430 && i<440) {
                    ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = (0xFF000000 | (c << 0) | (c << 8) | (c << 16));
                } else {
                    ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = 0;
                }
            } else {
                ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = 0;
            }
        }
    }
    NEXUS_Surface_Flush(app->framebuffer);

    NEXUS_Display_GetGraphicsSettings(app->display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.clip.width = surfaceCreateSettings.width;
    graphicsSettings.clip.height = surfaceCreateSettings.height;
    rc = NEXUS_Display_SetGraphicsSettings(app->display, &graphicsSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Display_SetGraphicsFramebuffer(app->display, app->framebuffer);
    BDBG_ASSERT(!rc);
}

static void setGfxSdrToHdr(App *app, short y, short cb, short cr)
{
    NEXUS_GraphicsSettings graphicsSettings;

    /* note: app might implement GUI for end user to adjust graphicsSettings.sdrToHdr because it is subjective
     * note: setting to graphicsSettings.sdrToHdr will ONLY take effect when display eotf is not sdr */
    NEXUS_Display_GetGraphicsSettings(app->display, &graphicsSettings);
    graphicsSettings.sdrToHdr.y = y;
    graphicsSettings.sdrToHdr.cb = cb;
    graphicsSettings.sdrToHdr.cr = cr;
    NEXUS_Display_SetGraphicsSettings(app->display, &graphicsSettings);
}

static void setDisplay(App *app, NEXUS_HdmiOutputStatus *hdmiStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DisplaySettings displaySettings;

    rc = NEXUS_HdmiOutput_GetEdidData(app->hdmi, &app->sinkEdid);
    BDBG_ASSERT(!rc);
    printEdid(app);

    /* If current display format is not supported by monitor, switch to monitor's preferred format.
       If other connected outputs do not support the preferred format, a harmless error will occur. */
    NEXUS_Display_GetSettings(app->display, &displaySettings);
    if ( !hdmiStatus->videoFormatSupported[displaySettings.format] ) {
        displaySettings.format = hdmiStatus->preferredVideoFormat;
        NEXUS_Display_SetSettings(app->display, &displaySettings);
    }

    setGfxSdrToHdr(app, 12288, 0, -16384);
}

/* registered HDMI hotplug handler -- changes the format (to monitor's default) if monitor doesn't support current format */
static void hotPlugCallback(void *pParam, int iParam)
{
    App *app = (App *) pParam;
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;

    BSTD_UNUSED(iParam);

    BKNI_AcquireMutex(app->lock);
    rc = NEXUS_HdmiOutput_GetStatus(app->hdmi, &hdmiStatus);
    fprintf(stdout, "HDMI hotplug: %s\n", hdmiStatus.connected? "connected" : "not connected");
    if ( !rc && hdmiStatus.connected )
    {
        setDisplay(app, &hdmiStatus);
        if (app->lastStreamInfoValid)
        {
            setHdmiOut(app, &app->lastStreamInfo, true);
        }
    }
    BKNI_ReleaseMutex(app->lock);
}

static bool sinkSupportsStreamEotf(App * app)
{
    bool support = false;

    if (app->lastStreamInfo.eotf < NEXUS_VideoEotf_eMax)
    {
        support = app->sinkEdid.hdrdb.eotfSupported[app->lastStreamInfo.eotf];
    }

    return support;
}

static void defaultArgs(Args * args)
{
    args->inputFilename = NULL;
    args->videoPid = 0x1001;
    args->videoCodec = NEXUS_VideoCodec_eH265;
    args->transportType = NEXUS_TransportType_eUnknown;
}

static void usage(void)
{
    fprintf(stdout, "usage: nexus eotf [options]\n");
    fprintf(stdout, "options:\n");
    fprintf(stdout, "  -v videoPid Required. Specifies the video pid to decode\n");
    fprintf(stdout, "  -f inputFilename Required. Specifies the file to play\n");
    fprintf(stdout, "  -t transportType Optional. Specifies the stream transport type. Defaults to MPEG-2 TS. Allowed values are: ts, mp4.\n");
    fprintf(stdout, "  -c videoCodec Optional. Specifies the video codec to decode.  Defaults to hevc.  Allowed values are: hevc, mpeg, avc.\n");
}

static const char * unknownString = "unknown";

typedef struct
{
    const char * name;
    NEXUS_VideoCodec codec;
} VideoCodecMapEntry;

static const VideoCodecMapEntry videoCodecMap[] =
{
    { "hevc", NEXUS_VideoCodec_eH265 },
    { "h265", NEXUS_VideoCodec_eH265 },
    { "mpeg2", NEXUS_VideoCodec_eMpeg2 },
    { "mpeg", NEXUS_VideoCodec_eMpeg2 },
    { "avc", NEXUS_VideoCodec_eH264 },
    { "h264", NEXUS_VideoCodec_eH264 },
    { NULL, NEXUS_VideoCodec_eUnknown },
};

static NEXUS_VideoCodec parseVideoCodec(const char * codecStr)
{
    const VideoCodecMapEntry * e;
    NEXUS_VideoCodec codec = NEXUS_VideoCodec_eUnknown;

    for (e = &videoCodecMap[0]; e->name; e++)
    {
        if (!strcmp(e->name, codecStr))
        {
            codec = e->codec;
            break;
        }
    }

    return codec;
}

static const char * getVideoCodecName(NEXUS_VideoCodec codec)
{
    const VideoCodecMapEntry * e;
    const char * name = unknownString;

    for (e = &videoCodecMap[0]; e->name; e++)
    {
        if (e->codec == codec)
        {
            name = e->name;
            break;
        }
    }

    return name;
}

typedef struct
{
    const char * name;
    NEXUS_TransportType type;
} TransportTypeMapEntry;

static const TransportTypeMapEntry transportTypeMap[] =
{
    { "ts", NEXUS_TransportType_eTs },
    { "mpeg2ts", NEXUS_TransportType_eTs },
    { "mp4", NEXUS_TransportType_eMp4 },
    { NULL, NEXUS_TransportType_eUnknown },
};

static NEXUS_TransportType parseTransportType(const char * typeStr)
{
    const TransportTypeMapEntry * e;
    NEXUS_TransportType type = NEXUS_TransportType_eUnknown;

    for (e = &transportTypeMap[0]; e->name; e++)
    {
        if (!strcmp(e->name, typeStr))
        {
            type = e->type;
            break;
        }
    }

    return type;
}

static const char * getTransportTypeName(NEXUS_TransportType type)
{
    const TransportTypeMapEntry * e;
    const char * name = unknownString;

    for (e = &transportTypeMap[0]; e->name; e++)
    {
        if (e->type == type)
        {
            name = e->name;
            break;
        }
    }

    return name;
}

static NEXUS_Error parseArgs(int argc, char * argv[], Args * args)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int i;

    if (argc == 1)
    {
        usage();
        rc = NEXUS_NOT_INITIALIZED;
        goto end;
    }

    for (i = 1; i < argc; i++)
    {
        if (!strncmp(argv[i], "-h", 2))
        {
            usage();
            rc = NEXUS_NOT_INITIALIZED;
            break;
        }
        else if (!strncmp(argv[i], "-f", 2))
        {
            if (++i < argc)
            {
                args->inputFilename = argv[i];
            }
            else
            {
                fprintf(stderr, "-f requires filename\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else if (!strncmp(argv[i], "-v", 2))
        {
            if (++i < argc)
            {
                args->videoPid = strtoul(argv[i], NULL, 0);
            }
            else
            {
                fprintf(stderr, "-v requires an integer argument (hex or dec)\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else if (!strncmp(argv[i], "-c", 2))
        {
            if (++i < argc)
            {
                args->videoCodec = parseVideoCodec(argv[i]);
            }
            else
            {
                fprintf(stderr, "-c requires a codec name\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else if (!strncmp(argv[i], "-t", 2))
        {
            if (++i < argc)
            {
                args->transportType = parseTransportType(argv[i]);
            }
            else
            {
                fprintf(stderr, "-t requires a type name\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else
        {
            fprintf(stdout, "Unrecognized option '%s' ignored.\n", argv[i]);
        }
    }

end:
    return rc;
}

static NEXUS_Error validateArgs(Args * args)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    FILE * in = NULL;

    if (!args->videoPid || args->videoPid > 0x1fff)
    {
        BDBG_ERR(("Invalid video pid specified: 0x%04x", args->videoPid));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    if ((args->videoCodec != NEXUS_VideoCodec_eH265)
        &&
        (args->videoCodec != NEXUS_VideoCodec_eMpeg2)
        &&
        (args->videoCodec != NEXUS_VideoCodec_eH264))
    {
        BDBG_ERR(("Invalid video codec specified: %s(%d)", getVideoCodecName(args->videoCodec), args->videoCodec));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    if (args->transportType == NEXUS_TransportType_eUnknown)
    {
        if (strstr(args->inputFilename, ".mp4") || strstr(args->inputFilename, ".m4v"))
        {
            args->transportType = NEXUS_TransportType_eMp4;
        }
        else
        {
            args->transportType = NEXUS_TransportType_eTs;
        }
    }

    if ((args->transportType != NEXUS_TransportType_eTs)
        &&
        (args->transportType != NEXUS_TransportType_eMp4))
    {
        BDBG_ERR(("Invalid transport type specified: %s(%d)", getTransportTypeName(args->transportType), args->transportType));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    in = fopen(args->inputFilename, "rb");
    if (!in)
    {
        BDBG_ERR(("Invalid file name specified: '%s'", args->inputFilename));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

end:
    if (in) fclose(in);
    return rc;
}

int main(int argc, char * argv[])
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_VideoDecoderSettings videoSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_HdmiOutputStatus hdmiStatus;
    App * app = NULL;
    Args args;

    defaultArgs(&args);
    rc = parseArgs(argc, argv, &args);
    if (rc) {
        if (rc != NEXUS_NOT_INITIALIZED)
        {
            rc = BERR_TRACE(rc);
        }
        goto end;
    }
    rc = validateArgs(&args);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
    memConfigSettings.videoDecoder[0].used = true; /* single decode */
    memConfigSettings.videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
    rc = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
    BDBG_ASSERT(!rc);

    app = BKNI_Malloc(sizeof(App));
    BDBG_ASSERT(app);
    BKNI_Memset(app, 0, sizeof(App));
    app->lastStreamInfo.eotf = NEXUS_VideoEotf_eMax;
    BKNI_Memcpy(&app->args, &args, sizeof(Args));

    BKNI_CreateMutex(&app->lock);

    NEXUS_Platform_GetConfiguration(&platformConfig);

    app->hdmi = platformConfig.outputs.hdmi[0];
    BDBG_ASSERT(app->hdmi);

    NEXUS_HdmiOutput_GetSettings(app->hdmi, &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = hotPlugCallback;
    hdmiSettings.hotplugCallback.context = app;
    NEXUS_HdmiOutput_SetSettings(app->hdmi, &hdmiSettings);

    app->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(app->playpump);

    app->playback = NEXUS_Playback_Create();

    NEXUS_Playback_GetSettings(app->playback, &playbackSettings);
    playbackSettings.playpump = app->playpump;
    playbackSettings.playpumpSettings.transportType = app->args.transportType;
    rc = NEXUS_Playback_SetSettings(app->playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    app->input = NEXUS_FilePlay_OpenPosix(app->args.inputFilename, app->args.transportType == NEXUS_TransportType_eTs ? NULL : app->args.inputFilename);
    BDBG_ASSERT(app->input);

    app->videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    BDBG_ASSERT(app->videoDecoder);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e3840x2160p30hz;
    app->display = NEXUS_Display_Open(0, &displaySettings);
    BDBG_ASSERT(app->display);

    NEXUS_Display_AddOutput(app->display, NEXUS_HdmiOutput_GetVideoConnector(app->hdmi));
    rc = NEXUS_HdmiOutput_GetStatus(app->hdmi, &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        setDisplay(app, &hdmiStatus);
    }

    setGraphics(app);

    app->window = NEXUS_VideoWindow_Open(app->display, 0);
    BDBG_ASSERT(app->window);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&app->videoStartSettings);
    app->videoStartSettings.codec = app->args.videoCodec;
    app->videoStartSettings.defaultTransferCharacteristics = NEXUS_TransferCharacteristics_eUnknown;
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eH265; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = false;
    playbackPidSettings.pidTypeSettings.video.decoder = app->videoDecoder;
    app->videoStartSettings.pidChannel = NEXUS_Playback_OpenPidChannel(app->playback, app->args.videoPid, &playbackPidSettings);
    app->videoStartSettings.frameRate = NEXUS_VideoFrameRate_e30;

    NEXUS_VideoDecoder_GetSettings(app->videoDecoder, &videoSettings);
    videoSettings.streamChanged.callback = &streamChangedCallback;
    videoSettings.streamChanged.context = app;
    videoSettings.maxWidth = 3840;
    videoSettings.maxHeight = 2160;
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoDecoder_SetSettings(app->videoDecoder, &videoSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_VideoWindow_AddInput(app->window, NEXUS_VideoDecoder_GetConnector(app->videoDecoder));
    BDBG_ASSERT(!rc);

    rc = NEXUS_VideoDecoder_Start(app->videoDecoder, &app->videoStartSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Playback_Start(app->playback, app->input, NULL);
    BDBG_ASSERT(!rc);

    BKNI_Sleep(2*1000);
    while (true)
    {
        char cmd;
        int y = 0, cb = 0, cr = 0;

        fprintf(stdout, "\nEnter cmd: setGfxSdrToHdr(s), checkEotf(c),  quit(q)\n");
        if (scanf("%s", &cmd) > 0) {
            if (cmd == 'q')
                break;
            else if (cmd == 's') {
                fprintf(stdout, "Enter y cb cr (-32768, 32767):\n");
                scanf("%d %d %d", &y, &cb, &cr);
                BKNI_AcquireMutex(app->lock);
                setGfxSdrToHdr(app, y, cb, cr);
                BKNI_ReleaseMutex(app->lock);
            }
            else {
                if (!app->lastStreamInfoValid)
                    fprintf(stdout, "No stream yet\n");
                else if (app->lastStreamInfo.eotf == NEXUS_VideoEotf_eSdr)
                    fprintf(stdout, "Stream reports SDR EOTF\n");
                else if (!sinkSupportsStreamEotf(app))
                    fprintf(stdout, "Stream reports %s EOTF.  Sink claims no support.  Video may be wonky.\n", eotfStrings[app->lastStreamInfo.eotf]);
                else
                    fprintf(stdout, "Stream reports %s EOTF. Sink claims supported.\n", eotfStrings[app->lastStreamInfo.eotf]);
            }
        }
    }

    NEXUS_Playback_Stop(app->playback);
    NEXUS_VideoDecoder_Stop(app->videoDecoder);

    NEXUS_Playback_CloseAllPidChannels(app->playback);
    NEXUS_FilePlay_Close(app->input);
    NEXUS_Playback_Destroy(app->playback);
    NEXUS_Playpump_Close(app->playpump);

    NEXUS_VideoWindow_Close(app->window);
    NEXUS_Display_Close(app->display);

    NEXUS_VideoDecoder_Close(app->videoDecoder);

    NEXUS_Surface_Destroy(app->framebuffer);

    NEXUS_Platform_Uninit();

    BKNI_DestroyMutex(app->lock);
end:
    if (app) BKNI_Free(app);
    return rc;
}

#else /* !NEXUS_NUM_HDMI_OUTPUTS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstd.h"
int main(int argc, char **argv)
{
    BSTD_UNUSED(argc) ;
    fprintf(stderr, "%s not supported on this platform\n", argv[0]);
    return 0;
}
#endif /* NEXUS_NUM_HDMI_OUTPUTS */
