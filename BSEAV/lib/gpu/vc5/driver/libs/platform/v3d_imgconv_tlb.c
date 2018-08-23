/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/

#include "v3d_platform.h"
#include "v3d_imgconv_internal.h"
#include "libs/core/v3d/v3d_tile_size.h"
#include "libs/core/v3d/v3d_cl.h"
#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"
#include "libs/core/v3d/v3d_align.h"

LOG_DEFAULT_CAT("v3d_imgconv_tlb")

typedef struct conv_data
{
   gmem_handle_t              control_list;
} conv_data;

static bool claim_conversion(
      const struct v3d_imgconv_base_tgt *dst,
      const struct v3d_imgconv_base_tgt *src,
      unsigned int width, unsigned int height,
      unsigned int depth, const conversion_info_t *info)
{
   if (!v3d_imgconv_valid_hw_conv_info(info))
      return false;

   /* only from/to origin */
   if (dst->x != 0 || dst->y != 0 || dst->z != 0 ||
       src->x != 0 || src->y != 0 || src->z != 0)
      return false;

   if (width != dst->desc.width || height != dst->desc.height ||
       depth != dst->desc.depth)
      return false;

   assert(width <= src->desc.width && height <= src->desc.height &&
          depth <= src->desc.depth);

   if (src->desc.num_planes != 1 || dst->desc.num_planes != 1)
      return false;

   if (!gfx_buffer_single_region(&src->desc) || !gfx_buffer_single_region(&dst->desc))
      return false;

   /* Source must be RSO, dest must be UIF - both must be 2D */
   if (!gfx_lfmt_is_rso(src->desc.planes[0].lfmt) ||
       !gfx_lfmt_is_uif_family(dst->desc.planes[0].lfmt) ||
       !gfx_lfmt_is_2d(src->desc.planes[0].lfmt)  ||
       !gfx_lfmt_is_2d(dst->desc.planes[0].lfmt))
       return false;

   /* yflip is buggy, so disallow */
   if (gfx_lfmt_get_yflip(&src->desc.planes[0].lfmt) ||
       gfx_lfmt_get_yflip(&dst->desc.planes[0].lfmt))
      return false;

   /* ONLY! case to allow 24bit RGB to 32bit UIF */
   /* last, as this code should be fairly portable. */
   if ((gfx_lfmt_fmt(src->desc.planes[0].lfmt) != GFX_LFMT_R8_G8_B8_UNORM) ||
       (gfx_lfmt_fmt(dst->desc.planes[0].lfmt) != GFX_LFMT_R8_G8_B8_X8_UNORM))
       return false;

   return true;
}

