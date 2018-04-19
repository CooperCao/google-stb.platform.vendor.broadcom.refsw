/***************************************************************************
 *  Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 * Simple virtual memory module, it's used to convert between physical and virtual memory
 *
 ***************************************************************************/

#include "bstd.h"
#include "botf_mem.h"
#include "bkni_multi.h"
#include "botf_priv.h"

BDBG_MODULE(botf_mem);

void
botf_mem_init(botf_mem *mem, BSTD_DeviceOffset addr, void *cached_ptr, uint64_t range, struct BOTF_Data *otf, void (*FlushCache)(const void *, size_t))
{
    mem->ptr = cached_ptr;
    mem->addr = addr;
    mem->range = range;
    mem->otf = otf;
    mem->FlushCache = FlushCache;
    BDBG_MSG(("mem:%p ptr:%p..%p addr:" BDBG_UINT64_FMT " .. " BDBG_UINT64_FMT "", (void *)mem, cached_ptr, (uint8_t *)cached_ptr + range, BDBG_UINT64_ARG(addr), BDBG_UINT64_ARG(addr+range)));
    return;
}

BSTD_DeviceOffset
botf_mem_paddr(botf_mem_t mem, const void *ptr)
{
    BDBG_ASSERT((const uint8_t *)ptr >= mem->ptr && (const uint8_t *)ptr < (mem->ptr + mem->range));
    return mem->addr + ((uint8_t *)ptr - (uint8_t *)mem->ptr);
}

void *
botf_mem_vaddr(botf_mem_t mem, BSTD_DeviceOffset addr)
{
    BDBG_ASSERT(addr >= mem->addr && addr < (mem->addr + mem->range));
    return (void *)(mem->ptr + (addr - mem->addr));
}

void botf_mem_flush(botf_mem_t mem, const void *ptr, size_t len)
{
    mem->FlushCache(ptr, len);
    return;
}

