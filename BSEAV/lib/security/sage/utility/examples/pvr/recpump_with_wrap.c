 /******************************************************************************
 *    (c)2008-2011 Broadcom Corporation
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
 *****************************************************************************/

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_recpump.h"
#include "nexus_security.h"
#include "nexus_dma.h"
#include "nexus_playback.h"

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/time.h>

#if NEXUS_HAS_SAGE
#else
#error export SAGE_SUPPORT=y and recompile both nexus and the application!
#endif


/* **************************** */

/*
 * Application Configuration
 */

/* Set USE_SECURE_HEAP to configure the video pipe to take advantage of the CRR (for Secure Video Path) */
#define USE_SECURE_HEAP 1

/* Set LOG_BUFFERS_INFO to 1 in order to log buffers virtual addresses and length during playback */
#define LOG_BUFFERS_INFO 1

/* Set SAGE_PRE_M2M to 1 to Save data into a file right before scrambling it through M2M (audio/video PIDs in clear, other PIDs scrambled)
 * Note: this is only possible if USE_SECURE_HEAP is set to 0 */
#define SAGE_PRE_M2M 0
static const char pre_m2m_fname[] = "videos/rec_pre_m2m.mpg"; /* Path of the file use to save data prior to m2m operation, if SAGE_PRE_M2M is set to 1 */

/* the following define the input file and its characteristics -- these will vary by input file
 * Note: this should be in sync with the values setup in playpump_scatter_gather.c */
static const char record_fname[] = "videos/stream.mpg";

/* Set RECORD_FROM_PLAYBACK to 1 to record from a file on the file system
 * Set RECORD_FROM_PLAYBACK to 0 to record from an Inbound tuner */
#define RECORD_FROM_PLAYBACK 1
static const char source_fname[] = "videos/spiderman_aes.ts";


#if 0

/* Values for MHD.ts */
#define VIDEO_PID 2059
#define VIDEO_CODEC NEXUS_VideoCodec_eH264
#define AUDIO_PID 2034
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define SUBT_PID 2035
#define PAT_PID 0
#define PMT_PID 2062

#else

/* Values for spiderman_aes.ts */
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define AUDIO_PID 0x14

/* #define SUBT_PID ??? */
#define PAT_PID  0
/* #define PMT_PID  ??? */

#define CA_STREAM 1

#endif

/* **************************** */


#if LOG_BUFFERS_INFO
#define dbg_buffers(...) printf(__VA_ARGS__);
#else
#define dbg_buffers(...)
#endif


#define CHUNK_SIZE        (188 * 128)
#define CHUNK_COUNT       (128)


static NEXUS_DmaJobHandle dmaJob = NULL;


int prepare_keyslots(int operation, NEXUS_KeySlotHandle *pM2mKeySlotHandle, NEXUS_KeySlotHandle *pCpKeySlotHandle, NEXUS_KeySlotHandle *pVideoCaKeySlotHandle, NEXUS_KeySlotHandle *pAudioCaKeySlotHandle);
void clean_keyslots(NEXUS_KeySlotHandle m2mKeySlotHandle, NEXUS_KeySlotHandle cpKeySlotHandle, NEXUS_KeySlotHandle videoCaKeySlotHandle, NEXUS_KeySlotHandle audioCaKeySlotHandle);


static void CompleteCallback(void *pParam, int iParam)
{
    NEXUS_DmaJobStatus status;
    BSTD_UNUSED(iParam);

    NEXUS_DmaJob_GetStatus(dmaJob, &status);
    if (status.currentState == NEXUS_DmaJobState_eComplete) {
        BKNI_SetEvent(pParam);
    }
}

static void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void overflow_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    printf("overflow %s\n", (const char *)context);
}

static bool g_done = false;

#if RECORD_FROM_PLAYBACK
static void end_of_stream(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    g_done = true;
}
#endif

#if 0
static void print_status(NEXUS_RecpumpHandle recpump)
{
    NEXUS_RecpumpStatus status;
    NEXUS_Recpump_GetStatus(recpump, &status);
    printf("status: \n");
    printf("  data:  %u total bytes \tfifo %u/%u\n", (unsigned)status.data.bytesRecorded, status.data.fifoDepth, status.data.fifoSize);
    printf("  index: %u total bytes \tfifo %u/%u\n", (unsigned)status.index.bytesRecorded, status.index.fifoDepth, status.index.fifoSize);
}
#endif

