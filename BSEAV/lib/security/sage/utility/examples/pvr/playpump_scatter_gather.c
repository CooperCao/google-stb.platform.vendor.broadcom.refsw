/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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
#include "nexus_video_decoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playpump.h"
#include "nexus_security.h"
#include "nexus_dma.h"
#include "nexus_message.h"

#include <stdio.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

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

/* Set SAVE_ON_INJECT to 1 to Save data into a file right before injecting data to playpump (after all cryptographic operations are done, i.e. data in clear)
 * Note: this is only possible if USE_SECURE_HEAP is set to 0 */
#define SAVE_ON_INJECT 0
static const char inject_fname[] = "videos/play0.mpg"; /* Path of the file use to save injected data, if SAVE_ON_INJECT is set to 1 */

/* Set LOG_BUFFERS_INFO to 1 in order to log buffers virtual addresses and length during playback */
#define LOG_BUFFERS_INFO 1

/* the following define the input file and its characteristics -- these will vary by input file
 * Note: this should be in sync with the values setup in recpump_with_wrap.c */
static const char record_fname[] = "videos/stream.mpg";

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
#endif

/* **************************** */


#if LOG_BUFFERS_INFO
#define dbg_buffers(...) printf(__VA_ARGS__);
#else
#define dbg_buffers(...)
#endif


static void callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

#define CHUNK_SIZE        (188 * 128)
#define CHUNK_COUNT       (128)


static NEXUS_DmaJobHandle dmaJob;


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

int main(void)
{
    FILE *file;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    BKNI_EventHandle event;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle filterPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_Error rc;
    const char record_fname[] = "videos/stream.mpg";
    void *buf[CHUNK_COUNT];
    size_t buf_size = CHUNK_SIZE;
    unsigned cur_buf;
    unsigned i,j;
    NEXUS_DmaHandle dma;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobBlockSettings blockSettings[1];
    void *pMem = NULL;
    BKNI_EventHandle dmaEvent;
    NEXUS_KeySlotHandle decKeyHandle;
    NEXUS_KeySlotHandle decCpHandle;
    NEXUS_PidChannelStatus pidStatus;
#if SAVE_ON_INJECT
    FILE *saveFile;
#endif
    NEXUS_MessageHandle msg;
    NEXUS_MessageSettings msgSettings;
    NEXUS_MessageStartSettings msgStartSettings;
    BKNI_EventHandle msgEvent;
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
    BERR_Code berr;
    int err;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

#ifdef NEXUS_EXPORT_HEAP
    /* Configure export heap since it's not allocated by nexus by default */
    platformSettings.heap[NEXUS_EXPORT_HEAP].size = 32*1024*1024;
#endif

    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
    /* Request secure picture buffers, i.e. URR
    * Only needed if SAGE is in use, and when SAGE_SECURE_MODE is NOT 1 */
    /* For now default to SVP2.0 type configuration (i.e. ALL buffers are
    * secure ONLY */
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++) {
        memConfigSettings.videoDecoder[i].secure = NEXUS_SecureVideo_eSecure;
    }
    for (i=0;i<NEXUS_NUM_DISPLAYS;i++) {
        for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++) {
            memConfigSettings.display[i].window[j].secure = NEXUS_SecureVideo_eSecure;
        }
    }
    rc = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    berr = BKNI_CreateEvent(&event);
    BDBG_ASSERT(berr == BERR_SUCCESS && event);

    err = prepare_keyslots(1/* playback application */, &decKeyHandle, NULL, NULL, NULL);
    BDBG_ASSERT(err == 0);

    dma = NEXUS_Dma_Open(0, NULL);
    BDBG_ASSERT(dma);

    /* This will be allocated in restricted memory since, when the encryption is removed it will need to be there */
    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
#if USE_SECURE_HEAP
    memSettings.heap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
