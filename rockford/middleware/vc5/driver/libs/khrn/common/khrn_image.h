/*=============================================================================
  Broadcom Proprietary and Confidential. (c)20013 Broadcom.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
   khrn_image description
=============================================================================*/
#ifndef KHRN_IMAGE_H
#define KHRN_IMAGE_H

#include "../glxx/gl_public_api.h"
#include "khrn_blob.h"
#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"

#include "libs/platform/v3d_imgconv.h"

#include "../glxx/glxx_server_state.h"

typedef struct
{
   /* pointer to the blob that contains this level */
   KHRN_BLOB_T *blob;

   unsigned level;   /* which level in the blob is this image for;
                      (blob->desc[level] gives us the info about this image */

   /* specifing elem makes sense only for array images, otheriwse start_elem =0
    * and num_array_elems = 0; a cube is the same as an array with 6 elements */
   unsigned start_elem;      /* at which element in the blob starts this image */
   unsigned num_array_elems; /* how many elements (from start_elem) in the blob
                                are for this image */

   /* specifying a slice makes sense only for 3D images, otherwise, start_slice
    * = 0 and num_slices=1 */
   unsigned start_slice;    /* at what slice in the blob starts this image */
   unsigned num_slices;     /* how many slices (form start slice) in the blob
                               are for this image */

   /* Original format set at the API level when image was created.
    * Format in blob needs to meet HW requirements for the
    * HW blocks that access the image (TMU, TLB, etc).
    * See middleware/khronos/doc/multisample.md for more details
    * of how we represent multi-sample formats in hardware */
   GFX_LFMT_T api_fmt;

} KHRN_IMAGE_T;

/* For functions below, the return KHRN_IMAGE_T* is allocated with
 * khrn_mem_alloc so it needs to be used
 * accordingly */

/* this image refers to all the slices in the blob;
 * this function increments the refcount of the blob */
extern KHRN_IMAGE_T* khrn_image_create(KHRN_BLOB_T* blob,
      unsigned start_elem, unsigned num_array_elems, unsigned level, GFX_LFMT_T api_fmt);

/* this image refers to one element and a slice in the blob; */
extern KHRN_IMAGE_T* khrn_image_create_one_elem_slice(KHRN_BLOB_T* blob,
      unsigned elem, unsigned slice, unsigned level, GFX_LFMT_T api_fmt);

static inline GFX_BUFFER_DESC_T const* khrn_image_get_desc(const KHRN_IMAGE_T *image)
{
   KHRN_BLOB_T const* blob = image->blob;
   assert(image->level < blob->num_mip_levels);
   return &blob->desc[image->level];
}

static inline GFX_BUFFER_DESC_PLANE_T const* khrn_image_get_plane_desc(const KHRN_IMAGE_T *image, unsigned plane_index)
{
   GFX_BUFFER_DESC_T const* desc = khrn_image_get_desc(image);
   assert(plane_index < desc->num_planes);
   return &desc->planes[plane_index];
}

extern GFX_LFMT_T khrn_image_get_lfmt(const KHRN_IMAGE_T *img,
      unsigned plane);

extern void khrn_image_get_lfmts(const KHRN_IMAGE_T *img,
      GFX_LFMT_T lfmts[GFX_BUFFER_MAX_PLANES], unsigned *num_planes);

/* Like get_lfmts but just returns formats, ie no dims/layout */
extern void khrn_image_get_fmts(const KHRN_IMAGE_T *img,
      GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES], unsigned *num_planes);

extern unsigned khrn_image_get_max_levels(const KHRN_IMAGE_T *img);

/* returns true if img1 has the dimensions and lfmt required to be mip map
 * level "mip_level" of img2 */
extern bool khrn_image_is_miplevel(const KHRN_IMAGE_T *img1,
      unsigned mip_level, const KHRN_IMAGE_T *img2);

/* returns true if img1 and img2 have the same width, height, depth and their
 * fmts are equal */
extern bool khrn_image_match_fmt_and_dim(const KHRN_IMAGE_T *img1,
      const KHRN_IMAGE_T *img2);

/* returns true if img1 and img2 have the same format (from layout format)*/
extern bool khrn_image_match_fmt(const KHRN_IMAGE_T *img1,
      const KHRN_IMAGE_T *img2);

