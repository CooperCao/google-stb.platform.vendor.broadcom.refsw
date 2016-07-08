/***************************************************************************
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
 * Module Description:
 *
 **************************************************************************/
#include "nxclient.h"
#include "nexus_dma.h"
#include "nexus_platform_client.h"
#include "nexus_base_os.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(dma);

/**
Run "dma #" to choose between 3 methods of managing the cache with DMA:

1) Recommended method (default)
    App-controlled cache flush, works with any heap, user or kernel mode

2) Less efficient but simple method
    Driver-controlled cache flush, must use eFull heap for kernel mode

3) Invalid method
    Driver-controlled cache flush, but with no driver-side memory mapping.
    This happens to work in user mode, but will fail in kernel mode.
*/

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent(context);
}

int main(int argc, char **argv)
{
    NEXUS_MemoryAllocationSettings allocSettings;
    int rc;
    unsigned bufferSize = 1024;
    void *buffer;
    NEXUS_DmaHandle dma;
    NEXUS_DmaJobHandle job;
    NEXUS_DmaJobSettings jobSettings;
    BKNI_EventHandle event;
    unsigned i;
    unsigned method = 1;

    if (argc > 1) method = atoi(argv[1]);

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    BKNI_CreateEvent(&event);

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    if (method == 2) {
        /* select a heap with driver-side mmap */
        allocSettings.heap = NEXUS_Heap_Lookup(NEXUS_HeapLookupType_eMain);
    }
    rc = NEXUS_Memory_Allocate(bufferSize, &allocSettings, &buffer);
    BDBG_ASSERT(!rc);

    dma = NEXUS_Dma_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(dma);

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.completionCallback.callback = complete;
    jobSettings.completionCallback.context = event;
    job = NEXUS_DmaJob_Create(dma, &jobSettings);
    BDBG_ASSERT(job);

    /* read from first half, write into second half */
    for (i=0;i<100;i++) {
        NEXUS_DmaJobBlockSettings blockSettings;
        unsigned char value = rand();
        unsigned halfBufferSize = bufferSize / 2;
        unsigned char *writeBuffer = (unsigned char *)buffer + halfBufferSize;
        unsigned j;

        BDBG_WRN(("test %d: writing %d", i, value));
        memset(buffer, value, halfBufferSize);
        if (method == 1) {
            /* flush before DMA read. see nexus_dma.h for full explanation. */
            NEXUS_FlushCache(buffer, halfBufferSize);
        }

        NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
        blockSettings.pSrcAddr = buffer;
        blockSettings.pDestAddr = writeBuffer;
        blockSettings.blockSize = halfBufferSize;
        if (method == 1) {
            blockSettings.cached = false; /* preferred */
        }
        else {
           blockSettings.cached = true; /* ask the driver to flush on app's behalf */
       }
        rc = NEXUS_DmaJob_ProcessBlocks(job, &blockSettings, 1);
        if (rc == NEXUS_DMA_QUEUED) {
            NEXUS_DmaJobStatus status;
            rc = BKNI_WaitForEvent(event, BKNI_INFINITE);
            BDBG_ASSERT(!rc);
            rc = NEXUS_DmaJob_GetStatus(job, &status);
            BDBG_ASSERT(!rc);
            BDBG_ASSERT(status.currentState == NEXUS_DmaJobState_eComplete);
        }
        else {
            BDBG_ASSERT(!rc);
        }

        if (method == 1) {
            /* flush after DMA write. see nexus_dma.h for full explanation. */
            NEXUS_FlushCache(writeBuffer, halfBufferSize);
        }
        for (j=0;j<halfBufferSize;j++) {
            BDBG_ASSERT(writeBuffer[j] == value);
        }
    }

    NEXUS_DmaJob_Destroy(job);
    NEXUS_Dma_Close(dma);
    BKNI_DestroyEvent(event);
    NxClient_Uninit();
    return 0;
}
