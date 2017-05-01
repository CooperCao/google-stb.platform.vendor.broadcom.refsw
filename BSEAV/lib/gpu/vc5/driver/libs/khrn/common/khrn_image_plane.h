/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_IMAGE_PLANE_H
#define KHRN_IMAGE_PLANE_H

#include "khrn_image.h"
#include "khrn_fmem.h"

typedef struct khrn_image_plane
{
   khrn_image *image;
   uint32_t plane_idx;
} khrn_image_plane;


static inline uint32_t khrn_image_plane_equal(const khrn_image_plane *lhs,
   const khrn_image_plane *rhs)
{
   /* If both images are missing, consider image planes equal */
   if ((lhs->image == NULL) && (rhs->image == NULL))
      return true;

   /* If one image is missing, one is not, consider image planes not equal */
   if ((lhs->image == NULL) || (rhs->image == NULL))
      return false;

   /* Both images are present, compare */

   /* plane_idx is only valid when images are present */
   if (lhs->plane_idx != rhs->plane_idx)
      return false;

   return khrn_image_equal(lhs->image, rhs->image);
}

static inline
GFX_LFMT_T khrn_image_plane_lfmt(const khrn_image_plane *img_plane)
{
   return khrn_image_get_lfmt(img_plane->image, img_plane->plane_idx);

}
static inline
GFX_LFMT_T khrn_image_plane_lfmt_maybe(const khrn_image_plane *img_plane)
{
   return (img_plane && img_plane->image) ?
      khrn_image_get_lfmt(img_plane->image, img_plane->plane_idx) :
      GFX_LFMT_NONE;
}

typedef enum
{
   KHRN_CHANGRP_STENCIL = 1u << 0,
   KHRN_CHANGRP_NONSTENCIL = 1u << 1,
   KHRN_CHANGRP_ALL = KHRN_CHANGRP_STENCIL | KHRN_CHANGRP_NONSTENCIL
} khrn_changrps_t;

static inline khrn_resource_parts_t khrn_image_plane_resource_parts(
   const khrn_image_plane *img_plane, khrn_changrps_t changrps, bool subset)
{
   assert(changrps);

   const khrn_image *img = img_plane->image;
   if (!img)
      return 0;

   unsigned changrps_per_plane = khrn_blob_changrps_per_plane(img->blob);
   unsigned start_changrp;
   unsigned num_changrps;
   if (changrps_per_plane == 1)
   {
      start_changrp = img_plane->plane_idx;
      num_changrps = 1;
   }
   else
   {
      assert(changrps_per_plane == 2);
      start_changrp = (img_plane->plane_idx * 2) + ((changrps & KHRN_CHANGRP_STENCIL) ? 0 : 1);
      num_changrps = ((changrps & KHRN_CHANGRP_STENCIL) ? 1 : 0) +
         ((changrps & KHRN_CHANGRP_NONSTENCIL) ? 1 : 0);
   }

   return khrn_blob_resource_parts(img->blob,
      img->start_elem, img->num_array_elems,
      img->level,
      start_changrp, num_changrps,
      img->start_slice, img->num_slices,
      subset);
}

static inline void khrn_image_plane_invalidate(khrn_image_plane *img_plane,
   khrn_changrps_t changrps)
{
   if (img_plane->image)
      khrn_resource_mark_undefined(khrn_image_get_resource(img_plane->image),
         khrn_image_plane_resource_parts(img_plane, changrps, /*subset=*/true));
}

#endif
