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
#include "nexus_platform.h"
#include <stdio.h>
#if !NEXUS_HAS_VIDEO_DECODER || !NEXUS_HAS_AUDIO
int main(void)
{
    printf("This application is not supported on this platform (needs audio and video decoder)!\n");
    return 0;
}
#else
#include <stdlib.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_core_utils.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_ir_input.h"

/* the following define the input file and its characteristics -- these will vary by input file */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0x21
#define AUDIO_PID 0x22

#define POWER_STANDBY_IR_CODE 0x600a

NEXUS_SurfaceHandle framebufferHD, framebufferSD;
NEXUS_DisplayHandle display0, display1;
NEXUS_Graphics2DHandle gfx;
NEXUS_PlatformConfiguration platformConfig;
NEXUS_StcChannelHandle stcChannel;
NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
NEXUS_VideoWindowHandle window0, window1;
NEXUS_VideoDecoderHandle videoDecoder;
NEXUS_VideoDecoderStartSettings videoProgram;
NEXUS_AudioDecoderHandle audioDecoder;
NEXUS_AudioDecoderStartSettings audioProgram;
NEXUS_FilePlayHandle file;
NEXUS_PlaypumpHandle playpump;
NEXUS_PlaybackHandle playback;
NEXUS_IrInputHandle irHandle;
BKNI_EventHandle event;
bool playback_started=false;

void irCallback(void *pParam, int iParam)
{
    size_t numEvents = 1;
    NEXUS_Error rc = 0;
    bool overflow;
    NEXUS_IrInputHandle irHandle = *(NEXUS_IrInputHandle *)pParam;
    BSTD_UNUSED(iParam);
    while (numEvents && !rc) {
        NEXUS_IrInputEvent irEvent;
        rc = NEXUS_IrInput_GetEvents(irHandle,&irEvent,1,&numEvents,&overflow);
        if (numEvents && (irEvent.code == POWER_STANDBY_IR_CODE)) {
            printf("Received Standby Command\n");
            BKNI_SetEvent(event);
        }
    }
}

void create_display(void)
{
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
    NEXUS_DisplayCapabilities capabilities;

    NEXUS_GetDisplayCapabilities(&capabilities);
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display0 = NEXUS_Display_Open(0, &displaySettings);
    /* Only open display1 if local outputs supported */
    if (capabilities.display[1].numVideoWindows) {
        NEXUS_DisplayStatus displayStatus;
        displaySettings.format = NEXUS_VideoFormat_eNtsc;
        display1 = NEXUS_Display_Open(1, &displaySettings);
        NEXUS_Display_GetStatus(display1, &displayStatus);
        if (displayStatus.timingGenerator == NEXUS_DisplayTimingGenerator_eEncoder) {
            NEXUS_Display_Close(display1);
            display1 = NULL;
        }
    }

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display0, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    if (display1) {
        NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    }
#endif
#if NEXUS_NUM_RFM_OUTPUTS
    if (display1) {
        NEXUS_Display_AddOutput(display1, NEXUS_Rfm_GetVideoConnector(platformConfig.outputs.rfm[0]));
    }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display0, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display0, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display0, &displaySettings);
		}
    }
#endif

    NEXUS_Display_GetSettings(display0, &displaySettings);
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
    NEXUS_Display_GetGraphicsSettings(display0, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.position.width = videoFormatInfo.width;
    graphicsSettings.position.height = videoFormatInfo.height;
    graphicsSettings.clip.width = 960;
    graphicsSettings.clip.height = 1080;
    NEXUS_Display_SetGraphicsSettings(display0, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display0, framebufferHD);
    window0 = NEXUS_VideoWindow_Open(display0, 0);

    if (display1) {
        NEXUS_Display_GetGraphicsSettings(display1, &graphicsSettings);
        graphicsSettings.enabled = true;
        NEXUS_Display_SetGraphicsSettings(display1, &graphicsSettings);
        NEXUS_Display_SetGraphicsFramebuffer(display1, framebufferSD);
        window1 = NEXUS_VideoWindow_Open(display1, 0);
    }

    NEXUS_VideoWindow_GetSettings(window0, &windowSettings);
    windowSettings.position.x = 180;
    windowSettings.position.y = 120;
    windowSettings.position.width = 720;
    windowSettings.position.height = 480;
    windowSettings.alpha = 0xFF;
    NEXUS_VideoWindow_SetSettings(window0, &windowSettings);

    if (window1) {
        NEXUS_VideoWindow_GetSettings(window1, &windowSettings);
        windowSettings.position.x = 180;
        windowSettings.position.y = 120;
        windowSettings.position.width = 360;
        windowSettings.position.height = 240;
        windowSettings.alpha = 0xFF;
        NEXUS_VideoWindow_SetSettings(window1, &windowSettings);
    }
}

void destroy_display(void)
{
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoWindow_Close(window0);
    if (window1) {
        NEXUS_VideoWindow_Close(window1);
    }
    NEXUS_Display_Close(display0);
    if (display1) {
        NEXUS_Display_Close(display1);
    }
}

