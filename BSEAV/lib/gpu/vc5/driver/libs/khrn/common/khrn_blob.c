/*=============================================================================
  Broadcom Proprietary and Confidential. (c)2016 Broadcom.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
   Implementation of texture blob
=============================================================================*/
#include "khrn_blob.h"
#include "libs/platform/gmem.h"
#include "khrn_mem.h"
#include "khrn_options.h"

khrn_interlock_parts_t khrn_blob_interlock_parts(const KHRN_BLOB_T *blob,
   unsigned start_elem, unsigned num_elems, /* Array elements */
   unsigned mip_level,
   /* changrps are numbered from 0 to (num_planes * khrn_blob_changrps_per_plane()) - 1 */
   unsigned start_changrp, unsigned num_changrps,
   unsigned start_slice, unsigned num_slices,
   /* If subset, parts returned will be a subset of the parts specified,
    * otherwise a superset */
   bool subset)
{
   assert((start_elem + num_elems) <= blob->num_array_elems);
   assert(mip_level < blob->num_mip_levels);
   /* All mip levels should have the same number of planes */
   unsigned blob_changrps = blob->desc[0].num_planes * khrn_blob_changrps_per_plane(blob);
   assert((start_changrp + num_changrps) <= blob_changrps);
   assert((start_slice + num_slices) <= blob->desc[mip_level].depth);

   if (blob->foreign_interlock)
      /* Don't know how interlock parts are assigned! */
      return subset ? 0 : KHRN_INTERLOCK_PARTS_ALL;

   /* There is an interlock part for each channel group in each 2D image plane.
    * Part i corresponds to
    * channel group ((i / a) % b) in
    * slice (i % a) in
    * mip level ((i / (a * b)) % c) in
    * array element (i / (a * b * c))
    * where
    * a = blob->desc[0].depth
    * b = blob_changrps
    * c = blob->num_mip_levels */

   khrn_interlock_parts_t parts = 0;
   for (unsigned elem = start_elem; elem != start_elem + num_elems; ++elem)
   {
      for (unsigned changrp = start_changrp; changrp != start_changrp + num_changrps; ++changrp)
      {
         unsigned p = elem;
         p = (p * blob->num_mip_levels) + mip_level;
         p = (p * blob_changrps) + changrp;
         p = (p * blob->desc[0].depth) + start_slice;
         parts |= khrn_interlock_parts(p, num_slices);

         if (parts & KHRN_INTERLOCK_PARTS_REST)
         {
            if (subset)
            {
               bool any_other_parts =
                  (p >= KHRN_INTERLOCK_PARTS_BITS) ||
                  ((start_slice + num_slices) != blob->desc[mip_level].depth) ||
                  ((start_changrp + num_changrps) != blob_changrps) ||
                  ((changrp != (blob_changrps - 1)) && (start_slice != 0)) ||
                  (mip_level != (blob->num_mip_levels - 1)) ||
                  ((start_elem + num_elems) != blob->num_array_elems) ||
                  ((elem != (blob->num_array_elems - 1)) &&
                     ((start_slice != 0) || (start_changrp != 0) || (mip_level != 0)));
               if (any_other_parts)
                  /* There are some parts covered by KHRN_INTERLOCK_PARTS_REST
                   * that are not included in the specified range. As we were
                   * asked for a subset, we cannot include
                   * KHRN_INTERLOCK_PARTS_REST. */
                  parts &= ~KHRN_INTERLOCK_PARTS_REST;
            }
            return parts;
         }
      }
   }
   return parts;
}

static KHRN_BLOB_T* blob_alloc(unsigned num_mip_levels, unsigned num_array_elems)
{
   KHRN_BLOB_T *blob;
   size_t struct_size;

   assert(num_mip_levels > 0 && num_array_elems > 0);

   /* allocate enough space for the struct and desc flexible array */
   struct_size = sizeof(KHRN_BLOB_T) + (num_mip_levels - 1) *
      sizeof(GFX_BUFFER_DESC_T);

   blob = khrn_mem_alloc(struct_size, "KHRN_BLOB_T");
   if (blob == NULL)
      return NULL;

   return blob;
}

