/*=============================================================================
  Broadcom Proprietary and Confidential. (c)20013 Broadcom.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
   Implementation of khrn_image_plane
=============================================================================*/
#include "khrn_image_plane.h"
#include "libs/platform/gmem.h"

v3d_addr_t khrn_image_plane_lock(const KHRN_IMAGE_PLANE_T *img_plane,
      KHRN_FMEM_T *fmem, uint32_t bin_rw_flags, uint32_t render_rw_flags)
{
   KHRN_RES_INTERLOCK_T *res_i;
   v3d_addr_t addr;

   assert(fmem && img_plane->image);

   res_i = khrn_image_get_res_interlock(img_plane->image);
   addr = khrn_fmem_lock_and_sync(fmem, res_i->handle, bin_rw_flags,
         render_rw_flags);
   addr += khrn_image_get_offset(img_plane->image, img_plane->plane_idx);
   return addr;
}
