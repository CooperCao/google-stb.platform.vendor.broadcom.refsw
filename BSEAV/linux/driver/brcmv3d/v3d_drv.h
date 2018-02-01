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

#include <linux/types.h>
#include <linux/genalloc.h>
#include <linux/bitops.h>
#include <linux/bitmap.h>
#include <linux/cma.h>
#include <linux/brcmstb/cma_driver.h>
#include <drm/drmP.h>
#include <drm/drm_gem.h>

#define V3D_PAGE_FLAG_VALID	BIT(28)
#define V3D_PAGE_FLAG_WRITE	BIT(29)
#define V3D_PAGE_FLAG_BIGPAGE	BIT(30)
#define V3D_PAGE_FLAG_MASK      0xf0000000

/* Valid page shifts are:
 * 12 - 4K pages in the MMU, 128K CMA allocations
 * 16 - 64K big pages in the MMU, 2MB CMA allocations
 */
#define V3D_HW_PAGE_SHIFT (12)
#define V3D_HW_PAGE_SIZE  (1UL << V3D_HW_PAGE_SHIFT)
#define V3D_HW_PAGE_MASK  (~((phys_addr_t)V3D_HW_PAGE_SIZE - 1))

#if (V3D_HW_PAGE_SIZE == 4096)
#define V3D_HW_DEFAULT_PAGE_FLAGS (V3D_PAGE_FLAG_VALID)
#else
#define V3D_HW_DEFAULT_PAGE_FLAGS (V3D_PAGE_FLAG_VALID | \
				   V3D_PAGE_FLAG_BIGPAGE)
#endif

/*
 * v3d_alloc_block Represents a physically contiguous block allocated
 * from a CMA device, from which we are going to allocate memory in 4k or 64k
 * pages for use by the hardware. These pages do not have to be physically
 * contiguous as we will use the hardware MMU to make them virtually contiguous.
 *
 * We may have a mixture of blocks from more than one CMA device if the
 * platform supports multiple memory controllers.
 */
#define V3D_CMA_ALLOC_BLOCK_SIZE (2UL * 1024 * 1024)
#define V3D_CMA_ALLOC_MAP_SIZE (V3D_CMA_ALLOC_BLOCK_SIZE / V3D_HW_PAGE_SIZE)

struct v3d_alloc_block {
	struct list_head node;
	DECLARE_BITMAP(alloc_map, V3D_CMA_ALLOC_MAP_SIZE); /* 1bit per page */
	int cma_dev;
	phys_addr_t phys_addr;
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
 * use the first 1GB. Note that the MMU page table always contains entries
 * for 4K pages, regardless of the fact we are using 64K bigpages or not. The
 * 16 entries in a 4K page are duplicated in that case. The pagetable size
 * will therefore be 1MB.
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
#define V3D_HW_VIRTUAL_ADDR_BASE (V3D_HW_PAGE_SIZE)
#define V3D_HW_VIRTUAL_ADDR_SIZE (1UL << 30)

#define V3D_HW_PAGE_TABLE_ALIGN		(4096)
#define V3D_HW_PAGE_TABLE_ENTRY_SIZE	(4)
#define V3D_HW_SMALLEST_PAGE_SHIFT	(12)

#define V3D_HW_PAGE_TABLE_ENTRIES \
	(V3D_HW_VIRTUAL_ADDR_SIZE >> V3D_HW_SMALLEST_PAGE_SHIFT)

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
 * Each open file handle on the driver will manage its own set of CMA
 * allocations and V3D hardware MMU table.
 *
 * Each userspace process accessing the V3D hardware as an EGL client  will
 * cause a new file handle to opened on this DRM driver. Therefore each
 * process will allocate graphics objects from its own private CMA allocations
 * and not share CMA blocks with other processes. This allows all of the
 * CMA blocks to be immediately released when a process is killed and
 * the file handle is closed. This is to help prevent the CMA driver
 * from running out of contiguous memory due to fragmentation and to
 * interop with the memory usage of the Broadcom STB middleware drivers
 * when switching between graphics intensive (games) and video playback
 * usecases.
 */
struct v3d_drm_file_private {
	/** Reference count of this object, must be first in structure */
	struct kref refcount;

	struct list_head alloc_blocks[MAX_CMA_AREAS];
	bool all_blocks_full[MAX_CMA_AREAS];
	int next_alloc_device;

	bool clear_pagetable_on_close;

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

	/** DMA address mask for the MMU hardware */
	u64 mmu_dma_mask;

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
	struct v3d_page_allocation *pages;
	u32 hw_virt_addr;
	u32 alloc_flags;
	char *desc;
};

#define to_v3d_obj(x) container_of(x, struct v3d_drm_gem_object, base)

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

#endif
