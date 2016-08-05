/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* Nexus example app: playback and decode */

#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif
#if NEXUS_HAS_SAGE
#include "nexus_security_client.h"
#endif

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(crr_playback);

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0x21
#define AUDIO_PID 0x22

#ifdef NEXUS_VIDEO_SECURE_HEAP
int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    const char *fname = FILE_NAME;
    NEXUS_Error rc;
    unsigned i,j;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
    platformSettings.openFrontend = false;

	for (i = 0; i < NEXUS_NUM_VIDEO_DECODERS; i++)
	{
		memConfigSettings.videoDecoder[i].secure = NEXUS_SecureVideo_eBoth;
	}
	for (i = 0; i < NEXUS_NUM_DISPLAYS; i++)
	{
		for (j = 0; j < NEXUS_NUM_VIDEO_WINDOWS;j++)
		{
			memConfigSettings.display[i].window[j].secure = NEXUS_SecureVideo_eBoth;
		}
	}

    rc = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Bring up video display and outputs */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
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

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    BDBG_ASSERT(stcChannel);

    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

#define TOTAL_TESTS 4
/*
test 0: GLR->GLR: should pass
test 1: CRR->GLR: negative test, should fail
test 2: GLR->URR: should pass
*/

/*
You must compile with SAGE to have the CRR actually enforced.
Without SAGE, the CRR is just a block of unmapped memory.
*/

    for (i=0;i<TOTAL_TESTS;i++) {
        NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
        NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;

        NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
        NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);

        switch (i) {
        case 0:
            BDBG_WRN(("Test 0: CDB in GLR, decode to GLR"));
            break;
        case 1:
            BDBG_WRN(("Test 1: CDB in CRR, decode to GLR (should fail)"));
            videoDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
            audioDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
            break;
		case 2:
			videoDecoderOpenSettings.secureVideo = true;
			BDBG_WRN(("Test 2: CDB in GLR, decode to URR"));
			break;
		case 3:
			videoDecoderOpenSettings.secureVideo = true;
            videoDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
            audioDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
			BDBG_WRN(("Test 3: CDB in CRR, decode to URR"));
			break;
        }
        videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
        rc = NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
        BDBG_ASSERT(!rc);

        audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
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

        NEXUS_Playback_GetSettings(playback, &playbackSettings);
        playbackSettings.playpump = playpump;
        /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
        playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
        playbackSettings.stcChannel = stcChannel;
        rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
        BDBG_ASSERT(!rc);

        /* Open the audio and video pid channels */
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
        videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

#if NEXUS_HAS_SAGE
        /* NOTE: this is needed for CRR playback with SAGE restricting the CRR */
        switch (i) {
        case 0: NEXUS_SetPidChannelBypassKeyslot(videoPidChannel, NEXUS_BypassKeySlot_eG2GR); break;
        case 1: NEXUS_SetPidChannelBypassKeyslot(videoPidChannel, NEXUS_BypassKeySlot_eGR2R); break;
        case 2: NEXUS_SetPidChannelBypassKeyslot(videoPidChannel, NEXUS_BypassKeySlot_eG2GR); break;
        case 3: NEXUS_SetPidChannelBypassKeyslot(videoPidChannel, NEXUS_BypassKeySlot_eGR2R); break;
        }
#endif

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
        audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, &playbackPidSettings);

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = VIDEO_CODEC;
        videoProgram.pidChannel = videoPidChannel;
        videoProgram.stcChannel = stcChannel;
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = AUDIO_CODEC;
        audioProgram.pidChannel = audioPidChannel;
        audioProgram.stcChannel = stcChannel;

        rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
        BDBG_ASSERT(!rc);
        rc = NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Playback_Start(playback, file, NULL);
        BDBG_ASSERT(!rc);

        BKNI_Sleep(5000);

        /* verify video has worked */
        if((i == 0) || (i == 2) || (i == 3))
        {
            /*GLR->GLR, verify 100 frames were decoded */
            NEXUS_VideoDecoderStatus status;
            NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
            BDBG_WRN(("%d pictures displayed", status.numDisplayed));
            BDBG_ASSERT(status.numDisplayed > 100);
        }
        else if (i == 1)
        {
            /*CRR->GLR, verify 0 frames were decoded */
            NEXUS_VideoDecoderStatus status;
            NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
            BDBG_WRN(("%d pictures displayed", status.numDisplayed));
            BDBG_ASSERT(status.numDisplayed == 0);
        }

        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_AudioDecoder_Stop(audioDecoder);
        NEXUS_Playback_Stop(playback);
        NEXUS_Playback_ClosePidChannel(playback, videoPidChannel);
        NEXUS_Playback_ClosePidChannel(playback, audioPidChannel);

        NEXUS_VideoDecoder_Close(videoDecoder);
        NEXUS_AudioDecoder_Close(audioDecoder);
    }

    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);

    NEXUS_Platform_Uninit();

    return 0;
}
#else
int main(void)
{
    printf("No CRR on this platform\n");
    return 0;
}
#endif
