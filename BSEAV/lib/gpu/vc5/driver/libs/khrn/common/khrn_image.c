/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_image.h"
#include "khrn_mem.h"
#include "khrn_process.h"
#include "khrn_options.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "libs/core/gfx_buffer/gfx_buffer_slow_conv.h"
#include "libs/core/v3d/v3d_tfu.h"
#include "libs/platform/gmem.h"
#include "khrn_fence.h"
#include "libs/platform/v3d_scheduler.h"
#include "../glxx/glxx_server.h"
#include "libs/platform/v3d_driver_api.h"

static void *image_map(const khrn_image *img,
      khrn_fence *fence_to_depend_on, bool write);
static void image_unmap(const khrn_image *img, void *ptr, bool write);

static void image_init(khrn_image *img,
      khrn_blob *blob, unsigned level,
      unsigned start_elem, unsigned num_array_elems,
      unsigned start_slice, unsigned num_slices,
      GFX_LFMT_T api_fmt)
{
   KHRN_MEM_ASSIGN(img->blob, blob);
   img->level = level;
   img->start_elem = start_elem;
   img->num_array_elems = num_array_elems;
   img->start_slice = start_slice;
   img->num_slices = num_slices;
   img->api_fmt = api_fmt;
}

static void image_term(void *v)
{
   khrn_image *img = v;
   KHRN_MEM_ASSIGN(img->blob, NULL);
}

static khrn_image* image_create(khrn_blob *blob, unsigned level,
      unsigned start_elem, unsigned num_array_elems,
      unsigned start_slice, unsigned num_slices,
      GFX_LFMT_T api_fmt)
{
   khrn_image *img;
   assert(khrn_blob_contains_level(blob, level) &&
               (start_elem + num_array_elems <= blob->num_array_elems) &&
               (start_slice + num_slices <= blob->desc[0].depth));

   img = KHRN_MEM_ALLOC_STRUCT(khrn_image);
   if (img == NULL)
      return NULL;

   image_init(img, blob, level, start_elem, num_array_elems,
      start_slice, num_slices, api_fmt);
   khrn_mem_set_term(img, image_term);
   return img;
}

khrn_image* khrn_image_create(khrn_blob *blob,
   unsigned start_elem, unsigned num_array_elems, unsigned level, GFX_LFMT_T api_fmt)
{
    return image_create(blob, level, start_elem, num_array_elems, 0,
       blob->desc[level].depth, api_fmt);
}

khrn_image* khrn_image_create_one_elem_slice(khrn_blob *blob,
   unsigned elem, unsigned slice, unsigned level, GFX_LFMT_T api_fmt)
{
   return image_create(blob, level, elem, 1, slice, 1, api_fmt);
}

khrn_image* khrn_image_shallow_blob_copy(const khrn_image* other)
{
   khrn_image *img = KHRN_MEM_ALLOC_STRUCT(khrn_image);
   if (img)
   {
      *img = *other;
      img->blob = khrn_blob_shallow_copy(img->blob);
      if (img->blob)
         khrn_mem_set_term(img, image_term);
      else
         KHRN_MEM_ASSIGN(img, NULL);
   }
   return img;
}

GFX_LFMT_T khrn_image_get_lfmt(const khrn_image *img,
      unsigned plane)
{
   const GFX_BUFFER_DESC_T *desc;
   desc = &img->blob->desc[img->level];

   if (plane >= desc->num_planes)
      return GFX_LFMT_NONE;
   return desc->planes[plane].lfmt;
}

unsigned khrn_image_get_max_levels(const khrn_image *img)
{
   const GFX_BUFFER_DESC_T *desc;
   unsigned max_size, max_levels;

   desc = &img->blob->desc[img->level];

   max_size = gfx_umax(gfx_umax(desc->width, desc->height),
         khrn_image_get_depth(img));
   /* mip_count = log2(maxsize) + 1 */
   max_levels = gfx_msb(max_size) + 1;
   return max_levels;
}

bool khrn_image_match_fmt(const khrn_image *img1,
      const khrn_image *img2)
{
   const GFX_BUFFER_DESC_T *desc1, *desc2;
   unsigned i;

   desc1 = &img1->blob->desc[img1->level];
   desc2 = &img2->blob->desc[img2->level];

   if (desc1->num_planes != desc2->num_planes)
      return false;

   for (i = 0; i < desc1->num_planes; i++)
   {
      if ( gfx_lfmt_fmt(desc1->planes[i].lfmt) !=
           gfx_lfmt_fmt(desc2->planes[i].lfmt))
         return false;
   }
   return true;
}

