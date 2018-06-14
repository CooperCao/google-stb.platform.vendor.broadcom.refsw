/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
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
#include "libs/core/v3d/v3d_supertile.h"
#include "libs/util/gfx_util/gfx_util_morton.h"

#include "libs/platform/gmem.h"

//#define GLXX_INNER_DEBUG 1

#include <stdio.h>

#include "../egl/egl_platform.h"
#include "../common/khrn_process.h"
#include "../common/khrn_resource.h"
#include "../common/khrn_counters.h"
#include "../common/khrn_render_state.h"
#include "../common/khrn_fmem.h"

LOG_DEFAULT_CAT("glxx_inner")

static const uint32_t c_initial_tile_alloc_block_size = V3D_TILE_ALLOC_BLOCK_SIZE_MIN;
static const uint32_t c_tile_alloc_block_size = 128;

static void log_trace_rs(const GLXX_HW_RENDER_STATE_T *rs, const char *context)
{
   if (!log_trace_enabled())
      return;

   const GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;
   log_trace(
      "[%s] ord = %u, label = %s, rt count = %u, fb size %u x %u, %s", context,
      khrn_hw_render_state_allocated_order(rs),
      "-",
      fb->rt_count,
      fb->width, fb->height,
      fb->ms ? "ms" : "non-ms");
   for (unsigned b = 0; b < V3D_MAX_RENDER_TARGETS; ++b)
      if (rs->color_buffer_state[b] != GLXX_BUFSTATE_MISSING)
      {
         GFX_LFMT_SPRINT(lfmt_desc, khrn_image_plane_lfmt_maybe(&fb->color[b]));
         GFX_LFMT_SPRINT(ms_lfmt_desc, khrn_image_plane_lfmt_maybe(&fb->color_ms[b]));
         log_trace("[%s] color %u lfmt=%s ms_lfmt=%s state=%s", context,
            b, lfmt_desc, ms_lfmt_desc, glxx_bufstate_desc(rs->color_buffer_state[b]));
      }

   GFX_LFMT_SPRINT(depth_lfmt_desc, khrn_image_plane_lfmt_maybe(&fb->depth));
   log_trace("[%s] depth lfmt=%s state=%s", context, depth_lfmt_desc, glxx_bufstate_desc(rs->depth_buffer_state));
   GFX_LFMT_SPRINT(stencil_lfmt_desc, khrn_image_plane_lfmt_maybe(&fb->stencil));
   log_trace("[%s] stencil lfmt=%s state=%s", context, stencil_lfmt_desc, glxx_bufstate_desc(rs->stencil_buffer_state));

   for (unsigned i = 0; i != rs->num_blits; ++i)
   {
      const GLXX_BLIT_T *blit = &rs->tlb_blits[i];
      if (blit->color)
      {
         for (uint32_t b = 0, mask = blit->color_draw_to_buf; mask; ++b, mask >>= 1)
         {
            if (mask & 1)
            {
               GFX_LFMT_SPRINT(dst_lfmt_desc, khrn_image_plane_lfmt(&blit->dst_fb.color[b]));
               log_trace("[%s] blit color %u --> %s", context, blit->color_read_buffer, dst_lfmt_desc);
            }
         }
      }
      if (blit->depth)
         log_trace("[%s] blit depth", context);
   }
}

static bool begin_img_plane_write(glxx_bufstate_t *bufstate, khrn_fmem *fmem,
   const khrn_image_plane *img_plane, khrn_changrps_t changrps)
{
   if (img_plane->image)
   {
      bool invalid;
      if (!khrn_fmem_record_resource_write(fmem,
            khrn_image_get_resource(img_plane->image), KHRN_STAGE_RENDER,
            khrn_image_plane_resource_parts(img_plane, changrps, /*subset=*/false),
            &invalid))
         return false;
      if (bufstate)
         *bufstate = glxx_bufstate_begin(invalid);
   }
   else if (bufstate)
      *bufstate = GLXX_BUFSTATE_MISSING;
   return true;
}

static void glxx_hw_render_state_delete(GLXX_HW_RENDER_STATE_T *rs)
{
   // ensure server isn't pointing to us after we're deleted
   if (rs->server_state->current_render_state == rs)
      rs->server_state->current_render_state = NULL;

   glxx_destroy_hw_framebuffer(&rs->installed_fb);

   for (unsigned i = 0; i < GLXX_Q_COUNT; i++)
   {
      KHRN_MEM_ASSIGN(rs->last_started_query[i].query, NULL);
      rs->last_started_query[i].instance = 0;
   }

   KHRN_MEM_ASSIGN(rs->tf.last_used, NULL);

   for (unsigned i=0; i<rs->num_blits; i++) {
      glxx_destroy_hw_framebuffer(&rs->tlb_blits[i].dst_fb);
   }
   rs->num_blits = 0;
   khrn_render_state_delete((khrn_render_state *)rs);
}