#endif
    rc = NEXUS_Memory_Allocate(CHUNK_COUNT * CHUNK_SIZE, &memSettings, &pMem);
    BDBG_ASSERT(rc == NEXUS_SUCCESS && pMem);
    berr = BKNI_CreateEvent(&dmaEvent);
    BDBG_ASSERT(berr == BERR_SUCCESS && dmaEvent);

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.dataFormat = NEXUS_DmaDataFormat_eBlock;
    jobSettings.numBlocks = 1;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context = dmaEvent;
    jobSettings.completionCallback.param = 0;
    jobSettings.keySlot = decKeyHandle;

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

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpumpOpenSettings.fifoSize = CHUNK_COUNT;
#if USE_SECURE_HEAP
    playpumpOpenSettings.heap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
    playpumpOpenSettings.dataNotCpuAccessible = true;
#endif
    playpump = NEXUS_Playpump_Open(0, &playpumpOpenSettings);
    BDBG_ASSERT(playpump);

    /* use stdio for file I/O to keep the example simple. */
    file = fopen(record_fname, "rb");
    BDBG_ASSERT(file);

#if SAVE_ON_INJECT
    saveFile = fopen(inject_fname, "wb");
    BDBG_ASSERT(saveFile);
#endif

    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = callback;
    playpumpSettings.dataCallback.context = event;
    /* setting mode = NEXUS_PlaypumpMode_eScatterGather is deprecated */
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);

    NEXUS_Playpump_Start(playpump);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, VIDEO_PID, NULL);
#if USE_SECURE_HEAP
    NEXUS_SetPidChannelBypassKeyslot(videoPidChannel, NEXUS_BypassKeySlot_eGR2R);
#endif

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    /* Bring up video decoder and display. No audio to keep the example simple. */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));

    NEXUS_Display_GetSettings(display, &displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i50hz;
    NEXUS_Display_SetSettings(display, &displaySettings);

    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
#if USE_SECURE_HEAP
    videoDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
#endif
    videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
    BDBG_ASSERT(videoDecoder);

    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    audioPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, AUDIO_PID, NULL);
#if USE_SECURE_HEAP
    NEXUS_SetPidChannelBypassKeyslot(audioPidChannel, NEXUS_BypassKeySlot_eGR2R);
#endif

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
#if USE_SECURE_HEAP
    audioDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
#endif
    audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
    BDBG_ASSERT(audioDecoder);

#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                               NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    /* buffers must be from the nexus heap to be used by playpump; therefore use NEXUS_Memory_Allocate */
    for (i=0;i<CHUNK_COUNT;i++) {
        rc = NEXUS_Memory_Allocate(buf_size, NULL, &buf[i]);
        BDBG_ASSERT(rc == NEXUS_SUCCESS && buf[i]);
    }

    berr = BKNI_CreateEvent(&msgEvent);
    BDBG_ASSERT(berr == BERR_SUCCESS && msgEvent);

    NEXUS_Message_GetDefaultSettings(&msgSettings);
    msgSettings.dataReady.callback = callback;
    msgSettings.dataReady.context = msgEvent;
    msg = NEXUS_Message_Open(&msgSettings);
    BDBG_ASSERT(msg);

    filterPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, PAT_PID, NULL);
    NEXUS_Message_GetDefaultStartSettings(msg, &msgStartSettings);
    msgStartSettings.pidChannel = filterPidChannel;
    rc = NEXUS_Message_Start(msg, &msgStartSettings);
    BDBG_ASSERT(rc == BERR_SUCCESS);

    for(cur_buf=0;;) {
        int n;
        NEXUS_Error rc;
        NEXUS_PlaypumpStatus status;

        rc = NEXUS_Playpump_GetStatus(playpump, &status);
        BDBG_ASSERT(!rc);
        /* printf("fifodepth = %d\n", status.descFifoDepth); */
        if(status.descFifoDepth == status.descFifoSize) {
            /* every buffer is in use */
            /* printf("Waiting for buffer\n"); */
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            /* printf("Wait complete\n"); */
            continue;
        }

        n = fread(buf[cur_buf], 1, buf_size, file);
        BDBG_ASSERT(n >= 0);
        if (n == 0) {
            /* wait for the decoder to reach the end of the content before looping */
            printf("looping\n");
            while (1) {
                NEXUS_VideoDecoderStatus status;
                NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
                if (!status.queueDepth) break;
            }
            fseek(file, 0, SEEK_SET);
            NEXUS_Playpump_Flush(playpump);
        }
        else {
            NEXUS_PlaypumpScatterGatherDescriptor desc;
            unsigned numConsumed;

            /* Use the DMA to decrypt the buffer */
            blockSettings[0].pSrcAddr = buf[cur_buf];
            blockSettings[0].pDestAddr = pMem + CHUNK_SIZE * cur_buf;
#if USE_SECURE_HEAP
            NEXUS_FlushCache(blockSettings[0].pSrcAddr, blockSettings[0].blockSize);
#endif

            dbg_buffers("[%d] src[0] = %p, dest[0] = %p, size[0] = %d\n",
                        cur_buf,
                        blockSettings[0].pSrcAddr,
                        blockSettings[0].pDestAddr,
                        blockSettings[0].blockSize);

            rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, blockSettings, 1);
            BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);

