/***************************************************************************
 * Copyright (C) 2017-2018 Broadcom.  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(playback_pip);
#include "nxapp_prompt.inc"

static struct dvrfile {
    const char *filename;
    NEXUS_TransportType transportType;
    NEXUS_VideoCodec videoCodec;
    unsigned videoPid;
} g_dvrfile[] = {
    {"videos/cnnticker.mpg", NEXUS_TransportType_eTs, NEXUS_VideoCodec_eMpeg2, 0x21},
    {"videos/spider_cc.mpg", NEXUS_TransportType_eTs, NEXUS_VideoCodec_eMpeg2, 0x11}
};

struct context {
    NxClient_AllocResults allocResults;
    unsigned connectId;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_FilePlayHandle file;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleStcChannelHandle stcChannel;
};

static int start_decode(unsigned index, struct context *context, const struct dvrfile *dvrfile)
{
    NxClient_AllocSettings allocSettings;
    NxClient_ConnectSettings connectSettings;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_Error rc;

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &context->allocResults);
    if (rc) return BERR_TRACE(rc);

    context->file = NEXUS_FilePlay_OpenPosix(dvrfile->filename, NULL);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&context->videoProgram);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    context->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    context->playback = NEXUS_Playback_Create();
    context->stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

    NEXUS_Playback_GetSettings(context->playback, &playbackSettings);
    playbackSettings.playpump = context->playpump;
    playbackSettings.simpleStcChannel = context->stcChannel;
    playbackSettings.stcTrick = true;
    rc = NEXUS_Playback_SetSettings(context->playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    context->videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(context->allocResults.simpleVideoDecoder[0].id);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = context->allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.type = index ? NxClient_VideoWindowType_ePip : NxClient_VideoWindowType_eMain;
    rc = NxClient_Connect(&connectSettings, &context->connectId);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = dvrfile->videoCodec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = context->videoDecoder;
    context->videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(context->playback, dvrfile->videoPid, &playbackPidSettings);
    context->videoProgram.settings.codec = dvrfile->videoCodec;

    NEXUS_SimpleVideoDecoder_SetStcChannel(context->videoDecoder, context->stcChannel);
    NEXUS_SimpleVideoDecoder_Start(context->videoDecoder, &context->videoProgram);
    NEXUS_Playback_Start(context->playback, context->file, NULL);
    return 0;
}

static void stop_decode(struct context *context)
{
    NEXUS_Playback_Stop(context->playback);
    NEXUS_Playback_Destroy(context->playback);
    NEXUS_Playpump_Close(context->playpump);
    NEXUS_FilePlay_Close(context->file);
    NEXUS_SimpleVideoDecoder_Release(context->videoDecoder);
    NEXUS_SimpleStcChannel_Destroy(context->stcChannel);
    NxClient_Disconnect(context->connectId);
    NxClient_Free(&context->allocResults);
    NxClient_Uninit();
}

int main(void)
{
    struct context context[2];
    start_decode(0, &context[0], &g_dvrfile[0]);
    start_decode(1, &context[1], &g_dvrfile[1]);
    nxapp_prompt("exit");
    stop_decode(&context[0]);
    stop_decode(&context[1]);
    return 0;
}
#else /* #if NEXUS_HAS_PLAYBACK */
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
