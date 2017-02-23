/***************************************************************************
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
 *
 * Module Description:
 *
 **************************************************************************/
#include "nxclient.h"
#include "nexus_playback.h"
#include "nexus_simple_audio_decoder.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(audio_ms12_mixing);

#define NUM_DECODES     4

/* For this example, use 1 files: primary and description. */
#define FILE_NAME0 "videos/Dual_m51_a1_ddp_29.97fps.trp"
#define FILE_NAME1 "videos/Dual_m51_a1_ddp_29.97fps.trp"
#define FILE_NAME2 "videos/48k_5.1_659hz_200hz_1000hz_-12dB.wav"
#define FILE_NAME3 "videos/48k_2000Hz_beep_5.1ch.wav"
#define TRANSPORT0 NEXUS_TransportType_eTs
#define TRANSPORT1 NEXUS_TransportType_eTs
#define TRANSPORT2 NEXUS_TransportType_eWav
#define TRANSPORT3 NEXUS_TransportType_eWav
#define AUDIO_CODEC0 NEXUS_AudioCodec_eAc3Plus
#define AUDIO_CODEC1 NEXUS_AudioCodec_eAc3Plus
#define AUDIO_CODEC2 NEXUS_AudioCodec_ePcmWav
#define AUDIO_CODEC3 NEXUS_AudioCodec_ePcmWav
#define AUDIO_PID0 0x30
#define AUDIO_PID1 0x31
#define AUDIO_PID2 0x1
#define AUDIO_PID3 0x1
int main(void)
{
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults[NUM_DECODES];
    NxClient_ConnectSettings connectSettings;
    unsigned connectId[NUM_DECODES];
    NEXUS_PlaypumpHandle playpump[NUM_DECODES];
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaybackHandle playback[NUM_DECODES];
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_FilePlayHandle file[NUM_DECODES];
    NEXUS_SimpleAudioDecoderSettings audioSettings;
    NEXUS_SimpleAudioDecoderHandle audioDecoder[NUM_DECODES];
    NEXUS_SimpleAudioDecoderStartSettings audioProgram[NUM_DECODES];
    NEXUS_SimpleStcChannelHandle stcChannel[NUM_DECODES];
    NEXUS_AudioCodec audioCodec[NUM_DECODES];
    unsigned audioPid[NUM_DECODES];
    const char * filename[NUM_DECODES];
    NEXUS_TransportType transportType[NUM_DECODES];
    NEXUS_Error rc;
    unsigned i;
    bool run = true;

    filename[0] = FILE_NAME0;
    filename[1] = FILE_NAME1;
    filename[2] = FILE_NAME2;
    filename[3] = FILE_NAME3;
    audioCodec[0] = AUDIO_CODEC0;
    audioCodec[1] = AUDIO_CODEC1;
    audioCodec[2] = AUDIO_CODEC2;
    audioCodec[3] = AUDIO_CODEC3;
    audioPid[0] = AUDIO_PID0;
    audioPid[1] = AUDIO_PID1;
    audioPid[2] = AUDIO_PID2;
    audioPid[3] = AUDIO_PID3;
    transportType[0] = TRANSPORT0;
    transportType[1] = TRANSPORT1;
    transportType[2] = TRANSPORT2;
    transportType[3] = TRANSPORT3;

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    for ( i = 0; i < NUM_DECODES; i++ )
    {
        NxClient_GetDefaultAllocSettings(&allocSettings);
        allocSettings.simpleAudioDecoder = 1;
        rc = NxClient_Alloc(&allocSettings, &allocResults[i]);
        if (rc) return BERR_TRACE(rc);

        if (allocResults[i].simpleAudioDecoder.id) {
            audioDecoder[i] = NEXUS_SimpleAudioDecoder_Acquire(allocResults[i].simpleAudioDecoder.id);
        }

        NxClient_GetDefaultConnectSettings(&connectSettings);
        connectSettings.simpleAudioDecoder.decoderCapabilities.type = NxClient_AudioDecoderType_ePersistent;
        connectSettings.simpleAudioDecoder.id = allocResults[i].simpleAudioDecoder.id;
        rc = NxClient_Connect(&connectSettings, &connectId[i]);
        if (rc) return BERR_TRACE(rc);

        if ( i != 1 ||
            (i == 1 && strcmp(filename[0], filename[i]) != 0) )
        {
            stcChannel[i] = NEXUS_SimpleStcChannel_Create(NULL);
            BDBG_ASSERT(stcChannel[i]);

            file[i] = NEXUS_FilePlay_OpenPosix(filename[i], NULL);
            if ( !file[i] )
            {
                BDBG_ERR(("Unable to open file %s", filename[i]));
                return -1;
            }

            NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
            playpump[i] = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
            playback[i] = NEXUS_Playback_Create();
            BDBG_ASSERT(playback[i]);

            NEXUS_Playback_GetSettings(playback[i], &playbackSettings);
            playbackSettings.playpumpSettings.transportType = transportType[i];
            playbackSettings.playpump = playpump[i];
            playbackSettings.simpleStcChannel = stcChannel[i];
            rc = NEXUS_Playback_SetSettings(playback[i], &playbackSettings);
            BDBG_ASSERT(!rc);
        }
        else {
            stcChannel[i] = NULL;
            playpump[i] = NULL;
            playback[i] = NULL;
            file[i] = NULL;
        }

        NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram[i]);

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder[i];
        audioProgram[i].primary.pidChannel = NEXUS_Playback_OpenPidChannel(playback[i]?playback[i]:playback[i-1], audioPid[i], &playbackPidSettings);
        audioProgram[i].primary.codec = audioCodec[i];

        if (audioProgram[i].primary.pidChannel) {
            NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder[i], stcChannel[i]?stcChannel[i]:stcChannel[i-1]);
        }
    }

    for ( i = 0; i < NUM_DECODES; i++ ) {
        switch ( i ) {
        default:
        case 0:
            audioProgram[i].master = true;
            /* intentional fallthrough */
        case 1:
            /*if ( file[1] == NULL ) {
                audioProgram[i].primary.mixingMode = NEXUS_AudioDecoderMixingMode_eDescription;
            }
            else */{
                audioProgram[i].primary.mixingMode = NEXUS_AudioDecoderMixingMode_eStandalone;
            }
            break;
        case 2:
            audioProgram[i].primary.mixingMode = NEXUS_AudioDecoderMixingMode_eSoundEffects;
            break;
        case 3:
            audioProgram[i].primary.mixingMode = NEXUS_AudioDecoderMixingMode_eApplicationAudio;
            break;
        }

        NEXUS_SimpleAudioDecoder_GetSettings(audioDecoder[i], &audioSettings);
        audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.connected = true;
        audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level = i==0 ? 100 : 20;
        audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.duration = 0;
        NEXUS_SimpleAudioDecoder_SetSettings(audioDecoder[i], &audioSettings);

        BDBG_ERR(("Start Audio Decoder %d", i));
        rc = NEXUS_SimpleAudioDecoder_Start(audioDecoder[i], &audioProgram[i]);
        BDBG_ASSERT(!rc);
    }

    for ( i = 0; i < NUM_DECODES; i++ ) {
        if ( playback[i] ) {
            rc = NEXUS_Playback_Start(playback[i], file[i], NULL);
            BDBG_ASSERT(!rc);
        }
    }

    i = 1;
    do
    {
        int tmp;
        NEXUS_Error rc;
        bool toggle = false;

        printf("Main Menu\n");
        printf(" 0) Exit\n");
        printf(" 1) Toggle decoder fade levels\n");
        printf("Enter Selection: \n");
        scanf("%d", &tmp);
        switch ( tmp ) {
        case 0:
            run = false;
            break;
        case 1:
            toggle = true;
            break;
        default:
            break;
        }

        if ( toggle ) {
            unsigned j;
            unsigned lowLevel=100, highLevel=0;
            for ( j = 0; j < NUM_DECODES; j++ ) {
                NEXUS_SimpleAudioDecoder_GetSettings(audioDecoder[j], &audioSettings);
                if ( audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level > highLevel )
                {
                    highLevel = audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level;
                }
                if ( audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level < lowLevel )
                {
                    lowLevel = audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level;
                }
            }
            for ( j = 0; j < NUM_DECODES; j++ ) {
                BDBG_ERR(("Set new fade value for decoder %d, i %d", j, i));
                NEXUS_SimpleAudioDecoder_GetSettings(audioDecoder[j], &audioSettings);
                audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level = (i==j) ? highLevel : lowLevel;
                audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.duration = 1250;
                BDBG_ERR(("  ... for %d ms, to level %u%%\n",
                          audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.duration,
                          audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].fade.settings.level));
                NEXUS_SimpleAudioDecoder_SetSettings(audioDecoder[j], &audioSettings);
            }

            #if 1
            /* wait for fade to activate */
            {
                NEXUS_AudioProcessorStatus processorStatus[NUM_DECODES];
                do {
                    BKNI_Sleep(100);
                    for ( j = 0; j < NUM_DECODES; j++ )
                    {
                        rc = NEXUS_SimpleAudioDecoder_GetProcessorStatus(audioDecoder[j], NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioPostProcessing_eFade, &processorStatus[j]);
                        if ( !rc && processorStatus[j].status.fade.active )
                        {
                            BDBG_ERR(("dec[%d]: rc %lu, type %lu, active %lu, remain %lu, lvl %d%%", j,(unsigned long)rc,(unsigned long)processorStatus[j].type,(unsigned long)processorStatus[j].status.fade.active,(unsigned long)processorStatus[j].status.fade.remaining,(int)processorStatus[j].status.fade.level));
                        }
                        if ( rc ) {
                            continue;
                        }
                    }
                } while ( rc != NEXUS_SUCCESS || processorStatus[i].type == NEXUS_AudioPostProcessing_eMax || processorStatus[i].status.fade.active );
            }
            #endif

            i = (i+1)%4;
        }
    } while ( run );

    for ( i = 0; i < NUM_DECODES; i++ ) {
        if ( playback[i] ) {
            NEXUS_Playback_Stop(playback[i]);
            NEXUS_Playback_Destroy(playback[i]);
        }
        if ( playpump[i] ) {
            NEXUS_Playpump_Close(playpump[i]);
        }
        if ( file[i] ) {
            NEXUS_FilePlay_Close(file[i]);
        }
    }
    for ( i = 0; i < NUM_DECODES; i++ ) {
        NxClient_Disconnect(connectId[i]);
        NxClient_Free(&allocResults[i]);
    }
    NxClient_Uninit();
    return 0;
}