bool khrn_image_match_fmt_and_dim(const khrn_image *img1,
      const khrn_image *img2)
{
   const GFX_BUFFER_DESC_T *desc1, *desc2;

   if (!khrn_image_match_fmt(img1, img2))
      return false;

   desc1 = &img1->blob->desc[img1->level];
   desc2 = &img2->blob->desc[img2->level];

   if (desc1->width != desc2->width ||
       desc1->height != desc2->height ||
       khrn_image_get_depth(img1) != khrn_image_get_depth(img2) ||
       khrn_image_get_num_elems(img1) != khrn_image_get_num_elems(img2))
      return false;

   return true;
}

bool khrn_image_is_miplevel(const khrn_image *img1,
      unsigned mip_level, const khrn_image *img2)
{
   const GFX_BUFFER_DESC_T *desc1, *desc2;

   assert(mip_level > 0);

   if (!khrn_image_match_fmt(img1, img2))
      return false;

   desc1 = &img1->blob->desc[img1->level];
   desc2 = &img2->blob->desc[img2->level];

   if( desc1->width != gfx_umax(desc2->width >> mip_level, 1) ||
       desc1->height != gfx_umax(desc2->height >> mip_level, 1) ||
       khrn_image_get_depth(img1) != gfx_umax(khrn_image_get_depth(img2) >> mip_level, 1) ||
       khrn_image_get_num_elems(img1) != khrn_image_get_num_elems(img2))
      return false;

   return true;
}

bool khrn_image_match_dim_and_fmt(const khrn_image *img,
      unsigned width, unsigned height, unsigned depth, unsigned num_array_elems,
      const GFX_LFMT_T *lfmts, unsigned num_planes)
{
   const GFX_BUFFER_DESC_T *desc;
   unsigned i;

   desc = &img->blob->desc[img->level];

   if (img->num_array_elems != num_array_elems ||
       desc->width != width || desc->height != height ||
       khrn_image_get_depth(img) != depth ||
       desc->num_planes != num_planes)
      return false;

   for (i = 0; i < num_planes; i++)
   {
      if (gfx_lfmt_fmt(desc->planes[i].lfmt) !=
            gfx_lfmt_fmt(lfmts[i]))
         return false;
   }
   return true;
}

static void begin_imgconv(struct v3d_imgconv_gmem_tgt *tgt,
      const khrn_image *img,
      khrn_fence *fence_to_depend_on,
      unsigned int x, unsigned int y, unsigned int z,
      unsigned int start_elem, bool write)
{
   khrn_resource *res = khrn_image_get_resource(img);

   v3d_scheduler_deps deps;
   if (write)
      deps = *khrn_resource_begin_submit_writer_jobs(res);
   else
      deps = *khrn_resource_begin_submit_reader_jobs(res);

   if (fence_to_depend_on)
   {
      const v3d_scheduler_deps *depend_deps = khrn_fence_get_deps(fence_to_depend_on);
      v3d_scheduler_merge_deps(&deps, depend_deps);
   }

   v3d_imgconv_init_gmem_tgt_multi_region(tgt,
      res->num_handles, res->handles, 0, &deps,
      &img->blob->desc[img->level], x, y,
      img->start_slice + z, img->start_elem + start_elem,
      img->blob->array_pitch);

   /* GL texture uploads require floating point depth values to be clamped
    * to a valid range as part of the upload process
    */
   if (write)
      tgt->base.conversion = V3D_IMGCONV_CONVERSION_CLAMP_DEPTH;
}

static void end_imgconv(const khrn_image *img, bool success,
      khrn_fence *fence, bool write, uint64_t job_id)
{
   v3d_scheduler_deps deps;
   deps.n = 0;

   if (job_id != 0)
   {
      deps.n = 1;
      deps.dependency[0] = job_id;
      if (fence)
         khrn_fence_job_add(fence, job_id);
   }

   khrn_resource* res = khrn_image_get_resource(img);
   if (write)
   {
      khrn_resource_parts_t parts = khrn_image_resource_parts(img, /*subset=*/false);
      khrn_resource_end_submit_writer_jobs(res, success, &deps, parts);
   }
   else
      khrn_resource_end_submit_reader_jobs(res, success, &deps);
}

