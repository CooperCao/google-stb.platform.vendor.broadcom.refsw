/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
/* xcode lib example app */
#include "namevalue.h"
#include "nexus_core_utils.h"
#include "bdbg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "transcode_test.h"

BDBG_MODULE(xcode_config);

#define BTST_TS_USER_DATA_ALL    (unsigned)(-1)
BTST_Context_t g_testContext;
char g_keyReturn = '\0';

const namevalue_t g_outputTypeStrs[] = {
    {"ts", BXCode_OutputType_eTs},
    {"es", BXCode_OutputType_eEs},
    {"mp4", BXCode_OutputType_eMp4File},
    {NULL, 0}
};

const namevalue_t g_inputTypeStrs[] = {
    {"file", BXCode_InputType_eFile},
    {"hdmi", BXCode_InputType_eHdmi},
    {"live", BXCode_InputType_eLive},
    {"stream", BXCode_InputType_eStream},
    {"image", BXCode_InputType_eImage},
    {NULL, 0}
};

static const namevalue_t g_audioChannelFormatStrs[] = {
    {"none", NEXUS_AudioMultichannelFormat_eNone},
    {"stereo", NEXUS_AudioMultichannelFormat_eStereo},
    {"5.1", NEXUS_AudioMultichannelFormat_e5_1},
    {"7.1", NEXUS_AudioMultichannelFormat_e7_1},
    {NULL, 0}
};

void print_xcoder_inputSetting(BTST_Transcoder_t *pContext)
{
    unsigned i;
    printf("\n****************************************************************\n");
    printf("Input Parameters\n");
    printf("SourceType   :  %s\n", lookup_name(g_inputTypeStrs, pContext->input.type));
    if(pContext->input.type == BXCode_InputType_eFile || pContext->input.type == BXCode_InputType_eStream)
        printf("filename     :  %s\n", pContext->input.data);
#if NEXUS_HAS_FRONTEND
    if(pContext->input.type == BXCode_InputType_eLive)
        printf("freq: %d MHz;  qam mode: %s\n", pContext->input.freq, lookup_name(g_qamModeStrs, pContext->input.qamMode));
#endif

    if((pContext->input.type != BXCode_InputType_eHdmi) && (pContext->input.type != BXCode_InputType_eImage))
    {
        printf("TransportType:  %s\n", lookup_name(g_transportTypeStrs, pContext->input.transportType));
        if(pContext->input.enableVideo) {
            printf("VideoCodec   :  %s\n", lookup_name(g_videoCodecStrs, pContext->input.vCodec));
            printf("VideoPid     :  %#x\n", pContext->input.videoPid);
        }
        printf("PcrPid       :  %#x\n", pContext->input.pcrPid);
    }
    if(pContext->input.enableAudio)
    {
        for(i=0; i<pContext->input.numAudios; i++) {
            printf("Audio Pid    :  %#x\n", pContext->input.audioPid[i]);
            printf("Audio Codec  :  %s\n", lookup_name(g_audioCodecStrs, pContext->input.aCodec[i]));
        }
    }
    printf("****************************************************************\n");
}

void print_xcoder_outputSetting(BTST_Transcoder_t *pContext)
{
    unsigned i;
    printf("\n****************************************************************\n");
    printf("Output Parameters\n");
    printf("Filename    : %s\n", pContext->output.data);
    if(pContext->input.enableVideo) {
        if(pContext->output.format == NEXUS_VideoFormat_eCustom2) {
            printf("Video Format: %ux%u%c%.3f\n", pContext->output.videoFormat.width,
                pContext->output.videoFormat.height,
                pContext->output.videoFormat.interlaced?'i':'p',
                (float)pContext->output.videoFormat.refreshRate/1000);
        } else {
            printf("Video Format: %s\n", lookup_name(g_videoFormatStrs, pContext->output.format));
        }
        printf("Frame Rate  : %s\n", lookup_name(g_videoFrameRateStrs, pContext->output.framerate));
        printf("Bit Rate    : %u\n", pContext->output.vBitrate);
        printf("P frames    : %d\nB frames    : %u\n", pContext->output.gopFramesP, pContext->output.gopFramesB);
        printf("Video Codec : %s\nProfile     : %s\nLevel       : %s\n",
            lookup_name(g_videoCodecStrs, pContext->output.vCodec),
            lookup_name(g_videoCodecProfileStrs, pContext->output.profile),
            lookup_name(g_videoCodecLevelStrs, pContext->output.level));
    }
    if(pContext->input.enableAudio) {
        for(i=0; i<pContext->input.numAudios && pContext->output.audioEncode[i]; i++) {
            printf("Audio Codec : %s\n", lookup_name(g_audioCodecStrs, pContext->output.audioCodec[i]));
        }
    }
    printf("****************************************************************\n");
}

void print_value_list(const namevalue_t *table)
{
    unsigned i;
    const char *sep=" {";
    for (i=0;table[i].name;i++) {
        /* skip aliases */
        if (i > 0 && table[i].value == table[i-1].value) continue;
        printf("%s (%u)%s",sep,table[i].value, table[i].name);
        sep = ",";
    }
    printf(" }\n");
}

