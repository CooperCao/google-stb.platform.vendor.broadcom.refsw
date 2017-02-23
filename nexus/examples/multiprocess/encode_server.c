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
 *****************************************************************************/
#if NEXUS_HAS_STREAM_MUX
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_platform_server.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_decoder.h"
#include "nexus_video_input.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_simple_video_decoder_server.h"
#include "nexus_simple_audio_decoder_server.h"
#include "nexus_simple_encoder_server.h"
#include "nexus_stc_channel.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(encode_server);

static int simple_encoder_create(NEXUS_AudioDecoderHandle audioDecoder);
static void simple_encoder_destroy(void);

static void print_usage(void)
{
    printf(
    "usage: nexus encode_server [--help|-h] [-unprotected]\n"
    "options:\n"
    "  --help|-h      print this help screen\n"
    "  -timeout X     exit after X seconds. default is to prompt for user.\n"
    );
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformStartServerSettings serverSettings;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderHandle simpleVideoDecoder;
    NEXUS_SimpleVideoDecoderServerHandle videoServer;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_SimpleAudioDecoderHandle simpleAudioDecoder;
    NEXUS_SimpleAudioDecoderServerHandle audioServer;
    NEXUS_Error rc;
    int curarg = 1;
    NEXUS_ClientMode clientMode = NEXUS_ClientMode_eProtected;
    unsigned timeout = 0;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = strtoul(argv[++curarg], NULL, 0);
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);

    /* create simple video decoder */
    {
        NEXUS_SimpleVideoDecoderServerSettings settings;
        videoServer = NEXUS_SimpleVideoDecoderServer_Create();
        NEXUS_SimpleVideoDecoder_GetDefaultServerSettings(&settings);
        settings.videoDecoder = videoDecoder;
        settings.stcIndex = 0;
        simpleVideoDecoder = NEXUS_SimpleVideoDecoder_Create(videoServer, 0, &settings);
    }

    /* create audio decoder. any index will work, but use 2 to stay out of main decoder's way. */
    audioDecoder = NEXUS_AudioDecoder_Open(2, NULL);

    /* create simple audio decoder */
    {
        NEXUS_SimpleAudioDecoderServerSettings settings;
        audioServer = NEXUS_SimpleAudioDecoderServer_Create();
        NEXUS_SimpleAudioDecoder_GetDefaultServerSettings(&settings);
        settings.primary = audioDecoder;
        simpleAudioDecoder = NEXUS_SimpleAudioDecoder_Create(audioServer, 0, &settings);
        NEXUS_SimpleAudioDecoder_SetStcIndex(audioServer, simpleAudioDecoder, 0); /* same as video */
    }

    NEXUS_Platform_GetDefaultStartServerSettings(&serverSettings);
    serverSettings.allowUnauthenticatedClients = true; /* client is written this way */
    serverSettings.unauthenticatedConfiguration.mode = clientMode;
    serverSettings.unauthenticatedConfiguration.heap[1] = platformConfig.heap[0];
    serverSettings.unauthenticatedConfiguration.resources.simpleVideoDecoder.id[0] = 0;
    serverSettings.unauthenticatedConfiguration.resources.simpleVideoDecoder.total = 1;
    serverSettings.unauthenticatedConfiguration.resources.simpleAudioDecoder.id[0] = 0;
    serverSettings.unauthenticatedConfiguration.resources.simpleAudioDecoder.total = 1;
    rc = NEXUS_Platform_StartServer(&serverSettings);
    BDBG_ASSERT(!rc);

    simple_encoder_create(audioDecoder);

    if (!timeout) {
        printf("Press ENTER to shutdown encode_server\n");
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

    simple_encoder_destroy();

    NEXUS_SimpleVideoDecoder_Destroy(simpleVideoDecoder);
    NEXUS_SimpleVideoDecoderServer_Destroy(videoServer);
    NEXUS_SimpleAudioDecoder_Destroy(simpleAudioDecoder);
    NEXUS_SimpleAudioDecoderServer_Destroy(audioServer);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_Platform_Uninit();
    return 0;
}

