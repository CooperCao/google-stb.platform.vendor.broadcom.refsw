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
#include <stdlib.h>
#include <string.h>
#include "nexus_platform.h"
#include "bxcode.h"
#include "b_os_lib.h"
#include "namevalue.h"
#include "bkni.h"
#include "bdbg.h"

BDBG_MODULE(transcode_stream);

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
    bool fileInput, fileOutput;
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
    false, false, /* default stream i/o */
    INPUT_VPID, INPUT_APID,
    INPUT_VCODEC,
    INPUT_ACODEC,
    1000000,
    640, 480
};

static const namevalue_t g_outputTypeStrs[] = {
    {"ts", BXCode_OutputType_eTs},
    {"es", BXCode_OutputType_eEs},
    {"mp4", BXCode_OutputType_eMp4File},
    {NULL, 0}
};

static char g_keyReturn = '\0';

static void printOutputStatus(BXCode_Handle hBxcode)
{
    BXCode_OutputStatus outputStatus;
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
}

static void doneHandler(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    BDBG_WRN(("Done!"));
    g_keyReturn = 'q';
}

static void inputDataReadyCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    BDBG_MSG(("Input ready!"));
}

static void outputDataReadyCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    BDBG_MSG(("Output ready!"));
}

typedef enum BTPCommand
{
   BTPCommand_eChunkId = 0x0D, /* reuse PICTURE_TAG */
   BTPCommand_eLast = 0x82, /* LAST */
   BTPCommand_eEOS = 0x0A /* protocol agnostic EOS or so-called INLINE_FLUSH or TPD */
} BTPCommand;

static void insertBTP(void *buf, uint32_t chunkId, unsigned pid, BTPCommand command)
{
    int i;
    static uint8_t btp[188] = {
            0x47, 0x1f, 0xff, 0x20,
            0xb7, 0x02, 0x2d, 0x00,
            'B',  'R',  'C',  'M',  /* signature */
            0x00, 0x00, 0x00, 0x0d, /* CHUNK_ID command (reuse PICTURE_TAG) */
            0x00, 0x00, 0x00, 0x00, /* dwod 1 */
            0x00, 0x00, 0x00, 0x00, /* dwod 2 */
            0x00, 0x00, 0x00, 0x00, /* dwod 3 */
            0x00, 0x00, 0x00, 0x00, /* dwod 4 */
            0x00, 0x00, 0x00, 0x00, /* dwod 5 */
            0x00, 0x00, 0x00, 0x00, /* dwod 6 */
            0x00, 0x00, 0x00, 0x00, /* dwod 7 */
            0x00, 0x00, 0x00, 0xbc, /* dwod 8 =188 to avoid RAVE dropping following packets */
            0x00, 0x00, 0x00, 0x00, /* dwod 9: chunkID in big endian */
            /* rest of BTP = 0x00 */
        };

    btp[1] = (pid >> 8) & 0x1f;
    btp[2] = pid & 0xff;  /* PID */

    btp[12 + 3] = command;

    switch(command)
    {
       case BTPCommand_eChunkId:
       case BTPCommand_eLast:
       case BTPCommand_eEOS:
        /* big endian dword[9] for CHUNK_ID BTP command's chunkID payload */
        btp[12+36] = (unsigned char) ((chunkId & 0xff000000) >> 24);
        btp[12+36+1] = (unsigned char) ((chunkId & 0x00ff0000) >> 16);
        btp[12+36+2] = (unsigned char) ((chunkId & 0x0000ff00) >> 8);
        btp[12+36+3] = (unsigned char) (chunkId & 0x000000ff);
        break;
    }
    BKNI_Memcpy(buf,(void *)btp,188);
    BDBG_MSG(("BTP:"));
    for(i=0; i<52; i+=4) {
        BDBG_MSG(("%02x %02x %02x %02x", btp[i], btp[i+1], btp[i+2], btp[i+3]));
    }
}

