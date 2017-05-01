/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/
#include "bpiff_encoder.h"
#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_video_adj.h"
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder_output.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#endif
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif
#include "nexus_core_utils.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(encode_piff_example);

#if 0
static char lic_data_SupperSpeedway[] = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>AmfjCTOPbEOl3WD/5mcecA==</KID><CHECKSUM>BGw1aYZ1YXM=</CHECKSUM><CUSTOMATTRIBUTES><IIS_DRM_VERSION>7.1.1064.0</IIS_DRM_VERSION></CUSTOMATTRIBUTES><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx</LA_URL><DS_ID>AH+03juKbUGbHl1V/QIwRA==</DS_ID></DATA></WRMHEADER>";
/*
static char lic_data_BearVideoOPLs[] = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>tusYN3uoeU+zLAXCJuHQ0w==</KID><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LA_URL><LUI_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LUI_URL><DS_ID>AH+03juKbUGbHl1V/QIwRA==</DS_ID><CHECKSUM>3hNyF98QQko=</CHECKSUM></DATA></WRMHEADER>";
*/
static char lic_data_BearVideoOPLs[] = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>tusYN3uoeU+zLAXCJuHQ0w==</KID><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LA_URL><LUI_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LUI_URL><CHECKSUM>3hNyF98QQk1=</CHECKSUM></DATA></WRMHEADER>";

static char DS_ID[] = "AH+03juKbUGbHl1V/QIwRA==";
#endif

PIFF_encoder_handle g_piff_handle;

static bool secure_video = false;

const char usage_str[] =
"\n"
"USAGE: "
"encode_piff [-i input_video.ts] [-o piff_file.mp4]\n"
"\n"
"DESCRIPTION:\n"
"  Encrypt input TS stream and outpu a well-formed PIFF file.\n"
"\n";

static int usage(void)
{
    fprintf(stderr, "%s\n", usage_str);
    return 0;
}

static void *getchar_thread(void *c)
{
    bool *key_pressed = c;
    getchar();
    *key_pressed = true;
    if (g_piff_handle != NULL)
    {
        printf("Key pressed, stopping the piff encoding here\n");
        piff_encode_stop(g_piff_handle);
        g_piff_handle = NULL;
    }
    return NULL;
}

static void play_endOfStreamCallback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);

    printf("End of stream detected\n");
    if (g_piff_handle != NULL)
    {
        printf("Stop piff encoding.\n");
        piff_encode_stop(g_piff_handle);
        g_piff_handle = NULL;
    }
	return;
}

