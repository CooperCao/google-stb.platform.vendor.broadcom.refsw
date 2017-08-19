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
 *****************************************************************************/
#if NEXUS_HAS_SIMPLE_DECODER && NEXUS_HAS_AUDIO
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_i2s_input.h"
#include "nexus_playback.h"
#include "media_probe.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#define FILE_NAME "videos/sexteto_fantasma.ts"

BDBG_MODULE(audio_ambisonic);

void print_usage(void)
{
    printf(
    "Usage: nexus.client audio_ambisonic OPTIONS\n"
    "\n"
    "  stream_url can be http://server/path, or [file://]path/file\n"
    "\n"
    "OPTIONS:\n"
    "  --help or -h for help\n"
    "  -file path/file\n  File to be used"
    "  -stereo"
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
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleAudioDecoderSettings audioSettings;
    NEXUS_Error rc;
    const char *filename = NULL;
    bool stereo = false;
    struct probe_results probe_results;
    int curarg = 1;

     while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-file") && argc>curarg+1) {
            filename = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-stereo")) {
            stereo = true;
        }
        curarg++;
     }

    if (!filename) {
        filename = FILE_NAME;
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

    rc = probe_media(filename, &probe_results);
    if (rc) {
        fprintf(stderr, "unable to probe %s\n", filename);
        goto error;
    }
    if (!probe_results.num_audio) {
        fprintf(stderr, "no audio found in %s\n", filename);
        goto error;
    }

    file = NEXUS_FilePlay_OpenPosix(filename, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", filename);
        goto error;
    }

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);


    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.simpleStcChannel = stcChannel;

    playbackSettings.playpumpSettings.transportType = probe_results.transportType;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

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
    audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(playback, probe_results.audio[0].pid, &playbackPidSettings);
    audioProgram.primary.codec = probe_results.audio[0].codec;

    NEXUS_SimpleAudioDecoder_GetSettings(audioDecoder, &audioSettings);
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.connected = true;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.contentType = NEXUS_AmbisonicContentType_eAmbisonic;
    if ( stereo ) {
        audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.connectorType = NEXUS_AudioConnectorType_eStereo;
    } else {
        audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.connectorType = NEXUS_AudioConnectorType_eMultichannel;
    }
    NEXUS_SimpleAudioDecoder_SetSettings(audioDecoder, &audioSettings);

    rc = NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Playback_Start(playback, file, NULL);
    BDBG_ASSERT(!rc);

    BDBG_WRN(("Press ENTER to exit"));
    getchar();

    if (playback) {
        NEXUS_Playback_Stop(playback);
        NEXUS_Playback_Destroy(playback);
        NEXUS_Playpump_Close(playpump);
        NEXUS_FilePlay_Close(file);
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