bool khrn_image_convert_from_ptr_tgt(khrn_image *dst,
      unsigned dst_x, unsigned dst_y, unsigned dst_z,
      unsigned dst_start_elem, const struct v3d_imgconv_ptr_tgt *src,
      unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems, glxx_context_fences *fences,
      bool secure_context)
{
   struct v3d_imgconv_gmem_tgt dst_tgt;
   uint64_t job_id = 0;
   bool ok;

   assert(dst_start_elem + num_array_elems <= dst->num_array_elems);
   assert(depth <= dst->num_slices);
   assert(dst_z < dst->num_slices);

   begin_imgconv(&dst_tgt, dst, fences->fence_to_depend_on,
         dst_x, dst_y, dst_z, dst_start_elem, true);

   ok = v3d_imgconv_convert_from_ptr(&dst_tgt, src, &job_id, width, height, depth,
         num_array_elems, secure_context);

   end_imgconv(dst, ok, fences->fence, true, job_id);

   return ok;
}

bool khrn_image_convert_to_ptr_tgt(khrn_image *src,
      unsigned src_x, unsigned src_y, unsigned src_z,
      unsigned src_start_elem, struct v3d_imgconv_ptr_tgt *dst,
      unsigned width, unsigned height, unsigned depth,
      unsigned num_array_elems,
      glxx_context_fences *fences,
      bool secure_context)
{
   struct v3d_imgconv_gmem_tgt src_tgt;
   bool ok;

   assert(src_start_elem + num_array_elems <= src->num_array_elems);
   assert(src->num_slices <= depth);
   assert(src_z < src->num_slices);

   begin_imgconv(&src_tgt, src, fences->fence_to_depend_on,
         src_x, src_y, src_z, src_start_elem, false);

   ok = v3d_imgconv_convert_to_ptr(dst, &src_tgt, width, height, depth,
         num_array_elems, secure_context);

   end_imgconv(src, ok, NULL,  false, 0);

   return ok;
}

static void* image_map(const khrn_image *img,
      khrn_fence *fence_to_depend_on,
      bool write)
{
   if (fence_to_depend_on)
   {
      khrn_fence_flush(fence_to_depend_on);
      khrn_fence_wait(fence_to_depend_on, V3D_SCHED_DEPS_COMPLETED);
   }

   khrn_access_flags_t access = KHRN_ACCESS_READ;
   khrn_resource_parts_t parts = KHRN_RESOURCE_PARTS_ALL;
   if (write)
   {
      access = KHRN_ACCESS_WRITE;
      parts = khrn_image_resource_parts(img, /*subset=*/false);
   }

   return khrn_resource_begin_access(
      &img->blob->res,
      img->start_elem * img->blob->array_pitch,
      img->num_array_elems * img->blob->array_pitch,
      access,
      parts);
}

static void image_unmap(const khrn_image *img, void* ptr, bool write)
{
   if (!ptr)
      return;

   khrn_resource_end_access(
      img->blob->res,
      img->start_elem * img->blob->array_pitch,
      img->num_array_elems * img->blob->array_pitch,
      write ? KHRN_ACCESS_WRITE : KHRN_ACCESS_READ);
}

