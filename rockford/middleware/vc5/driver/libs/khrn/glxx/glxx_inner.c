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
#include "../common/khrn_int_color.h"
#include "../common/khrn_options.h"

#include "glxx_server_internal.h"
#include "glxx_translate.h"
#include "glxx_hw.h"
#include "glxx_hw_master_cl.h"
#include "glxx_inner.h"
#include "gl_public_api.h"

#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/v3d/v3d_util.h"

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
#include "../common/khrn_control_list.h"

LOG_DEFAULT_CAT("glxx_inner")

static const uint32_t c_initial_tile_alloc_block_size = V3D_TILE_ALLOC_BLOCK_SIZE_MIN;
static const uint32_t c_tile_alloc_block_size = 128;

static void log_trace_rs(const GLXX_HW_RENDER_STATE_T *rs, const char *msg)
{
   if (!log_trace_enabled())
      return;

   const GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;
   GLXX_BUFSTATE_FLUSH_T depth_flush;
   GLXX_BUFSTATE_FLUSH_T stencil_flush;
   GLXX_BUFSTATE_FLUSH_T color_flush    [GLXX_MAX_RENDER_TARGETS];
   GLXX_BUFSTATE_FLUSH_T color_ms_flush [GLXX_MAX_RENDER_TARGETS];
   unsigned ord = khrn_hw_render_state_allocated_order(rs);
   unsigned b;

   if (rs == NULL)
      return;

   glxx_bufstate_flush(&depth_flush, rs->depth_buffer_state);
   glxx_bufstate_flush(&stencil_flush, rs->stencil_buffer_state);

   for (b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      glxx_bufstate_flush(&color_flush[b], rs->color_buffer_state[b]);
      glxx_bufstate_flush(&color_ms_flush[b], rs->ms_color_buffer_state[b]);
   }

   log_trace(
      "%s [rs] ord = %u, label = %s, rt count = %u, fb size %u x %u, %s",
      msg,
      ord,
      "-",
      fb->rt_count,
      fb->width, fb->height,
      fb->ms               ? "multisample "        : "");
   for (b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
      if (rs->color_buffer_state[b] != GLXX_BUFSTATE_MISSING)
      {
         GFX_LFMT_SPRINT(lfmt_desc, khrn_image_plane_lfmt(&fb->color[b]));
         log_trace(
            "[rs] color %u fmt: %s non-ms: %s %s%s ms: %s %s%s",
            b,
            lfmt_desc,
            glxx_bufstate_desc(rs->color_buffer_state[b]),
            color_flush[b].load ?  "load " : "",
            color_flush[b].store ? "store " : "",
            fb->ms ? glxx_bufstate_desc(rs->ms_color_buffer_state[b]) : "-",
            (fb->ms && color_ms_flush[b].load) ? "load " : "",
            (fb->ms && color_ms_flush[b].store) ? "store " : "");
      }

   log_trace(
      "[rs] depth: %s %s%s",
      glxx_bufstate_desc(rs->depth_buffer_state),
      depth_flush.load ? "load " : "",
      depth_flush.store ? "store " : "");

   log_trace(
      "[rs] stencil: %s %s%s",
      glxx_bufstate_desc(rs->stencil_buffer_state),
      stencil_flush.load ? "load " : "",
      stencil_flush.store ? "store " : "");
}

bool glxx_hw_record_write_if_image(KHRN_FMEM_T *fmem, KHRN_IMAGE_PLANE_T *image_plane)
{
   KHRN_RES_INTERLOCK_T *res_i;

   if (!image_plane->image)
      return true;
   res_i = khrn_image_get_res_interlock(image_plane->image);
   return khrn_fmem_record_res_interlock(fmem, res_i, true, ACTION_RENDER);
}

void glxx_hw_invalidate_internal(GLXX_HW_RENDER_STATE_T *rs, bool
      rt[GLXX_MAX_RENDER_TARGETS], bool color, bool multisample, bool depth, bool
      stencil)
{
   unsigned b;

   for (b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      if (rt[b] == false)
         continue;

      if (color)
         glxx_bufstate_invalidate(&rs->color_buffer_state[b]);

      if (multisample)
         glxx_bufstate_invalidate(&rs->ms_color_buffer_state[b]);
   }

   if (depth)
      glxx_bufstate_invalidate(&rs->depth_buffer_state);

   if (stencil)
      glxx_bufstate_invalidate(&rs->stencil_buffer_state);
}

static bool install_image_plane(KHRN_IMAGE_PLANE_T *src, bool readonly,
   KHRN_IMAGE_PLANE_T *dst, glxx_bufstate_t *buffer_state, KHRN_FMEM_T *fmem)
{
   KHRN_RES_INTERLOCK_T *res_i;
   bool write = !readonly;

   if (!src->image)
   {
      *buffer_state = GLXX_BUFSTATE_MISSING;
      return true;
   }

   khrn_image_plane_assign(dst, src);

   res_i = khrn_image_get_res_interlock(src->image);
   *buffer_state = glxx_bufstate_begin(!khrn_interlock_is_valid(&res_i->interlock));
   return khrn_fmem_record_res_interlock(fmem, res_i, write, ACTION_RENDER);
}

void glxx_hw_render_state_delete(GLXX_HW_RENDER_STATE_T *rs)
{
   // ensure server isn't pointing to us after we're deleted
   if (rs->server_state->current_render_state == rs)
      rs->server_state->current_render_state = NULL;

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
      GLXX_HW_FRAMEBUFFER_T *fb)
{
   bool ok = false;
   unsigned b;

   if (!khrn_fmem_init(&rs->fmem, (KHRN_RENDER_STATE_T *) rs))
      goto out;

   rs->installed_fb.rt_count     = fb->rt_count;
   rs->installed_fb.ms           = fb->ms;
   rs->installed_fb.width        = fb->width;
   rs->installed_fb.height       = fb->height;
   rs->installed_fb.max_bpp      = fb->max_bpp;

   /* Record security status */
   rs->fmem.br_info.secure = egl_context_gl_secure(rs->server_state->context);

   init_clear_colors(rs->clear_colors);

   for (b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      if (!install_image_plane(&fb->color[b], false,
               &rs->installed_fb.color[b], &rs->color_buffer_state[b], &rs->fmem))
            goto out;

      if (!install_image_plane(&fb->color_ms[b], false,
               &rs->installed_fb.color_ms[b], &rs->ms_color_buffer_state[b], &rs->fmem))
            goto out;
   }

   if (!install_image_plane(&fb->depth, false,
               &rs->installed_fb.depth, &rs->depth_buffer_state, &rs->fmem))
            goto out;

   if (!install_image_plane(&fb->stencil, false,
               &rs->installed_fb.stencil, &rs->stencil_buffer_state, &rs->fmem))
            goto out;


   // TODO: To be conservative we should set dither to false if color_load,
   //       otherwise we could wrongly dither a buffer loaded after a flush

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

   ok = true;

out:
   return ok;
}

static uint32_t get_rw_flags_for_tlb(bool load, bool store)
{
   uint32_t rw_flags = 0;

   if (load)
      rw_flags |= GMEM_SYNC_CORE_READ;

   if (store)
      rw_flags |= GMEM_SYNC_CORE_WRITE;

   return rw_flags;
}

static v3d_addr_t lock_image_plane_for_render(GLXX_HW_RENDER_STATE_T *rs, BUFFER_OPS_T const *op)
{
   return khrn_image_plane_lock(
      &op->short_form_img_plane,
      &rs->fmem,
      0,
      get_rw_flags_for_tlb(op->load, op->store)
   );
}

/* Both dims in tiles */
static unsigned next_valid_supertile_dim(unsigned supertile_dim, unsigned frame_dim)
{
   assert(supertile_dim <= frame_dim);
   if (khrn_get_v3d_version() < V3D_MAKE_VER(3,3,0,0))
   {
      /* Avoid supertile sizes which do not divide evenly into the frame size. See GFXH-1325. */
      while ((frame_dim % supertile_dim) != 0)
         ++supertile_dim;
   }
   return supertile_dim;
}

static void choose_supertile_sizes(unsigned tiles_x, unsigned tiles_y, unsigned *supertile_w, unsigned *supertile_h)
{
   // TODO Searching for supertile shape can be better than this
   unsigned frame_w_in_supertiles, frame_h_in_supertiles;

   // Start from min supertile sizes and try growing until we use fewer than the maximum allowed
   *supertile_w = next_valid_supertile_dim(khrn_options.min_supertile_w, tiles_x);
   *supertile_h = next_valid_supertile_dim(khrn_options.min_supertile_h, tiles_y);
   for (;;)
   {
      frame_w_in_supertiles = gfx_udiv_round_up(tiles_x, *supertile_w);
      frame_h_in_supertiles = gfx_udiv_round_up(tiles_y, *supertile_h);
      unsigned supertile_count = frame_w_in_supertiles * frame_h_in_supertiles;
      if (supertile_count <= khrn_options.max_supertiles)
         break;

      if ((*supertile_h < *supertile_w) && (*supertile_h < tiles_y))
         *supertile_h = next_valid_supertile_dim(*supertile_h + 1, tiles_y);
      else
         *supertile_w = next_valid_supertile_dim(*supertile_w + 1, tiles_x);
   }

   // The hardware doesn't support max x 1 supertile configurations
   if (frame_w_in_supertiles == V3D_MAX_SUPERTILES)
      *supertile_w = next_valid_supertile_dim(*supertile_w + 1, tiles_x);
   if (frame_h_in_supertiles == V3D_MAX_SUPERTILES)
      *supertile_h = next_valid_supertile_dim(*supertile_h + 1, tiles_y);

   assert(*supertile_w <= tiles_x);
   assert(*supertile_h <= tiles_y);
}

static v3d_empty_tile_mode check_empty_tile_fill_restrictions(KHRN_IMAGE_PLANE_T const* plane, v3d_addr_t addr)
{
   GFX_LFMT_T lfmt = khrn_image_plane_lfmt(plane);
   bool ok = !gfx_lfmt_get_yflip(&lfmt)                                    // yflip not allowed
      && gfx_lfmt_bytes_per_block(lfmt) != 3                               // 24bpp formats not allowed
      && v3d_addr_aligned(addr | khrn_image_plane_desc(plane)->pitch, 16); // 16 byte row alignment required
   return ok ? V3D_EMPTY_TILE_MODE_FILL : V3D_EMPTY_TILE_MODE_NONE;
}

static bool create_master_cl( GLXX_HW_RENDER_STATE_T *rs,
                              MASTER_CL_INFO_T const *info,
                              v3d_addr_t const *generic_tile_list,
                              uint32_t num_cores, uint32_t core)
{
   RCFG_INFO_T const *rcfg = &info->rcfg;
   GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;

   log_trace_rs(rs, "[create_master_cl]");;

   BUFFER_OPS_T const* stencil_ops = &rcfg->buffers[RCFG_STENCIL];
   bool stencil_rcfg = stencil_ops->short_form_img_plane.image != NULL &&
      (stencil_ops->load || stencil_ops->store);

   // Workaround GFXH-1320 if occlusion queries we used in this control list.
   bool workaround_gfxh_1320 =   khrn_get_v3d_version() < V3D_MAKE_VER(3,3,0,0)
                              && khrn_fmem_get_common_persist(&rs->fmem)->query_list != NULL;

   // Count size
   uint32_t rt_count1 = gfx_umax(fb->rt_count, 1u);
   size_t r_size = 0;  // Render list size
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                       // common
   r_size += rt_count1 * V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;           // color rts
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                       // z or z/s
   r_size += stencil_rcfg ? V3D_CL_TILE_RENDERING_MODE_CFG_SIZE : 0;    // stencil
   r_size += V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;                       // zs clear values
   r_size += V3D_CL_TILE_COORDS_SIZE;                                   // for clear
   r_size += workaround_gfxh_1320 ? glxx_workaround_gfxh_1320_size() : 0;
   r_size += V3D_CL_STORE_GENERAL_SIZE;                                 // for clearing
   r_size += V3D_CL_FLUSH_VCD_CACHE_SIZE;
   r_size += V3D_CL_TILE_LIST_INITIAL_BLOCK_SIZE_SIZE;
   r_size += V3D_CL_MULTICORE_RENDERING_SUPERTILE_CFG_SIZE;
   r_size += V3D_CL_GENERIC_TILE_LIST_SIZE;

   // Calculate size of the clear colour instructions, which depends on BPP and padding.
   for (unsigned b = 0; b < fb->rt_count; ++b)
   {
      GFX_BUFFER_RCFG_COLOR_TRANSLATION_T const *t = &rcfg->color_translation[b];
      r_size += v3d_cl_rcfg_clear_colors_size(t->internal_bpp, t->pad);
   }

   uint32_t supertile_w_in_tiles = 0;
   uint32_t supertile_h_in_tiles = 0;
   choose_supertile_sizes(info->num_tiles_x, info->num_tiles_y, &supertile_w_in_tiles, &supertile_h_in_tiles);
   uint32_t frame_w_in_supertiles = gfx_udiv_round_up(info->num_tiles_x, supertile_w_in_tiles);
   uint32_t frame_h_in_supertiles = gfx_udiv_round_up(info->num_tiles_y, supertile_h_in_tiles);
   r_size += V3D_CL_SUPERTILE_COORDS_SIZE * (frame_w_in_supertiles * frame_h_in_supertiles);
   r_size += V3D_CL_END_RENDER_SIZE;

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, r_size);
   if (!instr)
      return false;

   // Tile Rendering Mode (Common Configuration) (R)
   unsigned short_stores = info->rcfg.tile_ops[RCFG_STORE].short_op_mask;
   v3d_cl_tile_rendering_mode_cfg_common(&instr,
      rt_count1,
      fb->width,
      fb->height,
      fb->max_bpp,
      info->tlb_ms,
      info->tlb_double_buffer,
      false,                                 // Coverage
      rs->ez.rcfg_ez_direction,
      rs->ez.rcfg_ez_disable,
      (short_stores & RCFG_STENCIL_MASK) != 0,                                // short form stencil store
      (short_stores & RCFG_DEPTH_MASK) != 0,                                  // short form depth store
      ~(short_stores >> RCFG_COLOR0) & gfx_mask(GLXX_MAX_RENDER_TARGETS) );   // short form color store disable mask

   // For restricitons on empty tile processing, see CT1CFG Register
   v3d_empty_tile_mode et_mode = V3D_EMPTY_TILE_MODE_NONE;
   if (  khrn_get_v3d_version() >= V3D_MAKE_VER(3,3,0,0)
      && info->rcfg.tile_ops[RCFG_LOAD].num_general_ops == 0
      && info->rcfg.tile_ops[RCFG_STORE].num_general_ops == 0)
   {
      unsigned load_mask = info->rcfg.tile_ops[RCFG_LOAD].short_op_mask;
      unsigned store_mask = info->rcfg.tile_ops[RCFG_STORE].short_op_mask;

      // Skip empty tiles if stores are a subset of loads.
      if ((store_mask & ~load_mask) == 0)
      {
         if (!khrn_options.no_empty_tile_skip)
            et_mode = V3D_EMPTY_TILE_MODE_SKIP;
      }
      // Fill empty tiles if no loads.
      else if (load_mask == 0)
      {
         if (!khrn_options.no_empty_tile_fill && khrn_options.no_gfxh_1385)
            et_mode = V3D_EMPTY_TILE_MODE_FILL;
      }
   }

   // (Color) Render target configuration
   if (fb->rt_count == 0)
   {
      /* HW cannot be configured with 0 RTs. Setup a dummy RT. The values here
       * mostly don't matter but they ought to be valid and not limit the tile
       * size. */
      v3d_cl_tile_rendering_mode_cfg_color(&instr, /*rt=*/0,
         V3D_RT_BPP_32, V3D_RT_TYPE_8, V3D_DECIMATE_SAMPLE0,
         V3D_PIXEL_FORMAT_RGBA8, V3D_DITHER_OFF, V3D_MEMORY_FORMAT_RASTER,
         /*flipy=*/false, /*pad=*/0, /*addr=*/0);
   }
   else for (unsigned b = 0; b < fb->rt_count; ++b)
   {
      GFX_BUFFER_RCFG_COLOR_TRANSLATION_T const *t;
      BUFFER_OPS_T const *op  = &rcfg->buffers[RCFG_COLOR(b)];
      v3d_dither_t   dither   = V3D_DITHER_OFF;
      v3d_addr_t     addr     = 0;
      v3d_decimate_t decimate;

      if (op->short_form_ms)
      {
         assert(fb->ms);
         decimate = V3D_DECIMATE_ALL_SAMPLES;
      }
      else
      {
         decimate = info->tlb_ms ? V3D_DECIMATE_4X : V3D_DECIMATE_SAMPLE0;
      }

      t = &rcfg->color_translation[b];

      if (op->short_form_img_plane.image != NULL)
      {
         addr = lock_image_plane_for_render(rs, op);
         addr += t->addr_offset;

         if (et_mode == V3D_EMPTY_TILE_MODE_FILL)
            et_mode = check_empty_tile_fill_restrictions(&op->short_form_img_plane, addr);

         if (rs->dither && !khrn_options.force_dither_off)
         {
            GFX_LFMT_T lfmt = khrn_image_plane_lfmt(&op->short_form_img_plane);
            if (gfx_lfmt_has_alpha(lfmt) && (gfx_lfmt_alpha_bits(lfmt) > 1))
               dither = V3D_DITHER_RGBA;
            else
               dither = V3D_DITHER_RGB;
         }
      }

      v3d_cl_tile_rendering_mode_cfg_color(&instr,
         b, // render target
         t->internal_bpp,
         t->internal_type,
         decimate,
         t->pixel_format,
         dither,
         t->memory_format,
         t->flipy,
         t->pad,
         addr);
   }

   // Depth / packed depth and stencil
   {
      BUFFER_OPS_T const *op = &rcfg->buffers[RCFG_DEPTH];
      v3d_decimate_t decimate = op->short_form_ms ? V3D_DECIMATE_ALL_SAMPLES : V3D_DECIMATE_SAMPLE0;
      v3d_addr_t addr = 0;
      uint32_t uif_height_in_ub = 0;

      if (op->short_form_img_plane.image != NULL && (op->load || op->store))
      {
         if (et_mode == V3D_EMPTY_TILE_MODE_FILL)
            et_mode = check_empty_tile_fill_restrictions(&op->short_form_img_plane, addr);

         uif_height_in_ub = khrn_image_plane_maybe_uif_height_in_ub(&op->short_form_img_plane);
         addr = lock_image_plane_for_render(rs, op);
      }

      v3d_cl_tile_rendering_mode_cfg_z_stencil(&instr,
         rcfg->depth_type,
         decimate,
         rcfg->depth_output_format,
         rcfg->depth_memory_format,
         uif_height_in_ub,
         addr);
   }

   // Stencil
   if (stencil_rcfg)
   {
      BUFFER_OPS_T const *op = &rcfg->buffers[RCFG_STENCIL];
      v3d_decimate_t decimate = op->short_form_ms ? V3D_DECIMATE_ALL_SAMPLES : V3D_DECIMATE_SAMPLE0;
      v3d_addr_t addr = 0;
      uint32_t uif_height_in_ub = 0;

      assert(op->load || op->store);
      uif_height_in_ub = khrn_image_plane_maybe_uif_height_in_ub(&op->short_form_img_plane);
      addr = lock_image_plane_for_render(rs, op);

      if (et_mode == V3D_EMPTY_TILE_MODE_FILL)
         et_mode = check_empty_tile_fill_restrictions(&op->short_form_img_plane, addr);

      v3d_cl_tile_rendering_mode_cfg_separate_stencil(&instr,
         decimate,
         info->rcfg.stencil_memory_format,
         uif_height_in_ub,
         addr);
   }
   rs->fmem.br_info.empty_tile_mode = et_mode;

   // Color clear values
   for (unsigned b = 0; b < fb->rt_count; ++b)
   {
      GFX_BUFFER_RCFG_COLOR_TRANSLATION_T const *t = &rcfg->color_translation[b];
      KHRN_IMAGE_T const *img = fb->color_ms[b].image ? fb->color_ms[b].image : fb->color[b].image;
      uint32_t clear_colors[4];
      int c;

      for (c = 0; c < 4; ++c)
         clear_colors[c] = rs->clear_colors[b][c];

      // If render target does not have alpha channel, force clear alpha value to 1.0.
      // This is required to get blending work correctly.
      // Unfortunately, we really need to use api_fmt since multisample lfmt
      // might contain alpha and the original internalformat did not.
      if (!img || !gfx_lfmt_has_alpha(img->api_fmt))
         clear_colors[3] = gfx_float_to_bits(1.0f);

      v3d_cl_rcfg_clear_colors(&instr, b, clear_colors,
            t->internal_type, t->internal_bpp,
            t->pad,
            t->clear3_raster_padded_width_or_nonraster_height,
            t->clear3_uif_height_in_ub);
   }

   // Depth and stencil clear values
   // HW needs this to be last V3D_CL_TILE_RENDERING_MODE_CFG sub item
   v3d_cl_tile_rendering_mode_cfg_zs_clear_values(
      &instr,
      rs->stencil_value,
      v3d_snap_depth(rs->depth_value, rcfg->depth_type)
      );

#ifdef KHRN_GEOMD
   fmem_debug_info_insert(&rs->fmem,
      khrn_fmem_hw_address(&rs->fmem, instr),
      ~0, khrn_hw_render_state_allocated_order(rs));
#endif

   v3d_cl_tile_list_initial_block_size(
      &instr,
      v3d_translate_tile_alloc_block_size(c_initial_tile_alloc_block_size),
      /*chain=*/true
      );

   v3d_cl_multicore_rendering_supertile_cfg(&instr,
      supertile_w_in_tiles,
      supertile_h_in_tiles,
      frame_w_in_supertiles,
      frame_h_in_supertiles,
      info->num_tiles_x,
      info->num_tiles_y,
      num_cores > 1,
      V3D_SUPERTILE_ORDER_RASTER,
      rs->fmem.br_info.num_bins);  // TODO enable morton

   v3d_cl_generic_tile_list(&instr, generic_tile_list[0], generic_tile_list[1]);

   // Clear tile buffer
   // Note: If you plan to 'optimize' disable depth and stencil clears
   //       when not needed, be careful with packed depth/stencil
   v3d_cl_tile_coords(&instr, 0, 0);
   if (workaround_gfxh_1320 && !glxx_workaround_gfxh_1320(&instr, &rs->fmem))
      return false;
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

   v3d_cl_flush_vcd_cache(&instr);

   if (khrn_options.isolate_frame == khrn_fmem_frame_i)
   {
      // isolate requested supertile
      v3d_cl_supertile_coords(
         &instr,
         gfx_umin(khrn_options.isolate_supertile_x, frame_w_in_supertiles - 1u),
         gfx_umin(khrn_options.isolate_supertile_y, frame_w_in_supertiles - 1u)
         );
   }
   else
   {
      glxx_hw_populate_master_cl_supertiles(rs, num_cores, core, &instr, frame_w_in_supertiles, frame_h_in_supertiles);
   }

   v3d_cl_end_render(&instr);

   khrn_fmem_end_cle(&rs->fmem, instr);

   return true;
}

