/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_playback.h"
#include "nexus_surface_client.h"
#include "nexus_parser_band.h"
#include "nexus_platform.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(pip_swap);

struct gui {
    unsigned index;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceClientHandle surfaceClient;
    NEXUS_SurfaceHandle surface;
    BKNI_EventHandle windowMovedEvent;
};

static struct dvrfile {
    const char *filename;
    NEXUS_TransportType transportType;
    NEXUS_VideoCodec videoCodec;
    unsigned videoPid;
    NEXUS_AudioCodec audioCodec;
    unsigned audioPid;
} g_dvrfile[] = {
    {"videos/cnnticker.mpg", NEXUS_TransportType_eTs, NEXUS_VideoCodec_eMpeg2, 0x21, NEXUS_AudioCodec_eMpeg, 0x22},
    {"videos/spider_cc.mpg", NEXUS_TransportType_eTs, NEXUS_VideoCodec_eMpeg2, 0x11, NEXUS_AudioCodec_eAc3, 0x14}
};

struct context {
    unsigned index;
    NxClient_AllocResults allocResults;
    unsigned connectId;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_FilePlayHandle file;
    NEXUS_ParserBand parserBand;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleStcChannelHandle stcChannel;
    struct {
        int x, y, h;
    } move;
    NEXUS_SurfaceClientHandle videoSurfaceClient;
    NxClient_VideoWindowType videoWindowType;
};

static void complete2(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static int init_context(unsigned index, struct context *context)
{
    memset(context, 0, sizeof(*context));
    context->index = index;
    context->videoWindowType = index ? NxClient_VideoWindowType_ePip : NxClient_VideoWindowType_eMain;
    return 0;
}

static void set_move(struct context *context)
{
    if (context->videoWindowType == NxClient_VideoWindowType_eMain) {
        context->move.x = context->move.y = 1;
        context->move.h = -1;
    }
    else {
        context->move.x = -2;
        context->move.y = -1;
        context->move.h = 0;
    }
}

static int set_full(struct context *context)
{
    NEXUS_SurfaceClientSettings settings;
    bool pip = context->videoWindowType == NxClient_VideoWindowType_ePip;
    if (!context->connectId) return 0;
    NEXUS_SurfaceClient_GetSettings(context->videoSurfaceClient, &settings);
    settings.composition.contentMode = NEXUS_VideoWindowContentMode_eFull;
    settings.composition.position.width = pip ? 960 : 1920;
    settings.composition.position.height = pip ? 540 : 1080;
    settings.composition.position.x = pip ? 960 : 0;
    settings.composition.position.y = 0;
    settings.composition.zorder = pip?1:0;
    NEXUS_SurfaceClient_SetSettings(context->videoSurfaceClient, &settings);
    return 0;
}

static int start_decode(struct context *context, const struct dvrfile *dvrfile, unsigned live_program, const struct gui *gui)
{
    NxClient_AllocSettings allocSettings;
    NxClient_ConnectSettings connectSettings;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_Error rc;
    /* if gui->index is 1, then these are two video windows under two top-level SurfaceClients */
    unsigned windowId = gui->index ? 0 : context->index;

    if (context->connectId) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &context->allocResults);
    if (rc) return BERR_TRACE(rc);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&context->videoProgram);
    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&context->audioProgram);

    context->stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

    if (dvrfile) {
        context->file = NEXUS_FilePlay_OpenPosix(dvrfile->filename, NULL);
        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
        context->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
        context->playback = NEXUS_Playback_Create();

        NEXUS_Playback_GetSettings(context->playback, &playbackSettings);
        playbackSettings.playpump = context->playpump;
        playbackSettings.simpleStcChannel = context->stcChannel;
        playbackSettings.stcTrick = true;
        rc = NEXUS_Playback_SetSettings(context->playback, &playbackSettings);
        BDBG_ASSERT(!rc);
    }
    else {
        NEXUS_ParserBandSettings parserBandSettings;
        context->parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
        NEXUS_ParserBand_GetSettings(context->parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
        NEXUS_ParserBand_SetSettings(context->parserBand, &parserBandSettings);
    }

    context->videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(context->allocResults.simpleVideoDecoder[0].id);
    context->audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(context->allocResults.simpleAudioDecoder.id);

    context->videoSurfaceClient = NEXUS_SurfaceClient_AcquireVideoWindow(gui->surfaceClient, windowId);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = context->allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = gui->allocResults.surfaceClient[0].id;
    connectSettings.simpleVideoDecoder[0].windowId = windowId;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.type = context->videoWindowType;
    connectSettings.simpleAudioDecoder.id = context->allocResults.simpleAudioDecoder.id;
    connectSettings.simpleAudioDecoder.primer = true; /* don't grab audio in use. prime instead. */
    rc = NxClient_Connect(&connectSettings, &context->connectId);
    if (rc) return BERR_TRACE(rc);

    set_full(context);

    if (dvrfile) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = dvrfile->videoCodec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.simpleDecoder = context->videoDecoder;
        context->videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(context->playback, dvrfile->videoPid, &playbackPidSettings);
        context->videoProgram.settings.codec = dvrfile->videoCodec;

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.simpleDecoder = context->audioDecoder;
        context->audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(context->playback, dvrfile->audioPid, &playbackPidSettings);
        context->audioProgram.primary.codec = dvrfile->audioCodec;
        context->audioProgram.primer.pcm = true;
        context->audioProgram.primer.compressed = true;
    }
    else {
        NEXUS_SimpleStcChannelSettings stcSettings;

        context->videoProgram.settings.pidChannel = NEXUS_PidChannel_Open(context->parserBand, live_program?0x11:0x31, NULL);
        context->videoProgram.settings.codec = NEXUS_VideoCodec_eMpeg2;

        context->audioProgram.primary.pidChannel = NEXUS_PidChannel_Open(context->parserBand, live_program?0x14:0x34, NULL);
        context->audioProgram.primary.codec = NEXUS_AudioCodec_eAc3;

        NEXUS_SimpleStcChannel_GetSettings(context->stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_ePcr;
        stcSettings.modeSettings.pcr.pidChannel = context->videoProgram.settings.pidChannel;
        stcSettings.master = (context->index == 0);
        NEXUS_SimpleStcChannel_SetSettings(context->stcChannel, &stcSettings);
    }

    NEXUS_SimpleVideoDecoder_SetStcChannel(context->videoDecoder, context->stcChannel);
    NEXUS_SimpleAudioDecoder_SetStcChannel(context->audioDecoder, context->stcChannel);
    NEXUS_SimpleVideoDecoder_Start(context->videoDecoder, &context->videoProgram);
    NEXUS_SimpleAudioDecoder_Start(context->audioDecoder, &context->audioProgram);
    if (dvrfile) {
        NEXUS_Playback_Start(context->playback, context->file, NULL);
    }
    return 0;
}