unsigned get_display_index(unsigned encoder)
{
    static bool set = false;
    static NEXUS_VideoEncoderCapabilities cap;
    if (!set) {
        NEXUS_GetVideoEncoderCapabilities(&cap);
        set = true;
    }
    if (encoder < NEXUS_MAX_VIDEO_ENCODERS && cap.videoEncoder[encoder].supported) {
        return cap.videoEncoder[encoder].displayIndex;
    }
    return 0;
}

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void open_gfx( BTST_Transcoder_t *pContext )
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Error rc;
    BTST_Context_t *pTest = &g_testContext;

    if(pTest->gfxRefCnt == 0) {
        pTest->gfx = NEXUS_Graphics2D_Open(0, NULL);
        BKNI_CreateEvent(&pTest->checkpointEvent);
        BKNI_CreateEvent(&pTest->spaceAvailableEvent);
    }
    pTest->gfxRefCnt++;

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 100;
    createSettings.height = 100;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(get_display_index(pContext->id));
    pContext->framebuffer = NEXUS_Surface_Create(&createSettings);

    NEXUS_Graphics2D_GetSettings(pTest->gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = pTest->checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = pTest->spaceAvailableEvent;
    NEXUS_Graphics2D_SetSettings(pTest->gfx, &gfxSettings);

    /* fill with black */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = pContext->framebuffer;
    fillSettings.rect.width = createSettings.width;
    fillSettings.rect.height = createSettings.height;
    fillSettings.color = 0x60FF0000;
    NEXUS_Graphics2D_Fill(pTest->gfx, &fillSettings);

    rc = NEXUS_Graphics2D_Checkpoint(pTest->gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(pTest->checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);

    BXCode_GetSettings(pContext->hBxcode, &pContext->settings);
    pContext->settings.video.gfxSettings.enabled = true;
    pContext->settings.video.gfxSettings.position.x = 0;
    pContext->settings.video.gfxSettings.position.y = 0;
    pContext->settings.video.gfxSettings.position.width  = createSettings.width;
    pContext->settings.video.gfxSettings.position.height = createSettings.height;
    pContext->settings.video.gfxSettings.clip.width      = pContext->settings.video.gfxSettings.position.width;
    pContext->settings.video.gfxSettings.clip.height     = pContext->settings.video.gfxSettings.position.height;
    pContext->settings.video.frameBuffer = pContext->framebuffer;
    BXCode_SetSettings(pContext->hBxcode, &pContext->settings);
    BDBG_MSG(("Xcoder[%u] enabled gfx", pContext->id));
}

void close_gfx( BTST_Transcoder_t *pContext )
{
    BTST_Context_t *pTest = &g_testContext;

    if(pTest->gfxRefCnt == 0) return;

    /* disable gfx window */
    BXCode_GetSettings(pContext->hBxcode, &pContext->settings);
    pContext->settings.video.gfxSettings.enabled = false;
    BXCode_SetSettings(pContext->hBxcode, &pContext->settings);
    if(pContext->framebuffer) {
        NEXUS_Surface_Destroy(pContext->framebuffer);
        pContext->framebuffer = NULL;
    }

    if(--pTest->gfxRefCnt == 0) {
        NEXUS_Graphics2D_Close(pTest->gfx);
        BKNI_DestroyEvent(pTest->checkpointEvent);
        BKNI_DestroyEvent(pTest->spaceAvailableEvent);
        pTest->gfx = NULL;
    }
}

void xcode_index_filename(
    char *indexFile,
    const char *mediaFile,
    bool segmented,
    NEXUS_TransportType type)
{
    char *tmpPtr=NULL;
    int size = strlen(mediaFile);
    strcpy(indexFile, mediaFile);
    if(type == NEXUS_TransportType_eTs) {
        while(size-->1) {
            if(indexFile[size] == '.') {
                tmpPtr = &indexFile[size];
                break;
            }
        }
        if (tmpPtr) {
            strcpy(tmpPtr+1, segmented? "hls" : "nav");
        }
        else {
            strcat(indexFile, segmented? ".hls" : ".nav");
        }
    }
    BDBG_MSG(("Media file name: %s, index file name %s", mediaFile, indexFile));
}

static unsigned mylookup(const namevalue_t *table, const char *name)
{
    unsigned i;
    unsigned value;
    char *endptr;
    const char *valueName;
    for (i=0;table[i].name;i++) {
        if (!strcasecmp(table[i].name, name)) {
            return table[i].value;
        }
    }
    value = strtol(name, &endptr, 0);
    if(!endptr || *endptr) { /* if valid, *endptr = '\0' */
        value = table[0].value;
    }
    valueName = lookup_name(table, value);
    BDBG_MSG(("Unknown cmdline param '%s', using %u as value ('%s')", name, value, valueName?valueName:"unknown"));
    return value;
}

/* search for "name=value;" and get the value */
static unsigned getNameValue(char *input, const namevalue_t list[])
{
    char *pToken;
    scanf("%s", input);
    pToken = strstr(input, ";");
    if(pToken) { *pToken = '\0'; }
    pToken = strstr(input, "=");
    if (pToken) {
        /* ignoring "name=" for now */
        pToken++;
    }
    else {
        pToken = input;
    }
    return mylookup(list, pToken);
}

static unsigned getValue(char *input)
{
    char *pToken;
    scanf("%s", input);
    pToken = strstr(input, ";");
    if(pToken) { *pToken = '\0'; }
    pToken = strstr(input, "=");
    return strtoul(pToken? (pToken+1) : input, NULL, 0);
}

static const char *getString(char *input)
{
    char *pToken;
    scanf("%s", input);
    pToken = strstr(input, ";");
    if(pToken) { *pToken = '\0'; }
    pToken = strstr(input, "=");
    return (pToken? (pToken+1) : input);
}

void config_xcoder_context (
    BTST_Transcoder_t *pContext )
{
    unsigned choice, i;
    char input[80];
    pContext->custom = true;
    printf("choose input type:\n");
    print_value_list(g_inputTypeStrs);
    pContext->startSettings.input.type = pContext->input.type = getNameValue(input, g_inputTypeStrs);

    switch (pContext->input.type)
    {
    case BXCode_InputType_eHdmi:
#if !NEXUS_HAS_HDMI_INPUT
        fprintf(stderr, "HDMI input is not supported!\n");
#endif
        break;
    case BXCode_InputType_eLive:
#if NEXUS_HAS_FRONTEND
        printf("Front End QAM freq (Mhz): ");
        pContext->input.freq = getValue(input);
        printf("Front End QAM mode: ");
        print_value_list(g_qamModeStrs);
        pContext->input.qamMode = getNameValue(input, g_qamModeStrs);
        pContext->input.transportType = NEXUS_TransportType_eTs;
        if(pContext->input.enableVideo) {
            print_value_list(g_videoCodecStrs);
            pContext->input.vCodec = getNameValue(input, g_videoCodecStrs);
            printf("Video pid:\n");
            pContext->input.videoPid = getValue(input);
        }
        printf("Pcr   pid:\n");
        pContext->input.pcrPid = getValue(input);
#else
        printf("QAM tuning not supported by this platform!\n");
#endif
        break;
    case BXCode_InputType_eFile:
    case BXCode_InputType_eStream:
        printf("source stream file:\n");
        strcpy(pContext->input.data, getString(input));
        printf("Transport type:\n");
        print_value_list(g_transportTypeStrs);
        pContext->input.transportType = getNameValue(input, g_transportTypeStrs);
        if(pContext->input.enableVideo) {
            printf("Video codec type:\n");
            print_value_list(g_videoCodecStrs);
            pContext->input.vCodec = getNameValue(input, g_videoCodecStrs);
            printf("Video pid:\n");
            pContext->input.videoPid = getValue(input);
        }
        printf("Pcr   pid:\n");
        pContext->input.pcrPid = getValue(input);
        break;
    case BXCode_InputType_eImage:
        printf("source image file:\n");
        strcpy(pContext->input.data, getString(input));
        printf("Image width :\n");
        pContext->input.maxWidth  = getValue(input);
        printf("Image height:\n");
        pContext->input.maxHeight = getValue(input);
        break;
    default:
        printf("Input type %u is not supported.\n", pContext->input.type);
    }

    printf("Input Audio Parameters\n");
    printf("Enable Audio: (0) No (1) Yes\n");
    pContext->input.enableAudio = getValue(input);
    if(pContext->input.enableAudio) {
        if(pContext->input.numAudios > 1) {
            printf("Number of audio channels?\n");
            pContext->input.numAudios = getValue(input);
        } else pContext->input.numAudios = 1;
        if(pContext->input.numAudios > BXCODE_MAX_AUDIO_PIDS) {
            printf("numAudios %u > %u supported!.\n", pContext->input.numAudios, BXCODE_MAX_AUDIO_PIDS);
            pContext->input.numAudios = BXCODE_MAX_AUDIO_PIDS;
        }
        for(i=0; i<pContext->input.numAudios; i++)
        {
            /* HDMI input only support single audio? */
            if (pContext->input.type == BXCode_InputType_eHdmi)
            {
                printf("Is HDMI input audio uncompressed? (0)No; (1)Yes\n");
                pContext->input.pcmAudio = getValue(input);
                if(pContext->input.pcmAudio)
                {
                    pContext->output.audioEncode[i] = true;
                }
                pContext->input.numAudios = 1;/* HDMI input has single audio pid output */
                break;
            }
            else
            {
                printf("Audio Pid:\n");
                pContext->input.audioPid[i] = getValue(input);
            }

            printf("Audio codec type:\n");
            print_value_list(g_audioCodecStrs);
            pContext->input.aCodec[i] = getNameValue(input, g_audioCodecStrs);
            pContext->output.audioCodec[i] = pContext->input.aCodec[i];
            if(pContext->input.multiChanFmt) {
                printf("\n Multi-channel audio format: \n");
                print_value_list(g_audioChannelFormatStrs);
                pContext->input.multiChanFmt = getNameValue(input, g_audioChannelFormatStrs);
            }
            if(pContext->input.secondaryAudio) {/* TODO: do we support multi-stream for multiple pids? */
                printf("Secondary Audio Pid:\n");
                pContext->input.secondaryAudioPid = getValue(input);
                break;
            }
        }
    }

    if((pContext->input.type == BXCode_InputType_eFile) && pContext->input.enableVideo) {
        xcode_index_filename(pContext->input.index, pContext->input.data, false, pContext->input.transportType);
    }
    print_xcoder_inputSetting(pContext);

    printf("\n\nOutput Settings\n");
    if(pContext->output.type != BXCode_OutputType_eEs) {
        printf("Output file name:\n");
        strcpy(pContext->output.data, getString(input));
    }
    if(pContext->input.enableVideo) {
        if(pContext->output.type == BXCode_OutputType_eEs) {
            printf("Output video file name:\n");
            strcpy(pContext->output.data, getString(input));
        }
        printf("\n custom format: (0) No (1) Yes\n");
        if(getValue(input)) {
            printf("Video width: ");
            pContext->output.videoFormat.width = getValue(input);
            printf("Video height: ");
            pContext->output.videoFormat.height = getValue(input);
            printf("Interlaced? (0) No (1) Yes\n");
            pContext->output.videoFormat.interlaced = getValue(input);
            printf("Encode display refresh rate (in 1/1000th Hz): ");
            pContext->output.videoFormat.refreshRate = getValue(input);
            printf("Display aspect ratio:\n");
            print_value_list(g_displayAspectRatioStrs);
            pContext->output.videoFormat.aspectRatio = getNameValue(input, g_displayAspectRatioStrs);
            if(NEXUS_DisplayAspectRatio_eSar == pContext->output.videoFormat.aspectRatio)
            {
                printf("Please enter Sample Aspect Ratio X and Y: \n");
                pContext->output.videoFormat.sampleAspectRatio.x = getValue(input);
                pContext->output.videoFormat.sampleAspectRatio.y = getValue(input);
            }
            pContext->output.format = NEXUS_VideoFormat_eCustom2;
        }else {
            printf("Encoder display format:\n");
            print_value_list(g_videoFormatStrs);
            pContext->output.format = getNameValue(input, g_videoFormatStrs);
        }

        printf("Frame rate (fps):\n");
        print_value_list(g_videoFrameRateStrs);
        pContext->output.framerate = getNameValue(input, g_videoFrameRateStrs);
        if(pContext->output.orientation) {
            printf("3D orientation type:\n");
            print_value_list(g_videoOrientation);
            pContext->output.orientation = getNameValue(input, g_videoOrientation);
        }

        printf("Max Bitrate (bps):\n");
        pContext->output.vBitrate = getValue(input);
        if(pContext->output.targetBitrate) {
           printf("VBR target Bitrate (bps) [0=CBR; else VBR]:\n");
           pContext->output.targetBitrate = getValue(input);
        }
        printf("Number of P frames per GOP:\n");
        pContext->output.gopFramesP = getValue(input);
        printf("Number of B frames between reference frames:\n");
        pContext->output.gopFramesB = getValue(input);
        printf("Encode Video Codec: (%d) MPEG2 (%d) H264\n", NEXUS_VideoCodec_eMpeg2, NEXUS_VideoCodec_eH264);
        pContext->output.vCodec = getNameValue(input, g_videoCodecStrs);
        printf("Video codec profile:\n");
        print_value_list(g_videoCodecProfileStrs);
        pContext->output.profile = getNameValue(input, g_videoCodecProfileStrs);
        printf("video codec level:\n");
        print_value_list(g_videoCodecLevelStrs);
        pContext->output.level = getNameValue(input, g_videoCodecLevelStrs);
    }

    if(pContext->input.enableAudio)
    {
        for(i=0; i<pContext->input.numAudios; i++) {
            if(pContext->output.type == BXCode_OutputType_eEs) {
                printf("Output audio file name:\n");
                strcpy(pContext->output.audioFiles[i], getString(input));
            }
            if(pContext->input.pcmAudio)
            {
                pContext->output.audioEncode[i] = true;
            }
            else
            {
                printf("Enable audio transcode: (0) No (1) Yes\n");
                pContext->output.audioEncode[i] = getValue(input);
            }

            if(pContext->output.audioEncode[i])
            {
                printf("Output audio Codec: (%d)mp3 (%d)aac (%d)aacplus (%d)lpcm_1394 \n",
                    NEXUS_AudioCodec_eMp3,
                    NEXUS_AudioCodec_eAac,
                    NEXUS_AudioCodec_eAacPlus,
                    NEXUS_AudioCodec_eLpcm1394);
                pContext->output.audioCodec[i] = getNameValue(input, g_audioCodecStrs);
            }
            else {
                if(pContext->input.secondaryAudio) {
                    BDBG_ERR(("Multi audio streams cannot be passed through!"));
                }
            }
        }
    }
    if(pContext->input.type != BXCode_InputType_eLive)
    {
        printf("Non-Realtime transcode? [1=y/0=n]\n");
        pContext->nonRealTime = getValue(input);
    }

    /* Reset custom settings */
    if(pContext->input.enableVideo && (!pContext->nonRealTime || pContext->output.customizeDelay))
    {
        printf("Customize video encoder delay setting which might impact quality? [1=y/0=n]\n");
        pContext->output.customizeDelay = getValue(input);
        if(pContext->output.customizeDelay)
        {
            printf("Enable PicAFF repeat cadence coding(better bit efficiency with more delay)? [1=y/0=n]\n");
            pContext->output.enableFieldPairing = getValue(input);

            printf("Video encoder rate buffer delay (ms) [0 means default best quality with more delay]:\n");
            pContext->output.rateBufferDelay = getValue(input);

            printf("Video encode display minimum refresh rate (Hz):\n");
            print_value_list(g_videoFrameRateStrs);
            pContext->output.bounds.inputFrameRate.min = getNameValue(input, g_videoFrameRateStrs);

            printf("Video encoder output minimum frame rate (Hz):\n");
            print_value_list(g_videoFrameRateStrs);
            pContext->output.bounds.outputFrameRate.min = getNameValue(input, g_videoFrameRateStrs);

            printf("Video encode maximum resolution width:\n");
            pContext->output.bounds.inputDimension.max.width = getValue(input);
            printf("Video encode maximum resolution height:\n");
            pContext->output.bounds.inputDimension.max.height = getValue(input);
            printf("Video encoder pipeline low delay? [1=Y/0=N]:\n");
            choice = getValue(input);
            if(choice && (pContext->output.gopFramesP != 0xFFFFFFFF ||
                pContext->output.enableFieldPairing)) {
                BDBG_ERR(("Encoder HW pipeline low delay mode can only be enabled if '-type single' and framesP = 0xFFFFFFFF (or -1) and Field Pairing disabled"));
            }
            pContext->output.lowDelay = choice;
            printf("Upper bound for dynamic video bitrate:\n");
            pContext->output.bounds.bitrate.upper.bitrateMax = getValue(input);
            if(pContext->output.targetBitrate) {
                printf("VBR target Bitrate upper bound (bps):\n");
                pContext->output.bounds.bitrate.upper.bitrateTarget = getValue(input);
            }
            printf("Maximum number of B pictures between I or P reference pictures:\n");
            pContext->output.bounds.streamStructure.max.framesB = getValue(input);
        }
    }

    /* TS user data config */
    if(pContext->input.type != BXCode_InputType_eHdmi && pContext->output.type == BXCode_OutputType_eTs) {
        if(pContext->input.tsUserDataInput) {
            printf("PMT pid [0=auto detection]:                           ");
            pContext->input.pmtPid = getValue(input);
            printf("Do you want all TS user data passed thru? [1=y/0=n]:\n");
            choice = getValue(input);
            if(choice) {
                pContext->input.numUserDataPids  = BTST_TS_USER_DATA_ALL;
                pContext->input.remapUserDataPid = false;
            } else {
                printf("How many user data PIDs are passed thru?\n");
                pContext->input.numUserDataPids = getValue(input);
                pContext->input.numUserDataPids = (pContext->input.numUserDataPids > NEXUS_MAX_MUX_PIDS) ?
                    NEXUS_MAX_MUX_PIDS : pContext->input.numUserDataPids;
                printf("Please enter the input user data PIDs list:\n");
                for(choice=0; choice<pContext->input.numUserDataPids; choice++) {
                    pContext->input.userDataPid[choice] = getValue(input);
                }
                printf("Remap the output TS user data PIDs? [1=y/0=n]\n");
                pContext->input.remapUserDataPid = getValue(input);
                for(choice=0; choice<pContext->input.numUserDataPids; choice++) {
                    if(pContext->input.remapUserDataPid) {
                        pContext->input.remappedUserDataPid[choice] = BTST_MUX_USER_DATA_PID+choice;
                    } else {/* no change */
                        pContext->input.remappedUserDataPid[choice] = pContext->input.userDataPid[choice];
                    }
                }
            }
        }
    }

    if(pContext->output.type == BXCode_OutputType_eTs && pContext->output.file) {
        /* set record index file name and open the record file handle */
        if(pContext->input.enableVideo)
        {
            xcode_index_filename(pContext->output.index, pContext->output.data, pContext->output.segmented,
                (BXCode_OutputType_eTs==pContext->output.type)? NEXUS_TransportType_eTs :
                ((BXCode_OutputType_eMp4File==pContext->output.type)? NEXUS_TransportType_eMp4 : NEXUS_TransportType_eEs));
        }
        else BDBG_WRN(("no index record"));
    }
    print_xcoder_outputSetting(pContext);
    printf("%s transcode...%s \n", pContext->nonRealTime? "Non-Realtime":"Real time",
        pContext->output.lowDelay? "Low Delay pipeline Mode": "Normal delay pipeline Mode");
}

void print_usage(void) {
            printf("\ntranscode_ts usage:\n");
            printf("  Without options, it transcodes default stream file /data/videos/avatar_AVC_15M.ts into TS file: /data/BAT/encode.ts\n");
            printf("\nOptions:\n");
            printf("  -h        - to print the usage info\n");
            printf("  -cxt N    - to select xcoder N\n");
            printf("  -scr N    - to run in script mode and quit in N seconds\n");
            printf("  -cfg      - to set the test configuration\n");
            printf("  -fifo     - to use FIFO file record instead of unbounded file record for long term test\n");
            printf("  -loop     - to loop source file for long term file transcode test\n");
            printf("  -autoquit - to quit the test automatically when all transcoders are stopped\n");
            printf("  -tsud     - to enable TS layer user data passthrough.\n");
            printf("  -multichan - to enable audio multi-channel mode setting.\n");
            printf("  -6xaudio  - to enable up to 6x audio PIDs setting per transcoder.\n");
            printf("  -msaudio  - to enable audio multi-stream decode mode.\n");
            printf("  -tts_in binary|mod300  - Input TTS transport packets prepended with 4-byte timestamps in binary or mod300 mode.\n");
            printf("  -tts_out binary|mod300  - Output TTS transport packets prepended with 4-byte timestamps in binary or mod300 mode.\n");
            printf("  -win {box|zoom} - to start transcoder with specified aspect ratio correction mode (default bypass).\n");
            printf("  -loopbackon - to start transcoder with loopback player enabled (default OFF).\n");
            printf("  -vdecZeroUp [-maxSize W H] - to start transcoder with video decoder 0 up, optionally decoder 0 with max WxH size.\n");
            printf("  -vbr      - to enable VBR video encode.\n");
            printf("  -3d       - enable 3D encode\n");
            printf("  -vfr      - enable variable frame rate transcode\n");
            printf("  -sd       - enable CVBS debug display for source decoder.\n");
            printf("  -openGop  - enable open GOP encode.\n");
            printf("  -dynamicGop - enable new GOP on scene change.\n");
            printf("  -gopDuration N - GOP duration in NUM ms.\n");
            printf("  -progressive_only - Video encoder output format is progressive only.\n");
            printf("  -maxDimension WIDTH HEIGHT  - Video encoder maximum resolution.\n");
            printf("  -noVideo - Audio only transcode without video.\n");
            printf("  -noCCdata - Disable closed caption user data in video.\n");
            printf("  -output_type");
            print_list(g_outputTypeStrs);
            printf(" - output type. Default TS.\n");
            printf("  -output_stream - Output in stream mode.\n");
            printf("  -segmented - Output TS in segmented mode.\n");
            printf("  -in [FILE [-program N] [-rt]] - Probe the input FILE and take program N as source, optionally run in RT mode.\n");
            printf("  -video_size W,H - Encode output size WxH.\n");
            printf("  -video_bitrate N - Encode output bitrate N bps.\n");
            print_list_option("video_framerate", g_videoFrameRateStrs);
            print_list_option("video_refreshrate", g_videoFrameRateStrs);
}

int cmdline_parse(int argc, char **argv, BTST_Context_t *pTest)
{
    int i;
    BTST_Transcoder_t *pContext = &pTest->xcodeContext[0];

    /* default config */
    pTest->loopbackPlayer = false;/* default loopback OFF */
    pContext->output.ccUserdata = true;/* default cc user data ON */
    for(i=0; i<NEXUS_NUM_VIDEO_ENCODERS; i++) {
        pTest->xcodeContext[i].output.file = true;/* default file output */
        pTest->xcodeContext[i].input.enableVideo = true;/* default video ON */
    }
    if (argc == 1){ /* default test config for auto test */
            print_usage();
            printf("\nYou're testing with the default configuration:\n");
            /* Input setting */
            pContext->loop = true;/* looping source file */
            pContext->input.type = BXCode_InputType_eFile;
            BKNI_Snprintf(pContext->input.data, 256, "/data/videos/cnnticker.mpg");
            pContext->input.transportType = NEXUS_TransportType_eTs;
            pContext->input.vCodec        = NEXUS_VideoCodec_eMpeg2;
            pContext->input.numAudios     = 1;
            pContext->input.aCodec[0] = NEXUS_AudioCodec_eMpeg;
            pContext->input.pcmAudio=false;
            pContext->input.videoPid    = 0x21;
            pContext->input.audioPid[0] = 0x22;
            pContext->input.pcrPid      = 0x21;
            pContext->input.enableAudio = true;/* enable audio */
            pContext->input.enableVideo = true;/* enable video */
            pContext->custom            = false;

            /*Encode settings */
            pContext->output.type = BXCode_OutputType_eTs;
            BKNI_Snprintf(pContext->output.data, 256, "/data/BAT/encode.ts");
            xcode_index_filename(pContext->output.index, pContext->output.data, false, NEXUS_TransportType_eTs);
            pContext->output.format = NEXUS_VideoFormat_eCustom2;
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
            pContext->output.videoFormat.width  = 416;
            pContext->output.videoFormat.height = 224;
#else
            pContext->output.videoFormat.width  = 1280;
            pContext->output.videoFormat.height = 720;
            /* restrict encoder memory config for 720p xcode by default */
            pContext->output.videoEncoderMemConfig.progressiveOnly = true;
            pContext->output.videoEncoderMemConfig.maxWidth = 1280;
            pContext->output.videoEncoderMemConfig.maxHeight = 720;
#endif
            pContext->output.framerate = NEXUS_VideoFrameRate_e29_97;
            pContext->output.gopFramesP = 29;
            pContext->output.gopFramesB = 0;
            pContext->output.vCodec     = NEXUS_VideoCodec_eH264;
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
            pContext->output.profile = NEXUS_VideoCodecProfile_eBaseline;
            pContext->output.level   = NEXUS_VideoCodecLevel_e20;
            pContext->output.vBitrate= 400000;
#else
            pContext->output.profile  = NEXUS_VideoCodecProfile_eBaseline;
            pContext->output.level    = NEXUS_VideoCodecLevel_e31;
            pContext->output.vBitrate = 2500000;
#endif
            pContext->output.audioEncode[0] = true;/* transcode audio to AAC */
            pContext->output.audioCodec[0]  = NEXUS_AudioCodec_eAac;
            pTest->loopbackPlayer = false; /* disable loopback player by default */

            print_xcoder_inputSetting(pContext);
            print_xcoder_outputSetting(pContext);
    } else {
        for(i=1; i<argc; i++) {
            if(!strcmp("-h",argv[i])) {
                print_usage();
                return 0;
            }
            if(!strcmp("-in",argv[i]) && (i+1 < argc)) {
                #define BTST_P_DEFAULT(a, b) {if(a==0) a = b;}
                strcpy(pContext->input.data, argv[++i]);
                pContext->input.type  = BXCode_InputType_eFile; /* file input */
                pContext->input.probe = true; /* enabled media probe */
                BTST_P_DEFAULT(pContext->output.type, BXCode_OutputType_eTs);
#if !NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
                pContext->nonRealTime = true; /* enabled NRT mode by default */
#endif
                fprintf(stderr, "Input media file %s\n", pContext->input.data);
                if((i+2) < argc) { /* check optional -program N */
                    if(!strcmp("-program", argv[i+1])) {
                        i++;
                        pContext->input.program = strtoul(argv[++i], NULL, 0);
                        fprintf(stderr, "Selected program %u.\n", pContext->input.program);
                    }
                }
                if((i+1) < argc) { /* check optional -rt */
                    if(!strcmp("-rt", argv[i+1])) {
                        i++;
                        pContext->nonRealTime = false;
                        fprintf(stderr, "RT mode.\n");
                    }
                }
                /* set default encoder settings */
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
                #if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT /* 416x224p30@400Kbps */
                BTST_P_DEFAULT(pContext->output.videoFormat.width, 416);
                BTST_P_DEFAULT(pContext->output.videoFormat.height, 224);
                BTST_P_DEFAULT(pContext->output.vBitrate, 400000);
                #else /* 480p@2Mbps */
                BTST_P_DEFAULT(pContext->output.videoFormat.width, 720);
                BTST_P_DEFAULT(pContext->output.videoFormat.height, 480);
                BTST_P_DEFAULT(pContext->output.vBitrate, 2000000);
                #endif
#else
                BTST_P_DEFAULT(pContext->output.videoFormat.width, 1280);
                BTST_P_DEFAULT(pContext->output.videoFormat.height, 720);
                BTST_P_DEFAULT(pContext->output.vBitrate, 3000000);
#endif
                BTST_P_DEFAULT(pContext->output.videoFormat.refreshRate, 59940);
                BTST_P_DEFAULT(pContext->output.videoFormat.aspectRatio, NEXUS_DisplayAspectRatio_eSar);
                BTST_P_DEFAULT(pContext->output.videoFormat.sampleAspectRatio.x, 1);
                BTST_P_DEFAULT(pContext->output.videoFormat.sampleAspectRatio.y, 1);
                BTST_P_DEFAULT(pContext->output.framerate, NEXUS_VideoFrameRate_e29_97);
                pContext->output.format     = NEXUS_VideoFormat_eCustom2;
                pContext->output.gopFramesP = 29;
                pContext->output.gopFramesB = 0;
                pContext->output.vCodec     = NEXUS_VideoCodec_eH264;
                pContext->output.profile    = NEXUS_VideoCodecProfile_eMain;/* default CABAC on */
                pContext->output.level      = NEXUS_VideoCodecLevel_e31;
                pContext->output.audioEncode[0]= true;/* transcode audio to AAC */
                pContext->output.audioCodec[0] = NEXUS_AudioCodec_eAac;
            }
            if(!strcmp("-video_size",argv[i]) && (i+1 < argc)) {
                if (sscanf(argv[++i], "%u,%u", &pContext->output.videoFormat.width,
                   &pContext->output.videoFormat.height) != 2) {
                    print_usage();
                    return -1;
                }
                fprintf(stderr, "Video size %ux%u\n", pContext->output.videoFormat.width, pContext->output.videoFormat.height);
            }
            if(!strcmp("-interlaced",argv[i])) {
                pContext->output.videoFormat.interlaced = true;
                fprintf(stderr, "Video is interlaced\n");
            }
            if(!strcmp("-video_bitrate",argv[i]) && (i+1 < argc)) {
                pContext->output.vBitrate = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "Video bitrate is %u bps\n", pContext->output.vBitrate);
            }
            if(!strcmp("-video_framerate",argv[i]) && (i+1 < argc)) {
                pContext->output.framerate = mylookup(g_videoFrameRateStrs, argv[++i]);
                fprintf(stderr, "Video frame rate is %s fps\n", argv[i]);
            }
            if(!strcmp("-video_refreshrate",argv[i]) && (i+1 < argc)) {
                float rate;
                sscanf(argv[++i], "%f", &rate);
                pContext->output.videoFormat.refreshRate = rate * 1000;
                fprintf(stderr, "Video refresh rate is %f Hz\n", rate);
            }
            if(!strcmp("-fifo",argv[i])) {
                pContext->output.fifo = true;
                fprintf(stderr, "Enabled fifo record...\n");
            }
            if(!strcmp("-loop",argv[i])) {
                pContext->loop = true;
                fprintf(stderr, "Enabled file wraparound...\n");
            }
            if(!strcmp("-autoquit",argv[i])) {
                pTest->autoQuit = true;
                fprintf(stderr, "Enabled auto quit...\n");
            }
            if(!strcmp("-tsud",argv[i])) {
                pContext->input.tsUserDataInput = true;
                fprintf(stderr, "Enabled TS layer user data transcode...\n");
            }
            if(!strcmp("-multichan",argv[i])) {
                pContext->input.multiChanFmt = NEXUS_AudioMultichannelFormat_eStereo;
                fprintf(stderr, "To config multi-channel format for audio input later. must have fw mixer..\n");
            }
            if(!strcmp("-msaudio",argv[i])) {
                pContext->input.secondaryAudio = true;
                fprintf(stderr, "Multi-stream audio input. must have fw mixer.\n");
            }
            if(!strcmp("-6xaudio",argv[i])) {
                pContext->input.numAudios = 6;
                fprintf(stderr, "Enabled up to 6x audio PIDs passthrough.\n");
            }
            if(!strcmp("-nrtRate",argv[i])) {
                pContext->output.nrtRate = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "NRT rate = %u x\n", pContext->output.nrtRate);
            }
            if(!strcmp("-vbr",argv[i])) {
                pContext->output.targetBitrate = 1000000;
                fprintf(stderr, "VBR video encode target bitrate to be configured later.\n");
            }
            if(!strcmp("-customDelay",argv[i])) {
                pContext->output.customizeDelay = true;
                fprintf(stderr, "customize video encoder delay.\n");
            }
            if(!strcmp("-vfr",argv[i])) {
                pContext->output.variableFramerate = true;
                fprintf(stderr, "Variable frame rate video encode.\n");
            }
            if(!strcmp("-dynamicGop",argv[i])) {
                pContext->output.newGopOnSceneChange = true;
                fprintf(stderr, "Enabled new GOP on scene change.\n");
            }
            if(!strcmp("-openGop",argv[i])) {
                pContext->output.openGop = true;
                fprintf(stderr, "Enabled open GOP.\n");
            }
            if(!strcmp("-gopDuration",argv[i])) {
                pContext->output.gopDuration = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "GOP duration = %u ms.\n", pContext->output.gopDuration);
            }
            if(!strcmp("-3d",argv[i])) {
                pContext->output.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
                fprintf(stderr, "3D video encode.\n");
            }
            if(!strcmp("-noVideo",argv[i])) {
                pContext->input.enableVideo = false;
                fprintf(stderr, "No video encode.\n");
            }
            if(!strcmp("-noCCdata",argv[i])) {
                pContext->output.ccUserdata = false;
                fprintf(stderr, "No cc user data encode.\n");
            }
            if(!strcmp("-scr",argv[i])) {
                pTest->scriptMode = true;
                pTest->scriptModeSleepTime = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "Enabled script mode and quit in %u seconds...\n", pTest->scriptModeSleepTime);
            }
            if(!strcmp("-cxt",argv[i])) {
                pTest->selectedXcodeContextId = strtoul(argv[++i], NULL, 0);
                pTest->selectedXcodeContextId = (pTest->selectedXcodeContextId > NEXUS_NUM_VIDEO_ENCODERS-1)
                    ? (NEXUS_NUM_VIDEO_ENCODERS-1):pTest->selectedXcodeContextId;
                BKNI_Memcpy(&pTest->xcodeContext[pTest->selectedXcodeContextId], pContext, sizeof(BTST_Transcoder_t));
                BKNI_Memset(pContext, 0, sizeof(BTST_Transcoder_t));
                pContext = &pTest->xcodeContext[pTest->selectedXcodeContextId];
                fprintf(stderr, "Select xcoder context %d...\n", pTest->selectedXcodeContextId);
            }
            if(!strcmp("-win",argv[i])) {
                i++;
                if(!strcmp("box", argv[i])) {
                    pContext->output.contentMode = NEXUS_VideoWindowContentMode_eBox;
                } else if(!strcmp("zoom", argv[i])) {
                    pContext->output.contentMode = NEXUS_VideoWindowContentMode_eZoom;
                }
                fprintf(stderr, "Start transcoder with content mode %d.\n", pContext->output.contentMode);
            }
            if(!strcmp("-loopbackon",argv[i])) {
                pTest->loopbackPlayer = true;
                fprintf(stderr, "Start transcoder with loopback player ON.\n");
            }
            if(!strcmp("-vdecZeroUp", argv[i])) {
                pTest->decoderZeroUp = true;
                fprintf(stderr, "Start transcoder with video decoder 0 up.\n");
                if((i+3) < argc) { /* check optional -maxSize */
                    if(!strcmp("-maxSize", argv[i+1])) {
                        i++;
                        pContext->input.maxWidth  = strtoul(argv[++i], NULL, 0);
                        pContext->input.maxHeight = strtoul(argv[++i], NULL, 0);
                        fprintf(stderr, "Max decode resolution: %u x %u\n", pContext->input.maxWidth, pContext->input.maxHeight);
                    }
                }
            }
            if(!strcmp("-sd",argv[i])) {
                pTest->enableDebugSdDisplay = true;
                fprintf(stderr, "Enabled CVBS debug display for source decoder.\n");
            }
            if(!strcmp("-tts_in",argv[i])) {
                i++;
                if(!strcmp("binary",argv[i])) {
                    pContext->input.transportTimestamp = NEXUS_TransportTimestampType_e32_Binary;
                }
                else {
                    pContext->input.transportTimestamp = NEXUS_TransportTimestampType_e32_Mod300;
                }
                fprintf(stderr, "Input TS transport packets prepended with 4-byte timestamps mode %s.\n",
                    lookup_name(g_tsTimestampType, pContext->input.transportTimestamp));
            }
            if(!strcmp("-tts_out",argv[i])) {
                i++;
                if(!strcmp("binary",argv[i])) {
                    pContext->output.transportTimestamp = NEXUS_TransportTimestampType_e32_Binary;
                }
                else {
                    pContext->output.transportTimestamp = NEXUS_TransportTimestampType_e32_Mod300;
                }
                fprintf(stderr, "Output TS transport packets prepended with 4-byte timestamps mode %s.\n",
                    lookup_name(g_tsTimestampType, pContext->output.transportTimestamp));
            }
            if(!strcmp("-progressive_only",argv[i])) {
                pContext->output.videoEncoderMemConfig.progressiveOnly = true;
                fprintf(stderr, "Xcode output is progressive only format.\n");
            }
            if(!strcmp("-maxDimension",argv[i]) && i+2<argc) {
                pContext->output.videoEncoderMemConfig.maxWidth = strtoul(argv[++i], NULL, 0);
                pContext->output.videoEncoderMemConfig.maxHeight = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "Max encode resolution: %u x %u\n",
                    pContext->output.videoEncoderMemConfig.maxWidth, pContext->output.videoEncoderMemConfig.maxHeight);
            }
            if(!strcmp("-output_type", argv[i])) {
                if(i+1 < argc) {
                    pContext->output.type = lookup(g_outputTypeStrs, argv[++i]);
                    fprintf(stderr, "Output type %s = %d\n", argv[i], pContext->output.type);
                    pContext->output.file = (pContext->output.type != BXCode_OutputType_eEs);
                }
            }
            if(!strcmp("-progressive_download", argv[i])) {
                pContext->output.progressiveDownload = true;
                fprintf(stderr, "MP4 file output is progressive downloadable.\n");
            }
            if(!strcmp("-segmented", argv[i])) {
                pContext->output.segmented = true; fprintf(stderr, "Segmented TS output\n");
            }
            if(!strcmp("-output_stream", argv[i])) {
                pContext->output.file = false; fprintf(stderr, "Stream output type\n");
            }
            if(!strcmp("-fnrt", argv[i])) {
                pContext->vpipes = NEXUS_NUM_VIDEO_ENCODERS;
                if(i+1 < argc) {
                    if(!strcmp("-vpipes", argv[i+1])) {
                        i++;
                        pContext->vpipes = strtoul(argv[++i], NULL, 0);
                    }
                }
                fprintf(stderr, "FNRT mode video pipes num = %u\n", pContext->vpipes);
            }
        }
        if(!pContext->input.probe) {
            config_xcoder_context(pContext);/* transcoder 0 context user settings */
        }
    }
    return 1;
}