void glxx_hw_render_state_reset_z_prepass(GLXX_HW_RENDER_STATE_T *rs)
{
   rs->z_prepass_started = false;
   rs->z_prepass_stopped = false;
   rs->z_prepass_allowed = khrn_options.z_prepass && rs->installed_fb.depth.image != NULL;
}

static void init_clear_colors(uint32_t clear_colors[V3D_MAX_RENDER_TARGETS][4])
{
#if !defined(NDEBUG)
   // Bright pink background indicates undefined framebuffer contents.
   // You did not clear after swap buffers.
   float color[4] = {1.0, 0.0, 0.5, 1.0}; /*R,G,B,A */
#else
   float color[4] = {0.0, 0.0, 0.0, 1.0}; /*R,G,B,A */
#endif

   for (unsigned b = 0; b < V3D_MAX_RENDER_TARGETS; b++)
   {
      for (unsigned i = 0; i < 4; i++)
         clear_colors[b][i] = gfx_float_to_bits(color[i]);
   }
}

bool glxx_hw_start_frame_internal(GLXX_HW_RENDER_STATE_T *rs,
      const GLXX_HW_FRAMEBUFFER_T *fb)
{
   if (!khrn_fmem_init(&rs->fmem, (khrn_render_state *) rs))
      return false;

   /* Record security status */
   rs->fmem.br_info.details.secure = egl_context_gl_secure(rs->server_state->context);

   init_clear_colors(rs->clear_colors);
   rs->num_used_layers = 1;

   glxx_assign_hw_framebuffer(&rs->installed_fb, fb);

   for (unsigned b = 0; b < V3D_MAX_RENDER_TARGETS; ++b)
   {
      glxx_bufstate_t nonms_bufstate;
      if (!begin_img_plane_write(&nonms_bufstate, &rs->fmem, &fb->color[b], KHRN_CHANGRP_ALL))
         return false;

      glxx_bufstate_t ms_bufstate;
      if (!begin_img_plane_write(&ms_bufstate, &rs->fmem, &fb->color_ms[b], KHRN_CHANGRP_ALL))
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
#if KHRN_DEBUG
         assert(nonms_bufstate == GLXX_BUFSTATE_MEM || khrn_options.no_invalidate_fb);
#else
         assert(nonms_bufstate == GLXX_BUFSTATE_MEM);
#endif
         rs->color_load_from_ms[b] = true;
         rs->color_buffer_state[b] = GLXX_BUFSTATE_MEM;
         break;
      default:
         unreachable();
      }
   }

   if (!begin_img_plane_write(&rs->depth_buffer_state, &rs->fmem, &fb->depth, KHRN_CHANGRP_NONSTENCIL))
      return false;

   if (!begin_img_plane_write(&rs->stencil_buffer_state, &rs->fmem, &fb->stencil, KHRN_CHANGRP_STENCIL))
      return false;

   //  Default to true. Will be disabled if installed while dither is disabled
   rs->dither        = true;
   rs->depth_value   = 1.0f;
   rs->stencil_value = 0;

   early_z_init(&rs->ez, khrn_options.early_z);

   // initialise record keeping (assume rs is zero filled already)
   if (GLXX_MULTICORE_BIN_ENABLED)
   {
      rs->cl_record_threshold = GLXX_MULTICORE_BIN_SPLIT_THRESHOLD/4;
      rs->cl_record_remaining = GLXX_MULTICORE_BIN_SPLIT_THRESHOLD/4;
   }
#ifndef NDEBUG
   rs->cl_records[0].in_begin = GLXX_CL_STATE_NUM;
#endif

   glxx_hw_render_state_reset_z_prepass(rs);

   log_trace_rs(rs, __FUNCTION__);

   return true;
}

struct glxx_hw_tile_cfg
{
   bool ms, double_buffer;
   v3d_rt_bpp_t max_rt_bpp;
   unsigned num_tiles_x, num_tiles_y;
   unsigned tile_w, tile_h;
};

struct glxx_hw_supertile_cfg
{
   unsigned frame_w_in_supertiles, frame_h_in_supertiles;
   unsigned supertile_w_in_tiles, supertile_h_in_tiles;
   unsigned supertile_w, supertile_h;
};

