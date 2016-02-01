/******************************************************************************
 *    (c)2010-2013 Broadcom Corporation
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
#include "nexus_platform_client.h"
#include <stdio.h>

#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_core_utils.h"
#include "../../utils/namevalue.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "media_probe.h"

BDBG_MODULE(decode_client);

static void print_usage(void)
{
    printf(
        "Usage: nexus.client decode_client OPTIONS filename [indexfile]\n"
        "Options:\n"
        "  -video TRACK_NUM\n"
        "  -audio TRACK_NUM\n"
        );
}

BKNI_EventHandle g_endOfStreamEvent;

static void endOfStreamCallback(void *context, int param)
{
    NEXUS_PlaybackLoopMode endOfStreamAction = param;
    BSTD_UNUSED(context);

    BDBG_WRN(("end of stream"));
    if (endOfStreamAction != NEXUS_PlaybackLoopMode_eLoop) {
        BKNI_SetEvent(g_endOfStreamEvent);
    }
}

int main(int argc, char **argv)  {
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderHandle audioDecoder = NULL;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_PidChannelHandle pcrPidChannel = NULL;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_Error rc = 0;
    unsigned timeout = 0;
    int curarg = 1;
    const char *filename = NULL;
    const char *indexname = NULL;
    unsigned videoTrack = 0;
    unsigned audioTrack = 0;
    struct probe_results probe_results;
    NEXUS_PlaybackLoopMode beginningOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    NEXUS_PlaybackLoopMode endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-bof") && curarg+1<argc) {
            beginningOfStreamAction=lookup(g_endOfStreamActionStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-eof") && curarg+1<argc) {
            endOfStreamAction=lookup(g_endOfStreamActionStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-video") && curarg+1<argc) {
            videoTrack = strtoul(argv[++curarg], NULL, 10);
        }
        else if (!strcmp(argv[curarg], "-audio") && curarg+1<argc) {
            audioTrack = strtoul(argv[++curarg], NULL, 10);
        }
        else if (!filename) {
            filename = argv[curarg];
        }
        else if (!indexname) {
            indexname = argv[curarg];
        }
        curarg++;
    }
    if (!filename) {
        print_usage();
        return -1;
    }
    
    rc = NEXUS_Platform_AuthenticatedJoin(NULL);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }
    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    BKNI_CreateEvent(&g_endOfStreamEvent);

    rc = probe_media(filename, &probe_results);
    if (rc) return BERR_TRACE(rc);

    file = NEXUS_FilePlay_OpenPosix(filename, indexname);
    if (!file) {
        BDBG_ERR(("can't open files: %s %s", filename, indexname));
        rc = -1;
        goto done;
    }

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    BDBG_ASSERT(stcChannel);
    NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.modeSettings.Auto.transportType = probe_results.transportType;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    rc = NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = probe_results.transportType;
    playbackSettings.simpleStcChannel = stcChannel;
    playbackSettings.stcTrick = true;
    playbackSettings.beginningOfStreamAction = beginningOfStreamAction;
    playbackSettings.endOfStreamAction = endOfStreamAction;
    playbackSettings.endOfStreamCallback.callback = endOfStreamCallback;
    playbackSettings.endOfStreamCallback.param = endOfStreamAction;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up audio decoders and outputs */
    if (clientConfig.resources.simpleVideoDecoder.total) {
        videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(clientConfig.resources.simpleVideoDecoder.id[0]);
    }
    else {
        videoDecoder = NULL;
    }
    if (!videoDecoder) {
        BDBG_ERR(("video decoder not available"));
        rc = -1;
        goto done;
    }
    if (clientConfig.resources.simpleAudioDecoder.total) {
        audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(clientConfig.resources.simpleAudioDecoder.id[0]);
        if (!audioDecoder) {
            BDBG_WRN(("audio decoder not available"));
            /* for this example, audio is optional */
        }
    }

    /* open pid channels and configure start settings based on probe */
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);

    if (probe_results.video[0].pid) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = probe_results.video[0].codec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
        videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, probe_results.video[0].pid, &playbackPidSettings);
        videoProgram.settings.codec = probe_results.video[0].codec;
        /* The following is needed for 4K or 10 bit streams:
        videoProgram.maxWidth = 3840;
        videoProgram.maxHeight = 2160;
        NEXUS_SimpleVideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
        videoDecoderSettings.colorDepth = 10;
        NEXUS_SimpleVideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
        */
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    }

    if (probe_results.audio[0].pid) {
        NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder;
        audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(playback, probe_results.audio[0].pid, &playbackPidSettings);
        audioProgram.primary.codec = probe_results.audio[0].codec;
        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);
    }

    /* Start decoders */
    if (videoProgram.settings.pidChannel) {
        rc = NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
        BDBG_ASSERT(!rc);
    }
    if (audioDecoder && audioProgram.primary.pidChannel) {
        NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);
        /* decode may fail if audio codec not supported */
    }
    rc = NEXUS_Playback_Start(playback, file, NULL);
    BDBG_ASSERT(!rc);

    if (endOfStreamAction == NEXUS_PlaybackLoopMode_eLoop && !timeout) {
        printf("Press ENTER to stop\n");
        getchar();
    }
    else {
        if (timeout == 0) {
            timeout = BKNI_INFINITE;
        }
        else {
            timeout = timeout*1000;
        }
        /* auto close after timeout or at end of stream */
        BKNI_WaitForEvent(g_endOfStreamEvent, timeout);
    }

    /* Bring down system */
    NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_Stop(audioDecoder);
    }
    NEXUS_Playback_Stop(playback);

    if (videoProgram.settings.pidChannel) {
        NEXUS_Playback_ClosePidChannel(playback, videoProgram.settings.pidChannel);
    }
    if (audioProgram.primary.pidChannel) {
        NEXUS_Playback_ClosePidChannel(playback, audioProgram.primary.pidChannel);
    }
    if (pcrPidChannel) {
        NEXUS_Playback_ClosePidChannel(playback, pcrPidChannel);
    }

    NEXUS_SimpleVideoDecoder_Release(videoDecoder);
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_Release(audioDecoder);
    }

    if (playback) NEXUS_Playback_Destroy(playback);
    if (playpump) NEXUS_Playpump_Close(playpump);
    
done:
    if (file) NEXUS_FilePlay_Close(file);
    NEXUS_Platform_Uninit();
    return rc;
}
#else /* NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("This application is not supported on this platform (needs video decoder)!\n");
    return 0;
}
#endif
