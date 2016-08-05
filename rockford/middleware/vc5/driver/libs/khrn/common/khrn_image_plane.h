/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Image plane

FILE DESCRIPTION
  wrap an plane from an image
=============================================================================*/
#ifndef KHRN_IMAGE_PLANE_H
#define KHRN_IMAGE_PLANE_H

#include "khrn_image.h"
#include "khrn_fmem.h"

typedef struct {
   KHRN_IMAGE_T *image;
   uint32_t plane_idx;
} KHRN_IMAGE_PLANE_T;


static inline uint32_t khrn_image_plane_equal(const KHRN_IMAGE_PLANE_T *lhs,
   const KHRN_IMAGE_PLANE_T *rhs)
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
GFX_LFMT_T khrn_image_plane_lfmt(const KHRN_IMAGE_PLANE_T *img_plane)
{
   return khrn_image_get_lfmt(img_plane->image, img_plane->plane_idx);

}
static inline
GFX_LFMT_T khrn_image_plane_lfmt_maybe(const KHRN_IMAGE_PLANE_T *img_plane)
{
   return (img_plane && img_plane->image) ?
      khrn_image_get_lfmt(img_plane->image, img_plane->plane_idx) :
      GFX_LFMT_NONE;
}

static inline bool khrn_image_plane_invalidate(KHRN_IMAGE_PLANE_T *img_plane)
{
   if (!img_plane->image)
      return true;

   // If the image contains other planes, we cannot invalidate
   return (khrn_image_get_num_planes(img_plane->image) == 1) &&
      khrn_image_invalidate(img_plane->image);
}

static inline void khrn_image_plane_invalidate_two(
   KHRN_IMAGE_PLANE_T *a, KHRN_IMAGE_PLANE_T *b)
{
   if (a->image != b->image)
   {
      khrn_image_plane_invalidate(a);
      khrn_image_plane_invalidate(b);
   }
   else if (a->image)
   {
      // If the image contains other planes, we cannot invalidate
      unsigned num_planes = khrn_image_get_num_planes(a->image);
      if ((num_planes == 1) || ((num_planes == 2) && ((a->plane_idx + b->plane_idx) == 1)))
         khrn_image_invalidate(a->image);
   }
}

#endif
