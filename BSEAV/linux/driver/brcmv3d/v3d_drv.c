/*
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom.
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

/**
 * This is a GEM memory manager for Broadcom set-top-box SoCs using the
 * Broadcom V3D graphics driver.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <linux/pfn.h>
#include <linux/dma-buf.h>
#include <linux/list.h>
#include <uapi/drm/brcmv3d_drm.h>

#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include <asm/barrier.h>

#include "v3d_drv.h"

#define DRIVER_NAME	"brcmv3d"
#define DRIVER_DESC	"Broadcom V3D GEM provider"
#define DRIVER_DATE	"20161207"
#define DRIVER_MAJOR	1
#define DRIVER_MINOR	1

/*
 * Add a new debug level for verbose information from the memory allocations
 * while we are developing.
 */
#define DRM_UT_ALLOC 0x100
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 9)
#define DRM_DEBUG_ALLOC(fmt, args...)                                   \
	do {                                                            \
		if (unlikely(drm_debug & DRM_UT_ALLOC))                 \
			drm_ut_debug_printk(__func__, fmt, ##args);     \
	} while (0)
#else
#define DRM_DEBUG_ALLOC(fmt, ...)					\
	drm_printk(KERN_DEBUG, DRM_UT_ALLOC, fmt, ##__VA_ARGS__)
#endif

/*
 * General helpers
 */
static inline loff_t v3d_nr_hwpages(size_t size)
{
	BUG_ON((size & ~V3D_HW_PAGE_MASK) != 0);
	return size >> V3D_HW_PAGE_SHIFT;
}

static inline bool v3d_alloc_block_full(struct v3d_alloc_block *b)
{
	return ~(b->alloc_map) == 0;
}

static int v3d_cmadev_get_mem_unmapped(struct v3d_drm_dev_private *dp,
				       u64 *addr, int cmadev,
				       size_t size, size_t align)
{
	int err;

	DRM_DEBUG_ALLOC("cma_dev[%d]=0x%pK, size=%zu, align=%zu\n",
			cmadev, dp->cma_devs[cmadev], size, align);

	err = cma_dev_get_mem(dp->cma_devs[cmadev], addr, size, align);
	if (!err) {
		u64 lastaddr = *addr + size - 1;

		/*
		 * If the HW cannot address this memory,
		 * release it and return an error for this cma dev
		 */
		if ((lastaddr & ~dp->mmu_dma_mask) != 0) {
			cma_dev_put_mem(dp->cma_devs[cmadev], *addr, size);
			return -ENOMEM;
		}
	}
	return err;
}

static int v3d_cma_get_mem_unmapped(struct v3d_drm_dev_private *dp,
				    phys_addr_t *phys, int *cmadev,
				    size_t size, size_t align)
{
	int i, err;
	u64 addr;

	/*
	 * TODO: Implement some sort of smart balancing of memory between
	 * CMA areas of multiple MEMCs ?
	 */
	for (i = 0; i < MAX_CMA_AREAS; i++) {
		if (dp->cma_devs[i]) {
			err = v3d_cmadev_get_mem_unmapped(dp, &addr, i,
							  size, align);
			if (!err) {
				*cmadev = i;
				*phys = (phys_addr_t)addr;
				return 0;
			}
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

/*
 * v3d_hw_virtual_mem implementation
 */
static inline
void v3d_hw_pagetable_clear_range(uint32_t *pt, uint32_t hw_vaddr, size_t size)
{
	loff_t pg_offset = hw_vaddr >> V3D_HW_SMALLEST_PAGE_SHIFT;
	int npages = size >> V3D_HW_SMALLEST_PAGE_SHIFT;

	memset(pt + pg_offset, 0, npages * V3D_HW_PAGE_TABLE_ENTRY_SIZE);

	/* No cache flush for dma coherent memory, but it should be buffered */
	dma_wmb();
}

static void v3d_hw_pagetable_map_page(uint32_t *pt, uint32_t hw_vaddr,
				      phys_addr_t hw_physaddr, bool readonly)
{
	const int n = V3D_HW_PAGE_SHIFT > V3D_HW_SMALLEST_PAGE_SHIFT ? 16 : 1;
	loff_t pg_offset = hw_vaddr >> V3D_HW_SMALLEST_PAGE_SHIFT;
	uint32_t *page = pt + pg_offset;
	uint32_t entry;
	int i;

	entry = (hw_physaddr >> V3D_HW_SMALLEST_PAGE_SHIFT);

	/*
	 * The hardware can potentially handle 40bit physical output addresses
	 * which when shifted down by the 4K page size occupy the low bits
	 * of the pagetable entry.
	 *
	 * Note that not all the instantiations of the hardware actually
	 * output the top four bits, limiting them to generating 32bit
	 * addresses only.
	 */
	BUG_ON((entry & V3D_PAGE_FLAG_MASK) != 0);

	if (readonly)
		entry |= V3D_HW_DEFAULT_PAGE_FLAGS;
	else
		entry |= V3D_HW_DEFAULT_PAGE_FLAGS | V3D_PAGE_FLAG_WRITE;

	/*
	 * When using 64k "bigpages" we do not have to populate each
	 * table entry with a different 4K physical page address, but
	 * it makes it easier to switch the MMU back to 4K pages for debug
	 * experiments.
	 */
	for (i = 0; i < n; i++)
		*page++ = entry++;

	/* No cache flush for dma coherent memory, but it should be buffered */
	dma_wmb();
}

static int v3d_hw_virtual_mem_init(struct v3d_hw_virtual_mem *hw_vmem,
				   struct drm_device *dev)
{
	const size_t pt_size = V3D_HW_PAGE_TABLE_SIZE;
	struct gen_pool *pool;
	uint32_t *pt;
	dma_addr_t dma_handle;
	int err;

	pool = gen_pool_create(V3D_HW_PAGE_SHIFT, -1);
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

	hw_vmem->virtual_pool = pool;
	hw_vmem->pt_kaddr = pt;
	hw_vmem->pt_phys = (phys_addr_t)dma_handle;
	hw_vmem->clear_entries_on_free = true;

	DRM_DEBUG_DRIVER("Created pagetable @ %pa (entries %zu, %zu bytes)\n",
			 &hw_vmem->pt_phys,
			 (size_t)V3D_HW_PAGE_TABLE_ENTRIES, pt_size);

	BUG_ON((unsigned long)pt & (V3D_HW_PAGE_TABLE_ALIGN - 1));

	return 0;

pool_out:
	gen_pool_destroy(pool);
	return err;
}

static void v3d_hw_virtual_mem_destroy(struct v3d_hw_virtual_mem *hw_vmem,
				       struct drm_device *dev)
{
	DRM_DEBUG_DRIVER("hw_vmem=0x%pK drm_device=0x%pK dev=0x%pK\n",
			 hw_vmem, dev, dev ? dev->dev : NULL);

	if (hw_vmem->pt_kaddr) {
		dma_free_coherent(dev->dev, V3D_HW_PAGE_TABLE_SIZE,
				  hw_vmem->pt_kaddr,
				  (dma_addr_t)hw_vmem->pt_phys);
		hw_vmem->pt_kaddr = NULL;
	}

	if (hw_vmem->virtual_pool) {
		gen_pool_destroy(hw_vmem->virtual_pool);
		hw_vmem->virtual_pool = NULL;
	}
}

static inline
unsigned long v3d_hw_virtual_mem_allocate(struct v3d_hw_virtual_mem *hw_vmem,
					  size_t size)
{
	return gen_pool_alloc(hw_vmem->virtual_pool, size);
}

static inline
void v3d_hw_virtual_mem_free(struct v3d_hw_virtual_mem *hw_vmem,
			     unsigned long addr,
			     size_t size)
{
	if (hw_vmem->clear_entries_on_free) {
		v3d_hw_pagetable_clear_range(hw_vmem->pt_kaddr,
					     addr, size);
	}

	return gen_pool_free(hw_vmem->virtual_pool, addr, size);
}

/*
 * v3d_drm_file_private implementation
 */
static int v3d_drm_file_private_add_block(struct v3d_drm_file_private *fp)
{
	struct v3d_alloc_block *block;
	phys_addr_t phys;
	int cmadev = 0, err;

	block = kzalloc(sizeof(*block), GFP_KERNEL);
	if (!block)
		return -ENOMEM;

	INIT_LIST_HEAD(&block->node);

	/*
	 * To fit in with the rest of the Broadcom driver CMA+DTU usage
	 * we allocate 2MB blocks on a 2MB alignment.
	 */
	err = v3d_cma_get_mem_unmapped(fp->dev->dev_private, &phys, &cmadev,
				       V3D_CMA_ALLOC_BLOCK_SIZE,
				       V3D_CMA_ALLOC_BLOCK_SIZE);
	if (err)
		goto block_out;

	block->phys_addr = phys;
	block->cma_dev = cmadev;

	list_add_tail(&block->node, &fp->alloc_blocks);
	fp->all_blocks_full = false;

	DRM_DEBUG_ALLOC("new block @ %pa (%zu bytes)\n",
			&block->phys_addr,
			(size_t)V3D_CMA_ALLOC_BLOCK_SIZE);

	return 0;

block_out:
	kfree(block);
	return -ENOMEM;
}

static struct v3d_drm_file_private *
v3d_drm_file_private_create(struct drm_device *dev)
{
	static uint64_t magic_ids = 0xdeadc1e500000000ULL;
	int err;
	struct v3d_drm_file_private *fp;

	fp = kzalloc(sizeof(*fp), GFP_KERNEL);
	if (!fp)
		return ERR_PTR(-ENOMEM);

	kref_init(&fp->refcount);

	INIT_LIST_HEAD(&fp->dead_fp);
	/* The DRM device struct_mutex is held at this point, so this is safe */
	fp->magic_id = magic_ids++;

	INIT_LIST_HEAD(&fp->alloc_blocks);
	/* Force the first block to be allocated on the first object create */
	fp->all_blocks_full = true;

	err = v3d_hw_virtual_mem_init(&fp->hw_vmem, dev);
	if (err)
		goto out;

	fp->dev = dev;
	return fp;

out:
	kfree(fp);
	return ERR_PTR(err);
}

static void v3d_drm_file_private_release(struct kref *kref)
{
	struct v3d_drm_file_private *fp = (struct v3d_drm_file_private *)kref;
	struct v3d_alloc_block *itr, *itr_tmp;

	DRM_DEBUG_DRIVER("fp=0x%pK\n", fp);

	v3d_hw_virtual_mem_destroy(&fp->hw_vmem, fp->dev);

	if (!list_empty(&fp->alloc_blocks)) {
		list_for_each_entry_safe(itr, itr_tmp,
					 &fp->alloc_blocks, node) {
			DRM_DEBUG_ALLOC("free CMA block @ %pa\n",
					&itr->phys_addr);

			v3d_cma_put_mem_unmapped(fp->dev->dev_private,
						 itr->cma_dev,
						 itr->phys_addr,
						 V3D_CMA_ALLOC_BLOCK_SIZE);

			list_del(&itr->node);
			kfree(itr);
		}
	}

	kfree(fp);
}

static inline void
v3d_drm_file_private_reference(struct v3d_drm_file_private *fp)
{
	kref_get(&fp->refcount);
}

static inline void
v3d_drm_file_private_unreference(struct v3d_drm_file_private *fp)
{
	if (fp)
		kref_put(&fp->refcount, v3d_drm_file_private_release);
}

/*
 * v3d_drm_dev_private implementation
 */
static bool
v3d_drm_dev_is_cma_device_valid(struct cma_dev *dev)
{
	int err;
	phys_addr_t phys;

	err = cma_dev_get_mem(dev, &phys,
			      V3D_CMA_ALLOC_BLOCK_SIZE,
			      V3D_CMA_ALLOC_BLOCK_SIZE);

	if (err) {
		DRM_DEBUG_DRIVER("Ignoring CMA dev 0x%pK: No free memory\n",
				 dev);
		return false;
	}

#if defined(CONFIG_ARM)
	{
		struct page *pg = pfn_to_page(phys >> PAGE_SHIFT);

		/*
		 * We do not allow allocations from a cma region in lowmem
		 * on 32-bit ARM as you hit the multiple page mapping with
		 * different attributes architecture restriction between
		 * the lowmem kernel mapping and a userspace mapping.
		 *
		 * This is no longer an issue on ARM64, where all memory
		 * has a cached kernel mapping and additional non-cached
		 * CPU mappings of DMA memory are expected.
		 */
		if (pg && !PageHighMem(pg)) {
			DRM_DEBUG_DRIVER("Ignoring CMA dev 0x%pK: located in lowmem\n", dev);
			cma_dev_put_mem(dev, phys, V3D_CMA_ALLOC_BLOCK_SIZE);
			return false;
		}
	}
#endif

	cma_dev_put_mem(dev, phys, V3D_CMA_ALLOC_BLOCK_SIZE);
	return true;
}

static
struct v3d_drm_dev_private *v3d_drm_dev_private_create(struct drm_device *dev)
{
	struct v3d_drm_dev_private *dp;
	bool found_cma_dev = false;
	int i;

	dp = kzalloc(sizeof(*dp), GFP_KERNEL);
	if (!dp)
		return ERR_PTR(-ENOMEM);

	kref_init(&dp->refcount);
	INIT_LIST_HEAD(&dp->dead_fps);
	INIT_LIST_HEAD(&dp->dead_clients);

	dp->mmu_dma_mask = dev->dev->coherent_dma_mask;
	DRM_DEBUG_DRIVER("DMA Mask %#llx\n", dp->mmu_dma_mask);

	for (i = 0; i < MAX_CMA_AREAS; i++) {
		struct cma_dev *dev = cma_dev_get_cma_dev(i);

		if (dev) {
			phys_addr_t base = dev->range.base;

			DRM_DEBUG_DRIVER("Found CMA dev 0x%pK for MEMC%d (base %pa, size 0x%x)\n",
					 dev, dev->memc, &base,
					 dev->range.size);

			if (v3d_drm_dev_is_cma_device_valid(dev))
				found_cma_dev = true;
			else
				dev = NULL;
		}
		dp->cma_devs[i] = dev;
	}

	if (found_cma_dev)
		return dp;

	kfree(dp);
	return ERR_PTR(-ENODEV);
}

static
void v3d_drm_dev_private_flush_dead_clients(struct v3d_drm_dev_private *dp)
{

	DRM_DEBUG_DRIVER("dp=0x%pK\n", dp);

	if (!list_empty(&dp->dead_fps)) {
		struct v3d_drm_file_private *itr, *itr_tmp;

		list_for_each_entry_safe(itr, itr_tmp,
					 &dp->dead_fps, dead_fp) {
			list_del(&itr->dead_fp);
			/*
			 * This will finally cleanup the CMA allocations
			 * hanging off the file private structure. If the
			 * hardware is somehow still accessing the memory at
			 * this point then it is just tough luck.
			 */
			v3d_drm_file_private_unreference(itr);
		}
	}

	if (!list_empty(&dp->dead_clients)) {
		struct v3d_drm_dead_client *itr, *itr_tmp;

		list_for_each_entry_safe(itr, itr_tmp,
					 &dp->dead_clients, node) {
			list_del(&itr->node);
			kfree(itr);
		}
	}
}

static void v3d_drm_dev_private_term_client(struct v3d_drm_dev_private *dp,
					    uint64_t id)
{
	struct v3d_drm_dead_client *dead_client;

	DRM_DEBUG_DRIVER("dp=0x%pK\n", dp);

	if (!list_empty(&dp->dead_fps)) {
		struct v3d_drm_file_private *itr, *itr_tmp;

		list_for_each_entry_safe(itr, itr_tmp,
					 &dp->dead_fps, dead_fp) {
			if (itr->magic_id == id) {
				list_del(&itr->dead_fp);
				v3d_drm_file_private_unreference(itr);
				return;
			}
		}
	}

	/* We haven't seen this client id die yet, so we must assume
	 * it will do so at some point in the near future and we need to
	 * record that it should be cleaned up immediately and not deferred.
	 */
	dead_client = kmalloc(sizeof(*dead_client), GFP_KERNEL);

	if (!dead_client)
		return; /* If the system is this short of memory, tough luck */

	INIT_LIST_HEAD(&dead_client->node);
	dead_client->magic_id = id;
	list_add_tail(&dp->dead_clients, &dead_client->node);
}

/*
 * Allow the V3D magnum driver (built as a kernel module) to indicate a
 * client has finally terminated in the Nexus/Magnum world and that it is
 * now safe to release CMA allocations the hardware might have been
 * accessing back to the system.
 *
 * We would have liked to have used drm_minor_acquire/release to get hold
 * of the appropriate drm_device, but those interfaces are not exported
 * (even in the latest kernel tree), so it is global variable time.
 */
static struct drm_device *the_drm_device;

void v3d_drm_term_client(uint64_t id)
{
	struct v3d_drm_dev_private *dp;

	if (the_drm_device) {
		mutex_lock(&the_drm_device->struct_mutex);

		dp = the_drm_device->dev_private;
		v3d_drm_dev_private_term_client(dp, id);

		mutex_unlock(&the_drm_device->struct_mutex);
	}
}

/* symbol for bvc5 module to get hold of.  Compiler only inserts relative
branches, so trampoline via a uintptr_t */
uintptr_t v3d_drm_term_client_p = (uintptr_t)&v3d_drm_term_client;
EXPORT_SYMBOL(v3d_drm_term_client_p);

static void v3d_drm_dev_private_release(struct kref *kref)
{
	struct v3d_drm_dev_private *dp = (struct v3d_drm_dev_private *)kref;

	if (dp) {
		v3d_drm_dev_private_flush_dead_clients(dp);
		kfree(dp);
	}
}

static inline
void v3d_drm_dev_private_reference(struct v3d_drm_dev_private *dp)
{
	kref_get(&dp->refcount);
}

static inline
void v3d_drm_dev_private_unreference(struct v3d_drm_dev_private *dp)
{
	if (dp)
		kref_put(&dp->refcount, v3d_drm_dev_private_release);
}

/*
 * GEM object implementation
 */
static inline void v3d_free_hw_page(struct v3d_page_allocation *page)
{
	BUG_ON(!page->alloc_block);
	page->alloc_block->alloc_map &= ~BIT(page->block_page_nr);
}

static void v3d_free_hw_pages(loff_t num_pages,
			      struct v3d_page_allocation *pages)
{
	int i;

	BUG_ON(!pages);

	for (i = 0; i < num_pages; i++) {
		v3d_free_hw_page(&pages[i]);
		/*
		 * TODO: determine if/when we release empty alloc blocks
		 * back to the system.
		 */
		pages[i].alloc_block = NULL;
	}
}

static int v3d_alloc_hw_page(struct v3d_alloc_block *alloc_block,
			     struct v3d_page_allocation *page)
{
	unsigned block_page_nr;
	unsigned long paddr;

	block_page_nr = ffz(alloc_block->alloc_map);
	alloc_block->alloc_map |= BIT(block_page_nr);

	paddr = alloc_block->phys_addr + (block_page_nr << V3D_HW_PAGE_SHIFT);
	/*
	 * Get the Linux pfn for this physical address. If there are multiple
	 * (continuous) CPU pages per HW page then the mmap fault handler will
	 * calculate the correct sub-page pfn for a page fault from this.
	 *
	 * Note: PHYS_PFN macro not yet defined in 4.1 (is in 4.4)
	 */
	page->cpu_pfn = paddr >> PAGE_SHIFT;
	page->alloc_block = alloc_block;
	page->block_page_nr = block_page_nr;
	return 0;
}

static int v3d_allocate_hw_pages(struct v3d_drm_file_private *fp,
				 loff_t num_pages,
				 struct v3d_page_allocation *pages)
{
	struct v3d_alloc_block *alloc_block;
	int alloc_idx = 0, err;

	BUG_ON(!pages);
	BUG_ON(!num_pages);

	alloc_block = fp->all_blocks_full ? NULL : v3d_first_alloc_block(fp);

	while (alloc_idx < num_pages) {
		if (fp->all_blocks_full) {
			err = v3d_drm_file_private_add_block(fp);
			if (err)
				goto out;

			alloc_block = v3d_last_alloc_block(fp);
		} else {
			if (v3d_alloc_block_full(alloc_block)) {
				if (list_is_last(&alloc_block->node,
						 &fp->alloc_blocks)) {
					fp->all_blocks_full = true;
				} else {
					alloc_block =
						list_next_entry(alloc_block,
								node);
				}
				/*
				 * Round the loop again until we have a block
				 * with some free space or run out of system
				 * memory.
				 */
				continue;
			}
		}

		err = v3d_alloc_hw_page(alloc_block, &pages[alloc_idx]);
		if (err)
			goto out;

		alloc_idx++;
	}

	/*
	 * If we have just allocated the last free page in the last block
	 * then flag this we the next object allocation doesn't have to
	 * traverse the entire block list just to find out there is no
	 * space available.
	 */
	if (list_is_last(&alloc_block->node, &fp->alloc_blocks) &&
	    v3d_alloc_block_full(alloc_block)) {
		fp->all_blocks_full = true;
	}

	DRM_DEBUG_ALLOC("allocated %zu pages\n", (size_t)num_pages);

	return 0;

out:
	/*
	 * Cleanup partially allocated pages
	 */
	v3d_free_hw_pages(alloc_idx, pages);
	return -ENOMEM;
}

static void v3d_gem_put_pages(struct v3d_drm_gem_object *obj)
{
	/* Be safe against cleaning up a partially constructed object */
	if (obj->pages) {
		loff_t num_pages = v3d_nr_hwpages(obj->base.size);

		v3d_free_hw_pages(num_pages, obj->pages);
		vfree(obj->pages);
		obj->pages = NULL;
	}
}

static void v3d_gem_free_object(struct drm_gem_object *obj)
{
	struct v3d_drm_gem_object *v3d_obj = to_v3d_obj(obj);
	struct v3d_hw_virtual_mem *hw_vmem;

	DRM_DEBUG_ALLOC("obj=0x%pK\n", obj);

	/* Note: this is called with obj->dev->struct_mutex held */
	drm_gem_free_mmap_offset(obj);
	drm_gem_object_release(obj);

	BUG_ON(!v3d_obj->fp);
	hw_vmem = &v3d_obj->fp->hw_vmem;

	v3d_hw_virtual_mem_free(hw_vmem, v3d_obj->hw_virt_addr, obj->size);
	v3d_obj->hw_virt_addr = 0;

	v3d_gem_put_pages(v3d_obj);
	v3d_drm_file_private_unreference(v3d_obj->fp);
	kfree(v3d_obj->desc);
	kfree(v3d_obj);
}

static void v3d_gem_add_pages_to_pagetable(struct v3d_drm_gem_object *obj)
{
	struct v3d_drm_file_private *fp = obj->fp;
	const loff_t num_pages = v3d_nr_hwpages(obj->base.size);
	const bool readonly = !!(obj->alloc_flags & V3D_CREATE_HW_READONLY);
	int i;

	for (i = 0; i < num_pages; i++) {
		phys_addr_t phys;
		uint32_t virt;
		loff_t phys_pgoff;

		virt = obj->hw_virt_addr + (i << V3D_HW_PAGE_SHIFT);
		phys_pgoff = obj->pages[i].block_page_nr << V3D_HW_PAGE_SHIFT;
		phys = obj->pages[i].alloc_block->phys_addr + phys_pgoff;

		v3d_hw_pagetable_map_page(fp->hw_vmem.pt_kaddr,
					  virt, phys, readonly);
	}
}

static void v3d_gem_add_extobj_to_pagetable(struct v3d_drm_gem_object *obj,
					    phys_addr_t phys)
{
	struct v3d_drm_file_private *fp = obj->fp;
	const loff_t num_pages = v3d_nr_hwpages(obj->base.size);
	const bool readonly = !!(obj->alloc_flags & V3D_CREATE_HW_READONLY);
	uint32_t virt;
	int i;

	phys &= V3D_HW_PAGE_MASK;
	virt = obj->hw_virt_addr;

	for (i = 0; i < num_pages; i++) {
		v3d_hw_pagetable_map_page(fp->hw_vmem.pt_kaddr,
					  virt, phys, readonly);

		phys += V3D_HW_PAGE_SIZE;
		virt += V3D_HW_PAGE_SIZE;
	}
}

static int v3d_gem_get_pages(struct v3d_drm_gem_object *obj)
{
	struct v3d_page_allocation *pages;
	struct v3d_drm_file_private *fp = obj->fp;
	size_t size = obj->base.size;
	loff_t num_pages = v3d_nr_hwpages(size);
	int err;

	BUG_ON(obj->pages);
	BUG_ON(!fp);

	pages = vmalloc(sizeof(*pages) * num_pages);
	if (!pages)
		return -ENOMEM;

	/*
	 * First try and allocate a region in the hardware virtual space
	 */
	obj->hw_virt_addr = v3d_hw_virtual_mem_allocate(&fp->hw_vmem, size);
	if (!obj->hw_virt_addr) {
		err = -ENOMEM;
		goto out;
	}

	err = v3d_allocate_hw_pages(fp, num_pages, pages);
	if (err)
		goto hw_virtual;

	obj->pages = pages;
	v3d_gem_add_pages_to_pagetable(obj);

	return 0;

hw_virtual:
	v3d_hw_virtual_mem_free(&fp->hw_vmem, obj->hw_virt_addr, size);
	obj->hw_virt_addr = 0;
out:
	vfree(pages);
	return err;
}

static const struct vm_operations_struct v3d_gem_vm_ops = {
	.open  = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};

static struct drm_gem_object *
v3d_gem_create(struct drm_device *dev,
	       struct drm_file *file,
	       struct drm_v3d_gem_create *args)
{
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct v3d_drm_gem_object *obj;
	struct drm_gem_object *gem_object;
	uint32_t alloc_size;
	int err;

	BUG_ON(!fp);

	alloc_size = roundup(args->size, V3D_HW_PAGE_SIZE);

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj)
		return ERR_PTR(-ENOMEM);

	gem_object = &obj->base;

	drm_gem_private_object_init(dev, gem_object, alloc_size);

	v3d_drm_file_private_reference(fp);
	obj->fp = fp;
	obj->alloc_flags = args->flags;

	if (args->desc != NULL) {
		obj->desc = strndup_user(args->desc, 256);
		/* If the description is too big, ignore it */
		if (IS_ERR(obj->desc))
			obj->desc = NULL;
	}

	err = v3d_gem_get_pages(obj);
	if (err)
		goto object_out;

	err = drm_gem_handle_create(file, gem_object, &args->handle);
	if (err)
		goto pages_out;

	drm_gem_object_unreference_unlocked(gem_object);

	args->size = alloc_size;
	args->hw_addr = obj->hw_virt_addr;

	DRM_DEBUG_ALLOC("handle=%u, obj=0x%pK, size=%u, hw_virt_addr=0x%x\n",
			args->handle, gem_object, args->size,
			obj->hw_virt_addr);

	return gem_object;

pages_out:
	v3d_hw_virtual_mem_free(&fp->hw_vmem,
				obj->hw_virt_addr, gem_object->size);

	v3d_gem_put_pages(obj);
object_out:
	v3d_drm_file_private_unreference(obj->fp);
	drm_gem_object_release(gem_object);
	kfree(obj->desc);
	kfree(obj);
	return ERR_PTR(err);
}

static struct drm_gem_object *
v3d_gem_create_ext(struct drm_device *dev,
		   struct drm_file *file,
		   struct drm_v3d_gem_create_ext *args)
{
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct v3d_drm_gem_object *obj;
	struct drm_gem_object *gem_object;
	uint32_t alloc_size, page_offset;
	int err;

	BUG_ON(!fp);

	/*
	 * We are not allowing the mapping of an empty object, that must
	 * be an application error.
	 */
	if (args->size == 0)
		return ERR_PTR(-EINVAL);

	/*
	 * We need to fit the object into the right position and number
	 * of 64k pages, we do not require that the external object's
	 * pages are aligned or sized to 64k boundaries.
	 */
	page_offset = args->phys & ~V3D_HW_PAGE_MASK;
	alloc_size = roundup(args->size + page_offset, V3D_HW_PAGE_SIZE);

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj)
		return ERR_PTR(-ENOMEM);

	gem_object = &obj->base;

	drm_gem_private_object_init(dev, gem_object, alloc_size);

	v3d_drm_file_private_reference(fp);
	obj->fp = fp;
	obj->alloc_flags = args->flags;
	if (args->desc != NULL)
		obj->desc = strndup_user(args->desc, 32);

	obj->hw_virt_addr = v3d_hw_virtual_mem_allocate(&fp->hw_vmem,
							alloc_size);
	if (!obj->hw_virt_addr) {
		err = -ENOMEM;
		goto object_out;
	}

	v3d_gem_add_extobj_to_pagetable(obj, args->phys);

	err = drm_gem_handle_create(file, gem_object, &args->handle);
	if (err)
		goto virtual_out;

	drm_gem_object_unreference_unlocked(gem_object);

	/*
	 * Return the real virtual start address for the physical
	 * address of the external object, not the beginning of the
	 * first hardware page.
	 */
	args->hw_addr = obj->hw_virt_addr + page_offset;

	DRM_DEBUG_ALLOC("handle=%u, obj=0x%pK, size=%u, hw_virt_addr=0x%x\n",
			args->handle, gem_object,
			args->size, args->hw_addr);

	return gem_object;

virtual_out:
	v3d_hw_virtual_mem_free(&fp->hw_vmem,
				obj->hw_virt_addr, gem_object->size);

object_out:
	v3d_drm_file_private_unreference(obj->fp);
	drm_gem_object_release(gem_object);
	kfree(obj->desc);
	kfree(obj);
	return ERR_PTR(err);
}

static int v3d_gem_map_offset(struct drm_file *file, struct drm_device *dev,
			      uint32_t handle, uint64_t *offset)
{
	struct drm_gem_object *obj;
	int ret = 0;

	mutex_lock(&dev->struct_mutex);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 9)
	obj = drm_gem_object_lookup(dev, file, handle);
#else
	obj = drm_gem_object_lookup(file, handle);
#endif

	if (!obj) {
		ret = -ENOENT;
		goto unlock;
	}

	/*
	 * Don't allow the mapping of external contiguous physical pages
	 * that have been wrapped as GEM objects to manage the MMU pagetable
	 */
	if (!to_v3d_obj(obj)->pages) {
		ret = -EPERM;
		goto unref;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 9)
	if (!drm_vma_node_has_offset(&obj->vma_node)) {
#endif
		ret = drm_gem_create_mmap_offset(obj);
		if (ret)
			goto unref;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 9)
	}
#endif

	*offset = drm_vma_node_offset_addr(&obj->vma_node);

	DRM_DEBUG_ALLOC("handle=%u, obj=0x%pK, offset=0x%llx\n",
			handle, obj, *offset);

unref:
	drm_gem_object_unreference(obj);
unlock:
	mutex_unlock(&dev->struct_mutex);
	return ret;
}

/*
 * Driver ioctls and file operations
 */
static int v3d_mmu_pagetable_ioctl(struct drm_device *dev, void *data,
				   struct drm_file *file)
{
	struct v3d_drm_file_private *fp;
	struct drm_v3d_mmu_pt *args = data;

	mutex_lock(&dev->struct_mutex);

	fp = file->driver_priv;
	BUG_ON(!fp);

	args->pt_phys = fp->hw_vmem.pt_phys;
	args->va_size = V3D_HW_VIRTUAL_ADDR_SIZE;

	mutex_unlock(&dev->struct_mutex);

	return 0;
}

static int v3d_clear_pt_on_close_ioctl(struct drm_device *dev, void *data,
				       struct drm_file *file)
{
	struct v3d_drm_file_private *fp;

	mutex_lock(&dev->struct_mutex);

	fp = file->driver_priv;
	BUG_ON(!fp);

	fp->clear_pagetable_on_close = true;

	mutex_unlock(&dev->struct_mutex);

	return 0;
}

static int v3d_mem_total_ioctl(struct drm_device *dev, void *data,
			       struct drm_file *file)
{
	struct v3d_drm_file_private *fp;
	struct v3d_drm_dev_private *dp;
	struct drm_v3d_mem_total *args = data;
	size_t total = 0;
	int i;

	mutex_lock(&dev->struct_mutex);

	fp = file->driver_priv;
	BUG_ON(!fp);

	dp = dev->dev_private;

	BUG_ON(dp != fp->dev->dev_private);

	for (i = 0; i < MAX_CMA_AREAS; i++) {
		if (dp->cma_devs[i] != NULL)
			total += dp->cma_devs[i]->range.size;
	}

	args->size = total;

	mutex_unlock(&dev->struct_mutex);

	return 0;
}

static int v3d_file_token_ioctl(struct drm_device *dev, void *data,
				struct drm_file *file)
{
	struct v3d_drm_file_private *fp;
	struct drm_v3d_file_private_token *args = data;

	fp = file->driver_priv;
	BUG_ON(!fp);

	args->token = fp->magic_id;
	return 0;
}

static int v3d_client_term_ioctl(struct drm_device *dev, void *data,
				 struct drm_file *file)
{
	struct v3d_drm_file_private *fp;
	struct v3d_drm_dev_private *dp;
	struct drm_v3d_file_private_token *args = data;

	mutex_lock(&dev->struct_mutex);

	fp = file->driver_priv;
	BUG_ON(!fp);

	dp = dev->dev_private;
	BUG_ON(dp != fp->dev->dev_private);

	v3d_drm_dev_private_term_client(dp, args->token);

	mutex_unlock(&dev->struct_mutex);

	return 0;
}

static int v3d_gem_create_ioctl(struct drm_device *dev, void *data,
				struct drm_file *file)
{
	struct drm_gem_object *gem_object;
	struct drm_v3d_gem_create *args = data;

	DRM_DEBUG_ALLOC("size=%zu\n", (size_t)args->size);

	mutex_lock(&dev->struct_mutex);

	gem_object = v3d_gem_create(dev, file, args);

	mutex_unlock(&dev->struct_mutex);

	if (IS_ERR(gem_object)) {
		DRM_DEBUG_ALLOC("object creation failed\n");
		return PTR_ERR(gem_object);
	}

	return 0;
}

static int v3d_gem_create_ext_ioctl(struct drm_device *dev, void *data,
				    struct drm_file *file)
{
	struct drm_gem_object *gem_object;
	struct drm_v3d_gem_create_ext *args = data;

	DRM_DEBUG_ALLOC("phys=%pa size=%zu\n",
			&args->phys, (size_t)args->size);

	if (args->flags & ~V3D_CREATE_HW_READONLY)
		return -EINVAL;

	mutex_lock(&dev->struct_mutex);

	gem_object = v3d_gem_create_ext(dev, file, args);

	mutex_unlock(&dev->struct_mutex);

	if (IS_ERR(gem_object)) {
		DRM_DEBUG_DRIVER("external object creation failed\n");
		return PTR_ERR(gem_object);
	}

	return 0;
}

static int v3d_gem_mmap_offset_ioctl(struct drm_device *dev, void *data,
				     struct drm_file *file)
{
	struct drm_v3d_gem_mmap_offset *args = data;

	DRM_DEBUG_ALLOC("handle=%u\n", args->handle);

	return v3d_gem_map_offset(file, dev, args->handle, &args->offset);
}


static int v3d_drm_gem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct v3d_drm_gem_object *obj;
	struct drm_device *dev;
	unsigned long vmsize = vma->vm_end - vma->vm_start;
	unsigned long remaining = vmsize;
	unsigned long addr = vma->vm_start;
	loff_t num_hw_pages = v3d_nr_hwpages(vmsize);
	int hwpg, ret;

	ret = drm_gem_mmap(filp, vma);
	if (ret)
		return ret;

	obj = to_v3d_obj(vma->vm_private_data); /* set by drm_gem_mmap() */
	dev = obj->base.dev;

	mutex_lock(&dev->struct_mutex);

	if (!(obj->alloc_flags & V3D_CREATE_CPU_WRITECOMBINE)) {
		/*
		 * Override the default drm_gem_mmap() page settings to map
		 * cached (no allocate on write) instead of writecombined
		 * on ARM and revert it to normal cached memory on ARM64.
		 *
		 * Note: this matches the Broadcom STB reference software
		 * heap mapping. In general we do not want to allocate
		 * on write for graphics object memory. Mostly we are going
		 * to be writing large amounts of sequential data into it
		 * (textures, vertex data etc) and never reading it back.
		 * But on ARM64 we appear to have no choice in the matter.
		 *
		 */
		vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
#if defined(CONFIG_ARM)
		vma->vm_page_prot = __pgprot_modify(vma->vm_page_prot,
						    L_PTE_MT_MASK,
						    L_PTE_MT_WRITEBACK);
#endif
#if defined(CONFIG_ARM64)
		vma->vm_page_prot = __pgprot_modify(vma->vm_page_prot,
						    PTE_ATTRINDX_MASK,
						    PTE_ATTRINDX(MT_NORMAL) | PTE_PXN | PTE_UXN);
#endif
	}

	/*
	 * Map the entire object up front. If we defer and use a page fault
	 * handler we seem to hit a problem if the application then tries
	 * to access the mapping from multiple pthreads, causing two page
	 * faults to the same page at the same time on different CPUs.
	 * The result is at least one locked CPU and a very unhappy kernel.
	 *
	 * This unfortunately happens regularly with the GLES driver, which
	 * optimizes texture downloads to device memory on an SMP system by
	 * splitting the copy and swizzle to our internal texture format
	 * across multiple threads.
	 */
	for (hwpg = 0; hwpg < num_hw_pages; hwpg++) {
		unsigned long pfn = obj->pages[hwpg].cpu_pfn;
		unsigned long sz = remaining > V3D_HW_PAGE_SIZE ?
				   V3D_HW_PAGE_SIZE : remaining;

		ret = remap_pfn_range(vma, addr, pfn, sz, vma->vm_page_prot);
		if (ret)
			break;
		addr += sz;
		remaining -= sz;
	}

	mutex_unlock(&dev->struct_mutex);
	return ret;
}