static void piffEncodeComplete(void * context)
{
    printf("%s\n", __FUNCTION__);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char* argv[])
{
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_VIDEO_ENCODER
    NEXUS_PlatformSettings platformSettings;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel, stcChannelEncoder;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    NEXUS_DisplayHandle displayTranscode;
    NEXUS_VideoWindowHandle windowTranscode;
    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderStatus videoEncoderStatus;

    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_AudioDecoderStartSettings audioProgram;
 
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_AudioEncoderHandle audioEncoder;

    BKNI_EventHandle event;

    bool key_pressed = false;
    pthread_t getchar_thread_id;
    NEXUS_DisplaySettings displaySettings;
    char fname[] = "videos/avatar_AVC_15M.ts";
    char fout[] = "piff.mp4";

    /* DRM_Prdy specific */
    DRM_Prdy_Init_t prdyParamSettings;
    DRM_Prdy_Handle_t drm_context;

    /* PIFF specific */
    PIFF_encoder_handle piff_handle = NULL;
    PIFF_Encoder_Settings piffSettings;

#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_VideoWindowScalerSettings sclSettings;
    NEXUS_VideoWindowMadSettings madSettings;
    NEXUS_VideoWindowSettings windowSettings;
#endif

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

/*
    int ii;
    for (ii = 1; ii < argc; ii++) {
        if (!strcmp(argv[ii], "-i")){
            fname = argv[++ii];
        }
        else if (!strcmp(argv[ii], "-o")){
            fout = argv[++ii];
        }
    }

    if (fname == NULL || fout == NULL){
        usage();
        goto clean_exit;
    }
*/

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

#ifdef NEXUS_EXPORT_HEAP
    /* Configure export heap since it's not allocated by nexus by default */
    platformSettings.heap[NEXUS_EXPORT_HEAP].size = 32*1024*1024;
#endif

    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);

    if (NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings)) {
        fprintf(stderr, "NEXUS_Platform_Init failed\n");
        goto clean_exit;
    }

    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    printf("Creating the Event...\n");

    printf("Finish waiting...\n");
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        goto clean_exit;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* encoder requires different STC broadcast mode from decoder */
    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    stcChannelEncoder = NEXUS_StcChannel_Open(1, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, */
    /* i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ...                      */
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playbackSettings.stcChannel = stcChannel;

    playbackSettings.endOfStreamCallback.callback = play_endOfStreamCallback;
    playbackSettings.endOfStreamCallback.context = NULL;

    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    printf("Create a DRM_Prdy_context");
    DRM_Prdy_GetDefaultParamSettings(&prdyParamSettings);
    drm_context = DRM_Prdy_Initialize(&prdyParamSettings);
    if (drm_context == NULL)
    {
       printf("Failed to create drm_context, quitting....");
       goto clean_exit;
    }

    printf("Create a piff handle\n");
    piff_GetDefaultSettings(&piffSettings);
    piffSettings.destPiffFileName = fout;
    piffSettings.completionCallBack.callback = piffEncodeComplete;
    piffSettings.completionCallBack.context = event;
    /*
    piffSettings.licAcqDSId = DS_ID;
    piffSettings.licAcqDSIdLen = strlen(DS_ID);
    */
    piff_handle = piff_create_encoder_handle(&piffSettings, drm_context);
    if (piff_handle == NULL) {
        printf("FAILED to create piff handle, I'm quitting...\n");
        return 0;
    }
    printf("created a piff handle\n");
    g_piff_handle = piff_handle;

    /* bring up decoder and connect to local display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
     * which might cause encoder display GISB error since encoder display would
     * trigger RDC to program mailbox registers in ViCE2;
     */
    videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    assert(videoEncoder);

    /* Bring up video encoder display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    displaySettings.format = NEXUS_VideoFormat_e720p;
    displayTranscode = NEXUS_Display_Open(0, &displaySettings);
#else
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.format = NEXUS_VideoFormat_e720p24hz;/* bring up 480p first */
    displayTranscode = NEXUS_Display_Open(NEXUS_ENCODER_DISPLAY_IDX, &displaySettings);
#endif
    assert(displayTranscode);

    windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
    assert(windowTranscode);

#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_VideoWindow_GetSettings(windowTranscode, &windowSettings);
    windowSettings.position.width = 416;
    windowSettings.position.height = 224;
    windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    windowSettings.visible = false;
    NEXUS_VideoWindow_SetSettings(windowTranscode, &windowSettings);
    NEXUS_VideoWindow_GetScalerSettings(windowTranscode, &sclSettings);
    sclSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
    sclSettings.bandwidthEquationParams.delta = 1000000;
    NEXUS_VideoWindow_SetScalerSettings(windowTranscode, &sclSettings);
    NEXUS_VideoWindow_GetMadSettings(windowTranscode, &madSettings);
    madSettings.deinterlace = true;
    madSettings.enable22Pulldown = true;
    madSettings.enable32Pulldown = true;
    NEXUS_VideoWindow_SetMadSettings(windowTranscode, &madSettings);
#endif

    /* connect same decoder to encoder display
     * This simul mode is for video encoder bringup only; audio path may have limitation
     * for simul display+transcode mode;
     */
    NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* Open the video pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eH264; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x101, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eH264;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
 
    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e29_97;
    videoEncoderConfig.bitrateMax = 400 * 1000;
#else
    videoEncoderConfig.variableFrameRate = true;
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e24;
    videoEncoderConfig.bitrateMax = 6 * 1000 * 1000;
    videoEncoderConfig.streamStructure.framesP = 29; /* IPP GOP size = 30 */
    videoEncoderConfig.streamStructure.framesB = 0;
#endif
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    /* Open the audio pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder; /* must be told codec for correct handling */
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x104, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);

    audioProgram.codec = NEXUS_AudioCodec_eAc3;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Connect audio decoders to outputs */
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
    {
        NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
        NEXUS_AudioEncoderSettings encoderSettings;

        audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);

        NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
        encoderSettings.codec = NEXUS_AudioCodec_eAacAdts;
        audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);

        /* Connect encoder to decoder */
        NEXUS_AudioEncoder_AddInput(audioEncoder,
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        /* Connect mux to encoder */
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));

        /* Start audio mux output */
        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
        audioMuxStartSettings.stcChannel = stcChannelEncoder;
        NEXUS_AudioMuxOutput_Start(audioMuxOutput, &audioMuxStartSettings);
        NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
    }


    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    BKNI_Sleep(1000);

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e30;
    videoEncoderStartConfig.bounds.inputDimension.max.width = 416;
    videoEncoderStartConfig.bounds.inputDimension.max.height = 224;
    videoEncoderStartConfig.stcChannel = stcChannelEncoder;
