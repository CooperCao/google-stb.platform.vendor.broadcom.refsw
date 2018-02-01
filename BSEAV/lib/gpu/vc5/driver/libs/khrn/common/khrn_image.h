/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_IMAGE_H
#define KHRN_IMAGE_H

#include "../glxx/gl_public_api.h"
#include "khrn_blob.h"

#include "libs/platform/v3d_imgconv.h"

#include "../glxx/glxx_server_state.h"

typedef struct khrn_image
{
   /* pointer to the blob that contains this level */
   khrn_blob *blob;

   unsigned level;   /* which level in the blob is this image for;
                      (blob->desc[level] gives us the info about this image */

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

} khrn_image;

/* For functions below, the return khrn_image* is allocated with
 * khrn_mem_alloc so it needs to be used
 * accordingly */

/* this image refers to all the slices in the blob;
 * this function increments the refcount of the blob */
extern khrn_image* khrn_image_create(khrn_blob* blob,
      unsigned start_elem, unsigned num_array_elems, unsigned level, GFX_LFMT_T api_fmt);

/* this image refers to one element and a slice in the blob; */
extern khrn_image* khrn_image_create_one_elem_slice(khrn_blob* blob,
      unsigned elem, unsigned slice, unsigned level, GFX_LFMT_T api_fmt);

extern khrn_image* khrn_image_shallow_blob_copy(const khrn_image* other);

extern GFX_LFMT_T khrn_image_get_lfmt(const khrn_image *img,
      unsigned plane);

extern void khrn_image_get_lfmts(const khrn_image *img,
      GFX_LFMT_T lfmts[GFX_BUFFER_MAX_PLANES], unsigned *num_planes);

/* Like get_lfmts but just returns formats, ie no dims/layout */
extern void khrn_image_get_fmts(const khrn_image *img,
      GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES], unsigned *num_planes);

extern unsigned khrn_image_get_max_levels(const khrn_image *img);

/* returns true if img1 has the dimensions and lfmt required to be mip map
 * level "mip_level" of img2 */
extern bool khrn_image_is_miplevel(const khrn_image *img1,
      unsigned mip_level, const khrn_image *img2);

/* returns true if img1 and img2 have the same width, height, depth and their
 * fmts are equal */
extern bool khrn_image_match_fmt_and_dim(const khrn_image *img1,
      const khrn_image *img2);

/* returns true if img1 and img2 have the same format (from layout format)*/
extern bool khrn_image_match_fmt(const khrn_image *img1,
      const khrn_image *img2);

extern bool khrn_image_match_dim_and_fmt(const khrn_image *img,
      unsigned width, unsigned height, unsigned depth, unsigned
      num_array_elems, const GFX_LFMT_T *lfmts, unsigned num_planes);

/* For all the copy functions below, the specified rectangles from src and
 * destination must lie inside the src and dest rectangles*/

/* the images must have the same number of elements
 * and same number of slices */
extern bool khrn_image_convert(khrn_image *img_dst, const khrn_image *img_src,
      glxx_context_fences *fences, bool secure_context);

extern void khrn_image_calc_dst_usage(
      const khrn_image *src, unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems, unsigned num_mip_levels, const GFX_LFMT_T *lfmts,
      unsigned num_planes, bool secure_context,
      gfx_buffer_usage_t *dst_buf_usage, gmem_usage_flags_t *gmem_usage);

extern bool khrn_image_convert_from_ptr_tgt(khrn_image *dst,
      unsigned dst_x, unsigned dst_y, unsigned dst_z,
      unsigned dst_start_elem, const struct v3d_imgconv_ptr_tgt *src,
      unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems, glxx_context_fences *fences,
      bool secure_context);

extern bool khrn_image_convert_to_ptr_tgt(khrn_image *src,
      unsigned src_x, unsigned src_y, unsigned src_z,
      unsigned src_start_elem, struct v3d_imgconv_ptr_tgt *dst,
      unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems,
      glxx_context_fences *fences,
      bool secure_context);

extern bool khrn_image_convert_one_elem_slice(khrn_image *dst, unsigned dst_x,
      unsigned dst_y, unsigned dst_z, unsigned dst_start_elem, khrn_image
      *src, unsigned src_x, unsigned src_y, unsigned src_z, unsigned
      src_start_elem, unsigned width, unsigned height,
      glxx_context_fences *fences,
      bool secure_context);

extern bool khrn_image_memcpy_one_elem_slice(khrn_image *dst, unsigned dst_x,
      unsigned dst_y, unsigned dst_z, unsigned dst_start_elem, khrn_image
      *src, unsigned src_x, unsigned src_y, unsigned src_z, unsigned
      src_start_elem, unsigned src_width, unsigned src_height,
      glxx_context_fences *fences,
      bool secure_context);

extern bool khrn_image_subsample(khrn_image *dst, khrn_image const *src,
     bool force_no_srgb, glxx_context_fences *fences);

extern bool khrn_image_generate_mipmaps_tfu(
   khrn_image* src_image,
   khrn_image* const* dst_images,
   unsigned num_dst_levels,
   bool skip_dst_level_0, bool force_no_srgb,
   glxx_context_fences *fences,
   bool secure_context
   );

/* helper functions */
extern unsigned khrn_image_get_depth(const khrn_image *img);
extern unsigned khrn_image_get_width(const khrn_image *img);
extern unsigned khrn_image_get_height(const khrn_image *img);
extern unsigned khrn_image_get_num_elems(const khrn_image *img);
extern void khrn_image_get_dimensions(const khrn_image *img, unsigned
      *width, unsigned *height, unsigned *depth, unsigned *num_elems);
extern unsigned khrn_image_get_num_planes(const khrn_image *img);

static inline khrn_resource *khrn_image_get_resource(const khrn_image *img)
{
   return img->blob->res;
}

static inline khrn_resource_parts_t khrn_image_resource_parts(
   const khrn_image *img, bool subset)
{
   return khrn_blob_resource_parts(img->blob,
      img->start_elem, img->num_array_elems,
      img->level,
      0, img->blob->desc[0].num_planes * khrn_blob_changrps_per_plane(img->blob),
      img->start_slice, img->num_slices,
      subset);
}

static inline void khrn_image_invalidate(khrn_image *img)
{
   khrn_resource_mark_undefined(img->blob->res,
      khrn_image_resource_parts(img, /*subset=*/true));
}

/* offset from base pointer to the start of memory for the spefified plane, in bytes;
 * if the image is an element in an array, then this offset refers to the start
 * of that memory for that element for the specified plane;
 * if the image is a slice, it will also add the offset to the start_slice;
 */
extern unsigned khrn_image_get_offset(const khrn_image *img, unsigned plane);

// Returns address of specified plane, same semantics as khrn_image_get_offset
extern v3d_addr_t khrn_image_get_addr(const khrn_image *img, unsigned plane);

/* return true if this image is one element and one slice; eg: 2D images are
 * always one element and one slice; */
extern bool khrn_image_is_one_elem_slice(const khrn_image *img);

/* return true if img1 and img2 are equal (they wrap the same blob, refer to
 * the same slice and element in the blob, and have the same number of slices and elements */
extern bool khrn_image_equal(const khrn_image *img1, const khrn_image *img2);

static inline
const GFX_BUFFER_DESC_PLANE_T *khrn_image_get_desc_plane(const khrn_image *img, unsigned plane)
{
   assert(plane < img->blob->desc[img->level].num_planes);
   return &img->blob->desc[img->level].planes[plane];
}

static inline
const GFX_BUFFER_DESC_T *khrn_image_get_desc(const khrn_image *img)
{
   return &img->blob->desc[img->level];
}

#endif
