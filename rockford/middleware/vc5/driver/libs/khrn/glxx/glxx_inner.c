/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Functions for driving the hardware for both GLES1.1 and GLES2.0.
=============================================================================*/

#ifdef KHRN_GEOMD
#include "libs/tools/geomd/geomd.h"
#endif
#include "../common/khrn_int_common.h"
#include "../common/khrn_options.h"

#include "glxx_server_internal.h"
#include "glxx_translate.h"
#include "glxx_hw.h"
#include "glxx_hw_tile_list.h"
#include "glxx_inner.h"
#include "gl_public_api.h"

#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/v3d/v3d_util.h"
#include "libs/core/v3d/v3d_tile_size.h"
#include "libs/core/v3d/v3d_choose_supertile.h"
#include "libs/util/gfx_util/gfx_util_morton.h"

#include "libs/platform/gmem.h"

//#define GLXX_INNER_DEBUG 1

#include <stdio.h>

#include "../egl/egl_platform.h"
#include "../common/khrn_process.h"
#include "../common/khrn_interlock.h"
#include "../common/khrn_stats.h"
#include "../common/khrn_counters.h"
#include "../common/khrn_render_state.h"
#include "../common/khrn_fmem.h"

LOG_DEFAULT_CAT("glxx_inner")

static const uint32_t c_initial_tile_alloc_block_size = V3D_TILE_ALLOC_BLOCK_SIZE_MIN;
static const uint32_t c_tile_alloc_block_size = 128;

