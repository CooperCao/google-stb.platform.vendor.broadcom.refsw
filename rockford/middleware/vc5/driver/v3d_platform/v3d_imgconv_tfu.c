/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Image format conversion through the TFU

FILE DESCRIPTION
Sends tfu jobs to the scheduler to convert between different image formats
=============================================================================*/

#include "v3d_platform.h"
#include "v3d_imgconv_internal.h"

extern VCOS_LOG_CAT_T v3d_imgconv_log;
#define VCOS_LOG_CATEGORY (&v3d_imgconv_log)

#ifdef __arm__
#include <arm_neon.h>
#define USE_NEON_FAST_PATHS
#endif

static size_t imgconv_base_align(const struct v3d_imgconv_base_tgt * base)
{
   size_t align = 0;
   for (unsigned p = 0; p < base->desc.num_planes; p++)
      align = GFX_MAX(gfx_buffer_get_align(base->desc.planes[p].lfmt,
                      GFX_BUFFER_ALIGN_RECOMMENDED),
                      align);
   return align;
}

static void fill_yv12_conv_dst_desc(GFX_BUFFER_DESC_T *dst_desc,
                                    const GFX_BUFFER_DESC_T *src_desc)
{
   assert(src_desc->num_planes == 3);
   assert(src_desc->planes[0].lfmt == GFX_LFMT_Y8_UNORM_2D_RSO);
   assert(src_desc->planes[1].lfmt == GFX_LFMT_U8_2X2_UNORM_2D_RSO);
   assert(src_desc->planes[2].lfmt == GFX_LFMT_V8_2X2_UNORM_2D_RSO);

   dst_desc->width = src_desc->width;
   dst_desc->height = src_desc->height;
   dst_desc->depth = src_desc->depth;
   dst_desc->num_planes = 2;
   dst_desc->planes[0] = src_desc->planes[0];
   dst_desc->planes[1].lfmt = gfx_lfmt_to_2d(gfx_lfmt_to_rso(GFX_LFMT_U8_V8_2X2_UNORM));
   dst_desc->planes[1].offset = src_desc->planes[1].offset;
   dst_desc->planes[1].pitch = src_desc->planes[1].pitch * 2;
   dst_desc->planes[1].slice_pitch = src_desc->planes[1].slice_pitch * 2;
}

static bool claim_conversion(
      const struct v3d_imgconv_base_tgt *dst,
      const struct v3d_imgconv_base_tgt *src,
      unsigned int width, unsigned int height,
      unsigned int depth)
{
   /* only from/to origin */
   if (dst->x != 0 || dst->y != 0 || dst->z != 0 ||
        src->x != 0 || src->y != 0 || src->z != 0)
      return false;

   /* we must copy to the whole dst; we might be able to add a lesser
    * restriction , multiple of utiles, but we should be ok with this for the
    * moment */
   if ( width != dst->desc.width || height != dst->desc.height ||
        depth != dst->desc.depth)
      return false;

   assert(width <= src->desc.width && height <= src->desc.height &&
         depth <= src->desc.depth);

   V3D_TFU_COMMAND_T tfu_cmd;
   if (!v3d_build_tfu_cmd(&tfu_cmd, &src->desc, &dst->desc, 1, false, 0, 0,
                          v3d_scheduler_get_v3d_ver()))
      return false;

   return true;
}

