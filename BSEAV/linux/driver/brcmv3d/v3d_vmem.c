/*
 * Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software")
 * to deal in the software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * them Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTIBILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <asm/barrier.h>
#include "v3d_drv.h"

/*
 * v3d_hw_virtual_mem implementation
 */
int v3d_vmem_init(struct v3d_hw_virtual_mem *vmem, struct drm_device *dev)
{
	const size_t pt_size = V3D_HW_PAGE_TABLE_SIZE;
	struct gen_pool *pool;
	u32 *pt;
	dma_addr_t dma_handle;
	int err;

	/* Do nothing if already created */
	if (vmem->virtual_pool)
		return 0;

	pool = gen_pool_create(PAGE_SHIFT, -1);
	if (!pool)
		return -ENOMEM;

	err = gen_pool_add(pool,
			   V3D_HW_VIRTUAL_ADDR_BASE,
			   V3D_HW_VIRTUAL_ADDR_SIZE - V3D_HW_VIRTUAL_ADDR_BASE,
			   -1);

	if (err)
		goto pool_out;

	DRM_DEBUG_DRIVER("Created virtual pool min_order=%d, base=0x%p, size=0x%zx\n",
			 pool->min_alloc_order,
			 (void *)V3D_HW_VIRTUAL_ADDR_BASE,
			 (size_t)V3D_HW_VIRTUAL_ADDR_SIZE);

	/*
	 * Allocate the pagetable from the global linux cma pool, not from
	 * the Broadcom CMA devices. They do not have a valid dma mask
	 * set and are not intended to be used with the standard Linux
	 * interfaces. As we need to have a kernel mapping for the pagetables
	 * we must use the proper API, so we are forced to allocate from
	 * the default global pool which can be resized from the 16MB default
	 * size using the "cma=" kernel boot parameter.
	 */
	pt = dma_zalloc_coherent(dev->dev, pt_size, &dma_handle, GFP_KERNEL);
	if (!pt) {
		err = -ENOMEM;
		goto pool_out;
	}

	vmem->virtual_pool = pool;
	vmem->pt_kaddr = pt;
	vmem->pt_phys = (phys_addr_t)dma_handle;
	vmem->clear_entries_on_free = true;

	DRM_DEBUG_DRIVER("Created pagetable @ %pa (entries %zu, %zu bytes)\n",
			 &vmem->pt_phys,
			 (size_t)V3D_HW_PAGE_TABLE_ENTRIES, pt_size);

	if (WARN_ON((unsigned long)pt & (V3D_HW_PAGE_TABLE_ALIGN - 1))) {
		err = -ENOMEM;
		goto pool_out;
	}

	return 0;

pool_out:
	DRM_DEBUG_DRIVER("Failed to pagetable from CMA\n");
	gen_pool_destroy(pool);
	return err;
}

void v3d_vmem_destroy(struct v3d_hw_virtual_mem *vmem, struct drm_device *dev)
{
	DRM_DEBUG_DRIVER("vmem=0x%pK drm_device=0x%pK dev=0x%pK\n",
			 vmem, dev, dev ? dev->dev : NULL);

	if (vmem->pt_kaddr) {
		dma_free_coherent(dev->dev, V3D_HW_PAGE_TABLE_SIZE,
				  vmem->pt_kaddr,
				  (dma_addr_t)vmem->pt_phys);
		vmem->pt_kaddr = NULL;
	}

	if (vmem->virtual_pool) {
		gen_pool_destroy(vmem->virtual_pool);
		vmem->virtual_pool = NULL;
	}
}

unsigned long v3d_vmem_allocate(struct v3d_hw_virtual_mem *vmem, size_t size)
{
	return gen_pool_alloc(vmem->virtual_pool, size);
}

void v3d_vmem_free(struct v3d_hw_virtual_mem *vmem,
		   unsigned long addr, size_t size)
{
	if (vmem->clear_entries_on_free) {
		int c = (size >> PAGE_SHIFT) * V3D_HW_PAGE_TABLE_ENTRY_SIZE;

		memset(vmem->pt_kaddr + (addr >> PAGE_SHIFT), 0, c);
		dma_wmb();
	}

	gen_pool_free(vmem->virtual_pool, addr, size);
}

void v3d_vmem_add_pages_to_pagetable(struct v3d_hw_virtual_mem *vmem,
				     struct device *dev,
				     dma_addr_t *dma_addrs, size_t pages,
				     u32 va, bool readonly)
{
	u32 *page = vmem->pt_kaddr + (va >> PAGE_SHIFT);
	u32 page_flags = V3D_PAGE_FLAG_VALID;
	int i;

	if (!readonly)
		page_flags |= V3D_PAGE_FLAG_WRITE;

	for (i = 0; i < pages; i++) {
		phys_addr_t pa = dma_to_phys(dev, dma_addrs[i]);

		*page++ = (u32)(pa >> PAGE_SHIFT) | page_flags;
	}

	dma_wmb();
}

void v3d_vmem_add_extobj_to_pagetable(struct v3d_hw_virtual_mem *vmem,
				      phys_addr_t pa, size_t pages,
				      u32 va, bool readonly)
{
	u32 *page = vmem->pt_kaddr + (va >> PAGE_SHIFT);
	u32 page_flags = V3D_PAGE_FLAG_VALID;
	int i;

	if (!readonly)
		page_flags |= V3D_PAGE_FLAG_WRITE;

	pa &= V3D_HW_PAGE_MASK;

	for (i = 0; i < pages; i++) {
		*page++ = (u32)(pa >> PAGE_SHIFT) | page_flags;
		pa += PAGE_SIZE;
	}

	dma_wmb();
}