extern bool khrn_image_match_dim_and_fmt(const KHRN_IMAGE_T *img,
      unsigned width, unsigned height, unsigned depth, unsigned
      num_array_elems, const GFX_LFMT_T *lfmts, unsigned num_planes);

/* For all the copy functions below, the specified rectangles from src and
 * destination must lie inside the src and dest rectangles*/

/* the images must have the same number of elements
 * and same number of slices */
extern bool khrn_image_copy(KHRN_IMAGE_T *img_dst, const KHRN_IMAGE_T *img_src,
      glxx_context_fences *fences, bool secure_context);

extern bool khrn_image_copy_from_ptr_tgt(KHRN_IMAGE_T *dst,
      unsigned dst_x, unsigned dst_y, unsigned dst_z,
      unsigned dst_start_elem, const struct v3d_imgconv_ptr_tgt *src,
      unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems, glxx_context_fences *fences,
      bool secure_context);

extern bool khrn_image_copy_to_ptr_tgt(KHRN_IMAGE_T *src,
      unsigned src_x, unsigned src_y, unsigned src_z,
      unsigned src_start_elem, struct v3d_imgconv_ptr_tgt *dst,
      unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems,
      glxx_context_fences *fences,
      bool secure_context);

extern bool khrn_image_copy_one_elem_slice(KHRN_IMAGE_T *dst, unsigned dst_x,
      unsigned dst_y, unsigned dst_z, unsigned dst_start_elem, KHRN_IMAGE_T
      *src, unsigned src_x, unsigned src_y, unsigned src_z, unsigned
      src_start_elem, unsigned width, unsigned height,
      glxx_context_fences *fences,
      bool secure_context);

extern bool khrn_image_subsample(KHRN_IMAGE_T *dst, KHRN_IMAGE_T const *src,
     glxx_context_fences *fences);

extern bool khrn_image_generate_mipmaps_tfu(
   KHRN_IMAGE_T* src_image,
   KHRN_IMAGE_T* const* dst_images,
   unsigned num_dst_levels,
   bool skip_dst_level_0,
   glxx_context_fences *fences,
   bool secure_context
   );

/* helper functions */
extern unsigned khrn_image_get_depth(const KHRN_IMAGE_T *img);
extern unsigned khrn_image_get_width(const KHRN_IMAGE_T *img);
extern unsigned khrn_image_get_height(const KHRN_IMAGE_T *img);
extern unsigned khrn_image_get_num_elems(const KHRN_IMAGE_T *img);
extern void khrn_image_get_dimensions(const KHRN_IMAGE_T *img, unsigned
      *width, unsigned *height, unsigned *depth, unsigned *num_elems);
extern unsigned khrn_image_get_num_planes(const KHRN_IMAGE_T *img);

extern KHRN_RES_INTERLOCK_T*
khrn_image_get_res_interlock(const KHRN_IMAGE_T *img);

extern  void khrn_image_translate_rcfg_color(const KHRN_IMAGE_T *img,
   unsigned plane, unsigned frame_width, unsigned frame_height,
   GFX_BUFFER_RCFG_COLOR_TRANSLATION_T *t);

extern v3d_memory_format_t khrn_image_translate_memory_format(
      const KHRN_IMAGE_T *img, unsigned plane);

extern unsigned khrn_image_maybe_uif_height_in_ub(const KHRN_IMAGE_T *img,
      unsigned plane);

extern unsigned khrn_image_uif_height_in_ub(const KHRN_IMAGE_T *img,
      unsigned plane);

/* offset from base pointer to the start of memory for the spefified plane, in bytes;
 * if the image is an element in an array, then this offset refers to the start
 * of that memory for that element for the specified plane;
 * if the image is a slice, it will also add the offset to the start_slice;
 */
extern unsigned khrn_image_get_offset(const KHRN_IMAGE_T *img,
      unsigned plane);

/* return true if this image is one element and one slice; eg: 2D images are
 * always one element and one slice; */
extern bool khrn_image_is_one_elem_slice(const KHRN_IMAGE_T *img);

/* return true if img1 and img2 are equal (they wrap the same blob, refer to
 * the same slice and element in the blob, and have the same number of elements */
extern bool khrn_image_equal(const KHRN_IMAGE_T *img1, const KHRN_IMAGE_T *img2);

#endif
