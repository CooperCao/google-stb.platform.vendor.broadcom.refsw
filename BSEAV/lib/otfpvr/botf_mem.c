/***************************************************************************
 *	   Copyright (c) 2007-2013, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Simple virtual memory module, it's used to convert between physical and virtual memory
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "botf_mem.h"
#include "bmem.h"
#include "bkni_multi.h"
#include "botf_priv.h"

void 
botf_mem_init(botf_mem *mem, uint32_t addr, void *uncached_ptr, size_t range, struct BOTF_Data *otf)
{
    void *ptr;
    BERR_Code rc;

    rc = BMEM_Heap_ConvertAddressToCached(otf->hBMem, uncached_ptr, &ptr);
    BDBG_ASSERT(rc==BERR_SUCCESS);

    mem->base = (uint32_t)ptr - addr;
    mem->uncached_ptr= uncached_ptr;
    mem->ptr= ptr;
    mem->addr = addr;
    mem->range = range;
    mem->otf = otf;
    return;
}

uint32_t 
botf_mem_paddr(botf_mem_t mem, const void *ptr)
{
    BDBG_ASSERT((const uint8_t *)ptr >= mem->ptr && (const uint8_t *)ptr < (mem->ptr + mem->range));
    return (uint32_t)ptr - mem->base;
}

void *
botf_mem_vaddr(botf_mem_t mem, uint32_t addr)
{
    BDBG_ASSERT(addr >= mem->addr && addr < (mem->addr + mem->range));
    return (uint8_t *)mem->base + addr;
}

void botf_mem_flush(botf_mem_t mem, const void *ptr, size_t len)
{
    BERR_Code rc;

    rc=BMEM_Heap_FlushCache(mem->otf->hBMem, (void *)ptr, len);
    BDBG_ASSERT(rc==BERR_SUCCESS);
    return;
}