void printInputStatus(BTST_Transcoder_t  *pContext)
{
    BXCode_InputStatus inputStatus;
    unsigned i;
    BXCode_GetInputStatus(pContext->hBxcode, &inputStatus);
    if((pContext->input.type != BXCode_InputType_eHdmi) && (pContext->input.type != BXCode_InputType_eImage)) {
        for(i=0; i<pContext->input.numAudios && pContext->input.enableAudio; i++) {
            printf("BXCode[%d] input Audio[%u] Status:\n", pContext->id, i);
            printf("----------------------\n");
            printf("data buffer depth     = %u\n", inputStatus.audioDecoder[i].fifoDepth);
            printf("data buffer size      = %u\n", inputStatus.audioDecoder[i].fifoSize);
            printf("queued frames         = %u\n", inputStatus.audioDecoder[i].queuedFrames);
            printf("numDecoded count      = %u\n", inputStatus.audioDecoder[i].framesDecoded);
            printf("numDummyFrames        = %u\n", inputStatus.audioDecoder[i].dummyFrames);
            printf("numFifoOverflows      = %u\n", inputStatus.audioDecoder[i].numFifoOverflows);
            printf("numFifoUnderflows     = %u\n", inputStatus.audioDecoder[i].numFifoUnderflows);
            printf("current PTS (45KHz)   = 0x%x\n", inputStatus.audioDecoder[i].pts);
            printf("PTS error count       = %u\n", inputStatus.audioDecoder[i].ptsErrorCount);
        }
        if(pContext->input.enableVideo) {
            printf("\nBXCode[%u] input Video Decoder Status:\n", pContext->id);
            printf("----------------------\n");
            printf("data buffer depth     = %u\n", inputStatus.videoDecoder.fifoDepth);
            printf("data buffer size      = %u\n", inputStatus.videoDecoder.fifoSize);
            printf("queued frames         = %u\n", inputStatus.videoDecoder.queueDepth);
            printf("numDecoded count      = %u\n", inputStatus.videoDecoder.numDecoded);
            printf("numDisplayed          = %u\n", inputStatus.videoDecoder.numDisplayed);
            printf("numDecodeDrops        = %u\n", inputStatus.videoDecoder.numDecodeDrops);
            printf("numDisplayDrops       = %u\n", inputStatus.videoDecoder.numDisplayDrops);
            printf("numDecodeOverflows    = %u\n", inputStatus.videoDecoder.numDecodeOverflows);
            printf("numDisplayUnderflows  = %u\n", inputStatus.videoDecoder.numDisplayUnderflows);
            printf("current PTS (45KHz)   = 0x%x\n", inputStatus.videoDecoder.pts);
            printf("PTS error count       = %u\n", inputStatus.videoDecoder.ptsErrorCount);
        }
    }
}

