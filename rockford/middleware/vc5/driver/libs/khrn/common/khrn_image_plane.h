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

static inline GFX_BUFFER_DESC_PLANE_T const* khrn_image_plane_desc(const KHRN_IMAGE_PLANE_T *img_plane)
{
   return khrn_image_get_plane_desc(img_plane->image, img_plane->plane_idx);
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

static inline
void khrn_image_plane_assign(KHRN_IMAGE_PLANE_T *lhs, const KHRN_IMAGE_PLANE_T *rhs)
{
   if (rhs != NULL)
   {
      KHRN_MEM_ASSIGN(lhs->image, rhs->image);
      lhs->plane_idx = rhs->plane_idx;
   }
   else
   {
      KHRN_MEM_ASSIGN(lhs->image, NULL);
      /* plane_idx undefined */
   }
}

static inline
void khrn_image_plane_translate_rcfg_color(const KHRN_IMAGE_PLANE_T *img_plane,
      uint32_t frame_width, uint32_t frame_height,
      GFX_BUFFER_RCFG_COLOR_TRANSLATION_T *t)
{
   khrn_image_translate_rcfg_color(img_plane->image, img_plane->plane_idx,
         frame_width, frame_height, t);
}

static inline
v3d_memory_format_t khrn_image_plane_translate_memory_format(
      const KHRN_IMAGE_PLANE_T *img_plane)
{
   return khrn_image_translate_memory_format(img_plane->image,
         img_plane->plane_idx);
}

static inline
unsigned khrn_image_plane_maybe_uif_height_in_ub(
      const KHRN_IMAGE_PLANE_T *img_plane)
{
   return khrn_image_maybe_uif_height_in_ub(img_plane->image,
         img_plane->plane_idx);
}
static inline unsigned khrn_image_plane_uif_height_in_ub(
   const KHRN_IMAGE_PLANE_T *img_plane)
{
   return khrn_image_uif_height_in_ub(img_plane->image,
         img_plane->plane_idx);
}

/* see GMEM_SYNC_.* in v3d_mem_api.h for valid combinations of r/w flags */
extern v3d_addr_t khrn_image_plane_lock(const KHRN_IMAGE_PLANE_T *img_plane,
      KHRN_FMEM_T *fmem, uint32_t bin_rw_flags, uint32_t render_rw_flags);
#endif