static void log_trace_rs(const GLXX_HW_RENDER_STATE_T *rs, const char *msg)
{
   if (!log_trace_enabled() || !rs)
      return;

   const GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;
   log_trace(
      "%s [rs] ord = %u, label = %s, rt count = %u, fb size %u x %u, %s",
      msg,
      khrn_hw_render_state_allocated_order(rs),
      "-",
      fb->rt_count,
      fb->width, fb->height,
      fb->ms               ? "multisample "        : "");
   for (unsigned b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
      if (rs->color_buffer_state[b] != GLXX_BUFSTATE_MISSING)
      {
         GFX_LFMT_SPRINT(lfmt_desc, khrn_image_plane_lfmt(&fb->color[b]));
         log_trace("[rs] color %u lfmt=%s %s",
            b, lfmt_desc, glxx_bufstate_desc(rs->color_buffer_state[b]));
      }

   log_trace("[rs] depth: %s", glxx_bufstate_desc(rs->depth_buffer_state));
   log_trace("[rs] stencil: %s", glxx_bufstate_desc(rs->stencil_buffer_state));
}

bool glxx_hw_record_write_if_image(KHRN_FMEM_T *fmem, KHRN_IMAGE_PLANE_T *image_plane)
{
   KHRN_RES_INTERLOCK_T *res_i;

   if (!image_plane->image)
      return true;
   res_i = khrn_image_get_res_interlock(image_plane->image);
   return khrn_fmem_record_res_interlock(fmem, res_i, true, ACTION_RENDER);
}

static bool begin_img_write(glxx_bufstate_t *bufstate, KHRN_FMEM_T *fmem, const KHRN_IMAGE_T *img)
{
   if (img)
   {
      KHRN_RES_INTERLOCK_T *res_i = khrn_image_get_res_interlock(img);
      *bufstate = glxx_bufstate_begin(!khrn_interlock_is_valid(&res_i->interlock));
      return khrn_fmem_record_res_interlock(fmem, res_i, /*write=*/true, ACTION_RENDER);
   }
   else
   {
      *bufstate = GLXX_BUFSTATE_MISSING;
      return true;
   }
}

static void glxx_hw_render_state_delete(GLXX_HW_RENDER_STATE_T *rs)
{
   // ensure server isn't pointing to us after we're deleted
   if (rs->server_state->current_render_state == rs)
      rs->server_state->current_render_state = NULL;

   glxx_destroy_hw_framebuffer(&rs->installed_fb);

   KHRN_MEM_ASSIGN(rs->last_occlusion_query.query, NULL);
   rs->last_occlusion_query.instance = 0;

   for (unsigned i=0; i<rs->num_blits; i++) {
      glxx_destroy_hw_framebuffer(&rs->tlb_blits[i].dst_fb);
   }
   rs->num_blits = 0;
   khrn_render_state_delete((KHRN_RENDER_STATE_T *)rs);
}

void glxx_hw_render_state_reset_z_prepass(GLXX_HW_RENDER_STATE_T *rs)
{
   rs->z_prepass_started = false;
   rs->z_prepass_stopped = false;
   rs->z_prepass_allowed = khrn_options.z_prepass && rs->installed_fb.depth.image != NULL;
}

static void init_clear_colors(uint32_t clear_colors[GLXX_MAX_RENDER_TARGETS][4])
{
#if !defined(NDEBUG)
   // Bright pink background indicates undefined framebuffer contents.
   // You did not clear after swap buffers.
   float color[4] = {1.0, 0.0, 0.5, 1.0}; /*R,G,B,A */
#else
   float color[4] = {0.0, 0.0, 0.0, 1.0}; /*R,G,B,A */
#endif

   for (unsigned b = 0; b < GLXX_MAX_RENDER_TARGETS; b++)
   {
      for (unsigned i = 0; i < 4; i++)
         clear_colors[b][i] = gfx_float_to_bits(color[i]);
   }
}

bool glxx_hw_start_frame_internal(GLXX_HW_RENDER_STATE_T *rs,
      const GLXX_HW_FRAMEBUFFER_T *fb)
{
   if (!khrn_fmem_init(&rs->fmem, (KHRN_RENDER_STATE_T *) rs))
      return false;

   /* Record security status */
   rs->fmem.br_info.secure = egl_context_gl_secure(rs->server_state->context);

   init_clear_colors(rs->clear_colors);

   glxx_assign_hw_framebuffer(&rs->installed_fb, fb);

   for (unsigned b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      glxx_bufstate_t nonms_bufstate;
      if (!begin_img_write(&nonms_bufstate, &rs->fmem, fb->color[b].image))
         return false;

      glxx_bufstate_t ms_bufstate;
      if (!begin_img_write(&ms_bufstate, &rs->fmem, fb->color_ms[b].image))
         return false;

      if (nonms_bufstate == GLXX_BUFSTATE_MISSING)
      {
         /* Either RT not attached or only have multisample buffer */
         rs->color_load_from_ms[b] = true;
         rs->color_buffer_state[b] = ms_bufstate;
      }
      else switch (ms_bufstate)
      {
      case GLXX_BUFSTATE_MISSING:
      case GLXX_BUFSTATE_UNDEFINED:
         /* Multisample buffer missing or invalid: load from downsampled buffer */
         rs->color_load_from_ms[b] = false;
         rs->color_buffer_state[b] = nonms_bufstate;
         break;
      case GLXX_BUFSTATE_MEM:
         /* Have both downsampled and multisample buffer. Multisample buffer is
          * marked as valid so downsampled should be too. Load from multisample
          * buffer. */
         assert(nonms_bufstate == GLXX_BUFSTATE_MEM);
         rs->color_load_from_ms[b] = true;
         rs->color_buffer_state[b] = GLXX_BUFSTATE_MEM;
         break;
      default:
         unreachable();
      }
   }

   if (!begin_img_write(&rs->depth_buffer_state, &rs->fmem, fb->depth.image))
      return false;

   if (!begin_img_write(&rs->stencil_buffer_state, &rs->fmem, fb->stencil.image))
      return false;

   //  Default to true. Will be disabled if installed while dither is disabled
   rs->dither        = true;
   rs->depth_value   = 1.0f;
   rs->stencil_value = 0;

   glxx_ez_init(&rs->ez, khrn_options.early_z);

   // initialise record keeping (assume rs is zero filled already)
   if (GLXX_MULTICORE_BIN_ENABLED)
   {
      rs->cl_record_threshold = GLXX_MULTICORE_BIN_SPLIT_THRESHOLD/4;
      rs->cl_record_remaining = GLXX_MULTICORE_BIN_SPLIT_THRESHOLD/4;
      rs->do_multicore_bin = khrn_get_num_bin_subjobs() > 1;
   }
#ifndef NDEBUG
   rs->cl_records[0].in_begin = GLXX_CL_STATE_NUM;
#endif

   glxx_hw_render_state_reset_z_prepass(rs);

   return true;
}

static void master_cl_supertile_range(
   uint32_t *morton_flags, uint32_t *begin_supertile, uint32_t *end_supertile,
   uint32_t num_cores, uint32_t core,
   unsigned int num_supertiles_x, unsigned int num_supertiles_y)
{
   uint32_t num_supertiles = num_supertiles_y * num_supertiles_x;

   *morton_flags = 0;

   if (khrn_options.partition_supertiles_in_sw)
   {
      /* Same supertile order for all cores, but give each core an exclusive
       * subset of the supertiles... */

      uint32_t supertiles_per_core = num_supertiles / num_cores;
      uint32_t extra_supertiles = num_supertiles - (num_cores * supertiles_per_core);

      *begin_supertile = core * supertiles_per_core;
      *end_supertile = (core + 1) * supertiles_per_core;

      /* Distribute extra supertiles */
      *begin_supertile += gfx_umin(core, extra_supertiles);
      *end_supertile += gfx_umin(core + 1, extra_supertiles);

      assert(*begin_supertile <= *end_supertile);
      assert(*end_supertile <= num_supertiles);
   }
   else
   {
      /* All cores are given all supertiles, but in a different order */

      /* TODO Experiment with this */
      switch (khrn_options.all_cores_same_st_order ? 0 :
         /* TODO Quick hack for 8 cores */
         (core & 3))
      {
      case 0:  break;
      case 1:  *morton_flags |= GFX_MORTON_REVERSE; break;
      case 2:  *morton_flags |= GFX_MORTON_FLIP_Y; break;
      case 3:  *morton_flags |= GFX_MORTON_FLIP_Y | GFX_MORTON_REVERSE; break;
      default: not_impl();
      }

      *begin_supertile = 0;
      *end_supertile = num_supertiles;
   }

   /* TODO Is this sensible? */
   if (num_supertiles_x < num_supertiles_y)
      *morton_flags |= GFX_MORTON_TRANSPOSE;
}

struct glxx_hw_tile_cfg
{
   bool ms, double_buffer;
   v3d_rt_bpp_t max_rt_bpp;
   unsigned num_tiles_x, num_tiles_y;
};

static bool create_master_cl(GLXX_HW_RENDER_STATE_T *rs,
   const struct glxx_hw_tile_cfg *tile_cfg,
   const struct glxx_hw_tile_list_rcfg *rcfg,
   v3d_addr_t const generic_tile_list[2],
   uint32_t num_cores, uint32_t core)
{
   GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;

   log_trace_rs(rs, "[create_master_cl]");;

   size_t r_size = 0;  // Render list size

   // Rendering mode config
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // common
#if V3D_HAS_NEW_TLB_CFG
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // color
#else
   r_size += fb->rt_count * V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;  // color
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // z_stencil
   if (rcfg->sep_stencil)
      r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;              // sep_stencil
#endif
   for (unsigned b = 0; b < fb->rt_count; ++b)
      r_size += v3d_cl_rcfg_clear_colors_size(fb->color_internal_bpp[b]);
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // zs clear values

   // Initial TLB clear
#if V3D_HAS_NEW_TLB_CFG
   {
      size_t clear_size =
         V3D_CL_TILE_COORDS_SIZE +
         V3D_CL_END_LOADS_SIZE +
         V3D_CL_STORE_SIZE +
         V3D_CL_CLEAR_SIZE +
         V3D_CL_END_TILE_SIZE;
      r_size += (tile_cfg->double_buffer ? 2 : 1) * clear_size;
   }
#else
   r_size += V3D_CL_TILE_COORDS_SIZE;
#if !V3D_VER_AT_LEAST(3,3,0,0)
   // Workaround GFXH-1320 if occlusion queries were used in this control list.
   bool workaround_gfxh_1320 = khrn_fmem_get_common_persist(&rs->fmem)->occlusion_query_list != NULL;
   r_size += workaround_gfxh_1320 ? glxx_workaround_gfxh_1320_size() : 0;
#endif
   r_size += V3D_CL_STORE_GENERAL_SIZE;
#endif

   r_size += V3D_CL_FLUSH_VCD_CACHE_SIZE;
   r_size += V3D_CL_TILE_LIST_INITIAL_BLOCK_SIZE_SIZE;
   r_size += V3D_CL_MULTICORE_RENDERING_SUPERTILE_CFG_SIZE;
   r_size += V3D_CL_GENERIC_TILE_LIST_SIZE;

   uint32_t supertile_w_in_tiles = 0;
   uint32_t supertile_h_in_tiles = 0;
   v3d_choose_supertile_sizes(tile_cfg->num_tiles_x, tile_cfg->num_tiles_y,
                              khrn_options.min_supertile_w, khrn_options.min_supertile_h,
                              khrn_options.max_supertiles,
                              &supertile_w_in_tiles, &supertile_h_in_tiles);

   uint32_t frame_w_in_supertiles = gfx_udiv_round_up(tile_cfg->num_tiles_x, supertile_w_in_tiles);
   uint32_t frame_h_in_supertiles = gfx_udiv_round_up(tile_cfg->num_tiles_y, supertile_h_in_tiles);
   r_size += V3D_CL_SUPERTILE_COORDS_SIZE * frame_w_in_supertiles * frame_h_in_supertiles;

   r_size += V3D_CL_END_RENDER_SIZE;

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, r_size);
   if (!instr)
      return false;

   v3d_depth_type_t depth_type =
      fb->depth.image ? gfx_lfmt_translate_depth_type(khrn_image_plane_lfmt(&fb->depth)) : V3D_DEPTH_TYPE_32F;

   v3d_cl_tile_rendering_mode_cfg_common(&instr,
      fb->rt_count,
      fb->width,
      fb->height,
      tile_cfg->max_rt_bpp,
      tile_cfg->ms,
      tile_cfg->double_buffer,
      false, // Coverage
      rs->ez.rcfg_ez_direction,
      rs->ez.rcfg_ez_disable,
#if V3D_HAS_NEW_TLB_CFG
      depth_type,
      rcfg->early_ds_clear
#else
      rcfg->stencil_store,
      rcfg->depth_store,
      ~rcfg->rt_store_mask & gfx_mask(V3D_MAX_RENDER_TARGETS)
#endif
      );

#if V3D_HAS_NEW_TLB_CFG
   {
      V3D_CL_TILE_RENDERING_MODE_CFG_T i;
      i.type = V3D_RCFG_TYPE_COLOR;
      for (unsigned b = 0; b < V3D_MAX_RENDER_TARGETS; ++b)
      {
         if (b < fb->rt_count)
         {
            i.u.color.rts[b].internal_bpp = fb->color_internal_bpp[b];
            i.u.color.rts[b].internal_type = fb->color_internal_type[b];
         }
         else
         {
            i.u.color.rts[b].internal_bpp = V3D_RT_BPP_32;
            i.u.color.rts[b].internal_type = V3D_RT_TYPE_8;
         }
      }
      v3d_cl_tile_rendering_mode_cfg_indirect(&instr, &i);
   }
#else
   for (unsigned b = 0; b < fb->rt_count; ++b)
   {
      const struct v3d_tlb_ldst_params *ls = &rcfg->rt_ls[b];
      v3d_cl_tile_rendering_mode_cfg_color(&instr,
         b, // render target
         fb->color_internal_bpp[b],
         fb->color_internal_type[b],
         ls->decimate,
         ls->output_format.pixel,
         ls->dither,
         ls->memory_format,
         ls->flipy,
         /*pad=*/15,
         ls->addr);
   }

   // Depth / packed depth and stencil
   assert(!rcfg->depth_ls.flipy);
   assert(rcfg->depth_ls.dither == V3D_DITHER_OFF);
   v3d_cl_tile_rendering_mode_cfg_z_stencil(&instr,
      depth_type,
      rcfg->depth_ls.decimate,
      rcfg->depth_ls.output_format.depth,
      rcfg->depth_ls.memory_format,
      rcfg->depth_ls.stride,
      rcfg->depth_ls.addr);

   // Stencil
   if (rcfg->sep_stencil)
   {
      assert(!rcfg->sep_stencil_ls.flipy);
      assert(rcfg->sep_stencil_ls.dither == V3D_DITHER_OFF);
      v3d_cl_tile_rendering_mode_cfg_separate_stencil(&instr,
         rcfg->sep_stencil_ls.decimate,
         rcfg->sep_stencil_ls.memory_format,
         rcfg->sep_stencil_ls.stride,
         rcfg->sep_stencil_ls.addr);
   }
#endif

   // Color clear values
   for (unsigned b = 0; b < fb->rt_count; ++b)
   {
      uint32_t clear_colors[4];
      memcpy(clear_colors, rs->clear_colors[b], sizeof(clear_colors));

      /* If the render target does not have an alpha channel, we need alpha in
       * the TLB to be 1 to get correct blending. We ensure this by explicitly
       * clearing to 1 at the start, and by disabling alpha writes so it
       * doesn't change. Note that we must check api_fmt since the actual lfmt
       * might contain alpha when the original internalformat did not. */
      KHRN_IMAGE_T const *img = fb->color_ms[b].image ? fb->color_ms[b].image : fb->color[b].image;
      if (img && !gfx_lfmt_has_alpha(img->api_fmt))
         clear_colors[3] = gfx_float_to_bits(1.0f);

#if !V3D_HAS_NEW_TLB_CFG
      const struct v3d_tlb_ldst_params *ls = &rcfg->rt_ls[b];
#endif
      v3d_cl_rcfg_clear_colors(&instr, b, clear_colors,
         fb->color_internal_type[b], fb->color_internal_bpp[b]
#if !V3D_HAS_NEW_TLB_CFG
         , (ls->memory_format == V3D_MEMORY_FORMAT_RASTER) ? ls->stride : ls->flipy_height_px,
         v3d_memory_format_is_uif(ls->memory_format) ? ls->stride : 0
#endif
         );
   }

   // Depth and stencil clear values
   // HW needs this to be last V3D_CL_TILE_RENDERING_MODE_CFG sub item
   v3d_cl_tile_rendering_mode_cfg_zs_clear_values(&instr,
      rs->stencil_value,
      v3d_snap_depth(rs->depth_value, depth_type));

#ifdef KHRN_GEOMD
   fmem_debug_info_insert(&rs->fmem,
      khrn_fmem_hw_address(&rs->fmem, instr),
      ~0, khrn_hw_render_state_allocated_order(rs));
#endif

   // Initial TLB clear
#if V3D_HAS_NEW_TLB_CFG
   for (unsigned i = 0; i != (tile_cfg->double_buffer ? 2 : 1); ++i)
   {
      v3d_cl_tile_coords(&instr, 0, 0);
      v3d_cl_end_loads(&instr);
      v3d_cl_store(&instr, /* Dummy store... */
         V3D_LDST_BUF_NONE,
         V3D_MEMORY_FORMAT_RASTER,
         /*flipy=*/false,
         V3D_DITHER_OFF,
         V3D_DECIMATE_SAMPLE0,
         V3D_PIXEL_FORMAT_SRGB8_ALPHA8,
         /*clear=*/false,
         /*stride=*/0,
         /*height=*/0,
         /*addr=*/0);
      v3d_cl_clear(&instr, /*rts=*/true, /*depth_stencil=*/!rcfg->early_ds_clear);
      v3d_cl_end_tile(&instr);
   }
#else
   // Note: If you plan to 'optimize' disable depth and stencil clears
   //       when not needed, be careful with packed depth/stencil
   v3d_cl_tile_coords(&instr, 0, 0);
#if !V3D_VER_AT_LEAST(3,3,0,0)
   if (workaround_gfxh_1320 && !glxx_workaround_gfxh_1320(&instr, &rs->fmem))
      return false;
#endif
   v3d_cl_store_general(&instr,
      V3D_LDST_BUF_NONE,                  // buffer
      false,                              // raw_mode
      false,                              // disable_depth_clear
      false,                              // disable_stencil_clear
      false,                              // disable_color_clear
      false,                              // eof
      true,                               // disable_double_buf_swap
      V3D_LDST_MEMORY_FORMAT_UIF_NO_XOR,  // memory_format
      0,                                  // pad
      0);                                 // addr
#endif

   v3d_cl_flush_vcd_cache(&instr);

   v3d_cl_tile_list_initial_block_size(&instr,
      v3d_translate_tile_alloc_block_size(c_initial_tile_alloc_block_size),
      /*chain=*/true);

   v3d_cl_multicore_rendering_supertile_cfg(&instr,
      supertile_w_in_tiles,
      supertile_h_in_tiles,
      frame_w_in_supertiles,
      frame_h_in_supertiles,
      tile_cfg->num_tiles_x,
      tile_cfg->num_tiles_y,
      num_cores > 1,
      V3D_SUPERTILE_ORDER_RASTER, // TODO enable morton
      rs->fmem.br_info.num_bins);

   v3d_cl_generic_tile_list(&instr, generic_tile_list[0], generic_tile_list[1]);

   if (khrn_options.isolate_frame == khrn_fmem_frame_i)
   {
      // isolate requested supertile
      v3d_cl_supertile_coords(&instr,
         gfx_umin(khrn_options.isolate_supertile_x, frame_w_in_supertiles - 1u),
         gfx_umin(khrn_options.isolate_supertile_y, frame_w_in_supertiles - 1u));
   }
   else
   {
      uint32_t morton_flags, begin_supertile, end_supertile;
      master_cl_supertile_range(
         &morton_flags, &begin_supertile, &end_supertile,
         num_cores, core,
         frame_w_in_supertiles, frame_h_in_supertiles);

      GFX_MORTON_STATE_T morton;
      gfx_morton_init(&morton, frame_w_in_supertiles, frame_h_in_supertiles, morton_flags);

      uint32_t x, y;
      for (unsigned i = 0; gfx_morton_next(&morton, &x, &y, NULL); ++i)
         if (i >= begin_supertile && i < end_supertile)
            v3d_cl_supertile_coords(&instr, x, y);
   }

   v3d_cl_end_render(&instr);

   khrn_fmem_end_cle(&rs->fmem, instr);

   return true;
}