static void inputFeedThread(BXCode_Handle hBxcode)
{
    FILE *fp = fopen(g_testContext.inFile, "rb");
    if(fp == NULL) { BDBG_ERR(("Input feed thread failed to open input file %s", g_testContext.inFile)); return; }
    while (g_keyReturn != 'q') {
        void *buffer;
        size_t buffer_size;
        int n;
        NEXUS_Error rc;

        if (BXCode_Input_GetBuffer(hBxcode, &buffer, &buffer_size))
            break;
        if (buffer_size == 0) {
            BKNI_WaitForEvent(g_testContext.inputReadyEvent, BKNI_INFINITE);
            continue;
        }

        /* The first call to get_buffer will return the entire playback buffer.
        If we use it, we're going to have to wait until the descriptor is complete,
        and then we're going to underflow. So use a max size. */
#define MAX_READ (188*1024)
        if (buffer_size > MAX_READ)
            buffer_size = MAX_READ;

        n = fread(buffer, 1, buffer_size, fp);
        if (n < 0) goto error;
        if (n == 0) {
            if(!g_testContext.loop) {
                #define EOS_BTP_PACKETS_SIZE   (188*3)
                if(buffer_size >= EOS_BTP_PACKETS_SIZE) {
                    /* if no looping, insert EOS BTP to terminate transcoder */
                    insertBTP(buffer, 0, INPUT_VPID, BTPCommand_eEOS);
                    buffer = (uint8_t*)buffer + 188; buffer_size -= 188;
                    insertBTP(buffer, 0, INPUT_VPID, BTPCommand_eLast);
                    buffer = (uint8_t*)buffer + 188; buffer_size -= 188;
                    insertBTP(buffer, 0, INPUT_VPID, BTPCommand_eEOS);
                    buffer = (uint8_t*)buffer + 188; buffer_size -= 188;
                    rc = BXCode_Input_WriteComplete(hBxcode, 0, EOS_BTP_PACKETS_SIZE);
                    BDBG_ASSERT(!rc);
                } else { BKNI_Sleep(50); continue; } /* if no space for EOS, continue to try later */
            }
            /* wait for the decoder to reach the end of the content before looping */
            while (1) {
                BXCode_InputStatus inStatus;
                BXCode_GetInputStatus(hBxcode, &inStatus);
                if (!inStatus.playpump.fifoDepth) break;
                BKNI_Sleep(100);
            }
            if(g_testContext.loop) fseek(fp, 0, SEEK_SET);
            else break;
        }
        else {
            rc = BXCode_Input_WriteComplete(hBxcode, 0, n);
            BDBG_ASSERT(!rc);
            BDBG_MSG(("played %d bytes", n));
        }
    }
error:
    BDBG_WRN(("Input stream feed thread completed!"));
    fclose(fp);
    if(!g_testContext.nonRealTime) g_keyReturn = 'q';
    BKNI_SetEvent(g_testContext.inputDoneEvent);
}

static void outputRecordThread(BXCode_Handle hBxcode)
{
    BXCode_OutputStream output;
    unsigned count = 0;
    size_t total = 0, i=0, j=0, segment=0;
    FILE *fp = fopen(g_testContext.outFile, "wb");
    if(fp == NULL) { BDBG_ERR(("Output record thread failed to open output file %s", g_testContext.outFile)); return; }
    output.type = BXCode_OutputStreamType_eTs;
    output.id   = 0;
    while (count < 20 && g_keyReturn != 'q') {
        const void *data_buffer[2];
        BXCode_OutputTsDescriptor *pDesc;
        size_t data_buffer_size[2], size = 0;
        int n = 0;

        if (BXCode_Output_GetDescriptors(hBxcode, output, &data_buffer[0], &data_buffer_size[0], &data_buffer[1], &data_buffer_size[1]))
            break;
        if (data_buffer_size[0] == 0 && data_buffer_size[1] == 0) {
            BKNI_WaitForEvent(g_testContext.outputReadyEvent, 50);
            count++;
            continue;
        }
        count = 0;

        for(j = 0; j < 2; j++) {
            for(i = 0; i < data_buffer_size[j]; i++) {
                pDesc = (BXCode_OutputTsDescriptor *)data_buffer[j] + i;
                if(pDesc->flags & BXCODE_OUTPUTDESCRIPTOR_TSFLAG_SEGMENT_START) {
                    BDBG_MSG(("Segment[%u] starts @%#x", segment, total));
                    segment++;
                }
                if(pDesc->size) {
                   n = fwrite(pDesc->pData, 1, pDesc->size, fp);
                   if (n < 0) goto error;
                   total += n;
                   size += n;
                }
            }
        }

        BXCode_Output_ReadComplete(hBxcode, output, data_buffer_size[0]+data_buffer_size[1]);
        BDBG_MSG(("wrote %#x bytes data, %d+%d descriptors", size, data_buffer_size[0], data_buffer_size[1]));
    }
error:
    fclose(fp);
    BDBG_WRN(("Output stream record thread completed with %#x bytes!", total));
    g_keyReturn = 'q';
    BKNI_SetEvent(g_testContext.outputDoneEvent);
}

