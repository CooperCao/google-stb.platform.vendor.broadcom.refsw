/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>

BDBG_MODULE(dynamic_heap);

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
    unsigned graphicsHeapIndex;
    NEXUS_HeapHandle dynamicHeap;

    /* first, print default heaps. this is useful for system debug. */

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);

    for (graphicsHeapIndex=0;graphicsHeapIndex<NEXUS_MAX_HEAPS;graphicsHeapIndex++) {
        if (platformSettings.heap[graphicsHeapIndex].heapType & NEXUS_HEAP_TYPE_GRAPHICS) {
            break;
        }
    }
    if (graphicsHeapIndex == NEXUS_MAX_HEAPS) {
        return -1;
    }

    /* create dynamic heap in same MEMC region as graphics heap */
    platformSettings.heap[NEXUS_MAX_HEAPS-1].memcIndex = platformSettings.heap[graphicsHeapIndex].memcIndex;
    platformSettings.heap[NEXUS_MAX_HEAPS-1].subIndex = platformSettings.heap[graphicsHeapIndex].subIndex;
    platformSettings.heap[NEXUS_MAX_HEAPS-1].size = 4096; /* very small, but not zero */
    platformSettings.heap[NEXUS_MAX_HEAPS-1].memoryType = NEXUS_MEMORY_TYPE_MANAGED|NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;

    /* reduce graphics heap for non-dynamic uses */
    platformSettings.heap[graphicsHeapIndex].size = 32 * 1024 * 1024;

    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return rc;

    /* call NEXUS_Platform_GetConfiguration to get the heap handles */
    NEXUS_Platform_GetConfiguration(&platformConfig);
    dynamicHeap = platformConfig.heap[NEXUS_MAX_HEAPS-1];

    while (1) {
#define MAX_BLOCKS 1000
        NEXUS_MemoryBlockHandle block[MAX_BLOCKS];
        bool oom = false;
        unsigned i;

        for (i=0;i<MAX_BLOCKS && !oom;i++) {
            /* allocate once. if fails, grow then alloc once more. after that, we are done. */
            block[i] = NEXUS_MemoryBlock_Allocate(dynamicHeap, 1024*1024, 0, NULL);
            if (!block[i]) {
#define BLOCK_SIZE (16*1024*1024)
                rc = NEXUS_Platform_GrowHeap(dynamicHeap, BLOCK_SIZE);
                if (rc) {BERR_TRACE(rc);break;}
                block[i] = NEXUS_MemoryBlock_Allocate(dynamicHeap, 1024*1024, 0, NULL);
                if (!block[i]) {
                    oom = true;
                }
            }
            else {
                BDBG_WRN(("block[%d] %p", i, (void*)block[i]));
            }
        }

        BDBG_WRN(("allocated %d blocks", i));

        for (i=0;i<MAX_BLOCKS && block[i];i++) {
            NEXUS_MemoryBlock_Free(block[i]);
        }

        NEXUS_Platform_ShrinkHeap(dynamicHeap, BLOCK_SIZE, BLOCK_SIZE);
    }

    NEXUS_Platform_Uninit();
    return 0;
}