static bool create_bin_cl(GLXX_HW_RENDER_STATE_T *rs, const struct glxx_hw_tile_cfg *tile_cfg, uint8_t *draw_clist_ptr, GLXX_CL_RECORD_T* start_record)
{
   log_trace("[create_bin_cl] rs %u, num_tiles %u x %u, rt count = %u",
      khrn_hw_render_state_allocated_order(rs),
      tile_cfg->num_tiles_x, tile_cfg->num_tiles_y,
      rs->installed_fb.rt_count);

   {
      // allocate tile state memory
      v3d_addr_t tile_state_addr = khrn_fmem_tile_state_alloc(&rs->fmem, tile_cfg->num_tiles_x * tile_cfg->num_tiles_y * V3D_TILE_STATE_SIZE);
      if (!tile_state_addr)
         return false;

      unsigned size = 0
         + V3D_CL_TILE_BINNING_MODE_CFG_SIZE
         + V3D_CL_OCCLUSION_QUERY_COUNTER_ENABLE_SIZE
         + V3D_CL_FLUSH_VCD_CACHE_SIZE
         + V3D_CL_START_TILE_BINNING_SIZE
         + V3D_CL_PRIM_LIST_FORMAT_SIZE
         + V3D_CL_WAIT_TRANSFORM_FEEDBACK_SIZE;
      uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, size);
      if (!instr)
         return false;

      // At least Simpenrose wants 120.1 followed by 120.0 and not the other way around

      // CLE 120.0 - 9 bytes (1 + 8)
      v3d_cl_tile_binning_mode_cfg_part1(&instr,
         true, // auto_init_tile_state
         v3d_translate_tile_alloc_block_size(c_initial_tile_alloc_block_size),
         v3d_translate_tile_alloc_block_size(c_tile_alloc_block_size),
         tile_state_addr,
         tile_cfg->num_tiles_x,
         tile_cfg->num_tiles_y,
         rs->installed_fb.rt_count,
         tile_cfg->max_rt_bpp,
         tile_cfg->ms,
         tile_cfg->double_buffer
         );

#ifdef KHRN_GEOMD
      fmem_debug_info_insert(&rs->fmem,
         khrn_fmem_hw_address(&rs->fmem, instr),
         ~0, khrn_hw_render_state_allocated_order(rs));
#endif

      v3d_cl_flush_vcd_cache(&instr);

      v3d_cl_start_tile_binning(&instr);

      /* see GFXT-9 for an explanation:
       * http://jira.cam.broadcom.com/jira/browse/GFXT-9?focusedCommentId=121814#comment-121814 */
      v3d_cl_prim_list_format(&instr, 3, /*xy=*/false, /*d3dpvsf=*/false);

      /* Reset transform feedback counter to 0 (0 here is a special value meaning
       * reset to 0 -- 1 does not mean reset to 1!) */
      v3d_cl_wait_transform_feedback(&instr, 0);

      v3d_cl_occlusion_query_counter_enable(&instr, 0);

      khrn_fmem_end_cle_exact(&rs->fmem, instr);
   }

   // restore historical state for this start point
   uint8_t* branch_sub_ptr = draw_clist_ptr;
   if (start_record != NULL)
   {
      // use subroutine address from record
      branch_sub_ptr = start_record->start_sub_addr;

      // apply state from record
      if (!glxx_cl_record_apply(start_record, &rs->fmem))
         return false;
   }

   {
      uint8_t* instr = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_BRANCH_SIZE + V3D_CL_FLUSH_SIZE + V3D_CL_WAIT_PREV_FRAME_SIZE);
      if (!instr)
         return false;

      // jump to start of draw control list (if there was anything to draw, otherwise we're just clearing)
      if (branch_sub_ptr != NULL)
      {
         v3d_cl_branch_sub(&instr, khrn_fmem_hw_address(&rs->fmem, branch_sub_ptr));
      }

      // Terminate tile control lists if necessary
      v3d_cl_flush(&instr);

      khrn_fmem_end_cle(&rs->fmem, instr);
   }

   return true;
}

