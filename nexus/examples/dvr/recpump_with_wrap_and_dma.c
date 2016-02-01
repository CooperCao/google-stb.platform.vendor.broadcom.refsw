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
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_recpump.h"
#include "nexus_dma.h"

#include <stdio.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

BDBG_MODULE(recpump_with_wrap_and_dma);

#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define AUDIO_PID 0x14

/* imagine crypto DMA that must use 16 byte blocks. */
#define CHUNK_SIZE        (188 * 128)
#define CHUNK_COUNT       (128)
#define CIPHER_BLOCK_SIZE (16)

/* TODO: refactor to avoid global */
static NEXUS_DmaJobHandle dmaJob;

static void dma_complete(void *pParam, int iParam)
{
    NEXUS_DmaJobStatus status;
    BKNI_EventHandle event = (BKNI_EventHandle)pParam;
    BSTD_UNUSED(iParam);

    NEXUS_DmaJob_GetStatus(dmaJob, &status);
    if (status.currentState == NEXUS_DmaJobState_eComplete) {
        BKNI_SetEvent(event);
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
    BDBG_ERR(("overflow %s", (const char *)context));
}

int main(int argc, char **argv)
{
    FILE *data_file;
    const char data_fname[] = "videos/stream.mpg";
    BKNI_EventHandle event;
    NEXUS_InputBand inputBand;
    NEXUS_RecpumpOpenSettings openSettings;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpHandle recpump;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PidChannelHandle pidChannel[2];
    NEXUS_DmaHandle dma;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_RecpumpStatus status;
    void *pMem = NULL;
    NEXUS_Error rc;
    BKNI_EventHandle dmaEvent;
    unsigned i;
    NEXUS_PlatformSettings platformSettings;
    bool crr = false;

    if (argc > 1 && !strcmp(argv[1], "-crr")) {
        crr = true;
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    BKNI_CreateEvent(&event);

    dma = NEXUS_Dma_Open(0, NULL);
    BDBG_ASSERT(dma);

    /* This will be allocated in global memory since the CDB/ITB are in restricted memory */
    rc = NEXUS_Memory_Allocate(CHUNK_COUNT * CHUNK_SIZE, NULL, &pMem);
	BDBG_ASSERT(!rc);

    rc = BKNI_CreateEvent(&dmaEvent);
	BDBG_ASSERT(!rc);

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.dataFormat = NEXUS_DmaDataFormat_eBlock;
    jobSettings.numBlocks = 1;
    jobSettings.completionCallback.callback = dma_complete;
    jobSettings.completionCallback.context = dmaEvent;

    dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);
    BDBG_ASSERT(dmaJob);

    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.cached = false; /* flushing source and dest is inefficient */

    NEXUS_Recpump_GetDefaultOpenSettings(&openSettings);
    openSettings.data.alignment = 12;
    /* OpenSettings.data.atomSize is ignored with GetBufferWithWrap.
    The +256-188 is needed to avoid recpump truncating the CDB to meet HW wraparound requirements. */
    openSettings.data.bufferSize = (CHUNK_COUNT * CHUNK_SIZE) + 256 - 188;
    openSettings.data.dataReadyThreshold = CHUNK_SIZE;
    openSettings.useSecureHeap = crr;
    if (crr) {
        BDBG_WRN(("record into crr"));
    }
    recpump = NEXUS_Recpump_Open(0, &openSettings);
    BDBG_ASSERT(recpump);

    data_file = fopen(data_fname, "wb");
    if (!data_file) {
        fprintf(stderr, "can't open data file:%s\n", data_fname);
        goto error;
    }

    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.dataReady.context = event;
    recpumpSettings.data.overflow.callback = overflow_callback;
    recpumpSettings.data.overflow.context = "data";
    NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);

    pidChannel[0] = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    NEXUS_Recpump_AddPidChannel(recpump, pidChannel[0], NULL);
    pidChannel[1] = NEXUS_PidChannel_Open(parserBand, 0x14, NULL);
    NEXUS_Recpump_AddPidChannel(recpump, pidChannel[1], NULL);

    NEXUS_Recpump_Start(recpump);

    NEXUS_Recpump_GetStatus(recpump, &status);

    while (1) {
        const void *data_buffer[2], *index_buffer[2];
        size_t data_buffer_size[2], index_buffer_size[2];
        ssize_t n;

        rc = NEXUS_Recpump_GetDataBufferWithWrap(recpump, &data_buffer[0], &data_buffer_size[0], &data_buffer[1], &data_buffer_size[1]);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Recpump_GetIndexBufferWithWrap(recpump, &index_buffer[0], &index_buffer_size[0], &index_buffer[1], &index_buffer_size[1]);
        BDBG_ASSERT(!rc);
        BDBG_MSG(("got %d+%d data, %d+%d index", data_buffer_size[0], data_buffer_size[1], index_buffer_size[0], index_buffer_size[1]));

        /* atomSize doesn't work on GetBufferWithWrap. but because of how we sized the CDB,
        we should always by divisible by CIPHER_BLOCK_SIZE on wrap.
        an exception is when recpump stops: then the remainder must be dropped. */
        if (data_buffer_size[1]) {
            data_buffer_size[1] -= data_buffer_size[1] % CIPHER_BLOCK_SIZE;
        }
        else {
            data_buffer_size[0] -= data_buffer_size[0] % CIPHER_BLOCK_SIZE;
        }
        BDBG_ASSERT(data_buffer_size[0] % CIPHER_BLOCK_SIZE == 0);
        BDBG_ASSERT(data_buffer_size[1] % CIPHER_BLOCK_SIZE == 0);
        if (data_buffer_size[0] < CHUNK_SIZE && !data_buffer_size[1]) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }

        for (i=0;i<2;i++) {
            if (!data_buffer_size[i]) continue;

            blockSettings.pSrcAddr = (unsigned char *)data_buffer[i];
            blockSettings.pDestAddr = (unsigned char *)pMem;
            blockSettings.blockSize = data_buffer_size[i];
            rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, &blockSettings, 1);
            rc = BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
            BDBG_ASSERT(!rc);

            NEXUS_FlushCache(pMem, data_buffer_size[i]);
            n = fwrite(pMem, 1, data_buffer_size[i], data_file);
            if (n < 0) goto error;
        }
        rc = NEXUS_Recpump_DataReadComplete(recpump, data_buffer_size[0] + data_buffer_size[1]);
        BDBG_ASSERT(!rc);
        if (index_buffer_size[0] + index_buffer_size[1]) {
            /* Don't care about the index data */
            rc = NEXUS_Recpump_IndexReadComplete(recpump, index_buffer_size[0] + index_buffer_size[1]);
            BDBG_ASSERT(!rc);
        }
        BDBG_WRN(("wrote %d+%d data, %d+%d index", data_buffer_size[0], data_buffer_size[1], index_buffer_size[0], index_buffer_size[1]));
    }
    /* loops forever */

error:
    return 1;
}