static void keyHandler( BXCode_Handle  hBxcode )
{
    char key[256];
    int choice;
    int rcInput;

    /* Turn off buffering for stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    while (g_keyReturn != 'q')
    {
        fd_set rfds;
        struct timeval tv;
        int retval;

        printf("Menu:\n");
        printf("14) Get video xcoder status\n");
        printf("99) change DEBUG module setting\n");
        printf("\nEnter 'q' to quit\n\n");

        do {
            /* Watch stdin (fd 0) to see when it has input. */
            FD_ZERO(&rfds);
            FD_SET(0, &rfds);

            /* Wait up to 100 miliseconds. */
            tv.tv_sec = 0;
            tv.tv_usec = 100000;

            retval = select(1, &rfds, NULL, NULL, &tv);
            if(retval<0) BDBG_WRN(("select returns %d\n", retval));
            else if(retval) BDBG_MSG(("Data is available"));
        } while ( g_keyReturn != 'q' && retval <= 0 );

        if ( g_keyReturn == 'q' )
        {
            break;
        }

        rcInput = scanf("%s", key);
        if(!strcmp(key, "q"))
        {
            g_keyReturn = 'q';
            fprintf(stderr, "\nquit!\n\n");
            break;
        } else if(rcInput == EOF) return;
        choice = strtoul(key, NULL, 0);
        switch(choice)
        {
        case 14: /* get video encoder status */
            printOutputStatus(hBxcode);
            break;
        case 99: /* debug module setting */
#if BDBG_DEBUG_BUILD
{
            int  iDbgLevel;
            char achName[256];
            printf("\nPlease enter the debug module name: ");
            scanf("%s", achName);
            printf("(%d)Trace (%d)Message (%d)Warning (%d)Error\n",
                BDBG_eTrace, BDBG_eMsg, BDBG_eWrn, BDBG_eErr);
            printf("Which debug level do you want to set it to? ");
            scanf("%d", &iDbgLevel);
            BDBG_SetModuleLevel(achName, (BDBG_Level)iDbgLevel);
}
#endif
            break;
        default:
            break;
        }
    }
    if(g_testContext.doneEvent) {
        BKNI_SetEvent(g_testContext.doneEvent);
    }
}

