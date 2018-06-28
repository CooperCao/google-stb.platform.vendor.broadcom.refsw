/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nxclient.h"
#include <stdio.h>
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(decode_display_clip);

#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x21

typedef enum MyMoveState
{
    MyMoveState_eRight,
    MyMoveState_eDown,
    MyMoveState_eLeft,
    MyMoveState_eUp,
    MyMoveState_eRightDown,
    MyMoveState_eLeftDown,
    MyMoveState_eRandom
} MyMoveState;

typedef struct {
    const char *name; /* string for nexus enum */
    int value; /* nexus enum */
} namevalue_t;

static const namevalue_t s_triStateStrs[] = {
    {"auto", NEXUS_TristateEnable_eAuto},
    {"off",  NEXUS_TristateEnable_eDisable},
    {"on",   NEXUS_TristateEnable_eEnable},
    {NULL, 0}
};

static void source_changed_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BDBG_LOG(("source changed"));
    BKNI_SetEvent((BKNI_EventHandle)context);
}

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static unsigned b_parse_size(const char *parse)
{
    if (strchr(parse, 'M') || strchr(parse, 'm')) {
        return atof(parse)*1024*1024;
    }
    else if (strchr(parse, 'K') || strchr(parse, 'k')) {
        return atof(parse)*1024;
    }
    else {
        return strtoul(parse, NULL, 0);
    }
}

const char *lookup_name(const namevalue_t *table, int value)
{
    unsigned i;
    for (i=0;table[i].name;i++) {
        if (table[i].value == value) {
            return table[i].name;
        }
    }
    return NULL;
}
unsigned lookup(const namevalue_t *table, const char *name)
{
    unsigned i;
    unsigned value;
    char *endptr;
    const char *valueName;
    for (i=0;table[i].name;i++) {
        if (!strcasecmp(table[i].name, name)) {
            return table[i].value;
        }
    }
    value = strtol(name, &endptr, 0);
    if(!endptr || *endptr) { /* if valid, *endptr = '\0' */
        value = table[0].value;
    }
    valueName = lookup_name(table, value);
    printf("Unknown cmdline param '%s', using %u as value ('%s')\n", name, value, valueName?valueName:"unknown");
    return value;
}

static void print_usage(void)
{
    printf(
    "Usage: decode_display_clip OPTIONS\n"
    "\n"
    "OPTIONS:\n"
    "  --help or -h for help\n"
    "  -timeout SECONDS\n"
    "  -fillContentModeBars {auto|on|off}   video window pillarbox/letterbox bars filled with display background color\n"
    "  -stepSize N              video window movement step size in pixel lines or columns\n"
    "  -timeout SECONDS\n"
    );
}