static int v3d_drm_open(struct drm_device *dev, struct drm_file *file)
{
	struct v3d_drm_file_private *fp;
	struct v3d_drm_dev_private *dp;
	bool created_dp = false;
	int err;

	/*
	 * We have to create the driver private structure containing information
	 * about the available Broadcom CMA devices on an open, not when the
	 * device is registered. We cannot strictly rely on the two drivers
	 * being initialized in the right order, i.e. the CMA driver first.
	 *
	 * Do not be tempted to think you can use firstopen() to create
	 * the driver private structure and cma device list, as it
	 * gets called _after_ the driver open() f_op and we need the
	 * Broadcom CMA devices available before we call
	 * v3d_drm_file_private_create() to create the MMU pagetable.
	 */
	mutex_lock(&dev->struct_mutex);
	if (!dev->dev_private) {
		dp = v3d_drm_dev_private_create(dev);
		if (!IS_ERR(dp)) {
			dev->dev_private = dp;
			created_dp = true;
			v3d_drm_dev_private_reference(dp);
			drm_dev_ref(dev);
			the_drm_device = dev;
		}
	} else {
		dp = dev->dev_private;
		v3d_drm_dev_private_reference(dp);
	}
	mutex_unlock(&dev->struct_mutex);

	if (IS_ERR(dp))
		return PTR_ERR(dp);

	fp = v3d_drm_file_private_create(dev);
	if (IS_ERR(fp)) {
		err = PTR_ERR(fp);
		goto out;
	}

	file->driver_priv = fp;

	/*
	 * We do not actually care if this succeeds or not
	 */
	v3d_debugfs_create_file_entries(file);

	DRM_DEBUG_DRIVER("fp=0x%pK\n", fp);

	return 0;

out:
	v3d_drm_dev_private_unreference(dp);

	/* We must clean-up the device private pointer here if we just set it */
	if (created_dp) {
		mutex_lock(&dev->struct_mutex);
		dev->dev_private = NULL;
		the_drm_device = NULL;
		drm_dev_unref(dev);
		v3d_drm_dev_private_unreference(dp);
		mutex_unlock(&dev->struct_mutex);
	}

	return err;
}