enum {
    PID_INDEX_eVideo,
    PID_INDEX_eAudio,
    PID_INDEX_eSubtitle,
    PID_INDEX_ePAT,
    PID_INDEX_ePMT,
    PID_INDEX_eMax
};

int main(void)
{
    FILE *data_file;
#if SAGE_PRE_M2M
    FILE *saveFile = NULL;
#endif
    int err;
    BKNI_EventHandle event = NULL;
    NEXUS_RecpumpOpenSettings openSettings;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpHandle recpump = NULL;
    NEXUS_PidChannelHandle pidChannel[PID_INDEX_eMax] = {0};
    NEXUS_DmaHandle dma = NULL;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobBlockSettings blockSettings[1];
    NEXUS_RecpumpStatus status;
    uint8_t *pMem = NULL;
    NEXUS_Error rc;
    BKNI_EventHandle dmaEvent = NULL;
    unsigned i;
    unsigned offset;
    NEXUS_KeySlotHandle encKeyHandle = NULL;
    NEXUS_KeySlotHandle encCpHandle = NULL;
    NEXUS_KeySlotHandle decVideoCaHandle = NULL;
    NEXUS_KeySlotHandle decAudioCaHandle = NULL;
    NEXUS_PidChannelStatus pidStatus;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if RECORD_FROM_PLAYBACK
    NEXUS_PlaypumpHandle playpump = NULL;
    NEXUS_PlaybackHandle playback = NULL;
    NEXUS_FilePlayHandle playfile = NULL;
#else
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
#endif
    BERR_Code berr;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

#ifdef NEXUS_EXPORT_HEAP
    /* Configure export heap since it's not allocated by nexus by default */
    platformSettings.heap[NEXUS_EXPORT_HEAP].size = 32*1024*1024;
#endif

    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Memory_PrintHeaps();

#if RECORD_FROM_PLAYBACK
    {
        NEXUS_PlaybackSettings playbackSettings;

        playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
        BDBG_ASSERT(playpump);

        playback = NEXUS_Playback_Create();
        BDBG_ASSERT(playback);

        playfile = NEXUS_FilePlay_OpenPosix(source_fname, NULL);
        BDBG_ASSERT(playfile);

        NEXUS_Playback_GetSettings(playback, &playbackSettings);
        playbackSettings.playpump = playpump;
        playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
        playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
        playbackSettings.endOfStreamCallback.callback = end_of_stream;
        NEXUS_Playback_SetSettings(playback, &playbackSettings);

        /* Open Audio and Video PIDs */
        {
            NEXUS_PlaybackPidChannelSettings playbackPidSettings;

            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            pidChannel[0] = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);
        }
        pidChannel[1] = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, NULL);
    }
#else
    {
        NEXUS_ParserBandSettings parserBandSettings;
        NEXUS_InputBand inputBand;

        NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = inputBand;
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

        /* Open Audio and Video PIDs */
        pidChannel[PID_INDEX_eVideo] = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
        pidChannel[PID_INDEX_eAudio] = NEXUS_PidChannel_Open(parserBand, AUDIO_PID, NULL);
    }
#endif

    data_file = fopen(record_fname, "wb");
    BDBG_ASSERT(data_file);

#if SAGE_PRE_M2M
    saveFile = fopen(pre_m2m_fname, "wb");
    BDBG_ASSERT(saveFile);
#endif

    berr = BKNI_CreateEvent(&event);
    BDBG_ASSERT(berr == BERR_SUCCESS && event);

    err = prepare_keyslots(0/* NOT playback application */, &encKeyHandle, NULL, &decVideoCaHandle, &decAudioCaHandle);
    BDBG_ASSERT(err == 0);

    dma = NEXUS_Dma_Open(0, NULL);
    BDBG_ASSERT(dma);

    /* This will be allocated in global memory since the CDB/ITB are in restricted memory */
    {
        NEXUS_MemoryAllocationSettings allocateSettings;
        NEXUS_Error rc;

        NEXUS_Memory_GetDefaultAllocationSettings(&allocateSettings);
        allocateSettings.alignment = 1024;

        rc = NEXUS_Memory_Allocate(CHUNK_COUNT * CHUNK_SIZE, &allocateSettings, (void **)(&pMem));
        BDBG_ASSERT(rc == NEXUS_SUCCESS && pMem);

        dbg_buffers("buf %p, size = %d\n", pMem, CHUNK_COUNT * CHUNK_SIZE);
    }

    berr = BKNI_CreateEvent(&dmaEvent);
    BDBG_ASSERT(berr == BERR_SUCCESS && dmaEvent);

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.dataFormat = NEXUS_DmaDataFormat_eBlock;
    jobSettings.numBlocks = 1;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context = dmaEvent;
    jobSettings.completionCallback.param = 0;
    jobSettings.keySlot = encKeyHandle;

    dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);
    BDBG_ASSERT(dmaJob);

    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings[0]);
    blockSettings[0].blockSize = CHUNK_SIZE;
    blockSettings[0].scatterGatherCryptoStart = true;
    blockSettings[0].scatterGatherCryptoEnd = true;
    blockSettings[0].resetCrypto = false;
