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
 ***************************************************************************/

#include "bchp_sun_top_ctrl.h"
#include "common.h"
#include "arm.h"
#include "cache_ops.h"

/*******************************************************************
 * Function: i_cache_config
 * Description: enable/disable instruction cache
 * Arguments: i_cache_en - 1: enable, 0: disable
 * Returns: none
 *******************************************************************/
void i_cache_config(int i_cache_en)
{
	unsigned int tmp;

	__asm__ __volatile__("mrc " SCTLR("%0") : "=r" (tmp));

	if (i_cache_en)
		tmp |= SCTLR_ICACHE_ENABLE;
	else
		tmp &= ~SCTLR_ICACHE_ENABLE;

	__asm__ __volatile__("mcr " SCTLR("%0") : : "r" (tmp));

	invalidate_all_i_cache();

	BARRIER();
}

/*******************************************************************
 * Function: enable_caches
 * Description: enable L1 i-Cache/d-Cache, L2 unified cache,
 * and MMU (translation table must be set up)
 * Arguments: none
 * Returns: none
 *******************************************************************/
void enable_caches(void)
{
	unsigned long tmp, r1 = SCTLR_MMU_ENABLE |
		SCTLR_DUCACHE_ENABLE | SCTLR_ICACHE_ENABLE;

	__asm__ __volatile__(
		"mrc " SCTLR("%0") "\n"
		"orr %0, %0, %1\n"
		"mcr " SCTLR("%0") "\n"
		: "=&r" (tmp)
		: "r" (r1)
	);
}

/*******************************************************************
 * Function: disable_caches
 * Description: disable L1 i-Cache/d-Cache, L2 unified cache, and MMU
 * Arguments: none
 * Returns: none
 *******************************************************************/
void disable_caches(void)
{
	unsigned long tmp, r1 = ~(SCTLR_MMU_ENABLE |
		SCTLR_DUCACHE_ENABLE | SCTLR_ICACHE_ENABLE);

	__asm__ __volatile__(
		"mrc " SCTLR("%0") "\n"
		"and %0, %0, %1\n"
		"mcr " SCTLR("%0") "\n"
		: "=&r" (tmp)
		: "r" (r1)
	);
	BARRIER(); /* swbolt-797 */
}

/*******************************************************************
 * Function: invalidate_all_i_cache
 * Description: invalidate all instruction lines in both
 * L1 instruction cache and L2 unified cache
 * Arguments: none
 * Returns: none
 *******************************************************************/
void invalidate_all_i_cache(void)
{
	__asm__ __volatile__("mcr " ICIALLU("%0") "\n" : : "r" (0));

	/* invalidate branch predictor since i-cache lines are invalidated */
	__asm__ __volatile__("mcr " BPIALL("%0") "\n" : : "r" (0));

	BARRIER();
}

/*******************************************************************
 * Function: flush_cache
 * Description: perform cache maintenance
 * Arguments: level - cache level to perform maintenance on
 *            do_clean - 1: clean+invalidate, 0: invalidate only
 * Returns: none
 *******************************************************************/
void flush_cache(int level, int do_clean)
{
	const uint32_t id = get_ccsidr(level, 0);
	const int linesz = CACHE_LINE_SIZE(id);
	const unsigned int ways = CACHE_NUM_WAYS(id);
	const unsigned int way_bits = fls(ways) - 1;
	const unsigned int sets = CACHE_NUM_SETS(id);

	unsigned int sl, way;

	for (way = 0; way < ways; way++) {
		const unsigned int max_sl = sets * linesz;

		for (sl = 0; sl < max_sl; sl += linesz) {
			const uint32_t arg = ((level - 1) << 1) | sl;

			if (do_clean)
				do_dccisw(arg, way, way_bits);
			else
				do_dcisw(arg, way, way_bits);
		}
	}

	BARRIER();
}

/*******************************************************************
 * Function: clear_all_d_cache
 * Description: clean (write back) and invalidate all data
 * cache lines, first on L1 D-cache, then on L2 cache
 * Arguments: none
 * Returns: none
 *******************************************************************/
void clear_all_d_cache(void)
{
	dmb(); /* swbolt-797 */
	flush_cache(1, 1); /* L1 d-cache */
	flush_cache(2, 1); /* L2 cache */
}

/*******************************************************************
 * Function: invalidate_d_cache
 * Description: invalidate (no writeback) data cache lines in L1 D-cache and L2
 *     cache by virtual address and range
 *
 * Arguments: addr - virtual address (must be L1 line size aligned)
 *            len  - size in bytes (must be L1 line size aligned)
 * Returns: none
 *******************************************************************/
void invalidate_d_cache(void *addr, unsigned int len)
{
	uintptr_t lmask, line, addr_p = (uintptr_t)addr,
		addr_e = (uintptr_t)addr + len;

	line = get_l1_dcache_line_size();
	lmask = line - 1;

	/* Before we invalidate an area of memory check that
	the start and end addresses are cache line aligned,
	and if not clean the cache line immediately below the
	start address and/or immediately above the end address. Then
	adjust the start or end to be cache line aligned, if required.
	 This is done because for non cache line aligned addresses
	we may be invalidating adjacent memory allocated for something
	else and we clean it (commit to main store/dram) just in case,
	before we go throwing the cache	lines in the trash. */
	if (addr_p & lmask) {
		/* step down to next lowest cache line
		aligned memory and clean it. */
		addr_p &= ~lmask;
		__asm__ __volatile__(
			"mcr " DCCIMVAC("%0") "\n"
			"dsb\n" /* dsb not dmb, related to swbolt-797 */
			: : "r" (addr_p) :
		);
	}

	/* If we don't end on a cache line. */
	if (addr_e & lmask) {
		/* step down */
		addr_e &= ~lmask;
		__asm__ __volatile__(
			"mcr " DCCIMVAC("%0") "\n"
			"dsb\n"
			: : "r" (addr_e) :
		);
		/* align up to next highest cache line aligned address. */
		addr_e += line;
	}

	/* Now we can go nuking our lines without worrying about
	next door. The most that will happen to allocations either side
	of us is that they will have to be reloaded into the cache. */
	while (addr_p < addr_e) {
		__asm__ __volatile__(
			"mcr " DCIMVAC("%0") "\n"
			: : "r" (addr_p)
		);
		addr_p += line;
	}
	dsb(); /* swbolt-797 */
}

/*******************************************************************
 * Function: clear_d_cache
 * Description: clean (write back) and invalidate data cache lines in L1
 *     D-cache and L2 cache by virtual address and range
 * Arguments: addr - virtual address (must be L1 line size aligned)
 *            len  - size in bytes (must be L1 line size aligned)
 * Returns: none
 *******************************************************************/
void clear_d_cache(void *addr, unsigned int len)
{
	uintptr_t line, addr_e, addr_p = (uintptr_t)addr;

	line = get_l1_dcache_line_size();
	addr_e = ALIGN_UP_TO(addr_p + len, line);
	addr_p &= ~(line - 1);

	while (addr_p < addr_e) {
		__asm__ __volatile__(
			"mcr " DCCIMVAC("%0") "\n"
			"dmb\n"
			: : "r" (addr_p) :
		);
		addr_p += line;
	}
	dsb(); /* swbolt-797 */
}
