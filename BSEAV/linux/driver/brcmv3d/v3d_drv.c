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
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 9)
#include <linux/pfn.h>
#else
#include <linux/pfn_t.h>
#endif
#include <linux/dma-buf.h>
#include <linux/list.h>
#include <linux/pagemap.h>
#include <uapi/drm/brcmv3d_drm.h>

#include <asm/barrier.h>

#include "v3d_drv.h"

#define DRIVER_NAME	"brcmv3d"
#define DRIVER_DESC	"Broadcom V3D GEM provider"
#define DRIVER_DATE	"20180321"
#define DRIVER_MAJOR	2
#define DRIVER_MINOR	0
#define DRIVER_PATCH	1

static gfp_t shm_mapping_mask;

/* Do not use Broadcom CMA regions even if we find any */
unsigned int ignore_cma;

/*
 * v3d_drm_file_private implementation
 */
static struct v3d_drm_file_private *
v3d_drm_file_private_create(struct drm_device *dev)
{
	static u64 magic_ids = 0xdeadc1e500000000ULL;
	struct v3d_drm_file_private *fp;
	int i;

	fp = kzalloc(sizeof(*fp), GFP_KERNEL);
	if (!fp)
		return ERR_PTR(-ENOMEM);

	kref_init(&fp->refcount);

	INIT_LIST_HEAD(&fp->dead_pages);
	INIT_LIST_HEAD(&fp->dead_fp);
	for (i = 0; i < MAX_CMA_AREAS; i++) {
		INIT_LIST_HEAD(&fp->alloc_blocks[i]);
		/* Force a block to be allocated on the first object create */
		fp->all_blocks_full[i] = true;
	}

	/* The DRM device struct_mutex is held at this point, so this is safe */
	fp->magic_id = magic_ids++;
	fp->dev = dev;

	/*
	 * virtual memory and pagetable setup deferred to first real use
	 * to prevent wasting CMA for the Magnum driver open.
	 */
	return fp;
}

