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
#if NEXUS_HAS_DISPLAY && NEXUS_HAS_SIMPLE_DECODER && NEXUS_HAS_TRANSPORT && NEXUS_HAS_VIDEO_DECODER
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playpump.h"

#include <stdio.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/cnnticker.mpg"
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x21

BDBG_MODULE(playpump_sg_client);

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)
{
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    FILE *file;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    BKNI_EventHandle event;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_Error rc;

    const char *fname = FILE_NAME;
#define TOTAL_BUFFERS 10
    void *buf[TOTAL_BUFFERS];
    size_t buf_size = 128*1024;
    unsigned cur_buf;
    unsigned i;

    BSTD_UNUSED(argc);

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    BKNI_CreateEvent(&event);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpumpOpenSettings.fifoSize = 0;
    playpump = NEXUS_Playpump_Open(0, &playpumpOpenSettings);

    /* use stdio for file I/O to keep the example simple. */
    file = fopen(fname, "rb");
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        goto error;
    }

    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    playpumpSettings.dataNotCpuAccessible = true;
    /* setting mode = NEXUS_PlaypumpMode_eScatterGather is deprecated */
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);

    NEXUS_Playpump_Start(playpump);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, VIDEO_PID, NULL);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {BDBG_WRN(("unable to alloc transcode resources")); return -1;}

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) {BDBG_WRN(("unable to connect transcode resources")); return -1;}

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);

    NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);

    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.settings.codec = VIDEO_CODEC;
    videoProgram.settings.pidChannel = videoPidChannel;
    NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);

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
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }

        n = fread(buf[cur_buf], 1, buf_size, file);
        if (n < 0) goto error;
        if (n == 0) {
            /* wait for the decoder to reach the end of the content before looping */
            while (1) {
                NEXUS_VideoDecoderStatus status;
                NEXUS_SimpleVideoDecoder_GetStatus(videoDecoder, &status);
                if (!status.queueDepth) break;
            }
            fseek(file, 0, SEEK_SET);
            NEXUS_Playpump_Flush(playpump);
        }
        else {
            NEXUS_PlaypumpScatterGatherDescriptor desc;
            unsigned numConsumed;
            desc.addr = buf[cur_buf];
            desc.length = n;
            if (playpumpSettings.dataNotCpuAccessible) {
                NEXUS_FlushCache(desc.addr, desc.length);
            }
            rc = NEXUS_Playpump_SubmitScatterGatherDescriptor(playpump, &desc, 1, &numConsumed);
            BDBG_ASSERT(!rc);
            BDBG_ASSERT(numConsumed==1); /* we've already checked that there are descriptors available*/
            cur_buf = (cur_buf + 1)%TOTAL_BUFFERS; /* use the next buffer */
        }
    }
    return 0;

error:
    return 1;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs display, simple_decoder, transport and video_decoder)!\n");
    return 0;
}
#endif
