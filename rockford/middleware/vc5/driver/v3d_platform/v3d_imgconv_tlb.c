/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Image format conversion through the TLB

FILE DESCRIPTION
TLB based image conversion routines
=============================================================================*/

#include "v3d_platform.h"
#include "v3d_imgconv_internal.h"

extern VCOS_LOG_CAT_T v3d_imgconv_log;
#define VCOS_LOG_CATEGORY (&v3d_imgconv_log)

typedef struct conv_data
{
   struct gmem_lock_list      lock_list;
   struct gmem_v3d_sync_list  sync_list;
   gmem_handle_t              control_list;
} conv_data;

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
   if (width != dst->desc.width || height != dst->desc.height ||
       depth != dst->desc.depth)
      return false;

   assert(width <= src->desc.width && height <= src->desc.height &&
          depth <= src->desc.depth);

   if (src->desc.num_planes != 1 || dst->desc.num_planes != 1)
      return false;

   // Convert lfmt into v3d_pixel_format_t
   v3d_pixel_format_t pixFmt = gfx_lfmt_maybe_translate_pixel_format(src->desc.planes[0].lfmt);
   if (pixFmt == V3D_PIXEL_FORMAT_INVALID)
      return false;

   // Source must be RSO, dest must be UIF - both must be 2D
   if (!gfx_lfmt_is_rso(src->desc.planes[0].lfmt) ||
       !gfx_lfmt_is_uif(dst->desc.planes[0].lfmt) ||
       !gfx_lfmt_is_2d(src->desc.planes[0].lfmt)  ||
       !gfx_lfmt_is_2d(dst->desc.planes[0].lfmt))
       return false;

   // Source must be RSO version of dst (i.e. same when swizzling is ignored)
   if (src->desc.planes[0].lfmt != gfx_lfmt_to_rso(dst->desc.planes[0].lfmt))
      return false;

   return true;
}

static bool v3d_build_tlb_conv_clist(V3D_RENDER_INFO_T *render_info, conv_data *data,
                                     const GFX_BUFFER_DESC_T *src_desc,
                                     const GFX_BUFFER_DESC_T *dst_desc,
                                     unsigned num_dst_levels, bool skip_dst_level_0,
                                     v3d_addr_t src_base_addr, v3d_addr_t dst_base_addr)
{
   assert(gfx_lfmt_is_rso(src_desc->planes[0].lfmt));
   assert(gfx_lfmt_is_uif(dst_desc->planes[0].lfmt));
   assert(gfx_lfmt_is_2d(dst_desc->planes[0].lfmt));

   // Convert lfmt into v3d_pixel_format_t
   v3d_pixel_format_t pixFmt = gfx_lfmt_translate_pixel_format(src_desc->planes[0].lfmt);

   // Convert v3d_pixel_format_t into v3d_rt_type_t & v3d_rt_bpp_t
   v3d_rt_type_t  type;
   v3d_rt_bpp_t   bpp;
   v3d_pixel_format_internal_type_and_bpp(&type, &bpp, pixFmt);

   // Count size
   size_t       r_size = 0;  // Render list size
   unsigned int xtiles = 1 + (dst_desc->width - 1)  / 64;
   unsigned int ytiles = 1 + (dst_desc->height - 1) / 64;

   unsigned int numTiles = xtiles * ytiles;

   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // common
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // color rts
   r_size += v3d_cl_rcfg_clear_colors_size(bpp, 15);              // clear data
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // zs clear values
   r_size += numTiles * V3D_CL_LOAD_SIZE;                         // load
   r_size += numTiles * V3D_CL_TILE_COORDS_SIZE;                  // coords
   r_size += numTiles * V3D_CL_STORE_GENERAL_SIZE;                // store
   r_size += V3D_CL_END_RENDER_SIZE;                              // end

   data->control_list = gmem_alloc(r_size, V3D_MAX_CACHE_LINE_SIZE,
                                   GMEM_USAGE_CPU_WRITE | GMEM_USAGE_V3D_READ, "TLB conv control list");

   if (data->control_list == GMEM_HANDLE_INVALID)
      return false;

   uint8_t *start = (uint8_t*)gmem_map(data->control_list);
   if (!start)
   {
      gmem_free(data->control_list);
      return false;
   }

   // Color clear values
   uint32_t clear_colors[4] = { 0 };

   uint8_t *instr = start;

   // Tile Rendering Mode (Common Configuration)
   v3d_cl_tile_rendering_mode_cfg_common(&instr,
                                         1,                       // num RTs
                                         src_desc->width,         // width
                                         src_desc->height,        // height
                                         bpp,                     // bpp enum
                                         false,                   // ms
                                         false,                   // double buffer
                                         false,                   // coverage
                                         V3D_EZ_DIRECTION_LT_LE,  // ez direction
                                         true,                    // disable Early Z
                                         false,                   // short form stencil store
                                         false,                   // short form depth store
                                         0);                      // short form color store disable mask

   v3d_cl_tile_rendering_mode_cfg_color(&instr,
                                        0,                        // render target
                                        bpp,
                                        type,
                                        V3D_DECIMATE_SAMPLE0,
                                        pixFmt,
                                        V3D_DITHER_OFF,
                                        V3D_MEMORY_FORMAT_RASTER,
                                        false,      // Flip Y
                                        15,         // Pad
                                        src_base_addr);

   v3d_cl_rcfg_clear_colors(&instr,
                            0, // Render target
                            clear_colors,
                            type,
                            bpp,
                            15, // Pad
                            gfx_buffer_rso_padded_width(src_desc, 0),
                            gfx_buffer_uif_height_pad_in_ub(dst_desc, 0));

   // HW needs this to be last V3D_CL_TILE_RENDERING_MODE_CFG sub item
   v3d_cl_tile_rendering_mode_cfg_zs_clear_values(&instr, 0, 0.0f);

   for (unsigned int y = 0; y < ytiles; y++)
   {
      for (unsigned int x = 0; x < xtiles; x++)
      {
         v3d_cl_load(&instr,
                     false,   // stencil
                     false,   // depth
                     0);      // rt mask

         v3d_cl_tile_coords(&instr, x, y);

         v3d_cl_store_general(&instr,
                              V3D_LDST_BUF_COLOR0,                      // buffer
                              false,                                    // raw_mode
                              true,                                     // disable_depth_clear
                              true,                                     // disable_stencil_clear
                              true,                                     // disable_color_clear
                              false,                                    // eof
                              true,                                     // disable_double_buf_swap
                              gfx_lfmt_is_uif_xor(dst_desc->planes[0].lfmt) ?
                                                V3D_LDST_MEMORY_FORMAT_UIF_XOR :
                                                V3D_LDST_MEMORY_FORMAT_UIF_NO_XOR,
                              gfx_buffer_uif_height_in_ub(dst_desc, 0), // pad
                              dst_base_addr);                           // addr
      }
   }

   v3d_cl_end_render(&instr);

   assert(instr == start + r_size);

   gmem_unmap(data->control_list);

   render_info->num_renders = 1;
   render_info->render_begins[0] = gmem_lock(&data->lock_list, data->control_list);
   render_info->render_ends[0]   = render_info->render_begins[0] + r_size;

   return true;
}

