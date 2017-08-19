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
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <generic_delay_timer.h>
#include <mmio.h>
#include <xlat_tables.h>
#include <plat_private.h>
#include <platform.h>
#include <bcm_plat_common.h>
#include <bcm97268a0_def.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted memory
 ******************************************************************************/
#define BL31_END (unsigned long)(&__BL31_END__)

/*
 * The following constants identify the extents of the code & read-only data
 * regions. These addresses are used by the MMU setup code and therefore they
 * must be page-aligned.
 *
 * When the code and read-only data are mapped as a single atomic section
 * (i.e. when SEPARATE_CODE_AND_RODATA=0) then we treat the whole section as
 * code by specifying the read-only data section as empty.
 *
 * BL1 is different than the other images in the sense that its read-write data
 * originally lives in Trusted ROM and needs to be relocated in Trusted SRAM at
 * run-time. Therefore, the read-write data in ROM can be mapped with the same
 * memory attributes as the read-only data region. For this reason, BL1 uses
 * different macros.
 *
 * Note that BL1_ROM_END is not necessarily aligned on a page boundary as it
 * just points to the end of BL1's actual content in Trusted ROM. Therefore it
 * needs to be rounded up to the next page size in order to map the whole last
 * page of it with the right memory attributes.
 */
#if SEPARATE_CODE_AND_RODATA
#define BL31_BOOTSTRAP_BASE	(unsigned long)(&__BOOTSTRAP_START__)
#define BL31_BOOTSTRAP_LIMIT	(unsigned long)(&__BOOTSTRAP_END__)
#define BL31_CODE_BASE		(unsigned long)(&__TEXT_START__)
#define BL31_CODE_LIMIT		(unsigned long)(&__TEXT_END__)
#define BL31_RO_DATA_BASE	(unsigned long)(&__RODATA_START__)
#define BL31_RO_DATA_LIMIT	(unsigned long)(&__RODATA_END__)
#else
#define BL31_BOOTSTRAP_BASE	(unsigned long)(&__RO_START__)
#define BL31_BOOTSTRAP_LIMIT	(unsigned long)(&__RO_END__)
#define BL31_CODE_BASE		(unsigned long)(&__RO_START__)
#define BL31_CODE_LIMIT		(unsigned long)(&__RO_END__)
#define BL31_RO_DATA_BASE	0
#define BL31_RO_DATA_LIMIT	0
#endif /* SEPARATE_CODE_AND_RODATA */
#define BL31_STACKS_BASE	(unsigned long)(&__STACKS_START__)
#define BL31_STACKS_LIMIT	(unsigned long)(&__STACKS_END__)

#if USE_COHERENT_MEM
/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)
#endif

static entry_point_info_t bl32_ep_info;
static entry_point_info_t bl33_ep_info;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ? &bl33_ep_info : &bl32_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */

	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Perform any BL3-1 early platform setup. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables.
 * BL2 has flushed this information to memory, so we are guaranteed to pick up
 * good data.
 ******************************************************************************/
void bl31_early_platform_setup(bl31_params_t *from_bl2,
			       void *plat_params_from_bl2)
{
	uint32_t rw;

	console_init(BCM97268A0_UART0_BASE, BCM97268A0_UART_CLOCK, BCM97268A0_BAUDRATE);

	VERBOSE("bl31_setup\n");


	assert(from_bl2 != NULL);
	assert(from_bl2->h.type == PARAM_BL31);
	assert(from_bl2->h.version >= VERSION_1);

	bl32_ep_info = *from_bl2->bl32_ep_info;
	bl33_ep_info = *from_bl2->bl33_ep_info;

	rw = (bl33_ep_info.spsr >> MODE_RW_SHIFT) & MODE_RW_MASK;
	bl33_ep_info.spsr = plat_get_spsr_for_bl33_entry(rw);

	VERBOSE("bl31_setup exit\n");
}

/*******************************************************************************
 * Perform any BL31 platform setup code
 ******************************************************************************/
void bl31_platform_setup(void)
{
	VERBOSE("bl31_platform_setup\n");
	generic_delay_timer_init();

	/* Initialize the gic cpu and distributor interfaces */
	plat_bcm_gic_init();

	/* Topologies are best known to the platform. */
	bcm_setup_topology();

}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
__attribute__((section(".text.bootstrap"))) \
void bl31_plat_arch_setup(intptr_t bl31_offset)
{
	VERBOSE("bl31_plat_arch_setup\n");

	mmap_add_region(BL31_BOOTSTRAP_BASE, BL31_BOOTSTRAP_BASE,
			BL31_BOOTSTRAP_LIMIT - BL31_BOOTSTRAP_BASE,
			MT_CODE | MT_SECURE);

	mmap_add_region(BL31_STACKS_BASE, BL31_STACKS_BASE,
			BL31_STACKS_LIMIT - BL31_STACKS_BASE,
			MT_MEMORY | MT_RW | MT_SECURE);

	uintptr_t bl31_load_base = BL31_BASE + bl31_offset;
	plat_setup_mmu(bl31_offset,
		       bl31_load_base,
		       BL31_END - bl31_load_base,
		       BL31_CODE_BASE,
		       BL31_CODE_LIMIT,
		       BL31_RO_DATA_BASE,
		       BL31_RO_DATA_LIMIT
#if USE_COHERENT_MEM
		       , BL31_COHERENT_RAM_BASE,
		       BL31_COHERENT_RAM_LIMIT
#endif
		      );
#if 0
	{
		uint64_t cpu_data_ptr;
		__asm__ volatile("mrs	%[cdp], tpidr_el3" : [cdp] "=r" (cpu_data_ptr));
		cpu_data_ptr -= bl31_offset;
		__asm__ volatile("msr	tpidr_el3, %[cdp]" : : [cdp] "r" (cpu_data_ptr));
	}
#endif
}
