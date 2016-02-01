/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA)
#include "nexus_dma.h"
#endif
#include "nexus_memory.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "stdlib.h"

#define DMA_BLOCK   8*1024*1024

#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA)
static void CompleteCallback(void *pParam, int iParam)
{
    BSTD_UNUSED(iParam);
    fprintf(stderr, "CompleteCallback:%#lx fired\n", (unsigned long)pParam);
    BKNI_SetEvent(pParam);
}
#endif


#define DMA_JOBS 1

int main(int argc, char **argv)
{
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA)
    NEXUS_DmaHandle dma;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJobs[DMA_JOBS];
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJobStatus jobStatus;
    void *pMem1=NULL, *pMem2=NULL;
    unsigned i;
    unsigned block;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_Error rc;
    BKNI_EventHandle event;
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    NEXUS_Platform_GetDefaultSettings(&platformSettings); 
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    dma = NEXUS_Dma_Open(0, NULL);

    NEXUS_Memory_Allocate(DMA_BLOCK, NULL, &pMem1);
    NEXUS_Memory_Allocate(DMA_BLOCK, NULL, &pMem2);
    BKNI_CreateEvent(&event);

    for (block=DMA_BLOCK; block>16; block/=4) {
        fprintf(stderr, "---DMA block %u bytes----\n", block);
        NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
        jobSettings.numBlocks = 1;
        jobSettings.completionCallback.callback = CompleteCallback;
        for (i=0; i<DMA_JOBS; i++) {
            jobSettings.completionCallback.context = event;
            dmaJobs[i] = NEXUS_DmaJob_Create(dma, &jobSettings);
        }

        for (i=0; i<20; i++) {
            NEXUS_DmaJobHandle dmaJob = dmaJobs[i%DMA_JOBS];

            memset(pMem1, rand()%0x100, block);
            memset(pMem2, rand()%0x100, block);
            /* do our own cache flush, before and after the transfer */
            NEXUS_FlushCache(pMem1, block);
            NEXUS_FlushCache(pMem2, block);
   
            NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
            blockSettings.pSrcAddr = pMem1;
            blockSettings.pDestAddr = pMem2;
            blockSettings.blockSize = block;
            blockSettings.cached = false;
            rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, &blockSettings, 1);
            fprintf(stderr, "ProcessBlocks: rc=%d\n", rc);

            if (rc==NEXUS_DMA_QUEUED) {
                fprintf(stderr, "Waiting...\n"); 
                BKNI_WaitForEvent(event, BKNI_INFINITE);
                NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
                BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
            }

            /* compare */
            NEXUS_FlushCache(pMem2, block); /* do our own cache flush */
            if (memcmp(pMem1, pMem2, block)) {
                fprintf(stderr, "Failed\n");
            }
            else {
                fprintf(stderr, "Success [%u]\n", i);
            }
        }
        for (i=0; i<DMA_JOBS; i++) {
            NEXUS_DmaJob_Destroy(dmaJobs[i]);
        }
    }

    BKNI_DestroyEvent(event);
    NEXUS_Memory_Free(pMem2);
    NEXUS_Memory_Free(pMem1);

    NEXUS_Dma_Close(dma);
    NEXUS_Platform_Uninit();
#else
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);
#endif

    return 0;
}