void khrn_image_calc_dst_usage(
   const khrn_image *src, unsigned width, unsigned height, unsigned depth,
   unsigned num_array_elems, unsigned num_mip_levels, const GFX_LFMT_T *lfmts,
   unsigned num_planes, bool secure_context,
   gfx_buffer_usage_t *dst_buf_usage, gmem_usage_flags_t *gmem_usage)
{
   v3d_imgconv_gmem_tgt src_tgt;

   v3d_scheduler_deps deps;
   v3d_scheduler_deps_init(&deps);

   khrn_resource *res = khrn_image_get_resource(src);
   v3d_imgconv_init_gmem_tgt_multi_region(&src_tgt, res->num_handles, res->handles, 0, &deps,
                                          &src->blob->desc[src->level], 0, 0,
                                          src->start_slice, src->start_elem,
                                          src->blob->array_pitch);

   v3d_imgconv_base_tgt dst_base, dst_base_contiguous;
   memset(&dst_base, 0, sizeof(dst_base));
   memset(&dst_base_contiguous, 0, sizeof(dst_base_contiguous));

   size_t size, align;
   gfx_buffer_desc_gen(&dst_base.desc, &size, &align, *dst_buf_usage, width, height, depth,
                       num_mip_levels, num_planes, lfmts);

   /* The GFX_BUFFER_USAGE_M2MC or GFX_BUFFER_USAGE_M2MC_EVEN_UB_HEIGHT flags
    * can only be used with UIF or RSO destination buffers. For example
    * small buffers in UBLINEAR order are incompatible with M2MC usage flags.
    *
    * The gfx_buffer_desc_gen() has an assertion that verifies if this
    * condition is met.
    */
   if (dst_base.desc.num_planes == 1 &&
         (gfx_lfmt_is_uif(dst_base.desc.planes[0].lfmt) ||
          gfx_lfmt_is_rso(dst_base.desc.planes[0].lfmt)))
   {
      gfx_buffer_usage_t contiguous_buf_usage = *dst_buf_usage | GFX_BUFFER_USAGE_M2MC;
      if (v3d_scheduler_get_soc_quirks() & V3D_SOC_QUIRK_HWBCM7260_81)
         contiguous_buf_usage |= GFX_BUFFER_USAGE_M2MC_EVEN_UB_HEIGHT;

      gfx_buffer_desc_gen(&dst_base_contiguous.desc, &size, &align,
                        contiguous_buf_usage, width, height, depth,
                        num_mip_levels, num_planes, lfmts);

      *gmem_usage = v3d_imgconv_calc_dst_gmem_usage(&dst_base, &dst_base_contiguous, &src_tgt, secure_context);
      if (*gmem_usage & GMEM_USAGE_CONTIGUOUS)
         *dst_buf_usage = contiguous_buf_usage;
   }
   else
   {
      *gmem_usage = v3d_imgconv_calc_dst_gmem_usage(&dst_base, NULL, &src_tgt, secure_context);
   }
}

bool khrn_image_convert(khrn_image *dst, const khrn_image *src,
      glxx_context_fences *fences, bool secure_context)
{
   struct v3d_imgconv_gmem_tgt src_tgt, dst_tgt;
   uint64_t job_id = 0;
   bool ok;

   assert(src->num_array_elems == dst->num_array_elems);
   assert(src->num_slices == dst->num_slices);

   assert(khrn_image_get_width(src) <= khrn_image_get_width(dst));
   assert(khrn_image_get_height(src) <= khrn_image_get_height(dst));

   begin_imgconv(&dst_tgt, dst, NULL, 0, 0, 0, 0, true);
   begin_imgconv(&src_tgt, src, fences->fence_to_depend_on,
         0, 0, 0, 0, false);
   ok = v3d_imgconv_convert(&dst_tgt, &src_tgt, &job_id, src_tgt.base.desc.width,
         src_tgt.base.desc.height, src->num_slices, src->num_array_elems, secure_context);

   end_imgconv(dst, ok, fences->fence, true, job_id);
   end_imgconv(src, ok, NULL, false, job_id);

   return ok;
}

bool khrn_image_convert_one_elem_slice(khrn_image *dst, unsigned dst_x, unsigned
      dst_y, unsigned dst_z, unsigned dst_start_elem,
      khrn_image *src, unsigned src_x, unsigned src_y, unsigned src_z,
      unsigned src_start_elem,
      unsigned width, unsigned height,
      glxx_context_fences *fences,
      bool secure_context)
{
   struct v3d_imgconv_gmem_tgt src_tgt, dst_tgt;
   uint64_t job_id = 0;
   bool ok;

   assert(dst_start_elem < dst->num_array_elems);
   assert(src_start_elem < src->num_array_elems);
   assert(src_z < src->num_slices);
   assert(dst_z < dst->num_slices);
   assert(width <= khrn_image_get_width(src) && width <= khrn_image_get_width(dst));
   assert(height <= khrn_image_get_height(src) && height <= khrn_image_get_height(dst));

   begin_imgconv(&dst_tgt, dst, NULL, dst_x, dst_y, dst_z, dst_start_elem, true);
   begin_imgconv(&src_tgt, src, fences->fence_to_depend_on,
         src_x, src_y, src_z, src_start_elem, false);

   ok = v3d_imgconv_convert(&dst_tgt, &src_tgt, &job_id, width, height, 1, 1, secure_context);

   end_imgconv(dst, ok, fences->fence, true, job_id);
   end_imgconv(src, ok, NULL, false, job_id);

   return ok;
}

