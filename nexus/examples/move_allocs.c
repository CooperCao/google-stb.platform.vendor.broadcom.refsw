/******************************************************************************
 *    (c)2015 Broadcom Corporation
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
#include "nexus_memory.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>

BDBG_MODULE(move_allocs);

#if defined NEXUS_MEMC0_PICTURE_BUFFER_HEAP && defined NEXUS_MEMC0_MAIN_HEAP && NEXUS_HAS_VIDEO_DECODER
int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
#define MAX_BLOCKS 50
    NEXUS_MemoryBlockHandle block[MAX_BLOCKS];
    unsigned i, index;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);

#if 0
    index = NEXUS_MEMC0_MAIN_HEAP;
#else
    {
        unsigned graphicsHeapIndex;
        for (graphicsHeapIndex=0;graphicsHeapIndex<NEXUS_MAX_HEAPS;graphicsHeapIndex++) {
            if (platformSettings.heap[graphicsHeapIndex].heapType & NEXUS_HEAP_TYPE_GRAPHICS) {
                break;
            }
        }
        if (graphicsHeapIndex == NEXUS_MAX_HEAPS) {
            return -1;
        }
        /* create ondemand mapped heap */
        index = NEXUS_MAX_HEAPS-1;
        platformSettings.heap[index].memcIndex = platformSettings.heap[graphicsHeapIndex].memcIndex;
        platformSettings.heap[index].size = 8*1024*1028;
        platformSettings.heap[index].memoryType = NEXUS_MEMORY_TYPE_MANAGED|NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;
        /* reduce graphics heap to make space */
        platformSettings.heap[graphicsHeapIndex].size -= platformSettings.heap[index].size;
    }
#endif

    platformSettings.openFrontend = false;
    for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
        memConfigSettings.videoDecoder[i].dynamicPictureBuffers = true;
    }
    platformSettings.heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memoryType = NEXUS_MEMORY_TYPE_MANAGED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;
    rc = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
    if (rc) return rc;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    for (i=0;i<MAX_BLOCKS;i++) {
        block[i] = NEXUS_MemoryBlock_Allocate(platformConfig.heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP], 256, 0, NULL);
        BDBG_ASSERT(block[i]);
        if (i%2) {
            void *ptr;
            NEXUS_MemoryBlock_Lock(block[i], &ptr);
        }
    }

    while (1) {
        unsigned num;
        rc = NEXUS_Memory_MoveUnlockedBlocks(platformConfig.heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP], platformConfig.heap[index], 10, &num);
        BDBG_WRN(("moved %d blocks", num));
        if (rc || num < 10) break;
    }

    for (i=0;i<MAX_BLOCKS;i++) {
        NEXUS_MemoryBlock_Free(block[i]);
    }

    NEXUS_Platform_Uninit();
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
