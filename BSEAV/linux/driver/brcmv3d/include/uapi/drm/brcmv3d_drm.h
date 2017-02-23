/*
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _UAPI_BRCM_V3D_DRM_H_
#define _UAPI_BRCM_V3D_DRM_H_

#include <drm/drm.h>

struct drm_v3d_mmu_pt {
	/**
	 * Returned physical address of the V3D MMU pagetable
	 * being managed by the open filehandle that DRM_V3D_GET_MMU_PAGETABLE
	 * is being called on. This will be valid until the filehandle is
	 * closed.
	 *
	 * The interface is defined to allow returning 40bit physical addresses
	 * although not all hardware can use more than 32bits.
	 */
	__u64 pt_phys;

	/**
	 * Returned size of the virtual address space covered by the pagetable
	 */
	__u32 va_size;
};

struct drm_v3d_mem_total {
	/**
	 * Returned total memory size that may be available for GEM object
	 * allocations.
	 *
	 * As this is a shared memory system, there are no gaurentees that
	 * this amount of memory can actually be allocated. It is provided
	 * to satisfy API requirements in OpenCL and Vulkan to provide some
	 * reasonable number.
	 */
	__u64 size;
};

/* Flags for drm_v3d_gem_create
 */
#define V3D_CREATE_HW_READONLY      0x1
#define V3D_CREATE_CPU_WRITECOMBINE 0x2

struct drm_v3d_gem_create {
	/**
	 * Requested size for the object.
	 *
	 * The maximum size of the hardware's virtual address space
	 * is 4GB and to reduce the pagetable size we limit a single
	 * open drm filehandle to be able to map less than that
	 * (currently 1GB), hence the size of an object is limited to
	 * a 32bit value.
	 *
	 * The (hardware 64k page) rounded-up size for the object will also
	 * be returned in "size".
	 */
	__u32 size;
	/**
	 * Allocation flags
	 */
	__u32 flags;
	/**
	 * Returned GEM handle for the object.
	 *
	 * Object handles are nonzero unsigned integers.
	 */
	__u32 handle;
	/**
	 * Returned hardware virtual address of the object. The new object's
	 * pages are mapped in the file descriptor's MMU pagetable at
	 * this address until the object is closed.
	 *
	 * The hardware virtual address space uses 32bit addresses, although
	 * this may be mapped to portions of a 40bit physical address space
	 * above the 4GB boundary on some platforms.
	 */
	__u32 hw_addr;
	/**
	 * Optional description string for debug
	 */
	const char __user *desc;
};

struct drm_v3d_gem_mmap_offset {
	/** Handle for the object being mapped. */
	__u32 handle;
	__u32 pad;
	/**
	 * Returned offset to use for subsequent mmap call
	 *
	 * This is a fixed-size type for 32/64 compatibility.
	 */
	__u64 offset;
};

/**
 * This is used to wrap an external set of contiguous physical
 * pages as a GEM object, with a virtual mapping in the MMU
 * pagetable. It is required for interoperability with Nexus.
 *
 * Note: these GEM objects will not be able to be mapped back
 * into userspace, any attempt to do so will result in an error.
 */
struct drm_v3d_gem_create_ext {
	/**
	 * Physical address of the start of the external object.
	 *
	 * This does not have to be aligned to any particular page alignment.
	 */
	__u64 phys;
	/**
	 * Size of the object.
	 *
	 * This does not have to be a multiple of any particular page size.
	 */
	__u32 size;
	/**
	 * Only V3D_CREATE_HW_READONLY is valid in this call
	 */
	__u32 flags;
	/**
	 * Returned GEM handle for the object.
	 *
	 * Object handles are nonzero unsigned integers.
	 */
	__u32 handle;
	/**
	 * Returned hardware virtual address of the object. The new object's
	 * pages are mapped in the file descriptor's MMU pagetable at
	 * this address until the object is closed.
	 */
	__u32 hw_addr;
	/**
	 * Optional description string for debug
	 */
	const char __user *desc;
};

struct drm_v3d_file_private_token {
	__u64 token;
};

#define DRM_V3D_GET_MMU_PAGETABLE 0x1
#define DRM_V3D_GEM_CREATE	  0x2
#define DRM_V3D_GEM_MMAP_OFFSET	  0x3
#define DRM_V3D_GEM_CREATE_EXT    0x4
#define DRM_V3D_GET_MEM_TOTAL     0x5
#define DRM_V3D_GET_FILE_TOKEN    0x6
#define DRM_V3D_SET_CLIENT_TERM   0x7
#define DRM_V3D_CLEAR_PT_ON_CLOSE 0x8

#define DRM_IOCTL_V3D_GET_MMU_PAGETABLE DRM_IOR(DRM_COMMAND_BASE + DRM_V3D_GET_MMU_PAGETABLE, struct drm_v3d_mmu_pt)
#define DRM_IOCTL_V3D_GEM_CREATE	DRM_IOWR(DRM_COMMAND_BASE + DRM_V3D_GEM_CREATE, struct drm_v3d_gem_create)
#define DRM_IOCTL_V3D_GEM_MMAP_OFFSET	DRM_IOWR(DRM_COMMAND_BASE + DRM_V3D_GEM_MMAP_OFFSET, struct drm_v3d_gem_mmap_offset)
#define DRM_IOCTL_V3D_GEM_CREATE_EXT	DRM_IOWR(DRM_COMMAND_BASE + DRM_V3D_GEM_CREATE_EXT, struct drm_v3d_gem_create_ext)
#define DRM_IOCTL_V3D_GET_MEM_TOTAL	DRM_IOR(DRM_COMMAND_BASE + DRM_V3D_GET_MEM_TOTAL, struct drm_v3d_mem_total)
#define DRM_IOCTL_V3D_GET_FILE_TOKEN	DRM_IOR(DRM_COMMAND_BASE + DRM_V3D_GET_FILE_TOKEN, struct drm_v3d_file_private_token)
#define DRM_IOCTL_V3D_SET_CLIENT_TERM	DRM_IOW(DRM_COMMAND_BASE + DRM_V3D_SET_CLIENT_TERM, struct drm_v3d_file_private_token)
#define DRM_IOCTL_V3D_CLEAR_PT_ON_CLOSE	DRM_IO(DRM_COMMAND_BASE + DRM_V3D_CLEAR_PT_ON_CLOSE)

#endif /* _UAPI_BRCM_V3D_DRM_H_ */
