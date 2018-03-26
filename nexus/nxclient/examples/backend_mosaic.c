/******************************************************************************
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
 *****************************************************************************/

#include "nxclient.h"
#include "nexus_parser_band.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_surface_client.h"
#include "nexus_platform.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(backend_mosaic);

#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x31

int main(int argc, char **argv)
{
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleStcChannelSettings stcSettings;
#define NUM_MOSAIC 4
    NEXUS_Rect rect[NUM_MOSAIC];
    NEXUS_Error rc;
    unsigned i;
    NEXUS_SurfaceClientHandle surfaceClient, videoSurfaceClient[NUM_MOSAIC];
    unsigned timeout = 0;
    int curarg = 1;

    while (curarg < argc) {
        if (!strcmp("-timeout", argv[curarg]) && curarg+1 < argc) {
            timeout = atoi(argv[++curarg]);
        }
        curarg++;
    }

    NxClient_Join(NULL);

    /* cut up 1920x1080 into 4 tiles */
    for (i=0;i<NUM_MOSAIC;i++) {
        rect[i].x = i%2?960:0;
        rect[i].y = i<2?0:540;
        rect[i].width = 960;
        rect[i].height = 540;
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    BDBG_ASSERT(videoDecoder);
    surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    for (i=0;i<NUM_MOSAIC;i++) {
        videoSurfaceClient[i] = NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, i);
    }
    for (i=0;i<NUM_MOSAIC;i++) {
        NEXUS_SurfaceClientSettings settings;
        const unsigned remapping[] = {2,0,3,1};
        NEXUS_SurfaceClient_GetSettings(videoSurfaceClient[i], &settings);
        settings.composition.position  = rect[i];
        settings.composition.clipRect = rect[remapping[i]];
        settings.composition.clipBase.width = 1920;
        settings.composition.clipBase.height = 1080;
        rc = NEXUS_SurfaceClient_SetSettings(videoSurfaceClient[i], &settings);
        BDBG_ASSERT(!rc);
    }

    parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    for (i=0;i<NUM_MOSAIC;i++) {
        connectSettings.simpleVideoDecoder[i].id = i?NXCLIENT_CONNECT_BACKEND_MOSAIC:allocResults.simpleVideoDecoder[0].id;
        connectSettings.simpleVideoDecoder[i].surfaceClientId = allocResults.surfaceClient[0].id;
        connectSettings.simpleVideoDecoder[i].windowId = i;
    }
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.settings.pidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    videoProgram.settings.codec = VIDEO_CODEC;

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_ePcr;
    stcSettings.modeSettings.pcr.pidChannel = videoProgram.settings.pidChannel;
    NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);

    if (videoProgram.settings.pidChannel) {
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    }

    if (videoProgram.settings.pidChannel) {
        NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
    }

    if (timeout) {
        BKNI_Sleep(timeout * 1000);
    }
    else {
        BDBG_LOG(("Press ENTER to exit"));
        getchar();
    }

    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();

    return 0;
}
