/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_platform.h"
#include "v3d_imgconv_internal.h"

LOG_DEFAULT_CAT("v3d_imgconv_tfu")

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
   /* YV12 is provided by Android and BSG as YCrCb/YVU */
   assert(src_desc->num_planes == 3);
   assert(src_desc->planes[0].lfmt == GFX_LFMT_Y8_UNORM_2D_RSO);
   assert(src_desc->planes[1].lfmt == GFX_LFMT_V8_2X2_UNORM_2D_RSO);
   assert(src_desc->planes[2].lfmt == GFX_LFMT_U8_2X2_UNORM_2D_RSO);

   *dst_desc = *src_desc;
   dst_desc->num_planes = 2;
   dst_desc->planes[1].lfmt = GFX_LFMT_U8_V8_2X2_UNORM_2D_RSO;
   dst_desc->planes[1].pitch *= 2;
}

static bool claim_conversion(
      const struct v3d_imgconv_base_tgt *dst,
      const struct v3d_imgconv_base_tgt *src,
      unsigned int width, unsigned int height,
      unsigned int depth)
{
   if (!gfx_buffer_single_region(&dst->desc))
      return false;

   /* only from/to origin */
   if (dst->x != 0 || dst->y != 0 || dst->z != 0 ||
       src->x != 0 || src->y != 0 || src->z != 0)
      return false;

   /* we must copy to the whole dst; we might be able to add a lesser
    * restriction , multiple of utiles, but we should be ok with this for the
    * moment */
   if (width != dst->desc.width || height != dst->desc.height ||
       depth != dst->desc.depth)
      return false;

   assert(width <= src->desc.width && height <= src->desc.height &&
          depth <= src->desc.depth);

   if (gfx_lfmt_has_depth_stencil(dst->desc.planes[0].lfmt))
   {
      /* The TFU cannot mask writes to only the depth or stencil components
       * of a combined depth/stencil buffer
       */
      if (dst->conversion == V3D_IMGCONV_CONVERSION_DEPTH_ONLY ||
          dst->conversion == V3D_IMGCONV_CONVERSION_STENCIL_ONLY)
          return false;
   }

   /* We have been asked to clamp float depth channels but the TFU cannot do this */
   if (dst->conversion == V3D_IMGCONV_CONVERSION_CLAMP_DEPTH && gfx_buffer_any_float_depth(&dst->desc))
      return false;

   V3D_TFU_COMMAND_T tfu_cmd;
   if (!v3d_build_tfu_cmd(&tfu_cmd, &src->desc, &dst->desc, 1, false, 0, 0))
      return false;

   return true;
}

static bool claim_tfu_conversion(
      const struct v3d_imgconv_base_tgt *dst,
      const struct v3d_imgconv_base_tgt *src,
      unsigned int width, unsigned int height,
      unsigned int depth, const conversion_info_t *info)
{
   if (!v3d_imgconv_valid_hw_conv_info(info))
      return false;

   return claim_conversion(dst, src, width, height, depth);
}

static bool claim_memcpy_tfu_conversion(
      const struct v3d_imgconv_base_tgt *dst,
      const struct v3d_imgconv_base_tgt *src,
      unsigned int width, unsigned int height,
      unsigned int depth, const conversion_info_t *info)
{
   if (!v3d_imgconv_valid_hw_conv_info(info))
      return false;

   if (info->secure_context && info->secure_src)
      return false;

   return claim_conversion(dst, src, width, height, depth);
}

static bool claim_yv12_conversion(
      const struct v3d_imgconv_base_tgt *dst,
      const struct v3d_imgconv_base_tgt *src,
      unsigned int width, unsigned int height,
      unsigned int depth, const conversion_info_t *info)
{

#if V3D_VER_AT_LEAST(3,3,0,0)
   /* This code is only required for 3.2 or older h/w */
   /* The TFU should do the conversion for HW >= 3.3  */
   return false;
#else
   if (!v3d_imgconv_valid_hw_conv_info(info))
      return false;

   if (info->secure_context && info->secure_src)
      return false;

   if (!gfx_buffer_single_region(&dst->desc) || !gfx_buffer_single_region(&src->desc))
      return false; // TODO Multi-region support

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
          src->desc.planes[1].lfmt == GFX_LFMT_V8_2X2_UNORM_2D_RSO &&
          src->desc.planes[2].lfmt == GFX_LFMT_U8_2X2_UNORM_2D_RSO)
      {
         GFX_BUFFER_DESC_T tmp_desc;
         fill_yv12_conv_dst_desc(&tmp_desc, &src->desc);

         V3D_TFU_COMMAND_T tfu_cmd;
         if (!v3d_build_tfu_cmd(&tfu_cmd, &tmp_desc, &dst->desc, 1, false, 0, 0))
            return false;

         return true;
      }
   }

   return false;