static bool blob_init(KHRN_BLOB_T *blob,
         unsigned width, unsigned height, unsigned depth,
         unsigned num_mip_levels, unsigned num_array_elems,
         const GFX_LFMT_T *lfmts, unsigned num_planes,
         gfx_buffer_usage_t blob_usage, bool alloc_storage,
         bool secure)
{
   assert(num_mip_levels > 0 && num_array_elems > 0);
   assert(width > 0 && height > 0 && depth > 0 && num_planes > 0);

   if (num_array_elems > 1)
      assert(depth == 1);

   blob->usage = blob_usage;

   blob->num_mip_levels = num_mip_levels;
   blob->num_array_elems = num_array_elems;

   size_t descs_size;
   gfx_buffer_desc_gen(blob->desc,
      &descs_size, &blob->descs_align, blob_usage |
      (khrn_options.prefer_yflipped ? GFX_BUFFER_USAGE_YFLIP_IF_POSSIBLE : 0),
      width, height, depth, num_mip_levels, num_planes, lfmts);

   if (blob->num_array_elems > 1)
      descs_size = gfx_zround_up(descs_size, blob->descs_align);

   blob->array_pitch  = descs_size;
   blob->secure = secure;

   size_t total_size = blob->array_pitch * blob->num_array_elems;
   if (alloc_storage)
   {
      blob->res_i = khrn_res_interlock_create(total_size, blob->descs_align,
         GMEM_USAGE_V3D_RW | (secure ? GMEM_USAGE_SECURE : GMEM_USAGE_NONE), "khrn_blob");
   }
   else
      blob->res_i = khrn_res_interlock_create_no_handle();

   if (blob->res_i == NULL)
      return false;

   blob->foreign_interlock = false;

   // Contents of blob are initially undefined
   khrn_interlock_invalidate(&blob->res_i->interlock, KHRN_INTERLOCK_PARTS_ALL);

   return true;
}

static bool blob_init_from_storage(KHRN_BLOB_T *blob, gmem_handle_t handle,
      const GFX_BUFFER_DESC_T *descs, unsigned num_mip_levels,
      unsigned num_array_elems, unsigned array_pitch,
      gfx_buffer_usage_t blob_usage)
{
   assert(handle != GMEM_HANDLE_INVALID);
   assert(num_mip_levels > 0 && num_array_elems > 0 && array_pitch > 0);

   /* if we can get the size of the handle, we should be able to assert that
    * num_array_elems * array_pitch <= size_handle */

   /* array elements cannot be 3D, so depth must be 1 */
   if (num_array_elems > 1)
      assert(descs[0].depth == 1);

   blob->usage  = blob_usage;
   blob->secure = gmem_get_usage(handle) & GMEM_USAGE_SECURE;

   blob->num_mip_levels = num_mip_levels;
   blob->num_array_elems = num_array_elems;
   blob->array_pitch = array_pitch;
   memcpy(blob->desc, descs, num_mip_levels * sizeof(GFX_BUFFER_DESC_T));

   blob->res_i = khrn_res_interlock_create_with_handle(handle);
   if (blob->res_i == NULL)
      return false;

   blob->foreign_interlock = false;

   return true;
}

static void blob_init_from_res_interlock(KHRN_BLOB_T *blob,
      KHRN_RES_INTERLOCK_T *res_i, const GFX_BUFFER_DESC_T *descs,
      unsigned num_mip_levels, unsigned num_array_elems,
      unsigned array_pitch,
      gfx_buffer_usage_t blob_usage, bool secure)
{
   assert(num_mip_levels > 0 && num_array_elems > 0 && array_pitch > 0);
   assert(res_i->handle == GMEM_HANDLE_INVALID ||
          ((gmem_get_usage(res_i->handle) & GMEM_USAGE_SECURE) != 0) == secure);

   /* if we can get the size of the handle, we should be able to assert that
    * num_array_elems * array_pitch <= size_handle */

   /* array elements cannot be 3D, so depth must be 1 */
   if (num_array_elems > 1)
      assert(descs[0].depth == 1);

   blob->usage = blob_usage;
   blob->secure = secure;

   blob->num_mip_levels = num_mip_levels;
   blob->num_array_elems = num_array_elems;

   blob->array_pitch = array_pitch;
   memcpy(blob->desc, descs, blob->num_mip_levels * sizeof(GFX_BUFFER_DESC_T));

   khrn_res_interlock_refinc(res_i);
   blob->res_i = res_i;

   blob->foreign_interlock = true;
}

static void blob_term(void *v, size_t size)
{
   KHRN_BLOB_T *blob = v;
   khrn_res_interlock_refdec(blob->res_i);
}