static void stop_decode(struct context *context)
{
    if (context->playback) {
        NEXUS_Playback_Stop(context->playback);
        NEXUS_Playback_Destroy(context->playback);
        NEXUS_Playpump_Close(context->playpump);
        NEXUS_FilePlay_Close(context->file);
    }
    else {
    }
    NEXUS_SimpleVideoDecoder_Release(context->videoDecoder);
    NEXUS_SimpleAudioDecoder_Release(context->audioDecoder);
    NEXUS_SurfaceClient_ReleaseVideoWindow(context->videoSurfaceClient);
    context->videoSurfaceClient = NULL;
    NEXUS_SimpleStcChannel_Destroy(context->stcChannel);
    NxClient_Disconnect(context->connectId);
    context->connectId = 0;
    NxClient_Free(&context->allocResults);
}

static void move(struct context *context)
{
    NEXUS_SurfaceClientSettings settings;
    if (!context->connectId) return;
    BDBG_ASSERT(context->videoSurfaceClient);
    NEXUS_SurfaceClient_GetSettings(context->videoSurfaceClient, &settings);
    settings.composition.position.height += context->move.h;
    settings.composition.position.width = settings.composition.position.height * 16 / 9;
    settings.composition.position.x += context->move.x;
    settings.composition.position.y += context->move.y;
    NEXUS_SurfaceClient_SetSettings(context->videoSurfaceClient, &settings);
    if (context->move.h) {
        if (settings.composition.position.height > 1080 || settings.composition.position.height < 200) {
            context->move.h *= -1;
        }
    }
    if (settings.composition.position.x + settings.composition.position.width >= 1920 ||
        settings.composition.position.x < 0) context->move.x *= -1;
    if (settings.composition.position.y + settings.composition.position.height >= 1080 ||
        settings.composition.position.y < 0) context->move.y *= -1;
    BDBG_MSG(("%d: %d,%d,%d,%d      change by %d,%d",
        context->index,
        settings.composition.position.x, settings.composition.position.y, settings.composition.position.width, settings.composition.position.height,
        context->move.x, context->move.y));
}

