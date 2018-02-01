/*
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * Copyright (C) Paul Mackerras 2005
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

#include <linux/module.h>
#include <linux/compat.h>
#include <uapi/drm/brcmv3d_drm.h>

#include "v3d_drv.h"

struct drm_v3d_gem_create32 {
	__u32 size;
	__u32 flags;
	__u32 handle;
	__u32 hw_addr;
	__u32 desc; /* placeholder for 32bit pointer */
};

struct drm_v3d_gem_create_ext32 {
	__u64 phys;
	__u32 size;
	__u32 flags;
	__u32 handle;
	__u32 hw_addr;
	__u32 desc; /* placeholder for 32bit pointer */
};

static int compat_v3d_gem_create(struct file *filp, unsigned int cmd,
				 unsigned long arg)
{
	struct drm_v3d_gem_create32 req32;
	struct drm_v3d_gem_create __user *request;
	int err;

	if (copy_from_user(&req32, (void __user *)arg, sizeof(req32)))
		return -EFAULT;

	request = compat_alloc_user_space(sizeof(*request));
	if (!access_ok(VERIFY_WRITE, request, sizeof(*request)) ||
	    __put_user(req32.size, &request->size)              ||
	    __put_user(req32.flags, &request->flags)            ||
	    __put_user((void __user *)(unsigned long)req32.desc,
		       &request->desc))
		return -EFAULT;

	err = drm_ioctl(filp, DRM_IOCTL_V3D_GEM_CREATE,
			(unsigned long)request);

	if (err)
		return err;

	if (__get_user(req32.handle, &request->handle) ||
	    __get_user(req32.size, &request->size)     ||
	    __get_user(req32.hw_addr, &request->hw_addr))
		return -EFAULT;

	if (copy_to_user((void __user *)arg, &req32, sizeof(req32)))
		return -EFAULT;

	return 0;
}

static int compat_v3d_gem_create_ext(struct file *filp, unsigned int cmd,
				     unsigned long arg)
{
	struct drm_v3d_gem_create_ext32 req32;
	struct drm_v3d_gem_create_ext __user *request;
	int err;

	if (copy_from_user(&req32, (void __user *)arg, sizeof(req32)))
		return -EFAULT;

	request = compat_alloc_user_space(sizeof(*request));
	if (!access_ok(VERIFY_WRITE, request, sizeof(*request)) ||
	    __put_user(req32.phys, &request->phys)              ||
	    __put_user(req32.size, &request->size)              ||
	    __put_user(req32.flags, &request->flags)            ||
	    __put_user((void __user *)(unsigned long)req32.desc,
		       &request->desc))
		return -EFAULT;

	err = drm_ioctl(filp, DRM_IOCTL_V3D_GEM_CREATE_EXT,
			(unsigned long)request);

	if (err)
		return err;

	if (__get_user(req32.handle, &request->handle) ||
	    __get_user(req32.size, &request->size)     ||
	    __get_user(req32.hw_addr, &request->hw_addr))
		return -EFAULT;

	if (copy_to_user((void __user *)arg, &req32, sizeof(req32)))
		return -EFAULT;

	return 0;
}

drm_ioctl_compat_t *v3d_compat_ioctls[] = {
	[DRM_V3D_GEM_CREATE] = compat_v3d_gem_create,
	[DRM_V3D_GEM_CREATE_EXT] = compat_v3d_gem_create_ext,
};

long v3d_drm_compat_ioctl(struct file *filp, unsigned int cmd,
			  unsigned long arg)
{
	unsigned int nr = DRM_IOCTL_NR(cmd);
	drm_ioctl_compat_t *fn = NULL;
	int ret;

	if (nr < DRM_COMMAND_BASE)
		return drm_compat_ioctl(filp, cmd, arg);

	if (nr < DRM_COMMAND_BASE + ARRAY_SIZE(v3d_compat_ioctls))
		fn = v3d_compat_ioctls[nr - DRM_COMMAND_BASE];

	if (fn)
		ret = (*fn) (filp, cmd, arg);
	else
		ret = drm_ioctl(filp, cmd, arg);

	return ret;
}