static bool v3d_build_tlb_conv_clist(v3d_subjob *subjob, conv_data *data,
                                     const GFX_BUFFER_DESC_T *src_desc,
                                     const GFX_BUFFER_DESC_T *dst_desc,
                                     v3d_addr_t src_base_addr, v3d_addr_t dst_base_addr)
{
   assert(gfx_lfmt_is_rso(src_desc->planes[0].lfmt));
   assert(gfx_lfmt_is_uif_family(dst_desc->planes[0].lfmt));
   assert(gfx_lfmt_is_2d(dst_desc->planes[0].lfmt));

   struct v3d_tlb_ldst_params ls;
   gfx_buffer_translate_tlb_ldst(&ls, src_base_addr, src_desc, /*plane_i=*/0, /*slice=*/0, /*color=*/true, /*tlb_ms=*/false, /*ext_ms=*/false, V3D_DITHER_OFF);

   struct v3d_tlb_ldst_params dls;
   gfx_buffer_translate_tlb_ldst(&dls, dst_base_addr, dst_desc, /*plane_i=*/0, /*slice=*/0, /*color=*/true, /*tlb_ms=*/false, /*ext_ms=*/false, V3D_DITHER_OFF);

   // Convert v3d_pixel_format_t into v3d_rt_type_t & v3d_rt_bpp_t
   V3D_RT_FORMAT_T rt_format;
#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_pixel_format_to_rt_format(&rt_format, dls.pixel_format);
#else
   v3d_pixel_format_to_rt_format(&rt_format, ls.output_format.pixel);
#endif

   uint32_t tile_w, tile_h;
   v3d_tile_size_pixels(&tile_w, &tile_h, /*ms=*/false, /*double_buffer=*/false, /*num_rts=*/1, rt_format.bpp);

   // Count size
   size_t r_size = 0;  // render list size
   size_t i_size = 0;  // inner loop size
   uint32_t num_tiles_x = gfx_udiv_round_up(dst_desc->width, tile_w);
   uint32_t num_tiles_y = gfx_udiv_round_up(dst_desc->height, tile_h);
   unsigned int num_tiles = num_tiles_x * num_tiles_y;

   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // common
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // color rts
#if !V3D_VER_AT_LEAST(4,1,34,0)
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // stride
#endif
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // zs clear values

#if V3D_VER_AT_LEAST(4,1,34,0)
   i_size += V3D_CL_TILE_COORDS_SIZE;                             // coords
   i_size += V3D_CL_LOAD_SIZE;                                    // load
   i_size += V3D_CL_END_LOADS_SIZE;
#else
   i_size += V3D_CL_LOAD_SIZE;                                    // load
   i_size += V3D_CL_TILE_COORDS_SIZE;                             // coords
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
   i_size += V3D_CL_STORE_SIZE;
   i_size += V3D_CL_END_TILE_SIZE;
#else
   i_size += V3D_CL_STORE_GENERAL_SIZE;                           // store
#endif
   r_size += num_tiles * i_size;
   r_size += V3D_CL_END_RENDER_SIZE;                              // end

   data->control_list = gmem_alloc_and_map(
         r_size,
         V3D_CL_REC_ALIGN,
         GMEM_USAGE_V3D_READ | GMEM_USAGE_HINT_DYNAMIC,
         "TLB conv control list");

   if (data->control_list == GMEM_HANDLE_INVALID)
      return false;

   uint8_t *start = (uint8_t*)gmem_get_ptr(data->control_list);

   uint8_t *instr = start;

   // Tile Rendering Mode (Common Configuration)
   v3d_cl_tile_rendering_mode_cfg_common(&instr,
                                         1,                       // num RTs
                                         dst_desc->width,         // width
                                         dst_desc->height,        // height
                                         rt_format.bpp,           // bpp enum
                                         false,                   // ms
                                         false,                   // double buffer
                                         false,                   // coverage
                                         V3D_EZ_DIRECTION_LT_LE,  // ez direction
                                         true,                    // disable Early Z
#if V3D_VER_AT_LEAST(4,1,34,0)
                                         V3D_DEPTH_TYPE_32F,
                                         false                    // dont early_ds_clear
#else
                                         false,                   // short form stencil store
                                         false,                   // short form depth store
                                         0                        // short form color store disable mask
#endif
                                         );

#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_cl_tile_rendering_mode_cfg_color(&instr,
                                        rt_format.bpp,
                                        rt_format.type,
                                        rt_format.clamp,
                                        rt_format.bpp,
                                        rt_format.type,
                                        rt_format.clamp,
                                        rt_format.bpp,
                                        rt_format.type,
                                        rt_format.clamp,
                                        rt_format.bpp,
                                        rt_format.type,
                                        rt_format.clamp);
#else
   v3d_cl_tile_rendering_mode_cfg_color(&instr,
                                        0, // render target
                                        rt_format.bpp,
                                        rt_format.type,
                                        ls.decimate,
                                        ls.output_format.pixel,
                                        V3D_DITHER_OFF,
                                        ls.memory_format,
                                        ls.flipy,      // yflip
                                        15,            // Pad
                                        ls.addr);
#endif

#if !V3D_VER_AT_LEAST(4,1,34,0)
   // stride
   v3d_cl_tile_rendering_mode_cfg_clear_colors_part3(&instr,
                                                     0, // render target
                                                     0,
                                                     (ls.memory_format == V3D_MEMORY_FORMAT_RASTER) ? ls.stride : ls.flipy_height_px,
                                                     v3d_memory_format_is_uif(ls.memory_format) ? ls.stride : 0);