static void v3d_drm_preclose(struct drm_device *dev, struct drm_file *file)
{
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct v3d_drm_dev_private *dp = dev->dev_private;

	DRM_DEBUG_DRIVER("dp=0x%pK fp=0x%pK\n", dp, fp);

	/*
	 * To prevent issues with the hardware still using pages as a
	 * client is killed (as opposed to a clean shutdown) in a
	 * multi-3d app system, we disable pagetable clearing for
	 * any GEM objects that are about to be closed as part of the
	 * file descriptor close. The underlying memory for the objects
	 * and the pagetable will not get released back to CMA until
	 * we are informed it is really safe to do so via the magic
	 * client termination notification.
	 *
	 * But in a single process system, where we are not expecting to
	 * terminate without a clean shutdown (except in development),
	 * it is preferable to clear the pagetable entries to minimize
	 * the possibility of hardware writes to memory after it has
	 * been released via the lastclose callback.
	 *
	 * We have no way of knowing what to do, other than the platform
	 * integration telling us via the V3D_CLEAR_PT_ON_CLOSE IOCTL.
	 */
	if (!fp->clear_pagetable_on_close) {
		DRM_DEBUG_DRIVER("Disabled pagetable entry clear\n");
		fp->hw_vmem.clear_entries_on_free = false;
	}
}

