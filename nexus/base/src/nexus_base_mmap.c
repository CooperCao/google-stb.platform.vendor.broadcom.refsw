/***************************************************************************
*  Copyright (C) 2004-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_base.h"
#include "nexus_base_mmap.h"

BDBG_MODULE(nexus_base_mmap);

typedef struct NEXUS_P_MemoryMap {
    size_t length;
    NEXUS_Addr offset;
    NEXUS_AddrType cachedAddrType;
    NEXUS_AddrType uncachedAddrType;
    void *cached;
    void *uncached;
} NEXUS_P_MemoryMap;

static NEXUS_P_MemoryMap g_NEXUS_P_MemoryMap[16];

/**
NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE is the virtual address split between
kernel and user modes. Allow chip-specific override in nexus_platform_features.h.
**/
#ifndef NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE
#if NEXUS_CPU_ARM64
/* https://www.kernel.org/doc/Documentation/arm64/memory.txt  */
#define NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE 0xFFFF000000000000ul
#elif NEXUS_CPU_ARM
#define NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE 0xC0000000
#else
#define NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE 0x80000000
#endif
#endif


NEXUS_P_Base_MemoryRange g_NEXUS_P_CpuNotAccessibleRange = {
#if NEXUS_BASE_OS_ucos_ii || B_REFSW_SYSTEM_MODE_CLIENT
    NULL, 0
#elif NEXUS_MODE_driver || NEXUS_BASE_OS_linuxkernel
    NULL, NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE
#else
    (void *)NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE, ~0ul - NEXUS_KERNEL_MODE_VIRTUAL_ADDRESS_BASE
#endif
};

#define B_IS_INTERSECT(off1, len1, off2, len2) ((off1) <= ((off2)+(len2)-1) && (off2) <= ((off1)+(len1)-1))

bool NEXUS_P_CpuAccessibleAddress( const void *address )
{
    NEXUS_AddrType type = NEXUS_GetAddrType(address);
    if(type == NEXUS_AddrType_eCached || type == NEXUS_AddrType_eUncached) {
        return true;
    } else {
        return false;
    }
}

