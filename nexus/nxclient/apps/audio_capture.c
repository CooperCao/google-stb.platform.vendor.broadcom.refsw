/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *****************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "nexus_audio_capture.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_stc_channel.h"
#include "nexus_playpump.h"

#include <string.h>
#include <time.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "wav_file.h"

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/bugs_toys2_jurassic_q64_cd.mpg"
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define AUDIO_PID 0x14
#define AUDIO_PID2 0x24

BDBG_MODULE(audio_capture);

typedef struct capture_handles
{
    NEXUS_AudioCaptureHandle capture;
    FILE* capFile;
} capture_handles;

static void print_usage(void)
{
    printf(
        "Usage: audio_capture [FILE]\n"
        "  -format [stereo|stereo24|multichannel|compressed|compressed4x] sets capture format. Default is stereo"
        "  File names ending in .wav will be encapsulated as WAV file."
        );
}

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void capture_callback(void *pParam, int param)
{
    capture_handles * capHandles = (capture_handles*)pParam;
    NEXUS_AudioCaptureHandle capture;
    FILE *pFile;
    NEXUS_Error errCode;

    BDBG_ASSERT(capHandles);
    capture = capHandles->capture;
    BDBG_ASSERT(capture);
    pFile = (FILE*) capHandles->capFile;

    for ( ;; )
    {
        void *pBuffer;
        size_t bufferSize;

        /* Check available buffer space */
        errCode = NEXUS_AudioCapture_GetBuffer(capture, (void **)&pBuffer, &bufferSize);
        if ( errCode )
        {
            fprintf(stderr, "Error getting capture buffer\n");
            NEXUS_AudioCapture_Stop(capture);
            return;
        }

        if ( bufferSize > 0 )
        {
            printf("Capture received %lu bytes.", (unsigned long)bufferSize);
            if ( pFile )
            {
                /* Write samples to disk */
                printf("  writing %lu bytes to file.", (unsigned long)bufferSize);
                if ( 1 != fwrite(pBuffer, bufferSize, 1, pFile) )
                {
                    fprintf(stderr, "Error writing to disk\n");
                    NEXUS_AudioCapture_Stop(capture);
                    return;
                }
            }
            printf("\n");

            /*fprintf(stderr, "Data callback - Wrote %d bytes\n", (int)bufferSize);*/
            errCode = NEXUS_AudioCapture_ReadComplete(capture, bufferSize);
            if ( errCode )
            {
                fprintf(stderr, "Error committing capture buffer\n");
                NEXUS_AudioCapture_Stop(capture);
                return;
            }
        }
        else
        {
            break;
        }
    }
}