static void v3d_drm_postclose(struct drm_device *dev, struct drm_file *file)
{
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct v3d_drm_dev_private *dp = dev->dev_private;
	bool terminated = false;

	DRM_DEBUG_DRIVER("dp=0x%pK fp=0x%pK\n", dp, fp);

	if (!fp)
		return;

	v3d_debugfs_cleanup_file_entries(file);

	mutex_lock(&dev->struct_mutex);

	/*
	 * As this is postclose, all the GEM objects private to this
	 * filedescriptor should have been deleted and this should be the
	 * last reference on the file private state.
	 *
	 * If the client has been marked as terminated then clean up the
	 * file private structure immediately, otherwise put it on the
	 * device private list of dead clients to clean up later.
	 */
	if (!list_empty(&dp->dead_clients)) {
		struct v3d_drm_dead_client *itr, *itr_tmp;

		list_for_each_entry_safe(itr, itr_tmp,
					 &dp->dead_clients, node) {
			if (itr->magic_id == fp->magic_id) {
				list_del(&itr->node);
				v3d_drm_file_private_unreference(fp);
				terminated = true;
				break;
			}
		}

	}

	if (!terminated)
		list_add_tail(&fp->dead_fp, &dp->dead_fps);

	mutex_unlock(&dev->struct_mutex);

	file->driver_priv = NULL;

	v3d_drm_dev_private_unreference(dp);
}

