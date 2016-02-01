/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
/* Nexus example app: playback and decode */

#include <stdio.h>
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include <string.h>
#include <stdlib.h>
#include "nexus_platform.h"
#include "bxcode.h"

#include "bkni.h"
#include "bdbg.h"

BDBG_MODULE(transcode);

#define INPUT_VPID        0x21
#define INPUT_APID        0x22
#define INPUT_VCODEC      NEXUS_VideoCodec_eMpeg2
#define INPUT_ACODEC      NEXUS_AudioCodec_eMpeg

struct BTST_Transcoder_t
{
    char *inFile;
    char *outFile;
    BKNI_EventHandle inputReadyEvent, outputReadyEvent;
    BKNI_EventHandle inputDoneEvent, outputDoneEvent, doneEvent;
    bool loop;
    bool nonRealTime;
    unsigned inVpid, inApid;
    NEXUS_VideoCodec inVcodec;
    NEXUS_AudioCodec inAcodec;
    unsigned vBitrate;
    unsigned width, height;
} g_testContext =
{
    "videos/cnnticker.mpg",
    "videos/output.ts",
    NULL, NULL, NULL, NULL, NULL,
    false, false,
    INPUT_VPID, INPUT_APID,
    INPUT_VCODEC,
    INPUT_ACODEC,
    1000000,
    640, 480
};

static void doneHandler(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    BDBG_WRN(("Done!"));
}

