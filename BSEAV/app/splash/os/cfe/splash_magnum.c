/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#include "splash_magnum.h"
#include "lib_printf.h"


/* Tuning */
#define MEMORY_BASE				((void *)PHYS_TO_K1(0x5800000))
static uint8_t *ui64CurrentStaticMemoryPointer = MEMORY_BASE ;
static uint8_t *ui64CurrentStaticMemoryBase = MEMORY_BASE ;
static uint64_t ui64StaticMemorySize = 8*1024*1024 ;

static uint64_t AlignAddress(
    uint64_t	ui64Address,	/* [in] size in bytes of block to allocate */
    unsigned    alignment      	/* [in] alignment for the block */
);

#error CHEAP
BMMA_Block_Handle BMMA_Alloc
(
    BMMA_Heap_Handle heap,
    size_t size,
    unsigned alignment,
    const BMMA_AllocationSettings *pSettings
)
{
	uint64_t pAllocMem ;
	uint64_t ui64_adjSize ;

	ui64_adjSize = size + alignment-1 ;

	/* The simple static memory allocator works as follows :
	 * The running pointer always points to the starting address
	 * of the remaining and free portion of the static memory block.
	 */
	pAllocMem = (uint64_t)ui64CurrentStaticMemoryPointer ;

	/* Point the running pointer to the end+1 of the current buffer *
	 * but first check if the static memory block is not exhausted */
	if( (ui64CurrentStaticMemoryPointer+ui64_adjSize) >
		(ui64CurrentStaticMemoryBase+ui64StaticMemorySize) )
	{
		BDBG_ERR(("BMMA_Alloc : No more Memory available"));
		return NULL;
	}

	/* Align the address to the specified Bit position */
	pAllocMem = AlignAddress(pAllocMem, alignment) ;

	ui64CurrentStaticMemoryPointer += ui64_adjSize;

	BDBG_MSG(("Allocated Memory : %08lx, size %08lx, prealigned = %08lx", pAllocMem, size, ui64CurrentStaticMemoryPointer ));
	return (BMMA_Block_Handle)pAllocMem ;
}

static uint64_t AlignAddress(
    uint64_t	ui64Address,
    unsigned    alignment
)
{
	return (ui64Address+(alignment-1)) & ~(alignment-1) ;
}

void* BMMA_Lock
(
    BMMA_Block_Handle block
)
{
    return (void*)block;
}

BMMA_DeviceOffset BMMA_LockOffset
(
    BMMA_Block_Handle block
)
{
        BMMA_DeviceOffset offset;
	if ( (uint64_t)block >= 0xe0000000 )
	{
		offset = ((uint64_t)block - 0xe0000000)+0x90000000 ;
	}
	else
	{
		offset = K1_TO_PHYS(((uint64_t)block));
	}

	BDBG_MSG(("BMMA : Virtual %p Physical %08x", block, offset));
	return offset ;
}

void BMMA_Unlock
(
    BMMA_Block_Handle block,
    const void *addr
)
{
}

void BMMA_UnlockOffset
(
    BMMA_Block_Handle block,
    BMMA_DeviceOffset offset
)
{
}

void BMMA_FlushCache
(
    BMMA_Block_Handle block,
    const void *addr,
    size_t size
)
{
}

/* End of File */