// timh-todo: Maybe this can be refactored/combined with the regular
// create/destroy paths, I don't really like the duplication
bool glxx_hw_render_state_discard_and_restart(
   GLXX_SERVER_STATE_T *state,
   GLXX_HW_RENDER_STATE_T *rs)
{
   GLXX_HW_FRAMEBUFFER_T *installed_fb = &rs->installed_fb;
   unsigned b;
   bool ok = true;

   khrn_fmem_common_persist* fmem_common = khrn_fmem_get_common_persist(&rs->fmem);
   assert(fmem_common->occlusion_query_list == NULL);
   assert(rs->tf_used == false);

   if (!khrn_fmem_reset(&rs->fmem, (KHRN_RENDER_STATE_T*)rs))
      return false;

   if (  !khrn_fmem_record_fence_to_signal(&rs->fmem, state->fences.fence)
      || !khrn_fmem_record_fence_to_depend_on(&rs->fmem, state->fences.fence_to_depend_on) )
      return false;

   for (b = 0; b < installed_fb->rt_count; ++b)
   {
      ok = glxx_hw_record_write_if_image(&rs->fmem, &installed_fb->color[b]);
      if (!ok)
         return false;

      ok = glxx_hw_record_write_if_image(&rs->fmem, &installed_fb->color_ms[b]);
      if (!ok)
         return false;
   }
   ok = glxx_hw_record_write_if_image(&rs->fmem, &installed_fb->depth);
   if (!ok)
      return false;

   ok = glxx_hw_record_write_if_image(&rs->fmem, &installed_fb->stencil);
   if (!ok)
      return false;

   // set all dirty flags for the server-state so they are flushed through
   glxx_server_invalidate_for_render_state(state, rs);
   rs->flat_shading_flags_set = false;
   rs->centroid_flags_set = false;

   // initialise record keeping (this needs deduplicating, hopefully with this whole function)
   if (GLXX_MULTICORE_BIN_ENABLED)
   {
      memset(&rs->cl_records[0], 0, sizeof(rs->cl_records[0]));
      rs->num_cl_records = 0;
      rs->cl_record_threshold = GLXX_MULTICORE_BIN_SPLIT_THRESHOLD/4;
      rs->cl_record_remaining = GLXX_MULTICORE_BIN_SPLIT_THRESHOLD/4;
   }
#ifndef NDEBUG
   rs->cl_records[0].in_begin = GLXX_CL_STATE_NUM;
#endif

   glxx_hw_render_state_reset_z_prepass(rs);

   // reset workaround state
   rs->workaround_gfxh_1313 = false;

   // reset uses flow control state
   rs->fmem.br_info.bin_workaround_gfxh_1181 = false;
   rs->fmem.br_info.render_workaround_gfxh_1181 = false;
   rs->fmem.br_info.secure = egl_context_gl_secure(state->context);

   return true;
}

