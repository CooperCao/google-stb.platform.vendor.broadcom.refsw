/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>
#include <string.h>
#include <utils.h>
#include <xlat_tables.h>

uintptr_t page_align(uintptr_t value, unsigned dir)
{
	/* Round up the limit to the next page boundary */
	if (value & (PAGE_SIZE - 1)) {
		value &= ~(PAGE_SIZE - 1);
		if (dir == UP)
			value += PAGE_SIZE;
	}

	return value;
}

static inline unsigned int is_page_aligned (uintptr_t addr) {
	return (addr & (PAGE_SIZE - 1)) == 0;
}

#ifndef NDEBUG
/******************************************************************************
 * Determine whether the memory region delimited by 'addr' and 'size' is free,
 * given the extents of free memory.
 * Return 1 if it is free, 0 if it is not free or if the input values are
 * invalid.
 *****************************************************************************/
static int is_mem_free(uintptr_t free_base, size_t free_size,
		uintptr_t addr, size_t size)
{
	uintptr_t free_end, requested_end;

	/*
	 * Handle corner cases first.
	 *
	 * The order of the 2 tests is important, because if there's no space
	 * left (i.e. free_size == 0) but we don't ask for any memory
	 * (i.e. size == 0) then we should report that the memory is free.
	 */
	if (size == 0)
		return 1;	/* A zero-byte region is always free */
	if (free_size == 0)
		return 0;

	/*
	 * Check that the end addresses don't overflow.
	 * If they do, consider that this memory region is not free, as this
	 * is an invalid scenario.
	 */
	if (check_uptr_overflow(free_base, free_size - 1))
		return 0;
	free_end = free_base + (free_size - 1);

	if (check_uptr_overflow(addr, size - 1))
		return 0;
	requested_end = addr + (size - 1);

	/*
	 * Finally, check that the requested memory region lies within the free
	 * region.
	 */
	return (addr >= free_base) && (requested_end <= free_end);
}
#endif /* NDEBUG */

/******************************************************************************
 * Inside a given memory region, determine whether a sub-region of memory is
 * closer from the top or the bottom of the encompassing region. Return the
 * size of the smallest chunk of free memory surrounding the sub-region in
 * 'small_chunk_size'.
 *****************************************************************************/
static unsigned int choose_mem_pos(uintptr_t mem_start, uintptr_t mem_end,
				  uintptr_t submem_start, uintptr_t submem_end,
				  size_t *small_chunk_size)
{
	size_t top_chunk_size, bottom_chunk_size;

	assert(mem_start <= submem_start);
	assert(submem_start <= submem_end);
	assert(submem_end <= mem_end);
	assert(small_chunk_size != NULL);

	top_chunk_size = mem_end - submem_end;
	bottom_chunk_size = submem_start - mem_start;

	if (top_chunk_size < bottom_chunk_size) {
		*small_chunk_size = top_chunk_size;
		return TOP;
	} else {
		*small_chunk_size = bottom_chunk_size;
		return BOTTOM;
	}
}

/******************************************************************************
 * Reserve the memory region delimited by 'addr' and 'size'. The extents of free
 * memory are passed in 'free_base' and 'free_size' and they will be updated to
 * reflect the memory usage.
 * The caller must ensure the memory to reserve is free and that the addresses
 * and sizes passed in arguments are sane.
 *****************************************************************************/
void reserve_mem(uintptr_t *free_base, size_t *free_size,
		 uintptr_t addr, size_t size)
{
	size_t discard_size;
	size_t reserved_size;
	unsigned int pos;

	assert(free_base != NULL);
	assert(free_size != NULL);
	assert(is_mem_free(*free_base, *free_size, addr, size));

	if (size == 0) {
		WARN("Nothing to allocate, requested size is zero\n");
		return;
	}

	pos = choose_mem_pos(*free_base, *free_base + (*free_size - 1),
			     addr, addr + (size - 1),
			     &discard_size);

	reserved_size = size + discard_size;
	*free_size -= reserved_size;

	if (pos == BOTTOM)
		*free_base = addr + size;

	VERBOSE("Reserved 0x%zx bytes (discarded 0x%zx bytes %s)\n",
	     reserved_size, discard_size,
	     pos == TOP ? "above" : "below");
}

/*******************************************************************************
 * Print the content of an entry_point_info_t structure.
 ******************************************************************************/
void print_entry_point_info(const entry_point_info_t *ep_info)
{
	INFO("Entry point address = %p\n", (void *)ep_info->pc);
	INFO("SPSR = 0x%x\n", ep_info->spsr);

#define PRINT_IMAGE_ARG(n)					\
	VERBOSE("Argument #" #n " = 0x%llx\n",			\
		(unsigned long long) ep_info->args.arg##n)

	PRINT_IMAGE_ARG(0);
	PRINT_IMAGE_ARG(1);
	PRINT_IMAGE_ARG(2);
	PRINT_IMAGE_ARG(3);
	PRINT_IMAGE_ARG(4);
	PRINT_IMAGE_ARG(5);
	PRINT_IMAGE_ARG(6);
	PRINT_IMAGE_ARG(7);
#undef PRINT_IMAGE_ARG
}
