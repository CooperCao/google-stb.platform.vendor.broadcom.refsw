/***************************************************************************
 *     (c)2011-2013 Broadcom Corporation
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

BDBG_MODULE(playback);

/* for this example, use two files: primary and description. */
#define FILE0_NAME "videos/cnnticker.mpg"
#if 1
#define FILE1_NAME "videos/spider_cc.mpg"
#else
/* use this for one file with two pids */
#define FILE1_NAME NULL
#endif
#define AUDIO0_CODEC NEXUS_AudioCodec_eMpeg
#define AUDIO0_PID 0x22
#define AUDIO1_CODEC NEXUS_AudioCodec_eAc3
#define AUDIO1_PID 0x14

int main(void)
{
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_PlaypumpHandle playpump[2];
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaybackHandle playback[2];
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_FilePlayHandle file[2];
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_Error rc;
    unsigned i;
    const char *filename[2] = {FILE0_NAME, FILE1_NAME};

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    BDBG_ASSERT(stcChannel);
    for (i=0;i<2;i++) {
        if (!filename[i]) {
            playback[i] = NULL;
            continue;
        }
        file[i] = NEXUS_FilePlay_OpenPosix(filename[i], NULL);

        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
        playpump[i] = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
        playback[i] = NEXUS_Playback_Create();
        BDBG_ASSERT(playback[i]);

        NEXUS_Playback_GetSettings(playback[i], &playbackSettings);
        playbackSettings.playpump = playpump[i];
        playbackSettings.simpleStcChannel = stcChannel;
        rc = NEXUS_Playback_SetSettings(playback[i], &playbackSettings);
        BDBG_ASSERT(!rc);
    }

    if (allocResults.simpleAudioDecoder.id) {
        audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
    }

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder;
    audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(playback[0], AUDIO0_PID, &playbackPidSettings);
    audioProgram.primary.codec = AUDIO0_CODEC;
    audioProgram.description.pidChannel = NEXUS_Playback_OpenPidChannel(playback[1]?playback[1]:playback[0], AUDIO1_PID, &playbackPidSettings);
    audioProgram.description.codec = AUDIO1_CODEC;

    if (audioProgram.primary.pidChannel) {
        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);
    }
    rc = NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Playback_Start(playback[0], file[0], NULL);
    BDBG_ASSERT(!rc);
    if (playback[1]) {
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
}
