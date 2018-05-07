/*
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#ifndef _V3D_DRV_H_
#define _V3D_DRV_H_

#include <linux/version.h>
#include <linux/types.h>
#include <linux/genalloc.h>
#include <linux/bitops.h>
#include <linux/bitmap.h>
#include <linux/mutex.h>
#include <linux/ktime.h>
#include <linux/cma.h>
#include <linux/brcmstb/cma_driver.h>
#include <linux/file.h>
#include <drm/drmP.h>
#include <drm/drm_gem.h>

#define V3D_PAGE_FLAG_VALID	BIT(28)
#define V3D_PAGE_FLAG_WRITE	BIT(29)
#define V3D_PAGE_FLAG_BIGPAGE	BIT(30)
#define V3D_PAGE_FLAG_MASK      0xf0000000

/*
 * Note: MMU big pages (64k) are not supported with this variant of the
 * DRM driver, when using Linux shared memory backed GEM objects the
 * MMU page size must be the same as the CPU page size.
 */
#define V3D_HW_PAGE_MASK  (~((phys_addr_t)PAGE_SIZE - 1))

/*
 * v3d_alloc_block Represents a physically contiguous block allocated
 * from a CMA device, from which we are going to allocate memory in 4k
 * pages for use by the hardware. These pages do not have to be physically
 * contiguous as we will use the hardware MMU to make them virtually contiguous.
 *
 * We may have a mixture of blocks from more than one CMA device if the
 * platform supports multiple memory controllers.
 */
#define V3D_CMA_ALLOC_BLOCK_SIZE (2UL * 1024 * 1024)
#define V3D_CMA_ALLOC_MAP_SIZE (V3D_CMA_ALLOC_BLOCK_SIZE / PAGE_SIZE)

struct v3d_alloc_block {
	struct list_head node;
	DECLARE_BITMAP(alloc_map, V3D_CMA_ALLOC_MAP_SIZE); /* 1bit per page */
	int cma_dev;
	phys_addr_t phys_addr;
	int n_allocs;  /* number of pages allocated per block */
};

struct v3d_page_allocation {
	/**
	 * The Linux pfn for the first CPU page in this allocation.
	 */
	unsigned long cpu_pfn;

	/**
	 * For fast free without trawling the block list and
	 * matching addresses
	 */
	struct v3d_alloc_block *alloc_block;
	unsigned int block_page_nr;
};

/*
 * The V3D MMU virtual space management will start after the first page and
 * use the first 1GB. The pagetable size will therefore be 1MB.
 *
 * This is believed to be a reasonable compromise; each file open
 * on the device will get its own pagetable which eliminates any
 * fragmentation issues of having a single shared virtual space between
 * multiple clients at the cost of a 1MB CMA allocation per device open.
 *
 * We are assuming that we can update the pagetable in place and that the
 * V3D MAGNUM driver will cause the MMU cache to be cleared before any job
 * needing an updated mapping is run on the hardware. We should never be
 * asked to remove the mapping for an object that is used by a job that
 * is currently running or is queued in the hardware pipeline.
 *
 */
#define V3D_HW_VIRTUAL_ADDR_BASE (PAGE_SIZE)
#define V3D_HW_VIRTUAL_ADDR_SIZE (1UL << 30)

#define V3D_HW_PAGE_TABLE_ALIGN		(4096)
#define V3D_HW_PAGE_TABLE_ENTRY_SIZE	(4)

#define V3D_HW_PAGE_TABLE_ENTRIES (V3D_HW_VIRTUAL_ADDR_SIZE >> PAGE_SHIFT)

#define V3D_HW_PAGE_TABLE_SIZE \
	(V3D_HW_PAGE_TABLE_ENTRY_SIZE * V3D_HW_PAGE_TABLE_ENTRIES)

struct v3d_hw_virtual_mem {
	/** Hardware virtual memory address pool */
	struct gen_pool *virtual_pool;

	/**
	 * V3D MMU pagetable allocation details
	 *
	 */
	u32 *pt_kaddr;
	phys_addr_t pt_phys;
	bool clear_entries_on_free;
};

/*
 * If we don't know the hardware has finished with pages at the point the
 * file descriptor is closed (i.e. someone presses ctrl-C during development)
 * we need to hold on to the pages and the shmem file descriptor when a
 * shmem backed GEM object is destroyed until we think it is finally safe to
 * release them.
 */
struct v3d_drm_dead_pages {
	struct list_head node;

	struct page **pages;
	int npages;

	struct file *filp;
};

struct v3d_drm_file_private {
	/** Reference count of this object, must be first in structure */
	struct kref refcount;