static bool create_render_cl_common(GLXX_HW_RENDER_STATE_T *rs,
   const struct glxx_hw_tile_cfg *tile_cfg,
   const struct glxx_hw_supertile_cfg *supertile_cfg,
   const struct glxx_hw_tile_list_rcfg *rcfg,
   bool multicore)
{
   GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;

   size_t r_size = 0;  // Render list size

   // Rendering mode config
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // common
#if V3D_VER_AT_LEAST(4,1,34,0)
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // color
#else
   r_size += fb->rt_count * V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;  // color
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // z_stencil
   if (rcfg->sep_stencil)
      r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;              // sep_stencil
#endif
   for (unsigned b = 0; b < fb->rt_count; ++b)
      r_size += v3d_cl_rcfg_clear_colors_size(fb->color_rt_format[b].bpp);
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                 // zs clear values

   // Dummy tiles for clearing the TLB and working around HW bugs
#if V3D_VER_AT_LEAST(4,1,34,0)
   unsigned clear_tiles = tile_cfg->double_buffer ? 2 : 1;
   unsigned dummy_tiles = clear_tiles;
#if !V3D_HAS_GFXH1742_FIX
   // Need at least 2 dummy tiles to workaround GFXH-1742
   dummy_tiles = gfx_umax(dummy_tiles, 2);
#endif
   {
      size_t dummy_size =
         V3D_CL_TILE_COORDS_SIZE +
         V3D_CL_END_LOADS_SIZE +
         V3D_CL_STORE_SIZE +
         V3D_CL_END_TILE_SIZE;
      r_size += (dummy_tiles * dummy_size) + (clear_tiles * V3D_CL_CLEAR_SIZE);
   }
#else
   r_size += V3D_CL_TILE_COORDS_SIZE;
   r_size += V3D_CL_STORE_GENERAL_SIZE;
#endif
#if !V3D_VER_AT_LEAST(4,2,13,0)
   // Workaround GFXH-1320 & GFXH-1636 if occlusion queries were used in this control list.
   bool fill_ocq_cache = rs->fmem.persist->occlusion_query_list != NULL;
   if (fill_ocq_cache)
      r_size += glxx_fill_ocq_cache_size()
#if V3D_VER_AT_LEAST(3,3,0,0) && !V3D_VER_AT_LEAST(4,1,34,0)
         + V3D_CL_TILE_COORDS_SIZE
         + V3D_CL_STORE_GENERAL_SIZE
#endif
         ;
#endif

   r_size += V3D_CL_CLEAR_VCD_CACHE_SIZE;
   r_size += V3D_CL_TILE_LIST_INITIAL_BLOCK_SIZE_SIZE;
   r_size += V3D_CL_MULTICORE_RENDERING_SUPERTILE_CFG_SIZE;
   r_size += V3D_CL_RETURN_SIZE;

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, r_size);
   if (!instr)
      return false;
#if KHRN_GEOMD
   uint8_t* start_instr = instr;
   v3d_addr_t start_addr = khrn_fmem_cle_addr(&rs->fmem) - r_size;
#endif

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
#if V3D_VER_AT_LEAST(4,1,34,0)
      depth_type,
      rcfg->early_ds_clear
#else
      rcfg->stencil_store,
      rcfg->depth_store,
      ~rcfg->rt_store_mask & gfx_mask(V3D_MAX_RENDER_TARGETS)
#endif
      );