static KHRN_BLOB_T* blob_create(unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems, unsigned num_mip_levels,
      const GFX_LFMT_T *lfmts, unsigned num_planes,
      gfx_buffer_usage_t blob_usage, bool alloc_storage,
      bool secure)
{
    KHRN_BLOB_T *blob;

    blob = blob_alloc(num_mip_levels, num_array_elems);
    if (blob == NULL)
       return NULL;

    if (!blob_init(blob, width, height, depth,
             num_mip_levels, num_array_elems,
             lfmts, num_planes, blob_usage, alloc_storage,
             secure))
    {
       KHRN_MEM_ASSIGN(blob, NULL);
    }
    else
      khrn_mem_set_term(blob, blob_term);

    return blob;
}

KHRN_BLOB_T* khrn_blob_create(unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems, unsigned num_mip_levels,
      const GFX_LFMT_T *lfmts, unsigned num_planes,
      gfx_buffer_usage_t blob_usage, bool secure)
{
   return blob_create(width, height, depth, num_array_elems, num_mip_levels,
      lfmts, num_planes, blob_usage, true, secure);
}

extern KHRN_BLOB_T* khrn_blob_create_from_storage(gmem_handle_t handle,
      const GFX_BUFFER_DESC_T *descs, unsigned num_mip_levels,
      unsigned num_array_elems, unsigned array_pitch,
      gfx_buffer_usage_t blob_usage)
{
   KHRN_BLOB_T *blob;

   blob = blob_alloc(num_mip_levels, num_array_elems);
   if (blob == NULL)
      return NULL;

   if (!blob_init_from_storage(blob, handle, descs,
             num_mip_levels, num_array_elems, array_pitch, blob_usage))
   {
      KHRN_MEM_ASSIGN(blob, NULL);
   }
   else
      khrn_mem_set_term(blob, blob_term);

   return blob;
}

KHRN_BLOB_T* khrn_blob_create_from_res_interlock(KHRN_RES_INTERLOCK_T *res_i,
      const GFX_BUFFER_DESC_T *descs, unsigned num_mip_levels,
      unsigned num_array_elems, unsigned array_pitch,
      gfx_buffer_usage_t blob_usage, bool secure)
{
   KHRN_BLOB_T *blob;

   blob = blob_alloc(num_mip_levels, num_array_elems);
   if (blob == NULL)
      return NULL;

   blob_init_from_res_interlock(blob, res_i, descs,
             num_mip_levels, num_array_elems, array_pitch, blob_usage, secure);
   khrn_mem_set_term(blob, blob_term);

   return blob;
}


KHRN_BLOB_T* khrn_blob_create_no_storage(unsigned width, unsigned height,
      unsigned depth, unsigned num_array_elems, unsigned num_mip_levels,
      const GFX_LFMT_T *lfmts, unsigned num_planes,
      gfx_buffer_usage_t blob_usage, bool secure)
{
   return blob_create(width, height, depth, num_array_elems, num_mip_levels,
      lfmts, num_planes, blob_usage, false, secure);
}

bool khrn_blob_alloc_storage(KHRN_BLOB_T *blob)
{
   if(blob->res_i->handle != GMEM_HANDLE_INVALID)
      return true;

   return khrn_res_interlock_alloc(
      blob->res_i,
      blob->array_pitch * blob->num_array_elems,
      blob->descs_align,
      GMEM_USAGE_V3D_RW | (blob->secure ? GMEM_USAGE_SECURE : GMEM_USAGE_NONE),
      "khrn_blob.delayed_storage");
}

bool khrn_blob_contains_level(const KHRN_BLOB_T *blob,
      unsigned level)
{
   return (level < blob->num_mip_levels);
}

bool khrn_blob_has_level_with_spec(const KHRN_BLOB_T *blob,
      unsigned level, unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems, const GFX_LFMT_T *lfmts, unsigned num_planes)
{
   const GFX_BUFFER_DESC_T *desc;
   unsigned plane;

   if (!khrn_blob_contains_level(blob, level))
      return false;

   desc = &blob->desc[level];

   if (blob->num_array_elems != num_array_elems ||
       desc->width != width || desc->height != height || desc->depth != depth ||
       desc->num_planes != num_planes)
      return false;

   for (plane = 0; plane < num_planes; plane++)
   {
      if (gfx_lfmt_fmt(desc->planes[plane].lfmt) != gfx_lfmt_fmt(lfmts[plane]))
         return false;
   }
   return true;
}