static void v3d_drm_lastclose(struct drm_device *dev)
{
	struct v3d_drm_dev_private *dp;

	mutex_lock(&dev->struct_mutex);

	dp = dev->dev_private;
	DRM_DEBUG_DRIVER("dp=0x%pK\n", dp);

	/* Cleaup the global device pointer used by the exported entrypoint
	 * used by the v3d Magnum driver when built as a kernel module.
	 *
	 * Note: lastclose gets called twice, once on a real "lastclose" of
	 * of the device and again if the device is unregistered just for good
	 * measure. So we have to be careful not to unref the device more than
	 * we should.
	 */
	if (the_drm_device) {
		BUG_ON(dev != the_drm_device);
		drm_dev_unref(the_drm_device);
		the_drm_device = NULL;
	}

	/* Release the final reference and destroy the structure */
	v3d_drm_dev_private_unreference(dp);

	/*
	 * Clear dev_private after the final cleanup as it gets used by
	 * the "file private" cleanup of any remaining dead clients
	 */
	dev->dev_private = NULL;

	mutex_unlock(&dev->struct_mutex);
}

static int v3d_drm_load(struct drm_device *dev, unsigned long flags)
{
	/* Just so we can clean up on module unload */
	platform_set_drvdata(dev->platformdev, dev);
	return 0;
}

