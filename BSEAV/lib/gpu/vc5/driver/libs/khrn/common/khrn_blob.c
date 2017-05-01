/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_blob.h"
#include "libs/platform/gmem.h"
#include "khrn_mem.h"
#include "khrn_options.h"

khrn_resource_parts_t khrn_blob_resource_parts(const khrn_blob *blob,
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

   if (blob->foreign_resource)
      /* Don't know how resource parts are assigned! */
      return subset ? 0 : KHRN_RESOURCE_PARTS_ALL;

   /* There is a resource part for each channel group in each 2D image plane.
    * Part i corresponds to
    * channel group ((i / a) % b) in
    * slice (i % a) in
    * mip level ((i / (a * b)) % c) in
    * array element (i / (a * b * c))
    * where
    * a = blob->desc[0].depth
    * b = blob_changrps
    * c = blob->num_mip_levels */

   khrn_resource_parts_t parts = 0;
   for (unsigned elem = start_elem; elem != start_elem + num_elems; ++elem)
   {
      for (unsigned changrp = start_changrp; changrp != start_changrp + num_changrps; ++changrp)
      {
         unsigned p = elem;
         p = (p * blob->num_mip_levels) + mip_level;
         p = (p * blob_changrps) + changrp;
         p = (p * blob->desc[0].depth) + start_slice;
         parts |= khrn_resource_parts(p, num_slices);

         if (parts & KHRN_RESOURCE_PARTS_REST)
         {
            if (subset)
            {
               bool any_other_parts =
                  (p >= KHRN_RESOURCE_PARTS_BITS) ||
                  ((start_slice + num_slices) != blob->desc[mip_level].depth) ||
                  ((start_changrp + num_changrps) != blob_changrps) ||
                  ((changrp != (blob_changrps - 1)) && (start_slice != 0)) ||
                  (mip_level != (blob->num_mip_levels - 1)) ||
                  ((start_elem + num_elems) != blob->num_array_elems) ||
                  ((elem != (blob->num_array_elems - 1)) &&
                     ((start_slice != 0) || (start_changrp != 0) || (mip_level != 0)));
               if (any_other_parts)
                  /* There are some parts covered by KHRN_RESOURCE_PARTS_REST
                   * that are not included in the specified range. As we were
                   * asked for a subset, we cannot include
                   * KHRN_RESOURCE_PARTS_REST. */
                  parts &= ~KHRN_RESOURCE_PARTS_REST;
            }
            return parts;
         }
      }
   }
   return parts;
}

static khrn_blob* blob_alloc(unsigned num_mip_levels, unsigned num_array_elems)
{
   khrn_blob *blob;
   size_t struct_size;

   assert(num_mip_levels > 0 && num_array_elems > 0);

   /* allocate enough space for the struct and desc flexible array */
   struct_size = sizeof(khrn_blob) + (num_mip_levels - 1) *
      sizeof(GFX_BUFFER_DESC_T);

   blob = khrn_mem_alloc(struct_size, "khrn_blob");
   if (blob == NULL)
      return NULL;

   return blob;
}

static bool blob_init(khrn_blob *blob,
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
      blob->res = khrn_resource_create(total_size, blob->descs_align,
         GMEM_USAGE_V3D_RW | (secure ? GMEM_USAGE_SECURE : GMEM_USAGE_NONE), "khrn_blob");
   }
   else
      blob->res = khrn_resource_create_no_handle();

   if (blob->res == NULL)
      return false;

   blob->foreign_resource = false;

   // Contents of blob are initially undefined
   khrn_resource_mark_undefined(blob->res, KHRN_RESOURCE_PARTS_ALL);

   return true;
}

static bool blob_init_from_storage(khrn_blob *blob, gmem_handle_t handle,
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

   blob->res = khrn_resource_create_with_handle(handle);
   if (blob->res == NULL)
      return false;

   blob->foreign_resource = false;

   return true;
}

static void blob_init_from_resource(khrn_blob *blob,
      khrn_resource *res, const GFX_BUFFER_DESC_T *descs,
      unsigned num_mip_levels, unsigned num_array_elems,
      unsigned array_pitch,
      gfx_buffer_usage_t blob_usage, bool secure)
{
   assert(num_mip_levels > 0 && num_array_elems > 0 && array_pitch > 0);
   assert(res->handle == GMEM_HANDLE_INVALID ||
          ((gmem_get_usage(res->handle) & GMEM_USAGE_SECURE) != 0) == secure);

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

   khrn_resource_refinc(res);
   blob->res = res;

   blob->foreign_resource = true;
}

static void blob_term(void *v, size_t size)
{
   khrn_blob *blob = v;
   khrn_resource_refdec(blob->res);
}

static khrn_blob* blob_create(unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems, unsigned num_mip_levels,
      const GFX_LFMT_T *lfmts, unsigned num_planes,
      gfx_buffer_usage_t blob_usage, bool alloc_storage,
      bool secure)
{
    khrn_blob *blob;

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

khrn_blob* khrn_blob_create(unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems, unsigned num_mip_levels,
      const GFX_LFMT_T *lfmts, unsigned num_planes,
      gfx_buffer_usage_t blob_usage, bool secure)
{
   return blob_create(width, height, depth, num_array_elems, num_mip_levels,
      lfmts, num_planes, blob_usage, true, secure);
}

extern khrn_blob* khrn_blob_create_from_storage(gmem_handle_t handle,
      const GFX_BUFFER_DESC_T *descs, unsigned num_mip_levels,
      unsigned num_array_elems, unsigned array_pitch,
      gfx_buffer_usage_t blob_usage)
{
   khrn_blob *blob;

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

khrn_blob* khrn_blob_create_from_resource(khrn_resource *res,
      const GFX_BUFFER_DESC_T *descs, unsigned num_mip_levels,
      unsigned num_array_elems, unsigned array_pitch,
      gfx_buffer_usage_t blob_usage, bool secure)
{
   khrn_blob *blob;

   blob = blob_alloc(num_mip_levels, num_array_elems);
   if (blob == NULL)
      return NULL;

   blob_init_from_resource(blob, res, descs,
             num_mip_levels, num_array_elems, array_pitch, blob_usage, secure);
   khrn_mem_set_term(blob, blob_term);

   return blob;
}


khrn_blob* khrn_blob_create_no_storage(unsigned width, unsigned height,
      unsigned depth, unsigned num_array_elems, unsigned num_mip_levels,
      const GFX_LFMT_T *lfmts, unsigned num_planes,
      gfx_buffer_usage_t blob_usage, bool secure)
{
   return blob_create(width, height, depth, num_array_elems, num_mip_levels,
      lfmts, num_planes, blob_usage, false, secure);
}

bool khrn_blob_alloc_storage(khrn_blob *blob)
{
   if(blob->res->handle != GMEM_HANDLE_INVALID)
      return true;

   return khrn_resource_alloc(
      blob->res,
      blob->array_pitch * blob->num_array_elems,
      blob->descs_align,
      GMEM_USAGE_V3D_RW | (blob->secure ? GMEM_USAGE_SECURE : GMEM_USAGE_NONE),
      "khrn_blob.delayed_storage");
}

bool khrn_blob_contains_level(const khrn_blob *blob,
      unsigned level)
{
   return (level < blob->num_mip_levels);
}

bool khrn_blob_has_level_with_spec(const khrn_blob *blob,
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