extern bool khrn_image_memcpy_one_elem_slice(khrn_image *dst, unsigned dst_x,
      unsigned dst_y, unsigned dst_z, unsigned dst_start_elem, khrn_image
      *src, unsigned src_x, unsigned src_y, unsigned src_z, unsigned
      src_start_elem, unsigned src_width, unsigned src_height,
      glxx_context_fences *fences,
      bool secure_context)
{
   struct v3d_imgconv_gmem_tgt src_tgt, dst_tgt;
   uint64_t job_id = 0;
   bool ok;

   assert(dst_start_elem < dst->num_array_elems);
   assert(src_start_elem < src->num_array_elems);
   assert(src_z < src->num_slices);
   assert(dst_z < dst->num_slices);
   assert(src_width <= khrn_image_get_width(src));
   assert(src_height <= khrn_image_get_height(src));

   begin_imgconv(&dst_tgt, dst, NULL, dst_x, dst_y, dst_z, dst_start_elem, true);
   begin_imgconv(&src_tgt, src, fences->fence_to_depend_on,
         src_x, src_y, src_z, src_start_elem, false);

   ok = v3d_imgconv_memcpy(&dst_tgt, &src_tgt, &job_id, src_width, src_height, 1, 1, secure_context);

   end_imgconv(dst, ok, fences->fence, true, job_id);
   end_imgconv(src, ok, NULL, false, job_id);

   return ok;
}

/* Format of dst & src must match */
bool khrn_image_subsample(khrn_image *dst, const khrn_image *src,
      bool force_no_srgb, glxx_context_fences *fences)
{
   GFX_BUFFER_BLIT_TGT_T dst_b, src_b;

   assert(src->num_array_elems == dst->num_array_elems);

   bool ok = false;

   void *src_ptr = image_map(src, fences->fence_to_depend_on, false);
   void *dst_ptr = image_map(dst, NULL, true);
   if (src_ptr == NULL || dst_ptr == NULL)
      goto end;

   src_b.desc = src->blob->desc[src->level];
   gfx_buffer_blit_tgt_set_pos(&src_b, 0, 0, 0);
   src_b.p = src_ptr;

   dst_b.desc = dst->blob->desc[dst->level];
   gfx_buffer_blit_tgt_set_pos(&dst_b, 0, 0, 0);
   dst_b.p = dst_ptr;

   if (force_no_srgb)
   {
      assert(src_b.desc.num_planes == dst_b.desc.num_planes);
      for (unsigned i=0; i<src_b.desc.num_planes; i++)
      {
         src_b.desc.planes[i].lfmt = gfx_lfmt_srgb_to_unorm(src_b.desc.planes[i].lfmt);
         dst_b.desc.planes[i].lfmt = gfx_lfmt_srgb_to_unorm(dst_b.desc.planes[i].lfmt);
      }
   }

   for (unsigned i = 0; i < src->num_array_elems; i++)
   {
      // Match TFU...
#if V3D_VER_AT_LEAST(4,1,34,0)
      gfx_buffer_subsample(&dst_b, &src_b, NULL);
#else
      GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T transmute_options;
      gfx_buffer_default_transmute_options(&transmute_options);
      transmute_options.f16_to_uf_rtz = true;
      gfx_buffer_subsample(&dst_b, &src_b, &transmute_options);
#endif
      dst_b.p = (uint8_t*)dst_b.p + dst->blob->array_pitch;
      src_b.p = (uint8_t*)src_b.p + src->blob->array_pitch;
   }
   ok = true;

end:
   image_unmap(src, src_ptr, false);
   image_unmap(dst, dst_ptr, true);
   return ok;
}