void glxx_hw_render_state_rw(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs)
{
   unsigned i = 0;
   glxx_att_index_t att_index;
   while (glxx_fb_iterate_valid_draw_bufs(state->bound_draw_framebuffer, &i, &att_index))
   {
      unsigned b = att_index - GLXX_COLOR0_ATT;
      if (state->color_write & (0xf << (b * 4)))
         glxx_bufstate_rw(&rs->color_buffer_state[b]);
   }

   if (state->caps.depth_test && (state->depth_func != GL_NEVER))
   {
      if (state->depth_mask)
         glxx_bufstate_rw(&rs->depth_buffer_state);
      else if (state->depth_func != GL_ALWAYS)
         glxx_bufstate_read(&rs->depth_buffer_state);
   }

   // TODO Could do more fine grained checking if stencil is effectively on or off
   if (state->caps.stencil_test)
      glxx_bufstate_rw(&rs->stencil_buffer_state);
}

static bool create_master_cls(GLXX_HW_RENDER_STATE_T *rs,
   const struct glxx_hw_tile_cfg *tile_cfg,
   const struct glxx_hw_tile_list_rcfg *rcfg,
   v3d_addr_t const generic_tile_list[2])
{
   unsigned num_cores = khrn_get_num_render_subjobs();
   // Buffer writes are not suitably coherent on multiple cores and frame isolation
   // also disables multicore.
   if (rs->base.has_buffer_writes || khrn_options.isolate_frame == khrn_fmem_frame_i)
      num_cores = 1;

   for (unsigned core = 0; core != num_cores; ++core)
   {
      assert(core < V3D_MAX_RENDER_SUBJOBS);
      rs->fmem.br_info.render_begins[core] = khrn_fmem_begin_clist(&rs->fmem);
      if (!rs->fmem.br_info.render_begins[core] ||
         !create_master_cl(rs, tile_cfg, rcfg, generic_tile_list, num_cores, core))
      {
         return false;
      }
      rs->fmem.br_info.render_ends[core] = khrn_fmem_end_clist(&rs->fmem);
   }
   rs->fmem.br_info.num_renders = num_cores;

   return true;
}