#endif
}

static bool claim_sand_m2mc_tfu_conversion(
   const struct v3d_imgconv_base_tgt *dst,
   const struct v3d_imgconv_base_tgt *src,
   unsigned int width, unsigned int height,
   unsigned int depth, const conversion_info_t *info)
{
#if V3D_VER_AT_LEAST(3,3,0,0)
   /* This code is only required for 3.2 or older h/w */
   /* The TFU should do the conversion for HW >= 3.3  */
   return false;
#else
   if (!v3d_imgconv_valid_hw_conv_info(info))
      return false;

   if (info->secure_context && info->secure_src)
      return false;

   /* src must be in contiguous memory to use the M2MC */
   if (!info->contiguous_src)
      return false;

   /* only from/to origin */
   if (dst->x != 0 || dst->y != 0 || dst->z != 0 ||
       src->x != 0 || src->y != 0 || src->z != 0)
      return false;

   /* we must copy to the whole dst; we might be able to add a lesser
   * restriction , multiple of utiles, but we should be ok with this for the
   * moment */
   if (width != dst->desc.width || height != dst->desc.height ||
       depth != dst->desc.depth)
      return false;

   assert(width <= src->desc.width && height <= src->desc.height && depth <= src->desc.depth);

   if (src->desc.num_planes == 2)
   {
      /* Can only convert from SAND */
      if (gfx_lfmt_is_sand_family(src->desc.planes[0].lfmt) &&
          gfx_lfmt_is_sand_family(src->desc.planes[1].lfmt))
      {
         GFX_BUFFER_DESC_T tmp_desc;
         size_t            size, align;

         GFX_LFMT_T tmp_format = GFX_LFMT_R8_G8_B8_A8_UNORM_2D_RSO;
         gfx_buffer_desc_gen(&tmp_desc, &size, &align, GFX_BUFFER_USAGE_NONE,
                             width, height, depth, /*mipLevels=*/1, /*num planes=*/1, &tmp_format);

         V3D_TFU_COMMAND_T tfu_cmd;
         if (!v3d_build_tfu_cmd(&tfu_cmd, &tmp_desc, &dst->desc, 1, false, 0, 0))
            return false;

         v3d_imgconv_gmem_tgt src_gmem_tgt;
         memset(&src_gmem_tgt, 0, sizeof(src_gmem_tgt));
         src_gmem_tgt.base = *src;

         v3d_scheduler_deps null_deps;
         v3d_scheduler_deps_init(&null_deps);

         v3d_imgconv_gmem_tgt dst_gmem_tgt;
         v3d_imgconv_init_gmem_tgt(&dst_gmem_tgt, GMEM_HANDLE_INVALID, 0, &null_deps, &tmp_desc,
                                   0, 0, 0, 0, 0);

         if (!v3d_imgconv_m2mc_convert_surface(&src_gmem_tgt, 0,
                                               &dst_gmem_tgt, 0,
                                               /*validateOnly=*/true))
            return false;

         return true;
      }
   }

   return false;
#endif
}

static void convert_async_tfu(
   const struct v3d_imgconv_gmem_tgt *dst, unsigned int dst_off,
   const struct v3d_imgconv_gmem_tgt *src, unsigned int src_off,
   const v3d_scheduler_deps *deps, uint64_t *job_id,
   unsigned int width, unsigned int height, unsigned int depth,
   v3d_sched_completion_fn completion_fn, void *completion_data)
{
   src_off += src->base.start_elem * src->base.array_pitch;
   dst_off += dst->base.start_elem * dst->base.array_pitch;

   v3d_addr_t src_addrs[GFX_BUFFER_MAX_PLANES] = { 0, };
   for (uint32_t p = 0; p < GFX_BUFFER_MAX_PLANES; p++)
   {
      if (src->handles[p] != GMEM_HANDLE_INVALID)
         src_addrs[p] = gmem_get_addr(src->handles[p]) + src->offset + src_off;
   }

   v3d_addr_t dst_addr = gmem_get_addr(dst->handles[0]) + dst->offset + dst_off;

   V3D_TFU_COMMAND_T tfu_cmd;
   bool ok = v3d_build_tfu_cmd(&tfu_cmd, &src->base.desc, &dst->base.desc, 1,
                               false, src_addrs, dst_addr);
   assert(ok);

   bool has_l3c = v3d_scheduler_get_hub_identity()->has_l3c;
   v3d_cache_ops cache_ops =
      v3d_barrier_cache_flushes(V3D_BARRIER_MEMORY_WRITE, V3D_BARRIER_TFU_READ | V3D_BARRIER_TFU_WRITE, false, has_l3c)
      | v3d_barrier_cache_cleans(V3D_BARRIER_TFU_WRITE, V3D_BARRIER_MEMORY_READ, false, has_l3c);

   // This path is only claimed if dst is secure in secure context and
   // in unsecure context a secure dst is not allowed.
   bool secure_context = gmem_get_usage(dst->handles[0]) & GMEM_USAGE_SECURE;

   *job_id = v3d_scheduler_submit_tfu_job(
      deps,
      cache_ops,
      &tfu_cmd,
      secure_context,
      completion_fn,
      completion_data);

   log_trace("Queued TFU conv jobid %"PRIu64" : %dx%d, stride=%d, mips=%d, from(%s %s)->to(%s)",
      *job_id, tfu_cmd.width, tfu_cmd.height, tfu_cmd.src_strides[0],
      tfu_cmd.num_mip_levels,
      v3d_desc_tfu_iformat(tfu_cmd.src_memory_format),
      v3d_desc_tfu_type(tfu_cmd.src_ttype),
      v3d_desc_tfu_oformat(tfu_cmd.dst_memory_format));
}