static bool claim_yv12_conversion(
      const struct v3d_imgconv_base_tgt *dst,
      const struct v3d_imgconv_base_tgt *src,
      unsigned int width, unsigned int height,
      unsigned int depth)
{
   if (v3d_scheduler_get_v3d_ver() >= V3D_VER_TFU_YV12)
      /* This code is only required for 3.2 or older h/w */
      return false;

   /* only from/to origin */
   if (dst->x != 0 || dst->y != 0 || dst->z != 0 ||
        src->x != 0 || src->y != 0 || src->z != 0)
      return false;

   /* we must copy to the whole dst; we might be able to add a lesser
    * restriction , multiple of utiles, but we should be ok with this for the
    * moment */
   if ( width != dst->desc.width || height != dst->desc.height ||
        depth != dst->desc.depth)
      return false;

   assert(width <= src->desc.width && height <= src->desc.height &&
         depth <= src->desc.depth);

   if (src->desc.num_planes == 3)
   {
      /* Can only convert from YV12 */
      if (src->desc.planes[0].lfmt == GFX_LFMT_Y8_UNORM_2D_RSO &&
          src->desc.planes[1].lfmt == GFX_LFMT_U8_2X2_UNORM_2D_RSO &&
          src->desc.planes[2].lfmt == GFX_LFMT_V8_2X2_UNORM_2D_RSO)
      {
         GFX_BUFFER_DESC_T tmp_desc;
         fill_yv12_conv_dst_desc(&tmp_desc, &src->desc);

         V3D_TFU_COMMAND_T tfu_cmd;
         if (!v3d_build_tfu_cmd(&tfu_cmd, &tmp_desc, &dst->desc, 1, false, 0, 0,
                                v3d_scheduler_get_v3d_ver()))
            return false;

         return true;
      }
   }

   return false;
}

static bool convert_async(
      const struct v3d_imgconv_gmem_tgt *dst, unsigned int dst_off,
      const struct v3d_imgconv_gmem_tgt *src, unsigned int src_off,
      const v3d_scheduler_deps *deps, uint64_t *job_id,
      unsigned int width, unsigned int height,
      unsigned int depth)
{

   v3d_lock_sync *completion_data = v3d_lock_sync_create();
   if (!completion_data)
      return false;

   v3d_addr_t src_addr = gmem_lock(&completion_data->lock_list, src->handle);
   v3d_addr_t dst_addr = gmem_lock(&completion_data->lock_list, dst->handle);
   if (gmem_lock_list_is_bad(&completion_data->lock_list))
   {
      v3d_lock_sync_destroy(completion_data);
      return false;
   }

   src_off += src->base.start_elem * src->base.array_pitch;
   dst_off += dst->base.start_elem * dst->base.array_pitch;

   V3D_TFU_COMMAND_T tfu_cmd;
   bool ok = v3d_build_tfu_cmd(&tfu_cmd, &src->base.desc, &dst->base.desc, 1,
         false, src_addr + src_off, dst_addr + dst_off,
         v3d_scheduler_get_v3d_ver());
   assert(ok);

   for  (unsigned p = 0; p < src->base.desc.num_planes; p++)
   {
      size_t src_plane_off = src_off + src->base.desc.planes[p].offset;
      /* we sync the whole plane, thought the addreess could be inside this
       * plane */
      gmem_v3d_sync_list_add_range(&completion_data->sync_list, src->handle,
            src_plane_off, src->base.plane_sizes[p], GMEM_SYNC_TFU_READ | GMEM_SYNC_RELAXED);
   }

   size_t dst_plane_off = dst_off + dst->base.desc.planes[0].offset;
   gmem_v3d_sync_list_add_range(&completion_data->sync_list, dst->handle,
      dst_plane_off, dst->base.plane_sizes[0], GMEM_SYNC_TFU_WRITE | GMEM_SYNC_RELAXED);


   *job_id = v3d_scheduler_submit_tfu_job(deps, &completion_data->sync_list,
         &tfu_cmd, v3d_lock_sync_completion_and_destroy, completion_data);

   vcos_log_info("Queued TFU conv jobid %"PRIu64" : %dx%d, stride=%d, mips=%d, from(%s %s)->to(%s)",
                 *job_id, tfu_cmd.width, tfu_cmd.height, tfu_cmd.src_strides[0],
                 tfu_cmd.num_mip_levels,
                 v3d_desc_tfu_iformat(tfu_cmd.src_memory_format),
                 v3d_desc_tfu_type(tfu_cmd.src_ttype),
                 v3d_desc_tfu_oformat(tfu_cmd.dst_memory_format));

   return true;
}