static bool create_bin_cls(GLXX_HW_RENDER_STATE_T *rs, const struct glxx_hw_tile_cfg *tile_cfg, uint8_t* draw_clist_ptr)
{
   /* GFXH-1179 workaround */
   rs->fmem.br_info.bin_offset = 4096 - (4095 &
      gfx_uround_up_p2(
         c_initial_tile_alloc_block_size * tile_cfg->num_tiles_x * tile_cfg->num_tiles_y,
         c_tile_alloc_block_size
         ));

   // can only enable multi-core binning if we don't have transform feedback waits in CL
   unsigned max_cores = rs->do_multicore_bin && rs->tf_waited_count == 0 ? khrn_get_num_bin_subjobs() : 1;

   // estimated total number of shaded vertices in control list
   uint64_t bin_cost_total = rs->cl_records[rs->num_cl_records].bin_cost_cumulative;
   uint64_t bin_cost_remaining = bin_cost_total;
   unsigned next_record = 0u;
   bool do_z_prepass = rs->z_prepass_started;
   GLXX_CL_RECORD_T* record = NULL;

   unsigned core = 0;
   for (; core != max_cores; ++core)
   {
      // find start of draw list for next core
      if (GLXX_MULTICORE_BIN_ENABLED && core != 0)
      {
         // if the remaining work exceeds the threshold, then divide between up to all remaining cores
         // this isn't the optimal distribution, but it's fairly simple to compute
         if (bin_cost_remaining < GLXX_MULTICORE_BIN_SPLIT_THRESHOLD)
         {
            // no point in splitting further
            break;
         }

         // number of splits 1 less than number of cores remaining
         uint64_t num_splits = max_cores - core;

         // but also limited by amount of work remaining (don't want tiny pieces)
         num_splits = vcos_min(num_splits, bin_cost_remaining / GLXX_MULTICORE_BIN_SPLIT_THRESHOLD);

         // figure out cost to allocate to remaining cores
         uint64_t bin_cost_per_core = bin_cost_remaining / (num_splits + 1);
         uint64_t bin_cost_target = bin_cost_total - bin_cost_remaining + bin_cost_per_core;

         // binary chop (oh for std::lower_bound...)
         for (unsigned end_record = rs->num_cl_records; next_record < end_record; )
         {
            unsigned middle = (next_record + end_record - 1) / 2;
            if (rs->cl_records[middle].bin_cost_cumulative < bin_cost_target)
            {
               next_record = middle + 1;
            }
            else
            {
               end_record = middle;
            }
         }

         // check binary chop worked
         assert(next_record == rs->num_cl_records || rs->cl_records[next_record].bin_cost_cumulative >= bin_cost_target);
         assert(next_record == 0 || rs->cl_records[next_record - 1].bin_cost_cumulative < bin_cost_target);

         // we can't start at the end of the control list, so we can't use any more cores
         if (next_record == rs->num_cl_records)
         {
            break;
         }

         // don't bother splitting if remainder is 1/4 of the split threshold
         record = &rs->cl_records[next_record];
         bin_cost_remaining = bin_cost_total - record->bin_cost_cumulative;
         if (bin_cost_remaining < (GLXX_MULTICORE_BIN_SPLIT_THRESHOLD/4))
         {
            break;
         }

         // insert return at split point
         uint8_t* instr = record->start_sub_addr - V3D_CL_NOP_SIZE;
         static_assrt(V3D_CL_NOP_SIZE == V3D_CL_RETURN_SIZE);
         v3d_cl_return(&instr);

         // split after this record
         next_record += 1;

         // track which is the first core to not require z-prepass rendering
         if (do_z_prepass && record->z_prepass_stopped)
         {
            rs->num_z_prepass_bins = core;
            do_z_prepass = false;
         }
      }

      assert(core < V3D_MAX_BIN_SUBJOBS);
      rs->fmem.br_info.bin_begins[core] = khrn_fmem_begin_clist(&rs->fmem);
      if (!rs->fmem.br_info.bin_begins[core] ||
         !create_bin_cl(rs, tile_cfg, draw_clist_ptr, record))
      {
         return false;
      }
      rs->fmem.br_info.bin_ends[core] = khrn_fmem_end_clist(&rs->fmem);

      rs->fmem.br_info.min_initial_bin_block_size = gfx_uround_up_p2(
            tile_cfg->num_tiles_x * tile_cfg->num_tiles_y *
            c_initial_tile_alloc_block_size, V3D_TILE_ALLOC_ALIGN) +
            /* If HW runs out of memory immediately it may not raise an out of
             * memory interrupt (it only raises one on the transition from having
             * memory to not having memory). Avoid this by ensuring it starts with
             * at least one block. */
            V3D_TILE_ALLOC_GRANULARITY;
   }
   rs->fmem.br_info.num_bins = core;

   // all bin jobs have z-prepass rendering enabled
   if (do_z_prepass)
   {
      rs->num_z_prepass_bins = core;
   }

   // all ok
   return true;
}