/* create single top-level SurfaceClient for GUI */
static int create_gui(unsigned index, struct gui *gui)
{
    NxClient_AllocSettings allocSettings;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceMemory mem;
    int rc;
    NEXUS_SurfaceComposition comp;

    BKNI_Memset(gui, 0, sizeof(*gui));
    gui->index = index;
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &gui->allocResults);
    if (rc) return BERR_TRACE(rc);
    gui->surfaceClient = NEXUS_SurfaceClient_Acquire(gui->allocResults.surfaceClient[0].id);
    if (!gui->surfaceClient) {
        BDBG_ERR(("NEXUS_SurfaceClient_Acquire failed"));
        return -1;
    }

    /* full screen transparent surface */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = 480;
    gui->surface = NEXUS_Surface_Create(&createSettings);

    NEXUS_Surface_GetMemory(gui->surface, &mem);
    BKNI_Memset(mem.buffer, 0, mem.pitch * createSettings.height);
    NEXUS_Surface_Flush(gui->surface);
    rc = NEXUS_SurfaceClient_SetSurface(gui->surfaceClient, gui->surface);
    if (rc) return BERR_TRACE(rc);

    if (index == 0) {
        NEXUS_SurfaceClientSettings settings;
        BKNI_CreateEvent(&gui->windowMovedEvent);
        NEXUS_SurfaceClient_GetSettings(gui->surfaceClient, &settings);
        settings.windowMoved.callback = complete2;
        settings.windowMoved.context = gui->windowMovedEvent;
        NEXUS_SurfaceClient_SetSettings(gui->surfaceClient, &settings);
    }

    NxClient_GetSurfaceClientComposition(gui->allocResults.surfaceClient[0].id, &comp);
    comp.zorder = index;
    NxClient_SetSurfaceClientComposition(gui->allocResults.surfaceClient[0].id, &comp);
    return 0;
}

static void destroy_gui(const struct gui *gui)
{
    NEXUS_Surface_Destroy(gui->surface);
    NEXUS_SurfaceClient_Release(gui->surfaceClient);
    NxClient_Free(&gui->allocResults);
    if (gui->windowMovedEvent) {
        BKNI_DestroyEvent(gui->windowMovedEvent);
    }
}

/* NxClient_Reconfig will toggle server state, but if the client stops and restarts, it must
choose the correct NxClient_VideoWindowType. So there's a client side toggle that's required too. */
static void swap_windows(struct context *context0, struct context *context1)
{
    struct context temp = *context0;
    context0->videoWindowType = context1->videoWindowType;
    context1->videoWindowType = temp.videoWindowType;
}

static void swap_master(struct context *context0, struct context *context1)
{
    if (context0->playback) {
        BDBG_ERR(("master not supported with playback"));
    }
    else {
        /* have large window drive display/audio rate managers (even if not BVN CMP0_V0) */
        NEXUS_SimpleStcChannelSettings settings0, settings1;
        int rc;
        NEXUS_SimpleStcChannel_GetSettings(context0->stcChannel, &settings0);
        NEXUS_SimpleStcChannel_GetSettings(context1->stcChannel, &settings1);
        settings0.master = !settings0.master;
        settings1.master = !settings1.master;
        BDBG_ASSERT(settings0.master != settings1.master);
        rc = NEXUS_SimpleStcChannel_SetSettings(context0->stcChannel, &settings0);
        if (rc) BERR_TRACE(rc); /* keep going */
        rc = NEXUS_SimpleStcChannel_SetSettings(context1->stcChannel, &settings1);
        if (rc) BERR_TRACE(rc); /* keep going */

        BDBG_WRN(("%s is master", (settings0.master == (context0->videoWindowType == NxClient_VideoWindowType_eMain)) ? "Main" : "PIP"));
    }
}

