/*
 * Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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

#include <linux/list.h>

#include "v3d_drv.h"

/*
 * Broadcom CMA device block allocation management
 */
static inline bool v3d_alloc_block_full(struct v3d_alloc_block *b)
{
	return b->n_allocs == V3D_CMA_ALLOC_MAP_SIZE;
}

static int v3d_cmadev_get_mem_unmapped(struct v3d_drm_dev_private *dp,
				       u64 *addr, int cmadev,
				       size_t size, size_t align)
{
	DRM_DEBUG_ALLOC("cma_dev[%d]=0x%pK, size=%zu, align=%zu\n",
			cmadev, dp->cma_devs[cmadev], size, align);

	if (WARN_ON(cmadev >= dp->nr_cma_devs) ||
	    WARN_ON(!dp->cma_devs[cmadev]))
		return -ENOMEM;

	return cma_dev_get_mem(dp->cma_devs[cmadev], addr, size, align);
}

static int v3d_cma_get_mem_unmapped(struct v3d_drm_dev_private *dp,
				    phys_addr_t *phys, int *cmadev,
				    size_t size, size_t align)
{
	int i, err;
	u64 addr;

	/*
	 * cmadev contains the preferred device, but if that is out of memory
	 * try and find something in another device if we have more than one.
	 */
	for (i = 0; i < dp->nr_cma_devs; i++) {
		int dev_nr = (*cmadev + i) % dp->nr_cma_devs;

		err = v3d_cmadev_get_mem_unmapped(dp, &addr, dev_nr,
						  size, align);
		if (!err) {
			*cmadev = dev_nr;
			*phys = (phys_addr_t)addr;
			return 0;
		}
	}
	return -ENOMEM;
}

static inline
void v3d_cma_put_mem_unmapped(struct v3d_drm_dev_private *dp,
			      int cmadev, phys_addr_t phys, size_t size)
{
	cma_dev_put_mem(dp->cma_devs[cmadev], phys, size);
}

static int v3d_add_cma_alloc_block(struct v3d_drm_file_private *fp,
				   struct v3d_alloc_block **new_block)
{
	struct v3d_drm_dev_private *dp = fp->dev->dev_private;
	int cmadev = fp->next_alloc_device;
	struct v3d_alloc_block *block;
	phys_addr_t phys;
	int err;

	block = kzalloc(sizeof(*block), GFP_KERNEL);
	if (!block)
		return -ENOMEM;

	INIT_LIST_HEAD(&block->node);

	/* Try and alloc from the indicated CMA device, but if the device
	 * is out of memory any device with available memory will be used and
	 * cmadev updated to reflect which actual device the allocation came
	 * from.
	 */
	err = v3d_cma_get_mem_unmapped(dp, &phys, &cmadev,
				       V3D_CMA_ALLOC_BLOCK_SIZE,
				       V3D_CMA_ALLOC_BLOCK_SIZE);
	if (err)
		goto block_out;

	block->phys_addr = phys;
	block->cma_dev = cmadev;

	list_add_tail(&block->node, &fp->alloc_blocks[cmadev]);
	fp->all_blocks_full[cmadev] = false;
	fp->next_alloc_device = cmadev;

	DRM_DEBUG_ALLOC("new block @ %pa (%zu bytes)\n",
			&block->phys_addr,
			(size_t)V3D_CMA_ALLOC_BLOCK_SIZE);

	*new_block = block;
	return 0;

block_out:
	kfree(block);
	return -ENOMEM;
}

static inline void
v3d_release_cma_alloc_block(struct v3d_drm_file_private *fp,
			    struct v3d_drm_dev_private *dp,
			    int dev)
{
	struct v3d_alloc_block *itr, *itr_tmp;

	list_for_each_entry_safe(itr, itr_tmp,
				 &fp->alloc_blocks[dev], node) {
		DRM_DEBUG_ALLOC("free CMA block @ %pa\n", &itr->phys_addr);

		v3d_cma_put_mem_unmapped(dp, itr->cma_dev, itr->phys_addr,
					 V3D_CMA_ALLOC_BLOCK_SIZE);

		list_del(&itr->node);
		kfree(itr);
	}
}

void v3d_release_cma_blocks(struct v3d_drm_file_private *fp,
			    struct v3d_drm_dev_private *dp)
{
	int i;

	for (i = 0; i < dp->nr_cma_devs; i++) {
		if (!list_empty(&fp->alloc_blocks[i]))
			v3d_release_cma_alloc_block(fp, dp, i);
	}
}

/*
 * Device page allocation from CMA blocks for GEM objects
 */
static inline int v3d_alloc_cma_page(struct v3d_alloc_block *alloc_block,
				     struct v3d_page_allocation *page)
{
	int block_page_nr;

