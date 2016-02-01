/******************************************************************************
 *    (c)2008-2015 Broadcom Corporation
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
******************************************************************************/

#include "nexus_platform.h"
#include "dynrng_args.h"
#include "dynrng_args_priv.h"
#include "dynrng_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void ARGS_GetDefault(ARGS_Args * args)
{
    memset(args, 0, sizeof(ARGS_Args));
    args->inputFilename = NULL;
    args->videoPid = 0x1001;
    args->audioPid = 0x1002;
    args->videoCodec = NEXUS_VideoCodec_eH265;
    args->audioCodec = NEXUS_AudioCodec_eAac;
    args->transportType = NEXUS_TransportType_eTs;
    args->colorSpace = NEXUS_ColorSpace_eAuto;
    args->colorDepth = 0;
    args->format = NEXUS_VideoFormat_e3840x2160p30hz;
    args->smdFilename = NULL;
    args->osdMode = OSD_OsdMode_eOff;
}

void ARGS_Usage(const char * invocation)
{
    fprintf(stdout, "usage: nexus %s [options]\n", invocation);
    fprintf(stdout, "options:\n");
    fprintf(stdout, "  -v videoPid Required. Specifies the video pid to decode\n");
    fprintf(stdout, "  -a audioPid Optional. Specifies the audio pid to decode\n");
    fprintf(stdout, "  -f inputFilename Required. Specifies the file to play\n");
    fprintf(stdout, "  -t transportType Optional. Specifies the transport type of the source file.  Defaults to mpeg2ts.  Allowed values are: mpeg2ts, mp4.\n");
    fprintf(stdout, "  -c videoCodec Optional. Specifies the video codec to decode.  Defaults to hevc.  Allowed values are: hevc, mpeg, avc.\n");
    fprintf(stdout, "  -u audioCodec Optional. Specifies the audio codec to decode.  Defaults to aac.  Allowed values are: aac, ac3, mpeg.\n");
    fprintf(stdout, "  -s colorSpace Optional. Specifies the color space of the HDMI output.  Defaults to auto. Allowed values are auto, 422, 420, 444, rgb.  May not take effect if your tv doesn't support it.  Some combos of space and depth are not legal.\n");
    fprintf(stdout, "  -d colorDepth Optional. Specifies the color depth of the HDMI output.  Defaults to 0 (auto). Allowed values are 0, 8, 10, 12, and 16.  May not take effect if your tv doesn't support it. Some combos of space and depth are not legal.\n");
    fprintf(stdout, "  -o outputFormat Optional. Specifies the video output format.  Defaults to 2160p60, or whatever the display prefers.  Allowed values are 1080p50, 1080p60, 2160p30, 2160p50, and 2160p60\n");
    fprintf(stdout, "  -m metadataFilename Optional. Specifies the file from which to read DRM static metadata.\n");
    fprintf(stdout, "  -g Optional. Specifies the OSD mode to start in.  If -g is specified, OSD will be on by default; if not OSD will be off by default.\n");
}

static const UTILS_StringIntMapEntry videoCodecAliases[] =
{
    { "hevc", NEXUS_VideoCodec_eH265 },
    { "h265", NEXUS_VideoCodec_eH265 },
    { "mpeg2", NEXUS_VideoCodec_eMpeg2 },
    { "mpeg", NEXUS_VideoCodec_eMpeg2 },
    { "avc", NEXUS_VideoCodec_eH264 },
    { "h264", NEXUS_VideoCodec_eH264 },
    { NULL, NEXUS_VideoCodec_eMax },
};

NEXUS_VideoCodec ARGS_ParseVideoCodec(const char * codecStr)
{
    return (NEXUS_VideoCodec)UTILS_ParseTableAlias(videoCodecAliases, codecStr);
}

static const UTILS_StringIntMapEntry audioCodecAliases[] =
{
    { "aac", NEXUS_AudioCodec_eAac },
    { "ac3", NEXUS_AudioCodec_eAc3 },
    { "mpeg", NEXUS_AudioCodec_eMpeg },
    { NULL, NEXUS_AudioCodec_eMax },
};

NEXUS_AudioCodec ARGS_ParseAudioCodec(const char * codecStr)
{
    return (NEXUS_AudioCodec)UTILS_ParseTableAlias(audioCodecAliases, codecStr);
}

static const UTILS_StringIntMapEntry transportTypeAliases[] =
{
    { "mpeg2ts", NEXUS_TransportType_eTs },
    { "ts", NEXUS_TransportType_eTs },
    { "mp4", NEXUS_TransportType_eMp4 },
    { "es", NEXUS_TransportType_eEs, },
    { NULL, NEXUS_TransportType_eMax },
};

NEXUS_TransportType ARGS_ParseTransportType(const char * typeStr)
{
    return (NEXUS_TransportType)UTILS_ParseTableAlias(transportTypeAliases, typeStr);
}

NEXUS_ColorSpace ARGS_ParseColorSpace(const char * spaceStr)
{
    return UTILS_ParseColorSpace(spaceStr);
}

NEXUS_VideoFormat ARGS_ParseVideoFormat(const char * formatStr)
{
    return UTILS_ParseVideoFormat(formatStr);
}