void printOutputStatus(BTST_Transcoder_t  *pContext)
{
    unsigned i;
    BXCode_OutputStatus outputStatus;
    BXCode_GetOutputStatus(pContext->hBxcode, &outputStatus);
    printf("\nBXCode[%u] output Status:\n", pContext->id);
    printf("----------------------Mux\n");
    printf("Mux stream duration          = %d (ms)\n", outputStatus.mux.duration);
    printf("TS stream recorded data      = %#x (bytes)\n", (uint32_t)outputStatus.mux.recpumpStatus.data.bytesRecorded);
    printf("TS stream fifo depth/size    = %#x/%#x (bytes)\n", outputStatus.mux.recpumpStatus.data.fifoDepth, outputStatus.mux.recpumpStatus.data.fifoSize);
    printf("----------------------Video\n");
    printf("Video encoder fifo depth/size= %#x/%#x\n", outputStatus.video.data.fifoDepth, outputStatus.video.data.fifoSize);
    printf("Video error count            = %u\n", outputStatus.video.errorCount);
    printf("Video error flags            = %#x\n", outputStatus.video.errorFlags);
    printf("Video event flags            = %u\n", outputStatus.video.eventFlags);
    printf("picture drops due to error   = %u\n", outputStatus.video.picturesDroppedErrors);
    printf("picture drops due to FRC     = %u\n", outputStatus.video.picturesDroppedFRC);
    printf("pictures Encoded             = %u\n", outputStatus.video.picturesEncoded);
    printf("pictures Received            = %u\n", outputStatus.video.picturesReceived);
    printf("picture Id Last Encoded      = 0x%x\n", outputStatus.video.pictureIdLastEncoded);
    printf("pictures per second          = %u\n", outputStatus.video.picturesPerSecond);
    printf("----------------------Audio\n");
    printf("Num of audio PIDs            = %u\n", outputStatus.numAudios);
    for(i=0; i<pContext->input.numAudios && pContext->input.enableAudio; i++) {
        printf("Audio[%u] frames count        = %u\n", i, outputStatus.audio[i].numFrames);
        printf("Audio[%u] error frames count  = %u\n", i, outputStatus.audio[i].numErrorFrames);
        printf("AV sync[%u] error             = %.1f (ms)\n", i, outputStatus.avSyncErr[i]);
        printf("----------------------\n");
    }
}