#if USE_SECURE_HEAP
            NEXUS_FlushCache(blockSettings[0].pSrcAddr, blockSettings[0].blockSize);
#endif

            desc.addr = pMem + CHUNK_SIZE * cur_buf;
            desc.length = n;

#if SAVE_ON_INJECT
            fwrite(desc.addr, 1, desc.length, saveFile);
#endif

            /* printf("Submit %p (%d)\n", desc.addr, desc.length); */
            rc = NEXUS_Playpump_SubmitScatterGatherDescriptor(playpump, &desc, 1, &numConsumed);
            BDBG_ASSERT(!rc);
            BDBG_ASSERT(numConsumed==1); /* we've already checked that there are descriptors available*/
            cur_buf = (cur_buf + 1) % CHUNK_COUNT; /* use the next buffer */
            /* printf("Submit done, cur_buf = %d\n", cur_buf); */

            if (BKNI_WaitForEvent(msgEvent, 1) != BERR_TIMEOUT) {
                const uint8_t *buffer;
                size_t size;
                int programNum, message_length;

                rc = NEXUS_Message_GetBuffer(msg, (const void **)&buffer, &size);
                if (size) {
                    static int loopcount = 0;
#define TS_READ_16( BUF ) ((uint16_t)((BUF)[0]<<8|(BUF)[1]))
#define TS_PSI_GET_SECTION_LENGTH( BUF )    (uint16_t)(TS_READ_16( &(BUF)[1] ) & 0x0FFF)

                    /* We should always get whole PAT's because maxContiguousMessageSize is 4K */
                    message_length = TS_PSI_GET_SECTION_LENGTH(buffer) + 3;
                    BDBG_ASSERT(size >= (size_t)message_length);

                    if (loopcount++ % 100 == 0)
                    {
                        printf("Found PAT: id=%d size=%d\n", buffer[0], message_length);
                        for (programNum=0;programNum<(TS_PSI_GET_SECTION_LENGTH(buffer)-7)/4;programNum++) {
                            unsigned byteOffset = 8 + programNum*4;
                            printf("  program %d: pid 0x%x\n",
                                   TS_READ_16( &buffer[byteOffset] ),
                                   (uint16_t)(TS_READ_16( &buffer[byteOffset+2] ) & 0x1FFF));
                        }
                    }

                    rc = NEXUS_Message_ReadComplete(msg, size);
                }
            }
        }
    }

    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_VideoDecoder_Stop(videoDecoder);

    /* cleanup */

    NEXUS_Message_Close(msg);
    BKNI_DestroyEvent(msgEvent);
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_VideoDecoder_Close(videoDecoder);
#if SAVE_ON_INJECT
    fclose(saveFile);
#endif
    fclose(file);
    NEXUS_Playpump_Close(playpump);
    NEXUS_DmaJob_Destroy(dmaJob);
    BKNI_DestroyEvent(dmaEvent);
    NEXUS_Memory_Free(pMem);
    NEXUS_Dma_Close(dma);
    clean_keyslots(decKeyHandle, NULL, NULL, NULL);
    BKNI_DestroyEvent(event);

    return 0;
}