conv_data* create_conv_data(void)
{
   conv_data *data = malloc(sizeof(conv_data));
   if (data)
   {
      gmem_lock_list_init(&data->lock_list);
      gmem_v3d_sync_list_init(&data->sync_list);
      data->control_list = GMEM_HANDLE_INVALID;
   }
   return data;
}

void destroy_conv_data(void *data)
{
   conv_data *cdata = data;
   gmem_lock_list_unlock_and_destroy(&cdata->lock_list);
   gmem_v3d_sync_list_destroy(&cdata->sync_list);
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

   v3d_addr_t src_addr = gmem_lock(&completion_data->lock_list, src->handle);
   v3d_addr_t dst_addr = gmem_lock(&completion_data->lock_list, dst->handle);

   src_off += src->base.start_elem * src->base.array_pitch;
   dst_off += dst->base.start_elem * dst->base.array_pitch;

   V3D_RENDER_INFO_T render_info;
   bool ok = v3d_build_tlb_conv_clist(&render_info, completion_data, &src->base.desc, &dst->base.desc, 1,
                                      false, src_addr + src_off, dst_addr + dst_off);
   if (!ok)
   {
      destroy_conv_data(completion_data);
      return false;
   }

   if (gmem_lock_list_is_bad(&completion_data->lock_list))
   {
      destroy_conv_data(completion_data);
      return false;
   }

   for  (unsigned p = 0; p < src->base.desc.num_planes; p++)
   {
      size_t src_plane_off = src_off + src->base.desc.planes[p].offset;

      // we sync the whole plane, though the address could be inside this plane
      gmem_v3d_sync_list_add_range(&completion_data->sync_list, src->handle,
            src_plane_off, src->base.plane_sizes[p], GMEM_SYNC_CORE_READ | GMEM_SYNC_RELAXED);
   }

   size_t dst_plane_off = dst_off + dst->base.desc.planes[0].offset;
   gmem_v3d_sync_list_add_range(&completion_data->sync_list, dst->handle,
      dst_plane_off, dst->base.plane_sizes[0], GMEM_SYNC_CORE_WRITE | GMEM_SYNC_RELAXED);

   *job_id = v3d_scheduler_submit_render_job(deps, &completion_data->sync_list,
                                             &render_info, conversion_completed, completion_data);

   //vcos_log_set_level(&v3d_imgconv_log, VCOS_LOG_INFO);

   if (vcos_is_log_enabled(VCOS_LOG_CATEGORY, VCOS_LOG_INFO))
   {
      char fromStr[256];
      char toStr[256];
      gfx_lfmt_sprint(fromStr, 256, 0, src->base.desc.planes[0].lfmt);
      gfx_lfmt_sprint(toStr, 256, 0, dst->base.desc.planes[0].lfmt);

      vcos_log_info("Queued TLB conv jobid %"PRIu64" : %dx%d, from(%s)->to(%s)",
                     *job_id, width, height, fromStr, toStr);
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
