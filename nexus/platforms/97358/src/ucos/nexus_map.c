/***************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
***************************************************************************/
#include "bstd.h"
#include "nexus_map.h"
#include "nexus_base_types.h"
#include "bmem.h"
#include "bkni.h"

/**
n_map_memory() is a wrapper around fake + real memory mapping.
fake mapping is handled with a special BMEM heap using a fake address range.
it is totally separate from the real BMEM heaps created by nexus_core.
**/
static struct {
    struct nexus_map_settings settings;
    BMEM_ModuleHandle mem;
    BMEM_Heap_Handle heap;
    unsigned base;
    unsigned size;
} g_fake;

void nexus_p_get_default_map_settings(struct nexus_map_settings *p_settings)
{
    BKNI_Memset(p_settings, 0, sizeof(*p_settings));
}

int nexus_p_init_map(const struct nexus_map_settings *p_settings)
{
    BMEM_Settings mem_settings;
    BMEM_Heap_Settings heap_settings;
    int rc;

    if (g_fake.mem) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    g_fake.settings = *p_settings;

    BMEM_GetDefaultSettings(&mem_settings);
    rc = BMEM_Open(&g_fake.mem, &mem_settings);
    if (rc) return BERR_TRACE(rc);

    /* the fake address range is (2GB - 4K). subtracting 4K allows the code to avoid 0x0000_0000.
    for kernel mode, we can't have a valid fake address of 0x0000_0000. too much code depends on ptr == NULL meaning "no pointer".
    for user mode, we can't have base+size == 0x0000_0000. too much code will fail on the wrap around.
    only 1 byte is needed to avoid this situation, but using 4K avoid possible alignment bugs. */
#if NEXUS_MODE_driver
    g_fake.base = 0x00000000 + 4096;
    g_fake.size = 0x80000000 - 4096;
#else
    g_fake.base = 0x80000000;
    g_fake.size = 0x80000000 - 4096;
#endif
    BMEM_Heap_GetDefaultSettings(&heap_settings);
    heap_settings.eSafetyConfig = BMEM_SafetyConfig_eFastest;
    heap_settings.eBookKeeping = BMEM_BookKeeping_eSystem;
    rc = BMEM_Heap_Create(g_fake.mem, (void*)g_fake.base, 0, g_fake.size, &heap_settings, &g_fake.heap);
    if (rc) {
        BMEM_Close(g_fake.mem);
        g_fake.mem = NULL;
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

void nexus_p_uninit_map(void)
{
    BMEM_Heap_Destroy(g_fake.heap);
    g_fake.heap = NULL;
    BMEM_Close(g_fake.mem);
    g_fake.mem = NULL;
}

void *nexus_p_map_memory(unsigned long offset, unsigned long length, NEXUS_MemoryMapType type)
{
    switch (type) {
    case NEXUS_MemoryMapType_eFake:
        return BMEM_Heap_AllocAligned(g_fake.heap, length, 0, 0);
    case NEXUS_MemoryMapType_eCached:
        return (g_fake.settings.mmap)(offset, length, true);
    case NEXUS_MemoryMapType_eUncached:
        return (g_fake.settings.mmap)(offset, length, false);
    default:
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
}

void nexus_p_unmap_memory( void *pMem, unsigned long length )
{
    if ((unsigned)pMem >= g_fake.base && (unsigned)pMem < g_fake.base + g_fake.size) {
        BMEM_Free(g_fake.heap, pMem);
    }
    else {
        (g_fake.settings.munmap)(pMem, length);
    }
}