int main(int argc, char **argv)  {
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings;
    BXCode_Handle hBxcode;
    BXCode_StartSettings startSettings;
    BXCode_Settings settings;
    NEXUS_TransportTimestampType transportTypeIn = NEXUS_TransportType_eTs;
    BXCode_OutputType outputType = BXCode_OutputType_eTs;
    unsigned outVpid = 0x12, outApid = 0x13;
    B_ThreadHandle inputFeedHandle, outputRecordHandle;
    bool segmented = false;
    int i;

    for(i=0; i<argc; i++) {
        if(!strcmp("-if", argv[i])) {
            g_testContext.fileInput = true; fprintf(stderr, "File input type\n");
            g_testContext.inFile = argv[++i];
            if(i+2 < argc) {
                if(!strcmp("-type", argv[1+i])) {
                    i++;
                    transportTypeIn = lookup(g_transportTypeStrs, argv[++i]);
                    g_testContext.inVpid = 1;
                    g_testContext.inApid = 2;
                    fprintf(stderr, "Input transport type %s = %d\n", argv[i], transportTypeIn);
                }
            }
        }
        if(!strcmp("-of", argv[i])) {
            g_testContext.fileOutput = true; fprintf(stderr, "File output type\n");
            g_testContext.outFile = argv[++i];
            if(i+2 < argc) {
                if(!strcmp("-type", argv[1+i])) {
                    i++;
                    outputType = lookup(g_outputTypeStrs, argv[++i]);
                    outVpid = 1;
                    outApid = 2;
                    fprintf(stderr, "Output transport type %s = %d\n", argv[i], outputType);
                }
            }
        }
        if(!strcmp("-loop", argv[i])) {
            g_testContext.loop = true; fprintf(stderr, "Loop input\n");
        }
        if(!strcmp("-segmented", argv[i])) {
            segmented = true; fprintf(stderr, "Segmented TS output\n");
        }
        if(!strcmp("-is", argv[i])) {
            g_testContext.inFile = argv[++i]; fprintf(stderr, "Stream input type\n");
        }
        if(!strcmp("-os", argv[i])) {
            g_testContext.outFile = argv[++i]; fprintf(stderr, "Stream output type\n");
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
        if(!strcmp("-nrt", argv[i])) {
            g_testContext.nonRealTime = true; fprintf(stderr, "NRT mode transcode\n");
        }
    }
    fprintf(stderr, "Input file: %s\n", g_testContext.inFile);
    fprintf(stderr, "Output file: %s\n", g_testContext.outFile);
    B_Os_Init();
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    BKNI_CreateEvent(&g_testContext.doneEvent);
    BKNI_CreateEvent(&g_testContext.inputDoneEvent);
    BKNI_CreateEvent(&g_testContext.outputDoneEvent);
    BKNI_CreateEvent(&g_testContext.inputReadyEvent);
    BKNI_CreateEvent(&g_testContext.outputReadyEvent);

    hBxcode = BXCode_Open(0, NULL);
    BXCode_GetSettings(hBxcode, &settings);
    settings.video.width  = g_testContext.width;
    settings.video.height = g_testContext.height;
    settings.video.encoder.bitrateMax = g_testContext.vBitrate;
    BXCode_SetSettings(hBxcode, &settings);
    BXCode_GetDefaultStartSettings(&startSettings);
    startSettings.nonRealTime = g_testContext.nonRealTime;
    startSettings.input.type = g_testContext.fileInput? BXCode_InputType_eFile : BXCode_InputType_eStream;
    if(g_testContext.fileInput) {
        startSettings.input.data = g_testContext.inFile;
    }
    startSettings.input.transportType  = transportTypeIn;
    startSettings.input.vPid = g_testContext.inVpid;
    startSettings.input.vCodec = g_testContext.inVcodec;
    startSettings.input.aPid[0] = g_testContext.inApid;
    startSettings.input.aCodec[0] = g_testContext.inAcodec;
    startSettings.input.dataCallback.callback = inputDataReadyCallback;
    startSettings.input.dataCallback.context  = g_testContext.inputReadyEvent;
    startSettings.input.eofDone.callback = doneHandler;
    startSettings.input.eofDone.context  = g_testContext.doneEvent;

    startSettings.output.video.pid = outVpid;
    startSettings.output.audio[0].pid = outApid;
    if(g_testContext.fileOutput) {
        startSettings.output.transport.file = g_testContext.outFile;
        startSettings.output.transport.tmpDir = "videos"; /* for mp4 output */
    }
    startSettings.output.transport.type = outputType;
    startSettings.output.transport.pcrPid = 0x11;
    startSettings.output.transport.pmtPid = 0x10;
    startSettings.output.transport.segmented = segmented;
    startSettings.output.transport.dataCallback.callback = outputDataReadyCallback;
    startSettings.output.transport.dataCallback.context  = g_testContext.outputReadyEvent;
    BXCode_Start(hBxcode, &startSettings);

    if(!g_testContext.fileInput) { /* strema input feed thread */
        /* create input feed thread */
        inputFeedHandle = B_Thread_Create("Input feed thread", (B_ThreadFunc)inputFeedThread, hBxcode, NULL);
    }

    if(!g_testContext.fileOutput) { /* strema input feed thread */
        /* create output record thread */
        outputRecordHandle = B_Thread_Create("Output record thread", (B_ThreadFunc)outputRecordThread, hBxcode, NULL);
    }

    /****************************************************
     *                       key handler                                                 *
     *****************************************************/
    /* wait for key handler to exit the test */
    {
        B_ThreadHandle keyHandle;

        /* Turn off buffering for stdin */
        setvbuf(stdin, NULL, _IONBF, 0);

        keyHandle = B_Thread_Create("key handler", (B_ThreadFunc)keyHandler, hBxcode, NULL);
        while(BKNI_WaitForEvent(g_testContext.doneEvent, BKNI_INFINITE)!=BERR_SUCCESS);
        B_Thread_Destroy(keyHandle);
    }
    /* wait for strema i/o threads to return */
    while(!g_testContext.fileInput && (BKNI_WaitForEvent(g_testContext.inputDoneEvent, BKNI_INFINITE)!=BERR_SUCCESS));
    while(!g_testContext.fileOutput && (BKNI_WaitForEvent(g_testContext.outputDoneEvent, BKNI_INFINITE)!=BERR_SUCCESS));

    printOutputStatus(hBxcode);
    BXCode_Stop(hBxcode);
    if(!g_testContext.fileOutput) B_Thread_Destroy(outputRecordHandle);
    if(!g_testContext.fileInput) B_Thread_Destroy(inputFeedHandle);
    BXCode_Close(hBxcode);
    BKNI_DestroyEvent(g_testContext.inputReadyEvent);
    BKNI_DestroyEvent(g_testContext.outputReadyEvent);
    BKNI_DestroyEvent(g_testContext.doneEvent);
    BKNI_DestroyEvent(g_testContext.inputDoneEvent);
    BKNI_DestroyEvent(g_testContext.outputDoneEvent);
    NEXUS_Platform_Uninit();
    B_Os_Uninit();

    return 0;
}
#else
int main(void)  {
    printf("This platform doesn't support transcode!\n");
    return 0;
}
#endif /* NEXUS_HAS_VIDEO_ENCODER */