static bool create_bin_cl(GLXX_HW_RENDER_STATE_T *rs, MASTER_CL_INFO_T const *info, uint8_t *draw_clist_ptr, GLXX_CL_RECORD_T* start_record)
{
   GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;
   log_trace("[create_bin_cl] rs %u, num_tiles %u x %u, rt count = %u",
      khrn_hw_render_state_allocated_order(rs),
      info->num_tiles_x, info->num_tiles_y,
      fb->rt_count);

   {
      // allocate tile state memory
      v3d_addr_t tile_state_addr = khrn_fmem_tile_state_alloc(&rs->fmem, info->num_tiles_x * info->num_tiles_y * V3D_TILE_STATE_SIZE);
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
         info->num_tiles_x,
         info->num_tiles_y,
         fb->rt_count ? fb->rt_count : 1,
         fb->rt_count ? fb->max_bpp : V3D_RT_BPP_32,
         info->tlb_ms,
         info->tlb_double_buffer
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
   assert(fmem_common->query_list == NULL);
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

   // reset cached vcm cache size (sigh)
   rs->vcm_cache_size_bin = 0;
   rs->vcm_cache_size_render = 0;

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
   bool color     = state->color_write.r || state->color_write.g ||
                    state->color_write.b || state->color_write.a;
   bool depth_any = state->caps.depth_test && (state->depth_func != GL_NEVER);
   bool depth_r   = depth_any && (state->depth_func != GL_ALWAYS);
   bool depth_w   = depth_any && state->depth_mask;
   // TODO Could do more fine grained checking if stencil is effectively on or off
   bool stencil   = state->caps.stencil_test;
   const GLXX_FRAMEBUFFER_T *fb = state->bound_draw_framebuffer;

   if (color)
   {
      glxx_att_index_t att_index;
      unsigned i = 0;
      while (glxx_fb_iterate_valid_draw_bufs(fb, &i, &att_index))
      {
         unsigned b = att_index - GLXX_COLOR0_ATT;
         glxx_bufstate_rw(&rs->color_buffer_state[b]);
         glxx_bufstate_rw(&rs->ms_color_buffer_state[b]);
      }
   }

   if (depth_w)
      glxx_bufstate_rw(&rs->depth_buffer_state);
   else if (depth_r)
      glxx_bufstate_read(&rs->depth_buffer_state);

   if (stencil)
      glxx_bufstate_rw(&rs->stencil_buffer_state);
}

static bool create_master_cls(GLXX_HW_RENDER_STATE_T *rs, MASTER_CL_INFO_T const *info)
{
   // create and re-use generic tile list for each core
   v3d_addr_t tile_list[2];
   {
      uint8_t* tile_list_start_ptr = khrn_fmem_begin_clist(&rs->fmem);
      if (!tile_list_start_ptr || !glxx_hw_generic_tile_list(rs, info))
      {
         return false;
      }
      uint8_t* tile_list_end_ptr = khrn_fmem_end_clist(&rs->fmem);

      // convert to v3d_addr_t
      tile_list[0] = khrn_fmem_hw_address(&rs->fmem, tile_list_start_ptr);
      tile_list[1] = khrn_fmem_hw_address(&rs->fmem, tile_list_end_ptr);
   }

   unsigned num_cores = khrn_get_num_render_subjobs();
   // Buffer writes are not suitably coherent on multiple cores and frame isolation
   // also disables multicore.
   if (rs->base.has_buffer_writes || khrn_options.isolate_frame == khrn_fmem_frame_i)
      num_cores = 1;

   for (unsigned core = 0; core != num_cores; ++core)
   {
      uint8_t* start_ptr = khrn_fmem_begin_clist(&rs->fmem);
      if (!start_ptr || !create_master_cl(rs, info, tile_list, num_cores, core))
      {
         return false;
      }
      uint8_t *end_ptr = khrn_fmem_end_clist(&rs->fmem);

      assert(core < V3D_MAX_RENDER_SUBJOBS);
      rs->fmem.br_info.render_begins[core] = khrn_fmem_hw_address(&rs->fmem, start_ptr);
      rs->fmem.br_info.render_ends[core] = khrn_fmem_hw_address(&rs->fmem, end_ptr);
   }
   rs->fmem.br_info.num_renders = num_cores;

   return true;
}

static bool create_bin_cls(GLXX_HW_RENDER_STATE_T *rs, MASTER_CL_INFO_T const *info, uint8_t* draw_clist_ptr)
{
   /* GFXH-1179 workaround */
   rs->fmem.br_info.bin_offset = 4096 - (4095 &
      gfx_uround_up_p2(
         c_initial_tile_alloc_block_size * info->num_tiles_x * info->num_tiles_y,
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

      uint8_t *start_ptr = khrn_fmem_begin_clist(&rs->fmem);
      if (!start_ptr || !create_bin_cl(rs, info, draw_clist_ptr, record))
      {
         return false;
      }
      uint8_t *end_ptr = khrn_fmem_end_clist(&rs->fmem);

      assert(core < V3D_MAX_BIN_SUBJOBS);
      rs->fmem.br_info.bin_begins[core] = khrn_fmem_hw_address(&rs->fmem, start_ptr);
      rs->fmem.br_info.bin_ends[core] = khrn_fmem_hw_address(&rs->fmem, end_ptr);

      rs->fmem.br_info.min_initial_bin_block_size = round_up(
            info->num_tiles_x * info->num_tiles_y *
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

static void update_image_interlock_from_bufstate(KHRN_IMAGE_PLANE_T *image_plane, glxx_bufstate_t bufstate)
{
   if (image_plane->image == NULL)
      return;

   // This is annoying. We cannot invalidate interlock
   // for image planes when blob might contain something else.
   if (
      (image_plane->image->blob->num_array_elems > 1) ||
      (image_plane->image->blob->num_mip_levels > 1) ||
      (khrn_image_get_num_planes(image_plane->image) > 1)
   )
      return;

   if (glxx_bufstate_is_undefined(bufstate))
   {
      KHRN_RES_INTERLOCK_T *res_i;
      res_i = khrn_image_get_res_interlock(image_plane->image);
      khrn_interlock_invalidate(&res_i->interlock);
   }
}
static void update_image_interlock_from_bufstates(
   KHRN_IMAGE_PLANE_T *image_plane_a,
   KHRN_IMAGE_PLANE_T *image_plane_b,
   glxx_bufstate_t bufstate_a,
   glxx_bufstate_t bufstate_b)
{
   assert(image_plane_a->image != NULL);
   assert(image_plane_b->image != NULL);

   // This is annoying. We cannot invalidate interlock
   // for image planes when blob might contain something else.
   if (
      (image_plane_a->image->blob->num_array_elems > 1)      ||
      (image_plane_a->image->blob->num_mip_levels > 1)       ||
      (khrn_image_get_num_planes(image_plane_a->image) != 2) ||
      (image_plane_a->plane_idx + image_plane_b->plane_idx != 1)
   )
      return;

   if (
      glxx_bufstate_is_undefined(bufstate_a) &&
      glxx_bufstate_is_undefined(bufstate_b)
   )
   {
      KHRN_RES_INTERLOCK_T *res_i;
      res_i = khrn_image_get_res_interlock(image_plane_a->image);
      khrn_interlock_invalidate(&res_i->interlock);
   }
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
      uint8_t *instr = khrn_fmem_cle(&rs->fmem, V3D_CL_RETURN_SIZE);
      if (!instr)
         goto quit;
      v3d_cl_return(&instr);
   }

   // end the current control list
   khrn_fmem_end_clist(&rs->fmem);

   // build master cl info first
   MASTER_CL_INFO_T info;
   glxx_hw_build_master_cl_info(rs, &info);

   // create the binning control lists for each core
   bool ok = create_bin_cls(rs, &info, draw_clist_ptr);

   // create the render control lists for each core
   ok = ok && create_master_cls(rs, &info);

   // destroy master cl info
   glxx_hw_destroy_master_cl_info(&info);

   if (!ok)
      goto quit;

   khrn_stats_record_event(KHRN_STATS_INTERNAL_FLUSH);

   GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;
#ifdef KHRN_GEOMD
   if (khrn_options.geomd)
      geomd_open();
#endif

   // submits bin and render
   khrn_fmem_flush(&rs->fmem);

   for (unsigned int b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      update_image_interlock_from_bufstate(&fb->color[b], rs->color_buffer_state[b]);
      update_image_interlock_from_bufstate(&fb->color_ms[b], rs->ms_color_buffer_state[b]);
   }

   {
      if (fb->depth.image != NULL && fb->stencil.image != NULL) // packed depth and stencil
         update_image_interlock_from_bufstates(&rs->installed_fb.depth, &rs->installed_fb.stencil, rs->depth_buffer_state, rs->stencil_buffer_state);
      else
      {
         update_image_interlock_from_bufstate(&fb->depth, rs->depth_buffer_state);
         update_image_interlock_from_bufstate(&fb->stencil, rs->stencil_buffer_state);
      }
   }

   glxx_destroy_hw_framebuffer(fb);
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

   glxx_destroy_hw_framebuffer(&rs->installed_fb);

   khrn_fmem_discard(&rs->fmem);
   glxx_hw_render_state_delete(rs);
}