bool khrn_image_generate_mipmaps_tfu(khrn_image* src_image,
      khrn_image* const* dst_images,
      unsigned num_dst_levels, bool skip_dst_level_0, bool force_no_srgb,
      glxx_context_fences *fences, bool secure_context)
{
   khrn_blob* src_blob = src_image->blob;
   khrn_blob* dst_blob = dst_images[0]->blob;
   GFX_BUFFER_DESC_T const* src_desc = &src_blob->desc[src_image->level];
   GFX_BUFFER_DESC_T const* dst_desc = &dst_blob->desc[dst_images[0]->level];
   GFX_LFMT_T src_lfmt = src_desc->planes[0].lfmt;
   GFX_LFMT_T dst_lfmt = dst_desc->planes[0].lfmt;

   assert(src_blob->res->num_handles == 1);
   assert(dst_blob->res->num_handles == 1);

   // Check dst is same size and format as src.
   // We can cope with different layouts, with work we could also cope
   // with channel reordering and some YUV format conversion.
   assert(khrn_get_has_tfu());
   assert(num_dst_levels > 0 && (num_dst_levels > 1 || !skip_dst_level_0));
   assert(src_image->num_array_elems == dst_images[0]->num_array_elems);
   assert(src_desc->width == dst_desc->width);
   assert(src_desc->height == dst_desc->height);
   assert(src_desc->num_planes == dst_desc->num_planes);
   assert(gfx_lfmt_fmt(src_lfmt) == gfx_lfmt_fmt(dst_lfmt));

   // Check dst levels are consistent and contiguous.
   assert(dst_images[0]->level == 0);
   for (unsigned i = 1; i != num_dst_levels; ++i)
   {
      GFX_BUFFER_DESC_T const* dst_desc_i = &dst_blob->desc[i];
      assert(dst_images[i]->blob == dst_blob);
      assert(dst_images[i]->level == i);
      assert(dst_images[i]->num_array_elems == dst_images[0]->num_array_elems);
      assert(dst_desc_i->width == gfx_umax(1, dst_desc->width >> i));
      assert(dst_desc_i->height == gfx_umax(1, dst_desc->height >> i));
      assert(dst_desc_i->num_planes == dst_desc->num_planes);
      assert(gfx_lfmt_fmt(dst_desc_i->planes[0].lfmt) == gfx_lfmt_fmt(dst_lfmt));
   }

   V3D_TFU_COMMAND_T tfu_cmd;
   if (!v3d_build_tfu_cmd(&tfu_cmd, src_desc, dst_desc, num_dst_levels, skip_dst_level_0, 0, 0))
      return false;

   if (force_no_srgb)
      tfu_cmd.srgb = false;

   // Lock v3d addresses for src/dst blobs.
   gmem_handle_t src_gmemh = src_blob->res->handles[0];
   gmem_handle_t dst_gmemh = dst_blob->res->handles[0];
   v3d_addr_t src_addr = gmem_get_addr(src_gmemh);
   v3d_addr_t dst_addr = gmem_get_addr(dst_gmemh);

   /* we do not support yet textures that are a slice from a 3D image */
   assert(src_image->start_slice == 0 && dst_images[0]->start_slice == 0);
   tfu_cmd.src_base_addrs[0] += src_addr + src_image->start_elem * src_blob->array_pitch;
   tfu_cmd.dst_base_addr += dst_addr + dst_images[0]->start_elem * dst_blob->array_pitch;

   // TFU job is runnable when all dependencies for read and write are met.
   v3d_scheduler_deps tfu_job_deps;
   khrn_resource* dst_res = dst_blob->res;
   khrn_resource* src_res = src_blob->res;
   v3d_scheduler_copy_deps(&tfu_job_deps, khrn_resource_begin_submit_writer_jobs(dst_res));
   if (src_res != dst_res)
      v3d_scheduler_merge_deps(&tfu_job_deps, khrn_resource_begin_submit_reader_jobs(src_res));

   if (fences->fence_to_depend_on)
   {
      const v3d_scheduler_deps *depend_deps = khrn_fence_get_deps(fences->fence_to_depend_on);
      v3d_scheduler_merge_deps(&tfu_job_deps, depend_deps);
   }

   uint32_t src_offset = khrn_image_get_offset(src_image, 0);
   uint32_t dst_offset = khrn_image_get_offset(dst_images[0], 0);

   const V3D_HUB_IDENT_T* hub_ident = v3d_scheduler_get_hub_identity();
   v3d_cache_ops cache_ops =
         v3d_barrier_cache_flushes(V3D_BARRIER_MEMORY_WRITE, V3D_BARRIER_TFU_READ | V3D_BARRIER_TFU_WRITE, hub_ident)
       | v3d_barrier_cache_cleans(V3D_BARRIER_TFU_WRITE, V3D_BARRIER_MEMORY_READ, hub_ident);

   // todo, ideally the kernel API would support batch submission of TFU jobs.

   v3d_scheduler_deps final_deps;
   v3d_scheduler_deps_init(&final_deps);

   for (unsigned num_submitted = 0; num_submitted != src_image->num_array_elems; ++num_submitted)
   {
      // Submit the TFU job to the scheduler;
      uint64_t tfu_job_id = v3d_scheduler_submit_tfu_job(
         &tfu_job_deps,
         cache_ops,
         &tfu_cmd,
         secure_context,
         NULL, NULL);
      v3d_scheduler_add_dep(&final_deps, tfu_job_id);

      // Increment offsets for next array element.
      tfu_cmd.src_base_addrs[0] += src_blob->array_pitch;
      tfu_cmd.dst_base_addr += dst_blob->array_pitch;
      src_offset += src_blob->array_pitch;
      dst_offset += dst_blob->array_pitch;
   }

   // Update resource with tfu_job deps.

   /* Could figure out tighter parts here but probably not worth it... */
   khrn_resource_end_submit_writer_jobs(dst_res, true, &final_deps, KHRN_RESOURCE_PARTS_ALL);

   if (src_res != dst_res)
      khrn_resource_end_submit_reader_jobs(src_res, true, &final_deps);

   if (fences->fence)
      khrn_fence_deps_add(fences->fence, &final_deps);

   // Succeeded if we issued jobs for all the array elements. In the case of an error
   // things are left in a consistent state for the the fallback CPU mip generation code
   // to attempt to run.
   return true;
}