static void invalidate_interlocks_based_on_bufstates(GLXX_HW_RENDER_STATE_T *rs)
{
   GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;

   for (unsigned int b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      bool undefined = glxx_bufstate_is_undefined(rs->color_buffer_state[b]);
      if (undefined)
         khrn_image_plane_invalidate(&fb->color[b]);
      if (undefined || rs->color_discard_ms)
         /* We rely on the invalid bit being set correctly on the multisample
          * buffer -- we use the validity of the multisample buffer to
          * determine whether to load from it or the downsampled buffer */
         verif(khrn_image_plane_invalidate(&fb->color_ms[b]));
   }

   bool depth_undefined = glxx_bufstate_is_undefined(rs->depth_buffer_state);
   bool stencil_undefined = glxx_bufstate_is_undefined(rs->stencil_buffer_state);
   if (depth_undefined && stencil_undefined)
      khrn_image_plane_invalidate_two(&fb->depth, &fb->stencil);
   else
   {
      if (depth_undefined)
         khrn_image_plane_invalidate(&fb->depth);
      if (stencil_undefined)
         khrn_image_plane_invalidate(&fb->stencil);
   }
}

static bool calc_tile_cfg_and_prep_tile_list_fb_ops(
   struct glxx_hw_tile_cfg *tile_cfg,
   struct glxx_hw_tile_list_fb_ops *fb_ops,
   GLXX_HW_RENDER_STATE_T *rs)
{
   tile_cfg->ms = khrn_options.force_multisample || rs->installed_fb.ms;

   if (!glxx_hw_prep_tile_list_fb_ops(fb_ops, rs, tile_cfg->ms))
      return false;

   tile_cfg->double_buffer = khrn_options.nonms_double_buffer && !tile_cfg->ms &&
      /* No point in enabling double buffering if there are loads; see
       * GFXH-1396 */
      !glxx_hw_tile_list_any_loads(fb_ops);

   tile_cfg->max_rt_bpp = V3D_RT_BPP_32;
   for (unsigned b = 0; b != rs->installed_fb.rt_count; ++b)
      if (rs->installed_fb.color_internal_bpp[b] > tile_cfg->max_rt_bpp) /* Assume V3D_RT_BPP_* values are sensibly ordered... */
         tile_cfg->max_rt_bpp = rs->installed_fb.color_internal_bpp[b];

   uint32_t tile_w, tile_h;
   v3d_tile_size_pixels(&tile_w, &tile_h,
      tile_cfg->ms, tile_cfg->double_buffer,
      rs->installed_fb.rt_count, tile_cfg->max_rt_bpp);
   tile_cfg->num_tiles_x = gfx_udiv_round_up(rs->installed_fb.width, tile_w);
   tile_cfg->num_tiles_y = gfx_udiv_round_up(rs->installed_fb.height, tile_h);

   return true;
}