#endif

   // HW needs this to be last V3D_CL_TILE_RENDERING_MODE_CFG sub item
   v3d_cl_tile_rendering_mode_cfg_zs_clear_values(&instr, 0, 0.0f);

   for (unsigned y = 0; y < num_tiles_y; y++)
   {
      for (unsigned x = 0; x < num_tiles_x; x++)
      {
#if V3D_VER_AT_LEAST(4,1,34,0)
         v3d_cl_tile_coords(&instr, x, y);
         v3d_cl_load(&instr,
                     V3D_LDST_BUF_COLOR0,
                     ls.memory_format,
                     ls.flipy,                                 // yflip
                     ls.decimate,
                     ls.pixel_format,                          // v3d_pixel_format_t
                     ls.load_alpha_to_one,                     // alpha_to_one
                     ls.chan_reverse,                          // chan_reverse
                     ls.rb_swap,                               // rb_swap
                     ls.stride,                                // stride
                     ls.flipy_height_px,                       // height (only used for yflip)
                     ls.addr);
         v3d_cl_end_loads(&instr);
#else
         v3d_cl_load(&instr,
                     false,   // stencil
                     false,   // depth
                     0xe);    // rt mask
         v3d_cl_tile_coords(&instr, x, y);
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
         v3d_cl_store(&instr,
                      V3D_LDST_BUF_COLOR0,
                      dls.memory_format,
                      dls.flipy,                                        // yflip
                      dls.dither,
                      dls.decimate,
                      dls.pixel_format,
                      false,                                            // clear
                      dls.chan_reverse,                                 // chan_reverse
                      dls.rb_swap,                                      // rb_swap
                      dls.stride,                                       // pad
                      dls.flipy_height_px,                              // height (only used for yflip)
                      dls.addr);
         v3d_cl_end_tile(&instr);
#else
         v3d_cl_store_general(&instr,
                              V3D_LDST_BUF_COLOR0,                      // buffer
                              true,                                     // raw_mode
                              true,                                     // disable_depth_clear
                              true,                                     // disable_stencil_clear
                              true,                                     // disable_color_clear
                              false,                                    // eof
                              true,                                     // disable_double_buf_swap
                              v3d_memory_format_to_ldst(dls.memory_format),
                              dls.stride,                               // pad
                              dls.addr);                                // addr
#endif
      }
   }

   v3d_cl_end_render(&instr);

   assert(instr == start + r_size);

   subjob->start = gmem_get_addr(data->control_list);
   subjob->end   = subjob->start + r_size;

   gmem_flush_mapped_buffer(data->control_list);

   return true;
}

static conv_data* create_conv_data(void)
{
   conv_data *data = malloc(sizeof(conv_data));
   if (data)
      data->control_list = GMEM_HANDLE_INVALID;
   return data;
}

static void destroy_conv_data(void *data)
{
   conv_data *cdata = data;
   gmem_free(cdata->control_list);
   free(cdata);
}

static void conversion_completed(void *data, uint64_t job_id,
                                 v3d_sched_job_error job_error)
{
   destroy_conv_data(data);
}

static bool convert_async(
      const struct v3d_imgconv_gmem_tgt *dst, unsigned int dst_off,
      const struct v3d_imgconv_gmem_tgt *src, unsigned int src_off,
      const v3d_scheduler_deps *deps, uint64_t *job_id,
      unsigned int width, unsigned int height,
      unsigned int depth)
{

   conv_data *completion_data = create_conv_data();
   if (!completion_data)
      return false;

   v3d_addr_t src_addr = gmem_get_addr(src->handles[0]) + src->offset;
   v3d_addr_t dst_addr = gmem_get_addr(dst->handles[0]) + dst->offset;

   src_off += src->base.start_elem * src->base.array_pitch;
   dst_off += dst->base.start_elem * dst->base.array_pitch;

   V3D_RENDER_INFO_T render_info;
   memset(&render_info, 0, sizeof(render_info));
   render_info.subjobs_list.num_subjobs = 1;
   v3d_subjob subjob;
   render_info.subjobs_list.subjobs = &subjob;
   bool ok = v3d_build_tlb_conv_clist(&subjob, completion_data, &src->base.desc, &dst->base.desc, src_addr + src_off, dst_addr + dst_off);
   if (!ok)
   {
      destroy_conv_data(completion_data);
      return false;
   }

   // This path is only claimed if dst is secure in secure context and
   // in unsecure context a secure dst is not allowed.
   render_info.secure = gmem_get_usage(dst->handles[0]) & GMEM_USAGE_SECURE;

   const V3D_HUB_IDENT_T* hub_ident = v3d_scheduler_get_hub_identity();
   render_info.cache_ops = v3d_barrier_cache_flushes(V3D_BARRIER_MEMORY_WRITE, V3D_BARRIER_TLB_IMAGE_READ | V3D_BARRIER_TLB_IMAGE_WRITE, hub_ident)
                           | v3d_barrier_cache_cleans(V3D_BARRIER_TLB_IMAGE_WRITE, V3D_BARRIER_MEMORY_READ, hub_ident);

   *job_id = v3d_scheduler_submit_render_job(deps, &render_info, conversion_completed, completion_data);

   if (log_trace_enabled())
   {
      GFX_LFMT_SPRINT(src_desc, src->base.desc.planes[0].lfmt);
      GFX_LFMT_SPRINT(dst_desc, dst->base.desc.planes[0].lfmt);
      log_trace("Queued TLB conv jobid %"PRIu64" : %dx%d, from(%s)->to(%s)",
                     *job_id, width, height, src_desc, dst_desc);
   }

   return true;
}

static v3d_imgconv_methods tlb_path =
{
   .claim            = claim_conversion,
   .convert_async    = convert_async,
   .convert_sync     = NULL,
   .convert_prep     = NULL
};

const v3d_imgconv_methods* get_tlb_path(void)
{
   return &tlb_path;
}