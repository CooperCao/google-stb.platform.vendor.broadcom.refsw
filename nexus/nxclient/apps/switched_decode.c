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
/* Nexus example app: single live a/v decode from a streamer */

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_hdmi_output.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include "nxserverlib.h"
#include "nxclient.h"
#include "bgui.h"

/**
Demostrate integration of NxClient with single-process application that has direct access to
display, decoder and output resources.

The display and graphics are never closed. Decoders and the entire audio filter graph must be closed
and re-opened on the transition, but this should be quick and seamless.
**/

static int  start_nxserver(const NEXUS_PlatformSettings *pPlatformSettings);
static void stop_nxserver(void);
static int open_decode(void);
static void close_decode(void);

#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14

static struct {
    BKNI_MutexHandle lock;
    nxserver_t server;
    NEXUS_DisplayHandle display[2];
    NEXUS_VideoWindowHandle window[2];
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_ParserBand parserBand;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    bgui_t gui;
} g_app;

static int start_local_graphics(void)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    struct bgui_settings gui_settings;

    /* main app must use NxClient for graphics */
    bgui_get_default_settings(&gui_settings);
    gui_settings.width = 720;
    gui_settings.height = 480;
    g_app.gui = bgui_create(&gui_settings);
    if (!g_app.gui) return -1;

    /* put a small red box in the upper right corner */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = bgui_surface(g_app.gui);
    fillSettings.color = 0;
    NEXUS_Graphics2D_Fill(bgui_blitter(g_app.gui), &fillSettings);
    fillSettings.color = 0xFFFF0000;
    fillSettings.rect.x = 600;
    fillSettings.rect.y = 20;
    fillSettings.rect.width = 50;
    fillSettings.rect.height = 50;
    NEXUS_Graphics2D_Fill(bgui_blitter(g_app.gui), &fillSettings);
    bgui_checkpoint(g_app.gui);
    bgui_submit(g_app.gui);
    return 0;
}

