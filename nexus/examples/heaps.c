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

#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_surface.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>

/**
This app queries the default heaps per platform, typically one heap per memory controller.
**/

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
    unsigned i;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_SurfaceHandle surface;
    NEXUS_MemoryBlockHandle memBlock;
    void *buffer;

    /* first, print default heaps. this is useful for system debug. */

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Memory_PrintHeaps();

#if 0
/* comment out because this code cannot run on all platforms */
    printf("Press ENTER for custom heaps\n");
    getchar();
    NEXUS_Platform_Uninit();

    /* by default, nexus will create 1 heap per MEMC */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

    /* This is sample code for user-specified heaps (>1 heap per MEMC).
    More combinations are possible. This set of heaps may not work on all systems. */
    BDBG_CASSERT(NEXUS_MAX_HEAPS > 2);
    platformSettings.heap[0].memcIndex = 0; /* MEMC 0 */
    platformSettings.heap[0].size = -1; /* remainder after allocating heaps 1 & 2 */
    platformSettings.heap[0].alignment = 1;
    /* skip [1] so that default heap[0] is MEMC0 and default heap[1] is MEMC1 (where present) */
    /* now start our custom heaps */
    platformSettings.heap[2].memcIndex = 0; /* MEMC 0 */
    platformSettings.heap[2].size = 10 * 1024 * 1024; /* 10MB heap */
    platformSettings.heap[3].memcIndex = 0; /* MEMC 0 */
    platformSettings.heap[3].size = 20 * 1024 * 1024; /* 20MB heap */
    platformSettings.heap[3].memoryType = NEXUS_MemoryType_eSecure; /* user can specify only one secure heap. Nexus will detect it and
                                                                       pass the appropriate heap handles to Magnum. */

    /* NEXUS_Platform_Init creates the heaps */
    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Memory_PrintHeaps();
#endif

    printf("Press ENTER to test allocations\n");
    getchar();

    /* app allocates from heap[0] */
    memBlock = NEXUS_MemoryBlock_Allocate(NULL, 1000, 0, NULL);
    BDBG_ASSERT(memBlock);

    rc = NEXUS_MemoryBlock_Lock(memBlock, &buffer);
    BDBG_ASSERT(!rc);
    BDBG_ASSERT(buffer);

    /* example of dumping information about each allocation and free block */
    NEXUS_Platform_GetConfiguration(&platformConfig);
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (platformConfig.heap[i]) {
            /* Nexus requires export debug_mem=y to track allocations. */
            printf("Heap[%d] dump:\n", i);
            NEXUS_Heap_Dump(platformConfig.heap[i]);
        }
    }

    NEXUS_MemoryBlock_Unlock(memBlock);
    NEXUS_MemoryBlock_Free(memBlock);

    /* sample code for using heap handle for non-default heap usage */
    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.width = 100;
    surfaceCreateSettings.height = 100;
    surfaceCreateSettings.heap = platformConfig.heap[NEXUS_MEMC0_GRAPHICS_HEAP];
    surface = NEXUS_Surface_Create(&surfaceCreateSettings);
    /* app can use it */
    NEXUS_Surface_Destroy(surface);

    NEXUS_Platform_Uninit();

    return 0;
}