void printMenu(void)
{
    printf("Menu:\n");
    printf(" 0) xcode loopback player control\n");
    printf(" 1) change video encoder resolution\n");
    printf(" 2) change video encoder bitrate\n");
    printf(" 3) change video encoder frame rate\n");
    printf("12) change xcode aspect ratio correction mode\n");
    printf("14) Get BXCode status\n");
    printf("15) Select BXCode context to config\n");
    printf("16) Start new RAP\n");
    printf("17) change GOP structure setting\n");
    printf("20) Open/Start selected transcoder\n");
    printf("21) Stop/Close selected transcoder\n");
    printf("26) Change audio encoder bitrate\n");
    printf("27) Toggle audio encoder sample rate to 32KHz\n");
    printf("28) Open gfx window\n");
    printf("29) Close gfx window\n");
    printf("33) Toggle CBR/VBR encode on current xcode context\n");
    printf("34) Toggle 3D/2D encode on current xcode context\n");
    printf("35) Toggle 3D/2D decode input type on current xcode context\n");
    printf("36) Video encoder memory config on current xcode context\n");
    printf("37) Enable/disable audio or video output\n");
    printf("38) Configure selected BXCode output type for next start\n");
    printf("39) Enable/disable BXCode input file loop mode\n");
    printf("40) Enable/disable BXCode 6xaudio PIDs\n");
    printf("41) Enable/disable BXCode video input\n");
    printf("42) Enable/disable BXCode video cc user data\n");
    printf("50) random dynamic resolution test\n");
    printf("51) random dynamic bitrate test\n");
    printf("52) Stop stream mux to test encoder overflow recovery\n");
    printf("53) back to back NRT stop/start test\n");
    printf("99) change DEBUG module setting\n");
    printf("100) sleep\n");
}