int main(int argc, const char **argv)
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
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_Error rc;
    NEXUS_SurfaceClientHandle surfaceClient, videoSurfaceClient;
    NEXUS_SurfaceClientSettings surfaceClientSettings;
    BKNI_EventHandle sourceChangedEvent;
    unsigned starttime, timeout = 0; /* seconds */
    unsigned interval = 100;/* ms */
    int curarg = 1;
    MyMoveState moveState = MyMoveState_eRight;
    NEXUS_TristateEnable fillContentModeBars = NEXUS_TristateEnable_eAuto;
    int stepSize = 25;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-fillContentModeBars") && argc>curarg+1) {
            fillContentModeBars = lookup(s_triStateStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-stepSize") && argc>curarg+1) {
            stepSize = b_parse_size(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = b_parse_size(argv[++curarg]);
        }
        curarg++;
    }

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    BKNI_CreateEvent(&sourceChangedEvent);
    file = NEXUS_FilePlay_OpenPosix(FILE_NAME, NULL);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);
    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.simpleStcChannel = stcChannel;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);

    surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    videoSurfaceClient = NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, 0);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = allocResults.surfaceClient[0].id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
    videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);
    videoProgram.settings.codec = VIDEO_CODEC;

    NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);

    BDBG_LOG(("starting normal decode"));
    NEXUS_Playback_Start(playback, file, NULL);

    /* source clipping requires no auto A/R correction, so we switch to this first. If you need it, you'll
    have to do manual A/R correction by resizing the window. */
    BDBG_LOG(("box a/r"));
    NEXUS_SurfaceClient_GetSettings(videoSurfaceClient, &surfaceClientSettings);
    surfaceClientSettings.composition.contentMode = NEXUS_VideoWindowContentMode_eBox;
    NEXUS_SurfaceClient_SetSettings(videoSurfaceClient, &surfaceClientSettings);

    /* finally, how fast can we change source clipping in response to a sourceChanged callback?
    This requires a stream which changes source format. */
    NEXUS_SimpleVideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.sourceChanged.callback = source_changed_callback;
    videoDecoderSettings.sourceChanged.context = sourceChangedEvent;
    NEXUS_SimpleVideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);

    surfaceClientSettings.composition.clipBase.width = surfaceClientSettings.composition.position.width;
    surfaceClientSettings.composition.clipBase.height = surfaceClientSettings.composition.position.height;
    surfaceClientSettings.composition.clipRect.width = surfaceClientSettings.composition.position.width;
    surfaceClientSettings.composition.clipRect.height = surfaceClientSettings.composition.position.height;
    surfaceClientSettings.composition.fillContentModeBars = fillContentModeBars;
    surfaceClientSettings.composition.position.y = -surfaceClientSettings.composition.position.height/2;
    BDBG_LOG(("Moving right...-> %d,%d,%u,%u",
        surfaceClientSettings.composition.position.x, surfaceClientSettings.composition.position.y,
        surfaceClientSettings.composition.position.width, surfaceClientSettings.composition.position.height));

    starttime = b_get_time();
    while (1) {
        BKNI_Sleep(interval);

        switch(moveState)
        {
        case MyMoveState_eRight:
            surfaceClientSettings.composition.position.x += stepSize;
            if(surfaceClientSettings.composition.position.x >= surfaceClientSettings.composition.position.width)
            {
                BDBG_LOG(("Moving down..."));
                moveState = MyMoveState_eDown;
                surfaceClientSettings.composition.position.x = surfaceClientSettings.composition.position.width/2;
            }
            break;
        case MyMoveState_eDown:
            surfaceClientSettings.composition.position.y += stepSize;
            if(surfaceClientSettings.composition.position.y >= surfaceClientSettings.composition.position.height)
            {
                BDBG_LOG(("Moving left..."));
                moveState = MyMoveState_eLeft;
                surfaceClientSettings.composition.position.y = surfaceClientSettings.composition.position.height/2;
            }
            break;
        case MyMoveState_eLeft:
            surfaceClientSettings.composition.position.x -= stepSize;
            if(surfaceClientSettings.composition.position.x <= -surfaceClientSettings.composition.position.width)
            {
                BDBG_LOG(("Moving up..."));
                moveState = MyMoveState_eUp;
                surfaceClientSettings.composition.position.x = -surfaceClientSettings.composition.position.width/2;
            }
            break;
        case MyMoveState_eUp:
            surfaceClientSettings.composition.position.y -= stepSize;
            if(surfaceClientSettings.composition.position.y <= -surfaceClientSettings.composition.position.height)
            {
                BDBG_LOG(("Moving rightdown..."));
                moveState = MyMoveState_eRightDown;
                surfaceClientSettings.composition.position.x = -surfaceClientSettings.composition.position.width +1;
                surfaceClientSettings.composition.position.y = -surfaceClientSettings.composition.position.height+1;
            }
            break;
        case MyMoveState_eRightDown:
            surfaceClientSettings.composition.position.x += stepSize;
            surfaceClientSettings.composition.position.y += stepSize;
            if(surfaceClientSettings.composition.position.y >= surfaceClientSettings.composition.position.height ||
               surfaceClientSettings.composition.position.x >= surfaceClientSettings.composition.position.width)
            {
                BDBG_LOG(("Moving leftdown..."));
                moveState = MyMoveState_eLeftDown;
                surfaceClientSettings.composition.position.x = surfaceClientSettings.composition.position.width  -1;
                surfaceClientSettings.composition.position.y = -surfaceClientSettings.composition.position.height+1;
            }
            break;
        case MyMoveState_eLeftDown:
            surfaceClientSettings.composition.position.x -= stepSize;
            surfaceClientSettings.composition.position.y += stepSize;
            if(surfaceClientSettings.composition.position.y >= surfaceClientSettings.composition.position.height ||
               surfaceClientSettings.composition.position.x <= -surfaceClientSettings.composition.position.width)
            {
                BDBG_LOG(("Moving randomly..."));
                moveState = MyMoveState_eRandom;
                interval = 1000;
                surfaceClientSettings.composition.position.x = 0;
                surfaceClientSettings.composition.position.y = 0;
            }
            break;
        case MyMoveState_eRandom:
        default:
            surfaceClientSettings.composition.position.x = rand()%2?
                -(rand()%surfaceClientSettings.composition.position.width) :
                (rand()%surfaceClientSettings.composition.position.width);
            surfaceClientSettings.composition.position.y = rand()%2?
                -(rand()%surfaceClientSettings.composition.position.height) :
                (rand()%surfaceClientSettings.composition.position.height);
            break;
        }

        NEXUS_SurfaceClient_SetSettings(videoSurfaceClient, &surfaceClientSettings);
        BDBG_LOG(("set display clip to -> %d,%d,%u,%u",
            surfaceClientSettings.composition.position.x, surfaceClientSettings.composition.position.y,
            surfaceClientSettings.composition.position.width, surfaceClientSettings.composition.position.height));

        if (timeout && (b_get_time() - starttime) / 1000 > timeout) {
            break;
        }
    }

    NEXUS_Playback_Stop(playback);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_FilePlay_Close(file);
    NxClient_Disconnect(connectId);
    BKNI_DestroyEvent(sourceChangedEvent);
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