static void v3d_drm_file_private_release(struct kref *kref)
{
	struct v3d_drm_file_private *fp = (struct v3d_drm_file_private *)kref;
	struct v3d_drm_dev_private *dp = fp->dev->dev_private;

	DRM_DEBUG_DRIVER("fp=0x%pK\n", fp);

	v3d_vmem_destroy(&fp->hw_vmem, fp->dev);
	v3d_release_cma_blocks(fp, dp);

	if (!list_empty(&fp->dead_pages)) {
		struct v3d_drm_dead_pages *itr, *itr_tmp;
		int i;

		list_for_each_entry_safe(itr, itr_tmp,
					 &fp->dead_pages, node) {
			DRM_DEBUG_ALLOC("Release %d deferred pages\n",
					itr->npages);

			list_del(&itr->node);

			/* Emulate drm_gem_put_pages */
			for (i = 0; i < itr->npages; i++) {
				set_page_dirty(itr->pages[i]);
				put_page(itr->pages[i]);
			}

			kvfree(itr->pages);

			/* Now release the shm file pointer created for the gem
			 * object backed by these pages
			 */
			fput(itr->filp);
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
static
struct v3d_drm_dev_private *v3d_drm_dev_private_create(struct drm_device *dev)
{
	struct v3d_drm_dev_private *dp;
	int i;

	dp = kzalloc(sizeof(*dp), GFP_KERNEL);
	if (!dp)
		return ERR_PTR(-ENOMEM);

	kref_init(&dp->refcount);
	INIT_LIST_HEAD(&dp->dead_fps);
	INIT_LIST_HEAD(&dp->dead_clients);

	DRM_DEBUG_DRIVER("DMA mask = %pa\n", dev->dev->dma_mask);
	for (i = 0; i < MAX_CMA_AREAS; i++) {
		struct cma_dev *cma_dev = cma_dev_get_cma_dev(i);

		if (!ignore_cma && cma_dev) {
			phys_addr_t base = cma_dev->range.base;

			if (dma_capable(dev->dev, base, cma_dev->range.size)) {
				DRM_DEBUG_DRIVER("Found CMA dev 0x%pK for MEMC%d (base %pa, size 0x%x)\n",
						 cma_dev, cma_dev->memc, &base,
						 cma_dev->range.size);

				dp->cma_devs[dp->nr_cma_devs] = cma_dev;
				dp->nr_cma_devs++;
			}
		}
	}
	return dp;
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
			 * This will cleanup remaining allocations hanging off
			 * the file private structure.
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
					    u64 id)
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
	list_add_tail(&dead_client->node, &dp->dead_clients);
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

void v3d_drm_term_client(u64 id)
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
 * branches, so trampoline via a uintptr_t
 */
uintptr_t v3d_drm_term_client_p = (uintptr_t)&v3d_drm_term_client;
EXPORT_SYMBOL(v3d_drm_term_client_p);

static void v3d_drm_dev_private_release(struct kref *kref)
{
	struct v3d_drm_dev_private *dp = (struct v3d_drm_dev_private *)kref;

	if (dp) {
		DRM_DEBUG_DRIVER("dp=0x%pK\n", dp);
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
static inline void
v3d_gem_defer_shm_page_release(struct v3d_drm_gem_object *obj)
{
	struct v3d_drm_dead_pages *d;

	d = kmalloc(sizeof(*d), GFP_KERNEL);
	if (d) {
		DRM_DEBUG_ALLOC("Defer release of obj=0x%pK on fp=0x%pK\n",
				obj, obj->fp);
		INIT_LIST_HEAD(&d->node);
		d->pages = obj->shm_pages;
		d->npages = obj->base.size >> PAGE_SHIFT;
		d->filp = get_file(obj->base.filp);
		list_add_tail(&d->node, &obj->fp->dead_pages);
	} else {
		/* The system must be in a very bad way, so release the pages
		 * anyway and hope for the best
		 */
		drm_gem_put_pages(&obj->base, obj->shm_pages, true, false);
	}
}

static void v3d_gem_put_pages(struct v3d_drm_gem_object *obj)
{
	size_t num_pages = obj->base.size >> PAGE_SHIFT;

	if (obj->dma_addrs) {
		int i;

		for (i = 0; i < num_pages; i++) {
			if (obj->dma_addrs[i])
				dma_unmap_page(obj->base.dev->dev,
					       obj->dma_addrs[i],
					       PAGE_SIZE, DMA_TO_DEVICE);
		}
		kfree(obj->dma_addrs);
		obj->dma_addrs = NULL;
	}

	if (obj->cma_pages) {
		DRM_DEBUG_ALLOC("Free CMA pages for obj=0x%pK\n", obj);
		v3d_free_cma_pages(num_pages, obj->cma_pages);
		vfree(obj->cma_pages);
		obj->cma_pages = NULL;
	}

	if (obj->shm_pages) {
		DRM_DEBUG_ALLOC("Free SHM pages for obj=0x%pK\n", obj);
		if (obj->fp->defer_shm_page_release)
			v3d_gem_defer_shm_page_release(obj);
		else
			drm_gem_put_pages(&obj->base,
					  obj->shm_pages, true, false);

		obj->shm_pages = NULL;
	}
}

static void v3d_gem_free_object(struct drm_gem_object *obj)
{
	struct v3d_drm_gem_object *v3d_obj = to_v3d_obj(obj);

	DRM_DEBUG_ALLOC("obj=0x%pK\n", obj);

	/* Note: this is called with obj->dev->struct_mutex held */
	drm_gem_free_mmap_offset(obj);

	if (!WARN_ON(!v3d_obj->fp)) {
		struct v3d_hw_virtual_mem *vmem  = &v3d_obj->fp->hw_vmem;

		v3d_vmem_free(vmem, v3d_obj->hw_virt_addr, obj->size);
	}

	v3d_obj->hw_virt_addr = 0;

	v3d_gem_put_pages(v3d_obj);
	v3d_drm_file_private_unreference(v3d_obj->fp);

	drm_gem_object_release(obj);

	kfree(v3d_obj->desc);
	kfree(v3d_obj);
}

static int v3d_gem_get_pages(struct v3d_drm_gem_object *obj)
{
	struct v3d_drm_file_private *fp = obj->fp;
	struct v3d_drm_dev_private *dp = fp->dev->dev_private;
	struct device *dev = obj->base.dev->dev;
	size_t size = obj->base.size;
	size_t num_pages = size >> PAGE_SHIFT;
	bool readonly = !!(obj->alloc_flags & V3D_CREATE_HW_READONLY);
	int i, err;

	/*
	 * First try and allocate a region in the hardware virtual space
	 */
	obj->hw_virt_addr = v3d_vmem_allocate(&fp->hw_vmem, size);
	if (!obj->hw_virt_addr) {
		err = -ENOMEM;
		goto out;
	}

	if (dp->nr_cma_devs) {
		struct v3d_page_allocation *pages;

		DRM_DEBUG_ALLOC("Allocate CMA pages for obj=0x%pK\n", obj);
		pages = vmalloc(sizeof(*pages) * num_pages);
		if (!pages) {
			err = -ENOMEM;
			goto free_virtual;
		}

		err =  v3d_allocate_cma_pages(fp, num_pages, pages);
		if (err) {
			vfree(pages);
			goto free_virtual;
		}

		obj->cma_pages = pages;
	} else {
		DRM_DEBUG_ALLOC("Allocate SHM pages for obj=0x%pK\n", obj);
		obj->shm_pages = drm_gem_get_pages(&obj->base);
		if (IS_ERR(obj->shm_pages)) {
			err = PTR_ERR(obj->shm_pages);
			obj->shm_pages = NULL;
			goto free_virtual;
		}
	}

	obj->dma_addrs = kmalloc(num_pages * sizeof(*obj->dma_addrs),
				 GFP_KERNEL);

	if (!obj->dma_addrs) {
		err = -ENOMEM;
		goto free_pages;
	}

	for (i = 0; i < num_pages; i++) {
		struct page *p;

		if (obj->cma_pages)
			p = pfn_to_page(obj->cma_pages[i].cpu_pfn);
		else
			p = obj->shm_pages[i];

		obj->dma_addrs[i] = dma_map_page(dev, p, 0, PAGE_SIZE,
						 DMA_TO_DEVICE);

		if (dma_mapping_error(dev, obj->dma_addrs[i])) {
			for (i = i - 1; i >= 0; --i) {
				dma_unmap_page(dev, obj->dma_addrs[i],
					       PAGE_SIZE, DMA_TO_DEVICE);
			}

			err = -ENOMEM;
			goto free_addrs;
		}
	}

	v3d_vmem_add_pages_to_pagetable(&fp->hw_vmem, dev, obj->dma_addrs,
					num_pages, obj->hw_virt_addr, readonly);

	return 0;

free_addrs:
	kfree(obj->dma_addrs);
	obj->dma_addrs = NULL;
free_pages:
	if (obj->cma_pages) {
		v3d_free_cma_pages(num_pages, obj->cma_pages);
		vfree(obj->cma_pages);
		obj->cma_pages = NULL;
	}

	if (obj->shm_pages) {
		drm_gem_put_pages(&obj->base, obj->shm_pages, false, false);
		obj->shm_pages = NULL;
	}
free_virtual:
	v3d_vmem_free(&fp->hw_vmem, obj->hw_virt_addr, size);
	obj->hw_virt_addr = 0;
out:
	return err;
}

static struct drm_gem_object *
v3d_gem_create(struct drm_device *dev,
	       struct drm_file *file,
	       struct drm_v3d_gem_create *args)
{
	struct v3d_drm_dev_private *dp = dev->dev_private;
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct v3d_drm_gem_object *obj;
	struct drm_gem_object *gem_object;
	u32 alloc_size;
	int err;

	if (WARN_ON(!fp))
		return ERR_PTR(-EFAULT);

	alloc_size = roundup(args->size, PAGE_SIZE);

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj)
		return ERR_PTR(-ENOMEM);

	gem_object = &obj->base;

	if (dp->nr_cma_devs) {
		drm_gem_private_object_init(dev, gem_object, alloc_size);
	} else {
		err = drm_gem_object_init(dev, gem_object, alloc_size);
		if (err)
			goto obj_out;

		mapping_set_gfp_mask(gem_object->filp->f_mapping,
				     shm_mapping_mask);
	}

	v3d_drm_file_private_reference(fp);
	obj->fp = fp;
	obj->alloc_flags = args->flags;

	if (args->desc) {
		obj->desc = strndup_user(args->desc, 256);
		/* If the description is too big, ignore it */
		if (IS_ERR(obj->desc))
			obj->desc = NULL;
	}

	err = v3d_gem_get_pages(obj);
	if (err)
		goto gemobject_out;

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
	v3d_vmem_free(&fp->hw_vmem, obj->hw_virt_addr, gem_object->size);

	v3d_gem_put_pages(obj);
gemobject_out:
	v3d_drm_file_private_unreference(obj->fp);
	drm_gem_object_release(gem_object);
	kfree(obj->desc);
obj_out:
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
	u32 alloc_size, page_offset;
	bool readonly = !!(args->flags & V3D_CREATE_HW_READONLY);
	int err;

	if (WARN_ON(!fp))
		return ERR_PTR(-EFAULT);

	/*
	 * We are not allowing the mapping of an empty object, that must
	 * be an application error.
	 */
	if (args->size == 0)
		return ERR_PTR(-EINVAL);

	/*
	 * Check the physical address is mappable into the MMU, this should
	 * only be possible at all on a misconfigured 7260 platform using the
	 * MEMC0 extension space which the V3D MMU cannot see as it is only
	 * 32bit capable.
	 */
	if (WARN_ON(!dma_capable(dev->dev, args->phys, args->size)))
		return ERR_PTR(-EINVAL);

	/*
	 * We need to fit the object into the right position and number
	 * of pages, we do not require that the external object's
	 * pages are aligned or sized to the V3D HW page boundaries.
	 */
	page_offset = args->phys & ~V3D_HW_PAGE_MASK;
	alloc_size = roundup(args->size + page_offset, PAGE_SIZE);

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj)
		return ERR_PTR(-ENOMEM);

	gem_object = &obj->base;

	drm_gem_private_object_init(dev, gem_object, alloc_size);

	v3d_drm_file_private_reference(fp);
	obj->fp = fp;
	obj->alloc_flags = args->flags;
	if (args->desc)
		obj->desc = strndup_user(args->desc, 32);

	obj->hw_virt_addr = v3d_vmem_allocate(&fp->hw_vmem, alloc_size);
	if (!obj->hw_virt_addr) {
		err = -ENOMEM;
		goto object_out;
	}

	v3d_vmem_add_extobj_to_pagetable(&fp->hw_vmem, args->phys,
					 (alloc_size >> PAGE_SHIFT),
					 obj->hw_virt_addr, readonly);

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
	v3d_vmem_free(&fp->hw_vmem, obj->hw_virt_addr, gem_object->size);

object_out:
	v3d_drm_file_private_unreference(obj->fp);
	drm_gem_object_release(gem_object);
	kfree(obj->desc);
	kfree(obj);
	return ERR_PTR(err);
}

static int v3d_gem_map_offset(struct drm_file *file, struct drm_device *dev,
			      u32 handle, u64 *offset)
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
	if (!to_v3d_obj(obj)->cma_pages && !to_v3d_obj(obj)->shm_pages) {
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
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct drm_v3d_mmu_pt *args = data;
	int err;

	if (WARN_ON(!fp))
		return -EFAULT;

	mutex_lock(&dev->struct_mutex);

	err = v3d_vmem_init(&fp->hw_vmem, dev);
	if (!err) {
		args->pt_phys = fp->hw_vmem.pt_phys;
		args->va_size = V3D_HW_VIRTUAL_ADDR_SIZE;
	}

	mutex_unlock(&dev->struct_mutex);

	return err;
}

static int v3d_clear_pt_on_close_ioctl(struct drm_device *dev, void *data,
				       struct drm_file *file)
{
	struct v3d_drm_file_private *fp = file->driver_priv;

	if (WARN_ON(!fp))
		return -EFAULT;

	mutex_lock(&dev->struct_mutex);

	fp->clear_pagetable_on_close = true;

	mutex_unlock(&dev->struct_mutex);

	return 0;
}

static int v3d_mem_total_ioctl(struct drm_device *dev, void *data,
			       struct drm_file *file)
{
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct v3d_drm_dev_private *dp;
	struct drm_v3d_mem_total *args = data;
	size_t total = 0;
	int i;

	if (WARN_ON(!fp))
		return -EFAULT;

	mutex_lock(&dev->struct_mutex);

	dp = dev->dev_private;
	WARN_ON(dp != fp->dev->dev_private);

	for (i = 0; i < MAX_CMA_AREAS; i++) {
		if (dp->cma_devs[i])
			total += dp->cma_devs[i]->range.size;
	}

	/* TODO: what do we do here for Vulkan ? We cannot report 0 */
	if (!total)
		total = 256 * 1024 * 1024;

	args->size = total;

	mutex_unlock(&dev->struct_mutex);

	return 0;
}

static int v3d_file_token_ioctl(struct drm_device *dev, void *data,
				struct drm_file *file)
{
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct drm_v3d_file_private_token *args = data;

	if (WARN_ON(!fp))
		return -EFAULT;

	args->token = fp->magic_id;
	return 0;
}

static int v3d_client_term_ioctl(struct drm_device *dev, void *data,
				 struct drm_file *file)
{
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct v3d_drm_dev_private *dp;
	struct drm_v3d_file_private_token *args = data;

	if (WARN_ON(!fp))
		return -EFAULT;

	mutex_lock(&dev->struct_mutex);

	dp = dev->dev_private;
	WARN_ON(dp != fp->dev->dev_private);

	v3d_drm_dev_private_term_client(dp, args->token);

	mutex_unlock(&dev->struct_mutex);

	return 0;
}

static int v3d_gem_create_ioctl(struct drm_device *dev, void *data,
				struct drm_file *file)
{
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct drm_gem_object *gem_object;
	struct drm_v3d_gem_create *args = data;
	int err;

	DRM_DEBUG_ALLOC("size=%zu\n", (size_t)args->size);

	mutex_lock(&dev->struct_mutex);

	err = v3d_vmem_init(&fp->hw_vmem, dev);
	if (!err) {
		gem_object = v3d_gem_create(dev, file, args);

		if (IS_ERR(gem_object)) {
			DRM_DEBUG_ALLOC("object creation failed\n");
			err = PTR_ERR(gem_object);
		}
	}

	mutex_unlock(&dev->struct_mutex);

	return err;
}

static int v3d_gem_create_ext_ioctl(struct drm_device *dev, void *data,
				    struct drm_file *file)
{
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct drm_gem_object *gem_object;
	struct drm_v3d_gem_create_ext *args = data;
	int err;

	DRM_DEBUG_ALLOC("phys=%pa size=%zu\n",
			&args->phys, (size_t)args->size);

	if (args->flags & ~V3D_CREATE_HW_READONLY)
		return -EINVAL;

	mutex_lock(&dev->struct_mutex);

	err = v3d_vmem_init(&fp->hw_vmem, dev);
	if (!err) {
		gem_object = v3d_gem_create_ext(dev, file, args);

		if (IS_ERR(gem_object)) {
			DRM_DEBUG_DRIVER("external object creation failed\n");
			err = PTR_ERR(gem_object);
		}
	}

	mutex_unlock(&dev->struct_mutex);

	return err;
}

static int v3d_gem_mmap_offset_ioctl(struct drm_device *dev, void *data,
				     struct drm_file *file)
{
	struct drm_v3d_gem_mmap_offset *args = data;

	DRM_DEBUG_ALLOC("handle=%u\n", args->handle);

	return v3d_gem_map_offset(file, dev, args->handle, &args->offset);
}

static int v3d_gem_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct drm_gem_object *obj = vma->vm_private_data;
	struct v3d_drm_gem_object *v3d_obj = to_v3d_obj(obj);
	pgoff_t pgoff;
	unsigned long pfn;
	int err;

	mutex_lock(&obj->dev->struct_mutex);

	/* We don't use vmf->pgoff since that has the fake offset: */
	pgoff = ((unsigned long)vmf->virtual_address -
				vma->vm_start) >> PAGE_SHIFT;
	if (v3d_obj->cma_pages)
		pfn = v3d_obj->cma_pages[pgoff].cpu_pfn;
	else
		pfn = page_to_pfn(v3d_obj->shm_pages[pgoff]);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 9)
	err = vm_insert_mixed(vma, (unsigned long)vmf->virtual_address, pfn);
#else
	err = vm_insert_mixed(vma, (unsigned long)vmf->virtual_address,
			      __pfn_to_pfn_t(pfn, PFN_DEV));
#endif

	mutex_unlock(&obj->dev->struct_mutex);

	switch (err) {
	case 0:
	case -ERESTARTSYS:
	case -EINTR:
	case -EBUSY:
		/*
		 * Note: EBUSY  means that another thread has already mapped
		 *       this page.
		 */
		return VM_FAULT_NOPAGE;
	case -ENOMEM:
		return VM_FAULT_OOM;
	default:
		return VM_FAULT_SIGBUS;
	}
}

static const struct vm_operations_struct v3d_gem_vm_ops = {
	.fault = v3d_gem_fault,
	.open  = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};

static int v3d_drm_gem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct v3d_drm_gem_object *obj;
	bool wc;
	int ret;

	ret = drm_gem_mmap(filp, vma);
	if (ret)
		return ret;

	vma->vm_flags &= ~VM_PFNMAP;
	vma->vm_flags |= VM_MIXEDMAP;

	obj = to_v3d_obj(vma->vm_private_data); /* set by drm_gem_mmap() */

	wc = !!(obj->alloc_flags & V3D_CREATE_CPU_WRITECOMBINE);

	DRM_DEBUG_ALLOC("obj=0x%pK vma 0x%lx -> 0x%lx (%s)\n",
			obj, vma->vm_start, vma->vm_end, wc ? "wc" : "cached");

	if (!wc)
		vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);

	return 0;
}