#if USE_SECURE_HEAP
    blockSettings[0].cached = false;
#else
    blockSettings[0].cached = true;
#endif

    NEXUS_Recpump_GetDefaultOpenSettings(&openSettings);
    openSettings.data.alignment = 12;
    openSettings.data.atomSize = CHUNK_SIZE;
    openSettings.data.bufferSize = (CHUNK_COUNT * CHUNK_SIZE);
    openSettings.data.dataReadyThreshold = CHUNK_SIZE;

#ifdef NEXUS_EXPORT_HEAP
    openSettings.data.heap = platformConfig.heap[NEXUS_EXPORT_HEAP];
    openSettings.useSecureHeap = false;
#endif

#if RECORD_FROM_PLAYBACK
    /* set threshold to 80%. with band hold enabled, it's not actually a dataready threshold. it's
       a bandhold threshold. we are relying on the timer that's already in record. */
    openSettings.data.dataReadyThreshold = openSettings.data.bufferSize * 5 / 10;
    openSettings.index.dataReadyThreshold = openSettings.index.bufferSize * 5 / 10;
#endif
    recpump = NEXUS_Recpump_Open(0, &openSettings);
    BDBG_ASSERT(recpump);

    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.dataReady.context = event;
    recpumpSettings.index.dataReady.callback = dataready_callback; /* same callback */
    recpumpSettings.index.dataReady.context = event; /* same event */
    recpumpSettings.data.overflow.callback = overflow_callback;
    recpumpSettings.data.overflow.context = "data";
    recpumpSettings.index.overflow.callback = overflow_callback;
    recpumpSettings.index.overflow.context = "index";
#if RECORD_FROM_PLAYBACK
    /* enable bandhold. required for record from playback. */
    recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
#endif
    NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);

    /* Open Audio and Video PIDs */
    {
        NEXUS_RecpumpAddPidChannelSettings pidSettings;
        NEXUS_Recpump_GetDefaultAddPidChannelSettings(&pidSettings);
        pidSettings.pidType = NEXUS_PidType_eVideo;
        pidSettings.pidTypeSettings.video.index = true;
        pidSettings.pidTypeSettings.video.codec = VIDEO_CODEC;
        NEXUS_Recpump_AddPidChannel(recpump, pidChannel[PID_INDEX_eVideo], &pidSettings);
    }
    NEXUS_Recpump_AddPidChannel(recpump, pidChannel[PID_INDEX_eAudio], NULL);

#if CA_STREAM
    /* In case it is a CA-encrypted stream, attach Video and Audio PIDs to CA keyslots */
    NEXUS_PidChannel_GetStatus(pidChannel[PID_INDEX_eVideo], &pidStatus);
    NEXUS_Security_AddPidChannelToKeySlot(decVideoCaHandle, pidStatus.pidChannelIndex);
    NEXUS_PidChannel_GetStatus(pidChannel[PID_INDEX_eAudio], &pidStatus);
    NEXUS_Security_AddPidChannelToKeySlot(decAudioCaHandle, pidStatus.pidChannelIndex);
#endif

    NEXUS_Recpump_Start(recpump);

    /* To retreive buffer base */
    NEXUS_Recpump_GetStatus(recpump, &status);

#if RECORD_FROM_PLAYBACK
    printf("press ENTER to start recording\n");
    getchar();
    NEXUS_Playback_Start(playback, playfile, NULL);
#else
    printf("You can now start streaming\n");
