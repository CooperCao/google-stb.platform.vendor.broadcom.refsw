/*=============================================================================
  Copyright (c) 20013 Broadcom Europe Limited.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
   blob description
=============================================================================*/
#ifndef KHRN_BLOB_H
#define KHRN_BLOB_H

#include "interface/khronos/glxx/gl_public_api.h"
#include "helpers/gfx/gfx_lfmt.h"
#include "helpers/gfx/gfx_buffer.h"
#include "middleware/khronos/common/khrn_res_interlock.h"
#include "middleware/khronos/common/khrn_mem.h"

typedef struct
{
   gfx_buffer_usage_t usage;

   unsigned num_array_elems;  /* for GL_TEXTURE_1D_ARRAY or TEXTURE_2D_ARRAY
                                 is the number of elements in the array;
                                 for GL_TEXTURE_CUBE_MAP is 6,
                                 1 for anything else */
   unsigned num_mip_levels;

   /* for array element i, the GFX_BUFFERS_DESC_T offsets are relative to the
    * start of the handle in this resource plus (i * array_pitch) ;
    * for a blob that has one element, array_pitch is the size of that element*/
   size_t array_pitch;
   KHRN_RES_INTERLOCK_T *res_i;

   size_t descs_align;        /*alignment requirement for descs */

   /* Note: desc is a flexible array and must be the last member in this data
    * struct;
    * decs[0] to desc[num_mip_leves-1] describe the images in this blob */
   GFX_BUFFER_DESC_T desc[1];

}KHRN_BLOB_T;

/* the returned blob is allocated with khrn_mem_alloc so it needs to be used
 * accordingly;
 * blob_usage decides the alignment of the allocated storage (see
 * GFX_BUFFER_USAGE_V3D_.*) */
extern KHRN_BLOB_T* khrn_blob_create(unsigned width, unsigned height,
      unsigned depth, unsigned num_array_elems, unsigned num_mip_levels,
      const GFX_LFMT_T *lfmts, unsigned num_planes,
      gfx_buffer_usage_t blob_usage);

/* one has to supply as many descs as the number of mipmap levels;
 * for an image that is not an array, array_pitch is the size of handle */
extern KHRN_BLOB_T* khrn_blob_create_from_storage(gmem_handle_t handle,
      const GFX_BUFFER_DESC_T *descs, unsigned num_mip_levels,
      unsigned num_array_elems, unsigned array_pitch,
      gfx_buffer_usage_t blob_usage);

/* The same as khrn_blob_create_from_storage but the storage is created
 * obtained from a res interlock */
extern KHRN_BLOB_T* khrn_blob_create_from_res_interlock(KHRN_RES_INTERLOCK_T *res_i,
      const GFX_BUFFER_DESC_T *descs, unsigned num_mip_levels,
      unsigned num_array_elems, unsigned array_pitch,
      gfx_buffer_usage_t blob_usage);

/*creates a blob as khrn_blob_create but it doesn't allocate storage at this
 * point for the res_interlock handle; we will allocate the storage at some
 * later point (only if we need to use this blob) */
extern KHRN_BLOB_T* khrn_blob_create_no_storage(unsigned width, unsigned height,
      unsigned depth, unsigned num_array_elems, unsigned num_mip_levels,
      const GFX_LFMT_T *lfmts, unsigned num_planes,
      gfx_buffer_usage_t blob_usage);

/* this can be used in conjunction with the above function to delay storage
 * allocation till needed; if the blob already has storage, this function does nothing */
extern bool khrn_blob_alloc_storage(KHRN_BLOB_T *blob);

extern bool khrn_blob_contains_level(const KHRN_BLOB_T *blob,
      unsigned mip_level);

extern bool khrn_blob_has_level_with_spec(const KHRN_BLOB_T *blob,
      unsigned level, unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems, const GFX_LFMT_T *lfmts, unsigned num_planes);

#endif
