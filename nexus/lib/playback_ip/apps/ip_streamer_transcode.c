/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *  main test app for ip_streamer
 *
 ******************************************************************************/
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>

#include "bstd.h"
#include "bkni.h"
#include "blst_list.h"
#include "blst_queue.h"

#ifndef DMS_CROSS_PLATFORMS
#include "nexus_platform.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_parser_band.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_message.h"
#include "nexus_timebase.h"
#include "nexus_recpump.h"
#ifdef NEXUS_HAS_RECORD
#include "nexus_record.h"
#endif
#include "nexus_file_fifo.h"
#include "nexus_core_utils.h"
#endif /* DMS_CROSS_PLATFORMS */
#include "b_playback_ip_lib.h"

#ifdef B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#include "b_dtcp_constants.h"
#endif

#include "nexus_core_utils.h"
#include "ip_streamer_lib.h"
#include "ip_streamer.h"
#include "ip_http.h"
#include "b_os_lib.h"

#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_adj.h"
#include "nexus_audio_input.h"
#include "nexus_video_decoder_extra.h"
#endif

#include "nexus_audio_mixer.h"

BDBG_MODULE(ip_streamer);
#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN( x );
#else
#define BDBG_MSG_FLOW(x)
#endif
#define IP_NETWORK_JITTER 300
extern void closeNexusPidChannels(IpStreamerCtx *ipStreamerCtx);
extern int mapFrameRateToGopSize(NEXUS_VideoFrameRate frameRate);
#if 0
/* Test code */
int transcoderSetupDone = 0;
TranscoderDst *gTranscoderDst;
#endif

static void
ptsErrorCallback(void *context, int param)
{

    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    BDBG_MSG(("####################%s: callback", BSTD_FUNCTION));
}

static void
videoFirstPtsCallback(void *context, int param)
{

    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    BDBG_MSG(("####################%s: first pts callback", BSTD_FUNCTION));
}
static void
audioFirstPtsCallback(void *context, int param)
{

    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    BDBG_MSG(("####################%s: first passed pts callback", BSTD_FUNCTION));
}

static void
videoFirstPtsPassedCallback(void *context, int param)
{

    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    BDBG_MSG(("####################%s: first passed pts callback", BSTD_FUNCTION));
}

#if NEXUS_HAS_VIDEO_ENCODER && (!NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
void unInitNexusTranscoderPipe(
    TranscoderDst *transcoderDst
    )
{
    BSTD_UNUSED(transcoderDst);
    BDBG_MSG(("%s: Enter: TODO", BSTD_FUNCTION));
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif
    if (!transcoderDst)
        return;
    {
        BDBG_MSG(("%s: Un_innt transcoder pipe, ref cnt %d", BSTD_FUNCTION, transcoderDst->refCount));
#if NEXUS_HAS_SYNC_CHANNEL
        /* disconnect sync channel */
        if (transcoderDst->syncChannel) {
            NEXUS_SyncChannel_GetSettings(transcoderDst->syncChannel, &syncChannelSettings);
            syncChannelSettings.videoInput = NULL;
            syncChannelSettings.audioInput[0] = NULL;
            syncChannelSettings.audioInput[1] = NULL;
            NEXUS_SyncChannel_SetSettings(transcoderDst->syncChannel, &syncChannelSettings);
            NEXUS_SyncChannel_Destroy(transcoderDst->syncChannel);
            transcoderDst->syncChannel = NULL;
        }
#endif
        if (transcoderDst->timebase != NEXUS_Timebase_eInvalid)
            NEXUS_Timebase_Close(transcoderDst->timebase);
        transcoderDst->timebase = NEXUS_Timebase_eInvalid;

        /* now close the transcoder pipe */
        if (transcoderDst->videoEncoder)
            NEXUS_VideoEncoder_Close(transcoderDst->videoEncoder);
        transcoderDst->videoEncoder = NULL;
        BDBG_MSG(("Closed NEXUS_VideoEncoder_Close ...."));
        if (transcoderDst->streamMux)
            NEXUS_StreamMux_Destroy(transcoderDst->streamMux);
        transcoderDst->streamMux = NULL;
#ifdef B_HAS_DISPLAY_LOCAL_FOR_ENCODE
        if (transcoderDst->windowMain) {
#if NEXUS_HAS_HDMI_INPUT
            if (ipStreamerCtx->cfg.srcType == IpStreamerSrc_eHdmi)
                NEXUS_VideoWindow_RemoveInput(transcoderDst->windowMain, NEXUS_HdmiInput_GetVideoConnector(ipStreamerCtx->hdmiSrc->hdmiInput));
            else
#endif
                NEXUS_VideoWindow_RemoveInput(transcoderDst->windowMain, NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder));
            NEXUS_VideoWindow_Close(transcoderDst->windowMain);
        }
        transcoderDst->windowMain = NULL;
        if (transcoderDst->displayMain)
            NEXUS_Display_Close(transcoderDst->displayMain);
        transcoderDst->displayMain = NULL;
#endif
        if (transcoderDst->windowTranscode) {
            NEXUS_VideoWindow_RemoveAllInputs(transcoderDst->windowTranscode);
            NEXUS_VideoWindow_Close(transcoderDst->windowTranscode);
        }
        transcoderDst->windowTranscode = NULL;
        if (transcoderDst->videoDecoder) {
            NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder));
            NEXUS_VideoDecoder_Close(transcoderDst->videoDecoder);
        }
        transcoderDst->videoDecoder = NULL;
        if (transcoderDst->displayTranscode)
            NEXUS_Display_Close(transcoderDst->displayTranscode);
        transcoderDst->displayTranscode = NULL;

        if (transcoderDst->audioDecoder) {
            NEXUS_PlatformConfiguration platformConfig;

            NEXUS_Platform_GetConfiguration(&platformConfig);
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(transcoderDst->audioMuxOutput));
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
            NEXUS_AudioMixer_RemoveAllInputs(transcoderDst->audioMixer);
#endif
            NEXUS_AudioEncoder_RemoveAllInputs(transcoderDst->audioEncoder);
            NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(transcoderDst->audioEncoder));
            NEXUS_AudioEncoder_Close(transcoderDst->audioEncoder);

            {
                {
                    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[transcoderDst->contextId]));
                    NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[transcoderDst->contextId]));
                }
            }
            NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
            NEXUS_AudioMuxOutput_Destroy(transcoderDst->audioMuxOutput);
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
            NEXUS_AudioMixer_Close(transcoderDst->audioMixer);
#endif
            NEXUS_AudioDecoder_Close(transcoderDst->audioDecoder);
            transcoderDst->audioDecoder = NULL;
            transcoderDst->audioEncoder = NULL;
            transcoderDst->audioMuxOutput = NULL;
            transcoderDst->audioMixer = NULL;
        }
        transcoderDst->audioDecoder = NULL;
        if (transcoderDst->playpumpTranscodeVideo)
            NEXUS_Playpump_Close(transcoderDst->playpumpTranscodeVideo);
        transcoderDst->playpumpTranscodeVideo = NULL;
        if (transcoderDst->playpumpTranscodeAudio)
            NEXUS_Playpump_Close(transcoderDst->playpumpTranscodeAudio);
        transcoderDst->playpumpTranscodeAudio = NULL;
        if (transcoderDst->playpumpTranscodeSystemData)
            NEXUS_Playpump_Close(transcoderDst->playpumpTranscodeSystemData);
        transcoderDst->playpumpTranscodeSystemData = NULL;
        transcoderDst->inUse = false;
    }
    BDBG_MSG(("%s: Done", BSTD_FUNCTION));
}