#endif

    while (!g_done) {
        const void *data_buffer[2], *index_buffer[2];
        size_t data_buffer_size[2], index_buffer_size[2];
        size_t data_buffer_write[2];
        ssize_t n;
        ssize_t n2 = 0;
        void *write_buffer[2];

        if (NEXUS_Recpump_GetDataBufferWithWrap(recpump, &data_buffer[0], &data_buffer_size[0], &data_buffer[1], &data_buffer_size[1]))
            break;
        if (NEXUS_Recpump_GetIndexBufferWithWrap(recpump, &index_buffer[0], &index_buffer_size[0], &index_buffer[1], &index_buffer_size[1]))
            break;
        if (data_buffer_size[0] == 0 && index_buffer_size[0] == 0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }

        if (data_buffer_size[0] + data_buffer_size[1] < CHUNK_SIZE)
            continue;

        if (data_buffer_size[0]) {

            data_buffer_write[0] = data_buffer_size[0] / CHUNK_SIZE * CHUNK_SIZE;
            data_buffer_write[1] = data_buffer_size[1] / CHUNK_SIZE * CHUNK_SIZE;

            offset = data_buffer[0] - status.data.bufferBase;
            for (i = 0; i < data_buffer_write[0]; i += CHUNK_SIZE) {
                blockSettings[0].pSrcAddr = data_buffer[0] + i;
                blockSettings[0].pDestAddr = pMem + offset + i;

                dbg_buffers("buf[0] : src[0] = %p, dest[0] = %p, size[0] = %d\n",
                            blockSettings[0].pSrcAddr,
                            blockSettings[0].pDestAddr,
                            blockSettings[0].blockSize);

#if USE_SECURE_HEAP
                NEXUS_FlushCache(blockSettings[0].pDestAddr, blockSettings[0].blockSize);
#endif

                rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, blockSettings, 1);
                BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
#if USE_SECURE_HEAP
                NEXUS_FlushCache(blockSettings[0].pDestAddr, blockSettings[0].blockSize);
#endif
            }
            write_buffer[0] = pMem + offset;

            if (data_buffer_size[1]) {
                offset = data_buffer[1] - status.data.bufferBase;
                for (i = 0; i < data_buffer_size[1]; i += CHUNK_SIZE) {

                    blockSettings[0].pSrcAddr = data_buffer[1] + i;
                    blockSettings[0].pDestAddr = pMem + offset + i;

                    dbg_buffers("buf[0] : src[0] = %p, dest[0] = %p, size[0] = %d\n",
                                blockSettings[0].pSrcAddr,
                                blockSettings[0].pDestAddr,
                                blockSettings[0].blockSize);

#if USE_SECURE_HEAP
                    NEXUS_FlushCache(blockSettings[0].pDestAddr, blockSettings[0].blockSize);
#endif
                    rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, blockSettings, 1);
                    BDBG_ASSERT(!rc || rc == NEXUS_DMA_QUEUED);
                    rc = BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
                    BDBG_ASSERT(!rc);
#if USE_SECURE_HEAP
                    NEXUS_FlushCache(blockSettings[0].pDestAddr, blockSettings[0].blockSize);
#endif
                }
                write_buffer[1] = pMem + offset;
            }
            else {
                write_buffer[1] = 0;
            }
#if SAGE_PRE_M2M
            fwrite(data_buffer[0], 1, data_buffer_write[0], saveFile);
#endif
            n = fwrite(write_buffer[0], 1, data_buffer_write[0], data_file);
            BDBG_ASSERT(n >= 0 && (unsigned)n == data_buffer_write[0]);

            if (data_buffer_write[1]) {
#if SAGE_PRE_M2M
                fwrite(data_buffer[1], 1, data_buffer_write[1], saveFile);
#endif
                n2 = fwrite(write_buffer[1], 1, data_buffer_write[1], data_file);
                BDBG_ASSERT(n2 >= 0 && (unsigned)n2 == data_buffer_write[1]);
            }
            NEXUS_Recpump_DataReadComplete(recpump, n+n2);
        }
        if (index_buffer_size[0]) {
            /* Don't care about the index data */
            NEXUS_Recpump_IndexReadComplete(recpump, index_buffer_size[0] + index_buffer_size[1]);
        }
#if 0
        print_status(recpump);
#endif
    }

    printf("done\n");

    /* Cleanup */

    NEXUS_Recpump_Close(recpump);
    NEXUS_DmaJob_Destroy(dmaJob);
    BKNI_DestroyEvent(dmaEvent);
    NEXUS_Memory_Free(pMem);
    NEXUS_Dma_Close(dma);
    clean_keyslots(encKeyHandle, NULL, decVideoCaHandle, decAudioCaHandle);
    BKNI_DestroyEvent(event);
#if SAGE_PRE_M2M
    fclose(saveFile);
#endif
    fclose(data_file);
#if RECORD_FROM_PLAYBACK
    NEXUS_FilePlay_Close(playfile);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
#endif
    return 1;
}