static bool convert_async(
      const struct v3d_imgconv_gmem_tgt *dst, unsigned int dst_off,
      const struct v3d_imgconv_gmem_tgt *src, unsigned int src_off,
      const v3d_scheduler_deps *deps, uint64_t *job_id,
      unsigned int width, unsigned int height,
      unsigned int depth)
{
   convert_async_tfu(dst, dst_off, src, src_off, deps, job_id, width, height, depth, NULL, NULL);
   return true;
}

typedef struct
{
   v3d_imgconv_gmem_tgt src;
   unsigned int         src_off;
   v3d_imgconv_gmem_tgt dst;
   unsigned int         dst_off;
} user_mode_data;

static void user_mode_convert(void *data)
{
   user_mode_data *umd = (user_mode_data*)data;

   bool ok = v3d_imgconv_m2mc_convert_surface(&umd->src, umd->src_off,
                                              &umd->dst, umd->dst_off, /*validateOnly=*/false);

   assert(ok);
}

static void user_mode_cleanup(void *data, uint64_t jobId, enum bcm_sched_job_error error)
{
   user_mode_data *umd = (user_mode_data*)data;
   gmem_free(umd->dst.handles[0]);
   free(umd);
}

// Configure async intermediate conversion to RGBA using the M2MC then final pass using TFU
static bool convert_sand_m2mc_async_tfu(
   const struct v3d_imgconv_gmem_tgt *dst, unsigned int dst_off,
   const struct v3d_imgconv_gmem_tgt *src, unsigned int src_off,
   const v3d_scheduler_deps *deps, uint64_t *job_id,
   unsigned int width, unsigned int height,
   unsigned int depth)
{
   // Create an intermediate gmem buffer for m2mc conversion
   size_t            size, align;
   GFX_BUFFER_DESC_T tmp_desc;
   GFX_LFMT_T        tmp_format = GFX_LFMT_R8_G8_B8_A8_UNORM_2D_RSO;
   gfx_buffer_desc_gen(&tmp_desc, &size, &align, GFX_BUFFER_USAGE_NONE,
                       width, height, depth, /*mipLevels=*/1, /*num planes=*/1, &tmp_format);

   gmem_handle_t handle = gmem_alloc(size, align, GMEM_USAGE_V3D_READ | GMEM_USAGE_HINT_DYNAMIC |
                                     GMEM_USAGE_CONTIGUOUS, "imgconv_m2mc_scratch");
   if (handle == GMEM_HANDLE_INVALID)
      return false;

   // Build data for a usermode job
   user_mode_data *umd = (user_mode_data*)calloc(1, sizeof(user_mode_data));
   if (umd == NULL)
   {
      gmem_free(handle);
      return false;
   }
   umd->src     = *src;
   umd->src_off = src_off;
   umd->dst_off = 0;

   v3d_scheduler_deps null_deps;
   v3d_scheduler_deps_init(&null_deps);

   // Fill in the the scratch buffer data
   v3d_imgconv_init_gmem_tgt(&umd->dst, handle, /*offset*/0, &null_deps, &tmp_desc, 0, 0, 0, 0, 0);

   // Create a user-mode job that will do the SAND->RGBA conversion using the M2MC when the src
   // deps are satisfied
   uint64_t user_job = v3d_scheduler_submit_usermode_job(&src->deps, user_mode_convert, umd);

   log_trace("Queued M2MC conv jobid %"PRIu64" : %dx%d, to(%s)",
             user_job, width, height, gfx_lfmt_desc(tmp_format));

   // Combine all the deps so that the m2mc conversion is done and the dst buffer is ready before
   // the TFU job starts
   v3d_scheduler_deps tfu_deps = dst->deps;
   v3d_scheduler_add_dep(&tfu_deps, user_job);

   // Now make a TFU job to go from RGBA->dest
   convert_async_tfu(dst, dst_off, &umd->dst, 0, &tfu_deps, job_id, width, height, depth,
                     user_mode_cleanup, umd);
   return true;
}