#if V3D_VER_AT_LEAST(4,1,34,0)
   {
      V3D_CL_TILE_RENDERING_MODE_CFG_T i;
      i.type = V3D_RCFG_TYPE_COLOR;
      for (unsigned b = 0; b < V3D_MAX_RENDER_TARGETS; ++b)
      {
         if (b < fb->rt_count)
            i.u.color.rt_formats[b] = fb->color_rt_format[b];
         else
         {
            i.u.color.rt_formats[b].bpp = V3D_RT_BPP_32;
            i.u.color.rt_formats[b].type = V3D_RT_TYPE_8;
            i.u.color.rt_formats[b].clamp = V3D_RT_CLAMP_NONE;
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
         fb->color_rt_format[b].bpp,
         fb->color_rt_format[b].type,
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

      khrn_image const *img = fb->color_ms[b].image ? fb->color_ms[b].image : fb->color[b].image;
      if (img)
      {
#if !V3D_VER_AT_LEAST(4,1,34,0) // If V3D_VER_AT_LEAST(4,1,34,0), v3d_cl_rcfg_clear_colors will do this for us
         /* If the render target is ufloat then clamp negative colours to 0 */
         if ((img->api_fmt & GFX_LFMT_TYPE_MASK) == GFX_LFMT_TYPE_UFLOAT)
            for (int i=0; i<4; i++)
               clear_colors[i] = gfx_float_to_bits(fmaxf(gfx_float_from_bits(clear_colors[i]), 0));
#endif

         /* If the render target does not have an alpha channel, we need alpha in
          * the TLB to be 1 to get correct blending. We ensure this by explicitly
          * clearing to 1 at the start, and by disabling alpha writes so it
          * doesn't change. Note that we must check api_fmt since the actual lfmt
          * might contain alpha when the original internalformat did not. */
         unsigned alpha_bits = gfx_lfmt_alpha_bits(img->api_fmt);
         if (alpha_bits == 0)
            clear_colors[3] = gfx_float_to_bits(1.0f);
         else if (gfx_lfmt_alpha_type(img->api_fmt) == GFX_LFMT_TYPE_UNORM &&
                  (alpha_bits == 1 || alpha_bits == 2)  )
         {
            /* Quantise alpha to a representable value. We only convert alpha so
             * that colour can be dithered. We don't dither 1 or 2 bit alpha anyway. */
            float alpha = gfx_float_from_bits(clear_colors[3]);
            uint32_t clamped = gfx_float_to_unorm(alpha, alpha_bits);
            alpha = gfx_unorm_to_float(clamped, alpha_bits);
            clear_colors[3] = gfx_float_to_bits(alpha);
         }
      }

#if !V3D_VER_AT_LEAST(4,1,34,0)
      const struct v3d_tlb_ldst_params *ls = &rcfg->rt_ls[b];
#endif
      v3d_cl_rcfg_clear_colors(&instr, b, clear_colors,
         &fb->color_rt_format[b]
#if !V3D_VER_AT_LEAST(4,1,34,0)
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
      start_addr + (instr - start_instr),
      ~0, khrn_hw_render_state_allocated_order(rs));
#endif

   // Dummy tiles for clearing the TLB and working around HW bugs
#if V3D_VER_AT_LEAST(4,1,34,0)
   for (unsigned i = 0; i != dummy_tiles; ++i)
   {
      v3d_cl_tile_coords(&instr, 0, 0);
      v3d_cl_end_loads(&instr);
#if !V3D_VER_AT_LEAST(4,2,13,0)
      if ((i == 0) && fill_ocq_cache && !glxx_fill_ocq_cache(&instr, &rs->fmem, rs->installed_fb.width, rs->installed_fb.height))
         return false;
#endif
      v3d_cl_store(&instr, /* Dummy store... */
         V3D_LDST_BUF_NONE,
         V3D_MEMORY_FORMAT_RASTER,
         /*flipy=*/false,
         V3D_DITHER_OFF,
         V3D_DECIMATE_SAMPLE0,
         V3D_PIXEL_FORMAT_SRGB8_ALPHA8,
         /*clear=*/false,
         /*chan_reverse=*/false,
         /*rb_swap=*/false,
         /*stride=*/0,
         /*height=*/0,
         /*addr=*/0);
      if (i < clear_tiles)
         v3d_cl_clear(&instr, /*rts=*/true, /*depth_stencil=*/!rcfg->early_ds_clear);
      v3d_cl_end_tile(&instr);
   }
#else
   // GFXH-1668 means we cannot do the ocq cache filling in the first tile on
   // 3.3, so need two dummy tiles. Note we always want at least one dummy tile
   // to workaround GFXH-1742.
   unsigned dummy_tiles = (fill_ocq_cache && V3D_VER_AT_LEAST(3,3,0,0)) ? 2 : 1;
   for (unsigned i = 0; i != dummy_tiles; ++i)
   {
      bool last = i == (dummy_tiles - 1);
      v3d_cl_tile_coords(&instr, 0, 0);
      if (last && fill_ocq_cache && !glxx_fill_ocq_cache(&instr, &rs->fmem, rs->installed_fb.width, rs->installed_fb.height))
         return false;
      v3d_cl_store_general(&instr,
         V3D_LDST_BUF_NONE,                  // buffer
         false,                              // raw_mode
         !last,                              // disable_depth_clear
         !last,                              // disable_stencil_clear
         !last,                              // disable_color_clear
         false,                              // eof
         true,                               // disable_double_buf_swap
         V3D_LDST_MEMORY_FORMAT_UIF_NO_XOR,  // memory_format
         0,                                  // pad
         0);                                 // addr
   }
#endif

   v3d_cl_clear_vcd_cache(&instr);

   v3d_cl_tile_list_initial_block_size(&instr,
      v3d_translate_tile_alloc_block_size(c_initial_tile_alloc_block_size),
      /*chain=*/true);

   v3d_cl_multicore_rendering_supertile_cfg(&instr,
      supertile_cfg->supertile_w_in_tiles,
      supertile_cfg->supertile_h_in_tiles,
      supertile_cfg->frame_w_in_supertiles,
      supertile_cfg->frame_h_in_supertiles,
      tile_cfg->num_tiles_x,
      tile_cfg->num_tiles_y,
      multicore,
      V3D_SUPERTILE_ORDER_RASTER, // TODO enable morton
      rs->fmem.br_info.bin_subjobs.num_subjobs);

   v3d_cl_return(&instr);

   khrn_fmem_end_cle(&rs->fmem, instr);

   return true;
}

static v3d_size_t calc_tile_state_size(const struct glxx_hw_tile_cfg *tile_cfg, uint32_t num_used_layers)
{
   return num_used_layers * tile_cfg->num_tiles_x * tile_cfg->num_tiles_y * V3D_TILE_STATE_SIZE;
}

static bool create_bin_cl(GLXX_HW_RENDER_STATE_T *rs, const struct glxx_hw_tile_cfg *tile_cfg, uint32_t num_used_layers,
      GLXX_CL_RECORD_T* start_record)
{
   log_trace("[create_bin_cl] rs %u, layers %u num_tiles %u x %u, rt count = %u",
      khrn_hw_render_state_allocated_order(rs),
      rs->installed_fb.layers,
      tile_cfg->num_tiles_x, tile_cfg->num_tiles_y,
      rs->installed_fb.rt_count);

   {
    #if !V3D_VER_AT_LEAST(4,1,34,0)
      // allocate tile state memory
      v3d_addr_t tile_state_addr = khrn_fmem_tile_state_alloc(&rs->fmem, calc_tile_state_size(tile_cfg, num_used_layers));
      if (!tile_state_addr)
         return false;
    #endif

      unsigned size = 0
#if V3D_VER_AT_LEAST(4,1,34,0)
         + V3D_CL_NUM_LAYERS_SIZE
#endif
         + V3D_CL_TILE_BINNING_MODE_CFG_SIZE
         + V3D_CL_OCCLUSION_QUERY_COUNTER_ENABLE_SIZE
         + V3D_CL_CLEAR_VCD_CACHE_SIZE
         + V3D_CL_START_TILE_BINNING_SIZE
         + V3D_CL_WAIT_TRANSFORM_FEEDBACK_SIZE;
      uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, size);
      if (!instr)
         return false;
#if KHRN_GEOMD
      uint8_t* start_instr = instr;
      v3d_addr_t start_addr = khrn_fmem_cle_addr(&rs->fmem) - size;
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
      // this must come before bin mode cfg part
      v3d_cl_num_layers(&instr, num_used_layers);
#else
      assert(num_used_layers == 1);
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
      v3d_cl_tile_binning_mode_cfg(&instr,
         v3d_translate_tile_alloc_block_size(c_initial_tile_alloc_block_size),
         v3d_translate_tile_alloc_block_size(c_tile_alloc_block_size),
         rs->installed_fb.rt_count,
         tile_cfg->max_rt_bpp,
         tile_cfg->ms,
         tile_cfg->double_buffer,
         rs->installed_fb.width,
         rs->installed_fb.height);
#else
      v3d_cl_tile_binning_mode_cfg_part1(&instr,
         /*auto_init_tile_state=*/true,
         v3d_translate_tile_alloc_block_size(c_initial_tile_alloc_block_size),
         v3d_translate_tile_alloc_block_size(c_tile_alloc_block_size),
         tile_state_addr,
         tile_cfg->num_tiles_x,
         tile_cfg->num_tiles_y,
         rs->installed_fb.rt_count,
         tile_cfg->max_rt_bpp,
         tile_cfg->ms,
         tile_cfg->double_buffer);
#endif

#ifdef KHRN_GEOMD
      fmem_debug_info_insert(&rs->fmem,
         start_addr + (instr - start_instr),
         ~0, khrn_hw_render_state_allocated_order(rs));
#endif

      v3d_cl_clear_vcd_cache(&instr);

      v3d_cl_start_tile_binning(&instr);

      /* Reset transform feedback counter to 0 (0 here is a special value meaning
       * reset to 0 -- 1 does not mean reset to 1!) */
      v3d_cl_wait_transform_feedback(&instr, 0);

      v3d_cl_occlusion_query_counter_enable(&instr, 0);

      khrn_fmem_end_cle_exact(&rs->fmem, instr);
   }

   // restore historical state for this start point
   v3d_addr_t branch_sub_addr = rs->clist_start;
   if (start_record != NULL)
   {
      // use subroutine address from record
      branch_sub_addr = start_record->start_sub_addr;

      // apply state from record
      if (!glxx_cl_record_apply(start_record, &rs->fmem))
         return false;
   }

   {
      uint8_t* instr = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_BRANCH_SIZE + V3D_CL_FLUSH_SIZE + V3D_CL_WAIT_PREV_FRAME_SIZE);
      if (!instr)
         return false;

      // jump to start of draw control list (if there was anything to draw, otherwise we're just clearing)
      if (branch_sub_addr != 0)
         v3d_cl_branch_sub(&instr, branch_sub_addr);

      // Terminate tile control lists if necessary
      v3d_cl_flush(&instr);

      khrn_fmem_end_cle(&rs->fmem, instr);
   }

   return true;
}