int main(int argc, char **argv)
{
    struct context context[2];
    struct gui gui[2];
    int rc;
    bool done = false;
    unsigned i;
    bool dual_client = false;
    int curarg = 1;
    bool live = false;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-dual")) {
            dual_client = true;
        }
        else if (!strcmp(argv[curarg], "-live")) {
            live = true;
        }
        curarg++;
    }

    rc = NxClient_Join(NULL);
    if (rc) return rc;

    rc = create_gui(0, &gui[0]);
    if (rc) return rc;
    /* main/PIP can be done under one SurfaceClient or two SurfaceClients.
    if dual_client, use two SurfaceClients. */
    if (dual_client) {
        rc = create_gui(1, &gui[1]);
        if (rc) return rc;
    }
    init_context(0, &context[0]);
    init_context(1, &context[1]);
    rc = start_decode(&context[0], live?NULL:&g_dvrfile[0], 0, &gui[0]);
    if (rc) return rc;
    rc = start_decode(&context[1], live?NULL:&g_dvrfile[1], 1, dual_client?&gui[1]:&gui[0]);
    if (rc) return rc;
    while (!done) {
        NxClient_ReconfigSettings reconfigSettings;
        char buf[32];

        printf(
        "Commands\n"
        "0: quit\n"
        "1: swap main and pip video and audio and master\n"
        "2: swap main and pip video\n"
        "3: swap main and pip audio\n"
        "4: swap main and pip master\n"
        "5: start/stop main\n"
        "6: start/stop pip\n"
        "7: animate\n"
        );
        fgets(buf, sizeof(buf), stdin);
        switch (atoi(buf)) {
        case 0:
            if (buf[0] == '0') {
                done = true;
            }
            break;
        case 1:
            NxClient_GetDefaultReconfigSettings(&reconfigSettings);
            reconfigSettings.command[0].type = NxClient_ReconfigType_eRerouteVideoAndAudio;
            reconfigSettings.command[0].connectId1 = context[0].connectId;
            reconfigSettings.command[0].connectId2 = context[1].connectId;
            rc = NxClient_Reconfig(&reconfigSettings);
            if (!rc) {
                swap_windows(&context[0], &context[1]);
                swap_master(&context[0], &context[1]);
            }
            break;
        case 2:
            NxClient_GetDefaultReconfigSettings(&reconfigSettings);
            reconfigSettings.command[0].type = NxClient_ReconfigType_eRerouteVideo;
            reconfigSettings.command[0].connectId1 = context[0].connectId;
            reconfigSettings.command[0].connectId2 = context[1].connectId;
            rc = NxClient_Reconfig(&reconfigSettings);
            if (!rc) {
                swap_windows(&context[0], &context[1]);
            }
            break;
        case 3:
            NxClient_GetDefaultReconfigSettings(&reconfigSettings);
            reconfigSettings.command[0].type = NxClient_ReconfigType_eRerouteAudio;
            reconfigSettings.command[0].connectId1 = context[0].connectId;
            reconfigSettings.command[0].connectId2 = context[1].connectId;
            NxClient_Reconfig(&reconfigSettings);
            break;
        case 4:
            swap_master(&context[0], &context[1]);
            break;
        case 5:
            if (context[0].connectId) {
                stop_decode(&context[0]);
            }
            else {
                start_decode(&context[0], live?NULL:&g_dvrfile[0], 0, &gui[0]);
            }
            break;
        case 6:
            if (context[1].connectId) {
                stop_decode(&context[1]);
            }
            else {
                start_decode(&context[1], live?NULL:&g_dvrfile[1], 1, dual_client?&gui[1]:&gui[0]);
            }
            break;
        case 7:
            set_move(&context[0]);
            set_move(&context[1]);
            for (i=0;i<500;i++) {
                if (i % 100 == 0) BDBG_WRN(("%u moves", i));
                move(&context[0]);
                move(&context[1]);
                rc = BKNI_WaitForEvent(gui[0].windowMovedEvent, 5000);
                BDBG_ASSERT(!rc);
            }
            set_full(&context[0]);
            set_full(&context[1]);
            break;
        default:
            break;
        }
    }
    stop_decode(&context[0]);
    stop_decode(&context[1]);
    destroy_gui(&gui[0]);
    if (dual_client) {
        destroy_gui(&gui[1]);
    }
    NxClient_Uninit();
    return 0;
}