static void stop_local_graphics(void)
{
    if (g_app.gui) {
        bgui_destroy(g_app.gui);
        g_app.gui = NULL;
    }
}

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Bring up display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    g_app.display[0] = NEXUS_Display_Open(0, &displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    g_app.display[1] = NEXUS_Display_Open(1, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(g_app.display[0], NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(g_app.display[1], NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_SVIDEO_OUTPUTS
    NEXUS_Display_AddOutput(g_app.display[1], NEXUS_SvideoOutput_GetConnector(platformConfig.outputs.svideo[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(g_app.display[0], NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(g_app.display[0], &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(g_app.display[0], &displaySettings);
        }
    }
#endif

    rc = open_decode();
    BDBG_ASSERT(!rc);

    /* allow graphics-only nxclient apps right away. they are never interrupted. */
    rc = start_nxserver(&platformSettings);
    BDBG_ASSERT(!rc);

    start_local_graphics();

    while (1) {
        printf("Press ENTER to allow NxClient decode\n");
        getchar();

        /* single-process app gives up decode resources */
        close_decode();
        /* allow decode nxclient apps now */
        nxserverlib_allow_decode(g_app.server, true);

        printf("Press ENTER to disallow NxClient decode\n");
        getchar();

        /* TODO: notify client apps to gracefully shutdown */
        /* whether they shut down or not, yank all decode resources back from nxclient. */
        nxserverlib_allow_decode(g_app.server, false);
        /* single-process app can use decode resources again */
        rc = open_decode();
        BDBG_ASSERT(!rc);
    }

    stop_local_graphics();
    stop_nxserver();
    close_decode();

    NEXUS_Display_Close(g_app.display[0]);
    NEXUS_Display_Close(g_app.display[1]);
    NEXUS_Platform_Uninit();

    return 0;
}

/* simplistic scheme for allocating unique indices. a real scheme would vary the max index per type. */
#define MAX_INDEX 20
static bool g_indexAlloc[nxserverlib_index_type_max][MAX_INDEX];

static int alloc_index(void *callback_context, enum nxserverlib_index_type type, unsigned *pIndex)
{
    unsigned index;
    BDBG_ASSERT(callback_context == &g_app);
    for (index=0;index<MAX_INDEX;index++) {
        if (!g_indexAlloc[type][index]) {
            *pIndex = index;
            g_indexAlloc[type][index] = true;
            return 0;
        }
    }
    return -1;
}

static void free_index(void *callback_context, enum nxserverlib_index_type type, unsigned index)
{
    BDBG_ASSERT(callback_context == &g_app);
    if (index<MAX_INDEX) {
        g_indexAlloc[type][index] = false;
    }
}

static int start_nxserver(const NEXUS_PlatformSettings *pPlatformSettings)
{
    NEXUS_PlatformConfiguration platformConfig;
    struct nxserver_settings settings;
    int rc;
    int index;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateMutex(&g_app.lock);

    nxserver_get_default_settings(&settings);
    settings.lock = g_app.lock;

    index = nxserver_heap_by_type(pPlatformSettings,NEXUS_HEAP_TYPE_GRAPHICS);
    if (index != -1) {
        settings.client.heap[NXCLIENT_DEFAULT_HEAP] = platformConfig.heap[index];
    }
    index = nxserver_heap_by_type(pPlatformSettings,NEXUS_HEAP_TYPE_MAIN);
    if (index != -1) {
        settings.client.heap[NXCLIENT_FULL_HEAP] = platformConfig.heap[index];
    }
    index = nxserver_heap_by_type(pPlatformSettings,NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION);
    if (index != -1) {
        settings.client.heap[NXCLIENT_VIDEO_SECURE_HEAP] = platformConfig.heap[index];
    }
    index = nxserver_heap_by_type(pPlatformSettings,NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS);
    if (index != -1) {
        settings.client.heap[NXCLIENT_DEFAULT_HEAP] = platformConfig.heap[index];
        if (settings.client.heap[NXCLIENT_SECONDARY_GRAPHICS_HEAP] == settings.client.heap[NXCLIENT_DEFAULT_HEAP]) {
            settings.client.heap[NXCLIENT_SECONDARY_GRAPHICS_HEAP] = NULL;
        }
    }

    settings.externalApp.enabled = true;
    settings.externalApp.display[0].handle = g_app.display[0];
    settings.externalApp.display[1].handle = g_app.display[1];
    /* allocIndex allows nxserver to coordinate resources with main app.
    Could be extended to audio_decoder, but we recommend using NEXUS_ANY_ID for 14.2 and beyond.
    No need to extend to NEXUS_ANY_ID resources like playpump, recpump, stcchannel, etc.
    No need to extend to video window because use of video windows on display 0 and 1 (non-transcode) is mutually exclusive.
    The api assumes resources are orthogonal (e.g. some silicon has stc's that are video-only). */
    settings.externalApp.enableAllocIndex[nxserverlib_index_type_video_decoder] = true;
    settings.externalApp.enableAllocIndex[nxserverlib_index_type_stc_index] = true;
    settings.externalApp.allocIndex = alloc_index;
    settings.externalApp.freeIndex = free_index;
    settings.externalApp.callback_context = &g_app;

    g_app.server = nxserverlib_init(&settings);
    if (!g_app.server) return BERR_TRACE(-1);

    rc = nxserver_ipc_init(g_app.server, g_app.lock);
    if (rc) return BERR_TRACE(rc);

    /* local (non-ipc) NxClient_Join allows the server to make nxclient calls */
    NxClient_Join(NULL);

    return 0;
}

static void stop_nxserver(void)
{
    NxClient_Uninit();
    nxserver_ipc_uninit();
    nxserverlib_uninit(g_app.server);
    BKNI_DestroyMutex(g_app.lock);
}

static int open_decode(void)
{
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PlatformConfiguration platformConfig;
    int rc;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    g_app.pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    g_app.compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(g_app.pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    if ( AUDIO_CODEC == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
#if NEXUS_NUM_SPDIF_OUTPUTS
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(g_app.compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
#endif
    }
    else
    {
#if NEXUS_NUM_SPDIF_OUTPUTS
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(g_app.pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    }
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(g_app.pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    g_app.window[0] = NEXUS_VideoWindow_Open(g_app.display[0], 0);
    g_app.window[1] = NEXUS_VideoWindow_Open(g_app.display[1], 0);

    /* bring up decoder and connect to display */
    g_app.vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(g_app.window[0], NEXUS_VideoDecoder_GetConnector(g_app.vdecode));
    NEXUS_VideoWindow_AddInput(g_app.window[1], NEXUS_VideoDecoder_GetConnector(g_app.vdecode));

    /* For this example, get data from a streamer input. The input band is platform-specific.
    See nexus/examples/frontend for input from a demodulator. */
    rc = NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
    if (rc) return BERR_TRACE(rc);

    /* Map a parser band to the streamer input band. */
    g_app.parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
    NEXUS_ParserBand_GetSettings(g_app.parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(g_app.parserBand, &parserBandSettings);

    /* Open the audio and video pid channels */
    g_app.videoPidChannel = NEXUS_PidChannel_Open(g_app.parserBand, VIDEO_PID, NULL);
    g_app.audioPidChannel = NEXUS_PidChannel_Open(g_app.parserBand, AUDIO_PID, NULL);

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = g_app.videoPidChannel; /* PCR happens to be on video pid */
    g_app.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    BDBG_ASSERT(g_app.stcChannel);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = g_app.videoPidChannel;
    videoProgram.stcChannel = g_app.stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = g_app.audioPidChannel;
    audioProgram.stcChannel = g_app.stcChannel;

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(g_app.vdecode, &videoProgram);
    NEXUS_AudioDecoder_Start(g_app.pcmDecoder, &audioProgram);
    if ( AUDIO_CODEC == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        NEXUS_AudioDecoder_Start(g_app.compressedDecoder, &audioProgram);
    }

    return 0;
}

static void close_decode(void)
{
    NEXUS_AudioDecoder_Stop(g_app.pcmDecoder);
    NEXUS_AudioDecoder_Stop(g_app.compressedDecoder);
    NEXUS_AudioDecoder_Close(g_app.pcmDecoder);
    NEXUS_AudioDecoder_Close(g_app.compressedDecoder);
    NEXUS_VideoDecoder_Stop(g_app.vdecode);
    NEXUS_VideoWindow_Close(g_app.window[0]);
    NEXUS_VideoWindow_Close(g_app.window[1]);
    NEXUS_VideoDecoder_Close(g_app.vdecode);
    NEXUS_StcChannel_Close(g_app.stcChannel);
    NEXUS_PidChannel_Close(g_app.videoPidChannel);
    NEXUS_PidChannel_Close(g_app.audioPidChannel);
}