#else
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e31;
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.stcChannel = stcChannelEncoder;

	/******************************************
	 * add configurable delay to video path
	 */
	/* NOTE: ITFP is encoder feature to detect and lock on 3:2/2:2 cadence in the video content to help
	 * efficient coding for interlaced formats; disabling ITFP will impact the bit efficiency but reduce the encode delay. */
	videoEncoderConfig.enableFieldPairing = true;

	/* 0 to use default 750ms rate buffer delay; TODO: allow user to adjust it to lower encode delay at cost of quality reduction! */
	videoEncoderStartConfig.rateBufferDelay = 0;

	/* to allow 23.976p passthru; TODO: allow user to configure minimum framerate to achieve lower delay!
	 * Note: lower minimum framerate means longer encode delay */
	videoEncoderStartConfig.bounds.inputFrameRate.min = NEXUS_VideoFrameRate_e23_976;

	/* to allow 24 ~ 60p dynamic frame rate coding TODO: allow user to config higher minimum frame rate for lower delay! */
	videoEncoderStartConfig.bounds.outputFrameRate.min = NEXUS_VideoFrameRate_e23_976;
	videoEncoderStartConfig.bounds.outputFrameRate.max = NEXUS_VideoFrameRate_e60;

	/* max encode size allows 1080p encode; TODO: allow user to choose lower max resolution for lower encode delay */
	videoEncoderStartConfig.bounds.inputDimension.max.width = 1920;
	videoEncoderStartConfig.bounds.inputDimension.max.height = 1088;
#endif

    /* NOTE: video encoder delay is in 27MHz ticks */
    NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
    BDBG_WRN(("\n\tVideo encoder end-to-end delay = [%u ~ %u] ms", videoDelay.min/27000, videoDelay.max/27000));
    videoEncoderConfig.encoderDelay = videoDelay.min;

    printf("PIFF encoding starts. Press Enter to stop or wait for the encoding to finish.\n");
    fflush(stdout);

    /* note the Dee is set by SetSettings */
    NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);
    NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);
    NEXUS_VideoEncoder_GetStatus(videoEncoder, &videoEncoderStatus);

    printf("Create thread for the key pressed.\n");
    pthread_create(&getchar_thread_id, NULL, getchar_thread, &key_pressed);

    printf("Start piff encoding.\n");
    piff_encode_start(audioMuxOutput, videoEncoder, piff_handle);
    printf("piff encoding started...\n");

    BKNI_WaitForEvent(event, BKNI_INFINITE);

    printf("PIFF Encoding completed.\n");

    NEXUS_VideoEncoder_Stop(videoEncoder, NULL);

    /* Bring down system */
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_AudioMuxOutput_Stop(audioMuxOutput);
    NEXUS_AudioEncoder_RemoveAllInputs(audioEncoder);
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
    NEXUS_AudioMuxOutput_Destroy(audioMuxOutput);
    NEXUS_AudioEncoder_Close(audioEncoder);

    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoWindow_Close(windowTranscode);
    NEXUS_Display_Close(displayTranscode);

    NEXUS_Playpump_Close(playpump);

    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_StcChannel_Close(stcChannel);

    NEXUS_AudioDecoder_Close(audioDecoder);

    NEXUS_VideoEncoder_Close(videoEncoder);
    NEXUS_StcChannel_Close(stcChannelEncoder);

    piff_destroy_encoder_handle(piff_handle);

    DRM_Prdy_Uninitialize(drm_context);

    BKNI_DestroyEvent(event);

    NEXUS_Platform_Uninit();

#endif
    BSTD_UNUSED(usage);

clean_exit:

    return 0;
}