/* Create an intermediate gmem buffer to hold a copy of the src buffer */
static bool create_scratch_src_copy(struct v3d_imgconv_gmem_tgt *dst,
                                    const struct v3d_imgconv_base_tgt *src,
                                    size_t *copy_size)
{
   size_t         dst_align;

   *copy_size = v3d_imgconv_base_size(src);
   dst->base = *src;
   dst->base.start_elem = 0; /* src->base.start_elem is element 0 in the copy */
   dst->deps.n = 0; /* scratch has no dependencies */

   dst_align = imgconv_base_align(src);

   dst->handle = gmem_alloc(*copy_size, dst_align,
                            GMEM_USAGE_CPU_WRITE | GMEM_USAGE_V3D_READ | GMEM_USAGE_HINT_DYNAMIC,
                            "imgconv_scratch");

   return dst->handle != GMEM_HANDLE_INVALID;
}

/* Create an intermediate gmem buffer to hold a 2-plane YCbCr buffer */
static bool create_yv12_scratch(struct v3d_imgconv_gmem_tgt *dst,
                                const struct v3d_imgconv_base_tgt *src)
{
   dst->base = *src;
   dst->base.start_elem = 0;
   dst->deps.n = 0;
   dst->base.plane_sizes[1] = src->plane_sizes[1] + src->plane_sizes[2];

   fill_yv12_conv_dst_desc(&dst->base.desc, &src->desc);

   dst->handle = gmem_alloc(v3d_imgconv_base_size(&dst->base), imgconv_base_align(&dst->base),
                            GMEM_USAGE_CPU_WRITE | GMEM_USAGE_V3D_READ | GMEM_USAGE_HINT_DYNAMIC,
                            "imgconv_yv12_scratch");

   return dst->handle != GMEM_HANDLE_INVALID;
}

static void free_prep_buffer(void *data, uint64_t jobId, enum bcm_sched_job_error error)
{
   vcos_unused(jobId);
   vcos_unused(error);
   gmem_handle_t scratch = (gmem_handle_t)data;
   if (scratch != GMEM_HANDLE_INVALID)
      gmem_free(scratch);
}

bool prep_conv_memcpy(
   struct v3d_imgconv_gmem_tgt *dst,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height, unsigned int depth,
   v3d_sched_completion_fn *cleanup_fn, void **cleanup_data)
{
   bool           ok = false;
   size_t         copy_offset;
   size_t         copy_size;

   copy_offset = src->array_pitch * src->start_elem;

   if (!create_scratch_src_copy(dst, src, &copy_size))
      goto end;

   gmem_cpu_sync_list dst_sync_list;
   gmem_cpu_sync_list_init(&dst_sync_list);

   void *dst_data = v3d_imgconv_gmem_tgt_to_ptr(&dst_sync_list, dst, 0, true);
   if (dst_data != NULL)
   {
      gmem_sync_pre_cpu_access_list(&dst_sync_list);

      /* memcpy from cpu (src) into gmem (dst) */
      memcpy(dst_data, (uint8_t*)src_data + copy_offset, copy_size);

      gmem_sync_post_cpu_write_list(&dst_sync_list);

      gmem_unmap(dst->handle);

      ok = true;
   }

   gmem_cpu_sync_list_destroy(&dst_sync_list);

end:
   if (ok)
   {
      /* Ensure the data gets freed when the final conversion is complete */
      *cleanup_fn = free_prep_buffer;
      *cleanup_data = dst->handle;
   }
   else
   {
      if (dst->handle != GMEM_HANDLE_INVALID)
      {
         gmem_free(dst->handle);
         dst->handle = GMEM_HANDLE_INVALID;
      }

      *cleanup_fn = NULL;
      *cleanup_data = NULL;
   }

   return ok;
}

