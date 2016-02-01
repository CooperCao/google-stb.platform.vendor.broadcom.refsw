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
#if NEXUS_HAS_SIMPLE_DECODER
#include "nexus_platform_client.h"
#include "nexus_simple_audio_playback.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nxclient.h"
#include "wav_file.h"

BDBG_MODULE(playpcm);

#define MAX_VOLUME 10

static void print_usage(void)
{
    printf(
        "Usage: playpcm [PCMFILE]\n"
        "  -loop\n"
        "  -passive   Don't force HDMI and SPDIF to pcm mode. If compressed, you won't hear anything.\n"
        "  -s         Play from stdin (default if no cmdline params)\n"
        "  -q         Don't print PCM format\n"
        );
    printf(
        "  -vol {0..%d}\n",
        MAX_VOLUME
        );
}

#define CONVERT_TO_USER_VOL(vol,max_vol) (((vol) - NEXUS_AUDIO_VOLUME_LINEAR_MIN) * (max_vol) / (NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN))
#define CONVERT_TO_NEXUS_LINEAR_VOL(vol,max_vol) (((vol) * (NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN) / (max_vol)) + NEXUS_AUDIO_VOLUME_LINEAR_MIN)

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BDBG_MSG(("pcm callback"));
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_SimpleAudioPlaybackHandle audioPlayback;
    NEXUS_SimpleAudioPlaybackStartSettings startSettings;
    FILE *file;
    BKNI_EventHandle event;
    int rc;
    int curarg = 1;
    bool loop = false;
    const char *filename = NULL;
    bool forcePcm = true;
    bool useStdin = false;
    unsigned sampleRate = 44100;
    struct wave_header wave_header;
    bool quiet = false;
    int vol = -1;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-loop")) {
            loop = true;
        }
        else if (!strcmp(argv[curarg], "-passive")) {
            forcePcm = false;
        }
        else if (!strcmp(argv[curarg], "-s")) {
            useStdin = true;
        }
        else if (!strcmp(argv[curarg], "-sample_rate") && curarg+1<argc) {
            sampleRate = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-q")) {
            quiet = true;
        }
        else if (!strcmp(argv[curarg], "-vol") && curarg+1<argc) {
            vol = atoi(argv[++curarg]);
            if (vol > MAX_VOLUME) vol = MAX_VOLUME;
        }
        else if (!filename) {
            filename = argv[curarg];
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
    
    if (forcePcm) {
        NxClient_AudioSettings audioSettings;
        NxClient_GetAudioSettings(&audioSettings);
        audioSettings.hdmi.outputMode = NxClient_AudioOutputMode_ePcm;
        audioSettings.spdif.outputMode = NxClient_AudioOutputMode_ePcm;
        NxClient_SetAudioSettings(&audioSettings);
    }
    
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleAudioPlayback = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {rc = BERR_TRACE(rc); goto err_request;}
    
    audioPlayback = NEXUS_SimpleAudioPlayback_Acquire(allocResults.simpleAudioPlayback[0].id);
    if (!audioPlayback) {
        BDBG_ERR(("unable to acquire audio playback"));
        goto err_acquire;
    }

    if (vol != -1) {
        NEXUS_SimpleAudioPlaybackSettings settings;
        NEXUS_SimpleAudioPlayback_GetSettings(audioPlayback, &settings);
        settings.rightVolume = settings.leftVolume = CONVERT_TO_NEXUS_LINEAR_VOL(vol, MAX_VOLUME);
        NEXUS_SimpleAudioPlayback_SetSettings(audioPlayback, &settings);
    }
    
    /* connect client resources to server's resources */
    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleAudioPlayback[0].id = allocResults.simpleAudioPlayback[0].id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) {rc = BERR_TRACE(rc); goto err_connect;}

    BKNI_CreateEvent(&event);
    if (!useStdin && filename) {
        file = fopen(filename, "r");
        if (!file) {
            BDBG_ERR(("unable to open %s", filename));
            goto err_file;
        }
    }
    else {
        file = stdin;
    }

    NEXUS_SimpleAudioPlayback_GetDefaultStartSettings(&startSettings);

    if (!read_wave_header(file, &wave_header)) {
        startSettings.sampleRate = wave_header.samplesSec;
        startSettings.bitsPerSample = wave_header.bps;
        startSettings.stereo = wave_header.channels == 2;
    }
    else {
        startSettings.sampleRate = sampleRate;
        startSettings.bitsPerSample = 16;
        startSettings.stereo = true;
    }
    if (!quiet) {
        printf("PCM audio, %d samples/sec, %d bits/sample, %d channel(s)\n",
            startSettings.sampleRate, startSettings.bitsPerSample, startSettings.stereo?2:1);
    }

    startSettings.dataCallback.callback = complete;
    startSettings.dataCallback.context = event;
    startSettings.loopAround = false;
    rc = NEXUS_SimpleAudioPlayback_Start(audioPlayback, &startSettings);
    if (rc) {rc = BERR_TRACE(rc);goto err_start;}

    while (1) {
        void *buffer;
        size_t size;
        int n;

        rc = NEXUS_SimpleAudioPlayback_GetBuffer(audioPlayback, &buffer, &size);
        BDBG_ASSERT(!rc);
        if (size == 0) {
            BKNI_WaitForEvent(event, 1000);
            continue;
        }
        n = fread(buffer, 1, size, file);
        if (n < 0) {
            break;
        }
        else if (n == 0) {
            if (loop) {
                fseek(file, 0, SEEK_SET);
                continue;
            }
            else {
                /* eof */
                break;
            }
        }

        rc = NEXUS_SimpleAudioPlayback_WriteComplete(audioPlayback, n);
        BDBG_ASSERT(!rc);
        BDBG_MSG(("play %d bytes", n));
    }
    
    /* wait until file played, then stop */
    while (1) {
        NEXUS_SimpleAudioPlaybackStatus status;
        rc = NEXUS_SimpleAudioPlayback_GetStatus(audioPlayback, &status);
        BDBG_ASSERT(!rc);
        if (!status.queuedBytes) break;
    }

    NEXUS_SimpleAudioPlayback_Stop(audioPlayback);
    
err_start:
    if (filename) {
        fclose(file);
    }
err_file:
    BKNI_DestroyEvent(event);    
    if (connectId) {
        NxClient_Disconnect(connectId);
    }
err_connect:
    NEXUS_SimpleAudioPlayback_Release(audioPlayback);
err_acquire:
    NxClient_Free(&allocResults);
err_request:
    NxClient_Uninit();

    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs simple_decoder)!\n");
    return 0;
}
#endif
