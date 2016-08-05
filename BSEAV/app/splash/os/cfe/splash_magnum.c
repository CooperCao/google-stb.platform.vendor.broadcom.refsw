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
 *
 * Module Description:
 *
 ***************************************************************************/
#include "splash_magnum.h"
#include "lib_printf.h"

BERR_Code BMEM_ConvertAddressToOffset(BMEM_Handle heap, void* addr, uint32_t* offset)
{
	if ( (uint32_t)addr >= 0xe0000000 )
	{
#if BCHP_CHIP == 7420
		*offset = ((uint32_t)addr - 0xe0000000)+0x60000000 ;
#else /*BCHP_CHIP == 7425*/
		*offset = ((uint32_t)addr - 0xe0000000)+0x90000000 ;
#endif
	}
	else
	{
		*offset = K1_TO_PHYS(((uint32_t)addr));
	}

	BDBG_MSG(("BMEM : Virtual %p Physical %08x", addr, *offset));
	return 0 ;
}

#define MEMORY_BASE				((void *)PHYS_TO_K1(0x5800000))

static uint8_t *ui32CurrentStaticMemoryPointer = MEMORY_BASE ;
static uint8_t *ui32CurrentStaticMemoryBase = MEMORY_BASE ;
static uint32_t ui32StaticMemorySize = 8*1024*1024 ;

/* BMVD_P_AlignAddress : Aligns the address to the specified bit position
 */
uint32_t AlignAddress(
		uint32_t	ui32Address,	/* [in] size in bytes of block to allocate */
		unsigned int uiAlignBits	/* [in] alignment for the block */
		)
{
	return (ui32Address+((1<<uiAlignBits)-1)) & ~((1<<uiAlignBits)-1) ;
}


void *BMEM_AllocAligned
(
	BMEM_Handle       pheap,       /* Heap to allocate from */
	size_t            ulSize,      /* size in bytes of block to allocate */
	unsigned int      uiAlignBits, /* alignment for the block */
	unsigned int      Boundary     /* boundry restricting allocated value */
)
{
	uint32_t pAllocMem ;
	uint32_t ui32_adjSize ;

	ui32_adjSize = ulSize + (1<<uiAlignBits)-1 ;

	/* The simple static memory allocator works as follows :
	 * The running pointer always points to the starting address
	 * of the remaining and free portion of the static memory block.
	 */
	pAllocMem = (uint32_t)ui32CurrentStaticMemoryPointer ;

	/* Point the running pointer to the end+1 of the current buffer *
	 * but first check if the static memory block is not exhausted */
	if( (ui32CurrentStaticMemoryPointer+ui32_adjSize) >
		(ui32CurrentStaticMemoryBase+ui32StaticMemorySize) )
	{
		BDBG_ERR(("BMEM_AllocAligned : No more Memory available"));
		return NULL;
	}

	/* Align the address to the specified Bit position */
	pAllocMem = AlignAddress(pAllocMem, uiAlignBits) ;

	ui32CurrentStaticMemoryPointer += ui32_adjSize;

	BDBG_MSG(("Allocated Memory : %08lx, size %08lx, prealigned = %08lx", pAllocMem, ulSize, ui32CurrentStaticMemoryPointer ));
	return (void *)pAllocMem ;
}
BERR_Code BMEM_Heap_ConvertAddressToCached(
		BMEM_Handle Heap,
		void *pvAddress,
		void **ppvCachedAddress)
{
	*ppvCachedAddress = pvAddress;
	return 0;
}
BERR_Code BMEM_Heap_FlushCache(
		BMEM_Handle Heap,
		void *pvCachedAddress,
		size_t size)
{
	return 0;
}

/* End of File */