	block_page_nr = find_first_zero_bit(alloc_block->alloc_map,
					    V3D_CMA_ALLOC_MAP_SIZE);

	set_bit(block_page_nr, alloc_block->alloc_map);

	page->cpu_pfn = (alloc_block->phys_addr >> PAGE_SHIFT) + block_page_nr;
	page->alloc_block = alloc_block;
	page->block_page_nr = block_page_nr;
	alloc_block->n_allocs++;
	return 0;
}

static inline int v3d_next_cma_dev(int cur_dev, struct v3d_drm_dev_private *dp)
{
	return (cur_dev + 1) % dp->nr_cma_devs;
}

static void v3d_init_allocation_blocks(struct v3d_drm_file_private *fp,
				       struct v3d_drm_dev_private *dp,
				       struct v3d_alloc_block **blocks)
{
	int i;

	for (i = 0; i < dp->nr_cma_devs; i++) {
		blocks[i] = fp->all_blocks_full[i] ?
				NULL : v3d_first_alloc_block(fp, i);
	}
}

static inline bool v3d_is_last_alloc_block(struct v3d_alloc_block *block,
					   struct v3d_drm_file_private *fp)
{
	return list_is_last(&block->node, &fp->alloc_blocks[block->cma_dev]);
}

static struct v3d_alloc_block *
v3d_find_alloc_block_with_space(struct v3d_drm_file_private *fp,
				struct v3d_alloc_block **alloc_blocks)
{
	struct v3d_alloc_block *block = alloc_blocks[fp->next_alloc_device];

	while (true) {
		if (!block || fp->all_blocks_full[block->cma_dev]) {
			if (v3d_add_cma_alloc_block(fp, &block))
				return NULL;

			alloc_blocks[block->cma_dev] = block;
			return block;
		}

		if (!v3d_alloc_block_full(block))
			return block;

		if (v3d_is_last_alloc_block(block, fp)) {
			/* Trigger allocation of a new block */
			fp->all_blocks_full[block->cma_dev] = true;
		} else {
			/* Try the next block in the list for the CMA device */
			block = list_next_entry(block, node);
			alloc_blocks[block->cma_dev] = block;
		}
	}
}

static inline void v3d_free_cma_page(struct v3d_drm_file_private *fp,
				     struct v3d_page_allocation *page)
{
	page->alloc_block->n_allocs--;
	clear_bit(page->block_page_nr, page->alloc_block->alloc_map);
	fp->all_blocks_full[page->alloc_block->cma_dev] = false;
}

void v3d_free_cma_pages(struct v3d_drm_file_private *fp, size_t num_pages,
			struct v3d_page_allocation *pages)
{
	size_t i;

	for (i = 0; i < num_pages; i++) {
		v3d_free_cma_page(fp, &pages[i]);
		/*
		 * TODO: determine if/when we release empty alloc blocks
		 * back to the system.
		 */
		pages[i].alloc_block = NULL;
	}
}

int v3d_allocate_cma_pages(struct v3d_drm_file_private *fp,
			   size_t num_pages,
			   struct v3d_page_allocation *pages)
{
	struct v3d_drm_dev_private *dp = fp->dev->dev_private;
	struct v3d_alloc_block *alloc_blocks[MAX_CMA_AREAS];
	struct v3d_alloc_block *block;
	int alloc_idx, err;

	if (WARN_ON(!num_pages))
		return 0;

	v3d_init_allocation_blocks(fp, dp, alloc_blocks);

	for (alloc_idx = 0; alloc_idx < num_pages; alloc_idx++) {
		block = v3d_find_alloc_block_with_space(fp, alloc_blocks);
		if (!block)
			goto out;

		err = v3d_alloc_cma_page(block, &pages[alloc_idx]);
		if (err)
			goto out;

		/* Switch CMA device (if more than one) for the next page
		 * to stripe allocations between memory controllers
		 */
		fp->next_alloc_device = v3d_next_cma_dev(block->cma_dev, dp);
	}

	/*
	 * If we have just allocated the last free page in the last block
	 * then flag this so the next object allocation doesn't have to
	 * traverse the entire block list just to find out there is no
	 * space available.
	 */
	if (v3d_is_last_alloc_block(block, fp) && v3d_alloc_block_full(block))
		fp->all_blocks_full[block->cma_dev] = true;

	DRM_DEBUG_ALLOC("allocated %zu pages\n", (size_t)num_pages);

	return 0;

out:
	/*
	 * Cleanup partially allocated pages
	 */
	v3d_free_cma_pages(fp, alloc_idx, pages);
	return -ENOMEM;
}