void glxx_hw_render_state_rw(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs)
{
   if (state->caps.rasterizer_discard)
      return;
   rs->has_rasterization = true;

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

static bool create_render_subjob_cl(GLXX_HW_RENDER_STATE_T *rs,
      const struct glxx_hw_supertile_cfg *supertile_cfg,
      uint32_t core, uint32_t num_cores)
{
   size_t r_size = V3D_CL_END_RENDER_SIZE;
   if (rs->has_rasterization || rs->num_blits != 0)
      r_size += V3D_CL_SUPERTILE_COORDS_SIZE * supertile_cfg->frame_w_in_supertiles * supertile_cfg->frame_h_in_supertiles;

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, r_size);
   if (!instr)
      return false;

   if (rs->has_rasterization || rs->num_blits != 0)
   {
      if (khrn_options.isolate_frame == khrn_fmem_frame_i)
      {
         // isolate requested supertile
         v3d_cl_supertile_coords(&instr,
            gfx_umin(khrn_options.isolate_supertile_x, supertile_cfg->frame_w_in_supertiles - 1u),
            gfx_umin(khrn_options.isolate_supertile_y, supertile_cfg->frame_h_in_supertiles - 1u));
      }
      else
      {
         uint32_t morton_flags, begin_supertile, end_supertile;
         v3d_supertile_range(&morton_flags, &begin_supertile, &end_supertile,
                             num_cores, core,
                             supertile_cfg->frame_w_in_supertiles, supertile_cfg->frame_h_in_supertiles,
                             khrn_options.partition_supertiles_in_sw,
                             khrn_options.all_cores_same_st_order);

         GFX_MORTON_STATE_T morton;
         gfx_morton_init(&morton, supertile_cfg->frame_w_in_supertiles,
               supertile_cfg->frame_h_in_supertiles, morton_flags);

         GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;

         uint32_t st_w = supertile_cfg->supertile_w;
         uint32_t st_h = supertile_cfg->supertile_h;

         uint32_t x, y;
         for (unsigned i = 0; gfx_morton_next(&morton, &x, &y, NULL); ++i)
            if (i >= begin_supertile && i < end_supertile)
            {
               bool want_supertile = (fb->num_damage_rects <= 0);

               if (!want_supertile)
               {
                  gfx_rect supertile_rect = { x * st_w, y * st_h, st_w, st_h };

                  for (int32_t r = 0; r < fb->num_damage_rects; r++)
                  {
                     want_supertile = gfx_do_rects_intersect(&supertile_rect, &fb->damage_rects[r]);
                     if (want_supertile)
                        break;
                  }
               }

               if (want_supertile)
                  v3d_cl_supertile_coords(&instr, x, y);
            }
      }
   }

   v3d_cl_end_render(&instr);
   khrn_fmem_end_cle(&rs->fmem, instr);
   return true;
}