/* Create an intermediate gmem buffer to hold a copy of the src buffer */
static bool create_scratch_src_copy(struct v3d_imgconv_gmem_tgt *dst,
                                    const struct v3d_imgconv_base_tgt *src,
                                    size_t *copy_size)
{
   *copy_size = v3d_imgconv_base_size(src);
   dst->base = *src;
   dst->base.start_elem = 0; /* src->base.start_elem is element 0 in the copy */
   dst->deps.n = 0; /* scratch has no dependencies */

   size_t dst_align = imgconv_base_align(src);

   memset(dst->handles, 0, sizeof(dst->handles));
   dst->handles[0] = gmem_alloc_and_map(
      *copy_size,
      dst_align,
      GMEM_USAGE_V3D_READ | GMEM_USAGE_HINT_DYNAMIC,
      "imgconv_scratch");
   dst->offset = 0;

   return dst->handles[0] != GMEM_HANDLE_INVALID;
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

   memset(dst->handles, 0, sizeof(dst->handles));
   dst->handles[0] = gmem_alloc_and_map(v3d_imgconv_base_size(&dst->base), imgconv_base_align(&dst->base),
                            GMEM_USAGE_V3D_READ | GMEM_USAGE_HINT_DYNAMIC,
                            "imgconv_yv12_scratch");
   dst->offset = 0;

   return dst->handles[0] != GMEM_HANDLE_INVALID;
}

static void free_prep_buffer(void *data, uint64_t jobId, enum bcm_sched_job_error error)
{
   unused(jobId);
   unused(error);
   gmem_handle_t scratch = (gmem_handle_t)data;
   if (scratch != GMEM_HANDLE_INVALID)
      gmem_free(scratch);
}

static bool prep_conv_memcpy(
   struct v3d_imgconv_gmem_tgt *dst,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height, unsigned int depth,
   v3d_sched_completion_fn *cleanup_fn, void **cleanup_data)
{
   size_t copy_size;
   if (!create_scratch_src_copy(dst, src, &copy_size))
   {
      *cleanup_fn = NULL;
      *cleanup_data = NULL;
      return false;
   }

   /* memcpy from cpu (src) into gmem (dst) */
   size_t copy_offset = src->array_pitch * src->start_elem;
   memcpy(gmem_get_ptr(dst->handles[0]), (uint8_t*)src_data + copy_offset, copy_size);
   gmem_flush_mapped_buffer(dst->handles[0]);

   /* Ensure the data gets freed when the final conversion is complete */
   *cleanup_fn = free_prep_buffer;
   *cleanup_data = dst->handles[0];
   return true;
}

/* This function takes a YCrCb 3 planes buffer  */
/* into a YCbCr 2 plane buffer (Note the change */
/* between CrCb and CbCr)                       */
/* Both Android and BSG provide YCrCb buffers   */
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
   /* Create the intermediate buffer storage */
   if (!create_yv12_scratch(dst, src))
   {
      *cleanup_fn = NULL;
      *cleanup_data = NULL;
      return false;
   }

   /* Interleave the Cb & Cr planes into one CbCr plane */
   yv12_to_YCbCr(&dst->base, gmem_get_ptr(dst->handles[0]), src, src_data, width, height, depth);
   gmem_flush_mapped_buffer(dst->handles[0]);

   /* Ensure the data gets freed when the final conversion is complete */
   *cleanup_fn = free_prep_buffer;
   *cleanup_data = dst->handles[0];
   return true;
}

/* Standard gmem->gmem async TFU conversion */
static v3d_imgconv_methods tfu_path =
{
   .claim         = claim_tfu_conversion,
   .convert_async = convert_async,
   .convert_sync  = NULL,
   .convert_prep  = NULL,
};

/* CPU->memcpy->gmem->TFU */
static v3d_imgconv_methods memcpy_tfu_path =
{
   .claim = claim_memcpy_tfu_conversion,
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

/* gmem(sand)->M2MC->gmem(RGB)->TFU */
static v3d_imgconv_methods sand_m2mc_tfu_path =
{
   .claim = claim_sand_m2mc_tfu_conversion,
   .convert_async = convert_sand_m2mc_async_tfu,
   .convert_sync = NULL,
   .convert_prep = NULL
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

const v3d_imgconv_methods* get_sand_m2mc_tfu_path(void)
{
   if (v3d_scheduler_get_hub_identity()->has_tfu)
      return &sand_m2mc_tfu_path;
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