void glxx_hw_render_state_flush(GLXX_HW_RENDER_STATE_T *rs)
{
   khrn_render_state_begin_flush((KHRN_RENDER_STATE_T*)rs);

   log_trace_rs(rs, "\n[glxx_hw_render_state_flush] begin");

   /* Interlock transfer delayed until we know that we can't fail */

   // we we had any draw commands
   uint8_t *draw_clist_ptr = rs->fmem.cle_first;
   if (draw_clist_ptr != NULL)
   {
      // add return from sub-list
      uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_RETURN_SIZE);
      if (!instr)
         goto quit;
      v3d_cl_return(&instr);
      khrn_fmem_end_cle_exact(&rs->fmem, instr);
   }

   // end the current control list
   khrn_fmem_end_clist(&rs->fmem);

   struct glxx_hw_tile_cfg tile_cfg;
   struct glxx_hw_tile_list_fb_ops tile_list_fb_ops;
   if (!calc_tile_cfg_and_prep_tile_list_fb_ops(&tile_cfg, &tile_list_fb_ops, rs))
      goto quit;

   // create the binning control lists for each core
   if (!create_bin_cls(rs, &tile_cfg, draw_clist_ptr))
      goto quit;

   // create the generic tile list (must be done after binning control lists
   // have been created!)
   v3d_addr_t generic_tile_list[2];
   struct glxx_hw_tile_list_rcfg rcfg;
   if (!glxx_hw_create_generic_tile_list(generic_tile_list, &rcfg,
         rs, tile_cfg.ms, tile_cfg.double_buffer, &tile_list_fb_ops))
      goto quit;

   // create the render control lists for each core
   if (!create_master_cls(rs, &tile_cfg, &rcfg, generic_tile_list))
      goto quit;

   khrn_stats_record_event(KHRN_STATS_INTERNAL_FLUSH);

#ifdef KHRN_GEOMD
   if (khrn_options.geomd)
      geomd_open();
#endif

   // submits bin and render
   khrn_fmem_flush(&rs->fmem);

   invalidate_interlocks_based_on_bufstates(rs);

   glxx_hw_render_state_delete(rs);
   return;

quit:
   /* todo: report this error */
   log_trace_rs(rs, "[glxx_hw_render_state_flush] frame discarded");

   glxx_hw_discard_frame(rs);

   return;
}

v3d_compare_func_t glxx_hw_convert_test_function(GLenum function)
{
   switch (function)
   {
   case GL_NEVER   : return V3D_COMPARE_FUNC_NEVER   ;
   case GL_LESS    : return V3D_COMPARE_FUNC_LESS    ;
   case GL_EQUAL   : return V3D_COMPARE_FUNC_EQUAL   ;
   case GL_LEQUAL  : return V3D_COMPARE_FUNC_LEQUAL  ;
   case GL_GREATER : return V3D_COMPARE_FUNC_GREATER ;
   case GL_NOTEQUAL: return V3D_COMPARE_FUNC_NOTEQUAL;
   case GL_GEQUAL  : return V3D_COMPARE_FUNC_GEQUAL  ;
   case GL_ALWAYS  : return V3D_COMPARE_FUNC_ALWAYS  ;
   default:
      unreachable();
      return V3D_COMPARE_FUNC_INVALID;
   }
}

// This function is called when we fail to do what we were asked to do.
// We simply give up any unflushed rendering done so far.
void glxx_hw_discard_frame(GLXX_HW_RENDER_STATE_T *rs)
{
   assert(rs != NULL);

   // Do the tidying up that glxx_hw_flush and hw_callback do
   // but without flushing to the hardware

   log_trace("[hw_discard_frame]");

   khrn_fmem_discard(&rs->fmem);
   glxx_hw_render_state_delete(rs);
}