static void calc_supertile_cfg(struct glxx_hw_supertile_cfg *supertile_cfg,
      const struct glxx_hw_tile_cfg *tile_cfg)
{
   supertile_cfg->supertile_w_in_tiles = 0;
   supertile_cfg->supertile_h_in_tiles = 0;
   v3d_choose_supertile_sizes(tile_cfg->num_tiles_x, tile_cfg->num_tiles_y,
         khrn_options.min_supertile_w, khrn_options.min_supertile_h,
         khrn_options.max_supertiles, &supertile_cfg->supertile_w_in_tiles,
         &supertile_cfg->supertile_h_in_tiles);

   supertile_cfg->supertile_w = supertile_cfg->supertile_w_in_tiles * tile_cfg->tile_w;
   supertile_cfg->supertile_h = supertile_cfg->supertile_h_in_tiles * tile_cfg->tile_h;

   supertile_cfg->frame_w_in_supertiles =
      gfx_udiv_round_up(tile_cfg->num_tiles_x, supertile_cfg->supertile_w_in_tiles);
   supertile_cfg->frame_h_in_supertiles =
      gfx_udiv_round_up(tile_cfg->num_tiles_y, supertile_cfg->supertile_h_in_tiles);
}


static bool create_render_cls(GLXX_HW_RENDER_STATE_T *rs,
   const struct glxx_hw_tile_cfg *tile_cfg,
   const struct glxx_hw_tile_list_fb_ops *tile_list_fb_ops)
{
   unsigned num_cores = glxx_hw_render_state_num_render_subjobs_per_layer(rs);
   assert(num_cores <= V3D_MAX_CORES);

   struct glxx_hw_supertile_cfg supertile_cfg;
   calc_supertile_cfg(&supertile_cfg, tile_cfg);

   v3d_addr_range subjobs[V3D_MAX_CORES];
   for (unsigned core = 0; core != num_cores; ++core)
   {
      subjobs[core].begin =  khrn_fmem_begin_clist(&rs->fmem);
      if (!subjobs[core].begin || !create_render_subjob_cl(rs, &supertile_cfg, core, num_cores))
         return false;
      subjobs[core].end =  khrn_fmem_end_clist(&rs->fmem);
   }

   unsigned subindex = 0;
   v3d_addr_range render_common = {0,}; //initialise to stop compiler complaining
#if V3D_VER_AT_LEAST(4,1,34,0)
   struct glxx_hw_tile_list_rcfg prev_rcfg;
#endif
   v3d_subjob *render_subjobs = rs->fmem.br_info.render_subjobs.subjobs;

   for (unsigned layer = 0; layer < rs->num_used_layers; ++layer)
   {
      v3d_addr_range generic_tile_list;
      struct glxx_hw_tile_list_rcfg rcfg;
      if (!glxx_hw_create_generic_tile_list(&generic_tile_list,
               &rcfg, rs, tile_cfg->ms, tile_cfg->double_buffer,
               tile_list_fb_ops, layer))
         return false;

#if !V3D_VER_AT_LEAST(4,1,34,0)
      assert(layer == 0);
#endif
      /* if we have blits, rcfg for layer 0 can end up being different than
       * the ones for the other layers, since we blit only from level 0 in a layerd fb;
       * (see how rcfg->early_ds_clear gets set in write_stores_clears_end_and_rcfg);
       * if we do not have blits, we can create a common cl once,
       * otherwise, one common cl for layer 0 and one common cl for the other
       * layers */
      if (layer == 0
#if V3D_VER_AT_LEAST(4,1,34,0)
            || !glxx_hw_tile_list_rcfgs_equal(&rcfg, &prev_rcfg)
#endif
         )
      {
         render_common.begin = khrn_fmem_begin_clist(&rs->fmem);
         if (!render_common.begin || !create_render_cl_common(rs, tile_cfg, &supertile_cfg, &rcfg, num_cores > 1))
            return false;
         render_common.end = khrn_fmem_end_clist(&rs->fmem);
      }

      //per each core
      size_t r_size = V3D_CL_BRANCH_SIZE; //branch to common
      r_size += V3D_CL_GENERIC_TILE_LIST_SIZE;
      r_size += V3D_CL_BRANCH_SIZE; // branch to cl for each core

      for (unsigned core = 0; core != num_cores; ++core)
      {
         assert(core < V3D_MAX_RENDER_SUBJOBS);

         render_subjobs[subindex].start = khrn_fmem_begin_clist(&rs->fmem);
         if (!render_subjobs[subindex].start)
            return false;

         uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, r_size);
         if (!instr)
            return false;
         v3d_cl_branch_sub(&instr, render_common.begin);
         v3d_cl_generic_tile_list(&instr, generic_tile_list.begin, generic_tile_list.end);
         v3d_cl_branch(&instr, subjobs[core].begin);
         khrn_fmem_end_cle(&rs->fmem, instr);
         khrn_fmem_end_clist(&rs->fmem);
         render_subjobs[subindex].end = subjobs[core].end;
         ++subindex;
      }
#if V3D_VER_AT_LEAST(4,1,34,0)
      prev_rcfg = rcfg;
#endif
   }

   return true;
}