void keyHandler( BTST_Context_t *pContext )
{
    char key[256], *pValue;
    unsigned i, choice;
    int rcInput;
    BTST_Transcoder_t *pTranscoder = &pContext->xcodeContext[pContext->selectedXcodeContextId];

    printMenu();
    while (g_keyReturn != 'q')
    {
        fd_set rfds;
        struct timeval tv;
        int retval;

        printf("\nEnter 'h' to print menu\n");
        printf("Enter 'q' to quit\n\n");

        do {
            /* Watch stdin (fd 0) to see when it has input. */
            FD_ZERO(&rfds);
            FD_SET(0, &rfds);

            /* Wait up to 100 miliseconds. */
            tv.tv_sec = 0;
            tv.tv_usec = 100000;

            retval = select(1, &rfds, NULL, NULL, &tv);
            if(retval<0) {perror("### select returns"); goto done;}
            else if(retval) BDBG_MSG(("Data is available"));
        } while ( g_keyReturn != 'q' && retval <= 0 );

        if ( g_keyReturn == 'q' )
        {
            break;
        }

        rcInput = scanf("%s", key);
        if(pContext->scriptMode && rcInput == EOF)
        {
            BKNI_Sleep(1000*pContext->scriptModeSleepTime);
            g_keyReturn = 'q';
            break;
        }
        if(!strcmp(key, "q") || !strcmp(key, "quit") || g_keyReturn == 'q')
        {
            g_keyReturn = 'q';
            break;
        }
        else if(!strcmp(key, "h") ||!strcmp(key, "help")) {
            printMenu();
            continue;
        }
        pValue = strstr(key, "=");
        choice = strtoul(pValue? (pValue+1) : key, NULL, 0);
        if(!pTranscoder->input.enableVideo && (choice!=14) && (choice!=20) && (choice!=21)
           && ((choice<24) || (choice>27)) && ((choice<37) || (choice>41)) && (choice<99)) continue;
        if(!pTranscoder->started && ((choice!=20) && (choice!=99) && (choice!=15) && (choice<38 || choice>=50))) {
            printf("BXCode[%u] already stopped.\n", pTranscoder->id);
            continue;
        }
        B_Mutex_Lock(pTranscoder->mutexStarted);
        switch(choice)
        {
            case 0: /* xcoder loopback trick play control */
                if(pContext->loopbackXcodeId != (unsigned)(-1)) {
                    B_Mutex_Unlock(pTranscoder->mutexStarted);
                    loopbackPlayer(pContext);
                    B_Mutex_Lock(pTranscoder->mutexStarted);
                } else {
                    printf("An encoder display took away display 0 so loopback player is unavailable!\n");
                }
                break;
            case 1: /* resolution change */
                printf("xcode resolution change:\n");
                BXCode_GetSettings(pTranscoder->hBxcode, &pTranscoder->settings);

                /* typically resolution change combined with bit rate change */
                printf("Do you want to change bit rate synchronously as well? 0)No 1)Yes\n");
                choice = getValue(key);
                if(choice)
                {
                    printf("Bit rate(bps) change from %u to:\n", pTranscoder->output.vBitrate);
                    pTranscoder->output.vBitrate = getValue(key);

                    /* turn on the synchronous change feature first before resolution/bitrate change! */
                    pTranscoder->settings.video.encoder.bitrateMax = pTranscoder->output.vBitrate;
                }

                printf("Resolution width & height\n");
                pTranscoder->settings.video.width = getValue(key);
                pTranscoder->settings.video.height = getValue(key);
                pTranscoder->output.videoFormat.width = pTranscoder->settings.video.width;
                pTranscoder->output.videoFormat.height = pTranscoder->settings.video.height;
                printf("Aspect Ratio:\n");
                print_value_list(g_displayAspectRatioStrs);
                pTranscoder->settings.video.aspectRatio = getNameValue(key, g_displayAspectRatioStrs);
                pTranscoder->output.videoFormat.aspectRatio = pTranscoder->settings.video.aspectRatio;
                if(NEXUS_DisplayAspectRatio_eSar == pTranscoder->settings.video.aspectRatio)
                {
                    printf("Please enter Sample Aspect Ratio X and Y: \n");
                    pTranscoder->settings.video.sampleAspectRatio.x = getValue(key);
                    pTranscoder->settings.video.sampleAspectRatio.y = getValue(key);
                    pTranscoder->output.videoFormat.sampleAspectRatio.x = pTranscoder->settings.video.sampleAspectRatio.x;
                    pTranscoder->output.videoFormat.sampleAspectRatio.y = pTranscoder->settings.video.sampleAspectRatio.y;
                }
                /* resolution change typically could come with bit rate change; separate for now */
                BXCode_SetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                break;
            case 2: /* bitrate change */
                BXCode_GetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                printf("Bit rate(bps) change from %u to:\n", pTranscoder->settings.video.encoder.bitrateMax);
                pTranscoder->settings.video.encoder.bitrateMax = getValue(key);
                pTranscoder->output.vBitrate = pTranscoder->settings.video.encoder.bitrateMax;
                BXCode_SetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                break;
            case 3: /* frame rate change */
                BXCode_GetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                printf("frame rate change from (%u) %s to:\n", pTranscoder->settings.video.encoder.frameRate, lookup_name(g_videoFrameRateStrs, pTranscoder->settings.video.encoder.frameRate));
                print_value_list(g_videoFrameRateStrs);
                choice = getNameValue(key, g_videoFrameRateStrs);
                pTranscoder->settings.video.encoder.frameRate = (NEXUS_VideoFrameRate)choice;
                pTranscoder->output.framerate = pTranscoder->settings.video.encoder.frameRate;
                printf("Current encode variable frame rate mode: %d\n", pTranscoder->settings.video.encoder.variableFrameRate);
                printf("Change encode variable frame rate mode: [0=N/1=Y]\n");
                pTranscoder->settings.video.encoder.variableFrameRate = getValue(key);
                pTranscoder->output.variableFramerate = pTranscoder->settings.video.encoder.variableFrameRate;
                BXCode_SetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                break;
            case 12: /* change xcoder aspect ratio correction mode */
                printf("Please select transcoder aspect ratio correction mode:\n");
                print_value_list(g_contentModeStrs);
                BXCode_GetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                pTranscoder->settings.video.windowSettings.contentMode = pTranscoder->output.contentMode = getNameValue(key, g_contentModeStrs);
                BXCode_SetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                break;
            case 14: /* get bxcode status */
                printInputStatus(&pContext->xcodeContext[pContext->selectedXcodeContextId]);
                printOutputStatus(&pContext->xcodeContext[pContext->selectedXcodeContextId]);
                break;
            case 15: /* select xcoder context */
                printf("Please select Xcoder context to configure [0 ~ %d]:\n", NEXUS_NUM_VIDEO_ENCODERS-1);
                pContext->selectedXcodeContextId = getValue(key);
                pContext->selectedXcodeContextId = (pContext->selectedXcodeContextId > NEXUS_NUM_VIDEO_ENCODERS-1)
                    ? (NEXUS_NUM_VIDEO_ENCODERS-1):pContext->selectedXcodeContextId;
                B_Mutex_Unlock(pTranscoder->mutexStarted);
                pTranscoder = &pContext->xcodeContext[pContext->selectedXcodeContextId];
                B_Mutex_Lock(pTranscoder->mutexStarted);
                break;
            case 17: /* dynamic change GOP structure */
                BXCode_GetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                printf("Context%d current GOP structure: duration = %u, num of P = %u, num of B = %u, %s GOP%s.\n",
                    pTranscoder->id,
                    pTranscoder->settings.video.encoder.streamStructure.duration, pTranscoder->settings.video.encoder.streamStructure.framesP,
                    pTranscoder->settings.video.encoder.streamStructure.framesB, pTranscoder->settings.video.encoder.streamStructure.openGop?"open":"close",
                    pTranscoder->settings.video.encoder.streamStructure.newGopOnSceneChange? ", start new GOP on scene change":"");
                printf("Change GOP to fixed duration? [0: no; x: x ms]\n");
                choice = getValue(key);
                pTranscoder->settings.video.encoder.streamStructure.duration = pTranscoder->output.gopDuration = choice;
                if(choice == 0) {
                    printf("Enable new GOP on scene change? [0=n/1=y] ");
                    pTranscoder->settings.video.encoder.streamStructure.newGopOnSceneChange = pTranscoder->output.newGopOnSceneChange = getValue(key);
                }
                printf("Change num of P to: ");
                pTranscoder->settings.video.encoder.streamStructure.framesP = pTranscoder->output.gopFramesP = getValue(key);
                printf("Change num of B to: ");
                choice = getValue(key);
                pTranscoder->settings.video.encoder.streamStructure.framesB = pTranscoder->output.gopFramesB = choice;
                if(choice) {
                    printf("Enable open GOP? [0=n/1=y] ");
                    pTranscoder->settings.video.encoder.streamStructure.openGop = pTranscoder->output.openGop = getValue(key);
                }
                BXCode_SetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                break;
            case 20: /* start selected xcoder context */
                if(pTranscoder->started) {
                    printf("BXCode %u already started.\n", pTranscoder->id);
                    break;
                }
                printf("To start the xcoder%d...\n", pTranscoder->id);
                if(!pTranscoder->custom) config_xcoder_context(pTranscoder);
                else {
                    printf("Reconfigure the transcoder%u? [1=y/0=n] ", pTranscoder->id);
                    choice = getValue(key);
                    if(choice) config_xcoder_context(pTranscoder);
                }
                if(pContext->loopbackPlayer) {
                    if(((pContext->activeXcodeCount+1 >= NEXUS_NUM_VIDEO_DECODERS) ||
                        (0 == get_display_index(pTranscoder->id))) && pContext->loopbackStarted) {
                        xcode_loopback_shutdown(pContext);
                        pContext->loopbackXcodeId = (unsigned)-1;/* disable loopback */
                    }
                }
                /* open & start transcoder */
                bringup_transcode(pTranscoder);
                if(pContext->loopbackPlayer && (pContext->selectedXcodeContextId == pContext->loopbackXcodeId) && !pContext->loopbackStarted) {
                        /* need to have both free decoder and display to set up loopback player */
                        if(0 != get_display_index(pTranscoder->id) &&
                            (pContext->activeXcodeCount < NEXUS_NUM_VIDEO_DECODERS)) {
                            xcode_loopback_setup(pContext);
                        } else {
                            pContext->loopbackXcodeId = -1;/* disable loopback */
                        }
                }
                break;
            case 21: /* stop selected xcoder context */
                printf("To stop xcoder%d\n", pTranscoder->id);
                /* bringdown loopback path */
                if(pContext->loopbackPlayer && pContext->selectedXcodeContextId == pContext->loopbackXcodeId && pContext->loopbackStarted) {
                    xcode_loopback_shutdown(pContext);
                }
                BDBG_MSG(("activeXcodes: %d, loopbackId: %d", pContext->activeXcodeCount, pContext->loopbackXcodeId));
                /* if the encoder is shutdown with display matching with loopback display id, loopback is available now */
                if(pContext->loopbackPlayer && (0 == get_display_index(pTranscoder->id) ||
                    (pContext->activeXcodeCount <= NEXUS_NUM_VIDEO_DECODERS && (pContext->loopbackXcodeId==(unsigned)-1)))) {
                    pContext->loopbackXcodeId = 0;/* was -1 */
                }
                /* stop & close transcoder */
                shutdown_transcode(pTranscoder);
                if(pContext->autoQuit) {
                    int i;
                    for(i=0; i<NEXUS_NUM_VIDEO_ENCODERS; i++) {
                        if(pContext->xcodeContext[i].started) break;
                    }
                    /* when all contexts are stopped, quit the test */
                    if(i == NEXUS_NUM_VIDEO_ENCODERS) {
                        g_keyReturn = 'q';
                        B_Mutex_Unlock(pTranscoder->mutexStarted);
                        goto done;
                    }
                }
                break;
            case 26:
                if(pTranscoder->output.audioEncode[0])
                {
                    BXCode_GetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                    switch(pTranscoder->output.audioCodec[0]) {
                    case NEXUS_AudioCodec_eAac:
                        printf("BXCode[%u] AAC audio encoder bitrate = %u bps.\nPlease enter the new bitrate:\n", pTranscoder->id, pTranscoder->settings.audio[0].codec.codecSettings.aac.bitRate);
                        pTranscoder->settings.audio[0].codec.codecSettings.aac.bitRate = getValue(key);
                        break;
                    case NEXUS_AudioCodec_eAacPlus:
                    case NEXUS_AudioCodec_eAacPlusAdts:
                        printf("Context%d AAC+ audio encoder bitrate = %u bps.\nPlease enter the new bitrate:\n", pTranscoder->id, pTranscoder->settings.audio[0].codec.codecSettings.aacPlus.bitRate);
                        pTranscoder->settings.audio[0].codec.codecSettings.aacPlus.bitRate = getValue(key);
                        break;
                    case NEXUS_AudioCodec_eMp3:
                        printf("Context%d MP3 audio encoder bitrate = %u bps.\nPlease enter the new bitrate:\n", pTranscoder->id, pTranscoder->settings.audio[0].codec.codecSettings.mp3.bitRate);
                        pTranscoder->settings.audio[0].codec.codecSettings.mp3.bitRate = getValue(key);
                        break;
                    default:
                        printf("Unsupported audio encoder codec %d!\n", pTranscoder->output.audioCodec[0]);
                        break;
                    }
                    BXCode_SetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                }
                else {
                    printf("BXCode[%u] audio encoder is disabled!\n", pTranscoder->id);
                }
                break;
            case 27:
                if(pTranscoder->output.audioEncode[0])
                {
                    BXCode_GetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                    switch(pTranscoder->output.audioCodec[0]) {
                    case NEXUS_AudioCodec_eAac:
                        printf("Context%d AAC audio encoder sample rate = %u KHz.\n", pTranscoder->id, pTranscoder->settings.audio[0].codec.codecSettings.aac.sampleRate/1000);
                        pTranscoder->settings.audio[0].codec.codecSettings.aac.sampleRate = (pTranscoder->settings.audio[0].codec.codecSettings.aac.sampleRate == 32000)? 48000 : 32000;
                        printf("changed to %u KHz.\n", pTranscoder->settings.audio[0].codec.codecSettings.aac.sampleRate/1000);
                        break;
                    case NEXUS_AudioCodec_eAacPlus:
                    case NEXUS_AudioCodec_eAacPlusAdts:
                        printf("Context%d AAC+ audio encoder sample rate = %u Hz.\n", pTranscoder->id, pTranscoder->settings.audio[0].codec.codecSettings.aacPlus.sampleRate);
                        pTranscoder->settings.audio[0].codec.codecSettings.aacPlus.sampleRate = (pTranscoder->settings.audio[0].codec.codecSettings.aacPlus.sampleRate == 32000)? 48000 : 32000;
                        printf("changed to %u KHz.\n", pTranscoder->settings.audio[0].codec.codecSettings.aacPlus.sampleRate/1000);
                        break;
                    default:
                        printf("Unsupported audio encoder codec %d for sample rate conversion!\n", pTranscoder->output.audioCodec[0]);
                        break;
                    }
                    BXCode_SetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                }
                else {
                    printf("Context%d audio encoder is not enabled! No SRC.\n", pTranscoder->id);
                }
                break;
            case 28:
                printf("Open gfx window 100x100 at (0, 0)...\n");
                open_gfx(pTranscoder);
                break;
            case 29:
                printf("Close gfx window...\n");
                close_gfx(pTranscoder);
                break;
            case 33:
                printf("BXCode[%u] Toggle VBR video encode %s to %s.\n", pTranscoder->id, pTranscoder->output.targetBitrate?"on":"off", pTranscoder->output.targetBitrate?"off":"on");
                pTranscoder->output.targetBitrate = pTranscoder->output.targetBitrate? 0 : 1;
                break;
            case 34:
                printf("BXCode[%u] Toggle 3D/2D video encode %s to %s.\n", pTranscoder->id, pTranscoder->output.orientation?"3D":"2D", pTranscoder->output.orientation?"2D":"3D");
                pTranscoder->output.orientation = pTranscoder->output.orientation? NEXUS_VideoOrientation_e2D : NEXUS_VideoOrientation_e3D_LeftRight;
                BXCode_GetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                if(pTranscoder->output.orientation)
                {
                    printf("\n 3D orientation:\n"
                        " (%2d) Left/Right\n"
                        " (%2d) Over/Under\n",
                        (NEXUS_VideoOrientation_e3D_LeftRight),
                        (NEXUS_VideoOrientation_e3D_OverUnder));
                    pTranscoder->output.orientation = getNameValue(key, g_videoOrientation);
                    pTranscoder->settings.video.orientation         = pTranscoder->output.orientation;
                }
                BXCode_SetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                break;
            case 36:
                printf("Video encoder to support progressive-only formats? [1=Y/0=N]\n");
                pTranscoder->output.videoEncoderMemConfig.progressiveOnly = getValue(key);
                printf("Video encoder max width: \n");
                pTranscoder->output.videoEncoderMemConfig.maxWidth = getValue(key);
                printf("Video encoder max height: \n");
                pTranscoder->output.videoEncoderMemConfig.maxHeight = getValue(key);
                /* update encoder startSettings memory bound */
                if(pTranscoder->output.videoEncoderMemConfig.maxWidth) {
                    pTranscoder->startSettings.output.video.encoder.bounds.inputDimension.max.width = pTranscoder->output.videoEncoderMemConfig.maxWidth;
                }
                if(pTranscoder->output.videoEncoderMemConfig.maxHeight) {
                    pTranscoder->startSettings.output.video.encoder.bounds.inputDimension.max.height = pTranscoder->output.videoEncoderMemConfig.maxHeight;
                }
                break;
            case 37:
                printf("Enable video channel? [1=Y/0=N]\n");
                pTranscoder->settings.video.enabled = getValue(key);
                for(i=0; i<pTranscoder->input.numAudios; i++) {
                    printf("Enable audio[%u] channel? [1=Y/0=N]\n", i);
                    pTranscoder->settings.audio[i].enabled = getValue(key);
                }
                BXCode_SetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                break;
            case 38: /* set pTranscoder's output type for next start */
                print_list_option("BXCode output type", g_outputTypeStrs);
                pTranscoder->output.type = getNameValue(key, g_outputTypeStrs);
                if(pTranscoder->output.type == BXCode_OutputType_eTs) {
                    printf("BXCode[%u] segmented output? [1=Y/0=N]\n", pTranscoder->id);
                    pTranscoder->output.segmented = getValue(key);
                    printf("BXCode[%u] file output? [1=Y/0=N]\n", pTranscoder->id);
                    pTranscoder->output.file = getValue(key);
                } else if(pTranscoder->output.type == BXCode_OutputType_eEs) {
                    pTranscoder->output.file = false;
                } else {/* MP4 file */
                    pTranscoder->output.file = true;
                }
                break;
            case 39: /* set pTranscoder's input file loop mode */
                printf("Enable BXCode[%u] input file loop mode? [1=Y/0=N]\n", pTranscoder->id);
                pTranscoder->loop = getValue(key);
                break;
            case 99: /* debug module setting */
#if BDBG_DEBUG_BUILD
{
                char achName[256];
                printf("Please enter the debug module name:\n");
                strcpy(achName, getString(key));
                printf("(%d)Trace (%d)Message (%d)Warning (%d)Error\n",
                    BDBG_eTrace, BDBG_eMsg, BDBG_eWrn, BDBG_eErr);
                printf("Which debug level do you want to set it to? ");
                BDBG_SetModuleLevel(achName, getValue(key));
}
#endif
                break;
            case 40: /* enable/disable 6xaudio PIDs */
                printf("Enable BXCode[%u] 6xaudio passthrough? [1=Y/0=N]\n", pTranscoder->id);
                pTranscoder->input.numAudios = getValue(key)? 6:1;
                break;
            case 41: /* enable/disable video input */
                printf("Enable BXCode[%u] video input? [1=Y/0=N]\n", pTranscoder->id);
                pTranscoder->input.enableVideo = getValue(key);
                break;
            case 42: /* enable/disable video cc user data */
                printf("Enable BXCode[%u] video closed caption user data? [1=Y/0=N]\n", pTranscoder->id);
                pTranscoder->output.ccUserdata = getValue(key);
                break;
            case 50: /* random dynamic resolution change */
            {
                unsigned minW=64, minH=64, maxW=1920, maxH=1080;
                printf("random resolution change...\n");
                printf("max WxH[%ux%u]:\n", maxW, maxH);
                maxW = getValue(key);
                maxH = getValue(key);
                printf("How many times?\n"); choice = getValue(key);
                while(choice--) {
                    BXCode_GetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                    pTranscoder->settings.video.width = (rand()%(maxW+1))& (~0xf);/* MB aligned*/
                    if(pTranscoder->settings.video.width < minW)
                        pTranscoder->settings.video.width += minW;
                    pTranscoder->settings.video.height = rand()%(maxH+1) & (~0xf);
                    if(pTranscoder->settings.video.height < minH)
                        pTranscoder->settings.video.height += minH;
                    pTranscoder->settings.video.aspectRatio = NEXUS_DisplayAspectRatio_eSar;
                    pTranscoder->settings.video.sampleAspectRatio.x = 1;
                    pTranscoder->settings.video.sampleAspectRatio.y = 1;
                    printf("resolution loop %d: %ux%up30\n", choice, pTranscoder->settings.video.width, pTranscoder->settings.video.height);
                    BXCode_SetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                    BKNI_Sleep(3000); /* 3sec per loop */
                }
                /* resolution change typically could come with bit rate change; separate for now */
                break;
            }
            case 51: /* random dynamic bitrate change */
            {
                unsigned minBitrate=10000, maxBitrate=30000000;
                printf("random bitrate change...\n");
                printf("Bitrate [min, max]: %u ~ %u\n", minBitrate, maxBitrate);
                minBitrate = getValue(key);
                maxBitrate = getValue(key);
                printf("How many times?\n"); choice = getValue(key);
                while(choice--) {
                    BXCode_GetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                    pTranscoder->settings.video.encoder.bitrateMax = rand()%maxBitrate;
                    if(pTranscoder->settings.video.encoder.bitrateMax < minBitrate) pTranscoder->settings.video.encoder.bitrateMax += minBitrate;
                    BXCode_SetSettings(pTranscoder->hBxcode, &pTranscoder->settings);
                    BKNI_Sleep(3000); /* 3sec per loop */
                }
                /* resolution change typically could come with bit rate change; separate for now */
                break;
            }
            case 53: /* back to back stop/start test */
                printf("back to back stop/start test...\n");
                printf("How many times?\n"); choice = getValue(key);
                while(choice--) {
                    /* bringdown loopback path */
                    if(pContext->loopbackPlayer && pContext->selectedXcodeContextId == pContext->loopbackXcodeId && pContext->loopbackStarted) {
                        xcode_loopback_shutdown(pContext);
                    }
                    /* stop & close transcoder */
                    shutdown_transcode(pTranscoder);
                    printf("start loop %d\n", choice);
                    /* open & start transcoder */
                    bringup_transcode(pTranscoder);
                    if(pContext->loopbackPlayer && (pContext->selectedXcodeContextId == pContext->loopbackXcodeId) && !pContext->loopbackStarted &&
                       (0 != get_display_index(pTranscoder->id))) {
                        xcode_loopback_setup(pContext);
                    }
                    BKNI_Sleep(10000); /* 10 sec per loop */
                }
                /* resolution change typically could come with bit rate change; separate for now */
                break;
            case 100:
            {
                printf("How many seconds to sleep?\n");
                BKNI_Sleep(1000*getValue(key));
                break;
            }
            default:
                break;
        }
        B_Mutex_Unlock(pTranscoder->mutexStarted);
    }
done:
    BDBG_WRN(("Key handler exiting.."));
    if(g_testContext.doneEvent) {
        BKNI_SetEvent(g_testContext.doneEvent);
    }
}
#endif
