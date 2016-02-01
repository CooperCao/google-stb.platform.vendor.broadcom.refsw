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
#if NEXUS_HAS_DISPLAY && NEXUS_HAS_SIMPLE_DECODER
#include "nexus_platform_client.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_playback.h"
#include "nexus_surface_client.h"
#include "nexus_hddvi_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "nxclient.h"

BDBG_MODULE(analog_input);

static void print_usage(void)
{
    printf(
        "Usage: analog_input [INDEX]\n"
        );
}

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_SimpleAudioPlaybackHandle audioPlayback;
    NEXUS_SimpleAudioPlaybackStartSettings startSettings;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SurfaceClientHandle surfaceClient;
    NEXUS_HdDviInputHandle hdDviInput;
    int rc;
    int curarg = 1;
    unsigned index = 0;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-index") && curarg+1 < argc) {
            index = atoi(argv[++curarg]);
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

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioPlayback = 1;
    allocSettings.surfaceClient = 1; /* surface client required for video window */
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {rc = BERR_TRACE(rc); goto err_request;}

    if (allocResults.simpleVideoDecoder[0].id) {
        videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    }

    audioPlayback = NEXUS_SimpleAudioPlayback_Acquire(allocResults.simpleAudioPlayback[0].id);
    if (!audioPlayback) {
        BDBG_ERR(("unable to acquire audio playback"));
        goto err_acquire;
    }

    surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    if (surfaceClient) {
        /* creating the video window is necessasy so that SurfaceCompositor can resize the video window */
        NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, 0);
    }

    /* connect client resources to server's resources */
    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = allocResults.surfaceClient[0].id;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = 0; /* no decoder needed, just a window */
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = 0;
    connectSettings.simpleAudioPlayback[0].id = allocResults.simpleAudioPlayback[0].id;
    connectSettings.simpleAudioPlayback[0].i2s.enabled = true;
    connectSettings.simpleAudioPlayback[0].i2s.index = index;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) {rc = BERR_TRACE(rc); goto err_connect;}

    hdDviInput = NEXUS_HdDviInput_Open(index, NULL);
    if (!hdDviInput) {
        BDBG_ERR(("HdDviInput %d not available", index));
        return -1;
    }

    rc = NEXUS_SimpleVideoDecoder_StartHdDviInput(videoDecoder, hdDviInput, NULL);
    BDBG_ASSERT(!rc);

    NEXUS_SimpleAudioPlayback_GetDefaultStartSettings(&startSettings);
    startSettings.sampleRate = 44100;
    startSettings.bitsPerSample = 16;
    rc = NEXUS_SimpleAudioPlayback_Start(audioPlayback, &startSettings);
    if (rc) {rc = BERR_TRACE(rc);goto err_start;}

    BDBG_WRN(("HD-DVI%d and I2S%d inputs connected. Press ENTER to stop", index, index));
    getchar();

    NEXUS_SimpleAudioPlayback_Stop(audioPlayback);

err_start:
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
    printf("This application is not supported on this platform (needs display and simple_decoder)!\n");
    return 0;
}
#endif