static struct drm_ioctl_desc v3d_ioctls[] = {
	DRM_IOCTL_DEF_DRV(V3D_GET_MMU_PAGETABLE, v3d_mmu_pagetable_ioctl,
			  DRM_UNLOCKED | DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(V3D_GEM_CREATE, v3d_gem_create_ioctl,
			  DRM_UNLOCKED | DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(V3D_GEM_MMAP_OFFSET, v3d_gem_mmap_offset_ioctl,
			  DRM_UNLOCKED | DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(V3D_GEM_CREATE_EXT, v3d_gem_create_ext_ioctl,
			  DRM_UNLOCKED | DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(V3D_GET_MEM_TOTAL, v3d_mem_total_ioctl,
			  DRM_UNLOCKED | DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(V3D_GET_FILE_TOKEN, v3d_file_token_ioctl,
			  DRM_UNLOCKED | DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(V3D_SET_CLIENT_TERM, v3d_client_term_ioctl,
			  DRM_MASTER),
	DRM_IOCTL_DEF_DRV(V3D_CLEAR_PT_ON_CLOSE, v3d_clear_pt_on_close_ioctl,
			  DRM_UNLOCKED | DRM_RENDER_ALLOW),
};

static const struct file_operations v3d_driver_fops = {
	.owner		= THIS_MODULE,
	.open		= drm_open,
	.mmap		= v3d_drm_gem_mmap,
	.poll		= drm_poll,
	.read		= drm_read,
	.unlocked_ioctl = drm_ioctl,
	.release	= drm_release,
};

static struct drm_driver v3d_driver = {
	.driver_features	= DRIVER_GEM,
	.load			= v3d_drm_load,
	.open			= v3d_drm_open,
	.preclose		= v3d_drm_preclose,
	.postclose		= v3d_drm_postclose,
	.lastclose		= v3d_drm_lastclose,
	.gem_free_object	= v3d_gem_free_object,
	.gem_vm_ops		= &v3d_gem_vm_ops,
	.ioctls			= v3d_ioctls,
	.num_ioctls		= ARRAY_SIZE(v3d_ioctls),
	.fops			= &v3d_driver_fops,
	.name			= DRIVER_NAME,
	.desc			= DRIVER_DESC,
	.date			= DRIVER_DATE,
	.major			= DRIVER_MAJOR,
	.minor			= DRIVER_MINOR,
};

struct v3d_drm_config_data {
	u64 coherent_dma_mask;
};

static struct v3d_drm_config_data v3d_v3_3_0_0 = {
	.coherent_dma_mask = DMA_BIT_MASK(32)
};

static struct v3d_drm_config_data v3d_v3_3_1_0 = {
	.coherent_dma_mask = DMA_BIT_MASK(40)
};

/* The DT contains the v3d IP version, so map it to a compatible MMU revision */
static const struct of_device_id v3d_drm_of_table[] = {
	{ .compatible = "brcm,v3d-v3.3.0.0", .data = &v3d_v3_3_0_0 },
	{ .compatible = "brcm,v3d-v3.3.1.0", .data = &v3d_v3_3_1_0 },
	{ .compatible = "brcm,v3d-v4.0.2.0", .data = &v3d_v3_3_1_0 },
	{ .compatible = "brcm,v3d-v4.1.34.0",.data = &v3d_v3_3_1_0 },
	{ },
};
MODULE_DEVICE_TABLE(of, v3d_drm_of_table);

static int v3d_drm_probe(struct platform_device *pdev)
{
	struct v3d_drm_config_data *config = NULL;
	bool found_cma_dev = false;
	int i;

	DRM_DEBUG_DRIVER("Probed brcmv3d driver:\n");
	DRM_DEBUG_DRIVER("\tHW Page Size: %zu\n", (size_t)V3D_HW_PAGE_SIZE);
	DRM_DEBUG_DRIVER("\tCMA Alloc Block Size: %zu\n",
			 (size_t)V3D_CMA_ALLOC_BLOCK_SIZE);
	DRM_DEBUG_DRIVER("\tHW Virtual Size: 0x%zx\n",
			 (size_t)V3D_HW_VIRTUAL_ADDR_SIZE);

	if (dev_of_node(&pdev->dev)) {
		const struct of_device_id *match;

		match = of_match_node(v3d_drm_of_table,
				      dev_of_node(&pdev->dev));
		if (!match) {
			DRM_DEBUG_DRIVER("No OF match found for device\n");
			return -ENODEV;
		}

		config = (struct v3d_drm_config_data *)match->data;
	}

	if (!config) {
		DRM_DEBUG_DRIVER("Invalid device configuration data\n");
		return -ENODEV;
	}

	for (i = 0; i < MAX_CMA_AREAS; i++) {
		struct cma_dev *dev = cma_dev_get_cma_dev(i);

		if (dev) {
			phys_addr_t base = dev->range.base;

			DRM_DEBUG_DRIVER("Found CMA dev 0x%pK for MEMC%d (base %pa, size 0x%x)\n",
					 dev, dev->memc, &base,
					 dev->range.size);

			if (v3d_drm_dev_is_cma_device_valid(dev))
				found_cma_dev = true;
		}
	}

	if (!found_cma_dev) {
		DRM_DEBUG_DRIVER("No brcm_cma region defined\n");
		return -ENOMEM;
	}

	pdev->dev.coherent_dma_mask = config->coherent_dma_mask;
	return drm_platform_init(&v3d_driver, pdev);
}

static int v3d_drm_remove(struct platform_device *pdev)
{
	drm_put_dev(platform_get_drvdata(pdev));
	return 0;
}

static struct platform_driver v3d_drm_platform_driver = {
	.probe = v3d_drm_probe,
	.remove = v3d_drm_remove,
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = v3d_drm_of_table,
	},
};

module_platform_driver(v3d_drm_platform_driver);

MODULE_AUTHOR("Broadcom Limited.");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL and additional rights");
MODULE_ALIAS("platform:" DRIVER_NAME);
