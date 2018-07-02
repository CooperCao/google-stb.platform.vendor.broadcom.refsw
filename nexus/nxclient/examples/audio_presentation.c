/***************************************************************************
 * Copyright (C) 2017-2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
 **************************************************************************/
#include "nxclient.h"
#include <stdio.h>
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_simple_audio_decoder.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(audio_presentation);


/* For this example, use 1 files: primary and description. */
#define FILE_NAME "videos/rugby_final_6pres.trp"
#define TRANSPORT NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eH265
#define AUDIO_CODEC NEXUS_AudioCodec_eAc4
#define VIDEO_PID 0x30 /* Set to 0 to disable video */
#define AUDIO_PID 0x40
int main(void)
{
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_FilePlayHandle file;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_AudioDecoderCodecSettings codecSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_VideoCodec videoCodec;
    NEXUS_AudioCodec audioCodec;
    unsigned videoPid;
    unsigned audioPid;
    const char * filename;
    NEXUS_TransportType transportType;
    NEXUS_Error rc;
    bool run = true;

    filename = FILE_NAME;
    videoCodec = VIDEO_CODEC;
    videoPid = VIDEO_PID;
    audioCodec = AUDIO_CODEC;
    audioPid = AUDIO_PID;
    transportType = TRANSPORT;

    rc = NxClient_Join(NULL);
    if (rc) return -1;


    NxClient_GetDefaultAllocSettings(&allocSettings);
    if (videoPid != 0) {
        allocSettings.simpleVideoDecoder = 1;
        allocSettings.surfaceClient = 1;
    }
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleAudioDecoder.decoderCapabilities.type = NxClient_AudioDecoderType_ePersistent;
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = allocResults.surfaceClient[0].id;
    connectSettings.simpleVideoDecoder[0].windowId = 0;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[videoCodec] = true;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.type = NxClient_VideoWindowType_eMain;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    BDBG_ASSERT(stcChannel);
    file = NEXUS_FilePlay_OpenPosix(filename, NULL);
    if ( !file ) {
        BDBG_ERR(("Unable to open file %s", filename));
        return -1;
    }
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpumpSettings.transportType = transportType;
    playbackSettings.playpump = playpump;
    playbackSettings.simpleStcChannel = stcChannel;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    if (videoDecoder) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = videoCodec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
        videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, videoPid, &playbackPidSettings);
        if (!videoProgram.settings.pidChannel) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto cleanup;}
        rc = NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
        if (rc) {BERR_TRACE(rc); goto cleanup;}
        videoProgram.settings.codec = videoCodec;
    }

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder;
    audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(playback, audioPid, &playbackPidSettings);
    audioProgram.primary.codec = audioCodec;

    if (audioProgram.primary.pidChannel) {
        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);
    }
    audioProgram.master = true;
    audioProgram.primary.mixingMode = NEXUS_AudioDecoderMixingMode_eStandalone;

    if (audioCodec == NEXUS_AudioCodec_eAc4) {
        NEXUS_SimpleAudioDecoder_GetCodecSettings(audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc4, &codecSettings);
        codecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationIndex = 0;
        codecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].presentationIndex = 3;
        NEXUS_SimpleAudioDecoder_SetCodecSettings(audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
    }

    if (videoDecoder) {
        rc = NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
    }
    rc = NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Playback_Start(playback, file, NULL);
    BDBG_ASSERT(!rc);
    do
    {
        int tmp;
        NEXUS_AudioDecoderStatus status;

        printf("Main Menu\n");
        printf(" 0) Exit\n");
        printf(" 1) Get Presentation List\n");
        printf(" 2) Set Presentation Index\n");

        printf("Enter Selection: \n");
        scanf("%d", &tmp);
        switch ( tmp ) {
        case 0:
            run = false;
            break;
        case 1:
            NEXUS_SimpleAudioDecoder_GetStatus(audioDecoder, &status);
            if (status.codec == NEXUS_AudioCodec_eAc4) {
                NEXUS_AudioDecoderPresentationStatus presentationStatus;
                unsigned i;
                printf("    Main Presentation Index %u\n", status.codecStatus.ac4.currentPresentationIndex);
                printf("    Alternate Presentation Index %u\n", status.codecStatus.ac4.currentAlternateStereoPresentationIndex);
                for (i = 0; i < status.codecStatus.ac4.numPresentations; i++) {
                    NEXUS_SimpleAudioDecoder_GetPresentationStatus(audioDecoder, i, &presentationStatus);
                    printf("      Presentation: %d (%s)\n", presentationStatus.status.ac4.index, presentationStatus.status.ac4.language);
                }
            }
            break;
        case 2:
            NEXUS_SimpleAudioDecoder_GetStatus(audioDecoder, &status);
            if (status.codec == NEXUS_AudioCodec_eAc4) {
                int presentation;
                printf(" 0) Main Presentation\n");
                printf(" 1) Alternate Stereo Presentation\n");
                printf(" Which Presentation?\n");
                scanf("%d", &presentation);
                if (presentation == 0 || presentation == 1) {
                    printf(" Current Presentation Index %d\n", presentation == 0 ? status.codecStatus.ac4.currentPresentationIndex : status.codecStatus.ac4.currentAlternateStereoPresentationIndex);
                    printf(" Enter new index [0-%d]\n", status.codecStatus.ac4.numPresentations - 1);
                    scanf("%d", &tmp);
                    if (tmp >= 0 && tmp < (int)status.codecStatus.ac4.numPresentations) {
                        NEXUS_SimpleAudioDecoder_GetCodecSettings(audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc4, &codecSettings);
                        codecSettings.codecSettings.ac4.programs[presentation].presentationIndex = (unsigned)tmp;
                        NEXUS_SimpleAudioDecoder_SetCodecSettings(audioDecoder, NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
                    }
                }
            }
            break;
        default:
            break;
        }
    } while ( run );

    if (videoDecoder) {
        NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
    }
    NEXUS_SimpleAudioDecoder_Stop(audioDecoder);
    NEXUS_Playback_Stop(playback);

cleanup:
    if (playback) {
        NEXUS_Playback_Destroy(playback);
    }
    if (playpump) {
        NEXUS_Playpump_Close(playpump);
    }
    if (file) {
        NEXUS_FilePlay_Close(file);
    }
    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return 0;
}
#else /* #if NEXUS_HAS_PLAYBACK */
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