static void yv12_to_YCbCr(
   struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height, unsigned int depth)
{
   uint8_t *dd = (uint8_t*)dst_data;
   uint8_t *ss = (uint8_t*)src_data;

   // memcpy the Y plane - we use that as is
   memcpy(dd + dst->desc.planes[0].offset, ss + src->desc.planes[0].offset,
          height * src->desc.planes[0].pitch);

   assert((width & 1) == 0);
   assert((height & 1) == 0);

   assert(src->desc.planes[1].pitch == src->desc.planes[2].pitch);
   assert((src->desc.planes[1].pitch & 15) == 0);

   width /= 2;
   height /= 2;

   uint8_t *d  = dd + dst->desc.planes[1].offset;
   uint8_t *cr = ss + src->desc.planes[1].offset;
   uint8_t *cb = ss + src->desc.planes[2].offset;

#ifdef USE_NEON_FAST_PATHS
   uint32_t d_pad  = dst->desc.planes[1].pitch - (src->desc.planes[1].pitch * 2);

   // Now interleave the Cb & Cr planes
   for (unsigned y = 0; y < height; y++)
   {
      for (unsigned x = 0; x < src->desc.planes[1].pitch / 16; x++)
      {
         uint8x16x2_t data;
         data.val[0] = vld1q_u8(cb);
         data.val[1] = vld1q_u8(cr);
         vst2q_u8(d, data);

         d  += 32;
         cb += 16;
         cr += 16;
      }

      d += d_pad;
   }
#else
   uint32_t d_pad  = dst->desc.planes[1].pitch - (width * 2);
   uint32_t cr_pad = src->desc.planes[1].pitch - width;
   uint32_t cb_pad = src->desc.planes[2].pitch - width;

   // Now interleave the Cb & Cr planes
   for (unsigned y = 0; y < height; y++)
   {
      for (unsigned x = 0; x < width; x++)
      {
         *d++ = *cb++;
         *d++ = *cr++;
      }

      d  += d_pad;
      cr += cr_pad;
      cb += cb_pad;
   }
#endif
}

static bool prep_conv_yv12(
   struct v3d_imgconv_gmem_tgt *dst,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height, unsigned int depth,
   v3d_sched_completion_fn *cleanup_fn, void **cleanup_data)
{
   bool  ok = false;

   /* Create the intermediate buffer storage */
   if (!create_yv12_scratch(dst, src))
      goto end;

   gmem_cpu_sync_list dst_sync_list;
   gmem_cpu_sync_list_init(&dst_sync_list);

   void *dst_data = v3d_imgconv_gmem_tgt_to_ptr(&dst_sync_list, dst, 0, true);
   if (dst_data != NULL)
   {
      gmem_sync_pre_cpu_access_list(&dst_sync_list);

      /* Interleave the Cb & Cr planes into one CbCr plane */
      yv12_to_YCbCr(&dst->base, dst_data, src, src_data, width, height, depth);

      gmem_sync_post_cpu_write_list(&dst_sync_list);

      gmem_unmap(dst->handle);

      ok = true;
   }

   gmem_cpu_sync_list_destroy(&dst_sync_list);

end:
   if (ok)
   {
      /* Ensure the data gets freed when the final conversion is complete */
      *cleanup_fn = free_prep_buffer;
      *cleanup_data = dst->handle;
   }
   else
   {
      if (dst->handle != GMEM_HANDLE_INVALID)
      {
         gmem_free(dst->handle);
         dst->handle = GMEM_HANDLE_INVALID;
      }

      *cleanup_fn = NULL;
      *cleanup_data = NULL;
   }

   return ok;
}

/* Standard gmem->gmem async TFU conversion */
static v3d_imgconv_methods tfu_path =
{
   .claim         = claim_conversion,
   .convert_async = convert_async,
   .convert_sync  = NULL,
   .convert_prep  = NULL,
};

/* CPU->memcpy->gmem->TFU */
static v3d_imgconv_methods memcpy_tfu_path =
{
   .claim = claim_conversion,
   .convert_async = convert_async,
   .convert_sync = NULL,
   .convert_prep = prep_conv_memcpy
};

/* gmem(YV12)->gmem(YCbCr)->TFU */
static v3d_imgconv_methods yv12_tfu_path =
{
   .claim         = claim_yv12_conversion,
   .convert_async = convert_async,
   .convert_sync  = NULL,
   .convert_prep  = prep_conv_yv12
};

const v3d_imgconv_methods* get_tfu_path(void)
{
   if (v3d_scheduler_get_hub_identity()->has_tfu)
      return &tfu_path;
   else
      return NULL;
}

const v3d_imgconv_methods* get_yv12_tfu_path(void)
{
   if (v3d_scheduler_get_hub_identity()->has_tfu)
      return &yv12_tfu_path;
   else
      return NULL;
}

const v3d_imgconv_methods* get_memcpy_tfu_path(void)
{
   if (v3d_scheduler_get_hub_identity()->has_tfu)
      return &memcpy_tfu_path;
   else
      return NULL;
}