void playback_start(void)
{
    if (playback_started)
        return;

    /* Start decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    file = NEXUS_FilePlay_OpenPosix("videos/cnnticker.mpg", NULL);
    if (!file) {
        fprintf(stderr, "can't open file\n");
        BDBG_ASSERT(file);
    }

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    printf("\n  STARTED - Audio and Video should be present\n\n");
    playback_started = true;
}

void playback_stop(void)
{
    if (!playback_started)
        return;

    /* Stop decoders */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_AudioDecoder_Stop(audioDecoder);

    /*Stop playback */
    NEXUS_Playback_Stop(playback);

    /* Close File. Required for umount */
    NEXUS_FilePlay_Close(file);

    playback_started = false;
}

void start_app(void)
{
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_IrInputSettings irSettings;
    NEXUS_IrInputDataFilter irPattern;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);
    NEXUS_IrInput_GetDefaultSettings(&irSettings);
    irSettings.mode = NEXUS_IrInputMode_eRemoteA;
    irSettings.dataReady.callback = irCallback;
    irSettings.dataReady.context = &irHandle;
    irHandle = NEXUS_IrInput_Open(0, &irSettings);

    NEXUS_IrInput_GetDefaultDataFilter(&irPattern );
    irPattern.patternWord0 = POWER_STANDBY_IR_CODE;
    NEXUS_IrInput_EnableDataFilter(irHandle, &irPattern);

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 960;
    createSettings.height = 1080;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebufferHD = NEXUS_Surface_Create(&createSettings);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebufferHD;
    fillSettings.rect.width = createSettings.width;
    fillSettings.rect.height = createSettings.height;
    fillSettings.color = 0xFF0000FF;
    NEXUS_Graphics2D_Fill(gfx, &fillSettings);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = 480;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebufferSD = NEXUS_Surface_Create(&createSettings);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebufferSD;
    fillSettings.rect.width = createSettings.width;
    fillSettings.rect.height = createSettings.height;
    fillSettings.color = 0xFF0000FF;
    NEXUS_Graphics2D_Fill(gfx, &fillSettings);

    create_display();

    playpump = NEXUS_Playpump_Open(0, NULL);
    playback = NEXUS_Playback_Create();

    file = NEXUS_FilePlay_OpenPosix("videos/cnnticker.mpg", NULL);
    if (!file) {
        fprintf(stderr, "can't open file\n");
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up audio decoders and outputs */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS

    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window0, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    if (window1) {
        NEXUS_VideoWindow_AddInput(window1, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    }

    NEXUS_VideoWindow_GetSettings(window0, &windowSettings);
    /* punch through the picture-in-graphics hole */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebufferHD;
    fillSettings.rect = windowSettings.position;
    fillSettings.rect.x /=2;
    fillSettings.rect.width /=2; /* GFD is doing a horizontal upscale */
    fillSettings.color = 0x00cacaca;
    NEXUS_Graphics2D_Fill(gfx, &fillSettings);

    if (window1) {
        NEXUS_VideoWindow_GetSettings(window1, &windowSettings);
        /* punch through the picture-in-graphics hole */
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = framebufferSD;
        fillSettings.rect = windowSettings.position;
        fillSettings.color = 0x00cacaca;
        NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    }

    NEXUS_Graphics2D_Checkpoint(gfx, NULL);

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    playback_start();
}

void stop_app(void)
{
    playback_stop();
    /* Bring down system */
    NEXUS_IrInput_DisableDataFilter(irHandle);
    NEXUS_IrInput_Close(irHandle);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoWindow_RemoveInput(window0, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    if (window1) {
        NEXUS_VideoWindow_RemoveInput(window1, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    }
    destroy_display();
    NEXUS_VideoDecoder_Close(videoDecoder);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
#endif
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_Surface_Destroy(framebufferHD);
    NEXUS_Surface_Destroy(framebufferSD);
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_StcChannel_Close(stcChannel);
    BKNI_DestroyEvent(event);
}

struct standby_mode {
    NEXUS_PlatformStandbyMode mode;
    char *name;
} standby_state[NEXUS_PlatformStandbyMode_eMax] = {{NEXUS_PlatformStandbyMode_eOn, "S0"},
						   {NEXUS_PlatformStandbyMode_eActive, "S1"},
						   {NEXUS_PlatformStandbyMode_ePassive, "S2"},
						   {NEXUS_PlatformStandbyMode_eDeepSleep, "S3"}};

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformStandbySettings nexusStandbySettings;
    NEXUS_Error rc;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);

    start_app();

    while(1) {
        int i=rand()%NEXUS_PlatformStandbyMode_eMax;

        rc = BKNI_WaitForEvent(event, 5000);

        printf("ENTERING MODE : %s\n", standby_state[i].name);

        if(standby_state[i].mode != NEXUS_PlatformStandbyMode_eOn)
            playback_stop();

        NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
        nexusStandbySettings.mode = standby_state[i].mode;
        rc = NEXUS_Platform_SetStandbySettings(&nexusStandbySettings);
        BDBG_ASSERT(!rc);

        if(standby_state[i].mode == NEXUS_PlatformStandbyMode_eOn)
            playback_start();

        BKNI_ResetEvent(event);
    }

    BKNI_Sleep(5000);

    stop_app();

    NEXUS_Platform_Uninit();

    return 0;
}
#endif /* !NEXUS_HAS_VIDEO_DECODER || !NEXUS_HAS_AUDIO */