NEXUS_Error ARGS_Parse(int argc, char * argv[], ARGS_Args * args)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int i;

    if (argc == 1)
    {
        ARGS_Usage(argv[0]);
        rc = NEXUS_NOT_INITIALIZED;
        goto end;
    }

    for (i = 1; i < argc; i++)
    {
        if (!strncmp(argv[i], "-h", 2))
        {
            ARGS_Usage(argv[0]);
            rc = NEXUS_NOT_INITIALIZED;
            break;
        }
        else if (!strncmp(argv[i], "-f", 2))
        {
            if (++i < argc)
            {
                args->inputFilename = UTILS_SetString(args->inputFilename, argv[i]);
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
        else if (!strncmp(argv[i], "-a", 2))
        {
            if (++i < argc)
            {
                args->audioPid = strtoul(argv[i], NULL, 0);
            }
            else
            {
                fprintf(stderr, "-a requires an integer argument (hex or dec)\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else if (!strncmp(argv[i], "-d", 2))
        {
            if (++i < argc)
            {
                args->colorDepth = strtoul(argv[i], NULL, 0);
            }
            else
            {
                fprintf(stderr, "-d requires an integer argument\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else if (!strncmp(argv[i], "-c", 2))
        {
            if (++i < argc)
            {
                args->videoCodec = ARGS_ParseVideoCodec(argv[i]);
            }
            else
            {
                fprintf(stderr, "-c requires a codec name\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else if (!strncmp(argv[i], "-u", 2))
        {
            if (++i < argc)
            {
                args->audioCodec = ARGS_ParseAudioCodec(argv[i]);
            }
            else
            {
                fprintf(stderr, "-a requires a codec name\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else if (!strncmp(argv[i], "-t", 2))
        {
            if (++i < argc)
            {
                args->transportType = ARGS_ParseTransportType(argv[i]);
            }
            else
            {
                fprintf(stderr, "-t requires a transport type name\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else if (!strncmp(argv[i], "-s", 2))
        {
            if (++i < argc)
            {
                args->colorSpace = ARGS_ParseColorSpace(argv[i]);
            }
            else
            {
                fprintf(stderr, "-s requires a color space name\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else if (!strncmp(argv[i], "-o", 2))
        {
            if (++i < argc)
            {
                args->format = ARGS_ParseVideoFormat(argv[i]);
            }
            else
            {
                fprintf(stderr, "-o requires a video output format name\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else if (!strncmp(argv[i], "-m", 2))
        {
            if (++i < argc)
            {
                args->smdFilename = UTILS_SetString(args->smdFilename, argv[i]);
            }
            else
            {
                fprintf(stderr, "-m requires filename\n");
                rc = NEXUS_INVALID_PARAMETER;
                break;
            }
        }
        else if (!strncmp(argv[i], "-g", 2))
        {
            args->osdMode = OSD_OsdMode_eOn;
        }
        else
        {
            fprintf(stdout, "Unrecognized option '%s' ignored.\n", argv[i]);
        }
    }

end:
    return rc;
}

void ARGS_Print(const ARGS_Args * args)
{
    fprintf(stdout, "# arguments:\n");
    fprintf(stdout, "input-filename = %s\n", args->inputFilename);
    fprintf(stdout, "video-pid = %u\n", args->videoPid);
    fprintf(stdout, "video-codec = %s\n", UTILS_GetVideoCodecName(args->videoCodec));
    fprintf(stdout, "audio-pid = %u\n", args->audioPid);
    fprintf(stdout, "audio-codec = %s\n", UTILS_GetAudioCodecName(args->audioCodec));
    fprintf(stdout, "format = %s\n", UTILS_GetVideoFormatName(args->format));
    fprintf(stdout, "color-space = %s\n", UTILS_GetColorSpaceName(args->colorSpace));
    fprintf(stdout, "color-depth = %u\n", args->colorDepth);
    fprintf(stdout, "transport-type = %s\n", UTILS_GetTransportTypeName(args->transportType));
    fprintf(stdout, "smd-filename = %s\n", args->smdFilename);
    fprintf(stdout, "osdMode = %s\n", OSD_GetModeName(args->osdMode));
}

NEXUS_Error ARGS_Validate(const ARGS_Args * args)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    FILE * in = NULL;

    if (!args->videoPid || args->videoPid > 0x1fff)
    {
        fprintf(stderr, "Invalid video pid specified: 0x%04x\n", args->videoPid);
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    if (args->audioPid > 0x1fff)
    {
        /* zero is allowed and means no audio */
        fprintf(stderr, "Invalid audio pid specified: 0x%04x\n", args->audioPid);
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    if (args->colorDepth != 0
        &&
        args->colorDepth != 8
        &&
        args->colorDepth != 10
        &&
        args->colorDepth != 12
        &&
        args->colorDepth != 16)
    {
        fprintf(stderr, "Invalid color depth specified: 0x%04x\n", args->colorDepth);
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    if ((args->videoCodec != NEXUS_VideoCodec_eH265)
        &&
        (args->videoCodec != NEXUS_VideoCodec_eMpeg2)
        &&
        (args->videoCodec != NEXUS_VideoCodec_eH264))
    {
        fprintf(stderr, "Invalid video codec specified: %s(%d)\n", UTILS_GetVideoCodecName(args->videoCodec), args->videoCodec);
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    if ((args->audioCodec != NEXUS_AudioCodec_eMax)
        &&
        (args->audioCodec != NEXUS_AudioCodec_eAac)
        &&
        (args->audioCodec != NEXUS_AudioCodec_eMpeg)
        &&
        (args->audioCodec != NEXUS_AudioCodec_eAc3))
    {
        fprintf(stderr, "Invalid audio codec specified: %s(%d)\n", UTILS_GetVideoCodecName(args->videoCodec), args->videoCodec);
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    in = fopen(args->inputFilename, "rb");
    if (!in)
    {
        fprintf(stderr, "Invalid file name specified: '%s'\n", args->inputFilename);
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

end:
    if (in) fclose(in);
    return rc;
}