NEXUS_Error
NEXUS_P_AddMap(NEXUS_Addr offset, void *cached, NEXUS_AddrType cachedAddrType, void *uncached, NEXUS_AddrType uncachedAddrType, size_t length)
{
    unsigned i;

    /* offset==0 is normal in emulation environments */
    if (offset==0) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (uncached==NULL && cached==NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    for(i=0;i<sizeof(g_NEXUS_P_MemoryMap)/sizeof(*g_NEXUS_P_MemoryMap);i++) {
        NEXUS_P_MemoryMap *map = g_NEXUS_P_MemoryMap+i;
        if(map->length==0) {
            map->length = length;
            map->offset = offset;
            map->cached = cached;
            map->cachedAddrType = cachedAddrType;
            map->uncached = uncached;
            map->uncachedAddrType = uncachedAddrType;
            BDBG_MSG(("NEXUS_P_AddMap offset=" BDBG_UINT64_FMT " length=%#x(%uMBytes) cached_addr=%p..%p%s uncached_addr=%p..%p%s",
                BDBG_UINT64_ARG(offset), (unsigned)length, (unsigned)(length/(1024*1024)), cached,  (uint8_t *)cached + length, cachedAddrType == NEXUS_AddrType_eFake ? "[FAKE]":"", uncached, (uint8_t *)uncached + length, uncachedAddrType == NEXUS_AddrType_eFake ?"[FAKE]":""));
            return BERR_SUCCESS;
        }
        if(  B_IS_INTERSECT(offset,length, map->offset,map->length) ||
             B_IS_INTERSECT((uint8_t *)cached, length, (uint8_t *)map->cached, map->length) ||
            (uncached &&  B_IS_INTERSECT((uint8_t *)uncached, length, (uint8_t *)map->uncached, map->length)) ||
             B_IS_INTERSECT((uint8_t *)cached, length, (uint8_t *)map->uncached, map->length) ||
             (uncached && B_IS_INTERSECT((uint8_t *)uncached, length, (uint8_t *)map->cached, map->length)) 
             ) {
            BDBG_ERR(("duplicated map: (" BDBG_UINT64_FMT ",%p,%p):%u and (" BDBG_UINT64_FMT ",%p,%p):%u %s %s %s", BDBG_UINT64_ARG(offset), cached, uncached, (unsigned)length, BDBG_UINT64_ARG(map->offset), map->cached, map->uncached, (unsigned)map->length, B_IS_INTERSECT(offset,length, map->offset,map->length)?"O":"", B_IS_INTERSECT((uint8_t *)cached, length, (uint8_t *)map->cached, map->length)?"C":"",  B_IS_INTERSECT((uint8_t *)uncached, length, (uint8_t *)map->uncached, map->length)?"U":""));
            /* BDBG_ASSERT(0); */
        }
    }
    return BERR_TRACE(NEXUS_UNKNOWN);
}

NEXUS_Addr
NEXUS_AddrToOffset(const void *addr)
{
    unsigned i;
    const NEXUS_P_MemoryMap *map = g_NEXUS_P_MemoryMap;
    for(i=0;i<sizeof(g_NEXUS_P_MemoryMap)/sizeof(*g_NEXUS_P_MemoryMap);i++) {
        if(map[i].length==0) {
            break;
        }
        if( (uint8_t *)addr>=(uint8_t *)map[i].cached &&
            (uint8_t *)addr<((uint8_t *)map[i].cached + map[i].length) &&
            map[i].cached != NULL
            ) {
            return map[i].offset + ((uint8_t *)addr - (uint8_t *)map[i].cached);
        }
        if( (uint8_t *)addr>=(uint8_t *)map[i].uncached &&
            (uint8_t *)addr<((uint8_t *)map[i].uncached + map[i].length) &&
            map[i].uncached != NULL) {
            return map[i].offset + ((uint8_t *)addr - (uint8_t *)map[i].uncached);
        }
    }
    BDBG_WRN(("NEXUS_AddrToOffset: unknown address %p", addr));
    for(i=0;i<sizeof(g_NEXUS_P_MemoryMap)/sizeof(*g_NEXUS_P_MemoryMap);i++) {
        if(map[i].length==0) {
            break;
        }
        BDBG_LOG(("%u: %p -> %p[%p]:%u", i, addr, map[i].cached, map[i].uncached, (unsigned)map[i].length));
    }
    return 0;
}

NEXUS_AddrType NEXUS_GetAddrType(const void *addr)
{
    unsigned i;
    const NEXUS_P_MemoryMap *map = g_NEXUS_P_MemoryMap;
    for(i=0;i<sizeof(g_NEXUS_P_MemoryMap)/sizeof(*g_NEXUS_P_MemoryMap);i++) {
        if(map[i].length==0) {
            break;
        }
        if( (uint8_t *)addr>=(uint8_t *)map[i].cached &&
            (uint8_t *)addr<((uint8_t *)map[i].cached + map[i].length) &&
            map[i].cached != NULL
            ) {
            return map[i].cachedAddrType;
        }
        if( (uint8_t *)addr>=(uint8_t *)map[i].uncached &&
            (uint8_t *)addr<((uint8_t *)map[i].uncached + map[i].length) &&
            map[i].uncached != NULL) {
            return map[i].uncachedAddrType;
        }
    }
    return NEXUS_AddrType_eUnknown;
}

void *
NEXUS_OffsetToCachedAddr(NEXUS_Addr offset)
{
    unsigned i;
    const NEXUS_P_MemoryMap *map = g_NEXUS_P_MemoryMap;
    for(i=0;i<sizeof(g_NEXUS_P_MemoryMap)/sizeof(*g_NEXUS_P_MemoryMap);i++) {
        if(map[i].length==0) {
            break;
        }
        if( offset>=map[i].offset &&
            offset<(map[i].offset + map[i].length)) {
            uint8_t *addr;

            if(map[i].cached) {
                addr = map[i].cached;
            } else {
                addr = map[i].uncached;
            }
            return addr + (offset - map[i].offset);
        }
    }
    BDBG_WRN(("NEXUS_OffsetToCachedAddr: unknown address " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(offset)));
    for(i=0;i<sizeof(g_NEXUS_P_MemoryMap)/sizeof(*g_NEXUS_P_MemoryMap);i++) {
        if(map[i].length==0) {
            break;
        }
        BDBG_LOG(("%u: " BDBG_UINT64_FMT " -> " BDBG_UINT64_FMT ":%u", i, BDBG_UINT64_ARG(offset), BDBG_UINT64_ARG(map[i].offset), (unsigned)map[i].length));
    }
    return NULL;
}

void *
NEXUS_OffsetToUncachedAddr(NEXUS_Addr offset)
{
    unsigned i;
    const NEXUS_P_MemoryMap *map = g_NEXUS_P_MemoryMap;

    for(i=0;i<sizeof(g_NEXUS_P_MemoryMap)/sizeof(*g_NEXUS_P_MemoryMap);i++) {
        if(map[i].length==0) {
            break;
        }
        if( offset>=map[i].offset &&
            offset<(map[i].offset + map[i].length)) {
            return (uint8_t *)map[i].uncached + (offset - map[i].offset);
        }
    }
    BDBG_WRN(("NEXUS_OffsetToUncachedAddr: unknown address %#lx", (unsigned long)offset));
    return NULL;
}

void
NEXUS_P_MapInit(void)
{
    unsigned i;
    NEXUS_P_MemoryMap *map = g_NEXUS_P_MemoryMap;
    for(i=0;i<sizeof(g_NEXUS_P_MemoryMap)/sizeof(*g_NEXUS_P_MemoryMap);i++) {
        map[i].length = 0;
        map[i].offset = 0;
        map[i].cached = NULL;
        map[i].uncached = NULL;
    }
    return ;
}

