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
#if NEXUS_HAS_SIMPLE_DECODER
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_processing_types.h"
#include "nexus_playback.h"
#include "nexus_stc_channel.h"
#include "nexus_video_input.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "media_probe.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

/* the following define the input file and its characteristics -- these will vary by input file */
#define MUSIC_FILE_NAME "videos/Fox&Cats_WriteItOff_44.1k.mp3"
#define VOCAL_FILE_NAME "videos/44.1k_2000Hz_beep_1s_30s.wav"


BDBG_MODULE(audio_karaoke);

void print_usage(void)
{
    printf(
    "Usage: nexus.client audio_karaoke OPTIONS\n"
    "\n"
    "  stream_url can be http://server/path, or [file://]path/file\n"
    "\n"
    "OPTIONS:\n"
    "  --help or -h for help\n"
    "  -music path/file\n  File to be used for music"
    "  -vocal path/file\n  File to be used as simulated vocals"
    "  -i2s uses input for vocals\n"
    );
}








int main(int argc, char **argv)
{
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    NEXUS_PlatformConfiguration platformConfig;
    unsigned connectId;
    NEXUS_FilePlayHandle file[2];
    NEXUS_PlaypumpHandle playpump[2];
    NEXUS_PlaybackHandle playback[2];
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleAudioDecoderSettings audioSettings;
    NEXUS_I2sInputHandle i2s;
    NEXUS_I2sInputSettings i2sSettings;
    NEXUS_Error rc;
    const char *musicFname = NULL;
    const char *vocalFname = NULL;
    unsigned i;
    struct probe_results music_probe_results;
    struct probe_results vocal_probe_results;
    int curarg = 1;
    bool useI2SInput = false;

     while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-music") && argc>curarg+1) {
            musicFname = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-vocal") && argc>curarg+1) {
            vocalFname = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-i2s")) {
            useI2SInput = true;
        }
        curarg++;
     }

    if (!musicFname) {
        musicFname = MUSIC_FILE_NAME;
    }
    if (!vocalFname) {
        vocalFname = VOCAL_FILE_NAME;
    }


    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }
    NEXUS_Platform_GetConfiguration(&platformConfig);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

    rc = probe_media(musicFname, &music_probe_results);
    if (rc) {
        fprintf(stderr, "unable to probe %s\n", musicFname);
        goto error;
    }
    if (!music_probe_results.num_audio) {
        fprintf(stderr, "no audio found in %s\n", musicFname);
        goto error;
    }

    file[0] = NEXUS_FilePlay_OpenPosix(musicFname, NULL);
    if (!file[0]) {
        fprintf(stderr, "can't open file:%s\n", musicFname);
        goto error;
    }

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpump[0] = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    playback[0] = NEXUS_Playback_Create();
    BDBG_ASSERT(playback[0]);


    NEXUS_Playback_GetSettings(playback[0], &playbackSettings);
    playbackSettings.playpump = playpump[0];
    playbackSettings.simpleStcChannel = stcChannel;

    playbackSettings.playpumpSettings.transportType = music_probe_results.transportType;
    rc = NEXUS_Playback_SetSettings(playback[0], &playbackSettings);
    BDBG_ASSERT(!rc);

    if (!useI2SInput)
    {
        rc = probe_media(vocalFname, &vocal_probe_results);
        if (rc) {
            fprintf(stderr, "unable to probe %s\n", vocalFname);
            goto error;
        }
        if (!vocal_probe_results.num_audio) {
            fprintf(stderr, "no audio found in %s\n", vocalFname);
            goto error;
        }
        file[1] = NEXUS_FilePlay_OpenPosix(vocalFname, NULL);
        if (!file[1]) {
            fprintf(stderr, "can't open file:%s\n", vocalFname);
            goto error;
        }

        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
        playpump[1] = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
        playback[1] = NEXUS_Playback_Create();
        BDBG_ASSERT(playback[1]);

        NEXUS_Playback_GetSettings(playback[1], &playbackSettings);
        playbackSettings.playpump = playpump[1];
        playbackSettings.simpleStcChannel = stcChannel;

        playbackSettings.playpumpSettings.transportType = vocal_probe_results.transportType;
        rc = NEXUS_Playback_SetSettings(playback[1], &playbackSettings);
        BDBG_ASSERT(!rc);
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {BDBG_WRN(("unable to alloc decode resources")); return -1;}

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) {BDBG_WRN(("unable to connect decode resources")); return -1;}

    audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder;
    audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(playback[0], music_probe_results.audio[0].pid, &playbackPidSettings);
    audioProgram.primary.codec = music_probe_results.audio[0].codec;
    audioProgram.primary.karaokeModeEnabled = true;

    if (useI2SInput)
    {
        NEXUS_I2sInput_GetDefaultSettings(&i2sSettings);
        /* customize i2s settings here to match ADC */
        i2sSettings.sampleRate = 44100;
        i2s = NEXUS_I2sInput_Open(0, &i2sSettings);

        audioProgram.description.codec = NEXUS_AudioCodec_ePcm;
        audioProgram.description.input = NEXUS_I2sInput_GetConnector(i2s);
    }
    else
    {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder;
        audioProgram.description.pidChannel = NEXUS_Playback_OpenPidChannel(playback[1], vocal_probe_results.audio[0].pid, &playbackPidSettings);
        audioProgram.description.codec = vocal_probe_results.audio[0].codec;
    }

    NEXUS_SimpleAudioDecoder_GetSettings(audioDecoder, &audioSettings);
    audioSettings.primary.karaokeSettings.vocalSuppressionFrequency = 4688;
    audioSettings.primary.karaokeSettings.vocalSuppressionLevel = 75;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_eDescription].karaokeVocal.connected = true;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_eDescription].karaokeVocal.settings.echo.enabled = true;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_eDescription].karaokeVocal.settings.echo.attenuation = 20;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_eDescription].karaokeVocal.settings.echo.delay = 200;
    NEXUS_SimpleAudioDecoder_SetSettings(audioDecoder, &audioSettings);

    rc = NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Playback_Start(playback[0], file[0], NULL);
    BDBG_ASSERT(!rc);
    if (!useI2SInput )
    {
        rc = NEXUS_Playback_Start(playback[1], file[1], NULL);
        BDBG_ASSERT(!rc);
    }

    BDBG_WRN(("Press ENTER to exit"));
    getchar();

    for (i=0;i<2;i++) {
        if (playback[i]) {
            NEXUS_Playback_Stop(playback[i]);
            NEXUS_Playback_Destroy(playback[i]);
            NEXUS_Playpump_Close(playpump[i]);
            NEXUS_FilePlay_Close(file[i]);
        }
    }
    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return 0;

error:
    return 1;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
