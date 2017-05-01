/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_VIDEO_DECODER && NEXUS_HAS_AUDIO
#include "nexus_types.h"
#include "nexus_platform_server.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_decoder.h"
#include "nexus_video_input.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_mixer.h"
#include "nexus_simple_video_decoder_server.h"
#include "nexus_simple_audio_decoder_server.h"
#include "nexus_stc_channel.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(decode_server);

static void print_usage(void)
{
    printf(
    "usage: nexus decode_server [--help|-h]\n"
    "options:\n"
    "  --help|-h      print this help screen\n"
    "  -mode {verified|protected|untrusted} run clients in specified mode\n"
    "  -timeout X     exit after X seconds. default is to prompt for user.\n"
    );
}

#define NUM_VIDEO_DECODES 4

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformStartServerSettings serverSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderServerHandle videoServer;
    NEXUS_SimpleVideoDecoderHandle simpleVideoDecoder[NUM_VIDEO_DECODES];
    NEXUS_AudioDecoderHandle audioDecoder0, audioDecoder1;
    NEXUS_AudioMixerHandle audioMixer;
    NEXUS_SimpleAudioDecoderServerHandle audioServer;
    NEXUS_SimpleAudioDecoderHandle simpleAudioDecoder;
    NEXUS_AudioPlaybackHandle audioPlayback[2];
    NEXUS_SimpleAudioPlaybackHandle simpleAudioPlayback[2];
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_Error rc;
    int curarg = 1;
    NEXUS_ClientMode clientMode = NEXUS_ClientMode_eProtected;
    unsigned timeout = 0;
    unsigned i;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-mode") && argc>curarg+1) {
            ++curarg;
            if (!strcmp(argv[curarg], "verified")) {
                clientMode = NEXUS_ClientMode_eVerified;
            }
            else if (!strcmp(argv[curarg], "protected")) {
                clientMode = NEXUS_ClientMode_eProtected;
            }
            else if (!strcmp(argv[curarg], "untrusted")) {
                clientMode = NEXUS_ClientMode_eUntrusted;
            }
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.mode = NEXUS_ClientMode_eVerified;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* open display and connect video decoder to a window */
    display = NEXUS_Display_Open(0, NULL);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    rc = NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    BDBG_ASSERT(!rc);
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    rc = NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    BDBG_ASSERT(!rc);
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    BDBG_ASSERT(!rc);
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    window = NEXUS_VideoWindow_Open(display, 0);
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);

    /* create simple video decoder */
    videoServer = NEXUS_SimpleVideoDecoderServer_Create();
    for (i=0;i<NUM_VIDEO_DECODES;i++) {
        NEXUS_SimpleVideoDecoderServerSettings settings;
        NEXUS_SimpleVideoDecoder_GetDefaultServerSettings(&settings);
        settings.videoDecoder = videoDecoder;
        settings.window[0] = window; /* SimpleVideoDecoder will do the connection */
        settings.stcIndex = i;
        simpleVideoDecoder[i] = NEXUS_SimpleVideoDecoder_Create(videoServer, i, &settings);
    }
    /* SetCacheEnabled allows fcc_client to move between simple decoder instances without breaking the video/display connection. */
    NEXUS_SimpleVideoDecoderModule_SetCacheEnabled(videoServer, true);

    /* create audio decoders */
    audioDecoder0 = NEXUS_AudioDecoder_Open(0, NULL);
    audioDecoder1 = NEXUS_AudioDecoder_Open(1, NULL);
    audioMixer = NEXUS_AudioMixer_Open(NULL);
    {
        NEXUS_AudioPlaybackOpenSettings openSettings;
        NEXUS_AudioPlayback_GetDefaultOpenSettings(&openSettings);
        openSettings.heap = platformConfig.heap[0]; /* client must have access */
        audioPlayback[0] = NEXUS_AudioPlayback_Open(NEXUS_ANY_ID, &openSettings);
        audioPlayback[1] = NEXUS_AudioPlayback_Open(NEXUS_ANY_ID, &openSettings);
    }

    /* create simple audio decoder */
    audioServer = NEXUS_SimpleAudioDecoderServer_Create();
    {
        NEXUS_SimpleAudioDecoderServerSettings settings;
        NEXUS_AudioCapabilities audioCapabilities;
        unsigned i;

        NEXUS_SimpleAudioDecoder_GetDefaultServerSettings(&settings);
        settings.primary = audioDecoder0;
        settings.secondary = audioDecoder1;

        NEXUS_GetAudioCapabilities(&audioCapabilities);

        /* any mixed output must be connected outside of the simple decoder and are not configurable per codec.
        they are used for primary decoder PCM output as well as PCM playback. */
        rc = NEXUS_AudioMixer_AddInput(audioMixer, NEXUS_AudioDecoder_GetConnector(audioDecoder0, NEXUS_AudioDecoderConnectorType_eStereo));
        BDBG_ASSERT(!rc);
        rc = NEXUS_AudioMixer_AddInput(audioMixer, NEXUS_AudioPlayback_GetConnector(audioPlayback[0]));
        BDBG_ASSERT(!rc);
        rc = NEXUS_AudioMixer_AddInput(audioMixer, NEXUS_AudioPlayback_GetConnector(audioPlayback[1]));
        BDBG_ASSERT(!rc);

        if (audioCapabilities.numOutputs.dac > 0) {
            rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), NEXUS_AudioMixer_GetConnector(audioMixer));
            BDBG_ASSERT(!rc);
        }

        for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
            switch (i) {
#if 0
/* This causes audio_client's PCM playback to not be heard, to don't enable it.
If you enable this, HDMI/SPDIF will get compressed for ac3, mixed pcm for all other codecs. */
            case NEXUS_AudioCodec_eAc3:
                settings.hdmi.input[i] =
                settings.spdif.input[i] = NEXUS_AudioDecoder_GetConnector(audioDecoder1, NEXUS_AudioDecoderConnectorType_eCompressed);
                break;
#endif
            case NEXUS_AudioCodec_eUnknown: /* used for playback-only case */
            default:
                settings.hdmi.input[i] =
                settings.spdif.input[i] = NEXUS_AudioMixer_GetConnector(audioMixer);
                break;
            }
        }
