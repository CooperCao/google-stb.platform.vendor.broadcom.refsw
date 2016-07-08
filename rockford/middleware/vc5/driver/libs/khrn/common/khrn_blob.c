/*=============================================================================
  Broadcom Proprietary and Confidential. (c)20013 Broadcom.
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
         secure ? (GMEM_USAGE_V3D | GMEM_USAGE_SECURE) : GMEM_USAGE_ALL, "khrn_blob");
   }
   else
      blob->res_i = khrn_res_interlock_create_no_handle();

   if (blob->res_i == NULL)
      return false;

   // Contents of blob are initially undefined
   khrn_interlock_invalidate(&blob->res_i->interlock);

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
   gmem_handle_t handle;
   size_t total_size;

   if(blob->res_i->handle != GMEM_HANDLE_INVALID)
      return true;

   total_size = blob->array_pitch * blob->num_array_elems;
   bool secure = blob->secure;
   handle = gmem_alloc(total_size, blob->descs_align, secure ? (GMEM_USAGE_V3D | GMEM_USAGE_SECURE) : GMEM_USAGE_ALL,
         "khrn_blob.delayed_storage");

   if (handle == GMEM_HANDLE_INVALID)
      return false;

   khrn_res_interlock_set_handle(blob->res_i, handle);
   return true;
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
