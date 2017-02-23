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
 *
 *****************************************************************************/
/* Nexus example app: play and decode MP3 file to audio dacs and capture ancillary data */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#endif
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"

static bool g_exit=false;

static void eof(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    g_exit = true;
    fprintf(stderr, "EOF\n");
}

static void ancillary_data(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv) {
#if NEXUS_HAS_PLAYBACK
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings pidSettings;
    NEXUS_AudioDecoderOpenSettings decoderOpenSettings;
    NEXUS_AudioDecoderSettings decoderSettings;
    NEXUS_AudioDecoderHandle pcmDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    BKNI_EventHandle event;
    NEXUS_PlaybackStartSettings playbackStartSettings;
    const char *fname = "videos/sinetest1.mp3";
    NEXUS_Error rc;
    FILE *pAncFile;
    NEXUS_AudioCapabilities audioCapabilities;
    NEXUS_AudioOutputHandle audioDacHandle = NULL;

    if (argc > 1) {
        fname = argv[1];
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (audioCapabilities.numDecoders == 0)
    {
        printf("This application is not supported on this platform (requires a decoder).\n");
        return 0;
    }


    if (audioCapabilities.numOutputs.dac > 0)
    {
        audioDacHandle = NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]);
    }

    BKNI_CreateEvent(&event);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        goto error;
    }
    pAncFile = fopen("ancillary_data.dat", "wb+");
    if ( !pAncFile )
    {
        fprintf(stderr, "Can't open ancillary_data.dat for writing\n");
        goto error;
    }
    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eEs;
    playbackSettings.endOfStreamCallback.callback = eof;
    playbackSettings.endOfStreamCallback.context = event;
    playbackSettings.enableStreamProcessing = true; /* this should be set to true if application needs to seek in the MP3 file, it'd cause decoder to receive MPEG-2 PES data */
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&decoderOpenSettings);
    decoderOpenSettings.ancillaryDataFifoSize = 32*1024;
    pcmDecoder = NEXUS_AudioDecoder_Open(0, &decoderOpenSettings);

    NEXUS_AudioDecoder_GetSettings(pcmDecoder, &decoderSettings);
    decoderSettings.ancillaryDataEnabled = true;
    decoderSettings.ancillaryData.callback = ancillary_data;
    decoderSettings.ancillaryData.context = event;
    (void)NEXUS_AudioDecoder_SetSettings(pcmDecoder, &decoderSettings);

    if (audioDacHandle) {
    NEXUS_AudioOutput_AddInput(audioDacHandle,
                               NEXUS_AudioDecoder_GetConnector(pcmDecoder,
                                                               NEXUS_AudioDecoderConnectorType_eStereo));
    }

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_eMp3;

    NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    pidSettings.pidSettings.pidTypeSettings.audio.codec = audioProgram.codec;
    pidSettings.pidTypeSettings.audio.primary = pcmDecoder;
    audioProgram.pidChannel = NEXUS_Playback_OpenPidChannel(playback, 1, &pidSettings);

    NEXUS_Playback_GetDefaultStartSettings(&playbackStartSettings);
#if 0
    playbackStartSettings.mode = NEXUS_PlaybackMode_eFixedBitrate;
    playbackStartSettings.bitrate = 128000;
#else
    playbackStartSettings.mode = NEXUS_PlaybackMode_eAutoBitrate;
#endif

    NEXUS_Playback_Start(playback, file, &playbackStartSettings);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    while ( !g_exit ) {
        void *pAncBuf;
        size_t size;

        rc = NEXUS_AudioDecoder_GetAncillaryDataBuffer(pcmDecoder, &pAncBuf, &size);
        if ( BERR_SUCCESS == rc && size > 0 )
        {
            const NEXUS_AudioAncillaryDataHeader *pHeader = pAncBuf;
            if ( pHeader->payloadSize > 0 )
            {
                (void)fwrite(pHeader+1, pHeader->payloadSize, 1, pAncFile);
            }
            NEXUS_AudioDecoder_AncillaryDataReadComplete(pcmDecoder, pHeader->blockSize);
        }

        (void)BKNI_WaitForEvent(event, 50);
    }

    NEXUS_AudioDecoder_Stop(pcmDecoder);
    NEXUS_Playback_Stop(playback);
    if (audioDacHandle) {
        NEXUS_AudioOutput_RemoveAllInputs(audioDacHandle);
    }
    NEXUS_Playback_ClosePidChannel(playback, audioProgram.pidChannel);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close(pcmDecoder);
    fclose(pAncFile);
    NEXUS_FilePlay_Close(file);
    BKNI_DestroyEvent(event);
    NEXUS_Platform_Uninit();

    return 0;

error:
#endif
    return 1;
}
#else /* NEXUS_HAS_AUDIO */
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