int main(int argc, char **argv)  {
    NEXUS_PlatformConfiguration platformConfig;
    BXCode_Handle hBxcode;
    BXCode_StartSettings startSettings;
    BXCode_Settings settings;
    BXCode_OutputStatus outputStatus;
    BKNI_EventHandle doneEvent;
    bool segmented = false;
    int i;

    for(i=0; i<argc; i++) {
        if(!strcmp("-nrt", argv[i])) {
            g_testContext.nonRealTime = true; fprintf(stderr, "NRT mode transcode\n");
        }
        if(!strcmp("-loop", argv[i])) {
            g_testContext.loop = true; fprintf(stderr, "Loop input\n");
        }
        if(!strcmp("-segmented", argv[i])) {
            segmented = true; fprintf(stderr, "Segmented TS output\n");
        }
        if(!strcmp("-in", argv[i])) {
            g_testContext.inFile = argv[++i]; fprintf(stderr, "Input file: %s\n", g_testContext.inFile);
        }
        if(!strcmp("-out", argv[i])) {
            g_testContext.outFile = argv[++i]; fprintf(stderr, "Output file: %s\n", g_testContext.outFile);
        }
        if(!strcmp("-vpid", argv[i])) {
            g_testContext.inVpid = strtoul(argv[++i], NULL, 0); fprintf(stderr, "Input Vpid: %u\n", g_testContext.inVpid);
        }
        if(!strcmp("-apid", argv[i])) {
            g_testContext.inApid = strtoul(argv[++i], NULL, 0); fprintf(stderr, "Input Apid: %u\n", g_testContext.inApid);
        }
        if(!strcmp("-video_size", argv[i])) {
            g_testContext.width  = strtoul(argv[++i], NULL, 0);
            g_testContext.height = strtoul(argv[++i], NULL, 0);
            fprintf(stderr, "Encode video size: %ux%u\n", g_testContext.width, g_testContext.height);
        }
        if(!strcmp("-video_bitrate", argv[i])) {
            g_testContext.vBitrate  = strtoul(argv[++i], NULL, 0);
            fprintf(stderr, "Encode video bitrate: %u bps\n", g_testContext.vBitrate);
        }
        if(!strcmp("-vcodec", argv[i])) {
            i++;
            if(!strcmp(argv[i], "mpeg")) g_testContext.inVcodec = NEXUS_VideoCodec_eMpeg2;
            else if(!strcmp(argv[i], "avc") || !strcmp(argv[i], "h264")) g_testContext.inVcodec = NEXUS_VideoCodec_eH264;
            fprintf(stderr, "Input video codec: %u\n", g_testContext.inVcodec);
        }
        if(!strcmp("-acodec", argv[i])) {
            i++;
            if(!strcmp(argv[i], "aac")) g_testContext.inAcodec = NEXUS_AudioCodec_eAac;
            else if(!strcmp(argv[i], "ac3")) g_testContext.inAcodec = NEXUS_AudioCodec_eAc3;
            fprintf(stderr, "Input audio codec: %u(%s)\n", g_testContext.inAcodec, argv[i]);
        }
    }

    {
        NEXUS_PlatformSettings platformSettings;
        NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.openFrontend = false;
        NEXUS_Platform_Init(&platformSettings);
    }
    NEXUS_Platform_GetConfiguration(&platformConfig);
    BKNI_CreateEvent(&doneEvent);

    hBxcode = BXCode_Open(0, NULL);
    BXCode_GetSettings(hBxcode, &settings);
    settings.video.width  = g_testContext.width;
    settings.video.height = g_testContext.height;
    settings.video.encoder.bitrateMax = g_testContext.vBitrate;
    if(segmented) settings.video.encoder.streamStructure.duration = 2000; /* 2-second segment duration */
    BXCode_SetSettings(hBxcode, &settings);
    BXCode_GetDefaultStartSettings(&startSettings);
    startSettings.nonRealTime = g_testContext.nonRealTime;
    startSettings.input.type = BXCode_InputType_eFile;
    startSettings.input.data = g_testContext.inFile;
    startSettings.input.transportType  = NEXUS_TransportType_eTs;
    startSettings.input.vPid = g_testContext.inVpid;
    startSettings.input.vCodec = g_testContext.inVcodec;
    startSettings.input.aPid[0] = g_testContext.inApid;
    startSettings.input.aCodec[0] = g_testContext.inAcodec;
    startSettings.input.eofDone.callback = doneHandler;
    startSettings.input.eofDone.context  = doneEvent;

    startSettings.output.video.pid = 0x12;
    startSettings.output.audio[0].pid = 0x13;
    startSettings.output.transport.type = BXCode_OutputType_eTs;
    startSettings.output.transport.pcrPid = 0x11;
    startSettings.output.transport.pmtPid = 0x10;
    startSettings.output.transport.file = g_testContext.outFile;
    startSettings.output.transport.segmented = segmented;
    BDBG_MSG(("output type: %d", startSettings.output.transport.type));
    BXCode_Start(hBxcode, &startSettings);

    BKNI_WaitForEvent(doneEvent, BKNI_INFINITE);

    BXCode_GetOutputStatus(hBxcode, &outputStatus);
    fprintf(stderr, "TS stream duration           = %d (ms)\n", outputStatus.mux.duration);
    fprintf(stderr, "TS stream recorded data      = %#x (bytes)\n", (uint32_t)outputStatus.mux.recpumpStatus.data.bytesRecorded);
    fprintf(stderr, "TS stream fifo depth/size    = %#x/%#x (bytes)\n", outputStatus.mux.recpumpStatus.data.fifoDepth, outputStatus.mux.recpumpStatus.data.fifoSize);
    fprintf(stderr, "Video encoder fifo depth/size= %#x/%#x\n", outputStatus.video.data.fifoDepth, outputStatus.video.data.fifoSize);
    fprintf(stderr, "Video error count            = %u\n", outputStatus.video.errorCount);
    fprintf(stderr, "Video error flags            = %u\n", outputStatus.video.errorFlags);
    fprintf(stderr, "Video event flags            = %u\n", outputStatus.video.eventFlags);
    fprintf(stderr, "picture drops due to error   = %u\n", outputStatus.video.picturesDroppedErrors);
    fprintf(stderr, "picture drops due to FRC     = %u\n", outputStatus.video.picturesDroppedFRC);
    fprintf(stderr, "pictures Encoded             = %u\n", outputStatus.video.picturesEncoded);
    fprintf(stderr, "pictures Received            = %u\n", outputStatus.video.picturesReceived);
    fprintf(stderr, "picture Id Last Encoded      = 0x%x\n", outputStatus.video.pictureIdLastEncoded);
    fprintf(stderr, "pictures per second          = %u\n", outputStatus.video.picturesPerSecond);
    fprintf(stderr, "Num of audio PIDs            = %u\n", outputStatus.numAudios);
    fprintf(stderr, "Audio frames count           = %u\n", outputStatus.audio[0].numFrames);
    fprintf(stderr, "Audio error frames count     = %u\n", outputStatus.audio[0].numErrorFrames);
    fprintf(stderr, "\nAV sync error = %.1f (ms)\n\n", outputStatus.avSyncErr[0]);
    BXCode_Stop(hBxcode);
    BXCode_Close(hBxcode);
    BKNI_DestroyEvent(doneEvent);
    NEXUS_Platform_Uninit();

    return 0;
}
#else
int main(void)  {
    printf("This platform doesn't support transcode!\n");
    return 0;
}
#endif
