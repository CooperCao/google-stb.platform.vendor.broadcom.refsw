/*=============================================================================
  Broadcom Proprietary and Confidential. (c)2013 Broadcom.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
   Implementation of khrn_image
=============================================================================*/
#include "khrn_image.h"
#include "khrn_mem.h"
#include "khrn_process.h"
#include "khrn_options.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "libs/core/gfx_buffer/gfx_buffer_slow_conv.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/v3d/v3d_tfu.h"
#include "libs/platform/gmem.h"
#include "khrn_fence.h"
#include "libs/platform/v3d_scheduler.h"
#include "../glxx/glxx_server.h"

static void *image_map(const KHRN_IMAGE_T *img,
      KHRN_FENCE_T *fence_to_depend_on, bool write);
static void image_unmap(const KHRN_IMAGE_T *img, void *ptr, bool write);

static void image_init(KHRN_IMAGE_T *img,
      KHRN_BLOB_T *blob, unsigned level,
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

static void image_term(void *v, size_t size)
{
   KHRN_IMAGE_T *img = v;
   KHRN_MEM_ASSIGN(img->blob, NULL);
}

static KHRN_IMAGE_T* image_create(KHRN_BLOB_T *blob, unsigned level,
      unsigned start_elem, unsigned num_array_elems,
      unsigned start_slice, unsigned num_slices,
      GFX_LFMT_T api_fmt)
{
    KHRN_IMAGE_T *img;
    assert(khrn_blob_contains_level(blob, level) &&
                (start_elem + num_array_elems <= blob->num_array_elems) &&
                (start_slice + num_slices <= blob->desc[0].depth));

    img = KHRN_MEM_ALLOC_STRUCT(KHRN_IMAGE_T);
    if (img == NULL)
       return NULL;

    image_init(img, blob, level, start_elem, num_array_elems,
       start_slice, num_slices, api_fmt);
    khrn_mem_set_term(img, image_term);
    return img;
}

KHRN_IMAGE_T* khrn_image_create(KHRN_BLOB_T *blob,
   unsigned start_elem, unsigned num_array_elems, unsigned level, GFX_LFMT_T api_fmt)
{
    return image_create(blob, level, start_elem, num_array_elems, 0,
       blob->desc[level].depth, api_fmt);
}

KHRN_IMAGE_T* khrn_image_create_one_elem_slice(KHRN_BLOB_T *blob,
   unsigned elem, unsigned slice, unsigned level, GFX_LFMT_T api_fmt)
{
   return image_create(blob, level, elem, 1, slice, 1, api_fmt);
}

GFX_LFMT_T khrn_image_get_lfmt(const KHRN_IMAGE_T *img,
      unsigned plane)
{
   const GFX_BUFFER_DESC_T *desc;
   desc = &img->blob->desc[img->level];

   if (plane >= desc->num_planes)
      return GFX_LFMT_NONE;
   return desc->planes[plane].lfmt;
}

unsigned khrn_image_get_max_levels(const KHRN_IMAGE_T *img)
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

bool khrn_image_match_fmt(const KHRN_IMAGE_T *img1,
      const KHRN_IMAGE_T *img2)
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

bool khrn_image_match_fmt_and_dim(const KHRN_IMAGE_T *img1,
      const KHRN_IMAGE_T *img2)
{
   const GFX_BUFFER_DESC_T *desc1, *desc2;

   if (!khrn_image_match_fmt(img1, img2))
      return false;

   desc1 = &img1->blob->desc[img1->level];
   desc2 = &img2->blob->desc[img2->level];

   if (desc1->width != desc2->width ||
       desc1->height != desc2->height ||
       khrn_image_get_depth(img1) != khrn_image_get_depth(img2))
      return false;

   return true;
}

bool khrn_image_is_miplevel(const KHRN_IMAGE_T *img1,
      unsigned mip_level, const KHRN_IMAGE_T *img2)
{
   const GFX_BUFFER_DESC_T *desc1, *desc2;

   assert(mip_level > 0);

   if (!khrn_image_match_fmt(img1, img2))
      return false;

   desc1 = &img1->blob->desc[img1->level];
   desc2 = &img2->blob->desc[img2->level];

   if( desc1->width != gfx_umax(desc2->width >> mip_level, 1) ||
       desc1->height != gfx_umax(desc2->height >> mip_level, 1) ||
       khrn_image_get_depth(img1) !=
       gfx_umax(khrn_image_get_depth(img2) >> mip_level, 1))
      return false;

   return true;
}

bool khrn_image_match_dim_and_fmt(const KHRN_IMAGE_T *img,
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
      const KHRN_IMAGE_T *img,
      KHRN_FENCE_T *fence_to_depend_on,
      unsigned int x, unsigned int y, unsigned int z,
      unsigned int start_elem, bool write)
{
   v3d_scheduler_deps deps;
   const GFX_BUFFER_DESC_T *desc;
   KHRN_RES_INTERLOCK_T *res_i;

   res_i = khrn_image_get_res_interlock(img);
   if (write)
      deps = *khrn_interlock_begin_submit_writer_jobs(&res_i->interlock);
   else
      deps = *khrn_interlock_begin_submit_reader_jobs(&res_i->interlock);

   if (fence_to_depend_on)
   {
      khrn_fence_flush(fence_to_depend_on);
      v3d_scheduler_merge_deps(&deps, &fence_to_depend_on->deps);
   }

   desc = &img->blob->desc[img->level];
   v3d_imgconv_init_gmem_tgt(tgt, img->blob->res_i->handle, &deps, desc, x, y,
      img->start_slice + z, img->start_elem + start_elem,
      img->blob->array_pitch);
}

static void end_imgconv(const KHRN_IMAGE_T *img, bool success,
      KHRN_FENCE_T *fence, bool write, uint64_t job_id)
{
   KHRN_RES_INTERLOCK_T *res_i;
   res_i = khrn_image_get_res_interlock(img);

   v3d_scheduler_deps deps;
   deps.n = 0;

   if (job_id != 0)
   {
      deps.n = 1;
      deps.dependency[0] = job_id;
      if (fence)
         khrn_fence_job_add(fence, job_id);
   }

   if (write)
      khrn_interlock_end_submit_writer_jobs(&res_i->interlock, success, &deps);
   else
      khrn_interlock_end_submit_reader_jobs(&res_i->interlock, success, &deps);
}

bool khrn_image_copy_from_ptr_tgt(KHRN_IMAGE_T *dst,
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

   ok = v3d_imgconv_copy_from_ptr(&dst_tgt, src, &job_id, width, height, depth,
         num_array_elems, secure_context);

   end_imgconv(dst, ok, fences->fence, true, job_id);

   return ok;
}

bool khrn_image_copy_to_ptr_tgt(KHRN_IMAGE_T *src,
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

   ok = v3d_imgconv_copy_to_ptr(dst, &src_tgt, width, height, depth,
         num_array_elems, secure_context);

   end_imgconv(src, ok, NULL,  false, 0);

   return ok;
}

static void* image_map(const KHRN_IMAGE_T *img,
      KHRN_FENCE_T *fence_to_depend_on,
      bool write)
{
   if (fence_to_depend_on)
   {
      khrn_fence_flush(fence_to_depend_on);
      v3d_scheduler_wait_jobs(&fence_to_depend_on->deps, V3D_SCHED_DEPS_COMPLETED);
   }

   if (write)
   {
      khrn_interlock_write_now(&img->blob->res_i->interlock);
   }
   else
   {
      khrn_interlock_read_now(&img->blob->res_i->interlock);
   }

   return gmem_map_and_begin_cpu_access_range(
      img->blob->res_i->handle,
      img->start_elem * img->blob->array_pitch,
      img->num_array_elems * img->blob->array_pitch,
      (write ? GMEM_SYNC_CPU_WRITE : 0) | GMEM_SYNC_CPU_READ | GMEM_SYNC_RELAXED);
}

static void image_unmap(const KHRN_IMAGE_T *img, void* ptr, bool write)
{
   if (!ptr)
      return;

   gmem_end_cpu_access_range_and_unmap(
      img->blob->res_i->handle,
      img->start_elem * img->blob->array_pitch,
      img->num_array_elems * img->blob->array_pitch,
      (write ? GMEM_SYNC_CPU_WRITE : 0) | GMEM_SYNC_CPU_READ | GMEM_SYNC_RELAXED);
}

bool khrn_image_copy(KHRN_IMAGE_T *dst, const KHRN_IMAGE_T *src,
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
   ok = v3d_imgconv_copy(&dst_tgt, &src_tgt, &job_id, src_tgt.base.desc.width,
         src_tgt.base.desc.height, src->num_slices, src->num_array_elems, secure_context);

   end_imgconv(dst, ok, fences->fence, true, job_id);
   end_imgconv(src, ok, NULL, false, job_id);

   return ok;
}

bool khrn_image_copy_one_elem_slice(KHRN_IMAGE_T *dst, unsigned dst_x, unsigned
      dst_y, unsigned dst_z, unsigned dst_start_elem,
      KHRN_IMAGE_T *src, unsigned src_x, unsigned src_y, unsigned src_z,
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

   begin_imgconv(&dst_tgt, dst, NULL, dst_x, dst_y, dst_z, 0, true);
   begin_imgconv(&src_tgt, src, fences->fence_to_depend_on,
         src_x, src_y, src_z, 0, false);

   ok = v3d_imgconv_copy(&dst_tgt, &src_tgt, &job_id, width, height, 1, 1, secure_context);

   end_imgconv(dst, ok, fences->fence, true, job_id);
   end_imgconv(src, ok, NULL, false, job_id);

   return ok;
}

/* Format of dst & src must match */
bool khrn_image_subsample(KHRN_IMAGE_T *dst, const KHRN_IMAGE_T *src,
      glxx_context_fences *fences)
{
   GFX_BUFFER_BLIT_TGT_T dst_b, src_b;
   void *src_ptr, *dst_ptr;
   bool ok;
   unsigned i;

   assert(src->num_array_elems == dst->num_array_elems);

   ok = false;

   src_ptr = image_map(src, fences->fence_to_depend_on, false);
   dst_ptr = image_map(dst, NULL, true);
   if (src_ptr == NULL || dst_ptr == NULL)
      goto end;

   src_b.desc = src->blob->desc[src->level];
   gfx_buffer_blit_tgt_set_pos(&src_b, 0, 0, 0);
   src_b.p = src_ptr;

   dst_b.desc = dst->blob->desc[dst->level];
   gfx_buffer_blit_tgt_set_pos(&dst_b, 0, 0, 0);
   dst_b.p = dst_ptr;

   for (i = 0; i < src->num_array_elems; i++)
   {
      gfx_buffer_subsample(&dst_b, &src_b);
      dst_b.p = (uint8_t*)dst_b.p + dst->blob->array_pitch;
      src_b.p = (uint8_t*)src_b.p + src->blob->array_pitch;
   }
   ok = true;

end:
   image_unmap(src, src_ptr, false);
   image_unmap(dst, dst_ptr, true);
   return ok;
}


bool khrn_image_generate_mipmaps_tfu(KHRN_IMAGE_T* src_image,
      KHRN_IMAGE_T* const* dst_images,
      unsigned num_dst_levels, bool skip_dst_level_0,
      glxx_context_fences *fences, bool secure_context)
{
   KHRN_BLOB_T* src_blob = src_image->blob;
   KHRN_BLOB_T* dst_blob = dst_images[0]->blob;
   GFX_BUFFER_DESC_T const* src_desc = &src_blob->desc[src_image->level];
   GFX_BUFFER_DESC_T const* dst_desc = &dst_blob->desc[dst_images[0]->level];
   GFX_LFMT_T src_lfmt = src_desc->planes[0].lfmt;
   GFX_LFMT_T dst_lfmt = dst_desc->planes[0].lfmt;

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
   if (!v3d_build_tfu_cmd(&tfu_cmd, src_desc, dst_desc, num_dst_levels,
            skip_dst_level_0, 0, 0, khrn_get_v3d_version()))
      return false;;

   // Create a struct to hold sync/lock lists.
   v3d_lock_sync *completion_data = v3d_lock_sync_create();
   if (!completion_data)
      return false;;

   // Lock v3d addresses for src/dst blobs.
   gmem_handle_t src_gmemh = src_blob->res_i->handle;
   gmem_handle_t dst_gmemh = dst_blob->res_i->handle;
   v3d_addr_t src_addr = gmem_lock(&completion_data->lock_list, src_gmemh);
   v3d_addr_t dst_addr = dst_gmemh != src_gmemh ? gmem_lock(&completion_data->lock_list, dst_gmemh) : src_addr;
   if (gmem_lock_list_is_bad(&completion_data->lock_list))
   {
      v3d_lock_sync_destroy(completion_data);
      return false;
   }

   /* we do not support yet textures that are a slice from a 3D image */
   assert(src_image->start_slice == 0 && dst_images[0]->start_slice == 0);
   tfu_cmd.src_base_addrs[0] += src_addr + src_image->start_elem * src_blob->array_pitch;
   tfu_cmd.dst_base_addr += dst_addr + dst_images[0]->start_elem * dst_blob->array_pitch;

   // TFU job is runnable when all dependencies for read and write are met.
   v3d_scheduler_deps tfu_job_deps;
   KHRN_INTERLOCK_T* dst_interlock = &dst_blob->res_i->interlock;
   KHRN_INTERLOCK_T* src_interlock = &src_blob->res_i->interlock;
   v3d_scheduler_copy_deps(&tfu_job_deps, khrn_interlock_begin_submit_writer_jobs(dst_interlock));
   if (src_interlock != dst_interlock)
      v3d_scheduler_merge_deps(&tfu_job_deps, khrn_interlock_begin_submit_reader_jobs(src_interlock));;

   if (fences->fence_to_depend_on)
   {
      khrn_fence_flush(fences->fence_to_depend_on);
      v3d_scheduler_merge_deps(&tfu_job_deps, &fences->fence_to_depend_on->deps);
   }

   v3d_scheduler_deps out_deps;
   memset(&out_deps, 0, sizeof out_deps);

   uint32_t src_offset = khrn_image_get_offset(src_image, 0);
   uint32_t dst_offset = khrn_image_get_offset(dst_images[0], 0);
   uint32_t src_lev0_size = src_blob->array_pitch - (src_offset % src_blob->array_pitch); // cope with cube faces
   uint32_t dst_lev0_size = !skip_dst_level_0 ? dst_blob->array_pitch - (dst_offset % dst_blob->array_pitch) : 0;
   uint32_t dst_mips_size = num_dst_levels > 1 ? dst_offset - khrn_image_get_offset(dst_images[num_dst_levels-1], 0) : 0;

   uint64_t tfu_job_id = 0;

   // todo, ideally the kernel API would support batch submission of TFU jobs.
   for (unsigned num_submitted = 0; num_submitted != src_image->num_array_elems; ++num_submitted)
   {
      // Add read/write gmem address ranges to sync-lists.
      gmem_v3d_sync_list_add_range(&completion_data->sync_list, src_gmemh, src_offset, src_lev0_size, GMEM_SYNC_TFU_READ | GMEM_SYNC_RELAXED);
      gmem_v3d_sync_list_add_range(&completion_data->sync_list, dst_gmemh, dst_offset - dst_mips_size, dst_lev0_size + dst_mips_size, GMEM_SYNC_TFU_WRITE | GMEM_SYNC_RELAXED);

      // Submit the TFU job to the scheduler;
      // If this is the last job that we send, make it depend on the tfu jobs
      // submitted for the other elements in the array and add a completion
      // callback to unlock lock/sync lists
      if (num_submitted == src_image->num_array_elems -1)
      {
         v3d_scheduler_merge_deps(&tfu_job_deps, &out_deps);
         tfu_job_id = v3d_scheduler_submit_tfu_job(&tfu_job_deps,
               &completion_data->sync_list, &tfu_cmd, secure_context,
               v3d_lock_sync_completion_and_destroy, completion_data);
      }
      else
      {
         tfu_job_id = v3d_scheduler_submit_tfu_job(&tfu_job_deps,
               &completion_data->sync_list, &tfu_cmd, secure_context, NULL, NULL);
         v3d_scheduler_add_dep(&out_deps, tfu_job_id);

         // Increment offsets for next array element.
         tfu_cmd.src_base_addrs[0] += src_blob->array_pitch;
         tfu_cmd.dst_base_addr += dst_blob->array_pitch;
         src_offset += src_blob->array_pitch;
         dst_offset += dst_blob->array_pitch;
      }
   }


   // Update interlock with tfu_job_id
   v3d_scheduler_deps final_deps;
   final_deps.dependency[0] = tfu_job_id;
   final_deps.n = 1;
   khrn_interlock_end_submit_writer_jobs(dst_interlock, true, &final_deps);
   if (src_interlock != dst_interlock)
      khrn_interlock_end_submit_reader_jobs(src_interlock, true, &final_deps);

   if (tfu_job_id != 0 && fences->fence)
      khrn_fence_job_add(fences->fence, tfu_job_id);

   // Succeeded if we issued jobs for all the array elements. In the case of an error
   // things are left in a consistent state for the the fallback CPU mip generation code
   // to attempt to run.
   return true;
}

unsigned khrn_image_get_depth(const KHRN_IMAGE_T *img)
{
   assert(img->start_slice + img->num_slices <=
         img->blob->desc[img->level].depth);
   return img->num_slices;
}

unsigned khrn_image_get_width(const KHRN_IMAGE_T *img)
{
   return img->blob->desc[img->level].width;
}

unsigned khrn_image_get_height(const KHRN_IMAGE_T *img)
{
   return img->blob->desc[img->level].height;
}

unsigned khrn_image_get_num_elems(const KHRN_IMAGE_T *img)
{
   return img->num_array_elems;
}

unsigned khrn_image_get_num_planes(const KHRN_IMAGE_T *img)
{
   return img->blob->desc[img->level].num_planes;
}

void khrn_image_get_dimensions(const KHRN_IMAGE_T *img, unsigned *width,
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

void khrn_image_get_lfmts(const KHRN_IMAGE_T *img,
      GFX_LFMT_T lfmts[GFX_BUFFER_MAX_PLANES], unsigned *num_planes)
{
   unsigned i;
   GFX_BUFFER_DESC_T *desc = &img->blob->desc[img->level];

   *num_planes = desc->num_planes;

   for (i = 0; i < desc->num_planes; i++)
         lfmts[i] = desc->planes[i].lfmt;
}

void khrn_image_get_fmts(const KHRN_IMAGE_T *img,
      GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES], unsigned *num_planes)
{
   khrn_image_get_lfmts(img, fmts, num_planes);
   for (unsigned i = 0; i != *num_planes; ++i)
      fmts[i] = gfx_lfmt_fmt(fmts[i]);
}

KHRN_RES_INTERLOCK_T* khrn_image_get_res_interlock(const KHRN_IMAGE_T *img)
{
   return img->blob->res_i;
}

void khrn_image_translate_rcfg_color(const KHRN_IMAGE_T *img,
   unsigned plane, unsigned frame_width, unsigned frame_height,
   GFX_BUFFER_RCFG_COLOR_TRANSLATION_T *t)
{
   const GFX_BUFFER_DESC_T *desc = &img->blob->desc[img->level];
   assert(img->num_slices == 1);

   gfx_buffer_translate_rcfg_color(t, desc, plane, img->start_slice,frame_width, frame_height);
   if (khrn_options.use_rgba5551_am &&
       (t->pixel_format == V3D_PIXEL_FORMAT_A1_BGR5))
      t->pixel_format = V3D_PIXEL_FORMAT_A1_BGR5_AM;
}

v3d_memory_format_t khrn_image_translate_memory_format(const KHRN_IMAGE_T *img,
      unsigned plane)
{
   const GFX_BUFFER_DESC_T *desc = &img->blob->desc[img->level];
   return gfx_buffer_translate_memory_format(desc, plane, img->start_slice);
}

unsigned khrn_image_maybe_uif_height_in_ub(const KHRN_IMAGE_T *img,
      unsigned plane)
{
   const GFX_BUFFER_DESC_T *desc = &img->blob->desc[img->level];
   return gfx_buffer_maybe_uif_height_in_ub(desc, plane);
}

unsigned khrn_image_uif_height_in_ub(const KHRN_IMAGE_T *img,
      unsigned plane)
{
   const GFX_BUFFER_DESC_T *desc = &img->blob->desc[img->level];
   return gfx_buffer_uif_height_in_ub(desc, plane);
}

unsigned khrn_image_get_offset(const KHRN_IMAGE_T *img,
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

bool khrn_image_is_one_elem_slice(const KHRN_IMAGE_T *img)
{
   if (img->num_array_elems == 1 && img->num_slices == 1)
      return true;
   return false;
}

bool khrn_image_equal(const KHRN_IMAGE_T *img1, const KHRN_IMAGE_T *img2)
{
   if (img1->blob != img2->blob ||
       img1->level != img2->level ||
       img1->start_elem != img2->start_elem ||
       img1->start_slice != img2->start_slice ||
       img1->num_slices != img2->num_slices)
      return false;
   return true;
}