int
initNexusVice2TranscoderPipe(
        TranscoderDst *transcoderDst
        )
{
    int i;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_StreamMuxConfiguration streamMuxConfig;
    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    NEXUS_VideoWindowMadSettings madSettings;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_AudioEncoderSettings encoderSettings;
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    NEXUS_VideoWindowScalerSettings scalerSettings;
    NEXUS_AudioMixerSettings mixerSettings;
#endif
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif

    BDBG_MSG(("%s: setup transcoder: ctx %p", BSTD_FUNCTION, (void *)transcoderDst));

    /* now setup this transcoding pipeline */
#if NEXUS_HAS_SYNC_CHANNEL
    /* create a sync channel */
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    transcoderDst->syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
    if (!transcoderDst->syncChannel) {
        BDBG_ERR(("%s: ERROR: Failed to create nexus sync channel", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Created nexus sync channel", BSTD_FUNCTION));
#endif

    if ((transcoderDst->timebase = NEXUS_Timebase_Open(NEXUS_ANY_ID)) == NEXUS_Timebase_eInvalid) {
        BDBG_ERR(("%s: ERROR: NEXUS_Timebase_Open Failed to open a free Timebase", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: using timebase %d for decode", BSTD_FUNCTION, (int)transcoderDst->timebase));

    /* video decoder, stc channel are not needed for straight hdmi sources */
    {
        NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
        /* open decoder */
        NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
        videoDecoderOpenSettings.fifoSize = 5*1024*1024; /* 5MB: for over 1 sec of jitter buffer for 20Mpbs stream */
        BDBG_MSG(("%s: Increasing Video Decoder Fifo size to %d", BSTD_FUNCTION, videoDecoderOpenSettings.fifoSize));
        videoDecoderOpenSettings.enhancementPidChannelSupported = false;

        /* for xcode, we work backwords from the higher decoder */
        for (i=NEXUS_NUM_VIDEO_DECODERS-1; i>=0; i--)
        {
            transcoderDst->videoDecoder = NEXUS_VideoDecoder_Open(i, &videoDecoderOpenSettings); /* take default capabilities */
            if (transcoderDst->videoDecoder) {
                BDBG_MSG(("%s: using decoder idx %d", BSTD_FUNCTION, i));
                break;
            }
        }
        if (!transcoderDst->videoDecoder) {
            BDBG_ERR(("%s: ERROR: Can't get a free Decoder Handle, max idx %d", BSTD_FUNCTION, i));
            goto error;
        }

        /* turn off svc/mvc decodes to save heap memory space */
        NEXUS_VideoDecoder_GetSettings(transcoderDst->videoDecoder, &videoDecoderSettings);
        videoDecoderSettings.supportedCodecs[NEXUS_VideoCodec_eH264_Svc] = false;
        videoDecoderSettings.supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = false;
        videoDecoderSettings.firstPts.callback = videoFirstPtsCallback;
        videoDecoderSettings.firstPts.context = NULL;
        videoDecoderSettings.firstPtsPassed.callback = videoFirstPtsPassedCallback;
        videoDecoderSettings.firstPtsPassed.context = NULL;
        videoDecoderSettings.ptsError.callback = ptsErrorCallback;
        videoDecoderSettings.firstPtsPassed.context = NULL;

        if (NEXUS_VideoDecoder_SetSettings(transcoderDst->videoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to turn off SVC/MVC decode option on 2nd decoder", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: turned off SVC/MVC decode option on 2nd decoder", BSTD_FUNCTION));
    }

    /* open & configure encoder */
    for (i=0; i<NEXUS_NUM_VIDEO_ENCODERS; i++) {
        transcoderDst->videoEncoder = NEXUS_VideoEncoder_Open(i, NULL); /* take default capabilities */
        if (transcoderDst->videoEncoder)
            break;
    }
    if (!transcoderDst->videoEncoder) {
        BDBG_ERR(("%s: ERROR: Can't get a free Video Encoder Handle, max idx %d", BSTD_FUNCTION, i));
        goto error;
    }
    BDBG_MSG(("%s: opened video encoder %p for transcode ctx %p", BSTD_FUNCTION, (void *)transcoderDst->videoEncoder, (void *)transcoderDst));

#ifdef B_HAS_DISPLAY_LOCAL_FOR_ENCODE
    {
        NEXUS_PlatformConfiguration platformConfig;

        NEXUS_Platform_GetConfiguration(&platformConfig);
        /* Bring up video display and outputs */
        transcoderDst->displayMain = NEXUS_Display_Open(0, NULL);
#if NEXUS_NUM_COMPONENT_OUTPUTS
        if(platformConfig.outputs.component[0]){
            NEXUS_Display_AddOutput(transcoderDst->displayMain, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
        }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
        if(platformConfig.outputs.hdmi[0]){
            NEXUS_Display_AddOutput(transcoderDst->displayMain, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
        }
#endif
        transcoderDst->windowMain = NEXUS_VideoWindow_Open(transcoderDst->displayMain, 0);
        assert(transcoderDst->windowMain);
    }
#endif
    /* open & setup the display: note transcode video format settings are specified here */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.format = NEXUS_VideoFormat_e720p;
    displaySettings.frameRateMaster = NULL;/* disable frame rate tracking for now */

    transcoderDst->displayTranscode = NEXUS_Display_Open(transcoderDst->displayIndex, &displaySettings);/* cmp3 for transcoder */
    if (!transcoderDst->displayTranscode) {
        BDBG_ERR(("%s: ERROR: Can't get a free Display Handle, max idx %d", BSTD_FUNCTION, transcoderDst->displayIndex));
        goto error;
    }

    NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
    customFormatSettings.refreshRate = 30000;
    /* Low latency settings: start w/ the smallest resolution and then increase it after first few sements */
    customFormatSettings.width = 640;
    customFormatSettings.height = 480;
    customFormatSettings.aspectRatio = NEXUS_DisplayAspectRatio_e16x9;
    customFormatSettings.sampleAspectRatio.x = 640;
    customFormatSettings.sampleAspectRatio.y = 480;
    customFormatSettings.dropFrameAllowed = true;
    if (NEXUS_Display_SetCustomFormatSettings(transcoderDst->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings) != 0) {
        BDBG_ERR(("%s: ERROR: NEXUS_Display_SetCustomFormatSettings() Failed", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Successfully Set Custom Format settings for display", BSTD_FUNCTION));

    if ((transcoderDst->windowTranscode = NEXUS_VideoWindow_Open(transcoderDst->displayTranscode, 0)) == NULL) {
        BDBG_ERR(("%s: ERROR: Can't get a free Video Window on Display", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: display %p & video window %p are opened for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst->displayTranscode, (void *)transcoderDst->windowTranscode, (void *)transcoderDst));

    /* set transcoder minimum display format before AddInput to avoid black frame transition during dynamic resolution change */
    NEXUS_VideoWindow_GetSettings(transcoderDst->windowTranscode, &windowSettings);
    windowSettings.minimumDisplayFormat = NEXUS_VideoFormat_e1080p;
    windowSettings.forceCapture = false;
    windowSettings.scaleFactorRounding.enabled = false;
    windowSettings.scaleFactorRounding.horizontalTolerance = 0;
    windowSettings.scaleFactorRounding.verticalTolerance   = 0;
    if (NEXUS_VideoWindow_SetSettings(transcoderDst->windowTranscode, &windowSettings) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_SetSettings Failed for the transcode window", BSTD_FUNCTION));
        goto error;
    }

    /* set transcoder window vnet mode bias to avoid black frame transition during dynamic resolution change */
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    NEXUS_VideoWindow_GetScalerSettings(transcoderDst->windowTranscode, &scalerSettings);
    scalerSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerAfterCapture;
    scalerSettings.bandwidthEquationParams.delta = 1*1000*1000;
    if (NEXUS_VideoWindow_SetScalerSettings(transcoderDst->windowTranscode, &scalerSettings) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_SetScalerSettings Failed for transcode window", BSTD_FUNCTION));
        goto error;
    }
#endif
    /* connect decoder to the display (which feeds into encoder) */
    if (NEXUS_VideoWindow_AddInput(transcoderDst->windowTranscode, NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder)) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_AddInput() failed to connect decoder to display", BSTD_FUNCTION));
        goto error;
    }
#ifdef B_HAS_DISPLAY_LOCAL_FOR_ENCODE
    NEXUS_VideoWindow_AddInput(transcoderDst->windowMain, NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder));
#endif

    {
        NEXUS_PlatformConfiguration platformConfig;
        NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
        NEXUS_AudioCapabilities audioCap;

        NEXUS_Platform_GetConfiguration(&platformConfig);
        NEXUS_GetAudioCapabilities(&audioCap);
        NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
        audioDecoderOpenSettings.fifoSize = 512*1024; /* increasing to act as de-jitter buffer for ip */

        for (i=0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
        {
            transcoderDst->audioDecoder = NEXUS_AudioDecoder_Open(i, &audioDecoderOpenSettings);
            if (transcoderDst->audioDecoder)
                break;
        }
        if (!transcoderDst->audioDecoder) {
            BDBG_ERR(("%s: ERROR: Can't open audio decoder upto id %d", BSTD_FUNCTION, i-1));
            goto error;
        }
        BDBG_MSG(("%s: opened audio decoder %p for transcode ctx %p", BSTD_FUNCTION, (void *)transcoderDst->audioDecoder, (void *)transcoderDst));

        /* Create audio mux output */
        if ((transcoderDst->audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL)) == NULL) {
            BDBG_ERR(("%s: ERROR: Can't create Audio Stream Mux handle", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: Audio Stream Mux %p is created for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst->audioMuxOutput, (void *)transcoderDst));

        /* Open audio encoder */
        NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
        encoderSettings.codec = NEXUS_AudioCodec_eAac;
        transcoderDst->audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
        if (!transcoderDst->audioEncoder) {
            BDBG_ERR(("%s: ERROR: Can't open audio encoder handle", BSTD_FUNCTION));
            goto error;
        }

#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
        /* Open audio mixer.  The mixer can be left running at all times to provide continuous audio output despite input discontinuities.  */
        NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
        mixerSettings.mixUsingDsp = true;
        mixerSettings.dspIndex    = ((unsigned)i)>(NEXUS_NUM_AUDIO_DECODERS/audioCap.numDsps)? 1:0;
        mixerSettings.outputSampleRate = 48000;  /* NOTE: Set it to the desired value of the output sampling rate */
        transcoderDst->audioMixer = NEXUS_AudioMixer_Open(&mixerSettings);
        if (!transcoderDst->audioMixer) {
            BDBG_ERR(("%s: ERROR: Can't open audio encoder handle", BSTD_FUNCTION));
            goto error;
        }

        /* Connect decoder to mixer and set as master */
        if (NEXUS_AudioMixer_AddInput(transcoderDst->audioMixer, NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_AudioMixer_AddInput() failed to connect decoder to the mixer ", BSTD_FUNCTION));
            goto error;
        }
        mixerSettings.master = NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
        if (NEXUS_AudioMixer_SetSettings(transcoderDst->audioMixer, &mixerSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_AudioMixer_SetSettings() failed", BSTD_FUNCTION));
            goto error;
        }

        /* Connect encoder to decoder */
        if (NEXUS_AudioEncoder_AddInput(
                    transcoderDst->audioEncoder,
                    NEXUS_AudioMixer_GetConnector(transcoderDst->audioMixer)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_AudioEncoder_AddInput() failed to connect encoder w/ decoder", BSTD_FUNCTION));
            goto error;
        }

        /* Connect mux to encoder */
        if (NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioMuxOutput_GetConnector(transcoderDst->audioMuxOutput),
                    NEXUS_AudioEncoder_GetConnector(transcoderDst->audioEncoder)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_AudioOutput_AddInput() failed to connect mux to encoder", BSTD_FUNCTION));
            goto error;
        }
#else
        /* Connect encoder to decoder */
        if (NEXUS_AudioEncoder_AddInput(
                    transcoderDst->audioEncoder,
                    NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_AudioEncoder_AddInput() failed to connect encoder w/ decoder", BSTD_FUNCTION));
            goto error;
        }

        /* Connect mux to encoder */
        if (NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioMuxOutput_GetConnector(transcoderDst->audioMuxOutput),
                    NEXUS_AudioEncoder_GetConnector(transcoderDst->audioEncoder)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_AudioOutput_AddInput() failed to connect mux to encoder", BSTD_FUNCTION));
            goto error;
        }
#endif
        BDBG_MSG(("%s: audio encode is successfully setup", BSTD_FUNCTION));

#ifdef B_HAS_DISPLAY_LOCAL_FOR_ENCODE
#if NEXUS_NUM_HDMI_OUTPUTS
        if(platformConfig.outputs.hdmi[0]){
            NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        }
        NEXUS_AudioOutput_AddInput( NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

#endif
#endif
    }

    NEXUS_VideoWindow_GetMadSettings(transcoderDst->windowTranscode, &madSettings);
    if (NEXUS_VideoWindow_SetMadSettings(transcoderDst->windowTranscode, &madSettings) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_SetMadSettings() failed to enable MAD ", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Enabled MAD De-interlacer", BSTD_FUNCTION));

    /* open the stream mux */
    NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
    /* default stream mux memory allocation assumes no TS layer user data so takes much less memory allocation from system heap */
    NEXUS_StreamMux_GetDefaultConfiguration(&streamMuxConfig);
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    streamMuxConfig.nonRealTime = true;
#endif
    NEXUS_StreamMux_GetMemoryConfiguration(&streamMuxConfig,&muxCreateSettings.memoryConfiguration);
    if ((transcoderDst->streamMux = NEXUS_StreamMux_Create(&muxCreateSettings)) == NULL) {
        BDBG_ERR(("%s: ERROR: Can't create Stream Mux handle", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Stream Mux %p is created for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst->streamMux, (void *)transcoderDst));


    /* and finally open the playpump channels that are used by mux to feed the Audio & Video ES as well as System data to the xpt h/w */
    /* that is where the actual muxing of stream happens */

    /* reduce FIFO size allocated for playpump used for the transcoded streams */
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 512; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;

    if ((transcoderDst->playpumpTranscodeVideo = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig)) == NULL) {
        BDBG_ERR(("%s: ERROR: Failed to Open Nexus Playpump ", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Nexus Playpump is opened for video transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));
    if ((transcoderDst->playpumpTranscodeAudio = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig)) == NULL) {
        BDBG_ERR(("%s: ERROR: Failed to Open Nexus Playpump ", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Nexus Playpump is opened for audio transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));
    if ((transcoderDst->playpumpTranscodeSystemData = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig)) == NULL) {
        BDBG_ERR(("%s: ERROR: Failed to Open Nexus Playpump ", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Nexus Playpump is opened for system data transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));
    BDBG_MSG(("%s: setup transcoder ctx %p is successful", BSTD_FUNCTION, (void *)transcoderDst));

    return 0;

error:
    unInitNexusTranscoderPipe(transcoderDst);
    return -1;
}
#endif

int
initNexusTranscoderDstList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    int numEncoders = 0;
    NEXUS_MemoryConfigurationSettings *pMemConfigSettings;
    /* create the per dst list mutex */
    if (BKNI_CreateMutex(&ipStreamerGlobalCtx->transcoderDstMutex) != 0) {
        BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
        return -1;
    }

    pMemConfigSettings = BKNI_Malloc(sizeof(NEXUS_MemoryConfigurationSettings));
    BDBG_ASSERT(pMemConfigSettings);
    NEXUS_GetDefaultMemoryConfigurationSettings(pMemConfigSettings);

    for (i=0;i<NEXUS_MAX_VIDEO_ENCODERS;i++) {
        if(pMemConfigSettings->videoEncoder[i].used == true) {
            numEncoders++;
        }
    }
    BKNI_Free(pMemConfigSettings);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    for (i=0; i<NEXUS_NUM_DSP_VIDEO_ENCODERS; i++)
#else
    for (i=0; i<numEncoders; i++)
#endif
    {
        memset(&ipStreamerGlobalCtx->transcoderDstList[i], 0, sizeof(TranscoderDst));
        ipStreamerGlobalCtx->transcoderDstList[i].inUse = false;
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
        /* so far, RAAGA only has b/w to do 1 low res encode, that is why fixing it to 0 */
        ipStreamerGlobalCtx->transcoderDstList[i].displayIndex = 1;
#else
        ipStreamerGlobalCtx->transcoderDstList[i].displayIndex = NEXUS_ENCODER_DISPLAY_IDX-i;
#endif
         if (BKNI_CreateEvent(&ipStreamerGlobalCtx->transcoderDstList[i].finishEvent )) {
             BDBG_ERR(("Failed to create event at %d", __LINE__));
             return -1;
         }
#if NEXUS_HAS_VIDEO_ENCODER && (!NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
        if (initNexusVice2TranscoderPipe(&ipStreamerGlobalCtx->transcoderDstList[i]) != 0) {
             BDBG_ERR(("Failed to open transcoder pipe %d", __LINE__));
             return -1;
        }
#endif
    }

    BDBG_MSG(("%s: Transcoder Dst Initialized", BSTD_FUNCTION));
    return 0;
}

void
unInitNexusTranscoderDstList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    int numEncoders = 0;
    NEXUS_MemoryConfigurationSettings *pMemConfigSettings;

    pMemConfigSettings = BKNI_Malloc(sizeof(NEXUS_MemoryConfigurationSettings));
    BDBG_ASSERT(pMemConfigSettings);
    NEXUS_GetDefaultMemoryConfigurationSettings(pMemConfigSettings);

    for (i=0;i<NEXUS_MAX_VIDEO_ENCODERS;i++) {
        if(pMemConfigSettings->videoEncoder[i].used == true) {
            numEncoders++;
        }
    }
    BKNI_Free(pMemConfigSettings);

    if (ipStreamerGlobalCtx->transcoderDstMutex)
        BKNI_DestroyMutex(ipStreamerGlobalCtx->transcoderDstMutex);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    for (i=0; i<NEXUS_NUM_DSP_VIDEO_ENCODERS; i++)
#else
    for (i=0; i<numEncoders; i++)
#endif
    {
#if NEXUS_HAS_VIDEO_ENCODER && (!NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
        unInitNexusTranscoderPipe(&ipStreamerGlobalCtx->transcoderDstList[i]);
#endif
        if (ipStreamerGlobalCtx->transcoderDstList[i].finishEvent)
            BKNI_DestroyEvent(ipStreamerGlobalCtx->transcoderDstList[i].finishEvent);
    }
    BDBG_MSG(("%s: Transcoder Dst Un-Initialized", BSTD_FUNCTION));
}

#include "tshdrbuilder.h"
/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#define IP_STREAMER_TS_HEADER_BUF_LENGTH   189
static void
B_IpStreamer_AddPatPmt(
    void *pat,
    void *pmt,
    uint16_t pmtPid,
    uint16_t pcrPid,
    uint16_t vidPid,
    uint16_t audioPid,
    uint8_t  videoStreamType,
    uint8_t  audioStreamType
    )
{
    uint8_t pat_pl_buf[IP_STREAMER_TS_HEADER_BUF_LENGTH], pmt_pl_buf[IP_STREAMER_TS_HEADER_BUF_LENGTH];
    size_t pat_pl_size, pmt_pl_size;
    size_t buf_used = 0;
    size_t payload_pked = 0;
    unsigned streamNum;

    TS_PAT_state patState;
    TS_PSI_header psi_header;
    TS_PMT_state pmtState;
    TS_PAT_program program;
    TS_PMT_stream pmt_stream;

    TS_PID_info pidInfo;
    TS_PID_state pidState;

    /* == CREATE PSI TABLES == */
    TS_PSI_header_Init(&psi_header);
    TS_PAT_Init(&patState, &psi_header, pat_pl_buf, IP_STREAMER_TS_HEADER_BUF_LENGTH);

    TS_PAT_program_Init(&program, 1, pmtPid);
    TS_PAT_addProgram(&patState, &pmtState, &program, pmt_pl_buf, IP_STREAMER_TS_HEADER_BUF_LENGTH);

    TS_PMT_setPcrPid(&pmtState, pcrPid);

    TS_PMT_stream_Init(&pmt_stream, videoStreamType, vidPid);
    TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);

    if (audioPid) {
        TS_PMT_stream_Init(&pmt_stream, audioStreamType, audioPid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    TS_PID_info_Init(&pidInfo, 0x0, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pat, IP_STREAMER_TS_HEADER_BUF_LENGTH, &buf_used, patState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pat + buf_used, pat_pl_buf, pat_pl_size);

    TS_PID_info_Init(&pidInfo, pmtPid, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pmt, IP_STREAMER_TS_HEADER_BUF_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pmt + buf_used, pmt_pl_buf, pmt_pl_size);
}

static void
B_IpStreamer_InsertSystemDataTimer(
    void *context)
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    uint8_t ccByte;
    TranscoderDst *transcoderDst = context;

    if (!transcoderDst)
        return;

    ++transcoderDst->ccValue;/* increment CC synchronously with PAT/PMT */
    ccByte = *((uint8_t*)transcoderDst->pat[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4); /* the 1st byte of pat/pmt arrays is for TSheader builder use */

    /* need to increment CC value for PAT/PMT packets */
    ccByte = (ccByte & 0xf0) | (transcoderDst->ccValue & 0xf);
    *((uint8_t*)transcoderDst->pat[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4) = ccByte;
    /* need to flush the cache before set to TS mux hw */
    NEXUS_Memory_FlushCache((void*)((uint8_t*)transcoderDst->pat[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4), 1);
    /* ping pong PAT pointer */
    transcoderDst->psi[0].pData = (void*)((uint8_t*)transcoderDst->pat[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 1);

    ccByte = *((uint8_t*)transcoderDst->pmt[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4);
    ccByte = (ccByte & 0xf0) | (transcoderDst->ccValue & 0xf);
    *((uint8_t*)transcoderDst->pmt[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4) = ccByte;
    NEXUS_Memory_FlushCache((void*)((uint8_t*)transcoderDst->pmt[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4), 1);
    /* ping pong PMT pointer */
    transcoderDst->psi[1].pData = (void*)((uint8_t*)transcoderDst->pmt[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 1);

    rc = NEXUS_StreamMux_AddSystemDataBuffer(transcoderDst->streamMux, &transcoderDst->psi[0]);
    if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); goto error; }
    rc = NEXUS_StreamMux_AddSystemDataBuffer(transcoderDst->streamMux, &transcoderDst->psi[1]);
    if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); goto error; }
#if 0
    BDBG_MSG(("insert PAT&PMT... ccPAT = %x ccPMT=%x", *((uint8_t*)transcoderDst->pat[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4) & 0xf,
                *((uint8_t*)transcoderDst->pmt[transcoderDst->ccValue  % IP_STREAMER_PSI_QUEUE_CNT] + 4) & 0xf));
#endif
    if (transcoderDst->systemDataTimerIsStarted) {
        transcoderDst->systemDataTimer = B_Scheduler_StartTimer(
                transcoderDst->schedulerSystemData,transcoderDst->mutexSystemData, 1000, B_IpStreamer_InsertSystemDataTimer, transcoderDst);
        if (transcoderDst->systemDataTimer==NULL) {
            BDBG_ERR(("%s: Failed to schedule timer to periodically insert PAT/PMT", BSTD_FUNCTION));
            goto error;
        }
    }
error:
    return;
}

static void
B_IpStreamer_StopSystemData(
    IpStreamerCtx *ipStreamerCtx)
{
    unsigned i;
    TranscoderDst *transcoderDst;

    BDBG_MSG(("%s: enter", BSTD_FUNCTION));
    if (!ipStreamerCtx || !(transcoderDst = ipStreamerCtx->transcoderDst))
        return;

    /* cancel system data timer */
    if (transcoderDst->systemDataTimerIsStarted) {
        B_Scheduler_CancelTimer(transcoderDst->schedulerSystemData, transcoderDst->systemDataTimer);
        B_Scheduler_Stop(transcoderDst->schedulerSystemData);
        transcoderDst->systemDataTimerIsStarted = false;
    }
    if (transcoderDst->schedulerSystemData) {
        B_Scheduler_Destroy(transcoderDst->schedulerSystemData);
        transcoderDst->schedulerSystemData = NULL;
    }
    if (transcoderDst->schedulerThread) {
        B_Thread_Destroy(transcoderDst->schedulerThread);
        transcoderDst->schedulerThread = NULL;
    }
    if (transcoderDst->mutexSystemData) {
        B_Mutex_Destroy(transcoderDst->mutexSystemData);
        transcoderDst->mutexSystemData = NULL;
    }

    for (i=0; i<IP_STREAMER_PSI_QUEUE_CNT; i++) {
        if (transcoderDst->pat[i]) NEXUS_Memory_Free(transcoderDst->pat[i]);
        if (transcoderDst->pmt[i]) NEXUS_Memory_Free(transcoderDst->pmt[i]);
    }
    BDBG_MSG(("%s: exit", BSTD_FUNCTION));
}

/*******************************
 * Add system data to stream_mux
 */
static bool
B_IpStreamer_SetupSystemData(
    IpStreamerCtx *ipStreamerCtx
    )
{
    unsigned i;
    uint8_t videoStreamType = 0, audioStreamType = 0;
    uint16_t audioPid = 0, pcrPid, videoPid = 0, pmtPid;
    TranscoderDst *transcoderDst;
    B_ThreadSettings settingsThread;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_MemoryAllocationSettings allocSettings;

    if (!ipStreamerCtx || !(transcoderDst = ipStreamerCtx->transcoderDst))
        return false;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    if (ipStreamerCtx->globalCtx->globalCfg.heapHandle)
        allocSettings.heap = ipStreamerCtx->globalCtx->globalCfg.heapHandle;
    BDBG_MSG(("%s: CTX %p, transcoderDst %p", BSTD_FUNCTION, (void *)ipStreamerCtx, (void *)transcoderDst));
    for (i=0; i<IP_STREAMER_PSI_QUEUE_CNT; i++) {
        if (NEXUS_Memory_Allocate(IP_STREAMER_TS_HEADER_BUF_LENGTH, &allocSettings, &transcoderDst->pat[i])) {
            BDBG_ERR(("NEXUS Memory alloc Error at %d, returning..", __LINE__));
            goto error;
        }
        if (NEXUS_Memory_Allocate(IP_STREAMER_TS_HEADER_BUF_LENGTH, &allocSettings, &transcoderDst->pmt[i])) {
            BDBG_ERR(("NEXUS Memory alloc Error at %d, returning..", __LINE__));
            goto error;
        }
    }

    /* decide the av stream types to set in PMT */
    if (ipStreamerCtx->cfg.transcode.outVideoPid) {
        videoPid = ipStreamerCtx->cfg.transcode.outVideoPid;
        switch (ipStreamerCtx->cfg.transcode.outVideoCodec) {
            case NEXUS_VideoCodec_eMpeg2:         videoStreamType = 0x2; break;
            case NEXUS_VideoCodec_eMpeg4Part2:    videoStreamType = 0x10; break;
            case NEXUS_VideoCodec_eH264:          videoStreamType = 0x1b; break;
            case NEXUS_VideoCodec_eVc1SimpleMain: videoStreamType = 0xea; break;
            default: BDBG_ERR(("%s: Video encoder codec %d is not supported!", BSTD_FUNCTION, ipStreamerCtx->cfg.transcode.outVideoCodec)); goto error;
        }
        BDBG_MSG(("%s: video pid %d, nexus codec %d, stream type %x", BSTD_FUNCTION, videoPid, ipStreamerCtx->cfg.transcode.outVideoCodec, videoStreamType));
    }

    if (ipStreamerCtx->cfg.transcode.outAudio) {
        audioPid = ipStreamerCtx->cfg.transcode.outAudioPid;
        switch(ipStreamerCtx->cfg.transcode.outAudioCodec) {
            case NEXUS_AudioCodec_eMpeg:         audioStreamType = 0x4; break;
            case NEXUS_AudioCodec_eMp3:          audioStreamType = 0x4; break;
            case NEXUS_AudioCodec_eAacAdts:      audioStreamType = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacPlusAdts:  audioStreamType = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacLoas:      audioStreamType = 0x11; break;/* LOAS */
            case NEXUS_AudioCodec_eAacPlusLoas:  audioStreamType = 0x11; break;/* LOAS */
            case NEXUS_AudioCodec_eAc3:          audioStreamType = 0x81; break;
            default: BDBG_ERR(("Audio encoder codec %d is not supported!", ipStreamerCtx->cfg.transcode.outAudioCodec)); goto error;
        }
        BDBG_MSG(("%s: audio pid %d, nexus codec %d, stream type %x", BSTD_FUNCTION, audioPid, ipStreamerCtx->cfg.transcode.outAudioCodec, audioStreamType));
    }
    pcrPid = ipStreamerCtx->cfg.transcode.outPcrPid;
    pmtPid = ipStreamerCtx->cfg.transcode.outPmtPid;
    BDBG_MSG(("%s: pcr pid %d, pmt pid %d", BSTD_FUNCTION, pcrPid, pmtPid));
    B_IpStreamer_AddPatPmt(transcoderDst->pat[0], transcoderDst->pmt[0], pmtPid, pcrPid, videoPid, audioPid, videoStreamType, audioStreamType);

    for (i=0; i<IP_STREAMER_PSI_QUEUE_CNT; i++) {
        if (i > 0) {
            BKNI_Memcpy(transcoderDst->pat[i], transcoderDst->pat[0], IP_STREAMER_TS_HEADER_BUF_LENGTH);
            BKNI_Memcpy(transcoderDst->pmt[i], transcoderDst->pmt[0], IP_STREAMER_TS_HEADER_BUF_LENGTH);
        }
        NEXUS_Memory_FlushCache(transcoderDst->pat[i], IP_STREAMER_TS_HEADER_BUF_LENGTH);
        NEXUS_Memory_FlushCache(transcoderDst->pmt[i], IP_STREAMER_TS_HEADER_BUF_LENGTH);
    }

    if (!ipStreamerCtx->cfg.hlsSession) {
        /* for HLS Sessions, PAT/PMT are manually inserted at the start of each segment */
        BKNI_Memset(transcoderDst->psi, 0, sizeof(transcoderDst->psi));
        transcoderDst->psi[0].size = 188;
        /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
        transcoderDst->psi[0].pData = (void*)((uint8_t*)transcoderDst->pat[0] + 1);
        transcoderDst->psi[0].timestampDelta = 0;
        transcoderDst->psi[1].size = 188;
        /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
        transcoderDst->psi[1].pData = (void*)((uint8_t*)transcoderDst->pmt[0] + 1);
        transcoderDst->psi[1].timestampDelta = 0;
        rc = NEXUS_StreamMux_AddSystemDataBuffer(transcoderDst->streamMux, &transcoderDst->psi[0]);
        if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); goto error; }
        rc = NEXUS_StreamMux_AddSystemDataBuffer(transcoderDst->streamMux, &transcoderDst->psi[1]);
        if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); goto error; }
        BDBG_MSG(("insert PAT&PMT... ccPAT = %x ccPMT=%x", *((uint8_t*)transcoderDst->pat[0] + 4) & 0xf,
                    *((uint8_t*)transcoderDst->pmt[0] + 4) & 0xf));

        /* schedule a periodic timer to insert PAT/PMT */
        if ((transcoderDst->mutexSystemData = B_Mutex_Create(NULL)) == NULL) {
            BDBG_ERR(("%s: Failed to create mutex", BSTD_FUNCTION));
            goto error;
        }
        if ((transcoderDst->schedulerSystemData = B_Scheduler_Create(NULL)) == NULL) {
            BDBG_ERR(("%s: Failed to create scheduler object", BSTD_FUNCTION));
            goto error;
        }
        /* create thread to run scheduler */
        B_Thread_GetDefaultSettings(&settingsThread);
        transcoderDst->schedulerThread = B_Thread_Create("IpStreamer_SystemDataScheduler",
                (B_ThreadFunc)B_Scheduler_Run,
                transcoderDst->schedulerSystemData,
                &settingsThread);
        if (transcoderDst->schedulerThread == NULL) {
            BDBG_ERR(("%s: Failed to create scheduler thread", BSTD_FUNCTION));
            goto error;
        }
        transcoderDst->systemDataTimer = B_Scheduler_StartTimer(
                transcoderDst->schedulerSystemData,
                transcoderDst->mutexSystemData, 1000, B_IpStreamer_InsertSystemDataTimer, transcoderDst);
        if (transcoderDst->systemDataTimer==NULL) {
            BDBG_ERR(("%s: Failed to schedule timer to periodically insert PAT/PMT", BSTD_FUNCTION));
            goto error;
        }
        transcoderDst->systemDataTimerIsStarted = true;
    }
    BDBG_MSG(("%s: successful", BSTD_FUNCTION));
    return true;

error:
    B_IpStreamer_StopSystemData(ipStreamerCtx);
    return false;
}

void
stopNexusTranscoderDstNonRealTime(
    IpStreamerCtx *ipStreamerCtx
    )
{

    if (ipStreamerCtx->transcoderDst->inUse == 0) {
        BDBG_MSG(("%s: ctx: xcode ctx %p:%p session is already stoped, returning", BSTD_FUNCTION, (void *)ipStreamerCtx, (void *)ipStreamerCtx->transcoderDst));
        return;
    }
    if (ipStreamerCtx->transcoderDst->refCount > 1) {
        BDBG_MSG(("%s: session is being shared w/ another transcoding session, returning", BSTD_FUNCTION));
        return;
    }

    if (ipStreamerCtx->transcoderDst->systemDataTimerIsStarted)
        B_IpStreamer_StopSystemData(ipStreamerCtx);

    if (ipStreamerCtx->transcodingInProgress) {
        NEXUS_VideoEncoderStopSettings stopSettings;
        if (ipStreamerCtx->fileSrc && ipStreamerCtx->fileSrc->playbackHandle)
            NEXUS_Playback_Stop(ipStreamerCtx->fileSrc->playbackHandle);
        NEXUS_StopCallbacks(ipStreamerCtx->transcoderDst->videoDecoder);
        if (ipStreamerCtx->transcoderDst->videoDecoder)
            NEXUS_VideoDecoder_Stop(ipStreamerCtx->transcoderDst->videoDecoder);
        if (ipStreamerCtx->transcoderDst->audioDecoder)
            NEXUS_AudioDecoder_Stop(ipStreamerCtx->transcoderDst->audioDecoder);
        if (ipStreamerCtx->transcoderDst->audioMuxOutput)
            NEXUS_AudioMuxOutput_Stop(ipStreamerCtx->transcoderDst->audioMuxOutput);
        if (ipStreamerCtx->transcoderDst->audioMixer)
            NEXUS_AudioMixer_Stop(ipStreamerCtx->transcoderDst->audioMixer);
        /* wait for stream mux to cleanly finish */
        if (ipStreamerCtx->transcoderDst->streamMux) {
            NEXUS_StreamMux_Finish(ipStreamerCtx->transcoderDst->streamMux);
            /* we are no longer waiting for StreamMux to finish as we stopped the encoder in Abort mode above! */
            NEXUS_StreamMux_Stop(ipStreamerCtx->transcoderDst->streamMux);
        }
        /* stopping encoder after the stream mux */
        if (ipStreamerCtx->transcoderDst->videoEncoder) {
            /* TODO: need a way to see if this is a normal close where app is done sending all data in the file or channel change/error case */
            /* in the later case, we will tell encoder to abort and throw away any pending data */
            stopSettings.mode =  NEXUS_VideoEncoderStopMode_eAbort;
            NEXUS_VideoEncoder_Stop(ipStreamerCtx->transcoderDst->videoEncoder, &stopSettings);
        }
        if (ipStreamerCtx->ipDst && ipStreamerCtx->ipDst->recpumpHandle) {
            NEXUS_Recpump_Stop(ipStreamerCtx->ipDst->recpumpHandle);
            NEXUS_Recpump_RemoveAllPidChannels(ipStreamerCtx->ipDst->recpumpHandle);
        }
        /* since opening of decode stc channel is done in the Start call, closing of the stc channel will need to happen in this stop call */
        if (ipStreamerCtx->transcoderDst->videoStcChannel) {
            BDBG_MSG(("%s: Closing Video stcChannel %p", BSTD_FUNCTION, (void *)ipStreamerCtx->transcoderDst->videoStcChannel));
            NEXUS_StcChannel_Close(ipStreamerCtx->transcoderDst->videoStcChannel);
        }
        if (ipStreamerCtx->transcoderDst->audioStcChannel && ipStreamerCtx->transcoderDst->audioStcChannel != ipStreamerCtx->transcoderDst->videoStcChannel) {
            BDBG_MSG(("%s: Closing Audio stcChannel %p", BSTD_FUNCTION, (void *)ipStreamerCtx->transcoderDst->audioStcChannel));
            NEXUS_StcChannel_Close(ipStreamerCtx->transcoderDst->audioStcChannel);
        }
        ipStreamerCtx->transcoderDst->videoStcChannel = NULL;
        if (ipStreamerCtx->transcoderDst->encodeStcChannel && ipStreamerCtx->cfg.transcode.outAudio && ipStreamerCtx->transcoderDst->audioStcChannel != ipStreamerCtx->transcoderDst->encodeStcChannel) {
            BDBG_MSG(("%s: Closing Encoder stcChannel %p", BSTD_FUNCTION, (void *)ipStreamerCtx->transcoderDst->encodeStcChannel));
            NEXUS_StcChannel_Close(ipStreamerCtx->transcoderDst->encodeStcChannel);
        }
        ipStreamerCtx->transcoderDst->encodeStcChannel = NULL;
        ipStreamerCtx->transcoderDst->audioStcChannel = NULL;

        ipStreamerCtx->transcodingInProgress = false;
        ipStreamerCtx->transcoderDst->started = false;
        BDBG_MSG(("CTX %p: Transcoder dst %p is stopped", (void *)ipStreamerCtx, (void *)ipStreamerCtx->transcoderDst));
    }
}

#define HLS_ADJUSTED_GOP_DURATION 1000
#define HLS_INITIAL_GOP_SIZE_FACTOR 4 /* 1/4th of the frame rate */
void
adjustEncoderSettingsRampdown(IpStreamerCtx *ipStreamerCtx)
{
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    if (ipStreamerCtx->transcoderDst && ipStreamerCtx->transcoderDst->videoEncoder)
    {
        NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
        customFormatSettings.width = 640;
        customFormatSettings.height = 480;
        customFormatSettings.refreshRate = 30000;
        customFormatSettings.sampleAspectRatio.x = 640;
        customFormatSettings.sampleAspectRatio.y = 480;
        customFormatSettings.dropFrameAllowed = true;
        if (NEXUS_Display_SetCustomFormatSettings(ipStreamerCtx->transcoderDst->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings) != 0) {
            BDBG_ERR(("%s: ERROR: NEXUS_Display_SetCustomFormatSettings() Failed", BSTD_FUNCTION));
            return;
        }
        BDBG_MSG(("%s: Successfully Ramped down Custom Format settings for display to 640x480@30hz", BSTD_FUNCTION));

        NEXUS_VideoEncoder_GetSettings(ipStreamerCtx->transcoderDst->videoEncoder, &videoEncoderConfig);
        videoEncoderConfig.streamStructure.duration = HLS_ADJUSTED_GOP_DURATION/HLS_INITIAL_GOP_SIZE_FACTOR;
        NEXUS_VideoEncoder_SetSettings(ipStreamerCtx->transcoderDst->videoEncoder, &videoEncoderConfig);
        BDBG_MSG(("%s: Updated Video Encoder Settings: Duration %d", BSTD_FUNCTION, videoEncoderConfig.streamStructure.duration));
    }
}

void
adjustEncoderSettingsPlaylistChange(IpStreamerCtx *ipStreamerCtx, IpStreamerConfig *ipStreamerCfg)
{
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
    if (ipStreamerCtx->transcoderDst && ipStreamerCtx->transcoderDst->videoEncoder)
    {
        BDBG_MSG(("%s: ###################### change the target display resolution %dx%d", BSTD_FUNCTION, ipStreamerCtx->cfg.transcode.outWidth, ipStreamerCtx->cfg.transcode.outHeight));
        NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
        customFormatSettings.width = ipStreamerCfg->transcode.outWidth;
        customFormatSettings.height = ipStreamerCfg->transcode.outHeight;
        customFormatSettings.refreshRate = 60000;
        customFormatSettings.interlaced = ipStreamerCtx->cfg.transcode.outInterlaced;
        customFormatSettings.aspectRatio = NEXUS_DisplayAspectRatio_eSar;
        customFormatSettings.sampleAspectRatio.x = ipStreamerCfg->transcode.outWidth;
        customFormatSettings.sampleAspectRatio.y = ipStreamerCfg->transcode.outHeight;
        customFormatSettings.dropFrameAllowed = true;
        if (NEXUS_Display_SetCustomFormatSettings(ipStreamerCtx->transcoderDst->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings) != 0) {
            BDBG_ERR(("%s: ERROR: NEXUS_Display_SetCustomFormatSettings() Failed", BSTD_FUNCTION));
            return;
        }
        BDBG_MSG(("%s: Successfully Set Custom Format settings for display", BSTD_FUNCTION));

        NEXUS_VideoEncoder_GetSettings(ipStreamerCtx->transcoderDst->videoEncoder, &videoEncoderConfig);
        videoEncoderConfig.bitrateTarget = ipStreamerCfg->transcode.transportBitrate;
        NEXUS_VideoEncoder_SetSettings(ipStreamerCtx->transcoderDst->videoEncoder, &videoEncoderConfig);
        BDBG_MSG(("%s: Updated Video Encoder Settings: transportBitrate %d", BSTD_FUNCTION, videoEncoderConfig.bitrateTarget));
    }
}

int
seekNexusTranscoderPipeNonRealTime(
    IpStreamerCtx *ipStreamerCtx,
    TranscoderDst *transcoderDst,
    NEXUS_PlaybackPosition seekPosition
    )
{
    IpStreamerConfig *ipStreamerCfg = &ipStreamerCtx->cfg;
    if (ipStreamerCtx->transcoderDst->inUse == 0) {
        BDBG_MSG(("%s: ctx: xcode ctx %p:%p session is already stoped, returning", BSTD_FUNCTION, (void *)ipStreamerCtx, (void *)ipStreamerCtx->transcoderDst));
        return -1;
    }
    if (ipStreamerCtx->transcoderDst->refCount > 1) {
        BDBG_MSG(("%s: session is being shared w/ another transcoding session, returning", BSTD_FUNCTION));
        return -1;
    }

    BDBG_MSG(("CTX %p: Transcoder dst %p is being stopped", (void *)ipStreamerCtx, (void *)ipStreamerCtx->transcoderDst));
    {
        /* stop the pipe from front to back */
        NEXUS_VideoEncoderStopSettings stopSettings;
        if (ipStreamerCtx->fileSrc->playbackHandle)
            NEXUS_Playback_Stop(ipStreamerCtx->fileSrc->playbackHandle);

        /* stop streamMux before encoder to allow it to read complete the completed video descriptors */
        if (ipStreamerCtx->transcoderDst->streamMux) {
            /* we are no longer waiting for StreamMux to finish as we stopp the encoder in Abort mode! */
            NEXUS_StreamMux_Stop(ipStreamerCtx->transcoderDst->streamMux);
        }

        /* stop audio */
        if (ipStreamerCtx->transcoderDst->audioDecoder)
            NEXUS_AudioDecoder_Stop(ipStreamerCtx->transcoderDst->audioDecoder);
        if (ipStreamerCtx->transcoderDst->audioMixer)
            NEXUS_AudioMixer_Stop(ipStreamerCtx->transcoderDst->audioMixer);
        if (ipStreamerCtx->transcoderDst->audioMuxOutput)
            NEXUS_AudioMuxOutput_Stop(ipStreamerCtx->transcoderDst->audioMuxOutput);

        /* stop video */
        NEXUS_StopCallbacks(ipStreamerCtx->transcoderDst->videoDecoder);
        if (ipStreamerCtx->transcoderDst->videoDecoder)
            NEXUS_VideoDecoder_Stop(ipStreamerCtx->transcoderDst->videoDecoder);
        if (ipStreamerCtx->transcoderDst->videoEncoder) {
            NEXUS_VideoEncoder_GetDefaultStopSettings(&stopSettings);
            stopSettings.mode =  NEXUS_VideoEncoderStopMode_eAbort;
            NEXUS_VideoEncoder_Stop(ipStreamerCtx->transcoderDst->videoEncoder, &stopSettings);
        }

        if (ipStreamerCtx->ipDst && ipStreamerCtx->ipDst->recpumpHandle) {
            NEXUS_Recpump_Stop(ipStreamerCtx->ipDst->recpumpHandle);
        }
        /* Temporary workaround to flush pending descriptors from NEXUS_AudioMuxOutput prior to restarting it.
           Restarting will flush the pending descriptors. */
        if (ipStreamerCtx->cfg.transcode.outAudio && transcoderDst->audioDecoder)
        {
            const NEXUS_AudioMuxOutputFrame *pBuf,*pBuf2;
            size_t size,size2;
            NEXUS_Error errCode;

            do {
                errCode = NEXUS_AudioMuxOutput_GetBuffer(transcoderDst->audioMuxOutput, &pBuf, &size, &pBuf2, &size2);
                if ( BERR_SUCCESS == errCode )
                {
                    size += size2;
                    if ( size > 0 )
                    {
                        BDBG_MSG(("Flushing %u outstanding audio descriptors", size));
                        NEXUS_AudioMuxOutput_ReadComplete(transcoderDst->audioMuxOutput, size);
                    }
                }
            } while ( size > 0 );
        }
#if 0
        if (ipStreamerCtx->transcoderDst->videoStcChannel)
            NEXUS_StcChannel_Invalidate(ipStreamerCtx->transcoderDst->videoStcChannel);
        if (ipStreamerCtx->cfg.transcode.outAudio && ipStreamerCtx->transcoderDst->audioStcChannel && ipStreamerCtx->transcoderDst->audioStcChannel != ipStreamerCtx->transcoderDst->videoStcChannel)
            NEXUS_StcChannel_Invalidate(ipStreamerCtx->transcoderDst->audioStcChannel);
        if (ipStreamerCtx->transcoderDst->encodeStcChannel && ipStreamerCtx->cfg.transcode.outAudio && ipStreamerCtx->transcoderDst->audioStcChannel != ipStreamerCtx->transcoderDst->encodeStcChannel)
            NEXUS_StcChannel_Invalidate(ipStreamerCtx->transcoderDst->encodeStcChannel);
#endif

        BDBG_MSG(("CTX %p: Transcoder dst %p is stopped", (void *)ipStreamerCtx, (void *)ipStreamerCtx->transcoderDst));
    }
    /* restart the xcode pipe */
    {
        NEXUS_Error rc = NEXUS_UNKNOWN;
        /* start mux */
        rc = NEXUS_StreamMux_Start(transcoderDst->streamMux, &transcoderDst->muxConfig, &transcoderDst->muxOutput);
        if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); goto error; }

        BDBG_MSG(("%s: Stream Mux is started for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));

        rc = NEXUS_Recpump_Start(ipStreamerCtx->ipDst->recpumpHandle);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
        BDBG_MSG(("%s: Recpump is started for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));
        if (transcoderDst->videoDecoder)
        {
            /* Start decoder */
            rc = NEXUS_VideoDecoder_Start(transcoderDst->videoDecoder, &transcoderDst->videoProgram);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
                goto error;
            }
            BDBG_MSG(("%s: Video Decoder is started for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));
        }
        if (ipStreamerCtx->cfg.transcode.outAudio && transcoderDst->audioDecoder)
        {
            rc = NEXUS_AudioMuxOutput_Start(transcoderDst->audioMuxOutput, &transcoderDst->audioMuxStartSettings);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
                goto error;
            }
            BDBG_MSG(("%s: Audio Mux is started for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));
            rc = NEXUS_AudioMixer_Start(transcoderDst->audioMixer);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
                goto error;
            }
            rc = NEXUS_AudioDecoder_Start(transcoderDst->audioDecoder, &transcoderDst->audioProgram);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
                goto error;
            }
            BDBG_MSG(("%s: Audio Decoder is started for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));
        }
#ifdef NEXUS_HAS_PLAYBACK
        if (ipStreamerCfg->srcType == IpStreamerSrc_eFile && ipStreamerCtx->fileSrc->playbackHandle) {
            NEXUS_PlaybackSettings playbackSettings;
            NEXUS_PlaybackStartSettings startSettings;

            NEXUS_Playback_GetSettings(ipStreamerCtx->fileSrc->playbackHandle, &playbackSettings);
            playbackSettings.startPaused = true;
            if (NEXUS_Playback_SetSettings(ipStreamerCtx->fileSrc->playbackHandle, &playbackSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: ERROR: NEXUS_Playback_SetSettings Failed", BSTD_FUNCTION));
                return -1;
            }
            NEXUS_Playback_GetDefaultStartSettings(&startSettings);
            BDBG_MSG(("%s: restarted playback w/ pause to position %d", BSTD_FUNCTION, (int)seekPosition));
            if (NEXUS_Playback_Start(ipStreamerCtx->fileSrc->playbackHandle, ipStreamerCtx->fileSrc->filePlayHandle, &startSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: ERROR: Failed to start File Streaming handle", BSTD_FUNCTION));
                return -1;
            }
            if (NEXUS_Playback_Seek(ipStreamerCtx->fileSrc->playbackHandle, seekPosition) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Playback_Seek() Failed: seekPosition %d", BSTD_FUNCTION, (int)seekPosition));
                return -1;
            }
            BDBG_MSG(("%s: seeked playback to position %d", BSTD_FUNCTION, (int)seekPosition));
            if (NEXUS_Playback_Play(ipStreamerCtx->fileSrc->playbackHandle) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: ERROR: Failed to start File Streaming handle", BSTD_FUNCTION));
                return -1;
            }
        }
#endif

        /* start video encoder */
        adjustEncoderSettingsRampdown(ipStreamerCtx);
        if (NEXUS_VideoEncoder_SetSettings(transcoderDst->videoEncoder, &transcoderDst->videoEncoderConfig)) {
            BDBG_ERR(("%s: ERROR: Failed to set the Video Encoder Configuration", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: video encoder %p settings are updated for transcode ctx %p", BSTD_FUNCTION, (void *)transcoderDst->videoEncoder, (void *)transcoderDst));

        rc = NEXUS_VideoEncoder_Start(transcoderDst->videoEncoder, &transcoderDst->videoEncoderStartConfig);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            goto error;
        }
        BDBG_MSG(("%s: Video Encoder is started for transcoder ctx %p, CTX %p", BSTD_FUNCTION, (void *)transcoderDst, (void *)ipStreamerCtx));

        BDBG_MSG(("%s: CTX %p: Transcoder Dst %p is restarted ", BSTD_FUNCTION, (void *)ipStreamerCtx, (void *)transcoderDst));
        return 0;

error:
        /* we dont cleanup here as next seek or channel change would cleanup the xcode pipe */
        return -1;
    }
}

extern int stopNexusDst( IpStreamerCtx *ipStreamerCtx);
extern void closeNexusDst( IpStreamerCtx *ipStreamerCtx);

void
closeNexusVice2TranscoderPipe(
    IpStreamerCtx *ipStreamerCtx,
    TranscoderDst *transcoderDst
    )
{
    if (!ipStreamerCtx || !transcoderDst)
        return;
    if (transcoderDst->refCount > 0)
        transcoderDst->refCount--;
    if (transcoderDst->refCount == 0) {
        BDBG_MSG(("%s: Closing transcoder pipe, ref cnt %d", BSTD_FUNCTION, transcoderDst->refCount));
        if (ipStreamerCtx->transcodingInProgress) {
            /* stop the transcoding pipe first as its stop was delayed to the close call */
            stopNexusTranscoderDstNonRealTime(ipStreamerCtx);
            if (ipStreamerCtx->cfg.transcode.outAudio
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
                && !ipStreamerCtx->cfg.transcode.nonRealTime
#endif
            )
            {
                NEXUS_PlatformConfiguration platformConfig;

                NEXUS_Platform_GetConfiguration(&platformConfig);

#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
                /* Dis-connect mixer from dummy output */
                if (NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[transcoderDst->contextId]), NEXUS_AudioMixer_GetConnector(transcoderDst->audioMixer)) != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s: ERROR: NEXUS_AudioOutput_AddInput() failed to dis-connect mixer from dummy audio output", BSTD_FUNCTION));
                }
#else
                /* Dis-connect mixer from dummy output */
                if (NEXUS_AudioOutput_RemoveInput(
                    NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[transcoderDst->contextId]),
                    NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo)) != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s: ERROR: NEXUS_AudioOutput_AddInput() failed to dis-connect decoder from dummy audio output", BSTD_FUNCTION));
                }
                BDBG_MSG(("%s: dummy audio output removed from decoder", BSTD_FUNCTION));
#endif
            }

            /* now stop/close the destination */
            /* this is delayed from the normal stop call as transcoder had to be stopped before we can stop its consumer destination */
            stopNexusDst(ipStreamerCtx);
            closeNexusDst(ipStreamerCtx);
            closeNexusPidChannels(ipStreamerCtx);
            ipStreamerCtx->transcodingInProgress = false;
        }
        else {
            closeNexusDst(ipStreamerCtx);
            closeNexusPidChannels(ipStreamerCtx);
        }
        transcoderDst->inUse = false;
    }
    else {
        BDBG_MSG(("%s: session is being shared (ref cnt %d) w/ another transcoding session, returning", BSTD_FUNCTION, transcoderDst->refCount));
        if (ipStreamerCtx->ipStreamingInProgress) {
            BDBG_MSG(("%s: close ip dst resources", BSTD_FUNCTION));
            stopNexusDst(ipStreamerCtx);
            closeNexusDst(ipStreamerCtx);
        }
    }
    BDBG_MSG(("%s: Done", BSTD_FUNCTION));
}

void
stopNexusTranscoderDst(
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_DisplayCustomFormatSettings customFormatSettings;

    if (ipStreamerCtx->transcoderDst->inUse == 0) {
        BDBG_MSG(("%s: ctx: xcode ctx %p:%p session is already stoped, returning", BSTD_FUNCTION, (void *)ipStreamerCtx, (void *)ipStreamerCtx->transcoderDst));
        return;
    }
    if (ipStreamerCtx->transcoderDst->refCount > 1) {
        BDBG_MSG(("%s: session is being shared w/ another transcoding session, returning", BSTD_FUNCTION));
        return;
    }

    if (ipStreamerCtx->transcoderDst->systemDataTimerIsStarted)
        B_IpStreamer_StopSystemData(ipStreamerCtx);

    if (ipStreamerCtx->transcodingInProgress) {
        NEXUS_VideoEncoderStopSettings stopSettings;
        if (ipStreamerCtx->fileSrc && ipStreamerCtx->fileSrc->playbackHandle)
            NEXUS_Playback_Stop(ipStreamerCtx->fileSrc->playbackHandle);
        NEXUS_StopCallbacks(ipStreamerCtx->transcoderDst->videoDecoder);
        if (ipStreamerCtx->transcoderDst->videoDecoder)
            NEXUS_VideoDecoder_Stop(ipStreamerCtx->transcoderDst->videoDecoder);
        if (ipStreamerCtx->transcoderDst->videoEncoder) {
            /* TODO: need a way to see if this is a normal close where app is done sending all data in the file or channel change/error case */
            /* in the later case, we will tell encoder to abort and throw away any pending data */
            stopSettings.mode =  NEXUS_VideoEncoderStopMode_eAbort;
            NEXUS_VideoEncoder_Stop(ipStreamerCtx->transcoderDst->videoEncoder, &stopSettings);
        }
        if (ipStreamerCtx->transcoderDst->audioDecoder)
            NEXUS_AudioDecoder_Stop(ipStreamerCtx->transcoderDst->audioDecoder);
        if (ipStreamerCtx->transcoderDst->audioMuxOutput)
            NEXUS_AudioMuxOutput_Stop(ipStreamerCtx->transcoderDst->audioMuxOutput);
        if (ipStreamerCtx->transcoderDst->audioMixer)
            NEXUS_AudioMixer_Stop(ipStreamerCtx->transcoderDst->audioMixer);
        /* wait for stream mux to cleanly finish */
        if (ipStreamerCtx->transcoderDst->streamMux) {
            NEXUS_StreamMux_Finish(ipStreamerCtx->transcoderDst->streamMux);
            /* we are no longer waiting for StreamMux to finish as we stopped the encoder in Abort mode above! */

            /* for transcoded sessions, pid channels need to be removed before stream mux is stopped otherwise, it frees the pid channels */
            if (ipStreamerCtx->cfg.enableTimeshifting) {
                NEXUS_Record_Stop(ipStreamerCtx->ipDst->recordHandle);
                NEXUS_Record_RemoveAllPidChannels(ipStreamerCtx->ipDst->recordHandle);
            }
            else {
                if (ipStreamerCtx->ipStreamingInProgress == true) {
                    NEXUS_Recpump_Stop(ipStreamerCtx->ipDst->recpumpHandle);
                }
                if (ipStreamerCtx->ipDst && ipStreamerCtx->ipDst->recpumpHandle)
                    NEXUS_Recpump_RemoveAllPidChannels(ipStreamerCtx->ipDst->recpumpHandle);
            }
            if (ipStreamerCtx->transcoderDst->streamMux)
                NEXUS_StreamMux_Stop(ipStreamerCtx->transcoderDst->streamMux);
        }
        NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
        customFormatSettings.width = 640;
        customFormatSettings.height = 480;
        customFormatSettings.refreshRate = 30000;
        if (NEXUS_Display_SetCustomFormatSettings(ipStreamerCtx->transcoderDst->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings) != 0) {
            BDBG_ERR(("%s: ERROR: NEXUS_Display_SetCustomFormatSettings() Failed", BSTD_FUNCTION));
        }
        BDBG_MSG(("%s:Successfully Reset Display Custom Format settings to 480p", BSTD_FUNCTION));
        /* since opening of decode stc channel is done in the Start call, closing of the stc channel will need to happen in this stop call */
        if (ipStreamerCtx->transcoderDst->videoStcChannel)
            NEXUS_StcChannel_Close(ipStreamerCtx->transcoderDst->videoStcChannel);
        if (ipStreamerCtx->cfg.transcode.outAudio && ipStreamerCtx->transcoderDst->audioStcChannel && ipStreamerCtx->transcoderDst->audioStcChannel != ipStreamerCtx->transcoderDst->videoStcChannel)
            NEXUS_StcChannel_Close(ipStreamerCtx->transcoderDst->audioStcChannel);
        ipStreamerCtx->transcoderDst->videoStcChannel = NULL;
        if (ipStreamerCtx->transcoderDst->encodeStcChannel && ipStreamerCtx->cfg.transcode.outAudio && ipStreamerCtx->transcoderDst->audioStcChannel != ipStreamerCtx->transcoderDst->encodeStcChannel)
            NEXUS_StcChannel_Close(ipStreamerCtx->transcoderDst->encodeStcChannel);
        ipStreamerCtx->transcoderDst->encodeStcChannel = NULL;
        ipStreamerCtx->transcoderDst->audioStcChannel = NULL;

        ipStreamerCtx->transcodingInProgress = false;
        ipStreamerCtx->transcoderDst->started = false;
        BDBG_MSG(("CTX %p: Transcoder dst %p is stopped", (void *)ipStreamerCtx, (void *)ipStreamerCtx->transcoderDst));
    }
}

void
closeNexusRaagaTranscoderPipe(
    IpStreamerCtx *ipStreamerCtx,
    TranscoderDst *transcoderDst
    )
{
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif
    if (!ipStreamerCtx || !transcoderDst)
        return;
    if (transcoderDst->refCount > 0)
        transcoderDst->refCount--;
    if (transcoderDst->refCount == 0) {
        BDBG_MSG(("%s: Closing transcoder pipe, ref cnt %d", BSTD_FUNCTION, transcoderDst->refCount));
        if (ipStreamerCtx->transcodingInProgress) {
            /* stop the transcoding pipe first as its stop was delayed to the close call */
            stopNexusTranscoderDst(ipStreamerCtx);

            /* now stop/close the destination */
            /* this is delayed from the normal stop call as transcoder had to be stopped before we can stop its consumer destination */
            stopNexusDst(ipStreamerCtx);
            closeNexusDst(ipStreamerCtx);
            closeNexusPidChannels(ipStreamerCtx);
            ipStreamerCtx->transcodingInProgress = false;
        }
        else {
            closeNexusDst(ipStreamerCtx);
            closeNexusPidChannels(ipStreamerCtx);
        }
#if NEXUS_HAS_SYNC_CHANNEL
        /* disconnect sync channel */
        if (transcoderDst->syncChannel) {
            NEXUS_SyncChannel_GetSettings(transcoderDst->syncChannel, &syncChannelSettings);
            syncChannelSettings.videoInput = NULL;
            syncChannelSettings.audioInput[0] = NULL;
            syncChannelSettings.audioInput[1] = NULL;
            NEXUS_SyncChannel_SetSettings(transcoderDst->syncChannel, &syncChannelSettings);
            NEXUS_SyncChannel_Destroy(transcoderDst->syncChannel);
            transcoderDst->syncChannel = NULL;
        }
#endif
        if (transcoderDst->timebase != NEXUS_Timebase_eInvalid)
            NEXUS_Timebase_Close(transcoderDst->timebase);
        transcoderDst->timebase = NEXUS_Timebase_eInvalid;

        /* now close the transcoder pipe */
        if (transcoderDst->videoEncoder)
            NEXUS_VideoEncoder_Close(transcoderDst->videoEncoder);
        transcoderDst->videoEncoder = NULL;
        BDBG_MSG(("Closed NEXUS_VideoEncoder_Close ...."));
        if (transcoderDst->streamMux)
            NEXUS_StreamMux_Destroy(transcoderDst->streamMux);
        transcoderDst->streamMux = NULL;
#ifdef B_HAS_DISPLAY_LOCAL_FOR_ENCODE
        if (transcoderDst->windowMain) {
#if NEXUS_HAS_HDMI_INPUT
            if (ipStreamerCtx->cfg.srcType == IpStreamerSrc_eHdmi)
                NEXUS_VideoWindow_RemoveInput(transcoderDst->windowMain, NEXUS_HdmiInput_GetVideoConnector(ipStreamerCtx->hdmiSrc->hdmiInput));
            else
#endif
                NEXUS_VideoWindow_RemoveInput(transcoderDst->windowMain, NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder));
            NEXUS_VideoWindow_Close(transcoderDst->windowMain);
        }
        transcoderDst->windowMain = NULL;
        if (transcoderDst->displayMain)
            NEXUS_Display_Close(transcoderDst->displayMain);
        transcoderDst->displayMain = NULL;
#endif
        if (transcoderDst->windowTranscode) {
            NEXUS_VideoWindow_RemoveAllInputs(transcoderDst->windowTranscode);
            NEXUS_VideoWindow_Close(transcoderDst->windowTranscode);
        }
        transcoderDst->windowTranscode = NULL;
        if (transcoderDst->videoDecoder) {
            NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder));
            NEXUS_VideoDecoder_Close(transcoderDst->videoDecoder);
            if(ipStreamerCtx->globalCtx->globalCfg.multiProcessEnv)
            {   /*informing NEXUS server process that this decoder is free*/
                ipStreamerCtx->globalCtx->globalCfg.releaseDecoder(transcoderDst->decoderIndex);
            }
        }
        transcoderDst->videoDecoder = NULL;
        if (transcoderDst->displayTranscode)
            NEXUS_Display_Close(transcoderDst->displayTranscode);
        transcoderDst->displayTranscode = NULL;

        if (transcoderDst->audioDecoder) {
            NEXUS_PlatformConfiguration platformConfig;

            NEXUS_Platform_GetConfiguration(&platformConfig);
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(transcoderDst->audioMuxOutput));
            NEXUS_AudioMixer_RemoveAllInputs(transcoderDst->audioMixer);
            NEXUS_AudioEncoder_RemoveAllInputs(transcoderDst->audioEncoder);
            NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(transcoderDst->audioEncoder));
            NEXUS_AudioEncoder_Close(transcoderDst->audioEncoder);
            if(ipStreamerCtx->globalCtx->globalCfg.multiProcessEnv)
            {
                ipStreamerCtx->globalCtx->globalCfg.disconnectAudioDummy(transcoderDst->contextId);
            }
            else
            {
                if (!ipStreamerCtx->cfg.transcode.nonRealTime) {
                    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[transcoderDst->contextId]));
                    NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[transcoderDst->contextId]));
                }
            }
            NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
            NEXUS_AudioMuxOutput_Destroy(transcoderDst->audioMuxOutput);
            NEXUS_AudioMixer_Close(transcoderDst->audioMixer);
            NEXUS_AudioDecoder_Close(transcoderDst->audioDecoder);
            transcoderDst->audioDecoder = NULL;
            transcoderDst->audioEncoder = NULL;
            transcoderDst->audioMuxOutput = NULL;
            transcoderDst->audioMixer = NULL;
        }
        transcoderDst->audioDecoder = NULL;
        if (transcoderDst->playpumpTranscodeVideo)
            NEXUS_Playpump_Close(transcoderDst->playpumpTranscodeVideo);
        transcoderDst->playpumpTranscodeVideo = NULL;
        if (transcoderDst->playpumpTranscodeAudio)
            NEXUS_Playpump_Close(transcoderDst->playpumpTranscodeAudio);
        transcoderDst->playpumpTranscodeAudio = NULL;
        if (transcoderDst->playpumpTranscodeSystemData)
            NEXUS_Playpump_Close(transcoderDst->playpumpTranscodeSystemData);
        transcoderDst->playpumpTranscodeSystemData = NULL;
        transcoderDst->inUse = false;
    }
    else {
        BDBG_MSG(("%s: session is being shared (ref cnt %d) w/ another transcoding session, returning", BSTD_FUNCTION, transcoderDst->refCount));
        if (ipStreamerCtx->ipStreamingInProgress) {
            BDBG_MSG(("%s: close ip dst resources", BSTD_FUNCTION));
            stopNexusDst(ipStreamerCtx);
            closeNexusDst(ipStreamerCtx);
        }
    }
    BDBG_MSG(("%s: Done", BSTD_FUNCTION));
}

static void
sourceChangedCallback(void *context, int param)
{
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)context;
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
    BSTD_UNUSED(param);
    if (ipStreamerCtx->transcoderDst && ipStreamerCtx->transcoderDst->videoDecoder) {
        NEXUS_VideoDecoderStatus status;
        if (NEXUS_VideoDecoder_GetStatus(ipStreamerCtx->transcoderDst->videoDecoder, &status) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_VideoDecoder_GetStatus Failed", BSTD_FUNCTION));
            return;
        }
        BDBG_MSG(("####################%s: res: source %dx%d, coded %dx%d, display %dx%d, ar %d, fr %d, interlaced %d video format %d, muted %d", BSTD_FUNCTION,
                    status.source.width, status.source.height,
                    status.coded.width, status.coded.height,
                    status.display.width, status.display.height,
                    status.aspectRatio,
                    status.frameRate,
                    status.interlaced,
                    status.format,
                    status.muted
                    ));
        if (status.source.width < ipStreamerCtx->cfg.transcode.outWidth && status.source.height < ipStreamerCtx->cfg.transcode.outHeight) {
            /* TODO: may need to add check for increasing res as well */
            BDBG_WRN(("%s: lower the display resolution & bitrate, target res %dx%d, source res %dx%d", BSTD_FUNCTION,
                    ipStreamerCtx->cfg.transcode.outWidth, ipStreamerCtx->cfg.transcode.outHeight,
                    status.source.width, status.source.height));
            NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
            customFormatSettings.width = status.source.width;
            customFormatSettings.height = status.source.height;
            customFormatSettings.refreshRate = 30000;
            customFormatSettings.interlaced = ipStreamerCtx->cfg.transcode.outInterlaced;
            customFormatSettings.aspectRatio = NEXUS_DisplayAspectRatio_eSar;
            customFormatSettings.sampleAspectRatio.x = status.source.width;
            customFormatSettings.sampleAspectRatio.y = status.source.height;
            customFormatSettings.dropFrameAllowed = true;
            if (NEXUS_Display_SetCustomFormatSettings(ipStreamerCtx->transcoderDst->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings) != 0) {
                BDBG_ERR(("%s: ERROR: NEXUS_Display_SetCustomFormatSettings() Failed", BSTD_FUNCTION));
                return;
            }
            BDBG_WRN(("%s: Successfully Set Custom Format settings for display", BSTD_FUNCTION));
        }
    }
}

void
adjustEncoderSettings(IpStreamerCtx *ipStreamerCtx, IpStreamerConfig *ipStreamerCfg)
{
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
    if (ipStreamerCtx->transcoderDst && ipStreamerCtx->transcoderDst->videoEncoder)
    {
        BDBG_MSG(("%s: ###################### increase the target display resolution %dx%d", BSTD_FUNCTION, ipStreamerCtx->cfg.transcode.outWidth, ipStreamerCtx->cfg.transcode.outHeight));
        NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
        customFormatSettings.width = ipStreamerCfg->transcode.outWidth;
        customFormatSettings.height = ipStreamerCfg->transcode.outHeight;
        customFormatSettings.refreshRate = 60000;
        customFormatSettings.interlaced = ipStreamerCtx->cfg.transcode.outInterlaced;
        customFormatSettings.aspectRatio = NEXUS_DisplayAspectRatio_eSar;
        customFormatSettings.sampleAspectRatio.x = ipStreamerCfg->transcode.outWidth;
        customFormatSettings.sampleAspectRatio.y = ipStreamerCfg->transcode.outHeight;
        customFormatSettings.dropFrameAllowed = true;
        if (NEXUS_Display_SetCustomFormatSettings(ipStreamerCtx->transcoderDst->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings) != 0) {
            BDBG_ERR(("%s: ERROR: NEXUS_Display_SetCustomFormatSettings() Failed", BSTD_FUNCTION));
            return;
        }
        BDBG_MSG(("%s: Successfully Set Custom Format settings for display", BSTD_FUNCTION));

        NEXUS_VideoEncoder_GetSettings(ipStreamerCtx->transcoderDst->videoEncoder, &videoEncoderConfig);
        videoEncoderConfig.streamStructure.duration = HLS_ADJUSTED_GOP_DURATION;
        NEXUS_VideoEncoder_SetSettings(ipStreamerCtx->transcoderDst->videoEncoder, &videoEncoderConfig);
        BDBG_MSG(("%s: Updated Video Encoder Settings: Duration %d", BSTD_FUNCTION, videoEncoderConfig.streamStructure.duration));
    }
}

#if NEXUS_NUM_DSP_VIDEO_ENCODERS
static int openEncodeStcChannel(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx,
    TranscoderDst *transcoderDst
   )
{
    NEXUS_StcChannelSettings stcSettings;
#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
#else
    BSTD_UNUSED(ipStreamerCfg);
    BSTD_UNUSED(ipStreamerCtx);
#endif
    /* encoders/mux require different STC broadcast mode from decoder */
    /* open 2nd STC channel for this purpose */
    NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
    if (ipStreamerCfg->srcType == IpStreamerSrc_eHdmi)
        stcSettings.timebase = ipStreamerCtx->hdmiSrc->timebase;
    else
#endif
        stcSettings.timebase = transcoderDst->timebase;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;
    transcoderDst->encodeStcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
    if (!transcoderDst->encodeStcChannel) {
        BDBG_ERR(("%s: ERROR: Can't get a free STC Channel Handle", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: opened encode STC Channel", BSTD_FUNCTION));

    return 0;
error:
    return -1;
}
#endif

#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
static void
transcoderFinishCallback(void *context, int param)
{
    TranscoderDst *transcoderDst = (TranscoderDst *)context;

    BSTD_UNUSED(param);
    BDBG_MSG(("Finish callback invoked on transcoder ctx %p, now stop the transcoder", transcoderDst));
    BKNI_SetEvent(transcoderDst->finishEvent);
}

TranscoderDst *
openNexusRaagaTranscoderPipe(
        IpStreamerConfig *ipStreamerCfg,
        IpStreamerCtx *ipStreamerCtx
        )
{
    int i;
    TranscoderDst *transcoderDst = NULL;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    NEXUS_VideoWindowMadSettings madSettings;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoWindowScalerSettings scalerSettings;
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_ClientConfiguration clientConfig;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif

    BDBG_MSG(("%s: Setup transcoder", BSTD_FUNCTION));
    /* open available transcoder dst handle */
    for (transcoderDst=NULL,i=0; i<NEXUS_NUM_DSP_VIDEO_ENCODERS; i++) {
        if (!ipStreamerCtx->globalCtx->transcoderDstList[i].inUse) {
            transcoderDst = &ipStreamerCtx->globalCtx->transcoderDstList[i];
            ipStreamerCtx->transcoderDst = transcoderDst;
            transcoderDst->inUse = true;
            BDBG_MSG(("%s: Found Free Transcoder Dst entry: idx %d, addr %p, total Transcoder dst entries %d", BSTD_FUNCTION, i, transcoderDst, NEXUS_NUM_DSP_VIDEO_ENCODERS));
            break;
        }
    }
    if (!transcoderDst) {
        BDBG_ERR(("%s: ERROR: No Free Transcoder Dst entry, max %d, can't start new transcoder session", BSTD_FUNCTION, i));
        goto error;
    }
    ipStreamerCtx->transcoderDst->refCount = 1;

    /* now setup this transcoding pipeline */

#if NEXUS_HAS_SYNC_CHANNEL
    /* create a sync channel */
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    transcoderDst->syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
    if (!transcoderDst->syncChannel) {
        BDBG_ERR(("%s: ERROR: Failed to create nexus sync channel", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Created nexus sync channel", BSTD_FUNCTION));
#endif

    if ((transcoderDst->timebase = NEXUS_Timebase_Open(NEXUS_ANY_ID)) == NEXUS_Timebase_eInvalid) {
        BDBG_ERR(("%s: ERROR: NEXUS_Timebase_Open Failed to open a free Timebase", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: using timebase %d for decode", BSTD_FUNCTION, transcoderDst->timebase));

    /* video decoder is not needed for straight hdmi sources */
    if (ipStreamerCfg->srcType != IpStreamerSrc_eHdmi) {
        /* open decoder */
        NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
        NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
        if (ipStreamerCfg->srcType == IpStreamerSrc_eIp) {
            videoDecoderOpenSettings.fifoSize = 3*1024*1024; /* 3MB: for over 1 sec of jitter buffer for 20Mpbs stream */
            BDBG_MSG(("%s: Increasing Video Decoder Fifo size to %d", BSTD_FUNCTION, videoDecoderOpenSettings.fifoSize));
        }
        BDBG_MSG(("%s: Turning off svc/mvc decode feature for decoder idx %d", BSTD_FUNCTION, i));
        videoDecoderOpenSettings.enhancementPidChannelSupported = false;

        if(ipStreamerCtx->globalCtx->globalCfg.multiProcessEnv){
            /*in multiprocess environment, NEXUS server process returns the decoder to be used, we do not iterate through the loop*/
            i = ipStreamerCtx->globalCtx->globalCfg.requestDecoder();
            BDBG_MSG(("%s: decoder returned from NEXUS server %d", BSTD_FUNCTION, i));
            if(i != -1) {
                transcoderDst->videoDecoder = NEXUS_VideoDecoder_Open(i, &videoDecoderOpenSettings); /* take default capabilities */
                /*remember the decoder index, NEXUS server needs to be informed at the end of transcode session*/
                transcoderDst->decoderIndex = i;
            } else {
                BDBG_ERR(("%s: ERROR: Can't get a free Decoder from NEXUS server %d", BSTD_FUNCTION, i));
                goto error;
            }
            if(!transcoderDst->videoDecoder) {
                /*inform NEXUS server that this decoder is still unused*/
                ipStreamerCtx->globalCtx->globalCfg.releaseDecoder(transcoderDst->decoderIndex);
                BDBG_ERR(("%s: ERROR: Can't get a free Decoder Handle, max idx %d", BSTD_FUNCTION, i));
                goto error;
            }
         } else {
            /* leaving decoder 0 for local client app (BMC) as it is hardcoded to only use decoder 0 for main  and pip is not supported on raaga based chips yet */
            for (i=1; i<NEXUS_NUM_VIDEO_DECODERS; i++) {
                transcoderDst->videoDecoder = NEXUS_VideoDecoder_Open(i, &videoDecoderOpenSettings); /* take default capabilities */
                if (transcoderDst->videoDecoder)
                    break;
            }
            if (!transcoderDst->videoDecoder) {
                BDBG_ERR(("%s: ERROR: Can't get a free Decoder Handle, max idx %d", BSTD_FUNCTION, i));
                goto error;
            }
        }
      /* turn off svc/mvc decodes to save heap memory space */
        NEXUS_VideoDecoder_GetSettings(transcoderDst->videoDecoder, &videoDecoderSettings);
        videoDecoderSettings.supportedCodecs[NEXUS_VideoCodec_eH264_Svc] = false;
        videoDecoderSettings.supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = false;
        videoDecoderSettings.sourceChanged.callback = sourceChangedCallback;
        videoDecoderSettings.sourceChanged.context = ipStreamerCtx;
        videoDecoderSettings.firstPts.callback = videoFirstPtsCallback;
        videoDecoderSettings.firstPts.context = NULL;
        videoDecoderSettings.firstPtsPassed.callback = videoFirstPtsPassedCallback;
        videoDecoderSettings.firstPtsPassed.context = NULL;
        videoDecoderSettings.ptsError.callback = ptsErrorCallback;
        videoDecoderSettings.firstPtsPassed.context = NULL;

#ifndef IP_STREAMER_DEMO_MODE
        videoDecoderSettings.sourceChanged.callback = sourceChangedCallback;
        videoDecoderSettings.sourceChanged.context = ipStreamerCtx;
#endif
        if (ipStreamerCfg->srcType == IpStreamerSrc_eIp) {
            videoDecoderSettings.ptsOffset = IP_NETWORK_JITTER * 45;    /* In 45Khz clock */
        }
        if (NEXUS_VideoDecoder_SetSettings(transcoderDst->videoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to turn off SVC/MVC decode option on 2nd decoder", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: turned off SVC/MVC decode option on 2nd decoder", BSTD_FUNCTION));
    }

    openEncodeStcChannel(ipStreamerCfg, ipStreamerCtx, transcoderDst);

    /* open & configure encoder */
    for (i=0; i<NEXUS_NUM_DSP_VIDEO_ENCODERS; i++) {
        transcoderDst->videoEncoder = NEXUS_VideoEncoder_Open(i, NULL); /* take default capabilities */
        transcoderDst->contextId = i;
        if (transcoderDst->videoEncoder)
            break;
    }
    if (!transcoderDst->videoEncoder) {
        BDBG_ERR(("%s: ERROR: Can't get a free Video Encoder Handle, max idx %d", BSTD_FUNCTION, i));
        goto error;
    }
    BDBG_MSG(("%s: opened video encoder %p for transcode ctx %p", BSTD_FUNCTION, transcoderDst->videoEncoder, transcoderDst));

    /* open & setup the display: note transcode video format settings are specified here */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e480p;
#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
    if (ipStreamerCfg->srcType == IpStreamerSrc_eHdmi)
        displaySettings.timebase = ipStreamerCtx->hdmiSrc->timebase;
#endif
    transcoderDst->displayTranscode = NEXUS_Display_Open(transcoderDst->displayIndex, &displaySettings);
    if (!transcoderDst->displayTranscode) {
        BDBG_ERR(("%s: ERROR: Can't get a free Display Handle, max idx %d", BSTD_FUNCTION, transcoderDst->displayIndex));
        goto error;
    }

#ifdef B_HAS_DISPLAY_LOCAL_FOR_ENCODE
    {
        NEXUS_PlatformConfiguration platformConfig;
        NEXUS_VideoFormatInfo videoInfo;

        NEXUS_Platform_GetConfiguration(&platformConfig);
        /* Bring up video display and outputs */
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        displaySettings.format = NEXUS_VideoFormat_e720p;
        transcoderDst->displayMain = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
        if(platformConfig.outputs.component[0]){
            NEXUS_Display_AddOutput(transcoderDst->displayMain, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
        }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
        if(platformConfig.outputs.hdmi[0]){
            NEXUS_Display_AddOutput(transcoderDst->displayMain, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
        }
#endif
        if ((transcoderDst->windowMain = NEXUS_VideoWindow_Open(transcoderDst->displayMain, 1)) == NULL) {
            BDBG_ERR(("%s: ERROR: Can't get a free Video Window on Display", BSTD_FUNCTION));
            goto error;
        }
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoInfo);
        NEXUS_VideoWindow_GetSettings(transcoderDst->windowMain, &windowSettings);
        windowSettings.position.x = 0;
        windowSettings.position.y = 0;
        windowSettings.position.width = videoInfo.width;
        windowSettings.position.height = videoInfo.height;
        if (NEXUS_VideoWindow_SetSettings(transcoderDst->windowMain, &windowSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_SetSettings Failed", BSTD_FUNCTION));
            goto error;
        }

    }
#endif
#if 0
    /* TODO: check if this is needed, encode.c app doesn't set it */
    NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
    customFormatSettings.width = ipStreamerCfg->transcode.outWidth;
    customFormatSettings.height = ipStreamerCfg->transcode.outHeight;
    customFormatSettings.refreshRate = 30000;
    customFormatSettings.interlaced = ipStreamerCfg->transcode.outInterlaced;
    customFormatSettings.aspectRatio = NEXUS_DisplayAspectRatio_eSar;
    customFormatSettings.sampleAspectRatio.x = ipStreamerCfg->transcode.outWidth;
    customFormatSettings.sampleAspectRatio.y = ipStreamerCfg->transcode.outHeight;
    customFormatSettings.dropFrameAllowed = true;
    if (NEXUS_Display_SetCustomFormatSettings(transcoderDst->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings) != 0) {
        BDBG_ERR(("%s: ERROR: NEXUS_Display_SetCustomFormatSettings() Failed", BSTD_FUNCTION));
        goto error;
    }
    BDBG_WRN(("%s: Successfully Set Custom Format settings for display", BSTD_FUNCTION));
#endif

    if ((transcoderDst->windowTranscode = NEXUS_VideoWindow_Open(transcoderDst->displayTranscode, 0)) == NULL) {
        BDBG_ERR(("%s: ERROR: Can't get a free Video Window on Display", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: display %p & video window %p are opened for transcoder ctx %p", BSTD_FUNCTION, transcoderDst->displayTranscode, transcoderDst->windowTranscode, transcoderDst));

    /* set transcoder minimum display format before AddInput to avoid black frame transition during dynamic resolution change */
    NEXUS_VideoWindow_GetSettings(transcoderDst->windowTranscode, &windowSettings);
    windowSettings.position.width = ipStreamerCfg->transcode.outWidth;
    windowSettings.position.height = ipStreamerCfg->transcode.outHeight;
    windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;;
    windowSettings.visible = false;
    if (NEXUS_VideoWindow_SetSettings(transcoderDst->windowTranscode, &windowSettings) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_SetSettings Failed for the transcode window", BSTD_FUNCTION));
        goto error;
    }

    /* set transcoder window vnet mode bias to avoid black frame transition during dynamic resolution change */
    NEXUS_VideoWindow_GetScalerSettings(transcoderDst->windowTranscode, &scalerSettings);
    scalerSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
    scalerSettings.bandwidthEquationParams.delta = 1*1000*1000;
    if (NEXUS_VideoWindow_SetScalerSettings(transcoderDst->windowTranscode, &scalerSettings) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_SetScalerSettings Failed for transcode window", BSTD_FUNCTION));
        goto error;
    }

    /* enable deinterlacer to improve quality */
    NEXUS_VideoWindow_GetMadSettings(transcoderDst->windowTranscode, &madSettings);
    if (NEXUS_VideoWindow_SetMadSettings(transcoderDst->windowTranscode, &madSettings) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_SetMadSettings() failed to enable MAD ", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Enabled MAD De-interlacer", BSTD_FUNCTION));


#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
    if (ipStreamerCfg->srcType == IpStreamerSrc_eHdmi) {
        /* connect hdmi to the display (which feeds into encoder) */
        if (NEXUS_VideoWindow_AddInput(transcoderDst->windowTranscode, NEXUS_HdmiInput_GetVideoConnector(ipStreamerCtx->hdmiSrc->hdmiInput)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_AddInput() failed to connect decoder to display", BSTD_FUNCTION));
            goto error;
        }

#ifdef B_HAS_DISPLAY_LOCAL_FOR_ENCODE
        NEXUS_VideoWindow_AddInput(transcoderDst->windowMain, NEXUS_HdmiInput_GetVideoConnector(ipStreamerCtx->hdmiSrc->hdmiInput));
#endif
    }
    else
#endif
    {
        /* connect decoder to the display (which feeds into encoder) */
        if (NEXUS_VideoWindow_AddInput(transcoderDst->windowTranscode, NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_AddInput() failed to connect decoder to display", BSTD_FUNCTION));
            goto error;
        }
    }

#ifdef B_HAS_DISPLAY_LOCAL_FOR_ENCODE
    if (NEXUS_VideoWindow_AddInput(transcoderDst->windowMain, NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder)) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_AddInput() failed to connect decoder to local display", BSTD_FUNCTION));
        goto error;
    }
#endif

    if (ipStreamerCfg->transcode.outAudio) {
        NEXUS_PlatformConfiguration platformConfig;
        NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
        NEXUS_Platform_GetConfiguration(&platformConfig);
        NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
        if (ipStreamerCfg->srcType == IpStreamerSrc_eIp) {
            audioDecoderOpenSettings.fifoSize = 512*1024; /* increasing to act as de-jitter buffer for ip */
            BDBG_MSG(("%s: Increasing Audio Decoder Fifo size to %d", BSTD_FUNCTION, audioDecoderOpenSettings.fifoSize));
        }
        for (i=0; i<NEXUS_NUM_AUDIO_DECODERS; i++) {
            transcoderDst->audioDecoder = NEXUS_AudioDecoder_Open(i, &audioDecoderOpenSettings);
            if (transcoderDst->audioDecoder)
                break;
        }
        if (!transcoderDst->audioDecoder) {
            BDBG_ERR(("%s: ERROR: Can't open audio decoder id %d", BSTD_FUNCTION, i));
            goto error;
        }
        BDBG_MSG(("%s: opened audio decoder %p for transcode ctx %p", BSTD_FUNCTION, transcoderDst->audioDecoder, transcoderDst));

        /* Create audio mux output */
        if ((transcoderDst->audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL)) == NULL) {
            BDBG_ERR(("%s: ERROR: Can't create Audio Stream Mux handle", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: Audio Stream Mux %p is created for transcoder ctx %p", BSTD_FUNCTION, transcoderDst->audioMuxOutput, ipStreamerCtx->transcoderDst));

        /* Open audio encoder */
        NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
        encoderSettings.codec = ipStreamerCfg->transcode.outAudioCodec;
        transcoderDst->audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
        if (!transcoderDst->audioEncoder) {
            BDBG_ERR(("%s: ERROR: Can't open audio encoder handle", BSTD_FUNCTION));
            goto error;
        }

        /* Connect encoder to decoder */
        if (NEXUS_AudioEncoder_AddInput(
                    transcoderDst->audioEncoder,
                    NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_AudioEncoder_AddInput() failed to connect encoder w/ decoder", BSTD_FUNCTION));
            goto error;
        }

        /* Connect mux to encoder */
        if (NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioMuxOutput_GetConnector(transcoderDst->audioMuxOutput),
                    NEXUS_AudioEncoder_GetConnector(transcoderDst->audioEncoder)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_AudioOutput_AddInput() failed to connect mux to encoder", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: audio encode is successfully setup", BSTD_FUNCTION));
        /* Connect audio decoder to a dummy output so that encode path doesn't consume a real audio output which can be used for local decode */
        if(ipStreamerCtx->globalCtx->globalCfg.multiProcessEnv){
            /*in multi process environment, NEXUS server process makes the connection*/
            if(!ipStreamerCtx->globalCtx->globalCfg.connectAudioDummy(transcoderDst->audioDecoder, transcoderDst->contextId))   {
                BDBG_ERR(("%s: ERROR: NEXUS_AudioOutput_AddInput() failed to add spdif audio input", BSTD_FUNCTION));
                goto error;
            }
        } else {
            if (NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[transcoderDst->contextId]),
                    NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo)) != NEXUS_SUCCESS) {
                        BDBG_ERR(("%s: ERROR: NEXUS_AudioOutput_AddInput() failed to add spdif audio input", BSTD_FUNCTION));
                        goto error;
            }
        }

#ifdef B_HAS_DISPLAY_LOCAL_FOR_ENCODE
#if NEXUS_NUM_HDMI_OUTPUTS
        if(platformConfig.outputs.hdmi[0]){
            NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        }
        NEXUS_AudioOutput_AddInput( NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

#endif
#endif
        BDBG_MSG(("%s: audio encode is successfully setup", BSTD_FUNCTION));
    }

    /* display is connected to the encoder module during encoder start */

    /* open the stream mux */
    NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.finished.callback = transcoderFinishCallback;
    muxCreateSettings.finished.context = transcoderDst;
    muxCreateSettings.finished.param = 0;
    if ((transcoderDst->streamMux = NEXUS_StreamMux_Create(&muxCreateSettings)) == NULL) {
        BDBG_ERR(("%s: ERROR: Can't create Stream Mux handle", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Stream Mux %p is created for transcoder ctx %p", BSTD_FUNCTION, transcoderDst->streamMux, ipStreamerCtx->transcoderDst));


    /* and finally open the playpump channels that are used by mux to feed the Audio & Video ES as well as System data to the xpt h/w */
    /* that is where the actual muxing of stream happens */

    /* reduce FIFO size allocated for playpump used for the transcoded streams */
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;

    if(ipStreamerCtx->globalCtx->globalCfg.multiProcessEnv)
    {
        NEXUS_Platform_GetClientConfiguration(&clientConfig);
        playpumpConfig.heap = clientConfig.heap[1]; /* playpump requires heap with eFull mapping */
    }

    if ((transcoderDst->playpumpTranscodeVideo = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig)) == NULL) {
        BDBG_ERR(("%s: ERROR: Failed to Open Nexus Playpump idx ", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Nexus Playpump idx is opened for video transcoder ctx %p", BSTD_FUNCTION, ipStreamerCtx->transcoderDst));
    if ((transcoderDst->playpumpTranscodeAudio = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig)) == NULL) {
        BDBG_ERR(("%s: ERROR: Failed to Open Nexus Playpump idx ", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Nexus Playpump idx is opened for audio transcoder ctx %p", BSTD_FUNCTION, ipStreamerCtx->transcoderDst));
    if ((transcoderDst->playpumpTranscodeSystemData = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig)) == NULL) {
        BDBG_ERR(("%s: ERROR: Failed to Open Nexus Playpump idx ", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: Nexus Playpump idx is opened for system data transcoder ctx %p", BSTD_FUNCTION, ipStreamerCtx->transcoderDst));
    BDBG_MSG(("%s: transcoder ctx %p setup is successful", BSTD_FUNCTION, ipStreamerCtx->transcoderDst));

    return transcoderDst;

error:
    closeNexusRaagaTranscoderPipe(ipStreamerCtx, transcoderDst);
    ipStreamerCtx->transcoderDst = NULL;
    return NULL;
}

#else /* !NEXUS_NUM_DSP_VIDEO_ENCODERS */

TranscoderDst *
openNexusVice2TranscoderPipe(
        IpStreamerConfig *ipStreamerCfg,
        IpStreamerCtx *ipStreamerCtx
        )
{
    int i;
    int rc;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_DisplayStgSettings stgSettings;
    TranscoderDst *transcoderDst = NULL;

    BDBG_MSG(("%s: Setup transcoder", BSTD_FUNCTION));
    /* open available transcoder dst handle */
    for (transcoderDst=NULL,i=0; i<NEXUS_NUM_VIDEO_ENCODERS; i++) {
        if (!ipStreamerCtx->globalCtx->transcoderDstList[i].inUse) {
            transcoderDst = &ipStreamerCtx->globalCtx->transcoderDstList[i];
            transcoderDst->contextId = i;
            ipStreamerCtx->transcoderDst = transcoderDst;
            transcoderDst->inUse = true;
            BDBG_MSG(("%s: Found Free Transcoder Dst entry: idx %d, addr %p, total Transcoder dst entries %d", BSTD_FUNCTION, i, (void *)transcoderDst, NEXUS_NUM_VIDEO_ENCODERS));
            break;
        }
    }
    if (!transcoderDst) {
        BDBG_ERR(("%s: ERROR: No Free Transcoder Dst entry, max %d, can't start new transcoder session", BSTD_FUNCTION, i));
        goto error;
    }
    ipStreamerCtx->transcoderDst->refCount = 1;

    BDBG_MSG(("%s: ENTER", BSTD_FUNCTION));
    /* video decoder, stc channel are not needed for straight hdmi sources */
    if (ipStreamerCfg->srcType == IpStreamerSrc_eIp)
    {
        /* turn off svc/mvc decodes to save heap memory space */
        NEXUS_VideoDecoder_GetSettings(transcoderDst->videoDecoder, &videoDecoderSettings);
        videoDecoderSettings.ptsOffset = IP_NETWORK_JITTER * 45;    /* In 45Khz clock */
        if (NEXUS_VideoDecoder_SetSettings(transcoderDst->videoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to turn off SVC/MVC decode option on 2nd decoder", BSTD_FUNCTION));
            goto error;
        }
    }

#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    if (ipStreamerCfg->transcode.outInterlaced)
    {
        NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
        customFormatSettings.interlaced = ipStreamerCfg->transcode.outInterlaced;
        if (NEXUS_Display_SetCustomFormatSettings(transcoderDst->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings) != 0) {
            BDBG_ERR(("%s: ERROR: NEXUS_Display_SetCustomFormatSettings() Failed", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: Successfully Set Custom Format settings for display", BSTD_FUNCTION));
    }
#else
    {
        NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
        if (ipStreamerCfg->transcode.outWidth > 0 && ipStreamerCfg->transcode.outHeight > 0)
        {
            customFormatSettings.width = ipStreamerCfg->transcode.outWidth;
            customFormatSettings.height = ipStreamerCfg->transcode.outHeight;
            customFormatSettings.sampleAspectRatio.x = ipStreamerCfg->transcode.outWidth;
            customFormatSettings.sampleAspectRatio.y = ipStreamerCfg->transcode.outHeight;
        }
        customFormatSettings.refreshRate = 59940; /* fixed input rate */
        customFormatSettings.interlaced = false;
        customFormatSettings.aspectRatio = 0;
        if (NEXUS_Display_SetCustomFormatSettings(transcoderDst->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings) != 0) {
            BDBG_ERR(("%s: ERROR: NEXUS_Display_SetCustomFormatSettings() Failed", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: Successfully Set Custom Format settings for display", BSTD_FUNCTION));
    }
#endif

    BDBG_MSG(("############### dp xcode %p", (void *)transcoderDst->displayTranscode));
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    if (ipStreamerCfg->transcode.nonRealTime)
#endif
    {
        NEXUS_Display_GetStgSettings(transcoderDst->displayTranscode, &stgSettings);
        stgSettings.enabled = true;
        stgSettings.nonRealTime = ipStreamerCfg->transcode.nonRealTime;
        NEXUS_Display_SetStgSettings(transcoderDst->displayTranscode, &stgSettings);
        BDBG_MSG(("%s: stg settings setup", BSTD_FUNCTION));
    }

    NEXUS_VideoDecoder_GetSettings(transcoderDst->videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.sourceChanged.callback = sourceChangedCallback;
    videoDecoderSettings.sourceChanged.context = ipStreamerCtx;
    if (NEXUS_VideoDecoder_SetSettings(transcoderDst->videoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: Failed to turn off SVC/MVC decode option on 2nd decoder", BSTD_FUNCTION));
        goto error;
    }
#if 0
    /* need to study how long does this adding video decoder or hdmi input to transocde window takes. if it is 50+msec, then why pay penalty for file src where initial HLS latency is very important */
    /* ssood: Note: NEXUS_VideoWindow_AddInput() takes about 70msec */
#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
    if (ipStreamerCfg->srcType == IpStreamerSrc_eHdmi) {
        /* connect hdmi to the display (which feeds into encoder) */
        if (NEXUS_VideoWindow_AddInput(transcoderDst->windowTranscode, NEXUS_HdmiInput_GetVideoConnector(ipStreamerCtx->hdmiSrc->hdmiInput)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_AddInput() failed to connect decoder to display", BSTD_FUNCTION));
            goto error;
        }

#ifdef B_HAS_DISPLAY_LOCAL_FOR_ENCODE
        NEXUS_VideoWindow_AddInput(transcoderDst->windowMain, NEXUS_HdmiInput_GetVideoConnector(ipStreamerCtx->hdmiSrc->hdmiInput));
#endif
    }
    else
#endif
    {
        /* connect decoder to the display (which feeds into encoder) */
        if (NEXUS_VideoWindow_AddInput(transcoderDst->windowTranscode, NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_VideoWindow_AddInput() failed to connect decoder to display", BSTD_FUNCTION));
            goto error;
        }
#ifdef B_HAS_DISPLAY_LOCAL_FOR_ENCODE
        NEXUS_VideoWindow_AddInput(transcoderDst->windowMain, NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder));
#endif
    }
#endif /* if 0 */
    if (ipStreamerCfg->transcode.outAudio)
    {
        NEXUS_AudioEncoder_GetSettings(transcoderDst->audioEncoder, &encoderSettings);
        encoderSettings.codec = ipStreamerCfg->transcode.outAudioCodec;
        rc = NEXUS_AudioEncoder_SetSettings(transcoderDst->audioEncoder, &encoderSettings);
        if (rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: Can't set settings audio encoder handle", BSTD_FUNCTION));
            goto error;
        }
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
        if (!ipStreamerCfg->transcode.nonRealTime)
#endif
        {
            NEXUS_PlatformConfiguration platformConfig;

            NEXUS_Platform_GetConfiguration(&platformConfig);
            NEXUS_AudioOutputSettings outputSettings;
            NEXUS_AudioOutput_GetSettings(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]), &outputSettings);
            /* TODO: can we hardcoder nco to be same for different encoding sessions? */
            outputSettings.nco = NEXUS_AudioOutputNco_e2;
            if (NEXUS_AudioOutput_SetSettings(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]), &outputSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: ERROR: NEXUS_AudioOutput_SetSettings() failed", BSTD_FUNCTION));
                goto error;
            }
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
            /* Connect mixer to dummy */
            if (NEXUS_AudioOutput_AddInput(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[transcoderDst->contextId]), NEXUS_AudioMixer_GetConnector(transcoderDst->audioMixer)) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: ERROR: NEXUS_AudioOutput_AddInput() failed to connect mixer to dummy audio output", BSTD_FUNCTION));
            }
#else
            /* Connect decoder to dummy */
            if (NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[transcoderDst->contextId]),
                NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo)) != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s: ERROR: NEXUS_AudioOutput_AddInput() failed to connect decoder to dummy audio output", BSTD_FUNCTION));
                    goto error;
            }
            BDBG_MSG(("%s: dummy audio output attached to decoder", BSTD_FUNCTION));
#endif
        }
    }

    BDBG_MSG(("%s: Done", BSTD_FUNCTION));
    return transcoderDst;

error:
    closeNexusVice2TranscoderPipe(ipStreamerCtx, transcoderDst);
    return NULL;
}

#endif /* NEXUS_NUM_DSP_VIDEO_ENCODERS */

void
closeNexusTranscoderPipe(
    IpStreamerCtx *ipStreamerCtx,
    TranscoderDst *transcoderDst
    )
{
#if NEXUS_HAS_VIDEO_ENCODER && (!NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
    closeNexusVice2TranscoderPipe(ipStreamerCtx, transcoderDst);
#else
    closeNexusRaagaTranscoderPipe(ipStreamerCtx, transcoderDst);
#endif
}

TranscoderDst *
openNexusTranscoderPipe(
        IpStreamerConfig *ipStreamerCfg,
        IpStreamerCtx *ipStreamerCtx
        )
{
#if NEXUS_HAS_VIDEO_ENCODER && (!NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
    return openNexusVice2TranscoderPipe(ipStreamerCfg, ipStreamerCtx);
#else
    return openNexusRaagaTranscoderPipe(ipStreamerCfg, ipStreamerCtx);
#endif
}

bool
B_IpStreamer_InsertPatPmtTables(
    void *ctx
    )
{
#if 0
    /* commented out till I decide whether app should periodically insert PAT/PMT or let it happen via the a periodic timer thread */
    NEXUS_Error rc = NEXUS_UNKNOWN;
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)ctx;
    TranscoderDst *transcoderDst;

    if (!ipStreamerCtx)
        return false;
    transcoderDst = ipStreamerCtx->transcoderDst;
    if (!transcoderDst)
        return false;

    rc = NEXUS_StreamMux_AddSystemDataBuffer(transcoderDst->streamMux, &transcoderDst->psi[0]);
    if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); goto error; }

    rc = NEXUS_StreamMux_AddSystemDataBuffer(transcoderDst->streamMux, &transcoderDst->psi[1]);
    if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); goto error; }

    if (0)
    {
        /* test code to dynamically control the bitrate */
        int bitrate;
        NEXUS_VideoEncoderSettings videoEncoderConfig;
        BDBG_WRN(("enter new bitrate........."));
        scanf("%d", &bitrate);
        BDBG_WRN(("entered new bitrate.........: %d", bitrate));

        NEXUS_VideoEncoder_GetSettings(transcoderDst->videoEncoder, &videoEncoderConfig);
        videoEncoderConfig.bitrateMax = bitrate;
        NEXUS_VideoEncoder_SetSettings(transcoderDst->videoEncoder, &videoEncoderConfig);
        BDBG_WRN(("%s: ################### modified bitrate to %d ###############", BSTD_FUNCTION, bitrate));

    }
    /* TODO: this sleep shouldn't be needed anymore here, take it out */
    BKNI_Sleep(100);
    return true;;
error:
    return false;;
#else
    BSTD_UNUSED(ctx);
    return true;
#endif
}

extern void * B_PlaybackIp_UtilsAllocateMemory(int size);
#if 0
#define RECORD_CLEAR_DATA
#endif
#ifdef RECORD_CLEAR_DATA
#include <stdio.h>
static FILE * fclear = NULL;
#endif
int
preparePatPmt(
    IpStreamerCtx *ipStreamerCtx
    )
{
    TranscoderDst *transcoderDst = ipStreamerCtx->transcoderDst;
    uint8_t ccByte;

    if (!transcoderDst)
        return 0;

    if (ipStreamerCtx->cfg.streamingCfg.streamingProtocol != B_PlaybackIpProtocol_eHttp)
        /* we only manually insert PAT/PMT for HTTP protocol to meet iOS streaming requirements */
        return 0;

    /* Note: we are inserting PAT/PMT manually as for some reason non-Broadcom clients are not working without it */
    /* that needs to be further looked at */
    transcoderDst->ccValue++;/* increment CC synchronously with PAT/PMT */
    ccByte = *((uint8_t*)transcoderDst->pat[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4); /* the 1st byte of pat/pmt arrays is for TSheader builder use */

    /* need to increment CC value for PAT/PMT packets */
    ccByte = (ccByte & 0xf0) | (transcoderDst->ccValue & 0xf);
    *((uint8_t*)transcoderDst->pat[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4) = ccByte;
    /* need to flush the cache before set to TS mux hw */
    NEXUS_Memory_FlushCache((void*)((uint8_t*)transcoderDst->pat[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4), 1);
    /* ping pong PAT pointer */
    transcoderDst->psi[0].pData = (void*)((uint8_t*)transcoderDst->pat[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 1);

    ccByte = *((uint8_t*)transcoderDst->pmt[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4);
    ccByte = (ccByte & 0xf0) | (transcoderDst->ccValue & 0xf);
    *((uint8_t*)transcoderDst->pmt[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4) = ccByte;
    NEXUS_Memory_FlushCache((void*)((uint8_t*)transcoderDst->pmt[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 4), 1);
    /* ping pong PMT pointer */
    transcoderDst->psi[1].pData = (void*)((uint8_t*)transcoderDst->pmt[transcoderDst->ccValue % IP_STREAMER_PSI_QUEUE_CNT] + 1);

    return 0;
}

#if NEXUS_NUM_DSP_VIDEO_ENCODERS
#define startNexusTranscoderDst startNexusRaagaTranscoderDst
int
startNexusRaagaTranscoderDst(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    unsigned Dee = 0;
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    TranscoderDst *transcoderDst = ipStreamerCtx->transcoderDst;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif
    NEXUS_StcChannelSettings stcSettings;

    if (transcoderDst->started) {
        BDBG_MSG(("%s: transcoder dst (%p) is already started, refCount %d", BSTD_FUNCTION, transcoderDst, transcoderDst->refCount));
        ipStreamerCtx->transcodingInProgress = true;
        ipStreamerCtx->transcodeVideoPidChannel = transcoderDst->transcodeVideoPidChannelCopy;
        ipStreamerCtx->transcodeAudioPidChannel = transcoderDst->transcodeAudioPidChannelCopy;
        if (preparePatPmt(ipStreamerCtx) < 0)
            goto error;
        return 0;
    }

    if (!transcoderDst->encodeStcChannel)
    {
        openEncodeStcChannel(ipStreamerCfg, ipStreamerCtx, transcoderDst);
    }

    if (B_IpStreamer_SetupSystemData(ipStreamerCtx) == false) {
        BDBG_ERR(("%s: CTX %p: Failed to setup the system data for PSI insertion", BSTD_FUNCTION, ipStreamerCtx));
        goto error;
    }
    if (preparePatPmt(ipStreamerCtx) < 0)
        goto error;

#if NEXUS_HAS_SYNC_CHANNEL
    /* connect sync channel */
    if (ipStreamerCtx->cfg.srcType != IpStreamerSrc_eHdmi && !ipStreamerCtx->cfg.transcode.nonRealTime) {
        NEXUS_SyncChannel_GetSettings(transcoderDst->syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder);
        if (transcoderDst->audioDecoder)
            syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
        NEXUS_SyncChannel_SetSettings(transcoderDst->syncChannel, &syncChannelSettings);
    }
#endif

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
    videoEncoderStartConfig.codec = ipStreamerCtx->cfg.transcode.outVideoCodec;
    videoEncoderStartConfig.nonRealTime = false;
    if (ipStreamerCtx->cfg.transcode.outVideoCodec == NEXUS_VideoCodec_eH264) {
        videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
        videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e30;
    }
    else {
        videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eMain;
        videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_eHigh;
    }
    videoEncoderStartConfig.input = transcoderDst->displayTranscode;
    videoEncoderStartConfig.bounds.inputDimension.max.width = ipStreamerCfg->transcode.outWidth;
    videoEncoderStartConfig.bounds.inputDimension.max.height = ipStreamerCfg->transcode.outHeight;
    videoEncoderStartConfig.stcChannel = transcoderDst->encodeStcChannel;

    /* these encoder settings can also be updated during runtime, that is why they are in a different structure */
    NEXUS_VideoEncoder_GetSettings(transcoderDst->videoEncoder, &videoEncoderConfig);
    videoEncoderConfig.bitrateMax = ipStreamerCfg->transcode.transportBitrate;
    videoEncoderConfig.frameRate = ipStreamerCfg->transcode.outFrameRate;
    videoEncoderConfig.streamStructure.framesP = 29;
    videoEncoderConfig.streamStructure.framesB = 0;
    /* NOTE: video encoder delay is in 27MHz ticks */
    NEXUS_VideoEncoder_GetDelayRange(transcoderDst->videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
    BDBG_MSG(("%s: Video encoder end-to-end delay, min = %u ms; maximum allowed: %u ms", BSTD_FUNCTION, videoDelay.min/27000, videoDelay.max/27000));

    if (ipStreamerCtx->cfg.transcode.outAudio) {
        NEXUS_AudioMuxOutput_GetDelayStatus(transcoderDst->audioMuxOutput, ipStreamerCtx->cfg.transcode.outAudioCodec, &audioDelayStatus);
        BDBG_MSG(("%s: Audio codec %d end-to-end delay = %u ms", BSTD_FUNCTION, ipStreamerCtx->cfg.transcode.outAudioCodec, audioDelayStatus.endToEndDelay));
        Dee = audioDelayStatus.endToEndDelay * 27000; /* in 27MHz ticks */
        if (Dee > videoDelay.min) {
            if (Dee > videoDelay.max) {
                Dee = videoDelay.max;
                BDBG_WRN(("Audio Dee is way too big! Use video Dee max %u", Dee));
            }
            else {
                BDBG_MSG(("Use audio Dee %u ms %u ticks@27Mhz", Dee/27000, Dee));
            }
        } else {
            Dee = videoDelay.min;
            BDBG_MSG(("%s: Use video Dee %u ms or %u ticks@27Mhz!", BSTD_FUNCTION, Dee/27000, Dee));
        }
    }
    videoEncoderConfig.encoderDelay = Dee;
    BDBG_MSG(("%s: Video Encoder Settings: FR %d, VFR %d, Bitrate %d, #P %d, #B %d, Delay %d", BSTD_FUNCTION,
                videoEncoderConfig.frameRate,
                videoEncoderConfig.variableFrameRate,
                videoEncoderConfig.bitrateMax,
                videoEncoderConfig.streamStructure.framesP,
                videoEncoderConfig.streamStructure.framesB,
                videoEncoderConfig.encoderDelay
                ));
    if (NEXUS_VideoEncoder_SetSettings(transcoderDst->videoEncoder, &videoEncoderConfig)) {
        BDBG_ERR(("%s: ERROR: Failed to set the Video Encoder Configuration", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: video encoder %p settings are updated for transcode ctx %p", BSTD_FUNCTION, transcoderDst->videoEncoder, transcoderDst));

    /* start mux */
    NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
    muxConfig.transportType = ipStreamerCtx->cfg.transcode.transportType;
    muxConfig.stcChannel = transcoderDst->encodeStcChannel;
    muxConfig.nonRealTime = false;
    muxConfig.video[0].pid = ipStreamerCtx->cfg.transcode.outVideoPid;
    muxConfig.video[0].encoder = transcoderDst->videoEncoder;
    muxConfig.video[0].playpump = transcoderDst->playpumpTranscodeVideo;
    if (ipStreamerCtx->cfg.transcode.outAudio) {
        muxConfig.audio[0].pid = ipStreamerCtx->cfg.transcode.outAudioPid;
        muxConfig.audio[0].muxOutput = transcoderDst->audioMuxOutput;
        muxConfig.audio[0].playpump = transcoderDst->playpumpTranscodeAudio;
    }
    muxConfig.pcr.pid = ipStreamerCtx->cfg.transcode.outPcrPid;
    muxConfig.pcr.playpump = transcoderDst->playpumpTranscodeSystemData;
    muxConfig.pcr.interval = 50; /* PCR interval */
    rc = NEXUS_StreamMux_Start(transcoderDst->streamMux, &muxConfig, &muxOutput);
    if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); goto error; }

    BDBG_MSG(("%s: Stream Mux is started for transcoder ctx %p", BSTD_FUNCTION, transcoderDst));
    ipStreamerCtx->transcodeVideoPidChannel = muxOutput.video[0];
    transcoderDst->transcodeVideoPidChannelCopy = ipStreamerCtx->transcodeVideoPidChannel;
    if (ipStreamerCtx->cfg.transcode.outAudio) {
        ipStreamerCtx->transcodeAudioPidChannel = muxOutput.audio[0];
        transcoderDst->transcodeAudioPidChannelCopy = ipStreamerCtx->transcodeAudioPidChannel;
    }
    /* opening of videoStcChannel is deferred to this point as StcChannel_Open now requires app to provide pcrPidChannel */
    /* which is not available until PSI discovery is complete after the open call. */
    NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
    stcSettings.timebase = transcoderDst->timebase;
    if (ipStreamerCfg->srcType == IpStreamerSrc_eFile) {
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    }
#ifdef NEXUS_HAS_HDMI_INPUT
    else if (ipStreamerCfg->srcType == IpStreamerSrc_eHdmi) {
        stcSettings.timebase = ipStreamerCtx->hdmiSrc->timebase;
        /* need STC channel for audio decoder */
        stcSettings.autoConfigTimebase = false;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    }
#endif
    else if (ipStreamerCfg->srcType == IpStreamerSrc_eIp) {
        /* since live ip has much higher jitter, we need to increase the error thresholds in the dpcr block */

        /* Update STC Channel Settings to accomodate Network Jitter */
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* Live Mode */
        /* offset threshold: uses upper 32 bits (183ticks/msec) of PCR clock */
        stcSettings.modeSettings.pcr.offsetThreshold = IP_NETWORK_JITTER * 183;
        /* max pcr error: uses upper 32 bits (183ticks/msec) of PCR clock */
        stcSettings.modeSettings.pcr.maxPcrError =  IP_NETWORK_JITTER * 183;
        if (ipStreamerCtx->transcoderDst->pcrPidChannel)
            stcSettings.modeSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->pcrPidChannel;
        else
            stcSettings.modeSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->videoPidChannel;
        /*  PCR Offset "Jitter Adjustment" is not suitable for use with IP channels Channels, so disable it */
        stcSettings.modeSettings.pcr.disableJitterAdjustment = true;
        /* Disable Auto Timestamp correction for PCR Jitter */
        stcSettings.modeSettings.pcr.disableTimestampCorrection = true;
        /* We just configured the Timebase, so turn off auto timebase config */
        stcSettings.autoConfigTimebase = false;
        BDBG_MSG (("%s: Configured stc channel with high jitter %d", BSTD_FUNCTION, IP_NETWORK_JITTER));
    }
    else {
        stcSettings.mode = NEXUS_StcChannelMode_ePcr;
        if (ipStreamerCtx->pcrPidChannel)
            stcSettings.modeSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->pcrPidChannel;
        else
            stcSettings.modeSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->videoPidChannel; /* PCR happens to be on video pid */
    }
    if (ipStreamerCfg->srcType != IpStreamerSrc_eHdmi) {
        transcoderDst->videoStcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
        if (!transcoderDst->videoStcChannel) {
            BDBG_ERR(("%s: ERROR: Can't get a free STC Channel Handle", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: opened decode STC Channel", BSTD_FUNCTION));
    }
    if (ipStreamerCfg->srcType == IpStreamerSrc_eIp) {
        /* program the timebase 0: increase its track range & max pcr errors */
        NEXUS_TimebaseSettings timebaseSettings;
        NEXUS_Timebase_GetSettings(transcoderDst->timebase, &timebaseSettings);
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
        timebaseSettings.freeze = false;
        if (ipStreamerCtx->transcoderDst->pcrPidChannel)
            timebaseSettings.sourceSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->pcrPidChannel;
        else
            timebaseSettings.sourceSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->videoPidChannel;
        timebaseSettings.sourceSettings.pcr.maxPcrError = IP_NETWORK_JITTER * 183/2;    /* in milliseconds: based on 90Khz clock */
        timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e244ppm;
        rc = NEXUS_Timebase_SetSettings(transcoderDst->timebase, &timebaseSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, returning..", rc, __LINE__)); goto error;}
        BDBG_MSG (("%s: Configured timebase with high jitter %d", BSTD_FUNCTION, IP_NETWORK_JITTER));
    }

    if (ipStreamerCtx->cfg.srcType != IpStreamerSrc_eHdmi) {
        /* Start decoder */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = psi->videoCodec;
        videoProgram.nonRealTime = false;
        videoProgram.pidChannel = transcoderDst->videoPidChannel;
        videoProgram.stcChannel = transcoderDst->videoStcChannel;
        rc = NEXUS_VideoDecoder_Start(transcoderDst->videoDecoder, &videoProgram);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            goto error;
        }
        BDBG_MSG(("%s: Video Decoder is started for transcoder ctx %p", BSTD_FUNCTION, transcoderDst));
    }

    if (psi->audioPid && ipStreamerCtx->cfg.transcode.outAudio) {
        NEXUS_AudioDecoderStartSettings audioProgram;

        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
        audioMuxStartSettings.stcChannel = transcoderDst->encodeStcChannel;
        audioMuxStartSettings.presentationDelay = Dee/27000; /* in ms */
        audioMuxStartSettings.nonRealTime = false;

        rc = NEXUS_AudioMuxOutput_Start(transcoderDst->audioMuxOutput, &audioMuxStartSettings);
        if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); goto error; }
        BDBG_MSG(("%s: Audio Mux is started for transcoder ctx %p", BSTD_FUNCTION, transcoderDst));

        if (ipStreamerCfg->srcType == IpStreamerSrc_eIp) {
            NEXUS_AudioDecoderSettings audioDecoderSettings;
            NEXUS_AudioDecoder_GetSettings(transcoderDst->audioDecoder, &audioDecoderSettings);
            audioDecoderSettings.ptsOffset = IP_NETWORK_JITTER * 45;    /* In 45Khz clock */
            audioDecoderSettings.firstPts.callback = audioFirstPtsCallback;
            audioDecoderSettings.firstPts.context = NULL;
            rc = NEXUS_AudioDecoder_SetSettings(transcoderDst->audioDecoder, &audioDecoderSettings);
            if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning..", rc, __LINE__)); goto error;}
        }

        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
#if defined(NEXUS_HAS_HDMI_INPUT)
        if (ipStreamerCtx->cfg.srcType == IpStreamerSrc_eHdmi) {
            audioProgram.input = NEXUS_HdmiInput_GetAudioConnector(ipStreamerCtx->hdmiSrc->hdmiInput);
            audioProgram.latencyMode = NEXUS_AudioDecoderLatencyMode_eLow;
        }
        else
#endif
        {
            audioProgram.codec = psi->audioCodec;
            audioProgram.pidChannel = transcoderDst->audioPidChannel;
            audioProgram.stcChannel = transcoderDst->videoStcChannel;
            audioProgram.nonRealTime = false;
        }
        rc = NEXUS_AudioDecoder_Start(transcoderDst->audioDecoder, &audioProgram);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            goto error;
        }
        BDBG_MSG(("%s: Audio Decoder is started for transcoder ctx %p", BSTD_FUNCTION, transcoderDst));
    }

    if (!ipStreamerCfg->hlsSession) {
        NEXUS_StreamMux_AddSystemDataBuffer(transcoderDst->streamMux, &transcoderDst->psi[0]);
        NEXUS_StreamMux_AddSystemDataBuffer(transcoderDst->streamMux, &transcoderDst->psi[1]);
    }

    rc = NEXUS_VideoEncoder_Start(transcoderDst->videoEncoder, &videoEncoderStartConfig);
    if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); goto error; }
    BDBG_MSG(("%s: Video Encoder is started for transcoder ctx %p, CTX %p", BSTD_FUNCTION, transcoderDst, ipStreamerCtx));

    ipStreamerCtx->transcodingInProgress = true;
    transcoderDst->started = true;
    BDBG_MSG(("%s: CTX %p: Transcoder Dst %p is started", BSTD_FUNCTION, ipStreamerCtx, transcoderDst));
    return 0;

error:
    /* stop the transcoding pipe first as its stop was delayed to the close call */
    stopNexusTranscoderDst(ipStreamerCtx);
    return -1;
}
#else
#define startNexusTranscoderDst startNexusVice2TranscoderDst
int
startNexusVice2TranscoderDst(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_VideoEncoderDelayRange videoDelay;
    unsigned Dee = 0;
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif
    TranscoderDst *transcoderDst = ipStreamerCtx->transcoderDst;
    NEXUS_StcChannelSettings stcSettings;

    if (transcoderDst->started) {
        BDBG_MSG(("%s: transcoder dst (%p) is already started, refCount %d", BSTD_FUNCTION, (void *)transcoderDst, transcoderDst->refCount));
        ipStreamerCtx->transcodingInProgress = true;
        ipStreamerCtx->transcodeVideoPidChannel = transcoderDst->transcodeVideoPidChannelCopy;
        ipStreamerCtx->transcodeAudioPidChannel = transcoderDst->transcodeAudioPidChannelCopy;
        if (preparePatPmt(ipStreamerCtx) < 0)
            goto error;
        return 0;
    }

    if (B_IpStreamer_SetupSystemData(ipStreamerCtx) == false) {
        BDBG_ERR(("%s: CTX %p: Failed to setup the system data for PSI insertion", BSTD_FUNCTION, (void *)ipStreamerCtx));
        goto error;
    }
    if (preparePatPmt(ipStreamerCtx) < 0)
        goto error;

    NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
    stcSettings.timebase = transcoderDst->timebase;
    if (ipStreamerCfg->srcType == IpStreamerSrc_eFile) {
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    }
    else if (ipStreamerCfg->srcType == IpStreamerSrc_eIp) {
        /* since live ip has much higher jitter, we need to increase the error thresholds in the dpcr block */

        /* Update STC Channel Settings to accomodate Network Jitter */
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* Live Mode */
        /* offset threshold: uses upper 32 bits (183ticks/msec) of PCR clock */
        stcSettings.modeSettings.pcr.offsetThreshold = IP_NETWORK_JITTER * 183;
        /* max pcr error: uses upper 32 bits (183ticks/msec) of PCR clock */
        stcSettings.modeSettings.pcr.maxPcrError =  IP_NETWORK_JITTER * 183;
        if (ipStreamerCtx->transcoderDst->pcrPidChannel)
            stcSettings.modeSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->pcrPidChannel;
        else
            stcSettings.modeSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->videoPidChannel;
        /*  PCR Offset "Jitter Adjustment" is not suitable for use with IP channels Channels, so disable it */
        stcSettings.modeSettings.pcr.disableJitterAdjustment = true;
        /* Disable Auto Timestamp correction for PCR Jitter */
        stcSettings.modeSettings.pcr.disableTimestampCorrection = true;
        /* We just configured the Timebase, so turn off auto timebase config */
        stcSettings.autoConfigTimebase = false;
        BDBG_MSG (("%s: Configured stc channel with high jitter %d", BSTD_FUNCTION, IP_NETWORK_JITTER));
    }
#if NEXUS_HAS_HDMI_INPUT
    else if(ipStreamerCfg->srcType == IpStreamerSrc_eHdmi ) {
        stcSettings.autoConfigTimebase = false;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    }
#endif
    else {
        stcSettings.mode = NEXUS_StcChannelMode_ePcr;
        /* enable live mode for transcoding case: it is done here as pidChannel needs to be specified in Stc_SetSettings() */
        if (ipStreamerCtx->pcrPidChannel)
            stcSettings.modeSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->pcrPidChannel;
        else
            stcSettings.modeSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->videoPidChannel; /* PCR happens to be on video pid */
    }

    /* open STC channel for the decoders */
#if (BCHP_CHIP == 7445 && BCHP_VER < BCHP_VER_C0)/* 7445A/B video decoder only supports lower 6 STCs */
    transcoderDst->videoStcChannel = NEXUS_StcChannel_Open(transcoderDst->contextId, &stcSettings);
#else
    transcoderDst->videoStcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
#endif
    if (!transcoderDst->videoStcChannel) {
        BDBG_ERR(("%s: ERROR: Can't get a free STC Channel Handle", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: opened decode STC Channel %p, NRT %d", BSTD_FUNCTION, (void *)transcoderDst->videoStcChannel, ipStreamerCtx->cfg.transcode.nonRealTime ));

    /* For transcoding from file src, we can do either RT (default) or NRT (non-realtime) transcode */
    if (ipStreamerCfg->srcType == IpStreamerSrc_eFile && ipStreamerCtx->cfg.transcode.nonRealTime && ipStreamerCtx->cfg.transcode.outAudio) {
        /* if NRT mode is enabled for transcode from file src and audio is also enabled, */
        /* we need separate STCs for audio and video decoders */
#if (BCHP_CHIP == 7445 && BCHP_VER < BCHP_VER_C0)/* 7445A/B video decoder only supports lower 6 STCs */
        transcoderDst->audioStcChannel = NEXUS_StcChannel_Open(transcoderDst->contextId+1, &stcSettings);
#else
        transcoderDst->audioStcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
#endif
        if (!transcoderDst->audioStcChannel) {
            BDBG_ERR(("%s: ERROR: Can't get a free STC Channel Handle for Audio decode in NRT mode", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: opened audio decode STC Channel %p for NRT mode", BSTD_FUNCTION, (void *)transcoderDst->audioStcChannel));
    }
    else {
        /* RT mode uses same STC for audio and video decoders */
        transcoderDst->audioStcChannel = transcoderDst->videoStcChannel;
    }

    /* For transcoding from file src, we can do either RT (default) or NRT (non-realtime) transcode */
    /* NRT mode uses separate STCs for audio and video decoders */
    if (ipStreamerCfg->srcType == IpStreamerSrc_eFile && ipStreamerCtx->cfg.transcode.nonRealTime && ipStreamerCtx->cfg.transcode.outAudio) {
        /* if NRT mode is enabled for transcode from file src and audio is also enabled, */
        /* encoders dont need separate STC and instead can share the audio decoder one */
        transcoderDst->encodeStcChannel = transcoderDst->audioStcChannel;
    }
    else {
        /* RT mode needs separate STC for encoder & mux modules */
        NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
        stcSettings.timebase = transcoderDst->timebase;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;
        BDBG_MSG(("%s: using timebase %d for encode", BSTD_FUNCTION, (int)stcSettings.timebase));
        stcSettings.autoConfigTimebase = false;
#if (BCHP_CHIP == 7445 && BCHP_VER < BCHP_VER_C0)
        transcoderDst->encodeStcChannel = NEXUS_StcChannel_Open(transcoderDst->contextId + NEXUS_NUM_VIDEO_ENCODERS, &stcSettings);
#else
        transcoderDst->encodeStcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
#endif
    }
    if (!transcoderDst->encodeStcChannel) {
        BDBG_ERR(("%s: ERROR: Can't get a free STC Channel Handle", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: opened encode STC Channel %p", BSTD_FUNCTION, (void *)transcoderDst->encodeStcChannel));

#ifdef NEXUS_HAS_PLAYBACK
    if (ipStreamerCfg->srcType == IpStreamerSrc_eFile && ipStreamerCtx->fileSrc->playbackHandle) {
        /* inform stc channel in the nexus playback */
        NEXUS_PlaybackSettings playbackSettings;
        NEXUS_Playback_GetSettings(ipStreamerCtx->fileSrc->playbackHandle, &playbackSettings);
        playbackSettings.stcChannel = ipStreamerCtx->cfg.transcode.nonRealTime ?
            NULL:ipStreamerCtx->transcoderDst->videoStcChannel;
        if (ipStreamerCtx->cfg.transcode.nonRealTime)
            playbackSettings.playpumpSettings.maxDataRate = 108*1000*1000; /* setting the max rate for NRT case */
        if (NEXUS_Playback_SetSettings(ipStreamerCtx->fileSrc->playbackHandle, &playbackSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_Playback_SetSettings() Failed", BSTD_FUNCTION));
            goto error;
        }
    }
#endif

#if NEXUS_HAS_SYNC_CHANNEL
    /* connect sync channel */
    if (ipStreamerCfg->srcType != IpStreamerSrc_eHdmi) {
        NEXUS_SyncChannel_GetSettings(transcoderDst->syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(transcoderDst->videoDecoder);
        syncChannelSettings.enableMuteControl = false;
#if 0
        syncChannelSettings.enablePrecisionLipsync = false;
#endif
        if (transcoderDst->audioDecoder) {
            syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(transcoderDst->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
            /* NRT mode pairs AV stc channels */
            if (ipStreamerCtx->cfg.transcode.nonRealTime && ipStreamerCfg->srcType == IpStreamerSrc_eFile) {
                NEXUS_StcChannelPairSettings stcAudioVideoPair;
                NEXUS_StcChannel_GetDefaultPairSettings(&stcAudioVideoPair);
                stcAudioVideoPair.connected = true;
                stcAudioVideoPair.window = 300; /* 300ms AV window means when source discontinuity occurs, up to 300ms transition could occur with NRT transcoded stream */
                NEXUS_StcChannel_SetPairSettings(transcoderDst->videoStcChannel, transcoderDst->audioStcChannel, &stcAudioVideoPair);
            }
        }
        NEXUS_SyncChannel_SetSettings(transcoderDst->syncChannel, &syncChannelSettings);
    }
#endif

    NEXUS_VideoEncoder_GetDefaultStartSettings(&transcoderDst->videoEncoderStartConfig);
    transcoderDst->videoEncoderStartConfig.interlaced = ipStreamerCfg->transcode.outInterlaced;
    transcoderDst->videoEncoderStartConfig.codec = ipStreamerCtx->cfg.transcode.outVideoCodec;
    if (ipStreamerCtx->cfg.transcode.outVideoCodec == NEXUS_VideoCodec_eH264) {
        transcoderDst->videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eMain;
        transcoderDst->videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e31;
    }
    else {
        transcoderDst->videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eMain;
        transcoderDst->videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_eHigh;
    }
    transcoderDst->videoEncoderStartConfig.input = transcoderDst->displayTranscode;
    transcoderDst->videoEncoderStartConfig.stcChannel = transcoderDst->encodeStcChannel;
    transcoderDst->videoEncoderStartConfig.adaptiveLowDelayMode = true;
    transcoderDst->videoEncoderStartConfig.bounds.inputFrameRate.min = NEXUS_VideoFrameRate_e29_97;
    transcoderDst->videoEncoderStartConfig.bounds.outputFrameRate.min = NEXUS_VideoFrameRate_e29_97;
    transcoderDst->videoEncoderStartConfig.bounds.outputFrameRate.max = NEXUS_VideoFrameRate_e60;
    /* note we were use the default value of rateBufferDelay (3sec) but that seems to be causing quite a bit of initial latency */
    transcoderDst->videoEncoderStartConfig.rateBufferDelay = 1500;
    BDBG_MSG(("%s: ctx %p: rateBufferDelay %d", BSTD_FUNCTION, (void *)ipStreamerCtx, transcoderDst->videoEncoderStartConfig.rateBufferDelay));

    /* it seems encoder default memory bound setting adapted to box mode now; don't override until required so. */

    transcoderDst->videoEncoderStartConfig.nonRealTime = ipStreamerCfg->transcode.nonRealTime;

    /* these encoder settings can also be updated during runtime, that is why they are in a different structure */
    NEXUS_VideoEncoder_GetSettings(transcoderDst->videoEncoder, &transcoderDst->videoEncoderConfig);
    transcoderDst->videoEncoderConfig.bitrateMax = ipStreamerCfg->transcode.transportBitrate*0.9;


    /* NOTE: we are going w/ smaller duration for initial GOPs. This is done so that initial GOPs can be quickly endcoded and streamed out. */
    /* iOS7 clients require initial GOP to arrive in 1 - 1.5 sec duration. */
    transcoderDst->videoEncoderConfig.streamStructure.duration = HLS_ADJUSTED_GOP_DURATION/HLS_INITIAL_GOP_SIZE_FACTOR;
    transcoderDst->videoEncoderConfig.enableFieldPairing = false; /* disabled to lower the initial latency */
    transcoderDst->videoEncoderConfig.frameRate = ipStreamerCfg->transcode.outFrameRate;
    transcoderDst->videoEncoderConfig.variableFrameRate = false;

    /* NOTE: video encoder delay is in 27MHz ticks */
    NEXUS_VideoEncoder_GetDelayRange(transcoderDst->videoEncoder, &transcoderDst->videoEncoderConfig, &transcoderDst->videoEncoderStartConfig, &videoDelay);
    BDBG_MSG(("%s: Video encoder end-to-end delay, min = %u ms; maximum allowed: %u ms", BSTD_FUNCTION, videoDelay.min/27000, videoDelay.max/27000));

    if (psi->audioPid && ipStreamerCtx->cfg.transcode.outAudio) {
        NEXUS_AudioMuxOutput_GetDelayStatus(transcoderDst->audioMuxOutput, ipStreamerCtx->cfg.transcode.outAudioCodec, &audioDelayStatus);
        BDBG_MSG(("%s: Audio codec %d end-to-end delay = %u ms", BSTD_FUNCTION, ipStreamerCtx->cfg.transcode.outAudioCodec, audioDelayStatus.endToEndDelay));
        Dee = audioDelayStatus.endToEndDelay * 27000; /* in 27MHz ticks */
        if (Dee > videoDelay.min) {
            if (Dee > videoDelay.max) {
                Dee = videoDelay.max;
                BDBG_WRN(("Audio Dee is way too big! Use video Dee max %u", Dee));
            }
            else {
                BDBG_MSG(("Use audio Dee %u ms %u ticks@27Mhz", Dee/27000, Dee));
            }
        } else {
            Dee = videoDelay.min;
            BDBG_MSG(("%s: Use video Dee %u ms or %u ticks@27Mhz!", BSTD_FUNCTION, Dee/27000, Dee));
        }
        transcoderDst->videoEncoderConfig.encoderDelay = Dee;
    }
    else {
        transcoderDst->videoEncoderConfig.encoderDelay = videoDelay.min;
    }
    BDBG_MSG(("%s: Video Encoder Settings: FR %d, VFR %d, Bitrate %d, #P %d, #B %d, Delay %d", BSTD_FUNCTION,
                transcoderDst->videoEncoderConfig.frameRate,
                transcoderDst->videoEncoderConfig.variableFrameRate,
                transcoderDst->videoEncoderConfig.bitrateMax,
                transcoderDst->videoEncoderConfig.streamStructure.framesP,
                transcoderDst->videoEncoderConfig.streamStructure.framesB,
                transcoderDst->videoEncoderConfig.encoderDelay
                ));
    if (NEXUS_VideoEncoder_SetSettings(transcoderDst->videoEncoder, &transcoderDst->videoEncoderConfig)) {
        BDBG_ERR(("%s: ERROR: Failed to set the Video Encoder Configuration", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: video encoder %p settings are updated for transcode ctx %p", BSTD_FUNCTION, (void *)transcoderDst->videoEncoder, (void *)transcoderDst));

#ifdef HLS_DISABLE_RAMPUP
    adjustEncoderSettings(ipStreamerCtx, &ipStreamerCtx->cfg);
#endif
    /* start mux */
    NEXUS_StreamMux_GetDefaultStartSettings(&transcoderDst->muxConfig);
    transcoderDst->muxConfig.transportType = ipStreamerCtx->cfg.transcode.transportType;
    transcoderDst->muxConfig.stcChannel = transcoderDst->encodeStcChannel;
    transcoderDst->muxConfig.nonRealTime = ipStreamerCtx->cfg.transcode.nonRealTime;
    transcoderDst->muxConfig.nonRealTimeRate = 8*NEXUS_NORMAL_PLAY_SPEED; /* AFAP */
    transcoderDst->muxConfig.video[0].pid = ipStreamerCtx->cfg.transcode.outVideoPid;
    transcoderDst->muxConfig.video[0].encoder = transcoderDst->videoEncoder;
    transcoderDst->muxConfig.video[0].playpump = transcoderDst->playpumpTranscodeVideo;
    if (ipStreamerCtx->cfg.transcode.outAudio) {
        transcoderDst->muxConfig.audio[0].pid = ipStreamerCtx->cfg.transcode.outAudioPid;
        transcoderDst->muxConfig.audio[0].muxOutput = transcoderDst->audioMuxOutput;
        transcoderDst->muxConfig.audio[0].playpump = transcoderDst->playpumpTranscodeAudio;
    }
    transcoderDst->muxConfig.pcr.pid = ipStreamerCtx->cfg.transcode.outPcrPid;
    transcoderDst->muxConfig.pcr.playpump = transcoderDst->playpumpTranscodeSystemData;
    transcoderDst->muxConfig.servicePeriod = 20; /* ssood: lowering to see any startup latency improvement */ /* TODO: this can be ramped up ! */
    transcoderDst->muxConfig.pcr.interval = 50; /* PCR interval */
    transcoderDst->muxConfig.latencyTolerance = 20;
    transcoderDst->muxConfig.nonRealTime = ipStreamerCfg->transcode.nonRealTime;

    rc = NEXUS_StreamMux_Start(transcoderDst->streamMux, &transcoderDst->muxConfig, &transcoderDst->muxOutput);
    if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); goto error; }

    BDBG_MSG(("%s: Stream Mux is started for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));
    ipStreamerCtx->transcodeVideoPidChannel = transcoderDst->muxOutput.video[0];
    transcoderDst->transcodeVideoPidChannelCopy = ipStreamerCtx->transcodeVideoPidChannel;
    if (ipStreamerCtx->cfg.transcode.outAudio) {
        ipStreamerCtx->transcodeAudioPidChannel = transcoderDst->muxOutput.audio[0];
        transcoderDst->transcodeAudioPidChannelCopy = ipStreamerCtx->transcodeAudioPidChannel;
    }

    if (ipStreamerCfg->srcType == IpStreamerSrc_eIp) {
        /* program the timebase 0: increase its track range & max pcr errors */
        NEXUS_TimebaseSettings timebaseSettings;
        NEXUS_Timebase_GetSettings(transcoderDst->timebase, &timebaseSettings);
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
        timebaseSettings.freeze = false;
        if (ipStreamerCtx->transcoderDst->pcrPidChannel)
            timebaseSettings.sourceSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->pcrPidChannel;
        else
            timebaseSettings.sourceSettings.pcr.pidChannel = ipStreamerCtx->transcoderDst->videoPidChannel;
        timebaseSettings.sourceSettings.pcr.maxPcrError = IP_NETWORK_JITTER * 183/2;    /* in milliseconds: based on 90Khz clock */
        timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e244ppm;
        rc = NEXUS_Timebase_SetSettings(transcoderDst->timebase, &timebaseSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, returning..", rc, __LINE__)); goto error;}
        BDBG_MSG (("%s: Configured timebase with high jitter %d", BSTD_FUNCTION, IP_NETWORK_JITTER));
    }
#if NEXUS_HAS_HDMI_INPUT
    if (ipStreamerCtx->cfg.srcType == IpStreamerSrc_eHdmi) {
        NEXUS_TimebaseSettings timebaseSettings;
        NEXUS_Timebase_GetSettings(transcoderDst->timebase, &timebaseSettings);
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eHdDviIn;
        NEXUS_Timebase_SetSettings(transcoderDst->timebase, &timebaseSettings);
    }
    else
#endif
    {
        /* Start decoder */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&transcoderDst->videoProgram);
        transcoderDst->videoProgram.codec = psi->videoCodec;
        transcoderDst->videoProgram.pidChannel = transcoderDst->videoPidChannel;
        transcoderDst->videoProgram.stcChannel = transcoderDst->videoStcChannel;
        transcoderDst->videoProgram.nonRealTime = ipStreamerCtx->cfg.transcode.nonRealTime;

        rc = NEXUS_VideoDecoder_Start(transcoderDst->videoDecoder, &transcoderDst->videoProgram);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            goto error;
        }
        BDBG_MSG(("%s: Video Decoder is started for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));
    }
    if (psi->audioPid && ipStreamerCtx->cfg.transcode.outAudio) {
        NEXUS_AudioDecoderSettings audioDecoderSettings;

        if (ipStreamerCfg->srcType == IpStreamerSrc_eIp) {
            NEXUS_AudioDecoder_GetSettings(transcoderDst->audioDecoder, &audioDecoderSettings);
            audioDecoderSettings.ptsOffset = IP_NETWORK_JITTER * 45;    /* In 45Khz clock */
            audioDecoderSettings.firstPts.callback = audioFirstPtsCallback;
            audioDecoderSettings.firstPts.context = NULL;
            rc = NEXUS_AudioDecoder_SetSettings(transcoderDst->audioDecoder, &audioDecoderSettings);
            if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning..", rc, __LINE__)); goto error;}
        }
        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&transcoderDst->audioMuxStartSettings);
        transcoderDst->audioMuxStartSettings.stcChannel = transcoderDst->encodeStcChannel;
        transcoderDst->audioMuxStartSettings.presentationDelay = Dee/27000; /* in ms */
        transcoderDst->audioMuxStartSettings.nonRealTime = ipStreamerCtx->cfg.transcode.nonRealTime;

        rc = NEXUS_AudioMuxOutput_Start(transcoderDst->audioMuxOutput, &transcoderDst->audioMuxStartSettings);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            goto error;
        }
        BDBG_MSG(("%s: Audio Mux is started for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));

        NEXUS_AudioDecoder_GetDefaultStartSettings(&transcoderDst->audioProgram);
#if NEXUS_HAS_HDMI_INPUT
        if (ipStreamerCtx->cfg.srcType == IpStreamerSrc_eHdmi) {
            transcoderDst->audioProgram.input = NEXUS_HdmiInput_GetAudioConnector(ipStreamerCtx->hdmiSrc->hdmiInput);
            transcoderDst->audioProgram.stcChannel = transcoderDst->videoStcChannel;
         }
        else
#endif
        {
            transcoderDst->audioProgram.codec = psi->audioCodec;
            transcoderDst->audioProgram.pidChannel = transcoderDst->audioPidChannel;
            transcoderDst->audioProgram.stcChannel = ipStreamerCtx->cfg.transcode.nonRealTime?
                transcoderDst->encodeStcChannel : transcoderDst->videoStcChannel;
            transcoderDst->audioProgram.nonRealTime= ipStreamerCtx->cfg.transcode.nonRealTime;
        }
        rc = NEXUS_AudioMixer_Start(transcoderDst->audioMixer);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            goto error;
        }
        rc = NEXUS_AudioDecoder_Start(transcoderDst->audioDecoder, &transcoderDst->audioProgram);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            goto error;
        }
        BDBG_MSG(("%s: Audio Decoder is started for transcoder ctx %p", BSTD_FUNCTION, (void *)transcoderDst));
    }

    if (!ipStreamerCfg->hlsSession) {
        NEXUS_StreamMux_AddSystemDataBuffer(transcoderDst->streamMux, &transcoderDst->psi[0]);
        NEXUS_StreamMux_AddSystemDataBuffer(transcoderDst->streamMux, &transcoderDst->psi[1]);
    }

    rc = NEXUS_VideoEncoder_Start(transcoderDst->videoEncoder, &transcoderDst->videoEncoderStartConfig);
    if (rc) {
        BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
        goto error;
    }
    BDBG_MSG(("%s: Video Encoder is started for transcoder ctx %p, CTX %p", BSTD_FUNCTION, (void *)transcoderDst, (void *)ipStreamerCtx));

    ipStreamerCtx->transcodingInProgress = true;
    transcoderDst->started = true;
    BDBG_MSG(("%s: CTX %p: Transcoder Dst %p is started", BSTD_FUNCTION, (void *)ipStreamerCtx, (void *)transcoderDst));
    return 0;

error:
    /* stop the transcoding pipe first as its stop was delayed to the close call */
    stopNexusTranscoderDst(ipStreamerCtx);
    return -1;
}

#endif

int
setupNexusTranscoderPidChannels(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    int rc;
    NEXUS_PlaypumpOpenPidChannelSettings pidChannelSettings;

    /* note: we need to start the xcoder pipe as video pid channel is being created by mux mgr start call */
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    rc = startNexusTranscoderDst(psi, ipStreamerCfg, ipStreamerCtx);
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, returning...", rc, __LINE__)); return -1;}

    if (ipStreamerCfg->transcode.outPcrPid) {
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eOther;
        ipStreamerCtx->transcodePcrPidChannel = NEXUS_Playpump_OpenPidChannel(ipStreamerCtx->transcoderDst->playpumpTranscodeSystemData, ipStreamerCfg->transcode.outPcrPid, &pidChannelSettings);
    }
    else
        ipStreamerCtx->transcodePcrPidChannel = NULL;

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
    pidChannelSettings.pidType = NEXUS_PidType_eOther;
    ipStreamerCtx->transcodePatPidChannel = NEXUS_Playpump_OpenPidChannel(ipStreamerCtx->transcoderDst->playpumpTranscodeSystemData, ipStreamerCfg->transcode.outPatPid, &pidChannelSettings);

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
    pidChannelSettings.pidType = NEXUS_PidType_eOther;
    ipStreamerCtx->transcodePmtPidChannel = NEXUS_Playpump_OpenPidChannel(ipStreamerCtx->transcoderDst->playpumpTranscodeSystemData, ipStreamerCfg->transcode.outPmtPid, &pidChannelSettings);
    BDBG_MSG(("%s: allocated pat pid ch pat %p, ch %p, pcr %p", BSTD_FUNCTION, (void *)ipStreamerCtx->transcodePatPidChannel, (void *)ipStreamerCtx->transcodePmtPidChannel, (void *)ipStreamerCtx->transcodePcrPidChannel));
    return 0;
}

void
closeNexusTranscoderPidChannels(
    IpStreamerCtx *ipStreamerCtx
    )
{
    BDBG_MSG(("%s: Closing", BSTD_FUNCTION));
    if (ipStreamerCtx->transcodePcrPidChannel) {
        NEXUS_Playpump_ClosePidChannel(ipStreamerCtx->transcoderDst->playpumpTranscodeSystemData, ipStreamerCtx->transcodePcrPidChannel);
        ipStreamerCtx->transcodePcrPidChannel = NULL;
    }
    if (ipStreamerCtx->transcodePatPidChannel) {
        NEXUS_Playpump_ClosePidChannel(ipStreamerCtx->transcoderDst->playpumpTranscodeSystemData, ipStreamerCtx->transcodePatPidChannel);
        ipStreamerCtx->transcodePatPidChannel = NULL;
    }
    if (ipStreamerCtx->transcodePmtPidChannel) {
        NEXUS_Playpump_ClosePidChannel(ipStreamerCtx->transcoderDst->playpumpTranscodeSystemData, ipStreamerCtx->transcodePmtPidChannel);
        ipStreamerCtx->transcodePmtPidChannel = NULL;
    }

    if (ipStreamerCtx->srcType != IpStreamerSrc_eHdmi && ipStreamerCtx->srcType != IpStreamerSrc_eStreamer) {
        if (ipStreamerCtx->transcoderDst->videoPidChannel) {
            NEXUS_PidChannel_Close(ipStreamerCtx->transcoderDst->videoPidChannel);
            ipStreamerCtx->transcoderDst->videoPidChannel = NULL;
        }
        if (ipStreamerCtx->transcoderDst->audioPidChannel) {
            NEXUS_PidChannel_Close(ipStreamerCtx->transcoderDst->audioPidChannel);
            ipStreamerCtx->transcoderDst->audioPidChannel = NULL;
        }
        if (ipStreamerCtx->transcoderDst->pcrPidChannel) {
            NEXUS_PidChannel_Close(ipStreamerCtx->transcoderDst->pcrPidChannel);
            ipStreamerCtx->transcoderDst->pcrPidChannel = NULL;
        }
    }
    BDBG_MSG(("%s: Closed", BSTD_FUNCTION));
}

int
openNexusTranscoderPidChannels(
        B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    /* for transcode paths: live pid channels -> AV decoders && xcode pid channels -> recpump */
    if (ipStreamerCtx->transcoderDst->refCount == 1) {
        /* allocate pid channels for 1st instance of a xcode session (which can be shared among clients) */
        if (psi->videoPid)
            ipStreamerCtx->transcoderDst->videoPidChannel = NEXUS_PidChannel_Open(ipStreamerCtx->parserBandPtr->parserBand, psi->videoPid, NULL);
        else
            ipStreamerCtx->transcoderDst->videoPidChannel = NULL;
        if (psi->audioPid)
            ipStreamerCtx->transcoderDst->audioPidChannel = NEXUS_PidChannel_Open(ipStreamerCtx->parserBandPtr->parserBand, psi->audioPid, NULL);
        else
            ipStreamerCtx->transcoderDst->audioPidChannel = NULL;
        if (psi->pcrPid != psi->videoPid) {
            ipStreamerCtx->transcoderDst->pcrPidChannel = NEXUS_PidChannel_Open(ipStreamerCtx->parserBandPtr->parserBand, psi->pcrPid, NULL);
        }
        else
            ipStreamerCtx->transcoderDst->pcrPidChannel = NULL;
    }
    /* else: for xcode sessions w/ refCount > 1, simple xcode path is already setup, we dont need to create any pidChannels for live -> AV decoder path */

    if (setupNexusTranscoderPidChannels(psi, ipStreamerCfg, ipStreamerCtx) < 0) {
        BDBG_ERR(("%s: setupNexusTranscoderPidChannels() Failed", BSTD_FUNCTION));
        return -1;
    }
    return 0;
}