#if NEXUS_NUM_HDMI_OUTPUTS
        settings.hdmi.outputs[0] = platformConfig.outputs.hdmi[0];
#endif

        if (audioCapabilities.numOutputs.spdif > 0) {
            settings.spdif.outputs[0] = platformConfig.outputs.spdif[0];
        }

        simpleAudioDecoder = NEXUS_SimpleAudioDecoder_Create(audioServer, 0, &settings);
    }

    /* create simple audio playback. it is linked to the audio decoder for timebase.
    but it is acquired separately by the app.
    if more than one SimpleAudioDecoder is created, the user should create an index scheme to separate them. */
    {
        NEXUS_SimpleAudioPlaybackServerSettings settings;

        NEXUS_SimpleAudioPlayback_GetDefaultServerSettings(&settings);
        settings.decoder = simpleAudioDecoder;
        settings.playback = audioPlayback[0];
        simpleAudioPlayback[0] = NEXUS_SimpleAudioPlayback_Create(audioServer, 0, &settings);

        NEXUS_SimpleAudioPlayback_GetDefaultServerSettings(&settings);
        settings.decoder = simpleAudioDecoder;
        settings.playback = audioPlayback[1];
        simpleAudioPlayback[1] = NEXUS_SimpleAudioPlayback_Create(audioServer, 1, &settings);
    }

    NEXUS_Platform_GetDefaultStartServerSettings(&serverSettings);
    serverSettings.allowUnauthenticatedClients = true; /* client is written this way */
    serverSettings.unauthenticatedConfiguration.mode = clientMode;
    serverSettings.unauthenticatedConfiguration.heap[1] = platformConfig.heap[0]; /* for purposes of example, allow access to main heap */
    for (i=0;i<NUM_VIDEO_DECODES;i++) {
        serverSettings.unauthenticatedConfiguration.resources.simpleVideoDecoder.id[i] = i;
    }
    serverSettings.unauthenticatedConfiguration.resources.simpleVideoDecoder.total = NUM_VIDEO_DECODES;
    serverSettings.unauthenticatedConfiguration.resources.simpleAudioDecoder.id[0] = 0;
    serverSettings.unauthenticatedConfiguration.resources.simpleAudioDecoder.total = 1;
    rc = NEXUS_Platform_StartServer(&serverSettings);
    BDBG_ASSERT(!rc);

    if (!timeout) {
        printf("Press ENTER to shutdown decode_server\n");
        getchar();
    }
    else {
        /* auto close */
        BKNI_Sleep(timeout*1000);
    }

    /* stop the server before closing resources that may be in use by clients.
    if it's an untrusted client, handle verification may fail the call. but a trusted client bypasses the
    check and could kill the server. */
    NEXUS_Platform_StopServer();

    NEXUS_SimpleVideoDecoderModule_SetCacheEnabled(videoServer, false);
    for (i=0;i<NUM_VIDEO_DECODES;i++) {
        NEXUS_SimpleVideoDecoder_Destroy(simpleVideoDecoder[i]);
    }
    NEXUS_SimpleVideoDecoderServer_Destroy(videoServer);
    NEXUS_SimpleAudioPlayback_Destroy(simpleAudioPlayback[0]);
    NEXUS_SimpleAudioPlayback_Destroy(simpleAudioPlayback[1]);
    NEXUS_SimpleAudioDecoder_Destroy(simpleAudioDecoder);
    NEXUS_SimpleAudioDecoderServer_Destroy(audioServer);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);

    NEXUS_AudioPlayback_Close(audioPlayback[0]);
    NEXUS_AudioPlayback_Close(audioPlayback[1]);
    NEXUS_AudioMixer_Close(audioMixer);
    NEXUS_AudioDecoder_Close(audioDecoder0);
    NEXUS_AudioDecoder_Close(audioDecoder1);

    NEXUS_Platform_Uninit();
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