	struct list_head alloc_blocks[MAX_CMA_AREAS];
	bool all_blocks_full[MAX_CMA_AREAS];
	int next_alloc_device;

	bool clear_pagetable_on_close;
	bool defer_shm_page_release;

	struct drm_device *dev; /* Needed to release pagetable allocations */
	struct v3d_hw_virtual_mem hw_vmem;

	/** DebugFS support */
	struct dentry *debugfs_root;

	/**
	 * Unique identifier for signalling the memory cleanup of a dead
	 * client associated with this structure
	 */
	u64 magic_id;

	/** When placed on the dead client client cleanup list */
	struct list_head dead_pages;
	struct list_head dead_fp;
};

#define v3d_first_alloc_block(fp, mem_dev) \
	list_first_entry(&(fp)->alloc_blocks[mem_dev], \
	struct v3d_alloc_block, node)

struct v3d_drm_dead_client {
	struct list_head node;
	u64 magic_id;
};

struct v3d_drm_dev_private {
	/** Reference count of this object, must be first in structure */
	struct kref refcount;

	/** Available Broadcom CMA devices on the platform */
	struct cma_dev *cma_devs[MAX_CMA_AREAS];
	int nr_cma_devs;

	/** Deferred file private structures for cleanup of dead clients */
	struct list_head dead_fps;

	/**
	 * List of dead client magic values that we have been told have
	 * fully terminated, but we haven't yet seen their file descriptor close
	 */
	struct list_head dead_clients;
};

struct v3d_drm_gem_object {
	struct drm_gem_object base;

	/**
	 * Driver private data of the file handle that allocated this obj
	 * so we know how to free the pages when the object is destroyed
	 */
	struct v3d_drm_file_private *fp;
	struct v3d_page_allocation *cma_pages;
	struct page **shm_pages;
	dma_addr_t *dma_addrs;

	u32 hw_virt_addr;
	u32 alloc_flags;
	char *desc;
};

#define to_v3d_obj(x) container_of(x, struct v3d_drm_gem_object, base)

/*
 * Broadcom CMA allocation helpers
 */
void v3d_release_cma_blocks(struct v3d_drm_file_private *fp,
			    struct v3d_drm_dev_private *dp);

void v3d_free_cma_pages(size_t num_pages, struct v3d_page_allocation *pages);

int v3d_allocate_cma_pages(struct v3d_drm_file_private *fp, size_t num_pages,
			   struct v3d_page_allocation *pages);


/*
 * Hardware virtual memory/pagetable helpers
 */
int v3d_vmem_init(struct v3d_hw_virtual_mem *vmem, struct drm_device *dev);
void v3d_vmem_destroy(struct v3d_hw_virtual_mem *vmem, struct drm_device *dev);

unsigned long v3d_vmem_allocate(struct v3d_hw_virtual_mem *vmem, size_t size);

void v3d_vmem_free(struct v3d_hw_virtual_mem *hw_vmem,
		   unsigned long addr, size_t size);

void v3d_vmem_add_pages_to_pagetable(struct v3d_hw_virtual_mem *vmem,
				     struct device *dev,
				     dma_addr_t *dma_addrs, size_t pages,
				     u32 va, bool readonly);

void v3d_vmem_add_extobj_to_pagetable(struct v3d_hw_virtual_mem *vmem,
				      phys_addr_t pa, size_t pages,
				      u32 va, bool readonly);

/*
 * DebugFS file open/close helpers
 */
#ifdef CONFIG_DEBUG_FS
void v3d_debugfs_create_file_entries(struct drm_file *file);
void v3d_debugfs_cleanup_file_entries(struct drm_file *file);
#else
void v3d_debugfs_create_file_entries(struct drm_file *file) {}
void v3d_debugfs_cleanup_file_entries(struct drm_file *file) {}
#endif

#ifdef CONFIG_COMPAT
long v3d_drm_compat_ioctl(struct file *filp, unsigned int cmd,
			  unsigned long arg);
#endif

/*
 * Add a new DRM debug level for verbose information from the memory allocations
 * while we are developing.
 */
#define DRM_UT_ALLOC 0x100
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 9)
#define DRM_DEBUG_ALLOC(fmt, args...)				   \
	do {							    \
		if (unlikely(drm_debug & DRM_UT_ALLOC))		 \
			drm_ut_debug_printk(__func__, fmt, ##args);     \
	} while (0)
#else
#define DRM_DEBUG_ALLOC(fmt, ...)					\
	drm_printk(KERN_DEBUG, DRM_UT_ALLOC, fmt, ##__VA_ARGS__)
#endif

#endif