static bool create_bin_cls(GLXX_HW_RENDER_STATE_T *rs, const struct glxx_hw_tile_cfg *tile_cfg)
{
   unsigned max_cores = glxx_hw_render_state_max_bin_subjobs(rs);
   assert(max_cores <=  rs->fmem.br_info.bin_subjobs.num_subjobs);

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
         num_splits = gfx_umin64(num_splits, bin_cost_remaining / GLXX_MULTICORE_BIN_SPLIT_THRESHOLD);

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
         uint8_t* instr = record->ret_sub_ptr;
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
      v3d_subjob *bin_subjobs = rs->fmem.br_info.bin_subjobs.subjobs;
      bin_subjobs[core].start = khrn_fmem_begin_clist(&rs->fmem);
      if (!bin_subjobs[core].start ||
         !create_bin_cl(rs, tile_cfg, rs->num_used_layers, record))
      {
         return false;
      }
      bin_subjobs[core].end = khrn_fmem_end_clist(&rs->fmem);
   }

   rs->fmem.br_info.bin_subjobs.num_subjobs = core;
   rs->fmem.br_info.details.tile_alloc_layer_stride = tile_cfg->num_tiles_x * tile_cfg->num_tiles_y *
            c_initial_tile_alloc_block_size;
   rs->fmem.br_info.details.min_initial_bin_block_size = gfx_uround_up_p2(
         rs->num_used_layers * rs->fmem.br_info.details.tile_alloc_layer_stride, V3D_TILE_ALLOC_ALIGN) +
         /* If HW runs out of memory immediately it may not raise an out of
          * memory interrupt (it only raises one on the transition from having
          * memory to not having memory). Avoid this by ensuring it starts with
          * at least one block. */
         V3D_TILE_ALLOC_GRANULARITY;

#if V3D_VER_AT_LEAST(4,1,34,0)
   rs->fmem.br_info.details.bin_tile_state_size = calc_tile_state_size(tile_cfg, rs->num_used_layers);
#endif

   // all bin jobs have z-prepass rendering enabled
   if (do_z_prepass)
   {
      rs->num_z_prepass_bins = core;
   }

   // all ok
   return true;
}