static struct {
    NEXUS_SimpleEncoderServerSettings settings;
    NEXUS_SimpleEncoderServerHandle server;
    NEXUS_SimpleEncoderHandle client;
} g_encoder;

static int simple_encoder_create(NEXUS_AudioDecoderHandle audioDecoder)
{
    NEXUS_DisplaySettings displaySettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_AudioMixerSettings mixerSettings;
    unsigned i;
    int rc;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoEncoderCapabilities videoEncoderCap;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    g_encoder.server = NEXUS_SimpleEncoderServer_Create();
    g_encoder.client = NEXUS_SimpleEncoder_Create(g_encoder.server, 0);
    NEXUS_GetVideoEncoderCapabilities(&videoEncoderCap);

    NEXUS_SimpleEncoder_GetServerSettings(g_encoder.server, g_encoder.client, &g_encoder.settings);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    displaySettings.format = NEXUS_VideoFormat_e480p;
#else
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.frameRateMaster = NULL; /* disable frame rate tracking for now */
#endif
    g_encoder.settings.transcodeDisplayIndex = videoEncoderCap.videoEncoder[0].displayIndex;
    /* window is opened internally */

    g_encoder.settings.audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);

    g_encoder.settings.videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PLAYPUMPS;i++) {
        NEXUS_PlaypumpOpenSettings playpumpConfig;
        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
        playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
        playpumpConfig.numDescriptors = 64; /* set number of descriptors */
        playpumpConfig.streamMuxCompatible = true;
        g_encoder.settings.playpump[i] = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig);
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.stcIndex = 1; /* different from a/v */
    stcSettings.timebase = NEXUS_Timebase_e1; /* different from a/v */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42; /* ViCE2 requires 42-bit STC broadcast */
    g_encoder.settings.stcChannelTranscode = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
    g_encoder.settings.timebase = stcSettings.timebase;

    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
    mixerSettings.mixUsingDsp = true;
    g_encoder.settings.mixer = NEXUS_AudioMixer_Open(&mixerSettings);
    rc = NEXUS_AudioMixer_AddInput(g_encoder.settings.mixer,
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    BDBG_ASSERT(!rc);
    NEXUS_AudioMixer_GetSettings(g_encoder.settings.mixer, &mixerSettings);
    mixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    rc = NEXUS_AudioMixer_SetSettings(g_encoder.settings.mixer, &mixerSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]),
        NEXUS_AudioMixer_GetConnector(g_encoder.settings.mixer));
    BDBG_ASSERT(!rc);

    rc = NEXUS_SimpleEncoder_SetServerSettings(g_encoder.server, g_encoder.client, &g_encoder.settings);
    if (rc) {
        simple_encoder_destroy();
    }
    return rc;
}

static void simple_encoder_destroy(void)
{
    unsigned i;

    NEXUS_SimpleEncoder_Destroy(g_encoder.client);
    NEXUS_SimpleEncoderServer_Destroy(g_encoder.server);

    NEXUS_AudioMuxOutput_Destroy(g_encoder.settings.audioMuxOutput);
    NEXUS_AudioMixer_Close(g_encoder.settings.mixer);
    NEXUS_VideoEncoder_Close(g_encoder.settings.videoEncoder);
    for (i=0;i<NEXUS_SIMPLE_ENCODER_NUM_PLAYPUMPS;i++) {
        NEXUS_Playpump_Close(g_encoder.settings.playpump[i]);
    }
    NEXUS_StcChannel_Close(g_encoder.settings.stcChannelTranscode);
    memset(&g_encoder.settings, 0, sizeof(g_encoder.settings));
}
#else
#include <stdio.h>
int main(void) {
    printf("This application not supported on this platform.\n");
    return -1;
}
#endif