int main(int argc, char **argv)
{
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    FILE *file = NULL;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    BKNI_EventHandle event;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_PidChannelHandle audioPidChannel, audioPidChannel2;
    NEXUS_SimpleAudioDecoderHandle audioDecoder, audioDecoder2;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram, audioProgram2;
    NEXUS_SimpleAudioDecoderSettings audioSettings, audioSettings2;
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_AudioDecoderStatus audioStatus;
    NEXUS_Error rc;

    const char *fname = FILE_NAME;
#define TOTAL_BUFFERS 10
    void *buf[TOTAL_BUFFERS];
    size_t buf_size = 128*1024;
    unsigned cur_buf;
    unsigned i;
    uint32_t lastPts;

    NEXUS_AudioCaptureOpenSettings openSettings;
    NEXUS_AudioCaptureStartSettings startSettings;
    NxClient_AudioCaptureType captureType = NxClient_AudioCaptureType_e16BitStereo;
    NEXUS_AudioConnectorType connectorType = NEXUS_AudioConnectorType_eStereo;
    NEXUS_AudioMultichannelFormat multichFormat = NEXUS_AudioMultichannelFormat_eStereo;
    int curarg = 1;
    const char *filename = NULL;
    capture_handles capHandles = {NULL,NULL};
    bool wavFile = false;
    bool standalone = false;
    unsigned captureId;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-standalone")) {
            standalone = true;
        }
        else if (!strcmp(argv[curarg], "-format") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("stereo", argv[curarg])) {
                captureType = NxClient_AudioCaptureType_e16BitStereo;
                connectorType = NEXUS_AudioConnectorType_eStereo;
                multichFormat = NEXUS_AudioMultichannelFormat_eStereo;
            }
            else if (!strcmp("stereo24", argv[curarg])) {
                captureType = NxClient_AudioCaptureType_e24BitStereo;
                connectorType = NEXUS_AudioConnectorType_eStereo;
                multichFormat = NEXUS_AudioMultichannelFormat_eStereo;
            }
            else if (!strcmp("multichannel", argv[curarg])) {
                captureType = NxClient_AudioCaptureType_e24Bit5_1;
                connectorType = NEXUS_AudioConnectorType_eMultichannel;
                multichFormat = NEXUS_AudioMultichannelFormat_e5_1;
            }
            else if (!strcmp("compressed", argv[curarg])) {
                captureType = NxClient_AudioCaptureType_eCompressed;
                connectorType = NEXUS_AudioConnectorType_eCompressed;
                multichFormat = NEXUS_AudioMultichannelFormat_eStereo;
            }
            else if (!strcmp("compressed4x", argv[curarg])) {
                captureType = NxClient_AudioCaptureType_eCompressed4x;
                connectorType = NEXUS_AudioConnectorType_eCompressed4x;
                multichFormat = NEXUS_AudioMultichannelFormat_eStereo;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!filename) {
            filename = argv[curarg];
            if ( strlen(filename) > 4 )
            {
                if ( strstr(filename, ".wav") == filename + (strlen(filename) - 4) )
                {
                    /* Filename ends in .wav, write as .wav file */
                    wavFile = true;
                }
            }
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (filename) {
        BDBG_ERR(("Open wav file %s", filename));
        capHandles.capFile = fopen(filename, "w+");
        if (!capHandles.capFile) {
            fprintf(stderr, "### unable to open %s\n", filename);
            goto err_file;
        }
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    if ( standalone ) {
        BKNI_CreateEvent(&event);

        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
        playpumpOpenSettings.fifoSize = 0;
        /* allocate the last playpump to stay away from primary display resources */
        playpump = NEXUS_Playpump_Open(NEXUS_NUM_PLAYPUMPS-1, &playpumpOpenSettings);

        /* use stdio for file I/O to keep the example simple. */
        file = fopen(fname, "rb");
        if (!file) {
            fprintf(stderr, "can't open file:%s\n", fname);
            goto err_file;
        }

        NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
        playpumpSettings.dataCallback.callback = play_callback;
        playpumpSettings.dataCallback.context = event;
        playpumpSettings.dataNotCpuAccessible = true;
        /* setting mode = NEXUS_PlaypumpMode_eScatterGather is deprecated */
        NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);

        NEXUS_Playpump_Start(playpump);

        stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

        audioPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, AUDIO_PID, NULL);

        NxClient_GetDefaultAllocSettings(&allocSettings);
        allocSettings.simpleAudioDecoder = 1;
        rc = NxClient_Alloc(&allocSettings, &allocResults);
        if (rc) {BDBG_WRN(("unable to alloc decode resources")); return -1;}

        NxClient_GetDefaultConnectSettings(&connectSettings);
        connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
        connectSettings.simpleAudioDecoder.decoderCapabilities.type = NxClient_AudioDecoderType_eStandalone;
        rc = NxClient_Connect(&connectSettings, &connectId);
        if (rc) {BDBG_WRN(("unable to connect decode resources")); return -1;}

        audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
        if ( !audioDecoder ) {
            BDBG_ERR(("Unable to Acquire Audio Decoder 0"));
            return -1;
        }

        captureId = NEXUS_NUM_AUDIO_CAPTURES-1;
    }
    else {
        NxClient_GetDefaultAllocSettings(&allocSettings);
        allocSettings.audioCapture = 1;
        allocSettings.audioCaptureType.type = captureType;
        rc = NxClient_Alloc(&allocSettings, &allocResults);
        if (rc) {rc = BERR_TRACE(rc); goto err_request;}

        captureId = allocResults.audioCapture.id;
    }
    {
        NEXUS_AudioCapture_GetDefaultOpenSettings(&openSettings);
        openSettings.multichannelFormat = multichFormat;
        BDBG_ERR(("Open Capture"));
        /* allocate our capture at the end of the array, so we don't try to use the same resources as NxServer */
        capHandles.capture = NEXUS_AudioCapture_Open(captureId, &openSettings);
        if (!capHandles.capture) {
            BDBG_ERR(("  unable to open capture"));
            goto err_acquire;
        }

        NEXUS_AudioCapture_GetDefaultStartSettings(&startSettings);
        startSettings.dataCallback.callback = capture_callback;
        startSettings.dataCallback.context = &capHandles;
        BDBG_ERR(("Start Capture"));
        rc = NEXUS_AudioCapture_Start(capHandles.capture, &startSettings);
        if (rc) {rc = BERR_TRACE(rc);goto err_start;}

        if (wavFile) {
            /* write a WAV header */
            NEXUS_AudioCaptureStatus status;
            struct wave_header wave_header;

            BDBG_ERR(("Write WAV header"));

            get_default_wave_header(&wave_header);
            wave_header.riffCSize = 0xfffffff0;
            wave_header.dataLen = 0xffffffcc;
            switch (captureType) {
            case NxClient_AudioCaptureType_e16BitStereo:
                wave_header.bps = 16;
                wave_header.channels = 2;
                break;
            case NxClient_AudioCaptureType_e24BitStereo:
                wave_header.bps = 32;  /* We are actually capturing the data as 32 bit */
                wave_header.channels = 2;
                break;
            case NxClient_AudioCaptureType_e24Bit5_1:
                wave_header.bps = 32;  /* We are actually capturing the data as 32 bit */
                wave_header.channels = 6;
                break;
            default:
                BDBG_ERR(("Unsupported capture format(%d) for WAV file", captureType));
                goto err_start;
                break;
            }

            BDBG_WRN(("Capture format %u, bits per sample %u, num chs %u", captureType, wave_header.bps, wave_header.channels));
            rc = NEXUS_AudioCapture_GetStatus(capHandles.capture, &status);
            BDBG_ASSERT(!rc);
            if ( standalone ) {
                wave_header.samplesSec = 48000;
            }
            else {
                wave_header.samplesSec = status.sampleRate;
            }
            wave_header.bytesSec = wave_header.samplesSec * wave_header.channels * wave_header.bps / 8;
            wave_header.chbits = wave_header.channels * wave_header.bps / 8;
            write_wave_header(capHandles.capFile, &wave_header);
        }
    }

    if ( standalone ) {
        BDBG_ERR(("Add Capture as output to Standalone Simple Audio Decoder"));
        NEXUS_SimpleAudioDecoder_AddOutput(audioDecoder, NEXUS_AudioCapture_GetConnector(capHandles.capture), NEXUS_SimpleAudioDecoderSelector_ePrimary, connectorType);

        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);

        NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.primary.codec = AUDIO_CODEC;
        audioProgram.primary.pidChannel = audioPidChannel;
        BDBG_ERR(("Start Audio Decoder"));
        NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);

        /* buffers must be from the nexus heap to be used by playpump; therefore use NEXUS_Memory_Allocate */
        NEXUS_Platform_GetClientConfiguration(&clientConfig);
        NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
        memSettings.heap = clientConfig.heap[1]; /* heap 1 is the eFull heap for the nxlient. heap 0 is the graphics heap */

        for (i=0;i<TOTAL_BUFFERS;i++) {
            NEXUS_Memory_Allocate(buf_size, &memSettings, &buf[i]);
        }

        for(cur_buf=0;;) {
            int n;
            NEXUS_Error rc;
            NEXUS_PlaypumpStatus status;

            rc = NEXUS_Playpump_GetStatus(playpump, &status);
            BDBG_ASSERT(!rc);
            if(status.descFifoDepth == TOTAL_BUFFERS) {
                /* every buffer is in use */
                BKNI_WaitForEvent(event, 1000);
                continue;
            }

            n = fread(buf[cur_buf], 1, buf_size, file);
            if (n < 0) goto error;
            if (n == 0) {
                /* wait for the decoder to reach the end of the content before looping */
                while (1) {
                    NEXUS_AudioDecoderStatus status;
                    NEXUS_SimpleAudioDecoder_GetStatus(audioDecoder, &status);
                    if (!status.queuedFrames) break;
                }
                fseek(file, 0, SEEK_SET);
                NEXUS_Playpump_Flush(playpump);
            }
            else {
                NEXUS_PlaypumpScatterGatherDescriptor desc;
                size_t numConsumed;
                desc.addr = buf[cur_buf];
                desc.length = n;
                #if 1
                if (playpumpSettings.dataNotCpuAccessible) {
                    NEXUS_FlushCache(desc.addr, desc.length);
                }
                #endif
                rc = NEXUS_Playpump_SubmitScatterGatherDescriptor(playpump, &desc, 1, &numConsumed);
                BDBG_ASSERT(!rc);
                BDBG_ASSERT(numConsumed==1); /* we've already checked that there are descriptors available*/
                cur_buf = (cur_buf + 1)%TOTAL_BUFFERS; /* use the next buffer */
            }
        }
    }
    else {
        while (1) {
            BKNI_Sleep(1000);
            continue;
        }
    }

    NEXUS_AudioCapture_Stop(capHandles.capture);
    NEXUS_AudioCapture_Close(capHandles.capture);
    if ( file ) {
        fclose(file);
    }
    if ( capHandles.capFile ) {
        fclose(capHandles.capFile);
    }

    return 0;

error:
err_start:
    NEXUS_AudioCapture_Close(capHandles.capture);
err_acquire:
    NxClient_Free(&allocResults);
err_request:
    NxClient_Uninit();
err_file:
    if ( file ) {
        fclose(file);
    }
    if ( capHandles.capFile ) {
        fclose(capHandles.capFile);
    }

    return 1;
}
#else
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
