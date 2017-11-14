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
 *
 * Module Description:
 *
 *****************************************************************************/
/* Nexus example app: two PES files (one video, one audio) using two playpumps with lipsync */
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
#include "nexus_audio_output.h"
#include "nexus_playpump.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_parser_band.h"
#include "nexus_pid_channel.h"
#include "sage_srai.h"
#include "common_crypto.h"
#include "drm_prdy.h"

BDBG_MODULE(playpump_two_pes_files2);

/* the following define the input file and its characteristics -- these will vary by input file */
#define VIDEO_FILE_NAME "videos/cnnticker.video.pes"
#define AUDIO_FILE_NAME "videos/cnnticker.audio.pes"
#define TRANSPORT_TYPE NEXUS_TransportType_eMpeg2Pes
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0xe0
#define AUDIO_PID 0xc0

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(void)
{
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle videoPlaypump, audioPlaypump;
    BKNI_EventHandle event;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif
    FILE *vfile, *afile;
    const char *vfname = VIDEO_FILE_NAME;
    const char *afname = AUDIO_FILE_NAME;

    BSAGElib_State sage_platform_status;
    SRAI_PlatformHandle sage_platformHandle;
    BSAGElib_InOutContainer *container;

    BERR_Code sage_rc = BERR_SUCCESS;
    uint8_t *pSecureVideoHeapBuffer = NULL;
    uint8_t *pSecureAudioHeapBuffer = NULL;
    CommonCryptoJobSettings cryptoJobSettings;
    CommonCryptoHandle commonCryptoHandle;
    CommonCryptoSettings  cmnCryptoSettings;
    void *unsecureVideoBuffer = NULL;
    void *unsecureAudioBuffer = NULL;
    NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;

#define MAX_READ (188*1024)

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    CommonCrypto_GetDefaultSettings(&cmnCryptoSettings);
    commonCryptoHandle = CommonCrypto_Open(&cmnCryptoSettings);

#define USE_SECURE_VIDEO_PATH 1
#if USE_SECURE_VIDEO_PATH
    /* Open Sage */

    sage_rc = SRAI_Platform_Open(BSAGE_PLATFORM_ID_COMMONDRM, &sage_platform_status, &sage_platformHandle);
    if (sage_rc != BERR_SUCCESS)
    {
        printf("%s - Error calling platform_open", BSTD_FUNCTION);
        assert(sage_rc == BERR_SUCCESS);
    }
    if(sage_platform_status == BSAGElib_State_eUninit)
    {
        container = SRAI_Container_Allocate();
        printf("%s - container %p\n", BSTD_FUNCTION, (void *)container);
        if(container == NULL)
        {
            printf("%s - Error fetching container", BSTD_FUNCTION);
            assert(container);
        }
        sage_rc = SRAI_Platform_Init(sage_platformHandle, container);
        if (sage_rc != BERR_SUCCESS)
        {
            printf("%s - Error calling platform init", BSTD_FUNCTION);
            assert(sage_rc == BERR_SUCCESS);
        }
    }

#endif

    /* setup playpumps */
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
#if USE_SECURE_VIDEO_PATH
    /* Configure playpump for secure video path */
    playpumpOpenSettings.dataNotCpuAccessible = true;
    pSecureVideoHeapBuffer = SRAI_Memory_Allocate(playpumpOpenSettings.fifoSize, SRAI_MemoryType_SagePrivate);
    assert(pSecureVideoHeapBuffer);
    playpumpOpenSettings.memory = NEXUS_MemoryBlock_FromAddress(pSecureVideoHeapBuffer);
#endif
    videoPlaypump = NEXUS_Playpump_Open(0, &playpumpOpenSettings);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
#if USE_SECURE_VIDEO_PATH
    /* Configure playpump for secure audio path */
    playpumpOpenSettings.dataNotCpuAccessible = true;
    pSecureAudioHeapBuffer = SRAI_Memory_Allocate(playpumpOpenSettings.fifoSize, SRAI_MemoryType_SagePrivate);
    assert(pSecureAudioHeapBuffer);
    playpumpOpenSettings.memory = NEXUS_MemoryBlock_FromAddress(pSecureAudioHeapBuffer);
#endif
    audioPlaypump = NEXUS_Playpump_Open(1, &playpumpOpenSettings);

    /* use stdio for file I/O to keep the example simple. */
    vfile = fopen(vfname, "rb");
    if (!vfile) {
        fprintf(stderr, "can't open file:%s\n", vfname);
        goto error;
    }

    afile = fopen(afname, "rb");
    if (!afile) {
        fprintf(stderr, "can't open file:%s\n", afname);
        goto error;
    }

    /* setup playpumps */
    NEXUS_Playpump_GetSettings(videoPlaypump, &playpumpSettings);
    playpumpSettings.transportType = TRANSPORT_TYPE;
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    NEXUS_Playpump_SetSettings(videoPlaypump, &playpumpSettings);
    NEXUS_Playpump_Start(videoPlaypump);

    NEXUS_Playpump_GetSettings(audioPlaypump, &playpumpSettings);
    playpumpSettings.transportType = TRANSPORT_TYPE;
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    NEXUS_Playpump_SetSettings(audioPlaypump, &playpumpSettings);
    NEXUS_Playpump_Start(audioPlaypump);

    /* setup stc channel */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    videoPidChannel = NEXUS_Playpump_OpenPidChannel(videoPlaypump, VIDEO_PID, NULL);
#if USE_SECURE_VIDEO_PATH
    NEXUS_SetPidChannelBypassKeyslot(videoPidChannel, NEXUS_BypassKeySlot_eGR2R);
#endif

    audioPidChannel = NEXUS_Playpump_OpenPidChannel(audioPlaypump, AUDIO_PID, NULL);
#if USE_SECURE_VIDEO_PATH
    NEXUS_SetPidChannelBypassKeyslot(audioPidChannel, NEXUS_BypassKeySlot_eGR2R);
#endif


    /* setup decoders. both decoders get same stc channel for lipsync */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* bring up video decoder and display */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
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
#if USE_SECURE_VIDEO_PATH
    videoDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
#endif

    videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

#if USE_SECURE_VIDEO_PATH
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    audioDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
#endif
    /* bring up audio decoder and dacs */
    audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings); /* take default capabilities */
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    NEXUS_Memory_Allocate(MAX_READ, NULL, &unsecureVideoBuffer);
    NEXUS_Memory_Allocate(MAX_READ, NULL, &unsecureAudioBuffer);

    while (1) {
        void *secureVideobuffer, *secureAudiobuffer;
        size_t vbuffer_size, abuffer_size;
        int nVideo, nAudio;
        NEXUS_Error rc;
        NEXUS_VideoDecoderStatus vStatus;
        NEXUS_AudioDecoderStatus aStatus;
        uint32_t stc;
        NEXUS_DmaJobBlockSettings blkSettings;

        if (NEXUS_Playpump_GetBuffer(videoPlaypump, &secureVideobuffer, &vbuffer_size))
            break;

        if (NEXUS_Playpump_GetBuffer(audioPlaypump, &secureAudiobuffer, &abuffer_size))
            break;

        /* if both playpumps have no space available, wait for dataCallback from either one */
        if (vbuffer_size==0 && abuffer_size==0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }

        /* The first call to get_buffer will return the entire playback buffer.
        If we use it, we're going to have to wait until the descriptor is complete,
        and then we're going to underflow. So use a max size. */
        if (vbuffer_size) {
            if (vbuffer_size > MAX_READ)
                vbuffer_size = MAX_READ;

            nVideo = fread(unsecureVideoBuffer, 1, vbuffer_size, vfile);
            if (nVideo < 0) goto error;
            if (nVideo == 0) {
                /* wait for the video decoder to reach the end of the content before looping both PES files */
                while (1) {
                    NEXUS_VideoDecoderStatus status;
                    NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
                    if (!status.queueDepth) break;
                }
                fseek(vfile, 0, SEEK_SET);
                NEXUS_Playpump_Flush(videoPlaypump);

                fseek(afile, 0, SEEK_SET);
                NEXUS_Playpump_Flush(audioPlaypump);
                printf("seek to beginning of PES files and flush playpumps\n");
                continue;
            }
            else {
                {
                    int ii = 0;
                    int max;
                    uint8_t* p = (uint8_t* )unsecureVideoBuffer;
                    if(nVideo > 8)
                        max = 8;
                    else
                        max = nVideo;

                    printf("Unsecure buffer: ");
                    for(ii = 0; ii < max; ii++){
                        printf("%x ", p[ii]);
                    }
                    printf("\n");
                }

#if USE_SECURE_VIDEO_PATH
                NEXUS_DmaJob_GetDefaultBlockSettings(&blkSettings);
                blkSettings.pSrcAddr = unsecureVideoBuffer;
                blkSettings.pDestAddr = secureVideobuffer;
                blkSettings.blockSize = nVideo;
                blkSettings.resetCrypto = true;
                blkSettings.scatterGatherCryptoStart = true;
                blkSettings.scatterGatherCryptoEnd = true;


                /* Need to flush manually the source buffer (non secure heap). We need to flush manually as soon as we copy data into
                   the secure heap. Setting blkSettings[ii].cached = true would also try to flush the destination address in the secure heap
                   which is not accessible. This would cause the whole memory to be flushed at once. */
                NEXUS_FlushCache(blkSettings.pSrcAddr, blkSettings.blockSize);
                blkSettings.cached = false; /* Prevent the DMA from flushing the buffers later on */


                CommonCrypto_GetDefaultJobSettings(&cryptoJobSettings);
                CommonCrypto_DmaXfer(commonCryptoHandle,  &cryptoJobSettings, &blkSettings, 1);

                /* Need to flush manually the source buffer (non secure heap). We need to flush manually as soon as we copy data into
                the secure heap. Setting blkSettings[ii].cached = true would also try to flush the destination address in the secure heap
                which is not accessible. This would cause the whole memory to be flushed at once. */
                NEXUS_FlushCache(blkSettings.pSrcAddr, blkSettings.blockSize);
#endif

                /* played (nVideo) bytes */
                rc = NEXUS_Playpump_WriteComplete(videoPlaypump, 0, nVideo);
                BDBG_ASSERT(!rc);
            }
        }
        if (abuffer_size) {
            if (abuffer_size > MAX_READ)
                abuffer_size = MAX_READ;

            nAudio = fread(unsecureAudioBuffer, 1, abuffer_size, afile);
            if (nAudio < 0) goto error;
            if (nAudio == 0) {
                /* do nothing here and handle the looping in video instead */
            }
            else {
#if USE_SECURE_VIDEO_PATH
                NEXUS_DmaJob_GetDefaultBlockSettings(&blkSettings);
                blkSettings.pSrcAddr = unsecureAudioBuffer;
                blkSettings.pDestAddr = secureAudiobuffer;
                blkSettings.blockSize = nAudio;
                blkSettings.resetCrypto = true;
                blkSettings.scatterGatherCryptoStart = true;
                blkSettings.scatterGatherCryptoEnd = true;


                /* Need to flush manually the source buffer (non secure heap). We need to flush manually as soon as we copy data into
                   the secure heap. Setting blkSettings[ii].cached = true would also try to flush the destination address in the secure heap
                   which is not accessible. This would cause the whole memory to be flushed at once. */
                NEXUS_FlushCache(blkSettings.pSrcAddr, blkSettings.blockSize);
                blkSettings.cached = false; /* Prevent the DMA from flushing the buffers later on */


                CommonCrypto_GetDefaultJobSettings(&cryptoJobSettings);
                CommonCrypto_DmaXfer(commonCryptoHandle,  &cryptoJobSettings, &blkSettings, 1);

                /* Need to flush manually the source buffer (non secure heap). We need to flush manually as soon as we copy data into
                the secure heap. Setting blkSettings[ii].cached = true would also try to flush the destination address in the secure heap
                which is not accessible. This would cause the whole memory to be flushed at once. */
                NEXUS_FlushCache(blkSettings.pSrcAddr, blkSettings.blockSize);
#endif
                printf("Feeding audio data\n");
                /* played (nAudio) bytes */
                rc = NEXUS_Playpump_WriteComplete(audioPlaypump, 0, nAudio);
                BDBG_ASSERT(!rc);
            }
        }

#if USE_SECURE_VIDEO_PATH /* simple decode status */
        if (vbuffer_size && nVideo) {
            NEXUS_VideoDecoder_GetStatus(videoDecoder, &vStatus);
            NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
            printf("video %.4dx%.4d, pts %#x, stc %#x (diff %5d) fifo=%d%%\n",
                vStatus.source.width, vStatus.source.height, vStatus.pts, stc, vStatus.ptsStcDifference, vStatus.fifoSize?(vStatus.fifoDepth*100)/vStatus.fifoSize:0);
            NEXUS_AudioDecoder_GetStatus(audioDecoder, &aStatus);
            printf("audio            pts %#x, stc %#x (diff %5d) fifo=%d%%\n\n",
                aStatus.pts, stc, aStatus.ptsStcDifference, aStatus.fifoSize?(aStatus.fifoDepth*100)/aStatus.fifoSize:0);
        }
#endif

    }

#if USE_SECURE_VIDEO_PATH
    SRAI_Memory_Free(pSecureVideoHeapBuffer);
    SRAI_Memory_Free(pSecureAudioHeapBuffer);
#endif

    NEXUS_Memory_Free(unsecureVideoBuffer);
    NEXUS_Memory_Free(unsecureAudioBuffer);
    return 0;

error:
    if(unsecureVideoBuffer) NEXUS_Memory_Free(unsecureVideoBuffer);
    if(unsecureAudioBuffer) NEXUS_Memory_Free(unsecureAudioBuffer);

#if USE_SECURE_VIDEO_PATH
    if(pSecureVideoHeapBuffer) SRAI_Memory_Free(pSecureVideoHeapBuffer);
    if(pSecureAudioHeapBuffer) SRAI_Memory_Free(pSecureAudioHeapBuffer);
#endif


    return 1;
}
