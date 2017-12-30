/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_BLOB_H
#define KHRN_BLOB_H

#include "../glxx/gl_public_api.h"
#include "libs/core/lfmt/lfmt.h"
#include "libs/core/gfx_buffer/gfx_buffer.h"
#include "khrn_resource.h"
#include "khrn_mem.h"

typedef struct khrn_blob
{
   gfx_buffer_usage_t usage;

   unsigned num_array_elems;  /* for GL_TEXTURE_1D_ARRAY or TEXTURE_2D_ARRAY
                                 is the number of elements in the array;
                                 for GL_TEXTURE_CUBE_MAP_ARRAY is the number
                                 of elements in the array * 6,
                                 for GL_TEXTURE_CUBE_MAP is 6,
                                 1 for anything else */
   unsigned num_mip_levels;

   /* for array element i, the GFX_BUFFERS_DESC_T offsets are relative to the
    * start of the handle in this resource plus (i * array_pitch) ;
    * for a blob that has one element, array_pitch is the size of that element*/
   size_t array_pitch;
   khrn_resource *res;

   /* If this is true, we did not create the resource and so we do not know
    * how the resource parts are assigned */
   bool foreign_resource;

   size_t align;        /*alignment requirement for resource */

   bool   secure;

   /* desc[0] to desc[num_mip_leves-1] describe the images in this blob */
   GFX_BUFFER_DESC_T desc[];

}khrn_blob;

/* There are either 1 or 2 "changrps" (channel groups) per plane. When there
 * are 2 per plane, the first contains the stencil channel (if there is one)
 * and the second contains all other channels. */
static inline unsigned khrn_blob_changrps_per_plane(const khrn_blob *blob)
{
   for (unsigned plane = 0; plane != blob->desc[0].num_planes; ++plane)
   {
      uint32_t channels = gfx_lfmt_present_channels(blob->desc[0].planes[plane].lfmt);
      if ((channels & GFX_LFMT_CHAN_S_BIT) && (channels & ~GFX_LFMT_CHAN_S_BIT))
         return 2;
   }
   return 1;
}

extern khrn_resource_parts_t khrn_blob_resource_parts(const khrn_blob *blob,
   unsigned start_elem, unsigned num_elems, /* Array elements */
   unsigned mip_level,
   /* changrps are numbered from 0 to (num_planes * khrn_blob_changrps_per_plane()) - 1 */
   unsigned start_changrp, unsigned num_changrps,
   unsigned start_slice, unsigned num_slices,
   /* If subset, parts returned will be a subset of the parts specified,
    * otherwise a superset */
   bool subset);

/* the returned blob is allocated with khrn_mem_alloc so it needs to be used
 * accordingly;
 * blob_usage decides the alignment of the allocated storage (see
 * GFX_BUFFER_USAGE_V3D_.*) */
extern khrn_blob* khrn_blob_create(unsigned width, unsigned height,
      unsigned depth, unsigned num_array_elems, unsigned num_mip_levels,
      const GFX_LFMT_T *lfmts, unsigned num_planes,
      gfx_buffer_usage_t blob_usage, gmem_usage_flags_t gmem_usage);

/* one has to supply as many descs as the number of mipmap levels;
 * for an image that is not an array, array_pitch is the size of handle.
 * Ownership of res is not transferred; the blob will have a new reference to
 * the resource. */
extern khrn_blob* khrn_blob_create_from_resource(khrn_resource *res,
      const GFX_BUFFER_DESC_T *descs, unsigned num_mip_levels,
      unsigned num_array_elems, unsigned array_pitch,
      gfx_buffer_usage_t blob_usage, bool secure);

/* Similar to creating a new resource with khrn_resource_create_with_handles
 * then calling khrn_blob_create_from_resource, but the resource will not be
 * considered foreign (see foreign_resource) */
extern khrn_blob* khrn_blob_create_from_handles(
      unsigned num_handles, const gmem_handle_t *handles,
      const GFX_BUFFER_DESC_T *descs, unsigned num_mip_levels,
      unsigned num_array_elems, unsigned array_pitch,
      gfx_buffer_usage_t blob_usage);

/* creates a blob as khrn_blob_create but it doesn't allocate storage at this
 * point for the resource handle; we will allocate the storage at some
 * later point (only if we need to use this blob) */
extern khrn_blob* khrn_blob_create_no_storage(unsigned width, unsigned height,
      unsigned depth, unsigned num_array_elems, unsigned num_mip_levels,
      const GFX_LFMT_T *lfmts, unsigned num_planes,
      gfx_buffer_usage_t blob_usage, bool secure);

extern khrn_blob* khrn_blob_shallow_copy(const khrn_blob *other);

/* this can be used in conjunction with the above function to delay storage
 * allocation till needed; if the blob already has storage, this function does nothing */
extern bool khrn_blob_alloc_storage(khrn_blob *blob);

extern bool khrn_blob_contains_level(const khrn_blob *blob,
      unsigned mip_level);

extern bool khrn_blob_has_level_with_spec(const khrn_blob *blob,
      unsigned level, unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems, const GFX_LFMT_T *lfmts, unsigned num_planes);

#endif