static void invalidate_resources_based_on_bufstates(GLXX_HW_RENDER_STATE_T *rs)
{
   GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;

   for (unsigned int b = 0; b < V3D_MAX_RENDER_TARGETS; ++b)
   {
      bool undefined = glxx_bufstate_is_undefined(rs->color_buffer_state[b]);
      if (undefined)
         khrn_image_plane_invalidate(&fb->color[b], KHRN_CHANGRP_ALL);
      if (undefined || rs->color_discard_ms)
      {
         /* We rely on the multisample buffer having its own invalid bit and
          * thus always being able to set it correctly -- we use the validity
          * of the multisample buffer to determine whether to load from it or
          * the downsampled buffer */
         assert(khrn_image_plane_resource_parts(&fb->color_ms[b], KHRN_CHANGRP_ALL, /*subset=*/true) ==
            khrn_image_plane_resource_parts(&fb->color_ms[b], KHRN_CHANGRP_ALL, /*subset=*/false));
         khrn_image_plane_invalidate(&fb->color_ms[b], KHRN_CHANGRP_ALL);
      }
   }

   if (glxx_bufstate_is_undefined(rs->depth_buffer_state))
      khrn_image_plane_invalidate(&fb->depth, KHRN_CHANGRP_NONSTENCIL);
   if (glxx_bufstate_is_undefined(rs->stencil_buffer_state))
      khrn_image_plane_invalidate(&fb->stencil, KHRN_CHANGRP_STENCIL);
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
      if (rs->installed_fb.color_rt_format[b].bpp > tile_cfg->max_rt_bpp) /* Assume V3D_RT_BPP_* values are sensibly ordered... */
         tile_cfg->max_rt_bpp = rs->installed_fb.color_rt_format[b].bpp;

   v3d_tile_size_pixels(&tile_cfg->tile_w, &tile_cfg->tile_h,
      tile_cfg->ms, tile_cfg->double_buffer,
      rs->installed_fb.rt_count, tile_cfg->max_rt_bpp);
   tile_cfg->num_tiles_x = gfx_udiv_round_up(rs->installed_fb.width, tile_cfg->tile_w);
   tile_cfg->num_tiles_y = gfx_udiv_round_up(rs->installed_fb.height, tile_cfg->tile_h);

   return true;
}

static bool create_cls_and_flush(GLXX_HW_RENDER_STATE_T *rs)
{
   struct glxx_hw_tile_cfg tile_cfg;
   struct glxx_hw_tile_list_fb_ops tile_list_fb_ops;
   if (!calc_tile_cfg_and_prep_tile_list_fb_ops(&tile_cfg, &tile_list_fb_ops, rs))
      return false;

   rs->fmem.br_info.bin_subjobs.num_subjobs = glxx_hw_render_state_max_bin_subjobs(rs);
   rs->fmem.br_info.bin_subjobs.subjobs = alloca(rs->fmem.br_info.bin_subjobs.num_subjobs * sizeof(v3d_subjob));

   rs->fmem.br_info.num_layers = rs->num_used_layers;
   rs->fmem.br_info.render_subjobs.num_subjobs = rs->num_used_layers *
      glxx_hw_render_state_num_render_subjobs_per_layer(rs);
   rs->fmem.br_info.render_subjobs.subjobs = alloca(rs->fmem.br_info.render_subjobs.num_subjobs * sizeof(v3d_subjob));

   // create the binning control lists for each core
   if (!create_bin_cls(rs, &tile_cfg))
      return false;

   // create the render control lists
   // generic tile list must be done after binning control lists
   // have been created - because we are using num_bin_subjobs in
   // write_tile_list_branches)
   if (!create_render_cls(rs, &tile_cfg, &tile_list_fb_ops))
      return false;

#ifdef KHRN_GEOMD
   if (khrn_options.geomd)
      geomd_open();
#endif

   // submits bin and render
   khrn_fmem_flush(&rs->fmem);
   return true;
}

void glxx_hw_render_state_flush(GLXX_HW_RENDER_STATE_T *rs)
{
   khrn_render_state_begin_flush((khrn_render_state*)rs);

   log_trace_rs(rs, __FUNCTION__);

   // we we had any draw commands
   if (rs->clist_start)
   {
#if V3D_VER_AT_LEAST(4,1,34,0)
      /* write prim counts feedback if we have any prim_gen or prim_written
       * queries that were not written back yet */
      if (rs->last_started_query[GLXX_Q_PRIM_GEN].query ||
            rs->last_started_query[GLXX_Q_PRIM_WRITTEN].query)
      {
         if (!glxx_write_prim_counts_feedback(rs))
            goto quit;
      }

      if (!glxx_store_tf_buffers_state(rs))
         goto quit;

      /* Need to disable TF at end of frame to avoid issues with bin queueing.
       * See SWVC5-718. */
      glxx_tf_record_disable(rs);
#endif

      // add return from sub-list
      uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_RETURN_SIZE);
      if (!instr)
         goto quit;
      v3d_cl_return(&instr);
      khrn_fmem_end_cle_exact(&rs->fmem, instr);

      khrn_fmem_end_clist(&rs->fmem);
   }

   if (!create_cls_and_flush(rs))
      goto quit;

   invalidate_resources_based_on_bufstates(rs);

   glxx_hw_render_state_delete(rs);
   return;

quit:
   /* todo: report this error */
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
// It is also called to discard any rendering that is completely cleared over the top.
void glxx_hw_discard_frame(GLXX_HW_RENDER_STATE_T *rs)
{
   assert(rs != NULL);

   // Do the tidying up that glxx_hw_flush and hw_callback do
   // but without flushing to the hardware

   log_trace("[hw_discard_frame]");

   khrn_fmem_discard(&rs->fmem);
   glxx_hw_render_state_delete(rs);
}