unsigned khrn_image_get_depth(const khrn_image *img)
{
   assert(img->start_slice + img->num_slices <=
         img->blob->desc[img->level].depth);
   return img->num_slices;
}

unsigned khrn_image_get_width(const khrn_image *img)
{
   return img->blob->desc[img->level].width;
}

unsigned khrn_image_get_height(const khrn_image *img)
{
   return img->blob->desc[img->level].height;
}

unsigned khrn_image_get_num_elems(const khrn_image *img)
{
   return img->num_array_elems;
}

unsigned khrn_image_get_num_planes(const khrn_image *img)
{
   return img->blob->desc[img->level].num_planes;
}

void khrn_image_get_dimensions(const khrn_image *img, unsigned *width,
      unsigned *height, unsigned *depth, unsigned *num_elems)
{
   if (width)
      *width = khrn_image_get_width(img);

   if (height)
      *height = khrn_image_get_height(img);

   if (depth)
      *depth = khrn_image_get_depth(img);

   if (num_elems)
      *num_elems = khrn_image_get_num_elems(img);
}

void khrn_image_get_lfmts(const khrn_image *img,
      GFX_LFMT_T lfmts[GFX_BUFFER_MAX_PLANES], unsigned *num_planes)
{
   unsigned i;
   GFX_BUFFER_DESC_T *desc = &img->blob->desc[img->level];

   *num_planes = desc->num_planes;

   for (i = 0; i < desc->num_planes; i++)
         lfmts[i] = desc->planes[i].lfmt;
}

void khrn_image_get_fmts(const khrn_image *img,
      GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES], unsigned *num_planes)
{
   khrn_image_get_lfmts(img, fmts, num_planes);
   for (unsigned i = 0; i != *num_planes; ++i)
      fmts[i] = gfx_lfmt_fmt(fmts[i]);
}

unsigned khrn_image_get_offset(const khrn_image *img,
      unsigned plane)
{
   const GFX_BUFFER_DESC_T *desc;
   const GFX_BUFFER_DESC_PLANE_T *p;

   desc = &img->blob->desc[img->level];
   assert(plane < desc->num_planes);
   p = &desc->planes[plane];

   return p->offset + img->start_elem * img->blob->array_pitch +
      img->start_slice * p->slice_pitch;
}

v3d_addr_t khrn_image_get_addr(const khrn_image *img, unsigned plane)
{
   unsigned region = khrn_image_get_desc_plane(img, plane)->region;
   const khrn_resource *res = khrn_image_get_resource(img);
   assert(region < res->num_handles);
   return v3d_addr_offset(gmem_get_addr(res->handles[region]),
      khrn_image_get_offset(img, plane));
}

bool khrn_image_is_one_elem_slice(const khrn_image *img)
{
   if (img->num_array_elems == 1 && img->num_slices == 1)
      return true;
   return false;
}

bool khrn_image_equal(const khrn_image *img1, const khrn_image *img2)
{
   if (img1->blob != img2->blob ||
       img1->level != img2->level ||
       img1->start_elem != img2->start_elem ||
       img1->num_array_elems != img2->num_array_elems ||
       img1->start_slice != img2->start_slice ||
       img1->num_slices != img2->num_slices)
      return false;
   return true;
}