static int v3d_drm_open(struct drm_device *dev, struct drm_file *file)
{
	struct v3d_drm_file_private *fp;
	struct v3d_drm_dev_private *dp;
	int err;

	dp = dev->dev_private;
	v3d_drm_dev_private_reference(dp);

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

	/*
	 * If there is no current master, i.e. this open will become the
	 * master, flush any dead clients. Either this is being used
	 * by a Nexus single process app, or NxServer died and is being
	 * restarted. In both cases we will never get the client termination
	 * for previous uses, but in those cases the open can only happen after
	 * or as part of the Nexus/Magnum initialization process which
	 * resets the hardware. So it is safe to release allocations from
	 * previous usage.
	 *
	 * Note: we cannot use file->is_master as that hasn't been set up
	 *       yet.
	 */
	mutex_lock(&dev->master_mutex);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 9)
	if (drm_is_primary_client(file) && !file->minor->master)
#else
	if (!dev->master)
#endif
		v3d_drm_dev_private_flush_dead_clients(dp);

	mutex_unlock(&dev->master_mutex);

	return 0;

out:
	DRM_DEBUG_DRIVER("Failed to create file private\n");
	v3d_drm_dev_private_unreference(dp);
	return err;
}

static void v3d_drm_preclose(struct drm_device *dev, struct drm_file *file)
{
	struct v3d_drm_file_private *fp = file->driver_priv;
	struct v3d_drm_dev_private *dp = dev->dev_private;

	DRM_DEBUG_DRIVER("dp=0x%pK fp=0x%pK\n", dp, fp);

	mutex_lock(&dev->struct_mutex);

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

	/*
	 * If we have not yet had a termination notification to say it
	 * is safe to free the remaining GEM object backing pages then
	 * flag that their release should be deferred.
	 */
	fp->defer_shm_page_release = true;

	if (!list_empty(&dp->dead_clients)) {
		struct v3d_drm_dead_client *itr, *itr_tmp;

		list_for_each_entry_safe(itr, itr_tmp,
					 &dp->dead_clients, node) {
			if (itr->magic_id == fp->magic_id) {
				fp->defer_shm_page_release = false;
				break;
			}
		}
	}

	mutex_unlock(&dev->struct_mutex);
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

static int v3d_drm_load(struct drm_device *dev, unsigned long flags)
{
	struct v3d_drm_dev_private *dp = v3d_drm_dev_private_create(dev);

	if (IS_ERR(dp))
		return PTR_ERR(dp);

	dev->dev_private = dp;
	v3d_drm_dev_private_reference(dp);
	drm_dev_ref(dev);
	the_drm_device = dev;

	mapping_set_gfp_mask(dev->anon_inode->i_mapping, shm_mapping_mask);

	/* Just so we can clean up on module unload */
	platform_set_drvdata(dev->platformdev, dev);

	DRM_DEBUG_DRIVER("dp=0x%pK\n", dp);

	return 0;
}

static int v3d_drm_unload(struct drm_device *dev)
{
	struct v3d_drm_dev_private *dp = dev->dev_private;

	DRM_DEBUG_DRIVER("dp=0x%pK\n", dp);

	mutex_lock(&dev->struct_mutex);

	if (the_drm_device) {
		WARN_ON(dev != the_drm_device);
		drm_dev_unref(the_drm_device);
		v3d_drm_dev_private_unreference(dp);
		the_drm_device = NULL;
	}

	/* Release the final reference and destroy the structure */
	v3d_drm_dev_private_unreference(dp);
	dev->dev_private = NULL;

	mutex_unlock(&dev->struct_mutex);
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
#ifdef CONFIG_COMPAT
	.compat_ioctl   = v3d_drm_compat_ioctl,
#endif
	.release	= drm_release,
};

static struct drm_driver v3d_driver = {
	.driver_features	= DRIVER_GEM,
	.load			= v3d_drm_load,
	.unload			= v3d_drm_unload,
	.open			= v3d_drm_open,
	.preclose		= v3d_drm_preclose,
	.postclose		= v3d_drm_postclose,
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
	.patchlevel		= DRIVER_PATCH,
};

struct v3d_drm_config_data {
	u64 coherent_dma_mask;
	gfp_t gfp_mask;
};

static struct v3d_drm_config_data v3d_v3_3_0_0 = {
	.coherent_dma_mask = DMA_BIT_MASK(32),
	/* Limits memory to < 4GB boundary on aarch64 with CONFIG_ZONE_DMA
	 * enabled, note that the ARM archs do not use __GFP_DMA32
	 */
	.gfp_mask = GFP_USER | __GFP_DMA
};

static struct v3d_drm_config_data v3d_v3_3_1_0 = {
	.coherent_dma_mask = DMA_BIT_MASK(40),
	.gfp_mask = GFP_HIGHUSER
};

/* The DT contains the v3d IP version, so map it to a compatible MMU revision */
static const struct of_device_id v3d_drm_of_table[] = {
	{ .compatible = "brcm,v3d-v3.3.0.0", .data = &v3d_v3_3_0_0 },
	{ .compatible = "brcm,v3d-v3.3.1.0", .data = &v3d_v3_3_1_0 },
	{ .compatible = "brcm,v3d-v4.0.2.0", .data = &v3d_v3_3_1_0 },
	{ .compatible = "brcm,v3d-v4.1.34.0", .data = &v3d_v3_3_1_0 },
	{ },
};
MODULE_DEVICE_TABLE(of, v3d_drm_of_table);

static int v3d_drm_probe(struct platform_device *pdev)
{
	struct v3d_drm_config_data *config = NULL;

	DRM_DEBUG_DRIVER("Probed brcmv3d driver:\n");
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

	shm_mapping_mask = config->gfp_mask;
	DRM_DEBUG_DRIVER("\tGFP Flags: %lx\n", (unsigned long)shm_mapping_mask);

	/* The device DMA mask needs to be correct otherwise for 40bit capable
	 * MMUs the dma mapping code would start creating 32bit compatible
	 * bounce buffers for allocated pages >4GB, which is not good.
	 */
	pdev->dev.coherent_dma_mask = config->coherent_dma_mask;
	pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;

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
module_param_named(ignore_cma, ignore_cma, int, 0400);
